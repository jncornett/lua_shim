#pragma once

#include <functional>
#include <luajit-2.0/lua.hpp>

#include "shim_lua_util.h"

namespace Shim
{

template<typename F>
struct pusher {};

// static function
template<typename Return, typename... Args>
struct pusher<Return(*)(Args...)>
{
    template<typename F>
    static void push_static_method(lua_State*, F fn)
    {
        std::function<Return(Args...)> handle(fn);
    }

    template<typename Class, typename F>
    static void push_method(lua_State*, F fn)
    {
        std::function<Return(Args...)> handle(fn);
    }
};

// member function
template<typename Return, typename Class, typename... Args>
struct pusher<Return(Class::*)(Args...)>
{
};

// function object
template<typename Return, typename... Args>
struct pusher<std::function<Return(Args...)>>
{
    template<typename F>
    static int static_proxy(lua_State* L)
    {
        auto& fn = util::GC<F>::get(L, lua_upvalueindex(1));
        if ( std::is_void<Return>::value )
        {
        }
    }

    template<typename F>
    static void push_static_method(lua_State* L, F fn)
    {
        util::GC<F>::push(L, fn);
    }

    template<typename F>
    static void push_method(lua_State* L, F fn)
    {
        util::GC<F>::push(L, fn);
    }
};

// function object specialized for void return
template<typename... Args>
struct pusher<std::function<void(Args...)>>
{
    template<typename F>
    static int static_proxy(lua_State* L)
    {
        auto& fn = util::GC<F>::get(L, lua_upvalueindex(1));
        {
        }
    }

    template<typename F>
    static void push_static_method(lua_State* L, F fn)
    {
        util::GC<F>::push(L, fn);
    }

    template<typename F>
    static void push_method(lua_State* L, F fn)
    {
        util::GC<F>::push(L, fn);
    }
};

// raw lua cfunction
template<>
struct pusher<lua_CFunction>
{
    static void push(lua_State* L, lua_CFunction fn)
    { lua_pushcfunction(L, fn); }
};


}
