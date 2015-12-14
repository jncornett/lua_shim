#pragma once

#include <functional>

#include "functional_appliers.h"
#include "lua_gcobject.h"
#include "shim_dispatch.h"

namespace Lua
{

namespace util
{

struct Getter
{
    lua_State* L;

    template<typename T>
    T get(int n)
    { return stack::getx<T>(L, n); }
};

} // namespace util

namespace detail
{

// have to break this out of method_pusher to specialize on return type :(
template<typename Return, typename... Args>
struct proxy_inner
{
    template<typename Getter, typename Func>
    static int proxy(lua_State* L, Getter& getter, Func& func)
    {
        auto ret = functor_applier<1, Return, Args...>::apply(getter, func);
        stack::push(L, ret);
        return 1;
    }
};

template<typename... Args>
struct proxy_inner<void, Args...>
{
    template<typename Getter, typename Func>
    static int proxy(lua_State*, Getter& getter, Func& func)
    {
        functor_applier<1, void, Args...>::apply(getter, func);
        return 0;
    }
};

template<typename Return, typename... Args>
struct method_pusher
{
    using Func = std::function<Return(Args...)>;

    static int proxy(lua_State* L)
    {
        try
        {
            util::Getter getter { L };
            auto func = util::gc_object::cast<Func>(L, lua_upvalueindex(1));
            return proxy_inner<Return, Args...>::proxy(L, getter, func);
        }

        catch ( TypeError& e )
        {
            stack::push(L, e.what());
        }

        // wait until the catch block exits before calling lua_error
        // to ensure stack cleanup
        lua_error(L);

        // because the compiler doesn't know that lua_error does a long jump
        return 0;
    }

    static void push(lua_State* L, Func func)
    {
        util::gc_object::push(L, func);
        lua_pushcclosure(L, proxy, 1);
    }
};

template<typename Class, typename... Args>
struct constructor_pusher
{
    static int proxy(lua_State* L)
    {
        try
        {
            util::Getter getter { L };
            auto inst = new_applier<1, Class, Args...>::apply(getter);
            util::userdata<Class>::push(L, *inst);
            util::userdata<Class>::assign_metatable(L, -1);
            return 1;
        }

        catch ( TypeError& e )
        {
            stack::push(L, e.what());
        }

        // wait until the catch block exits before calling lua_error
        // to ensure stack cleanup
        lua_error(L);

        // since the compiler doesn't know that lua_error()
        // does a long jump
        return 0;
    }

    static void push(lua_State* L)
    { lua_pushcfunction(L, proxy); }
};

template<typename Class>
struct destructor_pusher
{
    static int proxy(lua_State* L)
    {
        try
        {
            stack::check<Class>(L, 1);
            util::userdata<Class>::destroy(L, 1);
            return 0;
        }

        catch ( TypeError& e )
        {
            stack::push(L, e.what());
        }

        // wait until the catch block exits before calling lua_error
        // to ensure stack cleanup
        lua_error(L);

        // since the compiler doesn't know that lua_error()
        // does a long jump
        return 0;
    }

    static void push(lua_State* L)
    { lua_pushcfunction(L, proxy); }
};

template<typename F>
struct auto_pusher {};

// function pointer
template<typename Return, typename... Args>
struct auto_pusher<Return(*)(Args...)>
{
    template<typename F>
    static void push(lua_State* L, F fn)
    { method_pusher<Return, Args...>::push(L, fn); }
};

// function
template<typename Return, typename... Args>
struct auto_pusher<Return(Args...)>
{
    template<typename F>
    static void push(lua_State* L, F fn)
    { method_pusher<Return, Args...>::push(L, fn); }
};

// member function pointer
template<typename Return, typename Class, typename... Args>
struct auto_pusher<Return(Class::*)(Args...)>
{
    template<typename F>
    static void push(lua_State* L, F fn)
    { method_pusher<Return, Class&, Args...>::push(L, std::mem_fn(fn)); }
};

// const member function pointer
template<typename Return, typename Class, typename... Args>
struct auto_pusher<Return(Class::*)(Args...) const>
{
    template<typename F>
    static void push(lua_State* L, F fn)
    { method_pusher<Return, Class&, Args...>::push(L, std::mem_fn(fn)); }
};

// function object
template<typename Return, typename... Args>
struct auto_pusher<std::function<Return(Args...)>>
{
    template<typename F>
    static void push(lua_State* L, F fn)
    { method_pusher<Return, Args...>::push(L, fn); }
};

// const function object
template<typename Return, typename... Args>
struct auto_pusher<const std::function<Return(Args...)>>
{
    template<typename F>
    static void push(lua_State* L, F fn)
    { method_pusher<Return, Args...>::push(L, fn); }
};

// raw lua cfunction pointer
template<>
struct auto_pusher<lua_CFunction>
{
    static void push(lua_State* L, lua_CFunction fn)
    { lua_pushcfunction(L, fn); }
};

// raw lua cfunction
template<>
struct auto_pusher<int(lua_State*)>
{
    static void push(lua_State* L, lua_CFunction fn)
    { lua_pushcfunction(L, fn); }
};

// FIXIT-L add specializations for extended custom functions

} // namespace util

}
