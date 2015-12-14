#pragma once

namespace Lua
{

namespace util
{

inline int abs_index(int top, int index)
{ return ( index < 0 ) ? top + index + 1 : index; }

} // namespace util

}
