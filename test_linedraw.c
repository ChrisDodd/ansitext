#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ansi.h"
#include "linedraw.h"

int main() {
    printf("%s%s", CLEARALL, gotoyx(0, 0));
    fputs(MOUSE_BTN_ENABLE MOUSE_SGR, stdout);
    fflush(stdout);
    linetype_t style = SINGLE;
    fill_missing_double2mixed();
    unsigned screen_size = getsize();
    size_t image_size = sizeof(image_t) + (GETX(screen_size)+1) * (GETY(screen_size)+1);
    image_t *image, *edit1, *edit2; 
    image = malloc(image_size);
    memset(image, 0, image_size);
    image->height = GETY(screen_size) + 1;
    image->width = GETX(screen_size) + 1;
    edit1 = malloc(image_size);
    edit2 = malloc(image_size);
    printf("%swidth=%d height=%d    ", gotoyx(image->height - 4, 3), image->width, image->height);
    while (1) {
        int ch = getch();
        if (ch & MOUSE) {
            int sx = MOUSE_X(ch), sy = MOUSE_Y(ch), tx=sx, ty=sy;
            printf("%sx=%d y=%d    ", gotoyx(image->height - 3, 3), sx, sy);
            fflush(stdout);
            memcpy(edit1, image, image_size);
            while ((ch = getch()) & MOUSE) {
                tx = MOUSE_X(ch);
                ty = MOUSE_Y(ch);
                printf("%sx=%d y=%d    ", gotoyx(image->height - 3, 3), tx, ty);
                fflush(stdout);
                memcpy(edit2, image, image_size);
                if (abs(tx-sx) < abs(ty-sy)) {
                    tx = sx;
                    drawvline(edit2, style, sy, ty, sx);
                } else {
                    ty = sy;
                    drawhline(edit2, style, sy, sx, tx);
                }
                update_image(edit1, edit2, TOPLEFT, screen_size, TOPLEFT);
                fputs(gotoyx(ty, tx), stdout);
                fflush(stdout);
                image_t *tmp = edit1;
                edit1 = edit2;
                edit2 = tmp;
                if (MOUSE_UP(ch)) break;
            }
            if (tx != sx || ty != sy) {
                if (abs(tx-sx) < abs(ty-sy)) {
                    drawvline(image, style, sy, ty, sx);
                } else {
                    drawhline(image, style, sy, sx, tx);
                }
                update_image(edit2, image, TOPLEFT, screen_size, TOPLEFT);
            }
        }
        switch (ch) {
        case KEY_END:
            fputs(gotoyx(999, 0), stdout);
            fflush(stdout);
            reset();
            exit(0);
        case KEY_F1: style = SINGLE; break;
        case KEY_F2: style = DOUBLE; break;
        case KEY_F3: style = HEAVY; break;
        case KEY_F4: style = NONE; break;
        }
    }
}
