#include "shim_dispatch.h"

#include "common.h"

#undef d_v
#define d_v 3

namespace t_shim_dispatch
{

// -----------------------------------------------------------------------------
// fixtures
// -----------------------------------------------------------------------------

template<typename T>
void integral_stack_test(lua_State* L, int type, T v, const char* name)
{
    using namespace Lua;

    CHECK( stack::type_name<T>() == name );

    stack::push(L, v);
    REQUIRE( lua_type(L, -1) == type );
    CHECK( stack::is<T>(L, -1) );
    CHECK( stack::cast<T>(L, -1) == v );
}

template<typename T>
void string_stack_test(lua_State* L, T v)
{
    using namespace Lua;

    CHECK( stack::type_name<T>() == "string" );

    stack::push(L, v);
    REQUIRE( lua_type(L, -1) == LUA_TSTRING );
    CHECK( stack::is<T>(L, -1) );

    std::string t = stack::cast<T>(L, -1);
    CHECK( t == v );
}

struct TUser
{ int x = d_v; };

enum TEnum : char { A = 1 };
enum class TEnumClass : char { A = 1 };

} // namespace t_shim_dispatch


// -----------------------------------------------------------------------------
// test cases
// -----------------------------------------------------------------------------

TEST_CASE ( "dispatch" )
{
    using namespace Lua;
    using namespace t_shim_dispatch;

    State lua;

    SECTION( "integral types" )
    {
        int i = 12;
        double d = 3.14;
        bool b = true;
        unsigned u = 5;

        integral_stack_test(lua, LUA_TNUMBER, i, "integer");
        integral_stack_test(lua, LUA_TNUMBER, d, "number");
        integral_stack_test(lua, LUA_TNUMBER, u, "unsigned");
        integral_stack_test(lua, LUA_TBOOLEAN, b, "boolean");

        SECTION ( "is not" )
        {
            lua_pushnil(lua);
            CHECK_FALSE( stack::is<int>(lua, -1) );
            CHECK_FALSE( stack::is<double>(lua, -1) );
            CHECK_FALSE( stack::is<bool>(lua, -1) );
            CHECK_FALSE( stack::is<unsigned>(lua, -1) );

            lua_pushinteger(lua, -42);
            CHECK_FALSE( stack::is<unsigned>(lua, -1) );
        }
    }

    SECTION( "string types" )
    {
        std::string s = "foo";
        const char* c = "bar";

        string_stack_test(lua, s);
        string_stack_test(lua, c);

        SECTION ( "is not" )
        {
            lua_pushnil(lua);
            CHECK_FALSE( stack::is<std::string>(lua, -1) );
            CHECK_FALSE( stack::is<const char*>(lua, -1) );
        }
    }

    SECTION( "enum type" )
    {
        SECTION( "old enum" )
        {
            stack::push(lua, TEnum::A);
            CHECK( stack::is<TEnum>(lua, -1) );
            CHECK( stack::cast<TEnum>(lua, -1) == TEnum::A );
        }

        SECTION( "enum class" )
        {
            stack::push(lua, TEnumClass::A);
            CHECK( stack::is<TEnumClass>(lua, -1) );
            CHECK( stack::cast<TEnumClass>(lua, -1) == TEnumClass::A );
        }
    }

    SECTION( "user type" )
    {
        // must register type first
        traits::type_name_storage<TUser>::value = "TUser";

        CHECK( stack::type_name<TUser>() == "TUser" );

        std::unique_ptr<TUser> u(new TUser);

        auto h = static_cast<TUser**>(lua_newuserdata(lua, sizeof(TUser*)));
        *h = u.get();

        luaL_newmetatable(lua, "TUser");
        lua_setmetatable(lua, -2);

        CHECK( stack::is<TUser>(lua, -1) );

        TUser x = stack::cast<TUser>(lua, -1);
        CHECK( u->x == x.x );

        TUser& r = stack::cast<TUser&>(lua, -1);
        CHECK( u->x == r.x );

        TUser* p = stack::cast<TUser*>(lua, -1);
        CHECK( u->x == p->x );

        SECTION( "is not" )
        {
            lua_pushnil(lua);
            CHECK_FALSE( stack::is<TUser>(lua, -1) );

            lua_pushinteger(lua, 3);
            CHECK_FALSE( stack::is<TUser>(lua, -1) );
        }
    }

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
                "TypeError: (arg #2) expected 'integer', got 'nil'" ;

            CHECK( e.what() == exp );
        }
        catch ( int )
        {
            FAIL( "failed to throw a TypeError" );
        }
    }
}
