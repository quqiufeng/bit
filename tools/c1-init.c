extern void __c1_init(void);
__attribute__((constructor)) static void c1_init_ctor(void) { __c1_init(); }
