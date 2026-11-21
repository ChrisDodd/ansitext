#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ansi.h"
#include "boxes.h"
#include "errors.h"
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

const char *boxedge_name[] = { "none", "t", "tr", "r", "br", "b", "bl", "l", "tl", 0 };

void write_boxes(const VECTOR(box_t) boxes, FILE *fp) {
    VECTOR_FOREACH(box, boxes) {
        fprintf(fp, "[%s %d %d %d %d", linetype_name[box->style], box->top, box->left,
                box->right, box->bottom);
        if (box->label) {
            fprintf(fp, " \"");
            quote_utf8(box->label, fp);
            fprintf(fp, "\""); }
        if (VECTOR_size(box->links)) {
            VECTOR_FOREACH(link, box->links) {
                fprintf(fp, " (%s %d", boxedge_name[link->edge], link->off);
                VECTOR_FOREACH(point, link->points)
                    fprintf(fp, " (%d %d)", point->row, point->col);
                fprintf(fp, " %d)", link->endbox); } }
        fprintf(fp, "]\n");
    }
}

void boxes_fixup(VECTOR(box_t) boxes) {
    VECTOR_FOREACH(box, boxes) {
        VECTOR_FOREACH(link, box->links) {
            loc_t *st = link->points;
            loc_t *end = link->points + VECTOR_size(link->points) - 1;
            if (end <= st) errmsg("only %zd points in a link", end - st + 1);
            boxedge_t edge = box_edge(box, st->row, st->col);
            int off = 0;
            if (edge == TOP || edge == BOTTOM)
                off = st->col - box->left;
            else if (edge == LEFT || edge == RIGHT)
                off = st->row - box->top;
            else if (edge == NONE) {
                errmsg("link does not start on the edge");
                edge = link->edge; }
            if (link->edge != edge) {
                errmsg("link edge wrong (%s instead of %s)", boxedge_name[link->edge],
                       boxedge_name[edge]);
                link->edge = edge; }
            if (link->off != off) {
                errmsg("link offset wrong (%d instead of %d)", link->off, off);
                off = link->off; }
            box_t *endbox = find_box(boxes, end->row, end->col, &edge);
            if (link->endbox != (endbox ? endbox - boxes : -1)) {
                errmsg("link endbox wrong (%d instead of %zd)", link->endbox,
                       (endbox ? endbox - boxes : -1));
                link->endbox = (endbox ? endbox - boxes : -1); }
            if (endbox) {
                inlink_t in;
                in.box = box - boxes;
                in.link = link - box->links;
                in.edge = edge;
                in.off = 0;
                if (edge == TOP || edge == BOTTOM)
                    in.off = end->col - endbox->left;
                else if (edge == LEFT || edge == RIGHT)
                    in.off = end->row - endbox->top;
                else if (edge == NONE)
                    errmsg("link does not end on the edge");
                VECTOR_add(endbox->inlinks, in);
            }
        }
    }
}

VECTOR(box_t) read_boxes(FILE *fp) {
    VECTOR(box_t) rv = 0;
    while (1) {
        char type[16];
        box_t box;
        memset(&box, 0, sizeof(box));
        if (fscanf(fp, " [%15s%d%d%d%d ", type, &box.top, &box.left,
                   &box.right, &box.bottom) != 5) {
            if (!feof(fp))
                errmsg("error reading boxes");
            break; }
        for (unsigned j = 0; linetype_name[j]; ++j)
            if (!strcmp(type, linetype_name[j])) {
                box.style = j;
                break; }
        char ch = getc(fp);
        if (ch == '"') {
            box.label = unquote_utf8(fp, '"');
            do { ch = getc(fp); } while (ch == ' '); }
        while (ch == '(') {
            outlink_t link;
            memset(&link, 0, sizeof(link));
            if (fscanf(fp, "%15s%d %c", type, &link.off, &ch) != 3) {
                errmsg("error reading boxes (bad link)");
                cleanup_box(&box);
                return rv; }
            for (unsigned j = 0; boxedge_name[j]; ++j)
                if (!strcmp(type, boxedge_name[j])) {
                    link.edge = j;
                    break; }
            while (ch == '(') {
                loc_t point;
                if (fscanf(fp, "%d%d ) %c", &point.row, &point.col, &ch) != 3) {
                    errmsg("error reading link (bad point)");
                    VECTOR_fini(link.points);
                    cleanup_box(&box);
                    return rv; }
                VECTOR_add(link.points, point); }
            ungetc(ch, fp);
            if (fscanf(fp, "%d %c", &link.endbox, &ch) != 2 || ch != ')') {
                errmsg("error reading link (bad endbox)");
                VECTOR_fini(link.points);
                cleanup_box(&box);
                return rv; }
            VECTOR_add(box.links, link);
            do { ch = getc(fp); } while (ch == ' '); }
        if (ch != ']') {
            errmsg("error reading boxes (missing ']')");
            cleanup_box(&box);
            break; }
        if (!box.style) {
            errmsg("bad box style %s", type);
            cleanup_box(&box);
            continue; }
        VECTOR_add(rv, box); }
    boxes_fixup(rv);
    return rv;
}

void cleanup_box(box_t *box) {
    VECTOR_fini(box->label);
    VECTOR_fini(box->fmt);
    VECTOR_FOREACH(link, box->links) VECTOR_fini(link->points);
    VECTOR_fini(box->links);
    VECTOR_fini(box->inlinks);
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
    if (box->label && VECTOR_size(box->label)) {
        if (!box->fmt || VECTOR_size(box->fmt) == 0 || box->fmt->start != box->label ||
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
    if (box->label && VECTOR_size(box->label) && width > 0 && height > 0) {
        if (!box->fmt || VECTOR_size(box->fmt) == 0 || box->fmt->start != box->label ||
            box->fmt_width != width || box->fmt_height != height) {
            // can only happen if the label contains NUL bytes
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

boxedge_t box_edge(box_t *b, int row, int col) {
    if (row == b->top)
        return col == b->left ? TOP_LEFT : col == b->right ? TOP_RIGHT : TOP;
    else if (row == b->bottom)
        return col == b->left ? BOTTOM_LEFT : col == b->right ? BOTTOM_RIGHT : BOTTOM;
    else
        return col == b->left ? LEFT : col == b->right ? RIGHT : NONE;
}

box_t *find_box(VECTOR(box_t) all, int row, int col, boxedge_t *edge) {
    for (box_t *b = VECTOR_begin(all); b < VECTOR_end(all); ++b) {
        if (row < b->top || row > b->bottom || col < b->left || col > b->right) continue;
        if (edge) *edge = box_edge(b, row, col);
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
    VECTOR_FOREACH(b, all) {
        if (b == box) continue;
        draw_box_image(background, b);
        VECTOR_FOREACH(l, b->links)
            if (l->endbox != box - all) draw_link_image(background, b, l); }
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
            adjust = box_edge(box, y, x);
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
        VECTOR_FOREACH(l, box->links) {
            l->off = fix_link_end(l->points, box, l->edge, l->off);
            draw_link_image(next, box, l); }
        VECTOR_FOREACH(in, box->inlinks) {
            outlink_t *l = &all[in->box].links[in->link];
            in->off = fix_link_end(&VECTOR_top(l->points), box, in->edge, in->off);
            draw_link_image(next, &all[in->box], l); }
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
    VECTOR_FOREACH(b, all) {
        if (b == box) continue;
        draw_box_image(background, b);
        VECTOR_FOREACH(l, b->links)
            if (l->endbox != box - all) draw_link_image(background, b, l); }
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
            VECTOR_FOREACH(l, box->links) {
                l->points[0].col += dx;
                l->points[0].row += dy;
                draw_link_image(next, box, l); }
            VECTOR_FOREACH(in, box->inlinks) {
                outlink_t *l = &all[in->box].links[in->link];
                VECTOR_top(l->points).col += dx;
                VECTOR_top(l->points).row += dy;
                draw_link_image(next, &all[in->box], l); }
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
