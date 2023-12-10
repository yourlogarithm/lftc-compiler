#pragma once

#include <stddef.h>

_Noreturn void err(const char *fmt, ...);
void *safeAlloc(size_t nBytes);
char *loadFile(const char *fileName);
