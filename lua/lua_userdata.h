#pragma once

#include "shim_types.h"

// helpers for working with Lua userdata

namespace Lua
{

namespace util
{

template<typename T>
struct userdata
{
    using base_type = typename util::base<T>::type;

// FIXIT-H enable when used
#if 0
    template<typename... Args>
    static base_type* emplace(lua_State*, T val);

    static void push(lua_State*, T);
#endif

    static base_type** extract(lua_State* L, int n)
    {
        auto h = static_cast<base_type**>(lua_touserdata(L, n));
        assert(h);
        return h;
    }

    static base_type** allocate(lua_State* L)
    {
        auto h = static_cast<base_type**>(lua_newuserdata(L, sizeof(base_type*)));
        assert(h);
        return h;
    }

    template<typename... Args>
    static base_type* emplace(lua_State* L, Args&&... args)
    {
        auto h = allocate(L);
        *h = new base_type(std::forward<Args>(args)...);
        assert(*h);
        return *h;
    }

    // deletes a userdata and sets its pointer to nullptr
    static void destroy(lua_State* L, int n)
    {
        auto h = extract(L, n);
        if ( *h )
        {
            delete *h;
            *h = nullptr;
        }
    }
};

} // namespace util

}
