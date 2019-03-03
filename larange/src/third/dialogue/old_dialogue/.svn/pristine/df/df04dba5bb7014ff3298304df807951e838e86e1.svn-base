#include "wtk_rexpr.h" 

wtk_rexpr_t* wtk_rexpr_new(wtk_heap_t *heap)
{
    wtk_rexpr_t *e;

    e = (wtk_rexpr_t*) wtk_heap_malloc(heap, sizeof(wtk_rexpr_t));
    e->name = NULL;
    wtk_queue3_init(&(e->item_q));
    e->xif = NULL;
    return e;
}

wtk_rexpr_item_t* wtk_rexpr_get_last_item(wtk_rexpr_t *expr)
{
    wtk_rexpr_item_t *item;

    if (expr->item_q.len <= 0) {
        return NULL;
    } else {
        item = data_offset2(expr->item_q.push, wtk_rexpr_item_t, q_n);
        return item;
    }
}

wtk_rexpr_item_t* wtk_rexpr_item_new(wtk_heap_t *heap)
{
    wtk_rexpr_item_t *item;

    item = (wtk_rexpr_item_t*) wtk_heap_malloc(heap, sizeof(wtk_rexpr_item_t));
    item->type = WTK_REXPR_ITEM_STRING;
    item->v.str = NULL;
    item->repeat_min = 1;
    item->repeat_max = 1;
    return item;
}

wtk_rexpr_item_t* wtk_rexpr_new_item_string(wtk_heap_t *heap, char *v,
        int v_len)
{
    wtk_rexpr_item_t *item;

    item = wtk_rexpr_item_new(heap);
    item->type = WTK_REXPR_ITEM_STRING;
    item->v.str = wtk_heap_dup_string(heap, v, v_len);
    return item;
}

wtk_rexpr_item_t* wtk_rexpr_new_item_var(wtk_heap_t *heap, char *v, int v_len)
{
    wtk_rexpr_item_t *item;

    item = wtk_rexpr_item_new(heap);
    item->type = WTK_REXPR_ITEM_VAR;
    item->v.var = wtk_heap_dup_string(heap, v, v_len);
    return item;
}

wtk_rexpr_cap_t* wtk_rexpr_cap_new(wtk_heap_t *heap)
{
    wtk_rexpr_cap_t *c;

    c = (wtk_rexpr_cap_t*) wtk_heap_malloc(heap, sizeof(wtk_rexpr_cap_t));
    wtk_queue3_init(&(c->or_q));
    return c;
}

wtk_rexpr_branch_t* wtk_rexpr_branch_new(wtk_heap_t *heap)
{
    wtk_rexpr_branch_t *b;

    b = (wtk_rexpr_branch_t*) wtk_heap_malloc(heap, sizeof(wtk_rexpr_branch_t));
    wtk_queue3_init(&(b->item_q));
    return b;
}

wtk_rexpr_item_t* wtk_rexpr_new_item_cap(wtk_heap_t *heap)
{
    wtk_rexpr_item_t *item;

    item = wtk_rexpr_item_new(heap);
    item->type = WTK_REXPR_ITEM_CAP;
    item->v.cap = wtk_rexpr_cap_new(heap);
    return item;
}

void wtk_rexpr_item_print(wtk_rexpr_item_t *item)
{
    wtk_queue_node_t *qn, *qn2;
    wtk_rexpr_branch_t *b;
    wtk_rexpr_item_t *vi;

    switch (item->type) {
        case WTK_REXPR_ITEM_D:
            printf("\\D");
            break;
        case WTK_REXPR_ITEM_STRING:
            printf("%.*s", item->v.str->len, item->v.str->data);
            break;
        case WTK_REXPR_ITEM_VAR:
            printf("${%.*s}", item->v.var->len, item->v.var->data);
            break;
        case WTK_REXPR_ITEM_CAP:
            printf("(");
            for (qn = item->v.cap->or_q.pop; qn; qn = qn->next) {
                if (qn != item->v.cap->or_q.pop) {
                    printf("|");
                }
                b = data_offset2(qn, wtk_rexpr_branch_t, q_n);
                for (qn2 = b->item_q.pop; qn2; qn2 = qn2->next) {
                    vi = data_offset2(qn2, wtk_rexpr_item_t, q_n);
                    //wtk_debug("b=%p vi=%p type=%d\n",b,vi,vi->type);
                    wtk_rexpr_item_print(vi);
                }
            }
            //exit(0);
            printf(")");
            break;
    }
    //? * +
    if (item->repeat_min == 1 && item->repeat_max == 1) {

    } else if (item->repeat_min == 0 && item->repeat_max == 1) {
        printf("?");
    } else if (item->repeat_min == 0 && item->repeat_max == -1) {
        printf("*");
    } else if (item->repeat_min == 1 && item->repeat_max == -1) {
        printf("+");
    } else {
        printf("/min=%d,max=%d/", item->repeat_min, item->repeat_max);
    }
}

void wtk_rexpr_print(wtk_rexpr_t *expr)
{
    wtk_queue_node_t *qn;
    wtk_rexpr_item_t *item;

    if (expr->xif) {
        printf("/");
        wtk_if_print(expr->xif);
        printf("/");
    }
    if (expr->name) {
        printf("%.*s=", expr->name->len, expr->name->data);
    }
    for (qn = expr->item_q.pop; qn; qn = qn->next) {
        item = data_offset2(qn, wtk_rexpr_item_t, q_n);
        wtk_rexpr_item_print(item);
    }
    printf("\n");
}

void wtk_rexpr_item_gen(wtk_rexpr_item_t *item, wtk_strbuf_t *buf, void *ths,
        wtk_rexpr_get_var_f get_var)
{
    wtk_queue_node_t *qn, *qn2;
    wtk_rexpr_branch_t *b;
    wtk_rexpr_item_t *vi;
    int l, r, t;
    int i, n;
    wtk_string_t *v;

    if (item->repeat_min == 1 && item->repeat_max == 1) {
        n = 1;
    } else {
        l = item->repeat_min;
        r = item->repeat_max;
        if (r < 0) {
            r = 10;
        }
        n = wtk_random(l, r);
    }
    //wtk_debug("n=%d [%.*s]\n",n,buf->pos,buf->data);
    for (i = 0; i < n; ++i) {
        //wtk_rexpr_item_print(item);
        //printf("\n");
        //wtk_debug("type=%d\n",item->type);
        switch (item->type) {
            case WTK_REXPR_ITEM_D:
                t = wtk_random(0, 9);
                wtk_strbuf_push_f(buf, "%d", t);
                break;
            case WTK_REXPR_ITEM_STRING:
                wtk_strbuf_push(buf, item->v.str->data, item->v.str->len);
                break;
            case WTK_REXPR_ITEM_VAR:
                //wtk_debug("[%.*s]\n",item->v.var->len,item->v.var->data);
                if (get_var) {
                    v = get_var(ths, item->v.var->data, item->v.var->len);
                    if (v) {
                        //wtk_debug("[%.*s]=[%.*s]\n",item->v.var->len,item->v.var->data,v->len,v->data);
                        wtk_strbuf_push(buf, v->data, v->len);
                    } else {
                        wtk_debug("[%.*s] not found\n", item->v.var->len,
                                item->v.var->data);
                        //exit(0);
                    }
                } else {
                    wtk_strbuf_push_f(buf, "${%.*s}", item->v.var->len,
                            item->v.var->data);
                }
                break;
            case WTK_REXPR_ITEM_CAP:
                //printf("(");
                t = wtk_random(0, item->v.cap->or_q.len - 1);
                //wtk_debug("t=%d\n",t);
                qn = wtk_queue3_peek(&(item->v.cap->or_q), t);
                //wtk_debug("b=%p\n",qn);
                b = data_offset2(qn, wtk_rexpr_branch_t, q_n);
                for (qn2 = b->item_q.pop; qn2; qn2 = qn2->next) {
                    vi = data_offset2(qn2, wtk_rexpr_item_t, q_n);
                    //wtk_debug("b=%p vi=%p type=%d\n",b,vi,vi->type);
                    wtk_rexpr_item_gen(vi, buf, ths, get_var);
                }
                //exit(0);
                //printf(")");
                break;
        }
    }
}

void wtk_rexpr_gen(wtk_rexpr_t *expr, wtk_strbuf_t *buf, void *ths,
        wtk_rexpr_get_var_f get_var)
{
    wtk_queue_node_t *qn;
    wtk_rexpr_item_t *item;

    //wtk_strbuf_reset(buf);
    //wtk_debug("n=%d\n",expr->item_q.len);
    for (qn = expr->item_q.pop; qn; qn = qn->next) {
        item = data_offset2(qn, wtk_rexpr_item_t, q_n);
        wtk_rexpr_item_gen(item, buf, ths, get_var);
    }
}

void wtk_rexpr_add_depend(wtk_rexpr_t *expr, char *data, int len,
        wtk_heap_t *heap)
{
    expr->xif = wtk_if_new(heap, data, len);
}
