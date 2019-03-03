#include "wtk_lex_cfg.h" 

int wtk_lex_cfg_init(wtk_lex_cfg_t *cfg)
{
    wtk_lexc_cfg_init(&(cfg->lexc));
    wtk_lexr_cfg_init(&(cfg->lexr));
    return 0;
}

int wtk_lex_cfg_clean(wtk_lex_cfg_t *cfg)
{
    wtk_lexc_cfg_clean(&(cfg->lexc));
    wtk_lexr_cfg_clean(&(cfg->lexr));
    return 0;
}

int wtk_lex_cfg_update_local(wtk_lex_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    int ret;

    lc = wtk_local_cfg_find_lc_s(main, "lexc");
    if (lc) {
        ret = wtk_lexc_cfg_update_local(&(cfg->lexc), lc);
        if (ret != 0) {
            goto end;
        }
    }
    lc = wtk_local_cfg_find_lc_s(main, "lexr");
    if (lc) {
        ret = wtk_lexr_cfg_update_local(&(cfg->lexr), lc);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lex_cfg_update(wtk_lex_cfg_t *cfg)
{
    int ret;

    ret = wtk_lexc_cfg_update(&(cfg->lexc));
    if (ret != 0) {
        goto end;
    }
    ret = wtk_lexr_cfg_update(&(cfg->lexr));
    if (ret != 0) {
        goto end;
    }
    end: return 0;
}

int wtk_lex_cfg_update2(wtk_lex_cfg_t *cfg, wtk_source_loader_t *sl)
{
    int ret;

    ret = wtk_lexc_cfg_update2(&(cfg->lexc), sl);
    if (ret != 0) {
        goto end;
    }
    ret = wtk_lexr_cfg_update2(&(cfg->lexr), sl);
    if (ret != 0) {
        goto end;
    }
    end: return 0;
}
