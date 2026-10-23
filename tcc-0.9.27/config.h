/* Minimal checked-in configuration for the bit-tcc fork. */
#ifndef CONFIG_TCCDIR
# define CONFIG_TCCDIR "/usr/local/lib/tcc"
#endif
#define TCC_VERSION "0.9.27"
#define CONFIG_TRIPLET "x86_64-linux-gnu"
#define CONFIG_TCC_CRTPREFIX "/usr/lib/x86_64-linux-gnu"
#define CONFIG_TCC_SYSINCLUDEPATHS "{B}/include:/usr/lib/gcc/x86_64-linux-gnu/13/include:/usr/local/include:/usr/include/x86_64-linux-gnu:/usr/include"
