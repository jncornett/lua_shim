#pragma once

#include <cassert>
#include <iostream>
#include <sstream>
#include <luajit-2.0/lua.hpp>

namespace Debug
{
inline void dump_table(std::ostream& os, lua_State* L, int index, int level = 0);

inline void dump_value(std::ostream& os, lua_State* L, int index, int level = 0)
{
    lua_pushvalue(L, index);
    auto p = lua_topointer(L, -1);
    lua_pop(L, 1);

    os << "[" << p << "]";

    switch ( lua_type(L, index) )
    {
        case LUA_TSTRING:
            os << "\"" << lua_tostring(L, index) << "\"";
            break;

        case LUA_TNUMBER:
            os << lua_tonumber(L, index);
            break;

        case LUA_TTABLE:
            if ( level )
            {
                dump_table(os, L, index, level);
                break;
            }

        default:
            os << lua_typename(L, lua_type(L, index));
            break;
    }
}

inline void dump_table(std::ostream& os, lua_State* L, int index, int level)
{
    auto type = lua_type(L, index);
    assert(type == LUA_TTABLE);
    bool empty = true;

    if ( !level )
    {
        lua_pushvalue(L, index);
        auto p = lua_topointer(L, -1);
        lua_pop(L, 1);
        os << "[" << p << "]";
    }

    os << "(" << lua_typename(L, type) << ") {";

    lua_pushnil(L);
    while ( lua_next(L, index) )
    {
        empty = false;

        os << "\n";

        for ( int i = 0; i < level + 1; ++i )
            os << "  ";

        auto key = lua_gettop(L) - 1;
        auto value = lua_gettop(L);

        dump_value(os, L, key);

        os << " = ";

        dump_value(os, L, value, level + 1);

        os << ",\n";
        lua_pop(L, 1);
    }

    os << "}";
}

}
