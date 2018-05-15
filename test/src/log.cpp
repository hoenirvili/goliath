#include <sstream>
#include <unordered_map>
#include <gtest/gtest.h>

#include "src/log.hpp"

using namespace std;

TEST(Log, init)
{
	auto* out = new stringstream();
	EXPECT_NO_THROW(Log::init(out));
}

struct write_params {
	Log::level l;
	const char* file;
	const int line;
	const char* function;
	const char* format;
};

struct test_write {
	write_params params;
	const char* expected;
};

static const test_write tests[] =
{ 
	{
		{ Log::level::error, "file_test", 5, "test", "test message" },
		"file_test:5:test |ERROR| test message\n"
	},
	{
		{ Log::level::warning, "file_test", 5, "test", "test message" },
		"file_test:5:test |ERROR| test message\nfile_test:5:test |WARNING| test message\n"
	},
	{
		{ Log::level::info, "file_test", 5, "test", "test message" },
		"file_test:5:test |ERROR| test message\nfile_test:5:test |WARNING| test message\nfile_test:5:test |INFO| test message\n"
	},
};

TEST(Log, write)
{
	auto* out = new stringstream();
	EXPECT_NO_THROW(
		Log::init(out)
	);

	for (const auto& test : tests) {
		auto params = test.params;
		auto expected = test.expected;
		EXPECT_NO_THROW(
			Log::write(
				params.l, params.file, params.line, 
				params.function, params.format
			)
		);

		auto str = out->str();
		auto got = str.c_str();
		EXPECT_STREQ(got, expected);
	}
}
