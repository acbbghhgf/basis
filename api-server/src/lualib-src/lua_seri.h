#ifndef LUA_SERIALIZE_H_
#define LUA_SERIALIZE_H_

#include "third/lua5.3.4/src/lua.h"

int luaseri_pack(lua_State *L);
int luaseri_unpack(lua_State *L);

#endif
