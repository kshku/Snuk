#include "darray.h"

#include "ds/darray.h"

u8 darray_create() {
    int *arr = darrayCreate(int);
    float *arr2 = darrayCreateWithSize(float, 20);
    if (darrayCapacity(arr) != 1) return FAIL;
    if (darrayCapacity(arr2) != 20) return FAIL;

    if (darrayLength(arr) != 0) return FAIL;
    if (darrayLength(arr2) != 0) return FAIL;

    if (_darrayGetHeaderField(arr, DARRAY_STRIDE) != sizeof(int)) return FAIL;
    if (_darrayGetHeaderField(arr2, DARRAY_STRIDE) != sizeof(float))
        return FAIL;

    return PASS;
}

Test *ds_darray_register_tests(Test *tests) {
    tests = testManagerRegister(tests, darray_create, "darray_create* failed");
    return tests;
}
