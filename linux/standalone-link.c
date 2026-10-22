
extern long long __c1_sysv_open(const char *, long long, long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern long long __c1_sysv_close(long long);
extern void *__c1_linux_HeapAlloc(long long, unsigned int, unsigned long long);
extern int __c1_linux_ReadFile(long long, char *, int, int *, void *);
extern long long __c1_linux_CreateFileA(char *, unsigned int, int, void *, int, int,
                                        long long);
extern int __c1_linux_CloseHandle(long long);
extern int puts(char *);
extern int __c0_getchar(void);
extern void *memset(void *, int, unsigned long long);

unsigned char lk_pool[8388608];
long long lk_pool_used;
unsigned char *lk_out;
unsigned char *lk_text_image;
unsigned char *lk_data_image;
int lk_object_count;
int lk_section_count;
int lk_symbol_count;
int lk_rela_count;
int lk_global_selected[65536];

long long lk_object_bytes[32];
int lk_object_sec_first[32];
int lk_object_sec_count[32];
int lk_object_sym_first[32];
int lk_object_sym_count[32];
int lk_object_rela_first[32];
int lk_object_rela_count[32];

int lk_section_type[2048];
long long lk_section_flags[2048];
long long lk_section_addr[2048];
long long lk_section_offset[2048];
long long lk_section_size[2048];
long long lk_section_align[2048];
long long lk_section_data[2048];
int lk_section_link[2048];
int lk_section_info[2048];
int lk_section_object[2048];
int lk_section_kind[2048];
long long lk_section_out[2048];

int lk_symbol_object[65536];
int lk_symbol_bind[65536];
int lk_symbol_type[65536];
int lk_symbol_section[65536];
long long lk_symbol_value[65536];
long long lk_symbol_size[65536];
long long lk_symbol_name[65536];

int lk_rela_object[65536];
int lk_rela_section[65536];
long long lk_rela_offset[65536];
int lk_rela_symbol[65536];
int lk_rela_type[65536];
long long lk_rela_addend[65536];

long long lk_text_size;
long long lk_data_size;
long long lk_bss_size;
long long lk_text_base;
long long lk_data_base;
long long lk_bss_base;

int lk_builtin_words[112] = {
    0xc3, 0x48, 0x31, 0xed, 0x49, 0x89, 0xe4, 0x48, 0x83, 0xe4, 0xf0,
    0xe8, 0x00, 0x00, 0x00, 0x00, 0x49, 0x8b, 0x3c, 0x24, 0x49, 0x8d,
    0x74, 0x24, 0x08, 0x49, 0x8d, 0x54, 0xfc, 0x10, 0xe8, 0x00, 0x00,
    0x00, 0x00, 0x89, 0xc7, 0xb8, 0x3c, 0x00, 0x00, 0x00, 0x0f, 0x05,
    0xf4, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3, 0xb8, 0x01,
    0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3, 0xb8, 0x02, 0x00, 0x00, 0x00,
    0x0f, 0x05, 0xc3, 0xb8, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3,
    0x49, 0x89, 0xca, 0xb8, 0x09, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3,
    0xb8, 0x0b, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3, 0xb8, 0x3c, 0x00,
    0x00, 0x00, 0x0f, 0x05, 0xf4, 0xb8, 0x08, 0x00, 0x00, 0x00, 0x0f,
    0x05, 0xc3
};

int lk_u16(unsigned char *p) {
    return ((unsigned int)p[0] & 255) | (((unsigned int)p[1] & 255) << 8);
}

unsigned int lk_u32(unsigned char *p) {
    return ((unsigned int)p[0] & 255) | (((unsigned int)p[1] & 255) << 8) |
           (((unsigned int)p[2] & 255) << 16) |
           (((unsigned int)p[3] & 255) << 24);
}

long long lk_u64(unsigned char *p) {
    return (long long)((unsigned long long)lk_u32(p) |
                       ((unsigned long long)lk_u32(p + 4) << 32));
}

void lk_w32(unsigned char *p, unsigned int v) {
    int i;
    for (i = 0; i < 4; i++) {
        p[i] = (unsigned char)v;
        v = v >> 8;
    }
}

void lk_w64(unsigned char *p, long long v) {
    int i;
    for (i = 0; i < 8; i++) {
        p[i] = (unsigned char)v;
        v = v >> 8;
    }
}

long long lk_align_up(long long value, long long alignment) {
    if (alignment < 1)
        alignment = 1;
    return (value + alignment - 1) / alignment * alignment;
}

int lk_store_name(char *text) {
    int at = (int)lk_pool_used;
    int i = 0;
    while (text[i]) {
        lk_pool[lk_pool_used] = (unsigned char)text[i];
        lk_pool_used = lk_pool_used + 1;
        i = i + 1;
    }
    lk_pool[lk_pool_used] = 0;
    lk_pool_used = lk_pool_used + 1;
    return at;
}

int lk_name_eq(long long offset, char *text) {
    int i = 0;
    while (lk_pool[offset + i] && (char)lk_pool[offset + i] == text[i])
        i = i + 1;
    return lk_pool[offset + i] == 0 && text[i] == 0;
}

int lk_add_symbol(int object, int section, long long value, long long size,
                  int bind, int type, long long name) {
    int at;
    if (lk_symbol_count == 65536)
        return 1;
    at = lk_symbol_count;
    lk_symbol_count = lk_symbol_count + 1;
    lk_symbol_object[at] = object;
    lk_symbol_section[at] = section;
    lk_symbol_value[at] = value;
    lk_symbol_size[at] = size;
    lk_symbol_bind[at] = bind;
    lk_symbol_type[at] = type;
    lk_symbol_name[at] = name;
    return 0;
}

int lk_add_rela(int object, int section, long long offset, int symbol,
                long long addend, int type) {
    int at;
    if (lk_rela_count == 65536)
        return 1;
    at = lk_rela_count;
    lk_rela_count = lk_rela_count + 1;
    lk_rela_object[at] = object;
    lk_rela_section[at] = section;
    lk_rela_offset[at] = offset;
    lk_rela_symbol[at] = symbol;
    lk_rela_addend[at] = addend;
    lk_rela_type[at] = type;
    return 0;
}

int lk_add_builtin_object(void) {
    int object, section, first_symbol;
    int i, name_start, name_init, name_volatile;
    name_start = 0;
    name_init = 0;
    name_volatile = 0;
    if (lk_object_count == 32 || lk_section_count + 2 > 2048)
        return 1;
    object = lk_object_count;
    lk_object_bytes[object] = 0;
    lk_object_sec_first[object] = lk_section_count;
    lk_object_sec_count[object] = 2;
    /* Null section, then the synthetic text section. */
    section = lk_section_count;
    lk_section_type[section] = 0;
    lk_section_flags[section] = 0;
    lk_section_addr[section] = 0;
    lk_section_offset[section] = 0;
    lk_section_size[section] = 0;
    lk_section_align[section] = 0;
    lk_section_data[section] = -1;
    lk_section_link[section] = 0;
    lk_section_info[section] = 0;
    lk_section_object[section] = object;
    lk_section_kind[section] = 0;
    lk_section_out[section] = 0;
    section = section + 1;
    lk_section_type[section] = 1;
    lk_section_flags[section] = 6;
    lk_section_addr[section] = 0;
    lk_section_offset[section] = 0;
    lk_section_size[section] = 112;
    lk_section_align[section] = 16;
    lk_section_data[section] = 0;
    lk_section_link[section] = 0;
    lk_section_info[section] = 0;
    lk_section_object[section] = object;
    lk_section_kind[section] = 0;
    lk_section_out[section] = 0;
    lk_section_count = lk_section_count + 2;
    for (i = 0; i < 112; i++)
        lk_pool[i] = (unsigned char)(lk_builtin_words[i] & 255);
    lk_pool_used = 112;
    name_init = lk_store_name("__c1_init");
    name_start = lk_store_name("_start");
    lk_object_sym_first[object] = lk_symbol_count;
    if (lk_add_symbol(object, 0, 0, 0, 0, 0, -1))
        return 1;
    if (lk_add_symbol(object, 1, 1, 0, 1, 2, name_start))
        return 1;
    if (lk_add_symbol(object, 1, 0, 0, 2, 2, name_init))
        return 1;
    if (lk_add_symbol(object, 0, 0, 0, 1, 2, lk_store_name("__c1_start")))
        return 1;
    if (lk_add_symbol(object, 1, 45, 0, 1, 2, lk_store_name("__c1_sysv_read")))
        return 1;
    if (lk_add_symbol(object, 1, 53, 0, 1, 2, lk_store_name("__c1_sysv_write")))
        return 1;
    if (lk_add_symbol(object, 1, 61, 0, 1, 2, lk_store_name("__c1_sysv_open")))
        return 1;
    if (lk_add_symbol(object, 1, 69, 0, 1, 2, lk_store_name("__c1_sysv_close")))
        return 1;
    if (lk_add_symbol(object, 1, 77, 0, 1, 2, lk_store_name("__c1_sysv_mmap")))
        return 1;
    if (lk_add_symbol(object, 1, 88, 0, 1, 2, lk_store_name("__c1_sysv_munmap")))
        return 1;
    if (lk_add_symbol(object, 1, 96, 0, 1, 2, lk_store_name("__c1_sysv_exit")))
        return 1;
    name_volatile = lk_store_name("__c1_sysv_lseek");
    if (lk_add_symbol(object, 1, 104, 0, 1, 2, name_volatile))
        return 1;
    lk_object_sym_count[object] = lk_symbol_count - lk_object_sym_first[object];
    lk_object_rela_first[object] = lk_rela_count;
    if (lk_add_rela(object, 1, 12, 2, -4, 2))
        return 1;
    if (lk_add_rela(object, 1, 31, 3, -4, 2))
        return 1;
    lk_object_rela_count[object] = lk_rela_count - lk_object_rela_first[object];
    lk_object_count = lk_object_count + 1;
    return 0;
}

int lk_read_file(char *path, int object) {
    long long handle;
    long long base;
    long long limit;
    long long total;
    int done;
    int got;
    int extra;
    if (lk_pool_used >= 8388608) {
        puts("c1: link input pool is full");
        return 1;
    }
    base = lk_pool_used;
    limit = 8388608 - base;
    if (limit < 1) {
        puts("c1: link input pool is full");
        return 1;
    }
    handle = __c1_linux_CreateFileA(path, 0, 0, 0, 0, 0, 0);
    if (handle < 0) {
        puts("c1: cannot open link input");
        puts(path);
        return 1;
    }
    total = 0;
    done = 0;
    while (total < limit - 1) {
        got = __c1_linux_ReadFile(handle, (char *)(lk_pool + base + total),
                                  (int)(limit - 1 - total), &done, 0);
        if (!got)
            break;
        if (done <= 0)
            break;
        total = total + done;
    }
    if (total == limit - 1) {
        extra = 0;
        got = __c1_linux_ReadFile(handle, (char *)(lk_pool + base + total), 1,
                                  &done, 0);
        if (got && done > 0) {
            __c1_linux_CloseHandle(handle);
            puts("c1: link input exceeds pool");
            puts(path);
            return 1;
        }
    }
    __c1_linux_CloseHandle(handle);
    lk_object_bytes[object] = base;
    lk_pool_used = base + total;
    return 0;
}

int lk_parse_object(char *path) {
    int object, sec_first, sym_first, rela_first, i, j;
    long long length, shoff;
    int shentsize, shnum, shstrndx;
    unsigned char *bytes;
    unsigned char *p;
    if (lk_object_count == 32)
        return 1;
    object = lk_object_count;
    if (lk_read_file(path, object)) {
        puts("c1: cannot read link input");
        puts(path);
        return 1;
    }
    length = lk_pool_used - lk_object_bytes[object];
    bytes = lk_pool + lk_object_bytes[object];
    if (length < 64 || bytes[0] != 0x7f || bytes[1] != 'E' || bytes[2] != 'L' ||
        bytes[3] != 'F' || bytes[4] != 2) {
        puts("c1: link input is not ELF64");
        puts(path);
        return 1;
    }
    shoff = lk_u64(bytes + 40);
    shentsize = (int)lk_u16(bytes + 58);
    shnum = (int)lk_u16(bytes + 60);
    shstrndx = (int)lk_u16(bytes + 62);
    if (shentsize < 64) {
        puts("c1: link input has small section headers");
        return 1;
    }
    if (shnum < 1) {
        puts("c1: link input has no sections");
        return 1;
    }
    if (shnum > 2048) {
        puts("c1: link input has too many sections");
        return 1;
    }
    if (shstrndx >= shnum) {
        puts("c1: link input has an invalid section name table");
        return 1;
    }
    if (shoff < 0 || shoff + (long long)shnum * shentsize > length) {
        puts("c1: link input section headers are out of range");
        return 1;
    }
    if (lk_section_count + shnum > 2048) {
        puts("c1: link section capacity exceeded");
        return 1;
    }
    sec_first = lk_section_count;
    for (i = 0; i < shnum; i++) {
        int at = lk_section_count;
        p = bytes + shoff + i * shentsize;
        lk_section_type[at] = (int)lk_u32(p + 4);
        lk_section_flags[at] = lk_u64(p + 8);
        lk_section_addr[at] = lk_u64(p + 16);
        lk_section_offset[at] = lk_u64(p + 24);
        lk_section_size[at] = lk_u64(p + 32);
        lk_section_link[at] = (int)lk_u32(p + 40);
        lk_section_info[at] = (int)lk_u32(p + 44);
        lk_section_align[at] = lk_u64(p + 48);
        lk_section_object[at] = object;
        lk_section_kind[at] = 0;
        lk_section_out[at] = 0;
        lk_section_data[at] = -1;
        if (lk_section_type[at] != 8 && lk_section_size[at] > 0) {
            if (lk_section_offset[at] < 0 ||
                lk_section_offset[at] + lk_section_size[at] > length) {
                puts("c1: link input section is out of range");
                return 1;
            }
            lk_section_data[at] = lk_object_bytes[object] + lk_section_offset[at];
        }
        lk_section_count = lk_section_count + 1;
    }
    sym_first = lk_symbol_count;
    for (i = 0; i < shnum; i++) {
        int gsec = sec_first + i;
        int count;
        int strsec;
        long long strdata;
        if (lk_section_type[gsec] != 2)
            continue;
        if (lk_section_link[gsec] < 0 || lk_section_link[gsec] >= shnum) {
            puts("c1: link input has invalid symbol string table");
            return 1;
        }
        strsec = sec_first + lk_section_link[gsec];
        strdata = lk_section_data[strsec];
        if (strdata < 0) {
            puts("c1: link input has no symbol string table");
            return 1;
        }
        count = (int)(lk_section_size[gsec] / 24);
        if (lk_symbol_count + count > 65536) {
            puts("c1: link symbol capacity exceeded");
            return 1;
        }
        for (j = 0; j < count; j++) {
            long long nameoff;
            p = lk_pool + lk_section_data[gsec] + j * 24;
            nameoff = (long long)lk_u32(p);
            if (nameoff < 0 || nameoff >= lk_section_size[strsec]) {
                puts("c1: link input has invalid symbol name");
                return 1;
            }
            lk_add_symbol(object, (int)lk_u16(p + 6), lk_u64(p + 8), lk_u64(p + 16),
                          (int)((p[4] & 255) >> 4), (int)(p[4] & 15), strdata + nameoff);
        }
        break;
    }
    lk_object_sym_first[object] = sym_first;
    lk_object_sym_count[object] = lk_symbol_count - sym_first;
    rela_first = lk_rela_count;
    for (i = 0; i < shnum; i++) {
        int gsec = sec_first + i;
        int count;
        if (lk_section_type[gsec] != 4)
            continue;
        if (lk_section_data[gsec] < 0) {
            puts("c1: link input has no relocation data");
            return 1;
        }
        count = (int)(lk_section_size[gsec] / 24);
        if (lk_rela_count + count > 65536) {
            puts("c1: link relocation capacity exceeded");
            return 1;
        }
        for (j = 0; j < count; j++) {
            long long info;
            p = lk_pool + lk_section_data[gsec] + j * 24;
            info = lk_u64(p + 8);
            lk_add_rela(object, lk_section_info[gsec], lk_u64(p),
                        (int)((unsigned long long)info >> 32),
                        lk_u64(p + 16), (int)(info & 0xffffffff));
        }
    }
    lk_object_rela_first[object] = rela_first;
    lk_object_rela_count[object] = lk_rela_count - rela_first;
    /* Keep the section base before advancing the object record. */
    lk_object_sec_first[object] = sec_first;
    lk_object_sec_count[object] = shnum;
    lk_object_count = lk_object_count + 1;
    return 0;
}

int lk_find_global(char *name) {
    int i, best;
    best = -1;
    for (i = 0; i < lk_symbol_count; i++)
        if (lk_global_selected[i] >= 0 && lk_symbol_name[i] >= 0 &&
            lk_name_eq(lk_symbol_name[i], name)) {
            if (best < 0 || lk_symbol_bind[i] == 1)
                best = i;
            if (lk_symbol_bind[i] == 1)
                break;
        }
    return best;
}

long long lk_section_base(int section) {
    if (lk_section_kind[section] == 1)
        return lk_text_base + lk_section_out[section];
    if (lk_section_kind[section] == 2)
        return lk_data_base + lk_section_out[section];
    if (lk_section_kind[section] == 3)
        return lk_bss_base + lk_section_out[section];
    return 0;
}

long long lk_selected_address(int index) {
    int section;
    if (lk_symbol_section[index] == 0)
        return 0;
    section = lk_object_sec_first[lk_symbol_object[index]] +
              lk_symbol_section[index];
    return lk_section_base(section) + lk_symbol_value[index];
}

long long lk_symbol_address(int object, int index) {
    int selected;
    int sym = lk_object_sym_first[object] + index;
    if (lk_symbol_bind[sym] != 0 && lk_symbol_name[sym] >= 0 &&
        lk_pool[lk_symbol_name[sym]])
        {
            selected = lk_find_global((char *)(lk_pool + lk_symbol_name[sym]));
            if (selected >= 0 && selected != sym)
                return lk_selected_address(selected);
        }
    return lk_selected_address(sym);
}

int lk_select_globals(void) {
    int i, j;
    for (i = 0; i < lk_symbol_count; i++)
        lk_global_selected[i] = -1;
    for (i = 0; i < lk_symbol_count; i++) {
        if (lk_symbol_bind[i] == 0 || lk_symbol_name[i] < 0 ||
            lk_symbol_section[i] == 0)
            continue;
        j = lk_find_global((char *)(lk_pool + lk_symbol_name[i]));
        if (j < 0 || (lk_symbol_bind[j] == 2 && lk_symbol_bind[i] == 1))
            lk_global_selected[i] = i;
        else if (j >= 0 && lk_symbol_bind[j] == 2 && lk_symbol_bind[i] == 2)
            lk_global_selected[j] = j;
    }
    return 0;
}

int lk_layout(void) {
    int i;
    for (i = 0; i < lk_section_count; i++) {
        if (!(lk_section_flags[i] & 2))
            continue;
        if (lk_section_type[i] == 1 && (lk_section_flags[i] & 4)) {
            lk_section_kind[i] = 1;
            lk_section_out[i] = lk_align_up(lk_text_size, lk_section_align[i]);
            lk_text_size = lk_section_out[i] + lk_section_size[i];
        } else if (lk_section_type[i] == 8) {
            lk_section_kind[i] = 3;
            lk_section_out[i] = lk_align_up(lk_bss_size, lk_section_align[i]);
            lk_bss_size = lk_section_out[i] + lk_section_size[i];
        } else if (lk_section_type[i] == 1) {
            lk_section_kind[i] = 2;
            lk_section_out[i] = lk_align_up(lk_data_size, lk_section_align[i]);
            lk_data_size = lk_section_out[i] + lk_section_size[i];
        }
    }
    lk_text_base = 0x401000;
    lk_data_base = lk_align_up(lk_text_base + lk_text_size, 0x1000);
    lk_bss_base = lk_data_base + lk_data_size;
    return 0;
}

int lk_build_images(void) {
    int i, j;
    lk_text_image = __c1_linux_HeapAlloc(0, 0, lk_text_size + 1);
    lk_data_image = __c1_linux_HeapAlloc(0, 0, lk_data_size + 1);
    if (!lk_text_image || !lk_data_image)
        return 1;
    memset(lk_text_image, 0, lk_text_size);
    memset(lk_data_image, 0, lk_data_size);
    for (i = 0; i < lk_section_count; i++) {
        if (lk_section_data[i] < 0 || lk_section_size[i] <= 0)
            continue;
        for (j = 0; j < lk_section_size[i]; j++) {
            if (lk_section_kind[i] == 1)
                lk_text_image[lk_section_out[i] + j] = lk_pool[lk_section_data[i] + j];
            else if (lk_section_kind[i] == 2)
                lk_data_image[lk_section_out[i] + j] = lk_pool[lk_section_data[i] + j];
        }
    }
    return 0;
}

int lk_apply_relocations(void) {
    int i;
    long long target, place, value;
    int section, sym;
    unsigned char *dest;
    for (i = 0; i < lk_rela_count; i++) {
        section = lk_object_sec_first[lk_rela_object[i]] + lk_rela_section[i];
        if (lk_section_kind[section] == 1)
            dest = lk_text_image + lk_section_out[section];
        else if (lk_section_kind[section] == 2)
            dest = lk_data_image + lk_section_out[section];
        else
            continue;
        sym = lk_object_sym_first[lk_rela_object[i]] + lk_rela_symbol[i];
        target = lk_symbol_address(lk_rela_object[i], lk_rela_symbol[i]);
        if (target == 0 && lk_symbol_section[sym] == 0) {
            puts("c1: unresolved symbol in standalone link");
            return 1;
        }
        place = lk_section_base(section) + lk_rela_offset[i];
        if (lk_rela_type[i] == 2 || lk_rela_type[i] == 4) {
            value = target + lk_rela_addend[i] - place;
            if (value < -2147483647LL - 1 || value > 2147483647LL) {
                puts("c1: relocation overflow in standalone link");
                return 1;
            }
            lk_w32(dest + lk_rela_offset[i], (unsigned int)value);
        } else {
            puts("c1: unsupported relocation in standalone link");
            return 1;
        }
    }
    return 0;
}

int lk_pool_strlen(long long offset) {
    int n = 0;
    while (lk_pool[offset + n])
        n = n + 1;
    return n;
}

int lk_write_cstr(long long at, char *text) {
    int i = 0;
    while (text[i]) {
        lk_out[at + i] = (unsigned char)text[i];
        i = i + 1;
    }
    lk_out[at + i] = 0;
    return i + 1;
}

int lk_write_output(char *path, long long entry) {
    long long image_base = 0x400000;
    long long text_file = 0x1000;
    long long data_file, data_end, sym_file, str_file, shstr_file, shoff;
    long long file_size, sym_size, str_size, total_names;
    int i, j, nsym, sym_index, section, shndx;
    int fd;
    long long value, offset, written;
    unsigned char *p;
    unsigned char *symtab;
    unsigned char *strtab;
    unsigned char *sh;
    nsym = 1;
    str_size = 1;
    for (i = 0; i < lk_symbol_count; i++) {
        if (lk_symbol_bind[i] == 0 || lk_symbol_section[i] == 0 ||
            lk_symbol_name[i] < 0 || !lk_pool[lk_symbol_name[i]])
            continue;
        nsym = nsym + 1;
        str_size = str_size + lk_pool_strlen(lk_symbol_name[i]) + 1;
    }
    data_file = lk_align_up(text_file + lk_text_size, 0x1000);
    data_end = data_file + lk_data_size;
    sym_file = lk_align_up(data_end, 8);
    sym_size = (long long)nsym * 24;
    str_file = sym_file + sym_size;
    shstr_file = str_file + str_size;
    shoff = lk_align_up(shstr_file + 44, 8);
    file_size = shoff + 7 * 64;
    lk_out = __c1_linux_HeapAlloc(0, 0, file_size + 1);
    if (!lk_out)
        return 1;
    memset(lk_out, 0, file_size);
    p = lk_out;
    p[0] = 0x7f;
    p[1] = 'E';
    p[2] = 'L';
    p[3] = 'F';
    p[4] = 2;
    p[5] = 1;
    p[6] = 1;
    lk_w32(p + 16, 2);
    lk_w32(p + 18, 62);
    lk_w32(p + 20, 1);
    lk_w64(p + 24, entry);
    lk_w64(p + 32, 64);
    lk_w64(p + 40, shoff);
    lk_w32(p + 48, 0);
    lk_w32(p + 52, 64);
    lk_w32(p + 54, 56);
    lk_w32(p + 56, 4);
    lk_w32(p + 58, 64);
    lk_w32(p + 60, 7);
    lk_w32(p + 62, 6);
    p = lk_out + 64;
    lk_w32(p, 6);
    lk_w32(p + 4, 5);
    lk_w64(p + 8, 64);
    lk_w64(p + 16, image_base + 0x40);
    lk_w64(p + 24, image_base + 0x40);
    lk_w64(p + 32, 288);
    lk_w64(p + 40, 288);
    lk_w64(p + 48, 8);
    p = p + 56;
    lk_w32(p, 1);
    lk_w32(p + 4, 5);
    lk_w64(p + 8, 0);
    lk_w64(p + 16, image_base);
    lk_w64(p + 24, image_base);
    lk_w64(p + 32, text_file + lk_text_size);
    lk_w64(p + 40, text_file + lk_text_size);
    lk_w64(p + 48, 0x1000);
    p = p + 56;
    lk_w32(p, 1);
    lk_w32(p + 4, 6);
    lk_w64(p + 8, data_file);
    lk_w64(p + 16, lk_data_base);
    lk_w64(p + 24, lk_data_base);
    lk_w64(p + 32, lk_data_size);
    lk_w64(p + 40, lk_data_size + lk_bss_size);
    lk_w64(p + 48, 0x1000);
    p = p + 56;
    lk_w32(p, 0x6474e551);
    lk_w32(p + 4, 6);
    lk_w64(p + 8, 0);
    lk_w64(p + 16, 0);
    lk_w64(p + 24, 0);
    lk_w64(p + 32, 0);
    lk_w64(p + 40, 0);
    lk_w64(p + 48, 0x10);
    for (i = 0; i < lk_text_size; i++)
        lk_out[text_file + i] = lk_text_image[i];
    for (i = 0; i < lk_data_size; i++)
        lk_out[data_file + i] = lk_data_image[i];
    symtab = lk_out + sym_file;
    strtab = lk_out + str_file;
    strtab[0] = 0;
    str_size = 1;
    sym_index = 1;
    for (i = 0; i < lk_symbol_count; i++) {
        if (lk_symbol_bind[i] == 0 || lk_symbol_section[i] == 0 ||
            lk_symbol_name[i] < 0 || !lk_pool[lk_symbol_name[i]])
            continue;
        section = lk_object_sec_first[lk_symbol_object[i]] +
                  lk_symbol_section[i];
        shndx = 0;
        value = 0;
        if (lk_section_kind[section] == 1) {
            shndx = 1;
            value = lk_text_base + lk_section_out[section] +
                    lk_symbol_value[i];
        } else if (lk_section_kind[section] == 2) {
            shndx = 2;
            value = lk_data_base + lk_section_out[section] +
                    lk_symbol_value[i];
        } else if (lk_section_kind[section] == 3) {
            shndx = 3;
            value = lk_bss_base + lk_section_out[section] +
                    lk_symbol_value[i];
        } else
            continue;
        p = symtab + sym_index * 24;
        lk_w32(p, (unsigned int)str_size);
        p[4] = (unsigned char)((lk_symbol_bind[i] << 4) | lk_symbol_type[i]);
        p[5] = 0;
        lk_w32(p + 6, (unsigned int)shndx);
        lk_w64(p + 8, value);
        lk_w64(p + 16, lk_symbol_size[i]);
        offset = lk_symbol_name[i];
        j = 0;
        while (lk_pool[offset + j]) {
            strtab[str_size] = lk_pool[offset + j];
            str_size = str_size + 1;
            j = j + 1;
        }
        strtab[str_size] = 0;
        str_size = str_size + 1;
        sym_index = sym_index + 1;
    }
    lk_out[shstr_file] = 0;
    offset = shstr_file + 1;
    offset = offset + lk_write_cstr(offset, ".text");
    offset = offset + lk_write_cstr(offset, ".data");
    offset = offset + lk_write_cstr(offset, ".bss");
    offset = offset + lk_write_cstr(offset, ".symtab");
    offset = offset + lk_write_cstr(offset, ".strtab");
    offset = offset + lk_write_cstr(offset, ".shstrtab");
    sh = lk_out + shoff;
    sh = sh + 64;
    lk_w32(sh, 1);
    lk_w32(sh + 4, 1);
    lk_w64(sh + 8, 6);
    lk_w64(sh + 16, lk_text_base);
    lk_w64(sh + 24, text_file);
    lk_w64(sh + 32, lk_text_size);
    lk_w32(sh + 40, 0);
    lk_w32(sh + 44, 0);
    lk_w64(sh + 48, 16);
    lk_w64(sh + 56, 0);
    sh = sh + 64;
    lk_w32(sh, 7);
    lk_w32(sh + 4, 1);
    lk_w64(sh + 8, 3);
    lk_w64(sh + 16, lk_data_base);
    lk_w64(sh + 24, data_file);
    lk_w64(sh + 32, lk_data_size);
    lk_w32(sh + 40, 0);
    lk_w32(sh + 44, 0);
    lk_w64(sh + 48, 16);
    lk_w64(sh + 56, 0);
    sh = sh + 64;
    lk_w32(sh, 13);
    lk_w32(sh + 4, 8);
    lk_w64(sh + 8, 3);
    lk_w64(sh + 16, lk_bss_base);
    lk_w64(sh + 24, data_end);
    lk_w64(sh + 32, lk_bss_size);
    lk_w32(sh + 40, 0);
    lk_w32(sh + 44, 0);
    lk_w64(sh + 48, 16);
    lk_w64(sh + 56, 0);
    sh = sh + 64;
    lk_w32(sh, 18);
    lk_w32(sh + 4, 2);
    lk_w64(sh + 8, 0);
    lk_w64(sh + 16, 0);
    lk_w64(sh + 24, sym_file);
    lk_w64(sh + 32, sym_size);
    lk_w32(sh + 40, 5);
    lk_w32(sh + 44, 1);
    lk_w64(sh + 48, 8);
    lk_w64(sh + 56, 24);
    sh = sh + 64;
    lk_w32(sh, 26);
    lk_w32(sh + 4, 3);
    lk_w64(sh + 8, 0);
    lk_w64(sh + 16, 0);
    lk_w64(sh + 24, str_file);
    lk_w64(sh + 32, str_size);
    lk_w32(sh + 40, 0);
    lk_w32(sh + 44, 0);
    lk_w64(sh + 48, 1);
    lk_w64(sh + 56, 0);
    sh = sh + 64;
    lk_w32(sh, 34);
    lk_w32(sh + 4, 3);
    lk_w64(sh + 8, 0);
    lk_w64(sh + 16, 0);
    lk_w64(sh + 24, shstr_file);
    lk_w64(sh + 32, 44);
    lk_w32(sh + 40, 0);
    lk_w32(sh + 44, 0);
    lk_w64(sh + 48, 1);
    lk_w64(sh + 56, 0);
    fd = (int)__c1_sysv_open(path, 1 | 64 | 512, 493);
    if (fd < 0) {
        puts("c1: cannot create standalone output");
        return 1;
    }
    offset = 0;
    written = 0;
    while (offset < file_size) {
        written = __c1_sysv_write(fd, lk_out + offset,
                                  (unsigned long long)(file_size - offset));
        if (written <= 0) {
            __c1_sysv_close(fd);
            return 1;
        }
        offset = offset + written;
    }
    __c1_sysv_close(fd);
    return 0;
}

char lk_paths[64][512];
int lk_path_count;

int lk_link(void) {
    int i, entry;
    if (lk_path_count < 3) {
        puts("c1l: manifest needs output, entry and object paths");
        return 2;
    }
    if (lk_add_builtin_object()) {
        puts("c1: standalone link capacity exceeded");
        return 1;
    }
    for (i = 2; i < lk_path_count; i++)
        if (lk_parse_object(lk_paths[i])) {
            puts("c1: invalid object for standalone link");
            return 1;
        }
    lk_select_globals();
    entry = -1;
    for (i = 0; i < lk_symbol_count; i++)
        if (lk_symbol_bind[i] != 0 && lk_symbol_name[i] >= 0 &&
            lk_name_eq(lk_symbol_name[i], lk_paths[1]) &&
            lk_symbol_section[i] != 0)
            entry = i;
    if (entry < 0) {
        puts("c1: missing standalone entry");
        return 1;
    }
    lk_layout();
    if (lk_build_images() || lk_apply_relocations())
        return 1;
    if (lk_write_output(lk_paths[0], lk_selected_address(entry)))
        return 1;
    puts("c1: linked");
    return 0;
}

int main(void) {
    int c, line;
    line = 0;
    lk_path_count = 0;
    lk_paths[0][0] = 0;
    c = __c0_getchar();
    while (c != -1) {
        if (c == 10) {
            lk_paths[lk_path_count][line] = 0;
            if (line) {
                lk_path_count = lk_path_count + 1;
                line = 0;
                if (lk_path_count == 64)
                    break;
                lk_paths[lk_path_count][0] = 0;
            }
        } else if (line < 511) {
            lk_paths[lk_path_count][line] = (char)c;
            line = line + 1;
        }
        c = __c0_getchar();
    }
    if (line) {
        lk_paths[lk_path_count][line] = 0;
        lk_path_count = lk_path_count + 1;
    }
    return lk_link();
}
