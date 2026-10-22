#include <glob.h>
#include <stdio.h>

int main(void) {
    glob_t paths;
    if (sizeof(glob_t) != 72)
        return 1;
    if (glob("/tmp/*", 0, 0, &paths) != 0)
        return 2;
    if (paths.gl_pathc == 0 || !paths.gl_pathv)
        return 3;
    globfree(&paths);
    puts("hosted glob ok");
    return 0;
}
