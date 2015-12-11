#pragma once

#include <luajit-2.0/lua.hpp>

#include "shim_builtin_defs.hpp"

namespace Shim
{

namespace detail
{

template<typename T>
inline constexpr bool is_integral()
{
    return (std::is_integral<T>::value && !is_boolean<T>()) && (
        (std::is_unsigned<T>::value && sizeof(T) < sizeof(lua_Integer)) ||
        (std::is_signed<T>::value && sizeof(T) <= sizeof(lua_Integer)));
}

template<typename T>
inline constexpr bool is_floating_point()
{ return std::is_floating_point<T>::value && sizeof(T) <= sizeof(lua_Number); }

template<typename T>
inline constexpr bool is_numeric()
{ return is_integral<T>() || is_floating_point<T>(); }

template<typename T>
inline constexpr bool is_builtin()
{ return is_boolean<T>() || is_numeric<T>() || is_string<T>(); }

} // namespace detail

namespace traits
{

template<typename T>
struct trait<T, util::enable_if<detail::is_integral<T>()>>
{ using tag = tags::integral; };

template<typename T>
struct trait<T, util::enable_if<detail::is_floating_point<T>()>>
{ using tag = tags::floating_point; };

template<>
struct trait<bool>
{ using tag = tags::boolean; };

template<>
struct trait<std::string>
{ using tag = tags::string; };

template<>
struct trait<const char*>
{ using tag = tags::cstring; };

template<typename T>
struct type_code<T, util::enable_if<detail::is_numeric<T>()>>
{ static constexpr int value = LUA_TNUMBER; };

template<>
struct type_code<bool>
{ static constexpr int value = LUA_TBOOLEAN; };

template<typename T>
struct type_code<T, util::enable_if<detail::is_string<T>()>>
{ static constexpr int value = LUA_TSTRING; };

} // namespace traits

namespace impl
{

// all builtin types

template<typename T>
inline bool is(tags::builtin, lua_State* L, int n)
{ return lua_type(L, n) == traits::type_code<T>::value; }

template<typename T>
inline std::string type_name(tags::builtin, lua_State* L)
{ return lua_typename(L, traits::type_code<T>::value); }

// integral types

template<typename T>
inline void push(tags::integral, lua_State* L, T val)
{ lua_pushinteger(L, val); }

template<typename T>
inline T cast(tags::integral, lua_State* L, int n)
{ return lua_tointeger(L, n); }

// floating point types

template<typename T>
inline void push(tags::floating_point, lua_State* L, T val)
{ lua_pushnumber(L, val); }

template<typename T>
inline T cast(tags::floating_point, lua_State* L, int n)
{ return lua_tonumber(L, n); }

// boolean types

template<typename T>
inline void push(tags::boolean, lua_State* L, T val)
{ lua_pushboolean(L, val); }

template<typename T>
inline T cast(tags::boolean, lua_State* L, int n)
{ return lua_toboolean(L, n); }

// string types

template<typename T>
inline void push(tags::string, lua_State* L, T val)
{ lua_pushlstring(L, val.c_str(), val.size()); }

template<typename T>
inline T cast(tags::string, lua_State* L, int n)
{
    size_t len;
    return T(lua_tolstring(L, n, &len), len);
}

// cstring types

template<typename T>
inline void push(tags::cstring, lua_State* L, T val)
{ lua_pushstring(L, val); }

template<typename T>
inline T cast(tags::cstring, lua_State* L, int n)
{ return lua_tostring(L, n); }

} // namespace impl

} // namespace Shim
