#pragma once

#include "cfgtrace/api/types.h"

#define PUBLIC_API extern "C" __declspec(dllexport)

PUBLIC_API BOOL DBTInit();
PUBLIC_API PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers);
PUBLIC_API size_t GetLayer();
PUBLIC_API PluginReport *DBTBranching(void *params, PluginLayer **layers);
PUBLIC_API PluginReport *DBTAfterExecute(void *params, PluginLayer **layers);
PUBLIC_API PluginReport *DBTFinish();