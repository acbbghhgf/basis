#include "wtk_nlgfst.h" 

wtk_nlgfst_t* wtk_nlgfst_new(wtk_rbin2_t *rbin, char *nlg_fn, char *fst_fn)
{
    wtk_nlgfst_t *fst;

    fst = (wtk_nlgfst_t*) wtk_malloc(sizeof(wtk_nlgfst_t));
    fst->nlg = wtk_nlg_new2(rbin, nlg_fn);
    //wtk_nlg_print(fst->nlg);
    //exit(0);
    fst->net = wtk_nlgnet_new(rbin, fst_fn);
    //wtk_nlgnet_print(fst->net);
    //exit(0);
    wtk_nlgnet_bind(fst->net, fst->nlg);
    fst->heap = wtk_heap_new(4096);
    fst->emit_process = NULL;
    fst->emit_ths = NULL;
    wtk_nlgfst_reset(fst);
    //int wtk_nlgnet_bind(wtk_nlgnet_t *net,wtk_nlg_t *nlg)
    return fst;
}

//wtk_nlgfst_t* wtk_nlgfst_new2(wtk_rbin2_t *rbin,char *nlg_fn,char *fst_fn,int use_nlg2)
//{
//}

void wtk_nlgfst_delete(wtk_nlgfst_t *fst)
{
    wtk_heap_delete(fst->heap);
    wtk_nlgnet_delete(fst->net);
    wtk_nlg_delete(fst->nlg);
    wtk_free(fst);
}

void wtk_nlgfst_set_emit(wtk_nlgfst_t *fst, void *ths, wtk_nlgfst_emit_f emit)
{
    fst->emit_ths = ths;
    fst->emit_process = emit;
}

void wtk_nlgfst_reset(wtk_nlgfst_t *fst)
{
    wtk_queue_init(&(fst->input_slot_q));
    wtk_heap_reset(fst->heap);
    fst->cur_state = fst->net->root;
    fst->cur_state_round = 0;
}

int wtk_nglfst_can_be_end(wtk_nlgfst_t *fst)
{
    //wtk_debug("cur=%p end=%p\n",fst->cur_state,fst->net->end);
    if (!fst->cur_state || fst->cur_state == fst->net->end) {
        return 1;
    }
    return 0;
}

void wtk_nlgfst_set_state(wtk_nlgfst_t *fst, char *state, int state_bytes)
{
    wtk_nlgnet_state_t *s;

    s = wtk_nlgnet_get_state(fst->net, state, state_bytes, 0);
    //wtk_debug("s=%p[%.*s]\n",s,s->name->len,s->name->data);
    if (s) {
        fst->cur_state = s;
        fst->cur_state_round = 0;
    } else {
        wtk_debug("[%.*s] not found\n", state_bytes, state);
    }
}

void wtk_nlgfst_add_kv(wtk_nlgfst_t *fst, wtk_string_t *k, wtk_string_t *v)
{
    wtk_nlgnet_arc_attr_t *atrr;

    atrr = wtk_nlgnet_new_arc_attr(fst->heap, k, v);
    wtk_queue_push(&(fst->input_slot_q), &(atrr->q_n));
}

void wtk_nlgfst_update_slot(wtk_nlgfst_t *fst, char *data, int bytes)
{
    wtk_str_attr_parse(data, bytes, fst, (wtk_str_attr_f) wtk_nlgfst_add_kv);
}

int wtk_nlgfst_has_attr(wtk_nlgfst_t *fst, wtk_nlgnet_arc_attr_t *attr)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_attr_t *ax;

    for (qn = fst->input_slot_q.pop; qn; qn = qn->next) {
        ax = data_offset2(qn, wtk_nlgnet_arc_attr_t, q_n);
        if (wtk_string_cmp(ax->k, attr->k->data, attr->k->len) == 0) {
            if (attr->v) {
                if (ax->v
                        && wtk_string_cmp(attr->v, ax->v->data, ax->v->len)
                                == 0) {
                    return 1;
                }
            } else {
                return 1;
            }
        }
    }
    return 0;
}

int wtk_nlgfst_match_arc(wtk_nlgfst_t *fst, wtk_nlgnet_arc_t *arc)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_attr_t *attr;
    int ret;

    if (arc->match_all_slot) {
        if (fst->input_slot_q.length != arc->attr_q.length) {
            return 0;
        }
    } else {
        if (fst->input_slot_q.length < arc->attr_q.length) {
            return 0;
        }
    }
    for (qn = arc->attr_q.pop; qn; qn = qn->next) {
        attr = data_offset2(qn, wtk_nlgnet_arc_attr_t, q_n);
        ret = wtk_nlgfst_has_attr(fst, attr);
        if (ret == 0) {
            return 0;
        }
    }
    return 1;
}

int wtk_nlgfst_step_arc(wtk_nlgfst_t *fst, wtk_nlgnet_arc_t *arc)
{
    wtk_nlgnet_state_t *state;
    wtk_nlgnet_state_t *last_state;
    int ret;

    if (arc->min_round > 0 && fst->cur_state_round < arc->min_round) {
        return -1;
    }
    if (arc->max_round > 0 && fst->cur_state_round >= arc->max_round) {
        return -1;
    }
    ret = wtk_nlgfst_match_arc(fst, arc);
    if (ret == 0) {
        ret = -1;
        goto end;
    }
    if (arc->emit_item) {
        fst->cur_state_round = 0;
        ret = fst->emit_process(fst->emit_ths, arc->emit_item);
        goto end;
    }
    state = wtk_nlgnet_arc_next(arc);
    if (!state) {
        ret = -1;
        goto end;
    }
    if (state->emit_item) {
        last_state = fst->cur_state;
        ret = fst->emit_process(fst->emit_ths, state->emit_item);
        if (ret == 0) {
            if (state != fst->cur_state) {
                if (fst->cur_state == last_state) {
                    fst->cur_state_round = 1;
                    fst->cur_state = state;
                } else {
                    fst->cur_state_round = 1;
                }
            } else {
                ++fst->cur_state_round;
            }
        }
    } else {
        ret = -1;
    }
    end: return ret;
}

void wtk_nlgfst_process_state(wtk_nlgfst_t *fst)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_t *arc;
    int ret;

    for (qn = fst->cur_state->output_arc_q.pop; qn; qn = qn->next) {
        arc = data_offset2(qn, wtk_nlgnet_arc_t, arc_n);
        ret = wtk_nlgfst_step_arc(fst, arc);
        if (ret == 0) {
            return;
        }
    }
}

void wtk_nlgfst_feed(wtk_nlgfst_t *fst, char *data, int bytes)
{
    wtk_nlgfst_update_slot(fst, data, bytes);
    wtk_nlgfst_process_state(fst);
}

int wtk_nlgfst_has_attr_nlgfst_act(wtk_nlgfst_t *fst,
        wtk_nlgnet_arc_attr_t *attr, wtk_nlgfst_act_t *act)
{
    wtk_string_t *v;

    if (attr->v) {
        v = act->get_value(act->ths, attr->k->data, attr->k->len);
        //wtk_debug("[%.*s]=%p v=%p\n",attr->k->len,attr->k->data,v,attr->v)
        if (v && wtk_string_cmp(attr->v, v->data, v->len) == 0) {
            //wtk_debug("[%.*s]\n",v->len,v->data);
            return 1;
        } else {
            return 0;
        }
    } else {
        return act->has_key(act->ths, attr->k->data, attr->k->len);
    }
    return 0;
}

int wtk_nlgfst_match_arc_nlgfst_act(wtk_nlgfst_t *fst, wtk_nlgnet_arc_t *arc,
        wtk_nlgfst_act_t *act)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_attr_t *attr;
    int ret;
    int nact;

    //wtk_debug("arc=%d\n",arc->or);
    if (!arc->or) {
        nact = act->nvalue(act->ths);
        //wtk_debug("nact=%d/%d\n",nact,arc->attr_q.length);
        if (arc->match_all_slot) {
            if (nact != arc->attr_q.length) {
                return 0;
            }
        } else {
            if (nact < arc->attr_q.length) {
                return 0;
            }
        }
        for (qn = arc->attr_q.pop; qn; qn = qn->next) {
            attr = data_offset2(qn, wtk_nlgnet_arc_attr_t, q_n);
            //wtk_debug("[%.*s]=%p\n",attr->k->len,attr->k->data,attr->v);
            ret = wtk_nlgfst_has_attr_nlgfst_act(fst, attr, act);
            //wtk_debug("ret=%d\n",ret);
            if (ret == 0) {
                return 0;
            }
        }
        return 1;
    } else {
        for (qn = arc->attr_q.pop; qn; qn = qn->next) {
            attr = data_offset2(qn, wtk_nlgnet_arc_attr_t, q_n);
            ret = wtk_nlgfst_has_attr_nlgfst_act(fst, attr, act);
            //wtk_debug("ret=%d\n",ret);
            if (ret == 1) {
                return 1;
            }
        }
        return 0;
    }
}

int wtk_nlgfst_feed_state_nlgfst_act(wtk_nlgfst_t *fst,
        wtk_nlgnet_state_t *state, wtk_nlgfst_act_t *act);

int wtk_nlgfst_step_arc_nlgfst_act2(wtk_nlgfst_t *fst, wtk_nlgnet_arc_t *arc,
        wtk_nlgfst_act_t *act)
{
    wtk_nlgnet_state_t *state, *last_state;
    int ret;

    //wtk_nlgnet_arc_print(arc);
    //wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
    if (arc->skip_fld) {
        ret = 1;
        goto end;
    }
    state = wtk_nlgnet_arc_next(arc);
    if (!state) {
        wtk_debug("next state not found.\n");
        ret = -1;
        goto end;
    }
    last_state = fst->cur_state;
    //wtk_debug("arc item=%p %p\n",arc->emit_item,arc->emit_func);
    if (arc->emit_item) {
        ret = wtk_nlg_process(fst->nlg, arc->emit_item, act->buf, act->ths,
                act->get_value, act->feed_lua);
        //wtk_debug("ret=%d\n",ret);
        if (ret == 0) {
            if (fst->cur_state == last_state) {
                fst->cur_state_round = 1;
                fst->cur_state = state;
                //wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
            }
            //wtk_debug("other=%p\n",fst->cur_state->other);
//			if(arc->clean_ctx)
//			{
//				wtk_semslot_reset(((wtk_semfld_t*)fst->emit_ths)->slot);
//			}
        }
        goto end;
    }
    if (!arc->use_emit) {
        ret = wtk_nlgfst_feed_state_nlgfst_act(fst, state, act);
        goto end;
    } else {
        //wtk_debug("emit=%p\n",state->emit_item);
        if (state->emit_item) {
            if (state != fst->cur_state) {
                if (fst->cur_state == last_state) {
                    fst->cur_state_round = 1;
                    fst->cur_state = state;
                    //wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
                }
                //wtk_debug("state: %.*s\n",fst->cur_state->name->len,fst->cur_state->name->data);
            } else {
                ++fst->cur_state_round;
            }
            //wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
            //wtk_nlg_item_print(state->emit_item);
            ret = wtk_nlg_process(fst->nlg, state->emit_item, act->buf,
                    act->ths, act->get_value, act->feed_lua);
            //wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
            //wtk_debug("ret=%d\n",ret);
            if (ret == 0) {
                //wtk_debug("other=%p\n",fst->cur_state->other);
//				if(arc->clean_ctx)
//				{
//					wtk_semslot_reset(((wtk_semfld_t*)fst->emit_ths)->slot);
//				}
            }
        } else {
            ret = -1;
        }
    }
    end:
    //wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
    //wtk_debug("ret=%d\n",ret);
    return ret;
}

int wtk_nlgfst_step_arc_nlgfst_act(wtk_nlgfst_t *fst, wtk_nlgnet_arc_t *arc,
        wtk_nlgfst_act_t *act)
{
    int ret;

    //wtk_nlgnet_arc_print(arc);
    if (arc->min_round > 0 && fst->cur_state_round < arc->min_round) {
        ret = -1;
        goto end;
    }
    if (arc->max_round > 0 && fst->cur_state_round >= arc->max_round) {
        ret = -1;
        goto end;
    }
    //wtk_nlgnet_arc_print(arc);
    ret = wtk_nlgfst_match_arc_nlgfst_act(fst, arc, act);
    //wtk_debug("match ret=%d\n",ret);
    if (ret == 0) {
        ret = -1;
        goto end;
    }
    ret = wtk_nlgfst_step_arc_nlgfst_act2(fst, arc, act);
    end:
    //wtk_debug("ret=%d\n",ret);
    return ret;
}

int wtk_nlgfst_feed_state_nlgfst_act(wtk_nlgfst_t *fst,
        wtk_nlgnet_state_t *state, wtk_nlgfst_act_t *act)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_t *arc;
    int ret;

    //wtk_debug("[%.*s] out_arc=%d\n",state->name->len,state->name->data,state->output_arc_q.length);
//	if(state==fst->net->root)
//	{
//		wtk_semslot_reset(((wtk_semfld_t*)fst->emit_ths)->slot);
//	}
    //wtk_act_print(act);
    for (qn = state->output_arc_q.pop; qn; qn = qn->next) {
        arc = data_offset2(qn, wtk_nlgnet_arc_t, arc_n);
        //wtk_nlgnet_arc_print(arc);
        ret = wtk_nlgfst_step_arc_nlgfst_act(fst, arc, act);
        //wtk_debug("ret=%d\n",ret);
        if (ret >= 0) {
            return ret;
        }
    }
    //wtk_debug("other=%p\n",fst->cur_state->other);
    //wtk_nlgnet_state_print(fst->cur_state);
    if (state->other) {
        ret = wtk_nlgfst_step_arc_nlgfst_act(fst, state->other, act);
        if (ret >= 0) {
            return ret;
        } else if (state->other->other) {
            return wtk_nlgfst_feed_state_nlgfst_act(fst, state->other->other,
                    act);
        }
    }
    return -1;
}

int wtk_nlgfst_feed2(wtk_nlgfst_t *fst, wtk_nlgfst_act_t *act)
{
    wtk_string_t v;
    int ret = -1;

    wtk_strbuf_reset(act->buf);
    if (!fst->cur_state) {
        wtk_debug("empty state\n");
        return -1;
    }
    if (fst->cur_state && fst->cur_state->pre) {
        v = act->feed_lua(act->ths, fst->cur_state->pre);
        if (v.len > 0) {
            wtk_strbuf_push(act->buf, v.data, v.len);
            goto end;
        }
    }
    ret = wtk_nlgfst_feed_state_nlgfst_act(fst, fst->cur_state, act);
    //wtk_debug("================> ret=%d\n",ret);
    if (ret == 0) {
        //wtk_debug("[%.*s] eps=%p\n",fst->cur_state->name->len,fst->cur_state->name->data,fst->cur_state->eps);
        while (fst->cur_state && fst->cur_state->eps) {
            fst->cur_state = fst->cur_state->eps;
        }
    }
    end:
    //wtk_debug("state=[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
    return ret;
}
