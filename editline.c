#include <stdio.h>
#include <stdbool.h>
#include "ansi.h"
#include "vector.h"
#include "layout_string.h"

int edit_string(VECTOR(char) *string) {
    int spos = getyx();
    // int size = getsize();
    fputs(gotoyx(GETY(spos), GETX(spos)), stdout);
    VECTOR_terminate(*string, 0);
    fputs(*string, stdout);
    int width = layout_width(VECTOR_begin(*string), VECTOR_end(*string));
    int cpos = width;
    int pos = VECTOR_size(*string);
    bool insert = true;
    fflush(stdout);
    while (1) {
        int ch = getch();
        switch (ch) {
        case KEY_END: case '\n':
            return ch;
        case KEY_LEFT:
            if (cpos > 0) {
                pos = layout_index(*string, --cpos) - *string;
                fputs("\x1b[D", stdout);
                fflush(stdout); }
            break;
        case KEY_RIGHT:
            if (cpos < width) {
                pos += layout_index(*string + pos, 1) - (*string + pos);
                ++cpos;
                fputs("\x1b[C", stdout);
                fflush(stdout); }
            break;
        case KEY_INSERT:
            insert = !insert;
            break;
        case '\b':
        case 0x7f:
        case KEY_DELETE:
            if (cpos > 0) {
                int opos = pos;
                pos = layout_index(*string, --cpos) - *string;
                VECTOR_erase(*string, pos, opos - pos);
                VECTOR_terminate(*string, 0);
                fputs("\x1b[C", stdout);
                fputs(*string + pos, stdout);
                fputc(' ', stdout);
                printf("\x1b[%dD", width - cpos + 1); }
            break;
        default:
            if (ch >= ' ' && ch < 0x7f) {
                if (insert) {
                    VECTOR_insert(*string, pos);
                } else {
                    int rwidth = layout_index(*string + pos, 1) - (*string + pos);
                    if (rwidth > 1) VECTOR_erase(*string, pos, rwidth-1); }
                (*string)[pos] = ch;
                VECTOR_terminate(*string, 0);
                fputs(*string + pos++, stdout);
                if (width > ++cpos) printf("\x1b[%dD", width - cpos);
                fflush(stdout); }
        break; } }
}
