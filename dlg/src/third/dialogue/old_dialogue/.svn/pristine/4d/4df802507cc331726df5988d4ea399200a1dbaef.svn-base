#include "wtk_faq2bin_cfg.h" 

int wtk_faq2bin_cfg_init(wtk_faq2bin_cfg_t *cfg)
{
    wtk_wrdvec_cfg_init(&(cfg->wrdvec));
    cfg->ncls = 100;
    cfg->use_cls = 0;
    return 0;
}

int wtk_faq2bin_cfg_clean(wtk_faq2bin_cfg_t *cfg)
{
    wtk_wrdvec_cfg_clean(&(cfg->wrdvec));
    return 0;
}

int wtk_faq2bin_cfg_update_local(wtk_faq2bin_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    wtk_string_t *v;
    int ret;

    lc = main;
    wtk_local_cfg_update_cfg_i(lc, cfg, ncls, v);
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

int wtk_faq2bin_cfg_update(wtk_faq2bin_cfg_t *cfg)
{
    int ret;

    ret = wtk_wrdvec_cfg_update(&(cfg->wrdvec));
    if (ret != 0) {
        goto end;
    }
    end: return ret;
}
