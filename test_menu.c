#include <stdio.h>
#include "ansi.h"
#include "menu.h"

void *doit(void *arg, struct menu_args *) {
    printf("%sMENU: %s%s", gotoyx(2, 0), (char *)arg, CLEAREOL);
    fflush(stdout);
    return "(end)";
}

struct menu M2 = {
    10, {
    { "act c", doit, "C" },
    { "act d", doit, "DD" },
    { 0, 0, 0 }
  }
};

struct menu M1 = {
    20, {
    { "act a", doit, "A" },
    { "act b", doit, "BBB" },
    { "-", 0, 0 },
    { "c", cascade_menu, &M2 },
    { 0, 0, 0 }
  }
};

int main() {
    printf("%s%s", CLEARALL, gotoyx(0, 0));
    fputs(MOUSE_BTN_ENABLE MOUSE_SGR, stdout);
    fflush(stdout);
    while (1) {
        int ch = getch();
        if (ch & MOUSE) {
            if (!popup_menu(MOUSE_Y(ch), MOUSE_X(ch), &M1)) {
                printf("%s%s", gotoyx(2, 0), CLEAREOL);
                fflush(stdout);
            }
        } else if (ch == 'q' || ch == KEY_END) break;
    }
    fputs(gotoyx(999, 0), stdout);
    fflush(stdout);
    reset();
    return 0;
}
