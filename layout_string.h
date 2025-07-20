#ifndef _layout_string_h_
#define _layout_string_h_

#include "vector.h"

struct line {
    const char *start;  // start of line
    unsigned size;      // size in bytes
    unsigned width;     // width in character cells
};

VECTOR(struct line) layout_string(const char *s, unsigned width, unsigned height);
char *layout_index(char *s, unsigned idx);
int layout_width(const char *s, const char *e);

#endif /* _layout_string_h_ */
