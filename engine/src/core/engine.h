#pragma once

#include "application_types.h"
#include "defines.h"

// Since main function is in entry.h and will be included in the application,
// need to make these public API

SAPI b8 initializeEngine(Application *app_inst);

SAPI void shutdownEngine(void);

SAPI b8 engineRun(void);
