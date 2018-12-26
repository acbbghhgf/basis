#include "wtk_lexr_lib_cfg.h" 

int wtk_lexr_lib_cfg_init(wtk_lexr_lib_cfg_t *cfg)
{
    cfg->tree_fn = NULL;
    cfg->map_hash = NULL;
    return 0;
}

int wtk_lexr_lib_cfg_clean(wtk_lexr_lib_cfg_t *cfg)
{
    if (cfg->map_hash) {
        wtk_str_hash_delete(cfg->map_hash);
    }
    return 0;
}

int wtk_lexr_lib_cfg_update_local(wtk_lexr_lib_cfg_t *cfg,
        wtk_local_cfg_t *main)
{
    wtk_local_cfg_t *lc;
    wtk_queue_node_t *qn;
    wtk_string_t *v;
    wtk_cfg_item_t *item;
    wtk_lexr_lib_item_t *li;
    int ret = -1;

    lc = main;
    wtk_local_cfg_update_cfg_str(lc, cfg, tree_fn, v);
    lc = wtk_local_cfg_find_lc_s(main, "table");
    if (lc) {
        cfg->map_hash = wtk_str_hash_new(lc->cfg->queue.length * 2 + 1);
        for (qn = lc->cfg->queue.pop; qn; qn = qn->next) {
            item = data_offset2(qn, wtk_cfg_item_t, n);
            li = (wtk_lexr_lib_item_t*) wtk_heap_malloc(cfg->map_hash->heap,
                    sizeof(wtk_lexr_lib_item_t));
            li->name = item->key;
            li->value = NULL;
            switch (item->type) {
                case WTK_CFG_STRING:
                    li->value = item->value.str;
                    break;
                case WTK_CFG_LC:
                    li->value = wtk_local_cfg_find_string_s(item->value.cfg,
                            "name");
                    break;
                default:
                    li->value = NULL;
                    break;
            }
            if (li->value) {
                wtk_str_hash_add(cfg->map_hash, li->name->data, li->name->len,
                        li);
            }
        }
    }
    ret = 0;
    return ret;
}

int wtk_lexr_lib_cfg_update(wtk_lexr_lib_cfg_t *cfg)
{
    return 0;
}
