#include <sstream>

#include <gtest/gtest.h>

#include "src/log.hpp"

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


TEST(log, constructor)
{
    auto *ss = new std::stringstream();
    Log first("FIRST_TEST", ss);
    Log second("SECOND_TEST");
    first.info("first");
    second.info("second");
    auto str = ss->str();
    auto got = str.c_str();
    EXPECT_STREQ(got, "|SECOND_TEST| - |INFO| - first\n|SECOND_TEST| - |INFO| - second\n");

    auto *last = new std::stringstream();
    Log third("THIRD_TEST", last);
    third.info("third");
    str = last->str();
    got = str.c_str();
    EXPECT_STREQ(got, "|THIRD_TEST| - |INFO| - third\n");


}
