#include "platform/atomic.h"

// NOTE: _Generic is compile time

#define DEFINE_ATOMIC_LOAD(type)                                                                          \
    type GET_FUNCTION_NAME(Load, type)(GET_ATOMIC_TYPE(type) * obj) {                                     \
        type value;                                                                                       \
        /* https://stackoverflow.com/questions/48046591/how-do-i-atomically-move-a-64bit-value-in-x86-asm \
         */                                                                                               \
        __asm__("mov %[obj], %[value]"                                                                    \
                : [value] "=r"(value)                                                                     \
                : [obj] "m"(*obj));                                                                       \
        return value;                                                                                     \
    }

#define DEFINE_ATOMIC_STORE(type)                                    \
    void GET_FUNCTION_NAME(Store, type)(GET_ATOMIC_TYPE(type) * obj, \
                                        type value) {                \
        __asm__ volatile("mov %[value], %[obj]"                      \
                         :                                           \
                         : [value] "r"(value), [obj] "m"(*obj)       \
                         : "memory");                                \
    }

#define DEFINE_ATOMIC_EXCHANGE(type)                                    \
    type GET_FUNCTION_NAME(Exchange, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type new_value) {            \
        __asm__("lock xchg %[new], %[old]"                              \
                : [new] "+r"(new_value), [old] "+m"(*obj)               \
                :                                                       \
                : "memory");                                            \
        return new_value;                                               \
    }

#define DEFINE_ATOMIC_COMPARE_EXCHANGE(type)                                  \
    b8 GET_FUNCTION_NAME(CompareExchange, type)(GET_ATOMIC_TYPE(type) * obj,  \
                                                type * expect, type new) {    \
        /* GET_ATOMIC_TYPE(type) new_val = SATOMIC_CREATE(new); */            \
        __asm__ goto("mov %[expect], %%rax\n\t"                               \
                     "lock cmpxchg %[new], %[obj]\n\t"                        \
                     "jne %l[not_equal]"                                      \
                     :                                                        \
                     : [new] "r"(new), [expect] "m"(*expect), [obj] "m"(*obj) \
                     : "rax", "cc", "memory"                                  \
                     : not_equal);                                            \
        return true;                                                          \
    not_equal:                                                                \
        return false;                                                         \
    }

#define DEFINE_ATOMIC_FETCH_ADD(type)                                   \
    type GET_FUNCTION_NAME(FetchAdd, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value) {                \
        __asm__("lock xadd %[value], %[obj]"                            \
                : [value] "+r"(value)                                   \
                : [obj] "m"(*obj)                                       \
                : "memory");                                            \
        return value;                                                   \
    }

#define DEFINE_ATOMIC_FETCH_SUB(type)                                   \
    type GET_FUNCTION_NAME(FetchSub, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value) {                \
        return SATOMIC_FETCH_ADD(obj, -value);                          \
    }

#define DEFINE_ATOMIC_FETCH_OR(type)                                   \
    type GET_FUNCTION_NAME(FetchOr, type)(GET_ATOMIC_TYPE(type) * obj, \
                                          type value) {                \
        type ret = SATOMIC_LOAD(obj);                                  \
        type new;                                                      \
        do {                                                           \
            new = ret | value;                                         \
        } while (!SATOMIC_COMPARE_EXCHANGE(obj, &ret, new));           \
        return ret;                                                    \
    }

#define DEFINE_ATOMIC_FETCH_XOR(type)                                   \
    type GET_FUNCTION_NAME(FetchXor, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value) {                \
        type ret = SATOMIC_LOAD(obj);                                   \
        type new;                                                       \
        do {                                                            \
            new = ret ^ value;                                          \
        } while (!SATOMIC_COMPARE_EXCHANGE(obj, &ret, new));            \
        return ret;                                                     \
    }

#define DEFINE_ATOMIC_FETCH_AND(type)                                   \
    type GET_FUNCTION_NAME(FetchAnd, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type value) {                \
        type ret = SATOMIC_LOAD(obj);                                   \
        type new;                                                       \
        do {                                                            \
            new = ret &value;                                           \
        } while (!SATOMIC_COMPARE_EXCHANGE(obj, &ret, new));            \
        return ret;                                                     \
    }

#define DEFINE_ATOMIC_FUNCTIONS(type)    \
    DEFINE_ATOMIC_LOAD(type)             \
    DEFINE_ATOMIC_STORE(type)            \
    DEFINE_ATOMIC_EXCHANGE(type)         \
    DEFINE_ATOMIC_COMPARE_EXCHANGE(type) \
    DEFINE_ATOMIC_FETCH_ADD(type)        \
    DEFINE_ATOMIC_FETCH_SUB(type)        \
    DEFINE_ATOMIC_FETCH_OR(type)         \
    DEFINE_ATOMIC_FETCH_XOR(type)        \
    DEFINE_ATOMIC_FETCH_AND(type)

DEFINE_ATOMIC_FUNCTIONS(i32);

DEFINE_ATOMIC_FUNCTIONS(b8);
