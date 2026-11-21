#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "ansi.h"
#include "boxes.h"
#include "errors.h"

void draw_link_image(image_t *image, const box_t *, const outlink_t *link) {
    VECTOR_FOREACH(n, link->points) {
        if (n == link->points) continue;
        if (n[-1].row == n->row) drawhline(image, SINGLE, n->row, n[-1].col, n->col);
        else if (n[-1].col == n->col) drawvline(image, SINGLE, n[-1].row, n->row, n->col);
        else if (abs(n[-1].row - n->row) <= abs(n[-1].col - n->col)) {
            int mid = (n[-1].col + n->col) / 2;
            drawhline(image, SINGLE, n[-1].row, n[-1].col, mid);
            drawvline(image, SINGLE, n[-1].row, n->row, mid);
            drawhline(image, SINGLE, n->row, mid, n->col);
        } else {
            int mid = (n[-1].row + n->row) / 2;
            drawvline(image, SINGLE, n[-1].row, mid, n[-1].col);
            drawhline(image, SINGLE, mid, n[-1].col, n->col);
            drawvline(image, SINGLE, mid, n->row, n->col);
        }
    }
}

int add_link(image_t *screen, box_t *from, boxedge_t edge, int row, int col, VECTOR(box_t) all) {
    int loc;
    unsigned screen_size = getsize();
    outlink_t link;
    memset(&link, 0, sizeof(link));
    link.endbox = -1;
    link.edge = edge;
    if (edge == TOP || edge == BOTTOM)
        link.off = col - from->left;
    else if (edge == LEFT || edge == RIGHT)
        link.off = row - from->top;
    VECTOR_resize(link.points, 2);
    link.points[0].row = link.points[1].row = row;
    link.points[0].col = link.points[1].col = col;

    image_t *prev = dupimage(screen);
    image_t *next = dupimage(screen);
    do {
        loc = getch();
        if (!(loc & MOUSE)) {
            if (loc == KEY_END) return loc;
            continue; }
        int x = MOUSE_X(loc);
        int y = MOUSE_Y(loc);
        box_t *b = find_box(all, y, x, &edge);
        if (b) {
            if (link.endbox == b - all && edge == NONE) continue;
            link.endbox = b - all;
        } else {
            link.endbox = -1;
        }
        link.points[1].row = y;
        link.points[1].col = x;
        draw_link_image(next, from, &link);
        update_image(prev, next, TOPLEFT, screen_size, TOPLEFT);
        fputs(gotoyx(y, x), stdout);
        fflush(stdout);
        image_t *tmp = prev;
        prev = next;
        copyimage(next = tmp, screen);
    } while (!MOUSE_UP(loc));
    if (link.points[0].row == link.points[1].row && link.points[0].col == link.points[1].col) {
        VECTOR_fini(link.points);
        return 0; }
    if (link.endbox >= 0) {
        box_t *endbox = &all[link.endbox];
        inlink_t in;
        in.box = from - all;
        in.link = VECTOR_size(from->links);
        in.edge = box_edge(endbox, link.points[1].row, link.points[1].col);
        if (in.edge == TOP || in.edge == BOTTOM)
            in.off = link.points[1].col - endbox->left;
        else if (in.edge == LEFT || in.edge == RIGHT)
            in.off = link.points[1].row - endbox->top;
        else if (in.edge == NONE)
            errmsg("link does not end on an edge");
        VECTOR_add(endbox->inlinks, in);
    }
    VECTOR_add(from->links, link);
    return 0;
}

int fix_link_end(loc_t *end, const box_t *box, boxedge_t edge, int off) {
    switch (edge) {
    case TOP:
        if (off > box->right - box->left) off = box->right - box->left;
        end->row = box->top;
        end->col = box->left + off;
        break;
    case TOP_RIGHT:
        end->row = box->top;
        end->col = box->right;
        off = 0;
        break;
    case RIGHT:
        if (off > box->bottom - box->top) off = box->bottom - box->top;
        end->row = box->top + off;
        end->col = box->right;
        break;
    case BOTTOM_RIGHT:
        end->row = box->bottom;
        end->col = box->right;
        off = 0;
        break;
    case BOTTOM:
        if (off > box->right - box->left) off = box->right - box->left;
        end->row = box->bottom;
        end->col = box->left + off;
        break;
    case BOTTOM_LEFT:
        end->row = box->bottom;
        end->col = box->left;
        off = 0;
        break;
    case LEFT:
        if (off > box->bottom - box->top) off = box->bottom - box->top;
        end->row = box->top + off;
        end->col = box->left;
        break;
    case TOP_LEFT:
        end->row = box->top;
        end->col = box->left;
        off = 0;
        break;
    default:
        errmsg("bad edge in fix_link_end");
    }
    return off;
}
