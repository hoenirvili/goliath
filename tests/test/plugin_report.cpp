#include "plugin_report.h"

#include <goliath/api/types.h>

static void _plugin_report_deleter(PluginReport *report)
{
    if (!report)
        return;

    if (report->content_after)
        VirtualFree(report->content_after, 0, MEM_RELEASE);
    if (report->content_before)
        VirtualFree(report->content_before, 0, MEM_RELEASE);

    delete report;
}

void _plugin_report_functor::operator()(PluginReport *report) const
{
    _plugin_report_deleter(report);
};