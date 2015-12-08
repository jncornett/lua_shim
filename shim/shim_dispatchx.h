#pragma once

#include "shim_dispatch.h"
#include "shim_exception.h"

struct lua_State;

namespace Shim
{

namespace stack
{

template<typename T>
inline void check(lua_State* L, int n)
{
    if ( !is<T>(L, n) )
        throw TypeError(
            util::abs_index(lua_gettop(L), n),
            type_name<T>(L),
            lua_typename(L, lua_type(L, n))
        );
}

template<typename T>
inline T getx(lua_State* L, int n)
{
    check<T>(L, n);
    return cast<T>(L, n);
}

} // namespace stack

} // namespace Shim
