#include "wtk_nlgfst2.h" 
int wtk_nlgfst2_feed_state_nlgfst_act(wtk_nlgfst2_t *fst,
        wtk_nlgnet_state_t *state, wtk_nlgfst_act_t *act);

wtk_nlgfst2_t* wtk_nlgfst2_new(wtk_nlg2_t *nlg)
{
    wtk_nlgfst2_t *f;

    f = (wtk_nlgfst2_t*) wtk_malloc(sizeof(wtk_nlgfst2_t));
    f->nlg = nlg;
    f->debug = 0;
    wtk_nlgfst2_reset(f);
    return f;
}

void wtk_nlgfst2_delete(wtk_nlgfst2_t *f)
{
    wtk_free(f);
}

void wtk_nlgfst2_reset(wtk_nlgfst2_t *f)
{
    f->cur_state = NULL;
    f->cur_state_round = 0;
    f->net = NULL;
}

void wtk_nlgfst2_set_net(wtk_nlgfst2_t *f, wtk_nlgnet_t *net)
{
    wtk_nlgfst2_reset(f);
    f->net = net;
    f->cur_state = net ? net->root : NULL;
}

int wtk_nlgfst2_has_attr_nlgfst_act(wtk_nlgfst2_t *fst,
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

int wtk_nlgfst2_match_arc_nlgfst_act(wtk_nlgfst2_t *fst, wtk_nlgnet_arc_t *arc,
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
            ret = wtk_nlgfst2_has_attr_nlgfst_act(fst, attr, act);
            //wtk_debug("ret=%d\n",ret);
            if (ret == 0) {
                return 0;
            }
        }
        return 1;
    } else {
        for (qn = arc->attr_q.pop; qn; qn = qn->next) {
            attr = data_offset2(qn, wtk_nlgnet_arc_attr_t, q_n);
            ret = wtk_nlgfst2_has_attr_nlgfst_act(fst, attr, act);
            //wtk_debug("ret=%d\n",ret);
            if (ret == 1) {
                return 1;
            }
        }
        return 0;
    }
}

int wtk_nglfst2_process_nlg(wtk_nlgfst2_t *fst, wtk_nlgfst_act_t *act,
        wtk_string_t *nlg)
{
    wtk_heap_t *heap = fst->nlg->rec_heap;
    wtk_nlg2_function_t f;
    int i;
    wtk_nlg2_item_t *item;
    wtk_nlg2_gen_env_t env;

    //wtk_debug("[%.*s]\n",nlg->len,nlg->data);
    wtk_nlg2_function_init(&(f));
    if (nlg->data[nlg->len - 1] == ')') {
        wtk_nlg2_function_parse(&(f), heap, nlg->data, nlg->len, NULL);
    } else {
        f.name = nlg;
    }
    //wtk_nlg2_function_print(&(f));
    for (i = 0; i < f.narg; ++i) {
        if (!f.args[i]->v) {
            f.args[i]->v = act->get_value(act->ths, f.args[i]->k->data,
                    f.args[i]->k->len);
        }
    }
    //wtk_nlg2_function_print(&(f));
    item = wtk_nlg2_find_item(fst->nlg, &(f));
    if (!item) {
        goto end;
    }
    env.ths = act->ths;
    env.lua_gen = NULL;
    env.lua_gen2 = (wtk_nlg2_get_lua_gen2_f) act->feed_lua;
    wtk_nlg2_item_gen(fst->nlg, item, act->buf, heap, &(f), &env);
    end:
    //wtk_debug("[%.*s]\n",act->buf->pos,act->buf->data);
    //wtk_heap_reset(heap);
    //exit(0);
    return 0;
}

int wtk_nlgfst2_step_arc_nlgfst_act2(wtk_nlgfst2_t *fst, wtk_nlgnet_arc_t *arc,
        wtk_nlgfst_act_t *act)
{
    wtk_nlgnet_state_t *state, *last_state;
    int ret;
    wtk_string_t v;

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
    if (arc->emit_func) {
        //wtk_debug("[%.*s]\n",arc->emit_func->len,arc->emit_func->data)
        ret = wtk_nglfst2_process_nlg(fst, act, arc->emit_func);
        //wtk_debug("[%.*s]\n",act->buf->pos,act->buf->data);
        //ret=wtk_nlg_process(fst->nlg,arc->emit_item,act->buf,act->ths,act->get_value,act->feed_lua);
        //wtk_debug("ret=%d\n",ret);
        if (ret == 0) {
            if (state->post) {
                v = act->feed_lua(act->ths, state->post);
                if (v.len > 0) {
                    wtk_strbuf_push(act->buf, v.data, v.len);
                }
            }
            if (fst->cur_state == last_state) {
                fst->cur_state_round = 1;
                fst->cur_state = state;
            }
        }
        goto end;
    }
    //wtk_debug("use_emit=%d\n",arc->use_emit);
    if (!arc->use_emit) {
        ret = wtk_nlgfst2_feed_state_nlgfst_act(fst, state, act);
        goto end;
    } else {
        //wtk_debug("emit=%p\n",state->emit_item);
        if (state->emit) {
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
            if (state->pre) {
                act->feed_lua(act->ths, state->pre);
            }
            //wtk_debug("emit=%.*s\n",state->emit->len,state->emit->data);
            ret = wtk_nglfst2_process_nlg(fst, act, state->emit);
            //exit(0);
            //wtk_debug("[%.*s]\n",act->buf->pos,act->buf->data);
            if (state->post) {
                v = act->feed_lua(act->ths, state->post);
                if (v.len > 0) {
                    wtk_strbuf_push(act->buf, v.data, v.len);
                }
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

int wtk_nlgfst2_step_arc_nlgfst_act(wtk_nlgfst2_t *fst, wtk_nlgnet_arc_t *arc,
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
    ret = wtk_nlgfst2_match_arc_nlgfst_act(fst, arc, act);
    //wtk_debug("match ret=%d\n",ret);
    if (ret == 0) {
        ret = -1;
        goto end;
    }
    ret = wtk_nlgfst2_step_arc_nlgfst_act2(fst, arc, act);
    end:
    //wtk_debug("ret=%d\n",ret);
    return ret;
}

int wtk_nlgfst2_feed_state_nlgfst_act(wtk_nlgfst2_t *fst,
        wtk_nlgnet_state_t *state, wtk_nlgfst_act_t *act)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_t *arc;
    wtk_string_t v;
    int ret;

    if (state && state->pre) {
        v = act->feed_lua(act->ths, state->pre);
        if (v.len > 0) {
            wtk_strbuf_push(act->buf, v.data, v.len);
            return 0;
        }
    }
    //wtk_debug("[%.*s] out_arc=%d\n",state->name->len,state->name->data,state->output_arc_q.length);
//	if(state==fst->net->root)
//	{
//		wtk_semslot_reset(((wtk_semfld_t*)fst->emit_ths)->slot);
//	}
    //wtk_act_print(act);
    for (qn = state->output_arc_q.pop; qn; qn = qn->next) {
        arc = data_offset2(qn, wtk_nlgnet_arc_t, arc_n);
        //wtk_nlgnet_arc_print(arc);
        ret = wtk_nlgfst2_step_arc_nlgfst_act(fst, arc, act);
        //wtk_debug("ret=%d\n",ret);
        if (ret >= 0) {
            return ret;
        }
    }
    //wtk_debug("other=%p\n",fst->cur_state->other);
    //wtk_nlgnet_state_print(fst->cur_state);
    if (state->other) {
        //wtk_nlgnet_arc_print(state->other);
        ret = wtk_nlgfst2_step_arc_nlgfst_act(fst, state->other, act);
        //wtk_debug("ret=%d\n",ret);
        if (ret >= 0) {
            return ret;
        } else if (state->other->other) {
            return wtk_nlgfst2_feed_state_nlgfst_act(fst, state->other->other,
                    act);
        }
    }
    return -1;
}

int wtk_nlgfst2_feed(wtk_nlgfst2_t *fst, wtk_nlgfst_act_t *act)
{
    //wtk_string_t v;
    int ret = -1;

    //fst->debug=1;
    wtk_strbuf_reset(act->buf);
    if (!fst->cur_state) {
        wtk_debug("empty state\n");
        return -1;
    }
    if (fst->debug && fst->cur_state) {
        wtk_debug("cur_state=%.*s\n", fst->cur_state->name->len,
                fst->cur_state->name->data);
    }
    //wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
    ret = wtk_nlgfst2_feed_state_nlgfst_act(fst, fst->cur_state, act);
    if (ret == 0) {
        //wtk_debug("[%.*s] eps=%p\n",fst->cur_state->name->len,fst->cur_state->name->data,fst->cur_state->eps);
        while (fst->cur_state && fst->cur_state->eps) {
            fst->cur_state = fst->cur_state->eps;
        }
    }
    if (fst->debug && fst->cur_state) {
        wtk_debug("cur_state=%.*s\n", fst->cur_state->name->len,
                fst->cur_state->name->data);
    }
    wtk_heap_reset(fst->nlg->rec_heap);
    //wtk_debug("state=[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
    return ret;
}
