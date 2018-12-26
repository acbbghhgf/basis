#include "wtk_rexpr_parser.h" 
#include <ctype.h>

wtk_rexpr_parser_t* wtk_rexpr_parser_new()
{
    wtk_rexpr_parser_t *p;

    p = (wtk_rexpr_parser_t*) wtk_malloc(sizeof(wtk_rexpr_parser_t));
    p->buf = wtk_strbuf_new(256, 1);
    p->stk_heap = wtk_heap_new(4096);
    wtk_rexpr_parser_reset(p);
    return p;
}

void wtk_rexpr_parser_delete(wtk_rexpr_parser_t *p)
{
    wtk_heap_delete(p->stk_heap);
    wtk_strbuf_delete(p->buf);
    wtk_free(p);
}

void wtk_rexpr_parser_reset(wtk_rexpr_parser_t *p)
{
    wtk_heap_reset(p->stk_heap);
    wtk_queue_init(&(p->stk_q));
    wtk_strbuf_reset(p->buf);
    p->state = WTK_REXPR_INIT;
    p->sub_state = -1;
}

void wtk_rexpr_parser_set_state(wtk_rexpr_parser_t *p, wtk_rexpr_state_t state)
{
    p->state = state;
    p->sub_state = -1;
}

int wtk_rexpr_parser_feed_init(wtk_rexpr_parser_t *p, wtk_string_t *v)
{
    if (v->len > 1 || !isspace(v->data[0])) {
        wtk_strbuf_reset(p->buf);
        wtk_strbuf_push(p->buf, v->data, v->len);
        wtk_rexpr_parser_set_state(p, WTK_REXPR_NAME);
    }
    return 0;
}

void wtk_rexpr_parser_push_stack(wtk_rexpr_parser_t *p, wtk_queue3_t *q)
{
    wtk_heap_t *heap = p->stk_heap;
    wtk_rexpr_stack_t *s;

    s = (wtk_rexpr_stack_t*) wtk_heap_malloc(heap, sizeof(wtk_rexpr_stack_t));
    s->v.item_q = q;
    s->is_cap = 0;
    wtk_queue_push(&(p->stk_q), &(s->q_n));
}

void wtk_rexpr_parser_push_stack_cap(wtk_rexpr_parser_t *p,
        wtk_rexpr_cap_t *cap)
{
    wtk_heap_t *heap = p->stk_heap;
    wtk_rexpr_stack_t *s;

    s = (wtk_rexpr_stack_t*) wtk_heap_malloc(heap, sizeof(wtk_rexpr_stack_t));
    s->v.cap = cap;
    s->is_cap = 1;
    wtk_queue_push(&(p->stk_q), &(s->q_n));
}

void wtk_rexpr_parser_pop_stack(wtk_rexpr_parser_t *p)
{
    wtk_queue_pop_back(&(p->stk_q));
}

wtk_rexpr_item_t* wtk_rexpr_parser_get_last_item(wtk_rexpr_parser_t *p)
{
    wtk_rexpr_stack_t *stk;
    wtk_rexpr_item_t *item;
    wtk_rexpr_branch_t *b;

    stk = data_offset2(p->stk_q.push, wtk_rexpr_stack_t, q_n);
    if (stk->is_cap) {
        b = data_offset2(stk->v.cap->or_q.push, wtk_rexpr_branch_t, q_n);
        item = data_offset2(b->item_q.push, wtk_rexpr_item_t, q_n);
    } else {
        item = data_offset2(stk->v.item_q->push, wtk_rexpr_item_t, q_n);
    }
    return item;
}

wtk_queue3_t* wtk_rexpr_parser_get_last_branch_q(wtk_rexpr_parser_t *p)
{
    wtk_rexpr_stack_t *stk;
    wtk_rexpr_branch_t *b;

    stk = data_offset2(p->stk_q.push, wtk_rexpr_stack_t, q_n);
    if (stk->is_cap) {
        b = data_offset2(stk->v.cap->or_q.push, wtk_rexpr_branch_t, q_n);
        return &(b->item_q);
    } else {
        return stk->v.item_q;
    }
}

wtk_rexpr_item_t* wtk_rexpr_parser_add_item_string(wtk_rexpr_parser_t *p,
        char *data, int len)
{
    wtk_rexpr_item_t *item;
    wtk_queue3_t *q;

    q = wtk_rexpr_parser_get_last_branch_q(p);
    item = wtk_rexpr_new_item_string(p->heap, data, len);
    wtk_queue3_push(q, &(item->q_n));
    return item;
}

wtk_rexpr_item_t* wtk_rexpr_parser_add_item_var(wtk_rexpr_parser_t *p,
        char *data, int len)
{
    wtk_queue3_t *q;
    wtk_rexpr_item_t *item;

    q = wtk_rexpr_parser_get_last_branch_q(p);
    item = wtk_rexpr_new_item_var(p->heap, data, len);
    wtk_queue3_push(q, &(item->q_n));
    return item;
}

wtk_rexpr_item_t* wtk_rexpr_parser_add_branch(wtk_rexpr_parser_t *p)
{
    wtk_rexpr_item_t *item;
    wtk_rexpr_branch_t *b;
    wtk_heap_t *heap = p->heap;
    wtk_queue3_t *q;

    item = wtk_rexpr_new_item_cap(heap);
    q = wtk_rexpr_parser_get_last_branch_q(p);
    wtk_queue3_push(q, &(item->q_n));

    b = wtk_rexpr_branch_new(heap);
    wtk_queue3_push(&(item->v.cap->or_q), &(b->q_n));
    wtk_rexpr_parser_push_stack_cap(p, item->v.cap);
    return item;
}

int wtk_rexpr_parser_add_branch2(wtk_rexpr_parser_t *p)
{
    wtk_rexpr_stack_t *stk;
    wtk_heap_t *heap = p->heap;
    wtk_rexpr_branch_t *b;
    wtk_rexpr_item_t *item;

    stk = data_offset2(p->stk_q.push, wtk_rexpr_stack_t, q_n);
    if (stk->is_cap) {
        //wtk_rexpr_print(p->expr);
        b = wtk_rexpr_branch_new(heap);
        wtk_queue3_push(&(stk->v.cap->or_q), &(b->q_n));
    } else {
        if (p->stk_q.length == 1) {
            //wtk_rexpr_print(p->expr);
            item = wtk_rexpr_new_item_cap(heap);
            b = wtk_rexpr_branch_new(heap);
            wtk_queue3_push(&(item->v.cap->or_q), &(b->q_n));
            b->item_q = *(stk->v.item_q);
            b = wtk_rexpr_branch_new(heap);
            wtk_queue3_push(&(item->v.cap->or_q), &(b->q_n));
            stk->is_cap = 1;
            stk->v.cap = item->v.cap;
            wtk_queue3_init(&(p->expr->item_q));
            wtk_queue3_push(&(p->expr->item_q), &(item->q_n));
            //exit(0);
        } else {
            return -1;
        }
    }
    return 0;
}

int wtk_rexpr_parser_feed_name(wtk_rexpr_parser_t *p, wtk_string_t *v)
{
    if (v->len == 1 && v->data[0] == '=') {
        wtk_strbuf_strip(p->buf);
        //wtk_debug("[%.*s]\n",p->buf->pos,p->buf->data);
        p->expr->name = wtk_heap_dup_string(p->heap, p->buf->data, p->buf->pos);
        wtk_rexpr_parser_set_state(p, WTK_EXPR_ITEM);
        wtk_rexpr_parser_push_stack(p, &(p->expr->item_q));
        //exit(0);
    } else {
        wtk_strbuf_push(p->buf, v->data, v->len);
    }
    return 0;
}

//\d  匹配任何十进制数；它相当于类 [0-9]。
//string
//var
//(你|我)
int wtk_rexpr_parser_feed_item(wtk_rexpr_parser_t *p, wtk_string_t *v)
{
    enum {
        WTK_REXPR_ITEM_INIT = -1,
        WTK_REXPR_ITEM_STR,
        WTK_REXPR_ITEM_ESC,
        WTK_REXPR_ITEM_VAR,
        WTK_REXPR_ITEM_ATTR,
    };
    wtk_strbuf_t *buf = p->buf;
    char c;
    wtk_rexpr_item_t *item;
    wtk_var_parse_t *var = &(p->varparser);
    int ret;

    switch (p->sub_state) {
        case WTK_REXPR_ITEM_INIT:
            wtk_strbuf_reset(buf);
            p->sub_state = WTK_REXPR_ITEM_STR;
            return wtk_rexpr_parser_feed_item(p, v);
            break;
        case WTK_REXPR_ITEM_STR:
            if (v->len > 1) {
                //push string;
                wtk_strbuf_push(buf, v->data, v->len);
            } else {
                c = v->data[0];
                switch (c) {
                    case ';':
                        if (buf->pos > 0) {
                            wtk_rexpr_parser_add_item_string(p, buf->data,
                                    buf->pos);
                            wtk_strbuf_reset(buf);
                        }
                        break;
                    case '\n':
                        if (buf->pos > 0) {
                            wtk_rexpr_parser_add_item_string(p, buf->data,
                                    buf->pos);
                            wtk_strbuf_reset(buf);
                        }
                        break;
                    case '\\':
                        p->sub_state = WTK_REXPR_ITEM_ESC;
                        break;
                    case '$':
                        if (buf->pos > 0) {
                            wtk_rexpr_parser_add_item_string(p, buf->data,
                                    buf->pos);
                            wtk_strbuf_reset(buf);
                        }
                        p->sub_state = WTK_REXPR_ITEM_VAR;
                        wtk_var_parse_init(var, p->buf);
                        break;
                    case '?':
                        if (buf->pos > 0) {
                            item = wtk_rexpr_parser_add_item_string(p,
                                    buf->data, buf->pos);
                            wtk_strbuf_reset(buf);
                        } else {
                            item = wtk_rexpr_parser_get_last_item(p);
                        }
                        item->repeat_min = 0;
                        item->repeat_max = 1;
                        break;
                    case '*':
                        if (buf->pos > 0) {
                            item = wtk_rexpr_parser_add_item_string(p,
                                    buf->data, buf->pos);
                            wtk_strbuf_reset(buf);
                        } else {
                            item = wtk_rexpr_parser_get_last_item(p);
                            if (!item) {
                                return -1;
                            }
                        }
                        item->repeat_min = 0;
                        item->repeat_max = -1;
                        break;
                    case '+':
                        if (buf->pos > 0) {
                            item = wtk_rexpr_parser_add_item_string(p,
                                    buf->data, buf->pos);
                            wtk_strbuf_reset(buf);
                        } else {
                            item = wtk_rexpr_parser_get_last_item(p);
                        }
                        item->repeat_min = 1;
                        item->repeat_max = -1;
                        break;
                    case '/':
                        if (buf->pos > 0) {
                            wtk_rexpr_parser_add_item_string(p, buf->data,
                                    buf->pos);
                            wtk_strbuf_reset(buf);
                        }
                        p->sub_state = WTK_REXPR_ITEM_ATTR;
                        break;
                    case '(':
                        if (buf->pos > 0) {
                            wtk_rexpr_parser_add_item_string(p, buf->data,
                                    buf->pos);
                            wtk_strbuf_reset(buf);
                        }
                        wtk_rexpr_parser_add_branch(p);
                        break;
                    case ')':
                        if (buf->pos > 0) {
                            wtk_rexpr_parser_add_item_string(p, buf->data,
                                    buf->pos);
                            wtk_strbuf_reset(buf);
                        }
                        wtk_rexpr_parser_pop_stack(p);
                        break;
                    case '|':
                        if (buf->pos > 0) {
                            wtk_rexpr_parser_add_item_string(p, buf->data,
                                    buf->pos);
                            wtk_strbuf_reset(buf);
                        }
                        ret = wtk_rexpr_parser_add_branch2(p);
                        return ret;
                        break;
                    default:
                        if (buf->pos == 0 && v->len == 1 && isspace(v->data[0])) {

                        } else {
                            wtk_strbuf_push(buf, v->data, v->len);
                        }
                        break;
                }
            }
            break;
        case WTK_REXPR_ITEM_ESC:
            if (v->len == 1 && v->data[0] == 'd') {
                wtk_debug("[%.*s]\n", v->len, v->data);
                wtk_strbuf_reset(buf);
                exit(0);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            p->sub_state = WTK_REXPR_ITEM_STR;
            break;
        case WTK_REXPR_ITEM_VAR:
            ret = wtk_var_parse(var, v->data, v->len);
            if (ret == -1) {
                return ret;
            }
            if (ret == 1) {
                //wtk_debug("[%.*s]\n",p->buf->pos,p->buf->data);
                wtk_rexpr_parser_add_item_var(p, p->buf->data, p->buf->pos);
                wtk_strbuf_reset(buf);
                p->sub_state = WTK_REXPR_ITEM_STR;
            }
            return 0;
            break;
        case WTK_REXPR_ITEM_ATTR:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            if (v->len == 1 && v->data[0] == '/') {
                wtk_strkv_parser_t sp;
                int ret;

                item = wtk_rexpr_parser_get_last_item(p);
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                wtk_strkv_parser_init(&(sp), buf->data, buf->pos);
                while (1) {
                    ret = wtk_strkv_parser_next(&(sp));
                    if (ret != 0) {
                        break;
                    }
                    //wtk_debug("[%.*s]=[%.*s]\n",sp.k.len,sp.k.data,sp.v.len,sp.v.data);
                    if (wtk_str_equal_s(sp.k.data, sp.k.len, "min")) {
                        item->repeat_min = wtk_str_atoi(sp.v.data, sp.v.len);
                    } else if (wtk_str_equal_s(sp.k.data, sp.k.len, "max")) {
                        item->repeat_max = wtk_str_atoi(sp.v.data, sp.v.len);
                    }
                }
                wtk_strbuf_reset(buf);
                p->sub_state = WTK_REXPR_ITEM_STR;
                //exit(0);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_rexpr_parser_feed(wtk_rexpr_parser_t *p, wtk_string_t *v)
{
    int ret = -1;

    switch (p->state) {
        case WTK_REXPR_INIT:
            ret = wtk_rexpr_parser_feed_init(p, v);
            break;
        case WTK_REXPR_NAME:
            ret = wtk_rexpr_parser_feed_name(p, v);
            break;
        case WTK_EXPR_ITEM:
            ret = wtk_rexpr_parser_feed_item(p, v);
            break;
    }
    return ret;
}

wtk_rexpr_t* wtk_rexpr_parser_process(wtk_rexpr_parser_t *p, wtk_heap_t *heap,
        char *s, int len)
{
    char *e; //,*px;
    int n;
    int ret;
    wtk_string_t v;
    wtk_rexpr_t *expr = NULL;
    int b = 0;

    //wtk_debug("[%.*s]\n",len,s);
    p->heap = heap;
    p->expr = wtk_rexpr_new(heap);
    e = wtk_str_chr(s, len, '=');
    if (e) {
        if (wtk_str_equal_s(s, (int )(e - s), "main")) {
            b = 0;
        } else {
            b = 1;
        }
//		wtk_debug("[%.*s]\n",(int)(e-s),s);
//		exit(0);
//		px=s;
//		b=1;
//		while(px<e)
//		{
//			n=wtk_utf8_bytes(*px);
//			if(n==1 && isalnum(*px))
//			{
//				b=0;
//				break;
//			}
//			px+=n;
//		}
    } else {
        b = 0;
    }
    if (b == 1) {
        wtk_rexpr_parser_set_state(p, WTK_REXPR_INIT);
    } else {
        wtk_rexpr_parser_set_state(p, WTK_EXPR_ITEM);
        wtk_rexpr_parser_push_stack(p, &(p->expr->item_q));
    }
    e = s + len;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        v.data = s;
        v.len = n;
        //wtk_debug("[%.*s]\n",v.len,v.data);
        //wtk_debug("state=%d %.*s\n",p->state,v.len,v.data);
        ret = wtk_rexpr_parser_feed(p, &v);
        if (ret != 0) {
            goto end;
        }
        s += n;
    }
    v.data = "\n";
    v.len = 1;
    ret = wtk_rexpr_parser_feed(p, &v);
    if (ret != 0) {
        goto end;
    }
    //wtk_debug("stk=%d\n",p->stk_q.length);
    ret = 0;
    expr = p->expr;
    end: wtk_rexpr_parser_reset(p);
    return expr;
}
