void *dlopen(char *name, int flags);
void *dlsym(void *handle, char *name);
int dlclose(void *handle);
int main(void) {
    void *handle = dlopen("libm.so.6", 1);
    double (*cosine)(double);
    if (!handle)
        return 1;
    cosine = (double (*)(double))dlsym(handle, "cos");
    if (!cosine)
        return 2;
    if (cosine(0.0) != 1.0)
        return 3;
    dlclose(handle);
    return 0;
}
