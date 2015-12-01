#pragma once

struct lua_State;

namespace Shim
{

template<typename T, typename = void>
class trait {};

template<typename T, typename = void>
class type_code {};

}
