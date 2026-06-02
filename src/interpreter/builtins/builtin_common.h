#pragma once

#include "snuk/darray.h"
#include "snuk/defines.h"
#include "snuk/interpreter/builtins/snuk_builtins.h"
#include "snuk/interpreter/native.h"
#include "snuk/parser/snuk_type.h"
#include "snuk/refcount.h"

extern SnukType to_int_type;
extern SnukType to_float_type;
extern SnukType to_bool_type;
extern SnukType to_str_type;
extern SnukType str_length_type;
extern SnukType str_get_type;
