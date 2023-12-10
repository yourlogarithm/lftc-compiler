#pragma once

#include <stdbool.h>

#include "domain.h"

void addPredefinedFns(Domain *domain);
void setRet(Ret* ret, int type, bool lval);