#pragma once

#include <luajit-2.0/lua.hpp>

#include "shim_user.hpp"

// utils that require the lua header

namespace Shim
{

namespace util
{

class Pop
{
public:
    Pop(lua_State* L) : L(L), top(lua_gettop(L)) { }

    ~Pop()
    {
        if ( L && lua_gettop(L) > top )
            lua_settop(L, top);
    }

    void disable()
    { L = nullptr; }

    void enable(lua_State* L_)
    { L = L_; }

    bool disabled() const
    { return L == nullptr; }

private:
    lua_State* L;
    const int top;
};

template<typename O>
struct GCObject
{
    static int gc(lua_State* L)
    { udata<O>::destroy(L, 1); return 0; }

    static void push(lua_State* L, O& o)
    {
        udata<O>::initialize(L, o);
        auto object = lua_gettop(L);

        lua_newtable(L);
        auto meta = lua_gettop(L);

        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, gc);
        lua_rawset(L, meta);

        lua_setmetatable(L, object);
    }

    static O& get(lua_State* L, int n)
    { return *udata<O>::extract_ptr(L, n); }

};

} // namespace util

} // namespace Shim

