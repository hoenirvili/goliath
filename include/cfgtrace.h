#pragma once

#include "cfgtrace/api/types.h"

#define CFGTRACE_EXPORT __declspec(dllexport)

extern "C" {
CFGTRACE_EXPORT BOOL DBTInit();
CFGTRACE_EXPORT PluginReport *
DBTBeforeExecute(void *params, PluginLayer **layers);
CFGTRACE_EXPORT size_t GetLayer();
CFGTRACE_EXPORT PluginReport *DBTBranching(void *params, PluginLayer **layers);
CFGTRACE_EXPORT PluginReport *
DBTAfterExecute(void *params, PluginLayer **layers);
CFGTRACE_EXPORT PluginReport *DBTFinish();
}