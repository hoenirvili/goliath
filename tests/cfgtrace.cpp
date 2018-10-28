#include "test/fake_graph.h"
#include "test/fake_output_streamer.h"
#include "test/virtual_memory.h"
#include <catch2/catch.hpp>
#include <cfgtrace.h>
#include <cfgtrace/engine/engine.h>
#include <cfgtrace/graph/graph.h>
#include <cfgtrace/logger/logger.h>
#include <typeinfo>

TEST_CASE("The plugin is assumed to be run in layer 2", "[GetLayer]")
{
    auto layer = GetLayer();
    REQUIRE(layer == PLUGIN_LAYER);
}

TEST_CASE("Test different states of initialisation", "[DBTInit]")
{
    SECTION("When the virtual memory is not initiliased")
    {
        BOOL state = DBTInit();
        REQUIRE_FALSE(state);
    }

    SECTION("When the virtual memory is set but the engine fails to initialise")
    {
        auto vm = virtual_memory();
        engine::custom_creation([](HANDLE file_mapping) -> engine::engine * {
            REQUIRE(file_mapping != nullptr);
            return nullptr;
        });
        BOOL state = DBTInit();
        REQUIRE_FALSE(state);

        engine::custom_creation(nullptr);
    }

    SECTION("When the engine is set but the logger fails to initialise")
    {
        auto vm = virtual_memory();
        vm.enable_log_name();
        std::string logger_name;
        auto fail = [&logger_name](const char *name) -> std::ostream * {
            REQUIRE(name != nullptr);
            logger_name = name;
            return nullptr;
        };
        logger::custom_creation(fail);

        BOOL state = DBTInit();
        REQUIRE_FALSE(state);
        REQUIRE(logger_name == vm.logger_name());
        engine::clean();
        logger::custom_creation(nullptr);
    }

    SECTION("When the engine, logger is set but the control flow graph fails to initialise")
    {
        auto vm = virtual_memory();
        vm.enable_log_name();
        auto fos = fake_output_streamer();
        logger::custom_creation(std::bind(&fake_output_streamer::writer, &fos, std::placeholders::_1));
        graph::custom_creation([]() -> graph::graph * { return nullptr; });

        BOOL state = DBTInit();
        REQUIRE_FALSE(state);

        std::string got = fos.name();
        auto expected = vm.logger_name();
        REQUIRE(got == expected);

        fos.contains("[CFGTrace] DBTInit engine and logger state are initiliased");

        engine::clean();
        logger::clean();
        logger::custom_creation(nullptr);
        graph::custom_creation(nullptr);
    }

    SECTION("When the engine, logger and control flow graph is initialised")
    {
        auto vm = virtual_memory();
        vm.enable_log_name();
        auto fos = fake_output_streamer();
        logger::custom_creation(std::bind(&fake_output_streamer::writer, &fos, std::placeholders::_1));

        BOOL state = DBTInit();
        REQUIRE(state == TRUE);

        fos.contains("[CFGTrace] DBTInit engine and logger state are initiliased");
        fos.contains("[CFGTrace] Init is called for iteration 1");

        auto it = vm.iteration_count();
        REQUIRE(it == 1);

        engine::clean();
        logger::clean();
        graph::clean();
        logger::custom_creation(nullptr);
    }

    SECTION("When the state is initialised and the logger is at iteration > 0")
    {
        auto vm = virtual_memory();
        vm.enable_log_name();
        auto fos = fake_output_streamer();
        logger::custom_creation(std::bind(&fake_output_streamer::writer, &fos, std::placeholders::_1));
        graph::custom_creation([]() -> graph::graph * {
            auto fk = new fake_graph();
            fk->_read = [](const std::byte *from) { REQUIRE(from != nullptr); };
            return fk;
        });

        // set iteration to be > 0
        vm.set_iteration_count(1);

        BOOL state = DBTInit();
        REQUIRE(state == TRUE);

        fos.contains("[CFGTrace] DBTInit engine and logger state are initiliased");
        fos.contains("[CFGTrace] Init is called for iteration 2");

        auto it = vm.iteration_count();
        REQUIRE(it == 2);

        // check instance graph
        auto g = graph::instance();
        auto is_fake_graph = dynamic_cast<fake_graph *>(g);
        REQUIRE(is_fake_graph != nullptr);

        engine::clean();
        logger::clean();
        graph::clean();
        logger::custom_creation(nullptr);
        graph::custom_creation(nullptr);
    }
}

// TEST_CASE("When the internal state is initiliased", "[DBTInit]")
// {
//     auto vm = virtual_memory();        // setup the shared virtual memory
//     vm.enable_log_name();              // write the logger name at the start of the block
//     auto fos = fake_output_streamer(); // create a new logger mock
//     auto fp = std::bind(&fake_output_streamer::writer, &fos, std::placeholders::_1);
//     logger::custom_creation(fp); // bind the fake logger

//     BOOL state = DBTInit();
//     REQUIRE(state == TRUE);

//     // check if the logger's name is initliased correctly
//     std::string name = fos.name();
//     auto logger_name = vm.logger_name();
//     REQUIRE(name == logger_name);

//     // check the iteration count of the algorithm
//     auto it = vm.iteration_count();
//     REQUIRE(it == 1);

//     // check the logger messages
//     fos.contains("[CFGTrace] DBTInit engine and logger state are initiliased");
//     fos.contains("[CFGTrace] Init is called for iteration 1");

//     // clean the graph and logger state
//     logger::clean();
//     graph::clean();
// }

// TEST_CASE("When the graph initialisation fails", "[DBTInit]")
// {
//     // setup all the mocks
//     auto vm = virtual_memory();
//     vm.enable_log_name();
//     auto fos = fake_output_streamer();
//     auto fp = std::bind(&fake_output_streamer::writer, &fos, std::placeholders::_1);
//     logger::custom_creation(fp);
//     // make the graph to fails
//     auto fail = []() -> graph::control_flow * { return nullptr; };
//     graph::custom_creation(fail);

//     BOOL state = DBTInit();
//     REQUIRE(state == FALSE);

//     // check the logger name is initliased correctly
//     std::string name = fos.name();
//     auto logger_name = vm.logger_name();
//     REQUIRE(name == logger_name);

//     // check the iteration count
//     // TODO(hoenir): why this is 1? why it's not 0?
//     auto it = vm.iteration_count();
//     REQUIRE(it == 0);

//     // clean the graph and logger
//     logger::clean();
// }

// TEST_CASE("Initilise only the logger", "[DBTInit]")
// {
// }

// TEST_CASE("When the internal virtual memory is initialised", "[DBTInit]")
// {
//     virtual_memory vm = virtual_memory();

//     SECTION("file_mapping is now constructed but log name is not available")
//     {
//         BOOL state = DBTInit();
//         REQUIRE(state == FALSE);
//         auto it = vm.interation_count();
//         REQUIRE(it == 0);
//     }
// }

// TEST_CASE("When the internal virtual memory log is initialised", "[DBTInit]")
// {
//     // enable the virtual memory map and assign at the start a logger file name
//     virtual_memory vm = virtual_memory();
//     vm.enable_log_name();

//     // create buffer and out streamer
//     auto buffer = std::stringbuf();
//     auto out_ptr = std::make_unique<std::ostream>(&buffer);
//     fake_output_streamer fom = fake_output_streamer();

//     // we should be sure the the logger is not already set
//     auto state = logger::is_writer_set();
//     REQUIRE_FALSE(state);

//     // assign the logger
//     logger::set_writer(out_ptr.get());

//     // l if the logger is set
//     state = logger::is_writer_set();
//     REQUIRE(state);

//     SECTION("file_mapping is constructed and log name is available")
//     {
//         BOOL state = DBTInit();
//         REQUIRE(state);
//         auto content = buffer.str();
//         REQUIRE_THAT(content, Contains("[CFGTrace] Init is called"));
//         REQUIRE_THAT(content, Contains("[CFGTrace] Init is called for iteration 1"));
//         auto it = vm.interation_count();
//         REQUIRE(it == 1);
//     }
// }

// TEST_CASE("When the internal virtual memory has a log and is initialised", "[DBTInit]")
// {
//     virtual_memory vm = virtual_memory();
//     vm.enable_log_name();
//     fake_output_streamer fom = fake_output_streamer();
//     bool is_set = logger::is_writer_set();
//     REQUIRE(is_set == false);
//     std::ostream *w = fom.writer();
//     logger::set_writer(w);
//     vm.set_iteration_count(1);
//     SECTION("load empty control flow graph from memory")
//     {
//         BOOL state = DBTInit();
//         REQUIRE(state == TRUE);
//         // fom.check("[CFGTrace] Init is called");
//         // fom.check("[CFGTrace] Init is called for iteration 2");
//         auto it = vm.interation_count();
//         REQUIRE(it == 2);
//     }
//     logger::unset_writer();
// }

// TEST_CASE("Append instruction in before", "[DBTBeforeExecute]")
// {
//     auto cpb = std::make_unique<custom_params>(0x55232288, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 2, 0);
//     auto plb = std::make_unique<plugin_layer>(2, "PFGTrace");

//     auto params = cpb->get();
//     auto layers = plb->get();
//     auto rptr = DBTBeforeExecute(params, layers);
//     REQUIRE(rptr != nullptr);
//     REQUIRE(rptr->content_after == nullptr);
//     REQUIRE(rptr->content_before != nullptr);
//     REQUIRE(rptr->plugin_name != nullptr);
//     auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
//     REQUIRE(compare == true);
//     auto report_ptr = plugin_report_ptr(rptr);
// }

// TEST_CASE("Append branch instruction in before", "[DBTBeforeExecute]")
// {
//     auto cpb =
//       std::make_unique<custom_params>(0x55232288, "call 0x312512", CallType, 4, 0x55232288 + 150, 0x55232288 + 2);
//     auto plb = std::make_unique<plugin_layer>(2, "OtherPlugin");

//     auto params = cpb->get();
//     auto layers = plb->get();
//     auto rptr = DBTBeforeExecute(params, layers);
//     REQUIRE(rptr != nullptr);
//     REQUIRE(rptr->content_after == nullptr);
//     REQUIRE(rptr->content_before != nullptr);
//     REQUIRE(rptr->plugin_name != nullptr);
//     auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
//     REQUIRE(compare == true);
//     auto report_ptr = plugin_report_ptr(rptr);
// }

// using vector_of_params = std::vector<custom_params_ptr>;

// TEST_CASE("Append multiple instructions in before", "[DBTBeforeExecute]")
// {
//     vector_of_params params;
//     params.emplace_back(std::make_unique<custom_params>(0x10000000, "mov eax, ebx", NO_BRANCH, 2, 0x1000000 + 2, 0));
//     params.emplace_back(std::make_unique<custom_params>(0x10000002, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 150,
//     0)); params.emplace_back(std::make_unique<custom_params>(0x10000004, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 +
//     150, 0)); params.emplace_back(std::make_unique<custom_params>(0x10000006, "mov eax, ebx", NO_BRANCH, 2,
//     0x55232288 + 150, 0)); params.emplace_back(
//       std::make_unique<custom_params>(0x10000008, "call 0x10000008", CallType, 4, 0x10000008 + 92, 0x10000008 + 2));
//     params.emplace_back(
//       std::make_unique<custom_params>(0x10000100, "add ecx, eax", NO_BRANCH, 2, 0x55232288 + 150, 0x55232288 + 2));
//     params.emplace_back(std::make_unique<custom_params>(0x10000102, "ret", RetType, 2, 0, 0));
//     params.emplace_back(
//       std::make_unique<custom_params>(0x10000010, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 150, 0x55232288 + 2));

//     auto plb = std::make_unique<plugin_layer>(2, "OtherPlugin");
//     auto layers = plb->get();

//     for (const auto &param : params) {
//         auto p = param->get();
//         auto rptr = DBTBeforeExecute(p, layers);
//         REQUIRE(rptr != nullptr);
//         REQUIRE(rptr->content_after == nullptr);
//         REQUIRE(rptr->content_before != nullptr);
//         REQUIRE(rptr->plugin_name != nullptr);
//         auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
//         REQUIRE(compare == true);
//         auto report_ptr = plugin_report_ptr(rptr);
//     }
// }

// TEST_CASE("Append instruction", "[DBTBranching]")
// {
//     auto cpb = std::make_unique<custom_params>(0x55232288, "mov eax, ebx", NO_BRANCH, 2, 0x55232288 + 2, 0);
//     auto plb = std::make_unique<plugin_layer>(2, "PFGTrace");

//     auto params = cpb->get();
//     auto layers = plb->get();
//     auto rptr = DBTBranching(params, layers);
//     REQUIRE(rptr != nullptr);
//     REQUIRE(rptr->content_after == nullptr);
//     REQUIRE(rptr->content_before != nullptr);
//     REQUIRE(rptr->plugin_name != nullptr);
//     auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
//     REQUIRE(compare == true);
//     auto report_ptr = plugin_report_ptr(rptr);
// }

// TEST_CASE("Append branch instruction", "[DBTBranching]")
// {
//     auto cpb =
//       std::make_unique<custom_params>(0x55232288, "call 0x312512", CallType, 4, 0x55232288 + 150, 0x55232288 + 2);
//     auto plb = std::make_unique<plugin_layer>(2, "OtherPlugin");

//     auto params = cpb->get();
//     auto layers = plb->get();
//     auto rptr = DBTBranching(params, layers);
//     REQUIRE(rptr != nullptr);
//     REQUIRE(rptr->content_after == nullptr);
//     REQUIRE(rptr->content_before != nullptr);
//     REQUIRE(rptr->plugin_name != nullptr);
//     auto compare = !strcmp(rptr->plugin_name, "CFGTrace");
//     REQUIRE(compare == true);
//     auto report_ptr = plugin_report_ptr(rptr);
// }