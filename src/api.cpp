#include "api.hpp"
#include "log.hpp"
#include "trace.hpp"
#include "partial_flow_graph.hpp"
#include <cstdio>
#include <fstream>
#include <memory>
#include <Windows.h>

using namespace std;

PluginLayer* GetPluginInterface(char *pluginname, size_t layer, PluginLayer **layers)
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

/**
 * engine_share_buff start of shared memory
 * from the engine with the plugin
 */
BYTE *engine_share_buff;

/**
 * GetLayer returns the type
 * of priority that the plugin has
 */
size_t GetLayer()
{
	return PLUGIN_LAYER;
}


unique_ptr<PartialFlowGraph> graph;


BOOL DBTInit()
{
	HANDLE file_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
	if (!file_mapping)
		return FALSE;

	engine_share_buff = (BYTE*)MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
	if (!engine_share_buff)
		return FALSE;

	const char *logname = LOGNAME_BUFFER(engine_share_buff);
	string name = (!logname) ? string() : string(logname);

	auto* file = new fstream(name, fstream::app);
	if (!(*file))
		return FALSE;

	log_init(file);
	log_info("[CFGTrace] Init is called");
	
	graph = make_unique<PartialFlowGraph>();
	
	return TRUE;
}

PluginReport* DBTBeforeExecute(void *params, PluginLayer **layers)
{
	log_info("[CFGTrace] Before execute is called");

	CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS*)params;
	DISASM* MyDisasm = custom_params->MyDisasm;
	TranslatorShellData* tdata = custom_params->tdata;
	size_t stack_trace = custom_params->stack_trace;

	char instr_bytes[100];
	char temp[MAX_PATH];
	size_t i;
	size_t len = custom_params->instrlen;

	PluginReport *report = (PluginReport*)
		VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
	if (!report) {
		log_error("cannot virtual alloc a new report");
		return nullptr;
	}

	char *content = (char*)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
	if (!content) {
		log_error("cannot alloc a new content block");
		return nullptr;
	}

	if (stack_trace) {
		auto ctx = (ExecutionContext*)tdata->PrivateStack;
		auto stack = Stack(ctx);
		stack.trace(content);
	} else
		memset(content, 0, 0x4000);

#ifndef _M_X64
	sprintf(temp, "%08X: ", (int)MyDisasm->VirtualAddr);
#else
	sprintf(temp, "%08X%08X: ", 
		(DWORD)(MyDisasm->VirtualAddr >> 32), 
		(DWORD)(MyDisasm->VirtualAddr & 0xFFFFFFFF));
#endif
	strcat(content, temp);

	memset(instr_bytes, 0, sizeof(instr_bytes));
	for (i = 0; i < len; i++)
		sprintf(instr_bytes + i * 3, "%02X ", *(BYTE*)(MyDisasm->EIP + i));
	sprintf(temp, "%-*s : %s", 45, instr_bytes, MyDisasm->CompleteInstr);
	strcat(content, temp);

	auto instruction = Instruction(
		MyDisasm->EIP,
		MyDisasm->CompleteInstr,
		MyDisasm->Instruction.BranchType,
		custom_params->instrlen,
		(size_t)MyDisasm->Instruction.AddrValue
	);

	graph->add(instruction);

	report->plugin_name = "CFGTrace";
	report->content_before = content;
	report->content_after = 0;

	return report;
}

PluginReport* DBTBranching(void *params, PluginLayer **layers)
{
	log_info("[CFGTrace] Branching is called");
	
	CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS*)params;
	DISASM* MyDisasm = custom_params->MyDisasm;

	auto instruction = Instruction(
		MyDisasm->EIP,
		MyDisasm->CompleteInstr,
		MyDisasm->Instruction.BranchType,
		custom_params->instrlen,
		(size_t)MyDisasm->Instruction.AddrValue
	);

	int err = graph->add_branch(instruction);
	if (err)
		return nullptr;

	return nullptr;
}

PluginReport* DBTAfterExecute(void *params, PluginLayer **layers)
{
	log_info("[CFGTrace] AferExecute is called");
	return nullptr;
}


PluginReport* DBTFinish()
{
	log_info("[CFGTrace] Finish is called");
	return nullptr;
}