#include "wtk_lexr.h" 
#include <ctype.h>
typedef enum {
    WTK_LEX_TOK_MATCH_AND_KEEP_POS,
    WTK_LEX_TOK_MATCH_AND_CAN_STEP,
    WTK_LEX_TOK_MATCH_AND_MUST_STEP,
    WTK_LEX_TOK_NOT_MATCH,
} wtk_lex_tok_match_no_t;

wtk_lex_tok_match_no_t wtk_lex_expr_repeat_match(wtk_lex_expr_repeat_t *repeat,
        int cnt);
void wtk_lexr_align_print3(wtk_lexr_align_t *a);

wtk_lexr_t* wtk_lexr_new(wtk_lexr_cfg_t *cfg, wtk_rbin2_t *rbin)
{
    wtk_lexr_t *r;

    r = (wtk_lexr_t*) wtk_malloc(sizeof(wtk_lexr_t));
    r->cfg = cfg;
    r->heap = wtk_heap_new(4096);
    r->rec_heap = wtk_heap_new(4096);
    r->buf = wtk_strbuf_new(256, 1);
    r->json = wtk_json_new();
    r->rec_json = wtk_json_new();
    r->get_var_f = NULL;
    r->get_var_ths = NULL;
    if (cfg->pron_fn) {
        if (rbin) {
            r->pron_kv = wtk_fkv_new4(rbin, cfg->pron_fn, 17003);
        } else {
            r->pron_kv = wtk_fkv_new3(cfg->pron_fn);
        }
        //wtk_debug("%s:%p\n",cfg->pron_fn,r->pron_kv);
    } else {
        r->pron_kv = NULL;
    }
    if (cfg->use_share_lib) {
        r->lib = NULL;
    } else {
        r->lib = wtk_lexr_lib_new(&(cfg->lib), rbin);
    }
    if (cfg->use_wrdvec) {
        if (cfg->use_share_wrdvec) {
            r->wrdvec = NULL;
        } else {
            r->wrdvec = wtk_wrdvec_new(&(cfg->wrdvec), NULL);
        }
    } else {
        r->wrdvec = NULL;
    }
    if (cfg->use_poseg) {
        r->poseg = wtk_poseg_new(&(cfg->poseg));
    } else {
        r->poseg = NULL;
    }
    r->hmmnr = wtk_hmmnr_new();
    wtk_lexr_reset(r);
    return r;
}

void wtk_lexr_set_wrdvec(wtk_lexr_t *r, wtk_wrdvec_t *wvec)
{
    r->wrdvec = wvec;
}

void wtk_lexr_delete(wtk_lexr_t *r)
{
    if (r->pron_kv) {
        wtk_fkv_delete(r->pron_kv);
    }
    if (r->hmmnr) {
        wtk_hmmnr_delete(r->hmmnr);
    }
    if (r->wrdvec && !r->cfg->use_share_wrdvec) {
        wtk_wrdvec_delete(r->wrdvec);
    }
    if (r->poseg) {
        wtk_poseg_delete(r->poseg);
    }
    if (!r->cfg->use_share_lib) {
        wtk_lexr_lib_delete(r->lib);
    }
    wtk_json_delete(r->rec_json);
    wtk_json_delete(r->json);
    wtk_strbuf_delete(r->buf);
    wtk_heap_delete(r->rec_heap);
    wtk_heap_delete(r->heap);
    wtk_free(r);
}

void wtk_lexr_reset(wtk_lexr_t *r)
{
    //wtk_debug("reset lex\n");
    r->has_var_f = NULL;
    r->has_var_ths = NULL;
    r->action = NULL;
    r->action_list = NULL;
    r->action_attr = NULL;
    r->wrds = NULL;
    r->wrd_pos = 0;
    wtk_json_reset(r->rec_json);
    wtk_json_reset(r->json);
    wtk_heap_reset(r->rec_heap);
    wtk_heap_reset(r->heap);
    wtk_queue_init(&(r->tok_q));
    wtk_queue_init(&(r->pend_q));
    wtk_queue_init(&(r->output_tok_q));
    wtk_queue_init(&(r->output_q));
    r->output_trans.match_wrd_cnt = 0;
    r->output_trans.v = NULL;
    if (r->poseg) {
        wtk_poseg_reset(r->poseg);
    }
}

void wtk_lexr_add_word(wtk_lexr_t *r, char *s, int n)
{
    int i;
    wtk_string_t **strs, *v;
    wtk_heap_t *heap = r->heap;

    if (r->cfg->filter) {
        strs = (wtk_string_t**) (r->cfg->filter->slot);
    } else {
        strs = NULL;
    }
    if (strs) {
        for (i = 0; i < r->cfg->filter->nslot; ++i) {
            //wtk_debug("v[%d]=[%.*s]\n",i,r->cfg->filter[i]->len,r->cfg->filter[i]->data);
            if (wtk_string_cmp(strs[i], s, n) == 0) {
                break;
            }
        }
        //wtk_debug("i=%d\n",i);
        if (i < r->cfg->filter->nslot) {
            return;
        }
    }
    //wtk_debug("[%.*s]=%p\n",n,s,strs);
    v = wtk_heap_dup_string(heap, s, n);
    //wtk_debug("[%.*s]\n",v->len,v->data);
    wtk_array_push2(r->wrds, &(v));
}

void wtk_lexr_update_words(wtk_lexr_t *r, char *data, int bytes,
        int use_eng_wrd)
{
    typedef enum {
        WTK_LEXR_STR_INIT, WTK_LEXR_STR_ENG,
    } wtk_lexr_str_state_t;
    wtk_heap_t *heap = r->heap;
    char *s, *e;
    char *str;
    int n;
    wtk_lexr_str_state_t state;

    r->wrds = wtk_array_new_h(heap, bytes / 2, sizeof(wtk_string_t*));
    s = data;
    e = s + bytes;
    if (use_eng_wrd) {
        str = NULL;
        state = WTK_LEXR_STR_INIT;
        while (s < e) {
            n = wtk_utf8_bytes(*s);
            switch (state) {
                case WTK_LEXR_STR_INIT:
                    if (n == 1 && s + n < e && isalpha(*s)) {
                        str = s;
                        state = WTK_LEXR_STR_ENG;
                    } else {
                        wtk_lexr_add_word(r, s, n);
                    }
                    break;
                case WTK_LEXR_STR_ENG:
                    if (n == 1 && isalpha(*s)) {
                        if (s + n >= e) {
                            wtk_lexr_add_word(r, str, e - str);
                        }
                    } else {
                        wtk_lexr_add_word(r, str, s - str);
                        wtk_lexr_add_word(r, s, n);
                        state = WTK_LEXR_STR_INIT;
                    }
                    break;
            }
            s += n;
        }
    } else {
        while (s < e) {
            n = wtk_utf8_bytes(*s);
            wtk_lexr_add_word(r, s, n);
            s += n;
        }
    }
    //exit(0);
    return;
}

void wtk_lexr_wrds_to_string(wtk_lexr_t *r, wtk_strbuf_t *buf)
{
    wtk_string_t **strs;
    int i;

    wtk_strbuf_reset(buf);
    strs = (wtk_string_t**) (r->wrds->slot);
    for (i = 0; i < r->wrds->nslot; ++i) {
        if (i > 0) {
            wtk_strbuf_push_c(buf, ' ');
        }
        wtk_strbuf_push(buf, strs[i]->data, strs[i]->len);
    }
}

wtk_lexr_align_t* wtk_lexr_new_align(wtk_lexr_t *r, wtk_lex_arc_t *arc)
{
    wtk_lexr_align_t *a;

    a = (wtk_lexr_align_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_align_t));
    a->arc = arc;
    a->v.str = NULL;
    a->wrd_cnt = 0;
    a->type = WTK_LEXR_ALIGN_STR;
    a->prev = NULL;
    //wtk_debug("new a=%p\n",a);
    return a;
}

void wtk_lexr_align_set_str(wtk_lexr_t *r, wtk_lexr_align_t *a, wtk_string_t *v)
{
    wtk_queue_node_t *qn;
    wtk_lex_like_item_t *item, *max_item;
    wtk_string_t *v1, t;
    float f;
    float max_f;

    a->type = WTK_LEXR_ALIGN_STR;
    //wtk_debug("[%.*s] arc=%d/%d\n",v->len,v->data,a->arc->value->attr.repeat.min_count,a->arc->value->attr.repeat.max_count);
//	wtk_lex_expr_item_value_print(a->arc->value);
//	printf("\n");
    if (a->arc && a->arc->value) {
        if (v->len == 0) {
            a->v.str = NULL;
            return;
        }
        if (a->arc->value->attr.replace) {
            wtk_lex_replace_item_t *item;

            for (qn = a->arc->value->attr.replace->pop; qn; qn = qn->next) {
                item = data_offset2(qn, wtk_lex_replace_item_t, q_n);
                if (wtk_string_cmp(v, item->f->data, item->f->len) == 0) {
                    a->v.str = item->t;
                    return;
                }
            }
        }
        if (r->hmmnr && a->arc->value->attr.ner) {
            wtk_lex_ner_item_t *ner_item;
            wtk_string_t *x;

            x = NULL;
            for (qn = a->arc->value->attr.ner->pop; qn; qn = qn->next) {
                ner_item = data_offset2(qn, wtk_lex_ner_item_t, q_n);
                r->hmmnr->ne = ner_item->ner->ne;
                if (ner_item->use_search) {
                    if (ner_item->ner->fkv) {
                        r->hmmnr->fkv = ner_item->ner->fkv;
                        //wtk_hmmnr_set_fkv(r->hmmnr,ner_item->ner->map);
                    }
                    t = wtk_hmmnr_rec2(r->hmmnr, v->data, v->len,
                            ner_item->wrd_pen, ner_item->prune_thresh);
//					wtk_debug("[%.*s] [%.*s]=%f wrd_pen=%f/%f/%f\n",ner_item->ner->name->len,ner_item->ner->name->data,t.len,t.data,r->hmmnr->prob,
//							ner_item->wrd_pen,ner_item->prune_thresh,ner_item->conf_thresh);
                    r->hmmnr->fkv = NULL;
                    if (t.len > 0 && r->hmmnr->prob > ner_item->conf_thresh) {
                        x = wtk_heap_dup_string(r->rec_heap, t.data, t.len);
                        //wtk_debug("[%.*s]=%f\n",x->len,x->data,r->hmmnr->prob);
                        //exit(0);
                        break;
                    }
                } else {
                    if (ner_item->ner->fkv) {
                        r->hmmnr->fkv = ner_item->ner->fkv;
                        //wtk_hmmnr_set_fkv(r->hmmnr,ner_item->ner->map);
                    }
                    f = wtk_hmmnr_process2(r->hmmnr, v->data, v->len,
                            ner_item->wrd_pen);
                    r->hmmnr->fkv = NULL;
                    //wtk_debug("[%.*s]=%f\n",v->len,v->data,f);
                    if (f > ner_item->conf_thresh) {
                        x = v;
                        break;
                    }
                }
            }
            v = x;
        }
        if (r->wrdvec && a->arc->value->attr.like) {
            v1 = NULL;
            max_item = NULL;
            max_f = -1e10;
            for (qn = a->arc->value->attr.like->pop; qn; qn = qn->next) {
                item = data_offset2(qn, wtk_lex_like_item_t, q_n);
                f = wtk_wrdvec_like(r->wrdvec, item->like->data,
                        item->like->len, v->data, v->len);
                if (!max_item || f > max_f) {
                    max_item = item;
                    max_f = f;
                }
            }
            t = wtk_wrdvec_best_like(r->wrdvec, max_item->like->data,
                    max_item->like->len, v->data, v->len, &f);
            //wtk_debug("[%.*s] [%.*s] [%.*s]=%f\n",max_item->like->len,max_item->like->data,v->len,v->data,t.len,t.data,f);
            if (f >= max_item->thresh) {
                v1 = &(t);
            }
            if (v1) {
                if (t.len != v->len) {
                    v = wtk_heap_dup_string(r->rec_heap, t.data, t.len);
                }
            } else {
                a->v.str = NULL;
                return;
            }
        }
    }
    a->v.str = v;
}

void wtk_lexr_align_set_pth(wtk_lexr_align_t *a, wtk_lexr_pth_t *pth)
{
    a->type = WTK_LEXR_ALIGN_PTH;
    a->v.pth = pth;
}

void wtk_lexr_align_set_json(wtk_lexr_align_t *a, wtk_json_item_t *ji)
{
    a->type = WTK_LEXR_ALIGN_JSON;
    a->v.json = ji;
}

void wtk_lexr_align_set_nil(wtk_lexr_align_t *a)
{
    a->type = WTK_LEXR_ALIGN_NIL;
    a->v.json = NULL;
}

wtk_lexr_align_t* wtk_lexr_add_tok_align(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lex_arc_t *arc, wtk_string_t *v)
{
    wtk_lexr_align_t *a;

    a = wtk_lexr_new_align(r, arc);
    //wtk_debug("output=%p\n",arc->value->attr.output);
    if (arc->value->attr.output) {
        v = arc->value->attr.output;
    }
    //wtk_debug("[%.*s]\n",v->len,v->data);
    //a->v.str=v;
    wtk_lexr_align_set_str(r, a, v);
    a->wrd_cnt = 1;
    a->pos = r->wrd_pos;
    a->prev = tok->pth->pth_item->align;
    tok->pth->pth_item->align = a;
    //wtk_debug("pth_item=%p prev=%p\n",tok->pth->pth_item,tok->pth->pth_item->prev);
    //wtk_debug("pth=%p  align=%p prev=%p [%.*s] \n",tok->pth,a,a->prev,v->len,v->data);
    return a;
}

wtk_lexr_align_t* wtk_lexr_add_tok_align2(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lex_arc_t *arc, wtk_lexr_pth_t *pth)
{
    wtk_lexr_align_t *a;

    a = wtk_lexr_new_align(r, arc);
    a->type = WTK_LEXR_ALIGN_PTH;
    a->v.pth = pth;
    a->prev = tok->pth->pth_item->align;
    tok->pth->pth_item->align = a;
    //wtk_debug("pth=%p  align=%p [%.*s]\n",tok->pth,a,v->len,v->data);
    return a;
}

wtk_lexr_align_t* wtk_lexr_add_tok_nil_align(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lex_arc_t *arc)
{
    wtk_lexr_align_t *a;

    a = wtk_lexr_new_align(r, arc);
    wtk_lexr_align_set_nil(a);
    a->prev = tok->pth->pth_item->align;
    tok->pth->pth_item->align = a;
    //wtk_debug("pth=%p  align=%p [%.*s]\n",tok->pth,a,v->len,v->data);
    return a;
}

int wtk_lexr_align_get_index2(wtk_lexr_align_t *a, int index,
        wtk_lexr_align_t **pa)
{
    int idx;

    if (a->prev) {
        idx = wtk_lexr_align_get_index2(a->prev, index, pa);
    } else {
        idx = 0;
    }
    if (!*pa && a->arc->value
            && a->arc->value->type == WTK_LEX_VALUE_PARENTHESES
            && a->arc->value->v.parentheses->capture) {
        ++idx;
        if (idx == index) {
            *pa = a;
        }
    }
    return idx;
}

wtk_lexr_align_t* wtk_lexr_align_get_index(wtk_lexr_align_t *a, int index)
{
    wtk_lexr_align_t *tx;

    tx = NULL;
    wtk_lexr_align_get_index2(a, index, &tx);
    return tx;
}

int wtk_lexr_align_wrd_count(wtk_lexr_align_t *a)
{
    int cnt = 0;
    float f;

#ifdef DEBUG_CNT
    wtk_debug("=============================\n");
#endif
    while (a) {
        if (a->type == WTK_LEXR_ALIGN_STR && !a->v.str) {
            //wtk_debug("return\n");
            //如果wordlike是中间某个align置信度低，将其设置为null,则返回失败;
            return -1;
        }
        //wtk_debug("cnt=%d w=%f\n",a->wrd_cnt,a->arc->value->attr.match_weight);
        if (a->arc && a->arc->value) {
            f = a->arc->value->attr.match_weight;
        } else {
            f = 1.0;
        }
        cnt += a->wrd_cnt * f;
#ifdef DEBUG_CNT
        wtk_debug("a=%p cnt=%d type=%d\n",a,a->wrd_cnt,a->type);
        switch(a->type)
        {
            case WTK_LEXR_ALIGN_STR:
            wtk_debug("%.*s\n",a->v.str->len,a->v.str->data);
            break;
            case WTK_LEXR_ALIGN_JSON:
            break;
            case WTK_LEXR_ALIGN_PTH:
            break;
            case WTK_LEXR_ALIGN_NIL:
            break;
        }
#endif
        a = a->prev;
    }
    return cnt;
}

wtk_string_t* wtk_lexr_post_process(wtk_lexr_t *r, wtk_string_t *v,
        wtk_string_t *post)
{
    char tmp[128];

    if (wtk_string_cmp_s(post,"tonumber") == 0) {
        int t;

        t = wtk_chnstr_atoi2(v->data, v->len);
        sprintf(tmp, "%d", t);
        v = wtk_heap_dup_string(r->json->heap, tmp, strlen(tmp));
        return v;
    } else {
        return v;
    }
}

wtk_json_item_t* wtk_lexr_expr_path_to_json(wtk_lexr_t *r,
        wtk_lex_expr_item_t *item, wtk_lexr_pth_t *pth, wtk_json_t *json)
{
    wtk_lex_expr_output_t *output = &(item->output);
    wtk_json_item_t *ji, *attr, *pi, *vi, *ti;
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_item_t *oi;
    wtk_string_t *k, *v;
    wtk_lexr_align_t *a;
    float prob;

    //wtk_lexr_pth_print(pth);
    ji = wtk_json_new_object(json);
    if (output->name) {
        wtk_json_obj_add_str2_s(json, ji, "action", output->name->data,
                output->name->len);
    }
    prob = item->prob;
    if (output->item_q.length) {
        attr = wtk_json_new_object(json);
        wtk_json_obj_add_item2_s(json, ji, "attr", attr);
        for (qn = output->item_q.pop; qn; qn = qn->next) {
            oi = data_offset2(qn, wtk_lex_expr_output_item_t, q_n);
            k = oi->k;
            v = NULL;
            vi = NULL;
            a = NULL;
            switch (oi->type) {
                case WTK_LEX_OUTPUT_ITEM_NONE:
                    break;
                case WTK_LEX_OUTPUT_ITEM_STR:
                    v = oi->v.str;
//				if(v && oi->post)
//				{
//					wtk_debug("post=%.*s %.*s\n",oi->post->len,oi->post->data,v->len,v->data);
//					exit(0);
//				}
                    wtk_json_obj_add_str2(json, attr, k->data, k->len, v->data,
                            v->len);
                    continue;
                    break;
                case WTK_LEX_OUTPUT_ITEM_VARSTR:
                    a = wtk_lexr_align_get_index(pth->pth_item->align,
                            oi->v.varstr.cap_index);
                    if (a) {
                        if (a->type == WTK_LEXR_ALIGN_STR
                                || a->type == WTK_LEXR_ALIGN_JSON) {
                            if (a->type == WTK_LEXR_ALIGN_STR) {
                                v = a->v.str;
                            } else {
                                v = wtk_json_item_get_str_value(a->v.json);
                            }
                            if (v) {
                                wtk_strbuf_t *buf;

                                buf = wtk_strbuf_new(256, 1);
                                if (oi->v.varstr.pre) {
                                    wtk_strbuf_push(buf, oi->v.varstr.pre->data,
                                            oi->v.varstr.pre->len);
                                }
                                wtk_strbuf_push(buf, v->data, v->len);
                                if (oi->v.varstr.pst) {
                                    wtk_strbuf_push(buf, oi->v.varstr.pst->data,
                                            oi->v.varstr.pst->len);
                                }
                                wtk_json_obj_add_str2(json, attr, k->data,
                                        k->len, buf->data, buf->pos);
                                wtk_strbuf_delete(buf);
                                continue;
                            }
                        }
                    }
                    break;
                case WTK_LEX_OUTPUT_ITEM_VAR:
                    a = wtk_lexr_align_get_index(pth->pth_item->align,
                            oi->v.var.cap_index);
                    if (!a) {
                        v = oi->v.var.def;
                    } else {
                        switch (a->type) {
                            case WTK_LEXR_ALIGN_STR:
                                v = a->v.str;
                                break;
                            case WTK_LEXR_ALIGN_JSON:
                                vi = a->v.json;
                                //wtk_json_item_print3(a->v.json);
//						wtk_json_item_print3(ji);
                                break;
                            case WTK_LEXR_ALIGN_PTH:
                                v = NULL;
                                wtk_debug("found err\n")
                                ;
                                break;
                            case WTK_LEXR_ALIGN_NIL:
                                v = NULL;
                                vi = NULL;
                                break;
                        }
                    }
                    break;
            }
            if (a && a->type == WTK_LEXR_ALIGN_NIL) {
                continue;
            }
            if (v && oi->post) {
                v = wtk_lexr_post_process(r, v, oi->post);
                //wtk_debug("post=%.*s %.*s\n",oi->post->len,oi->post->data,v->len,v->data);
                //exit(0);
            }
            if (!v || !vi) {
                prob -= oi->miss_pen;
            }
            if (vi) {
                if (!k || wtk_string_cmp_s(k,"*") == 0) {
                    if (vi->type == WTK_JSON_OBJECT) {
                        //wtk_json_item_print3(vi);
                        ti = wtk_json_obj_get_s(vi, "attr");
                        if (ti) {
                            wtk_json_copy_obj_dict(json, attr, ti);
                        }
                    }
                } else {
                    //wtk_json_item_print3(vi);
                    ti = wtk_json_obj_get_s(vi, "attr");
                    if (ti) {
                        vi = ti;
                    } else {
                        vi = wtk_json_item_dup(vi, json->heap);
                    }
                    //wtk_json_item_print3(vi);
                    wtk_json_obj_add_item2(json, attr, k->data, k->len, vi);
                    //wtk_json_item_print3(attr);
                    //exit(0);
                }
            } else if (k) {
                if (oi->k && oi->type == WTK_LEX_OUTPUT_ITEM_VAR && !v) {
                    continue;
                }
                if (wtk_string_cmp_s(k,"*") == 0) {
                    k = v;
                    v = NULL;
                }
                if (k) {
                    if (wtk_string_cmp_s(k,"+") == 0) {
                        pi = attr;
                    } else {
                        pi = wtk_json_new_object(json);
                    }
                    if (oi->hook) {
                        wtk_json_obj_add_str2_s(json, pi, "hook",
                                oi->hook->data, oi->hook->len);
                    }
                    if (v) {
                        wtk_json_obj_add_str2_s(json, pi, "_v", v->data,
                                v->len);
                        if (a) {
                            wtk_json_obj_add_ref_number_s(json, pi, "_s",
                                    a->pos);
                            wtk_json_obj_add_ref_number_s(json, pi, "_e",
                                    a->pos + a->wrd_cnt - 1);
                        }
                    }
                    if (pi != attr) {
                        wtk_json_obj_add_item2(json, attr, k->data, k->len, pi);
                    }
                    //wtk_json_item_print3(attr);
                }
            } else if (v) {
                pi = attr;
                if (oi->hook) {
                    wtk_json_obj_add_str2_s(json, pi, "hook", oi->hook->data,
                            oi->hook->len);
                }
                if (v) {
                    wtk_json_obj_add_str2_s(json, pi, "_v", v->data, v->len);
                    if (a) {
                        wtk_json_obj_add_ref_number_s(json, pi, "_s", a->pos);
                        wtk_json_obj_add_ref_number_s(json, pi, "_e",
                                a->pos + a->wrd_cnt - 1);
                    }
                }
            }
        }
    }
    if (prob != 1) {
        wtk_json_obj_add_ref_number_s(json, ji, "prob", prob);
    }
    //wtk_json_item_print3(ji);
    return ji;
}

void wtk_lexr_align_get_tot_pos_info(wtk_lexr_align_t *a, int *cnt, int *pos)
{
    *pos = 0;
    *cnt = 0;
    while (a) {
        *pos = a->pos;
        *cnt += a->wrd_cnt;
        a = a->prev;
    }
}

wtk_lexr_align_t* wtk_lexr_expr_pth_to_align(wtk_lexr_t *r, wtk_lexr_pth_t *pth)
{
    wtk_lexr_align_t *a;
    wtk_json_item_t *ji;
    int pos, cnt;
    wtk_string_t *v;

    a = pth->pth_item->align;
    wtk_lexr_align_get_tot_pos_info(a, &cnt, &pos);
    a = wtk_lexr_new_align(r, pth->arc);
    a->wrd_cnt = cnt;
    a->pos = pos;
    v = wtk_lex_expr_output_get_alias(&(pth->arc->value->v.expr->output));
    //wtk_debug("v=%p\n",v);
    if (!v) {
        ji = wtk_lexr_expr_path_to_json(r, pth->arc->value->v.expr, pth,
                r->rec_json);
        //wtk_debug("======== ji =%p \n",ji);
        //wtk_json_item_print3(ji);
        //wtk_json_item_print3(r->action);
        wtk_lexr_align_set_json(a, ji);
    } else {
        wtk_lexr_align_set_str(r, a, v);
    }
    //wtk_json_item_print3(ji);
    return a;
}

int wtk_lexr_align_has_json(wtk_lexr_align_t *a)
{
    while (a) {
        if (a->type == WTK_LEXR_ALIGN_JSON) {
            return 1;
        }
        a = a->prev;
    }
    return 0;
}

int wtk_lexr_align_valid_slot_cnt(wtk_lexr_align_t *a)
{
    int cnt = 0;

    while (a) {
        switch (a->type) {
            case WTK_LEXR_ALIGN_STR:
                if (a->v.str) {
                    ++cnt;
                }
                break;
            case WTK_LEXR_ALIGN_JSON:
                if (a->v.json) {
                    ++cnt;
                }
                break;
            case WTK_LEXR_ALIGN_PTH:
                if (a->v.pth) {
                    ++cnt;
                }
                break;
            case WTK_LEXR_ALIGN_NIL:
                break;
        }
        a = a->prev;
    }
    return cnt;
}

wtk_lexr_align_t* wtk_lexr_pth_item_get_align(wtk_lexr_pth_item_t *item,
        int index)
{
    int cnt;
    wtk_lexr_align_t *a;

    //wtk_debug("item=%p prev=%p\n",item,item->prev);
    a = item->align;
    cnt = 0;
    while (a) {
        //wtk_debug("type=%d,a=%p\n",a->arc->value->type,a);
        if (a->arc->value && a->arc->value->type == WTK_LEX_VALUE_PARENTHESES
                && a->arc->value->v.parentheses->capture) {
            ++cnt;
        }
        //wtk_debug("[%.*s] %d\n",a->v.str->len,a->v.str->data,a->arc->value->type);
        a = a->prev;
    }
    if (cnt <= 0) {
        return NULL;
    }
    //wtk_debug("cnt=%d,index=%d\n",cnt,index);
    index = cnt - index - 1;
    //wtk_debug("cnt=%d,index=%d\n",cnt,index);
    a = item->align;
    cnt = 0;
    while (a) {
        //wtk_debug("[%.*s]\n",a->v.str->len,a->v.str->data);
        if (a->arc->value && a->arc->value->type == WTK_LEX_VALUE_PARENTHESES
                && a->arc->value->v.parentheses->capture) {
            if (cnt == index) {
                return a;
            }
            ++cnt;
        }
        a = a->prev;
    }
    return NULL;
}

int wtk_lexr_trans_to_str(wtk_lexr_t *r, wtk_lex_expr_output_trans_item_t *item,
        wtk_lexr_pth_t *pth, wtk_strbuf_t *buf)
{
    wtk_lexr_align_t *a;
    wtk_string_t *v;
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_trans_filter_t *filter;
    char *data;
    int len;
    wtk_strbuf_t *tmp1, *tmp2;
    int ret;

    v = NULL;
    switch (item->type) {
        case WTK_LEX_OUTPUT_TRANS_NONE:
            break;
        case WTK_LEX_OUTPUT_TRANS_STR:
            v = item->v.str;
            break;
        case WTK_LEX_OUTPUT_TRANS_VAR:
            //wtk_debug("get[%p] index=%d\n",item,item->v.cap_index-1);
            a = wtk_lexr_pth_item_get_align(pth->pth_item,
                    item->v.cap_index - 1);
            //wtk_debug("get aling=%p\n",a);
            if (a) {
                if (a->type == WTK_LEXR_ALIGN_STR) {
                    v = a->v.str;
                } else {
                    return a->type == WTK_LEXR_ALIGN_NIL ? 0 : -1;
                }
            }
            break;
    }
    if (!v) {
        //wtk_debug("return\n");
        return -1;
    }
    data = v->data;
    len = v->len;
    //wtk_debug("[%.*s]\n",len,data);
    ret = 0;
    if (item->filter_q.length > 0) {
        //wtk_debug("[%.*s]\n",len,data);
        tmp1 = wtk_strbuf_new(256, 1);
        tmp2 = wtk_strbuf_new(256, 1);
        for (qn = item->filter_q.pop; qn; qn = qn->next) {
            filter = data_offset2(qn, wtk_lex_expr_output_trans_filter_t, q_n);
            ret = wtk_lex_expr_output_trans_filter_process(filter, data, len,
                    tmp2);
            if (ret != 0) {
                break;
            }
            wtk_strbuf_reset(tmp1);
            wtk_strbuf_push(tmp1, tmp2->data, tmp2->pos);
            data = tmp1->data;
            len = tmp1->pos;
        }
        //wtk_debug("[%.*s]\n",len,data);
        wtk_strbuf_push_word(buf, data, len);
        wtk_strbuf_delete(tmp1);
        wtk_strbuf_delete(tmp2);
        //exit(0);
    } else {
        wtk_strbuf_push_word(buf, data, len);
    }
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    return ret;
}

void wtk_lexr_align_to_str(wtk_lexr_align_t *a, wtk_strbuf_t *buf)
{
    if (!a) {
        return;
    }
    if (a->prev) {
        wtk_lexr_align_to_str(a->prev, buf);
    }
    if (a->type == WTK_LEXR_ALIGN_STR) {
        if (a->v.str) {
            wtk_strbuf_push(buf, a->v.str->data, a->v.str->len);
        }
    }
}

int wtk_lexr_expr_to_trans(wtk_lexr_t *r, wtk_lex_expr_output_t *output,
        wtk_lexr_pth_t *pth, wtk_strbuf_t *buf)
{
    //wtk_lex_expr_output_t *output;
    wtk_lexr_align_t *a;
    wtk_queue_node_t *qn;
    wtk_lex_expr_output_trans_item_t *item;
    int ret;

    //output=&(pth->arc->value->v.expr->output);
    //wtk_lex_expr_item_print(r->cur_expr);
    //wtk_lexr_pth_print(pth);
    wtk_strbuf_reset(buf);
    //wtk_debug("item_q=%d\n",output->item_q.length);
    if (output->item_q.length > 0) {
        for (qn = output->item_q.pop; qn; qn = qn->next) {
            item = data_offset2(qn, wtk_lex_expr_output_trans_item_t, q_n);
            //wtk_debug("type=%d\n",item->type);
            ret = wtk_lexr_trans_to_str(r, item, pth, buf);
            if (ret != 0) {
                return ret;
            }
        }
    } else {
        a = pth->pth_item->align;
        wtk_lexr_align_to_str(a, buf);
    }
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    //exit(0);
    return 0;
}

wtk_lexr_align_t* wtk_lexr_new_align2(wtk_lexr_t *r, wtk_lex_arc_t *arc,
        char *data, int len, int pos, int wrd_cnt)
{
    wtk_lexr_align_t *a;
    wtk_string_t *v;

    a = wtk_lexr_new_align(r, arc);
    if (wrd_cnt == 0) {
        wtk_lexr_align_set_nil(a);
    } else {
        v = wtk_heap_dup_string(r->heap, data, len);
        wtk_lexr_align_set_str(r, a, v);
        a->wrd_cnt = wrd_cnt;
        a->pos = pos;
    }
    return a;
}

wtk_lexr_align_t* wtk_lexr_align_merge_str(wtk_lexr_t *r, wtk_lexr_align_t *a,
        wtk_lex_arc_t *arc)
{
    wtk_strbuf_t *buf = r->buf;
    int cnt, pos;

    wtk_strbuf_reset(buf);
    wtk_lexr_align_to_str(a, buf);
    wtk_lexr_align_get_tot_pos_info(a, &cnt, &pos);
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    return wtk_lexr_new_align2(r, arc, buf->data, buf->pos, pos, cnt);
}

wtk_lexr_align_t* wtk_lexr_expr_trans_align_merge(wtk_lexr_t *r,
        wtk_lexr_pth_t *pth)
{
    wtk_strbuf_t *buf = r->buf;
    wtk_lexr_align_t *a;
    int pos, cnt;
    int ret;

    wtk_strbuf_reset(buf);
    a = pth->pth_item->align;
    if (pth->arc->value->attr.output) {
        wtk_lexr_align_get_tot_pos_info(a, &cnt, &pos);
        a = wtk_lexr_new_align(r, pth->arc);
        a->v.str = pth->arc->value->attr.output;
        a->wrd_cnt = cnt;
        a->pos = pos;
        return a;
    }
    if (pth->arc->value->type == WTK_LEX_VALUE_EXPR) {
        //wtk_lex_expr_item_print(pth->arc->value->v.expr);
        ret = wtk_lexr_expr_to_trans(r, &(pth->arc->value->v.expr->output), pth,
                buf);
        if (ret != 0) {
            return NULL;
        }
    } else {
        wtk_lexr_align_to_str(a, buf);
    }
    wtk_lexr_align_get_tot_pos_info(a, &cnt, &pos);
    return wtk_lexr_new_align2(r, pth->arc, buf->data, buf->pos, pos, cnt);
}

wtk_lexr_align_t* wtk_lexr_tok_pth_merge(wtk_lexr_t *r, wtk_lexr_pth_t *pth)
{
    wtk_strbuf_t *buf = r->buf;
    wtk_lexr_align_t *a;
    wtk_json_item_t *ji, *vi;
    wtk_json_t *json = r->rec_json;
    int has_json;
    int cnt, pos;

    if (pth->arc->value->attr.output) {
        a = pth->pth_item->align;
        pos = 0;
        cnt = 0;
        while (a) {
            if (a->type == WTK_LEXR_ALIGN_STR) {
                pos = a->pos;
                cnt += a->wrd_cnt;
            }
            a = a->prev;
        }
        a = wtk_lexr_new_align(r, pth->arc);
        a->v.str = pth->arc->value->attr.output;
        a->wrd_cnt = cnt;
        a->pos = pos;
        return a;
    }
    //wtk_debug("[%p]\n",pth->arc->value->attr.output);
    //wtk_lexr_pth_print(pth);
    if (r->input_net->script->use_act) {
        //wtk_json_item_print3(r->action);
        if (pth->arc->value->type == WTK_LEX_VALUE_PARENTHESES
                || (!wtk_lex_expr_output_has_redirect(
                        &(pth->arc->value->v.expr->output)))) {
            wtk_strbuf_reset(buf);
            a = pth->pth_item->align;
            has_json = wtk_lexr_align_has_json(a);
            if (has_json) {
                cnt = wtk_lexr_align_valid_slot_cnt(a);
                //wtk_debug("cnt=%d\n",cnt);
                if (cnt == 1) {
                    ji = NULL;
                    cnt = 0;
                    pos = 0;
                    while (a) {
                        pos = a->pos;
                        cnt += a->wrd_cnt;
                        if (a->type == WTK_LEXR_ALIGN_JSON) {
                            ji = a->v.json;
                            break;
                        }
                        a = a->prev;
                    }
                    //wtk_json_item_print3(ji);
                    a = wtk_lexr_new_align(r, pth->arc);
                    wtk_lexr_align_set_json(a, ji);
                    a->wrd_cnt = cnt;
                    a->pos = pos;
                } else {
                    ji = wtk_json_new_array(json);
                    cnt = 0;
                    pos = 0;
                    while (a) {
                        pos = a->pos;
                        cnt += a->wrd_cnt;
                        //wtk_debug("type=%d\n",a->type);
                        switch (a->type) {
                            case WTK_LEXR_ALIGN_STR:
                                vi = wtk_json_new_string(json, a->v.str->data,
                                        a->v.str->len);
                                wtk_json_array_add_item2(json, ji, vi, 1);
                                break;
                            case WTK_LEXR_ALIGN_JSON:
                                vi = wtk_json_obj_get_s(a->v.json, "attr");
                                if (vi) {
                                    wtk_json_array_add_item2(json, ji, vi, 1);
                                } else {
                                    wtk_json_array_add_item2(json, ji,
                                            a->v.json, 1);
                                }
                                break;
                            case WTK_LEXR_ALIGN_PTH:
                                wtk_debug("nerver be here\n")
                                ;
                                return NULL;
                                break;
                            case WTK_LEXR_ALIGN_NIL:
                                break;
                        }
                        a = a->prev;
                    }
                    a = wtk_lexr_new_align(r, pth->arc);
                    wtk_lexr_align_set_json(a, ji);
                    a->wrd_cnt = cnt;
                    a->pos = pos;
                    //wtk_json_item_print3(ji);
                    //exit(0);
                }
                //wtk_json_item_print3(ji);
                //wtk_debug("cnt=%d cnt=%d a=%p\n",a->wrd_cnt,cnt,a);
            } else {
                cnt = 0;
                while (a) {
                    if (a->type == WTK_LEXR_ALIGN_STR) {
                        pos = a->pos;
                        cnt += a->wrd_cnt;
                        //wtk_debug("[%.*s]=%d/%d\n",a->v.str->len,a->v.str->data,a->wrd_cnt,a->pos);
                        wtk_strbuf_push_front(buf, a->v.str->data,
                                a->v.str->len);
                    }
                    a = a->prev;
                }
                a = wtk_lexr_new_align(r, pth->arc);
                if (cnt == 0) {
                    wtk_lexr_align_set_nil(a);
                } else {
                    //a->v.str=wtk_heap_dup_string(r->heap,buf->data,buf->pos);
                    wtk_lexr_align_set_str(r, a,
                            wtk_heap_dup_string(r->heap, buf->data, buf->pos));
                    a->wrd_cnt = cnt;
                    a->pos = pos;
                    //wtk_debug("%p [%.*s]=%d/%d\n",a,a->v.str->len,a->v.str->data,cnt,a->pos);
                }
                //wtk_debug("a=%p\n",a);
            }
        } else {
            //wtk_lexr_pth_print(pth);
            a = wtk_lexr_expr_pth_to_align(r, pth);
        }
    } else {
        //wtk_lexr_pth_print(pth);
        a = wtk_lexr_expr_trans_align_merge(r, pth);
    }
    //wtk_json_item_print3(r->action);
    return a;
}

wtk_lexr_align_t* wtk_lexr_tok_pth_merge_align_act(wtk_lexr_t *r,
        wtk_lexr_pth_t *pth)
{
    wtk_strbuf_t *buf = r->buf;
    wtk_lexr_align_t *a;
    wtk_json_item_t *ji, *vi;
    wtk_json_t *json = r->rec_json;
    int has_json;
    int cnt, pos;

    wtk_strbuf_reset(buf);
    a = pth->pth_item->align;
    has_json = wtk_lexr_align_has_json(a);
    if (has_json) {
        cnt = wtk_lexr_align_valid_slot_cnt(a);
        //wtk_debug("cnt=%d\n",cnt);
        if (cnt == 1) {
            //如果只有一个json结果,
            ji = NULL;
            cnt = 0;
            pos = 0;
            while (a) {
                pos = a->pos;
                cnt += a->wrd_cnt;
                if (a->type == WTK_LEXR_ALIGN_JSON) {
                    ji = a->v.json;
                    break;
                }
                a = a->prev;
            }
            //wtk_json_item_print3(ji);
            a = wtk_lexr_new_align(r, pth->arc);
            wtk_lexr_align_set_json(a, ji);
            a->wrd_cnt = cnt;
            a->pos = pos;
        } else {
            //如果有个多个json结果;
            ji = wtk_json_new_array(json);
            cnt = 0;
            pos = 0;
            while (a) {
                pos = a->pos;
                cnt += a->wrd_cnt;
                //wtk_debug("type=%d\n",a->type);
                switch (a->type) {
                    case WTK_LEXR_ALIGN_STR:
                        vi = wtk_json_new_string(json, a->v.str->data,
                                a->v.str->len);
                        wtk_json_array_add_item2(json, ji, vi, 1);
                        break;
                    case WTK_LEXR_ALIGN_JSON:
                        vi = wtk_json_obj_get_s(a->v.json, "attr");
                        if (vi) {
                            wtk_json_array_add_item2(json, ji, vi, 1);
                        } else {
                            wtk_json_array_add_item2(json, ji, a->v.json, 1);
                        }
                        break;
                    case WTK_LEXR_ALIGN_PTH:
                        wtk_debug("nerver be here\n")
                        ;
                        return NULL;
                        break;
                    case WTK_LEXR_ALIGN_NIL:
                        break;
                }
                a = a->prev;
            }
            a = wtk_lexr_new_align(r, pth->arc);
            wtk_lexr_align_set_json(a, ji);
            a->wrd_cnt = cnt;
            a->pos = pos;
            //wtk_json_item_print3(ji);
            //exit(0);
        }
        //wtk_json_item_print3(ji);
        //wtk_debug("cnt=%d cnt=%d a=%p\n",a->wrd_cnt,cnt,a);
    } else {
        a = wtk_lexr_align_merge_str(r, pth->pth_item->align, pth->arc);
        if (r->hmmnr && pth->arc->value->attr.ner
                && a->type == WTK_LEXR_ALIGN_STR && !a->v.str) {
            return NULL;
        }
    }
    return a;
}

wtk_lexr_align_t* wtk_lexr_tok_pth_merge_expr_act(wtk_lexr_t *r,
        wtk_lexr_pth_t *pth)
{
    wtk_lexr_align_t *a;

    if (pth->arc->value->type == WTK_LEX_VALUE_PARENTHESES
            || (!wtk_lex_expr_output_has_redirect(
                    &(pth->arc->value->v.expr->output)))) {
        a = wtk_lexr_tok_pth_merge_align_act(r, pth);
    } else {
        //wtk_lexr_pth_print(pth);
        a = wtk_lexr_expr_pth_to_align(r, pth);
    }
    return a;
}

wtk_lexr_align_t* wtk_lexr_tok_pth_merge_expr(wtk_lexr_t *r,
        wtk_lexr_pth_t *pth)
{
    if (r->input_net->script->use_act) {
        return wtk_lexr_tok_pth_merge_expr_act(r, pth);
    } else {
        return wtk_lexr_expr_trans_align_merge(r, pth);
    }
}

wtk_lexr_align_t* wtk_lexr_tok_pth_merge_align(wtk_lexr_t *r,
        wtk_lexr_pth_t *pth)
{
    if (r->input_net->script->use_act) {
        return wtk_lexr_tok_pth_merge_align_act(r, pth);
    } else {
        return wtk_lexr_align_merge_str(r, pth->pth_item->align, pth->arc);
    }
}

void wtk_lexr_tok_push_path(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lexr_align_t *a)
{
    wtk_lexr_pth_t *pth;

    //reback path;
    tok->pth = tok->pth->prev;
    //link prev align
    a->prev = tok->pth->pth_item->align;

    pth = (wtk_lexr_pth_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_pth_t));
    *pth = *(tok->pth);
    pth->pth_item = (wtk_lexr_pth_item_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_pth_item_t));
    *(pth->pth_item) = *(tok->pth->pth_item);
    pth->pth_item->align = a;
    tok->pth = pth;
}

/**
 * 合并当前pth_item结果,退出当前子项path.
 *
 *  你好(\d+) =>
 *      \d+ 为要合并的子项pth;
 */
int wtk_lexr_tok_push_path_stack(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lex_arc_t *arc)
{
    wtk_lexr_align_t *a;

    a = wtk_lexr_tok_pth_merge_align(r, tok->pth);
    if (!a) {
        //exit(0);
        return -1;
    }
    wtk_lexr_tok_push_path(r, tok, a);
    return 0;
}

void wtk_lexr_tok_push_pth_item(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lexr_align_t *a)
{
    wtk_lexr_pth_t *pth;

    pth = (wtk_lexr_pth_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_pth_t));
    *pth = *(tok->pth);
    pth->pth_item = (wtk_lexr_pth_item_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_pth_item_t));
    if (tok->pth->pth_item->prev) {
        *(pth->pth_item) = *(tok->pth->pth_item->prev);
    } else {
        pth->pth_item->prev = NULL;
        pth->pth_item->align = NULL;
    }
    a->prev = pth->pth_item->align;
    pth->pth_item->align = a;
    tok->pth = pth;
}

/**
 *  合并当前子项输出,
 *  你好(1|2|3)/min=10,max=20/
 *  	--- 合并第一次捕获匹配，一个捕获匹配合并成一个align;
 */
int wtk_lexr_tok_push_pth_item_stack(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lex_arc_t *arc)
{
    wtk_lexr_align_t *a;

    a = wtk_lexr_tok_pth_merge_expr(r, tok->pth);
    if (!a) {
        //exit(0);
        return -1;
    }
    if (a->type == WTK_LEXR_ALIGN_STR && r->hmmnr && arc->value->attr.ner
            && !a->v.str) {
        return -1;
    }
    wtk_lexr_tok_push_pth_item(r, tok, a);
    return 0;
}

int wtk_lexr_pth_item_count(wtk_lexr_pth_t *pth)
{
    wtk_lexr_pth_item_t *item;
    int cnt = 0;

    item = pth->pth_item;
    while (item) {
        ++cnt;
        item = item->prev;
    }
    return cnt;
}

int wtk_lexr_pth_align_count(wtk_lexr_pth_t *pth)
{
    wtk_lexr_align_t *a;
    int cnt = 0;

    a = pth->pth_item->align;
    while (a) {
        ++cnt;
        a = a->prev;
    }
    return cnt;
}

int wtk_lexr_pth_align_word_count(wtk_lexr_pth_t *pth)
{
    wtk_lexr_align_t *a;
    int cnt = 0;

    a = pth->pth_item->align;
    while (a) {
        cnt += a->wrd_cnt;
        a = a->prev;
    }
    return cnt;
}

int wtk_lexr_pth_depth(wtk_lexr_pth_t *pth)
{
    int cnt = 0;

    while (pth) {
        ++cnt;
        pth = pth->prev;
    }
    return cnt;
}

void wtk_lexr_add_tok_pth_item(wtk_lexr_t *r, wtk_lexr_tok_t *tok)
{
    wtk_lexr_pth_item_t *item;

    item = (wtk_lexr_pth_item_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_pth_item_t));
    item->prev = tok->pth->pth_item;
    item->align = NULL;
    tok->pth->pth_item = item;
}

void wtk_lexr_add_tok_pth(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_lex_arc_t *arc)
{
    wtk_lexr_pth_t *pth;

    pth = (wtk_lexr_pth_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_pth_t));
    if (!tok->pth) {
        pth->depth = 0;
    } else {
        pth->depth = tok->pth->depth + 1;
    }
    pth->prev = tok->pth;
    pth->arc = arc;
    pth->pth_item = NULL;
    tok->pth = pth;
    wtk_lexr_add_tok_pth_item(r, tok);
    //wtk_debug("pth=%p prev=%p\n",tok->pth,tok->pth->prev);
}

wtk_lexr_tok_t* wtk_lexr_new_tok(wtk_lexr_t *r)
{
    wtk_lexr_tok_t *tok;

    tok = (wtk_lexr_tok_t*) wtk_heap_malloc(r->rec_heap,
            sizeof(wtk_lexr_tok_t));
    tok->arc = NULL;
    tok->pth = NULL;
    tok->match_cnt = 0;
    return tok;
}

wtk_lexr_pth_t *wtk_lexr_dup_pth(wtk_lexr_t *r, wtk_lexr_pth_t *cpy);
wtk_lexr_align_t* wtk_lexr_dup_pth_align(wtk_lexr_t *r, wtk_lexr_align_t *cpy);
wtk_lexr_pth_item_t* wtk_lexr_dup_pth_item(wtk_lexr_t *r,
        wtk_lexr_pth_item_t *cpy);

wtk_lexr_align_t* wtk_lexr_dup_pth_align(wtk_lexr_t *r, wtk_lexr_align_t *cpy)
{
    wtk_heap_t *heap = r->rec_heap;
    wtk_lexr_align_t *a;

    a = (wtk_lexr_align_t*) wtk_heap_malloc(heap, sizeof(wtk_lexr_align_t));
    *a = *cpy;
    if (cpy->prev) {
        a->prev = wtk_lexr_dup_pth_align(r, cpy->prev);
    }
    if (a->type == WTK_LEXR_ALIGN_PTH) {
        a->v.pth = wtk_lexr_dup_pth(r, cpy->v.pth);
    }
    return a;
}

wtk_lexr_pth_item_t* wtk_lexr_dup_pth_item(wtk_lexr_t *r,
        wtk_lexr_pth_item_t *cpy)
{
    wtk_heap_t *heap = r->rec_heap;
    wtk_lexr_pth_item_t *item;

    item = (wtk_lexr_pth_item_t*) wtk_heap_malloc(heap,
            sizeof(wtk_lexr_pth_item_t));
    *item = *cpy;
    if (cpy->prev) {
        item->prev = wtk_lexr_dup_pth_item(r, cpy->prev);
    }
    if (cpy->align) {
        item->align = wtk_lexr_dup_pth_align(r, cpy->align);
    }
    return item;
}

wtk_lexr_pth_t *wtk_lexr_dup_pth(wtk_lexr_t *r, wtk_lexr_pth_t *cpy)
{
    wtk_heap_t *heap = r->rec_heap;
    wtk_lexr_pth_t *pth;

    pth = (wtk_lexr_pth_t*) wtk_heap_malloc(heap, sizeof(wtk_lexr_pth_t));
    *pth = *cpy;
    if (cpy->prev) {
        pth->prev = wtk_lexr_dup_pth(r, cpy->prev);
    }
    if (cpy->pth_item) {
        pth->pth_item = wtk_lexr_dup_pth_item(r, cpy->pth_item);
    }
    return pth;
}

wtk_lexr_tok_t* wtk_lexr_dup_tok(wtk_lexr_t *r, wtk_lexr_tok_t *tok)
{
    wtk_lexr_tok_t *cpy;

    cpy = wtk_lexr_new_tok(r);
    if (tok) {
        *cpy = *tok;
//		cpy->arc=tok->arc;
//		cpy->pth=tok->pth;
//		cpy->match_cnt=tok->match_cnt;
        if (tok->pth) {
            cpy->pth = wtk_lexr_dup_pth(r, tok->pth);
        }
    } else {
        wtk_lexr_add_tok_pth(r, cpy, NULL);
    }
    return cpy;
}

void wtk_lexr_tok_set_arc(wtk_lexr_tok_t *tok, wtk_lex_arc_t *arc)
{
    tok->arc = arc;
    tok->match_cnt = 0;
}

void wtk_lexr_add_output_trans(wtk_lexr_t *r, wtk_lexr_tok_t *tok, int word_cnt)
{
    wtk_strbuf_t *buf = r->buf;
    int ret;

    //wtk_debug("type=%p word_cnt=%d\n",tok,word_cnt);
    wtk_lex_expr_item_print(r->cur_expr);		//pxj 打印匹配中的lex表达式
    if (word_cnt > r->output_trans.match_wrd_cnt) {
        //wtk_lexr_pth_print(tok->pth);
        //wtk_debug("arc=%p prev=%p\n",tok->pth->arc,tok->pth->prev);
        ret = wtk_lexr_expr_to_trans(r, &(r->cur_expr->output), tok->pth, buf);
        if (ret == 0) {
            //wtk_debug("[%.*s]\n",buf->pos,buf->data);
            r->output_trans.v = wtk_heap_dup_string(r->heap, buf->data,
                    buf->pos);
            r->output_trans.match_wrd_cnt = word_cnt;
        }
    }
}

void wtk_lexr_add_output(wtk_lexr_t *r, wtk_lexr_tok_t *tok)
{
    wtk_json_item_t *ji;
    wtk_lexr_output_item_t *item;
    int cnt;

    //wtk_debug("add output\n");
    //exit(0);
    //wtk_json_item_print3(r->action);
    if (r->cur_expr->attr.match_end) {
        //wtk_debug("wrd_pos=%d/%d\n",r->wrd_pos,r->poseg->nwrd);
        if (r->cur_expr->attr.use_seg) {
            //wtk_debug("wrd_pos=%d/%d\n",r->wrd_pos,r->poseg->nwrd);
            if (r->wrd_pos != r->poseg->nwrd) {
                return;
            }
        } else {
            if (r->wrd_pos != r->wrds->nslot) {
                return;
            }
        }
    }
    cnt = wtk_lexr_align_wrd_count(tok->pth->pth_item->align);
    if (cnt <= 0) {
        return;
    }
    if (r->cur_expr->attr.min_wrd_count > 0
            || r->cur_expr->attr.max_wrd_count > 0) {
        if (r->cur_expr->attr.min_wrd_count > 0
                && cnt < r->cur_expr->attr.min_wrd_count) {
            return;
        }
        if (r->cur_expr->attr.max_wrd_count > 0
                && cnt > r->cur_expr->attr.max_wrd_count) {
            return;
        }
    }
    if (r->input_net->script->use_act) {
        //cnt=wtk_lexr_align_wrd_count(tok->pth->pth_item->align);
        //wtk_debug("cnt=%d cnt=%d\n",cnt,tok->pth->pth_item->align->wrd_cnt);

        //#define DEBUG_X
#ifdef DEBUG_X
        wtk_debug("======== input depth=%d prev=%p =========\n",tok->pth->depth,tok->pth->prev);
        wtk_lexr_pth_print(tok->pth);printf("\n");
        if(tok->pth->depth>0)
        {
            exit(0);
        }
#endif
        ji = wtk_lexr_expr_path_to_json(r, r->cur_expr, tok->pth, r->rec_json);
        //wtk_json_item_print3(ji);
#ifdef DEBUG_X
        wtk_json_item_print3(ji);
        //exit(0);
#endif
        if (ji) {
            if (r->cfg->debug) {
                wtk_json_item_print3(ji);
            }
            //wtk_debug("cnt=%d\n",cnt);
            //wtk_json_item_print3(ji);
            ji = wtk_json_item_dup(ji, r->heap);
            item = (wtk_lexr_output_item_t*) wtk_heap_malloc(r->heap,
                    sizeof(wtk_lexr_output_item_t));
            item->ji = ji;
            item->match_wrd_cnt = cnt;
            r->match_wrds = cnt;
            //wtk_debug("cnt=%d\n",cnt);
            wtk_queue_push(&(r->output_tok_q), &(item->q_n));
        }
    } else {
        wtk_lexr_add_output_trans(r, tok, cnt);
    }
}

/**
 *	//item1 -item2;
 */
int wtk_lexr_cmp_output(wtk_lexr_t *r, wtk_lexr_output_item_t *item1,
        wtk_lexr_output_item_t *item2)
{
    float f1, f2;
    wtk_json_item_t *vi;
    wtk_lex_expr_item_attr_t *attr;

    //wtk_json_item_print3(item1->ji);
    //wtk_json_item_print3(item2->ji);
    //compare probality
    vi = wtk_json_obj_get_s(item1->ji, "prob");
    f1 = vi ? vi->v.number : 1;
    vi = wtk_json_obj_get_s(item2->ji, "prob");
    f2 = vi ? vi->v.number : 1;
    //wtk_debug("f1=%f f2=%f]\n",f1,f2);
    if (f1 != f2) {
        return f1 - f2;
    }
    attr = &(r->cur_expr->attr);
    //compare match var
    f1 = item1->match_wrd_cnt;
    f2 = item2->match_wrd_cnt;
    //wtk_debug("f1=%f f2=%f\n",f1,f2);
    if (f1 != f2) {
        if (attr->match_more_wrd) {
            return f1 - f2;
        } else {
            return f2 - f1;
        }
    }
    vi = wtk_json_obj_get_s(item1->ji, "attr");
    f1 = vi ? vi->v.object->length : 0;
    vi = wtk_json_obj_get_s(item2->ji, "attr");
    f2 = vi ? vi->v.object->length : 0;
    if (f1 != f2) {
        if (attr->match_more_var) {
            return f1 - f2;
        } else {
            return f2 - f1;
        }
    }
    return 0;
}

void wtk_lexr_init_action(wtk_lexr_t *r)
{
    wtk_json_t *json = r->json;

    r->action = wtk_json_new_object(json);
    r->action_list = wtk_json_new_array(json);
    wtk_json_obj_add_item2_s(json, r->action, "action", r->action_list);
    //wtk_lexr_wrds_to_string(r,r->buf);
    //wtk_json_obj_add_str2_s(json,r->action,"input",r->buf->data,r->buf->pos);
    {
        wtk_strbuf_t *buf;

        buf = r->buf;
        wtk_strbuf_reset(buf);
        wtk_strbuf_push_add_escape_str(buf, r->input->data, r->input->len);
        //wtk_json_obj_add_str2_s(json,r->action,"input",r->input->data,r->input->len);
        wtk_json_obj_add_str2_s(json, r->action, "input", r->buf->data,
                r->buf->pos);
    }
}

void wtk_lexr_add_out_item(wtk_lexr_t *r, wtk_lexr_output_item_t *item)
{
    wtk_queue_node_t *qn, *qn1;
    wtk_json_obj_item_t *oi;
    wtk_json_item_t *vi, *v1, *v2, *va, *v3;
    wtk_json_t *json = r->json;
    wtk_string_t *nm;
    wtk_json_array_item_t *ai;
    float prob, f1;
    int b;
    int s1, e1, cnt1;
    int s, e, cnt;
    wtk_queue_node_t *qn2;

//	wtk_debug("============== add =========\n");
    //wtk_json_item_print3(item->ji);
    //exit(0);
    //wtk_json_item_print3(r->action);
    vi = wtk_json_obj_get_s(item->ji, "action");
    if (!vi) {
        return;
    }
    nm = vi->v.str;
    v1 = wtk_json_obj_get(r->action, nm->data, nm->len);
    if (v1) {
        //v1 => request action 属性
        //获取添加属性
        vi = wtk_json_obj_get_s(item->ji, "attr");
        if (vi) {
            v2 = wtk_json_obj_get_s(item->ji, "prob");
            prob = v2 ? v2->v.number : 1;
            //遍历待添加的属性
            for (qn = vi->v.object->pop; qn; qn = qn->next) {
                oi = data_offset2(qn, wtk_json_obj_item_t, q_n);
                //wtk_debug("[%.*s]\n",oi->k.len,oi->k.data);
                //获取总的属性列表
                va = wtk_json_obj_get(v1, oi->k.data, oi->k.len);
                //wtk_debug("va=%p\n",va);
                if (va) {
//					wtk_debug("[%.*s] va=%p\n",oi->k.len,oi->k.data,va);
                    //wtk_json_item_print3(va);
//					wtk_json_item_print3(r->action);
                    b = -1;
                    //遍历，比较已有属性列表
                    for (qn1 = va->v.object->pop; qn1; qn1 = qn1->next) {
                        ai = data_offset2(qn1, wtk_json_array_item_t, q_n);
                        //wtk_json_item_print3(ai->item);
                        //wtk_json_item_print3(oi->item);
                        if (wtk_json_item_cmp(ai->item, oi->item) == 0) {
                            b = -1;
                            break;
                        }
                        v2 = wtk_json_obj_get_s(ai->item, "prob");
                        f1 = v2 ? v2->v.number : 1;
                        if (f1 < prob) {
                            b = 1;
                            break;
                        } else if (f1 == prob) {
                            b = 0;
                            break;
                        } else {
                            b = -1;
                            break;
                        }
                    }
                    //wtk_debug("b=%d\n",b);
                    if (b >= 0) {
                        wtk_string_t *vp1, *vp2;

                        v2 = wtk_json_obj_get_s(oi->item, "_v");
                        vp1 = v2 ? v2->v.str : NULL;
                        v2 = wtk_json_obj_get_s(oi->item, "_s");
                        s1 = v2 ? v2->v.number : -1;
                        v2 = wtk_json_obj_get_s(oi->item, "_e");
                        e1 = v2 ? v2->v.number : -1;
                        if (s1 >= 0 && e1 >= 0) {
                            cnt1 = e1 - s1 + 1;
                            for (qn1 = va->v.object->pop; qn1; qn1 = qn2) {
                                qn2 = qn1->next;
                                ai = data_offset2(qn1, wtk_json_array_item_t,
                                        q_n);
                                v2 = wtk_json_obj_get_s(ai->item, "_s");
                                if (!v2) {
                                    continue;
                                }
                                s = v2->v.number;
                                v2 = wtk_json_obj_get_s(ai->item, "_e");
                                if (!v2) {
                                    continue;
                                }
                                e = v2->v.number;
                                cnt = e - s + 1;
                                //wtk_debug("%d=%d %d=%d\n",s1,e1,s,e);
                                if (cnt > cnt1) {
                                    if (s1 <= e && e1 <= e) {
                                        b = -1;
                                        break;
                                    }
                                } else if (cnt == cnt1) {
                                    if (s1 == s && e1 == e) {
                                        b = -1;
                                        break;
                                    } else if (vp1) {
                                        v2 = wtk_json_obj_get_s(ai->item, "_v");
                                        vp2 = v2 ? v2->v.str : NULL;
                                        if (vp2
                                                && wtk_string_cmp(vp1,
                                                        vp2->data, vp2->len)
                                                        == 0) {
                                            b = -1;
                                            break;
                                        }
                                    }
                                } else {
                                    if (s <= e1 && e <= e1 && s >= s1) {
                                        //wtk_debug("remove %d=%d %d=%d\n",s1,e1,s,e);
                                        wtk_queue_remove(vi->v.array, qn1);
                                    }
                                }
                            }
                        }
                    }
                    //wtk_json_item_print3(r->action);
                    //wtk_debug("b=%d va=%p\n",b,va);
//					wtk_json_item_print3(va);
                    switch (b) {
                        case -1:
                            //skip
                            break;
                        case 0:
                            if (prob != 1
                                    && oi->item->type == WTK_JSON_OBJECT) {
                                wtk_json_obj_add_ref_number_s(json, oi->item,
                                        "prob", prob);
                            }
                            //wtk_json_item_print3(va);
                            wtk_json_array_add_item(json, va, oi->item);
                            //wtk_json_item_print3(va);
                            //wtk_json_item_print3(va);
                            //wtk_json_item_print3(vi);
                            //wtk_json_item_print3(oi->item);
                            //exit(0);
                            break;
                        case 1:
                            //wtk_json_item_print3(va);
                            //wtk_json_item_print3(oi->item);
                            wtk_queue_init((va->v.array));
                            if (prob != 1
                                    && oi->item->type == WTK_JSON_OBJECT) {
                                wtk_json_obj_add_ref_number_s(json, oi->item,
                                        "prob", prob);
                            }
                            //wtk_json_item_print3(va);
                            wtk_json_array_add_item(json, va, oi->item);
                            break;
                    }
                    //wtk_json_item_print3(r->action);
                } else {
                    v2 = wtk_json_new_array(json);
                    v3 = oi->item;
                    //wtk_debug("type=%d\n",v3->type);
                    if (prob != 1 && v3->type == WTK_JSON_OBJECT) {
                        wtk_json_obj_add_ref_number_s(json, v3, "prob", prob);
                    }
                    wtk_json_array_add_item(json, v2, v3);
                    wtk_json_obj_add_item2(json, v1, oi->k.data, oi->k.len, v2);
                }
            }
        }
    } else {
        wtk_json_array_add_str(json, r->action_list, nm->data, nm->len);
        vi = wtk_json_obj_get_s(item->ji, "attr");
        if (vi && vi->type == WTK_JSON_OBJECT) {
            v1 = wtk_json_obj_get_s(item->ji, "prob");
            prob = v1 ? v1->v.number : 1;
            v2 = wtk_json_new_object(json);
            for (qn = vi->v.object->pop; qn; qn = qn->next) {
                oi = data_offset2(qn, wtk_json_obj_item_t, q_n);
                v1 = wtk_json_new_array(json);
                //vi=wtk_json_item_dup(oi->item,json->heap);
                vi = oi->item;
                if (prob != 1 && vi->type == WTK_JSON_OBJECT) {
                    //wtk_json_item_print3(vi);
                    wtk_json_obj_add_ref_number_s(json, vi, "prob", prob);
                }
                wtk_json_array_add_item(json, v1, vi);
                wtk_json_obj_add_item2(json, v2, oi->k.data, oi->k.len, v1);
            }
            wtk_json_obj_add_item2(json, r->action, nm->data, nm->len, v2);
        }
    }
    //wtk_json_item_print3(r->action);
}

void wtk_lexr_clean_expr(wtk_lexr_t *r)
{
    wtk_queue_node_t *qn;
    wtk_lexr_output_item_t *item, *item1;

    if (r->output_tok_q.length <= 0) {
        return;
    }
    //wtk_debug("len=%d\n",r->output_tok_q.length);
    item = 0;
    for (qn = r->output_tok_q.pop; qn; qn = qn->next) {
        item1 = data_offset2(qn, wtk_lexr_output_item_t, q_n);
        if (!item || wtk_lexr_cmp_output(r, item1, item) > 0) {
            item = item1;
        }
    }
    if (item) {
        //add final output;
        wtk_queue_remove(&(r->output_tok_q), &(item->q_n));
        //wtk_json_item_print3(item->ji);
        wtk_lexr_add_out_item(r, item);
    }
    wtk_queue_init(&(r->output_tok_q));
}

wtk_lex_arc_t* wtk_lex_arc_next_capture_arc(wtk_lex_arc_t *arc)
{
    //wtk_lex_expr_item_value_t *v;
    wtk_lex_arc_t *arc2;

    //v=arc->value;
    //wtk_lex_expr_item_value_print(v);printf("\n");
    //wtk_debug("v=%p type=%d\n",v,v->type);
    arc2 = arc;
    while (arc2->to) {
        if (arc2->to->complex_input_arc == arc) {
            return arc2;
        }
        if (arc2->to->out_q.pop) {
            arc2 = data_offset2(arc2->to->out_q.pop, wtk_lex_arc_t, out_n);
        } else {
            break;
        }
    }
    return NULL;
}

void wtk_lexr_add_step_node(wtk_lexr_t *r, wtk_lex_node_t *node,
        wtk_lexr_tok_t *prev_tok);

void wtk_lexr_add_step_node2(wtk_lexr_t *r, wtk_lex_node_t *node,
        wtk_lexr_tok_t *prev_tok)
{
    wtk_queue_node_t *qn;
    wtk_lex_arc_t *arc;
    wtk_lexr_tok_t *tok;
    int ret;

    //wtk_debug("node=%p len=%d\n",node->out_q.pop,wtk_queue2_len(&(node->out_q)));
    if (!node->out_q.pop) {
        //wtk_debug("input arc=%p\n",node->complex_input_arc);
        wtk_lexr_add_output(r, prev_tok);
        return;
    }
    for (qn = node->out_q.pop; qn; qn = qn->next) {
        arc = data_offset2(qn, wtk_lex_arc_t, out_n);
        //wtk_debug("arc=%p\n",arc);
        if (arc->value) {
            if (wtk_lex_expr_item_value_is_atom(arc->value)) {
                tok = wtk_lexr_dup_tok(r, prev_tok);
                wtk_lexr_tok_set_arc(tok, arc);
                //wtk_lex_expr_item_value_print(arc->value);printf("\n");
                wtk_queue_push(&(r->tok_q), &(tok->tok_n));
                if (wtk_lex_expr_item_value_can_be_nil(arc->value)) {
                    //add nil
                    tok = wtk_lexr_dup_tok(r, tok);
                    //a=wtk_lexr_add_tok_align(r,tok,arc,NULL);
                    //wtk_lexr_align_set_nil(a);
                    wtk_lexr_add_tok_nil_align(r, tok, arc);
                    wtk_lexr_add_step_node2(r, arc->to, tok);
                }
            } else {
                tok = wtk_lexr_dup_tok(r, prev_tok);
                //printf("=========>");wtk_lexr_pth_print(tok->pth);printf("\n");
                wtk_lexr_add_tok_pth(r, tok, arc);
                //wtk_debug("pth=%d/%d\n",prev_tok->pth->depth,tok->pth->depth);
                //wtk_debug("pth=%p:%d prev=%p\n",tok->pth,tok->pth->depth,tok->pth->prev);
                wtk_lexr_add_step_node(r, arc->to, tok);
                if (wtk_lex_expr_item_value_can_be_nil(arc->value)) {
                    //tok=wtk_lexr_dup_tok(r,prev_tok);
                    tok = wtk_lexr_dup_tok(r, tok);
                    wtk_lexr_add_tok_nil_align(r, tok, arc);
                    ret = wtk_lexr_tok_push_path_stack(r, tok, arc);
                    if (ret == 0) {
                        arc = wtk_lex_arc_next_capture_arc(arc);
                        wtk_lexr_add_step_node2(r, arc->to, tok);
                    }
                }
            }
        } else {
            wtk_lexr_add_step_node(r, arc->to, prev_tok);
        }
    }
}

void wtk_lexr_add_step_node(wtk_lexr_t *r, wtk_lex_node_t *node,
        wtk_lexr_tok_t *prev_tok)
{
    wtk_lexr_tok_t *tok;
    wtk_lex_tok_match_no_t no;
    //wtk_lexr_align_t *a;
    int i;
    int ret;

    //wtk_debug("step node tok=%p in_arc=%p depth=%d\n",prev_tok,node->complex_input_arc,(prev_tok && prev_tok->pth)?prev_tok->pth->depth:-1);
    if (node->complex_input_arc) {
        //wtk_debug("==================>\n");
        tok = wtk_lexr_dup_tok(r, prev_tok);
        //将当前的子项合并成align
        ret = wtk_lexr_tok_push_pth_item_stack(r, tok, node->complex_input_arc);
        if (ret != 0) {
            return;
        }
        //wtk_lex_expr_item_value_print(node->complex_input_arc->value);
        //printf("\n");
//		wtk_lexr_pth_print(tok->pth);
//		i=wtk_lexr_pth_item_count(tok->pth);
        //wtk_debug("i=%d/%d %d/%d\n",wtk_lexr_pth_item_count(tok->pth),wtk_lexr_pth_align_count(tok->pth),node->complex_input_arc->value->attr.repeat.min_count,node->complex_input_arc->value->attr.repeat.max_count);
        if (node->complex_input_arc->value->attr.min_wrd_count > 0) {
            i = wtk_lexr_pth_align_word_count(tok->pth);
            if (i < node->complex_input_arc->value->attr.min_wrd_count) {
                return;
            }
        }
        //wtk_debug("input i=%d match=%d min=%d max=%d\n",i,tok->match_cnt,node->complex_input_arc->value->attr.min_wrd_count,node->complex_input_arc->value->attr.max_wrd_count);
        i = wtk_lexr_pth_align_count(tok->pth);
        no = wtk_lex_expr_repeat_match(
                &(node->complex_input_arc->value->attr.repeat), i);
        switch (no) {
            case WTK_LEX_TOK_MATCH_AND_KEEP_POS:
                //reback to sub expr
                //添加新的子项进行捕获
                wtk_lexr_add_tok_pth_item(r, tok);
                wtk_lexr_add_step_node(r, node->complex_input_arc->to, tok);
                return;
                break;
            case WTK_LEX_TOK_MATCH_AND_CAN_STEP:
                prev_tok = wtk_lexr_dup_tok(r, tok);
                //reback to sub expr
                wtk_lexr_add_tok_pth_item(r, tok);
                wtk_lexr_add_step_node(r, node->complex_input_arc->to, tok);
                //step to next expr;
                //合并当前的表达式，合并到
                ret = wtk_lexr_tok_push_path_stack(r, prev_tok,
                        node->complex_input_arc);
                if (ret != 0) {
                    return;
                }
                break;
            case WTK_LEX_TOK_MATCH_AND_MUST_STEP:
                //wtk_debug("push back depth=%p:%d\n",prev_tok,tok->pth->depth);
                //wtk_lexr_pth_print(tok->pth);
                ret = wtk_lexr_tok_push_path_stack(r, tok,
                        node->complex_input_arc);
                if (ret != 0) {
                    return;
                }
                //wtk_debug("pth=%d\n",tok->pth->depth);
                //wtk_lexr_pth_print(tok->pth);
                prev_tok = tok;
                break;
            case WTK_LEX_TOK_NOT_MATCH:
                return;
                break;
        }
    }
    //wtk_debug("depth=%d\n",(prev_tok && prev_tok->pth)?prev_tok->pth->depth:-1);
    wtk_lexr_add_step_node2(r, node, prev_tok);
}

void wtk_lexr_prepare(wtk_lexr_t *r, wtk_lex_node_t *node,
        wtk_lex_expr_item_t *expr, int index)
{
    r->match_wrds = 0;
    r->output_trans.match_wrd_cnt = 0;
    r->output_trans.v = NULL;
    wtk_heap_reset(r->rec_heap);
    wtk_json_reset(r->rec_json);
    wtk_queue_init(&(r->tok_q));
    //wtk_queue_init(&(r->output_tok_q));
    r->cur_expr = expr;
    if (expr->attr.match_start && index > 0) {
        return;
    }
    //use_fast_match;
    wtk_lexr_add_step_node(r, node, NULL);
    //wtk_debug("tok=%d\n",r->tok_q.length);
}

wtk_lex_tok_match_no_t wtk_lex_expr_repeat_match(wtk_lex_expr_repeat_t *repeat,
        int cnt)			//wtk_lexr_tok_t *tok)
{
    //wtk_debug("cnt=%d min=%d max=%d\n",cnt,repeat->min_count,repeat->max_count);
    if (repeat->min_count == 1 && repeat->max_count == 1) {
        if (cnt == 1) {
            return WTK_LEX_TOK_MATCH_AND_MUST_STEP;
        } else {
            return WTK_LEX_TOK_NOT_MATCH;
        }
    } else if (repeat->min_count == 0 && repeat->max_count == -1) {
        //*
        if (cnt < 1) {
            return WTK_LEX_TOK_NOT_MATCH;
        } else {
            return WTK_LEX_TOK_MATCH_AND_CAN_STEP;
        }
    } else if (repeat->min_count == 1 && repeat->max_count == -1) {
        //+
        if (cnt < 1) {
            return WTK_LEX_TOK_NOT_MATCH;
        } else {
            return WTK_LEX_TOK_MATCH_AND_CAN_STEP;
        }
    } else if (repeat->min_count == 0 && repeat->max_count == 1) {
        //?
        if (cnt != 1) {
            return WTK_LEX_TOK_NOT_MATCH;
        } else {
            return WTK_LEX_TOK_MATCH_AND_MUST_STEP;
        }
    } else if (repeat->min_count > 0 && repeat->max_count == -1) {
        if (cnt >= repeat->min_count) {
            return WTK_LEX_TOK_MATCH_AND_CAN_STEP;
        } else {
            return WTK_LEX_TOK_MATCH_AND_KEEP_POS;
        }
    } else {
        //{m,n}
        if (cnt > repeat->max_count) {
            return WTK_LEX_TOK_NOT_MATCH;
        } else if (cnt == repeat->max_count && cnt >= repeat->min_count) {
            return WTK_LEX_TOK_MATCH_AND_MUST_STEP;
        } else if (cnt >= repeat->min_count) {
            return WTK_LEX_TOK_MATCH_AND_CAN_STEP;
        } else if (cnt > 0) {
            return WTK_LEX_TOK_MATCH_AND_KEEP_POS;
        } else {
            return WTK_LEX_TOK_NOT_MATCH;
        }
    }
}

void wtk_lexr_add_tok(wtk_lexr_t *r, wtk_lexr_tok_t *tok)
{
    wtk_queue_push(&(r->tok_q), &(tok->tok_n));
}

int wtk_lexr_tok_match_ctx(wtk_lexr_t *r, wtk_lexr_tok_t *tok)
{
    wtk_lex_expr_item_value_attr_t *attr;
    wtk_lexr_align_t *a;
    wtk_string_t **strs, *v, *v1;
    wtk_string_t **wrds;
    wtk_strbuf_t *buf = r->buf;
    int i, b, pos, j;
    int nwrd;

    attr = &(tok->arc->value->attr);
    //wtk_debug("======================> min_wrd=%d/%d\n",attr->min_wrd_count,attr->max_wrd_count);
    if (tok->pth->pth_item->align
            && (attr->pre || attr->not_pre || attr->suc || attr->not_suc)) {
        a = tok->pth->pth_item->align;
        if (r->cur_expr->attr.use_seg) {
            wrds = (wtk_string_t**) (r->poseg->wrds);
            nwrd = r->poseg->nwrd;
        } else {
            wrds = (wtk_string_t**) (r->wrds->slot);
            nwrd = r->wrds->nslot;
        }
//		if(attr->not)
//		{
//			v=wrds[a->pos-1];
//			b=wtk_array_str_has(attr->not,v->data,v->len);
//			if(b)
//			{
//				return 0;
//			}
//		}
        //wtk_debug("pos=%d\n",a->pos);
        //wtk_debug("[%.*s]\n",wrds[a->pos-1]->len,wrds[a->pos-1]->data);
        if (attr->pre) {
            pos = a->pos;
            strs = (wtk_string_t**) attr->pre->slot;
            b = 0;
            for (i = 0; i < attr->pre->nslot; ++i) {
                v = strs[i];
                //wtk_debug("v=%p pre=%p\n",v,attr->pre);
                //wtk_debug("[%.*s]\n",v->len,v->data);
                wtk_strbuf_reset(buf);
                for (j = pos - 2; j >= 0; --j) {
                    v1 = wrds[j];
                    wtk_strbuf_push_front(buf, v1->data, v1->len);
                    //wtk_debug("[%.*s]\n",v1->len,v1->data);
                    if (buf->pos >= v->len) {
                        break;
                    }
                }
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                if (wtk_string_cmp(v, buf->data, buf->pos) == 0) {
                    b = 1;
                    break;
                }
            }
            if (!b) {
                return 0;
            }
        }
        if (attr->not_pre) {
            pos = a->pos;
            strs = (wtk_string_t**) attr->not_pre->slot;
            b = 0;
            for (i = 0; i < attr->not_pre->nslot; ++i) {
                v = strs[i];
                //wtk_debug("v=%p pre=%p\n",v,attr->pre);
                //wtk_debug("[%.*s]\n",v->len,v->data);
                wtk_strbuf_reset(buf);
                for (j = pos - 2; j >= 0; --j) {
                    v1 = wrds[j];
                    wtk_strbuf_push_front(buf, v1->data, v1->len);
                    //wtk_debug("[%.*s]\n",v1->len,v1->data);
                    if (buf->pos >= v->len) {
                        break;
                    }
                }
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                if (wtk_string_cmp(v, buf->data, buf->pos) == 0) {
                    b = 1;
                    break;
                }
            }
            if (b) {
                return 0;
            }
        }
        if (attr->suc) {
            pos = a->pos;
            strs = (wtk_string_t**) attr->suc->slot;
            b = 0;
            for (i = 0; i < attr->suc->nslot; ++i) {
                v = strs[i];
                //wtk_debug("v=%p pre=%p\n",v,attr->pre);
                //wtk_debug("[%.*s]\n",v->len,v->data);
                wtk_strbuf_reset(buf);
                //wtk_debug("j=%d/%d\n",pos,attr->suc->nslot);
                for (j = pos; j < nwrd; ++j) {
                    v1 = wrds[j];
                    wtk_strbuf_push(buf, v1->data, v1->len);
                    //wtk_debug("[%.*s]\n",v1->len,v1->data);
                    if (buf->pos >= v->len) {
                        break;
                    }
                }
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                if (wtk_string_cmp(v, buf->data, buf->pos) == 0) {
                    b = 1;
                    break;
                }
            }
            if (!b) {
                return 0;
            }
        }
        if (attr->not_suc) {
            pos = a->pos;
            strs = (wtk_string_t**) attr->not_suc->slot;
            b = 0;
            for (i = 0; i < attr->not_suc->nslot; ++i) {
                v = strs[i];
                //wtk_debug("v=%p pre=%p\n",v,attr->pre);
                //wtk_debug("[%.*s]\n",v->len,v->data);
                wtk_strbuf_reset(buf);
                //wtk_debug("j=%d/%d\n",pos,attr->suc->nslot);
                for (j = pos; j < nwrd; ++j) {
                    v1 = wrds[j];
                    wtk_strbuf_push(buf, v1->data, v1->len);
                    //wtk_debug("[%.*s]\n",v1->len,v1->data);
                    if (buf->pos >= v->len) {
                        break;
                    }
                }
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                if (wtk_string_cmp(v, buf->data, buf->pos) == 0) {
                    b = 1;
                    break;
                }
            }
            if (b) {
                return 0;
            }
        }
    }
    //wtk_debug("match_cnt=%d\n",tok->match_cnt);
    return 1;
}

wtk_lex_tok_match_no_t wtk_lexr_tok_match_expr(wtk_lexr_t *r,
        wtk_lexr_tok_t *tok)
{
    wtk_lex_tok_match_no_t no;
    int b;

    //wtk_debug("cnt=%d min=%d max=%d\n",tok->match_cnt,tok->arc->value->attr.min_wrd_count,tok->arc->value->attr.max_wrd_count);
    b = wtk_lexr_tok_match_ctx(r, tok);
    if (!b) {
        return WTK_LEX_TOK_NOT_MATCH;
    }
    no = wtk_lex_expr_repeat_match(&(tok->arc->value->attr.repeat),
            tok->match_cnt);
    return no;
}

int wtk_lexr_process_arc_match(wtk_lexr_t *r, wtk_lexr_tok_t *tok,
        wtk_string_t *v)
{
    wtk_string_t sp = wtk_string("");
    wtk_lex_tok_match_no_t no;
    int ret;
    wtk_lexr_align_t *a;

    //wtk_debug("[%.*s]\n",v->len,v->data);
    ++tok->match_cnt;
    //wtk_debug("tok=%p cnt=%d\n",tok,tok->match_cnt);
    if (tok->arc && tok->arc->value && tok->arc->value->attr.skip) {
        ret = wtk_str_str(tok->arc->value->attr.skip->data,
                tok->arc->value->attr.skip->len, v->data, v->len);
        if (ret == -1) {
            a = wtk_lexr_add_tok_align(r, tok, tok->arc, v);
        } else {
            v = &(sp);
            a = wtk_lexr_add_tok_align(r, tok, tok->arc, v);
        }
    } else {
        a = wtk_lexr_add_tok_align(r, tok, tok->arc, v);
    }
    if (r->hmmnr && tok->arc->value->attr.ner) {
        if (!a->v.str) {
            return 0;
        }
    }
    //wtk_lexr_pth_print(tok->pth);
    no = wtk_lexr_tok_match_expr(r, tok);
    //wtk_debug("match_cnt=%d %d/%d no=%d\n",tok->match_cnt,tok->arc->value->attr.repeat.min_count,tok->arc->value->attr.repeat.max_count,no);
    //no=wtk_lex_expr_repeat_match(&(tok->arc->value->attr.repeat),tok->match_cnt);
    switch (no) {
        case WTK_LEX_TOK_MATCH_AND_KEEP_POS:
            wtk_lexr_add_tok(r, tok);
            break;
        case WTK_LEX_TOK_MATCH_AND_CAN_STEP:
            wtk_lexr_add_tok(r, tok);
            wtk_lexr_add_step_node(r, tok->arc->to, tok);
            break;
        case WTK_LEX_TOK_MATCH_AND_MUST_STEP:
            wtk_lexr_add_step_node(r, tok->arc->to, tok);
            break;
        case WTK_LEX_TOK_NOT_MATCH:
            break;
    }
    return 0;
}

int wtk_lexr_process_arc_match2(wtk_lexr_t *r, wtk_lexr_tok_t *tok)
{
    wtk_lex_tok_match_no_t no;

    no = wtk_lex_expr_repeat_match(&(tok->arc->value->attr.repeat),
            tok->match_cnt);
    switch (no) {
        case WTK_LEX_TOK_MATCH_AND_KEEP_POS:
            wtk_lexr_add_tok(r, tok);
            break;
        case WTK_LEX_TOK_MATCH_AND_CAN_STEP:
            wtk_lexr_add_tok(r, tok);
            wtk_lexr_add_step_node(r, tok->arc->to, tok);
            break;
        case WTK_LEX_TOK_MATCH_AND_MUST_STEP:
            wtk_lexr_add_step_node(r, tok->arc->to, tok);
            break;
        case WTK_LEX_TOK_NOT_MATCH:
            break;
    }
    return 0;
}

wtk_string_t* wtk_lex_expr_item_value_attr_process(wtk_lexr_t *r,
        wtk_lex_expr_item_value_attr_t *attr, wtk_string_t *v)
{
    wtk_strbuf_t *buf = r->buf;
    int i;
    char c;
    int b;

    if (attr->txt != WTK_LEX_NORMAL || attr->chn2num || attr->skipws) {
        wtk_strbuf_reset(buf);
        b = 0;
        for (i = 0; i < v->len; ++i) {
            c = v->data[i];
            if (attr->skipws) {
                if (isspace(c)) {
                    b = 1;
                    continue;
                }
            }
            switch (attr->txt) {
                case WTK_LEX_LOWER:
                    if (isupper((unsigned char)c)) {
                 
						c = tolower(c);
                        b = 1;
                    }
                    break;
                case WTK_LEX_UPPER:
                    if (islower(c)) {
                        c = toupper(c);
                        b = 1;
                    }
                    break;
                default:
                    break;
            }
            wtk_strbuf_push_c(buf, c);
        }
        if (b) {
            v = wtk_heap_dup_string(r->heap, buf->data, buf->pos);
        }
        if (attr->chn2num && v->len > 1) {
            i = wtk_chnstr_atoi(v->data, v->len, 0);
            if (i != -1) {
                wtk_strbuf_reset(buf);
                wtk_strbuf_push_f(buf, "%d", i);
                v = wtk_heap_dup_string(r->heap, buf->data, buf->pos);
            }
        }
    }
    return v;
}

static wtk_string_t sp = wtk_string("");

int wtk_lexr_feed_var(wtk_lexr_t *r, wtk_lexr_tok_t *tok, wtk_string_t *v)
{
    wtk_lex_expr_item_value_attr_t *attr;
    wtk_treebin_env_t *env;
    wtk_treebin_env_t tenv;
    int ret = -1;
    int vt;

    //wtk_debug("[%.*s]\n",v->len,v->data);
    //wtk_lexr_pth_print(tok->pth);printf("\n");
    //if(!r->tbl){goto end;}
    attr = &(tok->arc->value->attr);
//	v=wtk_lex_expr_item_value_attr_process(r,attr,v);
//	if(!v||v->len==0){goto end;}
    //wtk_debug("pth=%d/%d\n",tok->pth->arc->value->type,tok->arc->value->type);
    env = &(tok->env);
    //!tok->pth->arc for:export expr1=${.菜名} => request(domain="菜谱")<0.610000>;
    if (!tok->pth->arc || tok->pth->arc->value->type != tok->arc->value->type) {
        //init env;
        *env = wtk_lexr_lib_get_env(r->lib,
                tok->arc->value->v.buildin_var->data,
                tok->arc->value->v.buildin_var->len);
        if (!env->tbl) {
            goto end;
        }
        wtk_lexr_add_tok_pth(r, tok, tok->arc);
    }
    tenv = *env;
    ret = wtk_lexr_lib_search(r->lib, env, v->data, v->len);
    //wtk_debug("[%.*s] ret=%d err=%d is_end=%d\n",v->len,v->data,ret,env->is_err,env->is_end);
    //exit(0);
    //wtk_treebin_env_print(env);
    if (ret != 0) {
        env->is_err = 1;
    }
    if (env->is_err) {
        //not match;
        //wtk_debug("skip=%p\n",attr->skip);
        if (attr->skip) {
            ret = wtk_str_str(attr->skip->data, attr->skip->len, v->data,
                    v->len);
            if (ret != -1) {
                //wtk_debug("[%.*s]\n",v->len,v->data);
                wtk_lexr_add_tok_align(r, tok, tok->arc, &sp);
                tok->env = tenv;
                wtk_lexr_add_tok(r, tok);
                ret = 0;
            }
        } else {
            ret = -1;
        }
        goto end;
    } else if (env->is_end) {
        //match end
        int cnt, pos;
        wtk_lexr_align_t *a;
        wtk_strbuf_t *buf = r->buf;
        wtk_lexr_pth_t *pth;

        wtk_lexr_add_tok_align(r, tok, tok->arc, v);
        cnt = 0;
        pos = 0;
        a = tok->pth->pth_item->align;
        wtk_strbuf_reset(buf);
        while (a) {
            if (a->type == WTK_LEXR_ALIGN_STR) {
                pos = a->pos;
                cnt += a->wrd_cnt;
                //wtk_debug("[%.*s]=%d/%d\n",a->v.str->len,a->v.str->data,a->wrd_cnt,a->pos);
                wtk_strbuf_push_front(buf, a->v.str->data, a->v.str->len);
            }
            a = a->prev;
        }
        a = wtk_lexr_new_align(r, tok->pth->arc);
        if (cnt == 0) {
            wtk_lexr_align_set_nil(a);
        } else {
            //a->v.str=wtk_heap_dup_string(r->heap,buf->data,buf->pos);
            wtk_lexr_align_set_str(r, a,
                    wtk_heap_dup_string(r->heap, buf->data, buf->pos));
            a->wrd_cnt = cnt;
            a->pos = pos;
            //wtk_debug("%p [%.*s]=%d/%d\n",a,a->v.str->len,a->v.str->data,cnt,a->pos);
        }
        //wtk_debug("[%.*s]\n",buf->pos,buf->data);
        //want more chars;
        wtk_lexr_add_tok(r, tok);
        vt = cnt;		//tok->match_cnt+1;
        //wtk_debug("vt=%d/%d [%d,%d]\n",cnt,vt,tok->arc->value->attr.min_wrd_count,tok->arc->value->attr.max_wrd_count);
        if (tok->arc->value->attr.min_wrd_count > 0
                && vt < tok->arc->value->attr.min_wrd_count) {
            ret = 0;
            goto end;
        }
        if (tok->arc->value->attr.max_wrd_count > 0
                && vt > tok->arc->value->attr.min_wrd_count) {
            ret = 0;
            goto end;
        }
        //wtk_debug("match=%d\n",tok->match_cnt);
        tok = wtk_lexr_dup_tok(r, tok);
        ++tok->match_cnt;
        //wtk_debug("match=%d\n",tok->match_cnt);
        tok->pth = tok->pth->prev;
        a->prev = tok->pth->pth_item->align;
        pth = (wtk_lexr_pth_t*) wtk_heap_malloc(r->rec_heap,
                sizeof(wtk_lexr_pth_t));
        *pth = *(tok->pth);
        pth->pth_item = (wtk_lexr_pth_item_t*) wtk_heap_malloc(r->rec_heap,
                sizeof(wtk_lexr_pth_item_t));
        *(pth->pth_item) = *(tok->pth->pth_item);
        pth->pth_item->align = a;
        tok->pth = pth;
        wtk_lexr_process_arc_match2(r, tok);
//		exit(0);
    } else {
        //match part;
        wtk_lexr_add_tok_align(r, tok, tok->arc, v);
        wtk_lexr_add_tok(r, tok);
    }
    end: return ret;
}

int wtk_lexr_match_py(wtk_lexr_t *r, wtk_string_t *dst, wtk_string_t *src)
{
    wtk_string_t *p1, *p2;
    wtk_fkv_t *kv = r->pron_kv;
    int ret = 0;
    wtk_heap_t *heap = r->heap;
    wtk_array_t *a1, *a2;
    wtk_string_t **strs1, **strs2;
    int i, j;

    //wtk_debug("[%.*s]=[%.*s]\n",dst->len,dst->data,src->len,src->data);
    p1 = wtk_fkv_get_str(kv, dst->data, dst->len);
    if (!p1) {
        goto end;
    }
    p2 = wtk_fkv_get_str(kv, src->data, src->len);
    if (!p2) {
        goto end;
    }
    a1 = wtk_str_to_array(heap, p1->data, p1->len, '|');
    a2 = wtk_str_to_array(heap, p2->data, p2->len, '|');
    strs1 = (wtk_string_t**) a1->slot;
    strs2 = (wtk_string_t**) a2->slot;
    for (i = 0; i < a2->nslot; ++i) {
        for (j = 0; j < a1->nslot; ++j) {
            if (wtk_string_cmp(strs1[j], strs2[i]->data, strs2[i]->len) == 0) {
                //wtk_debug("found [%.*s]=[%.*s]\n",strs1[j]->len,strs1[j]->data,strs2[i]->len,strs2[i]->data);
                ret = 1;
                goto end;
            }
        }
    }
    end:
    //exit(0);
    return ret;
}

int wtk_lexr_feed_tok(wtk_lexr_t *r, wtk_lexr_tok_t *tok, wtk_string_t *v)
{
    wtk_lex_expr_item_value_attr_t *attr;
    wtk_lexr_pth_t *pth;
    int ret;

    //wtk_debug("[%.*s]\n",v->len,v->data);
    //wtk_debug("input:[%.*s] tok=%p arc=%p type=%d ",v->len,v->data,tok,tok->arc,tok->arc->value->type);wtk_lex_expr_item_value_print(tok->arc->value);printf("\n");
//	if(tok->pth)
//	{
//		wtk_lexr_pth_print(tok->pth);printf("\n");
//	}
    attr = &(tok->arc->value->attr);
    //wtk_debug("[%.*s] min=%d max=%d minx=%d\n",v->len,v->data,attr->min_wrd_count,attr->max_wrd_count,attr->repeat.min_count);
    //wtk_lex_expr_item_value_print(tok->arc->value);
    //printf("\n");
    v = wtk_lex_expr_item_value_attr_process(r, attr, v);
    if (!v || v->len == 0) {
        wtk_lexr_add_tok(r, tok);
        goto end;
    }
    if (tok->arc->value->type == WTK_LEX_VALUE_BUILDIN_VAR) {
        ret = wtk_lexr_feed_var(r, tok, v);
    } else {
        if (tok->arc->value->attr.not) {
            ret = wtk_array_str_has(tok->arc->value->attr.not, v->data, v->len);
            //wtk_debug("[%.*s]=%d\n",v->len,v->data,ret);
            //wtk_array_print_string(tok->arc->value->attr.not);
            if (ret) {
                //wtk_debug("ret\n");
                return 0;
            }
        }
        ret = wtk_lexr_tok_match_ctx(r, tok);
        if (!ret) {
            goto end;
        }
        if (r->poseg && tok->arc->value->attr.pos) {
            //wtk_debug("v=[%.*s] %p\n",v->len,v->data,tok->arc->value->attr.not);
            //wtk_debug("[%.*s]\n",r->poseg->pos[r->wrd_pos-1]->len,r->poseg->pos[r->wrd_pos-1]->data);
            ret = wtk_array_str_in(tok->arc->value->attr.pos,
                    r->poseg->pos[r->wrd_pos - 1]->data,
                    r->poseg->pos[r->wrd_pos - 1]->len);
        } else {
            //wtk_debug("================ use_py=%d\n",tok->arc->value->attr.use_py);
            if (r->pron_kv && tok->arc->value->attr.use_py
                    && tok->arc->value->type == WTK_LEX_VALUE_STRING) {
                //wtk_debug("[%.*s]=[%.*s]\n",tok->arc->value->v.str->len,tok->arc->value->v.str->data,v->len,v->data);
                ret = wtk_lexr_match_py(r, tok->arc->value->v.str, v);
                if (ret == 1) {
                    v = tok->arc->value->v.str;
                }
            } else {
                ret = wtk_lex_expr_item_value_match(tok->arc->value, v);
            }
        }
        //wtk_debug("ret=%d\n",ret);
        if (ret) {
            ret = wtk_lexr_process_arc_match(r, tok, v);
        } else if (r->cur_expr->attr.check_skip) {
            pth = tok->pth;
            while (!attr->skip && pth && pth->arc && pth->arc->value) {
                attr = &(pth->arc->value->attr);
                pth = pth->prev;
            }
            //wtk_debug("attr=%p skip=%p\n",attr,attr->skip);
            //exit(0);
            if (attr->skip) {
                ret = wtk_str_str(attr->skip->data, attr->skip->len, v->data,
                        v->len);
                if (ret != -1) {
                    //wtk_debug("[%.*s]=[%.*s]\n",attr->skip->len,attr->skip->data,v->len,v->data);
                    wtk_lexr_add_tok_align(r, tok, tok->arc, &sp);
                    wtk_lexr_add_tok(r, tok);
                    ret = 0;
                }
            }
        }
    }
    end: return 0;
}

int wtk_lexr_feed(wtk_lexr_t *r, wtk_string_t *v)
{
    wtk_queue_t q;
    wtk_queue_node_t *qn;
    wtk_lexr_tok_t *tok;
    int ret;

    //wtk_debug("v[%d tok=%d]=%.*s\n",r->wrd_pos,r->tok_q.length,v->len,v->data);
    q = r->tok_q;
    wtk_queue_init(&(r->tok_q));
    while (1) {
        qn = wtk_queue_pop(&(q));
        if (!qn) {
            break;
        }
        tok = data_offset2(qn, wtk_lexr_tok_t, tok_n);
        ret = wtk_lexr_feed_tok(r, tok, v);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_lexr_has_var(wtk_lexr_t *r, char *act, int act_bytes, char *name,
        int name_bytes)
{
    wtk_json_item_t *ji;

    //wtk_debug("[%.*s].[%.*s]\n",act_bytes,act,name_bytes,name);
    //wtk_json_item_print3(r->action);
    ji = wtk_json_obj_get(r->action, act, act_bytes);
    if (!ji) {
        return 0;
    }
    ji = wtk_json_obj_get(ji, name, name_bytes);
    return ji ? 1 : 0;
}

wtk_string_t* wtk_lexr_get_var_value(wtk_json_item_t *ji)
{
    switch (ji->type) {
        case WTK_JSON_STRING:
            return ji->v.str;
            break;
        case WTK_JSON_OBJECT:
            ji = wtk_json_obj_get_s(ji, "_v");
            if (ji) {
                return wtk_lexr_get_var_value(ji);
            }
            break;
        case WTK_JSON_ARRAY:
            if (ji->v.array->length > 0) {
                wtk_json_array_item_t *ai;

                ai = data_offset2(ji->v.array->pop, wtk_json_array_item_t, q_n);
                return wtk_lexr_get_var_value(ai->item);
            }
            break;
        default:
            return NULL;
    }
    return NULL;
}

wtk_string_t* wtk_lexr_get_var(wtk_lexr_t *r, char *act, int act_bytes,
        char *name, int name_bytes)
{
    wtk_json_item_t *ji;

    //wtk_debug("[%.*s].[%.*s]\n",act_bytes,act,name_bytes,name);
    //wtk_json_item_print3(r->action);
    ji = wtk_json_obj_get(r->action, act, act_bytes);
    if (!ji) {
        return NULL;
    }
    ji = wtk_json_obj_get(ji, name, name_bytes);
    if (!ji) {
        return NULL;
    }
    return wtk_lexr_get_var_value(ji);
}

void wtk_lexr_del_var(wtk_lexr_t *r, wtk_string_t *act, wtk_string_t *var)
{
    wtk_json_item_t *item = r->action;
    wtk_json_item_t *vi;

    if (act) {
        vi = wtk_json_obj_get(item, act->data, act->len);
    } else {
        vi = wtk_json_obj_get_s(item, "request");
    }
    if (!vi) {
        return;
    }
    wtk_json_obj_remove(vi, var->data, var->len);
}

int wtk_lexr_json_has_value(wtk_json_item_t *a, char *v, int v_len)
{
    wtk_queue_node_t *qn;
    wtk_json_array_item_t *ji;
    wtk_json_item_t *vi;

    for (qn = a->v.array->pop; qn; qn = qn->next) {
        ji = data_offset2(qn, wtk_json_array_item_t, q_n);
        vi = wtk_json_obj_get_s(ji->item, "_v");
        if (vi && wtk_string_cmp(vi->v.str, v, v_len) == 0) {
            return 1;
        }
    }
    return 0;
}

void wtk_lexr_add_var(wtk_lexr_t *r, wtk_string_t *act, wtk_string_t *var,
        wtk_string_t *value)
{
    wtk_json_item_t *item = r->action;
    wtk_json_item_t *vi, *v1, *req;
    wtk_json_t *json = r->json;

    if (act) {
        req = wtk_json_obj_get(item, act->data, act->len);
    } else {
        req = wtk_json_obj_get_s(item, "request");
    }
    if (!req) {
        return;
    }
    vi = wtk_json_obj_get(req, var->data, var->len);
    if (vi) {
        if (wtk_lexr_json_has_value(vi, value->data, value->len) == 0) {
            v1 = wtk_json_new_object(json);
            wtk_json_obj_add_str2_s(json, v1, "_v", value->data, value->len);
            wtk_json_array_add_item(json, vi, v1);
        }
    } else {
        v1 = wtk_json_new_array(json);
        vi = wtk_json_new_object(json);
        wtk_json_obj_add_str2_s(json, vi, "_v", value->data, value->len);
        wtk_json_array_add_item(json, v1, vi);
        wtk_json_obj_add_item2(json, req, var->data, var->len, v1);
    }
}

void wtk_lexr_mv_var(wtk_lexr_t *r, wtk_string_t *act, wtk_string_t *var,
        wtk_string_t *act2, wtk_string_t *var2)
{
    static wtk_string_t xreq = wtk_string("request");
    wtk_json_item_t *item=r->action;
    wtk_json_item_t *vi,
    *req, *dst, *v2;
    wtk_json_t *json = r->json;
    wtk_queue_node_t *qn;
    wtk_json_array_item_t *ji;

    if (!act) {
        act = &(xreq);
    }
    if (!act2) {
        act = &(xreq);
    }
    if (act == act2 || wtk_string_cmp(act, act2->data, act2->len) == 0) {
        req = wtk_json_obj_get(item, act->data, act->len);
    } else {
        req = wtk_json_obj_remove(item, act->data, act->len);
    }
    if (!req) {
        return;
    }
    vi = wtk_json_obj_remove(req, var->data, var->len);
    if (!vi) {
        return;
    }
    dst = wtk_json_obj_get(item, act2->data, act2->len);
    if (dst) {
        v2 = wtk_json_obj_get(dst, var2->data, var2->len);
        if (v2) {
            for (qn = vi->v.array->pop; qn; qn = qn->next) {
                ji = data_offset2(qn, wtk_json_array_item_t, q_n);
                vi = wtk_json_obj_get_s(ji->item, "_v");
                if (vi
                        && wtk_lexr_json_has_value(v2, vi->v.str->data,
                                vi->v.str->len) == 0) {
                    wtk_json_array_add_item(json, v2, ji->item);
                }
            }
        } else {
            wtk_json_obj_add_item2(json, dst, var2->data, var2->len, vi);
        }
    } else {
        dst = wtk_json_new_object(json);
        wtk_json_obj_add_item2(json, dst, var2->data, var2->len, vi);
        //wtk_json_item_print3(dst);
        wtk_json_obj_add_item2(json, item, act2->data, act2->len, dst);
    }
    //wtk_json_item_print3(item);
}

void wtk_lexr_cpy_var(wtk_lexr_t *r, wtk_string_t *act, wtk_string_t *var,
        wtk_string_t *act2, wtk_string_t *var2)
{
    static wtk_string_t xreq = wtk_string("request");
    wtk_json_item_t *item=r->action;
    wtk_json_item_t *vi,
    *req, *dst, *v2;
    wtk_json_t *json = r->json;
    wtk_queue_node_t *qn;
    wtk_json_array_item_t *ji;

    if (!act) {
        act = &(xreq);
    }
    if (!act2) {
        act = &(xreq);
    }
    req = wtk_json_obj_get(item, act->data, act->len);
    if (!req) {
        return;
    }
    vi = wtk_json_obj_get(req, var->data, var->len);
    if (!vi) {
        return;
    }
    dst = wtk_json_obj_get(item, act2->data, act2->len);
    if (dst) {
        v2 = wtk_json_obj_get(dst, var2->data, var2->len);
        if (v2) {
            for (qn = vi->v.array->pop; qn; qn = qn->next) {
                ji = data_offset2(qn, wtk_json_array_item_t, q_n);
                vi = wtk_json_obj_get_s(ji->item, "_v");
                if (vi
                        && wtk_lexr_json_has_value(v2, vi->v.str->data,
                                vi->v.str->len) == 0) {
                    wtk_json_array_add_item(json, v2, ji->item);
                }
            }
        } else {
            wtk_json_obj_add_item2(json, dst, var2->data, var2->len, vi);
        }
    } else {
        dst = wtk_json_new_object(json);
        wtk_json_obj_add_item2(json, dst, var2->data, var2->len, vi);
        //wtk_json_item_print3(dst);
        wtk_json_obj_add_item2(json, item, act2->data, act2->len, dst);
    }
    //wtk_json_item_print3(item);
}

void wtk_lexr_process_cmd(wtk_lexr_t *r, wtk_lex_expr_cmd_t *cmd)
{
    switch (cmd->type) {
        case WTK_LEX_EXPR_CMD_DEL:
            wtk_lexr_del_var(r, cmd->op1_act, cmd->op1_var);
            break;
        case WTK_LEX_EXPR_CMD_ADD:
            wtk_lexr_add_var(r, cmd->op1_act, cmd->op1_var, cmd->op2_var);
            break;
        case WTK_LEX_EXPR_CMD_MV:
            wtk_lexr_mv_var(r, cmd->op1_act, cmd->op1_var, cmd->op2_act,
                    cmd->op2_var);
            break;
        case WTK_LEX_EXPR_CMD_CPY:
            wtk_lexr_cpy_var(r, cmd->op1_act, cmd->op1_var, cmd->op2_act,
                    cmd->op2_var);
            break;
        case WTK_LEX_EXPR_CMD_DEBUG:
            wtk_lexr_print(r);
            exit(0);
            break;
        case WTK_LEX_EXPR_CMD_RETURN:
            r->run = 0;
            break;
    }
}

int wtk_lexr_feed2(wtk_lexr_t *r, wtk_string_t *v)
{
    int m;

    switch (r->cur_expr->attr.txt) {
        case WTK_LEX_LOWER:
            v = wtk_heap_dup_string(r->rec_heap, v->data, v->len);
            for (m = 0; m < v->len; ++m) {
                v->data[0] = tolower(v->data[0]);
            }
            break;
        case WTK_LEX_UPPER:
            v = wtk_heap_dup_string(r->rec_heap, v->data, v->len);
            for (m = 0; m < v->len; ++m) {
                v->data[0] = toupper(v->data[0]);
            }
            break;
        default:
            break;
    }
    return wtk_lexr_feed(r, v);
}

int wtk_lexr_has_output(wtk_lexr_t *r, wtk_lex_expr_output_t *output)
{
    wtk_queue_node_t *qn;
    wtk_json_item_t *vi, *ti;
    wtk_string_t v;
    wtk_lex_expr_output_item_t *oi;

    if (output->name && output->name->len > 0) {
        v = *(output->name);
    } else {
        wtk_string_set_s(&(v), "request");
    }
    vi = wtk_json_obj_get(r->action, v.data, v.len);
    if (!vi) {
        //wtk_debug("[%.*s] not found\n",v.len,v.data);
        return 0;
    }
    //wtk_debug("output=%d\n",output->item_q.length);
    for (qn = output->item_q.pop; qn; qn = qn->next) {
        oi = data_offset2(qn, wtk_lex_expr_output_item_t, q_n);
        if (oi->k) {
            ti = wtk_json_obj_get(vi, oi->k->data, oi->k->len);
            //wtk_debug("[%.*s]=%p\n",oi->k->len,oi->k->data,ti);
            //wtk_json_item_print3(vi);
            if (!ti) {
                //wtk_debug("[%.*s] not found\n",oi->k->len,oi->k->data);
                return 0;
            }
        }
    }
    return 1;
}

void wtk_lexr_process_expr(wtk_lexr_t *r, wtk_lex_net_root_t *root)
{
    wtk_queue_node_t *qn;
    wtk_string_t **strs;
    int nstr;
    wtk_lex_expr_if_t *ixf;
    int j, k;
    int out;
    int b;

    strs = (wtk_string_t**) (r->wrds->slot);
    switch (root->expr->type) {
        case WTK_LEX_EXPR_ITEM:
            //wtk_debug("%d\n",r->input_net->script->use_fast_match);
            if (r->cfg->debug) {
                wtk_lex_expr_item_print(root->expr->v.expr);
            }
            //wtk_lex_expr_item_print(root->expr->v.expr);
            //wtk_debug("ki=%d, use_act=%d fast_match=%d\n",ki,r->input_net->script->use_act,r->input_net->script->use_fast_match);
            if (r->input_net->script->use_act
                    && r->input_net->script->use_fast_match) {
                if (wtk_lexr_has_output(r, &(root->expr->v.expr->output))) {
                    return;
                }
            }
            if (r->poseg && root->expr->v.expr->attr.use_seg) {
                nstr = r->poseg->nwrd;
                strs = r->poseg->wrds;
            } else {
                nstr = r->wrds->nslot;
            }
            j = 0;
            //wtk_debug("============> nslot=%d/%d\n",j,r->wrds->nslot);
            while (j < nstr) {
                wtk_lexr_prepare(r, root->v.node, root->expr->v.expr, j);
                out = r->output_tok_q.length;
                //wtk_debug("out=%d\n",r->output_q.length);
                if (r->tok_q.length > 0) {
                    for (k = j; k < nstr; ++k) {
                        //wtk_debug("v[%d]=[%.*s] tok=%d\n",k,strs[k]->len,strs[k]->data,r->tok_q.length);
                        r->wrd_pos = k + 1;
                        wtk_lexr_feed2(r, strs[k]);
                        if (r->tok_q.length == 0) {
                            break;
                        }
                    }
                    //wtk_debug("tok=%d\n",r->tok_q.length);
                }
                //exit(0);
                if (r->cur_expr->attr.step_search) {
                    ++j;
                } else {
                    if (r->output_tok_q.length > out && r->match_wrds > 0) {
                        //j=r->wrd_pos;'
                        j += r->match_wrds;
                    } else {
                        ++j;
                    }
                }
                wtk_lexr_clean_expr(r);
            }
            break;
        case WTK_LEX_EXPR_SCOPE:
            if (root->expr->v.scope->and) {
                b = 1;
            } else {
                b = 0;
            }
            //wtk_json_item_print3(r->action);
            //wtk_lex_expr_print(root->expr);
            for (qn = root->expr->v.scope->condition.pop; qn; qn = qn->next) {
#define DEF_ACT "request"
                char *data;
                int len;

                ixf = data_offset2(qn, wtk_lex_expr_if_t, q_n);
                //wtk_debug("[%.*s]\n",ixf->var->len,ixf->var->data);
                if (ixf->act) {
                    data = ixf->act->data;
                    len = ixf->act->len;
                } else {
                    data = DEF_ACT;
                    len = sizeof(DEF_ACT) - 1;
                }
                if (ixf->value) {
                    wtk_string_t *v;

                    if (ixf->var->data[0] == '.') {
                        if (r->get_var_f) {
                            v = r->get_var_f(r->get_var_ths, data, len,
                                    ixf->var->data, ixf->var->len);
                            if (v) {
                                j = wtk_string_cmp(v, ixf->value->data,
                                        ixf->value->len) == 0 ? 1 : 0;
                            } else {
                                j = 0;
                            }
                        } else {
                            j = 0;
                        }
                    } else {
                        v = wtk_lexr_get_var(r, data, len, ixf->var->data,
                                ixf->var->len);
                        if (v) {
                            j = wtk_string_cmp(v, ixf->value->data,
                                    ixf->value->len) == 0 ? 1 : 0;
                        } else {
                            j = 0;
                        }
                    }
                } else {
                    j = wtk_lexr_has_var(r, data, len, ixf->var->data,
                            ixf->var->len);
                    //wtk_debug("j=%d not=%d\n",j,ixf->not);
                    if (j == 0 && r->has_var_f) {
                        j = r->has_var_f(r->has_var_ths, data, len,
                                ixf->var->data, ixf->var->len);
                    }
                }
                //wtk_debug("j=%d\n",j);
                if (ixf->not) {
                    j = j == 0 ? 1 : 0;
                }
                if (root->expr->v.scope->and) {
                    if (j == 0) {
                        b = 0;
                        break;
                    }
                } else {
                    if (j == 1) {
                        b = 1;
                        break;
                    }
                }
            }
            //wtk_debug("b=%d\n",b);
            if (b) {
                for (j = 0; j < root->v.scope->nyes; ++j) {
                    wtk_lexr_process_expr(r, root->v.scope->yes[j]);
                    if (r->input_net->script->use_nbest == 0
                            && r->action_list->v.array->length > 0) {
                        break;
                    }
                }
            } else {
                for (j = 0; j < root->v.scope->nno; ++j) {
                    wtk_lexr_process_expr(r, root->v.scope->no[j]);
                    if (r->input_net->script->use_nbest == 0
                            && r->action_list->v.array->length > 0) {
                        break;
                    }
                }
            }
            //exit(0);
            break;
        case WTK_LEX_EXPR_CMD:
            wtk_lexr_process_cmd(r, root->expr->v.cmd);
            break;
    }
}

void wtk_lexr_process_expr_trans(wtk_lexr_t *r, wtk_lex_net_root_t *root,
        int index)
{
    wtk_string_t **strs;
    int j;

    //wtk_lex_expr_item_print(root->expr->v.expr);
    //exit(0);
    if (root->expr->type != WTK_LEX_EXPR_ITEM) {
        return;
    }
    strs = (wtk_string_t**) (r->wrds->slot);
    wtk_lexr_prepare(r, root->v.node, root->expr->v.expr, 0);
    for (j = index; j < r->wrds->nslot; ++j) {
        r->wrd_pos = j + 1;
        //wtk_debug("v[%d/%d]=[%.*s] len=%d\n",index,j,strs[j]->len,strs[j]->data,r->tok_q.length);
        wtk_lexr_feed2(r, strs[j]);
        //wtk_debug("tok=%d\n",r->tok_q.length);
        if (r->tok_q.length == 0) {
            break;
        }
    }
//	wtk_debug("output=%d\n",r->output_trans.match_wrd_cnt);
//	if(r->output_trans.v)
//	{
//		wtk_debug("[%.*s]\n",r->output_trans.v->len,r->output_trans.v->data);
//	}
//	exit(0);
}

void wtk_lexr_process_trans(wtk_lexr_t *r)
{
    wtk_lex_net_t *net = r->input_net;
    int i;
    int index;
    wtk_strbuf_t *buf;
    wtk_string_t **strs;
    int match_cnt = 0;

    buf = wtk_strbuf_new(256, 1);
    strs = (wtk_string_t**) (r->wrds->slot);
    for (index = 0; index < r->wrds->nslot;) {
        for (i = 0; i < net->nroot; ++i) {
            wtk_lexr_process_expr_trans(r, net->roots[i], index);
            if (r->output_trans.match_wrd_cnt > 0) {
                break;
            }
        }
        //wtk_debug("%d\n",r->output_trans.match_wrd_cnt);
        if (r->output_trans.match_wrd_cnt > 0) {
            wtk_strbuf_push_word(buf, r->output_trans.v->data,
                    r->output_trans.v->len);
            index += r->output_trans.match_wrd_cnt;
            match_cnt += r->output_trans.match_wrd_cnt;
        } else {
            wtk_strbuf_push_word(buf, strs[index]->data, strs[index]->len);
            //last_is_eng=wtk_utf8_bytes(strs[index]->data[0])>1?0:1;
            //wtk_debug("strs[index]->len=%d\n",strs[index]->len);
            ++index;
        }
        //wtk_debug("[%.*s] %d/%d\n",buf->pos,buf->data,index,r->wrds->nslot);
        //wtk_debug("last_is_eng-%d\n",last_is_eng);
    }
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    r->output_trans.v = wtk_heap_dup_string(r->heap, buf->data, buf->pos);
    r->output_trans.match_wrd_cnt = match_cnt;
    wtk_strbuf_delete(buf);
    //wtk_debug("[%.*s]\n",r->output_trans.v->len,r->output_trans.v->data);
    //exit(0);
}

void wtk_lexr_print_wrds(wtk_lexr_t *r)
{
    wtk_string_t **strs;
    int i;

    strs = (wtk_string_t**) (r->wrds->slot);
    for (i = 0; i < r->wrds->nslot; ++i) {
        printf("v[%d]=[%.*s]\n", i, strs[i]->len, strs[i]->data);
        if (i > 20) {
            exit(0);
        }
    }
}

void wtk_lexr_set_has_var_function(wtk_lexr_t *r, void *ths,
        wtk_lexr_has_var_f has_var)
{
    r->has_var_f = has_var;
    r->has_var_ths = ths;
}

void wtk_lexr_set_get_var_function(wtk_lexr_t *r, void *ths,
        wtk_lexr_get_var_f get_var)
{
    r->get_var_f = get_var;
    r->get_var_ths = ths;
}

int wtk_lexr_process(wtk_lexr_t *r, wtk_lex_net_t *net, char *input,
        int input_len)
{
    wtk_lex_net_root_t *root;
    int ret;
    int i;

    r->input = wtk_heap_dup_string(r->heap, input, input_len);
    r->run = 1;
    r->input_net = net;
    //wtk_debug("n=%d\n",net->nroot);
    wtk_lexr_update_words(r, input, input_len, net->script->use_eng_word);
    if (r->poseg && net->use_poseg) {
        wtk_poseg_process(r->poseg, input, input_len);
        //wtk_poseg_print_output(r->poseg);
    }
    wtk_lexr_init_action(r);
    if (r->wrds->nslot <= 0) {
        ret = 0;
        goto end;
    }
    if (net->script->use_act) {
        for (i = 0; i < net->nroot; ++i) {
            root = net->roots[i];
            //wtk_debug("i=%d/%d\n",i,net->nroot);
            wtk_lexr_process_expr(r, root);
            //wtk_debug("nbest=%d a=%d\n",net->script->use_nbest,r->action_list->v.array->length);
            if (net->script->use_nbest == 0
                    && r->action_list->v.array->length > 0) {
                break;
            }
            //wtk_debug("action=%p list=%p len=%d\n",r->action,r->action_list,r->action_list->v.array->length);
            if (!r->run) {
                break;
            }
        }
    } else {
        wtk_lexr_process_trans(r);
    }
    ret = 0;
    end: if (r->poseg) {
        wtk_poseg_reset(r->poseg);
    }
    return ret;
}

void wtk_lexr_pth_print2(wtk_lexr_pth_t *pth);

void wtk_lexr_align_print3(wtk_lexr_align_t *a)
{
    switch (a->type) {
        case WTK_LEXR_ALIGN_STR:
            if (a->v.str) {
                printf("%.*s", a->v.str->len, a->v.str->data);
            }
            break;
        case WTK_LEXR_ALIGN_JSON:
            wtk_json_item_print4(a->v.json);
            break;
        case WTK_LEXR_ALIGN_PTH: {
            wtk_lexr_pth_t *pth;
            int cnt;

            cnt = a->v.pth->depth;	//wtk_lexr_pth_depth(a->v.pth);
            pth = a->v.pth->prev;
            a->v.pth->prev = NULL;
            printf("%%(%d)", cnt);
            wtk_lexr_pth_print2(a->v.pth);
            printf("%%");
            a->v.pth->prev = pth;
        }
            break;
        case WTK_LEXR_ALIGN_NIL:
            break;
    }
}

void wtk_lexr_align_print2(wtk_lexr_align_t *a)
{
    if (a->prev) {
        wtk_lexr_align_print2(a->prev);
    }
    //wtk_debug("type=%d\n",a->type);
    switch (a->type) {
        case WTK_LEXR_ALIGN_STR:
            //wtk_debug("str=%p\n",a->v.str);
            if (a->v.str) {
                printf("[%.*s]", a->v.str->len, a->v.str->data);
            }
            break;
        case WTK_LEXR_ALIGN_JSON:
            wtk_json_item_print4(a->v.json);
            break;
        case WTK_LEXR_ALIGN_PTH: {
            wtk_lexr_pth_t *pth;
            int cnt;

            cnt = a->v.pth->depth;	//wtk_lexr_pth_depth(a->v.pth);
            pth = a->v.pth->prev;
            a->v.pth->prev = NULL;
            printf("%%(%d)", cnt);
            wtk_lexr_pth_print2(a->v.pth);
            printf("%%");
            a->v.pth->prev = pth;
        }
            break;
        case WTK_LEXR_ALIGN_NIL:
            break;
    }
}

void wtk_lexr_align_print(wtk_lexr_align_t *a)
{
    wtk_lexr_align_print2(a);
    //printf("\n");
}

void wtk_lexr_pth_item_print2(wtk_lexr_pth_item_t *item)
{
    //wtk_debug("prev=%p align=%p\n",item->prev,item->align);
    if (item->prev) {
        wtk_lexr_pth_item_print2(item->prev);
    }
    if (item->align) {
        if (item->prev) {
            printf("|");
        }
        wtk_lexr_align_print(item->align);
    }
}

void wtk_lexr_pth_item_print(wtk_lexr_pth_item_t *item)
{
    wtk_lexr_pth_item_print2(item);
    //printf("\n");
}

void wtk_lexr_pth_print2(wtk_lexr_pth_t *pth)
{
    //wtk_debug("pth=%p\n",pth);
    if (pth->prev) {
        wtk_lexr_pth_print2(pth->prev);
        //printf("\n");
    }
    if (pth->pth_item) {
        //printf("\n");
        if (pth->pth_item->align) {
            printf("(");
        }
        wtk_lexr_pth_item_print(pth->pth_item);
        if (pth->pth_item->align) {
            printf(")");
        }
    }
}

void wtk_lexr_pth_print(wtk_lexr_pth_t *pth)
{
    //wtk_debug("pth=%p\n",pth);
    wtk_lexr_pth_print2(pth);
    printf("\n");
}

void wtk_lexr_print(wtk_lexr_t *r)
{
    if (r->action) {
        wtk_json_item_print3(r->action);
    } else {
        wtk_debug("nil\n");
    }
}

wtk_string_t wtk_lexr_get_result(wtk_lexr_t *l)
{
    wtk_string_t v;
    wtk_strbuf_t *buf = l->buf;

    if (l->input_net->script->use_act) {
        wtk_strbuf_reset(buf);
        wtk_json_item_print(l->action, buf);
        wtk_string_set(&(v), buf->data, buf->pos);
    } else {
        if (l->output_trans.v) {
            wtk_string_set(&(v), l->output_trans.v->data,
                    l->output_trans.v->len);
        } else {
            wtk_string_set(&(v), 0, 0);
        }
    }
    return v;
}

int wtk_lexr_start(wtk_lexr_t *r, wtk_lex_net_t *net)
{
    if (net->nroot <= 0) {
        return -1;
    }
    wtk_lexr_prepare(r, net->roots[0]->v.node, net->roots[0]->expr->v.expr, 0);
    r->wrd_pos = 0;
    r->wrds = NULL;
    return 0;
}

int wtk_lexr_feed_str(wtk_lexr_t *r, wtk_string_t *v)
{
    int ret;

    //wtk_debug("feed [%.*s]\n",v->len,v->data);
    ++r->wrd_pos;
    ret = wtk_lexr_feed2(r, v);
    if (ret == 0) {
        if (r->tok_q.length == 0 && r->output_tok_q.length == 0) {
            ret = -1;
        }
    }
    return ret;
}

wtk_lexr_output_item_t* wtk_lexr_pop_output(wtk_lexr_t *r)
{
    wtk_queue_node_t *qn;
    wtk_lexr_output_item_t *oi;

    if (r->output_tok_q.pop) {
        qn = wtk_queue_pop(&(r->output_tok_q));
        oi = data_offset2(qn, wtk_lexr_output_item_t, q_n);
        return oi;
    } else {
        return NULL;
    }
}
