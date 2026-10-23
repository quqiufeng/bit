#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABELS 16384
#define MAX_NAME 64
#define BUF_SIZE (1u << 20)
#define OUT_SIZE (8u << 20)

struct label {
    char name[MAX_NAME];
    uint64_t vaddr;
    uint64_t offset;
    int defined;
};

static struct label labels[MAX_LABELS];
static int label_count;
static unsigned char out[OUT_SIZE];
static const char *src;
static size_t src_len, pos;
static uint64_t base_vaddr, cur_vaddr;
static size_t cur_offset, max_offset;

static void fail(const char *m) {
    fprintf(stderr, "a1: %s at byte %zu\n", m, pos);
    exit(1);
}

static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int namechar(int c) {
    return isalnum(c) || c == '_' || c == '.' || c == '$';
}

static uint64_t read_hex(unsigned n) {
    uint64_t v = 0;
    unsigned i;
    if (!n || n > 16) fail("bad hex width");
    for (i = 0; i < n; i++) {
        int c;
        if (pos >= src_len) fail("truncated hex");
        c = hexval((unsigned char)src[pos]);
        if (c < 0) fail("expected hex");
        v = (v << 4) | (unsigned)c;
        pos++;
    }
    return v;
}

static unsigned count_hex(void) {
    unsigned n = 0;
    while (pos + n < src_len && hexval((unsigned char)src[pos + n]) >= 0) n++;
    return n;
}

static void skip(void) {
    for (;;) {
        unsigned char c;
        if (pos >= src_len) return;
        c = (unsigned char)src[pos];
        if (c == ';') {
            while (pos < src_len && src[pos] != '\n') pos++;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            pos++;
            continue;
        }
        return;
    }
}

static void advance(size_t n) {
    cur_offset += n;
    cur_vaddr += n;
    if (cur_offset > max_offset) max_offset = cur_offset;
}

static struct label *find_label(const char *name) {
    int i;
    for (i = 0; i < label_count; i++)
        if (!strcmp(labels[i].name, name)) return &labels[i];
    return NULL;
}

static void add_label(const char *name) {
    struct label *l = find_label(name);
    if (!l) {
        if (label_count == MAX_LABELS) fail("too many labels");
        l = &labels[label_count++];
        {
            size_t n = strlen(name);
            memcpy(l->name, name, n + 1);
        }
    }
    l->vaddr = cur_vaddr;
    l->offset = cur_offset;
    l->defined = 1;
}

static void read_name(char *dst) {
    int n = 0;
    while (pos < src_len && namechar((unsigned char)src[pos])) {
        if (n == MAX_NAME - 1) fail("label too long");
        dst[n++] = src[pos++];
    }
    if (!n) fail("expected label");
    dst[n] = 0;
}

static void parse_string(int emit) {
    pos++;
    while (pos < src_len && src[pos] != '"') {
        int c = (unsigned char)src[pos++];
        if (c == '\\' && pos < src_len) {
            c = (unsigned char)src[pos++];
            if (c == 'n') c = '\n';
            else if (c == 'r') c = '\r';
            else if (c == 't') c = '\t';
        }
        if (emit) {
            if (cur_offset >= OUT_SIZE) fail("output overflow");
            out[cur_offset] = (unsigned char)c;
        }
        advance(1);
    }
    if (pos >= src_len) fail("unterminated string");
    pos++;
}

static void scan(int emit) {
    pos = 0;
    base_vaddr = 0;
    cur_vaddr = 0;
    cur_offset = 0;
    max_offset = 0;
    for (;;) {
        unsigned char c;
        skip();
        if (pos >= src_len) break;
        c = (unsigned char)src[pos];
        if (c == '^') {
            pos++;
            base_vaddr = read_hex(count_hex());
            cur_vaddr = base_vaddr + cur_offset;
            continue;
        }
        if (c == ':') {
            char name[MAX_NAME];
            unsigned n;
            size_t save = pos + 1;
            n = 0;
            while (save + n < src_len && hexval((unsigned char)src[save + n]) >= 0) n++;
            if (n == 4 && (save + n >= src_len || !namechar((unsigned char)src[save + n]))) {
                uint64_t v;
                pos++;
                v = read_hex(4);
                cur_offset = (size_t)v;
                cur_vaddr = base_vaddr + cur_offset;
                if (cur_offset > max_offset) max_offset = cur_offset;
            } else {
                pos++;
                read_name(name);
                add_label(name);
            }
            continue;
        }
        if (c == '%') {
            char name[MAX_NAME];
            unsigned n = 0;
            size_t save = pos + 1;
            while (save + n < src_len && hexval((unsigned char)src[save + n]) >= 0) n++;
            if (n == 4 && (save + n >= src_len || !namechar((unsigned char)src[save + n]))) {
                snprintf(name, sizeof(name), "%.4s", src + pos + 1);
                pos += 5;
            } else {
                pos++;
                read_name(name);
            }
            if (emit) {
                struct label *l = find_label(name);
                uint32_t rel;
                if (!l || !l->defined) fail("undefined label");
                rel = (uint32_t)(l->vaddr - (cur_vaddr + 4));
                if (cur_offset + 4 > OUT_SIZE) fail("output overflow");
                out[cur_offset + 0] = rel & 0xff;
                out[cur_offset + 1] = (rel >> 8) & 0xff;
                out[cur_offset + 2] = (rel >> 16) & 0xff;
                out[cur_offset + 3] = (rel >> 24) & 0xff;
            }
            advance(4);
            continue;
        }
        if (c == '"') {
            parse_string(emit);
            continue;
        }
        if (c == '.') {
            if (pos + 5 <= src_len && !memcmp(src + pos, ".zero", 5)) {
                unsigned long n;
                char *end;
                pos += 5;
                skip();
                n = strtoul(src + pos, &end, 0);
                if (end == src + pos) fail("expected zero count");
                pos = (size_t)(end - src);
                if (emit) {
                    if (cur_offset + n > OUT_SIZE) fail("output overflow");
                    memset(out + cur_offset, 0, n);
                }
                advance(n);
                continue;
            }
            if (pos + 4 <= src_len && !memcmp(src + pos, ".org", 4)) {
                unsigned long n;
                char *end;
                pos += 4;
                skip();
                n = strtoul(src + pos, &end, 0);
                if (end == src + pos) fail("expected org value");
                pos = (size_t)(end - src);
                cur_offset = (size_t)n;
                cur_vaddr = base_vaddr + n;
                if (cur_offset > max_offset) max_offset = cur_offset;
                continue;
            }
            fail("unknown directive");
        }
        if (hexval(c) >= 0) {
            unsigned char b;
            if (pos + 1 >= src_len || hexval((unsigned char)src[pos + 1]) < 0)
                fail("expected byte pair");
            b = (unsigned char)((hexval(c) << 4) | hexval((unsigned char)src[pos + 1]));
            pos += 2;
            if (emit) {
                if (cur_offset >= OUT_SIZE) fail("output overflow");
                out[cur_offset] = b;
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
    if (n < 0 || (unsigned long)n > BUF_SIZE) fail("input too large");
    fseek(stdin, 0, SEEK_SET);
    src_len = (size_t)n;
    {
        char *input = malloc(src_len + 1);
        if (!input) fail("out of memory");
        if (fread(input, 1, src_len, stdin) != src_len) fail("short read");
        input[src_len] = 0;
        src = input;
    }
    scan(0);
    scan(1);
    if (fwrite(out, 1, max_offset, stdout) != max_offset) fail("short write");
    return 0;
}
