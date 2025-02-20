#pragma once

#include "defines.h"

// TODO: There is not volatile in the parameters
// TODO: This is written to just get started

typedef enum sMemoryOrder {
    S_MEMORY_ORDER_NONE,
    S_MEMORY_ORDER_RELAXED = S_MEMORY_ORDER_NONE,
    S_MEMORY_ORDER_ACQUIRE,
    S_MEMORY_ORDER_RELEASE,
    S_MEMORY_ORDER_ACQ_REL,
    S_MEMORY_ORDER_TOTAL_ORDER,
    S_MEMORY_ORDER_SEQ_CST = S_MEMORY_ORDER_TOTAL_ORDER
} sMemoryOrder;

SAPI void smemoryFence(sMemoryOrder memory_order);

#define ATOMIC_PREFIX satomic

#define GET_ATOMIC_TYPE(type) CONCAT_EXPANDED3(ATOMIC_PREFIX, _, type)
#define GET_FUNCTION_NAME(name, type) \
    CONCAT_EXPANDED3(ATOMIC_PREFIX, name, type)

typedef struct satomic_flag {
        alignas(alignof(b8)) volatile b8 flag;
} satomic_flag;

/**
 * @brief Atomic test and set operation on atomic flag.
 *
 * @param flag The flag
 * @param memory_order The memory order to apply
 *
 * @return Returns the previous value of the flag.
 */
b8 satomic_flag_test_and_set(satomic_flag *flag, sMemoryOrder memory_order);

/**
 * @brief Clear the atomic flag.
 *
 * @param flag The flag to clear
 * @param memory_order The memory order to apply
 */
void satomic_flag_clear(satomic_flag *flag, sMemoryOrder memory_order);

/**
 * @brief Define a new atomic type.
 *
 * @param type The type of the atomic vairable
 * @param name Name for the atomic type
 */
#define DEFINE_ATOMIC_TYPE(type)                                               \
    typedef struct GET_ATOMIC_TYPE(type) {                                     \
            STATIC_ASSERT(alignof(type) <= 8,                                  \
                          "Cannot create a atomic type for type with alignof " \
                          "> 8 bytes!");                                       \
            alignas(alignof(type)) volatile type value;                        \
    } GET_ATOMIC_TYPE(type)

#define DECLARE_ATOMIC_LOAD(type)                                   \
    type GET_FUNCTION_NAME(Load, type)(GET_ATOMIC_TYPE(type) * obj, \
                                       sMemoryOrder memory_order)

#define DECLARE_ATOMIC_STORE(type)                                   \
    void GET_FUNCTION_NAME(Store, type)(GET_ATOMIC_TYPE(type) * obj, \
                                        type value, sMemoryOrder memory_order)

#define DECLARE_ATOMIC_EXCHANGE(type)                                   \
    type GET_FUNCTION_NAME(Exchange, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type new_value,              \
                                           sMemoryOrder memory_order)

#define DECLARE_ATOMIC_COMPARE_EXCHANGE(type)                                \
    b8 GET_FUNCTION_NAME(CompareExchange, type)(GET_ATOMIC_TYPE(type) * obj, \
                                                type * expect, type new,     \
                                                sMemoryOrder memory_order)

#define DECLARE_ATOMIC_FETCH_ADD(type)      \
    type GET_FUNCTION_NAME(FetchAdd, type)( \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order)

#define DECLARE_ATOMIC_FETCH_SUB(type)      \
    type GET_FUNCTION_NAME(FetchSub, type)( \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order)

#define DECLARE_ATOMIC_FETCH_OR(type)      \
    type GET_FUNCTION_NAME(FetchOr, type)( \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order)

#define DECLARE_ATOMIC_FETCH_XOR(type)      \
    type GET_FUNCTION_NAME(FetchXor, type)( \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order)

#define DECLARE_ATOMIC_FETCH_AND(type)      \
    type GET_FUNCTION_NAME(FetchAnd, type)( \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order)

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

CREATE_ATOMIC_TYPE(i8);
CREATE_ATOMIC_TYPE(i16);
CREATE_ATOMIC_TYPE(i32);
CREATE_ATOMIC_TYPE(i64);

CREATE_ATOMIC_TYPE(if8);
CREATE_ATOMIC_TYPE(if16);
CREATE_ATOMIC_TYPE(if32);
CREATE_ATOMIC_TYPE(if64);

CREATE_ATOMIC_TYPE(il8);
CREATE_ATOMIC_TYPE(il16);
CREATE_ATOMIC_TYPE(il32);
CREATE_ATOMIC_TYPE(il64);

CREATE_ATOMIC_TYPE(u8);
CREATE_ATOMIC_TYPE(u16);
CREATE_ATOMIC_TYPE(u32);
CREATE_ATOMIC_TYPE(u64);

CREATE_ATOMIC_TYPE(uf8);
CREATE_ATOMIC_TYPE(uf16);
CREATE_ATOMIC_TYPE(uf32);
CREATE_ATOMIC_TYPE(uf64);

CREATE_ATOMIC_TYPE(ul8);
CREATE_ATOMIC_TYPE(ul16);
CREATE_ATOMIC_TYPE(ul32);
CREATE_ATOMIC_TYPE(ul64);

#define GET_GENERIC_ATOMIC_FUNCTION(obj, name)                 \
    _Generic((obj),                                            \
        GET_ATOMIC_TYPE(i8) *: GET_FUNCTION_NAME(name, i8),    \
        GET_ATOMIC_TYPE(i16) *: GET_FUNCTION_NAME(name, i16),  \
        GET_ATOMIC_TYPE(i32) *: GET_FUNCTION_NAME(name, i32),  \
        GET_ATOMIC_TYPE(i64) *: GET_FUNCTION_NAME(name, i64),  \
                                                               \
        GET_ATOMIC_TYPE(if8) *: GET_FUNCTION_NAME(name, i8),   \
        GET_ATOMIC_TYPE(if16) *: GET_FUNCTION_NAME(name, i16), \
        GET_ATOMIC_TYPE(if32) *: GET_FUNCTION_NAME(name, i32), \
        GET_ATOMIC_TYPE(if64) *: GET_FUNCTION_NAME(name, i64), \
                                                               \
        GET_ATOMIC_TYPE(il8) *: GET_FUNCTION_NAME(name, i8),   \
        GET_ATOMIC_TYPE(il16) *: GET_FUNCTION_NAME(name, i16), \
        GET_ATOMIC_TYPE(il32) *: GET_FUNCTION_NAME(name, i32), \
        GET_ATOMIC_TYPE(il64) *: GET_FUNCTION_NAME(name, i64), \
                                                               \
        GET_ATOMIC_TYPE(u8) *: GET_FUNCTION_NAME(name, u8),    \
        GET_ATOMIC_TYPE(u16) *: GET_FUNCTION_NAME(name, u16),  \
        GET_ATOMIC_TYPE(u32) *: GET_FUNCTION_NAME(name, u32),  \
        GET_ATOMIC_TYPE(u64) *: GET_FUNCTION_NAME(name, u64),  \
                                                               \
        GET_ATOMIC_TYPE(uf8) *: GET_FUNCTION_NAME(name, u8),   \
        GET_ATOMIC_TYPE(uf16) *: GET_FUNCTION_NAME(name, u16), \
        GET_ATOMIC_TYPE(uf32) *: GET_FUNCTION_NAME(name, u32), \
        GET_ATOMIC_TYPE(uf64) *: GET_FUNCTION_NAME(name, u64), \
                                                               \
        GET_ATOMIC_TYPE(ul8) *: GET_FUNCTION_NAME(name, u8),   \
        GET_ATOMIC_TYPE(ul16) *: GET_FUNCTION_NAME(name, u16), \
        GET_ATOMIC_TYPE(ul32) *: GET_FUNCTION_NAME(name, u32), \
        GET_ATOMIC_TYPE(ul64) *: GET_FUNCTION_NAME(name, u64), \
        default: NULL)

#define SATOMIC_CREATE(value) {value}

#define SATOMIC_LOAD(obj) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, Load)(obj, S_MEMORY_ORDER_NONE)

#define SATOMIC_STORE(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, Store)(obj, value, S_MEMORY_ORDER_NONE)

#define SATOMIC_EXCHANGE(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, Exchange)(obj, value, S_MEMORY_ORDER_NONE)

#define SATOMIC_COMPARE_EXCHANGE(obj, expect, new)                      \
    GET_GENERIC_ATOMIC_FUNCTION(obj, CompareExchange)(obj, expect, new, \
                                                      S_MEMORY_ORDER_NONE)

#define SATOMIC_FETCH_ADD(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, FetchAdd)(obj, value, S_MEMORY_ORDER_NONE)

#define SATOMIC_FETCH_SUB(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, FetchSub)(obj, value, S_MEMORY_ORDER_NONE)

#define SATOMIC_FETCH_OR(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, FetchOr)(obj, value, S_MEMORY_ORDER_NONE)

#define SATOMIC_FETCH_XOR(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, FetchXor)(obj, value, S_MEMORY_ORDER_NONE)

#define SATOMIC_FETCH_AND(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, FetchAnd)(obj, value, S_MEMORY_ORDER_NONE)
