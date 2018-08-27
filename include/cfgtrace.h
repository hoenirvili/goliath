#pragma once

#include "cfgtrace/engine/types.h"

extern "C" {
CFGTRACE_EXPORT BOOL DBTInit();
CFGTRACE_EXPORT engine::PluginReport *
DBTBeforeExecute(void *params, engine::PluginLayer **layers);
CFGTRACE_EXPORT size_t GetLayer();
CFGTRACE_EXPORT engine::PluginReport *
DBTBranching(void *params, engine::PluginLayer **layers);
CFGTRACE_EXPORT engine::PluginReport *
DBTAfterExecute(void *params, engine::PluginLayer **layers);
CFGTRACE_EXPORT engine::PluginReport *DBTFinish();
}