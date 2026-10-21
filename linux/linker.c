typedef long long ssize_t;
typedef long long off_t;
typedef unsigned long long size_t;
#define NULL ((void *)0)
#define SEEK_SET 0
#define SEEK_END 2

extern long long __c1_sysv_open(const char *, long long, long long);
extern long long __c1_sysv_read(long long, void *, unsigned long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern long long __c1_sysv_close(long long);
extern long long __c1_sysv_lseek(long long, long long, long long);
extern void *__c1_linux_HeapAlloc(long long, unsigned int, unsigned long long);
extern int __c1_linux_HeapFree(long long, unsigned int, void *);
extern void *memcpy(void *, const void *, unsigned long long);
extern void *memset(void *, int, unsigned long long);
extern int puts(char *);
typedef unsigned long long u64;
typedef unsigned int u32;

#define MAX_OBJECTS 64
#define MAX_SECTIONS 2048
#define MAX_SYMBOLS 65536
#define MAX_RELAS 65536
#define MAX_NAME 128

struct section {
    char name[64];
    u32 type;
    u64 flags, addr, offset, size, align, entsize;
    u32 link, info;
    unsigned char *data;
    int object;
    int out_kind;
    u64 out_off;
};

struct symbol {
    char name[MAX_NAME];
    int bind, type, shndx;
    u64 value, size;
    int object;
};

struct rela {
    int object, target_sec, symbol;
    u64 offset;
    long long addend;
    u32 type;
};

struct object {
    unsigned char *bytes;
    u64 length;
    int sec_first, sec_count;
    int sym_first, sym_count;
    int rela_first, rela_count;
    int is_archive;
    int active;
    int sec_map[MAX_SECTIONS];
};

static struct section sections[MAX_SECTIONS];
static struct symbol symbols[MAX_SYMBOLS];
static struct rela relas[MAX_RELAS];
static struct object objects[MAX_OBJECTS];
static int section_count, symbol_count, rela_count, object_count;
static u64 text_size, data_size, bss_size;
static unsigned char *text_image, *data_image;
static u64 text_base, data_base, bss_base;
static int global_selected[MAX_SYMBOLS];

static u32 get16(unsigned char *p) {
    return (u32)p[0] | ((u32)p[1] << 8);
}

static u32 get32(unsigned char *p) {
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

static u64 get64(unsigned char *p) {
    return (u64)get32(p) | ((u64)get32(p + 4) << 32);
}

static void put32(unsigned char *p, u32 v) {
    int i;
    for (i = 0; i < 4; i++) {
        p[i] = (unsigned char)v;
        v >>= 8;
    }
}

static void put64(unsigned char *p, u64 v) {
    int i;
    for (i = 0; i < 8; i++) {
        p[i] = (unsigned char)v;
        v >>= 8;
    }
}

static u64 align_up(u64 value, u64 alignment) {
    if (alignment < 1)
        alignment = 1;
    return (value + alignment - 1) / alignment * alignment;
}

static int read_file(const char *path, unsigned char **out, u64 *length) {
    int fd;
    off_t size, offset;
    unsigned char *data;
    ssize_t got;
    fd = (int)__c1_sysv_open(path, 0, 0);
    if (fd < 0) {
        puts("c1link: cannot open file");
        return 1;
    }
    size = (off_t)__c1_sysv_lseek(fd, 0, SEEK_END);
    if (size < 0) {
        __c1_sysv_close(fd);
        return 1;
    }
    if (__c1_sysv_lseek(fd, 0, SEEK_SET) < 0) {
        __c1_sysv_close(fd);
        return 1;
    }
    data = __c1_linux_HeapAlloc(0, 0, (unsigned long long)size + 1);
    if (!data) {
        __c1_sysv_close(fd);
        return 1;
    }
    offset = 0;
    while (offset < size) {
        got = (ssize_t)__c1_sysv_read(fd, data + offset, (unsigned long long)(size - offset));
        if (got <= 0) {
            __c1_sysv_close(fd);
            return 1;
        }
        offset += got;
    }
    __c1_sysv_close(fd);
    *out = data;
    *length = (u64)size;
    return 0;
}

static int write_file(const char *path, unsigned char *data, u64 length) {
    int fd;
    u64 offset = 0;
    ssize_t written;
    fd = (int)__c1_sysv_open(path, 1 | 64 | 512, 493);
    if (fd < 0) {
        puts("c1link: cannot create output");
        return 1;
    }
    while (offset < length) {
        written = (ssize_t)__c1_sysv_write(fd, data + offset, (unsigned long long)(length - offset));
        if (written <= 0) {
            __c1_sysv_close(fd);
            return 1;
        }
        offset += (u64)written;
    }
    __c1_sysv_close(fd);
    return 0;
}

static void copy_name(char *to, unsigned char *from, int max) {
    int i = 0;
    while (i < max - 1 && from[i]) {
        to[i] = (char)from[i];
        i++;
    }
    to[i] = 0;
}

static int parse_memory(const char *path, unsigned char *bytes, u64 length) {
    struct object *o;
    unsigned char *shstr;
    u64 shstr_size;
    u64 shoff;
    int shentsize, shnum, shstrndx, i, j, ns, nr;
    if (object_count == MAX_OBJECTS)
        return 1;
    o = &objects[object_count];
    o->bytes = bytes;
    o->length = length;
    o->is_archive = 0;
    o->active = 1;
    if (o->length < 64 || o->bytes[0] != 0x7f || o->bytes[1] != 'E' ||
        o->bytes[2] != 'L' || o->bytes[3] != 'F' || o->bytes[4] != 2) {
        return 1;
    }
    shoff = get64(o->bytes + 40);
    shentsize = (int)get16(o->bytes + 58);
    shnum = (int)get16(o->bytes + 60);
    shstrndx = (int)get16(o->bytes + 62);
    if (shnum > MAX_SECTIONS || shstrndx >= shnum) {
        return 1;
    }
    o->sec_first = section_count;
    o->sec_count = shnum;
    for (i = 0; i < shnum; i++)
        o->sec_map[i] = section_count + i;
    shstr = o->bytes + get64(o->bytes + shoff + shstrndx * shentsize + 24);
    shstr_size = get64(o->bytes + shoff + shstrndx * shentsize + 32);
    for (i = 0; i < shnum; i++) {
        struct section *s = &sections[section_count++];
        unsigned char *p = o->bytes + shoff + i * shentsize;
        s->type = get32(p + 4);
        s->flags = get64(p + 8);
        s->addr = get64(p + 16);
        s->offset = get64(p + 24);
        s->size = get64(p + 32);
        s->link = get32(p + 40);
        s->info = get32(p + 44);
        s->align = get64(p + 48);
        s->entsize = get64(p + 56);
        s->object = object_count;
        s->out_kind = 0;
        s->out_off = 0;
        if (get32(p) < shstr_size)
            copy_name(s->name, shstr + get32(p), 64);
        else
            s->name[0] = 0;
        if (s->type != 8 && s->size)
            s->data = o->bytes + s->offset;
        else
            s->data = 0;
    }
    o->sym_first = symbol_count;
    for (i = 0; i < shnum; i++) {
        struct section *s = &sections[o->sec_first + i];
        if (s->type != 2) {
            continue;
        }
        struct section *str = &sections[o->sec_first + s->link];
        ns = (int)(s->size / 24);
        for (j = 0; j < ns; j++) {
            unsigned char *p = s->data + j * 24;
            struct symbol *y;
            if (symbol_count == MAX_SYMBOLS)
                return 1;
            y = &symbols[symbol_count++];
            y->bind = p[4] >> 4;
            y->type = p[4] & 15;
            y->shndx = (int)get16(p + 6);
            y->value = get64(p + 8);
            y->size = get64(p + 16);
            y->object = object_count;
            copy_name(y->name, str->data + get32(p), MAX_NAME);
        }
        break;
    }
    o->sym_count = symbol_count - o->sym_first;
    o->rela_first = rela_count;
    for (i = 0; i < shnum; i++) {
        struct section *s = &sections[o->sec_first + i];
        if (s->type != 4)
            continue;
        nr = (int)(s->size / 24);
        for (j = 0; j < nr; j++) {
            unsigned char *p = s->data + j * 24;
            struct rela *r;
            if (rela_count == MAX_RELAS)
                return 1;
            r = &relas[rela_count++];
            r->object = object_count;
            r->target_sec = (int)s->info;
            r->offset = get64(p);
            r->symbol = (int)(get64(p + 8) >> 32);
            r->type = (u32)get64(p + 8);
            r->addend = (long long)get64(p + 16);
        }
    }
    o->rela_count = rela_count - o->rela_first;
    object_count++;
    return 0;
}

static int same_name(const char *a, const char *b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

static int find_global(const char *name) {
    int i, best = -1;
    for (i = 0; i < symbol_count; i++)
        if (global_selected[i] >= 0 && same_name(symbols[i].name, name)) {
            if (best < 0 || symbols[i].bind == 1)
                best = i;
            if (symbols[i].bind == 1)
                break;
        }
    return best;
}

static u64 section_out_base(int global_section) {
    struct section *s = &sections[global_section];
    if (s->out_kind == 1)
        return text_base + s->out_off;
    if (s->out_kind == 2)
        return data_base + s->out_off;
    if (s->out_kind == 3)
        return bss_base + s->out_off;
    return 0;
}

static u64 symbol_address(int obj_index, int index);

static u64 selected_address(int index) {
    struct symbol *s = &symbols[index];
    if (s->shndx == 0)
        return 0;
    return section_out_base(objects[s->object].sec_map[s->shndx]) + s->value;
}

static u64 symbol_address(int obj_index, int index) {
    struct symbol *s = &symbols[objects[obj_index].sym_first + index];
    int selected;
    if (s->bind != 0 && s->name[0]) {
        selected = find_global(s->name);
        if (selected >= 0 && selected != objects[obj_index].sym_first + index)
            return selected_address(selected);
    }
    return selected_address(objects[obj_index].sym_first + index);
}

static int layout_sections(void) {
    int i;
    for (i = 0; i < section_count; i++) {
        struct section *s = &sections[i];
        if (!(s->flags & 2) || !objects[s->object].active)
            continue;
        if (s->type == 1 && (s->flags & 4)) {
            s->out_kind = 1;
            s->out_off = align_up(text_size, s->align);
            text_size = s->out_off + s->size;
        } else if (s->type == 8) {
            s->out_kind = 3;
            s->out_off = align_up(bss_size, s->align);
            bss_size = s->out_off + s->size;
        } else if (s->type == 1) {
            s->out_kind = 2;
            s->out_off = align_up(data_size, s->align);
            data_size = s->out_off + s->size;
        }
    }
    text_base = 0x401000;
    data_base = align_up(text_base + text_size, 0x1000);
    bss_base = data_base + data_size;
    return 0;
}

static int build_images(void) {
    int i;
    text_image = __c1_linux_HeapAlloc(0, 0, text_size + 1);
    data_image = __c1_linux_HeapAlloc(0, 0, data_size + 1);
    if (!text_image || !data_image)
        return 1;
    memset(text_image, 0, (size_t)text_size);
    memset(data_image, 0, (size_t)data_size);
    for (i = 0; i < section_count; i++) {
        struct section *s = &sections[i];
        if (!objects[s->object].active || !s->data || !s->size)
            continue;
        if (s->out_kind == 1)
            memcpy(text_image + s->out_off, s->data, (size_t)s->size);
        else if (s->out_kind == 2)
            memcpy(data_image + s->out_off, s->data, (size_t)s->size);
    }
    return 0;
}

static int apply_relocations(void) {
    int i;
    for (i = 0; i < rela_count; i++) {
        struct rela *r = &relas[i];
        struct object *o = &objects[r->object];
        int gsec;
        if (!o->active)
            continue;
        gsec = o->sec_map[r->target_sec];
        struct section *s = &sections[gsec];
        unsigned char *dest;
        u64 place, target;
        long long value;
        if (s->out_kind == 1)
            dest = text_image + s->out_off;
        else if (s->out_kind == 2)
            dest = data_image + s->out_off;
        else
            continue;
        target = symbol_address(r->object, r->symbol);
        if (!target && symbols[o->sym_first + r->symbol].shndx == 0) {
            puts("c1link: unresolved symbol");
            return 1;
        }
        place = section_out_base(gsec) + r->offset;
        if (r->type == 1) {
            put64(dest + r->offset, target + (u64)r->addend);
        } else if (r->type == 2 || r->type == 4) {
            value = (long long)(target + (u64)r->addend - place);
            if (value < -2147483647LL - 1 || value > 2147483647LL) {
                puts("c1link: relocation overflow");
                return 1;
            }
            put32(dest + r->offset, (u32)value);
        } else {
            puts("c1link: unsupported relocation");
            return 1;
        }
    }
    return 0;
}

static int active_references(const char *name) {
    int i, j;
    for (i = 0; i < object_count; i++) {
        struct object *o = &objects[i];
        if (!o->active)
            continue;
        for (j = 0; j < o->sym_count; j++) {
            struct symbol *s = &symbols[o->sym_first + j];
            if (s->shndx == 0 && s->bind != 0 && same_name(s->name, name))
                return 1;
        }
    }
    return 0;
}

static int activate_archives(void) {
    int changed, i, j, needed;
    do {
        changed = 0;
        for (i = 0; i < object_count; i++) {
            struct object *o = &objects[i];
            if (!o->is_archive || o->active)
                continue;
            needed = 0;
            for (j = 0; j < o->sym_count; j++) {
                struct symbol *s = &symbols[o->sym_first + j];
                if (s->shndx != 0 && s->bind != 0 && s->name[0] && active_references(s->name)) {
                    needed = 1;
                    break;
                }
            }
            if (needed) {
                o->active = 1;
                changed = 1;
            }
        }
    } while (changed);
    return 0;
}

static int select_globals(void) {
    int i, j;
    for (i = 0; i < symbol_count; i++)
        global_selected[i] = -1;
    for (i = 0; i < symbol_count; i++) {
        struct symbol *s = &symbols[i];
        if (s->bind == 0 || !s->name[0] || s->shndx == 0 || !objects[s->object].active)
            continue;
        j = find_global(s->name);
        if (j < 0 || (symbols[j].bind == 2 && s->bind == 1))
            global_selected[i] = i;
        else if (j >= 0 && symbols[j].bind == 2 && s->bind == 2)
            global_selected[j] = j;
    }
    return 0;
}

static int write_output(const char *path, u64 entry) {
    u64 text_file = 0x1000;
    u64 data_file = align_up(text_file + text_size, 0x1000);
    u64 file_size = data_file + data_size;
    unsigned char *image = __c1_linux_HeapAlloc(0, 0, file_size + 1);
    unsigned char *p;
    int i;
    if (!image)
        return 1;
    memset(image, 0, (size_t)file_size);
    p = image;
    p[0] = 0x7f; p[1] = 'E'; p[2] = 'L'; p[3] = 'F';
    p[4] = 2; p[5] = 1; p[6] = 1;
    put32(p + 16, 2);
    put32(p + 18, 62);
    put32(p + 20, 1);
    put64(p + 24, entry);
    put64(p + 32, 64);
    put64(p + 40, 0);
    put32(p + 48, 0);
    put32(p + 52, 64);
    put32(p + 54, 56);
    put32(p + 56, 4);
    put32(p + 58, 64);
    put32(p + 60, 0);
    put32(p + 62, 0);
    p = image + 64;
    put32(p, 6); put32(p + 4, 5);
    put64(p + 8, 64); put64(p + 16, 0x400040);
    put64(p + 24, 0x400040); put64(p + 32, 288);
    put64(p + 40, 288); put64(p + 48, 8);
    p += 56;
    put32(p, 1); put32(p + 4, 5);
    put64(p + 8, 0); put64(p + 16, 0x400000);
    put64(p + 24, 0x400000); put64(p + 32, text_file + text_size);
    put64(p + 40, text_file + text_size); put64(p + 48, 0x1000);
    p += 56;
    put32(p, 1); put32(p + 4, 6);
    put64(p + 8, text_file + text_size); put64(p + 16, data_base);
    put64(p + 24, data_base); put64(p + 32, data_size);
    put64(p + 40, data_size + bss_size); put64(p + 48, 0x1000);
    p += 56;
    put32(p, 0x6474e551); put32(p + 4, 6);
    put64(p + 8, 0); put64(p + 16, 0);
    put64(p + 24, 0); put64(p + 32, 0);
    put64(p + 40, 0); put64(p + 48, 0x10);
    memcpy(image + text_file, text_image, (size_t)text_size);
    if (data_size)
        memcpy(image + data_file, data_image, (size_t)data_size);
    i = write_file(path, image, file_size);
    __c1_linux_HeapFree(0, 0, image);
    return i;
}

static int parse_object(const char *path) {
    unsigned char *bytes;
    u64 length;
    if (read_file(path, &bytes, &length))
        return 1;
    return parse_memory(path, bytes, length);
}

static u64 ar_number(unsigned char *p) {
    int i;
    u64 n = 0;
    for (i = 0; i < 10 && p[i] >= '0' && p[i] <= '9'; i++)
        n = n * 10 + (u64)(p[i] - '0');
    return n;
}

static int parse_archive(const char *path, unsigned char *bytes, u64 length) {
    u64 off, size;
    unsigned char *header, *data;
    char name[64];
    if (length < 8)
        return 1;
    off = 8;
    while (off + 60 <= length) {
        header = bytes + off;
        if (header[58] != '`' || header[59] != 10)
            return 1;
        size = ar_number(header + 48);
        data = header + 60;
        if (data + size > bytes + length)
            return 1;
        copy_name(name, header, 16);
        if (!(name[0] == '/' && (name[1] == ' ' || name[1] == '/'))) {
            if (parse_memory(path, data, size))
                return 1;
            objects[object_count - 1].is_archive = 1;
            objects[object_count - 1].active = 0;
        }
        off = (u64)(data + size - bytes);
        if (off & 1)
            off++;
    }
    return 0;
}

static int parse_input(const char *path) {
    unsigned char *bytes;
    u64 length;
    if (read_file(path, &bytes, &length))
        return 1;
    if (length >= 8 && bytes[0] == '!' && bytes[1] == '<' && bytes[2] == 'a' &&
        bytes[3] == 'r' && bytes[4] == 'c' && bytes[5] == 'h' && bytes[6] == '>' &&
        bytes[7] == 10)
        return parse_archive(path, bytes, length);
    return parse_memory(path, bytes, length);
}

int main(int argc, char **argv) {
    int i, entry_index = -1;
    if (argc < 4) {
        puts("usage: c1link output entry object...");
        return 2;
    }
    for (i = 3; i < argc; i++)
        if (parse_input(argv[i])) {
            puts("c1link: invalid object");
            return 1;
        }
    activate_archives();
    select_globals();
    for (i = 0; i < symbol_count; i++)
        if (symbols[i].bind != 0 && same_name(symbols[i].name, argv[2]) &&
            symbols[i].shndx != 0 && objects[symbols[i].object].active)
            entry_index = i;
    if (entry_index < 0) {
        puts("c1link: missing entry");
        return 1;
    }
    layout_sections();
    if (build_images() || apply_relocations())
        return 1;
    if (write_output(argv[1], selected_address(entry_index)))
        return 1;
    puts("c1link: linked");
    return 0;
}
