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

static unsigned char builtin_startup[] = {
    0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
    0x09, 0x00, 0x08, 0x00, 0xc3, 0x48, 0x31, 0xed, 0x49, 0x89, 0xe4, 0x48,
    0x83, 0xe4, 0xf0, 0xe8, 0x00, 0x00, 0x00, 0x00, 0x49, 0x8b, 0x3c, 0x24,
    0x49, 0x8d, 0x74, 0x24, 0x08, 0x49, 0x8d, 0x54, 0xfc, 0x10, 0xe8, 0x00,
    0x00, 0x00, 0x00, 0x89, 0xc7, 0xb8, 0x3c, 0x00, 0x00, 0x00, 0x0f, 0x05,
    0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x22, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x5f, 0x5f, 0x63, 0x31, 0x5f, 0x69, 0x6e,
    0x69, 0x74, 0x00, 0x5f, 0x5f, 0x63, 0x31, 0x5f, 0x73, 0x74, 0x61, 0x72,
    0x74, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x2e, 0x73, 0x79, 0x6d, 0x74, 0x61, 0x62,
    0x00, 0x2e, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x68,
    0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x72, 0x65, 0x6c, 0x61,
    0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x2e, 0x64, 0x61, 0x74, 0x61, 0x00,
    0x2e, 0x62, 0x73, 0x73, 0x00, 0x2e, 0x6e, 0x6f, 0x74, 0x65, 0x2e, 0x47,
    0x4e, 0x55, 0x2d, 0x73, 0x74, 0x61, 0x63, 0x6b, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1b, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6d, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x31, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x11, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x18, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};
static unsigned char builtin_syscalls[] = {
    0x7f, 0x45, 0x4c, 0x46, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3e, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
    0x08, 0x00, 0x07, 0x00, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3,
    0xb8, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3, 0xb8, 0x02, 0x00, 0x00,
    0x00, 0x0f, 0x05, 0xc3, 0xb8, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3,
    0x49, 0x89, 0xca, 0xb8, 0x09, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3, 0xb8,
    0x0b, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3, 0xb8, 0x3c, 0x00, 0x00, 0x00,
    0x0f, 0x05, 0xf4, 0xb8, 0x08, 0x00, 0x00, 0x00, 0x0f, 0x05, 0xc3, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x2b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00,
    0x3b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x5f, 0x5f, 0x63, 0x31, 0x5f, 0x73, 0x79,
    0x73, 0x76, 0x5f, 0x72, 0x65, 0x61, 0x64, 0x00, 0x5f, 0x5f, 0x63, 0x31,
    0x5f, 0x73, 0x79, 0x73, 0x76, 0x5f, 0x77, 0x72, 0x69, 0x74, 0x65, 0x00,
    0x5f, 0x5f, 0x63, 0x31, 0x5f, 0x73, 0x79, 0x73, 0x76, 0x5f, 0x6f, 0x70,
    0x65, 0x6e, 0x00, 0x5f, 0x5f, 0x63, 0x31, 0x5f, 0x73, 0x79, 0x73, 0x76,
    0x5f, 0x63, 0x6c, 0x6f, 0x73, 0x65, 0x00, 0x5f, 0x5f, 0x63, 0x31, 0x5f,
    0x73, 0x79, 0x73, 0x76, 0x5f, 0x6d, 0x6d, 0x61, 0x70, 0x00, 0x5f, 0x5f,
    0x63, 0x31, 0x5f, 0x73, 0x79, 0x73, 0x76, 0x5f, 0x6d, 0x75, 0x6e, 0x6d,
    0x61, 0x70, 0x00, 0x5f, 0x5f, 0x63, 0x31, 0x5f, 0x73, 0x79, 0x73, 0x76,
    0x5f, 0x65, 0x78, 0x69, 0x74, 0x00, 0x5f, 0x5f, 0x63, 0x31, 0x5f, 0x73,
    0x79, 0x73, 0x76, 0x5f, 0x6c, 0x73, 0x65, 0x65, 0x6b, 0x00, 0x00, 0x2e,
    0x73, 0x79, 0x6d, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x73, 0x74, 0x72, 0x74,
    0x61, 0x62, 0x00, 0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62,
    0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x2e, 0x64, 0x61, 0x74, 0x61,
    0x00, 0x2e, 0x62, 0x73, 0x73, 0x00, 0x2e, 0x6e, 0x6f, 0x74, 0x65, 0x2e,
    0x47, 0x4e, 0x55, 0x2d, 0x73, 0x74, 0x61, 0x63, 0x6b, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x21, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
static int output_pie;
static int output_shared;

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
    text_base = (output_pie || output_shared) ? 0x1000 : 0x401000;
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
        } else if (r->type == 10 || r->type == 11) {
            value = (long long)(target + (u64)r->addend);
            if (r->type == 10 && (value < 0 || (u64)value > 0xffffffffULL)) {
                puts("c1link: abs32 overflow");
                return 1;
            }
            if (r->type == 11 && (value < -2147483647LL - 1 || value > 2147483647LL)) {
                puts("c1link: abs32s overflow");
                return 1;
            }
            put32(dest + r->offset, (u32)value);
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

static int strlen_c(const char *text) {
    int n = 0;
    while (text[n])
        n++;
    return n;
}

static int write_shared(const char *path) {
    u64 text_file = 0x1000;
    u64 data_file = align_up(text_file + text_size, 0x1000);
    u64 data_end = data_file + data_size;
    u64 meta_base = align_up(data_base + data_size, 0x1000);
    u64 meta_file = align_up(data_end, 0x1000);
    u64 dynsym_off = 0, dynstr_off, hash_off, dynamic_off, meta_size;
    u64 dynsym_size, dynstr_size = 1, hash_size, dynamic_size = 6 * 16;
    u64 shstr_off, shoff, file_size;
    u64 value;
    unsigned char *image, *p, *dynsym, *dynstr, *hash, *dynamic, *sh;
    int i, nsym = 1, sym_index, gsec, shndx;
    struct symbol *s;
    for (i = 0; i < symbol_count; i++) {
        s = &symbols[i];
        if (!objects[s->object].active || s->bind == 0 || s->shndx == 0 || !s->name[0])
            continue;
        nsym++;
        dynstr_size += (u64)strlen_c(s->name) + 1;
    }
    dynsym_size = (u64)nsym * 24;
    dynstr_off = dynsym_size;
    hash_off = align_up(dynstr_off + dynstr_size, 4);
    hash_size = (u64)(2 + 1 + nsym) * 4;
    dynamic_off = align_up(hash_off + hash_size, 8);
    meta_size = align_up(dynamic_off + dynamic_size, 16);
    shstr_off = meta_file + meta_size;
    shoff = align_up(shstr_off + 59, 8);
    file_size = shoff + 9 * 64;
    image = __c1_linux_HeapAlloc(0, 0, file_size + 1);
    if (!image)
        return 1;
    memset(image, 0, (size_t)file_size);
    p = image;
    p[0] = 0x7f; p[1] = 'E'; p[2] = 'L'; p[3] = 'F';
    p[4] = 2; p[5] = 1; p[6] = 1;
    put32(p + 16, 3);
    put32(p + 18, 62);
    put32(p + 20, 1);
    put64(p + 24, 0);
    put64(p + 32, 64);
    put64(p + 40, shoff);
    put32(p + 48, 0);
    put32(p + 52, 64);
    put32(p + 54, 56);
    put32(p + 56, 6);
    put32(p + 58, 64);
    put32(p + 60, 9);
    put32(p + 62, 8);
    p = image + 64;
    put32(p, 6); put32(p + 4, 5);
    put64(p + 8, 64); put64(p + 16, 0x40);
    put64(p + 24, 0x40); put64(p + 32, 64 + 6 * 56);
    put64(p + 40, 64 + 6 * 56); put64(p + 48, 8);
    p += 56;
    put32(p, 1); put32(p + 4, 5);
    put64(p + 8, 0); put64(p + 16, 0);
    put64(p + 24, 0); put64(p + 32, text_file + text_size);
    put64(p + 40, text_file + text_size); put64(p + 48, 0x1000);
    p += 56;
    put32(p, 1); put32(p + 4, 6);
    put64(p + 8, data_file); put64(p + 16, data_base);
    put64(p + 24, data_base); put64(p + 32, data_size);
    put64(p + 40, data_size + bss_size); put64(p + 48, 0x1000);
    p += 56;
    put32(p, 1); put32(p + 4, 6);
    put64(p + 8, meta_file); put64(p + 16, meta_base);
    put64(p + 24, meta_base); put64(p + 32, meta_size);
    put64(p + 40, meta_size); put64(p + 48, 0x1000);
    p += 56;
    put32(p, 2); put32(p + 4, 6);
    put64(p + 8, meta_file + dynamic_off); put64(p + 16, meta_base + dynamic_off);
    put64(p + 24, meta_base + dynamic_off); put64(p + 32, dynamic_size);
    put64(p + 40, dynamic_size); put64(p + 48, 8);
    p += 56;
    put32(p, 0x6474e551); put32(p + 4, 6);
    put64(p + 8, 0); put64(p + 16, 0);
    put64(p + 24, 0); put64(p + 32, 0);
    put64(p + 40, 0); put64(p + 48, 0x10);
    memcpy(image + text_file, text_image, (size_t)text_size);
    if (data_size)
        memcpy(image + data_file, data_image, (size_t)data_size);
    dynsym = image + meta_file + dynsym_off;
    dynstr = image + meta_file + dynstr_off;
    dynstr[0] = 0;
    dynstr_size = 1;
    sym_index = 1;
    for (i = 0; i < symbol_count; i++) {
        s = &symbols[i];
        if (!objects[s->object].active || s->bind == 0 || s->shndx == 0 || !s->name[0])
            continue;
        gsec = objects[s->object].sec_map[s->shndx];
        shndx = 0;
        value = 0;
        if (sections[gsec].out_kind == 1) {
            shndx = 1;
            value = text_base + sections[gsec].out_off + s->value;
        } else if (sections[gsec].out_kind == 2) {
            shndx = 2;
            value = data_base + sections[gsec].out_off + s->value;
        } else if (sections[gsec].out_kind == 3) {
            shndx = 3;
            value = bss_base + sections[gsec].out_off + s->value;
        } else
            continue;
        p = dynsym + sym_index * 24;
        put32(p, (u32)dynstr_size);
        p[4] = (unsigned char)((s->bind << 4) | s->type);
        p[5] = 0;
        put32(p + 6, (u32)shndx);
        put64(p + 8, value);
        put64(p + 16, s->size);
        {
            int j = 0;
            while (s->name[j])
                dynstr[dynstr_size++] = (unsigned char)s->name[j++];
            dynstr[dynstr_size++] = 0;
        }
        sym_index++;
    }
    hash = image + meta_file + hash_off;
    put32(hash, 1);
    put32(hash + 4, (u32)nsym);
    put32(hash + 8, nsym > 1 ? 1 : 0);
    for (i = 0; i < nsym; i++)
        put32(hash + 12 + i * 4, i + 1 < nsym ? (u32)(i + 1) : 0);
    dynamic = image + meta_file + dynamic_off;
    put64(dynamic, 4); put64(dynamic + 8, meta_base + hash_off);
    put64(dynamic + 16, 5); put64(dynamic + 24, meta_base + dynstr_off);
    put64(dynamic + 32, 6); put64(dynamic + 40, meta_base + dynsym_off);
    put64(dynamic + 48, 10); put64(dynamic + 56, dynstr_size);
    put64(dynamic + 64, 11); put64(dynamic + 72, 24);
    put64(dynamic + 80, 0); put64(dynamic + 88, 0);
    p = image + shstr_off;
    p[0] = 0;
    p += 1;
    {
        static char *names[8] = {".text", ".data", ".bss", ".dynsym", ".dynstr", ".hash",
                                 ".dynamic", ".shstrtab"};
        for (i = 0; i < 8; i++) {
            int j = 0;
            while (names[i][j])
                *p++ = (unsigned char)names[i][j++];
            *p++ = 0;
        }
    }
    sh = image + shoff;
    sh += 64;
    put32(sh, 1); put32(sh + 4, 1); put64(sh + 8, 6); put64(sh + 16, text_base);
    put64(sh + 24, text_file); put64(sh + 32, text_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 16); put64(sh + 56, 0);
    sh += 64;
    put32(sh, 7); put32(sh + 4, 1); put64(sh + 8, 3); put64(sh + 16, data_base);
    put64(sh + 24, data_file); put64(sh + 32, data_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 16); put64(sh + 56, 0);
    sh += 64;
    put32(sh, 13); put32(sh + 4, 8); put64(sh + 8, 3); put64(sh + 16, bss_base);
    put64(sh + 24, data_end); put64(sh + 32, bss_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 16); put64(sh + 56, 0);
    sh += 64;
    put32(sh, 18); put32(sh + 4, 11); put64(sh + 8, 0); put64(sh + 16, meta_base);
    put64(sh + 24, meta_file); put64(sh + 32, dynsym_size); put32(sh + 40, 5); put32(sh + 44, 1);
    put64(sh + 48, 8); put64(sh + 56, 24);
    sh += 64;
    put32(sh, 26); put32(sh + 4, 3); put64(sh + 8, 0); put64(sh + 16, meta_base + dynstr_off);
    put64(sh + 24, meta_file + dynstr_off); put64(sh + 32, dynstr_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 1); put64(sh + 56, 0);
    sh += 64;
    put32(sh, 34); put32(sh + 4, 5); put64(sh + 8, 0); put64(sh + 16, meta_base + hash_off);
    put64(sh + 24, meta_file + hash_off); put64(sh + 32, hash_size); put32(sh + 40, 4); put32(sh + 44, 0);
    put64(sh + 48, 4); put64(sh + 56, 0);
    sh += 64;
    put32(sh, 40); put32(sh + 4, 6); put64(sh + 8, 0); put64(sh + 16, meta_base + dynamic_off);
    put64(sh + 24, meta_file + dynamic_off); put64(sh + 32, dynamic_size); put32(sh + 40, 5); put32(sh + 44, 0);
    put64(sh + 48, 8); put64(sh + 56, 16);
    sh += 64;
    put32(sh, 49); put32(sh + 4, 3); put64(sh + 8, 0); put64(sh + 16, 0);
    put64(sh + 24, shstr_off); put64(sh + 32, 59); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 1); put64(sh + 56, 0);
    i = write_file(path, image, file_size);
    __c1_linux_HeapFree(0, 0, image);
    return i;
}

static int write_output(const char *path, u64 entry) {
    u64 image_base = output_pie ? 0 : 0x400000;
    u64 text_file = 0x1000;
    u64 data_file = align_up(text_file + text_size, 0x1000);
    u64 data_end = data_file + data_size;
    u64 sym_file, str_file, shstr_file, shoff, file_size, sym_size, str_size = 1, shstr_size = 44;
    u64 value;
    unsigned char *image, *p, *symtab, *strtab, *shstrtab, *sh;
    int i, nsym = 1, sym_index, gsec, shndx;
    struct symbol *s;
    for (i = 0; i < symbol_count; i++) {
        s = &symbols[i];
        if (!objects[s->object].active || s->bind == 0 || s->shndx == 0 || !s->name[0])
            continue;
        nsym++;
        str_size += (u64)strlen_c(s->name) + 1;
    }
    sym_file = align_up(data_end, 8);
    sym_size = (u64)nsym * 24;
    str_file = sym_file + sym_size;
    shstr_file = str_file + str_size;
    shoff = align_up(shstr_file + shstr_size, 8);
    file_size = shoff + 7 * 64;
    image = __c1_linux_HeapAlloc(0, 0, file_size + 1);
    if (!image)
        return 1;
    memset(image, 0, (size_t)file_size);
    p = image;
    p[0] = 0x7f; p[1] = 'E'; p[2] = 'L'; p[3] = 'F';
    p[4] = 2; p[5] = 1; p[6] = 1;
    put32(p + 16, output_pie ? 3 : 2);
    put32(p + 18, 62);
    put32(p + 20, 1);
    put64(p + 24, entry);
    put64(p + 32, 64);
    put64(p + 40, shoff);
    put32(p + 48, 0);
    put32(p + 52, 64);
    put32(p + 54, 56);
    put32(p + 56, 4);
    put32(p + 58, 64);
    put32(p + 60, 7);
    put32(p + 62, 6);
    p = image + 64;
    put32(p, 6); put32(p + 4, 5);
    put64(p + 8, 64); put64(p + 16, image_base + 0x40);
    put64(p + 24, image_base + 0x40); put64(p + 32, 288);
    put64(p + 40, 288); put64(p + 48, 8);
    p += 56;
    put32(p, 1); put32(p + 4, 5);
    put64(p + 8, 0); put64(p + 16, image_base);
    put64(p + 24, image_base); put64(p + 32, text_file + text_size);
    put64(p + 40, text_file + text_size); put64(p + 48, 0x1000);
    p += 56;
    put32(p, 1); put32(p + 4, 6);
    put64(p + 8, data_file); put64(p + 16, data_base);
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
    symtab = image + sym_file;
    strtab = image + str_file;
    strtab[0] = 0;
    str_size = 1;
    sym_index = 1;
    for (i = 0; i < symbol_count; i++) {
        s = &symbols[i];
        if (!objects[s->object].active || s->bind == 0 || s->shndx == 0 || !s->name[0])
            continue;
        gsec = objects[s->object].sec_map[s->shndx];
        shndx = 0;
        value = 0;
        if (sections[gsec].out_kind == 1) {
            shndx = 1;
            value = text_base + sections[gsec].out_off + s->value;
        } else if (sections[gsec].out_kind == 2) {
            shndx = 2;
            value = data_base + sections[gsec].out_off + s->value;
        } else if (sections[gsec].out_kind == 3) {
            shndx = 3;
            value = bss_base + sections[gsec].out_off + s->value;
        } else
            continue;
        p = symtab + sym_index * 24;
        put32(p, (u32)str_size);
        p[4] = (unsigned char)((s->bind << 4) | s->type);
        p[5] = 0;
        put32(p + 6, (u32)shndx);
        put64(p + 8, value);
        put64(p + 16, s->size);
        {
            int j = 0;
            while (s->name[j]) {
                strtab[str_size++] = (unsigned char)s->name[j];
                j++;
            }
            strtab[str_size++] = 0;
        }
        sym_index++;
    }
    shstrtab = image + shstr_file;
    shstrtab[0] = 0;
    p = shstrtab + 1;
    for (i = 0; i < 6; i++) {
        static char *names[6] = {".text", ".data", ".bss", ".symtab", ".strtab", ".shstrtab"};
        int j = 0;
        while (names[i][j])
            *p++ = (unsigned char)names[i][j++];
        *p++ = 0;
    }
    sh = image + shoff;
    /* NULL */
    sh += 64;
    /* .text */
    put32(sh, 1); put32(sh + 4, 1); put64(sh + 8, 6); put64(sh + 16, text_base);
    put64(sh + 24, text_file); put64(sh + 32, text_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 16); put64(sh + 56, 0);
    sh += 64;
    /* .data */
    put32(sh, 7); put32(sh + 4, 1); put64(sh + 8, 3); put64(sh + 16, data_base);
    put64(sh + 24, data_file); put64(sh + 32, data_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 16); put64(sh + 56, 0);
    sh += 64;
    /* .bss */
    put32(sh, 13); put32(sh + 4, 8); put64(sh + 8, 3); put64(sh + 16, bss_base);
    put64(sh + 24, data_end); put64(sh + 32, bss_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 16); put64(sh + 56, 0);
    sh += 64;
    /* .symtab */
    put32(sh, 18); put32(sh + 4, 2); put64(sh + 8, 0); put64(sh + 16, 0);
    put64(sh + 24, sym_file); put64(sh + 32, sym_size); put32(sh + 40, 5); put32(sh + 44, 1);
    put64(sh + 48, 8); put64(sh + 56, 24);
    sh += 64;
    /* .strtab */
    put32(sh, 26); put32(sh + 4, 3); put64(sh + 8, 0); put64(sh + 16, 0);
    put64(sh + 24, str_file); put64(sh + 32, str_size); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 1); put64(sh + 56, 0);
    sh += 64;
    /* .shstrtab */
    put32(sh, 34); put32(sh + 4, 3); put64(sh + 8, 0); put64(sh + 16, 0);
    put64(sh + 24, shstr_file); put64(sh + 32, 44); put32(sh + 40, 0); put32(sh + 44, 0);
    put64(sh + 48, 1); put64(sh + 56, 0);
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
    int i, entry_index = -1, base = 1, bootstrap = 0;
    if (argc > 1 && same_name(argv[1], "--bootstrap")) {
        bootstrap = 1;
        base = 2;
    } else if (argc > 1 && same_name(argv[1], "--pie")) {
        output_pie = 1;
        base = 2;
    } else if (argc > 1 && same_name(argv[1], "--shared")) {
        output_shared = 1;
        base = 2;
    }
    if (output_shared) {
        if (argc < base + 2) {
            puts("usage: c1link --shared output object...");
            return 2;
        }
        for (i = base + 1; i < argc; i++)
            if (parse_input(argv[i])) {
                puts("c1link: invalid object");
                return 1;
            }
        layout_sections();
        if (build_images() || apply_relocations())
            return 1;
        return write_shared(argv[base]);
    }
    if (argc < base + 3) {
        puts("usage: c1link [--pie] output entry object...");
        return 2;
    }
    if (bootstrap) {
        if (parse_memory("builtin-startup", builtin_startup, sizeof(builtin_startup)) ||
            parse_memory("builtin-syscalls", builtin_syscalls, sizeof(builtin_syscalls))) {
            puts("c1link: invalid builtin object");
            return 1;
        }
    }
    for (i = base + 2; i < argc; i++)
        if (parse_input(argv[i])) {
            puts("c1link: invalid object");
            return 1;
        }
    activate_archives();
    select_globals();
    for (i = 0; i < symbol_count; i++)
        if (symbols[i].bind != 0 && same_name(symbols[i].name, argv[base + 1]) &&
            symbols[i].shndx != 0 && objects[symbols[i].object].active)
            entry_index = i;
    if (entry_index < 0) {
        puts("c1link: missing entry");
        return 1;
    }
    layout_sections();
    if (build_images() || apply_relocations())
        return 1;
    if (write_output(argv[base], selected_address(entry_index)))
        return 1;
    puts("c1link: linked");
    return 0;
}
