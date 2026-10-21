#include <stdio.h>
#include <time.h>

int main(void) {
    time_t now = time(0);
    struct tm *value;
    char text[64];
    if (now <= 0)
        return 1;
    value = localtime(&now);
    if (!value)
        return 2;
    if (value->tm_year < 100)
        return 3;
    if (!strftime(text, sizeof(text), "%Y-%m-%d", value))
        return 4;
    puts(text);
    return 0;
}
