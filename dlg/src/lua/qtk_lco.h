#ifndef _LUA_QTK_LCO_H_
#define _LUA_QTK_LCO_H_

#include "third/lua5.3.4/src/lua.h"

int qtk_lreg_yield_co(lua_State *L, int timeout);
int qtk_lunreg_yield_co(lua_State *L, int ref);
lua_State *qtk_lget_yield_co(lua_State *L, int ref);

#endif
