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

static unique_ptr<control_flow_graph> graph;

static engine::engine main_engine;

BOOL DBTInit()
{
    HANDLE file_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
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
    if (name.empty())
        return FALSE;

    auto file = make_unique<fstream>(name, fstream::app);
    if (!(*file))
        return FALSE;

    logger::init(file.release());

    if (!graph)
        graph = make_unique<control_flow_graph>();

    logger_info("[CFGTrace] Init is called");
    auto it = main_engine.cfg_iteration();
    if (*it) {
        auto mem = main_engine.cfg_serialize_memory_region();
        auto mem_size = main_engine.cfg_size();
        graph->load_from_memory(mem);
    }
    (*it)++;
    logger_info("[CFGTrace] Iinit is called for iteration %d", *it);

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

// TODO(hoenir): the engine should fix this
static size_t compute_side_addr(CUSTOM_PARAMS *custom_params)
{
    BRANCH_TYPE type = (BRANCH_TYPE)custom_params->MyDisasm->Instruction.BranchType;

    if (direct_branch(type))
        return 0;
    if (is_ret(type))
        return 0;

    size_t eip = custom_params->MyDisasm->EIP;
    size_t len = custom_params->instrlen;

    size_t false_branch = eip + len;
    if (is_call(type)) {
        if (custom_params->next_addr == custom_params->side_addr && custom_params->next_addr == false_branch)
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

    char *content = (char *)VirtualAlloc(nullptr, 0x4000, MEM_COMMIT, PAGE_READWRITE);
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
    auto plugin = main_engine.plugin_interface("APIReporter", 1, layers);
    if (plugin) {
        instr.api_reporter = (char *)plugin->data->content_before;
        instr.branch_type = (BRANCH_TYPE)0; // make this a simple instruction
    }

    if (instr.is_branch()) {
        if (instr.is_call()) {
            try {
                graph->append_branch_instruction(instr);
            } catch (const exception &ex) {
                logger_error("%s", ex.what());
            }
        }
        return report;
    }

    try {
        graph->append_instruction(instr);
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

    // TODO(hoenir): why the engine treats *leave* instruction as a branch
    // instruction? this does not make sense because leave instruction means:
    // move EBP ESP
    // pop EBP
    if (instr.is_leave()) // skip leave instructions
        return report;

    try {
        graph->append_branch_instruction(instr);
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
    logger_info("[CFGTrace] Finish is called");
    auto graphviz = graph->graphviz();
    auto it = main_engine.cfg_iteration();
    logger_info("[CFGTrace] Finish is called for iteration %d", *it);
    auto out = fstream("partiaflowgraph.dot", fstream::out);
    try {
        graph->generate(graphviz, &out, *it);
    } catch (const exception &ex) {
        logger_error("%s", ex.what());
    }
    // TODO(honeir): do we still need volatile ?
    auto mem = main_engine.cfg_serialize_memory_region();
    volatile auto size = main_engine.cfg_size();
    *size = graph->mem_size();

    // TODO(hoenir): this should bail out or we should attempt to
    // tell the engine we need more more memory.. for now we should
    // squeeze the hole cfg in 2MB.
    auto total_memory = main_engine.cfg_memory_region_size();
    if ((*size) > total_memory) {
        logger_error("memory is full, cannot write more");
        return nullptr;
    }
    graph->load_to_memory(mem);

    return nullptr;
}

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, LPVOID reserved)
{
    (void)dll;
    (void)reserved;

    switch (reason) {
    case DLL_PROCESS_ATTACH:
        /**
         * The DLL is being loaded into the virtual address space of
         * the current process as a result of the process starting up or
         * as a result of a call to LoadLibrary.DLLs can use this
         * opportunity to initialize any instance data or to use the
         * TlsAlloc function to allocate a thread local storage(TLS)
         * index.The lpReserved parameter indicates whether the DLL is
         * being loaded statically or dynamically.
         */
        break;
    case DLL_THREAD_ATTACH:
        /**
         * The DLL is being unloaded from the virtual address space of
         * the calling process because it was loaded unsuccessfully or
         * the reference count has reached zero (the processes has either
         * terminated or called FreeLibrary one time for each time it called LoadLibrary).
         * The lpReserved parameter indicates whether the DLL is being
         * unloaded as a result of a FreeLibrary call, a failure to load, or
         * process termination The DLL can use this opportunity to call
         * the TlsFree function to free any TLS indices allocated by using
         * TlsAlloc and to free any thread local data Note that the thread
         * that receives the DLL_PROCESS_DETACH notification is not necessarily
         * the same thread that received the DLL_PROCESS_ATTACH
         * notification.
         */
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}