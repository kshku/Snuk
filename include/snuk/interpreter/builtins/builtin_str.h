#pragma once

#include "builtins_common.h"
#include "snuk/defines.h"

SnukValue builtin_str_get_member(SnukValue value, SnukStringView field);
