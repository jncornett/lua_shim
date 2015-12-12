#pragma once

#include <cassert>

#include <luajit-2.0/lua.hpp>

#include "shim_user_defs.hpp"
#include "shim_builtin.hpp"

namespace Shim
{

namespace detail
{

template<typename T>
inline constexpr bool is_user()
{
    return !is_builtin<T>() &&
        !std::is_pointer<T>::value &&
        !std::is_reference<T>::value;
}

template<typename T>
inline constexpr bool is_user_ref()
{
    return std::is_reference<T>::value &&
        is_user<typename util::base<T>::type>();
}

template<typename T>
inline constexpr bool is_user_ptr()
{
    return std::is_pointer<T>::value &&
        is_user<typename util::base<T>::type>();
}

} // namespace detail

// utils for working with Lua userdata
template<typename T>
struct udata
{
    using base_type = typename util::base<T>::type;

    static void assign_metatable(lua_State* L, int n)
    { lua_setmetatable(L, n); }

    static void assign_metatable(lua_State* L, const char* name, int n)
    {
        luaL_getmetatable(L, name);
        // type must be registered with lua before use
        assert(!lua_isnoneornil(L, -1));
        assign_metatable(L, n);
    }

    static base_type** allocate(lua_State* L)
    { return static_cast<base_type**>(lua_newuserdata(L, sizeof(base_type*))); }

    static void assign(lua_State* L, base_type* p)
    {
        assert(p);

        const auto name = type_name_storage<base_type>::value;
        assert(name);

        auto h = allocate(L);
        assert(h);

        *h = p;

        assign_metatable(L, name, lua_gettop(L));
    }

    template<typename... Args>
    static base_type* construct(lua_State* L, Args&&... args)
    {
        auto h = allocate(L);
        assert(h);

        *h = new base_type(std::forward<Args>(args)...);
        return *h;
    }

    static base_type** extract_handle(lua_State* L, int n)
    { return static_cast<base_type**>(lua_touserdata(L, n)); }

    static base_type* extract_ptr(lua_State* L, int n)
    {
        auto h = extract_handle(L, n);
        return h ? *h : nullptr;
    }

    template<typename... Args>
    static base_type* emplace(lua_State* L, Args&&... args)
    {
        auto p = new base_type(std::forward<Args>(args)...);
        assign(L, p);
        return p;
    }

    static void destroy(lua_State* L, int n)
    {
        auto h = extract_handle(L, n);
        assert(h);

        delete *h;
        *h = nullptr;
    }
};

namespace traits
{

template<typename T>
struct trait<T, util::enable_if<detail::is_user<T>()>>
{ using tag = tags::user; };

template<typename T>
struct trait<T, util::enable_if<detail::is_user_ref<T>()>>
{ using tag = tags::user_ref; };

template<typename T>
struct trait<T, util::enable_if<detail::is_user_ptr<T>()>>
{ using tag = tags::user_ptr; };

} // namespace traits

namespace impl
{

template<typename T>
inline bool is(tags::user, lua_State* L, int n)
{
    if ( lua_type(L, n) != LUA_TUSERDATA )
        return false;

    const auto name = type_name_storage<typename util::base<T>::type>::value;
    assert(name && *name);

    lua_getmetatable(L, n);

    // cannot determine user type if there is no metatable
    bool result = !lua_isnoneornil(L, -1);

    if ( result )
    {
        luaL_getmetatable(L, name);
        // compare the metatable identities
        result = lua_rawequal(L, -2, -1);
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
    return result;
}

template<typename T>
inline std::string type_name(tags::user, lua_State*)
{ return type_name_storage<typename util::base<T>::type>::value; }

template<typename T>
inline void push(tags::user, lua_State* L, T val)
{
    auto result = udata<T>::emplace(L, val);
    assert(result);
}

template<typename T>
inline void push(tags::user_ptr, lua_State* L, T val)
{
    auto result = udata<T>::emplace(L, *val);
    assert(result);
}

template<typename T>
inline T cast(tags::user, lua_State* L, int n)
{ return *udata<T>::extract_ptr(L, n); }

template<typename T>
inline T cast(tags::user_ptr, lua_State* L, int n)
{ return udata<T>::extract_ptr(L, n); }

} // namespace impl

} // namespace Shim
