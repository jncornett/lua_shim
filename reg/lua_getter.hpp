#pragma once

#include "shim_dispatchx.hpp"

namespace Reg
{

namespace util
{

using namespace Shim;

struct StackGetter
{
    lua_State* L;

    template<typename T>
    T get(int n)
    { return stack::getx<T>(L, n); }
};

} // namespace util

}
