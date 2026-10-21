/* Freestanding Linux runtime for the C1 host compiler.
   The same source is compiled by gcc for stage0 and by C1 for the
   stage1/stage2 executables.  Syscalls live in tiny assembly stubs. */

extern long long __c1_sysv_read(long long, void *, unsigned long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern long long __c1_sysv_open(const char *, long long, long long);
extern long long __c1_sysv_close(long long);
extern long long __c1_sysv_mmap(long long, unsigned long long, long long, long long, long long,
                                long long);
extern long long __c1_sysv_munmap(long long, unsigned long long);
extern void __c1_sysv_exit(long long);
#define C1_READ __c1_sysv_read
#define C1_WRITE __c1_sysv_write
#define C1_OPEN __c1_sysv_open
#define C1_CLOSE __c1_sysv_close
#define C1_MMAP __c1_sysv_mmap
#define C1_MUNMAP __c1_sysv_munmap
#define C1_EXIT __c1_sysv_exit

int main(int argc, char **argv);

void *memcpy(void *destination, const void *source, unsigned long long count) {
    unsigned char *d = (unsigned char *)destination;
    const unsigned char *s = (const unsigned char *)source;
    unsigned long long i;
    for (i = 0; i < count; i++)
        d[i] = s[i];
    return destination;
}

void *memmove(void *destination, const void *source, unsigned long long count) {
    unsigned char *d = (unsigned char *)destination;
    const unsigned char *s = (const unsigned char *)source;
    unsigned long long i;
    if (d < s)
        for (i = 0; i < count; i++)
            d[i] = s[i];
    else
        for (i = count; i > 0; i--)
            d[i - 1] = s[i - 1];
    return destination;
}

void *memset(void *destination, int value, unsigned long long count) {
    unsigned char *d = (unsigned char *)destination;
    unsigned long long i;
    for (i = 0; i < count; i++)
        d[i] = (unsigned char)value;
    return destination;
}

long long __c1_linux_GetStdHandle(int which) {
    return which;
}

long long __c1_linux_GetProcessHeap(void) {
    return 0;
}

void *__c1_linux_HeapAlloc(long long heap, unsigned int flags, unsigned long long bytes) {
    long long raw;
    unsigned long long *header;
    raw = C1_MMAP(0, bytes + 16, 3, 34, -1, 0);
    if (raw < 0 && raw > -4096)
        return 0;
    header = (unsigned long long *)raw;
    header[0] = bytes + 16;
    return (void *)(header + 2);
}

int __c1_linux_HeapFree(long long heap, unsigned int flags, void *memory) {
    unsigned long long *header;
    if (!memory)
        return 1;
    header = (unsigned long long *)memory - 2;
    return C1_MUNMAP((long long)header, header[0]) == 0;
}

int __c1_linux_ReadFile(long long handle, char *buffer, int length, int *done, void *overlap) {
    long long n, fd;
    fd = handle < 0 ? -handle - 10 : handle;
    n = C1_READ(fd, buffer, (unsigned long long)length);
    if (n < 0) {
        if (done)
            *done = 0;
        return 0;
    }
    if (done)
        *done = (int)n;
    return 1;
}

int __c1_linux_WriteFile(long long handle, char *buffer, int length, int *done, void *overlap) {
    long long n, fd;
    fd = handle < 0 ? -handle - 10 : handle;
    n = C1_WRITE(fd, buffer, (unsigned long long)length);
    if (n < 0) {
        if (done)
            *done = 0;
        return 0;
    }
    if (done)
        *done = (int)n;
    return 1;
}

long long __c1_linux_CreateFileA(char *path, unsigned int access, int share, void *security,
                                 int disposition, int flags, long long template_file) {
    long long fd = C1_OPEN(path, 0, 0);
    if (fd < 0)
        return -1;
    return fd;
}

int __c1_linux_CloseHandle(long long handle) {
    if (handle < 0)
        return 1;
    return C1_CLOSE(handle) == 0;
}

unsigned int __c1_linux_GetEnvironmentVariableA(char *name, char *buffer, unsigned int size) {
    return 0;
}

void __c1_linux_GetLocalTime(unsigned short *value) {
    int i;
    for (i = 0; i < 8; i++)
        value[i] = 0;
}

int __c1_linux_IsDBCSLeadByteEx(unsigned int code_page, unsigned char byte) {
    return 0;
}

int __c0_getchar(void) {
    unsigned char byte;
    long long n = C1_READ(0, &byte, 1);
    if (n != 1)
        return -1;
    return byte;
}

int __c0_putchar(int c) {
    unsigned char byte = (unsigned char)c;
    long long n = C1_WRITE(1, &byte, 1);
    if (n != 1)
        return -1;
    return byte;
}

int putchar(int c) {
    return __c0_putchar(c);
}

int puts(char *text) {
    int i = 0;
    while (text[i])
        if (__c0_putchar(text[i++]) < 0)
            return -1;
    return __c0_putchar('\n');
}

int __c1_start(long long argc, char **argv, char **envp) {
    return main((int)argc, argv);
}
