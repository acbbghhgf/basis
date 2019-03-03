#include "qtk_inst_cfg.h"
#include "qtk/os/qtk_base.h"
#include "qtk/os/qtk_alloc.h"

int qtk_inst_cfg_init(qtk_inst_cfg_t *cfg)
{
    cfg->cfn =  NULL;
    cfg->redisAddr = NULL;
    cfg->res = NULL;
    cfg->stateExpire = 0;
    cfg->tmp = 0;
    cfg->maxCnt = 0;
    return 0;
}

int qtk_inst_cfg_clean(qtk_inst_cfg_t *cfg)
{
    if (cfg->tmp && cfg->cfn) qtk_free(cfg->cfn);
    return 0;
}

int qtk_inst_cfg_update(qtk_inst_cfg_t *cfg)
{
    if (0 == wtk_string_cmp_s(cfg->res, "tmp")) cfg->tmp = 1;
    return 0;
}

int qtk_inst_cfg_update_local(qtk_inst_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_string_t *v;
    wtk_local_cfg_update_cfg_string(main, cfg, res, v);
    wtk_local_cfg_update_cfg_str(main, cfg, cfn, v);
    wtk_local_cfg_update_cfg_str(main, cfg, redisAddr, v);
    wtk_local_cfg_update_cfg_i(main, cfg, stateExpire, v);
    wtk_local_cfg_update_cfg_i(main, cfg, maxCnt, v);
    return 0;
}
