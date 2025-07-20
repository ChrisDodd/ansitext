#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "vector.h"

int init_raw_vector(void **vec, size_t elsize, size_t align, unsigned mincap)
{
    unsigned capacity = 32 / elsize;
    if (capacity < 4) capacity = 4;
    if (capacity < mincap) capacity = mincap;
    if (align < 2*sizeof(unsigned))
        align = 2*sizeof(unsigned);
    if (!(*vec = malloc(elsize * capacity + align)))
        return -1;
    *vec = (char *)*vec + align;
    ((unsigned *)*vec)[-2] = capacity;
    ((unsigned *)*vec)[-1] = 0;
    return 0;
}

void free_raw_vector(void *vec, size_t align) {
    if (align < 2*sizeof(unsigned))
        align = 2*sizeof(unsigned);
    if (vec) free((char *)vec - align);
}

int erase_raw_vector(void *vec, size_t elsize, unsigned i, unsigned cnt)
{
    unsigned *size = (unsigned *)vec - 1;
    if (!vec || i >= *size) return -1;
    if (cnt == 0) cnt = 1;
    if (i + cnt >= (unsigned)*size) {
	*size = i;
    } else {
	char *p = (char *)vec + i*elsize;
	memmove(p, p + elsize*cnt, elsize * (*size - i - cnt));
	*size -= cnt; }
    return 0;
}

int expand_raw_vector(void **vec, size_t elsize, size_t align)
{
    if (align < 2*sizeof(unsigned))
        align = 2*sizeof(unsigned);
    unsigned *cap = (unsigned *)*vec - 2;
    size_t ncap = *vec ? *cap * 2U : 0;
    if (ncap == 0) {
	ncap = 32 / elsize;
	if (ncap < 4) ncap = 4; }
    if (ncap > (size_t)UINT_MAX && (ncap = UINT_MAX) == *cap) {
	errno = ERANGE;
	return -1; }
    char *base = *vec ? (char *)*vec - align : 0;
    if (!(base = realloc(base, elsize * ncap + align))) return -1;
    cap = (*vec = base + align) - 2*sizeof(unsigned);
    *cap = ncap;
    return 0;
}

int insert_raw_vector(void **vec, size_t elsize, size_t align, unsigned i, unsigned cnt)
{
    if (align < 2*sizeof(unsigned))
        align = 2*sizeof(unsigned);
    unsigned *size = *vec ? (unsigned *)*vec - 1 : 0;
    if (i > (*vec ? *size : 0)) return -1;
    if (cnt == 0) cnt = 1;
    if (size && *size + cnt < cnt) {
	errno = ERANGE;
	return -1; }
    if (!*vec || *size + cnt > size[-1]) {
	unsigned newcap = (*vec ? *size : 0) + cnt;
	char *base = *vec ? (char *)*vec - align : 0;
        if (*vec) {
            if (newcap < size[-1] * 2) newcap = size[-1] * 2;
        } else if (newcap < 32/elsize || newcap < 4) {
            newcap = 32/elsize;
            if (newcap < 4) newcap = 4; }
	if (!(base = realloc(base, elsize * newcap + align))) return -1;
        size = (unsigned *)(base + align) - 1;
        size[-1] = newcap;
        if (!*vec) *size = 0;
        *vec = base + align; }
    if (i < *size) {
	char *p = (char *)*vec + i*elsize;
	memmove(p + cnt*elsize, p, elsize * (*size - i)); }
    *size += cnt;
    return 0;
}

int reserve_raw_vector(void **vec, size_t elsize, size_t align, unsigned size, int shrink)
{
    if (align < 2*sizeof(unsigned))
        align = 2*sizeof(unsigned);
    unsigned *cap = *vec ? (unsigned *)*vec - 2 : 0;
    char *base = *vec ? (char *)*vec - align : 0;
    if (!*vec || *cap < size || (shrink && *cap > size)) {
	if (!(base = realloc(base, elsize * size + align))) return -1;
        cap = (unsigned*)(base + align) - 2;
	*cap = size;
        if (!*vec)
            cap[1] = 0;
        else if (size < cap[1])
	    cap[1] = size;
        *vec = base + align;
            }
    return 0;
}

int shrink_raw_vector(void **vec, size_t elsize, size_t align)
{
    if (align < 2*sizeof(unsigned))
        align = 2*sizeof(unsigned);
    char *base = (char *)*vec - align;
    unsigned *size = (unsigned *)*vec - 1;
    if (*vec && *size < size[-1]) {
        if (*size == 0) {
            free(base);
            *vec = 0;
        } else if (!(base = realloc(base, elsize * *size + align))) return -1;
        size = (unsigned *)(*vec = base + align) - 1;
        size[-1] = *size; }
    return 0;
}
