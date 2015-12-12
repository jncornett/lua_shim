#include "shim_user.hpp"

#include <memory>

#include "common.hpp"

#undef d_v
#define d_v 3

namespace t_shim_user
{
// -----------------------------------------------------------------------------
// fixtures
// -----------------------------------------------------------------------------

using namespace Shim;

struct TUser
{
    int v = d_v;
    TUser() { }
    TUser(int v) : v(v) { }
};


// -----------------------------------------------------------------------------
// static asserts
// -----------------------------------------------------------------------------

static_assert(std::is_same<udata<int**>::base_type, int>::value, "base type");

} // namespace t_shim_user


// -----------------------------------------------------------------------------
// test cases
// -----------------------------------------------------------------------------

TEST_CASE ( "udata" )
{
    using namespace Shim;
    using namespace t_shim_user;

    State lua;

    using X = TUser;

    SECTION ( "allocate" )
    {
        auto ot = lua_gettop(lua);
        X** p = udata<X>::allocate(lua);

        CHECK( p );
        CHECK( lua_gettop(lua) == ot + 1 );
        CHECK( lua_type(lua, -1) == LUA_TUSERDATA );
    }

    SECTION ( "construct" )
    {
        auto ot = lua_gettop(lua);
        X* p = udata<X>::construct(lua);

        REQUIRE( p );
        CHECK( p->v == d_v );

        SECTION( "with args" )
        {
            int v = d_v + 1;
            X* p = udata<X>::construct(lua, v);
            REQUIRE( p );
            CHECK( p->v == v );
        }
    }

    std::unique_ptr<X> x(new X);

    SECTION ( "extract" )
    {
        X** v = static_cast<X**>(lua_newuserdata(lua, sizeof(X*)));
        REQUIRE( v );

        *v = x.get();

        SECTION ( "extract handle" )
        {
            X** h = udata<X>::extract_handle(lua, -1);
            CHECK( h == v );
        }

        SECTION ( "extract ptr" )
        {
            X* p = udata<X>::extract_ptr(lua, -1);
            CHECK( p == *v );
        }
    }

    SECTION ( "assign metatable" )
    {
        lua_newuserdata(lua, 1);
        auto u = lua_gettop(lua);

        SECTION ( "by index" )
        {

            lua_newtable(lua);
            auto t = lua_gettop(lua);

            lua_pushvalue(lua, -1);

            udata<void>::assign_metatable(lua, u);
            lua_getmetatable(lua, u);
            CHECK( lua_rawequal(lua, t, -1) );
        }

        SECTION ( "by name" )
        {
            // First we have to put the table in the registry
            luaL_newmetatable(lua, "X"); // don't care if it exists already or not
            auto t = lua_gettop(lua);

            udata<void>::assign_metatable(lua, "X", u);
            lua_getmetatable(lua, u);
            CHECK( lua_rawequal(lua, t, -1) );
        }
    }

    SECTION ( "create/destroy" )
    {
        // need to register this both for Lua and C++ side
        type_name_storage<X>::value = "X";

        luaL_newmetatable(lua, "X");
        auto t = lua_gettop(lua);

        int v = d_v + 1;

        SECTION ( "emplace/destroy" )
        {
            X* p = udata<X>::emplace(lua, v);
            REQUIRE( p );

            X** h = static_cast<X**>(lua_touserdata(lua, -1));
            udata<X>::destroy(lua, -1);
            CHECK_FALSE ( *h );
        }

        SECTION ( "assign" )
        {
            std::unique_ptr<X> x(new X(v));
            udata<X>::assign(lua, x.get());

            X** h = static_cast<X**>(lua_touserdata(lua, -1));
            REQUIRE( h );
            CHECK( *h == x.get() );
        }
    }
}
