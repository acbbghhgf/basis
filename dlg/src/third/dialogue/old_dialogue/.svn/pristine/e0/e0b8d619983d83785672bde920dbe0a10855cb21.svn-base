#include "wtk_lex_ner_cfg.h" 

int wtk_lex_ner_cfg_init(wtk_lex_ner_cfg_t *cfg)
{
    cfg->hash = NULL;
    cfg->n = 0;
    return 0;
}

int wtk_lex_ner_cfg_clean(wtk_lex_ner_cfg_t *cfg)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_lexr_ner_item_t *item;

    //wtk_debug("delete %p\n",cfg->hash);
    if (cfg->hash) {
        it = wtk_str_hash_iterator(cfg->hash);
        while (1) {
            node = wtk_str_hash_it_next(&(it));
            if (!node) {
                break;
            }
            item = (wtk_lexr_ner_item_t*) node->value;
            wtk_hmmne_delete(item->ne);
            if (item->fkv) {
                wtk_fkv_delete(item->fkv);
            }
        }
        wtk_str_hash_delete(cfg->hash);
    }
    return 0;
}

int wtk_lex_ner_cfg_add_ner(wtk_lex_ner_cfg_t *cfg, wtk_local_cfg_t *lc)
{
    wtk_lexr_ner_item_t *vi;
    wtk_string_t *v;

    vi = (wtk_lexr_ner_item_t*) wtk_heap_malloc(lc->heap,
            sizeof(wtk_lexr_ner_item_t));
    vi->name = NULL;
    vi->wrd_pen = 1.0;
    vi->prune_thresh = -21.0;
    vi->conf_thresh = vi->prune_thresh;
    vi->ne = NULL;
    vi->hash_hint = 3507;
    vi->fn = NULL;
    vi->map = NULL;
    vi->fkv = NULL;
    wtk_local_cfg_update_cfg_string(lc, vi, name, v);
    wtk_local_cfg_update_cfg_str(lc, vi, fn, v);
    wtk_local_cfg_update_cfg_str(lc, vi, map, v);
    wtk_local_cfg_update_cfg_f(lc, vi, wrd_pen, v);
    wtk_local_cfg_update_cfg_f(lc, vi, prune_thresh, v);
    wtk_local_cfg_update_cfg_f(lc, vi, conf_thresh, v);
    wtk_local_cfg_update_cfg_i(lc, vi, hash_hint, v);
    vi->ne = wtk_hmmne_new(vi->hash_hint);
//	ret=wtk_hmmne_load_file(vi->ne,vi->fn);
//	if(ret!=0){goto end;}
    wtk_str_hash_add(cfg->hash, vi->name->data, vi->name->len, vi);
//end:
    return 0;
}

int wtk_lex_ner_cfg_update_local(wtk_lex_ner_cfg_t *cfg, wtk_local_cfg_t *lc)
{
    wtk_queue_node_t *qn;
    wtk_cfg_item_t *item;
    int ret;

    //wtk_local_cfg_print(lc);
    cfg->hash = wtk_str_hash_new(lc->cfg->queue.length * 2 + 1);
    cfg->n = 0;
    for (qn = lc->cfg->queue.pop; qn; qn = qn->next) {
        item = data_offset2(qn, wtk_cfg_item_t, n);
        ret = wtk_lex_ner_cfg_add_ner(cfg, item->value.cfg);
        if (ret != 0) {
            goto end;
        }
        ++cfg->n;
    }
    ret = 0;
    end: return ret;
}

int wtk_lex_ner_cfg_update(wtk_lex_ner_cfg_t *cfg)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_lexr_ner_item_t *item;
    int ret;

    if (cfg->hash) {
        it = wtk_str_hash_iterator(cfg->hash);
        while (1) {
            node = wtk_str_hash_it_next(&(it));
            if (!node) {
                break;
            }
            item = (wtk_lexr_ner_item_t*) node->value;
            ret = wtk_hmmne_load_file(item->ne, item->fn);
            if (ret != 0) {
                wtk_debug("load file [%s] failed\n", item->fn);
                goto end;
            }
            if (item->map) {
                item->fkv = wtk_fkv_new3(item->map);
            }
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lex_ner_cfg_update2(wtk_lex_ner_cfg_t *cfg, wtk_source_loader_t *sl)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_lexr_ner_item_t *item;
    int ret;

    if (cfg->hash) {
        it = wtk_str_hash_iterator(cfg->hash);
        while (1) {
            node = wtk_str_hash_it_next(&(it));
            if (!node) {
                break;
            }
            item = (wtk_lexr_ner_item_t*) node->value;
            ret = wtk_source_loader_load(sl, item->ne,
                    (wtk_source_load_handler_t) wtk_hmmne_load, item->fn);
            if (ret != 0) {
                wtk_debug("load file [%s] failed\n", item->fn);
                goto end;
            }
        }
    }
    ret = 0;
    end:
    //wtk_debug("ret=%d\n",ret);
    return ret;
}
