#pragma once

#include <stdbool.h>

#include "domain.h"

void addPredefinedFns(Domain *domain);
void setRet(int type, bool lval);