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

#if 0
namespace
{

using namespace Shim;

inline void test_dispatch()
{
    lua_State* L = nullptr;

    stack::push<int>(int(), L);
    stack::push<double>(double(), L);
    stack::push<bool>(bool(), L);
    stack::push<std::string>(std::string(), L);

    const char* s = nullptr;
    stack::push<const char*>(s, L);

    stack::cast<int>(0, L);
    stack::cast<double>(0, L);
    stack::cast<bool>(0, L);
    stack::cast<std::string>(0, L);
    stack::cast<const char*>(0, L);

    stack::is<int>(0, L);
    stack::is<double>(0, L);
    stack::is<bool>(0, L);
    stack::is<std::string>(0, L);
    stack::is<const char*>(0, L);

    stack::type_name<int>(L);
    stack::type_name<double>(L);
    stack::type_name<bool>(L);
    stack::type_name<std::string>(L);
    stack::type_name<const char*>(L);
}

} // anonymous namespace
#endif
