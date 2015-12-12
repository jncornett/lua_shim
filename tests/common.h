#pragma once

#include <luajit-2.0/lua.hpp>

struct State
{
    lua_State* L;

    operator lua_State*() { return L; }
    State() : L(luaL_newstate()) { luaL_openlibs(L); }
    State(const State&) = delete;
    ~State() { lua_close(L); }
    State& operator=(const State&) = delete;
};

#include "catch.hpp"
