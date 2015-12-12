#pragma once

#include "shim_user.hpp"

namespace Reg
{

namespace util
{

using namespace Shim;

struct gc_object
{
    template<typename Object>
    static int gc(lua_State* L)
    { udata<Object>::destroy(L, 1); return 0; }

    template<typename Object>
    static void push(lua_State* L, Object& o)
    {
        udata<Object>::construct(L, o);
        auto object = lua_gettop(L);

        lua_newtable(L);
        auto meta = lua_gettop(L);

        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, gc<Object>);
        lua_rawset(L, meta);

        lua_setmetatable(L, object);
    }

    template<typename Object>
    static Object& get(lua_State* L, int n)
    { return *udata<Object>::extract_ptr(L, n); }

};

} // namespace util

}
