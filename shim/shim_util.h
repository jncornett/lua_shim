#pragma once

#include <type_traits>

namespace Shim
{
namespace util
{

template<bool B, typename T = void>
using enable_if = typename std::enable_if<B, T>::type;

template<typename T, typename = void>
struct base
{ using type = T; };

template<typename T>
struct base<T, enable_if<std::is_reference<T>::value>>
{
    using removed = typename std::remove_reference<T>::type;
    using type = typename base<removed>::type;
};

template<typename T>
struct base<T, enable_if<std::is_pointer<T>::value>>
{
    using removed = typename std::remove_pointer<T>::type;
    using type = typename base<removed>::type;
};

}
}
