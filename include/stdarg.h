#ifndef __C1_STDARG_H
#define __C1_STDARG_H
/* SysV AMD64 va_list storage: gp_offset, fp_offset, overflow_arg_area,
   reg_save_area.  The compiler builtins understand this 24-byte layout. */
typedef char va_list[24];
#define va_start(ap, last) ((void)__va_start((ap), last))
#define va_arg(ap, type) (__va_arg((ap), type))
#define va_end(ap) ((void)0)
#endif
