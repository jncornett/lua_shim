#include "shim/shim_dispatch.h"

#include "common.h"

using namespace Shim;

TEST_CASE( "dispatch push" )
{
    State lua;

    SECTION( "integral" )
    {
        int v = 11;
        stack::push(v, lua); // typed
        stack::push(11, lua); // implicit
        stack::push<decltype(v)>(v, lua); // explicit

        REQUIRE( lua_gettop(lua) == 3 );

        for ( auto i = -1; i >= -3; --i )
        {
            REQUIRE( lua_type(lua, i) == LUA_TNUMBER );
            CHECK( lua_tointeger(lua, i) == v );
        }
    }

    SECTION( "floating point" )
    {
        double v = 3.14;
        stack::push(v, lua); // typed
        stack::push(3.14, lua); // implicit
        stack::push<decltype(v)>(v, lua); // explicit

        REQUIRE( lua_gettop(lua) == 3 );

        for ( auto i = -1; i >= -3; --i )
        {
            REQUIRE( lua_type(lua, i) == LUA_TNUMBER );
            CHECK( lua_tonumber(lua, i) == v );
        }
    }

    SECTION( "boolean" )
    {
        bool v = true;
        stack::push(v, lua); // typed
        stack::push(true, lua); // implicit
        stack::push<decltype(v)>(v, lua); // explicit

        REQUIRE( lua_gettop(lua) == 3 );

        for ( auto i = -1; i >= -3; --i )
        {
            REQUIRE( lua_type(lua, i) == LUA_TBOOLEAN );
            CHECK( lua_toboolean(lua, i) == v );
        }
    }

    SECTION( "string" )
    {
        std::string v = "foo";
        stack::push(v, lua); // typed
        stack::push<decltype(v)>("foo", lua); // explicit/implicit
        stack::push<decltype(v)>(v, lua); // explicit

        REQUIRE( lua_gettop(lua) == 3 );

        for ( auto i = -1; i >= -3; --i )
        {
            REQUIRE( lua_type(lua, i) == LUA_TSTRING );
            size_t len;
            auto s = std::string(lua_tolstring(lua, i, &len), len);
            CHECK( s == v );
        }
    }

    SECTION( "cstring" )
    {
        const char* v = "foo";
        stack::push(v, lua); // typed
        stack::push<decltype(v)>(v, lua); // explicit

        REQUIRE( lua_gettop(lua) == 2 );

        for ( auto i = -1; i >= -2; --i )
        {
            REQUIRE( lua_type(lua, i) == LUA_TSTRING );
            std::string s = lua_tostring(lua, i);
            CHECK( s == v );
        }
    }
}

TEST_CASE( "dispatch cast/dispatch is" )
{
    State lua;

    SECTION( "integral" )
    {
        int v = 11;
        lua_pushinteger(lua, v);
        REQUIRE( lua_type(lua, -1) == LUA_TNUMBER );

        CHECK( stack::is<decltype(v)>(-1, lua) );
        CHECK( stack::cast<decltype(v)>(-1, lua) == v );
    }

    SECTION( "floating point" )
    {
        double v = 11.0;
        lua_pushnumber(lua, v);
        REQUIRE( lua_type(lua, -1) == LUA_TNUMBER );

        CHECK( stack::is<decltype(v)>(-1, lua) );
        CHECK( stack::cast<decltype(v)>(-1, lua) == v );
    }

    SECTION( "boolean" )
    {
        bool v = true;
        lua_pushboolean(lua, v);
        REQUIRE( lua_type(lua, -1) == LUA_TBOOLEAN );

        CHECK( stack::is<decltype(v)>(-1, lua) );
        CHECK( stack::cast<decltype(v)>(-1, lua) == v );
    }

    SECTION( "string" )
    {
        std::string v = "foo";
        lua_pushstring(lua, v.c_str());
        REQUIRE( lua_type(lua, -1) == LUA_TSTRING );

        CHECK( stack::is<decltype(v)>(-1, lua) );
        CHECK( stack::cast<decltype(v)>(-1, lua) == v );
    }

    SECTION( "cstring" )
    {
        const char* v = "foo";
        lua_pushstring(lua, v);
        REQUIRE( lua_type(lua, -1) == LUA_TSTRING );
        std::string s = stack::cast<decltype(v)>(-1, lua);

        CHECK( stack::is<decltype(v)>(-1, lua) );
        CHECK( s == v );
    }
}

TEST_CASE( "dispatch type_name" )
{
    State lua;

    SECTION( "integral" )
    {
        CHECK( stack::type_name<int>(lua) == "number" );
    }

    SECTION( "double" )
    {
        CHECK( stack::type_name<double>(lua) == "number" );
    }

    SECTION( "bool" )
    {
        CHECK( stack::type_name<bool>(lua) == "boolean" );
    }

    SECTION( "string" )
    {
        CHECK( stack::type_name<std::string>(lua) == "string" );
    }

    SECTION( "cstring" )
    {
        CHECK( stack::type_name<const char*>(lua) == "string" );
    }
}
