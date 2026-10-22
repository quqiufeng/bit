#include <stdio.h>
#include <wchar.h>

int main(void) {
    wchar_t text[16];
    wcscpy(text, L"hello");
    wcscat(text, L"!");
    if (wcslen(text) != 6)
        return 1;
    if (wcscmp(text, L"hello!") != 0)
        return 2;
    if (!wcschr(text, L'!'))
        return 3;
    puts("hosted wchar ok");
    return 0;
}
