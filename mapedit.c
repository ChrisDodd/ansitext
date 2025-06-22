#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ansi.h"
#include "mapedit.h"
#include "terrain.h"
#include "vector.h"
#include "vector.h"

struct map_pos_info_t {
    /* info to track a cursor position on a map */
    unsigned    orig_row, orig_col,     // upper left corner of screen
                row, col,               // cursos pos on map
                wrap,                   // map wraps east/west
                width, height,          // dimensions of map
                screen;                 // dimensions of screen/window
    int         redraw;                 // need to redraw (scroll)
};

static inline int even(int v) { return !(v & 1); }
static inline int odd(int v) { return v & 1; }

static void map_up(struct map_pos_info_t *pos) {
    if (pos->row > 0) {
        if (--pos->row < pos->orig_row) {
            pos->orig_row = pos->row;
            pos->redraw = 1; } }
}

static void map_down(struct map_pos_info_t *pos) {
    if (pos->row < pos->height - 1) {
        if (++pos->row - pos->orig_row >= GETY(pos->screen)) {
            pos->orig_row = GETY(pos->screen) - pos->row + 1;
            pos->redraw = 1; } }
}

static int map_left(struct map_pos_info_t *pos) {
    if (pos->col > 0) {
        if (--pos->col < pos->orig_col) {
            pos->orig_col = pos->col;
            pos->redraw = 1; }
        return 1; }
    if (pos->wrap) {
        pos->col = pos->width - 1;
        if (pos->orig_col == 0) {
            pos->orig_col = pos->col;
            pos->redraw = 1; }
        return 1; }
    return 0;
}

static int map_right(struct map_pos_info_t *pos) {
    if (++pos->col == pos->width) {
        if (!pos->wrap) {
            --pos->col;
            return 0; }
        pos->col = 0; }
    if ((pos->col + pos->width - pos->orig_col) % pos->width >= GETX(pos->screen)/2) {
        ++pos->orig_col;
        pos->redraw = 1; }
    return 1;
}

static void spot(VECTOR(VECTOR(char)) map, struct map_pos_info_t *pos, char ch, int size) {
    for (int x = -size; x <= size; ++x) {
        for (int y = -size; y <= size; ++y) {
            if ((int)pos->row + y < 0 || pos->row + y >= pos->height) continue;
            if ((int)pos->col + x < 0 || pos->col + x >= pos->width) continue;
            int r = y*y;
            if (odd(y)) {
                if (odd(pos->row)) r += (x+1)*x;
                else r += (x-1)*x; }
            else r += x*x;
            if (r >= size*(size+1)) continue;
            map[pos->row + y][pos->col + x] = ch;
        }
    }
    pos->redraw = 1;
}

VECTOR(VECTOR(char)) edit_map(VECTOR(VECTOR(char)) map, int wrap) {
    struct map_pos_info_t pos = { 0, 0, 0, 0, wrap, VECTOR_size(map[0]), VECTOR_size(map),
                                  getsize(), 1 };
    fputs(MOUSE_ENABLE, stdout);
    fputs(MOUSE_SGR, stdout);
    while(1) {
        if (pos.redraw) {
            pos.screen = getsize();
            draw_map(map, pos.orig_row, pos.orig_col, pos.wrap);
            pos.redraw = 0; }
        int off = (pos.col - pos.orig_col) * 2 + !(pos.row&1);
        if (pos.col < pos.orig_col)
            off = (off + pos.width*2) % (pos.width*2);
        fputs(gotoyx(pos.row-pos.orig_row + 1, off + 1), stdout);
        fflush(stdout);
        int ch;
        switch((ch = getch())) {
        case KEY_END: case '1':
            if (pos.row < pos.height - 1)
                if (even(pos.row) || map_left(&pos)) map_down(&pos);
            break;
        case KEY_PGDN: case '3':
            if (pos.row < pos.height - 1)
                if (odd(pos.row) || map_right(&pos)) map_down(&pos);
            break;
        case KEY_LEFT: case '4':
            map_left(&pos);
            break;
        case KEY_RIGHT: case '6':
            map_right(&pos);
            break;
        case KEY_HOME: case '7':
            if (pos.row > 0)
                if (even(pos.row) || map_left(&pos)) map_up(&pos);
            break;
        case KEY_PGUP: case '9':
            if (pos.row > 0)
                if (odd(pos.row) || map_right(&pos)) map_up(&pos);
            break;
        case MOD_SHIFT + KEY_LEFT:
            if (pos.width >= 10) {
                --pos.width;
                for (unsigned i = 0; i < VECTOR_size(map); ++i)
                    VECTOR_resize(map[i], pos.width);
                pos.redraw = 1; }
            break;
        case MOD_SHIFT + KEY_RIGHT:
            ++pos.width;
            for (unsigned i = 0; i < VECTOR_size(map); ++i) {
                VECTOR_resize(map[i], pos.width);
                map[i][pos.width-1] = ' '; }
            pos.redraw = 1;
            break;
        case MOD_SHIFT + KEY_UP:
            if (pos.height >= 10) {
                --pos.height;
                VECTOR_fini(map[pos.height]);
                VECTOR_resize(map, pos.height);
                pos.redraw = 1; }
            break;
        case MOD_SHIFT + KEY_DOWN:
            ++pos.height;
            VECTOR_resize(map, pos.height);
            VECTOR_init(map[pos.height-1], pos.width);
            VECTOR_resize(map[pos.height-1], pos.width);
            memset(map[pos.height-1], ' ', pos.width);
            pos.redraw = 1;
            break;
        case 'x':
            return map;
        case KEY_F1: case KEY_F2: case KEY_F3: case KEY_F4:
            spot(map, &pos, '+', ch - KEY_F0);
            break;
        case KEY_F5: case KEY_F6: case KEY_F7: case KEY_F8:
            spot(map, &pos, '.', ch - KEY_F4);
            break;
        default:
            if (ch & MOUSE) {
                pos.row = MOUSE_Y(ch) - 1 + pos.orig_row;
                pos.col = (MOUSE_X(ch) - 1)/2 + pos.orig_col;
                if (pos.row >= pos.height) pos.row = pos.height - 1;
                if (pos.col >= pos.width) pos.col = pos.width - 1;
            } else if (ch >= ' ' && ch < 127 && (terrain_map[ch] || ch == ' ')) {
                map[pos.row][pos.col] = ch;
                pos.redraw = 1;
            }
            break;
        }
    }
}
