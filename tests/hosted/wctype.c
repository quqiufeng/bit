#include <stdio.h>
#include <wctype.h>

int main(void) {
    if (!iswalpha(L'A') || !iswdigit(L'7') || !iswspace(L' '))
        return 1;
    if (towupper(L'a') != L'A' || towlower(L'Z') != L'z')
        return 2;
    if (!iswctype(L'A', wctype("alpha")))
        return 3;
    puts("hosted wctype ok");
    return 0;
}
