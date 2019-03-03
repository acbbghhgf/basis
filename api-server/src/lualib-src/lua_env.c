#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "main/qtk_env.h"

static int loptstring(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);
    const char *key = lua_tostring(L, 1);
    const char *opt = lua_tostring(L, 2);
    const char *ret = qtk_optstring(key, opt);
    if (ret) {
        lua_pushstring(L, ret);
        return 1;
    }
    return 0;
}

static int loptnumber(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TNUMBER);
    const char *key = lua_tostring(L, 1);
    lua_Number opt = lua_tonumber(L, 2);
    lua_Number ret = qtk_optnumber(key, opt);
    lua_pushnumber(L, ret);
    return 1;
}

static int loptboolean(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    const char *key = lua_tostring(L, 1);
    int opt = lua_toboolean(L, 2);
    int ret = qtk_optboolean(key, opt);
    lua_pushboolean(L, ret);
    return 1;
}

LUAMOD_API int luaopen_env(lua_State *L)
{
    luaL_Reg mlib[] = {
        {"optstring", loptstring},
        {"optnumber", loptnumber},
        {"optboolean", loptboolean},
        {NULL, NULL}
    };
    luaL_newlib(L, mlib);
    return 1;
}
