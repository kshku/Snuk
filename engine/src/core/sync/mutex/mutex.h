#pragma once

#include "defines.h"

typedef struct smutex {
        _Atomic b8 mutex;
        char padding[63];
} smutex;

SAPI void sMutexInit(smutex *mutex);

SAPI void sMutexLock(smutex *mutex);

SAPI b8 sMutexTryLock(smutex *mutex);

SAPI void sMutexUnlock(smutex *mutex);
