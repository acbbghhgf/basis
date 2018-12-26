#ifndef _MAIN_QTK_PROXY_WORKER_H
#define _MAIN_QTK_PROXY_WORKER_H

#include "third/lua5.3.4/src/lua.h"
#include "qtk_proxy_worker_cfg.h"
#include "os/qtk_thread.h"
#include "qtk_signal.h"

typedef struct qtk_proxy_worker qtk_proxy_worker_t;

struct qtk_proxy_worker {
    qtk_proxy_worker_cfg_t *cfg;
    qtk_thread_t routine;
    qtk_signal_t signal;
    lua_State *L;

    void *uplayer;
    unsigned run : 1;
};

qtk_proxy_worker_t *qtk_proxy_worker_new(void *uplayer, qtk_proxy_worker_cfg_t *cfg);
void qtk_proxy_worker_delete(qtk_proxy_worker_t *worker);
int qtk_proxy_worker_start(qtk_proxy_worker_t *worker);
int qtk_proxy_worker_stop(qtk_proxy_worker_t *worker);

#endif
