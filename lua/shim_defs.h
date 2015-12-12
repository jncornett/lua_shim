#pragma once

namespace Lua
{

namespace traits
{

template<typename T, typename = void>
struct trait {};

template<typename T, typename = void>
struct lua_type_code {};

} // namespace traits

namespace tags
{

struct builtin {};

struct numeric : builtin {};
struct integral : numeric {};
struct unsigned_integral : integral {};
struct floating_point : numeric {};
struct string : builtin {};
struct std_string : string {};
struct c_string : string {};
struct boolean : builtin {};

struct user {};
struct user_ref : user {};
struct user_ptr : user {};

} // namespace tags

}
