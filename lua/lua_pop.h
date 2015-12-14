#pragma once

#include <luajit-2.0/lua.hpp>

namespace Lua
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

}
