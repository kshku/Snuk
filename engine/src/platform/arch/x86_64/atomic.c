#include "platform/atomic.h"

#include "core/logger.h"

// NOTE: _Generic is compile time

#define LFENCE __asm__ volatile("lfence")
#define SFENCE __asm__ volatile("sfence")
#define MFENCE __asm__ volatile("mfence")

#define PRE_LOAD_FENCE(memory_order)                            \
    do {                                                        \
        if (memory_order == S_MEMORY_ORDER_TOTAL_ORDER) MFENCE; \
    } while (0)

#define POST_LOAD_FENCE(memory_order)                  \
    do {                                               \
        if (memory_order == S_MEMORY_ORDER_TOTAL_ORDER \
            || memory_order == S_MEMORY_ORDER_ACQUIRE) \
            LFENCE;                                    \
    } while (0)

#define PRE_STORE_FENCE(memory_order)                  \
    do {                                               \
        if (memory_order == S_MEMORY_ORDER_TOTAL_ORDER \
            || memory_order == S_MEMORY_ORDER_RELEASE) \
            LFENCE;                                    \
    } while (0)

#define POST_STORE_FENCE(memory_order)                          \
    do {                                                        \
        if (memory_order == S_MEMORY_ORDER_TOTAL_ORDER) MFENCE; \
    } while (0)

#define PRE_RMW_FENCE(memory_order)                                  \
    do {                                                             \
        if (memory_order == S_MEMORY_ORDER_RELEASE                   \
            || memory_order == S_MEMORY_ORDER_ACQ_REL)               \
            SFENCE;                                                  \
        else if (memory_order == S_MEMORY_ORDER_TOTAL_ORDER) MFENCE; \
    } while (0)

#define POST_RMW_FENCE(memory_order)                                 \
    do {                                                             \
        if (memory_order == S_MEMORY_ORDER_ACQUIRE                   \
            || memory_order == S_MEMORY_ORDER_ACQ_REL)               \
            LFENCE;                                                  \
        else if (memory_order == S_MEMORY_ORDER_TOTAL_ORDER) MFENCE; \
    } while (0)

#define DEFINE_ATOMIC_LOAD(type)                                                                          \
    type GET_FUNCTION_NAME(Load, type)(GET_ATOMIC_TYPE(type) * obj,                                       \
                                       sMemoryOrder memory_order) {                                       \
        PRE_LOAD_FENCE(memory_order);                                                                     \
                                                                                                          \
        type value;                                                                                       \
        /* https://stackoverflow.com/questions/48046591/how-do-i-atomically-move-a-64bit-value-in-x86-asm \
         */                                                                                               \
        __asm__("mov %[obj], %[value]"                                                                    \
                : [value] "=r"(value)                                                                     \
                : [obj] "m"(obj->value));                                                                 \
        return value;                                                                                     \
        POST_LOAD_FENCE(memory_order);                                                                    \
    }

#define DEFINE_ATOMIC_STORE(type)                                             \
    void GET_FUNCTION_NAME(Store, type)(                                      \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order) { \
        PRE_STORE_FENCE(memory_order);                                        \
                                                                              \
        __asm__ volatile("mov %[value], %[obj]"                               \
                         :                                                    \
                         : [value] "r"(value), [obj] "m"(obj->value)          \
                         : "memory");                                         \
                                                                              \
        POST_STORE_FENCE(memory_order);                                       \
    }

#define DEFINE_ATOMIC_EXCHANGE(type)                                    \
    type GET_FUNCTION_NAME(Exchange, type)(GET_ATOMIC_TYPE(type) * obj, \
                                           type new_value,              \
                                           sMemoryOrder memory_order) { \
        UNUSED(memory_order);                                           \
        __asm__("lock xchg %[new], %[old]"                              \
                : [new] "+r"(new_value), [old] "+m"(obj->value)         \
                :                                                       \
                : "memory");                                            \
        return new_value;                                               \
    }

#define DEFINE_ATOMIC_COMPARE_EXCHANGE(type)                                 \
    b8 GET_FUNCTION_NAME(CompareExchange, type)(GET_ATOMIC_TYPE(type) * obj, \
                                                type * expect, type new,     \
                                                sMemoryOrder memory_order) { \
        UNUSED(memory_order);                                                \
                                                                             \
        __asm__ goto(                                                        \
            "mov %[expect], %%rax\n\t"                                       \
            "lock cmpxchg %[new], %[obj]\n\t"                                \
            "jne %l[not_equal]"                                              \
            :                                                                \
            : [new] "r"(new), [expect] "m"(*expect), [obj] "m"(obj->value)   \
            : "rax", "cc", "memory"                                          \
            : not_equal);                                                    \
        return true;                                                         \
    not_equal:                                                               \
        return false;                                                        \
    }

#define DEFINE_ATOMIC_FETCH_ADD(type)                                         \
    type GET_FUNCTION_NAME(FetchAdd, type)(                                   \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order) { \
        UNUSED(memory_order);                                                 \
        __asm__("lock xadd %[value], %[obj]"                                  \
                : [value] "+r"(value)                                         \
                : [obj] "m"(obj->value)                                       \
                : "memory");                                                  \
        return value;                                                         \
    }

#define DEFINE_ATOMIC_FETCH_SUB(type)                                         \
    type GET_FUNCTION_NAME(FetchSub, type)(                                   \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order) { \
        return GET_FUNCTION_NAME(FetchAdd, type)(obj, -value, memory_order);  \
    }

#define DEFINE_ATOMIC_FETCH_OR(type)                                          \
    type GET_FUNCTION_NAME(FetchOr, type)(                                    \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order) { \
        PRE_RMW_FENCE(memory_order);                                          \
        type ret = GET_FUNCTION_NAME(Load, type)(obj, S_MEMORY_ORDER_NONE);   \
        type new;                                                             \
        do {                                                                  \
            new = ret | value;                                                \
        } while (!GET_FUNCTION_NAME(CompareExchange, type)(                   \
            obj, &ret, new, S_MEMORY_ORDER_NONE));                            \
        POST_RMW_FENCE(memory_order);                                         \
        return ret;                                                           \
    }

#define DEFINE_ATOMIC_FETCH_XOR(type)                                         \
    type GET_FUNCTION_NAME(FetchXor, type)(                                   \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order) { \
        PRE_RMW_FENCE(memory_order);                                          \
        type ret = GET_FUNCTION_NAME(Load, type)(obj, S_MEMORY_ORDER_NONE);   \
        type new;                                                             \
        do {                                                                  \
            new = ret ^ value;                                                \
        } while (!GET_FUNCTION_NAME(CompareExchange, type)(                   \
            obj, &ret, new, S_MEMORY_ORDER_NONE));                            \
        POST_RMW_FENCE(memory_order);                                         \
        return ret;                                                           \
    }

#define DEFINE_ATOMIC_FETCH_AND(type)                                         \
    type GET_FUNCTION_NAME(FetchAnd, type)(                                   \
        GET_ATOMIC_TYPE(type) * obj, type value, sMemoryOrder memory_order) { \
        PRE_RMW_FENCE(memory_order);                                          \
        type ret = GET_FUNCTION_NAME(Load, type)(obj, S_MEMORY_ORDER_NONE);   \
        type new;                                                             \
        do {                                                                  \
            new = ret &value;                                                 \
        } while (!GET_FUNCTION_NAME(CompareExchange, type)(                   \
            obj, &ret, new, S_MEMORY_ORDER_NONE));                            \
        POST_RMW_FENCE(memory_order);                                         \
        return ret;                                                           \
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

b8 satomic_flag_test_and_set(satomic_flag *flag, sMemoryOrder memory_order) {
    // Lock prefixed instruction enforce full barrier in x86_64
    UNUSED(memory_order);

    b8 set = true;
    __asm__ volatile("lock xchg %[flag], %[value]"
                     : [flag] "+m"(flag->flag), [value] "+r"(set)
                     :
                     : "memory");

    return set;
}

void satomic_flag_clear(satomic_flag *flag, sMemoryOrder memory_order) {
    PRE_STORE_FENCE(memory_order);

    __asm__ volatile("movb %[value], %[flag]"
                     : [flag] "+m"(flag->flag)
                     : [value] "i"(false)
                     : "memory");

    POST_STORE_FENCE(memory_order);
}

void smemoryFence(sMemoryOrder memory_order) {
    switch (memory_order) {
        case S_MEMORY_ORDER_NONE:
            // Do nothing
            break;
        case S_MEMORY_ORDER_ACQUIRE:
            LFENCE;
            break;
        case S_MEMORY_ORDER_RELEASE:
            SFENCE;
            break;
        case S_MEMORY_ORDER_ACQ_REL:
        case S_MEMORY_ORDER_TOTAL_ORDER:
            MFENCE;
            break;
        default:
            sFatal("Should not reach here, but still we are here!");
            break;
    }
}

DEFINE_ATOMIC_FUNCTIONS(i8);
DEFINE_ATOMIC_FUNCTIONS(i16);
DEFINE_ATOMIC_FUNCTIONS(i32);
DEFINE_ATOMIC_FUNCTIONS(i64);

DEFINE_ATOMIC_FUNCTIONS(if8);
DEFINE_ATOMIC_FUNCTIONS(if16);
DEFINE_ATOMIC_FUNCTIONS(if32);
DEFINE_ATOMIC_FUNCTIONS(if64);

DEFINE_ATOMIC_FUNCTIONS(il8);
DEFINE_ATOMIC_FUNCTIONS(il16);
DEFINE_ATOMIC_FUNCTIONS(il32);
DEFINE_ATOMIC_FUNCTIONS(il64);

DEFINE_ATOMIC_FUNCTIONS(u8);
DEFINE_ATOMIC_FUNCTIONS(u16);
DEFINE_ATOMIC_FUNCTIONS(u32);
DEFINE_ATOMIC_FUNCTIONS(u64);

DEFINE_ATOMIC_FUNCTIONS(uf8);
DEFINE_ATOMIC_FUNCTIONS(uf16);
DEFINE_ATOMIC_FUNCTIONS(uf32);
DEFINE_ATOMIC_FUNCTIONS(uf64);

DEFINE_ATOMIC_FUNCTIONS(ul8);
DEFINE_ATOMIC_FUNCTIONS(ul16);
DEFINE_ATOMIC_FUNCTIONS(ul32);
DEFINE_ATOMIC_FUNCTIONS(ul64);
