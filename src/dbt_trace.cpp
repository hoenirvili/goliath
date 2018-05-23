#include <memory>
#include <fstream>
#include <cerrno>
#include <assert.h>

#include "common.hpp"
#include "log.hpp"
#include "partial_flow_graph.hpp"
#include "instruction.hpp"

using namespace std;

/**
 *  GetLayer returns the type 
 * of priority that the plugin has
 */
size_t GetLayer()
{
	// the most highest priority
	return PLUGIN_LAYER;
}

/**
 * engine_share_buff start of shared memory
 * from the engine with the plugin
 */
BYTE* engine_share_buff; 

/**
 * file_mapping handler address to a shared
 * file location with the engine
 */
static HANDLE file_mapping;

/**
 * pfg holds the current partial flow graph nodes and structures
 */
static unique_ptr<PartialFlowGraph> pfg;

BOOL DBTInit()
{
	file_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memsharedname);
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
	
	log_info("[DBTITrace] Init is called");
	pfg = make_unique<PartialFlowGraph>();

	return TRUE;
}

//PluginReport* DBTAfterExecute(void* custom_params, PluginLayer** layers)
//{
//
//}

static size_t bit_index_reg_position(int byte)
{
	const ARGUMENTS_TYPE regs[] = {
		REG0,
		REG1,REG2, REG3,
		REG4, REG5, REG6,
		REG7, REG8, REG9,
		REG10, REG11, REG12,
		REG13, REG14, REG15 };

	size_t n = ARRAY_SIZE(regs);

	for (size_t i = 0; i < n; i++)
		if (byte & regs[i])
			return i + 1; // return bit poision found

	return 0; // invalid bit poisition
}

static size_t argument_value(const ARGTYPE& argument)
{
	// call dword [0x00c83374]
	if (argument.ArgType & MEMORY_TYPE) {
		// plug in the formula, if the formula failed return the displacement
		/*
		size_t r = argument.Memory.BaseRegister * argument.Memory.Scale *
			bit_index_reg_position(argument.Memory.IndexRegister);
		if (check(r))
			return r;
		*/
		if (argument.Memory.BaseRegister==0 && 
			argument.Memory.IndexRegister==0 && 
			argument.Memory.Scale==0)
			if (argument.Memory.Displacement > 0xFFFF)
				return (size_t)(*(DWORD*)argument.Memory.Displacement);
	}

	// call 0x00c83374 constat tuype
	if (argument.ArgType & CONSTANT_TYPE) {
		if (argument.Memory.Displacement)
			return (size_t)argument.Memory.Displacement;
	}
	
	return 0;
}

static size_t instruction_value(const DISASM* disass) noexcept
{
	size_t value = 0;
	if (disass->Instruction.AddrValue)
		return (size_t)disass->Instruction.AddrValue;

	if (value = argument_value(disass->Argument1))
		return value;
	if (value = argument_value(disass->Argument2))
		return value;
	if (value = argument_value(disass->Argument3))
		return value;

	//TODO(hoenir): more code required
	return value;
}

PluginReport* DBTBeforeExecute(void* custom_params, PluginLayer** layers)
{
	log_info("Am apelat before execute");
	CUSTOM_PARAMS* params = (CUSTOM_PARAMS*)custom_params;

	ExecutionContext* ctx = (ExecutionContext*)params->tdata->PrivateStack;
	size_t eip = params->MyDisasm->EIP;
	Int32 branch_type = params->MyDisasm->Instruction.BranchType;
	size_t value = instruction_value(params->MyDisasm);

	char* complete_instruction = params->MyDisasm->CompleteInstr;

	char instr_bytes[256] = { 0 };
	char temp[MAX_PATH] = { 0 };
	size_t len = params->instrlen;
    
	PluginReport* report = (PluginReport*)VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
	if (!report)
		return nullptr;

    char* content = (char*)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
	if (!content)
		return report;

    if (params->stack_trace)
        StackTrace(ctx, content);
    else
        memset(content, 0, 0x4000);

#ifndef _M_X64
    sprintf(temp, "%08X: ", (int)params->MyDisasm->VirtualAddr);
#else
    sprintf(temp, "%08X%08X: ", (DWORD)(MyDisasm->VirtualAddr >> 32), (DWORD)(MyDisasm->VirtualAddr & 0xFFFFFFFF));
#endif
	
    strcat(content, temp);
	for (size_t i = 0; i < len; i++)
        sprintf(instr_bytes + i * 3, "%02X ", *(BYTE*) (eip + i));

    sprintf(temp, "%-*s : %s", 45, instr_bytes, complete_instruction);
    strcat(content, temp);

	//auto instruction = Instruction(eip, complete_instruction, branch_type, len, value);
	//pfg->add(instruction);
    
	// pack the plugin response
    report->plugin_name = "DBTTrace"; // plugin name
    report->content_before = content;
    report->content_after = 0;

    return report;
}

PluginReport* DBTBranching(void* custom_params, PluginLayer** layers)
{
	log_info("am apelat dbtbranching");
	return NULL;
}

PluginReport* DBTAfterExecute(void* custom_params, PluginLayer** layers)
{
	log_info("am apelat dbtafterexecute");
	return NULL;
}

static int generate_control_flow_graph()
{
	fstream out("partiaflowgraph.dot", fstream::out);
	if (!out) {
		log_error("cannot open partialflowgraph.dot : %s", strerror(errno));
		return EFAULT;
	}

	auto graphviz = pfg->graphviz();
	int err = pfg->generate(graphviz, &out);
	if (err != 0) {
		log_error("cannot generate partial flow graph");
		return err;
	}
	
	return 0;
}

PluginReport* DBTFinish()
{
	log_info("[DBTITrace] Finish is called");
	return NULL;
	//
	//auto plugin = (PluginReport*)
	//	VirtualAlloc(0, sizeof(PluginReport), MEM_COMMIT, PAGE_READWRITE);
	//
	//if (!plugin) {
	//	log_error("Finish plugin virtual allocation failed");
	//	return plugin;
	//}

	//if (generate_control_flow_graph() != 0)
	//	return plugin;

	//BYTE *cfg_shared_mem = CFG(engine_share_buff);
	//auto from = PartialFlowGraph();

	//size_t size = cfg_buf_size();
	//int err = from.deserialize(cfg_shared_mem, size);
	//if (err != 0) {
	//	log_error("cannot deserialize from shard buffer engine a pfg");
	//	return plugin;
	//}

	//err = pfg->merge(from);
	//if (err != 0) {
	//	log_error("cannot merge two partial flow graphs");
	//	return plugin;
	//}

	//err = pfg->serialize(cfg_shared_mem, size);
	//if (err != 0) {
	//	log_error("cannot serialize from pfg to shared buffer engine");
	//	return plugin;
	//}

	//// pack the plugin response
	//plugin->plugin_name = "DBTTrace";
	//plugin->content_after = 0;

	//return plugin;
}