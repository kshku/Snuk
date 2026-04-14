#include "darray.h"

#include "memory.h"

#include <stdlib.h>
#include <string.h>

#define HEADER_SIZE (sizeof(uint64_t) * SNUK_DARRAY_MAX_FIELDS)

SNUK_INLINE void write_to_bytes(void *ptr, uint64_t value, bool reverse) {
    uint8_t *p = (uint8_t *)ptr;
    uint8_t inc = reverse ? -1 : 1;
    do {
        *p = value % 0x80;
        value >>= 7;
        if (!value) return;
        *p |= 0x80;
        p += inc;
    } while (value);
}

SNUK_INLINE uint64_t read_from_bytes(void *ptr, bool reverse) {
    uint8_t *p = (uint8_t *)ptr;
    uint8_t inc = reverse ? -1 : 1;
    uint64_t value = 0;
    uint64_t i = 0;

    while (*p & 0x80) {
        value |= (uint64_t)(*p & ~0x80) << i;
        i += 7;
        p += inc;
    }

    value |= (uint64_t)(*p) << i;

    return value;
}

#define ALIGN_BYTE(ptr) ((void *)((uint64_t)(ptr) - 1))
#define GET_ALIGN_SHIFT(ptr) (read_from_bytes((void *)((uint64_t)(ptr) - 1), true))
#define SET_ALIGN_SHIFT(ptr, shift) (write_to_bytes((void *)((uint64_t)(ptr) - 1), (shift), true))
#define GET_ALIGNED_NEXT(x, align) ((((uint64_t)(x)) + (align)) & ~((align) - 1))

void *impl_snuk_darray_create(uint64_t capacity, uint64_t stride, uint64_t align) {
    uint64_t total = (capacity * stride) + HEADER_SIZE + align;
    uint64_t *ptr = (uint64_t *)snuk_alloc(total, alignof(uint64_t));
    memset(ptr, 0, total);
    ptr[SNUK_DARRAY_CAPACITY] = capacity;
    ptr[SNUK_DARRAY_SIZE] = 0;
    ptr[SNUK_DARRAY_STRIDE] = stride;
    ptr[SNUK_DARRAY_ALIGN] = align;

    ptr = (uint64_t *)((uint64_t)ptr + HEADER_SIZE);

    uint64_t aligned = GET_ALIGNED_NEXT(ptr, align);
    SET_ALIGN_SHIFT(aligned, (aligned - (uint64_t)ptr));

    return (void *)aligned;
}

void impl_snuk_darray_destroy(void *arr) {
    uint64_t ptr = (uint64_t)arr;
    ptr -= GET_ALIGN_SHIFT(ptr);
    ptr -= HEADER_SIZE;
    snuk_free((void *)ptr);
}

void impl_snuk_darray_resize(void **parr, uint64_t capacity) {
    uint64_t ptr = (uint64_t)(*parr);
    uint64_t align_shift = GET_ALIGN_SHIFT(ptr);
    ptr -= align_shift;
    ptr -= HEADER_SIZE;

    uint64_t *p = (uint64_t *)ptr;

    p = (uint64_t *)snuk_realloc(p, (capacity * p[SNUK_DARRAY_STRIDE]) + HEADER_SIZE + p[SNUK_DARRAY_ALIGN], alignof(uint64_t));

    p[SNUK_DARRAY_CAPACITY] = capacity;

    ptr = (uint64_t)p;
    ptr += HEADER_SIZE;
    ptr += align_shift;

    *parr = (void *)ptr;
}

uint64_t impl_snuk_darray_header(void *arr, SnukDArrayHeader header) {
    uint64_t ptr = (uint64_t)arr;
    ptr -= GET_ALIGN_SHIFT(ptr);
    ptr -= HEADER_SIZE;
    return ((uint64_t *)ptr)[header];
}

void impl_snuk_darray_push(void **parr, void *element) {
    uint64_t ptr = (uint64_t)(*parr);
    uint64_t align_shift = GET_ALIGN_SHIFT(ptr);
    ptr -= align_shift;
    ptr -= HEADER_SIZE;
    uint64_t *p = (uint64_t *)ptr;

    if (p[SNUK_DARRAY_CAPACITY] <= p[SNUK_DARRAY_SIZE]) {
        snuk_darray_resize(parr, p[SNUK_DARRAY_CAPACITY] + SNUK_DARRAY_RESIZE_FACTOR);
        ptr = (uint64_t)(*parr);
        ptr -= align_shift;
        ptr -= HEADER_SIZE;
        p = (uint64_t *)ptr;
    }

    void *dest = ((uint8_t *)*parr) + (p[SNUK_DARRAY_SIZE] * p[SNUK_DARRAY_STRIDE]);
    memcpy(dest, element, p[SNUK_DARRAY_STRIDE]);
    ++p[SNUK_DARRAY_SIZE];
}

void impl_snuk_darray_push_at(void **parr, uint64_t index, void *element) {
    uint64_t ptr = (uint64_t)(*parr);
    uint64_t align_shift = GET_ALIGN_SHIFT(ptr);
    ptr -= align_shift;
    ptr -= HEADER_SIZE;
    uint64_t *p = (uint64_t *)ptr;

    if (index > p[SNUK_DARRAY_SIZE]) {
        if (p[SNUK_DARRAY_CAPACITY] <= index + 1) {
            snuk_darray_resize(parr, index + 1);
            ptr = (uint64_t)(*parr);
            ptr -= align_shift;
            ptr -= HEADER_SIZE;
            p = (uint64_t *)ptr;
        }

        memset(((uint8_t *)*parr) + (p[SNUK_DARRAY_SIZE] * p[SNUK_DARRAY_STRIDE]), 0,
                (index - p[SNUK_DARRAY_SIZE] * p[SNUK_DARRAY_STRIDE]));

        void *dest = ((uint8_t *)*parr) + (index * p[SNUK_DARRAY_STRIDE]);
        memcpy(dest, element, p[SNUK_DARRAY_STRIDE]);

        p[SNUK_DARRAY_SIZE] = index + 1;

        return;
    }

    if (p[SNUK_DARRAY_CAPACITY] <= p[SNUK_DARRAY_SIZE]) {
        snuk_darray_resize(parr, p[SNUK_DARRAY_CAPACITY] + SNUK_DARRAY_RESIZE_FACTOR);
        ptr = (uint64_t)(*parr);
        ptr -= align_shift;
        ptr -= HEADER_SIZE;
        p = (uint64_t *)ptr;
    }

    void *dest = (uint8_t *)(*parr) + (p[SNUK_DARRAY_STRIDE] * (index + 1));
    void *src = (uint8_t *)(*parr) + (p[SNUK_DARRAY_STRIDE] * (index));
    uint64_t size = p[SNUK_DARRAY_STRIDE] * (p[SNUK_DARRAY_SIZE] - index);
    memmove(dest, src, size);

    dest = (uint8_t *)(*parr) + (index * p[SNUK_DARRAY_STRIDE]);
    memcpy(dest, element, p[SNUK_DARRAY_STRIDE]);

    ++p[SNUK_DARRAY_SIZE];
}

void impl_snuk_darray_pop(void **parr, void *element) {
    uint64_t ptr = (uint64_t)(*parr);
    ptr -= GET_ALIGN_SHIFT(ptr);
    ptr -= HEADER_SIZE;
    uint64_t *p = (uint64_t *)ptr;

    if (p[SNUK_DARRAY_SIZE] == 0) return;

    --p[SNUK_DARRAY_SIZE];

    if (element) {
        void *src = ((uint8_t *)*parr) + (p[SNUK_DARRAY_SIZE] * p[SNUK_DARRAY_STRIDE]);
        memcpy(element, src, p[SNUK_DARRAY_STRIDE]);
    }

}

void impl_snuk_darray_pop_at(void **parr, uint64_t index, void *element) {
    uint64_t ptr = (uint64_t)(*parr);
    uint64_t align_shift = GET_ALIGN_SHIFT(ptr);
    ptr -= align_shift;
    ptr -= HEADER_SIZE;
    uint64_t *p = (uint64_t *)ptr;

    SNUK_ASSERT(index < p[SNUK_DARRAY_SIZE], "index out of bound while poping element");

    void *dest = ((uint8_t *)*parr) + (index * p[SNUK_DARRAY_STRIDE]);
    void *src = ((uint8_t *)*parr) + ((index + 1) * p[SNUK_DARRAY_STRIDE]);
    uint64_t size = (p[SNUK_DARRAY_SIZE] - index - 1) * p[SNUK_DARRAY_STRIDE];

    if (element) memcpy(element, dest, p[SNUK_DARRAY_STRIDE]);
    memmove(dest, src, size);

    --p[SNUK_DARRAY_SIZE];
}

void impl_snuk_darray_clear(void **parr) {
    uint64_t ptr = (uint64_t)(*parr);
    ptr -= GET_ALIGN_SHIFT(ptr);
    ptr -= HEADER_SIZE;
    uint64_t *p = (uint64_t *)ptr;
    p[SNUK_DARRAY_SIZE] = 0;
}

