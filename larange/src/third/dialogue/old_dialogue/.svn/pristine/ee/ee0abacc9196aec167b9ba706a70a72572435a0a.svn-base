#include "wtk_nlg2_parser.h"
#include <ctype.h>
int wtk_nlg2_paser_load_file(wtk_nlg2_parser_t *p, char *fn);

wtk_nlg2_parser_t* wtk_nlg2_parser_new()
{
    wtk_nlg2_parser_t *p;

    p = (wtk_nlg2_parser_t*) wtk_malloc(sizeof(wtk_nlg2_parser_t));
    p->heap = wtk_heap_new(4096);
    p->buf = wtk_strbuf_new(256, 1);
    p->strparser = wtk_string_parser2_new();
    p->args = wtk_larray_new(5, sizeof(void*));
    p->re_parser = wtk_rexpr_parser_new();
    wtk_nlg2_parser_reset(p);
    return p;
}

void wtk_nlg2_parser_delete(wtk_nlg2_parser_t *p)
{
    wtk_rexpr_parser_delete(p->re_parser);
    wtk_larray_delete(p->args);
    wtk_string_parser2_delete(p->strparser);
    wtk_heap_delete(p->heap);
    wtk_strbuf_delete(p->buf);
    wtk_free(p);
}

void wtk_nlg2_parser_reset(wtk_nlg2_parser_t *p)
{
    wtk_rexpr_parser_reset(p->re_parser);
    wtk_heap_reset(p->heap);
    wtk_strbuf_reset(p->buf);
    p->pwd = NULL;
    p->nlg = NULL;
    p->state = WTK_NLG2_INIT;
}

void wtk_nlg2_parser_set_state(wtk_nlg2_parser_t *p, wtk_nlg2_state_t state)
{
    p->state = state;
    p->sub_state = -1;
}

int wtk_nlg2_parser_feed(wtk_nlg2_parser_t *p, wtk_string_t *v);

int wtk_nlg2_parser_feed_init(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    if (v->len == 1 && v->data[0] == '#') {
        wtk_nlg2_parser_set_state(p, WTK_NLG2_COMMENT);
    } else if (v->len > 1 || !isspace(v->data[0])) {
        wtk_nlg2_parser_set_state(p, WTK_NLG2_ACTION);
        return wtk_nlg2_parser_feed(p, v);
    }
    return 0;
}

int wtk_nlg2_parser_feed_comment(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    wtk_strbuf_t *buf = p->buf;

    switch (p->sub_state) {
        case -1:
            wtk_strbuf_reset(buf);
            wtk_strbuf_push(buf, v->data, v->len);
            p->sub_state = 0;
            break;
        case 0:
            if (v->len == 1 && isspace(v->data[0])) {
                if (v->data[0] == '\n') {
                    wtk_nlg2_parser_set_state(p, WTK_NLG2_INIT);
                } else if (wtk_str_equal_s(buf->data, buf->pos, "include")) {
                    wtk_nlg2_parser_set_state(p, WTK_NLG2_INCLUDE);
                } else {
                    wtk_nlg2_parser_set_state(p, WTK_NLG2_INIT);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case 1:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_nlg2_parser_set_state(p, WTK_NLG2_INIT);
            }
            break;
    }
    return 0;
}

wtk_string_t* wtk_nlg2_parser_get_var(wtk_nlg2_parser_t *p, char *k, int k_len)
{
    if (wtk_str_equal_s(k, k_len, "pwd")) {
        return p->pwd;
    } else {
        return NULL;
    }
}

int wtk_nlg2_parser_feed_include(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    wtk_string_parser2_t * ps = p->strparser;
    int ret;

    if (p->sub_state == -1) {
        wtk_string_parser2_reset(ps);
        ps->ths = p;
        ps->get_var = (wtk_string_parser2_get_var_f) wtk_nlg2_parser_get_var;
        p->sub_state = 0;
    }
    ret = wtk_string_parse2(ps, v);
    switch (ret) {
        case 1:
            //end and input value is consumed
            //wtk_debug("[%.*s]\n",ps->buf->pos,ps->buf->data);
            wtk_strbuf_push_c(ps->buf, 0)
            ;
            wtk_nlg2_parser_set_state(p, WTK_NLG2_INIT);
            ret = wtk_nlg2_paser_load_file(p, ps->buf->data);
            //exit(0);
            return ret;
            break;
        case 2:
            //end and input value is not consumed
            //wtk_debug("[%.*s]\n",ps->buf->pos,ps->buf->data);
            wtk_strbuf_push_c(ps->buf, 0)
            ;
            wtk_nlg2_parser_set_state(p, WTK_NLG2_INIT);
            ret = wtk_nlg2_paser_load_file(p, ps->buf->data);
            if (ret != 0) {
                return ret;
            }
            ret = wtk_nlg2_parser_feed(p, v);
            break;
    }
    return ret;
}

int wtk_nlg2_parser_feed_action(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    wtk_strbuf_t *buf = p->buf;
    int ret;

    switch (p->sub_state) {
        case -1:
            wtk_strbuf_reset(buf);
            wtk_strbuf_push(buf, v->data, v->len);
            p->sub_state = 0;
            break;
        case 0:
            if (v->len == 1 && v->data[0] == '\n') {
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                p->item = wtk_nlg2_new_item(p->nlg);
                ret = wtk_nlg2_function_parse(&(p->item->function),
                        p->nlg->action_hash->heap, buf->data, buf->pos, NULL);
                if (ret != 0) {
                    return ret;
                }
                //wtk_nlg2_function_print(&(p->item->function));
                wtk_nlg2_add_item(p->nlg, p->item);
                //exit(0);
                wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_nlg2_parser_feed_item(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    enum {
        WTK_NLG2_ITEM_STATE_INIT = -1,
        WTK_NLG2_ITEM_STATE_COMMENT,
        WTK_NLG2_ITEM_STATE_END_HINT,
        WTK_NLG2_ITEM_STATE_COMMENT_WAIT_END,
        WTK_NLG2_ITEM_STATE_PRE,
        WTK_NLG2_ITEM_STATE_NXT,
        WTK_NLG2_ITEM_STATE_BEFORE,
    };
    wtk_strbuf_t *buf = p->buf;
    char c;

    switch (p->sub_state) {
        case WTK_NLG2_ITEM_STATE_INIT:
            if (v->len > 1) {
                wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM_EXPR);
                return wtk_nlg2_parser_feed(p, v);
            }
            c = v->data[0];
            switch (c) {
                case '#':
                    wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM_LUA);
                    //p->sub_state=WTK_NLG2_ITEM_STATE_COMMENT;
                    break;
                case '\n':
                    p->sub_state = WTK_NLG2_ITEM_STATE_END_HINT;
                    //wtk_nlg2_parser_set_state(p,WTK_NLG2_INIT);
                    break;
                case '+':
                    wtk_strbuf_reset(buf);
                    p->sub_state = WTK_NLG2_ITEM_STATE_PRE;
                    break;
                case '=':
                    wtk_strbuf_reset(buf);
                    p->sub_state = WTK_NLG2_ITEM_STATE_NXT;
                    break;
                case '<':
                    //wtk_debug("=============== check =================\n")
                    wtk_strbuf_reset(buf);
                    p->sub_state = WTK_NLG2_ITEM_STATE_BEFORE;
                    break;
                default:
                    wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM_EXPR);
                    return wtk_nlg2_parser_feed(p, v);
                    break;
//		case '*':
//			wtk_nlg2_parser_set_state(p,WTK_NLG2_ITEM_EXPR);
//			break;
//		default:
//			wtk_nlg2_parser_set_state(p,WTK_NLG2_INIT);
//			return wtk_nlg2_parser_feed(p,v);
//			break;
            }
            break;
        case WTK_NLG2_ITEM_STATE_PRE:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_strbuf_strip(buf);
                p->item->pre = wtk_heap_dup_str2(p->nlg->action_hash->heap,
                        buf->data, buf->pos);
                p->sub_state = WTK_NLG2_ITEM_STATE_INIT;
                return wtk_nlg2_parser_feed(p, v);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG2_ITEM_STATE_NXT:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_strbuf_strip(buf);
                p->item->nxt.str = wtk_heap_dup_string(p->heap, buf->data,
                        buf->pos);
                p->sub_state = WTK_NLG2_ITEM_STATE_INIT;
                return wtk_nlg2_parser_feed(p, v);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG2_ITEM_STATE_BEFORE:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_strbuf_strip(buf);
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                p->item->before.str = wtk_heap_dup_string(p->heap, buf->data,
                        buf->pos);
                p->sub_state = WTK_NLG2_ITEM_STATE_INIT;
                return wtk_nlg2_parser_feed(p, v);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG2_ITEM_STATE_COMMENT:
            if (v->len == 1 && v->data[0] == '\n') {
                p->sub_state = WTK_NLG2_ITEM_STATE_INIT;
            } else if (v->len == 1 && v->data[0] == '!') {
                wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM_LUA);
            } else {
                p->sub_state = WTK_NLG2_ITEM_STATE_COMMENT_WAIT_END;
            }
            break;
        case WTK_NLG2_ITEM_STATE_COMMENT_WAIT_END:
            if (v->len == 1 && v->data[0] == '\n') {
                p->sub_state = WTK_NLG2_ITEM_STATE_INIT;
            }
            break;
        case WTK_NLG2_ITEM_STATE_END_HINT:
            if (v->len > 1) {
                wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM_EXPR);
                return wtk_nlg2_parser_feed(p, v);
            }
            c = v->data[0];
            switch (c) {
                case '#':
                    p->sub_state = WTK_NLG2_ITEM_STATE_COMMENT;
                    break;
//		case '*':
//			wtk_nlg2_parser_set_state(p,WTK_NLG2_ITEM_EXPR);
//			break;
                case '\n':
                    wtk_nlg2_parser_set_state(p, WTK_NLG2_INIT);
                    break;
                case '+':
                    wtk_strbuf_reset(buf);
                    p->sub_state = WTK_NLG2_ITEM_STATE_PRE;
                    break;
                case '=':
                    wtk_strbuf_reset(buf);
                    p->sub_state = WTK_NLG2_ITEM_STATE_NXT;
                    break;
                case '<':
                    wtk_strbuf_reset(buf);
                    p->sub_state = WTK_NLG2_ITEM_STATE_BEFORE;
                    break;
                default:
                    wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM_EXPR);
                    return wtk_nlg2_parser_feed(p, v);
                    break;
            }
            break;
    }
    return 0;
}

int wtk_nlg2_parser_feed_item_lua(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    wtk_strbuf_t *buf = p->buf;

    switch (p->sub_state) {
        case -1:
            wtk_strbuf_reset(buf);
            p->sub_state = 0;
            wtk_strbuf_push(buf, v->data, v->len);
            break;
        case 0:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_strbuf_strip(buf);
                wtk_nlg2_add_global_lua(p->nlg, p->item, buf->data, buf->pos);
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM);
                //exit(0);
                return wtk_nlg2_parser_feed(p, v);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_nlg2_parser_feed_item_expr(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    wtk_strbuf_t *buf = p->buf;
    wtk_rexpr_t *expr;
    char *px;
    int len;
    wtk_string_t a;

    switch (p->sub_state) {
        case -1:
            wtk_strbuf_reset(buf);
            p->sub_state = 0;
            wtk_strbuf_push(buf, v->data, v->len);
            break;
        case 0:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_strbuf_strip(buf);
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                wtk_string_set(&(a), 0, 0);
                if (buf->data[0] == '/') {
                    px = wtk_str_chr(buf->data + 1, buf->pos - 1, '/');
                    //wtk_debug("px=%p\n",px);
                    if (px) {
                        //wtk_debug("[%.*s]\n",(int)(px-buf->data-1),buf->data+1);
                        wtk_string_set(&(a), buf->data + 1,
                                (int )(px - buf->data - 1));
                        px = px + 1;
                        len = buf->data + buf->pos - px;
                        //wtk_debug("[%.*s]\n",len,px);
                        //exit(0);
                    } else {
                        px = buf->data;
                        len = buf->pos;
                    }
                } else {
                    px = buf->data;
                    len = buf->pos;
                }
                //wtk_debug("[%.*s]\n",len,px);
                expr = wtk_rexpr_parser_process(p->re_parser,
                        p->nlg->action_hash->heap, px, len);
                if (!expr) {
                    return -1;
                }
                //wtk_rexpr_print(expr);
                if (!expr->name || wtk_string_cmp_s(expr->name,"main") == 0) {
                    //wtk_debug("[%.*s]\n",a.len,a.data);
                    if (a.len > 0) {
                        wtk_rexpr_add_depend(expr, a.data, a.len,
                                p->nlg->action_hash->heap);
                    }
                    //exit(0);
                    wtk_nlg2_add_global_expr(p->nlg, p->item, expr);
                } else {
                    wtk_nlg2_add_local_expr(p->nlg, p->item, expr);
                }
                wtk_nlg2_parser_set_state(p, WTK_NLG2_ITEM);
                return wtk_nlg2_parser_feed(p, v);
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
    }
    return 0;
}

int wtk_nlg2_parser_feed(wtk_nlg2_parser_t *p, wtk_string_t *v)
{
    int ret = -1;

    //wtk_debug("state=%d/%d [%.*s]\n",p->state,p->sub_state,v->len,v->data);
    switch (p->state) {
        case WTK_NLG2_INIT:
            ret = wtk_nlg2_parser_feed_init(p, v);
            break;
        case WTK_NLG2_COMMENT:
            ret = wtk_nlg2_parser_feed_comment(p, v);
            break;
        case WTK_NLG2_INCLUDE:
            ret = wtk_nlg2_parser_feed_include(p, v);
            break;
        case WTK_NLG2_ACTION:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            ret = wtk_nlg2_parser_feed_action(p, v);
            break;
        case WTK_NLG2_ITEM:
            ret = wtk_nlg2_parser_feed_item(p, v);
            break;
        case WTK_NLG2_ITEM_EXPR:
            ret = wtk_nlg2_parser_feed_item_expr(p, v);
            break;
        case WTK_NLG2_ITEM_LUA:
            ret = wtk_nlg2_parser_feed_item_lua(p, v);
            break;
    }
    return ret;
}

int wtk_nlg2_parser_load_string(wtk_nlg2_parser_t *p, char *data, int len)
{
    char *s, *e;
    int n;
    int ret;
    wtk_string_t v;

    s = data;
    e = s + len;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        wtk_string_set(&(v), s, n);
        ret = wtk_nlg2_parser_feed(p, &v);
        if (ret != 0) {
            //wtk_debug("compile [%s] err\n",len,data);
            wtk_debug("err=%d/%d [%.*s]\n", p->state, p->sub_state,
                    (int )(e - s), s);
            goto end;
        }
        s += n;
    }
    ret = 0;
    end: return ret;
}

int wtk_nlg2_paser_load_file(wtk_nlg2_parser_t *p, char *fn)
{
    int ret = -1;
    char *data = NULL;
    int len;
    wtk_string_t *pwd;

    data = file_read_buf(fn, &len);
    if (!data) {
        wtk_debug("%s load failed\n", fn)
        goto end;
    }
    pwd = p->pwd;
    p->pwd = wtk_dir_name(fn, '/');
    ret = wtk_nlg2_parser_load_string(p, data, len);
    if (p->pwd) {
        wtk_string_delete(p->pwd);
    }
    p->pwd = pwd;
    end:
    //wtk_debug("%s=%d\n",fn,ret);
    if (data) {
        wtk_free(data);
    }
    return ret;
}

wtk_nlg2_t* wtk_nlg2_paser_load(wtk_nlg2_parser_t *p, char *fn,
        wtk_rbin2_t *rbin)
{
    wtk_nlg2_t *nlg;
    int ret;

    nlg = wtk_nlg2_new(137);
    p->nlg = nlg;
    if (rbin) {
        wtk_rbin2_item_t* item;

        item = wtk_rbin2_get3(rbin, fn, strlen(fn), 0);
        if (!item) {
            ret = -1;
            wtk_debug("%s not found in rbin\n", fn);
            goto end;
        }
        ret = wtk_nlg2_parser_load_string(p, item->data->data, item->data->len);
        wtk_rbin2_item_clean(item);
    } else {
        ret = wtk_nlg2_paser_load_file(p, fn);
    }
    end: if (ret != 0) {
        wtk_nlg2_delete(nlg);
        nlg = NULL;
    } else {
        wtk_nlg2_update(nlg);
    }
    return nlg;
}

wtk_nlg2_t* wtk_nlg2_new_fn(char *fn, wtk_rbin2_t *rbin)
{
    wtk_nlg2_parser_t *p;
    wtk_nlg2_t *nlg;

    p = wtk_nlg2_parser_new();
    nlg = wtk_nlg2_paser_load(p, fn, rbin);
    wtk_nlg2_parser_delete(p);
    return nlg;
}

wtk_nlg2_t* wtk_nlg2_new_str(char *data, int len)
{
    wtk_nlg2_parser_t *p;
    wtk_nlg2_t *nlg;
    int ret;

    p = wtk_nlg2_parser_new();
    nlg = wtk_nlg2_new(137);
    p->nlg = nlg;
    ret = wtk_nlg2_parser_load_string(p, data, len);
    wtk_nlg2_parser_delete(p);
    if (ret != 0) {
        wtk_nlg2_delete(nlg);
        nlg = NULL;
    }
    return nlg;
}
