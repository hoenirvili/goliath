#include "cfgtrace.h"
#include "cfgtrace/api/types.h"
#include "cfgtrace/control_flow_graph.h"
#include "cfgtrace/engine/engine.h"
#include "cfgtrace/instruction.h"
#include "cfgtrace/logger/logger.h"
#include <cstdio>
#include <fstream>
#include <memory>
#include <windows.h>

using namespace std;

size_t GetLayer()
{
    return PLUGIN_LAYER;
}

static control_flow_graph graph;

static engine::engine main_engine;

BOOL DBTInit()
{
    HANDLE file_mapping =
      OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
    if (!file_mapping)
        return FALSE;

    try {
        main_engine = engine::engine(file_mapping);
    } catch (const exception &ex) {
        (void)ex;
        return FALSE;
    }
    const char *logname = main_engine.log_name();
    string name = (!logname) ? string() : string(logname);

    auto *file = new fstream(name, fstream::app);
    if (!(*file))
        return FALSE;

    logger_init(file);
    logger_info("[CFGTrace] Init is called");
    logger_info("[CFGTrace] Iteration %d", 1);
    return TRUE;
}

static inline bool direct_branch(BRANCH_TYPE type) noexcept
{
    return type == JmpType;
}

static inline bool is_ret(BRANCH_TYPE type) noexcept
{
    return type == RetType;
}

static inline bool is_call(BRANCH_TYPE type) noexcept
{
    return type == CallType;
}

static inline bool is_branch(BRANCH_TYPE type) noexcept
{
    switch (type) {
    case JO:
    case JC:
    case JE:
    case JA:
    case JS:
    case JP:
    case JL:
    case JG:
    case JB:
    case JECXZ:
    case JmpType:
    case CallType:
    case RetType:
    case JNO:
    case JNC:
    case JNE:
    case JNA:
    case JNS:
    case JNP:
    case JNL:
    case JNG:
    case JNB:
        return true;
    }

    return false;
}

// TODO(hoenir): add this to instruction.cpp
static size_t compute_side_addr(CUSTOM_PARAMS *custom_params)
{
    BRANCH_TYPE type =
      (BRANCH_TYPE)custom_params->MyDisasm->Instruction.BranchType;

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

static size_t compute_next_addr(CUSTOM_PARAMS *custom_params)
{
    return custom_params->next_addr;
}

static void compute_next_and_side_addr(CUSTOM_PARAMS *custom_params) noexcept
{
    custom_params->next_addr = compute_next_addr(custom_params);
    custom_params->side_addr = compute_side_addr(custom_params);
}

PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers)
{
    static int counter = 0;
    CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS *)params;
    compute_next_and_side_addr(custom_params);

    DISASM *MyDisasm = custom_params->MyDisasm;

    char *content =
      (char *)VirtualAlloc(nullptr, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return 0;

    auto report = new PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "counter=%d", counter++);
    report->content_before = content;
    report->content_after = nullptr;

    auto instr = instruction(MyDisasm->EIP,
                             MyDisasm->CompleteInstr,
                             (BRANCH_TYPE)MyDisasm->Instruction.BranchType,
                             custom_params->instrlen,
                             custom_params->next_addr,
                             custom_params->side_addr);
    if (auto plugin = main_engine.plugin_interface("APIReporter", 1, layers);
        plugin) {
        instr.api_reporter = (char *)plugin->data->content_before;
        instr.branch_type = (BRANCH_TYPE)0; // make this a simple instruction
    }

    if (instr.is_branch()) {
        if (instr.is_call()) {
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

PluginReport *DBTBranching(void *params, PluginLayer **layers)
{
    CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS *)params;
    compute_next_and_side_addr(custom_params);
    DISASM *MyDisasm = custom_params->MyDisasm;

    char *content = (char *)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return nullptr;

    auto report = new PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "BRANCH");
    report->content_before = content;
    report->content_after = nullptr;

    auto instr = instruction(MyDisasm->EIP,
                             MyDisasm->CompleteInstr,
                             (BRANCH_TYPE)MyDisasm->Instruction.BranchType,
                             custom_params->instrlen,
                             custom_params->next_addr,
                             custom_params->side_addr);
    if (instr.is_call()) // skip calls
        return report;

    if (instr.is_leave())
        return report;

    try {
        graph.append_branch_instruction(instr);
    } catch (const exception &ex) {
        logger_error("%s", ex.what());
    }

    return report;
}

PluginReport *DBTAfterExecute(void *params, PluginLayer **layers)
{
    static int counter = 1000;
    CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS *)params;
    compute_next_and_side_addr(custom_params);
    DISASM *MyDisasm = custom_params->MyDisasm;

    char *content = (char *)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return 0;

    auto report = new PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "counter=%d", counter++);
    report->content_before = nullptr;
    report->content_after = content;

    return report;
}

PluginReport *DBTFinish()
{
    auto graphviz = graph.graphviz();
    auto out = fstream("partiaflowgraph.dot", fstream::out);
    try {
        graph.generate(graphviz, &out);
    } catch (const exception &ex) {
        logger_error("%s", ex.what());
    }

    return nullptr;
}