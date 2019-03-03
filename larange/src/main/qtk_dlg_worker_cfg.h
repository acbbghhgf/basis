#ifndef QTK_DLG_WORKER_CFG_H_
#define QTK_DLG_WORKER_CFG_H_

#include "qtk/event/qtk_event.h"
#include "mod/qtk_inst_cfg.h"

typedef struct qtk_dlg_worker_cfg qtk_dlg_worker_cfg_t;

struct qtk_dlg_worker_cfg {
    qtk_event_cfg_t ev_cfg;
    wtk_queue_t inst_cfgs;
    char *lua;
    wtk_string_t *lua_path;
    wtk_string_t *lua_cpath;
    qtk_inst_cfg_t *tmp;
    int max_raw_cnt;
    int max_pending;
    char *mod_path;
};

int qtk_dlg_worker_cfg_init(qtk_dlg_worker_cfg_t *cfg);
int qtk_dlg_worker_cfg_clean(qtk_dlg_worker_cfg_t *cfg);
int qtk_dlg_worker_cfg_update(qtk_dlg_worker_cfg_t *cfg);
int qtk_dlg_worker_cfg_update_local(qtk_dlg_worker_cfg_t *cfg, wtk_local_cfg_t *lc);
qtk_inst_cfg_t *qtk_dlg_worker_find_inst_cfg(qtk_dlg_worker_cfg_t *cfg,
                                             const char *res, size_t len);

#endif
