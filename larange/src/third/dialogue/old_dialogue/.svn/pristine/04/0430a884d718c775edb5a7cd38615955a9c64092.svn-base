#include <ctype.h>
#include "wtk_owl_tree.h" 

wtk_owl_tree_t* wtk_owl_tree_new()
{
    wtk_owl_tree_t *tree;

    tree = (wtk_owl_tree_t*) wtk_malloc(sizeof(wtk_owl_tree_t));
    tree->heap = wtk_heap_new(4096);
    tree->inst_map = wtk_str_hash_new(137);
    tree->cls_map = wtk_str_hash_new(137);
    return tree;
}

void wtk_owl_tree_delete(wtk_owl_tree_t *owl)
{
    wtk_str_hash_delete(owl->inst_map);
    wtk_str_hash_delete(owl->cls_map);
    wtk_heap_delete(owl->heap);
    wtk_free(owl);
}

wtk_owl_item_t* wtk_owl_tree_new_item(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes)
{
    wtk_heap_t *heap = owl->heap;
    wtk_owl_item_t *item;

    item = (wtk_owl_item_t*) wtk_heap_malloc(heap, sizeof(wtk_owl_item_t));
    if (nm_bytes > 0) {
        item->k.str = wtk_heap_dup_string(heap, nm, nm_bytes);
    } else {
        item->k.str = NULL;
    }
    item->use_k_inst = 0;
    item->v = NULL;
    item->nv = 0;
    item->v_inst = NULL;
    item->n_vinst = 0;
    item->nitem = 0;
    item->items = NULL;
    item->hook = NULL;
    item->attr = NULL;
    item->parent_item = NULL;
    item->class_parent_item = NULL;
    return item;
}

//wtk_owl_prop_t* wtk_owl_tree_new_prop(wtk_owl_tree_t *owl)
//{
//	wtk_heap_t *heap=owl->heap;
//	wtk_owl_prop_t *prop;
//
//	prop=(wtk_owl_prop_t*)wtk_heap_malloc(heap,sizeof(wtk_owl_prop_t));
//	prop->items=NULL;
//	prop->nitem=0;
//	return prop;
//}

wtk_owl_inst_t* wtk_owl_tree_new_inst(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes)
{
    wtk_heap_t *heap = owl->heap;
    wtk_owl_inst_t *inst;

    inst = (wtk_owl_inst_t*) wtk_heap_malloc(heap, sizeof(wtk_owl_inst_t));
    inst->prop = NULL; //wtk_owl_tree_new_prop(owl);
    inst->item = wtk_owl_tree_new_item(owl, nm, nm_bytes);
    inst->class = NULL;
    //inst->class_item=NULL;
    //wtk_queue_push(&(owl->inst_q),&(inst->q_n));
    return inst;
}

wtk_owl_inst_t* wtk_owl_tree_find_inst(wtk_owl_tree_t *tree, char *nm,
        int nm_bytes, int insert)
{
    wtk_owl_inst_t *inst;

    inst = (wtk_owl_inst_t*) wtk_str_hash_find(tree->inst_map, nm, nm_bytes);
//	if(wtk_str_equal_s(nm,nm_bytes,"葫芦娃"))
//	{
//		wtk_debug("found bug inst=%p\n",inst);
//	}
    if (inst || !insert) {
//		if(wtk_str_equal_s(nm,nm_bytes,"葫芦娃"))
//		{
//			wtk_owl_inst_print(inst);
//			wtk_debug("return\n");
//		}
        goto end;
    }
    inst = wtk_owl_tree_new_inst(tree, nm, nm_bytes);
    wtk_str_hash_add(tree->inst_map, inst->item->k.str->data,
            inst->item->k.str->len, inst);
    end: return inst;
}

wtk_owl_class_t* wtk_owl_tree_new_class(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes)
{
    wtk_heap_t *heap = owl->heap;
    wtk_owl_class_t *cls;

    cls = (wtk_owl_class_t*) wtk_heap_malloc(heap, sizeof(wtk_owl_class_t));
    cls->prop = NULL;	//wtk_owl_tree_new_prop(owl);
    cls->item = wtk_owl_tree_new_item(owl, nm, nm_bytes);
    cls->parent = NULL;
    //wtk_queue_push(&(owl->cls_q),&(cls->q_n));
    return cls;
}

wtk_owl_class_t* wtk_owl_tree_find_class(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes, int insert)
{
    wtk_owl_class_t *cls;
    wtk_str_hash_t *hash = owl->cls_map;

    cls = (wtk_owl_class_t*) wtk_str_hash_find(hash, nm, nm_bytes);
    if (cls || !insert) {
        goto end;
    }
    cls = wtk_owl_tree_new_class(owl, nm, nm_bytes);
    wtk_str_hash_add(hash, cls->item->k.str->data, cls->item->k.str->len, cls);
    end: return cls;
}

wtk_owl_item_attr_t* wtk_owl_item_attr_new(wtk_heap_t *heap)
{
    wtk_owl_item_attr_t *a;

    a = (wtk_owl_item_attr_t*) wtk_heap_malloc(heap,
            sizeof(wtk_owl_item_attr_t));
    wtk_queue3_init(&(a->q));
    return a;
}

wtk_owl_item_attr_kv_t* wtk_owl_item_attr_kv_new(wtk_heap_t *heap, char *k,
        int k_bytes)
{
    wtk_owl_item_attr_kv_t *kv;

    kv = (wtk_owl_item_attr_kv_t*) wtk_heap_malloc(heap,
            sizeof(wtk_owl_item_attr_kv_t));
    kv->k = wtk_heap_dup_string(heap, k, k_bytes);
    wtk_queue3_init(&(kv->v));
    return kv;
}

wtk_owl_item_attr_v_t* wtk_owl_item_attr_v_new(wtk_heap_t *heap, char *v,
        int v_bytes)
{
    wtk_owl_item_attr_v_t *xv;
    char *px;
    wtk_string_t a;

    xv = (wtk_owl_item_attr_v_t*) wtk_heap_malloc(heap,
            sizeof(wtk_owl_item_attr_v_t));
    xv->xif = NULL;
    if (v[0] == '/') {
        //wtk_debug("[%.*s]\n",v_bytes,v);
        px = wtk_str_chr(v + 1, v_bytes - 1, '/');
        //wtk_debug("px=%p\n",px);
        if (px) {
            //wtk_debug("[%.*s]\n",(int)(px-buf->data-1),buf->data+1);
            wtk_string_set(&(a), v + 1, (int )(px - v - 1));
            //wtk_debug("[%.*s]\n",a.len,a.data);
            xv->xif = wtk_if_new(heap, a.data, a.len);
            px += 1;
            v_bytes = v + v_bytes - px;
            v = px;
            //wtk_debug("[%.*s]\n",v_bytes,v);
            //exit(0);
        }
    }
    xv->v = wtk_heap_dup_string(heap, v, v_bytes);
    return xv;
}

void wtk_owl_tree_add_attr(wtk_owl_tree_t *owl, wtk_owl_item_t *item, char *a,
        int a_bytes, char *v, int v_bytes)
{
    wtk_owl_item_attr_kv_t *kv;
    wtk_owl_item_attr_v_t *xv;

    kv = wtk_owl_tree_find_attr(owl, item, a, a_bytes, 0);
    if (!kv) {
        return;
    }
    xv = wtk_owl_item_attr_v_new(owl->heap, v, v_bytes);
    wtk_queue3_push(&(kv->v), &(xv->q_n));
}

#include "wtk/core/wtk_os.h"

wtk_string_t* wtk_owl_item_attr_kv_get_value(wtk_owl_item_attr_kv_t *kv,
        int idx)
{
    wtk_queue_node_t *qn;
    wtk_owl_item_attr_v_t *xv;

    if (kv->v.len <= 0) {
        return NULL;
    }
    if (idx < 0) {
        if (kv->v.len == 1) {
            idx = 0;
        } else {
            idx = wtk_random(0, kv->v.len - 1);
        }
    }
    //wtk_debug("len=%d idx=%d [%.*s]\n",kv->v.len,idx,kv->k->len,kv->k->data);
    qn = wtk_queue_peek((wtk_queue_t*) &(kv->v), idx);
    //wtk_debug("qn=%p\n",qn);
    xv = data_offset2(qn, wtk_owl_item_attr_v_t, q_n);
    return xv->v;
}

wtk_owl_item_attr_kv_t* wtk_owl_item_find_attr(wtk_owl_item_t *item, char *a,
        int a_bytes)
{
    wtk_owl_item_attr_kv_t *kv = NULL;
    wtk_queue_node_t *qn;
    wtk_owl_item_attr_kv_t *tmp;

    if (!item->attr) {
        goto end;
    }
    for (qn = item->attr->q.pop; qn; qn = qn->next) {
        tmp = data_offset2(qn, wtk_owl_item_attr_kv_t, q_n);
        //wtk_debug("[%.*s]\n",tmp->k->len,tmp->k->data);
        if (wtk_string_cmp(tmp->k, a, a_bytes) == 0) {
            return tmp;
        }
    }
    end: if (!kv && item->parent_item) {
        kv = wtk_owl_item_find_attr(item->parent_item, a, a_bytes);
    }
    if (!kv && item->class_parent_item) {
        kv = wtk_owl_item_find_attr(item->class_parent_item, a, a_bytes);
    }
    return kv;
}

wtk_string_t* wtk_owl_item_find_attr_value(wtk_owl_item_t *item, char *a,
        int a_bytes, int idx)
{
    wtk_string_t *v = NULL;
    wtk_owl_item_attr_kv_t *kv;

    kv = wtk_owl_item_find_attr(item, a, a_bytes);
    if (!kv) {
        goto end;
    }
    //wtk_owl_item_print(item);
    v = wtk_owl_item_attr_kv_get_value(kv, idx);
    end: return v;
}

wtk_owl_item_attr_kv_t* wtk_owl_tree_find_attr(wtk_owl_tree_t *owl,
        wtk_owl_item_t *item, char *a, int a_bytes, int insert)
{
    return wtk_owl_tree_find_attr2(owl, item, a, a_bytes, insert, 1);
}

wtk_owl_item_attr_kv_t* wtk_owl_tree_find_attr2(wtk_owl_tree_t *owl,
        wtk_owl_item_t *item, char *a, int a_bytes, int insert, int use_parent)
{
    wtk_heap_t *heap = owl->heap;
    wtk_owl_item_attr_kv_t *kv = NULL;
    wtk_queue_node_t *qn;
    wtk_owl_item_attr_kv_t *tmp;

    if (!item->attr && insert == 0) {
        goto end;
    }
    if (!item->attr) {
        item->attr = wtk_owl_item_attr_new(heap);
    } else {
        for (qn = item->attr->q.pop; qn; qn = qn->next) {
            tmp = data_offset2(qn, wtk_owl_item_attr_kv_t, q_n);
            //wtk_debug("[%.*s]\n",tmp->k->len,tmp->k->data);
            if (wtk_string_cmp(tmp->k, a, a_bytes) == 0) {
                return tmp;
            }
        }
    }
    if (!insert) {
        goto end;
    }
    kv = wtk_owl_item_attr_kv_new(heap, a, a_bytes);
    wtk_queue3_push(&(item->attr->q), &(kv->q_n));
    end: if (!kv && use_parent && item->parent_item) {
        kv = wtk_owl_tree_find_attr(owl, item->parent_item, a, a_bytes, insert);
    }
    return kv;
}

void wtk_owl_tree_item_notify(void *ths, wtk_string_t *k, wtk_string_t *v)
{
    void **p;
    wtk_heap_t *heap;
    wtk_larray_t *a;

    p = (void**) ths;
    //wtk_debug("[%.*s]\n",k->len,k->data);//,v->len,v->data);
    heap = (wtk_heap_t*) p[0];
    a = (wtk_larray_t*) p[1];
    k = wtk_heap_dup_string(heap, k->data, k->len);
    wtk_larray_push2(a, &k);
}

float wtk_owl_tree_string_cmp(void *ths, wtk_string_t **src, wtk_string_t **dst)
{
    wtk_string_t *v1, *v2;

    //wtk_debug("[%.*s]\n",src[0]->len,src[0]->data);
    v1 = src[0];
    v2 = dst[0];
    if (v1->len > v2->len) {
        return 1;
    } else if (v1->len < v2->len) {
        return -1;
    } else {
        return wtk_string_cmp(v1, v2->data, v2->len);
    }
    return 0;
}

void wtk_owl_tree_set_item_value(wtk_owl_tree_t *owl, wtk_owl_state_item_t *si,
        wtk_owl_item_t *item, char *v, int v_bytes)
{
    wtk_larray_t *a, *b;
    wtk_string_t *vp, *k;
    wtk_owl_inst_t *inst;
    wtk_strkv_parser_t psr;
    int ret;

    //wtk_debug("[%.*s]\n",v_bytes,v);
    a = wtk_larray_new(10, sizeof(wtk_string_t*));
    b = wtk_larray_new(5, sizeof(wtk_owl_inst_t*));
    wtk_strkv_parser_init(&(psr), v, v_bytes);
    while (1) {
        ret = wtk_strkv_parser_next(&(psr));
        if (ret != 0) {
            break;
        }
        //wtk_debug("[%.*s]=[%.*s]\n",psr.k.len,psr.k.data,psr.v.len,psr.v.data);
        if (psr.k.data[0] == '$') {
            inst = wtk_owl_tree_find_inst(owl, psr.k.data + 1, psr.k.len - 1,
                    1);
            wtk_larray_push2(b, &(inst));
        } else {
            k = wtk_heap_dup_string(owl->heap, psr.k.data, psr.k.len);
            if (si->type == WTK_OWL_CLASS) {
                //wtk_debug("foudn bug\n");
                wtk_str_hash_add(owl->cls_map, k->data, k->len, si->v.class);
            }
            wtk_larray_push2(a, &k);
        }
        if (wtk_strkv_parser_is_end(&(psr))) {
            break;
        }
    }
    if (a->nslot > 0) {
        wtk_qsort(a->slot, (char*)a->slot+ a->slot_size * (a->nslot - 1),
                a->slot_size, (wtk_qsort_cmp_f) wtk_owl_tree_string_cmp, NULL,
                &vp);
        item->nv = a->nslot;
        item->v = (wtk_string_t**) wtk_heap_malloc(owl->heap,
                sizeof(wtk_string_t*) * a->nslot);
        memcpy(item->v, a->slot, item->nv * sizeof(wtk_string_t*));
    }
    if (b->nslot > 0) {
        item->n_vinst = b->nslot;
        item->v_inst = (wtk_owl_inst_t**) wtk_heap_malloc(owl->heap,
                sizeof(wtk_owl_inst_t*) * b->nslot);
        memcpy(item->v_inst, b->slot, item->n_vinst * sizeof(wtk_owl_inst_t*));
    }
    //wtk_owl_item_print(item);
    wtk_larray_delete(a);
    wtk_larray_delete(b);
    //exit(0);
}

#include "wtk/core/wtk_os.h"

int wtk_owl_random(int n)
{
    static int cnt = 0;
    int v;

    srand(time_get_ms());
    if (cnt > 100) {
        cnt = 0;
    }
    v = rand() + cnt;
    ++cnt;
    v = v % n;
    return v;
}

wtk_string_t* wtk_owl_item_get_str_value(wtk_owl_item_t *item)
{
    int v;

    //wtk_owl_item_print(item);
    //wtk_debug("nv=%d/%d\n",item->nv,item->n_vinst);
    if (item->nv > 0) {
        if (item->nv == 1) {
            return item->v[0];
        } else {
            v = wtk_owl_random(item->nv);
            return item->v[v];
        }
    }
    if (item->n_vinst > 0) {
        if (item->n_vinst == 1) {
            return wtk_owl_item_get_str_value(item->v_inst[0]->item);
        } else {
            v = wtk_owl_random(item->n_vinst);
            return wtk_owl_item_get_str_value(item->v_inst[v]->item);
        }
    }
    if (item->use_k_inst) {
        return item->k.inst->item->k.str;
    } else {
        return item->k.str;
    }
    return NULL;
}

void wtk_owl_item_print(wtk_owl_item_t *item)
{
    int i;
    int cnt = 0;

    if (item->use_k_inst) {
        printf("<item=\"$%.*s\" v=\"", item->k.inst->item->k.str->len,
                item->k.inst->item->k.str->data);
    } else {
        if (item->k.str) {
            printf("<item=\"%.*s\" v=\"", item->k.str->len, item->k.str->data);
        } else {
            printf("<item  ");
        }
    }
    for (i = 0; i < item->nv; ++i) {
        ++cnt;
        if (i > 0) {
            printf(",");
        }
        printf("%.*s", item->v[i]->len, item->v[i]->data);
    }
    for (i = 0; i < item->n_vinst; ++i) {
        ++cnt;
        if (cnt > 1) {
            printf(",");
        }
        printf("$%.*s", item->v_inst[i]->item->k.str->len,
                item->v_inst[i]->item->k.str->data);
    }
    printf("\"");
    if (item->attr) {
        wtk_queue_node_t *qn, *qn2;
        wtk_owl_item_attr_kv_t *kv;
        wtk_owl_item_attr_v_t *xv;

        for (qn = item->attr->q.pop; qn; qn = qn->next) {
            kv = data_offset2(qn, wtk_owl_item_attr_kv_t, q_n);
            for (qn2 = kv->v.pop; qn2; qn2 = qn2->next) {
                xv = data_offset2(qn2, wtk_owl_item_attr_v_t, q_n);
                printf(" %.*s=\"%.*s\"", kv->k->len, kv->k->data, xv->v->len,
                        xv->v->data);
            }
        }
    }
    if (item->nitem == 0) {
        printf("/>\n");
    } else {
        printf(">\n");
        for (i = 0; i < item->nitem; ++i) {
            wtk_owl_item_print(item->items[i]);
        }
        printf("</item>\n");
    }
}

void wtk_owl_prop_print(wtk_owl_item_t *prop)
{
    int i;

    printf("<prop>\n");
    for (i = 0; i < prop->nitem; ++i) {
        wtk_owl_item_print(prop->items[i]);
    }
    printf("</prop>\n");
}

void wtk_owl_class_print(wtk_owl_class_t *cls)
{

    if (cls->prop) {
        printf("<class=\"%.*s\">\n", cls->item->k.str->len,
                cls->item->k.str->data);
        if (cls->parent) {
            printf("<parent=\"%.*s\">\n", cls->parent->item->k.str->len,
                    cls->parent->item->k.str->data);
        }
        wtk_owl_prop_print(cls->prop);
        printf("</class>\n");
    } else {
        printf("<class=\"%.*s\"/>\n", cls->item->k.str->len,
                cls->item->k.str->data);
    }
}

void wtk_owl_inst_print(wtk_owl_inst_t *inst)
{
    int i;

    if (inst->prop) {
        printf("<inst=\"%.*s\" v=\"", inst->item->k.str->len,
                inst->item->k.str->data);
        for (i = 0; i < inst->item->nv; ++i) {
            if (i > 0) {
                printf(",");
            }
            printf("%.*s", inst->item->v[i]->len, inst->item->v[i]->data);
        }
        printf("\">\n");
        wtk_owl_prop_print(inst->prop);
        printf("</inst>\n");
    } else {
        printf("<inst=\"%.*s\" v=\"", inst->item->k.str->len,
                inst->item->k.str->data);
        for (i = 0; i < inst->item->nv; ++i) {
            if (i > 0) {
                printf(",");
            }
            printf("%.*s", inst->item->v[i]->len, inst->item->v[i]->data);
        }
        printf("\"/>\n");
    }
}

void wtk_owl_tree_print(wtk_owl_tree_t *owl)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_owl_class_t *cls;
    wtk_owl_inst_t *inst;

    it = wtk_str_hash_iterator(owl->cls_map);
    while (1) {
        node = wtk_str_hash_it_next(&(it));
        if (!node) {
            break;
        }
        cls = (wtk_owl_class_t*) node->value;
        wtk_owl_class_print(cls);
        printf("\n");
    }
    it = wtk_str_hash_iterator(owl->inst_map);
    while (1) {
        node = wtk_str_hash_it_next(&(it));
        if (!node) {
            break;
        }
        inst = (wtk_owl_inst_t*) node->value;
        wtk_owl_inst_print(inst);
        printf("\n");
    }
}

void wtk_owl_tree_print_inst(wtk_owl_tree_t *owl)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_owl_inst_t *inst;
    int i;

    it = wtk_str_hash_iterator(owl->inst_map);
    while (1) {
        node = wtk_str_hash_it_next(&(it));
        if (!node) {
            break;
        }
        inst = (wtk_owl_inst_t*) node->value;
        for (i = 0; i < inst->item->nv; ++i) {
            printf("%.*s\n", inst->item->v[i]->len, inst->item->v[i]->data);
        }
    }
}

wtk_owl_expr_t* wtk_owl_expr_new(wtk_heap_t *heap, char *expr, int expr_bytes)
{
    typedef enum {
        WTK_OWL_EXPR_INIT,
        WTK_OWL_EXPR_WAIT_K,
        WTK_OWL_EXPR_K,
        WTK_OWL_EXPR_WAIT_NXT,
        WTK_OWL_EXPR_WAIT_EQ,
        WTK_OWL_EXPR_WAIT_V,
        WTK_OWL_EXPR_V,
    } wtk_owl_expr_state_t;
    wtk_owl_expr_t *x;
    char *s, *e;
    int n;
    wtk_owl_expr_state_t state;
    wtk_string_t k, *v;
    wtk_larray_t *a;

    wtk_string_set(&(k), 0, 0);
    a = wtk_larray_new(10, sizeof(void*));
    x = (wtk_owl_expr_t*) wtk_malloc(sizeof(wtk_owl_expr_t));
    x->k = NULL;
    x->nk = 0;
    x->v = NULL;
    s = expr;
    e = s + expr_bytes;
    state = WTK_OWL_EXPR_INIT;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        switch (state) {
            case WTK_OWL_EXPR_INIT:
                if (n == 1 && *s == '(') {
                    state = WTK_OWL_EXPR_WAIT_K;
                }
                break;
            case WTK_OWL_EXPR_WAIT_K:
                if (n > 1 || !isspace(*s)) {
                    k.data = s;
                    state = WTK_OWL_EXPR_K;
                }
                break;
            case WTK_OWL_EXPR_K:
                if (n == 1 && (*s == ',' || *s == ')' || isspace(*s))) {
                    k.len = s - k.data;
                    //wtk_debug("[%.*s]\n",k.len,k.data);
                    v = wtk_heap_dup_string(heap, k.data, k.len);
                    wtk_larray_push2(a, &v);
                    if (*s == ',') {
                        state = WTK_OWL_EXPR_WAIT_K;
                    } else if (*s == ')') {
                        state = WTK_OWL_EXPR_WAIT_EQ;
                    } else {
                        state = WTK_OWL_EXPR_WAIT_NXT;
                    }
                    //exit(0);
                }
                break;
            case WTK_OWL_EXPR_WAIT_NXT:
                if (*s == ',') {
                    state = WTK_OWL_EXPR_WAIT_K;
                } else if (*s == ')') {
                    state = WTK_OWL_EXPR_WAIT_EQ;
                }
                break;
            case WTK_OWL_EXPR_WAIT_EQ:
                if (n == 1 && *s == '=') {
                    state = WTK_OWL_EXPR_WAIT_V;
                }
                break;
            case WTK_OWL_EXPR_WAIT_V:
                if (n > 1 || !isspace(*s)) {
                    k.data = s;
                    state = WTK_OWL_EXPR_V;
                    if (n + s >= e) {
                        k.len = e - k.data;
                        x->v = wtk_heap_dup_string(heap, k.data, k.len);
                    }
                }
                break;
            case WTK_OWL_EXPR_V:
                if (isspace(*s)) {
                    k.len = s - k.data;
                    x->v = wtk_heap_dup_string(heap, k.data, k.len);
                    s = e;
                } else if (n + s >= e) {
                    k.len = e - k.data;
                    x->v = wtk_heap_dup_string(heap, k.data, k.len);
                }
                break;
        }
        s += n;
    }
    //wtk_debug("nslot=%d\n",a->nslot);
    x->k = (wtk_string_t**) wtk_heap_malloc(heap,
            sizeof(wtk_string_t*) * a->nslot);
    x->nk = a->nslot;
    memcpy(x->k, a->slot, a->nslot * a->slot_size);
    wtk_larray_delete(a);
    return x;
}

void wtk_owl_expr_print(wtk_owl_expr_t *expr)
{
    int i;

    printf("(");
    for (i = 0; i < expr->nk; ++i) {
        if (i > 0) {
            printf(",");
        }
        printf("%.*s", expr->k[i]->len, expr->k[i]->data);
    }
    printf(")=");
    printf("%.*s\n", expr->v->len, expr->v->data);
}

float wtk_owl_tree_string_cmp2(wtk_string_t *v1, wtk_string_t **dst)
{
    wtk_string_t *v2;

    //wtk_debug("[%.*s]\n",src[0]->len,src[0]->data);
    v2 = dst[0];
    //wtk_debug("[%.*s]=[%.*s]\n",v1->len,v1->data,v2->len,v2->data);
    if (v1->len > v2->len) {
        return 1;
    } else if (v1->len < v2->len) {
        return -1;
    } else {
        return wtk_string_cmp(v1, v2->data, v2->len);
    }
    return 0;
}

//wtk_owl_class_t* wtk_owl_tree_find_class(wtk_owl_tree_t *tree,char *nm,int nm_bytes)
//{
//	wtk_owl_class_t *cls;
//	wtk_queue_node_t *qn;
//
//	for(qn=tree->cls_q.pop;qn;qn=qn->next)
//	{
//		cls=data_offset2(qn,wtk_owl_class_t,q_n);
//		if(wtk_string_cmp(cls->item->k,nm,nm_bytes)==0)
//		{
//			return cls;
//		}
//	}
//	return NULL;
//}

wtk_owl_item_t* wtk_owl_item_find_item_by_class(wtk_owl_item_t *item, char *k,
        int k_len)
{
    wtk_owl_item_t *oi;
    int i;

    for (i = 0; i < item->nitem; ++i) {
        oi = item->items[i];
        if (oi->use_k_inst) {
            continue;
        }
        if (wtk_string_cmp(oi->k.str, k, k_len) == 0) {
            return oi;
        }
    }
    if (item->class_parent_item) {
        return wtk_owl_item_find_item_by_class(item->class_parent_item, k,
                k_len);
    } else {
        return NULL;
    }
}

void wtk_owl_item_update(wtk_owl_item_t *item)
{
    int i;
    wtk_owl_item_t *oi;

    for (i = 0; i < item->nitem; ++i) {
        oi = item->items[i];
        if (oi->class_parent_item || oi->use_k_inst) {
            continue;
        }
        oi->class_parent_item = wtk_owl_item_find_item_by_class(
                item->class_parent_item, oi->k.str->data, oi->k.str->len);
        if (oi->nitem > 0) {
            wtk_owl_item_update(oi);
        }
    }
}

void wtk_owl_class_update(wtk_owl_class_t *cls)
{
    if (cls->prop->class_parent_item || !cls->parent) {
        return;
    }
    if (cls->parent && !cls->parent->prop->class_parent_item) {
        wtk_owl_class_update(cls->parent);
    }
    cls->prop->class_parent_item = cls->parent->prop;
    wtk_owl_item_update(cls->prop);
}

void wtk_owl_tree_update(wtk_owl_tree_t *owl)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_owl_class_t *cls;

    it = wtk_str_hash_iterator(owl->cls_map);
    while (1) {
        node = wtk_str_hash_it_next(&(it));
        if (!node) {
            break;
        }
        cls = (wtk_owl_class_t*) node->value;
        if (cls->parent) {
            wtk_owl_class_update(cls);
        }
    }
}

wtk_owl_item_t* wtk_owl_prop_find_item(wtk_owl_prop_t *prop, wtk_string_t *k)
{
    wtk_owl_item_t *item;
    int i;

    //wtk_debug("[%.*s]\n",k->len,k->data);
    for (i = 0; i < prop->nitem; ++i) {
        item = prop->items[i];
        //wtk_owl_item_print(item);
        if (item->use_k_inst) {
            if (wtk_owl_inst_match(item->k.inst, k->data, k->len)) {
                return item;
            }
        } else {
            if (wtk_string_cmp(item->k.str, k->data, k->len) == 0) {
                return item;
            }
        }
    }
    return NULL;
}
wtk_owl_item_t* wtk_owl_item_find_item(wtk_owl_item_t *prop, wtk_string_t *k)
{
    wtk_owl_item_t *item;
    int i;

    //wtk_debug("[%.*s]\n",k->len,k->data);
    for (i = 0; i < prop->nitem; ++i) {
        item = prop->items[i];
        //wtk_owl_item_print(item);
        if (item->use_k_inst) {
            if (wtk_owl_inst_match(item->k.inst, k->data, k->len)) {
                return item;
            }
        } else {
            if (wtk_string_cmp(item->k.str, k->data, k->len) == 0) {
                return item;
            }
        }
    }
    return NULL;
}

wtk_owl_item_t* wtk_owl_item_find_item2(wtk_owl_item_t *prop, char *s, int len)
{
    wtk_owl_item_t *item;
    int i;

    //wtk_debug("[%.*s]\n",k->len,k->data);
    for (i = 0; i < prop->nitem; ++i) {
        item = prop->items[i];
        //wtk_owl_item_print(item);
        if (item->use_k_inst) {
            if (wtk_owl_inst_match(item->k.inst, s, len)) {
                return item;
            }
        } else {
            if (wtk_string_cmp(item->k.str, s, len) == 0) {
                return item;
            }
        }
    }
    if (prop->class_parent_item) {
        return wtk_owl_item_find_item2(prop->class_parent_item, s, len);
    }
    return NULL;
}

wtk_owl_item_t* wtk_owl_class_find_prop(wtk_owl_class_t *cls, wtk_string_t *k)
{
    wtk_owl_item_t *item;

    if (cls->prop) {
        item = wtk_owl_item_find_item(cls->prop, k);
        if (item) {
            return item;
        }
    }
    if (cls->parent) {
        return wtk_owl_class_find_prop(cls->parent, k);
    }
    return NULL;
}

wtk_owl_item_t* wtk_owl_inst_find_prop(wtk_owl_inst_t *inst, wtk_string_t *k)
{
    wtk_owl_item_t *item;

    if (inst->prop) {
        item = wtk_owl_item_find_item(inst->prop, k);
        if (item) {
            return item;
        }
    }
    if (inst->class) {
        item = wtk_owl_class_find_prop(inst->class, k);
        if (item) {
            return item;
        }
    }
    return NULL;
}

int wtk_owl_item_match_value(wtk_owl_item_t *item, char *nm, int nm_bytes)
{
    wtk_string_t k;
    wtk_string_t *v;
    int i;

    //wtk_debug("[%.*s]\n",nm_bytes,nm);
    if (item->nv > 0) {
        wtk_string_set(&(k), nm, nm_bytes);
        v = (wtk_string_t*) wtk_binary_search(item->v, item->v + item->nv - 1,
                sizeof(wtk_string_t*),
                (wtk_search_cmp_f) wtk_owl_tree_string_cmp2, &k);
        if (v) {
            return 1;
        }
    }
    if (item->n_vinst > 0) {
        for (i = 0; i < item->n_vinst; ++i) {
            if (wtk_owl_inst_match(item->v_inst[i], nm, nm_bytes)) {
                return 1;
            }
        }
    }
    return 0;
}

int wtk_owl_inst_match(wtk_owl_inst_t *inst, char *nm, int nm_bytes)
{
    return wtk_owl_item_match_value(inst->item, nm, nm_bytes);
}

wtk_owl_inst_t* wtk_owl_tree_match_inst(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes)
{
    wtk_str_hash_it_t it;
    hash_str_node_t *node;
    wtk_owl_inst_t *inst;
    int ret;

    //可以考虑哈希查找
    it = wtk_str_hash_iterator(owl->inst_map);
    while (1) {
        node = wtk_str_hash_it_next(&(it));
        if (!node) {
            break;
        }
        inst = (wtk_owl_inst_t*) node->value;
        ret = wtk_owl_inst_match(inst, nm, nm_bytes);
        if (ret) {
            return inst;
        }
    }
    return NULL;
}

wtk_owl_item_t* wtk_owl_tree_find_item(wtk_owl_tree_t *owl, wtk_string_t **k,
        int nk)
{
    wtk_owl_item_t *item = NULL;
    wtk_owl_inst_t *inst;
    int i;

    if (nk <= 0) {
        goto end;
    }
    inst = wtk_owl_tree_match_inst(owl, k[0]->data, k[0]->len);
    if (!inst) {
        goto end;
    }
    for (i = 1; i < nk; ++i) {
        item = wtk_owl_inst_find_prop(inst, k[i]);
        if (!item) {
            wtk_debug("[%.*s] property not found.\n", k[i]->len, k[i]->data);
            goto end;
        }
        if (i < (nk - 1)) {
            if (item->n_vinst == 1) {
                inst = item->v_inst[0];
            } else {
                //多个候选者，或者没有inst;
                item = NULL;
                goto end;
            }
        };
    }
    if (!item) {
        goto end;
    }
    end: return item;
}

int wtk_owl_tree_check_expr(wtk_owl_tree_t *owl, wtk_owl_expr_t *expr)
{
    wtk_owl_item_t *item = NULL;
    int ret = -1;

    item = wtk_owl_tree_find_item(owl, expr->k, expr->nk);
    if (!item) {
        goto end;
    }
    //wtk_owl_item_print(item);
    ret = wtk_owl_item_match_value(item, expr->v->data, expr->v->len);
    ret = ret == 1 ? 0 : -1;
    end: return ret;
}

int wtk_owl_tree_expr_is(wtk_owl_tree_t *owl, char *expr, int expr_bytes)
{
    wtk_heap_t *heap;
    wtk_owl_expr_t *x;
    int ret;

    heap = wtk_heap_new(4096);
    //wtk_debug("[%.*s]\n",expr_bytes,expr);
    x = wtk_owl_expr_new(heap, expr, expr_bytes);
    //wtk_owl_expr_print(x);
    ret = wtk_owl_tree_check_expr(owl, x);
    wtk_heap_delete(heap);
    return ret == 0 ? 1 : 0;
}

wtk_string_t* wtk_owl_tree_expr_value(wtk_owl_tree_t *owl, char *expr,
        int expr_bytes)
{
    wtk_owl_item_t *item = NULL;
    wtk_heap_t *heap;
    wtk_owl_expr_t *x;
    wtk_string_t *v = NULL;

    heap = wtk_heap_new(4096);
    //wtk_debug("[%.*s]\n",expr_bytes,expr);
    x = wtk_owl_expr_new(heap, expr, expr_bytes);
    //wtk_owl_expr_print(x);
    item = wtk_owl_tree_find_item(owl, x->k, x->nk);
    //wtk_debug("item=%p\n",item);
    if (item) {
        v = wtk_owl_item_get_str_value(item);
    }
    wtk_heap_delete(heap);
    return v;
}

void wtk_owl_item_print_path2(wtk_owl_item_t *item, wtk_strbuf_t *buf)
{
    //wtk_owl_item_attr_kv_t *kv;

    if (item->parent_item) {
        wtk_owl_item_print_path2(item->parent_item, buf);
    }
    if (!item->use_k_inst && item->k.str) {
        if (item->parent_item) {
//			kv=wtk_owl_item_find_attr_s(item->parent_item,"select");
//			if(kv)
//			{
//				return;
//			}
            wtk_strbuf_push_s(buf, ".");
        }
        wtk_strbuf_push(buf, item->k.str->data, item->k.str->len);
    }
}

void wtk_owl_item_print_path(wtk_owl_item_t *item, wtk_strbuf_t *buf)
{
    wtk_strbuf_reset(buf);
    wtk_owl_item_print_path2(item, buf);
}

wtk_owl_item_t* wtk_owl_item_find_path(wtk_owl_item_t *citem, char *item,
        int item_len)
{
    char *s, *e;
    int n;
    wtk_string_t x;
    wtk_owl_item_t *pv = NULL;
    wtk_owl_item_t *ci;

    s = item;
    e = s + item_len;
    x.data = NULL;
    x.len = 0;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        if (x.data == NULL) {
            x.data = s;
        } else if (*s == '.') {
            x.len = s - x.data;
            ci = wtk_owl_item_find_item2(citem, x.data, x.len);
            if (!ci) {
                wtk_debug("[%.*s] not found.\n", x.len, x.data);
                goto end;
            }
            citem = ci;
            x.data = NULL;
        }
        s += n;
    }
    if (x.data) {
        x.len = s - x.data;
        //wtk_debug("[%.*s]\n",x.len,x.data);
        ci = wtk_owl_item_find_item2(citem, x.data, x.len);
        if (!ci) {
            wtk_debug("[%.*s] not found.\n", x.len, x.data);
            goto end;
        }
        pv = ci;
    }
    end:
    //exit(0);
    return pv;
}

