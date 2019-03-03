#include <assert.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include "qtk/core/qtk_deque.h"


typedef int (*deque_strfunc)(qtk_deque_t *dq, char *buf, int sz);


static int buffer_pushstring(lua_State *L, deque_strfunc func, qtk_deque_t *dq, int sz) {
    if (sz > 4096) {
        char *tmp = wtk_malloc(sz);
        assert(sz == func(dq, tmp, sz));
        lua_pushlstring(L, tmp, sz);
        wtk_free(tmp);
    } else {
        char tmp[sz];
        assert(sz == func(dq, tmp, sz));
        lua_pushlstring(L, tmp, sz);
    }
    return 1;
}


static int lpushrawdata(lua_State *L) {
    assert(lua_gettop(L) == 3);
    qtk_deque_t *dq = lua_touserdata(L, 1);
    char *p = lua_touserdata(L, 2);
    size_t sz = lua_tointeger(L, 3);
    assert(dq && p);
    qtk_deque_push(dq, p, sz);
    return 0;
}


static int lpush(lua_State *L) {
    qtk_deque_t *dq = lua_touserdata(L, 1);
    size_t sz;
    const char *p = luaL_checklstring(L, 2, &sz);
    qtk_deque_push(dq, (char*)p, sz);
    return 0;
}


static int lpop(lua_State *L) {
    assert(2 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int sz = lua_tointeger(L, 2);
    if (sz > dq->len) {
        sz = dq->len;
    }
    return buffer_pushstring(L, qtk_deque_pop2, dq, sz);
}


static int lpopall(lua_State *L) {
    assert(1 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int sz = dq->len;
    return buffer_pushstring(L, qtk_deque_pop2, dq, sz);
}


static int lfront(lua_State *L) {
    assert(2 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int sz = lua_tointeger(L, 2);
    if (sz > dq->len) {
        sz = dq->len;
    }
    return buffer_pushstring(L, qtk_deque_front, dq, sz);
}


static int ltail(lua_State *L) {
    assert(2 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int sz = lua_tointeger(L, 2);
    if (sz > dq->len) {
        sz = dq->len;
    }
    return buffer_pushstring(L, qtk_deque_tail, dq, sz);
}


static int ltailfromidx(lua_State *L) {
    assert(2 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int idx = lua_tointeger(L, 2);
    if (idx >= dq->len) {
        return 0;
    } else {
        int sz = dq->len - idx;
        return buffer_pushstring(L, qtk_deque_tail, dq, sz);
    }
}


static int ltostring(lua_State *L) {
    assert(1 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int sz = dq->len;
    return buffer_pushstring(L, qtk_deque_front, dq, sz);
}


static int lclear(lua_State *L) {
    assert(1 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    qtk_deque_reset(dq);
    return 0;
}


static int llen(lua_State *L) {
    assert(1 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    lua_pushinteger(L, dq->len);
    return 1;
}


static int lgc(lua_State *L) {
    assert(1 == lua_gettop(L));
    qtk_deque_t *dq = lua_touserdata(L, 1);
    qtk_deque_clean(dq);
    return 0;
}


static int lnew(lua_State *L) {
    qtk_deque_t *dq = lua_newuserdata(L, sizeof(*dq));
    qtk_deque_init(dq, 1024, 1024*1024, 1);
    luaL_getmetatable(L, "buffer");
    lua_setmetatable(L, -2);
    return 1;
}


static int lclone(lua_State *L) {
    qtk_deque_t *cpy = lua_touserdata(L, 1);
    assert(cpy);
    qtk_deque_t *dq = lua_newuserdata(L, sizeof(*dq));
    qtk_deque_clone(dq, cpy);
    luaL_getmetatable(L, "buffer");
    lua_setmetatable(L, -2);
    return 1;
}


LUAMOD_API int luaopen_zeus_buffer(lua_State *L) {
    luaL_checkversion(L);
    assert(0 == luaL_getmetatable(L, "buffer"));
    luaL_Reg buffer_meta[] = {
        {"push_rawdata", lpushrawdata},
        {"push", lpush},
        {"pop", lpop},
        {"front", lfront},
        {"tail", ltail},
        {"tailfromidx", ltailfromidx},
        {"popall", lpopall},
        {"tostring", ltostring},
        {"clear", lclear},
        {"clone", lclone},
        {"len", llen},
        {"__gc", lgc},
        {"__len", llen},
        {NULL, NULL},
    };
    luaL_Reg buffer[] = {
        {"new", lnew},
        {NULL, NULL},
    };
    luaL_newmetatable(L, "buffer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, buffer_meta, 0);
    luaL_newlib(L, buffer);
    return 1;
}
