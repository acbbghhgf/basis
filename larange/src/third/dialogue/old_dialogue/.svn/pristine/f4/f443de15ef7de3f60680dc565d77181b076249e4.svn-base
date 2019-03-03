#include "wtk_hmmnr.h" 
#include "wtk/core/wtk_str_encode.h"

wtk_hmmnr_t* wtk_hmmnr_new()
{
    wtk_hmmnr_t *h;

    h = (wtk_hmmnr_t*) wtk_malloc(sizeof(wtk_hmmnr_t));
    h->ne = NULL;
    h->fkv = NULL;
    h->heap = wtk_heap_new(4096);
    h->wrd_pen = 0;
    wtk_hmmnr_reset(h);
    return h;
}

void wtk_hmmnr_set_fkv(wtk_hmmnr_t *n, char *fn)
{
    n->fkv = wtk_fkv_new3(fn);
}

void wtk_hmmnr_delete(wtk_hmmnr_t *n)
{
//	if(n->fkv)
//	{
//		wtk_fkv_delete(n->fkv);
//	}
    wtk_heap_delete(n->heap);
    wtk_free(n);
}

void wtk_hmmnr_reset(wtk_hmmnr_t *n)
{
    n->wrd_pen = 1.0;
    n->prune_thresh = -21.0;
    n->best_inst = NULL;
}

wtk_hmmnr_inst_t* wtk_hmmnr_new_inst(wtk_hmmnr_t *h)
{
    wtk_hmmnr_inst_t *inst;

    inst = (wtk_hmmnr_inst_t*) wtk_heap_malloc(h->heap,
            sizeof(wtk_hmmnr_inst_t));
    inst->prob = 0;
    inst->wrd_pos = h->pos;
    inst->end_pos = 0;
    inst->index = 0;
    //wtk_debug("=============== new inst=%p ===============\n",inst);
    return inst;
}

void wtk_hmmnr_add_output(wtk_hmmnr_t *n, wtk_hmmnr_inst_t *inst)
{
    //inst->prob/=inst->index;
    //inst->prob=wtk_hmmnr_prop2ppl(inst->prob,inst->index);
    //wtk_debug("%d/%d/%d\n",inst->index,inst->wrd_pos,inst->end_pos);
    inst->end_pos = n->end_pos;
//	{
//		static int ki=0;
//
//		++ki;
//		wtk_debug("v[%d]=[%.*s] %d/%f wrd=%d end=%d\n",ki,inst->end_pos-inst->wrd_pos,n->input.data+inst->wrd_pos,
//			inst->index,inst->prob,inst->wrd_pos,inst->end_pos);
//	}
    if (!n->best_inst || inst->prob > n->best_inst->prob) {
        n->best_inst = inst;
    }
    //wtk_queue_push(&(n->output_q),&(inst->q_n));
}

void wtk_hmmnr_feed_wrd(wtk_hmmnr_t *n, wtk_hmmnr_inst_t *inst,
        wtk_hmmnr_wrd_t *wrd)
{
    wtk_hmmnr_inst_t *inst2;
    float f;

    if (inst) {
        //wtk_hmmnr_feed_wrd(n,NULL,wrd);
        inst->prob += n->wrd_pen;
        if (inst->index == 1) {
            //BM
            f = inst->prob + n->ne->B_M + (wrd ? wrd->b_m : n->ne->unk_bm);
            if (f > n->prune_thresh) {
                inst2 = wtk_hmmnr_new_inst(n);
                inst2->wrd_pos = inst->wrd_pos;
                inst2->index = inst->index + 1;
                inst2->prob = f;
                //wtk_debug("add inst=%p len=%d index=%d\n",inst2,n->inst_q.length,inst2->index);
                wtk_queue_push(&(n->inst_q), &(inst2->q_n));
            }
            //BE
            f = inst->prob + n->ne->B_E + (wrd ? wrd->b_e : n->ne->unk_be);
            if (f > n->prune_thresh) {
                ++inst->index;
                inst->prob = f;
                wtk_hmmnr_add_output(n, inst);
            }
        } else {
            //MM
            f = inst->prob + n->ne->B_M + (wrd ? wrd->b_m : n->ne->unk_mm);
            if (f > n->prune_thresh) {
                inst2 = wtk_hmmnr_new_inst(n);
                //wtk_debug("add inst=%p len=%d index=%d\n",inst2,n->inst_q.length,inst2->index);
                wtk_queue_push(&(n->inst_q), &(inst2->q_n));
                inst2->wrd_pos = inst->wrd_pos;
                inst2->index = inst->index + 1;
                inst2->prob = f;
            }
            //ME
            f = inst->prob + n->ne->M_E + (wrd ? wrd->m_e : n->ne->unk_me);
            if (f > n->prune_thresh) {
                ++inst->index;
                inst->prob = f;
                wtk_hmmnr_add_output(n, inst);
            }
        }
    } else {
        if (wrd) {
            f = wrd->b;
        } else {
            f = n->ne->unk_b;
        }
        //wtk_debug("wrd=%p f=%f p=%f\n",wrd,f,n->prune_thresh);
        if (f > n->prune_thresh) {
            inst = wtk_hmmnr_new_inst(n);
            ++inst->index;
            inst->prob = f + n->wrd_pen;
            wtk_queue_push(&(n->inst_q), &(inst->q_n));
            //wtk_debug("wrd=%p/%f\n",wrd,wrd->s);
            if (wrd && wrd->s > n->ne->unk_b) {
                inst = wtk_hmmnr_new_inst(n);
                ++inst->index;
                inst->prob = wrd->s + n->wrd_pen;
                wtk_hmmnr_add_output(n, inst);
            }
        }
    }
}

void wtk_hmmnr_inst_print(wtk_hmmnr_t *h, wtk_hmmnr_inst_t *inst)
{
    //inst->end_pos=h->end_pos;
    wtk_debug("=============== inst=%p ===============\n", inst);
    wtk_debug("index=%d S=%d\n", inst->index, inst->wrd_pos);
}

void wtk_hmmnr_feed(wtk_hmmnr_t *n, wtk_hmmnr_wrd_t *wrd)
{
    wtk_queue_t q;
    wtk_queue_node_t *qn;
    wtk_hmmnr_inst_t *inst;

    if (n->inst_q.length > 0) {
        q = n->inst_q;
        wtk_queue_init(&(n->inst_q));
        wtk_hmmnr_feed_wrd(n, NULL, wrd);
        //wtk_debug("==============================\n");
        while (1) {
            qn = wtk_queue_pop(&(q));
            if (!qn) {
                break;
            }
            inst = data_offset2(qn, wtk_hmmnr_inst_t, q_n);
            //wtk_hmmnr_inst_print(n,inst);
            //wtk_debug("feed inst=%p\n",inst);
            wtk_hmmnr_feed_wrd(n, inst, wrd);
        }
    } else {
        wtk_hmmnr_feed_wrd(n, NULL, wrd);
    }
}

wtk_string_t wtk_hmmnr_rec2(wtk_hmmnr_t *h, char *data, int bytes,
        float wrd_pen, float prune_thresh)
{
    h->prune_thresh = prune_thresh;
    h->wrd_pen = wrd_pen;
    return wtk_hmmnr_rec(h, data, bytes);
}

wtk_string_t wtk_hmmnr_rec(wtk_hmmnr_t *h, char *data, int bytes)
{
    char *s, *e;
    wtk_hmmnr_wrd_t *wrd;
    int n;
    wtk_string_t v;

    wtk_string_set(&(h->input), data, bytes);
    s = data;
    e = s + bytes;
    h->best_inst = NULL;
    wtk_queue_init(&(h->inst_q));
    wtk_queue_init(&(h->output_q));
    h->pos = 0;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        h->end_pos = h->pos + n;
        //wtk_debug("[%.*s]\n",n,s);
        wrd = (wtk_hmmnr_wrd_t*) wtk_str_hash_find(h->ne->hash, s, n);
        if (!wrd) {
            h->best_inst = NULL;
            break;
        }
        wtk_hmmnr_feed(h, wrd);
        s += n;
        h->pos += n;
    }
    if (h->best_inst) {
        v.data = data + h->best_inst->wrd_pos;
        v.len = h->best_inst->end_pos - h->best_inst->wrd_pos;
        //wtk_debug("[%.*s]=%f\n",v.len,v.data,h->best_inst->prob);
        h->prob = h->best_inst->prob;
    } else {
        wtk_string_set(&(v), 0, 0);
        h->prob = -1e10;
    }
    wtk_heap_reset(h->heap);
    return v;
}

wtk_hmmnr_wrd_t* wtk_hmmnr_find_wrd(wtk_hmmnr_t *h, char *wrd, int wrd_len)
{
    wtk_string_t *v;

    if (h->fkv) {
        v = wtk_fkv_get_str(h->fkv, wrd, wrd_len);
        //wtk_debug("[%.*s]=%p\n",wrd_len,wrd,v);
        if (!v) {
            return NULL;
        }
        wrd = v->data;
        wrd_len = v->len;
    }
    return (wtk_hmmnr_wrd_t*) wtk_str_hash_find(h->ne->hash, wrd, wrd_len);
}

float wtk_hmmnr_process(wtk_hmmnr_t *h, char *data, int bytes)
{
    float prob;
    char *s, *e;
    int n;
    wtk_hmmnr_wrd_t *wrd;
    int nwrd;
    int idx;

    prob = 0;
    s = data;
    e = s + bytes;
    nwrd = 0;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        ++nwrd;
        s += n;
    }
    if (nwrd == 1) {
        //wrd=(wtk_hmmnr_wrd_t*)wtk_str_hash_find(h->ne->hash,data,bytes);
        wrd = wtk_hmmnr_find_wrd(h, data, bytes);
        //wtk_debug("%f/%f\n",wrd->s,h->ne->unk_b);
        //wtk_debug("[%.*s] b=%f bm=%f be=%f mm=%f me=%f\n",wrd->wrd->len,wrd->wrd->data,wrd->b,wrd->b_m,wrd->b_e,wrd->m_m,wrd->m_e);
        if (wrd && wrd->s > h->ne->unk_b) {
            //wtk_debug("%f/%f\n",h->ne->unk_b,wrd->s);
            return wrd->s + h->wrd_pen;
        } else {
            return -100;
        }
    }
    s = data;
    idx = 0;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        //wrd=(wtk_hmmnr_wrd_t*)wtk_str_hash_find(h->ne->hash,s,n);
        wrd = wtk_hmmnr_find_wrd(h, s, n);
        if (!wrd) {
            prob = -1e5;
            break;
        }
        //wtk_debug("[%.*s] b=%f bm=%f be=%f mm=%f me=%f\n",wrd->wrd->len,wrd->wrd->data,wrd->b,wrd->b_m,wrd->b_e,wrd->m_m,wrd->m_e);
        ++idx;
        if (idx == 1) {
            if (wrd) {
                prob += wrd->b;
            } else {
                prob += h->ne->unk_b;
            }
        } else if (idx == nwrd) {
            if (wrd) {
                if (idx == 2) {
                    prob += wrd->b_e + h->ne->B_E;
                } else {
                    prob += wrd->m_e + h->ne->M_E;
                }
            } else {
                if (idx == 2) {
                    prob += h->ne->unk_be + h->ne->B_E;
                } else {
                    prob += h->ne->unk_me + h->ne->M_E;
                }
            }
        } else {
            if (wrd) {
                if (idx == 2) {
                    prob += wrd->b_m + h->ne->B_M;
                } else {
                    prob += wrd->m_m + h->ne->M_M;
                }
            } else {
                if (idx == 2) {
                    prob += h->ne->unk_bm + h->ne->B_M;
                } else {
                    prob += h->ne->unk_mm + h->ne->M_M;
                }
            }
        }
        prob += h->wrd_pen;
        s += n;
    }
    return prob;
}

float wtk_hmmnr_process2(wtk_hmmnr_t *h, char *data, int bytes, float word_pen)
{
    h->wrd_pen = word_pen;
    return wtk_hmmnr_process(h, data, bytes);
}

