#include <stdlib.h>
#include <ctype.h>
#include "wtk/core/wtk_sort.h"
#include "wtk_ngram.h"

void wtk_ngram_node_init(wtk_ngram_node_t *node, int wrd_idx)
{
    node->wrd_idx = wrd_idx;
    node->n = 0;
    node->child.list = NULL;
    node->state = WTK_NGRAM_NODE_INIT;
    node->prob = 0;
    node->bow = 0;
    node->next = NULL;
    node->parent = NULL;
    node->type = WTK_NGRAM_NODE_STR;
    wtk_queue2_init(&(node->lex_var_q));
}

wtk_ngram_root_t* wtk_ngram_root_new(int n)
{
    wtk_ngram_root_t *r;
    int i;

    r = (wtk_ngram_root_t*) wtk_malloc(sizeof(wtk_ngram_root_t));
    r->n = n;
    r->nodes = (wtk_ngram_node_t*) wtk_calloc(n, sizeof(wtk_ngram_node_t));
    wtk_queue2_init(&(r->lex_var_q));
    for (i = 0; i < n; ++i) {
        wtk_ngram_node_init(r->nodes + i, -1);
    }
    return r;
}

void wtk_ngram_root_delete(wtk_ngram_root_t *r)
{
    wtk_free(r->nodes);
    wtk_free(r);
}

wtk_ngram_t* wtk_ngram_new(wtk_ngram_cfg_t *cfg)
{
    wtk_ngram_t *n;
    int ret;

    n = (wtk_ngram_t*) wtk_malloc(sizeof(wtk_ngram_t));
    n->cfg = cfg;
    n->order = 0;
    n->idxs = 0;
    n->heap = wtk_heap_new(4096);
    n->root = NULL;
    if (cfg->fn) {
        ret = wtk_ngram_load_file(n, cfg->fn);
        if (ret != 0) {
            wtk_debug("load %s failed.\n", cfg->fn);
            wtk_ngram_delete(n);
            n = NULL;
        }
    }
    return n;
}

void wtk_ngram_delete(wtk_ngram_t *n)
{
    if (n->idxs) {
        wtk_stridx_delete(n->idxs);
    }
    if (n->root) {
        wtk_ngram_root_delete(n->root);
    }
    wtk_heap_delete(n->heap);
    wtk_free(n);
}

void wtk_ngram_update_wrds(wtk_ngram_t *n, int nwrd)
{
    nwrd += 4; //<s> </s> <unk> -pau-
    n->idxs = wtk_stridx_new(nwrd);
    n->idx_unk = wtk_stridx_get_id(n->idxs, n->cfg->unk.data, n->cfg->unk.len,
            1);
    n->idx_snts = wtk_stridx_get_id(n->idxs, n->cfg->snts.data,
            n->cfg->snts.len, 1);
    n->idx_snte = wtk_stridx_get_id(n->idxs, n->cfg->snte.data,
            n->cfg->snte.len, 1);
    n->idx_pau = wtk_stridx_get_id(n->idxs, n->cfg->pau.data, n->cfg->pau.len,
            1);
    n->root = wtk_ngram_root_new(nwrd);
}

wtk_ngram_node_t* wtk_ngram_new_node(wtk_ngram_t *n, int wrd_idx)
{
    wtk_ngram_node_t* node;

    node = (wtk_ngram_node_t*) wtk_heap_malloc(n->heap,
            sizeof(wtk_ngram_node_t));
    wtk_ngram_node_init(node, wrd_idx);
    return node;
}

int wtk_ngram_node_depth(wtk_ngram_node_t *n)
{
    int depth;

    depth = 1;
    while (n->parent) {
        ++depth;
        n = n->parent;
    }
    return depth;
}

wtk_ngram_pnode_t wtk_ngram_node_parent(wtk_ngram_node_t *n)
{
    wtk_ngram_pnode_t p;

    p.depth = 1;
    while (n->parent) {
        ++p.depth;
        n = n->parent;
    }
    p.root = n;
    return p;
}

wtk_ngram_node_t* wtk_ngram_node_parent2(wtk_ngram_node_t *n)
{
    while (n->parent) {
        n = n->parent;
    }
    return n;
}

void wtk_ngram_print_parent(wtk_ngram_t *n, wtk_ngram_node_t *node)
{
    wtk_string_t *v;
    int i, d;

    if (node->parent) {
        wtk_ngram_print_parent(n, node->parent);
    }

    //wtk_debug("============= node=%p ==========\n",node);
    v = wtk_stridx_get_str(n->idxs, node->wrd_idx);
    d = wtk_ngram_node_depth(node);
    for (i = 0; i < d; ++i) {
        printf("\t");
    }
    //printf("%f %.*s[%d,p=%p parent=%p] %f\n",node->prob,v->len,v->data,node->wrd_idx,node,node->parent,node->bow);
    printf("%f %.*s %f [node=%p depth=%d]\n", node->prob, v->len, v->data,
            node->bow, node, d);

}

void wtk_ngram_print_node(wtk_ngram_t *n, wtk_ngram_node_t *node)
{
    wtk_ngram_node_t *p;
    wtk_string_t *v;
    int i, d;

    //wtk_debug("============= node=%p ==========\n",node);
    v = wtk_stridx_get_str(n->idxs, node->wrd_idx);
    d = wtk_ngram_node_depth(node);
    for (i = 0; i < d; ++i) {
        printf("\t");
    }
    //printf("%f %.*s[%d,p=%p parent=%p] %f\n",node->prob,v->len,v->data,node->wrd_idx,node,node->parent,node->bow);
    printf("%f %.*s %f [node=%p depth=%d id=%d ngram=%d]\n", node->prob, v->len,
            v->data, node->bow, node, d, node->wrd_idx, node->ngram);
    switch (node->state) {
        case WTK_NGRAM_NODE_INIT:
            p = node->child.list;
            while (p) {
                wtk_ngram_print_node(n, p);
                p = p->next;
            }
            break;
        case WTK_NGRAM_NODE_SORT:
            for (i = 0; i < node->n; ++i) {
                wtk_ngram_print_node(n, node->child.array[i]);
            }
            break;
    }
}

wtk_ngram_node_t* wtk_ngram_root_get_node(wtk_ngram_root_t* r, int idx)
{
    wtk_ngram_node_t *node;

    if (idx < 0 || idx >= r->n) {
        return NULL;
    }
    node = r->nodes + idx;
    if (node->wrd_idx == -1) {
        return NULL;
    }
    return node;
}

wtk_ngram_node_t* wtk_ngram_node_get_child_list(wtk_ngram_node_t *node, int id)
{
    wtk_ngram_node_t *p;

    p = node->child.list;
    while (p) {
        if (p->wrd_idx == id) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

int wtk_ngram_node_path_id(wtk_ngram_node_t *node, int *ids)
{
    int cnt;

    if (node->parent) {
        cnt = wtk_ngram_node_path_id(node->parent, ids);
    } else {
        cnt = 0;
    }
    ids[cnt] = node->wrd_idx;
    return cnt + 1;
}

double wtk_ngram_uni_bow(wtk_ngram_t *ngram, wtk_ngram_node_t *node)
{
    wtk_ngram_node_t *t;
    double prob;
    int wids[100];
    int i, cnt;

    prob = node->bow;
    if (node->parent) {
        cnt = wtk_ngram_node_path_id(node, wids);
        //wtk_debug("cnt=%d prob=%f\n",cnt,prob);
        for (i = 1; i < cnt; ++i) {
            t = wtk_ngram_prob(ngram, &wids[i], cnt - i, NULL);
            prob += t->bow;
            //wtk_debug("i=%d bow=%f/%f\n",i,t->bow,prob);
        }
    }
    return prob;
}

int wtk_ngram_node_cmp2(int *id, wtk_ngram_node_t **node)
{
    //wtk_debug("node=%p\n",*node);
    //exit(0);
    /*
     wtk_debug("%p: [%d]=%d prob=%f bow=%f pid=%d\n",node,*id,
     (*node)->wrd_idx,(*node)->prob,(*node)->bow,
     (*node)->parent->wrd_idx);
     */
    return *id - (*node)->wrd_idx;
}

wtk_ngram_node_t* wtk_ngram_node_get_child_array(wtk_ngram_node_t *node, int id)
{
    wtk_ngram_node_t** p;

    //wtk_debug("id=%d\n",id);
    if (node->n == 0) {
        return NULL;
    }
    if (node->n == 1) {
        node = node->child.array[0];
        return node->wrd_idx == id ? node : NULL;
    } else {
        //wtk_debug("%p:%d end=%p n=%d\n",node->child.array,node->n,node->child.array+node->n,node->n);
        p = wtk_binary_search(node->child.array,
                (void*) ((char*) (node->child.array)
                        + sizeof(wtk_ngram_node_t*) * (node->n - 1)),
                sizeof(wtk_ngram_node_t*),
                (wtk_search_cmp_f) wtk_ngram_node_cmp2, &id);
    }
    return p ? *p : NULL;
}

int wtk_ngram_node_cmp(const void *p1, const void *p2)
{
    wtk_ngram_node_t **n1 = (wtk_ngram_node_t**) p1;
    wtk_ngram_node_t **n2 = (wtk_ngram_node_t**) p2;

    return (*n1)->wrd_idx - (*n2)->wrd_idx;
}

void wtk_ngram_node_sort_child(wtk_ngram_t *n, wtk_ngram_node_t *node)
{
    wtk_ngram_node_t *p;
    int i;

    p = node->child.list;
    //wtk_debug("========= p=%p n=%d =========\n",p,node->n);
    node->child.array = (wtk_ngram_node_t**) wtk_heap_zalloc(n->heap,
            sizeof(wtk_ngram_node_t*) * node->n);
    i = 0;
    while (p) {
        //wtk_ngram_print_node(n,p);
        node->child.array[i++] = p;
        p = p->next;
    }
    qsort(node->child.array, node->n, sizeof(wtk_ngram_node_t*),
            wtk_ngram_node_cmp);
    node->state = WTK_NGRAM_NODE_SORT;
    //wtk_debug("=========================\n");
    //wtk_ngram_print_node(n,node);
}

void wtk_ngram_update_node(wtk_ngram_t *n, wtk_ngram_node_t *node)
{
    wtk_ngram_node_t *p;
    int i;

    if (node->state == WTK_NGRAM_NODE_INIT) {
        wtk_ngram_node_sort_child(n, node);
    }
    for (i = 0; i < node->n; ++i) {
        p = node->child.array[i];
        wtk_ngram_update_node(n, p);
    }
}

void wtk_ngram_update_root(wtk_ngram_t *n)
{
    wtk_ngram_node_t *node;
    int i;

    for (i = 0; i < n->root->n; ++i) {
        node = n->root->nodes + i;
        if (node->wrd_idx != -1) {
            wtk_ngram_update_node(n, node);
        }
    }
}

wtk_ngram_node_t* wtk_ngram_get_node2(wtk_ngram_t *n, wtk_ngram_node_t *node,
        int *wrds)
{
    wtk_ngram_node_t *p;
    wtk_string_t *v;
    int idx;

    idx = wrds[0];
    if (idx == -1) {
        return node;
    }
    switch (node->state) {
        case WTK_NGRAM_NODE_INIT:
            p = wtk_ngram_node_get_child_list(node, idx);
            if (p) {
                return wtk_ngram_get_node2(n, p, wrds + 1);
            } else {
                p = wtk_ngram_new_node(n, idx);
                p->parent = node;
                p->next = node->child.list;
                node->child.list = p;
                ++node->n;
                v = wtk_stridx_get_str(n->idxs, idx);
                if (v && v->len > 0 && v->data[0] == '['
                        && v->data[v->len - 1] == ']') {
                    p->type = WTK_NGRAM_NODE_LEXVAR;
                    wtk_queue2_push(&(node->lex_var_q), &(p->lex_var_n));
                }
                return p;
            }
            break;
        case WTK_NGRAM_NODE_SORT:
            //wtk_ngram_print_node(n,node);
            p = wtk_ngram_node_get_child_array(node, idx);
            if (p) {
                return wtk_ngram_get_node2(n, p, wrds + 1);
            } else {
                wtk_ngram_print_node(n, node);
                wtk_debug("warning: not found[%d]\n", idx);
                return NULL;
            }
            break;
    }
    return NULL;
}

wtk_ngram_node_t* wtk_ngram_get_node(wtk_ngram_t *n, int* wrds)
{
    wtk_ngram_node_t *node;
    wtk_string_t *v;
    int idx;

    idx = wrds[0];
    node = n->root->nodes + idx;
    if (node->wrd_idx == -1) {
        v = wtk_stridx_get_str(n->idxs, idx);
        if (v && v->len > 0 && v->data[0] == '['
                && v->data[v->len - 1] == ']') {
            node->type = WTK_NGRAM_NODE_LEXVAR;
            wtk_queue2_push(&(n->root->lex_var_q), &(node->lex_var_n));
        }
        node->wrd_idx = idx;
        return node;
    }
    return wtk_ngram_get_node2(n, node, wrds + 1);
}

void wtk_ngram_notify(wtk_string_t *strs, char *item, int len, int index)
{
    wtk_string_t *v;

    //wtk_debug("[%.*s]\n",len,item);
    //v=wtk_strpool_find(n->pool,item,len,1);
    v = strs + index;
    wtk_string_set(v, item, len);
}

int wtk_ngram_is_sep(void *p, char c)
{
    return isspace(c);
}

int wtk_ngram_load(wtk_ngram_t *n, wtk_source_t *src)
{
#define MAX_NUM_GRAM 100
    typedef enum {
        WTK_NGRAM_INIT = -1, WTK_NGRAM_HDR = 0,
    } wtk_ngram_state_t;
    wtk_ngram_state_t state;
    wtk_strbuf_t *buf;
    int ret;
    int esc;
    int order, ngrams, eof;
    int num_grams[MAX_NUM_GRAM + 1];
    int i, cnt;	//,j,t;
    wtk_string_t wrds[MAX_NUM_GRAM + 1 + 2];
    int wids[MAX_NUM_GRAM + 1 + 2];
    float prob;
    int max_order = 0;
    wtk_ngram_node_t *node;

    for (i = 0; i <= MAX_NUM_GRAM; ++i) {
        num_grams[i] = 0;
    }
    buf = wtk_strbuf_new(256, 1);
    state = WTK_NGRAM_INIT;
    while (1) {
        ret = wtk_source_skip_sp(src, NULL);
        if (ret != 0) {
            goto end;
        }
        ret = wtk_source_read_line2(src, buf, &eof);
        if (ret != 0) {
            goto end;
        }
        if (eof && buf->pos == 0) {
            ret = 0;
            goto end;
        }
        wtk_strbuf_strip(buf);
        if (buf->pos == 0) {
            continue;
        }
        //wtk_debug("[%.*s]\n",buf->pos,buf->data);
        if (buf->data[0] == '\\') {
            esc = 1;
        } else {
            esc = 0;
        }
        wtk_strbuf_push_c(buf, 0);
        --buf->pos;
        switch (state) {
            case WTK_NGRAM_INIT:
                if (esc && wtk_str_equal_s(buf->data, buf->pos, "\\data\\")) {
                    state = WTK_NGRAM_HDR;
                }
                break;
            case WTK_NGRAM_HDR:
                if (esc && sscanf(buf->data, "\\%d-grams", &state) == 1) {
                    if (state == 1) {
                        n->order = max_order;
                        wtk_ngram_update_wrds(n, num_grams[1]);
                    }
                    if (state < 1) {
                        ret = -1;
                        goto end;
                    }
                    continue;
                } else if (sscanf(buf->data, "ngram %d=%d", &order, &ngrams)
                        == 2) {
                    num_grams[order] = ngrams;
                    //wtk_debug("order=%d/%d\n",order,ngrams);
                    if (order > max_order) {
                        max_order = order;
                    }
                }
                break;
            default:
                //wtk_debug("%s\n",buf->data);
                if (esc && sscanf(buf->data, "\\%d-grams", &state) == 1) {
                    continue;
                } else if (esc
                        && wtk_str_equal_s(buf->data, buf->pos, "\\end\\")) {
                    wtk_ngram_update_root(n);
                } else {
                    cnt = wtk_str_split2(buf->data, buf->pos, wrds,
                            (wtk_str_split_f) wtk_ngram_notify,
                            wtk_ngram_is_sep);
                    //wtk_debug("[%.*s]=%d\n",buf->pos,buf->data,cnt);
                    if (cnt <= 0) {
                        goto end;
                    }
                    prob = wtk_str_atof(wrds[0].data, wrds[0].len);
                    //wtk_debug("prob=%f\n",prob);
                    //wtk_debug("state=%d\n",state);
                    for (i = 1; i < state + 1; ++i) {
                        wids[i - 1] = wtk_stridx_get_id(n->idxs, wrds[i].data,
                                wrds[i].len, 1);//wtk_ngram_get_index(n,wrds[i].data,wrds[i].len);
                        //wtk_debug("v[%.*s]=%d\n",wrds[i].len,wrds[i].data,wids[i-1]);
                    }
                    wids[state] = -1;
                    /*
                     for(i=0,j=state-1;i<j;++i,--j)
                     {
                     t=wids[i];
                     wids[i]=wids[j];
                     wids[j]=t;
                     }*/
                    /*
                     if(state==2)
                     {
                     for(i=0;i<state+1;++i)
                     {
                     wtk_debug("%d\n",wids[i]);
                     }
                     }*/
                    node = wtk_ngram_get_node(n, wids);
                    node->ngram = state;
                    node->prob = prob;
                    if (cnt == state + 2) {
                        node->bow = wtk_str_atof(wrds[cnt - 1].data,
                                wrds[cnt - 1].len);
                    }
                    //wtk_ngram_print_node(n,node);
                    //exit(0);
                }
                break;
        }
    }
    ret = 0;
    end: n->start = wtk_ngram_root_get_node(n->root, n->idx_snts);
    wtk_strbuf_delete(buf);
    return 0;
}

int wtk_ngram_load_file(wtk_ngram_t *n, char *fn)
{
    return wtk_source_load_file(n, (wtk_source_load_handler_t) wtk_ngram_load,
            fn);
}

void wtk_ngram_print(wtk_ngram_t *n)
{
    wtk_ngram_node_t *node;
    int i;

    for (i = 0; i < n->root->n; ++i) {
        node = n->root->nodes + i;
        if (node->wrd_idx != -1) {
            wtk_ngram_print_node(n, node);
        }
    }
}

int wtk_ngram_get_idx(wtk_ngram_t *n, char *data, int bytes)
{
    int idx;

    idx = wtk_stridx_get_id(n->idxs, data, bytes, 0);
    if (idx < 0) {
        idx = n->idx_unk;
    }
    return idx;
}

void wtk_ngram_print_id(wtk_ngram_t *n, int id)
{
    wtk_debug("idx=%d/%.*s\n", id, n->idxs->items[id]->str->len,
            n->idxs->items[id]->str->data);
}

wtk_ngram_node_t* wtk_ngram_prob(wtk_ngram_t *n, int *idx, int len, int *cnt)
{
    wtk_ngram_node_t *node, *p;
    int i;
    int nx;

    //if(len<=0){return NULL;}
    nx = 0;
    //wtk_debug("[%d/%d]=[%.*s]\n",idx[0],len,n->idxs->items[idx[0]]->str->len,n->idxs->items[idx[0]]->str->data);
    //wtk_ngram_print_id(n,idx[0]);
    node = wtk_ngram_root_get_node(n->root, idx[0]);
    //wtk_debug("[%d/%d]=%f\n",node->wrd_idx,idx[0],node->prob);
    if (node) {
        ++nx;
        for (i = 1; i < len; ++i) {
            //wtk_ngram_print_id(n,idx[i]);
            p = wtk_ngram_node_get_child_array(node, idx[i]);
            if (p) {
                ++nx;
                node = p;
                //wtk_debug("[%d/%d]=%f\n",node->wrd_idx,idx[i],node->prob);
            } else {
                //wtk_debug("prob=%f bow=%f\n",node->prob,node->bow);
                break;
            }
        }
    }
    if (cnt) {
        *cnt = nx;
    }
    return node;
}

wtk_ngram_prob_t wtk_ngram_prob2(wtk_ngram_t *n, int *pi, int len)
{
    wtk_ngram_prob_t prob;
    wtk_ngram_node_t *node;
    int cnt;
    int *tp;

    prob.node = NULL;
    prob.bow = 0;
    node = wtk_ngram_prob(n, pi, len, &cnt);
    /*
     {
     int i;

     for(i=0;i<cnt;++i)
     {
     wtk_debug("v[%d]=%d\n",i,pi[i]);
     }
     wtk_debug("cnt=%d/%d\n",cnt,len);
     }*/
    if (cnt != len) {
        prob.bow = node ? node->bow : 0;
        //wtk_ngram_print_node(r->ngram,node);
        tp = pi;
        while (len > 0 && (cnt != len)) {
            ++tp;
            --len;
            /*
             {
             int j;
             for(j=0;j<len;++j)
             {
             wtk_ngram_print_id(n,tp[j]);
             }
             }*/
            node = wtk_ngram_prob(n, tp, len, &cnt);
            //wtk_debug("cnt=%d/%d %p\n",cnt,len,node);
            if (!node) {
                //prob=-HUGE_VAL;
                break;
            }
            if (cnt == len) {
                prob.node = node;
                //wtk_ngram_print_node(n,node);
                //wtk_debug("prob=%f\n",node->prob);
                break;
            }
            prob.bow += node->bow;
        }
    } else {
        prob.node = node;
    }
    return prob;
}

wtk_ngram_prob_t wtk_ngram_next_node(wtk_ngram_t *ngram, wtk_ngram_node_t *node,
        int idx)
{
    wtk_ngram_prob_t pb;
    int wids[100];
    int *pi;
    int cnt;
    int add;

    cnt = wtk_ngram_node_path_id(node, wids);
    if (idx > 0) {
        wids[cnt] = idx;
        ++cnt;
    }
    /*
     {
     int i;

     for(i=0;i<cnt;++i)
     {
     wtk_debug("v[%d/%d]=%d\n",i,cnt,wids[i]);
     }
     }*/
    if (idx >= 0) {
        add = cnt - ngram->order;
    } else {
        add = 1;
    }
    if (add > 0) {
        cnt -= add;
        pi = wids + add;
        //wtk_debug("cnt=%d/%d\n",cnt,add);
    } else {
        pi = wids;
    }
    pb = wtk_ngram_prob2(ngram, pi, cnt);
    return pb;
}
