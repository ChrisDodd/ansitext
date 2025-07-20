#ifndef _vector_h_
#define _vector_h_

/* C code and macros for VECTOR objects similar to C++ std::vector
 * a VECTOR is just a pointer to an array with a size and capacity stored
 * before the start of the array
 */
#include <stddef.h>

#define VECTOR(TYPE)            TYPE *
#define RAW(X)                  X

/* access the store vector size and capacity stored before the vector contents */
#define VECTOR_size(vec)        ((vec) ? ((unsigned *)(vec))[-1] : 0)
#define VECTOR_capacity(vec)    ((vec) ? ((unsigned *)(vec))[-2] : 0)

/* VECTOR constructors/destructor
 * can safely use memset(&vec, 0, sizeof(vec)) for initial capacity of 0,
 * so global and calloc'd VECTORs are safe to use immediately
 * local and malloc's VECTORs must be initialized before use, as they may
 * contain garbage */

/* VECTOR_init(vec, capacity)
 *   initialize an empty vector with optional initial capacity
 * VECTOR_initcopy(vec, from)
 *   initialize a vector as a copy of an existing vector
 * VECTOR_initN(vec, val1, ...)
 *   initialize a vector with N values
 * RETURNS
 *   0  success
 *   -1 failure (out of memory), vector has capacity 0
 */
#define VECTOR_init(vec, ...) \
    init_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), RAW(__VA_ARGS__+0))

#define VECTOR_initcopy(vec, from) \
    (init_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), VECTOR_size(from)) ? -1 : \
     (from) ? memcpy((unsigned *)(vec) - 1, (unsigned *)(from) - 1,                               \
         (((unsigned *)(vec))[-1] = VECTOR_size(from)) * sizeof(*vec) + sizeof(unsigned)), 0 : 0)

#define VECTOR_init1(vec, v1) \
    (init_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), 1) ? -1 : \
     (((unsigned *)(vec))[-1] = 1, (vec)[0] = (v1), 0))
#define VECTOR_init2(vec, v1, v2) \
    (init_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), 2) ? -1 : \
     (((unsigned *)(vec))[-1] = 2, (vec)[0] = (v1), (vec)[1] = (v2), 0))
#define VECTOR_init3(vec, v1, v2, v3) \
    (init_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), 3) ? -1 : \
     (((unsigned *)(vec))[-1] = 3, (vec)[0] = (v1), (vec)[1] = (v2), (vec)[2] = (v3), 0))
#define VECTOR_init4(vec, v1, v2, v3, v4) \
    (init_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), 4) ? -1 : \
     (((unsigned *)(vec))[-1] = 4, (vec)[0] = (v1), (vec)[1] = (v2),              \
      (vec)[2] = (v3), vec[3] = (v4), 0))
#define VECTOR_init5(vec, v1, v2, v3, v4, v5) \
    (init_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), 5) ? -1 : \
     (((unsigned *)(vec))[-1] = 5, (vec)[0] = (v1), (vec)[1] = (v2),              \
      (vec)[2] = (v3), vec[3] = (v4), (vec)[4] = (v5), 0))

/* VECTOR_fini(vec)
 *   destroys a vector, freeing memory
 * RETURNS
 *   void
 */
#define VECTOR_fini(vec)	free_raw_vector(vec, _Alignof(*(vec)))

/* VECTOR methods */

/* VECTOR_add(vec, val)
 *   add a single value to the end of a vector, increasing its size (and
 *   capacity if necessary)
 * VECTOR_addcopy(vec, ptr, n)
 *   add a multiple values to the end of a vector, increasing its size (and
 *   capacity as necessary)
 * VECTOR_copy(vec, from)
 *   replace a vector with a copy of another vector
 * RETURNS
 *   0  success
 *   -1 failure (out of memory), vector is unchanged
 */
#define VECTOR_add(vec, val) \
    (((!(vec) || ((unsigned *)(vec))[-1] == ((unsigned *)(vec))[-2]) && \
      expand_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)))) ? -1 : \
     ((vec)[((unsigned *)(vec))[-1]++] = (val), 0))
#define VECTOR_addcopy(vec, ptr, n) \
    (VECTOR_reserve(vec, VECTOR_size(vec) + (n)) ? -1 : ( \
     memcpy((vec) + ((unsigned *)(vec))[-1], (ptr), (n) * sizeof(*(vec))), \
     ((unsigned *)(vec))[-1] += (n), 0))
#define VECTOR_copy(vec, from) \
    (VECTOR_reserve(vec, VECTOR_size(from)) ? -1 : \
     memcpy((unsigned *)(vec) - 1, (unsigned *)(from) - 1, \
            VECTOR_size(from) * sizeof(*(vec)) + sizeof(unsigned)), 0)

#define VECTOR_begin(vec)	(vec)
#define VECTOR_end(vec)		((vec) ? (vec) + ((unsigned *)(vec))[-1] : (vec))
#define VECTOR_empty(vec)	(!(vec) || ((unsigned *)(vec))[-1] == 0)

/* VECTOR_erase(vec, idx, cnt)
 *   erase cnt elements from a vector (defaults to 1).  If there are fewer
 *   than cnt elements in the vector after idx (inclusive), all will be
 *   erased
 * RETURNS
 *   0  success
 *   -1 idx is out of range
 */
#define VECTOR_erase(vec, idx, ...) \
    erase_raw_vector((vec), sizeof(*(vec)), idx, RAW(__VA_ARGS__+0))

/* VECTOR_expand(vec)
 *   increase the capacity of a vector, if possible.  Does not affect the size
 * RETURNS
 *   0  success
 *   -1 failure (out of memory), vector is unchanged
 */
#define VECTOR_expand(vec) expand_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)))

/* VECTOR_insert(vec, idx, cnt)
 *   increase the size of a vector, adding uninitialized space at idx, and
 *   moving later elements of the vector up.  cnt defaults to 1
 * RETURNS
 *   0  success
 *   -1 failure -- idx is out of range[ERANGE], or out of memeory[ENOMEM]
 *                 vector is unchanged
 */
#define VECTOR_insert(vec, idx, ...) \
    insert_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), idx, RAW(__VA_ARGS__+0))

#define VECTOR_pop(vec)		((vec)[--((unsigned *)(vec))[-1]])
#define VECTOR_push(vec, val)	VECTOR_add(vec, val)

/* VECTOR_reserve(vec, size, shrink)
 *   change the capacity of a vector.  If shrink is false (default), will only
 *   increase the capacity.
 * RETURNS
 *   0  success
 *   -1 failure (out of memory), vector is unchanged
 */
#define VECTOR_reserve(vec, size, ...) \
    reserve_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)), size, RAW(__VA_ARGS__+0))

/* VECTOR_resize(vec, size, shrink)
 *   change the size of a vector.  If shrink is false (default), will only
 *   increase the capacity.
 * RETURNS
 *   0  success
 *   -1 failure (out of memory), vector is unchanged
 */
#define VECTOR_resize(vec, sz, ...) \
    (VECTOR_reserve(vec, sz, __VA_ARGS__) ? -1 : (((unsigned *)(vec))[-1] = (sz), 0))

/* VECTOR_shrink_to_fit(vec)
 *   reduce capacity to match the size, releasing memory if possible
 * RETURNS
 *   0  success
 *   -1 failure (realloc failed to shrink?), vector is unchanged
 */
#define VECTOR_shrink_to_fit(vec) \
    shrink_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)))

/* VECTOR_terminate(vec, val)
 *   ensure that capacity is greater than size, and store val after
 *   the end of the vector.
 * RETURNS
 *   0  success
 *   -1 failure (out of memory), vector is unchanged
 */
#define VECTOR_terminate(vec, val) \
    (((!(vec) || ((unsigned *)(vec))[-1] == ((unsigned *)(vec))[-2]) && \
      expand_raw_vector((void **)&(vec), sizeof(*(vec)), _Alignof(*(vec)))) ? -1 : \
     ((vec)[((unsigned *)(vec))[-1]] = (val), 0))
#define VECTOR_top(vec)		((vec)[((unsigned *)(vec))[-1]-1])

extern int erase_raw_vector(void *vec, size_t elsize, unsigned idx, unsigned cnt);
extern int expand_raw_vector(void **vec, size_t elsize, size_t align);
extern void free_raw_vector(void *vec, size_t align);
extern int init_raw_vector(void **vec, size_t elsize, size_t align, unsigned mincap);
extern int insert_raw_vector(void **vec, size_t elsize, size_t align, unsigned idx, unsigned cnt);
extern int reserve_raw_vector(void **vec, size_t elsize, size_t align, unsigned size, int shrink);
extern int shrink_raw_vector(void **vec, size_t elsize, size_t align);

#endif /* _vector_h_ */
