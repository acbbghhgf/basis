#include "wtk_nlg2.h" 

wtk_nlg2_t* wtk_nlg2_new(int n)
{
    wtk_nlg2_t *nlg;

    nlg = (wtk_nlg2_t*) wtk_malloc(sizeof(wtk_nlg2_t));
    nlg->action_hash = wtk_str_hash_new(n);
    nlg->buf = wtk_strbuf_new(256, 1);
    nlg->rec_heap = wtk_heap_new(4096);
    nlg->global = NULL;
    return nlg;
}

void wtk_nlg2_delete(wtk_nlg2_t *nlg)
{
    wtk_heap_delete(nlg->rec_heap);
    wtk_str_hash_delete(nlg->action_hash);
    wtk_strbuf_delete(nlg->buf);
    wtk_free(nlg);
}

wtk_nlg2_slot_t* wtk_nlg2_find_slot(wtk_nlg2_t *nlg, char *k, int len,
        int insert)
{
    wtk_nlg2_slot_t *slot;
    wtk_heap_t *heap;

    slot = (wtk_nlg2_slot_t*) wtk_str_hash_find(nlg->action_hash, k, len);
    if (slot || !insert) {
        goto end;
    }
    heap = nlg->action_hash->heap;
    slot = (wtk_nlg2_slot_t*) wtk_heap_malloc(heap, sizeof(wtk_nlg2_slot_t));
    wtk_queue3_init(&(slot->item_q));
    wtk_str_hash_add2(nlg->action_hash, k, len, slot);
    end: return slot;
}

void wtk_nlg2_slot_print(wtk_nlg2_slot_t *slot)
{
    wtk_queue_node_t *qn;
    wtk_nlg2_item_t *item;

    for (qn = slot->item_q.pop; qn; qn = qn->next) {
        item = data_offset2(qn, wtk_nlg2_item_t, q_n);
        wtk_nlg2_item_print(item);
        printf("\n");
    }
    //printf("\n\n");
}

void wtk_nlg2_add_item(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item)
{
    wtk_strbuf_t *buf = nlg->buf;
    wtk_nlg2_slot_t *slot;
    wtk_queue_node_t *qn;
    wtk_nlg2_item_t *vi;
    int ret;
    int b;

    if (!nlg->global) {
        if (wtk_string_cmp_s(item->function.name,".global") == 0) {
            nlg->global = item;
        }
    }
    wtk_strbuf_reset(buf);
    wtk_nlg2_function_print_key(&(item->function), buf);
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    slot = wtk_nlg2_find_slot(nlg, buf->data, buf->pos, 1);
    if (slot->item_q.len == 0) {
        wtk_queue3_push(&(slot->item_q), &(item->q_n));
    } else {
        b = 0;
        for (qn = slot->item_q.pop; qn; qn = qn->next) {
            vi = data_offset2(qn, wtk_nlg2_item_t, q_n);
            ret = wtk_nlg2_function_cmp(&(vi->function), &(item->function));
            if (ret <= 0) {
                b = 1;
                wtk_queue3_insert_before(&(slot->item_q), qn, &(item->q_n));
                break;
            }
        }
        if (!b) {
            wtk_queue3_push(&(slot->item_q), &(item->q_n));
        }
        //wtk_nlg2_slot_print(slot);
        //exit(0);
    }
}

wtk_nlg2_item_t* wtk_nlg2_find_item(wtk_nlg2_t *nlg, wtk_nlg2_function_t *f)
{
    wtk_strbuf_t *buf;
    wtk_nlg2_slot_t *slot;
    wtk_queue_node_t *qn;
    wtk_nlg2_item_t *vi;
    int ret;

    buf = wtk_strbuf_new(256, 1);
    wtk_nlg2_function_print_key(f, buf);
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    slot = wtk_nlg2_find_slot(nlg, buf->data, buf->pos, 0);
    wtk_strbuf_delete(buf);
    if (slot) {
        for (qn = slot->item_q.pop; qn; qn = qn->next) {
            vi = data_offset2(qn, wtk_nlg2_item_t, q_n);
            ret = wtk_nlg2_function_match(&(vi->function), f);
            if (ret) {
                return vi;
            }
        }
    }
    return NULL;
}

wtk_nlg2_item_t* wtk_nlg2_new_item(wtk_nlg2_t *nlg)
{
    wtk_heap_t *heap = nlg->action_hash->heap;
    wtk_nlg2_item_t *item;

    item = (wtk_nlg2_item_t*) wtk_heap_malloc(heap, sizeof(wtk_nlg2_item_t));
    wtk_nlg2_function_init(&(item->function));
    item->local_expr_q = NULL;
    item->glb_expr_q = NULL;
    item->pre = NULL;
    item->nxt.str = NULL;
    item->before.str = NULL;
    return item;
}

wtk_nlg2_key_t* wtk_nlg2_new_key(wtk_nlg2_t *nlg, char *nm, int nm_len)
{
    wtk_heap_t *heap = nlg->action_hash->heap;
    wtk_nlg2_key_t *k;

    k = (wtk_nlg2_key_t*) wtk_heap_malloc(heap, sizeof(wtk_nlg2_key_t));
    if (nm_len > 0) {
        k->k = wtk_heap_dup_string(heap, nm, nm_len);
    } else {
        k->k = NULL;
    }
    k->v = NULL;
    return k;
}

void wtk_nlg2_expr_item_print(wtk_nlg2_expr_item_t *item)
{
    if (item->is_expr) {
        wtk_rexpr_print(item->v.expr);
    } else {
        printf("#!%s\n", item->v.lua);
    }
}

void wtk_nlg2_item_print(wtk_nlg2_item_t *item)
{
    wtk_queue_node_t *qn;
    //wtk_rexpr_t *expr;
    wtk_nlg2_expr_item_t *ti;

    wtk_nlg2_function_print(&(item->function));
    if (item->pre) {
        printf("+%s\n", item->pre);
    }
    if (item->before.item) {
        printf("<");
        wtk_nlg2_function_print(&(item->before.item->function));
    }
    if (item->nxt.item) {
        printf("=");
        wtk_nlg2_function_print(&(item->nxt.item->function));
    }
    if (item->local_expr_q) {
        for (qn = item->local_expr_q->pop; qn; qn = qn->next) {
            ti = data_offset2(qn, wtk_nlg2_expr_item_t, q_n);
            wtk_nlg2_expr_item_print(ti);
            //expr=data_offset2(qn,wtk_rexpr_t,q_n);
            //wtk_rexpr_print(expr);
        }
    }
    //wtk_debug("local=%p  global=%p\n",item->local_expr_q,item->glb_expr_q);
    if (item->glb_expr_q) {
        for (qn = item->glb_expr_q->pop; qn; qn = qn->next) {
            ti = data_offset2(qn, wtk_nlg2_expr_item_t, q_n);
            wtk_nlg2_expr_item_print(ti);
//			expr=data_offset2(qn,wtk_rexpr_t,q_n);
//			wtk_rexpr_print(expr);
        }
    }
}

void wtk_nlg2_add_local_expr(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item,
        wtk_rexpr_t *expr)
{
    wtk_heap_t *heap = nlg->action_hash->heap;
    wtk_nlg2_expr_item_t *ti;

    if (!item->local_expr_q) {
        item->local_expr_q = (wtk_queue3_t*) wtk_heap_malloc(heap,
                sizeof(wtk_queue3_t));
        wtk_queue3_init(item->local_expr_q);
    }
    ti = (wtk_nlg2_expr_item_t*) wtk_heap_malloc(heap,
            sizeof(wtk_nlg2_expr_item_t));
    ti->is_expr = 1;
    ti->v.expr = expr;
    wtk_queue3_push(item->local_expr_q, &(ti->q_n));
}

void wtk_nlg2_add_global_expr(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item,
        wtk_rexpr_t *expr)
{
    wtk_heap_t *heap = nlg->action_hash->heap;
    wtk_nlg2_expr_item_t *ti;

    if (!item->glb_expr_q) {
        item->glb_expr_q = (wtk_queue3_t*) wtk_heap_malloc(heap,
                sizeof(wtk_queue3_t));
        wtk_queue3_init(item->glb_expr_q);
    }
    ti = (wtk_nlg2_expr_item_t*) wtk_heap_malloc(heap,
            sizeof(wtk_nlg2_expr_item_t));
    ti->is_expr = 1;
    ti->v.expr = expr;
    wtk_queue3_push(item->glb_expr_q, &(ti->q_n));
}

void wtk_nlg2_add_global_lua(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item, char *data,
        int len)
{
    wtk_heap_t *heap = nlg->action_hash->heap;
    wtk_nlg2_expr_item_t *ti;

    if (!item->glb_expr_q) {
        item->glb_expr_q = (wtk_queue3_t*) wtk_heap_malloc(heap,
                sizeof(wtk_queue3_t));
        wtk_queue3_init(item->glb_expr_q);
    }
    ti = (wtk_nlg2_expr_item_t*) wtk_heap_malloc(heap,
            sizeof(wtk_nlg2_expr_item_t));
    ti->is_expr = 0;
    ti->v.lua = wtk_heap_dup_str2(heap, data, len);
    //wtk_debug("lua=[%.*s]\n",len,data);
    wtk_queue3_push(item->glb_expr_q, &(ti->q_n));
}

wtk_string_t* wtk_nlg2_item_get_var(void **ths, char *k, int k_len);

wtk_string_t* wtk_nlg2_item_get_local_var(void **ths, char *k, int k_len)
{
    wtk_queue_node_t *qn;
    wtk_rexpr_t *expr;
    wtk_strbuf_t *buf;
    wtk_string_t *v;
    wtk_heap_t *heap;
    wtk_nlg2_item_t *item;
    wtk_nlg2_t *nlg;
    wtk_nlg2_expr_item_t *ti;

    heap = ths[0];
    item = ths[1];
    if (item->local_expr_q) {
        for (qn = item->local_expr_q->pop; qn; qn = qn->next) {
            ti = data_offset2(qn, wtk_nlg2_expr_item_t, q_n);
            if (!ti->is_expr) {
                continue;
            }
            //expr=data_offset2(qn,wtk_rexpr_t,q_n);
            expr = ti->v.expr;
            //wtk_debug("[%.*s]\n",expr->name->len,expr->name->data);
            if (wtk_string_cmp(expr->name, k, k_len) == 0) {
                buf = wtk_strbuf_new(256, 1);
                wtk_rexpr_gen(expr, buf, ths,
                        (wtk_rexpr_get_var_f) wtk_nlg2_item_get_var);
                v = wtk_heap_dup_string(heap, buf->data, buf->pos);
                wtk_strbuf_delete(buf);
                return v;
            }
        }
    }
    nlg = ths[3];
    //wtk_debug("global=%p\n",nlg->global);
    if (nlg->global && nlg->global->local_expr_q) {
        for (qn = nlg->global->local_expr_q->pop; qn; qn = qn->next) {
            ti = data_offset2(qn, wtk_nlg2_expr_item_t, q_n);
            if (!ti->is_expr) {
                continue;
            }
            //expr=data_offset2(qn,wtk_rexpr_t,q_n);
            expr = ti->v.expr;
            //wtk_debug("[%.*s]\n",expr->name->len,expr->name->data);
            if (wtk_string_cmp(expr->name, k, k_len) == 0) {
                buf = wtk_strbuf_new(256, 1);
                wtk_rexpr_gen(expr, buf, ths,
                        (wtk_rexpr_get_var_f) wtk_nlg2_item_get_var);
                v = wtk_heap_dup_string(heap, buf->data, buf->pos);
                wtk_strbuf_delete(buf);
                return v;
            }
        }
    }
    return NULL;
}

wtk_string_t* wtk_nlg2_item_get_var(void **ths, char *k, int k_len)
{
    wtk_string_t *v;
    wtk_nlg2_function_t *f;
    int i;

    f = ths[2];
    for (i = 0; i < f->narg; ++i) {
        if (f->args[i]->v && wtk_string_cmp(f->args[i]->k, k, k_len) == 0) {
            //wtk_debug("[%.*s]\n",k_len,k);
            v = wtk_nlg2_item_get_local_var(ths, f->args[i]->v->data,
                    f->args[i]->v->len);
            //wtk_debug("v=%p/%p\n",v,f->args[i]->v);
            if (v) {
                return v;
            }
            return f->args[i]->v;
        }
    }
    v = wtk_nlg2_item_get_local_var(ths, k, k_len);
    return v;
}

wtk_string_t* wtk_nlg2_item_get_var2(void **ths, char *k, int k_len)
{
    wtk_nlg2_function_t *f;
    int i;

    f = ths[2];
    for (i = 0; i < f->narg; ++i) {
        if (f->args[i]->v && wtk_string_cmp(f->args[i]->k, k, k_len) == 0) {
            //wtk_debug("[%.*s]\n",k_len,k);
            return f->args[i]->v;
        }
    }
    return NULL;
}

#include "wtk/core/wtk_larray.h"

void wtk_nlg2_item_gen(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item,
        wtk_strbuf_t *buf, wtk_heap_t *heap, wtk_nlg2_function_t *f,
        wtk_nlg2_gen_env_t *env)
{
    wtk_queue_node_t *qn;
    wtk_rexpr_t *expr;
    int i;
    void *ths[4];
    wtk_larray_t *a;
    int ret;
    wtk_nlg2_expr_item_t *ti;

    //wtk_debug("glb=%p\n",item->glb_expr_q);
    if (item->before.item) {
        wtk_nlg2_item_gen(nlg, item->before.item, buf, heap, f, env);
    }
    if (!item->glb_expr_q) {
        wtk_debug("global expr not found\n");
        return;
    }
    ths[0] = heap;
    ths[1] = item;
    ths[2] = f;
    ths[3] = nlg;
    //wtk_debug("glob exp=%d\n",item->glb_expr_q->len);
    a = wtk_larray_new(item->glb_expr_q->len, sizeof(void*));
    for (qn = item->glb_expr_q->pop; qn; qn = qn->next) {
        ti = data_offset2(qn, wtk_nlg2_expr_item_t, q_n);
        if (!ti->is_expr) {
            wtk_larray_push2(a, &(ti));
            continue;
        }
        //expr=data_offset2(qn,wtk_rexpr_t,q_n);
        expr = ti->v.expr;
        if (expr->xif) {
            ret = wtk_if_check(expr->xif, ths,
                    (wtk_if_get_var_f) wtk_nlg2_item_get_var2);
            //wtk_debug("ret=%d\n",ret);
            if (ret == 1) {
                //wtk_rexpr_print(expr);
                wtk_larray_push2(a, &(ti));
            }
        } else {
            //wtk_rexpr_print(expr);
            wtk_larray_push2(a, &(ti));
        }
    }
    //wtk_debug("nslot=%d\n",a->nslot);
    //exit(0);
    if (a->nslot > 0) {
        i = wtk_random(0, a->nslot - 1);
//		qn=wtk_queue3_peek(item->glb_expr_q,i);
//		expr=data_offset2(qn,wtk_rexpr_t,q_n);
        ti = ((wtk_nlg2_expr_item_t**) a->slot)[i];

        //wtk_rexpr_print(expr);
        //wtk_debug("len=%d\n",expr->item_q.len);
        if (item->pre && env->lua_gen2) {
            wtk_string_t v;

            v = env->lua_gen2(env->ths, item->pre);
            if (v.len > 0) {
                wtk_strbuf_push(buf, v.data, v.len);
            }
        }
        if (ti->is_expr) {
            wtk_rexpr_gen(ti->v.expr, buf, ths,
                    (wtk_rexpr_get_var_f) wtk_nlg2_item_get_var);
        } else {
            if (env) {
                //wtk_debug("[%s]\n",ti->v.lua);
                if (env->lua_gen) {
                    env->lua_gen(env->ths, ti->v.lua, f, buf);
                } else if (env->lua_gen2) {
                    wtk_string_t v;

                    v = env->lua_gen2(env->ths, ti->v.lua);
                    if (v.len > 0) {
                        wtk_strbuf_push(buf, v.data, v.len);
                    }
                }
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
            }
            //wtk_debug("[%s]\n",ti->v.lua);
            //exit(0);
        }
    } else {
        wtk_debug("math empty slot\n");
    }
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    //exit(0);
    wtk_larray_delete(a);
    if (item->nxt.item) {
        wtk_nlg2_item_gen(nlg, item->nxt.item, buf, heap, f, env);
    }
}

wtk_string_t wtk_nlg2_process_function(wtk_nlg2_t *nlg, wtk_nlg2_function_t *f,
        wtk_nlg2_gen_env_t *env)
{
    wtk_strbuf_t *buf = nlg->buf;
    wtk_nlg2_item_t *item;
    wtk_heap_t *heap = nlg->rec_heap;
    wtk_string_t v;

    wtk_strbuf_reset(buf);
    item = wtk_nlg2_find_item(nlg, f);
    //wtk_debug("find item=%p\n",item);
    //wtk_debug("glb=%d\n",item->glb_expr_q->len);
    //wtk_nlg2_item_print(item);
    if (item) {
        wtk_nlg2_item_gen(nlg, item, buf, heap, f, env);
        //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    }
    wtk_string_set(&(v), buf->data, buf->pos);
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    wtk_heap_reset(heap);
    return v;
}

wtk_string_t wtk_nlg2_process(wtk_nlg2_t *nlg, char *data, int len,
        wtk_nlg2_gen_env_t *env)
{
    wtk_string_t v;
    wtk_nlg2_function_t f;
    int n;
    int ret;
    wtk_strbuf_t *buf;
    wtk_nlg2_item_t *item;
    wtk_heap_t *heap = nlg->rec_heap;

    buf = wtk_strbuf_new(256, 1);
    wtk_strbuf_reset(buf);
    //wtk_debug("[%.*s]\n",len,data);
    wtk_string_set(&(v), 0, 0);
    while (len > 0) {
        wtk_nlg2_function_init(&(f));
        //wtk_debug("%.*s\n",len,data);
        ret = wtk_nlg2_function_parse(&(f), heap, data, len, &n);
        if (ret != 0) {
            goto end;
        }
        //wtk_nlg2_function_print(&(f));
        item = wtk_nlg2_find_item(nlg, &(f));
        //wtk_debug("glb=%d\n",item->glb_expr_q->len);
        //wtk_nlg2_item_print(item);
        if (item) {
            wtk_nlg2_item_gen(nlg, item, buf, heap, &(f), env);
            //wtk_debug("[%.*s]\n",buf->pos,buf->data);
        } else {
            wtk_debug("=================> [%.*s] not found\n", len, data);
            //exit(0);
        }
        //exit(0);
        //wtk_debug("n=%d\n",n);
        data += n;
        len -= n;
        while (len > 0) {
            if (data[0] != ',') {
                break;
            } else {
                ++data;
                --len;
            }
        }
    }
    if (buf->pos > 0) {
        wtk_heap_fill_string(heap, &(v), buf->data, buf->pos);
    }
    //wtk_string_set(&(v),buf->data,buf->pos);
    end:
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    wtk_strbuf_delete(buf);
    //wtk_heap_reset(heap);
    return v;
}

//void wtk_nlg2_slot_print(wtk_nlg2_slot_t *slot)
//{
//	wtk_queue_node_t *qn;
//	wtk_nlg2_item_t *item;
//
//	for(qn=slot->item_q.pop;qn;qn=qn->next)
//	{
//		item=data_offset2(qn,wtk_nlg2_item_t,q_n);
//		wtk_nlg2_item_print(item);
//	}
//}

void wtk_nlg2_print(wtk_nlg2_t *nlg)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_nlg2_slot_t *slot;

    it = wtk_str_hash_iterator(nlg->action_hash);
    while (1) {
        node = wtk_str_hash_it_next(&(it));
        if (!node) {
            break;
        }
        slot = (wtk_nlg2_slot_t*) node->value;
        //wtk_debug("slot=%d\n",slot->item_q.len);
        wtk_nlg2_slot_print(slot);
    }
}

void wtk_nlg2_slot_update(wtk_nlg2_t *nlg, wtk_nlg2_slot_t *slot,
        wtk_heap_t *heap)
{
    wtk_queue_node_t *qn;
    wtk_nlg2_item_t *item;
    wtk_nlg2_function_t f;
    wtk_nlg2_item_t *ti;

    for (qn = slot->item_q.pop; qn; qn = qn->next) {
        item = data_offset2(qn, wtk_nlg2_item_t, q_n);
        if (item->before.str) {
            wtk_nlg2_function_init(&(f));
            wtk_nlg2_function_parse(&(f), heap, item->before.str->data,
                    item->before.str->len, NULL);
            ti = wtk_nlg2_find_item(nlg, &(f));
            if (ti) {
                item->before.item = ti;
            } else {
                wtk_debug("[%.*s] unfound\n", item->before.str->len,
                        item->before.str->data);
                exit(0);
            }
        }
        if (!item->nxt.str) {
            continue;
        }
        //wtk_debug("[%.*s]\n",item->nxt.str->len,item->nxt.str->data);
        wtk_nlg2_function_init(&(f));
        wtk_nlg2_function_parse(&(f), heap, item->nxt.str->data,
                item->nxt.str->len, NULL);
        ti = wtk_nlg2_find_item(nlg, &(f));
        if (ti) {
            item->nxt.item = ti;
        } else {
            wtk_debug("[%.*s] unfound\n", item->nxt.str->len,
                    item->nxt.str->data);
            exit(0);
        }
    }
    //printf("\n\n");
}

void wtk_nlg2_update(wtk_nlg2_t *nlg)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_nlg2_slot_t *slot;
    wtk_heap_t *heap;

    heap = wtk_heap_new(4096);
    it = wtk_str_hash_iterator(nlg->action_hash);
    while (1) {
        node = wtk_str_hash_it_next(&(it));
        if (!node) {
            break;
        }
        slot = (wtk_nlg2_slot_t*) node->value;
        //wtk_debug("slot=%d\n",slot->item_q.len);
        wtk_nlg2_slot_update(nlg, slot, heap);
    }
    wtk_heap_delete(heap);
}
