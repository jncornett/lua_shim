#include "lua_pop.h"
#include "common.h"

TEST_CASE( "lua pop" )
{
    State lua;

    lua_pushnil(lua);
    auto top = lua_gettop(lua);

    {
        Lua::Pop pop(lua);
        lua_pushnil(lua);
    }

    CHECK( top == lua_gettop(lua) );
}
