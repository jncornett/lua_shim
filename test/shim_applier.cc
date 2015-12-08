#include <memory>

#include "shim/shim_applier.h"
#include "common.h"

using namespace Shim;

namespace t_applier
{

struct X
{
    int x;
    bool y;

    X(int x, bool y) : x(x), y(y) { }
};

static bool void_return_called = false;
static void void_return(int) { void_return_called = true; }

static bool int_return_called = false;
static int int_return(int x, bool) { int_return_called = true; return x; }

} // namespace t_applier

TEST_CASE( "appliers" )
{
    State lua;

    TypeError expected_no_value(2, "boolean", "no value");
    TypeError expected_wrong_type(2, "boolean", "string");

    SECTION( "New" )
    {
        lua_pushinteger(lua, 42);
        lua_pushboolean(lua, true);

        std::unique_ptr<t_applier::X> object(
            appliers::New<1, int, bool>::apply<t_applier::X>(lua));

        CHECK( object->x == 42 );
        CHECK( object->y == true );

        SECTION( "throws" )
        {
            SECTION( "no value" )
            {
                lua_settop(lua, 0);
                lua_pushinteger(lua, 42);

                try
                {
                    std::unique_ptr<t_applier::X> object(
                        appliers::New<1, int, bool>::apply<t_applier::X>(lua));

                    throw 0;
                }

                catch ( TypeError& e )
                {
                    CHECK( e.what() == expected_no_value.what() );
                }

                catch ( int& )
                {
                    FAIL( "failed to throw TypeError" );
                }

            }

            SECTION( "wrong type" )
            {
                lua_settop(lua, 0);
                lua_pushinteger(lua, 42);
                lua_pushstring(lua, "foo");

                try
                {
                    std::unique_ptr<t_applier::X> object(
                        appliers::New<1, int, bool>::apply<t_applier::X>(lua));

                    throw 0;
                }

                catch ( TypeError& e )
                {
                    CHECK( e.what() == expected_wrong_type.what() );
                }

                catch ( int& )
                {
                    FAIL( "failed to throw TypeError" );
                }

            }
        }
    }

    SECTION( "Function" )
    {
        lua_pushinteger(lua, 42);

        SECTION( "void return" )
        {
            REQUIRE_FALSE( t_applier::void_return_called );

            appliers::Function<1, int>::apply<void>(lua,
                &t_applier::void_return);

            CHECK( t_applier::void_return_called );
        }

        SECTION( "int return" )
        {
            lua_pushboolean(lua, true);

            t_applier::int_return_called = false;

            auto ret =
                appliers::Function<1, int, bool>::apply<int>(lua,
                    &t_applier::int_return);

            CHECK( ret == 42 );
            CHECK( t_applier::int_return_called );

            SECTION( "throws" )
            {
                SECTION( "no value" )
                {
                    lua_settop(lua, 0);
                    lua_pushinteger(lua, 42);

                    try
                    {
                        appliers::Function<1, int, bool>::apply<int>(lua,
                            &t_applier::int_return);

                        throw 0;
                    }

                    catch ( TypeError& e )
                    {
                        CHECK( e.what() == expected_no_value.what() );
                    }

                    catch ( int& )
                    {
                        FAIL( "failed to throw TypeError" );
                    }
                }

                SECTION( "wrong type" )
                {
                    lua_settop(lua, 0);
                    lua_pushinteger(lua, 42);
                    lua_pushstring(lua, "foo");

                    try
                    {
                        appliers::Function<1, int, bool>::apply<int>(lua,
                            &t_applier::int_return);

                        throw 0;
                    }

                    catch ( TypeError& e )
                    {
                        CHECK( e.what() == expected_wrong_type.what() );
                    }

                    catch ( int& )
                    {
                        FAIL( "failed to throw TypeError" );
                    }

                }
            }
        }
    }
}
