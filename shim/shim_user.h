#pragma once

#include "shim_builtins.h"
#include "shim_util.h"

#include <cassert>
#include <string>
#include <utility>

#include <luajit-2.0/lua.hpp>

namespace Shim
{

template<typename T>
struct user_type_name_storage
{ static constexpr const char* value = nullptr; };

struct udata
{
    template<typename T>
    static T** allocate(lua_State* L)
    { return static_cast<T**>(lua_newuserdata(L, sizeof(T*))); }

    template<typename T, typename... Args>
    static T* construct(lua_State* L, Args&&... args)
    {
        auto h = allocate<T>(L);
        assert(h);

        if ( h )
        {
            *h = new T(std::forward<Args>(args)...);
            return *h;
        }

        return nullptr;
    }


    template<typename T>
    static T* extract(int n, lua_State* L)
    {
        auto h = static_cast<T**>(lua_touserdata(L, n));
        return h ? *h : nullptr;
    }

    static void assign_metatable(int n, lua_State* L)
    { lua_setmetatable(L, n); }

    static void assign_metatable(const std::string& name, lua_State* L)
    {
        luaL_getmetatable(L, name.c_str());
        // type must be registered with lua before use
        assert(!lua_isnoneornil(L, -1));
        assign_metatable(-2, L);
    }
};

namespace tags
{

struct user {};

struct user_ref : user {};
struct user_ptr : user {};

}

namespace impl
{

template<typename T>
inline bool is(tags::user, int n, lua_State* L)
{
    if ( lua_type(L, n) != LUA_TUSERDATA )
        return false;

    constexpr const char* name =
        user_type_name_storage<typename util::base<T>::type>::value;

    static_assert(*name != '\0', "");

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
{ return user_type_name_storage<T>::value; }

template<typename T>
inline void push(tags::user, T val, lua_State* L)
{
    using base_type = typename util::base<T>::type;

    if ( !udata::construct<base_type>(L, val) )
        return; // assert(detail::construct_userdata<base_type>(...));

    constexpr const char* name = user_type_name_storage<base_type>::value;
    static_assert(name != nullptr, "");

    udata::assign_metatable(name, L);
}

template<typename T>
inline void push(tags::user_ptr, T val, lua_State* L)
{
    using base_type = typename util::base<T>::type;

    if ( !udata::construct<base_type>(L, *val) )
        return; // assert(detail::construct_userdata<base_type>(...));

    const auto& name = user_type_name_storage<base_type>::value;

    if ( !name.empty() )
        assign_metatable(name, L);
}

template<typename T>
inline T cast(tags::user, int n, lua_State* L)
{ return *udata::extract<typename util::base<T>::type>(n, L); }


template<typename T>
inline T cast(tags::user_ptr, int n, lua_State* L)
{ return udata::extract<typename util::base<T>::type>(n, L); }

} // namespace impl

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

template<typename T>
struct trait<T, util::enable_if<detail::is_user<T>()>>
{ using tag = tags::user; };

template<typename T>
struct trait<T, util::enable_if<detail::is_user_ref<T>()>>
{ using tag = tags::user_ref; };

template<typename T>
struct trait<T, util::enable_if<detail::is_user_ptr<T>()>>
{ using tag = tags::user_ptr; };

} // namespace Shim
