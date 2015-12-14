#pragma once

#include <cassert>

#include "shim_types.h"
#include "shim_defs.h"
#include "lua_util.h"

// helpers for working with Lua userdata

namespace Lua
{

namespace traits
{

template<typename T>
struct type_name_storage
{ static std::string value; };

template<typename T>
std::string type_name_storage<T>::value = "";

} // namespace traits


namespace util
{

template<typename T>
struct userdata
{
    using base_type = typename util::base<T>::type;

    static base_type** extract(lua_State* L, int n)
    {
        auto h = static_cast<base_type**>(lua_touserdata(L, n));
        assert(h);
        return h;
    }

    static base_type** allocate(lua_State* L)
    {
        auto h = static_cast<base_type**>(lua_newuserdata(L, sizeof(base_type*)));
        assert(h);
        return h;
    }

    template<typename... Args>
    static base_type* emplace(lua_State* L, Args&&... args)
    {
        auto h = allocate(L);
        *h = new base_type(std::forward<Args>(args)...);
        assert(*h);
        return *h;
    }

    static void push(lua_State* L, base_type& o)
    {
        auto h = allocate(L);
        *h = &o;
    }

    static void assign_metatable(lua_State* L, int n)
    {
        assert(lua_type(L, n) == LUA_TUSERDATA);

        const auto& name = traits::type_name_storage<base_type>::value;
        assert(!name.empty());

        auto idx = util::abs_index(lua_gettop(L), n);

        luaL_getmetatable(L, name.c_str());
        assert(lua_type(L, -1) == LUA_TTABLE);

        lua_setmetatable(L, idx);
    }

    // deletes a userdata and sets its pointer to nullptr
    static void destroy(lua_State* L, int n)
    {
        auto h = extract(L, n);
        if ( *h )
        {
            delete *h;
            *h = nullptr;
        }
    }
};

} // namespace util

}
