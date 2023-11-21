#pragma once

#include <stdbool.h>

#include "da.h"

// adds in ST the predefined functions from example: puti, putr, puts.
// if they are not added, an error message would be thrown, because these would be undefined
void addPredefinedFns(Domain *domain);

// sets the global variable "ret" with the resulted type from a rule
void setRet(Ret* ret, int type, bool lval);