#include "wtk_lmrec_cfg.h"

int wtk_lmrec_cfg_init(wtk_lmrec_cfg_t *cfg)
{
    cfg->beam = -1;
    cfg->debug = 0;
    cfg->oov_pen = 0;
    cfg->lex_pen = 0;
    cfg->debug_best = 0;
    cfg->debug_all = 0;
    return 0;
}

int wtk_lmrec_cfg_clean(wtk_lmrec_cfg_t *cfg)
{
    return 0;
}

int wtk_lmrec_cfg_update_local(wtk_lmrec_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_string_t *v;
    wtk_local_cfg_t *lc;
    int ret;

    lc = main;
    wtk_local_cfg_update_cfg_b(lc, cfg, debug, v);
    wtk_local_cfg_update_cfg_f(lc, cfg, oov_pen, v);
    wtk_local_cfg_update_cfg_f(lc, cfg, beam, v);
    wtk_local_cfg_update_cfg_f(lc, cfg, lex_pen, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, debug_best, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, debug_all, v);
    ret = 0;
    return ret;
}

int wtk_lmrec_cfg_update(wtk_lmrec_cfg_t *cfg)
{
    return 0;
}
