#include "qtk_dlg.h"
#include "qtk_dlg_worker.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"
#include "qtk/core/cJSON.h"
#include "lua/qtk_lco.h"
#include "mod/qtk_fty.h"
#include "mod/qtk_incub.h"
#include "mod/qtk_inst_ctl.h"
#include "mod/qtk_inst.h"
#include "main/qtk_xfer.h"
#include "qtk/os/qtk_timer.h"

#include "third/lua5.3.4/src/lualib.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include <assert.h>

static qtk_dlg_worker_t *DW;

static void *qtk_dlg_worker_route(void *arg)
{
    qtk_dlg_worker_t *dw = (qtk_dlg_worker_t *)arg;
    int ret = luaL_loadfile(dw->L, dw->cfg->lua);
    if (ret != LUA_OK) {
        qtk_debug("Load lua %s failed: %s\n", dw->cfg->lua, lua_tostring(dw->L, -1));
        return NULL;
    }
    ret = lua_pcall(dw->L, 0, 0, 0);
    if (ret != LUA_OK) {
        qtk_debug("Run lua %s failed: %s\n", dw->cfg->lua, lua_tostring(dw->L, -1));
        return NULL;
    }
    for (;dw->dlg->run;) {
        qtk_event_module_process(dw->em, 10, 0);
        qtk_updatetime();
    }
    return NULL;
}

static int _pack_empty(void *ud, qtk_parser_t *ps)
{
    unuse(ud);
    ps->pack = qtk_pack_new();
    return 0;
}

static int _pack_ready(void *ud, qtk_parser_t *ps)
{
    qtk_dlg_worker_t *dw = ud;
    qtk_pack_t *p = ps->pack;
    wtk_string_t *v = qtk_pack_find_s(p, "dst");
    char tmp[v->len + 1];
    memcpy(tmp, v->data, v->len);
    tmp[v->len] = '\0';
    int ref = atoi(tmp);
    lua_State *co = qtk_lget_yield_co(dw->L, ref);
    if (co == NULL) {
        lua_pop(dw->L, 1);
        return 0;
    }
    v = qtk_pack_find_s(p, "res");
    lua_pushlstring(co, v->data, v->len);
    int ret = lua_resume(co, NULL, 1);
    if (ret > LUA_YIELD) {
        qtk_debug("%d\t%s\n", ret, lua_tostring(co, -1));
    }
    qtk_lunreg_yield_co(dw->L, ref);
    lua_pop(dw->L, 1);
    return 0;
}

static int _pack_unlink(void *ud, qtk_parser_t *ps)
{
    qtk_pack_delete(ps->pack);
    ps->pack = NULL;
    unuse(ud);
    return 0;
}

static void qtk_dlg_worker_init(qtk_dlg_worker_t *dw)
{
    qtk_timer_init();
    wtk_pipequeue_init(&dw->input_q);
    qtk_thread_init(&dw->route, qtk_dlg_worker_route, dw);
    qtk_parser_set_handler(dw->ps, _pack_empty, _pack_ready, _pack_unlink);
    dw->msg_sock_id = -1;

    luaL_openlibs(dw->L);
    lua_pushlightuserdata(dw->L, dw);
    lua_setfield(dw->L, LUA_REGISTRYINDEX, "worker");
    lua_pushlstring(dw->L, dw->cfg->lua_path->data, dw->cfg->lua_path->len);
    lua_setglobal(dw->L, "LUA_PATH");
    lua_pushlstring(dw->L, dw->cfg->lua_cpath->data, dw->cfg->lua_cpath->len);
    lua_setglobal(dw->L, "LUA_CPATH");
}

qtk_dlg_worker_t *qtk_dlg_worker_new(qtk_dlg_t *dlg, qtk_dlg_worker_cfg_t *cfg)
{
    assert(DW == NULL);
    qtk_dlg_worker_t *dw = qtk_calloc(1, sizeof(qtk_dlg_worker_t));
    dw->cfg = cfg;
    dw->dlg = dlg;
    dw->em = qtk_event_module_new(&cfg->ev_cfg);
    dw->L = luaL_newstate();
    dw->ps = qtk_parser_new(dw);
    qtk_dlg_worker_init(dw);
    DW = dw;
    return dw;
}

int qtk_dlg_worker_delete(qtk_dlg_worker_t *dw)
{
    qtk_fty_clean();
    qtk_incub_clean();
    qtk_ictl_clean();
    lua_close(dw->L);
    qtk_event_module_delete(dw->em);
    wtk_pipequeue_clean(&dw->input_q);
    qtk_parser_delete(dw->ps);
    qtk_free(dw);
    qtk_timer_clean();
    return 0;
}

static void _dlg_worker_watch_topic(qtk_dlg_worker_t *dw, const char *topic)
{
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "{\"%s\":\"\"}", topic);
    qtk_xfer_msg_t msg;
    msg.type = QTK_SFRAME_MSG_REQUEST;
    msg.param = HTTP_GET;
    wtk_string_t body = {tmp, strlen(tmp)};
    msg.body = &body;
    msg.content_type = "text/plain";
    msg.url = "/_watch";
    msg.id = dw->msg_sock_id;
    msg.gate = dw->dlg->gate;
    qtk_xfer_send(&msg);
    printf("Watch %s\n", tmp);
}

#define IS_JSON_STRING(item)           (item && item->type == cJSON_String && item->valuestring)
#define _dlg_worker_get_processer(dw)  lua_getglobal(dw->L, "processer")

static void _dlg_worker_process(qtk_dlg_worker_t *dw, cJSON *json)
{
    cJSON *item = cJSON_GetObjectItem(json, "script");
    if (!IS_JSON_STRING(item)) {
        qtk_debug("script not found\n");
        return;
    }
    const char *script = item->valuestring;

    item = cJSON_GetObjectItem(json, "id");
    if (!IS_JSON_STRING(item)) {
        qtk_debug("id not found\n");
        return;
    }
    const char *id = item->valuestring;

    _dlg_worker_get_processer(dw);
    lua_pushstring(dw->L, id);
    lua_pushstring(dw->L, script);
    if (lua_pcall(dw->L, 2, 0, 0))
        qtk_debug("run error: %s\n", lua_tostring(dw->L, -1));
}

static void _register(qtk_dlg_worker_t *dw)
{
    qtk_xfer_msg_t msg;
    msg.type = QTK_SFRAME_MSG_REQUEST;
    msg.param = HTTP_GET;
    wtk_string_t body = wtk_string("{\"tag\":\"dlg\",\"services\":[\"dlgv2.0\"]}");
    msg.body = &body;
    msg.url = "/register";
    msg.content_type = "text/plain";
    msg.id = dw->msg_sock_id;
    msg.gate = dw->dlg->gate;
    qtk_xfer_send(&msg);
}

static int _attach_msg(qtk_dlg_worker_t *dw)
{
    printf("attach %s\n", dw->dlg->cfg->msg_server);
    dw->msg_sock_id = qtk_xfer_estab(dw->dlg->gate, dw->dlg->cfg->msg_server, 0);
    return 0;
}

static int _dispatch_msg_sig(qtk_dlg_worker_t *dw, int sig)
{
    switch (sig) {
    case QTK_SFRAME_SIGCONNECT:
        _register(dw);
        _dlg_worker_get_processer(dw);
        lua_pushinteger(dw->L, dw->msg_sock_id);
        lua_pushliteral(dw->L, "CONNECT");
        if (lua_pcall(dw->L, 2, 0, 0))
            qtk_debug("run error: %s\n", lua_tostring(dw->L, -1));
        break;
    case QTK_SFRAME_SIGDISCONNECT:  //FALLTHROUGH
    case QTK_SFRAME_SIGECONNECT:
        qtk_timer_add(100, (timer_execute_func)_attach_msg, dw);
        _dlg_worker_get_processer(dw);
        lua_pushinteger(dw->L, dw->msg_sock_id);
        lua_pushliteral(dw->L, "DISCONNECT");
        if (lua_pcall(dw->L, 2, 0, 0))
            qtk_debug("run error: %s\n", lua_tostring(dw->L, -1));
        dw->msg_sock_id = -1;
        break;
    default:
        ;
    }
    return 0;
}

static int _dispatch_proxy_request(qtk_dlg_worker_t *dw, qtk_dlg_event_t *ev)
{
    char tmp[ev->d.data.len + 1];
    memset(tmp, 0, ev->d.data.len + 1);
    memcpy(tmp, ev->d.data.data, ev->d.data.len);
    cJSON *json = cJSON_Parse(tmp);
    if (NULL == json)
        return -1;

    cJSON *item = cJSON_GetObjectItem(json, "tag");
    if (IS_JSON_STRING(item)) {
        item = cJSON_GetObjectItem(json, "id");
        if (IS_JSON_STRING(item))
            _dlg_worker_watch_topic(dw, item->valuestring);
    } else {
        _dlg_worker_process(dw, json);
    }
    cJSON_Delete(json);
    return 0;
}

#define _dlg_worker_get_clientHandler(dw)  lua_getglobal(dw->L, "clientHandler")

static int _dispatch_client_request(qtk_dlg_worker_t *dw, qtk_dlg_event_t *ev)
{
    _dlg_worker_get_clientHandler(dw);
    lua_pushinteger(dw->L, ev->id);
    lua_pushlstring(dw->L, ev->d.data.data, ev->d.data.len);
    if (lua_pcall(dw->L, 2, 0, 0))
        qtk_debug("run error: %s\n", lua_tostring(dw->L, -1));
    return 0;
}

static int qtk_dlg_worker_dispatch(qtk_dlg_worker_t *dw, qtk_dlg_event_t *ev)
{
    if (ev->is_sig)         // for now, dlg worker only care signal form msg server
        return _dispatch_msg_sig(dw, ev->d.sig);

    if (ev->id == dw->msg_sock_id)
        _dispatch_proxy_request(dw, ev);
    else {
        _dispatch_client_request(dw, ev);
    }
    return 0;
}

static int qtk_dlg_worker_read(void *data, qtk_event_t *ev)
{
    unuse(ev);
    qtk_dlg_worker_t *dw = (qtk_dlg_worker_t *)data;
    wtk_queue_node_t *node = NULL;
    qtk_dlg_event_t *dlg_evt = NULL;

    while ((node = wtk_pipequeue_pop(&dw->input_q)) != NULL) {
        dlg_evt = data_offset(node, qtk_dlg_event_t, node);
        qtk_dlg_worker_dispatch(dw, dlg_evt);
        qtk_dlg_event_delete(dlg_evt);
    }
    return 0;
}

static void qtk_dlg_worker_prepare(qtk_dlg_worker_t *dw)
{
    const char *path = "/tmp/dlgd";
    if (0 == access(path, F_OK)) {
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
        int ret = system(cmd);
        unuse(ret);
    }
    mkdir(path, S_IRWXU);
    qtk_event_init(&dw->event, QTK_EVENT_READ, qtk_dlg_worker_read, NULL, dw);
    qtk_event_add_event(dw->em, dw->input_q.pipe_fd[0], &dw->event);
    qtk_fty_init();
    qtk_incub_init();
    qtk_ictl_init(dw);
    if (qtk_inst_mod_init(dw->cfg->mod_path))
        qtk_debug("inst mod init failed\n");
}

int qtk_dlg_worker_start(qtk_dlg_worker_t *dw)
{
    qtk_dlg_worker_prepare(dw);
    qtk_thread_start(&dw->route);
    if (dw->dlg->cfg->use_msg)
        _attach_msg(dw);
    return 0;
}

int qtk_dlg_worker_stop(qtk_dlg_worker_t *dw)
{
    wtk_pipequeue_touch_write(&dw->input_q);
    qtk_thread_join(&dw->route);
    return 0;
}

int qtk_dlg_worker_raise_err(qtk_dlg_worker_t *dw, const char *err, const char *et, const char *id)
{
    qtk_xfer_msg_t msg;
    msg.type = QTK_SFRAME_MSG_REQUEST;
    msg.param = HTTP_GET;
    msg.url = (char *)et;
    msg.id = dw->msg_sock_id;
    wtk_string_t body = {(char *)err, strlen(err)};
    msg.body = &body;
    msg.content_type = "text/plain";
    msg.gate = dw->dlg->gate;
    qtk_xfer_send(&msg);
    msg.url = "/stop";
    body.data = (char *)id;
    body.len = strlen(id);
    qtk_xfer_send(&msg);
    return 0;
}

qtk_dlg_worker_t *qtk_dlg_worker_self()
{
    return DW;
}
