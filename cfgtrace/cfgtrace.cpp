#include "cfgtrace/control_flow_graph.h"
#include "cfgtrace/engine/engine.h"
#include "cfgtrace/engine/types.h"
#include "cfgtrace/instruction.h"
#include "cfgtrace/logger/logger.h"
#include <cstdio>
#include <fstream>
#include <memory>
#include <windows.h>

using namespace std;

#define PLUGIN_LAYER 2
#define memsharedname "Local\\VDCApiLog"

static control_flow_graph graph;

static engine::engine main_engine;

size_t GetLayer()
{
    return PLUGIN_LAYER;
}

BOOL DBTInit()
{
    HANDLE file_mapping =
      OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
    if (!file_mapping)
        return FALSE;

    main_engine = engine::engine(file_mapping);
    const char *logname = main_engine.log_name();
    string name = (!logname) ? string() : string(logname);

    auto *file = new fstream(name, fstream::app);
    if (!(*file))
        return FALSE;

    logger_init(file);
    logger_info("[CFGTrace] Init is called");

    /*auto mem = cfg_shared_memory(engine_shared_memory);
    auto it = cfg_iteration(mem);
    if (*it) {
        auto size = cfg_size(mem);
        if (!*size) {
            *size = graph->mem_size();
        }
        try {
            graph->deserialize(mem, *size);
        }
        catch (const exception& ex) {
            log_error("%s", ex.what());
            return FALSE;
        }
    }*/

    logger_info("[CFGTrace] Iteration %d", 1);
    return TRUE;
}

static inline bool direct_branch(engine::BRANCH_TYPE type) noexcept
{
    return type == engine::JmpType;
}

static inline bool is_ret(engine::BRANCH_TYPE type) noexcept
{
    return type == engine::RetType;
}

static inline bool is_call(engine::BRANCH_TYPE type) noexcept
{
    return type == engine::CallType;
}

static inline bool is_branch(engine::BRANCH_TYPE type) noexcept
{
    switch (type) {
    case engine::JO:
    case engine::JC:
    case engine::JE:
    case engine::JA:
    case engine::JS:
    case engine::JP:
    case engine::JL:
    case engine::JG:
    case engine::JB:
    case engine::JECXZ:
    case engine::JmpType:
    case engine::CallType:
    case engine::RetType:
    case engine::JNO:
    case engine::JNC:
    case engine::JNE:
    case engine::JNA:
    case engine::JNS:
    case engine::JNP:
    case engine::JNL:
    case engine::JNG:
    case engine::JNB:
        return true;
    }

    return false;
}

static size_t compute_side_addr(engine::CUSTOM_PARAMS *custom_params)
{
    engine::BRANCH_TYPE type =
      (engine::BRANCH_TYPE)custom_params->MyDisasm->Instruction.BranchType;

    if (direct_branch(type))
        return 0;
    if (is_ret(type))
        return 0;

    size_t eip = custom_params->MyDisasm->EIP;
    size_t len = custom_params->instrlen;

    size_t false_branch = eip + len;
    if (is_call(type)) {
        if (custom_params->next_addr == custom_params->side_addr &&
            custom_params->next_addr == false_branch)
            return 0;

        return false_branch;
    }

    if (!is_call(type) && !is_ret(type))
        return custom_params->side_addr;

    return false_branch;
}

static size_t compute_next_addr(engine::CUSTOM_PARAMS *custom_params)
{
    return custom_params->next_addr;
}

static void
compute_next_and_side_addr(engine::CUSTOM_PARAMS *custom_params) noexcept
{
    custom_params->next_addr = compute_next_addr(custom_params);
    custom_params->side_addr = compute_side_addr(custom_params);
}

engine::PluginReport *
DBTBeforeExecute(void *params, engine::PluginLayer **layers)
{
    static int counter = 0;
    engine::CUSTOM_PARAMS *custom_params = (engine::CUSTOM_PARAMS *)params;
    compute_next_and_side_addr(custom_params);

    engine::DISASM *MyDisasm = custom_params->MyDisasm;

    char *content =
      (char *)VirtualAlloc(nullptr, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return 0;

    auto report = new engine::PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "counter=%d", counter++);
    report->content_before = content;
    report->content_after = nullptr;

    auto instr =
      instruction(MyDisasm->EIP,
                  MyDisasm->CompleteInstr,
                  (engine::BRANCH_TYPE)MyDisasm->Instruction.BranchType,
                  custom_params->instrlen,
                  custom_params->next_addr,
                  custom_params->side_addr);
    if (auto plugin = main_engine.plugin_interface("APIReporter", 1, layers);
        plugin) {
        instr.api_reporter = (char *)plugin->data->content_before;
        instr.branch_type =
          (engine::BRANCH_TYPE)0; // make this a simple instruction
    }

    if (is_branch(instr.branch_type)) {
        if (is_call(instr.branch_type)) {
            try {
                graph.append_branch_instruction(instr);
            } catch (const exception &ex) {
                logger_error("%s", ex.what());
            }
        }

        return report;
    }

    try {
        graph.append_instruction(instr);
    } catch (const exception &ex) {
        logger_error("%s", ex.what());
    }

    return report;
}

engine::PluginReport *DBTBranching(void *params, engine::PluginLayer **layers)
{
    engine::CUSTOM_PARAMS *custom_params = (engine::CUSTOM_PARAMS *)params;
    compute_next_and_side_addr(custom_params);
    engine::DISASM *MyDisasm = custom_params->MyDisasm;

    char *content = (char *)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return nullptr;

    auto report = new engine::PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "BRANCH");
    report->content_before = content;
    report->content_after = nullptr;

    auto instr =
      instruction(MyDisasm->EIP,
                  MyDisasm->CompleteInstr,
                  (engine::BRANCH_TYPE)MyDisasm->Instruction.BranchType,
                  custom_params->instrlen,
                  custom_params->next_addr,
                  custom_params->side_addr);
    // skip calls
    if (is_call(instr.branch_type))
        return report;

    try {
        graph.append_branch_instruction(instr);
    } catch (const exception &ex) {
        logger_error("%s", ex.what());
    }

    return report;
}

engine::PluginReport *
DBTAfterExecute(void *params, engine::PluginLayer **layers)
{
    static int counter = 1000;
    engine::CUSTOM_PARAMS *custom_params = (engine::CUSTOM_PARAMS *)params;
    compute_next_and_side_addr(custom_params);
    engine::DISASM *MyDisasm = custom_params->MyDisasm;

    char *content = (char *)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return 0;

    auto report = new engine::PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "counter=%d", counter++);
    report->content_before = nullptr;
    report->content_after = content;

    return report;
}

engine::PluginReport *DBTFinish()
{
    logger_info("[CFGTrace] Finish is called");
    /* auto mem = cfg_shared_memory(engine_shared_memory);
     auto it = cfg_iteration(mem);
     auto smem = cfg_serialize_shared_memory(mem);
     auto size = cfg_size(mem);
     *size = graph->mem_size();

     try {
         graph->serialize(smem, *size);
     } catch (const exception &ex){
         log_error("%s", ex.what());
         return nullptr;
     }*/

    auto graphviz = graph.graphviz();
    auto out = fstream("partiaflowgraph.dot", fstream::out);
    try {
        graph.generate(graphviz, &out);
    } catch (const exception &ex) {
        logger_error("%s", ex.what());
    }

    //(*it)++;
    return nullptr;
}