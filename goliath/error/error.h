#pragma once

#include "goliath/format/string.h"

#define ex(T, str) (T)(format::string("%s", str))
