#include "wtk_lexpool_cfg.h" 

int wtk_lexpool_cfg_init(wtk_lexpool_cfg_t *cfg)
{
    wtk_lexc_cfg_init(&(cfg->lexc));
    wtk_lexr_cfg_init(&(cfg->lexr));
    wtk_queue_init(&(cfg->item_q));
    cfg->cache_size = 10;
    return 0;
}

int wtk_lexpool_cfg_clean(wtk_lexpool_cfg_t *cfg)
{
    wtk_lexc_cfg_clean(&(cfg->lexc));
    wtk_lexr_cfg_clean(&(cfg->lexr));
    return 0;
}

int wtk_lexpool_cfg_update_var(wtk_lexpool_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_queue_node_t *qn;
    wtk_cfg_item_t *item;
    wtk_lexpool_item_cfg_t *li;
    wtk_string_t *v;

    for (qn = main->cfg->queue.pop; qn; qn = qn->next) {
        item = data_offset2(qn, wtk_cfg_item_t, n);
        if (item->type == WTK_CFG_LC) {
            li = (wtk_lexpool_item_cfg_t*) wtk_heap_malloc(main->heap,
                    sizeof(wtk_lexpool_item_cfg_t));
            wtk_local_cfg_update_cfg_str(item->value.cfg, li, fn, v);
            li->slot = wtk_local_cfg_find_string_s(item->value.cfg, "slot");
            li->name = wtk_local_cfg_find_array_s(item->value.cfg, "name");
            wtk_queue_push(&(cfg->item_q), &(li->q_n));
        }
    }
    return 0;
}

int wtk_lexpool_cfg_update_local(wtk_lexpool_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    wtk_string_t *v;
    int ret;

    lc = main;
    wtk_local_cfg_update_cfg_i(lc, cfg, cache_size, v);
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
    lc = wtk_local_cfg_find_lc_s(main, "var");
    if (lc) {
        ret = wtk_lexpool_cfg_update_var(cfg, lc);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lexpool_cfg_update(wtk_lexpool_cfg_t *cfg)
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
    cfg->lexr.use_share_lib = 1;
    ret = 0;
    end: return ret;
}
