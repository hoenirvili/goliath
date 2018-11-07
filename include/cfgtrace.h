#pragma once

#include "cfgtrace/api/types.h"
#include "cfgtrace/definition/generate.h"


#define PUBLIC_API extern "C" __declspec(dllexport)


//TODO(hoenir): for know this is the only thing implemented
#define GRAPHVIZ_FORMAT


#ifdef GRAPHVIZ_FORMAT
    #define GENERATION_FORMAT definition::FORMAT::GRAPHVIZ
#endif

#ifdef GDL_FORMAT
    #define GENERATION_FORMAT definition::FORMAT::GDL
#endif



PUBLIC_API BOOL DBTInit();
PUBLIC_API PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers);
PUBLIC_API size_t GetLayer();
PUBLIC_API PluginReport *DBTBranching(void *params, PluginLayer **layers);
PUBLIC_API PluginReport *DBTFinish();