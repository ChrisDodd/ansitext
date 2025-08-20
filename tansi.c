#include <stdio.h>
#include "ansi.h"

int main() {
    printf("%splain%s\n", PLAIN, PLAIN);
    printf("%sbold%s\n", BOLD, PLAIN);
    printf("%sfaint%s\n", FAINT, PLAIN);
    printf("%sitalic%s\n", ITALIC, PLAIN);
    printf("%sunderline%s\n", UNDERLINE, PLAIN);
    printf("%sblink%s\n", BLINK, PLAIN);
    printf("%sfastblink%s\n", FASTBLINK, PLAIN);
    printf("%sreverse%s\n", REVERSE, PLAIN);
    printf("%sconceal%s\n", CONCEAL, PLAIN);
    printf("%sstrikeout%s\n", STRIKEOUT, PLAIN);

    printf(MOUSE_BTN_ENABLE MOUSE_SGR); fflush(stdout);

    while(1) {
        int ch = getch();
        printf("%d", ch);
        if (ch >= ' ' && ch < 0x7f)
            printf(" '%c'", ch);
        printf(" 0x%x", ch);
        printf("\n");
        if (ch == -1 || ch == KEY_END) break;
        if (ch == KEY_F1)
            printf(MOUSE_ANY_ENABLE "[mouse any enable]\n"), fflush(stdout);
        if (ch == KEY_F2)
            printf(MOUSE_ANY_DISABLE "[mouse any disable]\n"), fflush(stdout);
    }
    reset();
}
