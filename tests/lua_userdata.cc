#include "lua_userdata.h"

#include <memory>

#include "common.h"

#undef d_v
#define d_v 3

namespace t_lua_userdata
{
// -----------------------------------------------------------------------------
// fixtures
// -----------------------------------------------------------------------------

struct TUser
{
    int v = d_v;
    TUser() { }
    TUser(int v) : v(v) { }
};


// -----------------------------------------------------------------------------
// static asserts
// -----------------------------------------------------------------------------

static_assert(std::is_same<Lua::util::userdata<int**>::base_type, int>::value,
    "base type");

} // namespace t_lua_userdata


// -----------------------------------------------------------------------------
// test cases
// -----------------------------------------------------------------------------

TEST_CASE ( "userdata" )
{
    using namespace t_lua_userdata;

    State lua;

    using X = TUser;

    SECTION ( "allocate" )
    {
        auto ot = lua_gettop(lua);
        X** p = Lua::util::userdata<X>::allocate(lua);

        CHECK( p );
        CHECK( lua_gettop(lua) == ot + 1 );
        CHECK( lua_type(lua, -1) == LUA_TUSERDATA );
    }

    std::unique_ptr<X> x(new X);

    SECTION ( "extract" )
    {
        X** v = static_cast<X**>(lua_newuserdata(lua, sizeof(X*)));
        REQUIRE( v );

        *v = x.get();

        X** h = Lua::util::userdata<X>::extract(lua, -1);
        CHECK( h == v );
    }

    SECTION ( "push" )
    {
        std::unique_ptr<X> x(new X);

        Lua::util::userdata<X>::push(lua, *x.get());
        CHECK( lua_type(lua, -1) == LUA_TUSERDATA );

        auto h = static_cast<X**>(lua_touserdata(lua, -1));
        REQUIRE( h );

        CHECK( *h == x.get() );
    }

    SECTION ( "emplace" )
    {
        Lua::util::userdata<X>::emplace(lua);
        CHECK( lua_type(lua, -1) == LUA_TUSERDATA );

        auto h = static_cast<X**>(lua_touserdata(lua, -1));
        REQUIRE( h );
        REQUIRE( *h );

        delete *h;
    }

    SECTION ( "destroy" )
    {
    }
}
