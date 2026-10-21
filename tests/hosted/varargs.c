#include <stdarg.h>
#include <stdio.h>

int log_values(const char *format, ...) {
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
