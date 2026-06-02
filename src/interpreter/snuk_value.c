#include "snuk/interpreter/snuk_value.h"

#include "snuk/interpreter/snuk_scope.h"
#include "snuk/io.h"

SnukValue snuk_value_copy(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_FN:
            if (value.fn_value.weak_ref)
                value.fn_value.closure = snuk_ref_counter_retain_weak(value.fn_value.closure);
            else value.fn_value.closure = snuk_ref_counter_retain(value.fn_value.closure);
            if (value.fn_value.instance)
                value.fn_value.instance = snuk_ref_counter_retain_weak(value.fn_value.instance);
            break;

        case SNUK_VALUE_FN_NATIVE:
            value.native_fn.closure = snuk_ref_counter_retain(value.native_fn.closure);
            if (value.native_fn.instance)
                value.native_fn.instance = snuk_ref_counter_retain_weak(value.native_fn.instance);

            break;

        case SNUK_VALUE_TYPE:
        case SNUK_VALUE_TYPE_INST:
            if (value.type_value.weak_ref)
                value.type_value.closure = snuk_ref_counter_retain_weak(value.type_value.closure);
            else value.type_value.closure = snuk_ref_counter_retain(value.type_value.closure);
            if (value.type_value.type_scope)
                value.type_value.type_scope = snuk_ref_counter_retain(value.type_value.type_scope);
            break;

        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_INT:
        case SNUK_VALUE_FLOAT:
        case SNUK_VALUE_BOOL:
        case SNUK_VALUE_STRING:
        case SNUK_VALUE_NULL:
        case SNUK_VALUE_INTERFACE:
        case SNUK_VALUE_MAX:
        default:
            break;
    }

    return value;
}

void snuk_value_free(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_FN:
            if (value.fn_value.weak_ref) snuk_ref_counter_release_weak(&value.fn_value.closure);
            else snuk_ref_counter_release(&value.fn_value.closure);
            if (value.fn_value.instance) snuk_ref_counter_release_weak(&value.fn_value.instance);
            break;

        case SNUK_VALUE_FN_NATIVE:
            snuk_ref_counter_release(&value.native_fn.closure);
            if (value.native_fn.instance) snuk_ref_counter_release_weak(&value.native_fn.instance);
            break;

        case SNUK_VALUE_TYPE:
        case SNUK_VALUE_TYPE_INST:
            if (value.type_value.weak_ref) snuk_ref_counter_release_weak(&value.type_value.closure);
            else snuk_ref_counter_release(&value.type_value.closure);
            if (value.type_value.type_scope) snuk_ref_counter_release(&value.type_value.type_scope);
            break;

        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_INT:
        case SNUK_VALUE_FLOAT:
        case SNUK_VALUE_BOOL:
        case SNUK_VALUE_STRING:
        case SNUK_VALUE_NULL:
        case SNUK_VALUE_INTERFACE:
        case SNUK_VALUE_MAX:
        default:
            break;
    }
}

void snuk_value_log(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_UNKOWN:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_UNKOWN));
            break;
        case SNUK_VALUE_INT:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_INT));
            log_trace("value: %ld", value.int_value);
            break;
        case SNUK_VALUE_FLOAT:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_FLOAT));
            log_trace("value: %lf", value.float_value);
            break;
        case SNUK_VALUE_BOOL:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_BOOL));
            log_trace("value: %s", value.bool_value ? "true" : "false");
            break;
        case SNUK_VALUE_STRING:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_STRING));
            log_trace("value: " SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(value.string_value));
            break;
        case SNUK_VALUE_NULL:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_NULL));
            break;
        case SNUK_VALUE_FN:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_FN));
            break;
        case SNUK_VALUE_TYPE:
            log_trace("type: %s", SNUK_STRINGIFY(SNUK_VALUE_TYPE));
            break;
        case SNUK_VALUE_TYPE_INST:
            log_trace("type %s", SNUK_STRINGIFY(SNUK_VALUE_TYPE_INST));
            break;
        case SNUK_VALUE_INTERFACE:
            log_trace("type %s", SNUK_STRINGIFY(SNUK_VALUE_INTERFACE));
            break;
        case SNUK_VALUE_ERROR:
            log_trace("error %s", SNUK_STRINGIFY(SNUK_VALUE_ERROR));
            log_trace("%s", value.err_msg);
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}
