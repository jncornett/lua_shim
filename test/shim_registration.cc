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

struct detail_Constructor_no_args
{
    int x = 2;
};

struct detail_Constructor_some_args
{
    int x;
    bool y;

    detail_Constructor_some_args(int x, bool y) : x(x), y(y) { }
};

struct detail_Callable
{
    bool called = false;
    void do_foo() { called = true; }
    int do_bar() { called = true; return 12; }

    void do_buzz(bool v, int) { called = v; }
    int do_bat(bool v, int x) { called = v; return x; }

    static void static_do_foo() { }
    static int static_do_bar() { return 12; }

    static void static_do_buzz(bool, int) { }
    static int static_do_bat(bool, int x) { return x; }
};

template<typename F>
struct Func {};

template<typename Return, typename... Args>
struct Func<Return(*)(Args...)>
{ using type = std::function<Return(Args...)>; };

template<typename F>
typename Func<F>::type make_func(F fn)
{ return fn; }

TEST_CASE( "registration detail" )
{
    State lua;

    SECTION( "Constructor" )
    {
        SECTION( "no args" )
        {
            std::unique_ptr<detail_Constructor_no_args> x(
                registration::detail::Constructor<0>::
                    construct<detail_Constructor_no_args>(lua)
            );

            CHECK( x->x == 2 );
        }

        SECTION( "some args" )
        {
            REQUIRE( lua_gettop(lua) == 0 );
            lua_pushinteger(lua, 2);
            lua_pushboolean(lua, true);

            std::unique_ptr<detail_Constructor_some_args> x(
                registration::detail::Constructor<1, int, bool>::
                    construct<detail_Constructor_some_args>(lua)
            );

            CHECK( x->x == 2 );
            CHECK( x->y == true );
        }

        SECTION( "raises exception" )
        {
            SECTION( "no value" )
            {
                REQUIRE( lua_gettop(lua) == 0 );
                lua_pushinteger(lua, 2);

                try
                {
                    std::unique_ptr<detail_Constructor_some_args> x(
                        registration::detail::Constructor<1, int, bool>::
                            construct<detail_Constructor_some_args>(lua)
                    );

                    throw 0;
                }
                catch ( TypeError& e )
                {
                    TypeError expect(2, "boolean", "no value");
                    CHECK( e.what() == expect.what() );
                }
                catch ( ... )
                {
                    FAIL( "did not throw TypeError" );
                }
            }

            SECTION( "wrong value" )
            {
                REQUIRE( lua_gettop(lua) == 0 );
                lua_pushinteger(lua, 2);
                lua_pushstring(lua, "foo");

                try
                {
                    std::unique_ptr<detail_Constructor_some_args> x(
                        registration::detail::Constructor<1, int, bool>::
                            construct<detail_Constructor_some_args>(lua)
                    );

                    throw 0;
                }
                catch ( TypeError& e )
                {
                    TypeError expect(2, "boolean", "string");
                    CHECK( e.what() == expect.what() );
                }
                catch ( ... )
                {
                    FAIL( "did not throw TypeError" );
                }
            }
        }
    }

    SECTION( "Callable" )
    {
        detail_Callable x;

        SECTION( "void return" )
        {
            SECTION( "no args" )
            {
                auto fn = std::mem_fn(&detail_Callable::do_foo);
                registration::detail::Caller<0>::call<void>(lua, fn, x);
                CHECK( x.called );
            }

            SECTION( "some args" )
            {
                REQUIRE( lua_gettop(lua) == 0 );

                lua_pushboolean(lua, true);
                lua_pushinteger(lua, 2);

                try
                {
                    auto fn = std::mem_fn(&detail_Callable::do_buzz);
                    registration::detail::Caller<1, bool, int>::
                        call<void>(lua, fn, x);

                    CHECK( x.called );
                }
                catch ( Exception& e )
                {
                    FAIL( e.what() );
                }
            }

            SECTION( "raises exception" )
            {
                REQUIRE( lua_gettop(lua) == 0 );
                lua_pushboolean(lua, true);

                auto fn = std::mem_fn(&detail_Callable::do_buzz);

                SECTION( "no value" )
                {
                    try
                    {
                        registration::detail::Caller<1, bool, int>::
                            call<void>(lua, fn, x);

                        throw 0;
                    }
                    catch ( TypeError& e )
                    {
                        TypeError expect(2, "number", "no value");
                        CHECK( e.what() == expect.what() );
                    }
                    catch ( ... )
                    {
                        FAIL( "TypeError not thrown" );
                    }
                }

                SECTION( "wrong type" )
                {
                    lua_pushstring(lua, "foo");

                    try
                    {
                        registration::detail::Caller<1, bool, int>::
                            call<void>(lua, fn, x);

                        throw 0;
                    }
                    catch ( TypeError& e )
                    {
                        TypeError expect(2, "number", "string");
                        CHECK( e.what() == expect.what() );
                    }
                    catch ( ... )
                    {
                        FAIL( "TypeError not thrown" );
                    }
                }
            }
        }

        SECTION( "int return" )
        {
            SECTION( "no args" )
            {
                auto fn = std::mem_fn(&detail_Callable::do_bar);
                auto r = registration::detail::Caller<0>::call<int>(lua, fn, x);
                CHECK( x.called );
                CHECK( r == 12 );
            }

            SECTION( "some args" )
            {
                REQUIRE( lua_gettop(lua) == 0 );

                lua_pushboolean(lua, true);
                lua_pushinteger(lua, 2);

                try
                {
                    auto fn = std::mem_fn(&detail_Callable::do_bat);
                    auto r = registration::detail::Caller<1, bool, int>::
                        call<int>(lua, fn, x);

                    CHECK( x.called );
                    CHECK( r == 2 );
                }
                catch ( Exception& e )
                {
                    FAIL( e.what() );
                }
            }

            SECTION( "raises exception" )
            {
                REQUIRE( lua_gettop(lua) == 0 );
                lua_pushboolean(lua, true);

                auto fn = std::mem_fn(&detail_Callable::do_bat);

                SECTION( "no value" )
                {
                    try
                    {
                        registration::detail::Caller<1, bool, int>::
                            call<int>(lua, fn, x);

                        throw 0;
                    }
                    catch ( TypeError& e )
                    {
                        TypeError expect(2, "number", "no value");
                        CHECK( e.what() == expect.what() );
                    }
                    catch ( ... )
                    {
                        FAIL( "TypeError not thrown" );
                    }
                }

                SECTION( "wrong type" )
                {
                    lua_pushstring(lua, "bar");

                    try
                    {
                        registration::detail::Caller<1, bool, int>::
                            call<int>(lua, fn, x);

                        throw 0;
                    }
                    catch ( TypeError& e )
                    {
                        TypeError expect(2, "number", "string");
                        CHECK( e.what() == expect.what() );
                    }
                    catch ( ... )
                    {
                        FAIL( "TypeError not thrown" );
                    }
                }
            }
        }
    }

    SECTION( "StaticCallable" )
    {
        SECTION( "function pointer" )
        {
            SECTION( "void return" )
            {
                SECTION( "no args" )
                {
                    registration::detail::StaticCaller<0>::
                        call<void>(lua, detail_Callable::static_do_foo);
                }

                SECTION( "some args" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );

                    lua_pushboolean(lua, true);
                    lua_pushinteger(lua, 2);

                    try
                    {
                        registration::detail::StaticCaller<1, bool, int>::
                            call<void>(lua, detail_Callable::static_do_buzz);
                    }
                    catch ( Exception& e )
                    {
                        FAIL( e.what() );
                    }
                }

                SECTION( "raises exception" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );
                    lua_pushboolean(lua, true);

                    SECTION( "no value" )
                    {
                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<void>(lua, detail_Callable::static_do_buzz);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "no value");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }

                    SECTION( "wrong type" )
                    {
                        lua_pushstring(lua, "foo");

                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<void>(lua, detail_Callable::static_do_buzz);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "string");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }
                }
            }

            SECTION( "int return" )
            {
                SECTION( "no args" )
                {
                    registration::detail::StaticCaller<0>::
                        call<int>(lua, detail_Callable::static_do_bar);
                }

                SECTION( "some args" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );

                    lua_pushboolean(lua, true);
                    lua_pushinteger(lua, 2);

                    try
                    {
                        auto r = registration::detail::StaticCaller<1, bool, int>::
                            call<int>(lua, detail_Callable::static_do_bat);

                        CHECK( r == 2 );
                    }
                    catch ( Exception& e )
                    {
                        FAIL( e.what() );
                    }
                }

                SECTION( "raises exception" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );
                    lua_pushboolean(lua, true);

                    SECTION( "no value" )
                    {
                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<int>(lua, detail_Callable::static_do_bat);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "no value");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }

                    SECTION( "wrong type" )
                    {
                        lua_pushstring(lua, "bar");

                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<int>(lua, detail_Callable::static_do_bat);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "string");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }
                }
            }
        }

        SECTION( "function object" )
        {
            SECTION( "void return" )
            {
                SECTION( "no args" )
                {
                    auto fn = make_func(detail_Callable::static_do_foo);
                    registration::detail::StaticCaller<0>::call<void>(lua, fn);
                }

                SECTION( "some args" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );

                    lua_pushboolean(lua, true);
                    lua_pushinteger(lua, 2);

                    auto fn = make_func(detail_Callable::static_do_buzz);

                    try
                    {
                        registration::detail::StaticCaller<1, bool, int>::
                            call<void>(lua, fn);
                    }
                    catch ( Exception& e )
                    {
                        FAIL( e.what() );
                    }
                }

                SECTION( "raises exception" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );
                    lua_pushboolean(lua, true);

                    auto fn = make_func(detail_Callable::static_do_buzz);

                    SECTION( "no value" )
                    {
                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<void>(lua, fn);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "no value");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }

                    SECTION( "wrong type" )
                    {
                        lua_pushstring(lua, "foo");

                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<void>(lua, fn);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "string");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }
                }
            }

            SECTION( "int return" )
            {
                SECTION( "no args" )
                {
                    auto fn = make_func(detail_Callable::static_do_bar);
                    registration::detail::StaticCaller<0>::call<int>(lua, fn);
                }

                SECTION( "some args" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );

                    lua_pushboolean(lua, true);
                    lua_pushinteger(lua, 2);

                    auto fn = make_func(detail_Callable::static_do_bat);

                    try
                    {
                        auto r =
                            registration::detail::StaticCaller<1, bool, int>::
                                call<int>(lua, fn);

                        CHECK( r == 2 );
                    }
                    catch ( Exception& e )
                    {
                        FAIL( e.what() );
                    }
                }

                SECTION( "raises exception" )
                {
                    REQUIRE( lua_gettop(lua) == 0 );
                    lua_pushboolean(lua, true);

                    auto fn = make_func(detail_Callable::static_do_bat);

                    SECTION( "no value" )
                    {
                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<int>(lua, fn);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "no value");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }

                    SECTION( "wrong type" )
                    {
                        lua_pushstring(lua, "bar");

                        try
                        {
                            registration::detail::StaticCaller<1, bool, int>::
                                call<int>(lua, fn);

                            throw 0;
                        }
                        catch ( TypeError& e )
                        {
                            TypeError expect(2, "number", "string");
                            CHECK( e.what() == expect.what() );
                        }
                        catch ( ... )
                        {
                            FAIL( "TypeError not thrown" );
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE( "proxy" )
{
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
            CHECK( false );
        }

        SECTION( "some args" )
        {
            SECTION( "no throw" )
            {
                CHECK( false );
            }

            SECTION( "throws" )
            {
                CHECK( false );
            }
        }
    }

    SECTION( "dtor" )
    {
        SECTION( "no throw" )
        {
            CHECK( false );
        }

        SECTION( "throws" )
        {
            CHECK( false );
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
