#include <sstream>

#include <gtest/gtest.h>

#include "src/log.hpp"

#include <iostream>
TEST(log, error)
{
    auto *ss = new std::stringstream();

    Log log("TEST_ERROR", ss);
    log.error("test");

    auto str = ss->str();
    auto got = str.c_str();
    EXPECT_STREQ(got, "|TEST_ERROR| - |ERROR| - test\n");
}


TEST(log, warning)
{
    auto *ss = new std::stringstream();

    Log log("TEST_WARNING", ss);
    log.warning("test");

    auto str = ss->str();
    auto got = str.c_str();
    EXPECT_STREQ(got, "|TEST_WARNING| - |WARNING| - test\n");
}


TEST(log, info)
{
    auto *ss = new std::stringstream();

    Log log("TEST_INFO", ss);
    log.info("test");

    auto str = ss->str();
    auto got = str.c_str();
    EXPECT_STREQ(got, "|TEST_INFO| - |INFO| - test\n");
}
