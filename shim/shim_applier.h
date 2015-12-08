#pragma once

#include <utility>
#include <luajit-2.0/lua.hpp>

#include "shim_dispatchx.h"

namespace Shim
{

namespace appliers
{

template<int N, typename... Pack>
struct New
{
    template<typename Class, typename... Args>
    static Class* apply(lua_State*, Args&&... args)
    { return new Class(std::forward<Args>(args)...); }
};

template<int N, typename Next, typename... Pack>
struct New<N, Next, Pack...>
{
    template<typename Class, typename... Args>
    static Class* apply(lua_State* L, Args&&... args)
    {
        return New<N+1, Pack...>::template apply<Class>(L,
            std::forward<Args>(args)..., stack::getx<Next>(L, N));
    }
};

template<int N, typename... Pack>
struct Function
{
    template<typename Return, typename F, typename... Args>
    static Return apply(lua_State*, F fn, Args&&... args)
    { return fn(std::forward<Args>(args)...); }
};

template<int N, typename Next, typename... Pack>
struct Function<N, Next, Pack...>
{
    template<typename Return, typename F, typename... Args>
    static Return apply(lua_State* L, F fn, Args&&... args)
    {
        return Function<N+1, Pack...>::template apply<Return>(L, fn,
            std::forward<Args>(args)..., stack::getx<Next>(L, N));
    }
};

} // namespace applier

}
