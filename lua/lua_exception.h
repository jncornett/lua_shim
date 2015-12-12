#pragma once

#include <sstream>
#include <string>

namespace Lua
{

class Exception
{
public:
    virtual ~Exception() { }
    virtual std::string what() const = 0;
};

class TypeError : public Exception
{
public:
    TypeError(int index, std::string expected, std::string actual) :
        index(index), expected(expected), actual(actual) { }

    std::string what() const override
    {
        std::ostringstream os;
        os << "TypeError: (arg #" << index << ")" <<
            " expected '" << expected << "'" <<
            ", got '" << actual << "'";

        return os.str();
    }

private:
    int index;
    std::string expected;
    std::string actual;
};

}
