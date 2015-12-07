#include <iostream>
#include <luajit-2.0/lua.hpp>
#include "shim/shim_dispatch.h"
#include "shim/shim_lua_util.h"
#include "catch.hpp"

struct State
{
    State() :
        L(luaL_newstate())
    { luaL_openlibs(L); }

    ~State()
    { lua_close(L); }

    operator lua_State*()
    { return L; }

    lua_State* L;
};

struct Pop
{
    Pop(lua_State* L) :
        L(L), top(lua_gettop(L)) { }

    ~Pop()
    {
        if ( top < lua_gettop(L) )
            lua_settop(L, top);
    }

    lua_State* L;
    int top;
};

template<typename T>
static inline int destructor(lua_State* L)
{
    auto h = static_cast<T**>(lua_touserdata(L, 1));
    assert(h && *h);
    delete *h;
    h = nullptr;
    return 0;
}

template<typename T>
static inline void quick_register(lua_State* L)
{
    Shim::util::Pop pop(L);
    const auto name = Shim::type_name_storage<T>::value;
    assert(name);

    // only need to define once per vm
    if ( luaL_newmetatable(L, name) )
    {
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, &destructor<T>);
        lua_rawset(L, -3);
    }
}

template<typename T>
static inline void quick_push(lua_State* L, T* p)
{
    auto* h = lua_newuserdata(L, sizeof(T*));
    assert(h);
    *static_cast<T**>(h) = p;

    luaL_getmetatable(L, Shim::type_name_storage<T>::value);
    assert(!lua_isnoneornil(L, -1));
    lua_setmetatable(L, -2);
}

