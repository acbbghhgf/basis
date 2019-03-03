#include "wtk_lmlex.h"

wtk_lmlex_t* wtk_lmlex_new(wtk_lmlex_cfg_t *cfg)
{
    wtk_lmlex_t *l;

    l = (wtk_lmlex_t*) wtk_malloc(sizeof(wtk_lmlex_t));
    l->cfg = cfg;
    l->lmrec = wtk_lmrec_new(&(cfg->lmrec));
    l->lmres = wtk_lmres_new(&(cfg->lmres));
    l->heap = wtk_heap_new(4096);
    wtk_lmlex_reset(l);
    return l;
}

void wtk_lmlex_delete(wtk_lmlex_t *l)
{
    wtk_lmres_delete(l->lmres);
    wtk_heap_delete(l->heap);
    wtk_lmrec_delete(l->lmrec);
    wtk_free(l);
}

void wtk_lmlex_reset(wtk_lmlex_t *l)
{
    wtk_lmrec_reset(l->lmrec);
    wtk_heap_reset(l->heap);
}

wtk_string_t wtk_lmlex_process(wtk_lmlex_t *l, char *data, int bytes)
{
    return wtk_lmrec_process2(l->lmrec, l->lmres, l->heap, data, bytes);
}

void wtk_lmlex_print(wtk_lmlex_t *l)
{
    //wtk_debug("nbest=%d\n",l->lmrec->tok_q.length);
    //wtk_lmrec_print(l->lmrec);
}
