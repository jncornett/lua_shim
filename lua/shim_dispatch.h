#pragma once

#include "lua_util.h"
#include "lua_exception.h"
#include "shim_builtin.h"
#include "shim_user.h"

namespace Lua
{

namespace stack
{

template<typename T>
inline void push(lua_State* L, T val)
{ impl::push(typename traits::trait<T>::tag(), L, val); }

template<typename T>
inline T cast(lua_State* L, int n)
{ return impl::cast<T>(typename traits::trait<T>::tag(), L, n); }

template<typename T>
inline bool is(lua_State* L, int n)
{ return impl::is<T>(typename traits::trait<T>::tag(), L, n); }

template<typename T>
inline std::string type_name()
{ return impl::type_name<T>(typename traits::trait<T>::tag()); }

template<typename T>
inline void check(lua_State* L, int n)
{
    if ( !is<T>(L, n) )
        throw TypeError(
            util::abs_index(lua_gettop(L), n),
            type_name<T>(),
            lua_typename(L, lua_type(L, n))
        );
}

template<typename T>
inline T getx(lua_State* L, int n)
{
    check<T>(L, n);
    return cast<T>(L, n);
}

} // namespace stack

}
