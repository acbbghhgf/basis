#ifndef QTK_DLG_MAIN_CFG_H_
#define QTK_DLG_MAIN_CFG_H_

#include "qtk_dlg_worker.h"
#include "wtk/core/cfg/wtk_local_cfg.h"

typedef struct qtk_dlg_cfg qtk_dlg_cfg_t;

struct qtk_dlg_cfg {
    char *msg_server;
    unsigned use_msg : 1;
    qtk_dlg_worker_cfg_t dw_cfg;
};

int qtk_dlg_cfg_init(qtk_dlg_cfg_t *cfg);
int qtk_dlg_cfg_clean(qtk_dlg_cfg_t *cfg);
int qtk_dlg_cfg_update_local(qtk_dlg_cfg_t *cfg, wtk_local_cfg_t *lc);
int qtk_dlg_cfg_update(qtk_dlg_cfg_t *cfg);

#endif
