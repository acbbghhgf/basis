#include "main/qtk_bifrost_main.h"
#include "main/qtk_bifrost_core.h"
#include "main/qtk_context.h"
#include "main/qtk_env.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"

#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include "zzip/zzip.h"
#include "zzip/plugin.h"

#include <assert.h>

struct gate {
    qtk_thread_t route;
    qtk_bifrost_t *b;
    qtk_context_t *ctx;
    lua_State *L;

    unsigned run : 1;
};

static void _gate_clean(struct gate *g)
{
    if (g) {
        if (g->L) { lua_close(g->L); }
        qtk_free(g);
    }
    qtk_debug("cleannig\n");
}

static int _gate_route_prepare(struct gate *g, const char *param)
{
    int ret = -1;
    lua_State *L = g->L = luaL_newstate();

    luaL_openlibs(L);
    lua_pushlightuserdata(L, g->ctx);
    lua_setfield(L, LUA_REGISTRYINDEX, "qtk_context");
    const char *path = qtk_optstring("serviceBinPath", NULL);

    lua_pushstring(L, path);
    lua_setglobal(L, "LUA_PPATH");

    char buf[1024];
    snprintf(buf, sizeof(buf), "%s&service/gate.lua", path);

    int r = luaL_loadfileq(L, buf);
    if (r != LUA_OK) {
        qtk_debug("gate.lua load error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        r = luaL_loadfile(L, "service/gate.lua");
        if (r != LUA_OK) {
            qtk_debug("gate.lua load retry error: %s\n", lua_tostring(L, -1));
            goto end;
        }
    }
    strncpy(buf, param, sizeof(buf));
    char *tmp = buf;
    int nargs = 0;
    while (1) {
        char *param = strsep(&tmp, " ");
        if (NULL == param) break;
        lua_pushstring(L, param);
        nargs ++;
    }
    r = lua_pcall(L, 3, 0, 0);
    if (r != LUA_OK) {
        qtk_debug("gate.lua run error: %s\n", lua_tostring(L, -1));
        goto end;
    }
    ret = 0;
end:
    return ret;
}

#define MEMEQ(s1, s2) (0 == memcmp(s1, s2, sizeof(s2)-1))
#define CASE_BODY                                  \
do {                                               \
    if (MEMEQ(key, "body")) {                      \
        wtk_strbuf_t *res = method->get_body(msg); \
        if (NULL == res) return 0;                 \
        lua_pushlstring(l, res->data, res->pos);   \
        return 1;                                  \
    }                                              \
} while(0)

#define CASE_TYPE                           \
do {                                        \
    if (MEMEQ(key, "type")) {               \
        int type = method->get_type(msg);   \
        if (type == QTK_SFRAME_MSG_REQUEST) \
            lua_pushliteral(l, "REQUEST");  \
        else {                              \
            lua_pushliteral(l, "RESPONSE"); \
        }                                   \
        return 1;                           \
    }                                       \
} while(0)

#define CASE_URL                                  \
do {                                              \
    if (MEMEQ(key, "url")) {                      \
        wtk_string_t *res = method->get_url(msg); \
        if (NULL == res) return 0;                \
        lua_pushlstring(l, res->data, res->len);  \
        return 1;                                 \
    }                                             \
} while(0)

#define CASE_REQ                                                    \
do {                                                                \
    if (MEMEQ(key, "req")) {                                        \
        lua_newtable(l);                                            \
        wtk_string_t *url = method->req_url(msg);                   \
        if (url != NULL) {                                          \
            lua_pushliteral(l, "url");                              \
            lua_pushlstring(l, url->data, url->len);                \
            lua_rawset(l, -3);                                      \
        }                                                           \
        wtk_strbuf_t *body = method->req_body(msg);                 \
        if (body != NULL) {                                         \
            lua_pushliteral(l, "body");                             \
            lua_pushlstring(l, body->data, body->pos);              \
            lua_rawset(l, -3);                                      \
        }                                                           \
        lua_pushliteral(l, "field");                                \
        lua_newtable(l);                                            \
        wtk_string_t *host = method->req_header(msg, "host");       \
        wtk_string_t *ct = method->req_header(msg, "content-type"); \
        wtk_string_t *sn = method->req_header(msg, "seq-no");       \
        if (host != NULL) {                                         \
            lua_pushliteral(l, "host");                             \
            lua_pushlstring(l, host->data, host->len);              \
            lua_rawset(l, -3);                                      \
        }                                                           \
        if (ct != NULL) {                                           \
            lua_pushliteral(l, "contentType");                      \
            lua_pushlstring(l, ct->data, ct->len);                  \
            lua_rawset(l, -3);                                      \
        }                                                           \
        if (sn != NULL) {                                           \
            lua_pushliteral(l, "sn");                               \
            lua_pushlstring(l, sn->data, sn->len);                  \
            lua_rawset(l, -3);                                      \
        }                                                           \
        lua_rawset(l, -3);                                          \
        return 1;                                                   \
    }                                                               \
} while(0)

#define CASE_ID                       \
do {                                  \
    if (MEMEQ(key, "id")) {           \
        int id = method->get_id(msg); \
        lua_pushinteger(l, id);       \
        return 1;                     \
    }                                 \
} while(0)

#define CASE_STATUS                           \
do {                                          \
    if (MEMEQ(key, "status")) {               \
        int status = method->get_status(msg); \
        lua_pushinteger(l, status);           \
        return 1;                             \
    }                                         \
} while(0)

#define CASE_ADDR                                               \
do {                                                            \
    if (MEMEQ(key, "addr")) {                                   \
        wtk_string_t *r = method->get_remote_addr(method, msg); \
        if (NULL == r) return 0;                                \
        lua_pushlstring(l, r->data, r->len);                    \
        return 1;                                               \
    }                                                           \
} while(0)


static int _gatemsg_index(lua_State *l)
{
    qtk_sframe_method_t *method = qtk_bifrost_self()->gate;
    qtk_sframe_msg_t *msg = lua_touserdata(l, 1);
    size_t len;
    const char *key = lua_tolstring(l, 2, &len);
    switch(len) {
    case 2:
        CASE_ID;
        break;
    case 3:
        CASE_URL;
        CASE_REQ;
        break;
    case 4:
        CASE_BODY;
        CASE_TYPE;
        CASE_ADDR;
        break;
    case 6:
        CASE_STATUS;
        break;
    default:
        ;
    }
    wtk_string_t *res = method->get_header(msg, key);
    if (NULL == res) return 0;
    lua_pushlstring(l, res->data, res->len);
    return 1;
}

#define CASE_ISPROTOBUF                 \
do {                                    \
    if (MEMEQ(key, "isprotobuf")) {     \
        lua_pushboolean(l, 1);          \
        return 1;                       \
    }                                   \
} while(0)

#define CASE_VARIANT                                        \
do {                                                        \
    if (MEMEQ(key, "variant")) {                            \
        lua_pushlightuserdata(l, method->get_pb_var(msg));  \
        return 1;                                           \
    }                                                       \
} while(0)

#define CASE_WSI                                            \
do {                                                        \
    if (MEMEQ(key, "wsi")) {                                \
        struct lws *wsi = method->get_ws(msg);              \
        if (wsi) {                                          \
            lua_pushlightuserdata(l, wsi);                  \
            return 1;                                       \
        }                                                   \
    }                                                       \
} while(0)

static int _gatemsg_protobuf_index(lua_State *l)
{
    qtk_sframe_method_t *method = qtk_bifrost_self()->gate;
    qtk_sframe_msg_t *msg = lua_touserdata(l, 1);
    size_t len;
    const char *key = lua_tolstring(l, 2, &len);
    switch(len) {
    case 2:
        CASE_ID;
        break;
    case 3:
        CASE_WSI;
        break;
    case 4:
        CASE_ADDR;
        break;
    case 7:
        CASE_VARIANT;
        break;
    case 10:
        CASE_ISPROTOBUF;
        break;
    default:
        ;
    }
    return 0;
}

static int _gatemsg_jwtpayload_index(lua_State *l)
{
    qtk_sframe_method_t *method = qtk_bifrost_self()->gate;
    qtk_sframe_msg_t *msg = lua_touserdata(l, 1);
    size_t len;
    const char *key = lua_tolstring(l, 2, &len);
    switch (len) {
    case 2:
        CASE_ID;
        break;
    case 3:
        CASE_WSI;
        if (MEMEQ(key, "jwt")) {
            lua_pushlstring(l, msg->info.jwt_payload->data, msg->info.jwt_payload->len);
            return 1;
        }
        break;
    case 5:
        if (MEMEQ(key, "isjwt")) {
            lua_pushboolean(l, 1);
            return 1;
        }
        break;
    default:
        ;
    }
    return 0;
}

#define _gate_get_processer(L)      lua_getglobal(L, "processer")

#define UD_PROTO            1
#define UD_PROTOBUF         2
#define UD_JWT_PAYLOAD      3

static void _gate_forward_ud(struct gate *g, qtk_sframe_msg_t *msg, int type)
{
    _gate_get_processer(g->L);
    lua_pushlightuserdata(g->L, msg);
    switch(type) {
    case UD_PROTO:
        if (luaL_newmetatable(g->L, "msg_metatable")) {
            lua_pushcfunction(g->L, _gatemsg_index);
            lua_setfield(g->L, -2, "__index");
        }
        break;
    case UD_PROTOBUF:
        if (luaL_newmetatable(g->L, "msg_protobuf_metatable")) {
            lua_pushcfunction(g->L, _gatemsg_protobuf_index);
            lua_setfield(g->L, -2, "__index");
        }
        break;
    case UD_JWT_PAYLOAD:
        if (luaL_newmetatable(g->L, "msg_jwtpayload_metatable")) {
            lua_pushcfunction(g->L, _gatemsg_jwtpayload_index);
            lua_setfield(g->L, -2, "__index");
        }
        break;
    }
    lua_setmetatable(g->L, -2);
    int ret = lua_pcall(g->L, 1, 0, 0);
    if (ret != LUA_OK)
        qtk_debug("error run: %s\n", lua_tostring(g->L, -1));
}

static void _gate_forward_proto(struct gate *g, qtk_sframe_msg_t *msg)
{
    _gate_get_processer(g->L);
    lua_pushlightuserdata(g->L, msg);
    if (luaL_newmetatable(g->L, "msg_metatable")) {
        lua_pushcfunction(g->L, _gatemsg_index);
        lua_setfield(g->L, -2, "__index");
    }
    lua_setmetatable(g->L, -2);
    int ret = lua_pcall(g->L, 1, 0, 0);
    if (ret != LUA_OK)
        qtk_debug("error run: %s\n", lua_tostring(g->L, -1));
}

static void _gate_forward_protobuf(struct gate *g, qtk_sframe_msg_t *msg)
{
    _gate_get_processer(g->L);
    lua_pushlightuserdata(g->L, msg);
    if (luaL_newmetatable(g->L, "msg_protobuf_metatable")) {
        lua_pushcfunction(g->L, _gatemsg_protobuf_index);
        lua_setfield(g->L, -2, "__index");
    }
    lua_setmetatable(g->L, -2);
    int ret = lua_pcall(g->L, 1, 0, 0);
    if (ret != LUA_OK)
        qtk_debug("error run: %s\n", lua_tostring(g->L, -1));
}

static void _gate_forward_jwt_payload(struct gate *g, qtk_sframe_msg_t *msg)
{
    _gate_forward_ud(g, msg, UD_JWT_PAYLOAD);
}

static void _gate_forward_notice(struct gate *g, qtk_sframe_msg_t *msg)
{
    qtk_sframe_method_t *method = g->b->gate;
    int signal = method->get_signal(msg);
    int id = method->get_id(msg);
    _gate_get_processer(g->L);
    lua_pushinteger(g->L, id);
    switch(signal) {
    case QTK_SFRAME_SIGCONNECT:
        lua_pushliteral(g->L, "CONNECT");
        break;
    case QTK_SFRAME_SIGDISCONNECT:
        lua_pushliteral(g->L, "DISCONNECT");
        break;
    case QTK_SFRAME_SIGECONNECT:
        lua_pushliteral(g->L, "ECONNECT");
        break;
    default:
        lua_pushliteral(g->L, "UNKNOWN");
    }
    int ret = lua_pcall(g->L, 2, 0, 0);
    if (ret != LUA_OK)
        qtk_debug("error run: %s\n", lua_tostring(g->L, -1));
}

static void *_gate_route(void *arg)
{
    struct gate *g = arg;
    qtk_bifrost_t *b = g->b;
    qtk_sframe_method_t *method = b->gate;
    qtk_sframe_msg_t *msg = NULL;
    for ( ; g->run; ) {
        msg = method->pop(method, 100);
        if (NULL == msg) { continue; }
        switch(method->get_type(msg)) {
        case QTK_SFRAME_MSG_NOTICE:
            _gate_forward_notice(g, msg);
            break;
        case QTK_SFRAME_MSG_REQUEST:
        case QTK_SFRAME_MSG_RESPONSE:
            if (QTK_SFRAME_OK == method->get_signal(msg)) {
                _gate_forward_proto(g, msg);
            } else {
                qtk_bifrost_log(g->ctx, "error", "framework forward non-ok proto");
            }
            break;
        case QTK_SFRAME_MSG_PROTOBUF:
            _gate_forward_protobuf(g, msg);
            break;
        case QTK_SFRAME_MSG_JWT_PAYLOAD:
            _gate_forward_jwt_payload(g, msg);
            break;
        case QTK_SFRAME_MSG_CMD:
        case QTK_SFRAME_MSG_UNKNOWN:
        default:
            ;
        }
        method->delete(method, msg);
    }
    return NULL;
}

struct gate *gate_create(void)
{
    struct gate *g = qtk_calloc(1, sizeof(struct gate));

    return g;
}

int gate_init(struct gate *g, qtk_context_t *ctx, const char *param)
{
    g->b = qtk_bifrost_self();
    g->ctx = ctx;
    qtk_thread_init(&g->route, _gate_route, g);
    g->run = 1;
    if (_gate_route_prepare(g, param)) { return -1; }
    qtk_thread_start(&g->route);

    return 0;
}


void gate_release(struct gate *g)
{
    g->run = 0;
    qtk_thread_join(&g->route);
    _gate_clean(g);
}

void gate_signal(struct gate *g, int signal)
{
    qtk_debug("recv a signal: %d\n", signal);
}
