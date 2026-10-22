#include <grp.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    struct group *entry = getgrgid(getgid());
    if (!entry)
        return 1;
    if (!entry->gr_name || !entry->gr_name[0])
        return 2;
    if (entry->gr_gid != getgid())
        return 3;
    puts("hosted grp ok");
    return 0;
}
