#include "wtk_lexpool.h" 

void wtk_lexpool_update_item(wtk_lexpool_t *p)
{
    wtk_queue_node_t *qn;
    wtk_lexpool_item_cfg_t *ci;
    wtk_lexpool_item_t *pi;
    wtk_heap_t *heap = p->net_map->heap;
    wtk_string_t **strs;
    int i;

    for (qn = p->cfg->item_q.pop; qn; qn = qn->next) {
        ci = data_offset2(qn, wtk_lexpool_item_cfg_t, q_n);
        pi = (wtk_lexpool_item_t*) wtk_heap_malloc(heap,
                sizeof(wtk_lexpool_item_t));
        pi->cfg = ci;
        pi->script = wtk_lexc_compile_file(p->lexc, ci->fn);
        wtk_lexc_reset(p->lexc);
        if (!pi->script) {
            wtk_debug("compile %s failed\n", ci->fn);
        } else {
            pi->net = wtk_lex_net_new(pi->script);
            strs = (wtk_string_t**) ci->name->slot;
            for (i = 0; i < ci->name->nslot; ++i) {
                wtk_str_hash_add(p->net_map, strs[i]->data, strs[i]->len, pi);
            }
            wtk_queue_push(&(p->item_q), &(pi->q_n));
        }
    }
}

wtk_lexpool_rec_t* wtk_lexpool_new_rec(wtk_lexpool_t *p)
{
    wtk_lexpool_rec_t *t;

    t = (wtk_lexpool_rec_t*) wtk_malloc(sizeof(wtk_lexpool_rec_t));
    t->rec = wtk_lexr_new(&(p->cfg->lexr), NULL);
    t->rec->lib = p->lib;
    return t;
}

void wtk_lexpool_rec_delete(wtk_lexpool_rec_t *t)
{
    wtk_lexr_delete(t->rec);
    wtk_free(t);
}

wtk_lexpool_t* wtk_lexpool_new(wtk_lexpool_cfg_t *cfg)
{
    wtk_lexpool_t *p;

    p = (wtk_lexpool_t*) wtk_malloc(sizeof(wtk_lexpool_t));
    p->cfg = cfg;
    p->lexc = wtk_lexc_new(&(cfg->lexc));
    p->net_map = wtk_str_hash_new(cfg->item_q.length * 2 + 1);
    p->lib = wtk_lexr_lib_new(&(cfg->lexr.lib), NULL);
    wtk_queue_init(&(p->item_q));
    wtk_hoard_init(&(p->rec_hoard), offsetof(wtk_lexpool_rec_t, q_n),
            cfg->cache_size, (wtk_new_handler_t) wtk_lexpool_new_rec,
            (wtk_delete_handler_t) wtk_lexpool_rec_delete, p);
    wtk_lexpool_update_item(p);
    return p;
}

void wtk_lexpool_delete(wtk_lexpool_t *p)
{
    wtk_lexpool_item_t *item;
    wtk_queue_node_t *n;

    while (1) {
        n = wtk_queue_pop(&(p->item_q));
        if (!n) {
            break;
        }
        item = data_offset2(n, wtk_lexpool_item_t, q_n);
        wtk_lex_net_delete(item->net);
        wtk_lex_script_delete(item->script);
    }
    wtk_hoard_clean(&(p->rec_hoard));
    wtk_lexr_lib_delete(p->lib);
    wtk_str_hash_delete(p->net_map);
    wtk_lexc_delete(p->lexc);
    wtk_free(p);
}

void wtk_lexpool_reset(wtk_lexpool_t *p)
{
    wtk_debug("reset ...\n");
}

wtk_lexpool_item_t* wtk_lexpool_get_item(wtk_lexpool_t *p, char *data,
        int bytes)
{
    return (wtk_lexpool_item_t*) wtk_str_hash_find(p->net_map, data, bytes);
}

wtk_lexpool_rec_t* wtk_lexpool_pop_rec(wtk_lexpool_t *p)
{
    return (wtk_lexpool_rec_t*) wtk_hoard_pop(&(p->rec_hoard));
}

void wtk_lexpool_push_rec(wtk_lexpool_t *p, wtk_lexpool_rec_t *r)
{
    wtk_lexr_reset(r->rec);
    wtk_hoard_push(&(p->rec_hoard), r);
}

wtk_string_t* wtk_lexpool_item_get_json_value(wtk_lexpool_item_t *pi,
        wtk_json_item_t *ji)
{
    wtk_json_item_t *vi;

    //wtk_json_item_print3(ji);
    vi = wtk_json_obj_get_s(ji, "attr");
    if (!vi) {
        wtk_debug("attr not found\n");
        goto end;
    }
    vi = wtk_json_obj_get(vi, pi->cfg->slot->data, pi->cfg->slot->len);
    if (!vi) {
        wtk_debug("[%.*s] not found\n", pi->cfg->slot->len, pi->cfg->slot->data);
        goto end;
    }
    vi = wtk_json_obj_get_s(vi, "_v");
    if (!vi) {
        goto end;
    }
    return vi->v.str;
    end: return NULL;
}
