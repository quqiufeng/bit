#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    struct passwd *entry = getpwuid(getuid());
    if (!entry)
        return 1;
    if (!entry->pw_name || !entry->pw_name[0])
        return 2;
    if (entry->pw_uid != getuid())
        return 3;
    puts("hosted pwd ok");
    return 0;
}
