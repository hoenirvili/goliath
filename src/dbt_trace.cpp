#include <memory>

#include "common.hpp"
#include "log.hpp"
#include "partial_flow_graph.hpp"

using namespace std;

size_t GetLayer()
{
	return PLUGIN_LAYER;
}

/**
 * engine_share_buffptr start of shared mem
 * from the engine with the plugin
 */
BYTE* engine_share_buff; 

/**
*
*/
static shared_ptr<Log> logger;

/**
*
*/
static unique_ptr<PartialFlowGraph> pfg;

BOOL DBTInit()
{
    HANDLE map = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
	if (!map)
		return FALSE;

    engine_share_buff = (BYTE*) MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (!engine_share_buff)
        return FALSE;

	const char *logname = LOGNAME_BUFFER(engine_share_buff);
	string name = (!logname) ? string() : string(logname);
    
	logger = Log::instance(name);
	
	logger->info("[DBTITrace] Init is called");
	
	pfg = make_unique<PartialFlowGraph>(logger);

	return TRUE;
}

PluginReport* DBTBranching(void* custom_params, PluginLayer** layers)
{
    return (PluginReport*) 
        VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
}

PluginReport *DBTBeforeExecute(void *custom_params, PluginLayer **layers)
{
	CUSTOM_PARAMS* params = (CUSTOM_PARAMS *)custom_params;
    DISASM* MyDisasm = params->MyDisasm;
    TranslatorShellData* tdata = params->tdata;
    size_t stack_trace = params->stack_trace;

    char instr_bytes[100];
    char temp[MAX_PATH];
    size_t i;
    size_t len = params->instrlen;
    PluginReport* report = (PluginReport*) VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
	if (!report)
		return nullptr;

    char* content = (char*) VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
	if (!content)
		return report;

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


	//TODO(hoenir): Deserialize from mem shared
	//TODO(hoenir): Keep adding nodes

    Instruction instruction = {
        MyDisasm->EIP,
        MyDisasm->CompleteInstr,
        MyDisasm->Instruction.BranchType,
        len, (size_t) MyDisasm->Instruction.AddrValue
    };

	pfg->add(instruction);
    
	// pack the plugin response
    report->plugin_name = "DBTTrace";
    report->content_before = content;
    report->content_after = 0;

    return report;
}

PluginReport* DBTAfterExecute(void* custom_params, PluginLayer** layers)
{
    return (PluginReport*) 
		VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
}

PluginReport* DBTFinish()
{
	logger->info("[DBTITrace] Finish is called");

	//TODO(hoenir): Generate pfg
	//TODO(hoenir): Merge if any
	//TODO(hoenir): Serialise
	//TODO(hoenir): Store serilise in mem shared aka CFG
	//TODO(hoenir): Clean up

    return (PluginReport*)
		VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
}


//TODO(hoenir): At finish deserialize one more time and generate cfg from all pfg

