#include "snuk_param.h"

#include "snuk_expr.h"
#include "snuk_type.h"

void snuk_param_log(SnukParam *param) {
    if (!param) return;
    log_trace("param: ", NULL);
    log_trace(SNUK_STRING_VIEW_FORMAT, SNUK_STRING_VIEW_ARG(param->name));
    if (param->type) log_trace("type: ", NULL);
    snuk_type_log(param->type);
    snuk_expr_log(param->default_value);
}
