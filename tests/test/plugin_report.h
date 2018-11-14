#pragma once

#include <cfgtrace/api/types.h>

#include <memory>

struct _plugin_report_functor {
    void operator()(PluginReport *report) const;
};

using plugin_report_ptr = std::unique_ptr<PluginReport, _plugin_report_functor>;

#define REQUIRE_INSTRUCTION(i, instr_and_or_api_reporter, true, false, branch) \
    do {                                                                       \
        REQUIRE(i.str() == instr_and_or_api_reporter);                         \
        REQUIRE(i.true_branch_address() == true);                              \
        REQUIRE(i.false_branch_address() == false);                            \
        REQUIRE(i.branch_type == branch);                                      \
    } while (0)

#define FREE_REPORT(report) auto __rep = plugin_report_ptr(report)