#include "shim/shim_dispatch.h"

#include "common.h"

using namespace Shim;

TEST_CASE( "dispatch push" )
{
    State lua;

    SECTION( "integral" )
    {
        int v = 11;
        stack::push(lua, v); // typed
        stack::push(lua, 11); // implicit
        stack::push<decltype(v)>(lua, v); // explicit

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
        stack::push(lua, v); // typed
        stack::push(lua, 3.14); // implicit
        stack::push<decltype(v)>(lua, v); // explicit

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
        stack::push(lua, v); // typed
        stack::push(lua, true); // implicit
        stack::push<decltype(v)>(lua, v); // explicit

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
        stack::push(lua, v); // typed
        stack::push<decltype(v)>(lua, "foo"); // explicit/implicit
        stack::push<decltype(v)>(lua, v); // explicit

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
        stack::push(lua, v); // typed
        stack::push<decltype(v)>(lua, v); // explicit

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

        CHECK( stack::is<decltype(v)>(lua, -1) );
        CHECK( stack::cast<decltype(v)>(lua, -1) == v );
    }

    SECTION( "floating point" )
    {
        double v = 11.0;
        lua_pushnumber(lua, v);
        REQUIRE( lua_type(lua, -1) == LUA_TNUMBER );

        CHECK( stack::is<decltype(v)>(lua, -1) );
        CHECK( stack::cast<decltype(v)>(lua, -1) == v );
    }

    SECTION( "boolean" )
    {
        bool v = true;
        lua_pushboolean(lua, v);
        REQUIRE( lua_type(lua, -1) == LUA_TBOOLEAN );

        CHECK( stack::is<decltype(v)>(lua, -1) );
        CHECK( stack::cast<decltype(v)>(lua, -1) == v );
    }

    SECTION( "string" )
    {
        std::string v = "foo";
        lua_pushstring(lua, v.c_str());
        REQUIRE( lua_type(lua, -1) == LUA_TSTRING );

        CHECK( stack::is<decltype(v)>(lua, -1) );
        CHECK( stack::cast<decltype(v)>(lua, -1) == v );
    }

    SECTION( "cstring" )
    {
        const char* v = "foo";
        lua_pushstring(lua, v);
        REQUIRE( lua_type(lua, -1) == LUA_TSTRING );
        std::string s = stack::cast<decltype(v)>(lua, -1);

        CHECK( stack::is<decltype(v)>(lua, -1) );
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
