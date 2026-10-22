typedef unsigned long long u64;
typedef unsigned int u32;

extern long long __c1_sysv_read(long long, void *, unsigned long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern void __c1_sysv_exit(long long);


static char label_name[8192][64];
static u64 label_vaddr[8192];
static u64 label_offset[8192];
static int label_defined[8192];
static int label_count;
static char src[1048576];
static unsigned char out[8388608];
static u64 src_len, pos, base_vaddr, cur_vaddr;
static u64 cur_offset, max_offset;

static void fail(void) {
    char b[32];
    int n = 0;
    u64 v = pos;
    __c1_sysv_write(2, "a1 error at ", 12);
    if (!v) b[n++] = '0';
    while (v) { b[n++] = (char)('0' + (v % 10)); v /= 10; }
    while (n) __c1_sysv_write(2, b + --n, 1);
    __c1_sysv_write(2, "\n", 1);
    __c1_sysv_exit(1);
}

static int hexval(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int namechar(int c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') || c == '_' || c == '.' || c == '$';
}

static int same(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

static void copy_name(char *d, const char *s) {
    int i = 0;
    while (s[i] && i < 64 - 1) {
        d[i] = s[i];
        i++;
    }
    d[i] = 0;
}

static u64 read_hex(u64 n) {
    u64 v = 0;
    u64 i;
    if (!n || n > 16) fail();
    for (i = 0; i < n; i++) {
        int c;
        if (pos >= src_len) fail();
        c = hexval((unsigned char)src[pos]);
        if (c < 0) fail();
        v = (v << 4) | (u64)c;
        pos++;
    }
    return v;
}

static u64 count_hex(void) {
    u64 n = 0;
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

static void advance(u64 n) {
    cur_offset += n;
    cur_vaddr += n;
    if (cur_offset > max_offset) max_offset = cur_offset;
}

static int find_label(const char *name) {
    int i;
    for (i = 0; i < label_count; i++)
        if (same(label_name[i], name)) return i;
    return -1;
}

static void add_label(const char *name) {
    int idx = find_label(name);
    if (idx < 0) {
        if (label_count == 8192) fail();
        idx = label_count;
        copy_name(label_name[idx], name);
        label_count = label_count + 1;
    }
    label_vaddr[idx] = cur_vaddr;
    label_offset[idx] = cur_offset;
    label_defined[idx] = 1;
}

static void read_name(char *dst) {
    int n = 0;
    while (pos < src_len && namechar((unsigned char)src[pos])) {
        if (n == 64 - 1) fail();
        dst[n++] = src[pos++];
    }
    if (!n) fail();
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
            if (cur_offset >= 8388608) fail();
            out[cur_offset] = (unsigned char)c;
        }
        advance(1);
    }
    if (pos >= src_len) fail();
    pos++;
}

static u64 parse_dec(void) {
    u64 v = 0;
    int any = 0;
    skip();
    while (pos < src_len && src[pos] >= '0' && src[pos] <= '9') {
        v = v * 10 + (u64)(src[pos] - '0');
        pos++;
        any = 1;
    }
    if (!any) fail();
    return v;
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
            char name[64];
            u64 n = 0;
            u64 save = pos + 1;
            while (save + n < src_len && hexval((unsigned char)src[save + n]) >= 0) n++;
            if (n == 4 && (save + n >= src_len || !namechar((unsigned char)src[save + n]))) {
                u64 v;
                pos++;
                v = read_hex(4);
                cur_offset = v;
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
            char name[64];
            u64 n = 0;
            u64 save = pos + 1;
            while (save + n < src_len && hexval((unsigned char)src[save + n]) >= 0) n++;
            if (n == 4 && (save + n >= src_len || !namechar((unsigned char)src[save + n]))) {
                int i;
                for (i = 0; i < 4; i++) name[i] = src[pos + 1 + i];
                name[4] = 0;
                pos += 5;
            } else {
                pos++;
                read_name(name);
            }
            if (emit) {
                int li = find_label(name);
                u32 rel;
                if (li < 0 || !label_defined[li]) fail();
                rel = (u32)(label_vaddr[li] - (cur_vaddr + 4));
                if (cur_offset + 4 > 8388608) fail();
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
            if (pos + 5 <= src_len && src[pos + 1] == 'z' && src[pos + 2] == 'e' &&
                src[pos + 3] == 'r' && src[pos + 4] == 'o') {
                u64 n;
                pos += 5;
                n = parse_dec();
                if (emit) {
                    u64 i;
                    if (cur_offset + n > 8388608) fail();
                    for (i = 0; i < n; i++) out[cur_offset + i] = 0;
                }
                advance(n);
                continue;
            }
            if (pos + 4 <= src_len && src[pos + 1] == 'o' && src[pos + 2] == 'r' &&
                src[pos + 3] == 'g') {
                u64 n;
                pos += 4;
                n = parse_dec();
                cur_offset = n;
                cur_vaddr = base_vaddr + n;
                if (n > max_offset) max_offset = n;
                continue;
            }
            fail();
        }
        if (hexval(c) >= 0) {
            unsigned char b;
            if (pos + 1 >= src_len || hexval((unsigned char)src[pos + 1]) < 0) fail();
            b = (unsigned char)((hexval(c) << 4) | hexval((unsigned char)src[pos + 1]));
            pos += 2;
            if (emit) {
                if (cur_offset >= 8388608) fail();
                out[cur_offset] = b;
            }
            advance(1);
            continue;
        }
        fail();
    }
}

int main(void) {
    long long n;
    src_len = 0;
    while ((n = __c1_sysv_read(0, src + src_len, 1048576 - src_len)) > 0) {
        src_len += (u64)n;
        if (src_len == 1048576) break;
    }
    scan(0);
    scan(1);
    __c1_sysv_write(1, out, max_offset);
    return 0;
}
