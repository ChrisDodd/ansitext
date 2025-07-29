#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ansi.h"
#include "linedraw.h"

#define TOP(X)          (((X) & 3) << 6)
#define LEFT(X)         (((X) & 3) << 4)
#define BOTTOM(X)       (((X) & 3) << 2)
#define RIGHT(X)        ((X) & 3)
#define VERTICAL(X)     (TOP(X)|BOTTOM(X))
#define HORIZ(X)        (RIGHT(X)|LEFT(X))

#define USE_CURVES 1

#if USE_CURVES
#define CURVE(A, B)     B
#else
#define CURVE(A, B)     A
#endif

char linechars[256][8] = {
    [0] = " ",
    [HORIZ(SINGLE)] = "\u2500", [HORIZ(HEAVY)] = "\u2501",
    [VERTICAL(SINGLE)] = "\u2502", [VERTICAL(HEAVY)] = "\u2503",
    [BOTTOM(SINGLE) + RIGHT(SINGLE)] = CURVE("\u250c", "\u256d"),
    [BOTTOM(SINGLE) + RIGHT(HEAVY)] = "\u250d",
    [BOTTOM(HEAVY) + RIGHT(SINGLE)] = "\u250e",
    [BOTTOM(HEAVY) + RIGHT(HEAVY)] = "\u250f",
    [BOTTOM(SINGLE) + LEFT(SINGLE)] = CURVE("\u2510", "\u256e"),
    [BOTTOM(SINGLE) + LEFT(HEAVY)] = "\u2511",
    [BOTTOM(HEAVY) + LEFT(SINGLE)] = "\u2512",
    [BOTTOM(HEAVY) + LEFT(HEAVY)] = "\u2513",
    [TOP(SINGLE) + RIGHT(SINGLE)] = CURVE("\u2514", "\u2570"),
    [TOP(SINGLE) + RIGHT(HEAVY)] = "\u2515",
    [TOP(HEAVY) + RIGHT(SINGLE)] = "\u2516",
    [TOP(HEAVY) + RIGHT(HEAVY)] = "\u2517",
    [TOP(SINGLE) + LEFT(SINGLE)] = CURVE("\u2518", "\u256f"),
    [TOP(SINGLE) + LEFT(HEAVY)] = "\u2519",
    [TOP(HEAVY) + LEFT(SINGLE)] = "\u251a",
    [TOP(HEAVY) + LEFT(HEAVY)] = "\u251b",
    [VERTICAL(SINGLE) + RIGHT(SINGLE)] = "\u251c",
    [VERTICAL(SINGLE) + RIGHT(HEAVY)] = "\u251d",
    [TOP(HEAVY) + BOTTOM(SINGLE) + RIGHT(SINGLE)] = "\u251e",
    [TOP(SINGLE) + BOTTOM(HEAVY) + RIGHT(SINGLE)] = "\u251f",
    [VERTICAL(HEAVY) + RIGHT(SINGLE)] = "\u2520",
    [TOP(HEAVY) + BOTTOM(SINGLE) + RIGHT(HEAVY)] = "\u2521",
    [TOP(SINGLE) + BOTTOM(HEAVY) + RIGHT(HEAVY)] = "\u2522",
    [VERTICAL(HEAVY) + RIGHT(HEAVY)] = "\u2523",
    [VERTICAL(SINGLE) + LEFT(SINGLE)] = "\u2524",
    [VERTICAL(SINGLE) + LEFT(HEAVY)] = "\u2525",
    [TOP(HEAVY) + BOTTOM(SINGLE) + LEFT(SINGLE)] = "\u2526",
    [TOP(SINGLE) + BOTTOM(HEAVY) + LEFT(SINGLE)] = "\u2527",
    [VERTICAL(HEAVY) + LEFT(SINGLE)] = "\u2528",
    [TOP(HEAVY) + BOTTOM(SINGLE) + LEFT(HEAVY)] = "\u2529",
    [TOP(SINGLE) + BOTTOM(HEAVY) + LEFT(HEAVY)] = "\u252a",
    [VERTICAL(HEAVY) + LEFT(HEAVY)] = "\u252b",
    [HORIZ(SINGLE) + BOTTOM(SINGLE)] = "\u252c",
    [HORIZ(SINGLE) + BOTTOM(HEAVY)] = "\u252d",
    [LEFT(HEAVY) + RIGHT(SINGLE) + BOTTOM(SINGLE)] = "\u252e",
    [LEFT(SINGLE) + RIGHT(HEAVY) + BOTTOM(SINGLE)] = "\u252f",
    [HORIZ(HEAVY) + BOTTOM(SINGLE)] = "\u2530",
    [LEFT(HEAVY) + RIGHT(SINGLE) + BOTTOM(HEAVY)] = "\u2531",
    [LEFT(SINGLE) + RIGHT(HEAVY) + BOTTOM(HEAVY)] = "\u2532",
    [HORIZ(HEAVY) + BOTTOM(HEAVY)] = "\u2533",
    [HORIZ(SINGLE) + TOP(SINGLE)] = "\u2534",
    [HORIZ(SINGLE) + TOP(HEAVY)] = "\u2535",
    [LEFT(HEAVY) + RIGHT(SINGLE) + TOP(SINGLE)] = "\u2536",
    [LEFT(SINGLE) + RIGHT(HEAVY) + TOP(SINGLE)] = "\u2537",
    [HORIZ(HEAVY) + TOP(SINGLE)] = "\u2538",
    [LEFT(HEAVY) + RIGHT(SINGLE) + TOP(HEAVY)] = "\u2539",
    [LEFT(SINGLE) + RIGHT(HEAVY) + TOP(HEAVY)] = "\u253a",
    [HORIZ(HEAVY) + TOP(HEAVY)] = "\u253b",
    [VERTICAL(SINGLE) + HORIZ(SINGLE)] = "\u253c",
    [VERTICAL(SINGLE) + LEFT(HEAVY) + RIGHT(SINGLE)] = "\u253d",
    [VERTICAL(SINGLE) + LEFT(SINGLE) + RIGHT(HEAVY)] = "\u253d",
    [VERTICAL(SINGLE) + HORIZ(HEAVY)] = "\u253f",
    [HORIZ(SINGLE) + TOP(HEAVY) + BOTTOM(SINGLE)] = "\u2540",
    [HORIZ(SINGLE) + TOP(SINGLE) + BOTTOM(HEAVY)] = "\u2541",
    [VERTICAL(HEAVY) + HORIZ(SINGLE)] = "\u2542",
    [TOP(HEAVY) + RIGHT(SINGLE) + BOTTOM(SINGLE) + LEFT(HEAVY)] = "\u2543",
    [TOP(HEAVY) + RIGHT(HEAVY) + BOTTOM(SINGLE) + LEFT(SINGLE)] = "\u2544",
    [TOP(SINGLE) + RIGHT(SINGLE) + BOTTOM(HEAVY) + LEFT(HEAVY)] = "\u2545",
    [TOP(SINGLE) + RIGHT(HEAVY) + BOTTOM(HEAVY) + LEFT(SINGLE)] = "\u2546",
    [HORIZ(HEAVY) + TOP(HEAVY) + BOTTOM(SINGLE)] = "\u2547",
    [HORIZ(HEAVY) + TOP(SINGLE) + BOTTOM(HEAVY)] = "\u2548",
    [VERTICAL(HEAVY) + LEFT(HEAVY) + RIGHT(SINGLE)] = "\u2549",
    [VERTICAL(HEAVY) + LEFT(SINGLE) + RIGHT(HEAVY)] = "\u254a",
    [VERTICAL(HEAVY) + HORIZ(HEAVY)] = "\u254b",

    [HORIZ(DOUBLE)] = "\u2550", [VERTICAL(DOUBLE)] = "\u2551",
    [BOTTOM(SINGLE) + RIGHT(DOUBLE)] = "\u2552",
    [BOTTOM(DOUBLE) + RIGHT(SINGLE)] = "\u2553",
    [BOTTOM(DOUBLE) + RIGHT(DOUBLE)] = "\u2554",
    [BOTTOM(SINGLE) + LEFT(DOUBLE)] = "\u2555",
    [BOTTOM(DOUBLE) + LEFT(SINGLE)] = "\u2556",
    [BOTTOM(DOUBLE) + LEFT(DOUBLE)] = "\u2557",
    [TOP(SINGLE) + RIGHT(DOUBLE)] = "\u2558",
    [TOP(DOUBLE) + RIGHT(SINGLE)] = "\u2559",
    [TOP(DOUBLE) + RIGHT(DOUBLE)] = "\u255a",
    [TOP(SINGLE) + LEFT(DOUBLE)] = "\u255b",
    [TOP(DOUBLE) + LEFT(SINGLE)] = "\u255c",
    [TOP(DOUBLE) + LEFT(DOUBLE)] = "\u255d",
    [VERTICAL(SINGLE) + RIGHT(DOUBLE)] = "\u255e",
    [VERTICAL(DOUBLE) + RIGHT(SINGLE)] = "\u255f",
    [VERTICAL(DOUBLE) + RIGHT(DOUBLE)] = "\u2560",
    [VERTICAL(SINGLE) + LEFT(DOUBLE)] = "\u2561",
    [VERTICAL(DOUBLE) + LEFT(SINGLE)] = "\u2562",
    [VERTICAL(DOUBLE) + LEFT(DOUBLE)] = "\u2563",
    [HORIZ(DOUBLE) + BOTTOM(SINGLE)] = "\u2564",
    [HORIZ(SINGLE) + BOTTOM(DOUBLE)] = "\u2565",
    [HORIZ(DOUBLE) + BOTTOM(DOUBLE)] = "\u2566",
    [HORIZ(DOUBLE) + TOP(SINGLE)] = "\u2567",
    [HORIZ(SINGLE) + TOP(DOUBLE)] = "\u2568",
    [HORIZ(DOUBLE) + TOP(DOUBLE)] = "\u2569",
    [VERTICAL(SINGLE) + HORIZ(DOUBLE)] = "\u256a",
    [VERTICAL(DOUBLE) + HORIZ(SINGLE)] = "\u256b",
    [VERTICAL(DOUBLE) + HORIZ(DOUBLE)] = "\u256c",

    [LEFT(SINGLE)] = "\u2574",
    [TOP(SINGLE)] = "\u2575",
    [RIGHT(SINGLE)] = "\u2576",
    [BOTTOM(SINGLE)] = "\u2577",
    [LEFT(HEAVY)] = "\u2578",
    [TOP(HEAVY)] = "\u2579",
    [RIGHT(HEAVY)] = "\u257a",
    [BOTTOM(HEAVY)] = "\u257b",
    [LEFT(SINGLE) + RIGHT(HEAVY)] = "\u257c",
    [TOP(SINGLE) + BOTTOM(HEAVY)] = "\u257d",
    [RIGHT(SINGLE) + LEFT(HEAVY)] = "\u257e",
    [BOTTOM(SINGLE) + TOP(HEAVY)] = "\u257f",
};

const char *linetype_name[] = { "blank", "single", "double", "heavy", 0 };

void fill_missing_fixed(const char *f) {
    assert(strlen(f) < 8);
    for (int ch = 0; ch < 256; ++ch) {
        if (linechars[ch][0]) continue;
        strcpy(linechars[ch], f);
    }
}

void fill_missing_trunc_double() {
    linechars[LEFT(DOUBLE)][0] = ' ';
    linechars[RIGHT(DOUBLE)][0] = ' ';
    linechars[TOP(DOUBLE)][0] = ' ';
    linechars[BOTTOM(DOUBLE)][0] = ' ';
}

void fill_missing_double2single() {
    for (int ch = 0; ch < 256; ++ch) {
        if (linechars[ch][0]) continue;
        int dmask = (~ch & 0x55) & ((ch >> 1) & 0x55);
        int alt = ch - dmask;
        assert(linechars[alt][0]);
        strcpy(linechars[ch], linechars[alt]);
    }
}

void fill_missing_double2heavy() {
    for (int ch = 0; ch < 256; ++ch) {
        if (linechars[ch][0]) continue;
        int alt = ch | ((ch & 0xaa) >> 1);
        assert(linechars[alt][0]);
        strcpy(linechars[ch], linechars[alt]);
    }
}

void fill_missing_double2mixed() {
    for (int ch = 0; ch < 256; ++ch) {
        if (linechars[ch][0]) continue;
        int dmask = (~ch & 0x55) & ((ch >> 1) & 0x55);
        int alt = ch;
        if ((ch & 0x55) & ((ch >> 1) & 0x55)) alt |= dmask;
        else if (ch & 0x55) alt -= dmask;
        else alt -= dmask << 1;
        assert(linechars[alt][0]);
        strcpy(linechars[ch], linechars[alt]);
    }
}

image_t *newimage(unsigned height, unsigned width) {
    image_t *rv = malloc(sizeof(image_t) + height * width);
    if (rv) {
        rv->height = height;
        rv->width = width;
        memset(rv->data, 0, height * width);
    }
    return rv;
}

image_t *dupimage(const image_t *src) {
    size_t size = sizeof(image_t) + src->height * src->width;
    image_t *rv = malloc(size);
    if (rv) memcpy(rv, src, size);
    return rv;
}

void clearimage(image_t *image) {
    memset(image->data, 0, image->height * image->width);
}

void copyimage(image_t *dest, const image_t *src) {
    if (dest->width == src->width) {
        if (dest->height <= src->height) {
            memcpy(dest->data, src->data, dest->width * dest->height);
        } else {
            memcpy(dest->data, src->data, dest->width * src->height);
            memset(dest->data + dest->width * src->height, 0,
                   (dest->height - src->height) * dest->width);
        }
    } else {
        unsigned char *d = dest->data;
        const unsigned char *s = src->data;
        unsigned lines = dest->height;
        if (src->height < lines) lines = src->height;
        for (;lines > 0; --lines, s += src->width, d += dest->width) {
            if (dest->width <= src->width) {
                memcpy(d, s, dest->width);
            } else {
                memcpy(d, s, src->width);
                memset(d + src->width, 0, dest->width - src->width);
            }
        }
        if (dest->height > src->height)
            memset(d, 0, (dest->height - src->height) * dest->width);
    }
}

void drawhline(image_t *image, linetype_t type, unsigned row, unsigned col1, unsigned col2) {
    if (col1 > col2) {
        unsigned tmp = col1;
        col1 = col2;
        col2 = tmp;
    }
    assert(row < image->height && col2 < image->width);
    unsigned char *pos = image->data + row * image->width + col1;
    while (col1 < col2) {
        *pos = (*pos & ~RIGHT(3)) | RIGHT(type);
        ++pos;
        *pos = (*pos & ~LEFT(3)) | LEFT(type);
        ++col1;
    }
}

void drawvline(image_t *image, linetype_t type, unsigned row1, unsigned row2, unsigned col) {
    if (row1 > row2) {
        unsigned tmp = row1;
        row1 = row2;
        row2 = tmp;
    }
    assert(row2 < image->height && col < image->width);
    unsigned char *pos = image->data + row1 * image->width + col;
    while (row1 < row2) {
        *pos = (*pos & ~BOTTOM(3)) | BOTTOM(type);
        pos += image->width;
        *pos = (*pos & ~TOP(3)) | TOP(type);
        ++row1;
    }
}

void drawbox(image_t *image, linetype_t type, unsigned top, unsigned left, unsigned bottom,
             unsigned right) {
    drawvline(image, type, top, bottom, left);
    drawvline(image, type, top, bottom, right);
    drawhline(image, type, top, left, right);
    drawhline(image, type, bottom, left, right);
}

void clearinsidebox(image_t *image, unsigned top, unsigned left, unsigned bottom, unsigned right) {
    if (top == bottom || left == right) return;
    if (top > bottom) {
        unsigned tmp = top;
        top = bottom;
        bottom = tmp;
    }
    if (left > right) {
        unsigned tmp = left;
        left = right;
        right = tmp;
    }
    unsigned char *p = image->data + top * image->width + left;
    unsigned width = right - left;
    for (unsigned i = 1; i < width; ++i) p[i] &= ~BOTTOM(3);
    p += image->width;
    for (unsigned j = top + 1; j < bottom; ++j, p += image->width) {
        p[0] &= ~RIGHT(3);
        for (unsigned i = 1; i < width; ++i) p[i] = 0;
        p[width] &= ~LEFT(3);
    }
    for (unsigned i = 1; i < width; ++i) p[i] &= ~TOP(3);
}

void draw_image(image_t *image, unsigned start, unsigned size, unsigned offset) {
    assert(GETX(offset) < image->width && GETY(offset) < image->height);
    int height = GETY(size), width = GETX(size);
    if (GETX(offset) + width > image->width) width = image->width - GETX(offset);
    if (GETY(offset) + height > image->height) height = image->height - GETY(offset);
    unsigned char *data = image->data + GETY(offset) * image->width + GETX(offset);
    for (int row = 0; row < height; ++row) {
        int scol = 0, ecol = width;
        while (scol < ecol && data[scol] == 0) ++scol;
        while (scol < ecol && data[ecol-1] == 0) --ecol;
        if (scol < ecol) {
            fputs(gotoyx(GETY(start) + row, GETX(start) + scol), stdout);
            for (int col = scol; col < ecol; ++col) {
                char *s = linechars[data[col]];
                fputs(*s ? s : "?", stdout);
            }
        }
        data += image->width;
    }
}

void update_image(image_t *old, image_t *image, unsigned start, unsigned size, unsigned offset) {
    assert(old->width == image->width && old->height == image->height);
    assert(GETX(offset) < image->width && GETY(offset) < image->height);
    int height = GETY(size), width = GETX(size);
    if (GETX(offset) + width > image->width) width = image->width - GETX(offset);
    if (GETY(offset) + height > image->height) height = image->height - GETY(offset);
    unsigned char *data = image->data + GETY(offset) * image->width + GETX(offset);

    unsigned char *odata = old->data + GETY(offset) * image->width + GETX(offset);
    for (int row = 0; row < height; ++row) {
        int scol = 0, ecol = width;
        while (scol < ecol && data[scol] == odata[scol]) ++scol;
        while (scol < ecol && data[ecol-1] == odata[ecol-1]) --ecol;
        if (scol < ecol) {
            fputs(gotoyx(GETY(start) + row, GETX(start) + scol), stdout);
            for (int col = scol; col < ecol; ++col) {
                char *s = linechars[data[col]];
                fputs(*s ? s : "?", stdout);
            }
        }
        data += image->width;
        odata += old->width;
    }
}

