#include "wtk_lmres_cfg.h"

int wtk_lmres_cfg_init(wtk_lmres_cfg_t *cfg)
{
    wtk_ngram_cfg_init(&(cfg->ngram));
    wtk_lexpool_cfg_init(&(cfg->lexpool));
    return 0;
}

int wtk_lmres_cfg_clean(wtk_lmres_cfg_t *cfg)
{
    wtk_lexpool_cfg_clean(&(cfg->lexpool));
    wtk_ngram_cfg_clean(&(cfg->ngram));
    return 0;
}

int wtk_lmres_cfg_update_local(wtk_lmres_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    int ret;

    lc = main;
    lc = wtk_local_cfg_find_lc_s(main, "ngram");
    if (lc) {
        ret = wtk_ngram_cfg_update_local(&(cfg->ngram), lc);
        if (ret != 0) {
            goto end;
        }
    }
    lc = wtk_local_cfg_find_lc_s(main, "lexpool");
    if (lc) {
        ret = wtk_lexpool_cfg_update_local(&(cfg->lexpool), lc);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lmres_cfg_update(wtk_lmres_cfg_t *cfg)
{
    int ret;

    ret = wtk_ngram_cfg_update(&(cfg->ngram));
    if (ret != 0) {
        goto end;
    }
    ret = wtk_lexpool_cfg_update(&(cfg->lexpool));
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    end: return ret;
}
