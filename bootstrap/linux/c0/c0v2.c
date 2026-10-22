/* C0 v2: type-system refactor. Phase 1 keeps legacy behavior. */
typedef unsigned long long u64;
typedef unsigned int u32;

extern long long __c1_sysv_read(long long, void *, unsigned long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern void __c1_sysv_exit(long long);

static char src[1048576];
static u64 src_len, pos, generated_bytes;
static int label_seq = 0x2000;
static char goto_names[128][64];
static int goto_ids[128];
static int goto_count;
static char g_name[1024][64];
static u64 g_size[1024];
static int g_type[1024];
static int g_elem[1024];
static int g_dims[1024];
static int g_row[1024];
static int g_has_init[1024];
static int g_init[1024];
static int g_init_vals[2048];
static int g_init_off[1024];
static int g_init_used;
static int g_init_count[1024];
static int g_pelem[1024];
static int l_pelem[256];
static int fn_count;
static char fn_name[1024][64];
static int fn_type[1024];
static int fn_elem[1024];
static int lv_ptrvar;
static int cur_ret_type, cur_ret_elem;
static int g_count;
static char l_name[256][64];
static int l_off[256];
static int l_type[256];
static int l_elem[256];
static int l_size[256];
static int l_dims[256];
static int l_row[256];
static int l_count;
static int brk_l[64], cont_l[64], brk_n;
static char cur_func[64];
static u64 last_pos;
static int stage;
static int lv_type, lv_elem, lv_ptr;
static int ex_type, ex_elem, ex_ptr;
static int assign_op;
static int last_type;

static void fail(void) {
    char b[32];
    int n = 0;
    u64 v = pos;
    __c1_sysv_write(2, "c0 error at ", 12);
    if (!v) b[n++] = '0';
    while (v) { b[n++] = (char)('0' + (v % 10)); v /= 10; }
    while (n) __c1_sysv_write(2, b + --n, 1);
    __c1_sysv_write(2, " stage ", 7);
    n = 0; v = (u64)stage;
    if (!v) b[n++] = '0';
    while (v) { b[n++] = (char)('0' + (v % 10)); v /= 10; }
    while (n) __c1_sysv_write(2, b + --n, 1);
    __c1_sysv_write(2, " fn ", 4);
    {
        int fi = 0;
        while (cur_func[fi]) { __c1_sysv_write(2, cur_func + fi, 1); fi++; }
    }
    __c1_sysv_write(2, " last ", 6);
    n = 0; v = last_pos;
    if (!v) b[n++] = '0';
    while (v) { b[n++] = (char)('0' + (v % 10)); v /= 10; }
    while (n) __c1_sysv_write(2, b + --n, 1);
    __c1_sysv_write(2, " len ", 5);
    n = 0; v = src_len;
    if (!v) b[n++] = '0';
    while (v) { b[n++] = (char)('0' + (v % 10)); v /= 10; }
    while (n) __c1_sysv_write(2, b + --n, 1);
    __c1_sysv_write(2, "\n", 1);
    __c1_sysv_exit(1);
}

static int strcmp(const char *a, const char *b) {
    int i;
    i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
        i = i + 1;
    }
    return (unsigned char)a[i] - (unsigned char)b[i];
}

static char *strcpy(char *d, const char *s) {
    int i;
    i = 0;
    while (s[i]) {
        d[i] = s[i];
        i = i + 1;
    }
    d[i] = 0;
    return d;
}

static void write_all(const char *p, u64 n) {
    while (n) {
        long long k = __c1_sysv_write(1, p, n);
        if (k <= 0) fail();
        p += k;
        n -= (u64)k;
    }
}

static void emit_decimal(u64 v) {
    char b[32];
    int i = 0;
    if (!v) {
        write_all("0", 1);
        return;
    }
    while (v) {
        b[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (i) {
        i--;
        write_all(b + i, 1);
    }
}

static void emit_hex(const char *hex) {
    int phase = 0;
    int i = 0;
    while (hex[i]) {
        char c = hex[i];
        i = i + 1;
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') continue;
        write_all(&c, 1);
        if (!phase) generated_bytes++;
        phase = !phase;
    }
    write_all(" ", 1);
}

static void emit_text(const char *s) {
    int i = 0;
    while (s[i]) {
        write_all(s + i, 1);
        i = i + 1;
    }
}

static void emit_label(int id) {
    char b[8];
    const char *h = "0123456789ABCDEF";
    b[0] = ':';
    b[1] = 'L';
    b[2] = h[(id >> 12) & 15];
    b[3] = h[(id >> 8) & 15];
    b[4] = h[(id >> 4) & 15];
    b[5] = h[id & 15];
    b[6] = '\n';
    write_all(b, 7);
}

static void emit_ref(int id) {
    char b[8];
    const char *h = "0123456789ABCDEF";
    b[0] = '%';
    b[1] = 'L';
    b[2] = h[(id >> 12) & 15];
    b[3] = h[(id >> 8) & 15];
    b[4] = h[(id >> 4) & 15];
    b[5] = h[id & 15];
    b[6] = ' ';
    write_all(b, 7);
    generated_bytes += 4;
}

static void emit_named_label(const char *name) {
    write_all("\n:", 2);
    emit_text(name);
    write_all("\n", 1);
}

static void emit_named_ref(const char *name) {
    write_all("%", 1);
    emit_text(name);
    write_all(" ", 1);
    generated_bytes += 4;
}

static void emit_imm32(u32 v) {
    char b[12];
    const char *h = "0123456789ABCDEF";
    b[0] = ' ';
    b[1] = h[(v >> 4) & 15];
    b[2] = h[v & 15];
    b[3] = ' ';
    b[4] = h[(v >> 12) & 15];
    b[5] = h[(v >> 8) & 15];
    b[6] = ' ';
    b[7] = h[(v >> 20) & 15];
    b[8] = h[(v >> 16) & 15];
    b[9] = ' ';
    b[10] = h[(v >> 28) & 15];
    b[11] = h[(v >> 24) & 15];
    write_all(b, 12);
    generated_bytes += 4;
}

static void emit_zero(u64 n) {
    emit_text(".zero ");
    emit_decimal(n);
    emit_text("\n");
    generated_bytes += n;
}

static int new_label(void) {
    return label_seq++;
}

static void skip(void) {
    for (;;) {
        char c;
        if (pos >= src_len) return;
        c = src[pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            pos++;
            continue;
        }
        if (c == '/' && pos + 1 < src_len && src[pos + 1] == '/') {
            while (pos < src_len && src[pos] != '\n') pos++;
            continue;
        }
        if (c == '/' && pos + 1 < src_len && src[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < src_len && !(src[pos] == '*' && src[pos + 1] == '/')) pos++;
            if (pos + 1 < src_len) pos += 2;
            continue;
        }
        return;
    }
}

static int ident(char *dst) {
    int n = 0;
    skip();
    if (pos >= src_len) return 0;
    if (!((src[pos] >= 'a' && src[pos] <= 'z') || (src[pos] >= 'A' && src[pos] <= 'Z') || src[pos] == '_'))
        return 0;
    while (pos < src_len && ((src[pos] >= 'a' && src[pos] <= 'z') ||
                             (src[pos] >= 'A' && src[pos] <= 'Z') ||
                             (src[pos] >= '0' && src[pos] <= '9') || src[pos] == '_')) {
        if (n < 63) dst[n++] = src[pos];
        pos++;
    }
    dst[n] = 0;
    return 1;
}

static int word(const char *w) {
    int i = 0;
    u64 save = pos;
    skip();
    while (w[i]) {
        if (pos >= src_len || src[pos] != w[i]) {
            pos = save;
            return 0;
        }
        i++;
        pos++;
    }
    if (pos < src_len) {
        char c = src[pos];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
            pos = save;
            return 0;
        }
    }
    return 1;
}

static int type_word(void) {
    if (word("const")) return type_word();
    if (word("unsigned") || word("signed")) return type_word();
    if (word("long")) { word("long"); last_type = 4; return 4; }
    if (word("int")) { last_type = 1; return 1; }
    if (word("char")) { last_type = 2; return 2; }
    if (word("short")) { last_type = 7; return 7; }
    if (word("void")) { last_type = 3; return 3; }
    if (word("u64")) { last_type = 4; return 4; }
    if (word("u32")) { last_type = 5; return 5; }
    return 0;
}

static int ch(char c) {
    skip();
    if (pos < src_len && src[pos] == c) {
        pos++;
        return 1;
    }
    return 0;
}

static int op(const char *s) {
    int i = 0;
    u64 save = pos;
    skip();
    while (s[i]) {
        if (pos >= src_len || src[pos] != s[i]) {
            pos = save;
            return 0;
        }
        i++;
        pos++;
    }
    return 1;
}

static int rest_end(void) {
    u64 p = pos;
    while (p < src_len) {
        char c = src[p];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '}') p++;
        else return 0;
    }
    return 1;
}

static int op1(char c) {
    skip();
    if (pos < src_len && src[pos] == c && (pos + 1 >= src_len || src[pos + 1] != c)) {
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
    while (pos < src_len && (src[pos] == 'U' || src[pos] == 'u' || src[pos] == 'L' || src[pos] == 'l')) pos++;
    return v;
}

static int find_global(const char *name) {
    int i;
    for (i = 0; i < g_count; i++)
        if (strcmp(g_name[i], name) == 0) return i;
    return -1;
}

static int find_local(const char *name) {
    int i;
    for (i = l_count - 1; i >= 0; i--)
        if (strcmp(l_name[i], name) == 0) return i;
    return -1;
}

static int type_size(int tw) {
    if (tw == 2) return 1;
    if (tw == 4) return 8;
    if (tw == 5) return 4;
    if (tw == 6) return 8;
    if (tw == 7) return 2;
    return 4;
}

static int is_ptr(int tw) { return tw == 6; }

static void emit_scale(int scale) {
    if (scale == 1) emit_hex("48 8D 04 08");
    else if (scale == 4) emit_hex("48 8D 04 88");
    else if (scale == 8) emit_hex("48 8D 04 C8");
    else {
        emit_hex("48 69 C9");
        emit_imm32((u32)scale);
        emit_text("\n");
        emit_hex("48 01 C8");
    }
    emit_text("\n");
}

static void store_rcx(int tw, int ptr) {
    if (ptr) fail();
    if (tw == 2) emit_hex("88 01");
    else if (tw == 4 || tw == 6) emit_hex("48 89 01");
    else if (tw == 5) emit_hex("89 01");
    else emit_hex("89 01");
    emit_text("\n");
}

static int add_local(const char *name, u64 bytes) {
    int i = l_count;
    int off = 0;
    if (i) off = l_off[i - 1];
    off = (off + (int)bytes + 7) & -8;
    if (i == 256) fail();
    strcpy(l_name[i], name);
    l_off[i] = off;
    l_size[i] = (int)bytes;
    l_count++;
    return i;
}

static void emit_addr(const char *name) {
    int l = find_local(name);
    if (l >= 0) {
        emit_hex("48 8D 85");
        emit_imm32((u32)(-l_off[l]));
        emit_text("\n");
        return;
    }
    if (find_global(name) >= 0) {
        emit_hex("48 8D 05");
        emit_named_ref(name);
        emit_text("\n");
        return;
    }
    fail();
}

static void ex_expr(void);

static void ex_lvalue(const char *name) {
    int l = find_local(name);
    int g;
    if (l < 0) g = find_global(name); else g = -1;
    int tw, elem, dims, row, pelem = 0;
    if (l >= 0) {
        tw = l_type[l]; elem = l_elem[l]; dims = l_dims[l]; row = l_row[l]; pelem = l_pelem[l];
    } else if (g >= 0) {
        tw = g_type[g]; elem = g_elem[g]; dims = g_dims[g]; row = g_row[g]; pelem = g_pelem[g];
    } else fail();
    if (tw == 6) elem = pelem;
    emit_addr(name);
    lv_type = tw;
    lv_elem = elem;
    lv_ptr = dims != 0;
    lv_ptrvar = 0;
    if (tw == 6 && dims == 0) {
        if (!ch('[')) {
            lv_ptrvar = 1;
            return;
        }
        emit_hex("48 8B 00");
        emit_text("\n");
        emit_hex("50");
        ex_expr();
        ch(']');
        emit_hex("48 89 C1 58");
        emit_scale(elem);
        if (elem == 1) lv_type = 2;
        else if (elem == 8) lv_type = 4;
        else lv_type = 1;
        lv_elem = elem;
        lv_ptr = 0;
        return;
    }
    if (dims == 0) return;
    if (ch('[')) {
        emit_hex("50");
        ex_expr();
        ch(']');
        emit_hex("48 89 C1 58");
        if (dims == 2) emit_scale(row); else emit_scale(elem);
        if (dims == 2) {
            dims = 1;
            lv_ptr = 1;
            lv_type = tw;
            if (ch('[')) {
                emit_hex("50");
                ex_expr();
                ch(']');
                emit_hex("48 89 C1 58");
                emit_scale(elem);
                lv_ptr = 0;
                dims = 0;
                lv_type = tw;
            }
        } else {
            lv_ptr = 0;
            dims = 0;
            lv_type = tw;
        }
    }
}

static int assign_ahead(void) {
    u64 save = pos;
    char name[64];
    assign_op = 0;
    if (!ident(name)) {
        pos = save;
        return 0;
    }
    for (;;) {
        int depth = 1;
        skip();
        if (pos >= src_len || src[pos] != '[') break;
        pos++;
        while (pos < src_len && depth) {
            if (src[pos] == '[') depth++;
            else if (src[pos] == ']') depth--;
            pos++;
        }
    }
    skip();
    if (op("+=")) assign_op = '+';
    else if (op("-=")) assign_op = '-';
    else if (op("*=")) assign_op = '*';
    else if (op("/=")) assign_op = '/';
    else if (op("%=")) assign_op = '%';
    else if (op("&=")) assign_op = '&';
    else if (op("|=")) assign_op = '|';
    else if (op("^=")) assign_op = '^';
    else if (op("<<=")) assign_op = 'L';
    else if (op(">>=")) assign_op = 'R';
    else if (pos + 1 < src_len && src[pos] == '=' && src[pos + 1] != '=') assign_op = 0;
    else {
        pos = save;
        return 0;
    }
    pos = save;
    return 1;
}

static void ex_add(void);
static void ex_shift(void);
static void ex_rel(void);
static void ex_eq(void);
static void ex_bitand(void);
static void ex_bitxor(void);
static void ex_bitor(void);

static void emit_incdec(int delta, int postfix) {
    emit_hex("50");
    emit_text("\n");
    emit_hex("48 8B 0C 24");
    emit_text("\n");
    emit_hex("48 63 01");
    emit_text("\n");
    if (postfix) {
        emit_hex("48 89 C2");
        emit_text("\n");
    }
    if (delta > 0) emit_hex("48 83 C0 01");
    else emit_hex("48 83 E8 01");
    emit_text("\n");
    emit_hex("89 01");
    emit_text("\n");
    if (postfix) {
        emit_hex("48 89 D0");
        emit_text("\n");
    }
    emit_hex("48 83 C4 08");
    emit_text("\n");
}

static int goto_id(const char *name) {
    int i;
    for (i = 0; i < goto_count; i++)
        if (strcmp(goto_names[i], name) == 0) return goto_ids[i];
    if (goto_count == 128) fail();
    strcpy(goto_names[goto_count], name);
    goto_ids[goto_count] = new_label();
    goto_count++;
    return goto_ids[goto_count - 1];
}

static void ex_primary(void) {
    char name[64];
    skip();
    if (word("sizeof")) {
        u64 sz = 0;
        ch('(');
        if (type_word()) {
            sz = (u64)type_size(last_type);
            if (ch('[')) {
                u64 n = number();
                ch(']');
                sz = sz * n;
            }
        } else {
            int li;
            int gi;
            if (!ident(name)) fail();
            li = find_local(name);
            if (li < 0) gi = find_global(name); else gi = -1;
            if (li >= 0) sz = (u64)l_size[li];
            else if (gi >= 0) sz = (u64)g_size[gi];
            else fail();
        }
        ch(')');
        emit_hex("B8");
        emit_imm32((u32)sz);
        emit_text("\n");
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (pos < src_len && src[pos] == '"') {
        int a = new_label();
        int b = new_label();
        emit_hex("E9");
        emit_ref(b);
        emit_label(a);
        write_all("\"", 1);
        pos++;
        while (pos < src_len && src[pos] != '"') {
            if (src[pos] == '\\' && pos + 1 < src_len) {
                write_all(src + pos, 2);
                generated_bytes++;
                pos += 2;
            } else {
                write_all(src + pos, 1);
                generated_bytes++;
                pos++;
            }
        }
        if (pos < src_len) pos++;
        write_all("\" 00\n", 5);
        generated_bytes += 1;
        emit_label(b);
        emit_hex("48 8D 05");
        emit_ref(a);
        emit_text("\n");
        ex_type = 2; ex_elem = 1; ex_ptr = 1;
        return;
    }
    if (pos < src_len && src[pos] >= '0' && src[pos] <= '9') {
        u64 v = number();
        emit_hex("B8");
        emit_imm32((u32)v);
        emit_text("\n");
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (pos < src_len && src[pos] == '\'') {
        u64 v;
        pos++;
        if (pos < src_len && src[pos] == '\\') {
            pos++;
            if (pos < src_len && src[pos] == 'n') v = 10;
            else if (pos < src_len && src[pos] == 'r') v = 13;
            else if (pos < src_len && src[pos] == 't') v = 9;
            else if (pos < src_len && src[pos] == '0') v = 0;
            else v = (unsigned char)src[pos];
            pos++;
        } else {
            v = (unsigned char)src[pos++];
        }
        ch('\'');
        emit_hex("B8");
        emit_imm32((u32)v);
        emit_text("\n");
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (ident(name)) {
        if (ch('(')) {
            int n = 0;
            while (!ch(')')) {
                ex_expr();
                emit_hex("50");
                n++;
                if (!ch(',')) {
                    ch(')');
                    break;
                }
            }
            if (n >= 4) emit_hex("59");
            if (n >= 3) emit_hex("5A");
            if (n >= 2) emit_hex("5E");
            if (n >= 1) emit_hex("5F");
            emit_hex("E8");
            emit_named_ref(name);
            emit_text("\n");
            {
                int i;
                int rt = 1, re = 4;
                for (i = 0; i < fn_count; i++) {
                    if (strcmp(fn_name[i], name) == 0) {
                        rt = fn_type[i];
                        re = fn_elem[i];
                    }
                }
                ex_type = rt;
                ex_elem = re;
                ex_ptr = (rt == 6);
            }
            return;
        }
        ex_lvalue(name);
        if (!lv_ptr && op("++")) {
            emit_incdec(1, 1);
            ex_type = 1; ex_elem = 4; ex_ptr = 0;
            return;
        }
        if (!lv_ptr && op("--")) {
            emit_incdec(-1, 1);
            ex_type = 1; ex_elem = 4; ex_ptr = 0;
            return;
        }
        if (lv_ptrvar) {
            emit_hex("48 8B 00");
            emit_text("\n");
            ex_type = 6; ex_elem = lv_elem; ex_ptr = 1;
            return;
        }
        if (lv_ptr) {
            ex_type = lv_type; ex_elem = lv_elem; ex_ptr = 1;
            return;
        }
        if (lv_type == 2) emit_hex("48 0F BE 00");
        else if (lv_type == 4) emit_hex("48 8B 00");
        else if (lv_type == 5) emit_hex("8B 00");
        else emit_hex("48 63 00");
        emit_text("\n");
        ex_type = lv_type; ex_elem = lv_elem; ex_ptr = 0;
        return;
    }
    if (ch('(')) {
        ex_expr();
        ch(')');
        return;
    }
    fail();
}

static void ex_unary(void) {
    u64 cast_save = pos;
    if (ch('(')) {
        int tw = type_word();
        if (tw && ch(')')) {
            ex_unary();
            if (tw == 2) {
                emit_hex("48 25 FF 00 00 00");
                emit_text("\n");
            } else if (tw == 7) {
                emit_hex("48 0F BF C0");
                emit_text("\n");
            }
            return;
        }
        pos = cast_save;
    }
    if (op("++")) {
        char name[64];
        if (!ident(name)) fail();
        ex_lvalue(name);
        emit_incdec(1, 0);
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (op("--")) {
        char name[64];
        if (!ident(name)) fail();
        ex_lvalue(name);
        emit_incdec(-1, 0);
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (ch('*')) {
        int pe;
        ex_unary();
        pe = ex_elem;
        if (pe == 1) {
            emit_hex("48 0F BE 00");
            emit_text("\n");
            ex_type = 2;
        } else if (pe == 8) {
            emit_hex("48 8B 00");
            emit_text("\n");
            ex_type = 6;
        } else {
            emit_hex("48 63 00");
            emit_text("\n");
            ex_type = 1;
        }
        ex_elem = pe; ex_ptr = 0;
        return;
    }
    if (ch('!')) {
        ex_unary();
        emit_hex("48 83 F8 00 0F 94 C0 48 0F B6 C0");
        emit_text("\n");
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (ch('-')) {
        ex_unary();
        emit_hex("48 F7 D8");
        emit_text("\n");
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (ch('~')) {
        ex_unary();
        emit_hex("48 F7 D0");
        emit_text("\n");
        ex_type = 1; ex_elem = 4; ex_ptr = 0;
        return;
    }
    if (ch('&')) {
        char name[64];
        int pe;
        if (!ident(name)) fail();
        ex_lvalue(name);
        pe = type_size(lv_type);
        ex_type = 6; ex_elem = pe; ex_ptr = 1;
        return;
    }
    ex_primary();
}

static void ex_mul(void) {
    ex_unary();
    for (;;) {
        int k = 0;
        if (op("*")) k = 1;
        else if (op("/")) k = 2;
        else if (op("%")) k = 3;
        else break;
        emit_hex("50");
        ex_unary();
        emit_hex("48 89 C1 58");
        if (k == 1) emit_hex("48 0F AF C1");
        else {
            emit_hex("48 99 48 F7 F9");
            if (k == 3) emit_hex("48 89 D0");
        }
        emit_text("\n");
    }
}

static void ex_shift(void) {
    ex_add();
    for (;;) {
        int k = 0;
        if (op("<<")) k = 1;
        else if (op(">>")) k = 2;
        else break;
        emit_hex("50");
        ex_add();
        emit_hex("48 89 C1 58");
        if (k == 1) emit_hex("48 D3 E0");
        else emit_hex("48 D3 F8");
        emit_text("\n");
    }
}

static void ex_add(void) {
    ex_mul();
    for (;;) {
        int k = 0;
        int lt = ex_type, le = ex_elem, lp = ex_ptr;
        if (op("+")) k = 1;
        else if (op("-")) k = 2;
        else break;
        emit_hex("50");
        ex_mul();
        emit_hex("48 89 C1 58");
        if (lp) {
            if (le != 1) {
                emit_hex("48 69 C9");
                emit_imm32((u32)le);
                emit_text("\n");
            }
            if (k == 1) emit_hex("48 01 C8");
            else emit_hex("48 29 C8");
            ex_type = lt; ex_elem = le; ex_ptr = 1;
        } else {
            if (k == 1) emit_hex("48 01 C8");
            else emit_hex("48 29 C8");
            ex_type = 1; ex_elem = 4; ex_ptr = 0;
        }
        emit_text("\n");
    }
}

static void ex_rel(void) {
    ex_shift();
    for (;;) {
        int k = 0;
        if (op("<=")) k = 1;
        else if (op(">=")) k = 2;
        else if (op("<")) k = 3;
        else if (op(">")) k = 4;
        else break;
        emit_hex("50");
        ex_shift();
        emit_hex("48 89 C1 58");
        emit_hex("48 39 C8");
        if (k == 1) emit_hex("48 0F 9E C0");
        if (k == 2) emit_hex("48 0F 9D C0");
        if (k == 3) emit_hex("48 0F 9C C0");
        if (k == 4) emit_hex("48 0F 9F C0");
        emit_hex("48 0F B6 C0");
        emit_text("\n");
    }
}

static void ex_bitand(void) {
    ex_eq();
    while (op1('&')) {
        emit_hex("50");
        ex_eq();
        emit_hex("48 89 C1 58");
        emit_hex("48 21 C8");
        emit_text("\n");
    }
}

static void ex_bitxor(void) {
    ex_bitand();
    while (op1('^')) {
        emit_hex("50");
        ex_bitand();
        emit_hex("48 89 C1 58");
        emit_hex("48 31 C8");
        emit_text("\n");
    }
}

static void ex_bitor(void) {
    ex_bitxor();
    while (op1('|')) {
        emit_hex("50");
        ex_bitxor();
        emit_hex("48 89 C1 58");
        emit_hex("48 09 C8");
        emit_text("\n");
    }
}

static void ex_eq(void) {
    ex_rel();
    for (;;) {
        int k = 0;
        if (op("==")) k = 1;
        else if (op("!=")) k = 2;
        else break;
        emit_hex("50");
        ex_rel();
        emit_hex("48 89 C1 58");
        emit_hex("48 39 C8");
        if (k == 1) emit_hex("48 0F 94 C0");
        else emit_hex("48 0F 95 C0");
        emit_hex("48 0F B6 C0");
        emit_text("\n");
    }
}

static void ex_and(void) {
    ex_bitor();
    while (op("&&")) {
        int f = new_label();
        int e = new_label();
        emit_hex("48 83 F8 00 0F 84");
        emit_ref(f);
        emit_text("\n");
        ex_bitor();
        emit_hex("48 83 F8 00 0F 95 C0 48 0F B6 C0");
        emit_text("\n");
        emit_hex("E9");
        emit_ref(e);
        emit_text("\n");
        emit_label(f);
        emit_hex("B8 00 00 00 00");
        emit_text("\n");
        emit_label(e);
    }
}

static void ex_or(void) {
    ex_and();
    while (op("||")) {
        int t = new_label();
        int e = new_label();
        emit_hex("48 83 F8 00 0F 85");
        emit_ref(t);
        emit_text("\n");
        ex_and();
        emit_hex("48 83 F8 00 0F 95 C0 48 0F B6 C0");
        emit_text("\n");
        emit_hex("E9");
        emit_ref(e);
        emit_text("\n");
        emit_label(t);
        emit_hex("B8 01 00 00 00");
        emit_text("\n");
        emit_label(e);
    }
}

static void ex_expr(void) {
    if (assign_ahead()) {
        char name[64];
        int aop = assign_op;
        int dt;
        int dp;
        ident(name);
        ex_lvalue(name);
        dt = lv_type;
        dp = lv_ptr;
        if (aop == 0) {
            emit_hex("50");
            emit_text("\n");
            ch('=');
            ex_expr();
            emit_hex("59");
            emit_text("\n");
            store_rcx(dt, dp);
            ex_type = dt; ex_elem = lv_elem; ex_ptr = 0;
            return;
        }
        emit_hex("50");
        emit_text("\n");
        emit_hex("48 8B 0C 24");
        emit_text("\n");
        if (lv_type == 2) emit_hex("48 0F BE 01");
        else if (lv_type == 4 || lv_type == 6) emit_hex("48 8B 01");
        else if (lv_type == 5) emit_hex("8B 01");
        else emit_hex("48 63 01");
        emit_text("\n");
        emit_hex("50");
        emit_text("\n");
        if (aop == '+') op("+=");
        else if (aop == '-') op("-=");
        else if (aop == '*') op("*=");
        else if (aop == '/') op("/=");
        else if (aop == '%') op("%=");
        else if (aop == '&') op("&=");
        else if (aop == '|') op("|=");
        else if (aop == '^') op("^=");
        else if (aop == 'L') op("<<=");
        else if (aop == 'R') op(">>=");
        ex_expr();
        emit_hex("48 89 C1 58");
        emit_text("\n");
        if (dt == 6 && lv_elem != 1) {
            emit_hex("48 69 C9");
            emit_imm32((u32)lv_elem);
            emit_text("\n");
        }
        if (aop == '+') emit_hex("48 01 C8");
        else if (aop == '-') emit_hex("48 29 C8");
        else if (aop == '*') emit_hex("48 0F AF C1");
        else if (aop == '/') emit_hex("48 99 48 F7 F9");
        else if (aop == '%') emit_hex("48 99 48 F7 F9 48 89 D0");
        else if (aop == '&') emit_hex("48 21 C8");
        else if (aop == '|') emit_hex("48 09 C8");
        else if (aop == '^') emit_hex("48 31 C8");
        else if (aop == 'L') emit_hex("48 D3 E0");
        else if (aop == 'R') emit_hex("48 D3 E8");
        emit_text("\n");
        emit_hex("5A");
        emit_text("\n");
        if (dt == 2) emit_hex("88 02");
        else if (dt == 4 || dt == 6) emit_hex("48 89 02");
        else emit_hex("89 02");
        emit_text("\n");
        ex_type = lv_type; ex_elem = lv_elem; ex_ptr = 0;
        return;
    }
    ex_or();
    if (op("?")) {
        int e = new_label();
        int d = new_label();
        emit_hex("48 83 F8 00 0F 84");
        emit_ref(e);
        emit_text("\n");
        ex_expr();
        ch(':');
        emit_hex("E9");
        emit_ref(d);
        emit_text("\n");
        emit_label(e);
        ex_expr();
        emit_label(d);
    }
}

static void ex_stmt(void);

static void ex_block(void) {
    while (!ch('}')) {
        skip();
        stage = 4;
        if (pos >= src_len) fail();
        ex_stmt();
    }
}

static void ex_stmt(void) {
    char gname[64];
    u64 gsave;
    skip();
    if (ch(';')) return;
    gsave = pos;
    if (ident(gname)) {
        if (ch(':')) {
            emit_label(goto_id(gname));
            return;
        }
        pos = gsave;
    }
    if (word("goto")) {
        if (!ident(gname)) fail();
        ch(';');
        emit_hex("E9");
        emit_ref(goto_id(gname));
        emit_text("\n");
        return;
    }
    if (word("do")) {
        int s = new_label();
        int e = new_label();
        int oldb = brk_n;
        brk_l[brk_n] = e;
        cont_l[brk_n] = s;
        brk_n++;
        emit_label(s);
        ex_stmt();
        word("while");
        ch('(');
        ex_expr();
        ch(')');
        ch(';');
        emit_hex("48 83 F8 00 0F 85");
        emit_ref(s);
        emit_text("\n");
        emit_label(e);
        brk_n = oldb;
        return;
    }
    last_pos = pos;
    if (word("return")) {
        if (!ch(';')) {
            ex_expr();
            ch(';');
        }
        emit_hex("48 89 EC 5D C3");
        emit_text("\n");
        return;
    }
    if (word("if")) {
        int e = new_label();
        int d = new_label();
        ch('(');
        ex_expr();
        ch(')');
        emit_hex("48 83 F8 00 0F 84");
        emit_ref(e);
        emit_text("\n");
        ex_stmt();
        if (word("else")) {
            emit_hex("E9");
            emit_ref(d);
            emit_text("\n");
            emit_label(e);
            ex_stmt();
            emit_label(d);
        } else emit_label(e);
        return;
    }
    if (word("while")) {
        int s = new_label();
        int e = new_label();
        int oldb = brk_n;
        brk_l[brk_n] = e;
        cont_l[brk_n] = s;
        brk_n++;
        emit_label(s);
        ch('(');
        ex_expr();
        ch(')');
        emit_hex("48 83 F8 00 0F 84");
        emit_ref(e);
        emit_text("\n");
        ex_stmt();
        emit_hex("E9");
        emit_ref(s);
        emit_text("\n");
        emit_label(e);
        brk_n = oldb;
        return;
    }
    if (word("for")) {
        int c = new_label();
        int in = new_label();
        int e = new_label();
        int oldb = brk_n;
        u64 inc_start, inc_end, after_body;
        int depth;
        int has_cond = 0;
        ch('(');
        if (!ch(';')) {
            ex_expr();
            ch(';');
        }
        brk_l[brk_n] = e;
        cont_l[brk_n] = in;
        brk_n++;
        emit_label(c);
        if (!ch(';')) {
            ex_expr();
            ch(';');
            has_cond = 1;
        }
        inc_start = pos;
        depth = 0;
        while (pos < src_len) {
            if (src[pos] == '(') depth++;
            else if (src[pos] == ')') {
                if (!depth) break;
                depth--;
            }
            pos++;
        }
        if (pos >= src_len) fail();
        inc_end = pos;
        ch(')');
        if (has_cond) {
            emit_hex("48 83 F8 00 0F 84");
            emit_ref(e);
            emit_text("\n");
        }
        ex_stmt();
        after_body = pos;
        emit_label(in);
        pos = inc_start;
        if (inc_start < inc_end) ex_expr();
        pos = after_body;
        emit_hex("E9");
        emit_ref(c);
        emit_text("\n");
        emit_label(e);
        brk_n = oldb;
        return;
    }
    if (word("break")) {
        if (!brk_n) fail();
        ch(';');
        emit_hex("E9");
        emit_ref(brk_l[brk_n - 1]);
        emit_text("\n");
        return;
    }
    if (word("continue")) {
        if (!brk_n) fail();
        ch(';');
        emit_hex("E9");
        emit_ref(cont_l[brk_n - 1]);
        emit_text("\n");
        return;
    }
    if (ch('{')) {
        ex_block();
        return;
    }
    if (op("*")) {
        int pt;
        int pe;
        ex_primary();
        pt = 1;
        pe = ex_elem;
        ch('=');
        emit_hex("50");
        emit_text("\n");
        ex_expr();
        emit_hex("59");
        emit_text("\n");
        if (pt == 2) emit_hex("88 01");
        else if (pt == 4 || pt == 6) emit_hex("48 89 01");
        else emit_hex("89 01");
        emit_text("\n");
        ex_type = pt; ex_elem = pe; ex_ptr = 0;
        ch(';');
        return;
    }
    {
        int tw = type_word();
        int pelem = type_size(tw);
        if (!tw) {
            ex_expr();
            ch(';');
            return;
        }
        while (ch('*')) {
            tw = 6;
            pelem = type_size(last_type);
        }
        for (;;) {
            char name[64];
            int elem, dims = 0, row = 0;
            u64 total;
            if (!ident(name)) fail();
            elem = type_size(tw);
            if (ch('[')) {
                u64 n1 = number();
                ch(']');
                dims = 1;
                total = n1 * elem;
                if (ch('[')) {
                    u64 n2 = number();
                    ch(']');
                    dims = 2;
                    row = n2 * elem;
                    total = n1 * row;
                }
            } else {
                total = 8;
            }
            {
                int li = add_local(name, total);
                l_type[li] = tw;
                l_elem[li] = elem;
                l_dims[li] = dims;
                l_row[li] = row;
                l_pelem[li] = pelem;
            }
            if (ch('=')) {
                emit_addr(name);
                emit_hex("50");
                emit_text("\n");
                ex_expr();
                emit_hex("59");
                emit_text("\n");
                store_rcx(tw, 0);
            }
            if (ch(',')) continue;
            ch(';');
            break;
        }
        return;
    }
    ex_expr();
    ch(';');
}

static void ex_function(char *name) {
    int p = 0;
    if (fn_count < 1024) {
        strcpy(fn_name[fn_count], name);
        fn_type[fn_count] = cur_ret_type;
        fn_elem[fn_count] = cur_ret_elem;
        fn_count++;
    }
    strcpy(cur_func, name);
    emit_named_label(name);
    emit_hex("55 48 89 E5 48 81 EC 00 40 00 00");
    emit_text("\n");
    l_count = 0;
    ch('(');
    word("void");
    while (!ch(')')) {
        {
            int tw = type_word();
            int pelem = type_size(tw);
            char pn[64];
            int li;
            if (!tw) fail();
            while (ch('*')) {
                tw = 6;
                pelem = type_size(last_type);
            }
            if (!ident(pn)) fail();
            li = add_local(pn, 8);
            l_type[li] = tw;
            l_elem[li] = type_size(tw);
            l_pelem[li] = pelem;
            l_dims[li] = 0;
            l_row[li] = 0;
            p++;
        }
        ch(',');
    }
    {
        int i;
        for (i = 0; i < p; i++) {
            if (i == 0) emit_hex("48 89 BD");
            else if (i == 1) emit_hex("48 89 B5");
            else if (i == 2) emit_hex("48 89 95");
            else if (i == 3) emit_hex("48 89 8D");
            else if (i == 4) emit_hex("4C 89 85");
            else emit_hex("4C 89 8D");
            emit_imm32((u32)(-l_off[i]));
            emit_text("\n");
        }
    }
    if (ch('{')) {
        while (!ch('}')) {
            stage = 3;
            ex_stmt();
        }
    }
    emit_hex("48 89 EC 5D C3");
    emit_text("\n");
    (void)p;
}

static void emit_header(void) {
    emit_text("^00400000\n");
    emit_hex("7F 45 4C 46 02 01 01 00 00 00 00 00 00 00 00 00");
    emit_text("\n");
    emit_hex("02 00 3E 00 01 00 00 00 00 01 40 00 00 00 00 00");
    emit_text("\n");
    emit_hex("40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
    emit_text("\n");
    emit_hex("00 00 00 00 40 00 38 00 02 00 00 00 00 00 00 00");
    emit_text("\n");
    emit_hex("01 00 00 00 05 00 00 00 00 00 00 00 00 00 00 00");
    emit_text("\n");
    emit_hex("00 00 40 00 00 00 00 00 00 00 40 00 00 00 00 00");
    emit_text("\n");
    emit_hex("00 00 08 00 00 00 00 00 00 00 08 00 00 00 00 00 00 10 00 00 00 00 00 00");
    emit_text("\n");
    emit_hex("01 00 00 00 06 00 00 00 00 00 08 00 00 00 00 00");
    emit_text("\n");
    emit_hex("00 00 50 00 00 00 00 00 00 00 50 00 00 00 00 00");
    emit_text("\n");
    emit_hex("00 00 40 00 00 00 00 00 00 00 40 00 00 00 00 00 00 10 00 00 00 00 00 00");
    emit_text("\n");
    generated_bytes = 176;
    emit_zero(80);
    emit_text(":0100\n:start\n");
    emit_text("E8 ");
    emit_named_ref("main");
    emit_text("\n89 C7\nB8 3C 00 00 00\n0F 05\n");
    generated_bytes += 2 + 5 + 2;
    emit_named_label("__c0_getchar");
    emit_hex("48 83 EC 10 31 C0 31 FF 48 89 E6 BA 01 00 00 00 0F 05 48 83 F8 01");
    emit_hex("0F 85");
    emit_named_ref("__c0_getchar_eof");
    emit_text("\n0F B6 04 24 48 83 C4 10 C3\n");
    emit_named_label("__c0_getchar_eof");
    emit_hex("48 C7 C0 FF FF FF FF 48 83 C4 10 C3");
    emit_text("\n");
    emit_named_label("__c0_putchar");
    emit_hex("48 83 EC 10 40 88 3C 24 B8 01 00 00 00 BF 01 00 00 00 48 89 E6 BA 01 00 00 00 0F 05 0F B6 04 24 48 83 C4 10 C3");
    emit_text("\n");
    emit_named_label("__c1_sysv_write");
    emit_hex("B8 01 00 00 00 0F 05 C3");
    emit_text("\n");
    emit_named_label("__c1_sysv_read");
    emit_hex("31 C0 0F 05 C3");
    emit_text("\n");
    emit_named_label("__c1_sysv_exit");
    emit_hex("B8 3C 00 00 00 0F 05 F4");
    emit_text("\n");
}

int main(void) {
    long long n;
    src_len = 0;
    while ((n = __c1_sysv_read(0, src + src_len, 1048576 - src_len)) > 0) {
        src_len += (u64)n;
        if (src_len == 1048576) break;
    }
    emit_header();
    for (;;) {
        skip();
        stage = 1;
        if (src_len - pos <= 2) break;
        while (word("static")) skip();
        if (word("extern")) {
            while (pos < src_len && !ch(';')) pos++;
            continue;
        }
        if (word("typedef")) {
            while (pos < src_len && !ch(';')) pos++;
            continue;
        }
        if (!type_word()) {
            if (rest_end()) break;
            fail();
        }
        stage = 2;
        {
            int gtw = last_type;
            int pelem = type_size(gtw);
            char name[64];
            while (ch('*')) {
                gtw = 6;
                pelem = type_size(last_type);
            }
            if (!ident(name)) fail();
            if (ch('(')) {
                u64 after = pos;
                int depth = 1;
                while (pos < src_len && depth) {
                    if (src[pos] == '(') depth++;
                    else if (src[pos] == ')') depth--;
                    pos++;
                }
                if (ch(';')) continue;
                pos = after;
                cur_ret_type = gtw;
                cur_ret_elem = pelem;
                ex_function(name);
            } else {
                for (;;) {
                    int elem = type_size(gtw);
                    int dims = 0, row = 0;
                    u64 total = (u64)elem;
                    if (ch('[')) {
                        u64 n1 = number();
                        ch(']');
                        dims = 1;
                        total = n1 * elem;
                        if (ch('[')) {
                            u64 n2 = number();
                            ch(']');
                            dims = 2;
                            row = n2 * elem;
                            total = n1 * row;
                        }
                    }
                    if (g_count == 1024) fail();
                    strcpy(g_name[g_count], name);
                    g_type[g_count] = gtw;
                    g_elem[g_count] = elem;
                    g_dims[g_count] = dims;
                    g_row[g_count] = row;
                    g_pelem[g_count] = pelem;
                    g_size[g_count] = total;
                    g_has_init[g_count] = 0;
                    if (ch('=')) {
                        g_has_init[g_count] = 1;
                        if (ch('{')) {
                            int nv = 0;
                            g_init_off[g_count] = g_init_used;
                            for (;;) {
                                if (g_init_used + nv >= 2048) fail();
                                g_init_vals[g_init_used + nv] = (int)number();
                                nv++;
                                if (ch(',')) continue;
                                ch('}');
                                break;
                            }
                            g_init_count[g_count] = nv;
                            g_init_used += nv;
                        } else {
                            int neg = ch('-');
                            g_init[g_count] = (int)number();
                            if (neg) g_init[g_count] = -g_init[g_count];
                        }
                    }
                    g_count++;
                    if (ch(',')) {
                        if (!ident(name)) fail();
                        continue;
                    }
                    ch(';');
                    break;
                }
            }
        }
    }
    if (generated_bytes > 0x80000) fail();
    if (generated_bytes < 0x80000) emit_zero(0x80000 - generated_bytes);
    emit_text("^00500000\n.org 524288\n");
    {
        int i;
        for (i = 0; i < g_count; i++) {
            emit_named_label(g_name[i]);
            if (g_init_count[i] > 0) {
                int j;
                for (j = 0; j < (int)(g_size[i] / 4); j++) {
                    emit_imm32((u32)(j < g_init_count[i] ? g_init_vals[g_init_off[i] + j] : 0));
                    emit_text("\n");
                }
            } else if (g_has_init[i] && g_size[i] == 4) {
                emit_imm32((u32)g_init[i]);
                emit_text("\n");
            } else {
                emit_zero(g_size[i]);
            }
        }
    }
    if (generated_bytes < 0x480000) emit_zero(0x480000 - generated_bytes);
    return 0;
}
