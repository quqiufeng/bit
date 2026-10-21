#include <stdbool.h>

int main(void) {
    bool state = true;
    if (!state)
        return 1;
    state = false;
    return state ? 2 : 0;
}
