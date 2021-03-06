#pragma once

#include "shim_defs.h"
#include "shim_types.h"
#include "lua_pop.h"
#include "lua_userdata.h"
#include "lua_util.h"

namespace Lua
{

namespace traits
{

using namespace util;

template<typename T>
struct trait<T, enable_for<is_user_object<T>() or is_user_ref<T>()>>
{ using tag = tags::user; };

template<typename T>
struct trait<T, enable_for<is_user_ptr<T>()>>
{ using tag = tags::user_ptr; };

template<typename T>
struct lua_type_code<T, enable_for<is_user<T>()>>
{ static constexpr auto value = LUA_TUSERDATA; };

} // namespace traits

namespace impl
{

template<typename T>
inline bool is(tags::user, lua_State* L, int n)
{
    if ( lua_type(L, n) != traits::lua_type_code<T>::value )
        return false;

    const auto& name =
        traits::type_name_storage<typename util::base<T>::type>::value;

    // can't do further comparison w/out type name
    if ( name.empty() )
        return false;

    // we'll be altering the stack, so we need the absolute index
    auto idx = util::abs_index(lua_gettop(L), n);

    Pop pop(L);

    lua_getmetatable(L, idx);
    if ( !lua_istable(L, -1) )
        return false;

    luaL_getmetatable(L, name.c_str());
    return lua_rawequal(L, -2, -1);
}

template<typename T>
inline std::string type_name(tags::user)
{
    const auto& name = traits::type_name_storage<T>::value;
    if ( name.empty() )
        return "unregistered userdata";

    return name;
}

// Disable pushing for user data structures
// template<typename T>
// inline void push(tags::user, lua_State* L, const T val)
// { userdata<T>::push(L, val); }

// template<typename T>
// inline void push(tags::user_ptr, lua_State* L, T val)
// { userdata<T>::push(L, *val); }

template<typename T>
inline T cast(tags::user, lua_State* L, int n)
{
    auto* p = *util::userdata<T>::extract(L, n);
    assert(p);
    return *p;
}

template<typename T>
inline T cast(tags::user_ptr, lua_State* L,  int n)
{ return *util::userdata<T>::extract(L, n); }

} // namespace impl

}
