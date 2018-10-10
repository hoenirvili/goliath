#include <catch2/catch.hpp>
#include <cfgtrace.h>
#include <cfgtrace/api/types.h>
#include <cfgtrace/engine/engine.h>
#include <cfgtrace/logger/logger.h>
#include <sstream>
#include <string_view>
#include <windows.h>

class virtual_memory
{
public:
    HANDLE handler;
    unsigned char *file_view;
    const size_t size = engine::engine::BUFFER_SIZE;
    virtual_memory()
    {
        /**
         * CreateFileMappingA
         * creates a file mapping object of a specified size that is backed by
         * the system paging file instead of by a file in the file system
         */
        this->handler = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, this->size, memsharedname);
        if (!this->handler)
            throw std::runtime_error("CreateFileMappingA");

        /**
         * in oder to write to that memory region we need
         * to first create a view into that handler
         */
        LPVOID view = MapViewOfFile(this->handler, FILE_MAP_ALL_ACCESS, 0, 0, this->size);
        if (!view) {
            CloseHandle(this->handler);
            throw std::runtime_error("MapViewOfFile");
        }
        this->file_view = static_cast<unsigned char *>(view);
    }

    void enable_log_name()
    {
        if (!this->file_view)
            throw std::runtime_error("file_view has not been mapped");
        /**
         * write the first bytes the test log file name
         */
        const char *file_log = "test_log_file.log";
        size_t file_log_size = strlen(file_log);
        memcpy(this->file_view, file_log, file_log_size);
        /**
         * we are done writing to that memory region
         * make room for other processes to view in there
         */
        UnmapViewOfFile(this->file_view);
        this->file_view = nullptr;
    }

    ~virtual_memory()
    {
        if (this->file_view)
            UnmapViewOfFile(this->file_view);

        CloseHandle(this->handler);
    }
};

TEST_CASE("The plugin is assumed to be run in layer 2", "[GetLayer]")
{
    size_t layer = GetLayer();
    REQUIRE(layer == PLUGIN_LAYER);
}

TEST_CASE("When the internal virtual memory is not initialised by the engine", "[DBTInit]")
{
    BOOL state = DBTInit();
    REQUIRE(state == FALSE);
}

class fake_output_streamer
{
private:
    std::stringbuf buffer;

public:
    fake_output_streamer() = default;
    ~fake_output_streamer() = default;

    std::ostream *writer() noexcept
    {
        return new std::ostream(&this->buffer);
    }

    void check(std::string_view message)
    {
        std::string buff = this->buffer.str();
        auto n = buff.find(message, 0);
        auto state = (n != std::string::npos);
        REQUIRE(state == true);
    };
};

TEST_CASE("When the internal virtual memory has been initialised by the engine", "[DBTInit]")
{
    virtual_memory vm = virtual_memory();

    SECTION("file_mapping is now constructed but log name is not available")
    {
        BOOL state = DBTInit();
        REQUIRE(state == FALSE);
    }

    vm.enable_log_name();

    // create a new fake output streamer to redirect logger messages
    fake_output_streamer fom = fake_output_streamer();
    std::ostream *w = fom.writer();
    logger::init(w);

    SECTION("file_mapping is constructed and log name is available")
    {
        BOOL state = DBTInit();
        REQUIRE(state == TRUE);
        fom.check("[CFGTrace] Init is called");
        fom.check("[CFGTrace] [CFGTrace] Iinit is called for iteration 1");
    }

    logger::clean();
}