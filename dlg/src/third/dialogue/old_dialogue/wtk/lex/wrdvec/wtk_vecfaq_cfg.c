#include "wtk_vecfaq_cfg.h" 

int wtk_vecfaq_cfg_init(wtk_vecfaq_cfg_t *cfg)
{
    wtk_wrdvec_cfg_init(&(cfg->wrdvec));
    cfg->dat = NULL;
    cfg->ndat = 0;
    cfg->use_share_wrdvec = 0;

    cfg->map = NULL;
    cfg->nmap = 0;
    return 0;
}

int wtk_vecfaq_cfg_clean(wtk_vecfaq_cfg_t *cfg)
{
    wtk_wrdvec_cfg_clean(&(cfg->wrdvec));
    return 0;
}

int wtk_vecfaq_cfg_update_local(wtk_vecfaq_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    wtk_string_t *v;
    int ret;

    lc = main;
    wtk_local_cfg_update_cfg_b(lc, cfg, use_share_wrdvec, v);
    lc = wtk_local_cfg_find_lc_s(main, "map");
    if (lc) {
        wtk_queue_node_t *qn;
        wtk_cfg_item_t *item;
        wtk_vecfaq_dat_cfg_t *dcfg;

        cfg->nmap = 0;
        cfg->map = (wtk_vecfaq_dat_cfg_t*) wtk_heap_malloc(main->heap,
                sizeof(wtk_vecfaq_dat_cfg_t) * lc->cfg->queue.length);
        for (qn = lc->cfg->queue.pop; qn; qn = qn->next) {
            item = data_offset2(qn, wtk_cfg_item_t, n);
            if (item->type == WTK_CFG_LC) {
                dcfg = cfg->map + cfg->nmap;
                ++cfg->nmap;
                dcfg->nm = item->key;
                wtk_local_cfg_update_cfg_str(item->value.cfg, dcfg, fn, v);
                wtk_local_cfg_update_cfg_f(item->value.cfg, dcfg, thresh, v);
                wtk_local_cfg_update_cfg_f(item->value.cfg, dcfg, best_thresh,
                        v);
            }
        }
    }
    lc = wtk_local_cfg_find_lc_s(main, "dat");
    if (lc) {
        wtk_queue_node_t *qn;
        wtk_cfg_item_t *item;
        wtk_vecfaq_dat_cfg_t *dcfg;

        cfg->ndat = 0;
        cfg->dat = (wtk_vecfaq_dat_cfg_t*) wtk_heap_malloc(main->heap,
                sizeof(wtk_vecfaq_dat_cfg_t) * lc->cfg->queue.length);
        for (qn = lc->cfg->queue.pop; qn; qn = qn->next) {
            item = data_offset2(qn, wtk_cfg_item_t, n);
            if (item->type == WTK_CFG_LC) {
                dcfg = cfg->dat + cfg->ndat;
                dcfg->nm = item->key;
                ++cfg->ndat;
                wtk_local_cfg_update_cfg_str(item->value.cfg, dcfg, fn, v);
                wtk_local_cfg_update_cfg_f(item->value.cfg, dcfg, thresh, v);
                wtk_local_cfg_update_cfg_f(item->value.cfg, dcfg, best_thresh,
                        v);
            }
        }
    } else {
        cfg->ndat = 1;
        cfg->dat = (wtk_vecfaq_dat_cfg_t*) wtk_heap_malloc(main->heap,
                sizeof(wtk_vecfaq_dat_cfg_t));
        lc = main;
        wtk_local_cfg_update_cfg_str(lc, cfg->dat, fn, v);
        wtk_local_cfg_update_cfg_f(lc, cfg->dat, thresh, v);
        wtk_local_cfg_update_cfg_f(lc, cfg->dat, best_thresh, v);
    }
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

int wtk_vecfaq_cfg_update(wtk_vecfaq_cfg_t *cfg)
{
    int ret;

    if (!cfg->use_share_wrdvec) {
        ret = wtk_wrdvec_cfg_update(&(cfg->wrdvec));
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_vecfaq_cfg_update2(wtk_vecfaq_cfg_t *cfg, wtk_source_loader_t *sl)
{
    int ret;

    if (!cfg->use_share_wrdvec) {
        ret = wtk_wrdvec_cfg_update2(&(cfg->wrdvec), sl);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}
