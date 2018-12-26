#include <assert.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include "wtk/os/wtk_socket.h"
#include "qtk/core/qtk_deque.h"
#include "qtk/zeus/qtk_zeus.h"
#include "qtk/zeus/qtk_zeus_server.h"
#include "qtk_serialize.h"
#include "qtk/zeus/qtk_zeus_socket.h"


static int lparsebuffer(lua_State *L) {
    /* wtk_stack_t *stk = lua_touserdata(L, 1); */
    return 0;
}


/*
  input: msg, sz
  output: type, id[, msg-content, content-length]
 */
static int lunpack(lua_State *L) {
    assert(2 == lua_gettop(L));
    if (lua_isnoneornil(L, 1)) {
        return 0;
    }
    char *msg = lua_touserdata(L, 1);
    int sz = lua_tointeger(L, 2);
    qtk_zsocket_msg_t smsg;
    int nret = 0;
    assert(sz >= sizeof(smsg));
    memcpy(&smsg, msg, sizeof(smsg));
    lua_pushinteger(L, smsg.type);
    lua_pushinteger(L, smsg.id);
    nret = 2;
    sz -= sizeof(smsg);
    if (sz) {
        msg += sizeof(smsg);
        lua_pushlightuserdata(L, msg);
        lua_pushinteger(L, sz);
        nret += 2;
    }
    return nret;
}



static int llisten(lua_State *L) {
    assert(3 == lua_gettop(L));
    size_t sz;
    const char *p = luaL_checklstring(L, 1, &sz);
    int port = lua_tointeger(L, 2);
    int backlog = lua_tointeger(L, 3);
    struct qtk_zcontext *ctx = lua_touserdata(L, lua_upvalueindex(1));
    size_t addr_sz;
    char addr_buf[64];
    addr_sz = sprintf(addr_buf, "%.*s:%d", (int)sz, p, port);
    int id = qtk_zsocket_listen(qtk_zcontext_handle(ctx),
                                addr_buf, addr_sz, backlog);
    lua_pushinteger(L, id);
    return 1;
}


static int lconnect(lua_State *L) {
    assert(2 == lua_gettop(L));
    size_t sz;
    const char *p = luaL_checklstring(L, 1, &sz);
    int port = lua_tointeger(L, 2);
    struct qtk_zcontext * ctx = lua_touserdata(L, lua_upvalueindex(1));
    size_t addr_sz;
    char addr_buf[64];
    addr_sz = sprintf(addr_buf, "%.*s:%d", (int)sz, p, port);
    int id = qtk_zsocket_open(qtk_zcontext_handle(ctx), addr_buf, addr_sz, ZSOCKET_MODE_RDWR);
    lua_pushinteger(L, id);
    return 1;
}


static int lopen(lua_State *L) {
    size_t sz;
    const char *p = luaL_checklstring(L, 1, &sz);
    struct qtk_zcontext * ctx = lua_touserdata(L, lua_upvalueindex(1));
    int mode = lua_tointeger(L, 2);
    int id = qtk_zsocket_open(qtk_zcontext_handle(ctx), p, sz, mode);
    lua_pushinteger(L, id);
    return 1;
}


static int lbind(lua_State *L) {
    int fd = luaL_checkinteger(L, 1);
    char tmp[64];
    int sz;
    sz = sprintf(tmp, ":%d", fd);
    struct qtk_zcontext * ctx = lua_touserdata(L, lua_upvalueindex(1));
    int mode = lua_tointeger(L, 2);
    int id = qtk_zsocket_open(qtk_zcontext_handle(ctx), tmp, sz, mode);
    lua_pushinteger(L, id);
    return 1;
}


static int lclose(lua_State *L) {
    struct qtk_zcontext * ctx = lua_touserdata(L, lua_upvalueindex(1));
    int id = luaL_checkinteger(L, 1);
    qtk_zsocket_close(qtk_zcontext_handle(ctx), id);
    return 0;
}


static int lsend(lua_State *L) {
    struct qtk_zcontext *ctx = lua_touserdata(L, lua_upvalueindex(1));
    int id = luaL_checkinteger(L, 1);
    int type = lua_type(L, 2);
    assert(type == LUA_TSTRING || type == LUA_TUSERDATA);
    if (LUA_TSTRING == type) {
        size_t sz = 0;
        const char *p = luaL_checklstring(L, 2, &sz);
        qtk_zsocket_send(qtk_zcontext_handle(ctx), id, p, sz);
    } else {
        qtk_deque_t *dq = lua_touserdata(L, 2);
        qtk_zsocket_send_buffer(qtk_zcontext_handle(ctx), id, dq);
    }
    return 0;
}


static int lkeepalive(lua_State *L) {
    assert(0);
    return 0;
}


typedef struct http_conn {
    uint32_t handle;
    int id;
} http_conn_t;

typedef struct http_pool {
} http_pool_t;


static int lhttpcon(lua_State *L) {
    struct qtk_zcontext *ctx = lua_touserdata(L, lua_upvalueindex(1));
    uint32_t handle = qtk_zcontext_handle(ctx);
    assert(2 == lua_gettop(L));
    size_t sz;
    const char *p = luaL_checklstring(L, 1, &sz);
    int port = lua_tointeger(L, 2);
    size_t addr_sz;
    char addr_buf[64];
    addr_sz = sprintf(addr_buf, "%.*s:%d", (int)sz, p, port);
    int id = qtk_zsocket_open(handle, addr_buf, addr_sz, ZSOCKET_MODE_RDWR);
    lua_pushinteger(L, id);
    http_conn_t *hc = lua_newuserdata(L, sizeof(*hc));
    hc->id = id;
    hc->handle = handle;
    luaL_getmetatable(L, "http_connection");
    lua_setmetatable(L, -2);
    return 2;
}


static int lhttpc_request(lua_State *L) {
    http_conn_t *hc = lua_touserdata(L, 1);
    size_t sz = 0;
    const char *p = luaL_checklstring(L, 2, &sz);
    qtk_zsocket_send(hc->handle, hc->id, p, sz);
    return 0;
}


static int lhttpc_gc(lua_State *L) {
    http_conn_t *hc = lua_touserdata(L, 1);
    qtk_zsocket_close(hc->handle, hc->id);
    return 0;
}


static int lhttpp_request(lua_State *L) {
    return 0;
}


static int lhttpp_gc(lua_State *L) {
    return 0;
}


static int lhttppool(lua_State *L) {
    assert(0);
    http_pool_t *hp = lua_newuserdata(L, sizeof(*hp));
    luaL_getmetatable(L, "http_pool");
    lua_setmetatable(L, -2);
    return 0;
}


LUAMOD_API int luaopen_zeus_socket_driver(lua_State *L) {
    luaL_checkversion(L);

    assert(0 == luaL_getmetatable(L, "http_connection"));
    luaL_Reg httpconn[] = {
        {"request", lhttpc_request},
        {"__gc", lhttpc_gc},
        {NULL, NULL},
    };
    luaL_newmetatable(L, "http_connection");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, httpconn, 0);

    assert(0 == luaL_getmetatable(L, "http_pool"));
    luaL_Reg httppool[] = {
        {"request", lhttpp_request},
        {"__gc", lhttpp_gc},
        {NULL, NULL},
    };
    luaL_newmetatable(L, "http_pool");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, httppool, 0);

    luaL_Reg l[] = {
        {"unpack", lunpack},
        {"parsebuffer", lparsebuffer},
        {NULL, NULL},
    };
    luaL_newlib(L, l);
    luaL_Reg l2[] = {
        {"connect", lconnect},
        {"open", lopen},
        {"bind", lbind},
        {"close", lclose},
        {"listen", llisten},
        /* {"shutdown", lshutdown}, */
        {"send", lsend},
        /* {"nodelay", lnodelay}, */
        /* {"reuseaddr", lreuseaddr}, */
        {"keepalive", lkeepalive},
        {"httpcon", lhttpcon},
        {"httppool", lhttppool},
        {NULL, NULL},
    };
    lua_getfield(L, LUA_REGISTRYINDEX, "qtk_zcontext");
    if (NULL == lua_touserdata(L, -1)) {
        return luaL_error(L, "Init qtk_zcontext first");
    }
    luaL_setfuncs(L, l2, 1);
    return 1;
}
