#pragma once

#include "defines.h"

#define SNUK_DARRAY_DEFAULT_CAPACITY 5
#define SNUK_DARRAY_RESIZE_FACTOR 5

typedef enum SnukDArrayHeader {
    SNUK_DARRAY_CAPACITY,
    SNUK_DARRAY_SIZE,
    SNUK_DARRAY_STRIDE,
    SNUK_DARRAY_ALIGN,

    SNUK_DARRAY_MAX_FIELDS
} SnukDArrayHeader;

void *impl_snuk_darray_create(uint64_t capacity, uint64_t stride, uint64_t align);

void impl_snuk_darray_destroy(void *arr);

void impl_snuk_darray_resize(void **parr, uint64_t capacity);

uint64_t impl_snuk_darray_header(void *arr, SnukDArrayHeader header);

void impl_snuk_darray_push(void **parr, void *element);

void impl_snuk_darray_push_at(void **parr, uint64_t index, void *element);

void impl_snuk_darray_pop(void **parr, void *element);

void impl_snuk_darray_pop_at(void **parr, uint64_t index, void *element);

void impl_snuk_darray_clear(void **parr);

/**
 * @brief Create darray with given type and capacity.
 *
 * @param capacity The initial capacity of the array
 * @param type Type of the element
 *
 * @return The array or NULL on failure.
 */
#define snuk_darray_create_with_capacity(capacity, type) \
    (type *)impl_snuk_darray_create(capacity, sizeof(type), alignof(type))

/**
 * @brief Create a darray of given type.
 *
 * @param type Type of the elements of the array
 *
 * @return The array or NULL on failure.
 */
#define snuk_darray_create(type) \
    snuk_darray_create_with_capacity(SNUK_DARRAY_DEFAULT_CAPACITY, type)

/**
 * @brief Destroy the darray.
 *
 * @param parr The array
 */
#define snuk_darray_destroy(arr) impl_snuk_darray_destroy(arr)

/**
 * @brief Resize the darray to given capacity.
 *
 * @param parr Pointer to the array
 * @param capacity The new capacity (number of elements)
 */
#define snuk_darray_resize(parr, capacity) \
    impl_snuk_darray_resize((void **)parr, capacity)

/**
 * @brief Get the length of the darray (number of elements).
 *
 * @param arr The array
 *
 * @return The length of the array.
 */
#define snuk_darray_get_length(arr) \
    impl_snuk_darray_header(arr, SNUK_DARRAY_SIZE)

/**
 * @brief Get the current capacity of the darray.
 *
 * Returns the number of elements it can hold before resizing.
 *
 * @param arr The array
 *
 * @return The capacity of the array.
 */
#define snuk_darray_get_capacity(arr) \
    impl_snuk_darray_header(arr, SNUK_DARRAY_CAPACITY)

/**
 * @brief Clear the darray.
 *
 * Just sets the length of the array to zero. The elements should be considered
 * garbage values.
 *
 * @param parr Pointer to the array
 */
#define snuk_darray_clear(parr) impl_snuk_darray_clear((void **)parr)

/**
 * @brief Push the given element to the end of the array.
 *
 * @param parr Pointer to the array
 * @param element The element
 * @param res Result
 */
#define snuk_darray_push(parr, element) \
    impl_snuk_darray_push((void **)parr, (__typeof__(element)[]){element})

/**
 * @brief Pop the element from the end of the array.
 *
 * @param parr Pointer to the array
 * @param element Pointer to store poped element (can be NULL)
 */
#define snuk_darray_pop(parr, element) impl_snuk_darray_pop((void **)parr, element)

/**
 * @brief Push the element to given index of the array.
 *
 * @param parr Pointer to the array
 * @param index Index to insert the element to
 * @param element The element to insert
 */
#define snuk_darray_push_at(parr, index, element) \
    impl_snuk_darray_push_at((void **)parr, index, (__typeof__(element)[]){element})

/**
 * @brief Pop the element at given index of the array.
 *
 * @param parr Pointer to the array
 * @param index Index to pop the element
 * @param element Pointer to store element (can be NULL)
 */
#define snuk_darray_pop_at(parr, index, element) \
    impl_snuk_darray_pop_at((void **)parr, index, element)
