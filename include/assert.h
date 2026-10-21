#ifndef __C1_ASSERT_H
#define __C1_ASSERT_H
void abort(void);
#ifdef NDEBUG
#define assert(expression) ((void)0)
#else
#define assert(expression) ((expression) ? (void)0 : abort())
#endif
#endif
