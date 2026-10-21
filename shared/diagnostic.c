/* Shared diagnostic buffer and formatter.
   The platform supplies __c1_diag_write; no OS headers live here. */
int __c1_diag_write(char *buffer, int length);

char diagnostic_buffer[4096];
int diagnostic_used;
int diagnostic_sent;
int diagnostic_broken;

int diagnostic_flush(void) {
    if (!diagnostic_used || diagnostic_broken)
        return 0;
    if (__c1_diag_write(diagnostic_buffer, diagnostic_used) != diagnostic_used)
        diagnostic_broken = 1;
    else
        diagnostic_used = 0;
    return 0;
}

int diagnostic_put(int c) {
    if (diagnostic_broken)
        return 0;
    diagnostic_buffer[diagnostic_used++] = (char)c;
    if (diagnostic_used == 4096)
        diagnostic_flush();
    return 0;
}

int diagnostic_text(char *text) {
    int i = 0;
    while (text[i])
        diagnostic_put(text[i++]);
    return 0;
}

int diagnostic_number(unsigned int number) {
    char digits[10];
    int n = 0;
    do {
        digits[n++] = (char)('0' + number % 10);
        number /= 10;
    } while (number);
    while (n)
        diagnostic_put(digits[--n]);
    return 0;
}

int diagnostic_end(void) {
    diagnostic_put(10);
    diagnostic_flush();
    diagnostic_sent = 1;
    return 0;
}

