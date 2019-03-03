#define _XOPEN_SOURCE 500
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "wtk/core/wtk_strbuf.h"

static int lmalloc_trim(lua_State *L)
{
    lua_pushboolean(L, malloc_trim(0));
    return 1;
}

static int lget_time(lua_State *L)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    lua_pushinteger(L, tv.tv_sec*1000 + tv.tv_usec/1000);
    return 1;
}

static int lget_epoch_from_string(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);
    const char *fmt = luaL_checkstring(L, 2);
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    strptime(str, fmt, &tm);
    time_t t = mktime(&tm);
    lua_pushinteger(L, t);
    return 1;
}

static int lget_hostname(lua_State *L)
{
    char tmp[1024];
    if (gethostname(tmp, sizeof(tmp))) {
        return 0;
    }
    lua_pushstring(L, tmp);
    return 1;
}

static int lloadbin(lua_State *L)
{
    const char *bin_path = luaL_checkstring(L, 1);
    const char *fn = luaL_checkstring(L, 2);
    int env_idx = (!lua_isnone(L, 3) ? 3 : 0);
    char path[256];
    snprintf(path, sizeof(path), "%s&%s", bin_path, fn);
    int ret = luaL_loadfileq(L, path);
    if (ret != LUA_OK) {
        ret = luaL_loadfile(L, fn);
        if (ret != LUA_OK) {
            lua_pushnil(L);
            lua_insert(L, -2);
            return 2;
        }
    }
    if (env_idx != 0) {
        lua_pushvalue(L, env_idx);
        if (!lua_setupvalue(L, -2, 1))
            lua_pop(L, 1);
    }
    return 1;
}

LUAMOD_API int luaopen_sys(lua_State *L)
{
    luaL_Reg mlib[] = {
        {"mallocTrim", lmalloc_trim},
        {"getTime", lget_time},
        {"str2epoch", lget_epoch_from_string},
        {"hostname", lget_hostname},
        {"loadbin", lloadbin},
        {NULL, NULL}
    };
    luaL_newlib(L, mlib);
    return 1;
}
