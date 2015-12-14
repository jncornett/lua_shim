#pragma once

#include "lua_userdata.h"

namespace Lua
{

namespace util
{

struct gc_object
{
    template<typename T>
    static int dtor_proxy(lua_State* L)
    {
        userdata<T>::destroy(L, 1);
        return 0;
    }

    template<typename T>
    static void push(lua_State* L, T& o)
    {
        static_assert(std::is_copy_constructible<T>::value,
            "must be copy-constructible");

        userdata<T>::emplace(L, o);
        auto n = lua_gettop(L);

        lua_newtable(L);
        auto meta = lua_gettop(L);

        lua_pushliteral(L, "__gc");
        lua_pushcfunction(L, dtor_proxy<T>);
        lua_rawset(L, meta);

        lua_setmetatable(L, n);
    }

    // 'T' must be a non-reference, non-pointer type
    template<typename T>
    static T& cast(lua_State* L, int n)
    {
        auto p = *userdata<T>::extract(L, n);
        assert(p);
        return *p;
    }
};

} // namespace util

}
