#include <stddef.h>
int main(void) {
    wchar_t *text = L"AB";
    if (sizeof(wchar_t) != 4)
        return 1;
    if (text[0] != 65 || text[1] != 66 || text[2] != 0)
        return 2;
    if (L'A' != 65)
        return 3;
    return 0;
}
