#pragma once

#include <cassert>
#include <string>
#include <luajit-2.0/lua.hpp>

#include "shim_types.h"
#include "lua_pop.h"
#include "functional_pushers.h"

namespace Lua
{

namespace registration
{

struct TypeInfo
{
    std::string name = "";
    int methods = 0;
    int meta = 0;
    bool has_ctor = false;
    bool has_dtor = false;
    bool has_tostring = false;
};

} // namespace registration

namespace detail
{

inline bool check_key(lua_State* L, int table, const char* key, int type)
{
    Pop pop(L);

    lua_pushstring(L, key);
    lua_rawget(L, table);

    return lua_type(L, -1) == type;
}

inline int get_key(lua_State* L, int table, const char* key)
{
    lua_pushstring(L, key);
    lua_rawget(L, table);
    return lua_gettop(L);
}

registration::TypeInfo open_type(lua_State* L, const char* name)
{
    assert(name);

    registration::TypeInfo info;
    info.name = name;

    bool exists = !luaL_newmetatable(L, name);
    info.meta = lua_gettop(L);

    assert(lua_istable(L, info.meta));

    if ( exists )
    {
        // check for __gc
        info.has_dtor =
            detail::check_key(L, info.meta, "__gc", LUA_TFUNCTION);

        // check for __tostring
        info.has_tostring =
            detail::check_key(L, info.meta, "__tostring", LUA_TFUNCTION);

        // access the methods table
        info.methods =
            detail::get_key(L, info.meta, "__index");

        assert(lua_istable(L, info.methods));

        // check for ctor
        info.has_ctor =
            detail::check_key(L, info.methods, "new", LUA_TFUNCTION);
    }

    else
    {
        lua_newtable(L);
        info.methods = lua_gettop(L);

        // set methods table as metatable index
        lua_pushstring(L, "__index");
        lua_pushvalue(L, info.methods);
        lua_rawset(L, info.meta);

        // register methods table globally
        lua_pushvalue(L, info.methods);
        lua_setglobal(L, info.name.c_str());
    }

    return info;
}

template<typename T>
constexpr bool has_default_ctor()
{ return std::is_default_constructible<T>::value; }

// hack to only add default dtor if T is default constructible
template<typename T, typename = void>
struct default_ctor_adder
{
    static bool add(lua_State*, int)
    { return false; }
};

template<typename T>
struct default_ctor_adder<T, util::enable_for<has_default_ctor<T>()>>
{
    static bool add(lua_State* L, int t)
    {
        lua_pushliteral(L, "new");
        detail::constructor_pusher<T>::push(L);
        lua_rawset(L, t);
        return true;
    }
};

} // namespace detail

namespace registration
{

template<typename T>
class Editor
{
public:
    Editor(lua_State* L) :
        L(L), pop(new Pop(L)), info(L, traits::type_name_storage<T>::value)
    { }

    Editor(lua_State* L, const char* name) :
        L(L), pop(new Pop(L)), info(detail::open_type(L, name))
    { traits::type_name_storage<T>::value = name; }

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
        detail::constructor_pusher<T, Args...>::push(L);
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
        detail::destructor_pusher<T>::push(L);
        lua_rawset(L, info.meta);
        info.has_dtor = true;
        return *this;
    }

    const TypeInfo& get_info() const
    { return info; }

private:
    void add_default_ctor()
    {
        if ( detail::default_ctor_adder<T>::add(L, info.methods) )
            info.has_ctor = true;
    }

    template<typename F>
    void push_function(int table, const char* key, F fn)
    {
        lua_pushstring(L, key);
        detail::auto_pusher<F>::push(L, fn);
        lua_rawset(L, table);
    }

    lua_State* L;
    Pop* pop;
    TypeInfo info;

    // FIXIT-L find a better spot for this
    static int tostring_proxy(lua_State* L)
    {
        stack::push(L, stack::type_name<T>(L));
        return 1;
    }
};

} // namespace registration

}
