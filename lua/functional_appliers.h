#pragma once

#include <utility>

// function argument applier helpers

// Possible implementation of a Getter:
//
//     struct Getter
//     {
//         lua_State* L;
//
//         template<typename T>
//         T get(int argn)
//         { return stack::cast<T>(L, argn); }
//     };

namespace Lua
{

namespace detail
{

// base case
template<int N, typename Class, typename... Pack>
struct new_applier
{
    template<typename Getter, typename... Args>
    static Class* apply(Getter&, Args&&... args)
    { return new Class(std::forward<Args>(args)...); }
};

template<int N, typename Class, typename Next, typename... Pack>
struct new_applier<N, Class, Next, Pack...>
{
    template<typename Getter, typename... Args>
    static Class* apply(Getter& getter, Args&&... args)
    {
        return new_applier<N+1, Class, Pack...>::apply(getter,
            std::forward<Args>(args)..., getter.template get<Next>(N));
    }
};

// usage:
//     T* instance = new_applier<1, T, Params...>::apply(getter, concrete_args...);
// 
// - to start at stack position 2, replace N = 1 with N = 2, etc.
// - concrete_args... can be any args that do not need to be extracted from the getter

// base case
template<int N, typename Return, typename... Pack>
struct functor_applier
{
    template<typename Getter, typename F, typename... Args>
    static Return apply(Getter&, F fn, Args&&... args)
    { return fn(std::forward<Args>(args)...); }
};

template<int N, typename Return, typename Next, typename... Pack>
struct functor_applier<N, Return, Next, Pack...>
{
    template<typename Getter, typename F, typename... Args>
    static Return apply(Getter& getter, F fn, Args&&... args)
    {
        return functor_applier<N+1, Return, Pack...>::apply(getter, fn,
            std::forward<Args>(args)..., getter.template get<Next>(N));
    }
};

// usage:
//     Return ret = functor_applier<1, Return, Params...>::apply(getter, concrete_args...);
// 
// - to start at stack position 2, replace N = 1 with N = 2, etc.
// - concrete_args... can be any args that do not need to be extracted from the getter

} // namespace detail

}
