#include "common.hpp"
#include "log.hpp"
#include "partial_flow_graph.hpp"

BYTE* shared_mem;

size_t GetLayer()
{
    return PLUGIN_LAYER;
}

// For every call to DBTBeforeExecute this will keep
// the current partial flow graph state
PartialFlowGraph partialFlowGraph;

BOOL DBTInit()
{
    Log* logger = Log::GetInstance();
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
    shared_mem = (BYTE*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (!shared_mem) {
        logger->DebugMsg("Cannot access map file");
        return FALSE;
    }
    if (shared_mem[0])
        logger->ChangeLogName(LOGNAME_BUFFER(shared_mem));
    return FALSE;

    return TRUE;
}

PluginReport* DBTBranching(void* custom_params, PluginLayer** layers)
{
    return (PluginReport*) VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
}

PluginReport* DBTBeforeExecute(CUSTOM_PARAMS* custom_params, PluginLayer** layers)
{

    DISASM* MyDisasm = custom_params->MyDisasm;
    TranslatorShellData* tdata = custom_params->tdata;
    size_t stack_trace = custom_params->stack_trace;

    char instr_bytes[100];
    char temp[MAX_PATH];
    size_t i;
    size_t len = custom_params->instrlen;
    PluginReport* report = (PluginReport*) VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
    if (!report)
        throw std::runtime_error::exception("cannot access virual page");
    char* content = (char*) VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
    if (!content)
        return 0;

    if (stack_trace)
        StackTrace((ExecutionContext*) tdata->PrivateStack, content);
    else
        memset(content, 0, 0x4000);
#ifndef _M_X64
    sprintf(temp, "%08X: ", (int) MyDisasm->VirtualAddr);
#else
    sprintf(temp, "%08X%08X: ", (DWORD)(MyDisasm->VirtualAddr >> 32), (DWORD)(MyDisasm->VirtualAddr & 0xFFFFFFFF));
#endif
    strcat(content, temp);
    memset(instr_bytes, 0, sizeof(instr_bytes));
    for (i = 0; i < len; i++)
        sprintf(instr_bytes + i * 3, "%02X ", *(BYTE*) (MyDisasm->EIP + i));
    sprintf(temp, "%-*s : %s", 45, instr_bytes, MyDisasm->CompleteInstr);
    strcat(content, temp);

    Instruction param = {
        MyDisasm->EIP, /*the current instruction pointer*/
        MyDisasm->CompleteInstr, /*the complete string assembly instruction*/
        (unsigned int) MyDisasm->Instruction.BranchType, /*the type of instruction if it's a jump
                                                            like instruction*/
        len, /*the length of the current instruction*/
        (size_t) MyDisasm->Instruction.AddrValue /*the addr on which the instruction could jump*/
    };

    partialFlowGraph.add_instruction(param);

    // pack the plugin response
    report->plugin_name = "DBTTrace";
    report->content_before = content;
    report->content_after = 0;

    return report;
}

PluginReport* DBTAfterExecute(void* custom_params, PluginLayer** layers)
{
    return (PluginReport*) VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
}

PluginReport* DBTFinish()
{
    return (PluginReport*) VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
}
