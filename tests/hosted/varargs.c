#include <stdarg.h>
int vprintf(char *format, va_list ap);

int log_values(char *format, ...) {
    va_list ap;
    int result;
    va_start(ap, format);
    result = vprintf(format, ap);
    va_end(ap);
    return result;
}

int main(void) {
    log_values("hosted vprintf=%d float=%f\n", 42, 1.5);
    return 0;
}
