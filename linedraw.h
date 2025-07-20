#ifndef _linedraw_h_
#define _linedraw_h_

/* screen coordinates go from 1,1 (for the top left corner), but image_t coords go
 * from 0,0.  So you need to either adjust by 1 everywhere or just skip row and
 * column 0 of the image */
typedef struct image_t {
    unsigned height, width;
    /* height * width bytes of data */
    unsigned char data[];
} image_t;

extern char linechars[256][8];

typedef enum linetype_t { NONE, SINGLE, DOUBLE, HEAVY } linetype_t;
void fill_missing_fixed(const char *f);
void fill_missing_trunc_double();
void fill_missing_double2single();
void fill_missing_double2heavy();
void fill_missing_double2mixed();

image_t *newimage(unsigned height, unsigned width);
image_t *dupimage(const image_t *src);
void copyimage(image_t *dest, const image_t *src);

void drawhline(image_t *image, linetype_t type, unsigned row, unsigned col1, unsigned col2);
void drawvline(image_t *image, linetype_t type, unsigned row1, unsigned row2, unsigned col);
void drawbox(image_t *image, linetype_t type, unsigned top, unsigned left, unsigned bottom,
             unsigned right);

void draw_image(image_t *image, unsigned start, unsigned size, unsigned offset);
void update_image(image_t *old, image_t *image, unsigned start, unsigned size, unsigned offset);

#endif  /* _linedraw_h_ */
