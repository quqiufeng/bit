#include <getopt.h>
#include <stdio.h>

int main(void) {
    char *arguments[5];
    int option, seen_a = 0, seen_b = 0;
    arguments[0] = "program";
    arguments[1] = "-a";
    arguments[2] = "-b";
    arguments[3] = "value";
    arguments[4] = 0;
    optind = 1;
    while ((option = getopt(4, arguments, "ab:")) != -1) {
        if (option == 'a')
            seen_a = 1;
        if (option == 'b' && optarg && optarg[0] == 'v')
            seen_b = 1;
    }
    if (!seen_a || !seen_b || optind != 4)
        return 1;
    puts("hosted getopt ok");
    return 0;
}
