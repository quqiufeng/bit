#include <regex.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    regex_t expression;
    regmatch_t match;
    char message[64];
    if (regcomp(&expression, "^[a-z]+[0-9]+$", REG_EXTENDED) != 0)
        return 1;
    if (regexec(&expression, "abc123", 1, &match, 0) != 0)
        return 2;
    if (match.rm_so != 0 || match.rm_eo != 6)
        return 3;
    if (regexec(&expression, "ABC", 0, 0, 0) != REG_NOMATCH)
        return 4;
    if (regerror(REG_NOMATCH, &expression, message, sizeof(message)) == 0)
        return 5;
    regfree(&expression);
    puts("hosted regex ok");
    return 0;
}
