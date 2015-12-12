#pragma once

#include <cassert>
#include <luajit-2.0/lua.hpp>

#include "shim_util.hpp"
#include "shim_user.hpp"
#include "lua_pop.hpp"
#include "functor_pushers.hpp"

struct lua_State;

namespace Reg
{

using namespace Shim;

namespace registration
{

struct Registry
{
    const char* name;
    int methods = 0;
    int meta = 0;
    bool has_ctor = false;
    bool has_dtor = false;
    bool has_tostring = false;
    bool exists = false;

    Registry(lua_State* L, const char* name) :
        name(name)
    {
        assert(name);

        exists = !luaL_newmetatable(L, name);
        meta = lua_gettop(L);

        assert(lua_istable(L, meta));

        if ( exists )
        {
            // check for __gc
            lua_pushstring(L, "__gc");
            lua_rawget(L, meta);
            has_dtor = lua_isfunction(L, -1);
            lua_pop(L, 1);

            // check for __tostring
            lua_pushstring(L, "__tostring");
            lua_rawget(L, meta);
            has_tostring = lua_isfunction(L, -1);
            lua_pop(L, 1);

            // access the methods table
            lua_pushstring(L, "__index");
            lua_rawget(L, meta);
            methods = lua_gettop(L);

            assert(lua_istable(L, methods));

            // check for ctor
            lua_pushstring(L, "new");
            lua_rawget(L, methods);
            // FIXIT-L J should this be lua_type(L, -1) == LUA_TFUNCTION?
            has_ctor = lua_isfunction(L, -1);
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
constexpr bool has_default_ctor()
{ return std::is_default_constructible<T>::value; }

// hack to only add default dtor if T is default constructible
template<typename T, typename = void>
struct AddDefaultCtor
{
    static bool add(lua_State*, int)
    { return false; }
};

template<typename T>
struct AddDefaultCtor<T, Shim::util::enable_if<has_default_ctor<T>()>>
{
    static bool add(lua_State* L, int t)
    {
        lua_pushstring(L, "new");
        util::constructor_pusher<T>::push(L);
        lua_rawset(L, t);
        return true;
    }
};

template<typename T>
class Editor
{
public:
    Editor(lua_State* L) :
        L(L), pop(new Pop(L)), info(L, type_name_storage<T>::value)
    { }

    Editor(lua_State* L, const char* name) :
        L(L), pop(new Pop(L)), info(L, name)
    { type_name_storage<T>::value = name; }

    // disable copy construction
    Editor(const Editor&) = delete;

    Editor(Editor&& o) :
        L(o.L), pop(o.pop), info(o.info)
    { o.pop = nullptr; }

    ~Editor()
    { finish(); }

    // disable copy/move assignment
    Editor& operator=(const Editor&) = delete;
    Editor& operator=(Editor&&) = delete;

    void finish()
    {
        if ( pop )
        {
            if ( !info.has_ctor )
                add_default_ctor();

            if ( !info.has_dtor )
                add_dtor();

            if ( !info.has_tostring )
            {
                push_function(info.meta, "__tostring", tostring_proxy);
                info.has_tostring = true;
            }

            delete pop;
            pop = nullptr;
        }
    }

    template<typename F>
    Editor& add_method(const char* key, F fn)
    {
        assert(pop);
        push_function(info.methods, key, fn);
        return *this;
    }

    template<typename F>
    Editor& add_ctor(F fn)
    {
        assert(pop);
        push_function(info.methods, "new", fn);
        info.has_ctor = true;
        return *this;
    }

    template<typename... Args>
    Editor& add_ctor()
    {
        assert(pop);
        lua_pushstring(L, "new");
        util::constructor_pusher<T, Args...>::push(L);
        lua_rawset(L, info.methods);
        info.has_ctor = true;
        return *this;
    }

    template<typename F>
    Editor& add_dtor(F fn)
    {
        assert(pop);
        push_function(info.meta, "__gc", fn);
        info.has_dtor = true;
        return *this;
    }

    Editor& add_dtor()
    {
        assert(pop);
        lua_pushstring(L, "__gc");
        util::destructor_pusher<T>::push(L);
        lua_rawset(L, info.meta);
        info.has_dtor = true;
        return *this;
    }

    const Registry& get_info() const
    { return info; }

private:
    void add_default_ctor()
    {
        if ( AddDefaultCtor<T>::add(L, info.methods) )
            info.has_ctor = true;
    }

    template<typename F>
    void push_function(int table, const char* key, F fn)
    {
        lua_pushstring(L, key);
        util::auto_pusher<F>::push(L, fn);
        lua_rawset(L, table);
    }

    lua_State* L;
    Pop* pop;
    Registry info;

    // FIXIT-L find a better spot for this
    static int tostring_proxy(lua_State* L)
    {
        stack::push(L, stack::type_name<T>(L));
        return 1;
    }
};

} // namespace registration

}
