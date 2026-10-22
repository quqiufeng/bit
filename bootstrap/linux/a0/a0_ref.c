#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABELS 65536
#define BUF_SIZE (1u << 20)

struct label {
    uint64_t vaddr;
    uint64_t offset;
    int defined;
};

static struct label labels[MAX_LABELS];
static unsigned char out[BUF_SIZE];
static const char *src;
static size_t src_len;
static size_t pos;
static uint64_t base_vaddr;
static uint64_t cur_vaddr;
static size_t cur_offset;
static size_t max_offset;

static void fail(const char *message) {
    fprintf(stderr, "a0: %s at byte %zu\n", message, pos);
    exit(1);
}

static int hex_value(int c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static uint64_t read_hex(unsigned digits) {
    uint64_t value = 0;
    unsigned i;
    if (digits == 0 || digits > 16)
        fail("invalid hex width");
    for (i = 0; i < digits; i++) {
        int c;
        if (pos >= src_len)
            fail("truncated hex value");
        c = hex_value((unsigned char)src[pos]);
        if (c < 0)
            fail("expected hex digit");
        value = (value << 4) | (uint64_t)c;
        pos++;
    }
    return value;
}

static unsigned count_hex(void) {
    unsigned n = 0;
    while (pos + n < src_len && hex_value((unsigned char)src[pos + n]) >= 0)
        n++;
    return n;
}

static void skip_space(void) {
    while (pos < src_len) {
        unsigned char c = (unsigned char)src[pos];
        if (c == ';') {
            while (pos < src_len && src[pos] != '\n')
                pos++;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            pos++;
            continue;
        }
        break;
    }
}

static void advance(unsigned bytes) {
    cur_offset += bytes;
    cur_vaddr += bytes;
    if (cur_offset > max_offset)
        max_offset = cur_offset;
}

static void scan(int emit) {
    pos = 0;
    base_vaddr = 0;
    cur_vaddr = 0;
    cur_offset = 0;
    max_offset = 0;

    for (;;) {
        unsigned char c;
        skip_space();
        if (pos >= src_len)
            break;
        c = (unsigned char)src[pos];

        if (c == '^') {
            unsigned n;
            pos++;
            n = count_hex();
            base_vaddr = read_hex(n);
            cur_vaddr = base_vaddr + cur_offset;
            continue;
        }

        if (c == ':') {
            uint64_t id;
            pos++;
            id = read_hex(4);
            cur_offset = (size_t)id;
            cur_vaddr = base_vaddr + cur_offset;
            if (id >= MAX_LABELS)
                fail("label id out of range");
            labels[id].vaddr = cur_vaddr;
            labels[id].offset = cur_offset;
            labels[id].defined = 1;
            if (cur_offset > max_offset)
                max_offset = cur_offset;
            continue;
        }

        if (c == '%') {
            uint64_t id;
            uint32_t rel;
            pos++;
            id = read_hex(4);
            if (id >= MAX_LABELS)
                fail("label id out of range");
            if (emit) {
                if (!labels[id].defined)
                    fail("undefined label");
                rel = (uint32_t)(labels[id].vaddr - (cur_vaddr + 4));
                if (cur_offset + 4 > BUF_SIZE)
                    fail("output overflow");
                out[cur_offset + 0] = (unsigned char)(rel & 0xff);
                out[cur_offset + 1] = (unsigned char)((rel >> 8) & 0xff);
                out[cur_offset + 2] = (unsigned char)((rel >> 16) & 0xff);
                out[cur_offset + 3] = (unsigned char)((rel >> 24) & 0xff);
            }
            advance(4);
            continue;
        }

        if (hex_value(c) >= 0) {
            unsigned char byte;
            if (pos + 1 >= src_len || hex_value((unsigned char)src[pos + 1]) < 0)
                fail("expected hex byte pair");
            byte = (unsigned char)((hex_value((unsigned char)src[pos]) << 4) |
                                   hex_value((unsigned char)src[pos + 1]));
            pos += 2;
            if (emit) {
                if (cur_offset >= BUF_SIZE)
                    fail("output overflow");
                out[cur_offset] = byte;
            }
            advance(1);
            continue;
        }

        fail("unknown syntax");
    }
}

int main(void) {
    long n;
    fseek(stdin, 0, SEEK_END);
    n = ftell(stdin);
    if (n < 0 || (unsigned long)n > BUF_SIZE)
        fail("input too large");
    fseek(stdin, 0, SEEK_SET);
    src_len = (size_t)n;
    {
        char *input = malloc(src_len + 1);
        if (!input)
            fail("out of memory");
        if (fread(input, 1, src_len, stdin) != src_len)
            fail("short read");
        input[src_len] = 0;
        src = input;
    }

    scan(0);
    scan(1);
    if (fwrite(out, 1, max_offset, stdout) != max_offset)
        fail("short write");
    return 0;
}
