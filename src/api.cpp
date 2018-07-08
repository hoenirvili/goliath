#include "api.h"
#include "log.h"
#include "control_flow_graph.h"
#include "instruction.h"
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

BYTE *engine_share_buff;

size_t GetLayer()
{
	return PLUGIN_LAYER;
}

unique_ptr<ControlFlowGraph> graph;

static inline int* iteration(BYTE *mem)
{
	return (int*)(mem);
}

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
	
	graph = make_unique<ControlFlowGraph>();
	
	auto cfg = CFG(engine_share_buff);
	auto it = iteration(cfg);
	
	log_info("[CFGTrace] Iteration %d", *it);

	return TRUE;
}

PluginReport* DBTBeforeExecute(void *params, PluginLayer **layers)
{
	static int counter = 0;
	CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS*)params;
	DISASM * MyDisasm = custom_params->MyDisasm;

	char *content = (char*)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
	if (!content)
		return 0;

	auto report = new PluginReport();

	report->plugin_name = "CFGTrace";

	sprintf(content, "counter=%d", counter++);
	report->content_before = content;
	report->content_after = nullptr;
	auto instruction = Instruction(
		MyDisasm->EIP,
		MyDisasm->CompleteInstr,
		MyDisasm->Instruction.BranchType,
		custom_params->instrlen,
		custom_params->next_addr,
		custom_params->side_addr
	);

	int err = graph->append_instruction(instruction);
	if (err)
		log_error("cannot append instruction before call");
	return report;
}

PluginReport* DBTBranching(void *params, PluginLayer **layers)
{
	CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS*)params;
	DISASM *MyDisasm = custom_params->MyDisasm;

	char *content = (char*)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
	if (!content)
		return nullptr;

	auto report = new PluginReport();

	report->plugin_name = "CFGTrace";

	sprintf(content, "BRANCH");
	report->content_before = content;
	report->content_after = nullptr;
	
	return report;
}

PluginReport* DBTAfterExecute(void *params, PluginLayer **layers)
{
	static int counter = 1000;
	CUSTOM_PARAMS *custom_params = (CUSTOM_PARAMS*)params;
	DISASM * MyDisasm = custom_params->MyDisasm;

	char *content = (char*)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
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

	if ((*it) == 0) {

		auto graphviz = graph->graphviz();
		auto out = fstream("partiaflowgraph.dot", fstream::out);
		int err = graph->generate(graphviz, &out);
		if (err != 0) {
			log_error("cannot generate partial flow graph");
			return 0;
		}
	}

	(*it)++;
	return nullptr;
}