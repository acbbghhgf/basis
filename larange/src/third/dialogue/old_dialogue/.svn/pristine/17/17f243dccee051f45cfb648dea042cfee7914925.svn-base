#include "wtk_nlgnet.h" 
#include <ctype.h>
#include <stdlib.h>
#include "wtk/core/wtk_os.h"

wtk_nlgnet_state_t* wtk_nlgnet_new_state(wtk_nlgnet_t *fst, char *nm,
        int nm_bytes)
{
    wtk_nlgnet_state_t *state;

    //wtk_debug("[%.*s]\n",nm_bytes,nm);
    state = (wtk_nlgnet_state_t*) wtk_heap_malloc(fst->heap,
            sizeof(wtk_nlgnet_state_t));
    state->name = wtk_heap_dup_string(fst->heap, nm, nm_bytes);
    state->emit = NULL;
    state->pre = NULL;
    state->post = NULL;
    state->eps = NULL;
    state->emit_item = NULL;
    wtk_queue_init(&(state->output_arc_q));
    state->other = NULL;
    return state;
}

void wtk_nlgnet_arc_print(wtk_nlgnet_arc_t *arc)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_attr_t *attr;
    wtk_nglnet_arc_to_item_t *to;

    if (arc->attr_q.length > 0) {
        for (qn = arc->attr_q.pop; qn; qn = qn->next) {
            attr = data_offset2(qn, wtk_nlgnet_arc_attr_t, q_n);
            if (qn != arc->attr_q.pop) {
                printf(",");
            }
            printf("%.*s", attr->k->len, attr->k->data);
            if (attr->v) {
                printf("=\"%.*s\"", attr->v->len, attr->v->data);
            }
        }
    } else {
        printf("nil");
    }
    printf("=>");
    for (qn = arc->to_q.pop; qn; qn = qn->next) {
        to = data_offset2(qn, wtk_nglnet_arc_to_item_t, q_n);
        if (qn != arc->to_q.pop) {
            printf("|");
        }
        printf("%.*s", to->state->name->len, to->state->name->data);
    }
    if (arc->min_round != 0 || arc->max_round != -1) {
        int pad = 0;

        printf(" /");
        if (arc->min_round != 0) {
            printf("min_round=%d", arc->min_round);
            pad = 1;
        }
        if (arc->max_round != -1) {
            if (pad) {
                printf(",");
            }
            printf("max_round=%d", arc->max_round);
            pad = 1;
        }
        printf("/");
    }
    printf("\n");
}

void wtk_nlgnet_state_print(wtk_nlgnet_state_t *state)
{
    wtk_queue_node_t *qn;
    wtk_nlgnet_arc_t *arc;

    printf("%.*s:\n", state->name->len, state->name->data);
    if (state->emit) {
        printf("EMIT: %.*s\n", state->emit->len, state->emit->data);
    }
    if (state->pre) {
        printf("PRE: %s\n", state->pre);
    }
    if (state->eps) {
        printf("EPS: %.*s\n", state->eps->name->len, state->eps->name->data);
    }
    if (state->output_arc_q.length > 0 || state->other) {
        printf("OUTPUT:\n");
    }
    for (qn = state->output_arc_q.pop; qn; qn = qn->next) {
        arc = data_offset2(qn, wtk_nlgnet_arc_t, arc_n);
        wtk_nlgnet_arc_print(arc);
    }
    if (state->other) {
        printf("other:\n");
        wtk_nlgnet_arc_print(state->other);
    }
    printf("\n");
}

wtk_nlgnet_state_t* wtk_nlgnet_get_state(wtk_nlgnet_t *fst, char *nm,
        int nm_bytes, int insert)
{
    wtk_nlgnet_state_t *state;
    wtk_queue_node_t *qn;

    for (qn = fst->state_q.pop; qn; qn = qn->next) {
        state = data_offset2(qn, wtk_nlgnet_state_t, q_n);
        if (wtk_string_cmp(state->name, nm, nm_bytes) == 0) {
            return state;
        }
    }
    if (insert) {
        state = wtk_nlgnet_new_state(fst, nm, nm_bytes);
        wtk_queue_push(&(fst->state_q), &(state->q_n));
        return state;
    } else {
        return NULL;
    }
}

typedef enum {
    WTK_nlgnet_PARSER_INIT = 0,
    WTK_nlgnet_PARSER_STATE,
    WTK_nlgnet_PARSER_STATE_WAIT_COLON,
    WTK_nlgnet_PARSER_STATE_WAIT_INFO,
    WTK_nlgnet_PARSER_STATE_INFO,
    WTK_nlgnet_PARSER_STATE_INFO_EMIT,
    WTK_nlgnet_PARSER_STATE_INFO_INPUT,
    WTK_nlgnet_PARSER_STATE_INFO_ESP,
    WTK_nlgnet_PARSER_STATE_INFO_PRE,
    WTK_nlgnet_PARSER_STATE_INFO_POST,
    WTK_nlgnet_PARSER_COMMENT,
} wtk_nlgnet_parser_state_t;

typedef struct {
    wtk_nlgnet_t *fst;
    wtk_strbuf_t *buf;
    wtk_nlgnet_parser_state_t state;
    wtk_nlgnet_parser_state_t nxt_colon_state;
    wtk_nlgnet_state_t *s;
    wtk_nlgnet_arc_t *arc;
    int sub_state;
} wtk_nlgnet_parser_t;

int wtk_nlgnet_parser_feed(wtk_nlgnet_parser_t *p, wtk_string_t *v);

void wtk_nlgnet_parser_set_state(wtk_nlgnet_parser_t *p,
        wtk_nlgnet_parser_state_t state)
{
    p->state = state;
    p->sub_state = -1;
}

int wtk_nlgnet_parser_feed_init(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    if (v->len > 1 || !isspace(v->data[0])) {
        if (v->len == 1 && v->data[0] == '#') {
            //p->state=WTK_nlgnet_PARSER_COMMENT;
            wtk_nlgnet_parser_set_state(p, WTK_nlgnet_PARSER_COMMENT);
        } else {
            wtk_strbuf_reset(p->buf);
            //p->state=WTK_nlgnet_PARSER_STATE;
            wtk_nlgnet_parser_set_state(p, WTK_nlgnet_PARSER_STATE);
            return wtk_nlgnet_parser_feed(p, v);
        }
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    //wtk_debug("[%.*s]=[%.*s]\n",v->len,v->data,p->buf->pos,p->buf->data);
    if (v->len == 1 && (isspace(v->data[0]) || v->data[0] == ':')) {
        //wtk_debug("[%.*s]\n",v->len,v->data);
        //p->s=wtk_nlgnet_new_state(p->fst,p->buf->data,p->buf->pos);
        p->s = wtk_nlgnet_get_state(p->fst, p->buf->data, p->buf->pos, 1);
        if (v->data[0] != ':') {
            //p->state=WTK_nlgnet_PARSER_STATE_WAIT_COLON;
            wtk_nlgnet_parser_set_state(p, WTK_nlgnet_PARSER_STATE_WAIT_COLON);
            p->nxt_colon_state = WTK_nlgnet_PARSER_STATE_WAIT_INFO;
        } else {
            //p->state=WTK_nlgnet_PARSER_STATE_WAIT_INFO;
            wtk_nlgnet_parser_set_state(p, WTK_nlgnet_PARSER_STATE_WAIT_INFO);
        }
    } else {
        wtk_strbuf_push(p->buf, v->data, v->len);
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state_colon(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    if (v->len == 1 && (v->data[0] == '\n' || v->data[0] == ':')) {
        p->state = p->nxt_colon_state;
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state_wait_info(wtk_nlgnet_parser_t *p,
        wtk_string_t *v)
{
    //wtk_debug("sub=%d\n",p->sub_state);
    if (p->sub_state == 0) {
        if (v->len == 1 && v->data[0] == '\n') {
            p->sub_state = -1;
        }
        return 0;
    }
    if (v->len > 1 || (!isspace(v->data[0]))) {
        if (v->len == 1 && v->data[0] == '#') {
            p->sub_state = 0;
        } else {
            wtk_strbuf_reset(p->buf);
            p->state = WTK_nlgnet_PARSER_STATE_INFO;
            return wtk_nlgnet_parser_feed(p, v);
        }
    } else if (v->data[0] == '\n') {
        if (p->s->emit) {
            //wtk_nlgnet_state_print(p->s);
            p->s = NULL;
            p->state = WTK_nlgnet_PARSER_INIT;
            p->sub_state = -1;
        }
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state_info(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    //wtk_debug("[%.*s]=[%.*s]\n",p->buf->pos,p->buf->data,v->len,v->data);
    if (v->len == 1 && (isspace(v->data[0]) || v->data[0] == ':')) {
        //wtk_debug("[%.*s]\n",p->buf->pos,p->buf->data);
        //wtk_debug("[%.*s]\n",v->len,v->data);
        if (wtk_str_equal_s(p->buf->data, p->buf->pos, "OUTPUT")) {
            //wtk_debug("[%.*s]\n",p->buf->pos,p->buf->data);
            //exit(0);
            if (v->data[0] == ':') {
                p->state = WTK_nlgnet_PARSER_STATE_INFO_INPUT;
            } else {
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_COLON;
                p->nxt_colon_state = WTK_nlgnet_PARSER_STATE_INFO_INPUT;
            }
            p->sub_state = -1;
        } else if (wtk_str_equal_s(p->buf->data, p->buf->pos, "EMIT")) {
            if (v->data[0] == ':') {
                p->state = WTK_nlgnet_PARSER_STATE_INFO_EMIT;
            } else {
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_COLON;
                p->nxt_colon_state = WTK_nlgnet_PARSER_STATE_INFO_EMIT;
            }
            p->sub_state = -1;
        } else if (wtk_str_equal_s(p->buf->data, p->buf->pos, "PRE")) {
            if (v->data[0] == ':') {
                p->state = WTK_nlgnet_PARSER_STATE_INFO_PRE;
            } else {
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_COLON;
                p->nxt_colon_state = WTK_nlgnet_PARSER_STATE_INFO_PRE;
            }
            p->sub_state = -1;
        } else if (wtk_str_equal_s(p->buf->data, p->buf->pos, "POST")) {
            if (v->data[0] == ':') {
                p->state = WTK_nlgnet_PARSER_STATE_INFO_POST;
            } else {
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_COLON;
                p->nxt_colon_state = WTK_nlgnet_PARSER_STATE_INFO_POST;
            }
            p->sub_state = -1;
        } else if (wtk_str_equal_s(p->buf->data, p->buf->pos, "EPS")) {
            //wtk_debug("==============>\n");
            if (v->data[0] == ':') {
                p->state = WTK_nlgnet_PARSER_STATE_INFO_ESP;
            } else {
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_COLON;
                p->nxt_colon_state = WTK_nlgnet_PARSER_STATE_INFO_ESP;
            }
            p->sub_state = -1;
        } else {
            p->s = wtk_nlgnet_new_state(p->fst, p->buf->data, p->buf->pos);
            if (v->data[0] != ':') {
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_COLON;
                p->nxt_colon_state = WTK_nlgnet_PARSER_STATE_WAIT_INFO;
            } else {
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_INFO;
            }
            p->sub_state = -1;
        }
    } else {
        wtk_strbuf_push(p->buf, v->data, v->len);
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state_post(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    enum wtk_nlgnet_post_state_t {
        WTK_NLGNET_POST_INIT = -1, WTK_NLGNET_POST_WRD,
    };
    wtk_strbuf_t *buf = p->buf;

    switch (p->sub_state) {
        case WTK_NLGNET_POST_INIT:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                wtk_strbuf_push(buf, v->data, v->len);
                p->sub_state = WTK_NLGNET_POST_WRD;
            }
            break;
        case WTK_NLGNET_POST_WRD:
            if (v->len == 1 && isspace(v->data[0])) {
                p->s->post = wtk_heap_dup_str2(p->fst->heap, buf->data,
                        buf->pos);
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_INFO;
                if (v->len == 1 && v->data[0] == '\n') {
                    return wtk_nlgnet_parser_feed(p, v);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state_pre(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    enum wtk_nlgnet_pre_state_t {
        WTK_NLGNET_PRE_INIT = -1, WTK_NLGNET_PRE_WRD,
    };
    wtk_strbuf_t *buf = p->buf;

    switch (p->sub_state) {
        case WTK_NLGNET_PRE_INIT:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                wtk_strbuf_push(buf, v->data, v->len);
                p->sub_state = WTK_NLGNET_PRE_WRD;
            }
            break;
        case WTK_NLGNET_PRE_WRD:
            if (v->len == 1 && isspace(v->data[0])) {
                p->s->pre = wtk_heap_dup_str2(p->fst->heap, buf->data,
                        buf->pos);
                p->state = WTK_nlgnet_PARSER_STATE_WAIT_INFO;
                if (v->len == 1 && v->data[0] == '\n') {
                    return wtk_nlgnet_parser_feed(p, v);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state_emit(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    enum wtk_nlgnet_emit_state_t {
        WTK_nlgnet_EMIT_WAIT = -1, WTK_nlgnet_EMIT_BODY, WTK_NLGNET_EMIT_ESC,
    };
    wtk_strbuf_t *buf = p->buf;

    //wtk_debug("[%.*s]\n",v->len,v->data);
    switch (p->sub_state) {
        case WTK_nlgnet_EMIT_WAIT:
            if (v->len > 1 || !isspace(v->data[0])) {
                if (v->data[0] == '\"') {
                    p->sub_state = WTK_NLGNET_EMIT_ESC;
                    wtk_strbuf_reset(buf);
                } else {
                    p->sub_state = WTK_nlgnet_EMIT_BODY;
                    wtk_strbuf_reset(buf);
                    wtk_strbuf_push(buf, v->data, v->len);
                }
            }
            break;
        case WTK_NLGNET_EMIT_ESC:
            if (v->len == 1 && v->data[0] == '\"') {
                p->s->emit = wtk_heap_dup_string(p->fst->heap, buf->data,
                        buf->pos);
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                //exit(0);
                //p->state=WTK_nlgnet_PARSER_STATE_WAIT_INFO;
                wtk_nlgnet_parser_set_state(p,
                        WTK_nlgnet_PARSER_STATE_WAIT_INFO);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_nlgnet_EMIT_BODY:
            if (v->len == 1 && v->data[0] == '\n')		// isspace(v->data[0]))
                    {
                wtk_strbuf_strip(buf);
                p->s->emit = wtk_heap_dup_string(p->fst->heap, buf->data,
                        buf->pos);
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                //exit(0);
                //p->state=WTK_nlgnet_PARSER_STATE_WAIT_INFO;
                wtk_nlgnet_parser_set_state(p,
                        WTK_nlgnet_PARSER_STATE_WAIT_INFO);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_nlgnet_parser_feed_state_eps(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    enum wtk_nlgnet_emit_state_t {
        WTK_nlgnet_EPS_WAIT = -1, WTK_nlgnet_EPS_BODY,
    };
    wtk_strbuf_t *buf = p->buf;

    //wtk_debug("[%.*s]\n",v->len,v->data);
    switch (p->sub_state) {
        case WTK_nlgnet_EPS_WAIT:
            if (v->len > 1 || !isspace(v->data[0])) {
                p->sub_state = WTK_nlgnet_EPS_BODY;
                wtk_strbuf_reset(buf);
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_nlgnet_EPS_BODY:
            if (v->len == 1 && v->data[0] == '\n')		//isspace(v->data[0]))
                    {
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                wtk_strbuf_strip(buf);
                p->s->eps = wtk_nlgnet_get_state(p->fst, buf->data, buf->pos,
                        1);
                //p->state=WTK_nlgnet_PARSER_STATE_WAIT_INFO;
                wtk_nlgnet_parser_set_state(p,
                        WTK_nlgnet_PARSER_STATE_WAIT_INFO);
                //exit(0);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

wtk_nlgnet_arc_t* wtk_nlgnet_new_arc(wtk_nlgnet_t *fst,
        wtk_nlgnet_state_t *from)
{
    wtk_nlgnet_arc_t *arc;

    arc = (wtk_nlgnet_arc_t*) wtk_heap_malloc(fst->heap,
            sizeof(wtk_nlgnet_arc_t));
    wtk_queue_init(&(arc->attr_q));
    wtk_queue_init(&(arc->to_q));
    arc->from = from;
    arc->min_round = 0;
    arc->max_round = -1;
    arc->min_conf = -1e6;
    arc->match_all_slot = 1;
    arc->use_emit = 1;
    arc->or = 0;
    arc->clean_ctx = 0;
    arc->emit_func = NULL;
    arc->emit_item = NULL;
    arc->other = NULL;
    arc->skip_fld = 0;
    arc->domained = 1;
    arc->must_domain = 0;
    arc->playing = 0;
    arc->fail_re_doamin = 0;
    return arc;
}

wtk_nlgnet_arc_attr_t* wtk_nlgnet_new_arc_attr(wtk_heap_t *heap,
        wtk_string_t *k, wtk_string_t *v)
{
    wtk_nlgnet_arc_attr_t *a;

    a = (wtk_nlgnet_arc_attr_t*) wtk_heap_malloc(heap,
            sizeof(wtk_nlgnet_arc_attr_t));
    a->k = wtk_heap_dup_string(heap, k->data, k->len);
    if (v) {
        a->v = wtk_heap_dup_string(heap, v->data, v->len);
    } else {
        a->v = NULL;
    }
    return a;
}

void wtk_nlgnet_parser_notify_arc_attr(wtk_nlgnet_parser_t *p, wtk_string_t *k,
        wtk_string_t *v)
{
    wtk_nlgnet_arc_attr_t *a;

//	printf("[%.*s]",k->len,k->data);
//	if(v)
//	{
//		printf("=[%.*s]",v->len,v->data);
//	}
//	printf("\n");
    if (wtk_string_cmp_s(k,"nil") == 0) {
        return;
    }
    a = wtk_nlgnet_new_arc_attr(p->fst->heap, k, v);
    wtk_queue_push(&(p->arc->attr_q), &(a->q_n));
}

void wtk_nlgnet_parser_notify_arc_attr2(wtk_nlgnet_parser_t *p, wtk_string_t *k,
        wtk_string_t *v)
{
    wtk_nlgnet_arc_t *arc;

    arc = p->arc;
    if (wtk_string_cmp_s(k,"min_round") == 0) {
        arc->min_round = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"max_round") == 0) {
        arc->max_round = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"match_all_slot") == 0) {
        arc->match_all_slot = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"use_emit") == 0) {
        arc->use_emit = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"or") == 0) {
        arc->or = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"clean_ctx") == 0) {
        arc->clean_ctx = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"emit") == 0) {
        arc->emit_func = wtk_heap_dup_string(p->fst->heap, v->data, v->len);
    } else if (wtk_string_cmp_s(k,"skip_fld") == 0) {
        arc->skip_fld = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"domained") == 0) {
        arc->domained = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"min_conf") == 0) {
        arc->min_conf = wtk_str_atof(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"playing") == 0) {
        arc->playing = 1;
    } else if (wtk_string_cmp_s(k,"other") == 0) {
        arc->other = wtk_nlgnet_get_state(p->fst, v->data, v->len, 1);
    } else if (wtk_string_cmp_s(k,"fail_re_doamin") == 0) {
        arc->fail_re_doamin = wtk_str_atoi(v->data, v->len);
    } else if (wtk_string_cmp_s(k,"must_domain") == 0) {
        arc->must_domain = wtk_str_atoi(v->data, v->len);
    }
}

void wtk_nlgnet_arc_add_to_state(wtk_nlgnet_t *fst, wtk_nlgnet_arc_t *arc,
        char *nm, int nm_bytes)
{
    wtk_nlgnet_state_t *state;
    wtk_nglnet_arc_to_item_t *item;

    if (wtk_str_equal_s(nm, nm_bytes, "$")) {
        state = arc->from;
    } else {
        state = wtk_nlgnet_get_state(fst, nm, nm_bytes, 1);
    }
    item = (wtk_nglnet_arc_to_item_t*) wtk_heap_malloc(fst->heap,
            sizeof(wtk_nglnet_arc_to_item_t));
    item->state = state;
    wtk_queue_push(&(arc->to_q), &(item->q_n));
}

int wtk_nlgnet_parser_feed_state_input(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    enum wtk_nlgnet_input_state_t {
        WTK_nlgnet_INPUT_WAIT = -1,
        WTK_nlgnet_INPUT_ARC_ATTR,
        WTK_nlgnet_INPUT_ARC,
        WTK_nlgnet_INPUT_ARC_END_HINT,
        WTK_nlgnet_INPUT_ARC_WAIT_NXT_STATE,
        WTK_nlgnet_INPUT_ARC_NXT_STATE,
        WTK_nlgnet_INPUT_ARC_NXT_STATE_HINT,
        WTK_nlgnet_INPUT_END_HINT,
        WTK_nlgnet_INPUT_COMMENT,
    };
    wtk_strbuf_t *buf = p->buf;
    wtk_nlgnet_arc_t *arc;
    char c;

    //if(v->data[0]=='\n')
//	{
//		wtk_debug("state=%d %.*s\n",p->sub_state,v->len,v->data);
//	}
    //wtk_debug("state=%d %.*s\n",p->sub_state,v->len,v->data);
    switch (p->sub_state) {
        case WTK_nlgnet_INPUT_WAIT:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                wtk_strbuf_push_s(buf, "[");
                wtk_strbuf_push(buf, v->data, v->len);
                p->sub_state = WTK_nlgnet_INPUT_ARC;
            } else if (v->data[0] == '#') {
                p->sub_state = WTK_nlgnet_INPUT_COMMENT;
            }
            break;
        case WTK_nlgnet_INPUT_COMMENT:
            if (v->len == 1 && v->data[0] == '\n') {
                p->sub_state = WTK_nlgnet_INPUT_WAIT;
            }
            break;
        case WTK_nlgnet_INPUT_ARC:
            if (v->len == 1 && v->data[0] == '=') {
                p->sub_state = WTK_nlgnet_INPUT_ARC_END_HINT;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_nlgnet_INPUT_ARC_END_HINT:
            if (v->len == 1 && v->data[0] == '>') {
                wtk_strbuf_strip(buf);
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                if (wtk_str_equal_s(buf->data, buf->pos, "[other")) {
                    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                    arc = wtk_nlgnet_new_arc(p->fst, p->s);
                    p->s->other = arc;
                    p->arc = arc;
                    p->sub_state = WTK_nlgnet_INPUT_ARC_WAIT_NXT_STATE;
                    //wtk_nlgnet_state_print(p->s);
                } else {
                    wtk_strbuf_push_s(buf, "]");
                    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                    arc = wtk_nlgnet_new_arc(p->fst, p->s);
                    wtk_queue_push(&(p->s->output_arc_q), &(arc->arc_n));
                    p->arc = arc;
                    wtk_str_attr_parse(buf->data, buf->pos, p,
                            (wtk_str_attr_f) wtk_nlgnet_parser_notify_arc_attr);
                    p->sub_state = WTK_nlgnet_INPUT_ARC_WAIT_NXT_STATE;
                }
            } else {
                wtk_strbuf_push_s(buf, "=");
                wtk_strbuf_push(buf, v->data, v->len);
                p->sub_state = WTK_nlgnet_INPUT_ARC;
            }
            break;
        case WTK_nlgnet_INPUT_ARC_WAIT_NXT_STATE:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                wtk_strbuf_push(buf, v->data, v->len);
                p->sub_state = WTK_nlgnet_INPUT_ARC_NXT_STATE;
            }
            break;
        case WTK_nlgnet_INPUT_ARC_NXT_STATE:
            if (v->len == 1) {
                c = v->data[0];
                //wtk_debug("[%.*s] c=%c\n",buf->pos,buf->data,c);
                if (c == '|') {
                    wtk_nlgnet_arc_add_to_state(p->fst, p->arc, buf->data,
                            buf->pos);
                    p->sub_state = WTK_nlgnet_INPUT_ARC_WAIT_NXT_STATE;
                } else if (isspace(c)) {
                    wtk_nlgnet_arc_add_to_state(p->fst, p->arc, buf->data,
                            buf->pos);
                    if (c == '\n') {
                        p->sub_state = WTK_nlgnet_INPUT_END_HINT;
                    } else {
                        p->sub_state = WTK_nlgnet_INPUT_ARC_NXT_STATE_HINT;
                    }
                } else if (c == '/' && buf->data[buf->pos - 1] != '<') {
                    wtk_nlgnet_arc_add_to_state(p->fst, p->arc, buf->data,
                            buf->pos);
                    p->sub_state = WTK_nlgnet_INPUT_ARC_ATTR;
                    wtk_strbuf_reset(buf);
                    wtk_strbuf_push_s(buf, "[");
                } else {
                    wtk_strbuf_push(buf, v->data, v->len);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_nlgnet_INPUT_ARC_NXT_STATE_HINT:
            if (v->len == 1) {
                c = v->data[0];
                if (c == '|') {
                    p->sub_state = WTK_nlgnet_INPUT_ARC_WAIT_NXT_STATE;
                } else if (c == '\n') {
                    p->sub_state = WTK_nlgnet_INPUT_END_HINT;
                } else if (c == '/') {
                    p->sub_state = WTK_nlgnet_INPUT_ARC_ATTR;
                    wtk_strbuf_reset(buf);
                    wtk_strbuf_push_s(buf, "[");
                }
            }
            break;
        case WTK_nlgnet_INPUT_ARC_ATTR:
            if (v->len == 1 && v->data[0] == '/') {
                wtk_strbuf_push_s(buf, "]");
                //wtk_debug("%.*s\n",buf->pos,buf->data);
                wtk_str_attr_parse(buf->data, buf->pos, p,
                        (wtk_str_attr_f) wtk_nlgnet_parser_notify_arc_attr2);
                p->sub_state = WTK_nlgnet_INPUT_ARC_NXT_STATE_HINT;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_nlgnet_INPUT_END_HINT:
            if (v->len == 1 && v->data[0] == '\n') {
                //wtk_nlgnet_state_print(p->s);
                p->s = NULL;
                p->state = WTK_nlgnet_PARSER_INIT;
                p->sub_state = -1;
                //exit(0);
            } else {
                p->sub_state = WTK_nlgnet_INPUT_WAIT;
                return wtk_nlgnet_parser_feed_state_input(p, v);
            }
            break;
    }
    return 0;
}

int wtk_nlgnet_parser_feed(wtk_nlgnet_parser_t *p, wtk_string_t *v)
{
    int ret;

    switch (p->state) {
        case WTK_nlgnet_PARSER_INIT:
            ret = wtk_nlgnet_parser_feed_init(p, v);
            break;
        case WTK_nlgnet_PARSER_COMMENT:
            if (v->len == 1 && v->data[0] == '\n') {
                p->state = WTK_nlgnet_PARSER_INIT;
            }
            ret = 0;
            break;
        case WTK_nlgnet_PARSER_STATE:
            ret = wtk_nlgnet_parser_feed_state(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_WAIT_COLON:
            ret = wtk_nlgnet_parser_feed_state_colon(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_WAIT_INFO:
            ret = wtk_nlgnet_parser_feed_state_wait_info(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_INFO:
            ret = wtk_nlgnet_parser_feed_state_info(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_INFO_EMIT:
            ret = wtk_nlgnet_parser_feed_state_emit(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_INFO_ESP:
            ret = wtk_nlgnet_parser_feed_state_eps(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_INFO_INPUT:
            ret = wtk_nlgnet_parser_feed_state_input(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_INFO_PRE:
            ret = wtk_nlgnet_parser_feed_state_pre(p, v);
            break;
        case WTK_nlgnet_PARSER_STATE_INFO_POST:
            ret = wtk_nlgnet_parser_feed_state_post(p, v);
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

void wtk_nlgnet_update_root(wtk_nlgnet_t *fst)
{
    wtk_nlgnet_state_t *state;
    wtk_queue_node_t *qn;

    for (qn = fst->state_q.pop; qn; qn = qn->next) {
        state = data_offset2(qn, wtk_nlgnet_state_t, q_n);
        if (wtk_string_cmp_s(state->name,"<s>") == 0) {
            fst->root = state;
            if (fst->end) {
                return;
            }
        } else if (wtk_string_cmp_s(state->name,"</s>") == 0) {
            fst->end = state;
            if (fst->root) {
                return;
            }
        }
    }
}

int wtk_nlgnet_load(wtk_nlgnet_t *fst, wtk_source_t *src)
{
    wtk_strbuf_t *buf;
    int ret;
    wtk_nlgnet_parser_t parser;
    wtk_string_t v;

    buf = wtk_strbuf_new(256, 1);
    parser.fst = fst;
    parser.buf = wtk_strbuf_new(256, 1);
    parser.state = WTK_nlgnet_PARSER_INIT;
    //wtk_source_read_utf8_char
    while (1) {
        ret = wtk_source_read_utf8_char(src, buf);
        if (ret != 0) {
            ret = 0;
            break;
        }
        //wtk_debug("v[%d]=[%.*s]\n",parser.state,buf->pos,buf->data);
        wtk_string_set(&(v), buf->data, buf->pos);
        wtk_nlgnet_parser_feed(&(parser), &v);
        //exit(0);
    }
    wtk_string_set_s(&v, "\n");
    wtk_nlgnet_parser_feed(&(parser), &v);
    wtk_nlgnet_update_root(fst);
    ret = 0;
    wtk_strbuf_delete(parser.buf);
    wtk_strbuf_delete(buf);
    //wtk_nlgnet_print(fst);
    //exit(0);
    return ret;
}

wtk_nlgnet_t* wtk_nlgnet_new(wtk_rbin2_t *rbin, char *fn)
{
    wtk_nlgnet_t *fst;

    fst = (wtk_nlgnet_t*) wtk_malloc(sizeof(wtk_nlgnet_t));
    fst->heap = wtk_heap_new(4096);
    fst->root = NULL;
    fst->end = NULL;
    wtk_queue_init(&(fst->state_q));
    if (rbin) {
        wtk_rbin2_load_file(rbin, fst,
                (wtk_source_load_handler_t) wtk_nlgnet_load, fn);
    } else {
        wtk_source_load_file(fst, (wtk_source_load_handler_t) wtk_nlgnet_load,
                fn);
    }
    //wtk_nlgnet_print(fst);
    //exit(0);
    return fst;
}

wtk_nlgnet_t* wtk_nlgnet_new2(wtk_heap_t *heap, char *data, int len)
{
    wtk_nlgnet_t *fst;
    wtk_source_t src;
    int ret;

    fst = (wtk_nlgnet_t*) wtk_heap_malloc(heap, sizeof(wtk_nlgnet_t));
    fst->heap = heap;
    fst->root = NULL;
    fst->end = NULL;
    wtk_queue_init(&(fst->state_q));

    wtk_source_init_str(&(src), data, len);
    ret = wtk_nlgnet_load(fst, &(src));
    wtk_source_clean_str(&(src));
    if (ret != 0) {
        wtk_nlgnet_delete(fst);
        fst = NULL;
    }
    return fst;
}

void wtk_nlgnet_delete(wtk_nlgnet_t *fst)
{
    wtk_heap_delete(fst->heap);
    wtk_free(fst);
}

void wtk_nlgnet_print(wtk_nlgnet_t *net)
{
    wtk_nlgnet_state_t *state;
    wtk_queue_node_t *qn;

    for (qn = net->state_q.pop; qn; qn = qn->next) {
        state = data_offset2(qn, wtk_nlgnet_state_t, q_n);
        wtk_nlgnet_state_print(state);
    }
}

int wtk_nlgnet_bind(wtk_nlgnet_t *net, wtk_nlg_t *nlg)
{
    wtk_nlgnet_state_t *state;
    wtk_nlg_root_t *root;
    wtk_queue_node_t *qn, *qn2;
    wtk_nlgnet_arc_t *arc;
    int ret;

    for (qn = net->state_q.pop; qn; qn = qn->next) {
        state = data_offset2(qn, wtk_nlgnet_state_t, q_n);
        if (state->emit) {
            //wtk_debug("[%.*s]\n",state->emit->len,state->emit->data);
            if (state->emit->data[state->emit->len - 1] == ')') {
                state->emit_item = wtk_nlg_get3(nlg, state->emit->data,
                        state->emit->len);
            } else {
                root = wtk_nlg_get_root(nlg, state->emit->data,
                        state->emit->len, 0);
                if (root && root->item) {
                    state->emit_item = root->item;
                }
            }
            if (!state->emit_item) {
                wtk_debug("[%.*s] not found\n", state->emit->len,
                        state->emit->data);
                ret = -1;
                goto end;
            }
        }
        for (qn2 = state->output_arc_q.pop; qn2; qn2 = qn2->next) {
            arc = data_offset2(qn2, wtk_nlgnet_arc_t, arc_n);
            if (arc->emit_func) {
                if (arc->emit_func->data[arc->emit_func->len - 1] == ')') {
                    arc->emit_item = wtk_nlg_get3(nlg, arc->emit_func->data,
                            arc->emit_func->len);
                } else {
                    root = wtk_nlg_get_root(nlg, arc->emit_func->data,
                            arc->emit_func->len, 0);
                    if (root && root->item) {
                        arc->emit_item = root->item;
                    }
                }
                if (!arc->emit_item) {
                    wtk_debug("[%.*s] not found\n", arc->emit_func->len,
                            arc->emit_func->data);
                    ret = -1;
                    goto end;
                }
            }
        }
        if (state->other && state->other->emit_func) {
            arc = state->other;
            if (arc->emit_func->data[arc->emit_func->len - 1] == ')') {
                arc->emit_item = wtk_nlg_get3(nlg, arc->emit_func->data,
                        arc->emit_func->len);
            } else {
                root = wtk_nlg_get_root(nlg, arc->emit_func->data,
                        arc->emit_func->len, 0);
                if (root && root->item) {
                    arc->emit_item = root->item;
                }
            }
        }
//		wtk_debug("root=%p/%p\n",root,root->item);
//		wtk_nlg_item_print(root->item);
//		exit(0);
    }
    ret = 0;
    end: return ret;
}

wtk_nlgnet_state_t* wtk_nlgnet_arc_next(wtk_nlgnet_arc_t *arc)
{
    wtk_nglnet_arc_to_item_t *item;
    wtk_queue_node_t *qn;
    int idx;

    if (arc->to_q.length <= 0) {
        return NULL;
    } else if (arc->to_q.length == 1) {
        item = data_offset2(arc->to_q.pop, wtk_nglnet_arc_to_item_t, q_n);
        return item->state;
    } else {
        srand(time_get_ms());
        idx = rand() % (arc->to_q.length);
        qn = wtk_queue_peek(&(arc->to_q), idx);
        item = data_offset2(qn, wtk_nglnet_arc_to_item_t, q_n);
        return item->state;
    }
}
