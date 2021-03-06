#include "functional_pushers.h"

#include "common.h"

namespace t_functional_pushers
{
// -----------------------------------------------------------------------------
// fixtures
// -----------------------------------------------------------------------------

static bool static_function_spy = false;

static void void_static_function(int, bool b)
{ static_function_spy = b; }

static int int_static_function(int i, bool b)
{ static_function_spy = b;  return i; }

static int raw_static_function(lua_State* L)
{
    static_function_spy = lua_toboolean(L, 1);
    return 0;
}

struct X
{
    bool member_function_spy = false;
    mutable bool const_member_function_spy = false;

    static int destructor_calls;

    int constructor_init = 0;
    bool constructor_called = false;

    void void_member_function(int, bool b)
    { member_function_spy = b; }

    int int_member_function(int i, bool b)
    { member_function_spy = b; return i; }

    void const_member_function() const
    { const_member_function_spy = true; }

    X(int i) : constructor_init(i), constructor_called(true) { }

    ~X()
    { ++destructor_calls; }
};

int X::destructor_calls = 0;

} // namespace t_functor_pushers

// -----------------------------------------------------------------------------
// test cases
// -----------------------------------------------------------------------------

TEST_CASE ( "pushers" )
{
    using namespace t_functional_pushers;
    State lua;

    Lua::traits::type_name_storage<X>::value = "X";
    luaL_newmetatable(lua, "X");
    lua_pop(lua, 1);

    SECTION( "auto pusher" )
    {
        SECTION( "with function pointer" )
        {
            SECTION( "int return" )
            {
                using func_type = decltype(int_static_function);
                Lua::detail::auto_pusher<func_type>::
                    push(lua, int_static_function);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );
                static_function_spy = false;

                int v = 4;
                lua_pushinteger(lua, v);
                lua_pushboolean(lua, true);

                if ( lua_pcall(lua, 2, 1, 0) )
                    FAIL( lua_tostring(lua, -1) );

                CHECK( static_function_spy );
                REQUIRE( lua_type(lua, -1) == LUA_TNUMBER );
                CHECK( lua_tointeger(lua, -1) == v );
            }

            SECTION( "void return" )
            {
                using func_type = decltype(void_static_function);
                Lua::detail::auto_pusher<func_type>::
                    push(lua, void_static_function);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

                static_function_spy = false;
                lua_pushinteger(lua, 4);
                lua_pushboolean(lua, true);

                if ( lua_pcall(lua, 2, 0, 0) )
                    FAIL( lua_tostring(lua, -1) );

                CHECK( static_function_spy );
            }

            SECTION( "exception handled" )
            {
                using func_type = decltype(void_static_function);
                Lua::detail::auto_pusher<func_type>::
                    push(lua, void_static_function);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

                lua_pushinteger(lua, 4);

                if ( !lua_pcall(lua, 1, 0, 0) )
                    FAIL( "expected a TypeError" );

                std::string e = lua_tostring(lua, -1);
                CHECK( e == "TypeError: (arg #2) expected 'boolean', got 'no value'" );
            }
        }

        SECTION( "with member function" )
        {
            std::unique_ptr<X> x(new X(0));
            auto h = static_cast<X**>(lua_newuserdata(lua, sizeof(X*)));
            REQUIRE( h );

            *h = x.get();
            auto u = lua_gettop(lua);

            luaL_getmetatable(lua, "X");
            lua_setmetatable(lua, u);

            SECTION( "int return" )
            {
                using func_type = decltype(&X::int_member_function);
                Lua::detail::auto_pusher<func_type>::
                    push(lua, &X::int_member_function);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

                lua_pushvalue(lua, u);

                int v = 4;
                lua_pushinteger(lua, v);
                lua_pushboolean(lua, true);

                if ( lua_pcall(lua, 3, 1, 0) )
                    FAIL( lua_tostring(lua, -1) );

                CHECK( x->member_function_spy );
                REQUIRE( lua_type(lua, -1) == LUA_TNUMBER );
                CHECK( lua_tointeger(lua, -1) == v );
            }

            SECTION( "void return" )
            {
                using func_type = decltype(&X::void_member_function);
                Lua::detail::auto_pusher<func_type>::
                    push(lua, &X::void_member_function);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

                lua_pushvalue(lua, u);

                lua_pushinteger(lua, 4);
                lua_pushboolean(lua, true);

                if ( lua_pcall(lua, 3, 1, 0) )
                    FAIL( lua_tostring(lua, -1) );

                CHECK( x->member_function_spy );
            }

            SECTION( "const" )
            {
                using func_type = decltype(&X::const_member_function);
                Lua::detail::auto_pusher<func_type>::
                    push(lua, &X::const_member_function);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

                lua_pushvalue(lua, u);

                if ( lua_pcall(lua, 1, 0, 0) )
                    FAIL( lua_tostring(lua, -1) );

                CHECK( x->const_member_function_spy );
            }
        }

        SECTION( "with function object" )
        {
            SECTION( "normal operation" )
            {
                std::function<void(int, bool)> fn = void_static_function;
                Lua::detail::auto_pusher<decltype(fn)>::push(lua, fn);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

                static_function_spy = false;

                lua_pushinteger(lua, 4);
                lua_pushboolean(lua, true);

                if ( lua_pcall(lua, 2, 0, 0) )
                    FAIL( lua_tostring(lua, -1) );

                CHECK( static_function_spy );
            }

            SECTION( "const" )
            {
                const std::function<void(int, bool)> fn = void_static_function;
                Lua::detail::auto_pusher<decltype(fn)>::push(lua, fn);

                REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

                static_function_spy = false;

                lua_pushinteger(lua, 4);
                lua_pushboolean(lua, true);

                if ( lua_pcall(lua, 2, 0, 0) )
                    FAIL( lua_tostring(lua, -1) );

                CHECK( static_function_spy );
            }
        }

        SECTION( "with raw function" )
        {
            Lua::detail::auto_pusher<decltype(raw_static_function)>::push(lua,
                raw_static_function);

            REQUIRE( lua_type(lua, -1) == LUA_TFUNCTION );

            static_function_spy = false;

            lua_pushboolean(lua, true);

            if ( lua_pcall(lua, 1, 0, 0) )
                FAIL( lua_tostring(lua, -1) );

            CHECK( static_function_spy );
        }
    }

    SECTION( "constructor pusher" )
    {
        Lua::detail::constructor_pusher<X, int>::push(lua);
        CHECK( lua_type(lua, -1) == LUA_TFUNCTION );

        SECTION( "normal operation" )
        {
            int v = 4;
            lua_pushinteger(lua, v);
            if ( lua_pcall(lua, 1, 1, 0) )
                FAIL( lua_tostring(lua, -1) );

            REQUIRE( lua_type(lua, -1) == LUA_TUSERDATA );
            auto h = static_cast<X**>(lua_touserdata(lua, -1));
            REQUIRE( h );
            REQUIRE( *h );
            auto p = *h;

            CHECK( p->constructor_called );
            CHECK( p->constructor_init == v );
            delete p;
        }

        SECTION( "handles exception" )
        {
            if ( !lua_pcall(lua, 0, 1, 0) )
                FAIL( "expected a TypeError" );

            std::string e = lua_tostring(lua, -1);
            CHECK( e == "TypeError: (arg #1) expected 'integer', got 'no value'" );
        }
    }

    SECTION( "destructor pusher" )
    {
        Lua::detail::destructor_pusher<X>::push(lua);
        CHECK( lua_type(lua, -1) == LUA_TFUNCTION );

        SECTION( "normal operation" )
        {
            X::destructor_calls = 0;
            auto h = static_cast<X**>(lua_newuserdata(lua, sizeof(X*)));
            REQUIRE( h );
            *h = new X(0);

            // FIXIT-H cryptic TypeError error message when metatable is not assigned
            luaL_getmetatable(lua, "X");
            lua_setmetatable(lua, -2);

            if ( lua_pcall(lua, 1, 0, 0) )
            {
                // make sure to cleanup
                if ( *h )
                    delete *h;

                FAIL( lua_tostring(lua, -1) );
            }

            CHECK( X::destructor_calls == 1 );
            CHECK_FALSE( *h );
        }

        SECTION( "handles exception" )
        {
            if ( !lua_pcall(lua, 0, 0, 0) )
                FAIL( "expected a TypeError" );

            std::string e = lua_tostring(lua, -1);
            CHECK( e == "TypeError: (arg #1) expected 'X', got 'no value'" );
        }
    }
}
