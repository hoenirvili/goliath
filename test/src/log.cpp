#include "gtest\gtest.h"
#include <src/log.hpp>

TEST(log, GetInstance)
{
    auto* log = Log::GetInstance();
    bool is = (log != nullptr);
    EXPECT_TRUE(is);
}
