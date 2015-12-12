#pragma once

#include <functional>

#include "shim_builtin.hpp"
#include "shim_user.hpp"
#include "shim_exception.hpp"
#include "functional_appliers.hpp"
#include "lua_gcobject.hpp"
#include "lua_getter.hpp"

namespace Reg
{

namespace util
{

using namespace Shim;

// have to break this out of method_pusher to specialize on return type :(
template<typename Return, typename... Args>
struct method_proxy_inner
{
    template<typename Getter, typename Func>
    static int proxy(lua_State* L, Getter& getter, Func& func)
    {
        auto ret = FunctorApplier<Return, Args...>::call(getter, func);
        stack::push(L, ret);
        return 1;
    }
};

template<typename... Args>
struct method_proxy_inner<void, Args...>
{
    template<typename Getter, typename Func>
    static int proxy(lua_State*, Getter& getter, Func& func)
    {
        FunctorApplier<void, Args...>::call(getter, func);
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
            StackGetter getter { L };
            auto func = gc_object::get<Func>(L, lua_upvalueindex(1));
            return method_proxy_inner<Return, Args...>::proxy(L, getter, func);
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
        gc_object::push(L, func);
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
            StackGetter getter { L };
            auto cls = NewApplier<Class, Args...>::call(getter);
            udata<Class>::assign(L, cls);
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
            udata<Class>::destroy(L, 1);
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
