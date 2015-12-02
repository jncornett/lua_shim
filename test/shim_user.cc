#include "shim/shim_dispatch.h"

#include "common.h"

using namespace Shim;

struct Foo
{
    int x;
};

template<>
struct user_type_name_storage<Foo>
{ static constexpr const char* value = "Foo"; };

template<typename T>
static void quick_register(lua_State* L)
{
    Pop pop(L);

    constexpr const char* name = user_type_name_storage<T>::value;
    if ( !luaL_newmetatable(L, name) )
        return; // it's already been registered

    //... that's it!
}

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
            stack::push(v, lua);
            REQUIRE( lua_type(lua, -1) == LUA_TUSERDATA );
            auto* r = static_cast<Foo*>(lua_touserdata(lua, -1));
            REQUIRE( r );
            CHECK( r->x == foo.x );
        }

        SECTION( "ref" )
        {
            auto& v = foo;
            stack::push(v, lua);
            REQUIRE( lua_type(lua, -1) == LUA_TUSERDATA );
            auto* r = static_cast<Foo*>(lua_touserdata(lua, -1));
            REQUIRE( r );
            CHECK( r->x == foo.x );
        }

        SECTION( "ptr" )
        {
            auto* v = &foo;
            stack::push(v, lua);
            REQUIRE( lua_type(lua, -1) == LUA_TUSERDATA );
            auto* r = static_cast<Foo*>(lua_touserdata(lua, -1));
            REQUIRE( v );
            CHECK( v->x == foo.x );
        }
    }
}
