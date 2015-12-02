#include <luajit-2.0/lua.hpp>
#include "shim/shim_dispatch.h"
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
static inline void quick_register(lua_State* L)
{
    Pop pop(L);
    constexpr auto name = Shim::user_type_name_storage<T>::value;
    static_assert(name != nullptr, "");
    luaL_newmetatable(L, name);
}
