#include "shim_dispatchx.hpp"

#include <memory>

#include "common.hpp"

// -----------------------------------------------------------------------------
// test cases
// -----------------------------------------------------------------------------

TEST_CASE ( "dispatchx" )
{
    using namespace Shim;

    State lua;

    int i = 42;
    lua_pushinteger(lua, i);
    auto v = lua_gettop(lua);

    lua_pushnil(lua);
    auto n = lua_gettop(lua);

    SECTION( "checkx" )
    {
        CHECK_NOTHROW( stack::check<int>(lua, v) );
        CHECK_THROWS_AS( stack::check<int>(lua, n), TypeError );
    }

    SECTION( "getx" )
    {
        CHECK_THROWS_AS( stack::check<int>(lua, n), TypeError );
        CHECK( stack::getx<int>(lua, v) == i);
    }

    SECTION( "exception content" )
    {
        try
        {
            stack::check<int>(lua, n);
            throw 0;
        }
        catch ( TypeError& e )
        {
            const char exp[] =
                "TypeError: (arg #2) expected 'number', got 'nil'" ;

            CHECK( e.what() == exp );
        }
        catch ( int )
        {
            FAIL( "failed to throw a TypeError" );
        }
    }
}
