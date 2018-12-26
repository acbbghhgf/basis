#include "wtk_lexc_cfg.h" 

int wtk_lexc_cfg_init(wtk_lexc_cfg_t *cfg)
{
    wtk_lex_ner_cfg_init(&(cfg->ner));
    cfg->include_path = NULL;
    cfg->var_miss_pen = 0;
    cfg->use_eng_word = 0;
    cfg->use_act = 1;
    return 0;
}

int wtk_lexc_cfg_clean(wtk_lexc_cfg_t *cfg)
{
    wtk_lex_ner_cfg_clean(&(cfg->ner));
    return 0;
}

int wtk_lexc_cfg_update_local(wtk_lexc_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_string_t *v;
    wtk_local_cfg_t *lc;
    int ret;

    lc = main;
    wtk_local_cfg_update_cfg_f(lc, cfg, var_miss_pen, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_eng_word, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_act, v);
    cfg->include_path = wtk_local_cfg_find_array_s(lc, "include_path");
    lc = wtk_local_cfg_find_lc_s(main, "ner");
    if (lc) {
        ret = wtk_lex_ner_cfg_update_local(&(cfg->ner), lc);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lexc_cfg_update(wtk_lexc_cfg_t *cfg)
{
    int ret;

    ret = wtk_lex_ner_cfg_update(&(cfg->ner));
    if (ret != 0) {
        goto end;
    }
    end: return ret;
}

int wtk_lexc_cfg_update2(wtk_lexc_cfg_t *cfg, wtk_source_loader_t *sl)
{
    int ret;

    ret = wtk_lex_ner_cfg_update2(&(cfg->ner), sl);
    if (ret != 0) {
        goto end;
    }
    end: return ret;
}
