#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ansi.h"
#include "boxes.h"
#include "utf8.h"

static inline int min(int a, int b) { return a < b ? a : b; }
static inline int max(int a, int b) { return a > b ? a : b; }

#if 0
const style_t styles[] = {
    { "square", "\u250c", "\u2510", "\u2514", "\u2518", "\u2500", "\u2502",
      "\u2534", "\u2524", "\u251c", "\u252c" },
    { "rounded", "\u256d", "\u256e", "\u2570", "\u256f", "\u2500", "\u2502",
      "\u2534", "\u2524", "\u251c", "\u252c" },
    { "double", "\u2554", "\u2557", "\u255a", "\u255d", "\u2550", "\u2551",
      "\u2567", "\u2562", "\u255f", "\u2564" },
    { "heavy", "\u250f", "\u2513", "\u2517", "\u251b", "\u2501", "\u2503",
      "\u2537", "\u2528", "\u2520", "\u252f" },
};
const unsigned styles_count = sizeof(styles) / sizeof(styles[0]);
#endif

void write_boxes(const VECTOR(box_t) box, FILE *fp) {
    for (unsigned i = 0; i < VECTOR_size(box); ++i) {
        fprintf(fp, "[%s %d %d %d %d", linetype_name[box[i].style], box[i].top, box[i].left,
                box[i].right, box[i].bottom);
        if (box[i].label) {
            fprintf(fp, " \"");
            quote_utf8(box[i].label, fp);
            fprintf(fp, "\""); }
        fprintf(fp, "]\n");
    }
}

VECTOR(box_t) read_boxes(FILE *fp) {
    VECTOR(box_t) rv = 0;
    while (1) {
        char type[16];
        box_t box;
        if (fscanf(fp, " [%15s%d%d%d%d ", type, &box.top, &box.left,
                   &box.right, &box.bottom) != 5) {
            if (!feof(fp))
                fprintf(stderr, "error reading boxes\n");
            break; }
        box.style = 0;
        for (unsigned j = 0; linetype_name[j]; ++j)
            if (!strcmp(type, linetype_name[j])) {
                box.style = j;
                break; }
        int ch = getc(fp);
        if (ch == '"') {
            box.label = unquote_utf8(fp, '"');
            do { ch = getc(fp); } while (ch == ' ');
        } else {
            box.label = 0; }
        box.fmt = 0;
        box.fmt_width = box.fmt_height = 0;
        if (ch != ']') {
            fprintf(stderr, "error reading boxes (missing ']')\n");
            break; }
        if (!box.style) {
            fprintf(stderr, "bad box style %s\n", type);
            continue; }
        VECTOR_add(rv, box); }
    return rv;
}

void erase_box(const box_t *box) {
    int width = box->right - box->left + 1;
    if (width <= 0) return;
    for (int row = box->top; row <= box->bottom; ++row)
        printf("%s%*s", gotoyx(row, box->left), width, "");
    fflush(stdout);
}

void draw_box_image(image_t *image, const box_t *box) {
    if (box->top >= box->bottom || box->left >= box->right) return;
    drawbox(image, box->style, box->top, box->left, box->bottom, box->right);
}

void draw_box_label(box_t *box) {
    int width = box->right - box->left - 1;     // "inside" dimensions of the box
    int height = box->bottom - box->top - 1;
    if (width <= 0 || height <= 0) return;
    for (int r = box->top+1; r < box->bottom; ++r)
        printf("%s%*s", gotoyx(r, box->left+1), width, "");
    if (box->label) {
        if (!box->fmt || box->fmt->start != box->label ||
            box->fmt_width != width || box->fmt_height != height) {
            if (box->fmt) VECTOR_fini(box->fmt);
            box->fmt = layout_string(box->label, (box->fmt_width = width),
                                     (box->fmt_height = height)); }
        int start = box->top + 1 + (height - VECTOR_size(box->fmt))/2;
        for (unsigned i = 0; i < VECTOR_size(box->fmt); ++i)
            printf("%s%.*s", gotoyx(start + i, box->left + 1 + (width - box->fmt[i].width)/2),
                   box->fmt[i].size, box->fmt[i].start); }
}

int set_box_edit(box_t *box, char *edit) {
    int width = box->right - box->left - 1;     // "inside" dimensions of the box
    int height = box->bottom - box->top - 1;
    int x, y;
    if (box->label && width > 0 && height > 0) {
        if (!box->fmt || box->fmt->start != box->label ||
            box->fmt_width != width || box->fmt_height != height) {
            assert(false);  // should never happen
            if (box->fmt) VECTOR_fini(box->fmt);
            box->fmt = layout_string(box->label, (box->fmt_width = width),
                                     (box->fmt_height = height)); }
        y = box->top + 1 + (height - VECTOR_size(box->fmt))/2;
        struct line *line = VECTOR_end(box->fmt);
        while (line > VECTOR_begin(box->fmt) && edit < (--line)->start);
        y += line - VECTOR_begin(box->fmt);
        if (edit <= line->start + line->size) {
            x = box->left + 1 + (width - line->width)/2;
            x += layout_width(line->start, edit);
        } else
            x = box->right;
    } else {
        y = box->top + height/2 + 1;
        x = box->left + width/2 + 1;
    }
    fputs(gotoyx(y, x), stdout);
    return MOUSE | (x << 12) | y;
}

char *get_box_edit(box_t *box, int y, int x) {
    if (!box->label) return 0;
    int width = box->right - box->left - 1;     // "inside" dimensions of the box
    int height = box->bottom - box->top - 1;
    if (width <= 0 || height <= 0) return 0;
    int start = box->top + 1 + (height - VECTOR_size(box->fmt))/2;
    if (y < start) return box->label;
    if (y >= start + (int)VECTOR_size(box->fmt)) return VECTOR_end(box->label);
    struct line *line = &box->fmt[y - start];
    start = box->left + 1 + (width - line->width)/2;
    if (x < start) return (char *)line->start;
    if (x >= start + (int)line->width) return (char *)line->start + line->size;
    return layout_index((char *)line->start, x - start);
}

box_t *find_box(VECTOR(box_t) all, int row, int col, boxedge_t *edge) {
    for (box_t *b = VECTOR_begin(all); b < VECTOR_end(all); ++b) {
        if (row < b->top || row > b->bottom || col < b->left || col > b->right) continue;
        if (edge) {
            if (row == b->top)
                *edge = col == b->left ? TOP_LEFT : col == b->right ? TOP_RIGHT : TOP;
            else if (row == b->bottom)
                *edge = col == b->left ? BOTTOM_LEFT : col == b->right ? BOTTOM_RIGHT : BOTTOM;
            else
                *edge = col == b->left ? LEFT : col == b->right ? RIGHT : NONE; }
        return b; }
    return 0;
}

bool box_overlaps_any(const VECTOR(box_t) all, const box_t *box) {
    for (const box_t *b = VECTOR_begin(all); b < VECTOR_end(all); ++b) {
        if (b == box) continue;
        if (b->top <= box->bottom && b->bottom >= box->top &&
            b->left <= box->right && b->right >= box->left)
            return true;
    }
    return false;
}

int resize_box(image_t *screen, box_t *box, boxedge_t adjust, const VECTOR(box_t) all) {
    int loc;
    image_t *background = dupimage(screen);
    clearimage(background);
    unsigned screen_size = getsize();
    for (const box_t *b = VECTOR_begin(all); b != VECTOR_end(all); ++b) {
        if (b == box) continue;
        draw_box_image(background, b);
    }
    image_t *prev = dupimage(screen);
    image_t *next = dupimage(background);
    do {
        loc = getch();
        if (!(loc & MOUSE)) {
            if (loc == KEY_END) return loc;
            continue; }
        int x = MOUSE_X(loc);
        int y = MOUSE_Y(loc);
        int oldpos[4] = { box->top, box->left, box->right, box-> bottom };
        switch (adjust) {
        case NONE:
            if (y == box->top)
                adjust = x == box->left ? TOP_LEFT : x == box->right ? TOP_RIGHT : TOP;
            else if (y == box->bottom)
                adjust = x == box->left ? BOTTOM_LEFT : x == box->right ? BOTTOM_RIGHT : BOTTOM;
            else
                adjust = x == box->left ? LEFT : x == box->right ? RIGHT : NONE;
            break;
        case TOP:
            box->top = min(y, box->bottom - 1);
            break;
        case TOP_RIGHT:
            box->top = min(y, box->bottom - 1);
            box->right = max(x, box->left + 1);
            break;
        case RIGHT:
            box->right = max(x, box->left + 1);
            break;
        case BOTTOM_RIGHT:
            box->bottom = max(y, box->top + 1);
            box->right = max(x, box->left + 1);
            break;
        case BOTTOM:
            box->bottom = max(y, box->top + 1);
            break;
        case BOTTOM_LEFT:
            box->bottom = max(y, box->top + 1);
            box->left = min(x, box->right - 1);
            break;
        case LEFT:
            box->left = min(x, box->right - 1);
            break;
        case TOP_LEFT:
            box->top = min(y, box->bottom - 1);
            box->left = min(x, box->right - 1);
            break; }
        if (all && box_overlaps_any(all, box)) {
            box->top = oldpos[0];
            box->left = oldpos[1];
            box->right = oldpos[2];
            box->bottom = oldpos[3];
        }
        draw_box_image(next, box);
        update_image(prev, next, TOPLEFT, screen_size, TOPLEFT);
        draw_box_label(box);
        fputs(gotoyx(y, x), stdout);
        fflush(stdout);
        image_t *tmp = prev;
        prev = next;
        copyimage(next = tmp, background);
    } while (!MOUSE_UP(loc));
    copyimage(screen, prev);
    free(background);
    free(prev);
    free(next);
    return loc;
}

int move_box(image_t *screen, box_t *box, int offy, int offx, const VECTOR(box_t) all) {
    int loc;
    image_t *background = dupimage(screen);
    clearimage(background);
    unsigned screen_size = getsize();
    for (const box_t *b = VECTOR_begin(all); b != VECTOR_end(all); ++b) {
        if (b == box) continue;
        draw_box_image(background, b);
    }
    image_t *prev = dupimage(screen);
    image_t *next = dupimage(background);
    do {
        loc = getch();
        if (!(loc & MOUSE)) {
            if (loc == KEY_END) return loc;
            continue; }
        int x = MOUSE_X(loc);
        int y = MOUSE_Y(loc);
        int dx = x - offx - box->left;
        int dy = y - offy - box->top;
        box->top += dy;
        box->bottom += dy;
        box->left += dx;
        box->right += dx;
        if (box_overlaps_any(all, box)) {
            box->top -= dy;
            box->bottom -= dy;
            box->left -= dx;
            box->right -= dx;
            dx = dy = 0;
        }
        if (dx != 0 || dy != 0) {
            draw_box_image(next, box);
            update_image(prev, next, TOPLEFT, screen_size, TOPLEFT);
            draw_box_label(box);
            fputs(gotoyx(y, x), stdout);
            fflush(stdout);
            image_t *tmp = prev;
            prev = next;
            copyimage(next = tmp, background);
        }
    } while (!MOUSE_UP(loc));
    copyimage(screen, prev);
    free(background);
    free(prev);
    free(next);
    return loc;
}
