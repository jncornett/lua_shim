#pragma once

#include "shim_defs.h"
#include "shim_types.h"

namespace Lua
{

namespace traits
{

using namespace util;

template<typename T>
struct trait<T, enable_for<is_unsigned_int<T>()>>
{ using tag = tags::unsigned_integral; };

template<typename T>
struct trait<T, enable_for<is_int<T>() and !is_unsigned_int<T>()>>
{ using tag = tags::integral; };

template<typename T>
struct trait<T, enable_for<is_fp<T>()>>
{ using tag = tags::floating_point; };

template<typename T>
struct trait<T, enable_for<is_bool<T>()>>
{ using tag = tags::boolean; };

template<typename T>
struct trait<T, enable_for<is_std_str<T>()>>
{ using tag = tags::std_string; };

template<typename T>
struct trait<T, enable_for<is_c_str<T>()>>
{ using tag = tags::c_string; };

template<typename T>
struct lua_type_code<T, enable_for<is_num<T>()>>
{ static constexpr auto value = LUA_TNUMBER; };

template<typename T>
struct lua_type_code<T, enable_for<is_bool<T>()>>
{ static constexpr auto value = LUA_TBOOLEAN; };

template<typename T>
struct lua_type_code<T, enable_for<is_str<T>()>>
{ static constexpr auto value = LUA_TSTRING; };

} // namespace traits

namespace impl
{

// all builtin types

template<typename T>
inline bool is(tags::builtin, lua_State* L, int n)
{ return lua_type(L, n) == traits::lua_type_code<T>::value; }

// integral types

template<typename T>
inline std::string type_name(tags::integral)
{ return "integer"; }

template<typename T>
inline std::string type_name(tags::unsigned_integral)
{ return "unsigned"; }

// special type-checking overload for unsigned
template<typename T>
inline bool is(tags::unsigned_integral, lua_State* L, int n)
{
    if ( !is<T>(tags::integral(), L, n) )
        return false;

    return lua_tointeger(L, n) > 0;
}

template<typename T>
inline void push(tags::integral, lua_State* L, T val)
{ lua_pushinteger(L, val); }

template<typename T>
inline T cast(tags::integral, lua_State* L, int n)
{ return lua_tointeger(L, n); }

// floating point types

template<typename T>
inline std::string type_name(tags::floating_point)
{ return "number"; }

template<typename T>
inline void push(tags::floating_point, lua_State* L, T val)
{ lua_pushnumber(L, val); }

template<typename T>
inline T cast(tags::floating_point, lua_State* L, int n)
{ return lua_tonumber(L, n); }

// boolean types

template<typename T>
inline std::string type_name(tags::boolean)
{ return "boolean"; }

template<typename T>
inline void push(tags::boolean, lua_State* L, T val)
{ lua_pushboolean(L, val); }

template<typename T>
inline T cast(tags::boolean, lua_State* L, int n)
{ return lua_toboolean(L, n); }

// string types

template<typename T>
inline std::string type_name(tags::string)
{ return "string"; }

template<typename T>
inline void push(tags::std_string, lua_State* L, T val)
{ lua_pushlstring(L, val.c_str(), val.size()); }

template<typename T>
inline void push(tags::c_string, lua_State* L, T val)
{ lua_pushstring(L, val); }

template<typename T>
inline T cast(tags::std_string, lua_State* L, int n)
{
    size_t len;
    return T(lua_tolstring(L, n, &len), len);
}

template<typename T>
inline T cast(tags::c_string, lua_State* L, int n)
{ return lua_tostring(L, n); }

} // namespace impl

}
