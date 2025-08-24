#pragma once

#include "defines.h"

/**
 * @brief Memory order constants.
 *
 * @note The default memory order used by all the snuk_atomic_* functions is
 * SNUK_MEMORY_ORDER_NONE.
 */
typedef enum snukMemoryOrder {
    SNUK_MEMORY_ORDER_NONE, /**< No memory order */
    SNUK_MEMORY_ORDER_RELAXED =
        SNUK_MEMORY_ORDER_NONE, /**< Alias for SNUK_MEMORY_ORDER_NONE */
    SNUK_MEMORY_ORDER_ACQUIRE, /**< Acquire operation */
    SNUK_MEMORY_ORDER_RELEASE, /**< Release operation */
    SNUK_MEMORY_ORDER_ACQ_REL, /**< Both acquire and release operation */
    SNUK_MEMORY_ORDER_TOTAL_ORDER, /**< Total ordering */
    SNUK_MEMORY_ORDER_SEQ_CST =
        SNUK_MEMORY_ORDER_TOTAL_ORDER /**< Alias for
                                       * SNUK_MEMORY_ORDER_TOTAL_ORDER
                                       */
} snukMemoryOrder;

/**
 * @brief Add a memory fence(barrier).
 *
 * @param fence The type of the memory fence
 */
void snuk_memory_fence(snukMemoryOrder fence);

/**
 * @brief Prefix used by the engine for atomic types.
 */
#define ATOMIC_PREFIX snuk_atomic_

/**
 * @brief Get the atomic type using the prefix for give data type.
 *
 * @param type Type for which to get the atomic type
 */
#define GET_ATOMIC_TYPE(type) SNUK_CONCAT_EXPANDED(ATOMIC_PREFIX, type)

/**
 * @brief Get the atomic function for the given type and name.
 *
 * @param name The name like store, load, etc
 * @param type The type for which to get the funciton
 */
#define GET_ATOMIC_FUNCTION(name, type) \
    SNUK_CONCAT_EXPANDED3(ATOMIC_PREFIX, SNUK_CONCAT(name, _), type)

/**
 * @brief Atomic flags type.
 */
typedef struct snuk_atomic_flag {
        SNUK_STATIC_ASSERT(alignof(b8) <= 8, "Align of b8 is > 8 bytes!");
        alignas(alignof(b8)) volatile b8 flag;
} snuk_atomic_flag;

/**
 * @brief TAS operation on the snuk_atomic_flag with given memory order.
 *
 * Sets the flag and returns the previous value of flag.
 *
 * @param obj The atomic flag on which to perfrom TAS
 * @param memory_order The memory order to use
 *
 * @return Value of the flag before setting it.
 */
b8 snuk_atomic_flag_test_and_set_explicit(volatile snuk_atomic_flag *obj,
                                          snukMemoryOrder memory_order);

/**
 * @brief Clear(unset) the atomic flag with given memory order.
 *
 * @param obj The atomic flag to be cleared
 * @param memory_order The memory order to use
 */
void snuk_atomic_flag_clear_explicit(volatile snuk_atomic_flag *obj,
                                     snukMemoryOrder memory_order);

/**
 * @brief Load the value in the atomic flag with given memory order.
 *
 * Memory order must be one of SNUK_MEMORY_ORDER_NONE,
 * SNUK_MEMORY_ORDER_ACQUIRE, SNUK_MEMORY_ORDER_TOTAL_ORDER.
 *
 * @param obj The atomic flag object
 * @param memory_order The memory order to use
 *
 * @return Returns true if flag is set, else false.
 */
b8 snuk_atomic_flag_load_explicit(volatile snuk_atomic_flag *obj,
                                  snukMemoryOrder memory_order);

/**
 * @brief TAS with default memory order.
 * Refer snuk_atomic_flag_test_and_set_explicit
 */
#define snuk_atomic_flag_test_and_set(obj) \
    snuk_atomic_flag_test_and_set_explicit(obj, SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Clear the atomic flag with default memory order.
 * Refer snuk_atomic_flag_clear_explicit
 */
#define snuk_atomic_flag_clear(obj) \
    snuk_atomic_flag_clear_explicit(obj, SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Load the value in the atomic flag with default memory order.
 * Refer snuk_atomic_flag_load_explicit
 */
#define snuk_atomic_flag_load(obj) \
    snuk_atomic_flag_load_explicit(obj, SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Initialize the atomic flag variable.
 */
#define SNUK_ATOMIC_FLAG_INIT {.flag = false}

/**
 * @brief Define a atomic type for given type.
 *
 * @note The align(type) should be less than or equal to 8 bytes.
 */
#define DEFINE_ATOMIC_TYPE(type)                                     \
    typedef struct GET_ATOMIC_TYPE(type) {                           \
            SNUK_STATIC_ASSERT(                                      \
                alignof(type) <= 8,                                  \
                "Cannot create a atomic type for type with alignof " \
                "> 8 bytes!");                                       \
            alignas(alignof(type)) volatile type value;              \
    } GET_ATOMIC_TYPE(type)

/**
 * @brief Declare atomic load function for given type.
 */
#define DECLARE_ATOMIC_LOAD(type)                   \
    type GET_ATOMIC_FUNCTION(load, type)(           \
        const volatile GET_ATOMIC_TYPE(type) * obj, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare atomic store function for given type.
 */
#define DECLARE_ATOMIC_STORE(type)                        \
    void GET_ATOMIC_FUNCTION(store, type)(                \
        volatile GET_ATOMIC_TYPE(type) * obj, type value, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare atomic exchange function for given type.
 */
#define DECLARE_ATOMIC_EXCHANGE(type)                     \
    type GET_ATOMIC_FUNCTION(exchange, type)(             \
        volatile GET_ATOMIC_TYPE(type) * obj, type value, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare atomic compare exchange strong function for given type.
 */
#define DECLARE_ATOMIC_COMPARE_EXCHANGE_STRONG(type)                     \
    b8 GET_ATOMIC_FUNCTION(compare_exchange_strong, type)(               \
        volatile GET_ATOMIC_TYPE(type) * obj, type * expect, type value, \
        snukMemoryOrder success, snukMemoryOrder fail)

/**
 * @brief Declare atomic compare exchange weak function for given type.
 */
#define DECLARE_ATOMIC_COMPARE_EXCHANGE_WEAK(type)                       \
    b8 GET_ATOMIC_FUNCTION(compare_exchange_weak, type)(                 \
        volatile GET_ATOMIC_TYPE(type) * obj, type * expect, type value, \
        snukMemoryOrder success, snukMemoryOrder fail)

/**
 * @brief Declare atomic fetch_add function for given type.
 */
#define DECLARE_ATOMIC_FETCH_ADD(type)                    \
    type GET_ATOMIC_FUNCTION(fetch_add, type)(            \
        volatile GET_ATOMIC_TYPE(type) * obj, type value, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare atomic fetch_sub function for given type.
 */
#define DECLARE_ATOMIC_FETCH_SUB(type)                    \
    type GET_ATOMIC_FUNCTION(fetch_sub, type)(            \
        volatile GET_ATOMIC_TYPE(type) * obj, type value, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare atomic fetch_or function for given type.
 */
#define DECLARE_ATOMIC_FETCH_OR(type)                     \
    type GET_ATOMIC_FUNCTION(fetch_or, type)(             \
        volatile GET_ATOMIC_TYPE(type) * obj, type value, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare atomic fetch_xor function for given type.
 */
#define DECLARE_ATOMIC_FETCH_XOR(type)                    \
    type GET_ATOMIC_FUNCTION(fetch_xor, type)(            \
        volatile GET_ATOMIC_TYPE(type) * obj, type value, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare atomic fetch_and function for given type.
 */
#define DECLARE_ATOMIC_FETCH_AND(type)                    \
    type GET_ATOMIC_FUNCTION(fetch_and, type)(            \
        volatile GET_ATOMIC_TYPE(type) * obj, type value, \
        snukMemoryOrder memory_order)

/**
 * @brief Declare all the atomic functions for given type.
 */
#define DECLARE_ATOMIC_FUNCTIONS(type)            \
    DECLARE_ATOMIC_LOAD(type);                    \
    DECLARE_ATOMIC_STORE(type);                   \
    DECLARE_ATOMIC_EXCHANGE(type);                \
    DECLARE_ATOMIC_COMPARE_EXCHANGE_STRONG(type); \
    DECLARE_ATOMIC_COMPARE_EXCHANGE_WEAK(type);   \
    DECLARE_ATOMIC_FETCH_ADD(type);               \
    DECLARE_ATOMIC_FETCH_SUB(type);               \
    DECLARE_ATOMIC_FETCH_OR(type);                \
    DECLARE_ATOMIC_FETCH_XOR(type);               \
    DECLARE_ATOMIC_FETCH_AND(type)

/**
 * @brief Creates the atomic type with all function declarations.
 */
#define CREATE_ATOMIC_TYPE(type) \
    DEFINE_ATOMIC_TYPE(type);    \
    DECLARE_ATOMIC_FUNCTIONS(type)

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

/**
 * @brief Get the atomic function for given object type.
 */
#define GET_GENERIC_ATOMIC_FUNCTION(obj, name)                    \
    _Generic((obj),                                               \
        GET_ATOMIC_TYPE(i8) *: GET_ATOMIC_FUNCTION(name, i8),     \
        GET_ATOMIC_TYPE(i16) *: GET_ATOMIC_FUNCTION(name, i16),   \
        GET_ATOMIC_TYPE(i32) *: GET_ATOMIC_FUNCTION(name, i32),   \
        GET_ATOMIC_TYPE(i64) *: GET_ATOMIC_FUNCTION(name, i64),   \
                                                                  \
        GET_ATOMIC_TYPE(if8) *: GET_ATOMIC_FUNCTION(name, if8),   \
        GET_ATOMIC_TYPE(if16) *: GET_ATOMIC_FUNCTION(name, if16), \
        GET_ATOMIC_TYPE(if32) *: GET_ATOMIC_FUNCTION(name, if32), \
        GET_ATOMIC_TYPE(if64) *: GET_ATOMIC_FUNCTION(name, if64), \
                                                                  \
        GET_ATOMIC_TYPE(il8) *: GET_ATOMIC_FUNCTION(name, il8),   \
        GET_ATOMIC_TYPE(il16) *: GET_ATOMIC_FUNCTION(name, il16), \
        GET_ATOMIC_TYPE(il32) *: GET_ATOMIC_FUNCTION(name, il32), \
        GET_ATOMIC_TYPE(il64) *: GET_ATOMIC_FUNCTION(name, il64), \
                                                                  \
        GET_ATOMIC_TYPE(u8) *: GET_ATOMIC_FUNCTION(name, u8),     \
        GET_ATOMIC_TYPE(u16) *: GET_ATOMIC_FUNCTION(name, u16),   \
        GET_ATOMIC_TYPE(u32) *: GET_ATOMIC_FUNCTION(name, u32),   \
        GET_ATOMIC_TYPE(u64) *: GET_ATOMIC_FUNCTION(name, u64),   \
                                                                  \
        GET_ATOMIC_TYPE(uf8) *: GET_ATOMIC_FUNCTION(name, uf8),   \
        GET_ATOMIC_TYPE(uf16) *: GET_ATOMIC_FUNCTION(name, uf16), \
        GET_ATOMIC_TYPE(uf32) *: GET_ATOMIC_FUNCTION(name, uf32), \
        GET_ATOMIC_TYPE(uf64) *: GET_ATOMIC_FUNCTION(name, uf64), \
                                                                  \
        GET_ATOMIC_TYPE(ul8) *: GET_ATOMIC_FUNCTION(name, ul8),   \
        GET_ATOMIC_TYPE(ul16) *: GET_ATOMIC_FUNCTION(name, ul16), \
        GET_ATOMIC_TYPE(ul32) *: GET_ATOMIC_FUNCTION(name, ul32), \
        GET_ATOMIC_TYPE(ul64) *: GET_ATOMIC_FUNCTION(name, ul64), \
        default: NULL)

/**
 * @brief Initialize the newly created atomic variable.
 *
 * @param value The value for initializing
 */
#define SNUK_ATOMIC_VAR_INIT(val) {.value = val}

/**
 * @brief Atomic load operation.
 *
 * Memory order should be one of SNUK_MEMORY_ORDER_NONE,
 * SNUK_MEMORY_ORDER_ACQUIRE, SNUK_MEMORY_ORDER_TOTAL_ORDER.
 */
#define snuk_atomic_load_explicit(obj, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, load)(obj, memory_order)

/**
 * @brief Atomic store operation.
 *
 * Memory order should be one of SNUK_MEMORY_ORDER_NONE,
 * SNUK_MEMORY_ORDER_RELEASE, SNUK_MEMORY_ORDER_TOTAL_ORDER.
 */
#define snuk_atomic_store_explicit(obj, value, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, store)(obj, value, memory_order)

/**
 * @brief Atomic exchange operation.
 */
#define snuk_atomic_exchange_explicit(obj, value, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, exchange)(obj, value, memory_order)

/**
 * @brief Atomic comapre exchange strong operation.
 */
#define snuk_atomic_compare_exchange_strong_explicit(obj, expect, value, \
                                                     success, fail)      \
    GET_GENERIC_ATOMIC_FUNCTION(obj, compare_exchange_strong)(           \
        obj, expect, value, success, fail)

/**
 * @brief Atomic comapre exchange weak operation.
 */
#define snuk_atomic_compare_exchange_weak_explicit(obj, expect, value, \
                                                   success, fail)      \
    GET_GENERIC_ATOMIC_FUNCTION(obj, compare_exchange_weak)(           \
        obj, expect, value, success, fail)

/**
 * @brief Atomic fetch_add operation.
 */
#define snuk_atomic_fetch_add_explicit(obj, value, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_add)(obj, value, memory_order)

/**
 * @brief Atomic fetch_sub operation.
 */
#define snuk_atomic_fetch_sub_explicit(obj, value, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_sub)(obj, value, memory_order)

/**
 * @brief Atomic fetch_or operation.
 */
#define snuk_atomic_fetch_or_explicit(obj, value, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_or)(obj, value, memory_order)

/**
 * @brief Atomic fetch_xor operation.
 */
#define snuk_atomic_fetch_xor_explicit(obj, value, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_xor)(obj, value, memory_order)

/**
 * @brief Atomic fetch_and operation.
 */
#define snuk_atomic_fetch_and_explicit(obj, value, memory_order) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_and)(obj, value, memory_order)

/**
 * @brief Atomic load operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj Atomic object to load the value from
 *
 * @return Returns value stored in the atomic object.
 */
#define snuk_atomic_load(obj) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, load)(obj, SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic stroe operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj Atomic object to store the value
 * @param value The value to store
 */
#define snuk_atomic_store(obj, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, store)(obj, value, SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic exchange operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param value The value to exchange with
 *
 * @return Returns the previous value in the atomic object.
 */
#define snuk_atomic_exchange(obj, value)                   \
    GET_GENERIC_ATOMIC_FUNCTION(obj, exchange)(obj, value, \
                                               SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic compare exchange strong operation with default memory order.
 *
 * Exchanges the object's value with given value if the current value of object
 * is the expected value. If expected value is not what is stored in the object
 * then copies the current value of object to expect variable.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param expect The value expected in the object
 * @param value Value to exchange with
 *
 * @return Returns true if the value is exchanged else false.
 */
#define snuk_atomic_compare_exchange_strong(obj, expect, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, compare_exchange_strong)(  \
        obj, expect, value, SNUK_MEMORY_ORDER_NONE, SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic compare exchange weak operation with default memory order.
 *
 * Exchanges the object's value with given value if the current value of object
 * is the expected value. If expected value is not what is stored in the object
 * then copies the current value of object to expect variable.
 * Allowed spurious fail.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param expect The value expected in the object
 * @param value Value to exchange with
 *
 * @return Returns true if the value is exchanged else false.
 */
#define snuk_atomic_compare_exchange_weak(obj, expect, value) \
    GET_GENERIC_ATOMIC_FUNCTION(obj, compare_exchange_weak)(  \
        obj, expect, value, SNUK_MEMORY_ORDER_NONE, SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic fetch_add operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param value The value to add
 *
 * @return Previous value in the atomic object.
 */
#define snuk_atomic_fetch_add(obj, value)                   \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_add)(obj, value, \
                                                SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic fetch_sub operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param value The value to subtract
 *
 * @return Previous value of the atomic object.
 */
#define snuk_atomic_fetch_sub(obj, value)                   \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_sub)(obj, value, \
                                                SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic fetch_or operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param value The value to or with
 *
 * @return Returns the previous value of atomic object.
 */
#define snuk_atomic_fetch_or(obj, value)                   \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_or)(obj, value, \
                                               SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic fetch_xor operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param value The value to xor with
 *
 * @return Returns the previous value of atomic object.
 */
#define snuk_atomic_fetch_xor(obj, value)                   \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_xor)(obj, value, \
                                                SNUK_MEMORY_ORDER_NONE)

/**
 * @brief Atomic fetch_and operation with default memory order.
 *
 * @note Default memory order is SNUK_MEMORY_ORDER_NONE.
 *
 * @param obj The atomic object
 * @param value The value to and with
 *
 * @return Returns the previous value of atomic object.
 */
#define snuk_atomic_fetch_and(obj, value)                   \
    GET_GENERIC_ATOMIC_FUNCTION(obj, fetch_and)(obj, value, \
                                                SNUK_MEMORY_ORDER_NONE)

#undef DEFINE_ATOMIC_TYPE
#undef DECLARE_ATOMIC_LOAD
#undef DECLARE_ATOMIC_STORE
#undef DECLARE_ATOMIC_EXCHANGE
#undef DECLARE_ATOMIC_COMPARE_EXCHANGE_STRONG
#undef DECLARE_ATOMIC_COMPARE_EXCHANGE_WEAK
#undef DECLARE_ATOMIC_FETCH_ADD
#undef DECLARE_ATOMIC_FETCH_SUB
#undef DECLARE_ATOMIC_FETCH_OR
#undef DECLARE_ATOMIC_FETCH_XOR
#undef DECLARE_ATOMIC_FETCH_AND
#undef DECLARE_ATOMIC_FUNCTIONS
#undef CREATE_ATOMIC_TYPE
