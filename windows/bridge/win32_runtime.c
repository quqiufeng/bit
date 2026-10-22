long long __c1_linux_GetStdHandle(int which) {
    return which;
}

long long __c1_linux_GetProcessHeap(void) {
    return 0;
}

void *__c1_linux_HeapAlloc(long long heap, unsigned int flags, unsigned long long bytes) {
    typedef void *Allocate(void *, unsigned int, unsigned long long);
    Allocate *allocate = *(Allocate **)0x401088;
    return allocate((void *)heap, flags, bytes);
}

int __c1_linux_HeapFree(long long heap, unsigned int flags, void *memory) {
    typedef int Release(void *, unsigned int, void *);
    Release *release = *(Release **)0x401098;
    return release((void *)heap, flags, memory);
}

int __c1_linux_ReadFile(long long handle, char *buffer, int length, int *done, void *overlap) {
    typedef int Read(void *, void *, unsigned int, unsigned int *, void *);
    Read *read = *(Read **)0x401048;
    unsigned int got = 0;
    int ok = read((void *)handle, buffer, (unsigned int)length, &got, overlap);
    if (done)
        *done = (int)got;
    return ok;
}

int __c1_linux_WriteFile(long long handle, char *buffer, int length, int *done, void *overlap) {
    typedef int Write(void *, const void *, unsigned int, unsigned int *, void *);
    Write *write = *(Write **)0x401050;
    unsigned int put = 0;
    int ok = write((void *)handle, buffer, (unsigned int)length, &put, overlap);
    if (done)
        *done = (int)put;
    return ok;
}

long long __c1_linux_CreateFileA(char *path, unsigned int access, int share, void *security,
                                 int disposition, int flags, long long template_file) {
    typedef void *Open(const char *, unsigned int, unsigned int, void *, unsigned int, unsigned int,
                       void *);
    Open *open = *(Open **)0x401060;
    return (long long)open(path, access, (unsigned int)share, security, (unsigned int)disposition,
                           (unsigned int)flags, (void *)template_file);
}

int __c1_linux_CloseHandle(long long handle) {
    typedef int Close(void *);
    Close *close = *(Close **)0x401068;
    return close((void *)handle);
}

unsigned int __c1_linux_GetEnvironmentVariableA(char *name, char *buffer, unsigned int size) {
    typedef unsigned int Get(char *, char *, unsigned int);
    Get *get = *(Get **)0x401070;
    return get(name, buffer, size);
}

void __c1_linux_GetLocalTime(unsigned short *value) {
    typedef void GetTime(void *);
    GetTime *get = *(GetTime **)0x401078;
    get(value);
}

int __c1_linux_IsDBCSLeadByteEx(unsigned int code_page, unsigned char byte) {
    return 0;
}

int __c1_diag_write(char *buffer, int length) {
    int offset = 0;
    while (offset < length) {
        int done = 0;
        if (!__c1_linux_WriteFile(-12, buffer + offset, length - offset, &done, 0) || done <= 0)
            return -1;
        offset = offset + done;
    }
    return length;
}

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
