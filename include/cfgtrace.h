#pragma once

#include "cfgtrace/engine/types.h"

extern "C" {

/**
 *
 *
 */
EXPORT BOOL DBTInit();
/**
 *
 *
 */
EXPORT PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers);
/**
 *
 *
 */
EXPORT size_t GetLayer();
/**
 *
 *
 */
EXPORT PluginReport *DBTBranching(void *params, PluginLayer **layers);
/**
 *
 *
 */
EXPORT PluginReport *DBTAfterExecute(void *params, PluginLayer **layers);
/**
 *
 *
 */
EXPORT PluginReport *DBTFinish();
}