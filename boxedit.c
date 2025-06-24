#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ansi.h"
#include "vector.h"
#include "layout_string.h"
#include "utf8.h"

static inline int min(int a, int b) { return a < b ? a : b; }
static inline int max(int a, int b) { return a > b ? a : b; }

struct style_t {
    char *name;
    char *tl, *tr, *bl, *br, *horiz, *vert;
    char *t, *l, *r, *b;
} styles[] = {
    { "square", "\u250c", "\u2510", "\u2514", "\u2518", "\u2500", "\u2502",
      "\u2534", "\u2524", "\u251c", "\u252c" },
    { "rounded", "\u256d", "\u256e", "\u2570", "\u256f", "\u2500", "\u2502",
      "\u2534", "\u2524", "\u251c", "\u252c" },
    { "double", "\u2554", "\u2557", "\u255a", "\u255d", "\u2550", "\u2551",
      "\u2567", "\u2562", "\u255f", "\u2564" },
    { "heavy", "\u250f", "\u2513", "\u2517", "\u251b", "\u2501", "\u2503",
      "\u2537", "\u2528", "\u2520", "\u252f" },
};
unsigned styles_count = sizeof(styles) / sizeof(styles[0]);

enum adjust_t { TOP, TOP_RIGHT, RIGHT, BOTTOM_RIGHT, BOTTOM, BOTTOM_LEFT, LEFT, TOP_LEFT };

struct box_t {
    const struct style_t *style;
    int top, left, right, bottom;
    VECTOR(char) label;
    VECTOR(struct line) fmt;
    int fmt_width, fmt_height;
};

void write_boxes(const VECTOR(struct box_t) box, FILE *fp) {
    for (unsigned i = 0; i < VECTOR_size(box); ++i) {
        fprintf(fp, "[%s %d %d %d %d", box[i].style->name, box[i].top, box[i].left,
                box[i].right, box[i].bottom);
        if (box[i].label) {
            fprintf(fp, " \"");
            quote_utf8(box[i].label, fp);
            fprintf(fp, "\""); }
        fprintf(fp, "]\n");
    }
}

VECTOR(struct box_t) read_boxes(FILE *fp) {
    VECTOR(struct box_t) rv = 0;
    while (1) {
        char type[16];
        struct box_t box;
        if (fscanf(fp, " [%15s%d%d%d%d ", type, &box.top, &box.left,
                   &box.right, &box.bottom) != 5) {
            if (!feof(fp))
                fprintf(stderr, "error reading boxes\n");
            break; }
        box.style = 0;
        for (unsigned j = 0; j < styles_count; ++j)
            if (!strcmp(type, styles[j].name)) {
                box.style = &styles[j];
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

void erase_box(const struct box_t *box) {
    int width = box->right - box->left + 1;
    if (width <= 0) return;
    for (int row = box->top; row <= box->bottom; ++row)
        printf("%s%*s", gotoyx(row, box->left), width, "");
    fflush(stdout);
}

void draw_box(struct box_t *box) {
    if (box->top >= box->bottom || box->left >= box->right) return;
    int width = box->right - box->left - 1;     // "inside" dimensions of the box
    int height = box->bottom - box->top - 1;
    printf("%s%s", gotoyx(box->top, box->left), box->style->tl);
    for (int i = width; i > 0; --i) fputs(box->style->horiz, stdout);
    fputs(box->style->tr, stdout);
    for (int row = box->top + 1; row < box->bottom; ++row)
        printf("%s%s%*s%s", gotoyx(row, box->left), box->style->vert,
               width, "", box->style->vert);
    printf("%s%s", gotoyx(box->bottom, box->left), box->style->bl);
    for (int i = width; i > 0; --i) fputs(box->style->horiz, stdout);
    fputs(box->style->br, stdout);
    if (box->label && width > 0 && height > 0) {
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

int resize_box(struct box_t *box, enum adjust_t adjust) {
    int loc;
    do {
        loc = getch();
        if (!(loc & MOUSE)) {
            if (loc == KEY_END) return loc;
            continue; }
        int x = MOUSE_X(loc);
        int y = MOUSE_Y(loc);
        switch (adjust) {
        case TOP:
            if (y > box->top) erase_box(box);
            box->top = min(y, box->bottom - 1);
            break;
        case TOP_RIGHT:
            if (x < box->right || y > box->top) erase_box(box);
            box->top = min(y, box->bottom - 1);
            box->right = max(x, box->left + 1);
            break;
        case RIGHT:
            if (x < box->right) erase_box(box);
            box->right = max(x, box->left + 1);
            break;
        case BOTTOM_RIGHT:
            if (x < box->right || y < box->bottom) erase_box(box);
            box->bottom = max(y, box->top + 1);
            box->right = max(x, box->left + 1);
            break;
        case BOTTOM:
            if (y < box->bottom) erase_box(box);
            box->bottom = max(y, box->top + 1);
            break;
        case BOTTOM_LEFT:
            if (x > box->left || y < box->bottom) erase_box(box);
            box->bottom = max(y, box->top + 1);
            box->left = min(x, box->right - 1);
            break;
        case LEFT:
            if (x > box->left) erase_box(box);
            box->left = min(x, box->right - 1);
            break;
        case TOP_LEFT:
            if (x > box->left || y > box->top) erase_box(box);
            box->top = min(y, box->bottom - 1);
            box->left = min(x, box->right - 1);
            break; }
        draw_box(box);
        fputs(gotoyx(y, x), stdout);
        fflush(stdout);
    } while (!MOUSE_UP(loc));
    return 0;
}

int main(int ac, char **av) {
    int style_idx = 0;
    VECTOR(struct box_t) boxes = 0;
    printf("%s%s", CLEARALL, gotoyx(0, 0));
    fflush(stdout);
    if (ac == 2) {
        FILE *fp = fopen(av[1], "r");
        if (fp) {
            boxes = read_boxes(fp);
        } else {
            fprintf(stderr, "Can't open %s\n", av[1]); } }
    fputs(MOUSE_BTN_ENABLE MOUSE_SGR, stdout);
    fflush(stdout);
    for (struct box_t *b = VECTOR_begin(boxes); b < VECTOR_end(boxes); ++b) draw_box(b);
    fflush(stdout);
    while (1) {
        int ch = getch();
        if (ch & MOUSE) {
            if (MOUSE_1(ch)) {
            } else if (MOUSE_3(ch)) {
                struct box_t tmp;
                memset(&tmp, 0, sizeof tmp);
                tmp.style = &styles[style_idx];
                tmp.label = "The quick brown fox\njumps over the lazy dog";
                tmp.top = tmp.bottom = MOUSE_Y(ch);
                tmp.left = tmp.right = MOUSE_X(ch);
                tmp.bottom++;
                tmp.right++;
                VECTOR_add(boxes, tmp);
                ch = resize_box(&VECTOR_top(boxes), BOTTOM_RIGHT);
            } else {
                printf("%smouse: %x", gotoyx(0, 50), ch);
                fflush(stdout);
                // other button -- menu?
            }
        }
        switch (ch) {
        case KEY_END:
            fputs(gotoyx(999, 0), stdout);
            fflush(stdout);
            reset();
            if (ac == 2 && !VECTOR_empty(boxes)) {
                FILE *fp = fopen(av[1], "w");
                if (fp) {
                    write_boxes(boxes, fp);
                    fclose(fp);
                } else {
                    fprintf(stderr, "Can't write to %s\n", av[1]); } }
            exit(0);
        case KEY_F1: style_idx = 0; break;
        case KEY_F2: style_idx = 1; break;
        case KEY_F3: style_idx = 2; break;
        case KEY_F4: style_idx = 3; break;
        }
    }
}
