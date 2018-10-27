#pragma once

#include <catch2/catch.hpp>
#include <sstream>

class IntRange : public Catch::MatcherBase<int>
{
private:
    int value;

public:
    IntRange(int value) : value(value) {}

    virtual bool match(int const &i) const override { return i == this->value; }

    virtual std::string describe() const
    {
        std::ostringstream ss;
        ss << "the value passed is" << this->value;
        return ss.str();
    }
};
