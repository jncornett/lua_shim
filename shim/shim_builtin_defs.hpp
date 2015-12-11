#pragma once

#include <string>

#include "shim_util.hpp"
#include "shim_defs.hpp"

namespace Shim
{

namespace tags
{

struct builtin {};

struct integral : builtin {};
struct floating_point : builtin {};
struct boolean : builtin {};
struct string : builtin {};
struct cstring : builtin {};

} // namespace tags;

namespace detail
{

template<typename T>
inline constexpr bool is_boolean()
{ return std::is_same<T, bool>::value; }

template<typename T>
inline constexpr bool is_string()
{
    return std::is_same<T, std::string>::value ||
        std::is_same<T, char const*>::value; 
}

} // namespace detail

}
