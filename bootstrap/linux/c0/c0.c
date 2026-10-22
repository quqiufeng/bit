typedef unsigned long long u64;

extern long long __c1_sysv_read(long long, void *, unsigned long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern void __c1_sysv_exit(long long);

#define BUF_SIZE (1u << 20)
static char src[BUF_SIZE];
static u64 src_len, pos;

static const char head[] =
    "^00400000\n"
    "7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00\n"
    "02 00 3E 00 01 00 00 00\n"
    "B0 00 40 00 00 00 00 00\n"
    "40 00 00 00 00 00 00 00\n"
    "00 00 00 00 00 00 00 00\n"
    "00 00 00 00\n"
    "40 00 38 00 02 00 00 00 00 00 00 00\n"
    "01 00 00 00 05 00 00 00\n"
    "00 00 00 00 00 00 00 00\n"
    "00 00 40 00 00 00 00 00\n"
    "00 00 40 00 00 00 00 00\n"
    "00 01 00 00 00 00 00 00\n"
    "00 01 00 00 00 00 00 00\n"
    "00 10 00 00 00 00 00 00\n"
    "01 00 00 00 06 00 00 00\n"
    "00 10 00 00 00 00 00 00\n"
    "00 00 41 00 00 00 00 00\n"
    "00 00 41 00 00 00 00 00\n"
    "00 00 00 00 00 00 00 00\n"
    "00 00 02 00 00 00 00 00\n"
    "00 10 00 00 00 00 00 00\n"
    ":00B0\n"
    ":start\n";

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
    write_all("BF", 2);
    hex_byte((unsigned int)(value & 0xff));
    hex_byte((unsigned int)((value >> 8) & 0xff));
    hex_byte((unsigned int)((value >> 16) & 0xff));
    hex_byte((unsigned int)((value >> 24) & 0xff));
    write_all("B8 3C 00 00 00\n0F 05\n.zero 56\n", 30);
    return 0;
}
