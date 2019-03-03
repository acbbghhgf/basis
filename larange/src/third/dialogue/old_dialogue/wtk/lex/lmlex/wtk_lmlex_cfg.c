#include "wtk_lmlex_cfg.h"

int wtk_lmlex_cfg_init(wtk_lmlex_cfg_t *cfg)
{
    wtk_lmres_cfg_init(&(cfg->lmres));
    wtk_lmrec_cfg_init(&(cfg->lmrec));
    return 0;
}

int wtk_lmlex_cfg_clean(wtk_lmlex_cfg_t *cfg)
{
    wtk_lmres_cfg_clean(&(cfg->lmres));
    wtk_lmrec_cfg_clean(&(cfg->lmrec));
    return 0;
}

int wtk_lmlex_cfg_update_local(wtk_lmlex_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    int ret;

    lc = wtk_local_cfg_find_lc_s(main, "lmrec");
    if (lc) {
        ret = wtk_lmrec_cfg_update_local(&(cfg->lmrec), lc);
        if (ret != 0) {
            goto end;
        }
    }
    lc = wtk_local_cfg_find_lc_s(main, "lmres");
    if (lc) {
        ret = wtk_lmres_cfg_update_local(&(cfg->lmres), lc);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lmlex_cfg_update(wtk_lmlex_cfg_t *cfg)
{
    int ret;

    ret = wtk_lmrec_cfg_update(&(cfg->lmrec));
    if (ret != 0) {
        goto end;
    }
    ret = wtk_lmres_cfg_update(&(cfg->lmres));
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    end: return ret;
}
