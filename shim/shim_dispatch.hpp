#pragma once

#include <string>

#include "shim_builtin.hpp"
#include "shim_user.hpp"

struct lua_State;

namespace Shim
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
inline std::string type_name(lua_State* L)
{ return impl::type_name<T>(typename traits::trait<T>::tag(), L); }

} // namespace stack

} // namespace Shim
