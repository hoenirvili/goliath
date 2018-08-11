#include "api.h"
#include "control_flow_graph.h"
#include "instruction.h"
#include "log.h"
#include <Windows.h>
#include <array>
#include <cstdio>
#include <fstream>
#include <memory>

using namespace std;

PluginLayer *
GetPluginInterface(const char *pluginname, size_t layer, PluginLayer **layers)
{
    PluginLayer *scanner = layers[layer];

    while (scanner) {
        if (scanner->data && scanner->data->plugin_name &&
            !strcmp(scanner->data->plugin_name, pluginname))
            return scanner;

        scanner = scanner->nextnode;
    }

    return nullptr;
}

BYTE *engine_share_buff;

size_t GetLayer()
{
    return PLUGIN_LAYER;
}

unique_ptr<control_flow_graph> graph;

static inline int *iteration(BYTE *mem)
{
    return (int *)(mem);
}

BOOL DBTInit()
{
    HANDLE file_mapping =
      OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
    if (!file_mapping)
        return FALSE;

    engine_share_buff = (BYTE *)MapViewOfFile(
      file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (!engine_share_buff)
        return FALSE;

    const char *logname = LOGNAME_BUFFER(engine_share_buff);
    string name = (!logname) ? string() : string(logname);

    auto *file = new fstream(name, fstream::app);
    if (!(*file))
        return FALSE;

    log_init(file);
    log_info("[CFGTrace] Init is called");

    graph = make_unique<control_flow_graph>();

    auto cfg = CFG(engine_share_buff);
    auto it = iteration(cfg);

    log_info("[CFGTrace] Iteration %d", *it);

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
    if (auto plugin = GetPluginInterface("APIReporter", 1, layers); plugin) {
        instr.api_reporter = (char *)plugin->data->content_before;
        instr.branch_type = (BRANCH_TYPE)0; // make this a simple instruction
    }

    if (is_branch(instr.branch_type)) {
        if (is_call(instr.branch_type)) {
            try {
                graph->append_branch_instruction(instr);
            } catch (const exception &ex) {
                log_error("%s", ex.what());
            }
        }

        return report;
    }

    try {
        graph->append_instruction(instr);
    } catch (const exception &ex) {
        log_error("%s", ex.what());
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
    // skip calls
    if (is_call(instr.branch_type))
        return report;

    try {
        graph->append_branch_instruction(instr);
    } catch (const exception &ex) {
        log_error("%s", ex.what());
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
    log_info("[CFGTrace] Finish is called");
    auto cfg = CFG(engine_share_buff);
    auto it = iteration(cfg);

    // TODO(hoenir): You need to fill up the code for the n-th interation
    if ((*it) == 0) {
        auto graphviz = graph->graphviz();
        auto out = fstream("partiaflowgraph.dot", fstream::out);
        try {
            graph->generate(graphviz, &out);
        } catch (const exception &ex) {
            log_error("%s", ex.what());
        }

        (*it)++;
        return nullptr;
    }

    return nullptr;
}
