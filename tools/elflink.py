#!/usr/bin/env python3
import struct, sys

PT_LOAD=1; PT_PHDR=6; PT_GNU_STACK=0x6474e551
SHT_SYMTAB=2; SHT_RELA=4; SHT_NOBITS=8; SHT_PROGBITS=1
SHF_ALLOC=2; SHF_EXECINSTR=4; SHF_WRITE=1
STB_LOCAL=0; STB_GLOBAL=1; STB_WEAK=2
R_X86_64_PC32=2; R_X86_64_PLT32=4

class Obj:
    pass

def align(x,a): return (x+a-1)&-a

def read_obj(path):
    data=open(path,'rb').read()
    if data[:4]!=b'\x7fELF' or data[4]!=2:
        raise SystemExit(f'{path}: not ELF64')
    e=struct.unpack_from('<16sHHIQQQIHHHHHH',data,0)
    shoff=e[6]; shentsize=e[11]; shnum=e[12]; shstrndx=e[13]
    secs=[]
    for i in range(shnum):
        off=shoff+i*shentsize
        sh=struct.unpack_from('<IIQQQQIIQQ',data,off)
        secs.append({'name_off':sh[0],'type':sh[1],'flags':sh[2],'addr':sh[3],'offset':sh[4],'size':sh[5],'link':sh[6],'info':sh[7],'align':sh[8],'entsize':sh[9],'data':None})
    shstr=secs[shstrndx]
    shstr_data=data[shstr['offset']:shstr['offset']+shstr['size']]
    def name_at(o):
        end=shstr_data.find(b'\0',o)
        return shstr_data[o:end].decode('latin1')
    for sec in secs:
        sec['name']=name_at(sec['name_off'])
        if sec['type']!=SHT_NOBITS and sec['size']:
            sec['data']=data[sec['offset']:sec['offset']+sec['size']]
    # symbols
    syms=[]
    for sec in secs:
        if sec['type']==SHT_SYMTAB:
            strtab=secs[sec['link']]
            strs=strtab['data'] or b''
            for off in range(0,sec['size'],24):
                st_name,st_info,st_other,st_shndx,st_value,st_size=struct.unpack_from('<IBBHQQ',sec['data'],off)
                end=strs.find(b'\0',st_name)
                nm=strs[st_name:end].decode('latin1') if st_name else ''
                syms.append({'name':nm,'bind':st_info>>4,'type':st_info&15,'shndx':st_shndx,'value':st_value,'size':st_size})
            break
    rels=[]
    for sec in secs:
        if sec['type']==SHT_RELA:
            for off in range(0,sec['size'],24):
                r_offset,r_info,r_addend=struct.unpack_from('<QQq',sec['data'],off)
                rels.append({'target_sec':sec['info'],'offset':r_offset,'sym':r_info>>32,'type':r_info&0xffffffff,'addend':r_addend})
    return {'path':path,'secs':secs,'syms':syms,'rels':rels}

def link(output,entry_name,paths):
    objs=[read_obj(p) for p in paths]
    # output sections
    text_size=0; data_size=0; bss_size=0
    for o in objs:
        for i,s in enumerate(o['secs']):
            if not (s['flags']&SHF_ALLOC): continue
            if s['type']==SHT_PROGBITS and (s['flags']&SHF_EXECINSTR):
                o.setdefault('text_off',{})[i]=align(text_size,s['align'] or 1); text_size=o['text_off'][i]+s['size']
            elif s['type']==SHT_NOBITS:
                o.setdefault('bss_off',{})[i]=align(bss_size,s['align'] or 1); bss_size=o['bss_off'][i]+s['size']
            elif s['type']==SHT_PROGBITS:
                o.setdefault('data_off',{})[i]=align(data_size,s['align'] or 1); data_size=o['data_off'][i]+s['size']
    base=0x400000; text_v=0x401000; text_f=0x1000
    data_v=align(text_v+text_size,0x1000); data_f=align(text_f+text_size,0x1000)
    bss_v=data_v+data_size
    # section bases
    def sec_base(o,idx):
        s=o['secs'][idx]
        if idx in o.get('text_off',{}): return text_v+o['text_off'][idx]
        if idx in o.get('data_off',{}): return data_v+o['data_off'][idx]
        if idx in o.get('bss_off',{}): return bss_v+o['bss_off'][idx]
        return None
    # global symbols
    globals={}
    for o in objs:
        for idx,s in enumerate(o['syms']):
            if s['shndx']==0 or not s['name']: continue
            if s['bind'] not in (STB_GLOBAL,STB_WEAK): continue
            old=globals.get(s['name'])
            if old is None or (old[0]['syms'][old[1]]['bind']==STB_WEAK and s['bind']==STB_GLOBAL):
                globals[s['name']]=(o,idx)
    def sym_addr(o,idx):
        s=o['syms'][idx]
        if s['bind'] != STB_LOCAL and s['name']:
            ent=globals.get(s['name'])
            if ent is not None and not (ent[0] is o and ent[1] == idx):
                return sym_addr(*ent)
        if s['shndx']==0:
            if not s['name']: return None
            ent=globals.get(s['name'])
            if ent is None: return None
            return sym_addr(*ent)
        b=sec_base(o,s['shndx'])
        if b is None: return None
        return b+s['value']
    # build images
    text=bytearray(text_size); data=bytearray(data_size)
    for o in objs:
        for i,s in enumerate(o['secs']):
            if i in o.get('text_off',{}) and s['data'] is not None: off=o['text_off'][i]; text[off:off+s['size']]=s['data']
            elif i in o.get('data_off',{}) and s['data'] is not None: off=o['data_off'][i]; data[off:off+s['size']]=s['data']
    # relocations
    for o in objs:
        for r in o['rels']:
            tgt_sec=o['secs'][r['target_sec']]
            if r['target_sec'] in o.get('text_off',{}):
                dest=text; off=o['text_off'][r['target_sec']]+r['offset']
            elif r['target_sec'] in o.get('data_off',{}):
                dest=data; off=o['data_off'][r['target_sec']]+r['offset']
            else:
                continue
            taddr=sym_addr(o,r['sym'])
            if taddr is None:
                s=o['syms'][r['sym']]
                raise SystemExit(f"unresolved {s['name']} in {o['path']}")
            P=sec_base(o,r['target_sec'])+r['offset']
            if r['type']==1: # ABS64
                val=taddr+r['addend']
                dest[off:off+8]=struct.pack('<Q',val); continue
            if r['type']==10: # ABS32
                val=taddr+r['addend']
                if val < 0 or val >= 2**32:
                    raise SystemExit(f"abs32 overflow {o['path']}")
                dest[off:off+4]=struct.pack('<I',val); continue
            if r['type']==11: # ABS32S
                val=taddr+r['addend']
                if val < -2**31 or val >= 2**31:
                    raise SystemExit(f"abs32s overflow {o['path']}")
                dest[off:off+4]=struct.pack('<i',val); continue
            if r['type'] not in (R_X86_64_PC32,R_X86_64_PLT32):
                raise SystemExit(f"unsupported reloc {r['type']} in {o['path']}")
            val=taddr+r['addend']-P
            if val < -2**31 or val >= 2**31:
                raise SystemExit(f"relocation overflow {o['path']}")
            dest[off:off+4]=struct.pack('<i',val)
    # entry
    if entry_name not in globals: raise SystemExit(f'missing entry {entry_name}')
    entry=sym_addr(*globals[entry_name])
    # ELF header + phdrs
    ehsize=64; phentsize=56; phnum=4
    phoff=ehsize
    hdr_size=ehsize+phentsize*phnum
    file_size=data_f+len(data)
    # segments
    ph=[]
    # PHDR
    ph.append(struct.pack('<IIQQQQQQ',PT_PHDR,5,phoff,base+phoff,base+phoff,hdr_size,hdr_size,8))
    # text load includes headers + text
    ph.append(struct.pack('<IIQQQQQQ',PT_LOAD,5,0,base,base,text_f+len(text),text_f+len(text),0x1000))
    # data load
    if data_size or bss_size:
        ph.append(struct.pack('<IIQQQQQQ',PT_LOAD,6,data_f,data_v,data_v,len(data),len(data)+bss_size,0x1000))
    else:
        ph.append(struct.pack('<IIQQQQQQ',PT_LOAD,6,0,0,0,0,0,0x1000))
    ph.append(struct.pack('<IIQQQQQQ',PT_GNU_STACK,6,0,0,0,0,0,0x10))
    ehdr=struct.pack('<16sHHIQQQIHHHHHH',b'\x7fELF\x02\x01\x01'+b'\0'*9,2,62,1,entry,phoff,0,0,ehsize,phentsize,phnum,64,0,0)
    out=bytearray()
    out+=ehdr+b''.join(ph)
    out+=b'\0'*(text_f-len(out))
    out+=text
    if data_size:
        out+=b'\0'*(data_f-len(out)); out+=data
    open(output,'wb').write(out)
    import os; os.chmod(output,0o755)
    print(f'linked {output}: text={len(text)} data={len(data)} bss={bss_size} entry=0x{entry:x}')

if __name__=='__main__':
    if len(sys.argv)<3:
        print('usage: elflink.py output entry object...'); sys.exit(2)
    link(sys.argv[1],sys.argv[2],sys.argv[3:])
