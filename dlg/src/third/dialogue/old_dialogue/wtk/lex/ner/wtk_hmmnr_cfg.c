#include "wtk_hmmnr_cfg.h" 

int wtk_hmmnr_cfg_init(wtk_hmmnr_cfg_t *cfg)
{
    cfg->ne = NULL;
    cfg->hash_hint = 3507;
    cfg->fn = NULL;
    cfg->map_fn = NULL;
    return 0;
}

int wtk_hmmnr_cfg_clean(wtk_hmmnr_cfg_t *cfg)
{
    if (cfg->ne) {
        wtk_hmmne_delete(cfg->ne);
    }
    return 0;
}

int wtk_hmmnr_cfg_update_local(wtk_hmmnr_cfg_t *cfg, wtk_local_cfg_t *lc)
{
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_str(lc, cfg, map_fn, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, fn, v);
    wtk_local_cfg_update_cfg_i(lc, cfg, hash_hint, v);
    return 0;
}

int wtk_hmmnr_cfg_update(wtk_hmmnr_cfg_t *cfg)
{
    int ret;

    if (cfg->fn) {
        cfg->ne = wtk_hmmne_new(cfg->hash_hint);
        ret = wtk_source_load_file(cfg->ne,
                (wtk_source_load_handler_t) wtk_hmmne_load, cfg->fn);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end:
    //wtk_debug("[%f/%f/%f/%f/%f]\n",cfg->unk_b,cfg->unk_be,cfg->unk_bm,cfg->unk_me,cfg->unk_mm);
    return ret;
}

