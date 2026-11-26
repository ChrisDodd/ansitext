#ifndef _boxes_h_
#define _boxes_h_

#include <stdio.h>
#include <stdbool.h>
#include "layout_string.h"
#include "linedraw.h"
#include "vector.h"

#if 0
typedef struct style_t {
    const char *name;
    const char *tl, *tr, *bl, *br, *horiz, *vert;
    const char *t, *l, *r, *b;
} style_t;
extern const style_t styles[];
extern const unsigned styles_count;
#endif

typedef struct loc_t {
    int row, col;
} loc_t;

typedef enum boxedge_t {
    NONE, TOP, TOP_RIGHT, RIGHT, BOTTOM_RIGHT, BOTTOM, BOTTOM_LEFT, LEFT, TOP_LEFT
} boxedge_t;

typedef struct outlink_t {
    boxedge_t edge;
    int off;
    VECTOR(loc_t) points;
    int endbox;
} outlink_t;

typedef struct inlink_t {
    int box, link;
    boxedge_t edge;
    int off;
} inlink_t;

typedef struct box_t {
    linetype_t style;
    int top, left, right, bottom;
    VECTOR(char) label;
    VECTOR(struct line) fmt;
    VECTOR(outlink_t) links;
    VECTOR(inlink_t) inlinks;
    int fmt_width, fmt_height;
} box_t;

typedef struct boxmap_t boxmap_t;

int add_link(image_t *, box_t *, boxedge_t, int row, int col, boxmap_t *, VECTOR(box_t));
boxedge_t box_edge(box_t *b, int row, int col);
bool box_overlaps_any(const VECTOR(box_t) all, const box_t *box);
void cleanup_box(box_t *);
boxmap_t *create_boxmap(const VECTOR(box_t) boxes, unsigned screen_size);
void draw_box_image(image_t *image, const box_t *box);
void draw_box_label(box_t *box);
void draw_link_image(image_t *, const box_t *, const outlink_t *, linetype_t);
void erase_box(const box_t *box);
box_t *find_box(boxmap_t *bm, VECTOR(box_t) all, int row, int col, boxedge_t *edge, int *link);
void fix_boxes_after_removal(VECTOR(box_t) boxes, int removed);
int fix_link_end(loc_t *end, const box_t *box, boxedge_t edge, int off);
char *get_box_edit(box_t *box, int y, int x);
int move_box(image_t *screen, box_t *box, int dy, int dx, const VECTOR(box_t));
VECTOR(box_t) read_boxes(FILE *fp);
void regen_boxmap(boxmap_t *bm, const VECTOR(box_t) boxes);
void remove_link(VECTOR(box_t) all, box_t *box, int link);
void remove_all_links(VECTOR(box_t) all, box_t *box);
int resize_box(image_t *screen, box_t *box, boxedge_t adjust, const VECTOR(box_t));
int set_box_edit(box_t *box, char *edit);
void write_boxes(const VECTOR(box_t) box, FILE *fp);

static inline unsigned boxTL(box_t *box) { return MAKEYX(box->top, box->left); }
static inline unsigned boxBR(box_t *box) { return MAKEYX(box->bottom, box->right); }

#endif /* _boxes_h_ */
