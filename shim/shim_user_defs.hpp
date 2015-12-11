#pragma once

#include "shim_util.hpp"
#include "shim_defs.hpp"

namespace Shim
{

template<typename T>
struct type_name_storage
{ static const char* value; };

template<typename T>
const char* type_name_storage<T>::value = nullptr;

namespace tags
{

struct user {};

struct user_ref : user {};
struct user_ptr : user {};

} // namespace tags

} // namespace Shim
