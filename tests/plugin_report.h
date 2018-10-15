#pragma once
#include <cfgtrace/api/types.h>
#include <memory>

struct _plugin_report_functor {
    void operator()(PluginReport *report) const;
};

using plugin_report_ptr = std::unique_ptr<PluginReport, _plugin_report_functor>;
