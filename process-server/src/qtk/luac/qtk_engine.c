#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "wtk/core/wtk_alloc.h"
#include "qtk/core/qtk_deque.h"

/*
  input: integer, string
  output: string
 */
static int lpackstring(lua_State *L) {
    size_t sz = 0;
    int len = 0;
    const char *p = NULL;
    int cmd = lua_tointeger(L, 1);
    p = luaL_checklstring(L, 2, &sz);
    len = sz + sizeof(cmd);
    sz = sizeof(len) + len;
    char *msg = wtk_malloc(sz);
    memcpy(msg, &len, sizeof(len));
    memcpy(msg+sizeof(len), &cmd, sizeof(cmd));
    memcpy(msg+sizeof(len)+sizeof(cmd), p, len-sizeof(cmd));
    lua_pushlstring(L, msg, sz);
    wtk_free(msg);
    return 1;
}


/*
  input: integer
  output: buf
 */
static int lpackcmd(lua_State *L) {
    int cmd = lua_tointeger(L, 1);
    int len = sizeof(int);
    size_t sz = sizeof(len) + len;
    char msg[sz];
    memcpy(msg, &len, sizeof(len));
    memcpy(msg+sizeof(len), &cmd, sizeof(cmd));
    lua_pushlstring(L, msg, sz);
    return 1;
}

/*
  input: integer, buf
  output: buf
 */
static int lpackbuffer(lua_State *L) {
    int cmd = lua_tointeger(L, 1);
    qtk_deque_t *dq = lua_touserdata(L, 2);
    qtk_deque_push_front(dq, (const char*)&cmd, sizeof(cmd));
    int sz = dq->len;
    qtk_deque_push_front(dq, (const char*)&sz, sizeof(sz));
    lua_pushvalue(L, -1);
    return 1;
}


/*
  input: inbuf, outbuf
  output: outbuf if raise pack, otherwise nil
 */
static int lunpack(lua_State *L) {
    qtk_deque_t *dq = lua_touserdata(L, 1);
    char *msg = lua_touserdata(L, 2);
    int sz = lua_tointeger(L, 3);
    qtk_deque_push(dq, msg, sz);
    return 0;
}


/*
  input: inbuf
  output: packbuf
 */
static int lparse(lua_State *L) {
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int len;
    if (dq->len <= 4) {
        return 0;
    }
    qtk_deque_pop(dq, (char*)&len, sizeof(len));
    if (dq->len < len) {
        qtk_deque_push_front(dq, (char*)&len, sizeof(len));
        return 0;
    } else {
        char buf[1024];
        int res_code;
        assert(len >= sizeof(res_code));
        qtk_deque_pop(dq, (char*)&res_code, sizeof(res_code));
        lua_pushinteger(L, res_code);
        qtk_deque_t *pack = lua_newuserdata(L, sizeof(pack));
        luaL_setmetatable(L, "buffer");
        qtk_deque_init(pack, 1024, 1024*1024, 1);
        while (len > 0) {
            int l = min(len, sizeof(buf));
            qtk_deque_pop(dq, buf, l);
            qtk_deque_push(pack, buf, l);
            len -= l;
        }
        return 2;
    }
}


/*
  input: inbuf
  output: string
 */
static int lparse2string(lua_State *L) {
    qtk_deque_t *dq = lua_touserdata(L, 1);
    int len;
    if (dq->len <= 4) {
        return 0;
    }
    qtk_deque_pop(dq, (char*)&len, sizeof(len));
    if (dq->len < len) {
        qtk_deque_push_front(dq, (char*)&len, sizeof(len));
        return 0;
    } else {
        int res_code;
        len -= sizeof(res_code);
        assert(len >= 0);
        qtk_deque_pop(dq, (char*)&res_code, sizeof(res_code));
        lua_pushinteger(L, res_code);
        if (len > 0) {
            char buf[len];
            qtk_deque_pop(dq, buf, len);
            lua_pushlstring(L, buf, len);
            return 2;
        } else {
            return 1;
        }
    }
}


LUAMOD_API int luaopen_zeus_engine(lua_State *L) {
    luaL_checkversion(L);

    luaL_Reg l[] = {
        {"packcmd", lpackcmd},
        {"packstring", lpackstring},
        {"packbuffer", lpackbuffer},
        {"unpack", lunpack},
        {"parse", lparse},
        {"parse2string", lparse2string},
        {NULL, NULL},
    };
    luaL_newlib(L, l);
    /* luaL_Reg l2[] = { */
    /*     {"connect", lconnect}, */
    /*     {"open", lopen}, */
    /*     {"bind", lbind}, */
    /*     {"close", lclose}, */
    /*     /\* {"shutdown", lshutdown}, *\/ */
    /*     {"send", lsend}, */
    /*     /\* {"nodelay", lnodelay}, *\/ */
    /*     /\* {"reuseaddr", lreuseaddr}, *\/ */
    /*     {"keepalive", lkeepalive}, */
    /*     {NULL, NULL}, */
    /* }; */
    /* lua_getfield(L, LUA_REGISTRYINDEX, "qtk_zcontext"); */
    /* if (NULL == lua_touserdata(L, -1)) { */
    /*     return luaL_error(L, "Init qtk_zcontext first"); */
    /* } */
    /* luaL_setfuncs(L, l2, 1); */
    return 1;
}
