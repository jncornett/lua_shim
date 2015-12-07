#include "shim/shim_dispatch.h"

#include <memory>
#include "common.h"

using namespace Shim;

struct Foo
{
    int x;
};

template<>
const char* type_name_storage<Foo>::value = "Foo";

TEST_CASE( "dispatch" )
{
    State lua;

    quick_register<Foo>(lua);

    Foo foo { 1 };
    Foo& ref = foo;
    Foo* ptr = &foo;

    SECTION( "push" )
    {
        SECTION( "plain" )
        {
            auto v = foo;
            stack::push(lua, v);
            REQUIRE( lua_type(lua, -1) == LUA_TUSERDATA );
            auto* r = *static_cast<Foo**>(lua_touserdata(lua, -1));
            REQUIRE( r );
            CHECK( r->x == foo.x );
        }

        SECTION( "ref" )
        {
            auto& v = foo;
            stack::push(lua, v);
            REQUIRE( lua_type(lua, -1) == LUA_TUSERDATA );
            auto* r = *static_cast<Foo**>(lua_touserdata(lua, -1));
            REQUIRE( r );
            CHECK( r->x == foo.x );
        }

        SECTION( "ptr" )
        {
            auto* v = &foo;
            stack::push(lua, v);
            REQUIRE( lua_type(lua, -1) == LUA_TUSERDATA );
            auto* r = *static_cast<Foo**>(lua_touserdata(lua, -1));
            REQUIRE( r );
            CHECK( r->x == foo.x );
        }
    }

    SECTION( "cast" )
    {
        // lua manages resource
        quick_push(lua, new Foo { 13 });

        SECTION( "plain" )
        {
            auto v = stack::cast<Foo>(lua, -1);
            CHECK( v.x == 13 );
        }

        SECTION( "ref" )
        {
            auto& r = stack::cast<Foo&>(lua, -1);
            CHECK( r.x == 13 );
        }

        SECTION( "ptr" )
        {
            auto* p = stack::cast<Foo*>(lua, -1);
            CHECK( p->x == 13 );
        }
    }

    SECTION( "is" )
    {
        // lua manages resource
        quick_push(lua, new Foo { 13 });

        CHECK( stack::is<Foo>(lua, -1) );
        CHECK( stack::is<Foo&>(lua, -1) );
        CHECK( stack::is<Foo*>(lua, -1) );
    }

    SECTION( "type_name" )
    {
        CHECK( stack::type_name<Foo>(lua) == "Foo" );
        CHECK( stack::type_name<Foo&>(lua) == "Foo" );
        CHECK( stack::type_name<Foo*>(lua) == "Foo" );
    }
}
