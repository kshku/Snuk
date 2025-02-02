#pragma once

#include "defines.h"

#ifndef DARRAY_DEFAULT_CAPACIY
    #define DARRAY_DEFAULT_CAPACITY 1
#endif

#ifndef DARRAY_RESIZE_FACTOR
    #define DARRAY_RESIZE_FACTOR 2
#endif

typedef enum DarrayHeaderField {
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_HEADER_FIELDS_MAX
} DarrayHeaderField;

SAPI void *darrayCreateImpl(const u64 capacity, const u64 stride);

SAPI void darrayDestroyImpl(void *arr);

SAPI b8 darrayResizeImpl(void **arr, const u64 capacity);

SAPI u64 darrayGetHeaderFieldImpl(void *arr, const DarrayHeaderField field);

SAPI void darraySetHeaderFieldImpl(void *arr, const DarrayHeaderField field,
                                   const u64 value);

SAPI void darrayPushImpl(void **arr, void *element);

SAPI void darrayPopImpl(void *arr, void *element);

SAPI void darrayPushAtImpl(void **arr, const u32 index, void *element);

SAPI void darrayPopAtImpl(void *arr, const u32 index, void *element);

/**
 * @brief Create a darray with given initial capacity.
 *
 * @param type Type of the elements
 * @param capacity The initial capacity of array
 *
 * @return Typecasted pointer to the array.
 */
#define darrayCreateWithSize(type, capacity) \
    (type *)darrayCreateImpl(capacity, sizeof(type))

/**
 * @brief Create a darray with default initial capacity.
 *
 * @param type Type of the elements
 *
 * @return Typecasted pointer to the array.
 */
#define darrayCreate(type) darrayCreateWithSize(type, DARRAY_DEFAULT_CAPACITY)

/**
 * @brief Destroy the darray.
 *
 * @param arr Pointer to the array
 */
#define darrayDestroy(arr) darrayDestroyImpl(arr)

/**
 * @brief Clear the array.
 *
 * The length will be set to zero and array will not be resized.
 *
 * @param arr Pointer to the array
 */
#define darrayClear(arr) darraySetHeaderFieldImpl(arr, DARRAY_LENGTH, 0)

/**
 * @brief Resize array to given capacity.
 *
 * Similar to realloc, contents in the array will be unchanged in the range from
 * the start of the region up to the minimum of old and new sizes.
 *
 * @param arr Pointer to the array
 * @param capacity The new capacity of array
 *
 * @return Returns true if resize was successfull, else false.
 */
#define darrayResize(arr, capacity) darrayResizeImpl(&arr, capacity)

/**
 * @brief Get number of elements in the array.
 *
 * @param arr Pointer to the array
 *
 * @return The length of the array.
 */
#define darrayLength(arr) darrayGetHeaderFieldImpl(arr, DARRAY_LENGTH)

/**
 * @brief Get the capacity of the array.
 *
 * @param arr Pointer to the array
 *
 * @return The size of the memroy allocation to the array in bytes.
 */
#define darrayCapacity(arr) darrayGetHeaderFieldImpl(arr, DARRAY_CAPACITY)

/**
 * @brief Get the stride of the array.
 *
 * @param arr Pointer to the array
 *
 * @return Returns the size of stride in bytes.
 */
#define darrayStride(arr) darrayGetHeaderFieldImpl(arr, DARRAY_STRIDE)

/**
 * @brief Push the element to end of the array.
 *
 * @param arr Pointer to the array
 * @param element The element to be pushed
 */
#define darrayPush(arr, element)                \
    do {                                        \
        __typeof__(element) temp = element;     \
        darrayPushImpl((void **)(&arr), &temp); \
    } while (0)

/**
 * @brief Pop the element from the end of the array.
 *
 * @param arr Pointer to the array
 * @param element If not null poped element will be stored in this
 */
#define darrayPop(arr, element) darrayPopImpl(arr, element)

/**
 * @brief Push the element to the given index of array.
 *
 * @param arr Pointer to the array
 * @param index Index position to insert the element
 * @param element The element to be pushed
 */
#define darrayPushAt(arr, index, element)                \
    do {                                                 \
        typeof(element) temp = element;                  \
        darrayPushAtImpl((void **)(&arr), index, &temp); \
    } while (0)

/**
 * @brief Pop the element at the given index of array.
 *
 * @param arr Pointer to the array
 * @param index Index position to pop the element
 * @param element If not null poped element will be stored in this
 */
#define darrayPopAt(arr, index, element) darrayPopAtImpl(arr, index, element)
