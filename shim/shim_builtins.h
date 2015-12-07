#pragma once

#include <string>

#include "shim_defs.h"
#include "shim_util.h"

#include <luajit-2.0/lua.hpp>

namespace Shim
{

namespace tags
{

struct builtin {};

struct integral : builtin {};
struct floating_point : builtin {};
struct boolean : builtin {};
struct string : builtin {};
struct cstring : builtin {};

}

namespace impl
{

// all builtin types

template<typename T>
inline bool is(tags::builtin, lua_State* L, int n)
{ return lua_type(L, n) == type_code<T>::value; }

template<typename T>
inline std::string type_name(tags::builtin, lua_State* L)
{ return lua_typename(L, type_code<T>::value); }

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

namespace detail
{

template<typename T>
inline constexpr bool is_boolean()
{ return std::is_same<T, bool>::value; }

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
inline constexpr bool is_string()
{
    return std::is_same<T, std::string>::value ||
        std::is_same<T, char const*>::value; 
}

template<typename T>
inline constexpr bool is_builtin()
{ return is_boolean<T>() || is_numeric<T>() || is_string<T>(); }

} // namespace detail

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

} // namespace Shim

#if 0
namespace
{
using namespace Shim;
using std::is_same;


static_assert(is_same<typename trait<int>::tag, tags::integral>::value,"");
static_assert(is_same<typename trait<unsigned>::tag, tags::integral>::value, "");
static_assert(is_same<typename trait<double>::tag, tags::floating_point>::value, "");
static_assert(is_same<typename trait<std::string>::tag, tags::string>::value, "");
static_assert(is_same<typename trait<const char*>::tag, tags::cstring>::value, "");

static_assert(type_code<int>::value == LUA_TNUMBER, "");
static_assert(type_code<double>::value == LUA_TNUMBER, "");
static_assert(type_code<bool>::value == LUA_TBOOLEAN, "");
static_assert(type_code<std::string>::value == LUA_TSTRING, "");
static_assert(type_code<const char*>::value == LUA_TSTRING, "");

template<typename T>
inline void test_is_dispatching()
{
    lua_State* L = nullptr;
    impl::is<T>(typename trait<T>::tag(), 0, L);
}

template<typename T>
inline void test_type_name_dispatching()
{
    lua_State* L = nullptr;
    impl::type_name<T>(typename trait<T>::tag(), L);
}

template<typename T>
inline void test_push_dispatching()
{
    lua_State* L = nullptr;
    T val = T();
    impl::push<T>(typename trait<T>::tag(), val, L);
}

template<typename T>
inline void test_cast_dispatching()
{
    lua_State* L = nullptr;
    impl::cast<T>(typename trait<T>::tag(), 0, L);
}

inline void test_builtins()
{
    test_is_dispatching<int>();
    test_is_dispatching<unsigned>();
    test_is_dispatching<double>();
    test_is_dispatching<bool>();
    test_is_dispatching<std::string>();
    test_is_dispatching<const char*>();

    test_type_name_dispatching<int>();
    test_type_name_dispatching<unsigned>();
    test_type_name_dispatching<double>();
    test_type_name_dispatching<bool>();
    test_type_name_dispatching<std::string>();
    test_type_name_dispatching<const char*>();

    test_push_dispatching<int>();
    test_push_dispatching<unsigned>();
    test_push_dispatching<double>();
    test_push_dispatching<bool>();
    test_push_dispatching<std::string>();
    test_push_dispatching<const char*>();

    test_cast_dispatching<int>();
    test_cast_dispatching<unsigned>();
    test_cast_dispatching<double>();
    test_cast_dispatching<bool>();
    test_cast_dispatching<std::string>();
    test_cast_dispatching<const char*>();
}

} // anonymous namespace
#endif
