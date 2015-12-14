#pragma once

#include <limits>

#include "shim_defs.h"
#include "shim_types.h"
#include "shim_builtin.h"

namespace Lua
{

namespace traits
{

using namespace util;

template<typename T>
struct trait<T, enable_for<is_enum<T>()>>
{ using tag = tags::enumeration; };

} // namespace traits

namespace impl
{

template<typename T>
inline bool is(tags::enumeration, lua_State* L, int n)
{
    using u_type = typename std::underlying_type<T>::type;

    if ( !is<u_type>(tags::integral(), L, n) )
        return false;

    auto val = lua_tointeger(L, n);
    return val >= std::numeric_limits<u_type>::min() and
        val <= std::numeric_limits<u_type>::max();
}

template<typename T>
inline std::string type_name(tags::enumeration)
{ return "enumeration"; }

template<typename T>
inline void push(tags::enumeration, lua_State* L, T val)
{ lua_pushinteger(L, static_cast<lua_Integer>(val)); }

template<typename T>
inline T cast(tags::enumeration, lua_State* L, int n)
{ return static_cast<T>(lua_tointeger(L, n)); }

} // namespace impl

}
