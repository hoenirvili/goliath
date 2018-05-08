#include <sstream>
#include <gtest/gtest.h>
#include "src/log.hpp"

TEST(log, constructor)
{
	EXPECT_NO_THROW(Log());
}

TEST(log, instance)
{
	auto *ss = new std::stringstream();
	auto logger1 = Log::instance(ss);
	bool expected = (logger1 != nullptr);
	EXPECT_TRUE(expected);

	auto logger2 = Log::instance();
	expected = (logger2 != nullptr);
	EXPECT_TRUE(expected);

	expected = (logger1 == logger2);
	EXPECT_TRUE(expected);

	auto logger3 = Log::instance("");
	expected = (logger3 != nullptr);
	EXPECT_TRUE(expected);

	expected = (logger1 == logger3);
	EXPECT_TRUE(expected);
}

TEST(log, error)
{
    auto *ss = new std::stringstream();

    auto log = Log::instance(ss);
    log->error("test");

	auto str = ss->str();
	auto got = str.c_str();
    EXPECT_STREQ(got, "|ERROR| - test\n");
}


TEST(log, warning)
{
    auto *ss = new std::stringstream();
    auto log = Log::instance(ss);
    log->warning("test");
    
    auto str = ss->str();
    auto got = str.c_str();
    EXPECT_STREQ(got, "|WARNING| - test\n");
}

TEST(log, info)
{
    auto *ss = new std::stringstream();

	auto log = Log::instance(ss);
    log->info("test");

	auto str = ss->str();
    auto got = str.c_str();
    EXPECT_STREQ(got, "|INFO| - test\n");
}

TEST(log, redirect)
{

	auto *ss = new std::stringstream();
	auto log = Log::instance(ss);
	log->info("test");
	auto str = ss->str();
    auto got = str.c_str();
	EXPECT_STREQ(got, "|INFO| - test\n");

	auto *nss = new std::stringstream();
	log->redirect(nss);
	log->info("test");
	str = nss->str();
    got = str.c_str();
	EXPECT_STREQ(got, "|INFO| - test\n");
}

