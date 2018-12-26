#ifndef _MOD_QTK_INST_CFG_H_
#define _MOD_QTK_INST_CFG_H_

#include "wtk/core/cfg/wtk_local_cfg.h"

typedef struct qtk_inst_cfg qtk_inst_cfg_t;

struct qtk_inst_cfg {
    wtk_string_t *res;
    char *cfn;
    const char *redisAddr;
    int stateExpire;
    int maxCnt;
    wtk_queue_node_t node;
    unsigned tmp : 1;
};

int qtk_inst_cfg_init(qtk_inst_cfg_t *cfg);
int qtk_inst_cfg_clean(qtk_inst_cfg_t *cfg);
int qtk_inst_cfg_update(qtk_inst_cfg_t *cfg);
int qtk_inst_cfg_update_local(qtk_inst_cfg_t *cfg, wtk_local_cfg_t *main);

#endif
