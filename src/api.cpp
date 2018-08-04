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

PluginLayer* GetPluginInterface(const char* pluginname, size_t layer, PluginLayer** layers)
{
    PluginLayer* scanner = layers[layer];

    while (scanner) {
        if (scanner->data && scanner->data->plugin_name && !strcmp(scanner->data->plugin_name, pluginname))
            return scanner;

        scanner = scanner->nextnode;
    }

    return nullptr;
}

BYTE* engine_share_buff;

size_t GetLayer()
{
    return PLUGIN_LAYER;
}

unique_ptr<control_flow_graph> graph;

static inline int* iteration(BYTE* mem)
{
    return (int*) (mem);
}

BOOL DBTInit()
{
    HANDLE file_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
    if (!file_mapping)
        return FALSE;

    engine_share_buff = (BYTE*) MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (!engine_share_buff)
        return FALSE;

    const char* logname = LOGNAME_BUFFER(engine_share_buff);
    string name = (!logname) ? string() : string(logname);

    auto* file = new fstream(name, fstream::app);
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

static size_t get_next_addr_call(DISASM* disasm)
{
    array<char*, 3> args = {
		disasm->Argument1.ArgMnemonic, 
		disasm->Argument2.ArgMnemonic, 
		disasm->Argument3.ArgMnemonic
	};

    size_t r = 0xffffffff;
	
	for (const auto& arg : args)
    {
        try {
            r = stoul(arg, nullptr, 16);
            goto _exit;
        }
        catch (const invalid_argument&) {
            continue;
        }
        catch (const out_of_range&) {
            return r;
		}
    }

_exit:
    return r;
}

PluginReport* DBTBeforeExecute(void* params, PluginLayer** layers)
{
    static int counter = 0;
    CUSTOM_PARAMS* custom_params = (CUSTOM_PARAMS*) params;
    DISASM* MyDisasm = custom_params->MyDisasm;

    char* content = (char*) VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return 0;

    auto report = new PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "counter=%d", counter++);
    report->content_before = content;
    report->content_after = nullptr;

    size_t next_addr = 0;
    if ((next_addr = custom_params->next_addr); next_addr == 0xffffffff) {
        if (auto plugin = GetPluginInterface("APIReporter", 1, layers); plugin) {
            auto api_reporter = (char*) plugin->data->content_before;
            next_addr = get_next_addr_call(MyDisasm);
        }
    }

    auto instr = instruction(MyDisasm->EIP,
        MyDisasm->CompleteInstr,
        MyDisasm->Instruction.BranchType,
        custom_params->instrlen,
        next_addr,
        custom_params->side_addr);

    if (instr.is_branch()) {
        if (instr.is_call()) {
			if (auto plugin = GetPluginInterface("APIReporter", 1, layers); plugin)
                instr.api_reporter = (char*) plugin->data->content_before;

            try {
                graph->append_branch_instruction(instr);
            }
            catch (const exception& ex) {
                log_error("%s", ex.what());
            }
        }
        return report;
    }

    try {
        graph->append_instruction(instr);
    }
    catch (const exception& ex) {
        log_error("%s", ex.what());
    }

    return report;
}

PluginReport* DBTBranching(void* params, PluginLayer** layers)
{
    CUSTOM_PARAMS* custom_params = (CUSTOM_PARAMS*) params;
    DISASM* MyDisasm = custom_params->MyDisasm;

    char* content = (char*) VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return nullptr;

    auto report = new PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "BRANCH");
    report->content_before = content;
    report->content_after = nullptr;

    auto instr = instruction(MyDisasm->EIP,
        MyDisasm->CompleteInstr,
        MyDisasm->Instruction.BranchType,
        custom_params->instrlen,
        custom_params->next_addr,
        custom_params->side_addr);

    // skip calls
    if (instr.is_call())
        return report;

    try {
        graph->append_branch_instruction(instr);
    }
    catch (const exception& ex) {
        log_error("%s", ex.what());
    }

    return report;
}

PluginReport* DBTAfterExecute(void* params, PluginLayer** layers)
{
    static int counter = 1000;
    CUSTOM_PARAMS* custom_params = (CUSTOM_PARAMS*) params;
    DISASM* MyDisasm = custom_params->MyDisasm;

    char* content = (char*) VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return 0;

    auto report = new PluginReport();

    report->plugin_name = "CFGTrace";
    sprintf(content, "counter=%d", counter++);
    report->content_before = nullptr;
    report->content_after = content;

    return report;
}

PluginReport* DBTFinish()
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
        }
        catch (const exception& ex) {
            log_error("%s", ex.what());
        }

        (*it)++;
        return nullptr;
    }

    return nullptr;
}