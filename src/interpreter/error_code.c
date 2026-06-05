#include "snuk/interpreter/error_code.h"

static const char *error_messages[] = {
    [SNUK_ERROR_NONE] = "All is well",
    [SNUK_ERROR_SHOULD_NOT_REACH_HERE] = "Shouldn't reach here",
    [SNUK_ERROR_SOMETHING_WENT_WRONG] = "Something went wrong",
    [SNUK_ERROR_CONTROL_FLOW] = "control flow item outside scope",
    [SNUK_ERROR_EXISTS] = "variable already exists",
    [SNUK_ERROR_NON_TYPE] = "expected a type",
    [SNUK_ERROR_EXPECT_ASSIGN] = "expected assignment expression",
    [SNUK_ERROR_BUILTIN_INVALID_VALUE] = "invalid value to builtin type member value",
    [SNUK_ERROR_MEMBER_INITIALIZE] = "failed to initialize member",
    [SNUK_ERROR_SELF_CREATION] = "failed to create self",
    [SNUK_ERROR_PARAM_CREATION] = "failed to create parameter",
    [SNUK_ERROR_NON_FN] = "call expression on non function",
    [SNUK_ERROR_PARAM_COUNT] = "parameter count mismatch",
    [SNUK_ERROR_NO_PARAM] = "parameter doesn't exists",
    [SNUK_ERROR_PARAM] = "error in parameter passing",
    [SNUK_ERROR_PARAM_REQUIRED] = "parameter is required",
    [SNUK_ERROR_SELF] = "failed to get self",
    [SNUK_ERROR_SET_ENV_FAIL] = "failed to set env value",
    [SNUK_ERROR_MEMBER] = "couldn't find the member",
    [SNUK_ERROR_INTERFACE] = "failed to create interface",
    [SNUK_ERROR_TYPE_MISMATCH] = "types are not same",
};

const char *snuk_error_code_get_msg(SnukErrorCode code) {
    return error_messages[code];
}
