#include "type_registration.hpp"
#include "common.hpp"

namespace t_type_registration
{
// -----------------------------------------------------------------------------
// fixtures
// -----------------------------------------------------------------------------

struct TUser
{
    static int foo() { return 1; }
    int bar() const { return 2; }
    int buzz() { return 3; }

    int x = 0;

    TUser() { }
    TUser(int y) : x(y) { }
};

static_assert(std::is_default_constructible<TUser>::value, "");

template<typename T>
static void register_class(lua_State* L, const char* name)
{
    Shim::type_name_storage<T>::value = name;
    luaL_newmetatable(L, name);
    auto meta = lua_gettop(L);
    lua_newtable(L);
    auto methods = lua_gettop(L);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, methods);
    lua_rawset(L, meta);
    lua_setglobal(L, name);
    lua_pop(L, 1);
}

template<typename T>
static void unregister_class(lua_State* L)
{
    const char* name = Shim::type_name_storage<T>::value;
    if ( !name )
        return;

    lua_pushstring(L, name);
    lua_pushnil(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
    lua_pushnil(L);
    lua_setglobal(L, name);

    Shim::type_name_storage<T>::value = nullptr;
}

}

// -----------------------------------------------------------------------------
// test cases
// -----------------------------------------------------------------------------

TEST_CASE( "registry info" )
{
    using namespace Reg;
    using namespace t_type_registration;

    State lua;

    SECTION( "on a new object" )
    {
        // erase any existing info
        unregister_class<TUser>(lua);

        registration::Registry info(lua, "TUser");

        std::string s = info.name;
        CHECK( s == "TUser" );
        CHECK( lua_istable(lua, info.methods) );
        CHECK( lua_istable(lua, info.meta) );
        CHECK_FALSE( info.has_ctor );
        CHECK_FALSE( info.has_dtor );
    }

    SECTION( "on an object already registered" )
    {
        register_class<TUser>(lua, "TUser");

        SECTION( "no ctor/dtor" )
        {
            registration::Registry info(lua, "TUser");
            CHECK_FALSE( info.has_ctor );
            CHECK_FALSE( info.has_dtor );
        }

        SECTION( "has ctor" )
        {
            {
                // append a ctor
                REQUIRE_FALSE( luaL_newmetatable(lua, "TUser") );
                REQUIRE( lua_istable(lua, -1) );
                lua_pushstring(lua, "__index");
                lua_rawget(lua, -2);
                REQUIRE( lua_istable(lua, -1) );
                lua_pushstring(lua, "new");
                lua_pushcfunction(lua, [](lua_State*) { return 0; });
                lua_rawset(lua, -3);
                lua_pop(lua, 2);
            }

            registration::Registry info(lua, "TUser");
            CHECK( info.has_ctor );
        }

        SECTION( "has dtor" )
        {
            {
                // append a dtor
                REQUIRE_FALSE( luaL_newmetatable(lua, "TUser") );
                lua_pushstring(lua, "__gc");
                lua_pushcfunction(lua, [](lua_State*) { return 0; });
                lua_rawset(lua, -3);
                lua_pop(lua, 1);
            }

            registration::Registry info(lua, "TUser");
            CHECK( info.has_dtor );
        }
    }
}

TEST_CASE( "class editor" )
{
    using namespace Reg;
    using namespace t_type_registration;

    State lua;

    SECTION( "new class" )
    {
        unregister_class<TUser>(lua);


        SECTION( "ctor/dtor automatically added" )
        {
            registration::Editor<TUser> editor(lua, "TUser");
            editor.finish();

            const auto& info = editor.get_info();

            CHECK( info.has_ctor );
            CHECK( info.has_dtor );
        }
    }

    SECTION( "full test" )
    {
        unregister_class<TUser>(lua);

        {
            registration::Editor<TUser>(lua, "TUser")
                .add_method("foo", &TUser::foo)
                .add_method("bar", &TUser::bar)
                .add_method("buzz", &TUser::buzz)
                .add_ctor<int>();
        }

        luaL_dostring(lua, "print(TUser)");
        luaL_dostring(lua, "for i, v in pairs(TUser) do print(i, v) end");
        luaL_dostring(lua, "u = TUser.new(2)");
        luaL_dostring(lua, "print(u)");
    }
}
