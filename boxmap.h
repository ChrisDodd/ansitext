#ifndef _boxmap_h_
#define _boxmap_h_

#include "boxes.h"

typedef struct boxmap_t boxmap_t;

boxmap_t *create_boxmap(const VECTOR(box_t) boxes, unsigned screen_size);
void regen_boxmap(boxmap_t *bm, const VECTOR(box_t) boxes);
box_t *find_box(boxmap_t *bm, VECTOR(box_t) all, int row, int col, boxedge_t *edge, int *link);

#endif /* _boxmap_h_ */
