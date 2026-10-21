#if !defined(__linux__) || !defined(__unix__) || !defined(__ELF__)
#error missing Linux target macros
#endif
#if !defined(__x86_64__) || !defined(__LP64__) || !defined(_LP64)
#error missing x86-64/LP64 target macros
#endif
#if __linux__ != 1 || __x86_64__ != 1 || __LP64__ != 1 || __C1__ != 1
#error target macro value
#endif
int main(void) {
    return 0;
}
