#pragma once

#include "cfgtrace/format/string.h"

#define ex(T, str) (T)(format::string("%s:%d : %s", str))