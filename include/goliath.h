#pragma once

#include "goliath/api/types.h"
#include "goliath/definition/generate.h"

#define PUBLIC_API extern "C" __declspec(dllexport)

// TODO(hoenir): for know this is the only thing implemented
#define GDL_FORMAT

#ifdef GRAPHVIZ_FORMAT
#define GENERATION_FORMAT definition::FORMAT::GRAPHVIZ
#endif

#ifdef GDL_FORMAT
#define GENERATION_FORMAT definition::FORMAT::GDL
#endif

/**
 * DBTInit initialises the internal state and memory of the plugin
 * this will manage the initialisation of the internal implementation
 * of control flow graph, windows handlers, engine memory handlers, file
 * creation and loggers.
 *
 * If any initialisation procedure fails this will return FALSE. If the
 * initialisation succeeds the function will return TRUE
 */
PUBLIC_API BOOL DBTInit();

/**
 * DBTBeforeExecute recives a CUSTOM_PARAMS* and an array of
 * link lists that holds all the information from all the other layers
 * Every plugin has the capability to put itself on any layer that he wants
 * using the GetLayer api function.
 * All the assembly information, context execution of the instruction are
 * gathered by the engine in this place. Based on this information we should try
 * and create a representation. DBTBeforeExecute will be called when ever the
 * engine has encountered a new assembly instruction. An assembly instruction
 * can be a simple instruction or a branching instruction. Nonetheless, if the
 * engine finds a branching instruction it will first execute DBTBranching and
 * after it will pass the same instruction to DBTBeforeExecute.
 *
 * After figuring out the instruction context and adding internally into our
 * graph the assembly instruction we always return a new ptr to a newly
 * allocated PluginReport* The engine will take care of freeing the memory. In
 * case of an error we should return nullptr and log the error out.
 *
 * Notice: On some wierd states the engine finds some instruction and it can't
 * quite figure out what is the next instruction. It set's the internal
 * side_addr and next_addr badly we will use a hack that's found in the package
 * assembly in order to fix this issue.
 *
 */
PUBLIC_API PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers);

/**
 * GetLayer returns the layer number that the plugin
 * is registred. The plugin will always operate on layer 2
 */
PUBLIC_API size_t GetLayer();

/**
 * DBTBranching recives a CUSTOM_PARAMS* and an array of
 * link lists that holds all the information from all the other layers
 * Every plugin has the capability to put itself on any layer that he wants
 * using the GetLayer api function.
 * All the assembly information, context execution of the instruction are
 * gathered by the engine in this place. Based on this information we should try
 * and create a representation. DBTBranching will be called when ever the
 * engine has encountered a new assembly branch instruction.
 *
 * After figuring out the instruction context and adding internally into our
 * graph the assembly instruction we always return a new ptr to a newly
 * allocated PluginReport* The engine will take care of freeing the memory. In
 * case of an error we should return nullptr and log the error out.
 *
 * Notice: On some wierd states the engine finds some instruction and it can't
 * quite figure out what is the next instruction. It set's the internal
 * side_addr and next_addr badly we will use a hack that's found in the package
 * assembly in order to fix this issue.
 * DBTBranching treat's the LEAVE instruction as branching instruction so we
 * should delegate the responsabilities to DBTBeforeExecute.
 *
 */
PUBLIC_API PluginReport *DBTBranching(void *params, PluginLayer **layers);

/**
 * DBTFinish the finish functions that's called when the engine terminates
 * an analysing run. The engine will call the pair DBTInit and DBTFinish
 * multiple times, when the engine is run with the -concolic flag. The concolic
 * flag start's the multi-run analysing process of the binary. In this
 * case, the binary is runned multiple times in order to find all the branches
 * and execution paths. Using this technique we can generate in a DEBUG mode
 * fashion a picture of the internal graph. It is important that in this method
 * we should take care of cleaning and freeing all the state that DBTInit had
 * previously initiliased. There are some things we need to do before the plugin
 * exists it's context. The DBTFinish should call the loading
 * memory serialization in order to write the graph into the shared memory
 * space.
 */
PUBLIC_API PluginReport *DBTFinish();