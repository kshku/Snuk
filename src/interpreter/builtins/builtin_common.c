#include "builtin_common.h"

SnukType to_int_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &int_type,
    },
};

SnukType to_float_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &float_type,
    },
};

SnukType to_bool_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &bool_type,
    },
};

SnukType to_str_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &str_type,
    },
};

SnukType str_length_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &int_type,
    },
};
SnukType str_get_type = {
    .type = TYPE_FN,
    .fn = {
        .return_type = &str_type,
    },
};

void snuk_builtins_init(SnukInterpreter *intpret) {
    if (!to_int_type.fn.param_types)
        to_int_type.fn.param_types = snuk_darray_create_with_capacity(0, SnukType *, &intpret->allocator);
    if (!to_float_type.fn.param_types)
        to_float_type.fn.param_types = snuk_darray_create_with_capacity(0, SnukType *, &intpret->allocator);
    if (!to_bool_type.fn.param_types)
        to_bool_type.fn.param_types = snuk_darray_create_with_capacity(0, SnukType *, &intpret->allocator);
    if (!to_str_type.fn.param_types)
        to_str_type.fn.param_types = snuk_darray_create_with_capacity(0, SnukType *, &intpret->allocator);

    if (!str_length_type.fn.param_types)
        str_length_type.fn.param_types = snuk_darray_create_with_capacity(0, SnukType *, &intpret->allocator);
    if (!str_get_type.fn.param_types) {
        str_get_type.fn.param_types = snuk_darray_create_with_capacity(2, SnukType *, &intpret->allocator);
        snuk_darray_push(&str_get_type.fn.param_types, &int_type);
        snuk_darray_push(&str_get_type.fn.param_types, &int_type);
    }
}

void snuk_builtins_deinit(SnukInterpreter *intpret) {
    SNUK_UNUSED(intpret);
}

bool snuk_builtins_create_builtin_types(SnukInterpreter *intpret, bool weak_ref) {
    for (uint64_t i = 0; i < SNUK_ARRAY_LENGTH(builtin_types); ++i) {
        SnukValue type = snuk_builtins_create_type(intpret, builtin_types[i].val_type, weak_ref);
        if (type.type == SNUK_VALUE_UNKOWN) return false;
        if (!snuk_native_add_value(intpret, builtin_types[i].type, &type_type, type, false))
            return false;
        snuk_value_free(type);
    }

    return true;
}

SnukValue builtin_int_create_type(SnukInterpreter *intpret, bool weak_ref);
SnukValue builtin_float_create_type(SnukInterpreter *intpret, bool weak_ref);
SnukValue builtin_bool_create_type(SnukInterpreter *intpret, bool weak_ref);
SnukValue builtin_str_create_type(SnukInterpreter *intpret, bool weak_ref);

SnukValue snuk_builtins_create_type(SnukInterpreter *intpret, SnukValueType type, bool weak_ref) {
    switch (type) {
        case SNUK_VALUE_INT:
            return builtin_int_create_type(intpret, weak_ref);
        case SNUK_VALUE_FLOAT:
            return builtin_float_create_type(intpret, weak_ref);
        case SNUK_VALUE_BOOL:
            return builtin_bool_create_type(intpret, weak_ref);
        case SNUK_VALUE_STRING:
            return builtin_str_create_type(intpret, weak_ref);

        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}
