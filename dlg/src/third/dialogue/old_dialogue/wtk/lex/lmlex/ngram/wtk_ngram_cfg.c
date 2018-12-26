#include "wtk_ngram_cfg.h"

int wtk_ngram_cfg_init(wtk_ngram_cfg_t *cfg)
{
    wtk_string_set_s(&(cfg->unk), "<unk>");
    wtk_string_set_s(&(cfg->snts), "<s>");
    wtk_string_set_s(&(cfg->snte), "</s>");
    wtk_string_set_s(&(cfg->pau), "-pau-");
    cfg->fn = NULL;
    return 0;
}

int wtk_ngram_cfg_clean(wtk_ngram_cfg_t *cfg)
{
    return 0;
}

int wtk_ngram_cfg_update_local(wtk_ngram_cfg_t *cfg, wtk_local_cfg_t *lc)
{
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_string_v(lc, cfg, unk, v);
    wtk_local_cfg_update_cfg_string_v(lc, cfg, snts, v);
    wtk_local_cfg_update_cfg_string_v(lc, cfg, snte, v);
    wtk_local_cfg_update_cfg_string_v(lc, cfg, pau, v);
    wtk_local_cfg_update_cfg_str(lc, cfg, fn, v);
    return 0;
}

int wtk_ngram_cfg_update(wtk_ngram_cfg_t *cfg)
{
    return 0;
}
