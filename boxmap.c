#include <stdlib.h>
#include <string.h>
#include "ansi.h"
#include "boxes.h"
#include "errors.h"

struct boxmap_t {
    unsigned            height, width;
    /* height * width elements of data */
    unsigned short      data[];
};

static void setrow(unsigned short *p, int box, int l, int c, int r, int width) {
    *p = box + ((l + 1) << 10l);
    while (--width > 0) *++p = box + ((c + 1) << 10);
    *++p = box + ((r + 1) << 10);
}

static void setlink(unsigned short *p, int label) {
    if (*p == 0) {
        *p = label;
    } else if (*p != label) {
        if ((*p >> 10) >= 10) *p = 0xffff;
    }
}

static unsigned short *hline(unsigned short *p, int label, int from, int to) {
    if (from < to)
        for (int i = from; i < to; ++i)
            setlink(++p, label);
    else
        for (int i = from; i > to; --i)
            setlink(--p, label);
    return p;
}

static unsigned short *vline(boxmap_t *bm, unsigned short *p, int label, int from, int to) {
    if (from < to)
        for (int i = from; i < to; ++i)
            setlink(p += bm->width, label);
    else
        for (int i = from; i > to; --i)
            setlink(p -= bm->width, label);
    return p;
}

// must match draw_link_image in links.c
static void drawlink(boxmap_t *bm, int label, const loc_t *from, const loc_t *to) {
    unsigned short *p = bm->data + from->row * bm->width + from->col;
    if (from->row == to->row) {
        p = hline(p, label, from->col, to->col);
    } else if (from->col == to->col) {
        p = vline(bm, p, label, from->row, to->row);
    } else if (abs(from->row - to->row) <= abs(from->col - to->col)) {
        int mid = (from->col + to->col) / 2;
        p = hline(p, label, from->col, mid);
        p = vline(bm, p, label, from->row, to->row);
        p = hline(p, label, mid, to->col);
    } else {
        int mid = (from->row + to->row) / 2;
        p = vline(bm, p, label, from->row, mid);
        p = hline(p, label, from->col, to->col);
        p = vline(bm, p, label, mid, to->row);
    }
}

boxmap_t *create_boxmap(const VECTOR(box_t) boxes, unsigned screen_size) {
    int width = GETX(screen_size) + 1;
    int height = GETY(screen_size) + 1;
    unsigned maxlinks = 0;
    VECTOR_FOREACH(b, boxes) {
        if (b->right >= width) width = b->right + 1;
        if (b->bottom >= height) height = b->bottom + 1;
        if (VECTOR_size(b->links) > maxlinks) maxlinks = VECTOR_size(b->links);
        VECTOR_FOREACH(l, b->links) {
            VECTOR_FOREACH(p, l->points) {
                if (p->col >= width) width = p->col + 1;
                if (p->row >= height) height = p->row + 1; } } }
    if (maxlinks > 52 || VECTOR_size(boxes) > 1022)
        errmsg("boxmap overflow(boxes=%d, links=%d)", VECTOR_size(boxes), maxlinks);
    boxmap_t *rv = malloc(sizeof(boxmap_t) + sizeof(unsigned short) * width * height);
    rv->width = width;
    rv->height = height;
    regen_boxmap(rv, boxes);
    return rv;
}

void regen_boxmap(boxmap_t *bm, const VECTOR(box_t) boxes) {
    memset(bm->data, 0, sizeof(unsigned short) * bm->width * bm->height);

    VECTOR_FOREACH(b, boxes) {
        unsigned short *p = bm->data + b->top * bm->width + b->left;
        setrow(p, b - boxes, TOP_LEFT, TOP, TOP_RIGHT, b->right - b->left);
        p += bm->width;
        for (int r = b->top+1; r < b->bottom; ++r, p += bm->width)
            setrow(p, b - boxes, LEFT, NONE, RIGHT, b->right - b->left);
        setrow(p, b - boxes, BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT, b->right - b->left);
        VECTOR_FOREACH(l, b->links) {
            loc_t *prev = 0;
            VECTOR_FOREACH(p, l->points) {
                if (prev) drawlink(bm, b - boxes + ((l - b->links + 10) << 10), prev, p);
                prev = p; } } }
}

box_t *find_box(boxmap_t *bm, VECTOR(box_t) all, int row, int col, boxedge_t *edge, int *link) {
    unsigned label = bm->data[row * bm->width + col];
    if (label && label != 0xffff) {
        box_t *rv = all + (label & 0x3ff);
        label >>= 10;
        if (label >= 10) {
            *link = label - 10;
            *edge = NONE;
        } else {
            *link = -1;
            *edge = label - 1;
        }
        return rv;
    }
    return nullptr;
}
