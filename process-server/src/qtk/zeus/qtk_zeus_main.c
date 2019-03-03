#include "stdio.h"
#include "qtk_zeus_server.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "qtk_zeus_mq.h"
#include "qtk/zeus/qtk_zeus_cfg.h"
#include <assert.h>
#include "qtk/sframe/qtk_sframe.h"


static int conf_file_delete(qtk_zeus_cfg_t* c) {
    qtk_zeus_cfg_clean(c);
    wtk_free(c);
    return 0;
}


static qtk_zeus_cfg_t* conf_file_new(wtk_local_cfg_t *lc, qtk_log_t *log) {
    qtk_zeus_cfg_t *conf;
    int ret = -1;

    conf = (qtk_zeus_cfg_t*)wtk_calloc(1, sizeof(*conf));
    qtk_zeus_cfg_init(conf);
    ret = qtk_zeus_cfg_update_local(conf, lc);
    if (ret) {
        qtk_log_log0(log, "update zeus local configure failed.");
        goto end;
    }
    ret = qtk_zeus_cfg_update(conf);
    if (ret) {
        qtk_log_log0(log, "update zeus local dependents configure failed.");
        goto end;
    }
end:
    if (ret != 0 && conf) {
        conf_file_delete(conf);
        conf = NULL;
    }
    return conf;
}


int main() {
    struct lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_close(L);
    /* qtk_zeus_mq_t *zmq = qtk_zmq_new(); */
    /* qtk_zmesg_queue_t *q = qtk_zmq_newq(2); */
    /* q->in_mq = 1; */
    /* qtk_zmesg_t msg; */
    /* qtk_zmq_push(q, &msg, NULL); */
    /* qtk_zmq_pushq(zmq, q); */
    /* assert(qtk_zmq_pop(qtk_zmq_popq(zmq))); */
    /* qtk_zmq_delete(zmq); */
    /* zmq = NULL; */
    qtk_zeus_cfg_t *conf = conf_file_new(NULL, NULL);
    qtk_zserver_t *z = qtk_zserver_new(conf, NULL);
    qtk_zserver_start(z);
    qtk_zserver_join(z);
    qtk_zserver_delete(z);
    conf_file_delete(conf);
    return 0;
}


qtk_zserver_t* ahive_new(void *frame) {
    qtk_sframe_method_t *method = frame;
    wtk_local_cfg_t *lc = method->get_cfg(method);
    qtk_log_t *log = method->get_log(method);
    qtk_zeus_cfg_t *conf = conf_file_new(lc, log);
    return qtk_zserver_new(conf, log);
}


int ahive_start(qtk_zserver_t *z) {
    return qtk_zserver_start(z);
}


int ahive_stop(qtk_zserver_t *z) {
    return qtk_zserver_stop(z);
}


int ahive_delete(qtk_zserver_t *z) {
    return qtk_zserver_delete(z);
}
