#include "snuk_value.h"

#include "io.h"
#include "snuk_scope.h"

SnukValue snuk_value_copy(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_FN:
            value.fn_value.closure = snuk_ref_counter_retain(value.fn_value.closure);
            break;
        case SNUK_VALUE_TYPE:
        case SNUK_VALUE_TYPE_INST:
            value.type_value.closure = snuk_ref_counter_retain(value.type_value.closure);
            break;

        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_INT:
        case SNUK_VALUE_FLOAT:
        case SNUK_VALUE_BOOL:
        case SNUK_VALUE_STRING:
        case SNUK_VALUE_NULL:
        case SNUK_VALUE_MAX:
        default:
            break;
    }

    return value;
}

void snuk_value_free(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_FN:
            snuk_ref_counter_release(&value.fn_value.closure);
            break;
        case SNUK_VALUE_TYPE:
        case SNUK_VALUE_TYPE_INST: {
            SnukScope *scope = GET_SCOPE(value.type_value.closure);
            uint64_t count = snuk_darray_get_length(scope->vars);
            uint64_t ignore_count = 0;
            for (uint64_t i = 0; i < count; ++i) {
                SnukEnv *env = scope->vars[i];
                switch (env->value.type) {
                    case SNUK_VALUE_FN:
                    case SNUK_VALUE_TYPE:
                        ignore_count++;
                        break;
                    default:
                        break;
                }
            }

            if (ignore_count + 1 == value.type_value.closure->ref_count)
                snuk_scope_destroy_envs(scope);

            snuk_ref_counter_release(&value.type_value.closure);
            break;
        }

        case SNUK_VALUE_UNKOWN:
        case SNUK_VALUE_INT:
        case SNUK_VALUE_FLOAT:
        case SNUK_VALUE_BOOL:
        case SNUK_VALUE_STRING:
        case SNUK_VALUE_NULL:
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
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}

void snuk_value_print(SnukValue value) {
    switch (value.type) {
        case SNUK_VALUE_UNKOWN:
            snuk_println("Something went wrong, value was UNKNOWN!");
            break;
        case SNUK_VALUE_INT:
            snuk_println("%ld", value.int_value);
            break;
        case SNUK_VALUE_FLOAT:
            snuk_println("%lf", value.float_value);
            break;
        case SNUK_VALUE_BOOL:
            snuk_println("%s", value.bool_value ? "true" : "false");
            break;
        case SNUK_VALUE_STRING:
            snuk_println(SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(value.string_value));
            break;
        case SNUK_VALUE_NULL:
            snuk_println("null", NULL);
            break;
        case SNUK_VALUE_FN:
            snuk_print("fn:", NULL);
            break;
        case SNUK_VALUE_TYPE:
            snuk_println("type:", NULL);
            break;
        case SNUK_VALUE_TYPE_INST:
            snuk_println("type inst:", NULL);
            break;
        default:
            SNUK_SHOULD_NOT_REACH_HERE;
            break;
    }
}
