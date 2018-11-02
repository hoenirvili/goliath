#include "cfgtrace.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/assembly/instruction.h"
#include "cfgtrace/definition/generate.h"
#include "cfgtrace/engine/engine.h"
#include "cfgtrace/graph/control_flow.h"
#include "cfgtrace/logger/logger.h"
#include "cfgtrace/memory/loader.h"

#include <string>
#include <stdexcept>

/**
 *  new_plugin_report it will create a new PluginReport and
 *  fill with all the required information that the engine will recieve
 */
static PluginReport *new_plugin_report() noexcept
{
    char *content = (char *)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content) {
        logger_error("VirtualAlloc failed, cannot alloc a page of size 0x4000");
        return nullptr;
    }
    auto report = new PluginReport;
    report->plugin_name = "CFGTrace";
    report->content_after = content;
    report->content_before = nullptr;
    return report;
}

/**
 * GetLayer returns the layer number that the plugin
 * is registred. The plugin will always operate on layer 2
 */
size_t GetLayer()
{
    return PLUGIN_LAYER;
}

/**
 * DBTInit initialises the internal state and memory of the plugin
 * this will manage the initialisation of the internal implementation
 * of control flow graph, windows handlers, engine memory handlers, file
 * creation and loggers.
 *
 * If any initialisation procedure fails this will return FALSE. If the
 * initialisation succeeds the function will return TRUE
 */
BOOL DBTInit()
{
    engine::engine *e;
    if (!engine::is_initialised()) {
        if (e = engine::instance(); !e)
            return FALSE;
    }

    auto log_name = e->log_name();
    if (!logger::initialise(log_name))
        return FALSE;

    logger_info("[CFGTrace] DBTInit engine and logger state are initiliased");
    graph::graph *g;
    if (!graph::is_initialised()) {
        if (g = graph::instance(); !g)
            return FALSE;
    }

    auto it = e->cfg_iteration();
    if (*it) {
        auto smr = e->cfg_serialize_memory_region();
        memory::loader(g, smr);
    }

    (*it)++;
    logger_info("[CFGTrace] Init is called for iteration %d", *it);

    return TRUE;
}

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
PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers)
{
    // we know ahead that this is a CUSTOM_PARAMS*
    CUSTOM_PARAMS *cp = static_cast<CUSTOM_PARAMS *>(params);
    assembly::patch_next_and_side_addr(cp); // patch the next and side

    DISASM *d = cp->MyDisasm;

    auto instruction = assembly::instruction(
      d->EIP, d->CompleteInstr, (BRANCH_TYPE)(d->Instruction.BranchType),
      cp->instrlen, cp->next_addr, cp->side_addr);

    /**
     * The binary that the engine analyses could call external API functions
     * such as user speace kernel function and we don't want to just reference
     * those by name and add them as if they were simple instruction. We will
     * use the information gathered by the APIReporter in order to capture and
     * extract the information that we need.
     */
    auto engine = engine::instance();
    auto plugin = engine->plugin_interface("APIReporter", 1, layers);
    if (plugin) {
        // extract the information gathered by the APIReporter
        instruction.api_reporter =
          static_cast<char *>(plugin->data->content_before);
        // we don't want to follow up the instruction, treat it as a simple
        // instruction
        instruction.branch_type = static_cast<BRANCH_TYPE>(0);
    }

    auto graph = graph::instance();

    try {
        graph->append(instruction);
    } catch (const std::exception &ex) {
        logger_error("cannot append instruction, %s", ex.what());
        return nullptr;
    }

    return new_plugin_report();
}

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
PluginReport *DBTBranching(void *params, PluginLayer **layers)
{
    // we know ahead that this is a CUSTOM_PARAMS*
    CUSTOM_PARAMS *cp = static_cast<CUSTOM_PARAMS *>(params);
    assembly::patch_next_and_side_addr(cp); // path the next and side

    DISASM *d = cp->MyDisasm;

    auto instruction = assembly::instruction(
      d->EIP, d->CompleteInstr, (BRANCH_TYPE)(d->Instruction.BranchType),
      cp->instrlen, cp->next_addr, cp->side_addr);

    /**
     *  Ff this is a call instruction skip it
     *  let the DBTBeforeExecute treat this case
     */
    if (instruction.is_call())
        return new_plugin_report();
    /**
     *  This unexpected behaviour is a bug so in order
     *  to repair this we skip this instruction and let
     *  the expected DBTBeforeExecute methood treat it
     *  as a simple instruction
     */
    if (instruction.is_leave())
        return new_plugin_report();

    auto graph = graph::instance();
    try {
        graph->append(instruction);
    } catch (const std::exception &ex) {
        logger_error("cannot append branch instruction %s", ex.what());
        return nullptr;
    }

    return new_plugin_report();
}

#define GENERATION_FORMAT definition::FORMAT::GRAPHVIZ
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
PluginReport *DBTFinish()
{
    logger_info("[CFGTrace] Finish is called");
    auto graph = graph::instance();
    std::string definitions = "";
    try {
        definitions = definition::generate(graph, GENERATION_FORMAT);
    } catch(const std::exception &ex) {
        logger_info("cannot generate definitions, %s", ex.what());
        goto _exit;
    }

    auto engine = engine::instance();
    auto to = engine->cfg_memory_region();
    memory::unloader(graph, to);

_exit:
    graph::clean();
    logger::clean();
    engine::clean();

    return new_plugin_report();
}