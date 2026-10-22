#!/usr/bin/env python3
import struct, sys
STUBS = bytes.fromhex(
 '4889c8c3'  # getstd
 '4889cfb83c0000000f05f4'  # exit
 '57564d89ca4889cf48f7df4883ef0a4889d64c89c231c00f054885c0780b418902b8010000005e5fc3'
 '41c7020000000031c05e5fc3'
 '57564d89ca4889cf48f7df4883ef0a4889d64c89c2b8010000000f054885c0780b418902b8010000005e5fc3'
 '41c7020000000031c05e5fc3')
OFF = {'GetStdHandle':0,'ExitProcess':4,'ReadFile':15,'WriteFile':68}
def u16(d,o): return struct.unpack_from('<H',d,o)[0]
def u32(d,o): return struct.unpack_from('<I',d,o)[0]
def u64(d,o): return struct.unpack_from('<Q',d,o)[0]
def p32(d,o,v): struct.pack_into('<I',d,o,v)
def p64(d,o,v): struct.pack_into('<Q',d,o,v)
def align(x,a): return (x+a-1)&-a
def main(src,dst):
    d=open(src,'rb').read()
    pe=u32(d,0x3c)
    if d[pe:pe+4]!=b'PE\0\0': raise SystemExit('not PE')
    coff=pe+4
    nsec=u16(d,coff+2); optsz=u16(d,coff+16)
    opt=coff+20
    if u16(d,opt)!=0x20b: raise SystemExit('not PE32+')
    entry=u32(d,opt+16); base=u64(d,opt+24); size=u32(d,opt+56)
    imp_rva=u32(d,opt+120); imp_size=u32(d,opt+124)
    image=bytearray(size)
    sec=opt+optsz
    for i in range(nsec):
        s=sec+i*40
        vsize=u32(d,s+8); rva=u32(d,s+12); rawsize=u32(d,s+16); rawptr=u32(d,s+20)
        image[rva:rva+rawsize]=d[rawptr:rawptr+rawsize]
    need=imp_rva
    while need and u64(image,need) or (need and u64(image,need+12)) or (need and u64(image,need+16)):
        oft=u64(image,need); name=u64(image,need+12); ft=u64(image,need+16)
        if ft==0: break
        thunk=oft if oft else ft
        i=0
        while True:
            ent=u64(image,thunk+i*8)
            if ent==0: break
            if not (ent & (1<<63)):
                no=ent
                end=image.find(b'\0',no+2)
                nm=image[no+2:end].decode('latin1')
                if nm in OFF:
                    p64(image,ft+i*8,0x900000+OFF[nm])
            i+=1
        need+=20
    # ELF layout
    stub_off=align(0x1000+size,0x1000)
    shoff=align(stub_off+len(STUBS),8)
    total=shoff
    out=bytearray(total)
    out[0:4]=b'\x7fELF'; out[4]=2; out[5]=1; out[6]=1
    p32=struct.pack_into
    p32('<H',out,16,2); p32('<H',out,18,62); p32('<I',out,20,1)
    struct.pack_into('<Q',out,24,base+entry); struct.pack_into('<Q',out,32,64)
    struct.pack_into('<Q',out,40,0); p32('<I',out,48,0); p32('<H',out,52,64)
    p32('<H',out,54,56); p32('<H',out,56,3); p32('<H',out,58,64); p32('<H',out,60,0); p32('<H',out,62,0)
    ph=64
    # image LOAD
    struct.pack_into('<IIQQQQQQ',out,ph,1,7,0x1000,base,base,size,size,0x1000); ph+=56
    # stubs LOAD
    struct.pack_into('<IIQQQQQQ',out,ph,1,5,stub_off,0x900000,0x900000,len(STUBS),len(STUBS),0x1000); ph+=56
    # stack
    struct.pack_into('<IIQQQQQQ',out,ph,0x6474e551,6,0,0,0,0,0,0x10)
    out[0x1000:0x1000+size]=image
    out[stub_off:stub_off+len(STUBS)]=STUBS
    open(dst,'wb').write(out)
    import os; os.chmod(dst,0o755)
    print('wrote',dst,len(out))
if __name__=='__main__': main(sys.argv[1],sys.argv[2])
