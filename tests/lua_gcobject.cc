#include "lua_gcobject.h"

#include "common.h"

namespace t_lua_gcobject
{

struct TUser
{
    static bool destructor_called;

    int x;
    TUser(int y) : x(y) { }
    ~TUser() { destructor_called = true; }
};

bool TUser::destructor_called = false;

} // namespace t_lua_gcobject


TEST_CASE( "gc object" )
{
    using namespace t_lua_gcobject;

    {
        State lua;

        int v = 4;

        {
            TUser u(v);
            Lua::util::gc_object::push(lua, u);
        }

        auto ur = Lua::util::gc_object::cast<TUser>(lua, -1);
        CHECK( ur.x == v );

        TUser::destructor_called = false;
    }

    CHECK( TUser::destructor_called );
}
