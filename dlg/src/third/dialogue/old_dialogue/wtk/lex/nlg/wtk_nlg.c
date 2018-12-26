#include <ctype.h>
#include <stdarg.h>
#include "wtk_nlg.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/wtk_str_encode.h"
#include "wtk/core/wtk_sort.h"
#include "wtk/core/wtk_larray.h"
#include "wtk/core/wtk_os.h"

void wtk_nlg_set_state(wtk_nlg_t *nlg, wtk_nlg_state_t state)
{
    nlg->state = state;
    nlg->sub_state = 0;
}

wtk_nlg_item_t* wtk_nlg_new_item(wtk_nlg_t *nlg)
{
    wtk_nlg_item_t *item;
    wtk_heap_t *heap = nlg->heap;

    item = (wtk_nlg_item_t*) wtk_heap_malloc(heap, sizeof(wtk_nlg_item_t));
    item->action = NULL;
    item->comment = NULL;
    item->nxt.str = NULL;
    item->attr = wtk_array_new_h(heap, 3, sizeof(wtk_nlg_key_t*));
    item->value = wtk_array_new_h(heap, 3, sizeof(wtk_nlg_value_t*));
    item->match_all_sot = 1;
    return item;
}

int wtk_nlg_feed_init(wtk_nlg_t *nlg, wtk_string_t *v)
{
    wtk_strbuf_t *buf = nlg->buf;

    if (v->len == 1 && v->data[0] == '#') {
        wtk_nlg_set_state(nlg, WTK_NLG_COMM);
    } else if (v->len > 1 || !isspace(v->data[0])) {
        wtk_strbuf_reset(buf);
        wtk_strbuf_push(buf, v->data, v->len);
        wtk_nlg_set_state(nlg, WTK_NLG_KEY);
        nlg->item = wtk_nlg_new_item(nlg);
    }
    return 0;
}

int wtk_nlg_feed_comment(wtk_nlg_t *nlg, wtk_string_t *v)
{
    if (v->len == 1 && v->data[0] == '\n') {
        wtk_nlg_set_state(nlg, WTK_NLG_INIT);
    }
    return 0;
}

void wtk_nlg_item_update_attr(wtk_nlg_item_t *item, wtk_string_t *k,
        wtk_string_t *v)
{
    if (wtk_string_cmp_s(k,"match_all_slot") == 0) {
        item->match_all_sot = wtk_str_atoi(v->data, v->len);
    }
}

int wtk_nlg_feed_key(wtk_nlg_t *nlg, wtk_string_t *v)
{
    enum {
        WTK_NLG_KEY_INIT = 0,
        WTK_NLG_KEY_WAIT_LEFT_BRACE,
        WTK_NLG_KEY_WAIT_KEY,
        WTK_NLG_KEY_KEY,
        WTK_NLG_KEY_WAIT_EQ,
        WTK_NLG_KEY_WAIT_VALUE,
        WTK_NLG_KEY_END,
        WTK_NLG_KEY_ATTR,
    };
    wtk_strbuf_t *buf = nlg->buf;
    char c;

    switch (nlg->sub_state) {
        case WTK_NLG_KEY_INIT:
            if (v->len == 1 && (v->data[0] == '(' || isspace(v->data[0]))) {
                nlg->item->action = wtk_heap_dup_string(nlg->heap, buf->data,
                        buf->pos);
                wtk_strbuf_reset(buf);
                if (v->data[0] == '(') {
                    nlg->sub_state = WTK_NLG_KEY_WAIT_KEY;
                } else {
                    nlg->sub_state = WTK_NLG_KEY_WAIT_LEFT_BRACE;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG_KEY_WAIT_LEFT_BRACE:
            if (v->len == 1 && v->data[0] == '(') {
                nlg->sub_state = WTK_NLG_KEY_WAIT_KEY;
            }
            break;
        case WTK_NLG_KEY_WAIT_KEY:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case ',':
                        break;
                    case ')':
                        nlg->sub_state = WTK_NLG_KEY_END;
                        break;
                    default:
                        if (!isspace(c)) {
                            wtk_strbuf_reset(buf);
                            wtk_strbuf_push(buf, v->data, v->len);
                            nlg->sub_state = WTK_NLG_KEY_KEY;
                        }
                        break;
                }
            } else {
                wtk_strbuf_reset(buf);
                wtk_strbuf_push(buf, v->data, v->len);
                nlg->sub_state = WTK_NLG_KEY_KEY;
            }
            break;
        case WTK_NLG_KEY_END:
            if (v->len == 1) {
                if (v->data[0] == '\n') {
                    wtk_nlg_set_state(nlg, WTK_NLG_VALUE);
                } else if (v->data[0] == '/') {
                    wtk_strbuf_reset(buf);
                    wtk_strbuf_push_s(buf, "[");
                    nlg->sub_state = WTK_NLG_KEY_ATTR;
                }
            }
            break;
        case WTK_NLG_KEY_ATTR:
            if (v->len == 1 && v->data[0] == '/') {
                //int wtk_str_attr_parse(char *data,int bytes,void *ths,wtk_str_attr_f attr_notify);
                wtk_strbuf_push_s(buf, "]");
                wtk_str_attr_parse(buf->data, buf->pos, nlg->item,
                        (wtk_str_attr_f) wtk_nlg_item_update_attr);
                nlg->sub_state = WTK_NLG_KEY_END;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG_KEY_KEY:
            if (v->len == 1) {
                c = v->data[0];
                if (c == ',' || c == ')' || c == '=' || isspace(c)) {
                    //wtk_debug("[%.*s]\n",v->len,v->data);
                    nlg->key = (wtk_nlg_key_t*) wtk_heap_malloc(nlg->heap,
                            sizeof(wtk_nlg_key_t));
                    nlg->key->k = wtk_heap_dup_string(nlg->heap, buf->data,
                            buf->pos);
                    nlg->key->v = NULL;
                    wtk_array_push2(nlg->item->attr, &(nlg->key));
                    if (c == ',') {
                        nlg->sub_state = WTK_NLG_KEY_WAIT_KEY;
                    } else if (c == ')') {
                        nlg->sub_state = WTK_NLG_KEY_END;
                    } else if (c == '=') {
                        wtk_string_parser_init(&(nlg->parser), buf);
                        nlg->sub_state = WTK_NLG_KEY_WAIT_VALUE;
                    } else {
                        nlg->sub_state = WTK_NLG_KEY_WAIT_EQ;
                    }
                } else {
                    wtk_strbuf_push(buf, v->data, v->len);
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG_KEY_WAIT_EQ:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case '=':
                        wtk_string_parser_init(&(nlg->parser), buf);
                        nlg->sub_state = WTK_NLG_KEY_WAIT_VALUE;
                        break;
                    case ',':
                        nlg->sub_state = WTK_NLG_KEY_WAIT_KEY;
                        break;
                    case ')':
                        nlg->sub_state = WTK_NLG_KEY_END;
                        break;
                }
            }
            break;
        case WTK_NLG_KEY_WAIT_VALUE: {
            int ret;

            ret = wtk_string_parse(&(nlg->parser), v->data, v->len);
            switch (ret) {
                case 0:
                    break;
                case 1:
                    //wtk_debug("[%.*s]=[%.*s]\n",buf->pos,buf->data,v->len,v->data);
                    nlg->key->v = wtk_heap_dup_string(nlg->heap, buf->data,
                            buf->pos);
                    nlg->sub_state = WTK_NLG_KEY_WAIT_KEY;
                    break;
                case 2:
                    //wtk_debug("[%.*s]=[%.*s]\n",buf->pos,buf->data,v->len,v->data);
                    nlg->key->v = wtk_heap_dup_string(nlg->heap, buf->data,
                            buf->pos);
                    nlg->sub_state = WTK_NLG_KEY_WAIT_KEY;
                    return wtk_nlg_feed_key(nlg, v);
                    break;
                default:
                    return -1;
            }
        }
            break;
    }
    return 0;
}

wtk_nlg_root_t* wtk_nlg_get_root(wtk_nlg_t *nlg, char* action, int action_bytes,
        int insert)
{
    wtk_queue_node_t *qn;
    wtk_nlg_root_t *root;

    for (qn = nlg->tree_q.pop; qn; qn = qn->next) {
        root = data_offset2(qn, wtk_nlg_root_t, q_n);
        if (wtk_string_cmp(root->action, action, action_bytes) == 0) {
            return root;
        }
    }
    if (insert) {
        root = (wtk_nlg_root_t*) wtk_heap_malloc(nlg->heap,
                sizeof(wtk_nlg_root_t));
        root->action = wtk_heap_dup_string(nlg->heap, action, action_bytes);
        root->item = NULL;
        wtk_queue_init(&(root->attr_q));
        wtk_queue_push(&(nlg->tree_q), &(root->q_n));
        return root;
    } else {
        return NULL;
    }
}

int wtk_nlg_key_cmp(wtk_nlg_key_t *key, wtk_string_t *k, wtk_string_t *v,
        int insert)
{
    //wtk_debug("%.*s v=%p/%p v=%p\n",k->len,k->data,v,key->v,v);
    if (v && key->v) {
        //wtk_debug("[%.*s]=[%.*s]\n",v->len,v->data,key->v->len,key->v->data);
        if (wtk_string_cmp(key->v, v->data, v->len) != 0) {
            //wtk_debug("v not equla\n");
            return -1;
        } else {
            return wtk_string_cmp(key->k, k->data, k->len);
        }
    }
    if (insert) {
        if (v || key->v) {
            return -1;
        }
    } else {
        if (key->v) {
            return -1;
        }
    }
    return wtk_string_cmp(key->k, k->data, k->len);
}

wtk_nlg_node_t* wtk_nlg_root_get_node(wtk_nlg_t *nlg, wtk_nlg_root_t *root,
        wtk_string_t *k, wtk_string_t *v, int insert)
{
    wtk_queue_node_t *qn;
    wtk_nlg_node_t *node;
    wtk_nlg_key_t *key;

    //wtk_debug("k=[%.*s:%p],v=%p\n",k->len,k->data,k,v);
    for (qn = root->attr_q.pop; qn; qn = qn->next) //目前没用。。。
            {
        node = data_offset2(qn, wtk_nlg_node_t, q_n);
        //wtk_debug("[%.*s]\n",node->key->k->len,node->key->k->data);
        if (wtk_nlg_key_cmp(node->key, k, v, insert) == 0) {
            if (v && node->key->v) {
                return node;
            }
        }
    }
    for (qn = root->attr_q.pop; qn; qn = qn->next) //目前没用。。。
            {
        node = data_offset2(qn, wtk_nlg_node_t, q_n);
        //wtk_debug("[%.*s]\n",node->key->k->len,node->key->k->data);
        if (wtk_nlg_key_cmp(node->key, k, v, insert) == 0) {
            return node;
        }
    }
    if (insert) {
        node = (wtk_nlg_node_t*) wtk_heap_malloc(nlg->heap,
                sizeof(wtk_nlg_node_t));
        key = (wtk_nlg_key_t*) wtk_heap_malloc(nlg->heap,
                sizeof(wtk_nlg_key_t));
        key->k = wtk_heap_dup_string(nlg->heap, k->data, k->len);
        if (v) {
            key->v = wtk_heap_dup_string(nlg->heap, v->data, v->len);
        } else {
            key->v = NULL;
        }
        node->key = key;
        wtk_queue_init(&(node->attr_q));
        node->item = NULL;
        wtk_queue_push(&(root->attr_q), &(node->q_n));
        return node;
    } else {
        return NULL;
    }
}

wtk_nlg_node_t* wtk_nlg_node_get_child(wtk_nlg_t *nlg, wtk_nlg_node_t *root,
        wtk_string_t *k, wtk_string_t *v, int insert)
{
    wtk_queue_node_t *qn;
    wtk_nlg_node_t *node;
    wtk_nlg_key_t *key;

    //wtk_debug("attr=%d\n",root->attr_q.length);
    for (qn = root->attr_q.pop; qn; qn = qn->next) {
        node = data_offset2(qn, wtk_nlg_node_t, q_n);
        //wtk_debug("[%.*s]=[%.*s]\n",k->len,k->data,v->len,v->data);
        if (wtk_nlg_key_cmp(node->key, k, v, insert) == 0) {
            return node;
        }
    }
    if (insert) {
        node = (wtk_nlg_node_t*) wtk_heap_malloc(nlg->heap,
                sizeof(wtk_nlg_node_t));
        key = (wtk_nlg_key_t*) wtk_heap_malloc(nlg->heap,
                sizeof(wtk_nlg_key_t));
        key->k = wtk_heap_dup_string(nlg->heap, k->data, k->len);
        if (v) {
            key->v = wtk_heap_dup_string(nlg->heap, v->data, v->len);
        } else {
            key->v = NULL;
        }
        node->key = key;
        wtk_queue_init(&(node->attr_q));
        node->item = NULL;
        wtk_queue_push(&(root->attr_q), &(node->q_n));
        return node;
    } else {
//		wtk_debug("cmp %.*s\n",k->len,k->data);
//		if(wtk_nlg_key_cmp(root->key,k,v,insert)==0)
//		{
//			return root;
//		}else
//		{
//			return NULL;
//		}
        return NULL;
    }
}

float wtk_ngl_cmp_key(void *p, wtk_nlg_key_t **k, wtk_nlg_key_t **v)
{
    //wtk_debug("[%.*s]=[%.*s]\n",(*k)->len,(*k)->data,(*v)->len,(*v)->data);
    return wtk_string_cmp((*k)->k, ((*v)->k)->data, ((*v)->k)->len);
}

float wtk_ngl_cmp_string(void *p, wtk_string_t **k, wtk_string_t **v)
{
    //wtk_debug("[%.*s]=[%.*s]\n",(*k)->len,(*k)->data,(*v)->len,(*v)->data);
    return wtk_string_cmp(*k, (*v)->data, (*v)->len);
}

void wtk_nlg_update_item(wtk_nlg_t *nlg)
{
    wtk_nlg_root_t *root;
    wtk_nlg_node_t *node = NULL;
    wtk_nlg_item_t *item;
    wtk_nlg_key_t **keys;
    wtk_string_t *v;
    int i;

    //wtk_nlg_item_print(nlg->item);
    item = nlg->item;
    root = wtk_nlg_get_root(nlg, item->action->data, item->action->len, 1);
    if (item->attr->nslot > 1) {
        wtk_qsort3(item->attr->slot, item->attr->nslot, sizeof(wtk_nlg_key_t*),
                (wtk_qsort_cmp_f) wtk_ngl_cmp_key, NULL, &v);
    }
    //wtk_nlg_item_print(nlg->item);
    keys = (wtk_nlg_key_t**) (item->attr->slot);
    for (i = 0; i < item->attr->nslot; ++i) {
        //wtk_debug("v[%d]=[%.*s]\n",i,keys[i]->k->len,keys[i]->k->data);
        if (i == 0) {
            //wtk_debug("v[%d]=[%.*s]\n",i,keys[i]->k->len,keys[i]->k->data);
            node = wtk_nlg_root_get_node(nlg, root, keys[i]->k, keys[i]->v, 1);
        } else {
            //wtk_debug("v[%d]=[%.*s]\n",i,keys[i]->k->len,keys[i]->k->data);
            node = wtk_nlg_node_get_child(nlg, node, keys[i]->k, keys[i]->v, 1);
        }
    }
    if (node) {
        node->item = item;
    } else {
        root->item = item;
    }
    //wtk_nlg_node_print(node);
    //wtk_nlg_root_print(root);
    nlg->item = NULL;
    //exit(0);
}

wtk_nlg_value_str_t* wtk_nlg_new_value_str(wtk_nlg_t *nlg)
{
    wtk_nlg_value_str_t *s;

    s = (wtk_nlg_value_str_t*) wtk_heap_malloc(nlg->heap,
            sizeof(wtk_nlg_value_str_t));
    wtk_queue_init(&(s->str_q));
    return s;
}

wtk_nlg_value_t* wtk_nlg_new_value(wtk_nlg_t *nlg, int is_str)
{
    wtk_nlg_value_t *v;

    v = (wtk_nlg_value_t*) wtk_heap_malloc(nlg->heap, sizeof(wtk_nlg_value_t));
    v->is_str = is_str;
    if (v->is_str) {
        v->v.str = wtk_nlg_new_value_str(nlg);
    } else {
        v->v.var = NULL;
    }
    return v;
}

wtk_nlg_value_str_slot_t* wtk_nlg_add_value_slot(wtk_nlg_t *nlg,
        wtk_nlg_value_t *v, char *data, int bytes, int is_str)
{
    wtk_nlg_value_str_slot_t *slot;

    slot = (wtk_nlg_value_str_slot_t*) wtk_heap_malloc(nlg->heap,
            sizeof(wtk_nlg_value_str_slot_t));
    slot->is_str = is_str;
    if (is_str) {
        slot->v.str = wtk_heap_dup_string(nlg->heap, data, bytes);
    } else {
        slot->v.var = wtk_heap_dup_string(nlg->heap, data, bytes);
    }
    wtk_queue_push(&(v->v.str->str_q), &(slot->q_n));
    return slot;
}

int wtk_nlg_feed_value(wtk_nlg_t *nlg, wtk_string_t *v)
{
    enum {
        WTK_NLG_VALUE_INIT = 0,
        WTK_NLG_VALUE_V,
        WTK_NLG_VALUE_VAR,
        WTK_NLG_VALUE_WAIT_NEXT,
        WTK_NLG_VALUE_STR_VAR,
        WTK_NLG_VALUE_COMMENT,
        WTK_NLG_VALUE_NXT,
    };
    wtk_strbuf_t *buf = nlg->buf;
    char c;

    //wtk_debug("[%.*s]\n",v->len,v->data);
    c = v->data[0];
    switch (nlg->sub_state) {
        case WTK_NLG_VALUE_INIT:
            if (v->len == 1) {
                if (c == '#') {
                    wtk_strbuf_reset(buf);
                    nlg->sub_state = WTK_NLG_VALUE_VAR;
                    nlg->value = wtk_nlg_new_value(nlg, 0);
                    wtk_array_push2(nlg->item->value, &(nlg->value));
                } else if (c == '+') {
                    wtk_strbuf_reset(buf);
                    nlg->sub_state = WTK_NLG_VALUE_COMMENT;
                } else if (c == '=') {
                    wtk_strbuf_reset(buf);
                    nlg->sub_state = WTK_NLG_VALUE_NXT;
                } else if (!isspace(c)) {
                    wtk_strbuf_reset(buf);
                    //wtk_strbuf_push(buf,v->data,v->len);
                    nlg->sub_state = WTK_NLG_VALUE_V;
                    nlg->value = wtk_nlg_new_value(nlg, 1);
                    wtk_array_push2(nlg->item->value, &(nlg->value));
                    return wtk_nlg_feed_value(nlg, v);
                }
            } else {
                wtk_strbuf_reset(buf);
                wtk_strbuf_push(buf, v->data, v->len);
                nlg->sub_state = WTK_NLG_VALUE_V;
                nlg->value = wtk_nlg_new_value(nlg, 1);
                wtk_array_push2(nlg->item->value, &(nlg->value));
            }
            break;
        case WTK_NLG_VALUE_COMMENT:
            if (v->len == 1 && c == '\n') {
                wtk_strbuf_strip(buf);
                nlg->item->comment = wtk_heap_dup_string(nlg->heap, buf->data,
                        buf->pos);
                nlg->sub_state = WTK_NLG_VALUE_WAIT_NEXT;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG_VALUE_NXT:
            if (v->len == 1 && c == '\n') {
                wtk_strbuf_strip(buf);
                nlg->item->nxt.str = wtk_heap_dup_string(nlg->heap, buf->data,
                        buf->pos);
                nlg->sub_state = WTK_NLG_VALUE_WAIT_NEXT;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG_VALUE_VAR:
            if (v->len == 1 && c == '\n') {
                wtk_strbuf_strip(buf);
                nlg->value->v.var = wtk_heap_dup_string(nlg->heap, buf->data,
                        buf->pos);
                nlg->sub_state = WTK_NLG_VALUE_WAIT_NEXT;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG_VALUE_V:
            if (v->len == 1 && (c == '\n' || c == '$')) {
                if (c == '$') {
                    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                    wtk_nlg_add_value_slot(nlg, nlg->value, buf->data, buf->pos,
                            1);
                    nlg->sub_state = WTK_NLG_VALUE_STR_VAR;
                    wtk_var_parse_init(&(nlg->var_parser), nlg->buf);
                } else {
                    wtk_strbuf_strip(buf);
                    wtk_nlg_add_value_slot(nlg, nlg->value, buf->data, buf->pos,
                            1);
                    nlg->sub_state = WTK_NLG_VALUE_WAIT_NEXT;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_NLG_VALUE_STR_VAR: {
            int ret;

            ret = wtk_var_parse(&(nlg->var_parser), v->data, v->len);
            switch (ret) {
                case 0:
                    break;
                case 1:
                    //wtk_debug("[%.*s]=[%.*s]\n",buf->pos,buf->data,v->len,v->data);
                    if (wtk_str_equal_s(buf->data, buf->pos, "pwd")) {
                        //wtk_debug("[%.*s]=[%.*s]\n",buf->pos,buf->data,v->len,v->data);
                        if (nlg->pwd) {
                            wtk_nlg_add_value_slot(nlg, nlg->value,
                                    nlg->pwd->data, nlg->pwd->len, 1);
                        } else {
                            wtk_nlg_add_value_slot(nlg, nlg->value, "./", 2, 1);
                        }
                        wtk_strbuf_reset(buf);
                        nlg->sub_state = WTK_NLG_VALUE_V;
                    } else {
                        wtk_nlg_add_value_slot(nlg, nlg->value, buf->data,
                                buf->pos, 0);
                        wtk_strbuf_reset(buf);
                        nlg->sub_state = WTK_NLG_VALUE_V;
                    }
                    break;
                default:
                    return -1;
            }
        }
            break;
        case WTK_NLG_VALUE_WAIT_NEXT:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            if (v->len == 1 && isspace(c)) {
                if (c == '\n') {
                    wtk_nlg_update_item(nlg);
                    wtk_nlg_set_state(nlg, WTK_NLG_INIT);
                }
            } else {
                nlg->sub_state = WTK_NLG_VALUE_INIT;
                return wtk_nlg_feed_value(nlg, v);
            }
            break;
    }
    return 0;
}

int wtk_nlg_feed(wtk_nlg_t *nlg, wtk_string_t *v)
{
    int ret;

    switch (nlg->state) {
        case WTK_NLG_INIT:
            ret = wtk_nlg_feed_init(nlg, v);
            break;
        case WTK_NLG_COMM:
            ret = wtk_nlg_feed_comment(nlg, v);
            break;
        case WTK_NLG_KEY:
            ret = wtk_nlg_feed_key(nlg, v);
            break;
        case WTK_NLG_VALUE:
            ret = wtk_nlg_feed_value(nlg, v);
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

void wtk_nlg_update_ref_node(wtk_nlg_t *nlg, wtk_nlg_node_t *node)
{
    wtk_queue_node_t *qn;

    for (qn = node->attr_q.pop; qn; qn = qn->next) {
        node = data_offset2(qn, wtk_nlg_node_t, q_n);
        if (node->item && node->item->nxt.str) {
            //wtk_debug("[%.*s]\n",node->item->nxt.str->len,node->item->nxt.str->data);
            node->item->nxt.item = wtk_nlg_get3(nlg, node->item->nxt.str->data,
                    node->item->nxt.str->len);
        }
        wtk_nlg_update_ref_node(nlg, node);
    }
}

void wtk_nlg_update_ref(wtk_nlg_t *nlg)
{
    wtk_queue_node_t *qn, *qn2;
    wtk_nlg_root_t *root;
    wtk_nlg_node_t *node;

    for (qn = nlg->tree_q.pop; qn; qn = qn->next) {
        root = data_offset2(qn, wtk_nlg_root_t, q_n);
        if (root->item && root->item->nxt.str) {
            //wtk_debug("[%.*s]\n",root->item->nxt.str->len,root->item->nxt.str->data);
            root->item->nxt.item = wtk_nlg_get3(nlg, root->item->nxt.str->data,
                    root->item->nxt.str->len);
        }
        for (qn2 = root->attr_q.pop; qn2; qn2 = qn2->next) {
            node = data_offset2(qn2, wtk_nlg_node_t, q_n);
            if (node->item && node->item->nxt.str) {
                //wtk_debug("[%.*s]\n",node->item->nxt.str->len,node->item->nxt.str->data);
                node->item->nxt.item = wtk_nlg_get3(nlg,
                        node->item->nxt.str->data, node->item->nxt.str->len);
            }
            wtk_nlg_update_ref_node(nlg, node);
        }
    }
}

int wtk_nlg_load2(wtk_nlg_t *nlg, char *data, int bytes)
{
    char *s, *e;
    int ret = -1;
    int n;
    wtk_string_t v;

    nlg->item = NULL;
    s = data;
    e = s + bytes;
    wtk_nlg_set_state(nlg, WTK_NLG_INIT);
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        wtk_string_set(&(v), s, n);
        ret = wtk_nlg_feed(nlg, &v);
        if (ret != 0) {
            goto end;
        }
        s += n;
    }
    wtk_string_set(&(v), "\n", 1);
    wtk_nlg_feed_value(nlg, &(v));
    if (nlg->item && nlg->item->value->nslot > 0) {
        wtk_nlg_update_item(nlg);
    }
    wtk_nlg_update_ref(nlg);
    ret = 0;
    end: return ret;
}

int wtk_nlg_load(wtk_nlg_t *nlg, char *fn)
{
    char *data = NULL;
    int len;
    int ret = -1;
    wtk_rbin2_item_t *item = NULL;

    if (nlg->rbin) {
        item = wtk_rbin2_get3(nlg->rbin, fn, strlen(fn), 0);
        if (!item) {
            goto end;
        }
        data = item->data->data;
        len = item->data->len;
    } else {
        data = file_read_buf(fn, &len);
    }
    if (!data) {
        goto end;
    }
    ret = wtk_nlg_load2(nlg, data, len);
    end: if (nlg->rbin) {
        if (item) {
            wtk_rbin2_item_clean(item);
        }
    } else {
        if (data) {
            wtk_free(data);
        }
    }
    return ret;
}
#include "wtk/core/wtk_os.h"

wtk_nlg_t* wtk_nlg_new(char *fn)
{
    wtk_string_t v;
    wtk_nlg_t *nlg;
    int ret;

    nlg = (wtk_nlg_t*) wtk_malloc(sizeof(wtk_nlg_t));
    nlg->rbin = NULL;
    nlg->state = WTK_NLG_INIT;
    nlg->heap = wtk_heap_new(4096);
    nlg->buf = wtk_strbuf_new(256, 1);
    nlg->tmp = wtk_strbuf_new(256, 1);
    nlg->cnt = 0;
    nlg->item = NULL;
    if (fn) {
        v = wtk_dir_name2(fn, strlen(fn), '/');
        //wtk_debug("[%.*s] %s\n",v.len,v.data,fn);
        nlg->pwd = wtk_heap_dup_string(nlg->heap, v.data, v.len);
    } else {
        nlg->pwd = NULL;
        //nlg->pwd=wtk_heap_dup_string(nlg->heap,"./");
    }
    wtk_queue_init(&(nlg->tree_q));
    if (fn) {
        ret = wtk_nlg_load(nlg, fn);
        if (ret != 0) {
            wtk_nlg_delete(nlg);
            nlg = NULL;
        }
    }
    srand(clock());
    return nlg;
}

wtk_nlg_t* wtk_nlg_new2(wtk_rbin2_t *rbin, char *fn)
{
    wtk_string_t v;
    wtk_nlg_t *nlg;
    int ret;

    nlg = (wtk_nlg_t*) wtk_malloc(sizeof(wtk_nlg_t));
    nlg->rbin = rbin;
    nlg->state = WTK_NLG_INIT;
    nlg->heap = wtk_heap_new(4096);
    nlg->buf = wtk_strbuf_new(256, 1);
    nlg->tmp = wtk_strbuf_new(256, 1);
    nlg->cnt = 0;
    nlg->item = NULL;
    //wtk_debug("%s\n",fn);
    if (fn) {
        v = wtk_dir_name2(fn, strlen(fn), '/');
        nlg->pwd = wtk_heap_dup_string(nlg->heap, v.data, v.len);
    } else {
        nlg->pwd = NULL;
        //nlg->pwd=wtk_heap_dup_string(nlg->heap,"./");
    }
    wtk_queue_init(&(nlg->tree_q));
    if (fn) {
        ret = wtk_nlg_load(nlg, fn);
        if (ret != 0) {
            wtk_nlg_delete(nlg);
            nlg = NULL;
        }
    }

    srand(clock());
    return nlg;
}

void wtk_nlg_delete(wtk_nlg_t *nlg)
{
    wtk_strbuf_delete(nlg->tmp);
    wtk_strbuf_delete(nlg->buf);
    wtk_heap_delete(nlg->heap);
    wtk_free(nlg);
}

void wtk_nlg_print(wtk_nlg_t *nlg)
{
    wtk_queue_node_t *qn;
    wtk_nlg_root_t *root;

    for (qn = nlg->tree_q.pop; qn; qn = qn->next) {
        root = data_offset2(qn, wtk_nlg_root_t, q_n);
        wtk_nlg_root_print(root);
    }
}

wtk_nlg_value_t* wtk_nlg_get_item_value(wtk_nlg_t *nlg, wtk_nlg_item_t *item)
{
    wtk_nlg_value_t **vs;
    int n, index, v;

    vs = (wtk_nlg_value_t**) item->value->slot;
    n = item->value->nslot;
    if (n == 0) {
        return NULL;
    } else if (n == 1) {
        return vs[0];
    }
    srand(time_get_ms());	//nlg->cnt);
    //v=random()+nlg->random_cnt;
    if (nlg->cnt > 100) {
        //srandom(clock());
        nlg->cnt = 0;
    }
    v = rand() + nlg->cnt;
    ++nlg->cnt;
    index = v % n;
    //wtk_debug("v=%d n=%d index=%d %d\n",v,n,index,nlg->cnt);
    return vs[index];
}

wtk_nlg_item_t* wtk_nlg_get(wtk_nlg_t *nlg, wtk_string_t *action, ...)
{
    wtk_nlg_item_t *item = NULL;
    va_list ap;
    wtk_string_t *v;
    wtk_larray_t *a = NULL;
    wtk_string_t **strs;
    int i;
    wtk_nlg_root_t *root;
    wtk_nlg_node_t *node;

    root = wtk_nlg_get_root(nlg, action->data, action->len, 0);
    //wtk_debug("root=%p\n",root);
    if (!root) {
        goto end;
    }
    //wtk_nlg_root_print(root);
    a = wtk_larray_new(4, sizeof(wtk_string_t*));
    va_start(ap, action);
    while (1) {
        v = va_arg(ap, wtk_string_t*);
        if (!v) {
            break;
        }
        //wtk_debug("[%.*s]\n",v->len,v->data);
        wtk_larray_push2(a, &(v));
    }
    if (a->nslot > 0) {
        wtk_qsort3(a->slot, a->nslot, sizeof(wtk_string_t*),
                (wtk_qsort_cmp_f) wtk_ngl_cmp_string, NULL, &v);
        strs = (wtk_string_t**) a->slot;
        //wtk_nlg_root_print(root);
        node = NULL;
        for (i = 0; i < a->nslot; ++i) {
            //wtk_debug("[%.*s]\n",strs[i]->len,strs[i]->data);
            if (i == 0) {
                node = wtk_nlg_root_get_node(nlg, root, strs[i], NULL, 0);
            } else {
                node = wtk_nlg_node_get_child(nlg, node, strs[i], NULL, 0);
            }
            if (!node) {
                goto end;
            }
        }
        if (!node) {
            goto end;
        }
        item = node->item;
    } else {
        item = root->item;
    }
    end: if (a) {
        wtk_larray_delete(a);
    }
    //wtk_debug("item=%p\n",item);
    return item;
}

float wtk_nlg_cmp_string2(void *p, wtk_string_t *k, wtk_string_t *v)
{
    //wtk_debug("[%.*s]=[%.*s]\n",(k)->len,(k)->data,(v)->len,(v)->data);
    return wtk_string_cmp(k, (v)->data, (v)->len);
}

void wtk_nlg_add_hash_kv(wtk_str_hash_t *kv, char *k, int k_len, char *v,
        int v_len)
{
    wtk_string_t *pv;
    int pos;
    char *s;
    char *e;
    wtk_strkv_parser_t parser;
    int ret;
    wtk_strbuf_t *buf;

    //wtk_debug("[%.*s]=[%.*s]\n",k_len,k,v_len,v);
    pos = wtk_str_str(v, v_len, "/", 1);
    if (pos > 0) {
        //wtk_debug("[%.*s]\n",pos,v);
        pv = wtk_heap_dup_string(kv->heap, v, pos);
        wtk_str_hash_add2(kv, k, k_len, pv);
        s = v + pos + 1;
        e = v + v_len - 1;
        while (*e != '/' && e >= s) {
            --e;
        }
        if (e <= s) {
            return;
        }
        if (*e == '/') {
            --e;
        }
        //wtk_debug("[%.*s]\n",(int)(e-s+1),s);
        wtk_strkv_parser_init(&(parser), s, (int) (e - s + 1));
        buf = wtk_strbuf_new(256, 1);
        while (1) {
            ret = wtk_strkv_parser_next(&(parser));
            if (ret != 0) {
                break;
            }
            //wtk_debug("[%.*s]=[%.*s]\n",parser.k.len,parser.k.data,parser.v.len,parser.v.data);
            if (parser.k.len <= 0 || parser.v.len <= 0) {
                continue;
            }
            wtk_strbuf_reset(buf);
            wtk_strbuf_push(buf, k, k_len);
            wtk_strbuf_push_s(buf, ".");
            wtk_strbuf_push(buf, parser.k.data, parser.k.len);
            pv = wtk_heap_dup_string(kv->heap, parser.v.data, parser.v.len);
            wtk_str_hash_add2(kv, buf->data, buf->pos, pv);
        }
        wtk_strbuf_delete(buf);
        //exit(0);
    } else {
        pv = wtk_heap_dup_string(kv->heap, v, v_len);
        wtk_str_hash_add2(kv, k, k_len, pv);
    }

}

wtk_nlg_item_t* wtk_nlg_get2(wtk_nlg_t *nlg, char *cmd, int cmd_bytes,
        wtk_str_hash_t *kv)
{
    typedef enum {
        WTK_NLG_CMD_INIT,
        WTK_NGL_CMD_ACTION,
        WTK_NGL_CMD_WAIT_ACT_SP,
        WTK_NGL_CMD_WAIT_KEY,
        WTK_NGL_CMD_KEY,
        WTK_NLG_CMD_KEY_COMER,
        WTK_NLG_CMD_WAIT_VALUE,
        WTK_NLG_CMD_VALUE,
        WTK_NLG_CMD_VALUE_QUOTE,
        WTK_NGL_CMD_END,
    } wtk_nlg_cmd_state_t;
    char *s, *e;
    int n;
    wtk_nlg_cmd_state_t state;
    wtk_string_t act;
    wtk_string_t value[32];
    int idx = 0;
    wtk_larray_t *a;
    wtk_nlg_root_t *root;
    wtk_nlg_node_t *node;
    wtk_string_t *strs;
    wtk_string_t v, *xs;
    wtk_nlg_item_t *item = NULL;
    int i;
    char quote = '\"';
    wtk_string_t *pv;

    wtk_string_set(&(act), 0, 0);
    a = wtk_larray_new(4, sizeof(wtk_string_t));
    s = cmd;
    e = s + cmd_bytes;
    state = WTK_NLG_CMD_INIT;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        //wtk_debug("%d=[%.*s]\n",state,n,s);
        switch (state) {
            case WTK_NLG_CMD_INIT:
                if (n > 1 || !isspace(*s)) {
                    act.len = 0;
                    act.data = s;
                    state = WTK_NGL_CMD_ACTION;
                }
                break;
            case WTK_NGL_CMD_ACTION:
                if (n == 1 && (*s == '(' || isspace(*s))) {
                    act.len = s - act.data;
                    //wtk_debug("[%.*s]\n",act.len,act.data);
                    if (*s == '(') {
                        state = WTK_NGL_CMD_WAIT_KEY;
                    } else {
                        state = WTK_NGL_CMD_WAIT_ACT_SP;
                    }
                }
                break;
            case WTK_NGL_CMD_WAIT_ACT_SP:
                if (n == 1 && *s == '(') {
                    state = WTK_NGL_CMD_WAIT_KEY;
                }
                break;
            case WTK_NGL_CMD_WAIT_KEY:
                if (n > 1 || !isspace(*s)) {
                    if (*s == ')') {
                        state = WTK_NGL_CMD_END;
                        break;
                    } else {
                        value[idx].data = s;
                        value[idx].len = 0;
                        state = WTK_NGL_CMD_KEY;
                    }
                }
                break;
            case WTK_NGL_CMD_KEY:
                if (n == 1
                        && (*s == ')' || *s == ',' || *s == '=' || isspace(*s))) {
                    value[idx].len = s - value[idx].data;
                    wtk_larray_push2(a, (value + idx));
                    //wtk_debug("[%.*s]\n",value[idx].len,value[idx].data);
                    ++idx;
                    if (isspace(*s)) {
                        state = WTK_NLG_CMD_KEY_COMER;
                    } else if (*s == ',') {
                        state = WTK_NGL_CMD_WAIT_KEY;
                    } else if (*s == ')') {
                        state = WTK_NGL_CMD_END;
                    } else if (*s == '=') {
                        state = WTK_NLG_CMD_WAIT_VALUE;
                    }
                }
                break;
            case WTK_NLG_CMD_WAIT_VALUE:
                if (n > 1 || !isspace(*s)) {
                    if (*s == '\'' || *s == '\"') {
                        quote = *s;
                        v.data = s + 1;
                        state = WTK_NLG_CMD_VALUE_QUOTE;
                    } else {
                        v.data = s;
                        state = WTK_NLG_CMD_VALUE;
                    }
                }
                break;
            case WTK_NLG_CMD_VALUE_QUOTE:
                if (n == 1 && *s == quote) {
                    v.len = s - v.data;
                    //wtk_debug("[%.*s]=[%.*s]\n",v.len,v.data,value[idx-1].len,value[idx-1].data);
                    if (kv) {
                        wtk_nlg_add_hash_kv(kv, value[idx - 1].data,
                                value[idx - 1].len, v.data, v.len);
//					void wtk_nlg_add_hash_kv(wtk_str_hash_t *kv,char *k,int k_len,char *v,int v_len)
//					pv=wtk_heap_dup_string(kv->heap,v.data,v.len);
//					wtk_str_hash_add2(kv,value[idx-1].data,value[idx-1].len,pv);
                    }
                    state = WTK_NLG_CMD_KEY_COMER;
                }
                break;
            case WTK_NLG_CMD_VALUE:
                if (n == 1 && (*s == ',' || *s == ')' || isspace(*s))) {
                    v.len = s - v.data;
                    if (kv) {
                        pv = wtk_heap_dup_string(kv->heap, v.data, v.len);
                        wtk_str_hash_add2(kv, value[idx - 1].data,
                                value[idx - 1].len, pv);
                    }
                    //wtk_debug("[%.*s]\n",v.len,v.data);
                    if (n == ',') {
                        state = WTK_NGL_CMD_WAIT_KEY;
                    } else if (*s == ')') {
                        state = WTK_NGL_CMD_END;
                    } else {
                        state = WTK_NLG_CMD_KEY_COMER;
                    }
                }
                break;
            case WTK_NLG_CMD_KEY_COMER:
                if (n == 1 && (*s == ',' || *s == ')')) {
                    if (*s == ',') {
                        state = WTK_NGL_CMD_WAIT_KEY;
                    } else if (*s == ')') {
                        state = WTK_NGL_CMD_END;
                    }
                }
                break;
            case WTK_NGL_CMD_END:
                break;
        }
        s += n;
    }
    if (act.len == 0) {
        //wtk_debug("============== 1 =========\n");
        goto end;
    }
//	{
//		strs=(wtk_string_t*)a->slot;
//		wtk_debug("[%.*s]=%d\n",act.len,act.data,a->nslot);
//	    for(i=0;i<a->nslot;++i)
//	    {
//	    	wtk_debug("[%.*s]\n",strs[i].len,strs[i].data);
//	    }
//	}
    root = wtk_nlg_get_root(nlg, act.data, act.len, 0);
    if (!root) {
        goto end;
    }
    if (a->nslot == 0) {
        item = root->item;
        goto end;
    }
    wtk_qsort3(a->slot, a->nslot, sizeof(wtk_string_t),
            (wtk_qsort_cmp_f) wtk_nlg_cmp_string2, NULL, &v);
    strs = (wtk_string_t*) a->slot;
//    wtk_debug("================= 1 ===================\n");
//    wtk_nlg_root_print(root);
//    wtk_debug("================= 2 ===================\n");
//	{
//		strs=(wtk_string_t*)a->slot;
//		wtk_debug("[%.*s]\n",act.len,act.data);
//		for(i=0;i<a->nslot;++i)
//		{
//			wtk_debug("[%.*s]\n",strs[i].len,strs[i].data);
//		}
//	}
    //exit(0);
    node = NULL;
    for (i = 0; i < a->nslot; ++i) {
        xs = wtk_str_hash_find(kv, strs[i].data, strs[i].len);
        //wtk_debug("================== get i=%d =================\n",i);
        if (i == 0) {
            node = wtk_nlg_root_get_node(nlg, root, strs + i, xs, 0);
        } else {
            node = wtk_nlg_node_get_child(nlg, node, strs + i, xs, 0);
        }
        //wtk_debug("[%.*s] node=%p [%.*s]\n",strs[i].len,strs[i].data,node,xs->len,xs->data);
        if (!node) {
            goto end;
        }
    }
    if (!node) {
        goto end;
    }
    item = node->item;
    end: wtk_larray_delete(a);
    return item;
}

wtk_nlg_item_t* wtk_nlg_get3(wtk_nlg_t *nlg, char *cmd, int cmd_bytes)
{
    wtk_str_hash_t *kv;
    wtk_nlg_item_t *item;

    kv = wtk_str_hash_new(7);
    item = wtk_nlg_get2(nlg, cmd, cmd_bytes, kv);
    wtk_str_hash_delete(kv);
    return item;
}

void wtk_nlg_value_print(wtk_nlg_value_t *v)
{
    wtk_queue_node_t *qn;
    wtk_nlg_value_str_slot_t *slot;

    if (v->is_str) {
        for (qn = v->v.str->str_q.pop; qn; qn = qn->next) {
            slot = data_offset2(qn, wtk_nlg_value_str_slot_t, q_n);
            if (slot->is_str) {
                printf("%.*s", slot->v.str->len, slot->v.str->data);
            } else {
                printf("${%.*s}", slot->v.var->len, slot->v.var->data);
            }
        }
        printf("\n");
    } else {
        printf("#%.*s\n", v->v.var->len, v->v.var->data);
    }
}

void wtk_nlg_item_print(wtk_nlg_item_t *item)
{
    wtk_nlg_key_t **keys;
    wtk_nlg_value_t **v;
    int i;

    if (item->action) {
        printf("%.*s", item->action->len, item->action->data);
    }
    printf("(");
    keys = (wtk_nlg_key_t**) item->attr->slot;
    for (i = 0; i < item->attr->nslot; ++i) {
        if (i > 0) {
            printf(",");
        }
        printf("%.*s", keys[i]->k->len, keys[i]->k->data);//strs[i]->len,strs[i]->data);
        if (keys[i]->v) {
            printf("=\"%.*s\"", keys[i]->v->len, keys[i]->v->data);
        }
    }
    printf(")\n");
    v = (wtk_nlg_value_t**) item->value->slot;
    for (i = 0; i < item->value->nslot; ++i) {
        if (v[i]->is_str) {
            wtk_queue_node_t *qn;
            wtk_nlg_value_str_slot_t *slot;

            for (qn = v[i]->v.str->str_q.pop; qn; qn = qn->next) {
                slot = data_offset2(qn, wtk_nlg_value_str_slot_t, q_n);
                if (slot->is_str) {
                    printf("%.*s", slot->v.str->len, slot->v.str->data);
                } else {
                    printf("${%.*s}", slot->v.var->len, slot->v.var->data);
                }
            }
            printf("\n");
        } else {
            printf("#%.*s\n", v[i]->v.var->len, v[i]->v.var->data);
        }
    }

}

void wtk_nlg_node_print2(wtk_nlg_node_t *node, int depth)
{
    wtk_queue_node_t *qn;
    wtk_nlg_node_t *n;
    int i;

    for (i = 0; i < depth; ++i) {
        printf(" ");
    }
    printf("[%.*s]", node->key->k->len, node->key->k->data);
    if (node->key->v) {
        printf("=[%.*s]", node->key->v->len, node->key->v->data);
    }
    printf("\n");
    if (node->item) {
        wtk_nlg_item_print(node->item);
    }
    for (qn = node->attr_q.pop; qn; qn = qn->next) {
        n = data_offset2(qn, wtk_nlg_node_t, q_n);
        wtk_nlg_node_print2(n, depth + 1);
    }
}

void wtk_nlg_node_print(wtk_nlg_node_t *node)
{
    wtk_nlg_node_print2(node, 0);
}

void wtk_nlg_root_print(wtk_nlg_root_t *node)
{
    wtk_queue_node_t *qn;
    wtk_nlg_node_t *n;

    wtk_debug("=========== root ============\n");
    printf("[%.*s]\n", node->action->len, node->action->data);
    if (node->item) {
        wtk_nlg_item_print(node->item);
    }
    for (qn = node->attr_q.pop; qn; qn = qn->next) {
        n = data_offset2(qn, wtk_nlg_node_t, q_n);
        wtk_nlg_node_print2(n, 1);
    }
}

int wtk_nlg_process_item_str(wtk_nlg_t *nlg, wtk_nlg_value_str_t *vi, void *ths,
        wtk_nlg_act_get_value_f get_value, wtk_strbuf_t *buf)
{
    //wtk_strbuf_t *buf=nlg->buf;
    wtk_queue_node_t *qn;
    wtk_nlg_value_str_slot_t *slot;
    wtk_string_t *v;

    //wtk_act_print(act);
    //wtk_debug("len=%d\n",vi->str_q.length);
    //wtk_strbuf_reset(buf);
    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
    for (qn = vi->str_q.pop; qn; qn = qn->next) {
        slot = data_offset2(qn, wtk_nlg_value_str_slot_t, q_n);
        //wtk_debug("slot=%p [%.*s]\n",slot,slot->v.str->len,slot->v.str->data);
        if (slot->is_str) {
            wtk_strbuf_push(buf, slot->v.str->data, slot->v.str->len);
        } else {
            v = get_value(ths, slot->v.var->data, slot->v.var->len);
            if (!v) {
                wtk_debug("[%.*s] not found\n", slot->v.var->len,
                        slot->v.var->data);
                return -1;
            }
            wtk_strbuf_push(buf, v->data, v->len);
        }
    }
    wtk_debug("[%.*s]\n", buf->pos, buf->data);
    return 0;
}

int wtk_nlg_process2(wtk_nlg_t *nlg, wtk_nlg_item_t *item, wtk_strbuf_t *result,
        void *ths, wtk_nlg_act_get_value_f get_value,
        wtk_nlg_act_get_lua_f get_lua)
{
    wtk_nlg_value_t *v;
    wtk_strbuf_t *buf;	//=nlg->buf;
    int ret = -1;
    wtk_string_t t;

    //wtk_debug("check in.............\n");
    buf = wtk_strbuf_new(256, 1);
    v = wtk_nlg_get_item_value(nlg, item);
    //wtk_debug("v=%p\n",v);
    if (!v) {
        goto end;
    }
    //wtk_nlg_value_print(v);
    if (item->comment) {
        wtk_strbuf_reset(buf);
        wtk_strbuf_push(buf, item->comment->data, item->comment->len);
        wtk_strbuf_push_c(buf, 0);
        t = get_lua(ths, buf->data);
        if (t.len > 0) {
            wtk_strbuf_push(result, t.data, t.len);
            ret = 0;
            goto end;
        }
    }
    if (v->is_str) {
        ret = wtk_nlg_process_item_str(nlg, v->v.str, ths, get_value, result);
    } else {
        wtk_strbuf_reset(buf);
        wtk_strbuf_push(buf, v->v.var->data, v->v.var->len);
        wtk_strbuf_push_c(buf, 0);
        t = get_lua(ths, buf->data);
        if (t.len > 0) {
            wtk_strbuf_push(result, t.data, t.len);
            ret = 0;
            goto end;
        }
        ret = 0;
    }
    if (ret != 0) {
        goto end;
    }
    //wtk_debug("[%.*s] %p\n",result->pos,result->data,item->nxt.item);
    if (item->nxt.item) {
        wtk_strbuf_reset(buf);
        ret = wtk_nlg_process2(nlg, item->nxt.item, buf, ths, get_value,
                get_lua);
        if (ret != 0) {
            goto end;
        }
        if (buf->pos > 0) {
            wtk_strbuf_push(result, buf->data, buf->pos);
        }
    }
    end:
    //wtk_debug("ret=%d [%.*s]\n",ret,result->pos,result->data);
    wtk_strbuf_delete(buf);
    //wtk_debug("output=%p ret=%d\n",fld->output,ret);
    return ret;
}

int wtk_nlg_process(wtk_nlg_t *nlg, wtk_nlg_item_t *item, wtk_strbuf_t *result,
        void *ths, wtk_nlg_act_get_value_f get_value,
        wtk_nlg_act_get_lua_f get_lua)
{
    wtk_strbuf_reset(result);
    return wtk_nlg_process2(nlg, item, result, ths, get_value, get_lua);
}

wtk_nlg_item_t* wtk_nlg_node_get_default_item(wtk_nlg_node_t *node)
{
    if (node->item) {
        return node->item;
    }
    if (node->attr_q.length <= 0) {
        return NULL;
    }
    node = data_offset2(node->attr_q.pop, wtk_nlg_node_t, q_n);
    return wtk_nlg_node_get_default_item(node);
}

wtk_nlg_item_t* wtk_nlg_root_get_default_item(wtk_nlg_root_t *root)
{
    wtk_nlg_node_t *node;

    if (root->item) {
        return root->item;
    }
    if (root->attr_q.length <= 0) {
        return NULL;
    }
    node = data_offset2(root->attr_q.pop, wtk_nlg_node_t, q_n);
    return wtk_nlg_node_get_default_item(node);
}

wtk_string_t wtk_nlg_process_nlg_str(wtk_nlg_t *nlg, char *data, int bytes)
{
    wtk_str_hash_t *hash;
    wtk_nlg_item_t *item;
    int ret = -1;
    wtk_strbuf_t *buf = nlg->tmp;
    wtk_string_t v;

    wtk_string_set(&(v), 0, 0);
    wtk_strbuf_reset(buf);
    //wtk_debug("[%.*s]\n",bytes,data);
    hash = wtk_str_hash_new(4);
    item = wtk_nlg_get2(nlg, data, bytes, hash);
    if (!item) {
        wtk_debug("[%.*s] not found\n", bytes, data);
        goto end;
    }
    //wtk_nlg_item_print(item);
    ret = wtk_nlg_process(nlg, item, buf, hash,
            (wtk_nlg_act_get_value_f) wtk_str_hash_find, NULL);
    //wtk_debug("ret=%d [%.*s]\n",ret,buf->pos,buf->data);
    if (ret == 0) {
        wtk_string_set(&(v), buf->data, buf->pos);
    }
    end: wtk_str_hash_delete(hash);
    //wtk_debug("[%.*s]\n",v.len,v.data);
    //exit(0);
    return v;
}

wtk_string_t* wtk_nlg_process_nlg_str2_get_value(void **ths, char *k, int k_len)
{
    wtk_str_hash_t *hash;

    hash = ths[0];
    return wtk_str_hash_find(hash, k, k_len);
}

wtk_string_t wtk_nlg_process_nlg_str2_get_lua(void **ths, char *func)
{
    void *ls;
    wtk_nlg_act_get_lua_f get;
    wtk_string_t v;

    ls = ths[1];
    get = ths[2];
    if (get) {
        return get(ls, func);
    } else {
        wtk_string_set(&(v), 0, 0);
        return v;
    }
}

wtk_string_t wtk_nlg_process_nlg_str2(wtk_nlg_t *nlg, char *data, int bytes,
        void *get_lua_ths, wtk_nlg_act_get_lua_f get_lua)
{
    wtk_str_hash_t *hash;
    wtk_nlg_item_t *item;
    int ret = -1;
    wtk_strbuf_t *buf = nlg->tmp;
    wtk_string_t v;
    void *ths[3];

    wtk_string_set(&(v), 0, 0);
    wtk_strbuf_reset(buf);
    //wtk_debug("[%.*s]\n",bytes,data);
    hash = wtk_str_hash_new(4);
    item = wtk_nlg_get2(nlg, data, bytes, hash);
    if (!item) {
        wtk_debug("[%.*s] not found\n", bytes, data);
        goto end;
    }
    ths[0] = hash;
    ths[1] = get_lua_ths;
    ths[2] = get_lua;
    //wtk_nlg_item_print(item);
    ret = wtk_nlg_process(nlg, item, buf, ths,
            (wtk_nlg_act_get_value_f) wtk_nlg_process_nlg_str2_get_value,
            (wtk_nlg_act_get_lua_f) wtk_nlg_process_nlg_str2_get_lua);
    //wtk_debug("ret=%d [%.*s]\n",ret,buf->pos,buf->data);
    if (ret == 0) {
        wtk_string_set(&(v), buf->data, buf->pos);
    }
    end: wtk_str_hash_delete(hash);
    //wtk_debug("[%.*s]\n",v.len,v.data);
    //exit(0);
    return v;
}

