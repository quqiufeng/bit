#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "pe2elf_stubs.h"

struct unresolved_import {
    char *dll;
    char *name;
    uint64_t iat_rva;
    uint64_t index;
};

static int read16(const unsigned char *d, size_t n, size_t off, uint16_t *v)
{
    if (off > n || n - off < 2)
        return 0;
    *v = (uint16_t)((uint16_t)d[off] | ((uint16_t)d[off + 1] << 8));
    return 1;
}

static int read32(const unsigned char *d, size_t n, size_t off, uint32_t *v)
{
    if (off > n || n - off < 4)
        return 0;
    *v = (uint32_t)d[off] |
         ((uint32_t)d[off + 1] << 8) |
         ((uint32_t)d[off + 2] << 16) |
         ((uint32_t)d[off + 3] << 24);
    return 1;
}

static int read64(const unsigned char *d, size_t n, size_t off, uint64_t *v)
{
    uint32_t lo, hi;
    if (!read32(d, n, off, &lo) || !read32(d, n, off + 4, &hi))
        return 0;
    *v = (uint64_t)lo | ((uint64_t)hi << 32);
    return 1;
}

static void put16(unsigned char *d, size_t off, uint16_t v)
{
    d[off] = (unsigned char)(v & 0xff);
    d[off + 1] = (unsigned char)((v >> 8) & 0xff);
}

static void put32(unsigned char *d, size_t off, uint32_t v)
{
    d[off] = (unsigned char)(v & 0xff);
    d[off + 1] = (unsigned char)((v >> 8) & 0xff);
    d[off + 2] = (unsigned char)((v >> 16) & 0xff);
    d[off + 3] = (unsigned char)((v >> 24) & 0xff);
}

static void put64(unsigned char *d, size_t off, uint64_t v)
{
    put32(d, off, (uint32_t)(v & 0xffffffffu));
    put32(d, off + 4, (uint32_t)(v >> 32));
}

static uint64_t align64(uint64_t x, uint64_t a)
{
    return (x + a - 1) & ~(a - 1);
}

static size_t align_size(size_t x, size_t a)
{
    return (x + a - 1) & ~(a - 1);
}

static char *dup_string(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}

static char *image_cstr(const unsigned char *image, size_t size, uint64_t rva)
{
    size_t start, end;
    char *s;

    if (rva >= size) {
        s = malloc(1);
        if (s)
            s[0] = '\0';
        return s;
    }
    start = (size_t)rva;
    end = start;
    while (end < size && image[end] != 0)
        end++;
    s = malloc(end - start + 1);
    if (!s)
        return NULL;
    memcpy(s, image + start, end - start);
    s[end - start] = '\0';
    return s;
}

static int find_stub_offset(const char *name, unsigned int *off)
{
    size_t i;
    for (i = 0; i < PE2ELF_STUB_OFFSET_COUNT; i++) {
        if (strcmp(name, pe2elf_stub_offsets[i].name) == 0) {
            *off = pe2elf_stub_offsets[i].offset;
            return 1;
        }
    }
    return 0;
}

static int add_unresolved(struct unresolved_import **list, size_t *count,
                          size_t *cap, const char *dll, const char *name,
                          uint64_t iat_rva, uint64_t index)
{
    struct unresolved_import *p;
    size_t ncap;

    if (*count == *cap) {
        ncap = *cap ? *cap * 2 : 8;
        p = realloc(*list, ncap * sizeof(*p));
        if (!p)
            return 0;
        *list = p;
        *cap = ncap;
    }
    p = &(*list)[*count];
    p->dll = dup_string(dll);
    p->name = dup_string(name);
    if (!p->dll || !p->name) {
        free(p->dll);
        free(p->name);
        return 0;
    }
    p->iat_rva = iat_rva;
    p->index = index;
    (*count)++;
    return 1;
}

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *f;
    long end;
    size_t size;
    unsigned char *data;

    f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "pe2elf: cannot open %s\n", path);
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (end = ftell(f)) < 0 ||
        fseek(f, 0, SEEK_SET) != 0) {
        fprintf(stderr, "pe2elf: cannot read %s\n", path);
        fclose(f);
        return NULL;
    }
    size = (size_t)end;
    data = malloc(size ? size : 1);
    if (!data) {
        fprintf(stderr, "pe2elf: out of memory reading %s\n", path);
        fclose(f);
        return NULL;
    }
    if (size != 0 && fread(data, 1, size, f) != size) {
        fprintf(stderr, "pe2elf: cannot read %s\n", path);
        free(data);
        fclose(f);
        return NULL;
    }
    if (fclose(f) != 0) {
        fprintf(stderr, "pe2elf: cannot close %s\n", path);
        free(data);
        return NULL;
    }
    *out_size = size;
    return data;
}

static int write_file(const char *path, const unsigned char *data, size_t size)
{
    FILE *f = fopen(path, "wb");

    if (!f) {
        fprintf(stderr, "pe2elf: cannot open %s\n", path);
        return 0;
    }
    if (fwrite(data, 1, size, f) != size) {
        fprintf(stderr, "pe2elf: cannot write %s\n", path);
        fclose(f);
        return 0;
    }
    if (fclose(f) != 0) {
        fprintf(stderr, "pe2elf: cannot close %s\n", path);
        return 0;
    }
    if (chmod(path, 0755) != 0) {
        fprintf(stderr, "pe2elf: cannot chmod %s\n", path);
        return 0;
    }
    return 1;
}

static void free_unresolved(struct unresolved_import *list, size_t count)
{
    size_t i;
    for (i = 0; i < count; i++) {
        free(list[i].dll);
        free(list[i].name);
    }
    free(list);
}

int main(int argc, char **argv)
{
    char **args;
    int nargs = 0;
    int allow = 0;
    int i;
    const char *c1_output = NULL;
    const char *src;
    const char *dst;
    unsigned char *d = NULL;
    unsigned char *image = NULL;
    unsigned char *stubs = NULL;
    unsigned char *out = NULL;
    size_t dsize = 0;
    size_t image_size;
    size_t image_len;
    size_t stub_len;
    size_t stub_off;
    size_t tramp_off;
    size_t shoff;
    uint32_t pe;
    size_t coff;
    size_t opt;
    uint16_t nsec;
    uint16_t optsz;
    uint16_t magic;
    uint32_t entry;
    uint64_t base;
    uint32_t size;
    uint32_t imp_rva;
    uint32_t imp_size;
    struct unresolved_import *unresolved = NULL;
    size_t unresolved_count = 0;
    size_t unresolved_cap = 0;

    args = malloc((size_t)(argc > 0 ? argc : 1) * sizeof(*args));
    if (!args) {
        fprintf(stderr, "pe2elf: out of memory\n");
        return 1;
    }
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--allow-unresolved") == 0)
            allow = 1;
        else
            args[nargs++] = argv[i];
    }
    for (i = 0; i < nargs; i++) {
        if (strcmp(args[i], "--c1-output") == 0) {
            if (i + 1 >= nargs) {
                fprintf(stderr, "pe2elf: --c1-output needs a value\n");
                free(args);
                return 2;
            }
            c1_output = args[i + 1];
            memmove(&args[i], &args[i + 2],
                    (size_t)(nargs - i - 2) * sizeof(*args));
            nargs -= 2;
            break;
        }
    }
    if (nargs != 2) {
        fprintf(stderr, "usage: pe2elf [--allow-unresolved] "
                        "[--c1-output object] <input.exe> <output.elf>\n");
        free(args);
        return 2;
    }
    src = args[0];
    dst = args[1];
    free(args);

    if (c1_output != NULL && strcmp(c1_output, "object") != 0) {
        fprintf(stderr, "pe2elf: --c1-output only supports object\n");
        return 2;
    }

    d = read_file(src, &dsize);
    if (!d)
        return 1;

    if (dsize < 0x40 || !read32(d, dsize, 0x3c, &pe) ||
        (size_t)pe > dsize || dsize - (size_t)pe < 4 ||
        memcmp(d + pe, "PE\0\0", 4) != 0) {
        fprintf(stderr, "pe2elf: not a PE file\n");
        goto fail;
    }
    coff = (size_t)pe + 4;
    if (!read16(d, dsize, coff + 2, &nsec) ||
        !read16(d, dsize, coff + 16, &optsz)) {
        fprintf(stderr, "pe2elf: truncated PE header\n");
        goto fail;
    }
    opt = coff + 20;
    if (!read16(d, dsize, opt, &magic)) {
        fprintf(stderr, "pe2elf: truncated PE header\n");
        goto fail;
    }
    if (magic != 0x20b) {
        fprintf(stderr, "pe2elf: not PE32+\n");
        goto fail;
    }
    if (!read32(d, dsize, opt + 16, &entry) ||
        !read64(d, dsize, opt + 24, &base) ||
        !read32(d, dsize, opt + 56, &size) ||
        !read32(d, dsize, opt + 120, &imp_rva) ||
        !read32(d, dsize, opt + 124, &imp_size)) {
        fprintf(stderr, "pe2elf: truncated PE optional header\n");
        goto fail;
    }

    image_size = size;
    image_len = image_size ? image_size : 1;
    image = calloc(image_len, 1);
    if (!image) {
        fprintf(stderr, "pe2elf: out of memory\n");
        goto fail;
    }

    {
        size_t sec = opt + (size_t)optsz;
        uint16_t si;

        for (si = 0; si < nsec; si++) {
            size_t s = sec + (size_t)si * 40;
            uint32_t rawsize, rva, rawptr;
            size_t copy;

            if (!read32(d, dsize, s + 16, &rawsize) ||
                !read32(d, dsize, s + 12, &rva) ||
                !read32(d, dsize, s + 20, &rawptr)) {
                fprintf(stderr, "pe2elf: truncated section table\n");
                goto fail;
            }
            copy = rawsize;
            if ((uint64_t)rawptr + copy > dsize) {
                if (rawptr >= dsize)
                    copy = 0;
                else
                    copy = dsize - rawptr;
            }
            if ((uint64_t)rva + copy > image_size) {
                if (rva >= image_size)
                    copy = 0;
                else
                    copy = image_size - rva;
            }
            if (copy)
                memcpy(image + rva, d + rawptr, copy);
        }
    }

    {
        uint64_t need = imp_rva;
        uint64_t limit = (uint64_t)imp_rva + imp_size;

        while (need && need < limit) {
            uint32_t oft, name_rva, ft;
            char *dll;
            char *nm;
            char nmbuf[32];
            uint64_t thunk;
            uint64_t idx;

            if (!read32(image, image_size, (size_t)need, &oft) ||
                !read32(image, image_size, (size_t)need + 12, &name_rva) ||
                !read32(image, image_size, (size_t)need + 16, &ft)) {
                fprintf(stderr, "pe2elf: truncated import table\n");
                goto fail;
            }
            if (ft == 0)
                break;
            dll = image_cstr(image, image_size, name_rva);
            if (!dll) {
                fprintf(stderr, "pe2elf: out of memory\n");
                goto fail;
            }
            thunk = oft ? oft : ft;
            for (idx = 0;; idx++) {
                uint64_t ent, iat_rva;
                unsigned int off;

                if ((uint64_t)thunk + idx * 8 > image_size ||
                    image_size - (size_t)((uint64_t)thunk + idx * 8) < 8) {
                    fprintf(stderr, "pe2elf: truncated import thunk\n");
                    free(dll);
                    goto fail;
                }
                if (!read64(image, image_size,
                            (size_t)((uint64_t)thunk + idx * 8), &ent)) {
                    fprintf(stderr, "pe2elf: truncated import thunk\n");
                    free(dll);
                    goto fail;
                }
                if (ent == 0)
                    break;
                iat_rva = (uint64_t)ft + idx * 8;
                if (ent & (UINT64_C(1) << 63)) {
                    snprintf(nmbuf, sizeof(nmbuf), "#%llu",
                             (unsigned long long)(ent & 0xffff));
                    nm = dup_string(nmbuf);
                } else {
                    nm = image_cstr(image, image_size, ent + 2);
                }
                if (!nm) {
                    fprintf(stderr, "pe2elf: out of memory\n");
                    free(dll);
                    goto fail;
                }
                if (find_stub_offset(nm, &off)) {
                    if (iat_rva + 8 > image_size) {
                        fprintf(stderr, "pe2elf: IAT entry outside image\n");
                        free(nm);
                        free(dll);
                        goto fail;
                    }
                    put64(image, (size_t)iat_rva, UINT64_C(0x900000) + off);
                } else {
                    if (!add_unresolved(&unresolved, &unresolved_count,
                                        &unresolved_cap, dll, nm,
                                        iat_rva, idx)) {
                        fprintf(stderr, "pe2elf: out of memory\n");
                        free(nm);
                        free(dll);
                        goto fail;
                    }
                }
                free(nm);
            }
            free(dll);
            need += 20;
        }
    }

    if (unresolved_count) {
        size_t ui;

        for (ui = 0; ui < unresolved_count; ui++) {
            fprintf(stderr, "pe2elf: unresolved import %s!%s\n",
                    unresolved[ui].dll, unresolved[ui].name);
            fprintf(stderr, "        IAT RVA 0x%04llX, thunk index %llu\n",
                    (unsigned long long)unresolved[ui].iat_rva,
                    (unsigned long long)unresolved[ui].index);
        }
        if (!allow) {
            fprintf(stderr, "pe2elf: failed due to unresolved imports\n");
            goto fail;
        }
        fprintf(stderr, "pe2elf: --allow-unresolved, continuing with "
                        "unresolved IAT entries\n");
    }

    stub_len = PE2ELF_STUB_BLOB_SIZE;
    stubs = malloc(stub_len + 16 + 19);
    if (!stubs) {
        fprintf(stderr, "pe2elf: out of memory\n");
        goto fail;
    }
    memcpy(stubs, pe2elf_stub_blob, stub_len);
    if (c1_output != NULL) {
        unsigned int off;
        if (!find_stub_offset("c1_output", &off) || (size_t)off + 7 > stub_len) {
            fprintf(stderr, "pe2elf: missing c1_output stub data\n");
            goto fail;
        }
        memcpy(stubs + off, "object\0", 7);
    }

    stub_off = (size_t)align64(UINT64_C(0x1000) + size, UINT64_C(0x1000));
    tramp_off = align_size(stub_len, 16);
    if (stub_len < tramp_off)
        memset(stubs + stub_len, 0, tramp_off - stub_len);
    {
        int32_t disp = (int32_t)(UINT64_C(0x910000) -
                                 (UINT64_C(0x900000) +
                                  (uint64_t)tramp_off + 7));
        unsigned char *p = stubs + tramp_off;
        uint64_t entry_addr = base + entry;

        p[0] = 0x48;
        p[1] = 0x89;
        p[2] = 0x25;
        put32(p, 3, (uint32_t)disp);
        p[7] = 0x48;
        p[8] = 0xb8;
        put64(p, 9, entry_addr);
        p[17] = 0xff;
        p[18] = 0xe0;
        stub_len = tramp_off + 19;
    }

    shoff = align_size(stub_off + stub_len, 8);
    out = calloc(shoff ? shoff : 1, 1);
    if (!out) {
        fprintf(stderr, "pe2elf: out of memory\n");
        goto fail;
    }

    out[0] = 0x7f;
    out[1] = 'E';
    out[2] = 'L';
    out[3] = 'F';
    out[4] = 2;
    out[5] = 1;
    out[6] = 1;
    put16(out, 16, 2);
    put16(out, 18, 62);
    put32(out, 20, 1);
    put64(out, 24, UINT64_C(0x900000) + tramp_off);
    put64(out, 32, 64);
    put64(out, 40, 0);
    put32(out, 48, 0);
    put16(out, 52, 64);
    put16(out, 54, 56);
    put16(out, 56, 4);
    put16(out, 58, 64);
    put16(out, 60, 0);
    put16(out, 62, 0);

    put32(out, 64, 1);
    put32(out, 68, 7);
    put64(out, 72, 0x1000);
    put64(out, 80, base);
    put64(out, 88, base);
    put64(out, 96, size);
    put64(out, 104, size);
    put64(out, 112, 0x1000);

    put32(out, 120, 1);
    put32(out, 124, 5);
    put64(out, 128, stub_off);
    put64(out, 136, UINT64_C(0x900000));
    put64(out, 144, UINT64_C(0x900000));
    put64(out, 152, stub_len);
    put64(out, 160, stub_len);
    put64(out, 168, 0x1000);

    put32(out, 176, 1);
    put32(out, 180, 6);
    put64(out, 184, 0);
    put64(out, 192, UINT64_C(0x910000));
    put64(out, 200, UINT64_C(0x910000));
    put64(out, 208, 0);
    put64(out, 216, 0x10000);
    put64(out, 224, 0x1000);

    put32(out, 232, UINT32_C(0x6474e551));
    put32(out, 236, 6);
    put64(out, 240, 0);
    put64(out, 248, 0);
    put64(out, 256, 0);
    put64(out, 264, 0);
    put64(out, 272, 0);
    put64(out, 280, 0x10);

    memcpy(out + 0x1000, image, image_size);
    memcpy(out + stub_off, stubs, stub_len);

    if (!write_file(dst, out, shoff)) {
        goto fail;
    }
    printf("wrote %s %llu\n", dst, (unsigned long long)shoff);

    free_unresolved(unresolved, unresolved_count);
    free(out);
    free(stubs);
    free(image);
    free(d);
    return 0;

fail:
    free_unresolved(unresolved, unresolved_count);
    free(out);
    free(stubs);
    free(image);
    free(d);
    return 1;
}
