#pragma once

#include "defines.h"

// TODO: There is not volatile in the parameters
// TODO: This is written to just get started

#define ATOMIC_PREFIX satomic

#define GET_ATOMIC_TYPE(type) CONCAT_EXPANDED3(ATOMIC_PREFIX, _, type)
#define GET_FUNCTION_NAME(name, type) \
    CONCAT_EXPANDED3(ATOMIC_PREFIX, name, type)

/**
 * @brief Define a new atomic type.
 *
 * @param type The type of the atomic vairable
 * @param name Name for the atomic type
 */
#define DEFINE_ATOMIC_TYPE(type)             \
    typedef struct GET_ATOMIC_TYPE(type) {   \
            volatile _Alignas(8) type value; \
    } GET_ATOMIC_TYPE(type)

#define DECLARE_ATOMIC_LOAD(type) \
    type GET_FUNCTION_NAME(Load, type)(GET_ATOMIC_TYPE(type) * obj)

#define DECLARE_ATOMIC_STORE(type) \
    void GET_FUNCTION_NAME(Store, type)(GET_ATOMIC_TYPE(type) * obj, type value)

#define DECLARE_ATOMIC_EXCHANGE(type)                                   \
    type GET_FUNCTION_NAME(Exchange, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type new_value)

#define DECLARE_ATOMIC_COMPARE_EXCHANGE(type)                                \
    b8 GET_FUNCTION_NAME(CompareExchange, type)(GET_ATOMIC_TYPE(type) * obj, \
                                                type * expect, type new)

#define DECLARE_ATOMIC_FETCH_ADD(type)                                  \
    type GET_FUNCTION_NAME(FetchAdd, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value)

#define DECLARE_ATOMIC_FETCH_SUB(type)                                  \
    type GET_FUNCTION_NAME(FetchSub, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value)

#define DECLARE_ATOMIC_FETCH_OR(type)                                  \
    type GET_FUNCTION_NAME(FetchOr, type)(GET_ATOMIC_TYPE(type) * obj, \
                                          type value)

#define DECLARE_ATOMIC_FETCH_XOR(type)                                  \
    type GET_FUNCTION_NAME(FetchXor, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value)

#define DECLARE_ATOMIC_FETCH_AND(type)                                  \
    type GET_FUNCTION_NAME(FetchAnd, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value)

#define CREATE_ATOMIC_TYPE(type)                \
    DEFINE_ATOMIC_TYPE(type);                   \
    SAPI DECLARE_ATOMIC_LOAD(type);             \
    SAPI DECLARE_ATOMIC_STORE(type);            \
    SAPI DECLARE_ATOMIC_EXCHANGE(type);         \
    SAPI DECLARE_ATOMIC_COMPARE_EXCHANGE(type); \
    SAPI DECLARE_ATOMIC_FETCH_ADD(type);        \
    SAPI DECLARE_ATOMIC_FETCH_SUB(type);        \
    SAPI DECLARE_ATOMIC_FETCH_OR(type);         \
    SAPI DECLARE_ATOMIC_FETCH_XOR(type);        \
    SAPI DECLARE_ATOMIC_FETCH_AND(type)

CREATE_ATOMIC_TYPE(i32);

#define SATOMIC_CREATE(value) {value}

#define SATOMIC_LOAD(obj) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(Load, i32))(obj)

#define SATOMIC_STORE(obj, value) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(Store, i32))(obj, value)

#define SATOMIC_EXCHANGE(obj, value) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(Exchange, i32))(obj, value)

#define SATOMIC_COMPARE_EXCHANGE(obj, expect, new)                           \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(CompareExchange, i32))( \
        obj, expect, new)

#define SATOMIC_FETCH_ADD(obj, value) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(FetchAdd, i32))(obj, value)

#define SATOMIC_FETCH_SUB(obj, value) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(FetchSub, i32))(obj, value)

#define SATOMIC_FETCH_OR(obj, value) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(FetchOr, i32))(obj, value)

#define SATOMIC_FETCH_XOR(obj, value) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(FetchXor, i32))(obj, value)

#define SATOMIC_FETCH_AND(obj, value) \
    _Generic((obj), satomic_i32 *: GET_FUNCTION_NAME(FetchAnd, i32))(obj, value)
