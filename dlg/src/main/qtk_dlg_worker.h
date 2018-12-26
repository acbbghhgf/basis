#ifndef QTK_DLG_WORKER_H_
#define QTK_DLG_WORKER_H_

#include "qtk_dlg_worker_cfg.h"
#include "wtk/os/wtk_pipequeue.h"
#include "qtk/os/qtk_thread.h"
#include "qtk/event/qtk_event_timer.h"
#include "pack/qtk_parser.h"

#include "third/lua5.3.4/src/lua.h"

#define qtk_dlg_worker_get_time(dw)       (dw)->em->timer->time.wtk_cached_time

struct qtk_dlg;
typedef struct qtk_dlg_worker qtk_dlg_worker_t;

struct qtk_dlg_worker {
    qtk_event_t event;
    qtk_dlg_worker_cfg_t *cfg;
    wtk_pipequeue_t input_q;
    qtk_event_module_t *em;
    qtk_parser_t *ps;
    qtk_thread_t route;
    lua_State *L;
    int msg_sock_id;
    struct qtk_dlg *dlg;
};

qtk_dlg_worker_t *qtk_dlg_worker_new(struct qtk_dlg *dlg, qtk_dlg_worker_cfg_t *cfg);
int qtk_dlg_worker_delete(qtk_dlg_worker_t *dw);
int qtk_dlg_worker_start(qtk_dlg_worker_t *dw);
int qtk_dlg_worker_stop(qtk_dlg_worker_t *dw);
int qtk_dlg_worker_raise_err(qtk_dlg_worker_t *dw, const char *err, const char *et, const char *id);
qtk_dlg_worker_t *qtk_dlg_worker_self();

#endif
