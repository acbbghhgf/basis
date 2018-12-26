#include "third/lua5.3.4/src/lauxlib.h"
#include "third/lua5.3.4/src/lualib.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk_proxy_worker.h"
#include "main/qtk_xfer.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"
#include "qtk_timer.h"
#include "qtk_proxy.h"

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
    lua_getfield(l, LUA_REGISTRYINDEX, "worker");
    qtk_proxy_worker_t *worker = lua_touserdata(l, -1);
    qtk_sframe_method_t *method = ((qtk_proxy_t *)worker->uplayer)->gate;
    qtk_sframe_msg_t *msg = lua_touserdata(l, 1);
    size_t len;
    const char *key = lua_tolstring(l, 2, &len);
    switch(len) {
    case 2:
        CASE_ID;
        break;
    case 3:
        CASE_URL;
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

static int _worker_attach_msg(qtk_proxy_worker_t *worker)
{
    qtk_proxy_t *p = (qtk_proxy_t *)worker->uplayer;
    return qtk_xfer_estab(p->gate, p->cfg->msg_addr, 0);
}

static void _worker_dispatch_signal(qtk_proxy_worker_t *worker)
{
    int signal;
    for (;;) {
        signal = qtk_signal_dispatch(&worker->signal);
        if (0 == signal) { break; }
        qtk_debug("recv signal: %d\n", signal);
    }
}

#define _worker_get_processer(L)    lua_getglobal(L, "processer")

static void _worker_process_sig_connect(qtk_proxy_worker_t *worker, qtk_sframe_msg_t *msg)
{
    lua_State *l = worker->L;
    qtk_proxy_t *p = (qtk_proxy_t *)worker->uplayer;
    qtk_sframe_method_t *method = p->gate;
    wtk_string_t *addr = method->get_remote_addr(method, msg);
    if (0 == strcmp(p->cfg->msg_addr, addr->data)) {
        _worker_get_processer(l);
        lua_pushstring(l, "CONNECT");
        int id = method->get_id(msg);
        lua_pushinteger(l, id);
        int ret = lua_pcall(l, 2, 0, 0);
        if (ret != LUA_OK)
            qtk_debug("run error: %s\t%d\n", lua_tostring(l, -1), ret);
    }
}

static void _worker_process_sig_disconnect(qtk_proxy_worker_t *worker, qtk_sframe_msg_t *msg)
{
    qtk_proxy_t *p = (qtk_proxy_t *)worker->uplayer;
    qtk_sframe_method_t *method = p->gate;
    lua_State *l = worker->L;
    wtk_string_t *addr = method->get_remote_addr(method, msg);
    if (0 == strcmp(p->cfg->msg_addr, addr->data)) {
        qtk_timer_add(100, (timer_execute_func)_worker_attach_msg, worker);
        _worker_get_processer(l);
        lua_pushstring(l, "DISCONNECT");
        int id = method->get_id(msg);
        lua_pushinteger(l, id);
        int ret = lua_pcall(l, 2, 0, 0);
        if (ret != LUA_OK)
            qtk_debug("run error: %s\n", lua_tostring(l, -1));
    }
    qtk_xfer_destroy(method, msg->id);
}

static void _worker_process_notice(qtk_proxy_worker_t *worker, qtk_sframe_msg_t *msg)
{
    qtk_proxy_t *p = (qtk_proxy_t *)worker->uplayer;
    qtk_sframe_method_t *method = p->gate;
    int signal = method->get_signal(msg);
    switch (signal) {
    case QTK_SFRAME_SIGCONNECT:
        _worker_process_sig_connect(worker, msg);
        break;
    case QTK_SFRAME_SIGDISCONNECT: case QTK_SFRAME_SIGECONNECT:
        _worker_process_sig_disconnect(worker, msg);
        break;
    default:
        qtk_debug("unknow signal: %d\n", signal);
    }
}

static void _worker_process_proto(qtk_proxy_worker_t *worker, qtk_sframe_msg_t *msg)
{
    lua_State *l = worker->L;
    _worker_get_processer(l);
    lua_pushlightuserdata(l, msg);
    if (luaL_newmetatable(l, "msg_metatable")) {
        lua_pushcfunction(l, _gatemsg_index);
        lua_setfield(l, -2, "__index");
    }
    lua_setmetatable(l, -2);
    int ret = lua_pcall(l, 1, 0, 0);
    if (ret != LUA_OK)
        qtk_debug("run error: %s\n", lua_tostring(l, -1));
}

static void _worker_process(qtk_proxy_worker_t *worker)
{
    qtk_sframe_msg_t *msg = NULL;
    qtk_proxy_t *p = (qtk_proxy_t *)worker->uplayer;
    qtk_sframe_method_t *method = p->gate;
    msg = method->pop(method, 10);
    if (NULL == msg) { return; }
    int type = method->get_type(msg);
    switch (type) {
        case QTK_SFRAME_MSG_NOTICE:
            _worker_process_notice(worker, msg);
            break;
        case QTK_SFRAME_MSG_REQUEST:    // FALLTHROUGH
        case QTK_SFRAME_MSG_RESPONSE:
            _worker_process_proto(worker, msg);
            break;
        case QTK_SFRAME_MSG_CMD:
        case QTK_SFRAME_MSG_UNKNOWN:
        default:
            ;
    }
    method->delete(method, msg);
    _worker_dispatch_signal(worker);
}

static void *_routine(void *arg)    //arg = d->work
{
    qtk_proxy_worker_t *worker = (qtk_proxy_worker_t *)arg;
    lua_State *L = worker->L;

    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%.*s&%s",
            worker->cfg->lua_ppath->len, worker->cfg->lua_ppath->data, worker->cfg->lua);
    int ret = luaL_loadfileq(L, tmp);
    if (ret != LUA_OK) {        //add load main.lua when can not service.bin .
        ret = luaL_loadfile(L, worker->cfg->lua);
        if (ret != LUA_OK) {   
        qtk_debug("Can't load %s: %s\n", worker->cfg->lua, lua_tostring(L, -1));
        return NULL;    
        }        
    }
    ret = lua_pcall(L, 0, 0, 0);
    if (ret != LUA_OK) {
        qtk_debug("Error run %s: %s\n", worker->cfg->lua, lua_tostring(L, -1));
        return NULL;
    }

    for ( ; worker->run; ) {
        _worker_process(worker);
        qtk_updatetime();
    }

    return NULL;
}

static int _worker_listen(qtk_proxy_worker_t *worker)
{
    qtk_proxy_t *p = (qtk_proxy_t *)worker->uplayer;
    return qtk_xfer_listen(p->gate, p->cfg->listen_addr, p->cfg->listen_backlog);
}

static int _worker_prepare(qtk_proxy_worker_t *worker)
{
    _worker_attach_msg(worker);
    _worker_listen(worker);
    return 0;
}

static int _worker_init(qtk_proxy_worker_t *worker)
{
    qtk_timer_init();
    qtk_thread_init(&worker->routine, _routine, worker);
    qtk_signal_init(&worker->signal);

    lua_State *L = worker->L;
    luaL_openlibs(L);
    lua_pushlightuserdata(L, worker);
    lua_setfield(L, LUA_REGISTRYINDEX, "worker");

    lua_pushlstring(L, worker->cfg->lua_path->data, worker->cfg->lua_path->len);
    lua_setglobal(L, "LUA_PATH");
    lua_pushlstring(L, worker->cfg->lua_cpath->data, worker->cfg->lua_cpath->len);
    lua_setglobal(L, "LUA_CPATH");
    lua_pushlstring(L, worker->cfg->lua_ppath->data, worker->cfg->lua_ppath->len);
    lua_setglobal(L, "LUA_PPATH");
    return 0;
}

qtk_proxy_worker_t *qtk_proxy_worker_new(void *uplayer, qtk_proxy_worker_cfg_t *cfg)
{
    qtk_proxy_worker_t *worker = qtk_calloc(1, sizeof(qtk_proxy_worker_t));
    worker->cfg = cfg;
    worker->uplayer = uplayer;
    worker->L = luaL_newstate();
    _worker_init(worker);
    return worker;
}

void qtk_proxy_worker_delete(qtk_proxy_worker_t *worker)
{
    qtk_signal_clean(&worker->signal);
    lua_close(worker->L);
    qtk_free(worker);
}

int qtk_proxy_worker_start(qtk_proxy_worker_t *worker)
{
    if (_worker_prepare(worker)) { return -1; }
    worker->run = 1;
    qtk_thread_start(&worker->routine);
    return 0;
}

int qtk_proxy_worker_stop(qtk_proxy_worker_t *worker)
{
    worker->run = 0;
    qtk_thread_join(&worker->routine);
    return 0;
}
