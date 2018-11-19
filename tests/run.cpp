#include "machinery.h"
#include "test/custom_params.h"
#include "test/fake_definition.h"
#include "test/fake_graph.h"
#include "test/fake_output_streamer.h"
#include "test/plugin_layer.h"
#include "test/plugin_report.h"
#include "test/virtual_memory.h"

#include <cfgtrace.h>
#include <cfgtrace/assembly/instruction.h>
#include <cfgtrace/engine/engine.h>
#include <cfgtrace/graph/control_flow.h>
#include <cfgtrace/graph/graph.h>
#include <cfgtrace/logger/logger.h>

#include <catch2/catch.hpp>
#include <memory>

struct fake_control_flow : public graph::control_flow {
    fake_control_flow() = default;
    ~fake_control_flow() = default;

    definition::definition *generate(definition::FORMAT format) override;
};

definition::definition *fake_control_flow::generate(definition::FORMAT format)
{
    auto def = __super::generate(format);
    REQUIRE(def != nullptr);
    return new fake_definition(); // this is no-op
}

TEST_CASE("Test engine single run without adding any single branch")
{
    // initialise the state
    auto vm = virtual_memory();
    vm.enable_log_name();
    auto fos = fake_output_streamer();
    logger::custom_creation(
      std::bind(&fake_output_streamer::writer, &fos, std::placeholders::_1));
    graph::custom_creation(
      []() -> graph::graph * { return new fake_control_flow(); });

    // call the starting function
    BOOL state = DBTInit();
    REQUIRE(state == TRUE);

    fos.contains("[CFGTrace] DBTInit engine and logger state are initiliased");
    fos.contains("[CFGTrace] Init is called for iteration [1]");

    auto it = vm.iteration_count();
    REQUIRE(it == 1);

    // call the ending function
    auto report = DBTFinish();
    REQUIRE(report != nullptr);
    FREE_REPORT(report);

    fos.contains("[CFGTrace] Finish is called at iteration [1]");
    logger::custom_creation(nullptr);
    graph::custom_creation(nullptr);
}

TEST_CASE("Test engine single run adding multiple instructions")
{
    auto vm = virtual_memory();
    vm.enable_log_name();
    auto fos = fake_output_streamer();
    logger::custom_creation(
      std::bind(&fake_output_streamer::writer, &fos, std::placeholders::_1));

    // first run
    graph::custom_creation(
      []() -> graph::graph * { return new fake_control_flow(); });

    // call the starting function
    BOOL state = DBTInit();
    REQUIRE(state == TRUE);

    fos.contains("[CFGTrace] DBTInit engine and logger state are initiliased");
    fos.contains("[CFGTrace] Init is called for iteration [1]");

    auto it = vm.iteration_count();
    REQUIRE(it == 1);

    // use the same layer on all calls
    auto layers = plugin_layer(
      {{1, "PluginOne", nullptr, nullptr}, {2, "PluginTwo", nullptr, nullptr}});

    /*

    first run
    +-------------------+
    |  0x00776611       |
    |                   +-----------+
    |  call 0x00776614  |           |
    +--------+----------+           |
             |                      |
             |               +------v--------+
     +-------v--------+      |  0x00776613   |
     | 0x00776614     |      |               |
     | xor eax, eax   |      +---------------+
     | ret            |
     |                |
     +----------------+

    */

    auto params = std::make_unique<custom_params>(
      0x00776611, "call 0x00776614", CallType, 1, 0x00776614, 0x00776613);

    auto report = DBTBranching(params->get(), layers.get());
    REQUIRE(report != nullptr);
    delete report;

    report = DBTBeforeExecute(params->get(), layers.get());
    REQUIRE(report != nullptr);
    delete report;

    params = std::make_unique<custom_params>(0x00776614, "xor eax, eax",
                                             NO_BRANCH, 2, 0x00776616, 0);

    report = DBTBeforeExecute(params->get(), layers.get());
    REQUIRE(report != nullptr);
    delete report;

    params = std::make_unique<custom_params>(0x00776616, "ret", RetType, 1,
                                             0x00776613, 0);

    report = DBTBranching(params->get(), layers.get());
    REQUIRE(report != nullptr);
    delete report;

    report = DBTBeforeExecute(params->get(), layers.get());
    REQUIRE(report != nullptr);
    delete report;

    // call the ending function
    report = DBTFinish();
    REQUIRE(report != nullptr);
    delete report;

    fos.contains("[CFGTrace] Finish is called at iteration [1]");
    logger::custom_creation(nullptr);
    graph::custom_creation(nullptr);
}

TEST_CASE("Test engine multiple runs adding multiple instructions")
{
    /*

        first run
        +-------------------+
        |  0x00776611       |
        |                   +-----------+
        |  call 0x00776614  |           |
        +--------+----------+           |
                 |                      |
                 |               +------v--------+
         +-------v--------+      |  0x00776613   |
         | 0x00776614     |      |               |
         | xor eax, eax   |      +---------------+
         | ret            |
         |                |
         +----------------+

          second run
          +-----------------+
          |  0x00776611     |
          |                 +-----------------+
          |  call 0x00776614|                 |
          +--------+--------+                 |
                   |                          |
          +--------v--------+          +------v-------+
          | 0x00776614      |          | 0x00776613   |
          |                 |          |              |
          |                 |          | push ebp     |
          +-----------------+          | sub esp, 0x4 |
                                       | ret          |
                                       +--------------+
    */

    auto vm = virtual_memory();
    vm.enable_log_name();
    auto fos = fake_output_streamer();

    graph::custom_creation(
      []() -> graph::graph * { return new fake_control_flow(); });

    auto m = machinery();
    m.add_single_layer(
      {{1, "PluginOne", nullptr, nullptr}, {2, "PluginTwo", nullptr, nullptr}});

    m.add_custom_params(
      // start
      {
        // first run
        {{0x00776611, "call 0x00776614", CallType, 1, 0x00776614, 0x00776613},
         {0x00776614, "xor eax, eax", NO_BRANCH, 2, 0x00776616, 0},
         {0x00776616, "ret", RetType, 1, 0x00776613, 0}},

        // second run
        {{0x00776611, "call 0x00776614", CallType, 1, 0x00776613, 0x00776614},
         {0x00776613, "push ebp", NO_BRANCH, 1, 0x00776614, 0}}
        // end
      });

    m.run_before_dbtinit = [&fos]() {
        logger::custom_creation(std::bind(&fake_output_streamer::writer, &fos,
                                          std::placeholders::_1));
    };

    m.run_after_dbtinit = [&fos, &vm](size_t it) {
        fos.contains(
          "[CFGTrace] DBTInit engine and logger state are initiliased");
        switch (it) {
        case 1:
            fos.contains("[CFGTrace] Init is called for iteration [1]");
            break;
        case 2:
            fos.contains("[CFGTrace] Init is called for iteration [2]");
            break;
        }
    };

    m.inspect_plugin_report = [](PluginReport *report, size_t it) {
        REQUIRE(report != nullptr);
    };

    m.inspect_finish_report = [](PluginReport *report, size_t it) {
        REQUIRE(report != nullptr);
    };

    m.run_after_dbtfinish = [&fos](size_t it) {
        switch (it) {
        case 1:
            fos.contains("[CFGTrace] Finish is called at iteration [1]");
            break;
        case 2:
            fos.contains("[CFGTrace] Finish is called at iteration [2]");
            break;
        }

        fos.reset(); // make the logger create a new writer
        logger::custom_creation(nullptr);
        graph::custom_creation(nullptr);
    };

    m.start();
}