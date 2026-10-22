#ifndef __C1_ICONV_H
#define __C1_ICONV_H
#include <stddef.h>
typedef void *iconv_t;
iconv_t iconv_open(const char *target, const char *source);
size_t iconv(iconv_t conversion, char **input, size_t *input_left,
             char **output, size_t *output_left);
int iconv_close(iconv_t conversion);
#endif
