#!/usr/bin/env python3
import struct
import sys

import json
import os
HERE = os.path.dirname(os.path.abspath(__file__))
STUBS = open(os.path.join(HERE, 'pe2elf_stubs.bin'), 'rb').read()
OFF = json.load(open(os.path.join(HERE, 'pe2elf_stubs.json')))


def u16(d, o):
    return struct.unpack_from('<H', d, o)[0]


def u32(d, o):
    return struct.unpack_from('<I', d, o)[0]


def u64(d, o):
    return struct.unpack_from('<Q', d, o)[0]


def p32(d, o, v):
    struct.pack_into('<I', d, o, v)


def p64(d, o, v):
    struct.pack_into('<Q', d, o, v)


def align(x, a):
    return (x + a - 1) & -a


def cstr(image, rva):
    if rva < 0 or rva >= len(image):
        return ''
    end = image.find(b'\0', rva)
    if end < 0:
        end = len(image)
    return image[rva:end].decode('latin1')


def main(argv):
    allow = '--allow-unresolved' in argv
    args = [a for a in argv if a != '--allow-unresolved']
    if len(args) != 2:
        sys.stderr.write('usage: pe2elf.py [--allow-unresolved] <input.exe> <output.elf>\n')
        return 2
    src, dst = args

    d = open(src, 'rb').read()
    pe = u32(d, 0x3c)
    if d[pe:pe + 4] != b'PE\0\0':
        sys.stderr.write('pe2elf: not a PE file\n')
        return 1
    coff = pe + 4
    nsec = u16(d, coff + 2)
    optsz = u16(d, coff + 16)
    opt = coff + 20
    if u16(d, opt) != 0x20b:
        sys.stderr.write('pe2elf: not PE32+\n')
        return 1
    entry = u32(d, opt + 16)
    base = u64(d, opt + 24)
    size = u32(d, opt + 56)
    imp_rva = u32(d, opt + 120)
    imp_size = u32(d, opt + 124)

    image = bytearray(size)
    sec = opt + optsz
    for i in range(nsec):
        s = sec + i * 40
        rawsize = u32(d, s + 16)
        rva = u32(d, s + 12)
        rawptr = u32(d, s + 20)
        image[rva:rva + rawsize] = d[rawptr:rawptr + rawsize]

    unresolved = []
    need = imp_rva
    limit = imp_rva + imp_size
    while need and need < limit:
        oft = u32(image, need)
        name_rva = u32(image, need + 12)
        ft = u32(image, need + 16)
        if ft == 0:
            break
        dll = cstr(image, name_rva)
        thunk = oft if oft else ft
        i = 0
        while True:
            ent = u64(image, thunk + i * 8)
            if ent == 0:
                break
            iat_rva = ft + i * 8
            if ent & (1 << 63):
                nm = '#%d' % (ent & 0xffff)
            else:
                nm = cstr(image, ent + 2)
            if nm in OFF:
                p64(image, iat_rva, 0x900000 + OFF[nm])
            else:
                unresolved.append((dll, nm, iat_rva, i))
            i += 1
        need += 20

    if unresolved:
        for dll, nm, iat_rva, idx in unresolved:
            sys.stderr.write('pe2elf: unresolved import %s!%s\n' % (dll, nm))
            sys.stderr.write('        IAT RVA 0x%04X, thunk index %d\n' % (iat_rva, idx))
        if not allow:
            sys.stderr.write('pe2elf: failed due to unresolved imports\n')
            return 1
        sys.stderr.write('pe2elf: --allow-unresolved, continuing with unresolved IAT entries\n')

    stub_off = align(0x1000 + size, 0x1000)
    shoff = align(stub_off + len(STUBS), 8)
    out = bytearray(shoff)
    out[0:4] = b'\x7fELF'
    out[4] = 2
    out[5] = 1
    out[6] = 1
    p32(out, 16, 2)
    p32(out, 18, 62)
    p32(out, 20, 1)
    struct.pack_into('<Q', out, 24, base + entry)
    struct.pack_into('<Q', out, 32, 64)
    struct.pack_into('<Q', out, 40, 0)
    p32(out, 48, 0)
    p32(out, 52, 64)
    p32(out, 54, 56)
    p32(out, 56, 3)
    p32(out, 58, 64)
    p32(out, 60, 0)
    p32(out, 62, 0)
    ph = 64
    struct.pack_into('<IIQQQQQQ', out, ph, 1, 7, 0x1000, base, base, size, size, 0x1000)
    ph += 56
    struct.pack_into('<IIQQQQQQ', out, ph, 1, 5, stub_off, 0x900000, 0x900000, len(STUBS), len(STUBS), 0x1000)
    ph += 56
    struct.pack_into('<IIQQQQQQ', out, ph, 0x6474e551, 6, 0, 0, 0, 0, 0, 0x10)
    out[0x1000:0x1000 + size] = image
    out[stub_off:stub_off + len(STUBS)] = STUBS
    open(dst, 'wb').write(out)
    import os
    os.chmod(dst, 0o755)
    print('wrote', dst, len(out))
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv[1:]))
