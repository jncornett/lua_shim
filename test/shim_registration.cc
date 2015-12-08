#include "shim/shim_registration.h"

#include "common.h"

using namespace Shim;

struct Bar
{
    int x;
    void setx(int x)
    { this->x = x; }

    int getx()
    { return x; }
};

struct proxy_ctor_no_args {};

struct proxy_ctor_some_args
{
    proxy_ctor_some_args(int, bool) {}
};

struct proxy_dtor {};

template<typename F>
struct Func {};

template<typename Return, typename... Args>
struct Func<Return(*)(Args...)>
{ using type = std::function<Return(Args...)>; };

template<typename F>
typename Func<F>::type make_func(F fn)
{ return fn; }

TEST_CASE( "proxy" )
{
    State lua;

    SECTION( "static_fn" )
    {
        CHECK( false );
    }

    SECTION( "fn" )
    {
        CHECK( false );
    }

    SECTION( "ctor" )
    {
        SECTION( "no args" )
        {
            lua_pushcfunction(lua,
                registration::proxy<proxy_ctor_no_args>::ctor<>);

            CHECK( !lua_pcall(lua, 0, 1, 0) );
            CHECK( lua_type(lua, -1) == LUA_TUSERDATA );
        }

        SECTION( "some args" )
        {
            lua_CFunction fn = registration::proxy<proxy_ctor_some_args>::
                ctor<int, bool>;

            lua_pushcfunction(lua, fn);
            lua_pushinteger(lua, 2);

            SECTION( "no throw" )
            {
                lua_pushboolean(lua, true);

                if ( lua_pcall(lua, 2, 1, 0) )
                    FAIL( "LUA: " << lua_tostring(lua, -1) );

                CHECK( lua_type(lua, -1) == LUA_TUSERDATA );
            }

            SECTION( "throws" )
            {
                if ( lua_pcall(lua, 1, 1, 0) )
                {
                    TypeError expect(2, "boolean", "userdata");
                    CHECK( expect.what() == lua_tostring(lua, -1) );
                }

                else
                    FAIL( "expected an error" );
            }
        }
    }

    SECTION( "dtor" )
    {
        lua_pushcfunction(lua, registration::proxy<proxy_dtor>::dtor);

        SECTION( "no throw" )
        {
            auto h = lua_newuserdata(lua, sizeof(proxy_dtor*));
            REQUIRE( h );

            *static_cast<proxy_dtor**>(h) = new proxy_dtor();
            REQUIRE( *static_cast<void**>(h) );

            if ( lua_pcall(lua, 1, 0, 0) )
                FAIL( "LUA: " << lua_tostring(lua, -1) );

            CHECK( *static_cast<void**>(h) == nullptr );
        }

        SECTION( "throws" )
        {
            // FIXIT-H J currently, the dtor does not do type-checking
            // CHECK( false );
        }
    }

    SECTION( "raw" )
    {
        CHECK( false );
    }
}

TEST_CASE( "registration interface" )
{
#if 0
    State lua;
    open_class<Bar>("Bar", lua).finalize();
    luaL_dostring(lua, "print(Bar)");
    luaL_dostring(lua, "print(Bar.new)");
#endif
    CHECK( false );
}
