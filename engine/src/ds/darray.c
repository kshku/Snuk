#include "darray.h"

#include "core/assertions.h"
#include "core/logger.h"
#include "core/memory.h"

#define HEADER_SIZE (DARRAY_HEADER_FIELDS_MAX * sizeof(u64))

/**
 * @brief Create the darray.
 *
 * @param capacity The initial capacity of array
 * @param stride The size of the each element
 *
 * @return Returns the pointer to the start of the array(excluding the header).
 */
void *_darrayCreate(const u64 capacity, const u64 stride) {
    u64 *ptr = (u64 *)sMalloc((capacity * stride) + HEADER_SIZE);

    if (!ptr) {
        sError("Failed to create darray");
        return ptr;
    }

    ptr[DARRAY_CAPACITY] = capacity;
    ptr[DARRAY_LENGTH] = 0;
    ptr[DARRAY_STRIDE] = stride;

    return (void *)(ptr + DARRAY_HEADER_FIELDS_MAX);
}

/**
 * @brief Destroy the darray.
 *
 * @param arr Pointer to the array.
 */
void _darrayDestroy(void *arr) {
    sFree(((u64 *)arr) - DARRAY_HEADER_FIELDS_MAX);
}

/**
 * @brief Resize the darray to given capacity.
 *
 * Similar to realloc, contents in the array will be unchanged in the range from
 * the start of the region up to the minimum of old and new sizes.
 *
 * @param arr Pointer to pointer to the array
 * @param capacity New capacity of the array
 *
 * @return Returns true if the resize was successfull, else false.
 */
b8 _darrayResize(void **arr, const u64 capacity) {
    u64 *ptr =
        (u64 *)sRealloc((((u64 *)(*arr)) - DARRAY_HEADER_FIELDS_MAX),
                        ((capacity * _darrayGetHeaderField(*arr, DARRAY_STRIDE))
                         + HEADER_SIZE));

    if (!ptr) {
        sError("Failed to resize the darray");
        return false;
    }

    ptr[DARRAY_CAPACITY] = capacity;
    *arr = (void *)(ptr + DARRAY_HEADER_FIELDS_MAX);

    return true;
}

/**
 * @brief Get the value in the darray header filed.
 *
 * @param arr Pointer to the array
 * @param field Which field's value to return
 *
 * @return Returns the value in the field.
 */
u64 _darrayGetHeaderField(void *arr, const DarrayHeaderField field) {
    return (((u64 *)arr) - DARRAY_HEADER_FIELDS_MAX)[field];
}

/**
 * @brief Set the value in the darray header field to given value.
 *
 * @param arr Pointer to the array
 * @param filed Which filed's value to be changed
 * @param value The value to which field is to be set
 */
void _darraySetHeaderField(void *arr, const DarrayHeaderField field,
                           const u64 value) {
    (((u64 *)arr) - DARRAY_HEADER_FIELDS_MAX)[field] = value;
}

/**
 * @brief Push an element to the end of darray.
 *
 * @param arr Pointer to pointer to the array
 * @param element The element to be pushed
 */
void _darrayPush(void **arr, void *element) {
    // TODO: How to handle resize error?
    u64 *ptr = (u64 *)(*arr) - DARRAY_HEADER_FIELDS_MAX;

    if (ptr[DARRAY_LENGTH] >= ptr[DARRAY_CAPACITY]) {
        sassert_msg(
            _darrayResize(arr, (ptr[DARRAY_CAPACITY] * DARRAY_RESIZE_FACTOR)),
            "Haven't handled _darrayResize error yet");
        // Ptr shold be updated too since the arr have been realloced
        ptr = (u64 *)(*arr) - DARRAY_HEADER_FIELDS_MAX;
    }

    sMemCopy(((*arr) + (ptr[DARRAY_STRIDE] * ptr[DARRAY_LENGTH])), element,
             ptr[DARRAY_STRIDE]);

    ++ptr[DARRAY_LENGTH];
}

/**
 * @brief Pop an element from the end of the darray.
 *
 * @param arr Pointer to the array
 * @param element If not null poped element will be stored in this
 */
void _darrayPop(void *arr, void *element) {
    u64 *ptr = (u64 *)arr - DARRAY_HEADER_FIELDS_MAX;

    --ptr[DARRAY_LENGTH];

    if (element)
        sMemCopy(element, (arr + (ptr[DARRAY_LENGTH] * ptr[DARRAY_STRIDE])),
                 ptr[DARRAY_STRIDE]);
}

/**
 * @brief Push an element to the darray at given index position.
 *
 * @param arr Pointer to pointer to the array
 * @param index Index position at which the element is to be pushed
 * @param element The element to be pushed
 */
void _darrayPushAt(void **arr, const u32 index, void *element) {
    // TODO: How to handle resize error?
    u64 *ptr = (u64 *)(*arr) - DARRAY_HEADER_FIELDS_MAX;

    if (index > ptr[DARRAY_LENGTH]) {
        sError("Darray index out of bound. Tried to access index %u but the "
               "length was %u",
               index, ptr[DARRAY_LENGTH]);
        return;
    }

    if (ptr[DARRAY_LENGTH] >= ptr[DARRAY_CAPACITY]) {
        sassert_msg(
            _darrayResize(arr, (ptr[DARRAY_CAPACITY] * DARRAY_RESIZE_FACTOR)),
            "Haven't handled _darrayResize error yet");
        // Ptr shold be updated too since the arr have been realloced
        ptr = (u64 *)(*arr) - DARRAY_HEADER_FIELDS_MAX;
    }

    sMemMove(((*arr) + (ptr[DARRAY_STRIDE] * (index + 1))),
             ((*arr) + (ptr[DARRAY_STRIDE] * index)),
             (ptr[DARRAY_STRIDE] * (ptr[DARRAY_LENGTH] - index)));

    sMemCopy(((*arr) + (ptr[DARRAY_STRIDE] * index)), element,
             ptr[DARRAY_STRIDE]);

    ++ptr[DARRAY_LENGTH];
}

/**
 * @brief Pop the element from the darray at given index position.
 *
 * @param arr Pointer to the array
 * @param index Index position at which the element is to be poped.
 * @param element If not null poped element will be stored in this
 *
 * @note If index was out of bound then an error message will be printed and if
 * parameter element was not null, then will be set to NULL.
 */
void _darrayPopAt(void *arr, const u32 index, void *element) {
    u64 *ptr = (u64 *)arr - DARRAY_HEADER_FIELDS_MAX;

    if (index >= ptr[DARRAY_LENGTH]) {
        if (element) element = NULL;
        sError("Darray index out of bound. Tried to access index %u but the "
               "length was %u",
               index, ptr[DARRAY_LENGTH]);
        return;
    }

    if (element)
        sMemCopy(element, (arr + (ptr[DARRAY_STRIDE] * index)),
                 ptr[DARRAY_STRIDE]);

    sMemMove((arr + (ptr[DARRAY_STRIDE] * index)),
             (arr + (ptr[DARRAY_STRIDE] * (index + 1))),
             (ptr[DARRAY_STRIDE] * (ptr[DARRAY_LENGTH] - index)));

    --ptr[DARRAY_LENGTH];
}
