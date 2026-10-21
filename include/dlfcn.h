#ifndef __C1_DLFCN_H
#define __C1_DLFCN_H
#define RTLD_LAZY 1
#define RTLD_NOW 2
void *dlopen(const char *name, int flags);
void *dlsym(void *handle, const char *name);
int dlclose(void *handle);
#endif
