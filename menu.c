#include <stdio.h>
#include <string.h>
#include "menu.h"
#include "ansi.h"
#include "layout_string.h"

#define HORIZ "\u2500"
#define VERT "\u2502"
#define TL "\u250c"
#define BL "\u2514"
#define TR "\u2510"
#define BR "\u2518"
#define VL "\u251c"
#define VR "\u2524"

static void hline(unsigned w) { while(w-- > 0) fputs(HORIZ, stdout); }

void draw_menu(unsigned row, unsigned col, const struct menu *menu) {
    printf("%s" TL, gotoyx(row++, col));
    hline(menu->width);
    fputs(TR, stdout);
    char *l;
    for (unsigned i = 0; (l = menu->entries[i].label); ++i) {
        if (l[0] == '-' && l[1] == 0) {
            printf("%s" VL, gotoyx(row++, col));
            hline(menu->width);
            fputs(VR, stdout);
        } else {
            unsigned w = layout_width(l, l + strlen(l));
            printf("%s" VERT "%s%*s" VERT, gotoyx(row++, col), l, menu->width - w, "");
        }
    }
    printf("%s" BL, gotoyx(row, col));
    hline(menu->width);
    fputs(BR, stdout);
    fflush(stdout);
}

void erase_rect(unsigned row, unsigned col, unsigned width, unsigned height) {
    for (;height > 0; --height, ++row)
        printf("%s%*s", gotoyx(row, col), width, "");
    fflush(stdout);
}

void *popup_menu(unsigned row, unsigned col, struct menu *menu) {
    struct menu_args args = { row, col, menu, 0 };
    unsigned screen = getsize();
    unsigned height = 0;
    int loc;
    char *l;
    for (; (l = menu->entries[height].label); ++height) {
        unsigned w = layout_width(l, l + strlen(l));
        if (w > menu->width) menu->width = w;
    }
    if (row + height + 2 > GETY(screen))
        args.row = GETY(screen) - height - 2;
    if (col + menu->width + 2 > GETX(screen))
        args.col = GETX(screen) - menu->width - 2;
    draw_menu(args.row, args.col, menu);
    while (1) {
        loc = getch();
        if (!(loc & MOUSE)) {
            if (loc == KEY_END) return nullptr;
            continue;
        }
        int r = MOUSE_Y(loc) - args.row - 1;
        if (MOUSE_UP(loc)) {
            if (MOUSE_Y(loc) == row && MOUSE_X(loc) == col) continue;
            void *rv = nullptr;
            if (r >= 0 && r < (int)height && menu->entries[r].action) {
                args.index = r;
                rv = menu->entries[r].action(menu->entries[r].action_arg, &args);
            }
            erase_rect(args.row, args.col, menu->width+2, height+2);
            return rv;
        } else if (r < 0) {
            fputs(gotoyx(args.row, args.col + 1), stdout);
        } else if (r < (int)height) {
            fputs(gotoyx(args.row + r + 1, args.col + 1), stdout);
        } else {
            fputs(gotoyx(args.row + height + 1, args.col + 1), stdout);
        }
        fflush(stdout);
    }
}

void *cascade_menu(void *menu, struct menu_args *args) {
    return popup_menu(args->row + args->index + 1, args->col + args->menu->width + 1, menu);
}
