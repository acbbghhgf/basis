#include "wtk_vecdb_cfg.h" 

int wtk_vecdb_cfg_init(wtk_vecdb_cfg_t *cfg)
{
    wtk_wrdvec_cfg_init(&(cfg->wrdvec));
    cfg->use_wrdvec = 0;
    cfg->db = NULL;
    cfg->vec_size = 50;
    cfg->get_like_thresh = 0.95;
    cfg->set_like_thresh = 0.99;
    cfg->use_share_wrdvec = 0;
    return 0;
}

int wtk_vecdb_cfg_clean(wtk_vecdb_cfg_t *cfg)
{
    wtk_wrdvec_cfg_clean(&(cfg->wrdvec));
    return 0;
}

int wtk_vecdb_cfg_update_local(wtk_vecdb_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_string_t *v;
    wtk_local_cfg_t *lc;
    int ret;

    lc = main;
    wtk_local_cfg_update_cfg_b(lc, cfg, use_wrdvec, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_share_wrdvec, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, vec_size, v);
    wtk_local_cfg_update_cfg_f(lc, cfg, get_like_thresh, v);
    wtk_local_cfg_update_cfg_f(lc, cfg, set_like_thresh, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, db, v);
    lc = wtk_local_cfg_find_lc_s(main, "wrdvec");
    if (lc) {
        ret = wtk_wrdvec_cfg_update_local(&(cfg->wrdvec), lc);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_vecdb_cfg_update(wtk_vecdb_cfg_t *cfg)
{
    int ret;

    if (cfg->use_wrdvec && !cfg->use_share_wrdvec) {
        ret = wtk_wrdvec_cfg_update(&(cfg->wrdvec));
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_vecdb_cfg_update2(wtk_vecdb_cfg_t *cfg, wtk_source_loader_t *sl)
{
    int ret;

    if (cfg->use_wrdvec && !cfg->use_share_wrdvec) {
        ret = wtk_wrdvec_cfg_update2(&(cfg->wrdvec), sl);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}
