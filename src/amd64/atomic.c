#include "atomic.h"

// https://en.cppreference.com/w/c/atomic/memory_order
#ifdef SNUK_ARCH_AMD64

    #if defined(SNUK_COMPILER_MSVC)
    // Get the size and dispatch right atomic function for fast and least types.

        /**
         * @brief Define the atomic load function for given type.
         */
        #define DEFINE_ATOMIC_SFL_LOAD(type)                            \
            type GET_ATOMIC_FUNCTION(load, type)(                       \
                const volatile GET_ATOMIC_TYPE(type) * obj,             \
                snukMemoryOrder memory_order) {                         \
                switch (sizeof(type)) {                                 \
                    case 1:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, i8)(     \
                            (GET_ATOMIC_TYPE(i8) *)obj, memory_order);  \
                    case 2:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, i16)(    \
                            (GET_ATOMIC_TYPE(i16) *)obj, memory_order); \
                    case 4:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, i32)(    \
                            (GET_ATOMIC_TYPE(i32) *)obj, memory_order); \
                    case 8:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, i64)(    \
                            (GET_ATOMIC_TYPE(i64) *)obj, memory_order); \
                    default:                                            \
                        return (type)0;                                 \
                }                                                       \
            }

        /**
         * @brief Define the atomic load function for given type.
         */
        #define DEFINE_ATOMIC_UFL_LOAD(type)                            \
            type GET_ATOMIC_FUNCTION(load, type)(                       \
                const volatile GET_ATOMIC_TYPE(type) * obj,             \
                snukMemoryOrder memory_order) {                         \
                switch (sizeof(type)) {                                 \
                    case 1:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, u8)(     \
                            (GET_ATOMIC_TYPE(u8) *)obj, memory_order);  \
                    case 2:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, u16)(    \
                            (GET_ATOMIC_TYPE(u16) *)obj, memory_order); \
                    case 4:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, u32)(    \
                            (GET_ATOMIC_TYPE(u32) *)obj, memory_order); \
                    case 8:                                             \
                        return (type)GET_ATOMIC_FUNCTION(load, u64)(    \
                            (GET_ATOMIC_TYPE(u64) *)obj, memory_order); \
                    default:                                            \
                        return (type)0;                                 \
                }                                                       \
            }

        /**
         * @brief Define the atomic store function for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_STORE(type)                        \
            void GET_ATOMIC_FUNCTION(store, type)(                   \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,    \
                snukMemoryOrder memory_order) {                      \
                switch (sizeof(type)) {                              \
                    case 1:                                          \
                        GET_ATOMIC_FUNCTION(store, i8)(              \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8)value,   \
                            memory_order);                           \
                        break;                                       \
                    case 2:                                          \
                        GET_ATOMIC_FUNCTION(store, i16)(             \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16)value, \
                            memory_order);                           \
                        break;                                       \
                    case 4:                                          \
                        GET_ATOMIC_FUNCTION(store, i32)(             \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32)value, \
                            memory_order);                           \
                        break;                                       \
                    case 8:                                          \
                        GET_ATOMIC_FUNCTION(store, i64)(             \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64)value, \
                            memory_order);                           \
                        break;                                       \
                    default:                                         \
                        break;                                       \
                }                                                    \
            }

        /**
         * @brief Define the atomic store function for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_STORE(type)                        \
            void GET_ATOMIC_FUNCTION(store, type)(                   \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,    \
                snukMemoryOrder memory_order) {                      \
                switch (sizeof(type)) {                              \
                    case 1:                                          \
                        GET_ATOMIC_FUNCTION(store, u8)(              \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8)value,   \
                            memory_order);                           \
                        break;                                       \
                    case 2:                                          \
                        GET_ATOMIC_FUNCTION(store, u16)(             \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16)value, \
                            memory_order);                           \
                        break;                                       \
                    case 4:                                          \
                        GET_ATOMIC_FUNCTION(store, u32)(             \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32)value, \
                            memory_order);                           \
                        break;                                       \
                    case 8:                                          \
                        GET_ATOMIC_FUNCTION(store, u64)(             \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64)value, \
                            memory_order);                           \
                        break;                                       \
                    default:                                         \
                        break;                                       \
                }                                                    \
            }

        /**
         * @brief Define the atomic exchange function for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_EXCHANGE(type)                         \
            type GET_ATOMIC_FUNCTION(exchange, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,        \
                snukMemoryOrder memory_order) {                          \
                switch (sizeof(type)) {                                  \
                    case 1:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, i8)(  \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8)value,       \
                            memory_order);                               \
                    case 2:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, i16)( \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16)value,     \
                            memory_order);                               \
                    case 4:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, i32)( \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32)value,     \
                            memory_order);                               \
                    case 8:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, i64)( \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64)value,     \
                            memory_order);                               \
                    default:                                             \
                        return (type)0;                                  \
                }                                                        \
            }

        /**
         * @brief Define the atomic exchange function for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_EXCHANGE(type)                         \
            type GET_ATOMIC_FUNCTION(exchange, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,        \
                snukMemoryOrder memory_order) {                          \
                switch (sizeof(type)) {                                  \
                    case 1:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, u8)(  \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8)value,       \
                            memory_order);                               \
                    case 2:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, u16)( \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16)value,     \
                            memory_order);                               \
                    case 4:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, u32)( \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32)value,     \
                            memory_order);                               \
                    case 8:                                              \
                        return (type)GET_ATOMIC_FUNCTION(exchange, u64)( \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64)value,     \
                            memory_order);                               \
                    default:                                             \
                        return (type)0;                                  \
                }                                                        \
            }

        /**
         * @brief Define atomic compare exchange strong function for fast/least
         * type.
         */
        #define DEFINE_ATOMIC_SFL_COMPARE_EXCHANGE_STRONG(type)              \
            b8 GET_ATOMIC_FUNCTION(compare_exchange_strong, type)(           \
                volatile GET_ATOMIC_TYPE(type) * obj, type * expect,         \
                type value, snukMemoryOrder success, snukMemoryOrder fail) { \
                switch (sizeof(type)) {                                      \
                    case 1:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i8)(                    \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8 *)expect,        \
                            (i8)value, success, fail);                       \
                    case 2:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i16)(                   \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16 *)expect,      \
                            (i16)value, success, fail);                      \
                    case 4:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i32)(                   \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32 *)expect,      \
                            (i32)value, success, fail);                      \
                    case 8:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i64)(                   \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64 *)expect,      \
                            (i64)value, success, fail);                      \
                    default:                                                 \
                        return (type)0;                                      \
                }                                                            \
            }

        // NOTE: In amd64, there is no weak cas
        /**
         * @brief Define atomic compare exchange weak function for fast/least
         * type.
         */
        #define DEFINE_ATOMIC_SFL_COMPARE_EXCHANGE_WEAK(type)                \
            b8 GET_ATOMIC_FUNCTION(compare_exchange_weak, type)(             \
                volatile GET_ATOMIC_TYPE(type) * obj, type * expect,         \
                type value, snukMemoryOrder success, snukMemoryOrder fail) { \
                switch (sizeof(type)) {                                      \
                    case 1:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i8)(                    \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8 *)expect,        \
                            (i8)value, success, fail);                       \
                    case 2:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i16)(                   \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16 *)expect,      \
                            (i16)value, success, fail);                      \
                    case 4:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i32)(                   \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32 *)expect,      \
                            (i32)value, success, fail);                      \
                    case 8:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, i64)(                   \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64 *)expect,      \
                            (i64)value, success, fail);                      \
                    default:                                                 \
                        return (type)0;                                      \
                }                                                            \
            }

        /**
         * @brief Define atomic compare exchange strong function for fast/least
         * type.
         */
        #define DEFINE_ATOMIC_UFL_COMPARE_EXCHANGE_STRONG(type)              \
            b8 GET_ATOMIC_FUNCTION(compare_exchange_strong, type)(           \
                volatile GET_ATOMIC_TYPE(type) * obj, type * expect,         \
                type value, snukMemoryOrder success, snukMemoryOrder fail) { \
                switch (sizeof(type)) {                                      \
                    case 1:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u8)(                    \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8 *)expect,        \
                            (u8)value, success, fail);                       \
                    case 2:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u16)(                   \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16 *)expect,      \
                            (u16)value, success, fail);                      \
                    case 4:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u32)(                   \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32 *)expect,      \
                            (u32)value, success, fail);                      \
                    case 8:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u64)(                   \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64 *)expect,      \
                            (u64)value, success, fail);                      \
                    default:                                                 \
                        return (type)0;                                      \
                }                                                            \
            }

        // NOTE: In amd64, there is no weak cas
        /**
         * @brief Define atomic compare exchange weak function for fast/least
         * type.
         */
        #define DEFINE_ATOMIC_UFL_COMPARE_EXCHANGE_WEAK(type)                \
            b8 GET_ATOMIC_FUNCTION(compare_exchange_weak, type)(             \
                volatile GET_ATOMIC_TYPE(type) * obj, type * expect,         \
                type value, snukMemoryOrder success, snukMemoryOrder fail) { \
                switch (sizeof(type)) {                                      \
                    case 1:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u8)(                    \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8 *)expect,        \
                            (u8)value, success, fail);                       \
                    case 2:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u16)(                   \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16 *)expect,      \
                            (u16)value, success, fail);                      \
                    case 4:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u32)(                   \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32 *)expect,      \
                            (u32)value, success, fail);                      \
                    case 8:                                                  \
                        return (type)GET_ATOMIC_FUNCTION(                    \
                            compare_exchange_strong, u64)(                   \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64 *)expect,      \
                            (u64)value, success, fail);                      \
                    default:                                                 \
                        return (type)0;                                      \
                }                                                            \
            }

        /**
         * @brief Define the atomic fetch_add function for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_FETCH_ADD(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_add, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, i8)(  \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, i16)( \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, i32)( \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, i64)( \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define the atomic fetch_add function for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_FETCH_ADD(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_add, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, u8)(  \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, u16)( \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, u32)( \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_add, u64)( \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define the atomic fetch_sub function for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_FETCH_SUB(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_sub, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, i8)(  \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, i16)( \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, i32)( \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, i64)( \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define the atomic fetch_sub function for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_FETCH_SUB(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_sub, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, u8)(  \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, u16)( \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, u32)( \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_sub, u64)( \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define the atomic fetch_or function for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_FETCH_OR(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_or, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,        \
                snukMemoryOrder memory_order) {                          \
                switch (sizeof(type)) {                                  \
                    case 1:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, i8)(  \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8)value,       \
                            memory_order);                               \
                    case 2:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, i16)( \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16)value,     \
                            memory_order);                               \
                    case 4:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, i32)( \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32)value,     \
                            memory_order);                               \
                    case 8:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, i64)( \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64)value,     \
                            memory_order);                               \
                    default:                                             \
                        return (type)0;                                  \
                }                                                        \
            }

        /**
         * @brief Define the atomic fetch_or function for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_FETCH_OR(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_or, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,        \
                snukMemoryOrder memory_order) {                          \
                switch (sizeof(type)) {                                  \
                    case 1:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, u8)(  \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8)value,       \
                            memory_order);                               \
                    case 2:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, u16)( \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16)value,     \
                            memory_order);                               \
                    case 4:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, u32)( \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32)value,     \
                            memory_order);                               \
                    case 8:                                              \
                        return (type)GET_ATOMIC_FUNCTION(fetch_or, u64)( \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64)value,     \
                            memory_order);                               \
                    default:                                             \
                        return (type)0;                                  \
                }                                                        \
            }

        /**
         * @brief Define the atomic fetch_xor function for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_FETCH_XOR(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_xor, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, i8)(  \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, i16)( \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, i32)( \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, i64)( \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define the atomic fetch_xor function for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_FETCH_XOR(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_xor, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, u8)(  \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, u16)( \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, u32)( \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_xor, u64)( \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define the atomic fetch_and function for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_FETCH_AND(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_and, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, i8)(  \
                            (GET_ATOMIC_TYPE(i8) *)obj, (i8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, i16)( \
                            (GET_ATOMIC_TYPE(i16) *)obj, (i16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, i32)( \
                            (GET_ATOMIC_TYPE(i32) *)obj, (i32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, i64)( \
                            (GET_ATOMIC_TYPE(i64) *)obj, (i64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define the atomic fetch_and function for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_FETCH_AND(type)                         \
            type GET_ATOMIC_FUNCTION(fetch_and, type)(                    \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,         \
                snukMemoryOrder memory_order) {                           \
                switch (sizeof(type)) {                                   \
                    case 1:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, u8)(  \
                            (GET_ATOMIC_TYPE(u8) *)obj, (u8)value,        \
                            memory_order);                                \
                    case 2:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, u16)( \
                            (GET_ATOMIC_TYPE(u16) *)obj, (u16)value,      \
                            memory_order);                                \
                    case 4:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, u32)( \
                            (GET_ATOMIC_TYPE(u32) *)obj, (u32)value,      \
                            memory_order);                                \
                    case 8:                                               \
                        return (type)GET_ATOMIC_FUNCTION(fetch_and, u64)( \
                            (GET_ATOMIC_TYPE(u64) *)obj, (u64)value,      \
                            memory_order);                                \
                    default:                                              \
                        return (type)0;                                   \
                }                                                         \
            }

        /**
         * @brief Define all the atomic functions for fast/least type.
         */
        #define DEFINE_ATOMIC_SFL_FUNCTIONS(type)           \
            DEFINE_ATOMIC_SFL_LOAD(type)                    \
            DEFINE_ATOMIC_SFL_STORE(type)                   \
            DEFINE_ATOMIC_SFL_EXCHANGE(type)                \
            DEFINE_ATOMIC_SFL_COMPARE_EXCHANGE_STRONG(type) \
            DEFINE_ATOMIC_SFL_COMPARE_EXCHANGE_WEAK(type)   \
            DEFINE_ATOMIC_SFL_FETCH_ADD(type)               \
            DEFINE_ATOMIC_SFL_FETCH_SUB(type)               \
            DEFINE_ATOMIC_SFL_FETCH_OR(type)                \
            DEFINE_ATOMIC_SFL_FETCH_XOR(type)               \
            DEFINE_ATOMIC_SFL_FETCH_AND(type)

        /**
         * @brief Define all the atomic functions for fast/least type.
         */
        #define DEFINE_ATOMIC_UFL_FUNCTIONS(type)           \
            DEFINE_ATOMIC_UFL_LOAD(type)                    \
            DEFINE_ATOMIC_UFL_STORE(type)                   \
            DEFINE_ATOMIC_UFL_EXCHANGE(type)                \
            DEFINE_ATOMIC_UFL_COMPARE_EXCHANGE_STRONG(type) \
            DEFINE_ATOMIC_UFL_COMPARE_EXCHANGE_WEAK(type)   \
            DEFINE_ATOMIC_UFL_FETCH_ADD(type)               \
            DEFINE_ATOMIC_UFL_FETCH_SUB(type)               \
            DEFINE_ATOMIC_UFL_FETCH_OR(type)                \
            DEFINE_ATOMIC_UFL_FETCH_XOR(type)               \
            DEFINE_ATOMIC_UFL_FETCH_AND(type)

// NOTE: MSVC throws error if we nest the macro too much

DEFINE_ATOMIC_SFL_FUNCTIONS(if8);
DEFINE_ATOMIC_SFL_FUNCTIONS(if16);
DEFINE_ATOMIC_SFL_FUNCTIONS(if32);
DEFINE_ATOMIC_SFL_FUNCTIONS(if64);

DEFINE_ATOMIC_SFL_FUNCTIONS(il8);
DEFINE_ATOMIC_SFL_FUNCTIONS(il16);
DEFINE_ATOMIC_SFL_FUNCTIONS(il32);
DEFINE_ATOMIC_SFL_FUNCTIONS(il64);

DEFINE_ATOMIC_UFL_FUNCTIONS(uf8);
DEFINE_ATOMIC_UFL_FUNCTIONS(uf16);
DEFINE_ATOMIC_UFL_FUNCTIONS(uf32);
DEFINE_ATOMIC_UFL_FUNCTIONS(uf64);

DEFINE_ATOMIC_UFL_FUNCTIONS(ul8);
DEFINE_ATOMIC_UFL_FUNCTIONS(ul16);
DEFINE_ATOMIC_UFL_FUNCTIONS(ul32);
DEFINE_ATOMIC_UFL_FUNCTIONS(ul64);

    #else  // defined(SNUK_COMPILER_MSVC)

    // NOTE: LOCK prefix enforces full barrier!

        #define LFENCE __asm__ volatile("lfence" ::: "memory")
        #define SFENCE __asm__ volatile("sfence" ::: "memory")
        #define MFENCE __asm__ volatile("mfence" ::: "memory")

        /**
         * @brief Fences to be used before atomic load operation.
         */
        // Just ignoring if we get unsupported memroy order
        #define PRE_ATOMIC_LOAD_FENCE(memory_order)                        \
            do                                                             \
                if (memory_order == SNUK_MEMORY_ORDER_TOTAL_ORDER) MFENCE; \
            while (0)

        /**
         * @brief Fences to be used after atomic load operation.
         */
        // Just ignoring if we get unsupported memroy order
        #define POST_ATOMIC_LOAD_FENCE(memory_order)                  \
            do                                                        \
                if (memory_order == SNUK_MEMORY_ORDER_ACQUIRE         \
                    || memory_order == SNUK_MEMORY_ORDER_TOTAL_ORDER) \
                    LFENCE;                                           \
            while (0)

        /**
         * @brief Fences to be used after atomic store operation.
         */
        // Just ignoring if we get unsupported memroy order
        #define PRE_ATOMIC_STORE_FENCE(memory_order)                  \
            do                                                        \
                if (memory_order == SNUK_MEMORY_ORDER_RELEASE         \
                    || memory_order == SNUK_MEMORY_ORDER_TOTAL_ORDER) \
                    SFENCE;                                           \
            while (0)

        /**
         * @brief Fences to be used after atomic store operation.
         */
        // Just ignoring if we get unsupported memroy order
        #define POST_ATOMIC_STORE_FENCE(memory_order)                      \
            do                                                             \
                if (memory_order == SNUK_MEMORY_ORDER_TOTAL_ORDER) MFENCE; \
            while (0)

        /**
         * @brief Fences to be used after atomic Read Modify Wirte operation.
         */
        #define PRE_ATOMIC_RMW_FENCE(memory_order)                      \
            do                                                          \
                if (memory_order == SNUK_MEMORY_ORDER_RELEASE           \
                    || memory_order == SNUK_MEMORY_ORDER_ACQ_REL)       \
                    SFENCE;                                             \
                else if (memory_order == SNUK_MEMORY_ORDER_TOTAL_ORDER) \
                    MFENCE;                                             \
            while (0)

        /**
         * @brief Fences to be used after atomic Read Modify Wirte operation.
         */
        #define POST_ATOMIC_RMW_FENCE(memory_order)                     \
            do                                                          \
                if (memory_order == SNUK_MEMORY_ORDER_ACQUIRE           \
                    || memory_order == SNUK_MEMORY_ORDER_ACQ_REL)       \
                    LFENCE;                                             \
                else if (memory_order == SNUK_MEMORY_ORDER_TOTAL_ORDER) \
                    MFENCE;                                             \
            while (0)

        /**
         * @brief Define the atomic load function for given type.
         */
        #define DEFINE_ATOMIC_LOAD(type)                    \
            type GET_ATOMIC_FUNCTION(load, type)(           \
                const volatile GET_ATOMIC_TYPE(type) * obj, \
                snukMemoryOrder memory_order) {             \
                PRE_ATOMIC_LOAD_FENCE(memory_order);        \
                                                            \
                type value;                                 \
                __asm__ volatile("mov %[obj], %[value]"     \
                                 : [value] "=r"(value)      \
                                 : [obj] "rm"(obj->value)); \
                                                            \
                POST_ATOMIC_LOAD_FENCE(memory_order);       \
                                                            \
                return value;                               \
            }

        /**
         * @brief Define the atomic store function for given type.
         */
        #define DEFINE_ATOMIC_STORE(type)                         \
            void GET_ATOMIC_FUNCTION(store, type)(                \
                volatile GET_ATOMIC_TYPE(type) * obj, type value, \
                snukMemoryOrder memory_order) {                   \
                PRE_ATOMIC_STORE_FENCE(memory_order);             \
                                                                  \
                __asm__ volatile("mov %[value], %[obj]"           \
                                 : [obj] "=m"(obj->value)         \
                                 : [value] "ir"(value)            \
                                 : "memory");                     \
                                                                  \
                POST_ATOMIC_STORE_FENCE(memory_order);            \
            }

        /**
         * @brief Define the atomic exchange function for given type.
         */
        #define DEFINE_ATOMIC_EXCHANGE(type)                                   \
            type GET_ATOMIC_FUNCTION(exchange, type)(                          \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,              \
                snukMemoryOrder memory_order) {                                \
                SNUK_UNUSED(memory_order);                                     \
                                                                               \
                __asm__ volatile("lock xchg %[obj], %[value]"                  \
                                 : [obj] "+m"(obj->value), [value] "+r"(value) \
                                 :                                             \
                                 : "memory");                                  \
                return value; /* Now has old value */                          \
            }

        /**
         * @brief Define the atomic compare exchange strong function for given
         * type.
         */
        #define DEFINE_ATOMIC_COMPARE_EXCHANGE_STRONG(type)                   \
            b8 GET_ATOMIC_FUNCTION(compare_exchange_strong, type)(            \
                volatile GET_ATOMIC_TYPE(type) * obj, type * expect,          \
                type value, snukMemoryOrder success, snukMemoryOrder fail) {  \
                SNUK_UNUSED(success);                                         \
                SNUK_UNUSED(fail);                                            \
                                                                              \
                __asm__ goto("mov %[expect], %%rax\n\t"                       \
                             "lock cmpxchg %[value], %[obj]\n\t"              \
                             "mov %%rax, %[expect]\n\t"                       \
                             "jne %l[not_equal]"                              \
                             : [expect] "+m"(*expect), [obj] "+m"(obj->value) \
                             : [value] "ir"(value)                            \
                             : "rax", "cc", "memory"                          \
                             : not_equal);                                    \
                                                                              \
                return true;                                                  \
            not_equal:                                                        \
                return false;                                                 \
            }

    // NOTE: There is no weak cas in amd64
        /**
         * @brief Define the atomic compare exchange weak function for given
         * type.
         */
        #define DEFINE_ATOMIC_COMPARE_EXCHANGE_WEAK(type)                     \
            b8 GET_ATOMIC_FUNCTION(compare_exchange_weak, type)(              \
                volatile GET_ATOMIC_TYPE(type) * obj, type * expect,          \
                type value, snukMemoryOrder success, snukMemoryOrder fail) {  \
                SNUK_UNUSED(success);                                         \
                SNUK_UNUSED(fail);                                            \
                                                                              \
                __asm__ goto("mov %[expect], %%rax\n\t"                       \
                             "lock cmpxchg %[value], %[obj]\n\t"              \
                             "mov %%rax, %[expect]\n\t"                       \
                             "jne %l[not_equal]"                              \
                             : [expect] "+m"(*expect), [obj] "+m"(obj->value) \
                             : [value] "ir"(value)                            \
                             : "rax", "cc", "memory"                          \
                             : not_equal);                                    \
                                                                              \
                return true;                                                  \
            not_equal:                                                        \
                return false;                                                 \
            }

        /**
         * @brief Define the atomic fetch_add function for given type.
         */
        #define DEFINE_ATOMIC_FETCH_ADD(type)                                  \
            type GET_ATOMIC_FUNCTION(fetch_add, type)(                         \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,              \
                snukMemoryOrder memory_order) {                                \
                SNUK_UNUSED(memory_order);                                     \
                __asm__ volatile("lock xadd %[value], %[obj]"                  \
                                 : [value] "+r"(value), [obj] "+m"(obj->value) \
                                 :                                             \
                                 : "cc", "memory");                            \
                return value;                                                  \
            }

        /**
         * @brief Define the atomic fetch_sub function for given type.
         */
        #define DEFINE_ATOMIC_FETCH_SUB(type)                                  \
            type GET_ATOMIC_FUNCTION(fetch_sub, type)(                         \
                volatile GET_ATOMIC_TYPE(type) * obj, type value,              \
                snukMemoryOrder memory_order) {                                \
                SNUK_UNUSED(memory_order);                                     \
                /* Calling fetch_add won't work because of type casting */     \
                __asm__ volatile("neg %[value]\n\t"                            \
                                 "lock xadd %[value], %[obj]"                  \
                                 : [value] "+r"(value), [obj] "+m"(obj->value) \
                                 :                                             \
                                 : "cc", "memory");                            \
                return value;                                                  \
            }

        /**
         * @brief Do a RMW operation using CAS.
         */
        #define ATOMIC_OP_USING_CAS(type, obj, op, value, old_var)         \
            do {                                                           \
                type new_val;                                              \
                old_var = GET_ATOMIC_FUNCTION(load, type)(                 \
                    obj, SNUK_MEMORY_ORDER_NONE);                          \
                do new_val = old_var op value;                             \
                while (GET_ATOMIC_FUNCTION(compare_exchange_strong, type)( \
                    obj, &old_var, new_val, SNUK_MEMORY_ORDER_NONE,        \
                    SNUK_MEMORY_ORDER_NONE));                              \
            } while (0)

        /**
         * @brief Define the atomic fetch_or function for given type.
         */
        #define DEFINE_ATOMIC_FETCH_OR(type)                      \
            type GET_ATOMIC_FUNCTION(fetch_or, type)(             \
                volatile GET_ATOMIC_TYPE(type) * obj, type value, \
                snukMemoryOrder memory_order) {                   \
                PRE_ATOMIC_RMW_FENCE(memory_order);               \
                                                                  \
                type old;                                         \
                ATOMIC_OP_USING_CAS(type, obj, |, value, old);    \
                                                                  \
                POST_ATOMIC_RMW_FENCE(memory_order);              \
                                                                  \
                return old;                                       \
            }

        /**
         * @brief Define the atomic fetch_xor function for given type.
         */
        #define DEFINE_ATOMIC_FETCH_XOR(type)                     \
            type GET_ATOMIC_FUNCTION(fetch_xor, type)(            \
                volatile GET_ATOMIC_TYPE(type) * obj, type value, \
                snukMemoryOrder memory_order) {                   \
                PRE_ATOMIC_RMW_FENCE(memory_order);               \
                                                                  \
                type old;                                         \
                ATOMIC_OP_USING_CAS(type, obj, ^, value, old);    \
                                                                  \
                POST_ATOMIC_RMW_FENCE(memory_order);              \
                return old;                                       \
            }

        /**
         * @brief Define the atomic fetch_and function for given type.
         */
        #define DEFINE_ATOMIC_FETCH_AND(type)                     \
            type GET_ATOMIC_FUNCTION(fetch_and, type)(            \
                volatile GET_ATOMIC_TYPE(type) * obj, type value, \
                snukMemoryOrder memory_order) {                   \
                PRE_ATOMIC_RMW_FENCE(memory_order);               \
                                                                  \
                type old;                                         \
                ATOMIC_OP_USING_CAS(type, obj, &, value, old);    \
                                                                  \
                POST_ATOMIC_RMW_FENCE(memory_order);              \
                return old;                                       \
            }

/**
 * @brief Add a memory fence.
 */
void snuk_memory_fence(snukMemoryOrder fence) {
    switch (fence) {
        case SNUK_MEMORY_ORDER_NONE:
            // Nothing to do
            break;
        case SNUK_MEMORY_ORDER_ACQUIRE:
            LFENCE;
            break;
        case SNUK_MEMORY_ORDER_RELEASE:
            SFENCE;
            break;
        case SNUK_MEMORY_ORDER_ACQ_REL:
        case SNUK_MEMORY_ORDER_TOTAL_ORDER:
            MFENCE;
            break;
    }
}

/**
 * @brief Atomic flag test and set operation.
 */
b8 snuk_atomic_flag_test_and_set_explicit(volatile snuk_atomic_flag *obj,
                                          snukMemoryOrder memory_order) {
    // LOCK enforces full memory barrier
    SNUK_UNUSED(memory_order);

    b8 ret = true;
    __asm__ volatile("lock xchg %[flag], %[value]"
                     : [flag] "+m"(obj->flag), [value] "+r"(ret)
                     :
                     : "memory");
    return ret;
}

/**
 * @brief Atomic flag clear.
 */
void snuk_atomic_flag_clear_explicit(volatile snuk_atomic_flag *obj,
                                     snukMemoryOrder memory_order) {
    // As standard library allows any memory order, we do as well
    PRE_ATOMIC_RMW_FENCE(memory_order);

    b8 reset = false;
    __asm__ volatile("mov %[value], %[flag]"
                     : [flag] "=m"(obj->flag)
                     : [value] "ir"(reset)
                     : "memory");

    POST_ATOMIC_RMW_FENCE(memory_order);
}

/**
 * @brief Atomic flag load.
 */
b8 snuk_atomic_flag_load_explicit(volatile snuk_atomic_flag *obj,
                                  snukMemoryOrder memory_order) {
    PRE_ATOMIC_LOAD_FENCE(memory_order);

    b8 ret;
    __asm__ volatile("mov %[flag], %[value]\n\t"
                     : [value] "=r"(ret)
                     : [flag] "m"(obj->flag));

    POST_ATOMIC_LOAD_FENCE(memory_order);

    return ret;
}

        /**
         * @brief Define all the atomic functions for given type.
         */
        #define DEFINE_ATOMIC_FUNCTIONS(type)           \
            DEFINE_ATOMIC_LOAD(type)                    \
            DEFINE_ATOMIC_STORE(type)                   \
            DEFINE_ATOMIC_EXCHANGE(type)                \
            DEFINE_ATOMIC_COMPARE_EXCHANGE_STRONG(type) \
            DEFINE_ATOMIC_COMPARE_EXCHANGE_WEAK(type)   \
            DEFINE_ATOMIC_FETCH_ADD(type)               \
            DEFINE_ATOMIC_FETCH_SUB(type)               \
            DEFINE_ATOMIC_FETCH_OR(type)                \
            DEFINE_ATOMIC_FETCH_XOR(type)               \
            DEFINE_ATOMIC_FETCH_AND(type)

DEFINE_ATOMIC_FUNCTIONS(i8)
DEFINE_ATOMIC_FUNCTIONS(i16)
DEFINE_ATOMIC_FUNCTIONS(i32)
DEFINE_ATOMIC_FUNCTIONS(i64)

DEFINE_ATOMIC_FUNCTIONS(if8)
DEFINE_ATOMIC_FUNCTIONS(if16)
DEFINE_ATOMIC_FUNCTIONS(if32)
DEFINE_ATOMIC_FUNCTIONS(if64)

DEFINE_ATOMIC_FUNCTIONS(il8)
DEFINE_ATOMIC_FUNCTIONS(il16)
DEFINE_ATOMIC_FUNCTIONS(il32)
DEFINE_ATOMIC_FUNCTIONS(il64)

DEFINE_ATOMIC_FUNCTIONS(u8)
DEFINE_ATOMIC_FUNCTIONS(u16)
DEFINE_ATOMIC_FUNCTIONS(u32)
DEFINE_ATOMIC_FUNCTIONS(u64)

DEFINE_ATOMIC_FUNCTIONS(uf8)
DEFINE_ATOMIC_FUNCTIONS(uf16)
DEFINE_ATOMIC_FUNCTIONS(uf32)
DEFINE_ATOMIC_FUNCTIONS(uf64)

DEFINE_ATOMIC_FUNCTIONS(ul8)
DEFINE_ATOMIC_FUNCTIONS(ul16)
DEFINE_ATOMIC_FUNCTIONS(ul32)
DEFINE_ATOMIC_FUNCTIONS(ul64)
    #endif  // defined(SNUK_COMPILER_MSVC)

#endif  // SNUK_ARCH_AMD64
