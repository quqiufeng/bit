#include <dirent.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    DIR *directory = opendir("/tmp");
    struct dirent *entry;
    int found = 0;
    if (!directory)
        return 1;
    while ((entry = readdir(directory)) != 0) {
        if (strcmp(entry->d_name, ".") == 0)
            found = 1;
    }
    closedir(directory);
    if (!found)
        return 2;
    puts("hosted dirent ok");
    return 0;
}
