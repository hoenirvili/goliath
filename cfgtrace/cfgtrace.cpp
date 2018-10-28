#include "cfgtrace.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/engine/engine.h"
#include "cfgtrace/graph/control_flow.h"
#include "cfgtrace/logger/logger.h"
#include "cfgtrace/memory/loader.h"

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
 * of control flow graph, windows handlers, engine memory handlers, file creation
 * and loggers.
 *
 * If any initialisation procedure fails this will return FALSE. If the initialisation
 * succeeds the function will return TRUE
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

PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers)
{
    return nullptr;
}

PluginReport *DBTBranching(void *params, PluginLayer **layers)
{
    return nullptr;
}

PluginReport *DBTFinish()
{
    return nullptr;
}

// static PluginReport *create_plugin_report()
// {
//     char *content = (char *)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
//     if (!content)
//         return nullptr;

//     auto report = new PluginReport();
//     report->plugin_name = "CFGTrace";
//     report->content_before = content;
//     report->content_after = nullptr;
//     return report;
// }

// PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers)
// {
//     CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS *)params;
//     assembly::compute_next_and_side_addr(custom_params);

//     DISASM *MyDisasm = custom_params->MyDisasm;

//     auto instr =
//       assembly::instruction(MyDisasm->EIP, MyDisasm->CompleteInstr,
//       (BRANCH_TYPE)MyDisasm->Instruction.BranchType,
//                             custom_params->instrlen, custom_params->next_addr, custom_params->side_addr);
//     auto plugin = _engine.plugin_interface("APIReporter", 1, layers);
//     if (plugin) {
//         instr.api_reporter = (char *)plugin->data->content_before;
//         instr.branch_type = (BRANCH_TYPE)0; // make this a simple instruction
//     }

//     if (instr.is_branch()) {
//         if (instr.is_call()) {
//             try {
//                 control_flow_graph->append_branch_instruction(instr);
//             } catch (const std::exception &ex) {
//                 logger_error("%s", ex.what());
//             }
//         }
//         return create_plugin_report();
//     }

//     try {
//         control_flow_graph->append_instruction(instr);
//     } catch (const std::exception &ex) {
//         logger_error("%s", ex.what());
//     }

//     return create_plugin_report();
// }

// PluginReport *DBTBranching(void *params, PluginLayer **layers)
// {
//     (void)layers;
//     CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS *)params;
//     assembly::compute_next_and_side_addr(custom_params);
//     DISASM *MyDisasm = custom_params->MyDisasm;

//     auto instr =
//       assembly::instruction(MyDisasm->EIP, MyDisasm->CompleteInstr,
//       (BRANCH_TYPE)MyDisasm->Instruction.BranchType,
//                             custom_params->instrlen, custom_params->next_addr, custom_params->side_addr);
//     if (instr.is_call()) // skip calls
//         return create_plugin_report();
//     ;

//     // TODO(hoenir): why the engine treats *leave* instruction as a branch
//     // instruction? this does not make sense because leave instruction means:
//     // move EBP ESP
//     // pop EBP
//     if (instr.is_leave()) // skip leave instructions
//         return create_plugin_report();

//     try {
//         control_flow_graph->append_branch_instruction(instr);
//     } catch (const std::exception &ex) {
//         logger_error("%s", ex.what());
//     }

//     return create_plugin_report();
// }

// PluginReport *DBTFinish()
// {
//     logger_info("[CFGTrace] Finish is called");
//     auto graphviz = control_flow_graph->graphviz();
//     auto it = _engine.cfg_iteration();
//     logger_info("[CFGTrace] Finish is called for iteration %d", *it);
//     auto out = std::fstream("partiaflowgraph.dot", std::fstream::out);

//     try {
//         control_flow_graph->generate(graphviz, &out, *it);
//     } catch (const std::exception &ex) {
//         logger_error("%s", ex.what());
//     }

//     auto mem = _engine.cfg_serialize_memory_region();
//     auto size = _engine.cfg_size();
//     *size = control_flow_graph->mem_size();
//     auto total_memory = _engine.cfg_memory_region_size();

//     if ((*size) > total_memory) {
//         logger_error("memory is full, cannot write more");
//         return nullptr;
//     }

//     control_flow_graph->load_to_memory(mem);
//     logger::unset_writer();
//     return nullptr;
// }
