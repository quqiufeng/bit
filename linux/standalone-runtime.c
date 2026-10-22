extern long long __c1_sysv_read(long long, void *, unsigned long long);
extern long long __c1_sysv_write(long long, const void *, unsigned long long);
extern long long __c1_sysv_open(const char *, long long, long long);
extern long long __c1_sysv_close(long long);
extern long long __c1_sysv_mmap(long long, unsigned long long, long long, long long,
                                long long, long long);

void *memset(void *destination, int value, unsigned long long count) {
    unsigned char *d = (unsigned char *)destination;
    unsigned long long i;
    for (i = 0; i < count; i++)
        d[i] = (unsigned char)value;
    return destination;
}

int puts(char *text) {
    int i = 0;
    long long n;
    while (text[i]) {
        n = __c1_sysv_write(1, text + i, 1);
        if (n != 1)
            return -1;
        i++;
    }
    n = __c1_sysv_write(1, "\n", 1);
    if (n != 1)
        return -1;
    return 0;
}

void *__c1_linux_HeapAlloc(long long heap, unsigned int flags,
                           unsigned long long bytes) {
    long long raw;
    unsigned long long *header;
    raw = __c1_sysv_mmap(0, bytes + 16, 3, 34, -1, 0);
    if (raw < 0 && raw > -4096)
        return 0;
    header = (unsigned long long *)raw;
    header[0] = bytes + 16;
    return (void *)(header + 2);
}

long long __c1_linux_CreateFileA(char *path, unsigned int access, int share,
                                 void *security, int disposition, int flags,
                                 long long template_file) {
    long long fd = __c1_sysv_open(path, 0, 0);
    if (fd < 0)
        return -1;
    return fd;
}

int __c1_linux_ReadFile(long long handle, char *buffer, int length, int *done,
                        void *overlap) {
    long long n;
    n = __c1_sysv_read(handle, buffer, (unsigned long long)length);
    if (n < 0) {
        if (done)
            *done = 0;
        return 0;
    }
    if (done)
        *done = (int)n;
    return 1;
}

int __c1_linux_CloseHandle(long long handle) {
    if (handle < 0)
        return 1;
    return __c1_sysv_close(handle) == 0;
}
