#pragma once

#include "shim_defs.h"
#include "shim_types.h"

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
    using underlying = typename std::underlying_type<T>::type;
    return lua_type(L, n) != traits::lua_type_code<underlying>::value;
}

template<typename T>
inline std::string type_name(tags::enumeration)
{ return "enumeration"; }

template<typename T>
inline void push(tags::enumeration, lua_State* L, const T val)
{

} // namespace impl


} // namespace Lua

