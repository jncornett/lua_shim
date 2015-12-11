#pragma once

#include <utility>

// function argument applier helpers

// Possible implementation of a Getter:
//
//     struct Getter
//     {
//         std::vector<void*> args;
//
//         template<typename T>
//         T get(int argn)
//         { return *(T*)args[argn]; }
//     };

namespace Shim
{

namespace util
{

namespace detail
{

template<int N, typename... Pack>
struct NewApplier
{
    template<typename Class, typename Getter, typename... Args>
    static Class* apply(Getter&, Args&&... args)
    { return new Class(std::forward<Args>(args)...); }
};

template<int N, typename Next, typename... Pack>
struct NewApplier<N, Next, Pack...>
{
    template<typename Class, typename Getter, typename... Args>
    static Class* apply(Getter& getter, Args&&... args)
    {
        return NewApplier<N+1, Pack...>::template apply<Class>(getter,
            std::forward<Args>(args)..., getter.template get<Next>(N));
    }
};

template<int N, typename... Pack>
struct FunctorApplier
{
    template<typename Return, typename Getter, typename F, typename... Args>
    static Return apply(Getter&, F fn, Args&&... args)
    { return fn(std::forward<Args>(args)...); }
};

template<int N, typename Next, typename... Pack>
struct FunctorApplier<N, Next, Pack...>
{
    template<typename Return, typename Getter, typename F, typename... Args>
    static Return apply(Getter& getter, F fn, Args&&... args)
    {
        return FunctorApplier<N+1, Pack...>::template apply<Return>(getter, fn,
            std::forward<Args>(args)..., getter.template get<Next>(N));
    }
};

} // namespace detail

// front end for detail::NewApplier inverts the arguments to make it easier to use
template<typename Class, typename... Pack>
struct NewApplier
{
    template<typename Getter, typename... Args>
    static Class* call(Getter& getter, Args&&... args)
    {
        return detail::NewApplier<1, Pack...>::template apply<Class>(getter,
            std::forward<Args>(args)...);
    }
};

// front end for detail::FunctorApplier inverts the arguments to make it easier to use
template<typename Return, typename... Pack>
struct FunctorApplier
{
    template<typename Getter, typename F, typename... Args>
    static Return call(Getter& getter, F fn, Args&&... args)
    {
        return detail::FunctorApplier<1, Pack...>::template apply<Return>(
            getter, fn, std::forward<Args>(args)...);
    }
};

} // namespace util

}

#if 0
namespace _test_applier
{

struct Getter
{
    template<typename T>
    T get(int N)
    { return N; }
};

struct Class
{
    Class(int, float) {}
};

void void_functor(bool, int) {}
int int_functor(bool, int) { return 0; }


void test()
{
    Getter g;
    Class* cls = Shim::util::NewApplier<Class, float>::call(g, 1);
    (void)cls;

    Shim::util::FunctorApplier<void, int>::call(g, void_functor, true);

    int result = Shim::util::FunctorApplier<int, int>::call(g, int_functor, true);
    (void)result;
}

} // _test_applier
#endif
