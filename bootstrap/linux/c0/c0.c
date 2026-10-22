typedef unsigned long long u64;
typedef unsigned int u32;

extern long long __c1_sysv_read(long long, void *, unsigned long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern void __c1_sysv_exit(long long);

#define BUF_SIZE (1u << 20)
static char src[BUF_SIZE];
static u64 src_len, pos;
static u64 generated_bytes;
static int label_counter = 0x2000;

static void write_all(const char *p, u64 n);
static void emit_decimal(u64 v);
static void emit_code_hex(const char *hex);
static void emit_imm32(u32 v);

static const char head[] =
    "^00400000\n"
    "7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00\n"
    "02 00 3E 00 01 00 00 00\n"
    "00 01 40 00 00 00 00 00\n"
    "40 00 00 00 00 00 00 00\n"
    "00 00 00 00 00 00 00 00\n"
    "00 00 00 00\n"
    "40 00 38 00 02 00 00 00 00 00 00 00\n"
    "01 00 00 00 05 00 00 00\n"
    "00 00 00 00 00 00 00 00\n"
    "00 00 40 00 00 00 00 00\n"
    "00 00 40 00 00 00 00 00\n"
    "00 00 01 00 00 00 00 00\n"
    "00 00 01 00 00 00 00 00\n"
    "00 10 00 00 00 00 00 00\n"
    "01 00 00 00 06 00 00 00\n"
    "00 00 01 00 00 00 00 00\n"
    "00 00 41 00 00 00 00 00\n"
    "00 00 41 00 00 00 00 00\n"
    "00 00 00 00 00 00 00 00\n"
    "00 00 20 00 00 00 00 00\n"
    "00 10 00 00 00 00 00 00\n";

static void emit_decimal(u64 v) {
    char buf[32];
    int n = 0;
    if (!v) {
        write_all("0", 1);
        return;
    }
    while (v) {
        buf[n++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (n) {
        n--;
        write_all(buf + n, 1);
    }
}

static void emit_code_hex(const char *hex) {
    int high = 1;
    while (*hex) {
        char c = *hex++;
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            continue;
        write_all(&c, 1);
        if (!high)
            generated_bytes++;
        high = !high;
    }
}

static void emit_imm32(u32 v) {
    char out[12];
    const char *h = "0123456789ABCDEF";
    out[0] = ' ';
    out[1] = h[(v >> 4) & 15];
    out[2] = h[v & 15];
    out[3] = ' ';
    out[4] = h[(v >> 12) & 15];
    out[5] = h[(v >> 8) & 15];
    out[6] = ' ';
    out[7] = h[(v >> 20) & 15];
    out[8] = h[(v >> 16) & 15];
    out[9] = ' ';
    out[10] = h[(v >> 28) & 15];
    out[11] = h[(v >> 24) & 15];
    write_all(out, 12);
    generated_bytes += 4;
}

static void emit_string_literal(const char *str) {
    u64 n = 0;
    write_all("\"", 1);
    while (str[n]) {
        write_all(str + n, 1);
        n++;
    }
    write_all("\" ", 2);
    generated_bytes += n + 1;
}

static void emit_reference(int id) {
    char buf[8];
    const char *h = "0123456789ABCDEF";
    buf[0] = '%';
    buf[1] = 'L';
    buf[2] = h[(id >> 12) & 15];
    buf[3] = h[(id >> 8) & 15];
    buf[4] = h[(id >> 4) & 15];
    buf[5] = h[id & 15];
    buf[6] = ' ';
    buf[7] = 0;
    write_all(buf, 7);
    generated_bytes += 4;
}

static void emit_label(int id) {
    const char *h = "0123456789ABCDEF";
    char buf[8];
    buf[0] = ':';
    buf[1] = 'L';
    buf[2] = h[(id >> 12) & 15];
    buf[3] = h[(id >> 8) & 15];
    buf[4] = h[(id >> 4) & 15];
    buf[5] = h[id & 15];
    buf[6] = '\n';
    write_all(buf, 7);
}

static void fail(void) {
    __c1_sysv_write(2, "c0 error\n", 9);
    __c1_sysv_exit(1);
}

static void write_all(const char *p, u64 n) {
    while (n) {
        long long k = __c1_sysv_write(1, p, n);
        if (k <= 0) fail();
        p += k;
        n -= (u64)k;
    }
}

static void skip(void) {
    while (pos < src_len) {
        char c = src[pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            pos++;
            continue;
        }
        if (c == '/' && pos + 1 < src_len && src[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < src_len && !(src[pos] == '*' && src[pos + 1] == '/')) pos++;
            if (pos + 1 < src_len) pos += 2;
            continue;
        }
        if (c == '/' && pos + 1 < src_len && src[pos + 1] == '/') {
            while (pos < src_len && src[pos] != '\n') pos++;
            continue;
        }
        break;
    }
}

static int word(const char *s) {
    u64 i = 0;
    while (s[i]) {
        if (pos + i >= src_len || src[pos + i] != s[i]) return 0;
        i++;
    }
    if (pos + i < src_len) {
        char c = src[pos + i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
            return 0;
    }
    pos += i;
    return 1;
}

static int ch(char c) {
    if (pos < src_len && src[pos] == c) {
        pos++;
        return 1;
    }
    return 0;
}

static u64 number(void) {
    u64 v = 0;
    int any = 0;
    skip();
    if (pos + 1 < src_len && src[pos] == '0' && (src[pos + 1] == 'x' || src[pos + 1] == 'X')) {
        pos += 2;
        while (pos < src_len) {
            char c = src[pos];
            int d;
            if (c >= '0' && c <= '9') d = c - '0';
            else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
            else break;
            v = (v << 4) | (u64)d;
            pos++;
            any = 1;
        }
    } else {
        while (pos < src_len && src[pos] >= '0' && src[pos] <= '9') {
            v = v * 10 + (u64)(src[pos] - '0');
            pos++;
            any = 1;
        }
    }
    if (!any) fail();
    return v;
}

static void hex_byte(unsigned int b) {
    char out[4];
    const char *h = "0123456789ABCDEF";
    out[0] = ' ';
    out[1] = h[(b >> 4) & 15];
    out[2] = h[b & 15];
    out[3] = '\n';
    write_all(out, 4);
}

int main(void) {
    long long n;
    u64 value;
    src_len = 0;
    while ((n = __c1_sysv_read(0, src + src_len, BUF_SIZE - src_len)) > 0) {
        src_len += (u64)n;
        if (src_len == BUF_SIZE) break;
    }
    skip();
    if (!word("int")) fail();
    skip();
    if (!word("main")) fail();
    skip();
    if (!ch('(')) fail();
    skip();
    if (word("void")) skip();
    if (!ch(')')) fail();
    skip();
    if (!ch('{')) fail();
    skip();
    if (!word("return")) fail();
    value = number();
    skip();
    if (!ch(';')) fail();
    skip();
    if (!ch('}')) fail();
    skip();
    if (pos != src_len) fail();

    write_all(head, sizeof(head) - 1);
    generated_bytes = 176;
    write_all(".zero 80\n:0100\n:start\n", 22);
    generated_bytes += 80;
    emit_code_hex("BF");
    emit_imm32((u32)value);
    write_all("\n", 1);
    emit_code_hex("B8 3C 00 00 00");
    write_all("\n", 1);
    emit_code_hex("0F 05");
    write_all("\n", 1);
    if (generated_bytes > 65536) fail();
    if (generated_bytes < 65536) {
        u64 pad = 65536 - generated_bytes;
        write_all(".zero ", 6);
        emit_decimal(pad);
        write_all("\n", 1);
        generated_bytes = 65536;
    }
    return 0;
}
