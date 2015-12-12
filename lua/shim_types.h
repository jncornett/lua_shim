#pragma once

#include <string>
#include <type_traits>

#include <luajit-2.0/lua.hpp>

// type traits helpers

namespace Lua
{

namespace util
{

template<typename T, typename U>
constexpr bool is_same()
{ return std::is_same<typename std::remove_const<T>::type, U>::value; }

template<typename T>
constexpr bool is_bool()
{ return is_same<T, bool>(); }

template<typename T>
constexpr bool is_signed_int()
{ return std::is_signed<T>::value and sizeof(T) <= sizeof(lua_Integer); }

template<typename T>
constexpr bool is_unsigned_int()
{
    return std::is_unsigned<T>::value and
        sizeof(T) < sizeof(lua_Integer) and
        !is_bool<T>();
}

template<typename T>
constexpr bool is_int()
{ return is_signed_int<T>() or is_unsigned_int<T>(); }

template<typename T>
constexpr bool is_fp()
{ return std::is_floating_point<T>::value and sizeof(T) <= sizeof(lua_Number); }

template<typename T>
constexpr bool is_num()
{ return is_int<T>() || is_fp<T>(); }

template<typename T>
constexpr bool is_c_str()
{
    return (
        std::is_pointer<T>::value and
        is_same<typename std::remove_pointer<T>::type, char>()
    ) or (
        std::is_array<T>::value and
        is_same<typename std::remove_extent<T>::type, char>()
    );
}

template<typename T>
constexpr bool is_std_str()
{ return is_same<T, std::string>(); }

template<typename T>
constexpr bool is_str()
{ return is_std_str<T>() or is_c_str<T>(); }

template<typename T>
constexpr bool is_builtin()
{ return is_bool<T>() or is_num<T>() or is_str<T>(); }

template<bool B, typename T = void>
using enable_for = typename std::enable_if<B, T>::type;

template<typename T, typename = void>
struct base
{ using type = typename std::remove_const<T>::type; };

template<typename T>
struct base<T, enable_for<std::is_reference<T>::value>>
{ using type = typename base<typename std::remove_reference<T>::type>::type; };

template<typename T>
struct base<T, enable_for<std::is_pointer<T>::value>>
{ using type = typename base<typename std::remove_pointer<T>::type>::type; };

template<typename T>
constexpr bool is_user_object()
{ return std::is_class<T>::value and !is_builtin<T>(); }

template<typename T>
constexpr bool is_user_ref()
{ return std::is_reference<T>::value and is_user_object<typename base<T>::type>(); }

template<typename T>
constexpr bool is_user_ptr()
{ return std::is_pointer<T>::value and is_user_object<typename base<T>::type>(); }

template<typename T>
constexpr bool is_user()
{ return is_user_object<T>() or is_user_ref<T>() or is_user_ptr<T>(); }

template<typename T>
constexpr bool is_user_pod()
{ return (is_user_object<T>() or is_user_ref<T>()) and std::is_pod<T>::value; }

} // namespace util

}
