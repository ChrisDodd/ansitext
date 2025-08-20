#include <stdio.h>
#include "ansi.h"

int main() {
    printf(MOUSE_DISABLE);
    printf(MOUSE_BTN_DISABLE);
    printf(MOUSE_ANY_DISABLE); fflush(stdout);
}
