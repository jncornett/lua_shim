#pragma once

#include <functional>
#include <luajit-2.0/lua.hpp>

#include "shim_user.h"
#include "shim_lua_util.h"
#include "shim_dispatchx.h"
#include "shim_applier.h"

#include "lua_debug.h"

namespace Shim
{

namespace registration
{

namespace deconstruction
{

template<typename F>
struct deconstruct_mem_fn {};

template<typename R, typename... Args>
struct deconstruct_mem_fn<R(Args...)>
{
    static void foo()
    { std::cout << "deconstructing R(Args...)" << std::endl; }
};

template<typename R, typename... Args>
struct deconstruct_mem_fn<R(*)(Args...)>
{
    static void foo()
    { std::cout << "deconstructing R(*)(Args...)" << std::endl; }
};

template<typename R, typename Class, typename... Args>
struct deconstruct_mem_fn<R(Class::*)(Args...)>
{
    static void foo()
    { std::cout << "deconstructing R(Class::*)(Args...)" << std::endl; }
};

template<typename R, typename... Args>
struct deconstruct_mem_fn<std::function<R(Args...)>>
{
    static void foo()
    { std::cout << "deconstructing std::function<R(Args...)>" << std::endl; }
};

template<typename R, typename Class, typename... Args>
struct deconstruct_mem_fn<std::function<R(Class&, Args...)>>
{
    static void foo()
    { std::cout << "deconstructing std::function<R(Class&, Args...)>" << std::endl; }
};





} // namespace deconstruction

template<typename F>
struct anon
{
    static int gc(lua_State* L)
    { udata<F>::destroy(L, 1); return 0; }

    static F& get(lua_State* L, int n)
    { return udata<F>::extract_handle(L, n); }
};

template<typename Class>
struct proxy
{
    template<typename F, typename Return, typename... Args>
    static int static_fn(lua_State* L);

    template<typename F, typename Return, typename... Args>
    static int fn(lua_State* L);

    template<typename... Args>
    static int ctor(lua_State* L)
    {
        auto h = udata<Class>::allocate(L);
        assert(h);

        try
        {
            *h = appliers::New<1, Args...>::template apply<Class>(L);
            assert(*h);
        }
        catch (Exception& e)
        {
            // FIXIT-H J do we need to perform cleanup for e?
            lua_pushstring(L, e.what().c_str());
            lua_error(L);
        }

        return 1;
    }

    static int dtor(lua_State* L)
    {
        // FIXIT-L do we need to protect this call with a getx?
        udata<Class>::destroy(L, 1);
        return 0;
    }

    template<typename F>
    static int raw_fn(lua_State* L)
    { return anon<F>::get(L, lua_upvalueindex(1))(L); }
};

inline void push_cfunction(lua_CFunction fn, int t, const char* key,
    lua_State* L)
{
    lua_pushstring(L, key);
    lua_pushcfunction(L, fn);
    lua_rawset(L, t);
}

struct Registry
{
    const char* name;
    int methods;
    int meta;
    bool has_ctor;
    bool has_dtor;
    bool exists;

    Registry(lua_State* L, const char* name) :
        name(name), has_ctor(false), has_dtor(false)
    {
        assert(name);

        exists = !luaL_newmetatable(L, name);
        meta = lua_gettop(L);

        // FIXIT-L J should this be lua_type(L, meta) == LUA_TTABLE?
        assert(!lua_isnoneornil(L, meta));
        if ( exists )
        {
            // check for dtor
            lua_pushstring(L, "__gc");
            lua_rawget(L, meta);
            has_dtor = !lua_isnoneornil(L, -1);
            lua_pop(L, 1);

            lua_pushstring(L, "__index");
            lua_rawget(L, meta);
            methods = lua_gettop(L);

            // FIXIT-L J should this be lua_type(L, methods) == LUA_TTABLE?
            assert(!lua_isnoneornil(L, methods));

            // check for ctor
            lua_pushstring(L, "new");
            // FIXIT-L J should this be lua_type(L, -1) == LUA_TFUNCTION?
            has_ctor = !lua_isnoneornil(L, -1);
            lua_pop(L, 1);
        }

        else
        {
            lua_newtable(L);
            methods = lua_gettop(L);

            lua_pushstring(L, "__index");
            lua_pushvalue(L, methods);
            lua_rawset(L, meta);

            lua_pushvalue(L, methods);
            lua_setglobal(L, name);
        }
    }
};

template<typename T>
class Adder
{
public:
    Adder(lua_State* L, const char* name) :
        L(L), pop(new util::Pop(L)), info(L, name) { }

    Adder(const Adder&) = delete;
    Adder(Adder&& o) :
        L(o.L), pop(o.pop), info(o.info)
    { pop = nullptr; }

    virtual ~Adder()
    { finalize(); }

    Adder& operator=(const Adder&) = delete;

    // FIXIT-M J specialize on signature
    template<typename F>
    Adder& add_ctor(F)
    {
        assert(!finalized());
        return *this;
    }

    template<typename... Args>
    Adder& add_ctor()
    {
        assert(!finalized());
        push_raw_method("new", proxy<T>::template ctor<Args...>);
        return *this;
    }

    // FIXIT-M J specialize on signature
    template<typename F>
    Adder& add_dtor(F)
    {
        assert(!finalized());
        return *this;
    }

    Adder& add_dtor()
    {
        assert(!finalized());
        push_raw_metamethod("__gc", proxy<T>::dtor);
        return *this;
    }

    // FIXIT-M J specialize on signature
    template<typename F>
    Adder& add_function(const char*, F)
    {
        assert(!finalized());
        deconstruction::deconstruct_mem_fn<F>::foo();
        return *this;
    }

    // FIXIT-M J specialize on signature
    template<typename F>
    Adder& add_static_function(const char*, F)
    {
        assert(!finalized());
        return *this;
    }

    virtual void finalize()
    {
        if ( !finalized() )
        {
            if ( !info.has_ctor )
                add_ctor();

            if ( !info.has_dtor )
                add_dtor();

            delete pop;
            pop = nullptr;
        }
    }

protected:
    bool finalized() const
    { return pop == nullptr; }

    void push_raw_function(int t, const char* n, lua_CFunction fn)
    {
        lua_pushstring(L, n);
        lua_pushcfunction(L, fn);
        lua_rawset(L, t);
    }

    void push_raw_metamethod(const char* n, lua_CFunction fn)
    { push_raw_function(info.meta, n, fn); }

    void push_raw_method(const char* n, lua_CFunction fn)
    { push_raw_function(info.methods, n, fn); }

    lua_State* L;
    util::Pop* pop;
    Registry info;
};

template<typename T>
class Editor : public Adder<T>
{
public:
    Editor(lua_State* L) :
        Adder<T>(L, Registry(L, type_name_storage<T>::value)) { }
};

template<typename T>
class Creator : public Adder<T>
{
public:
    Creator(lua_State* L, const char* name) :
        Adder<T>(L, name)
    { type_name_storage<T>::value = name; }

    Creator(Creator&& o) : Adder<T>(std::move(o)) { }
};

} // namespace registration

template<typename T>
registration::Creator<T> open_class(const char* name, lua_State* L)
{ return registration::Creator<T>(L, name); }

template<typename T>
registration::Editor<T> open_class(lua_State* L)
{ return registration::Editor<T>(L); }

}
