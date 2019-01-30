#include "goliath.h"
#include "goliath/api/types.h"
#include "goliath/assembly/instruction.h"
#include "goliath/definition/generate.h"
#include "goliath/engine/engine.h"
#include "goliath/graph/control_flow.h"
#include "goliath/logger/logger.h"
#include "goliath/memory/loader.h"

#include <stdexcept>
#include <string>
#include <windows.h>

/**
 *  new_plugin_report it will create a new PluginReport and
 *  fill with all the required information that the engine will recieve
 */
static PluginReport *new_plugin_report() noexcept
{
	char *content = (char *)VirtualAlloc(0, 0x4000, MEM_COMMIT, PAGE_READWRITE);
	if (!content) {
		logger_error("VirtualAlloc failed, cannot alloc a page of size 0x4000");
		return nullptr;
	}
	auto report = new PluginReport;
	report->plugin_name = "Goliath";
	report->content_after = content;
	report->content_before = nullptr;
	return report;
}

size_t GetLayer()
{
	return PLUGIN_LAYER;
}

static size_t current_iteration = 0;

BOOL DBTInit()
{
	engine::engine *e;
	if (!engine::is_initialised()) {
		if (e = engine::instance(); !e)
			return FALSE;
	}

	auto log_name = e->log_name();
	if (!logger::initialise(log_name))
		return FALSE;

	logger_info("[CFGTrace] DBTInit engine and logger state are initiliased");
	graph::graph *g;
	if (!graph::is_initialised()) {
		if (g = graph::instance(); !g)
			return FALSE;
	}

	auto it = e->cfg_iteration();
	if (*it) {
		auto smr = e->cfg_serialize_memory_region();
		memory::loader(g, smr);
	}

	// it will be 1 if we are at the start of the process
	// every node graph that encounters a loop should always check
	// if the node when is created equals to the current iteration count
	(*it)++;
	logger_info("[CFGTrace] Init is called for iteration [%d]", *it);

	current_iteration = *it;
	return TRUE;
}

PluginReport *DBTBeforeExecute(void *params, PluginLayer **layers)
{
	// we know ahead that this is a CUSTOM_PARAMS*
	CUSTOM_PARAMS *cp = static_cast<CUSTOM_PARAMS *>(params);
	assembly::patch_next_and_side_addr(cp); // patch the next and side

	DISASM *d = cp->MyDisasm;

	auto instruction = assembly::instruction(
		d->EIP, d->CompleteInstr, (BRANCH_TYPE)(d->Instruction.BranchType),
		cp->instrlen, cp->next_addr, cp->side_addr);

	/**
	 * The binary that the engine analyses could call external API functions
	 * such as user speace kernel function and we don't want to just reference
	 * those by name and add them as if they were simple instruction. We will
	 * use the information gathered by the APIReporter in order to capture and
	 * extract the information that we need.
	 */
	auto engine = engine::instance();
	auto plugin = engine->plugin_interface("APIReporter", 1, layers);
	if (plugin) {
		// extract the information gathered by the APIReporter
		instruction.api_reporter =
			static_cast<char *>(plugin->data->content_before);
		// we don't want to follow up the instruction, treat it as a simple
		// instruction
		instruction.branch_type = static_cast<BRANCH_TYPE>(0);
	}

	if (instruction.is_branch() && !instruction.is_call())
		return new_plugin_report();

	auto graph = graph::instance();

	try {
		graph->append(instruction, current_iteration);
	}
	catch (const std::exception &ex) {
		logger_error("cannot append instruction, %s", ex.what());
		return nullptr;
	}

	return new_plugin_report();
}

PluginReport *DBTBranching(void *params, PluginLayer **layers)
{
	// we know ahead that this is a CUSTOM_PARAMS*
	CUSTOM_PARAMS *cp = static_cast<CUSTOM_PARAMS *>(params);
	assembly::patch_next_and_side_addr(cp); // path the next and side

	DISASM *d = cp->MyDisasm;

	auto instruction = assembly::instruction(
		d->EIP, d->CompleteInstr, (BRANCH_TYPE)(d->Instruction.BranchType),
		cp->instrlen, cp->next_addr, cp->side_addr);

	/**
	 *  if this is a call instruction skip it
	 *  let the DBTBeforeExecute treat this case
	 */
	if (instruction.is_call())
		return new_plugin_report();
	/**
	 *  This unexpected behaviour is a bug so in order
	 *  to repair this we skip this instruction and let
	 *  the expected DBTBeforeExecute methood treat it
	 *  as a simple instruction
	 */
	if (instruction.is_leave())
		return new_plugin_report();

	auto graph = graph::instance();
	try {
		graph->append(instruction, current_iteration);
	}
	catch (const std::exception &ex) {
		logger_error("cannot append branch instruction %s", ex.what());
		return nullptr;
	}

	return new_plugin_report();
}

PluginReport *DBTFinish()
{
	auto engine = engine::instance();
	auto it = engine->cfg_iteration();
	logger_info("[CFGTrace] Finish is called at iteration [%d]", *it);

	definition::definition *definitions = nullptr;
	PluginReport *report = nullptr;
	auto graph = graph::instance();

	try {
		definitions = definition::generate(graph, GENERATION_FORMAT);
	}
	catch (const std::exception &ex) {
		logger_error("cannot generate definitions, %s", ex.what());
		goto _clean;
	}

	auto to = engine->cfg_serialize_memory_region();
	memory::unloader(graph, to);

	// todo(hoenir): in release mode there should be a flag or a notice by the
	// engine in order to save the last cfg state
#ifndef NDEBUG // run only in debug mode
	try {
		if (definitions)
			definitions->execute();
	}
	catch (const std::exception &ex) {
		logger_error("cannot execute definitions, %s", ex.what());
		goto _clean;
	}
#endif

	report = new_plugin_report();

_clean:
	if (definitions)
		delete definitions;

	graph::clean();
	logger::clean();
	engine::clean();

	return report;
}
