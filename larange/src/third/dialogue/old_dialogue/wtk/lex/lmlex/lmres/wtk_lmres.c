#include "wtk_lmres.h"

wtk_lmres_t* wtk_lmres_new(wtk_lmres_cfg_t *cfg)
{
    wtk_lmres_t* res;

    res = (wtk_lmres_t*) wtk_malloc(sizeof(wtk_lmres_t));
    res->cfg = cfg;
    res->ngram = wtk_ngram_new(&(cfg->ngram));
    res->lexpool = wtk_lexpool_new(&(cfg->lexpool));
    return res;
}

void wtk_lmres_delete(wtk_lmres_t *r)
{
    if (r->lexpool) {
        wtk_lexpool_delete(r->lexpool);
    }
    if (r->ngram) {
        wtk_ngram_delete(r->ngram);
    }
    wtk_free(r);
}

void wtk_lmres_reset(wtk_lmres_t *res)
{
    //wtk_ngram_reset(res->ngram);
    wtk_lexpool_reset(res->lexpool);
}
