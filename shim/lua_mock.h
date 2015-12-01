#pragma once

#include <cstddef>

struct lua_State;
using lua_Integer = unsigned long;
using lua_Number = double;

void lua_pushinteger(lua_State*, lua_Integer);
void lua_pushnumber(lua_State*, lua_Number);
void lua_pushboolean(lua_State*, bool);
void lua_pushstring(lua_State*, const char*);
void lua_pushlstring(lua_State*, const char*, size_t);

lua_Integer lua_tointeger(lua_State*, int);
lua_Number lua_tonumber(lua_State*, int);
bool lua_toboolean(lua_State*, int);
const char* lua_tostring(lua_State*, int);
const char* lua_tolstring(lua_State*, int, size_t*);

int lua_type(lua_State*, int);
const char* lua_typename(lua_State*, int);

bool lua_isnoneornil(lua_State* L, int);
void lua_pop(lua_State* L, int);
void lua_getmetatable(lua_State* L, int);
bool lua_rawequal(lua_State* L, int, int);

void* lua_newuserdata(lua_State* L, size_t);
void* lua_touserdata(lua_State* L, int);
void lua_setmetatable(lua_State* L, int);
void luaL_getmetatable(lua_State* L, const char*);





enum
{
    LUA_TNUMBER,
    LUA_TBOOLEAN,
    LUA_TSTRING,
    LUA_TUSERDATA
};
