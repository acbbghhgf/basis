#include "wtk_lexr_cfg.h" 

int wtk_lexr_cfg_init(wtk_lexr_cfg_t *cfg)
{
    wtk_wrdvec_cfg_init(&(cfg->wrdvec));
    wtk_poseg_cfg_init(&(cfg->poseg));
    cfg->filter = NULL;
    wtk_lexr_lib_cfg_init(&(cfg->lib));
    cfg->use_share_lib = 0;
    cfg->use_wrdvec = 0;
    cfg->use_poseg = 0;
    cfg->use_share_wrdvec = 0;
    cfg->pron_fn = NULL;
    cfg->debug = 0;
    return 0;
}

int wtk_lexr_cfg_clean(wtk_lexr_cfg_t *cfg)
{
    wtk_poseg_cfg_clean(&(cfg->poseg));
    wtk_wrdvec_cfg_clean(&(cfg->wrdvec));
    wtk_lexr_lib_cfg_clean(&(cfg->lib));
    return 0;
}

int wtk_lexr_cfg_update_local(wtk_lexr_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    wtk_string_t *v;
    int ret;

    lc = main;
    wtk_local_cfg_update_cfg_str(lc, cfg, pron_fn, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, debug, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_share_lib, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_wrdvec, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_poseg, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_share_wrdvec, v);
    cfg->filter = wtk_local_cfg_find_array_s(lc, "filter");
    lc = wtk_local_cfg_find_lc_s(main, "lib");
    if (lc) {
        ret = wtk_lexr_lib_cfg_update_local(&(cfg->lib), lc);
        if (ret != 0) {
            goto end;
        }
    }
    if (cfg->use_wrdvec) {
        lc = wtk_local_cfg_find_lc_s(main, "wrdvec");
        if (lc) {
            ret = wtk_wrdvec_cfg_update_local(&(cfg->wrdvec), lc);
            if (ret != 0) {
                goto end;
            }
        }
    }
    if (cfg->use_poseg) {
        lc = wtk_local_cfg_find_lc_s(main, "poseg");
        if (lc) {
            ret = wtk_poseg_cfg_update_local(&(cfg->poseg), lc);
            if (ret != 0) {
                goto end;
            }
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lexr_cfg_update(wtk_lexr_cfg_t *cfg)
{
    int ret;

    if (!cfg->use_share_wrdvec && cfg->use_wrdvec) {
        ret = wtk_wrdvec_cfg_update(&(cfg->wrdvec));
        if (ret != 0) {
            goto end;
        }
    }
    if (cfg->use_poseg) {
        ret = wtk_poseg_cfg_update(&(cfg->poseg));
        if (ret != 0) {
            goto end;
        }
    }
    ret = wtk_lexr_lib_cfg_update(&(cfg->lib));
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    end: return ret;
}

int wtk_lexr_cfg_update2(wtk_lexr_cfg_t *cfg, wtk_source_loader_t *sl)
{
    int ret;

    if (!cfg->use_share_wrdvec && cfg->use_wrdvec) {
        ret = wtk_wrdvec_cfg_update2(&(cfg->wrdvec), sl);
        if (ret != 0) {
            goto end;
        }
    }
    if (cfg->use_poseg) {
        ret = wtk_poseg_cfg_update2(&(cfg->poseg), sl);
        if (ret != 0) {
            goto end;
        }
    }
    ret = wtk_lexr_lib_cfg_update(&(cfg->lib));
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    end: return ret;
}
