#include <ctype.h>
#include <math.h>
#include "wtk_lmrec.h"
void wtk_lmrec_update(wtk_lmrec_t *r);
void wtk_lmrec_add_next_lex_tok(wtk_lmrec_t *r, wtk_lmrec_tok_t *tok,
        wtk_ngram_node_t *n, double bow);

wtk_lmrec_t* wtk_lmrec_new(wtk_lmrec_cfg_t *cfg)
{
    wtk_lmrec_t *lr;

    lr = (wtk_lmrec_t*) wtk_malloc(sizeof(wtk_lmrec_t));
    lr->cfg = cfg;
    lr->heap = wtk_heap_new(4096);
    lr->tokpool = wtk_vpool_new(sizeof(wtk_lmrec_tok_t), 1024);
    lr->res = NULL;
    lr->json = wtk_json_new();
    lr->buf = wtk_strbuf_new(256, 1);
    wtk_lmrec_reset(lr);
    return lr;
}

void wtk_lmrec_delete(wtk_lmrec_t *r)
{
    wtk_strbuf_delete(r->buf);
    wtk_json_delete(r->json);
    wtk_vpool_delete(r->tokpool);
    wtk_heap_delete(r->heap);
    wtk_free(r);
}

void wtk_lmrec_reset(wtk_lmrec_t *r)
{
    r->action = NULL;
    wtk_json_reset(r->json);
    wtk_vpool_reset(r->tokpool);
    wtk_queue_init(&(r->tok_q));
    wtk_queue_init(&(r->lex_tok_q));
    wtk_heap_reset(r->heap);
    r->ppl = 0;
    r->prob = 0;
    r->best_like = LZERO;
    r->best_tok = NULL;
    r->max_tok = NULL;
}

wtk_array_t* wtk_lmrec_str2id(wtk_lmrec_t* r, char *data, int bytes)
{
    typedef enum {
        WTK_LMREC_INIT, WTK_LMREC_KEY,
    } wtk_lmrec_state_t;
    wtk_heap_t *heap = r->heap;
    wtk_array_t *a;
    char *s, *e;
    int n;
    wtk_string_t k;
    wtk_lmrec_state_t state;
    int idx;

    state = WTK_LMREC_INIT;
    a = wtk_array_new_h(heap, bytes / 2, sizeof(int));
    s = data;
    e = s + bytes;
    wtk_string_set(&(k), 0, 0);
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        //wtk_debug("[%.*s]\n",n,s);
        switch (state) {
            case WTK_LMREC_INIT:
                if (n > 1 || !isspace(*s)) {
                    k.data = s;
                    state = WTK_LMREC_KEY;
                }
                break;
            case WTK_LMREC_KEY:
                if (((n == 1) && isspace(*s))) {
                    k.len = s - k.data;
                    state = WTK_LMREC_INIT;
                } else if (s + n >= e) {
                    k.len = e - k.data;
                    state = WTK_LMREC_INIT;
                }
                if (state == WTK_LMREC_INIT) {
                    idx = wtk_ngram_get_idx(r->res->ngram, k.data, k.len);
                    //wtk_debug("[%.*s]:%d\n",k.len,k.data,idx);
                    wtk_array_push2(a, &(idx));
                }
                break;
        }
        s += n;
    }
    return a;
}

double wtk_lmrec_prob2ppl(double prob, int nwrd)
{
    return pow(pow(10.0, prob), -1.0 / nwrd);
}

double wtk_lmrec_process(wtk_lmrec_t *r, char *data, int bytes)
{
    wtk_array_t *a;
    int *pi; //,*pe;
    int i, n, cnt;
    wtk_ngram_node_t *node;
    int *tp;
    int len;
    double bow, prob, tot;

    //wtk_debug("[%.*s]\n",bytes,data);
    a = wtk_lmrec_str2id(r, data, bytes);
    pi = (int*) a->slot;
    n = a->nslot;
    if (n >= 2) {
        r->nwrd = n - 2;
    } else {
        r->nwrd = 0;
    }
    //pe=pi+n;
    tot = 0;
    for (i = 2, tp = pi; i <= n; ++i) {
        len = (int) (i - (tp - pi));
        /*
         wtk_debug("len=%d\n",len);
         {
         wtk_string_t *v;
         int j;

         for(j=0;j<len;++j)
         {
         v=wtk_stridx_get_str(r->ngram->idxs,tp[j]);
         wtk_debug("v[%d]=[%.*s]\n",j,v->len,v->data);
         }
         }*/
        node = wtk_ngram_prob(r->res->ngram, tp, len, &cnt);
        //wtk_ngram_print_node(r->ngram,node);
        //wtk_debug("cnt=%d\n",cnt);
        if (cnt != len) {
            bow = node ? node->bow : 0;
            //wtk_ngram_print_node(r->ngram,node);
            while ((tp - pi < i) && (cnt != len)) {
                ++tp;
                len = (int) (i - (tp - pi));
                node = wtk_ngram_prob(r->res->ngram, tp, len, &cnt);
                if (!node) {
                    //prob=-HUGE_VAL;
                    break;
                }
                if (cnt == len) {
                    prob = bow + node->prob;
                    tot += prob;
                    break;
                }
                bow += node->bow;
            }
        } else {
            prob = node->prob;
            tot += prob;
            //wtk_debug("prob=%f/%f\n",prob,tot);
            if (cnt == r->res->ngram->order) {
                ++tp;
            }
        }
        /*
         wtk_debug("v[%d/%.*s]=%f\n",i,
         r->ngram->idxs->items[pi[i-1]]->str->len,r->ngram->idxs->items[pi[i-1]]->str->data,
         prob);
         */
    }
    r->prob = tot;
    if (a->nslot > 2) {
        r->ppl = wtk_lmrec_prob2ppl(tot, a->nslot - 2);
    }
    if (r->cfg->debug) {
        printf("logprob=%f ppl=%f sent= %.*s\n", tot, r->ppl, bytes, data);
    }
    return tot;
}

//------------------------- decoding --------------------------------
wtk_lmrec_align_t* wtk_lmrec_pop_align(wtk_lmrec_t *r, wtk_string_t *var,
        wtk_ngram_node_t *node)
{
    wtk_lmrec_align_t *a;

    a = (wtk_lmrec_align_t*) wtk_heap_malloc(r->heap,
            sizeof(wtk_lmrec_align_t));
    a->str = var;
    a->node = node;
    a->prev = NULL;
    a->prob = 0;
    return a;
}

wtk_lmrec_align_t* wtk_lmrec_add_align(wtk_lmrec_t *r, wtk_lmrec_tok_t *tok,
        wtk_ngram_node_t *node, double prob)
{
    wtk_lmrec_align_t *a;

    a = wtk_lmrec_pop_align(r, NULL, node);
    a->prob = prob;		//node->prob;
    a->prev = tok->align;
    a->str = NULL;
    tok->align = a;
    return a;
}

wtk_lmrec_tok_t* wtk_lmrec_dup_tok(wtk_lmrec_t *r, wtk_lmrec_tok_t *src)
{
    wtk_lmrec_tok_t *dst;

    dst = (wtk_lmrec_tok_t*) wtk_vpool_pop(r->tokpool);
    *dst = *src;
    return dst;
}

void wtk_lmrec_push_tok(wtk_lmrec_t *r, wtk_lmrec_tok_t *tok)
{
    if (tok->rec) {
        wtk_lexpool_push_rec(r->res->lexpool, tok->rec);
        tok->rec = NULL;
        tok->item = NULL;
    }
    wtk_vpool_push(r->tokpool, tok);
}

wtk_lmrec_tok_t* wtk_lmrec_new_tok(wtk_lmrec_t *r, wtk_ngram_node_t *n)
{
    wtk_lmrec_tok_t *tok = NULL;
    wtk_string_t *v;
    wtk_lexpool_item_t *item;

    if (n->type == WTK_NGRAM_NODE_LEXVAR) {
        exit(0);
        v = wtk_stridx_get_str(r->res->ngram->idxs, n->wrd_idx);
        if (!v) {
            goto end;
        }
        item = wtk_lexpool_get_item(r->res->lexpool, v->data + 1, v->len - 2);
        if (!item) {
            //wtk_debug("[%.*s] lex not found\n",v->len,v->data);
            goto end;
        }
        tok = (wtk_lmrec_tok_t*) wtk_vpool_pop(r->tokpool);
        tok->node = n;
        tok->item = item;
        tok->rec = wtk_lexpool_pop_rec(r->res->lexpool);
        wtk_lexr_start(tok->rec->rec, item->net);
    } else {
        tok = (wtk_lmrec_tok_t*) wtk_vpool_pop(r->tokpool);
        tok->node = n;
        tok->rec = NULL;
        tok->item = NULL;
    }
    tok->align = NULL;
    tok->prob = 0;
    tok->pen = 0;
    wtk_lmrec_add_align(r, tok, n, n->prob);
    end: return tok;
}

wtk_lmrec_tok_t* wtk_lmrec_add_tok(wtk_lmrec_t *r, wtk_ngram_node_t *n)
{
    wtk_lmrec_tok_t *tok;
    //wtk_queue_node_t *qn2;
    //wtk_ngram_node_t *p;

    tok = wtk_lmrec_new_tok(r, n);
    if (tok) {
        wtk_queue_push(&(r->tok_q), &(tok->q_n));
    }
    return tok;
}

void wtk_lmrec_start(wtk_lmrec_t *r, wtk_lmres_t *res)
{
    r->res = res;
    wtk_lmrec_add_tok(r, r->res->ngram->start);
}

void wtk_lmrec_step_tok(wtk_lmrec_t *r, wtk_lmrec_tok_t *tok,
        wtk_ngram_node_t *nxt, float prob)
{
    wtk_lmrec_add_align(r, tok, nxt, prob);
    tok->node = nxt;
    tok->prob += prob;
}

int wtk_lmrec_has_tok(wtk_lmrec_t *r, wtk_queue_t *q, wtk_lmrec_tok_t *tok,
        wtk_ngram_node_t *nxt)
{
    wtk_queue_node_t *qn;
    wtk_lmrec_tok_t *t;
    int b = 0;

    wtk_debug("================ check ==================\n");
    wtk_lmrec_print_align(r, tok->align);
    for (qn = q->pop; qn; qn = qn->next) {
        t = data_offset(qn, wtk_lmrec_tok_t, q_n);
        wtk_lmrec_print_align(r, t->align);
        if (t->align->node == nxt && t->align->prev == tok->align) {
            wtk_debug("found dup\n");
            exit(0);
        }
    }
    return b;
}

void wtk_lmrec_feed_lm_tok(wtk_lmrec_t *r, wtk_string_t *v)
{
    wtk_queue_node_t *qn;
    wtk_ngram_node_t *child;
    wtk_lmrec_tok_t *tok;
    wtk_lmrec_tok_t *dup_tok;
    wtk_ngram_t *ngram = r->res->ngram;
    int idx;
    wtk_queue_t q;
    double prob, bow;
    wtk_ngram_prob_t pb;

    idx = wtk_stridx_get_id(ngram->idxs, v->data, v->len, 0);
    if (idx < 0) {
        idx = ngram->idx_unk;
    }
    //wtk_debug("[%.*s]\n",v->len,v->data);
    wtk_queue_init(&(q));
    while (1) {
        qn = wtk_queue_pop(&(r->tok_q));
        if (!qn) {
            break;
        }
        tok = data_offset(qn, wtk_lmrec_tok_t, q_n);
        //wtk_lmrec_print_align(r,tok->align);

        //add uni-gram tok;
        child = wtk_ngram_root_get_node(ngram->root, idx);
        if (child) {
            //wtk_lmrec_has_tok(r,&(q),tok,child);
            dup_tok = wtk_lmrec_dup_tok(r, tok);
            //prob=tok->node->bow+child->prob;
            if (tok->node) {
                prob = wtk_ngram_uni_bow(ngram, tok->node);
            } else {
                prob = 0;
            }
            prob += child->prob;
            //wtk_debug("bow=%f child=%f prob=%f\n",tok->node->bow,child->prob,prob);
            wtk_lmrec_step_tok(r, dup_tok, child, prob);
            if (dup_tok->prob > r->best_like) {
                r->best_like = dup_tok->prob;
                r->max_tok = dup_tok;
            }
            wtk_queue_push(&(q), &(dup_tok->q_n));
        } else {
            //attach root lex;
            wtk_lmrec_add_next_lex_tok(r, tok, NULL, 0);
        }

        //do next lex var match;
        if (tok->node && tok->node->lex_var_q.pop) {
            wtk_lmrec_add_next_lex_tok(r, tok, tok->node, 0);
        }
        bow = 0;
        //do child match
        if (tok->node) {
            child = wtk_ngram_node_get_child_array(tok->node, idx);
            //wtk_debug("child=%p ngram=%d\n",child,tok->node->ngram);
            if (!child) {
                if (tok->node->ngram > 2) {
                    pb = wtk_ngram_next_node(ngram, tok->node, idx);
                    //wtk_debug("ngram=%d\n",pb.node->ngram);
                    if (pb.node->ngram > 1) {
                        child = pb.node;
                        bow = pb.bow;
                    }
                    pb = wtk_ngram_next_node(ngram, tok->node, -1);
                    //wtk_ngram_print_node(ngram,pb.node);
                    //trace back to parent;
                    if (pb.node && pb.node->lex_var_q.pop) {
                        wtk_lmrec_add_next_lex_tok(r, tok, pb.node, pb.bow);
                    }
                }
            }
        } else {
            child = NULL;
            //child=wtk_ngram_root_get_node(ngram->root,idx);
        }
        if (child) {
            //found child;
            //wtk_ngram_print_node(ngram,child);
            //wtk_lmrec_has_tok(r,&(q),tok,child);
            prob = child->prob + bow;
            wtk_lmrec_step_tok(r, tok, child, prob);
            if (tok->prob > r->best_like) {
                r->best_like = tok->prob;
                r->max_tok = tok;
            }
            wtk_queue_push(&(q), &(tok->q_n));
        } else {
            //do oov;
            //wtk_lmrec_has_tok(r,&(q),tok,child);
            wtk_lmrec_add_align(r, tok, NULL, r->cfg->oov_pen);
            tok->node = NULL;
            tok->prob += r->cfg->oov_pen;
            tok->pen += r->cfg->oov_pen;
            if (tok->prob > r->best_like) {
                r->best_like = tok->prob;
                r->max_tok = tok;
            }
            wtk_queue_push(&(q), &(tok->q_n));
        }
    }
    r->tok_q = q;
#ifdef DEBUG_LM_TOK
    if(r->nwrd>=5)
    {
        wtk_debug("tok=%d lex=%d\n",r->tok_q.length,r->lex_tok_q.length);
        exit(0);
    }
#endif
}

void wtk_lmrec_add_next_lex_tok(wtk_lmrec_t *r, wtk_lmrec_tok_t *tok,
        wtk_ngram_node_t *n, double bow)
{
    wtk_ngram_node_t *child;
    wtk_queue2_t *q;
    wtk_queue_node_t *qn;
    wtk_lmrec_tok_t *dup_tok;
    wtk_string_t *v;
    wtk_ngram_t *ngram = r->res->ngram;
    wtk_lexpool_t *lexpool = r->res->lexpool;
    wtk_lexpool_item_t *item;

    q = n ? &(n->lex_var_q) : &(r->res->ngram->root->lex_var_q);
    for (qn = q->pop; qn; qn = qn->next) {
        child = data_offset(qn, wtk_ngram_node_t, lex_var_n);
        v = wtk_stridx_get_str(ngram->idxs, child->wrd_idx);
        if (!v) {
            continue;
        }
        //wtk_debug("%.*s\n",v->len,v->data);
        item = wtk_lexpool_get_item(lexpool, v->data + 1, v->len - 2);
        if (!item) {
            continue;
        }
        dup_tok = wtk_lmrec_dup_tok(r, tok);
        dup_tok->node = child;
        dup_tok->item = item;
        dup_tok->rec = wtk_lexpool_pop_rec(lexpool);
        dup_tok->prob += bow;
        wtk_lexr_start(dup_tok->rec->rec, item->net);
        wtk_queue_push(&(r->lex_tok_q), &(dup_tok->q_n));
    }
}

void wtk_lmrec_feed_lex_tok(wtk_lmrec_t *r, wtk_string_t *str)
{
    wtk_queue_node_t *qn;
    wtk_lmrec_tok_t *tok, *dup_tok;
    wtk_queue_t q;
    wtk_string_t *v;
    wtk_lmrec_align_t *a;
    int ret;
    double prob;
    wtk_lexr_output_item_t *item;

    //wtk_debug("feed lex=%d\n",r->lex_tok_q.length);
    wtk_queue_init(&(q));
    while (1) {
        qn = wtk_queue_pop(&(r->lex_tok_q));
        if (!qn) {
            break;
        }
        tok = data_offset(qn, wtk_lmrec_tok_t, q_n);
        ret = wtk_lexr_feed_str(tok->rec->rec, str);
#ifdef DEBUG_LEX_TOK
        wtk_debug("[%.*s]=[%.*s] ret=%d\n",tok->item->cfg->slot->len,tok->item->cfg->slot->data,
                str->len,str->data,ret);
#endif
        if (ret == 0) {
            while (tok->rec->rec->output_tok_q.length > 0) {
                item = NULL;
                item = wtk_lexr_pop_output(tok->rec->rec);
                //wtk_json_item_print3(item->ji);
                if (item->match_wrd_cnt == tok->rec->rec->wrd_pos) {
                    v = wtk_lexpool_item_get_json_value(tok->item, item->ji);
                    //wtk_debug("v[%.*s]\n",v->len,v->data);
                    if (r->cfg->debug) {
                        wtk_debug("[%.*s]=[%.*s]\n", tok->item->cfg->slot->len,
                                tok->item->cfg->slot->data, v->len, v->data);
                    }
                    if (v) {
                        v = wtk_heap_dup_string(r->heap, v->data, v->len);
                        dup_tok = wtk_lmrec_dup_tok(r, tok);
                        prob = dup_tok->node->prob + r->cfg->lex_pen;
                        a = wtk_lmrec_add_align(r, dup_tok, dup_tok->node,
                                prob);
                        a->str = v;
                        dup_tok->prob += prob;
                        dup_tok->pen += r->cfg->lex_pen;
                        /*
                         if(dup_tok->prob>r->best_like)
                         {
                         r->best_like=dup_tok->prob;
                         }*/
                        dup_tok->rec = NULL;
                        dup_tok->item = NULL;
                        wtk_queue_push(&(r->tok_q), &(dup_tok->q_n));
                    }
                }
            }
            wtk_queue_push(&(q), &(tok->q_n));
        } else {
            wtk_lmrec_push_tok(r, tok);
        }
    }
    r->lex_tok_q = q;
}

void wtk_lmrec_prune_tok(wtk_lmrec_t *r)
{
    wtk_queue_node_t *qn, *qn2;
    wtk_lmrec_tok_t *tok;
    double thresh;

    thresh = r->best_like - r->cfg->beam;
    //wtk_debug("thresh=%f\n",thresh);
    for (qn = r->tok_q.pop; qn; qn = qn2) {
        qn2 = qn->next;
        tok = data_offset(qn, wtk_lmrec_tok_t, q_n);
        //wtk_lmrec_print_align(r,tok->align);
        //wtk_debug("v[%d]=%f/%f\n",r->nwrd,thresh,tok->prob);
        if (tok->prob < thresh) {
            wtk_queue_remove(&(r->tok_q), &(tok->q_n));
            wtk_lmrec_push_tok(r, tok);
        }
    }
}

void wtk_lmrec_feed_tok(wtk_lmrec_t *r, wtk_string_t *v)
{
    //wtk_debug("[%.*s]\n",v->len,v->data);
    r->max_tok = NULL;
    r->best_like = LZERO;
    if (r->tok_q.length > 0) {
        wtk_lmrec_feed_lm_tok(r, v);
    }
    if (r->lex_tok_q.length > 0) {
        wtk_lmrec_feed_lex_tok(r, v);
    }
    if (r->cfg->beam > 0 && r->best_like > LZERO) {
        wtk_lmrec_prune_tok(r);
    }

#ifdef DEBUG_TOK
    if(r->nwrd>=3)
    {
        wtk_debug("tok=%d lex=%d\n",r->tok_q.length,r->lex_tok_q.length);
        exit(0);
    }
#endif

}

int wtk_lmrec_feed(wtk_lmrec_t *r, char *data, int bytes)
{
    wtk_string_t k;
    wtk_string_t *v;
    char *s, *e;
    int n;
    //int ret;
    int esc = 0;

    wtk_string_set(&(k), 0, 0);
    s = data;
    e = s + bytes;
    r->nwrd = 0;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        v = NULL;
        if (n == 1 && (*s == '[' || (esc && *s == ']'))) {
            if (esc) {
                k.len = s - k.data + 1;
                v = wtk_heap_dup_string(r->heap, k.data, k.len);
                esc = 0;
            } else {
                k.data = s;
                esc = 1;
            }
        } else if (!esc) {
            //wtk_string_set(&(v),s,n);
            if (n == 1 && isspace(*s)) {
                v = NULL;
            } else {
                v = wtk_heap_dup_string(r->heap, s, n);
            }
        }
        if (v) {
            ++r->nwrd;
            if (r->cfg->debug) {
                wtk_debug("v[%d][%.*s]: id=%d ntok=%d lextok=%d\n", r->nwrd,
                        v->len, v->data,
                        wtk_stridx_get_id(r->res->ngram->idxs, v->data, v->len,
                                0), r->tok_q.length, r->lex_tok_q.length);
            }
            wtk_lmrec_feed_tok(r, v);
            if (r->tok_q.length == 0 && r->lex_tok_q.length == 0) {
                goto end;
            }
        }
        s += n;
    }
    v = &(r->res->ngram->cfg->snte);
    wtk_lmrec_feed_tok(r, v);
    //ret=0;
    end: wtk_lmrec_update(r);
    //wtk_lmrec_print_all(r);
    return 0;
}

void wtk_lmrec_update(wtk_lmrec_t *r)
{
    wtk_queue_node_t *qn;
    wtk_lmrec_tok_t *tok;
    wtk_lmrec_tok_t *t = NULL;
    for (qn = r->tok_q.pop; qn; qn = qn->next) {
        tok = data_offset(qn, wtk_lmrec_tok_t, q_n);
        //wtk_debug("prob=%f eidx=%d tok=%p\n",tok->prob,tok->node->wrd_idx,tok);
        //r->prob=tok->prob;
        //wtk_lmrec_print_align(r,tok->align);
        if (!t || tok->prob > t->prob) {
            t = tok;
        }
    }
    if (t) {
        r->prob = t->prob;
        r->best_tok = t;
    } else {
        r->best_tok = NULL;
    }
}

int wtk_lmrec_tok_tostring(wtk_lmrec_t *r, wtk_lmrec_tok_t *tok,
        wtk_strbuf_t *buf)
{
    return 0;
}

int wtk_lmrec_tostring(wtk_lmrec_t *r, wtk_strbuf_t *buf)
{
    int ret = -1;

    wtk_strbuf_reset(buf);
    if (!r->best_tok) {
        goto end;
    }
    ret = wtk_lmrec_tok_tostring(r, r->best_tok, buf);
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    end: return ret;
}

wtk_lmrec_tok_t* wtk_lmrec_get_best_tok(wtk_lmrec_t *r)
{
    return r->best_tok;
}

void wtk_lmrec_print_align2(wtk_lmrec_t *r, wtk_lmrec_align_t *a,
        wtk_strbuf_t *buf)
{
    wtk_string_t *v;

    if (a->prev) {
        wtk_lmrec_print_align2(r, a->prev, buf);
    }
    if (buf->pos) {
        //wtk_strbuf_push_c(buf,' ');
    }
    if (a->node) {
        v = wtk_stridx_get_str(r->res->ngram->idxs, a->node->wrd_idx);
        wtk_strbuf_push(buf, v->data, v->len);
    } else {
        wtk_strbuf_push_s(buf, "OOV");
    }
    if (a->str) {
        wtk_strbuf_push_s(buf, "(");
        wtk_strbuf_push(buf, a->str->data, a->str->len);
        wtk_strbuf_push_s(buf, ")");
    }
    wtk_strbuf_push_f(buf, "[f=%f,n=%d p=%p prob=%f]\n", a->prob,
            a->node ? a->node->ngram : 0, a,
            a->node ? a->node->prob : -10000.0);
}

void wtk_lmrec_print_align(wtk_lmrec_t *r, wtk_lmrec_align_t *a)
{
    wtk_strbuf_t *buf;

    buf = wtk_strbuf_new(256, 1);
    wtk_lmrec_print_align2(r, a, buf);
    printf("---------- align[%p] ---------\n%.*s\n", a, buf->pos, buf->data);
    wtk_strbuf_delete(buf);
}

void wtk_lmrec_print_all(wtk_lmrec_t *r)
{
    wtk_queue_node_t *qn;
    wtk_lmrec_tok_t *tok;

    wtk_debug("================== lmrec[nbest=%d] ======================\n",
            r->tok_q.length);
    for (qn = r->tok_q.pop; qn; qn = qn->next) {
        tok = data_offset(qn, wtk_lmrec_tok_t, q_n);
        wtk_debug("prob=%f logprob=%f tok=%p\n", tok->prob,
                tok->prob - tok->pen, tok);
        wtk_lmrec_print_align(r, tok->align);
        //wtk_debug("%f/%f\n",tok->prob,t?t->prob:-1);
    }
    wtk_lmrec_print(r);
}

void wtk_lmrec_align_to_act2(wtk_lmrec_t *r, wtk_lmrec_align_t *a,
        wtk_strbuf_t *buf)
{
    if (a->prev) {
        wtk_lmrec_align_to_act2(r, a->prev, buf);
    }
    if (a->str) {
        wtk_strbuf_push(buf, a->str->data, a->str->len);
    }
}

void wtk_lmrec_align_to_act(wtk_lmrec_t *r, wtk_lmrec_align_t *a,
        wtk_strbuf_t *buf)
{
    wtk_strbuf_reset(buf);
    wtk_strbuf_push_s(buf, "request(");
    wtk_lmrec_align_to_act2(r, a, buf);
    wtk_strbuf_push_c(buf, ')');
}

void wtk_lmrec_print(wtk_lmrec_t *r)
{
    wtk_debug("============== best=%p =============\n", r->best_tok);
    if (r->best_tok) {
        wtk_debug("prob=%f lmprob=%f\n", r->best_tok->prob,
                r->best_tok->prob - r->best_tok->pen);
        wtk_lmrec_print_align(r, r->best_tok->align);
        /*
         {
         wtk_strbuf_t *buf;

         buf=wtk_strbuf_new(256,1);
         wtk_lmrec_align_to_act(r,r->best_tok->align,buf);
         wtk_debug("[%.*s]\n",buf->pos,buf->data);
         wtk_strbuf_delete(buf);
         }*/
    }
}

//------------------- lmlex to actor ---------------------------

int wtk_lmrec_add_slot(wtk_lmrec_t *l, char *data, int bytes, int cls_idx)
{
    wtk_string_t *k;
    wtk_string_t t;
    int ret = -1;
    wtk_json_item_t *ji;

    k = wtk_stridx_get_str(l->res->ngram->idxs, cls_idx);
    if (!k) {
        goto end;
    }
    //[到达城市]
    t.data = k->data + 1;
    t.len = k->len - 2;
    //wtk_debug("[%.*s]=[%.*s]\n",t.len,t.data,bytes,data);
    ji = wtk_json_new_object(l->json);
    wtk_json_obj_add_str2_s(l->json, ji, "_v", data, bytes);
    wtk_json_obj_add_item2(l->json, l->request, t.data, t.len, ji);
    //wtk_json_item_print3(l->action);
    //exit(0);
    //wtk_debug("[%.*s]=[%.*s]\n",t.len,t.data,item->cfg->slot->len,item->cfg->slot->data);
    //wtk_act_print(act);
    //wtk_act_dup_item(tgt_act,act,item->cfg->slot,&t);
    //wtk_act_print(tgt_act);
    ret = 0;
    end:
    //exit(0);
    return ret;
}

int wtk_lmrec_update_tok_align(wtk_lmrec_t *r, wtk_lmrec_align_t *a)
{
    int ret;

    if (a->prev) {
        ret = wtk_lmrec_update_tok_align(r, a->prev);
        if (ret != 0) {
            goto end;
        }
    }
    if (a->str) {
        //wtk_debug("[%.*s]\n",a->str->len,a->str->data);
        ret = wtk_lmrec_add_slot(r, a->str->data, a->str->len,
                a->node->wrd_idx);
        //wtk_debug("ret=%d\n",ret);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lmrec_update_tok(wtk_lmrec_t *r, wtk_lmrec_tok_t *tok)
{
    wtk_lmrec_align_t *a;
    int ret;

    a = tok->align;
    ret = wtk_lmrec_update_tok_align(r, a);
    return ret;
}

void wtk_lmrec_init_json(wtk_lmrec_t *r, char *data, int bytes)
{
    wtk_json_t *json = r->json;
    wtk_json_item_t *list;

    r->action = wtk_json_new_object(json);
    list = wtk_json_new_array(json);
    wtk_json_array_add_str_s(json, list, "request");
    wtk_json_obj_add_item2_s(json, r->action, "action", list);
    wtk_json_obj_add_str2_s(json, r->action, "input", data, bytes);
    r->request = wtk_json_new_object(json);
    wtk_json_obj_add_item2_s(json, r->action, "request", r->request);
    //wtk_json_item_print3(r->action);
}

wtk_string_t wtk_lmrec_process2(wtk_lmrec_t *rec, wtk_lmres_t *res,
        wtk_heap_t *heap, char *data, int bytes)
{
    wtk_lmrec_tok_t *tok;
    wtk_string_t v;
    int ret;

    wtk_debug("[%d/%d]\n", res->lexpool->rec_hoard.use_length,
            res->lexpool->rec_hoard.cur_free);
    wtk_string_set(&(v), 0, 0);
    wtk_lmrec_start(rec, res);
    wtk_lmrec_init_json(rec, data, bytes);
    ret = wtk_lmrec_feed(rec, data, bytes);
    if (rec->cfg->debug_all) {
        wtk_lmrec_print_all(rec);
    }
    if (rec->cfg->debug_best) {
        wtk_lmrec_print(rec);
    }
    if (ret != 0) {
        goto end;
    }
    tok = wtk_lmrec_get_best_tok(rec);
    if (!tok) {
        ret = -1;
        goto end;
    }
    ret = wtk_lmrec_update_tok(rec, tok);
    if (ret != 0) {
        goto end;
    }
    //wtk_json_obj_add_ref_number_s(rec->json,rec->action,"prob",tok->prob-tok->pen);
    wtk_json_item_print(rec->action, rec->buf);
    wtk_string_set(&(v), rec->buf->data, rec->buf->pos);
    ///act->prob=tok->prob-tok->pen;
    ret = 0;
    end: wtk_lmrec_reset(rec);
    return v;
}
