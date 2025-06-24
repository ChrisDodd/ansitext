#include "vector.h"

struct line {
    const char *start;  // start of line
    unsigned size;      // size in bytes
    unsigned width;     // width in character cells
};

VECTOR(struct line) layout_string(const char *s, unsigned width, unsigned height);
