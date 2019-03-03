#include <ctype.h>
#include "wtk_owl.h" 

wtk_owl_t* wtk_owl_new()
{
    wtk_owl_t *owl;

    owl = (wtk_owl_t*) wtk_malloc(sizeof(wtk_owl_t));
    owl->rbin = NULL;
    owl->heap = wtk_heap_new(4096);
    owl->buf = wtk_strbuf_new(256, 1);
    owl->tmp = wtk_strbuf_new(256, 1);
    owl->pwd = NULL;
    owl->k = NULL;
    wtk_queue_init(&(owl->state_q));
    wtk_owl_set_state(owl, WTK_OWL_STATE_INIT);
    //wtk_owl_compile_file(owl,fn);
    return owl;
}

void wtk_owl_delete(wtk_owl_t *owl)
{
    wtk_heap_delete(owl->heap);
    wtk_strbuf_delete(owl->buf);
    wtk_strbuf_delete(owl->tmp);
    wtk_free(owl);
}

void wtk_owl_set_state(wtk_owl_t *owl, wtk_owl_state_t state)
{
    owl->state = state;
    owl->sub_state = -1;
}

void wtk_owl_save(wtk_owl_t *owl)
{
    owl->bak_state = owl->state;
    owl->bak_sub_state = owl->sub_state;
}

void wtk_owl_restore(wtk_owl_t *owl)
{
    owl->state = owl->bak_state;
    owl->sub_state = owl->bak_sub_state;
}

void wtk_owl_save_string_state(wtk_owl_t *owl)
{
    owl->str_state = owl->state;
    owl->str_sub_state = owl->sub_state;
    wtk_owl_set_state(owl, WTK_OWL_STATE_STRING);
}

void wtk_owl_restore_string_state(wtk_owl_t *owl)
{
    owl->state = owl->str_state;
    owl->sub_state = owl->str_sub_state;
}

int wtk_owl_feed_init(wtk_owl_t *owl, wtk_string_t *v)
{
    enum {
        WTK_OWL_INIT_INIT = -1, WTK_OWL_COMMENT2_HINT,
    };
    char c;

    if (v->len == 1) {
        c = v->data[0];
        switch (owl->sub_state) {
            case WTK_OWL_INIT_INIT:
                if (c == '#') {
                    wtk_owl_save(owl);
                    wtk_owl_set_state(owl, WTK_OWL_STATE_COMMENT1);
                    return 0;
                } else if (c == '/') {
                    owl->sub_state = WTK_OWL_COMMENT2_HINT;
                    return 0;
                } else if (c == '<') {
                    wtk_owl_save(owl);
                    wtk_owl_set_state(owl, WTK_OWL_XML_ITEM);
                    return wtk_owl_feed(owl, v);
                } else if (isspace(c)) {
                    return 0;
                } else {
                    return -1;
                }
                break;
            case WTK_OWL_COMMENT2_HINT:
                if (c == '*') {
                    owl->sub_state = WTK_OWL_INIT_INIT;
                    wtk_owl_save(owl);
                    wtk_owl_set_state(owl, WTK_OWL_STATE_COMMENT2);
                    return 0;
                } else {
                    return -1;
                }
                break;
        }
        return -1;
    } else {
        return -1;
    }
}

int wtk_owl_feed_comment1(wtk_owl_t *owl, wtk_string_t *v)
{
    enum wtk_owl_comment1_state {
        WTK_OWL_COMMENT1_INIT = -1,
        WTK_OWL_COMMENT1_WRD,
        WTK_OWL_COMMENT1_WAIT,
        WTK_OWL_COMMENT1_STRING,
    };
    wtk_strbuf_t *buf = owl->buf;

    switch (owl->sub_state) {
        case WTK_OWL_COMMENT1_INIT:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_owl_restore(owl);
            } else {
                if (v->len > 1 || !isspace(v->data[0])) {
                    wtk_strbuf_reset(buf);
                    wtk_strbuf_push(buf, v->data, v->len);
                    owl->sub_state = WTK_OWL_COMMENT1_WRD;
                }
            }
            break;
        case WTK_OWL_COMMENT1_WRD:
            if (v->len == 1 && isspace(v->data[0])) {
                if (wtk_str_equal_s(buf->data, buf->pos, "include")) {
                    //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                    owl->sub_state = WTK_OWL_COMMENT1_STRING;
                    wtk_owl_save_string_state(owl);
                    //exit(0);
                } else {
                    owl->sub_state = WTK_OWL_COMMENT1_WAIT;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_COMMENT1_WAIT:
            if (v->len == 1 && v->data[0] == '\n') {
                wtk_owl_restore(owl);
            }
            break;
        case WTK_OWL_COMMENT1_STRING:
            //wtk_debug("[%.*s]\n",buf->pos,buf->data);
        {
            char *data;
            wtk_string_t *pwd;
            wtk_string_t v;
            int len, ret;
            wtk_rbin2_item_t *item = NULL;

            pwd = owl->pwd;
            v = wtk_dir_name2(buf->data, buf->pos, '/');
            if (owl->rbin) {
                item = wtk_rbin2_get3(owl->rbin, buf->data, buf->pos, 0);
                //wtk_debug("[%.*s]=%p\n",buf->pos,buf->data,item);
                if (item) {
                    data = item->data->data;
                    len = item->data->len;
                } else {
                    //wtk_rbin2_print(owl->rbin);
                    //exit(0);
                    data = NULL;
                }
            } else {
                wtk_strbuf_push_c(buf, 0);
                data = file_read_buf(buf->data, &len);
            }
            if (!data) {
                wtk_debug("read file failed [%s].\n", buf->data);
                return -1;
            }
            owl->pwd = wtk_heap_dup_string(owl->heap, v.data, v.len);
            wtk_owl_set_state(owl, WTK_OWL_STATE_INIT);
            ret = wtk_owl_compile_string(owl, data, len);
            owl->pwd = pwd;
            if (owl->rbin) {
                wtk_rbin2_item_clean(item);
            } else {
                wtk_free(data);
            }
            if (ret != 0) {
                wtk_debug("compile file failed [%s].\n", buf->data);
                return -1;
            }
            wtk_owl_set_state(owl, WTK_OWL_STATE_INIT);
            return 0;
        }
            break;
    }
    return 0;
}

int wtk_owl_feed_comment2(wtk_owl_t *owl, wtk_string_t *v)
{
    enum {
        WTK_OWL_COMMENT2_INIT = -1, WTK_OWL_COMMENT2_HINT,
    };

    switch (owl->sub_state) {
        case WTK_OWL_COMMENT2_INIT:
            if (v->len == 1 && v->data[0] == '*') {
                owl->sub_state = WTK_OWL_COMMENT2_HINT;
            }
            return 0;
            break;
        case WTK_OWL_COMMENT2_HINT:
            if (v->len == 1 && v->data[0] == '/') {
                wtk_owl_restore(owl);
                return 0;
            } else {
                return -1;
            }
            break;
    }
    return 0;
}

wtk_owl_item_t* wtk_owl_state_item_get_item(wtk_owl_state_item_t *item)
{
    switch (item->type) {
        case WTK_OWL_CLASS:
            return item->v.class->item;
            break;
        case WTK_OWL_INST:
            return item->v.inst->item;
            break;
        case WTK_OWL_PROP:
            return NULL;
            break;
        case WTK_OWL_ITEM:
            return item->v.item;
            break;
        case WTK_OWL_CLASS_ITEM:
            return &(item->v.class_item);
            break;
        case WTK_OWL_CLASS_PARENT:
            return &(item->v.parent_item);
            break;
    }
    return NULL;
}

wtk_owl_state_item_t* wtk_owl_push_state(wtk_owl_t *owl, char *data, int bytes)
{
    wtk_heap_t *heap = owl->heap;
    wtk_owl_state_item_t *item, *last_item;
    wtk_owl_tree_t *tree = owl->tree;

    //wtk_debug("[%.*s]=%d\n",bytes,data,owl->state_q.length);
    if (owl->state_q.length > 0) {
        last_item = data_offset2(owl->state_q.push, wtk_owl_state_item_t, q_n);
    } else {
        last_item = NULL;
    }
    item = (wtk_owl_state_item_t*) wtk_heap_malloc(heap,
            sizeof(wtk_owl_state_item_t));
    item->v.prop = NULL;
    item->a = wtk_larray_new(10, sizeof(void*));
    //item->nm=wtk_heap_dup_string(heap,data,bytes);
    if (wtk_str_equal_s(data, bytes, "class")) {
        if (last_item) {
            if (last_item->type == WTK_OWL_INST) {
                item->v.class_item.k.str = NULL;
                item->type = WTK_OWL_CLASS_ITEM;
            } else {
                return NULL;
            }
        } else {
            item->type = WTK_OWL_CLASS;
        }
    } else if (wtk_str_equal_s(data, bytes, "inst")) {
        if (last_item) {
            return NULL;
        } else {
            item->type = WTK_OWL_INST;
        }
    } else if (wtk_str_equal_s(data, bytes, "prop")) {
        if (last_item) {
            item->type = WTK_OWL_PROP;
            switch (last_item->type) {
                case WTK_OWL_CLASS:
                    if (last_item->v.class->prop) {
                        item->v.prop = last_item->v.class->prop;
                    } else {
                        item->v.prop = wtk_owl_tree_new_item(tree, NULL, 0);
                        last_item->v.class->prop = item->v.prop;
                    }
                    break;
                case WTK_OWL_INST:
                    if (last_item->v.inst->prop) {
                        item->v.prop = last_item->v.inst->prop;
                    } else {
                        item->v.prop = wtk_owl_tree_new_item(tree, NULL, 0);
                        last_item->v.inst->prop = item->v.prop;
                    }
                    break;
                default:
                    return NULL;
                    break;
            }
        } else {
            return NULL;
        }
    } else if (wtk_str_equal_s(data, bytes, "item")) {
        if (last_item) {
            item->v.item = wtk_owl_tree_new_item(tree, NULL, 0);
            item->type = WTK_OWL_ITEM;
            switch (last_item->type) {
                case WTK_OWL_PROP:
                    wtk_larray_push2(last_item->a, &(item->v.item));
                    break;
                case WTK_OWL_ITEM:
                    wtk_larray_push2(last_item->a, &(item->v.item));
                    break;
                default:
                    return NULL;
                    break;
            }
        } else {
            return NULL;
        }
    } else if (wtk_str_equal_s(data, bytes, "parent")) {
        if (last_item && last_item->type == WTK_OWL_CLASS) {
            item->type = WTK_OWL_CLASS_PARENT;
        } else {
            return NULL;
        }
    } else {
        wtk_debug("unspported %.*s\n", bytes, data);
        return NULL;
    }
    wtk_queue_push(&(owl->state_q), &(item->q_n));
    //wtk_debug("new %.*s item=%p stk=%d\n",bytes,data,item,owl->state_q.length);
    return item;
}

void wtk_owl_pop_state(wtk_owl_t *owl)
{
    wtk_owl_state_item_t *item;
    wtk_queue_node_t *qn;
    int i;

    //wtk_debug("pop stk=%d\n",owl->state_q.length);
    if (owl->state_q.length <= 0) {
        wtk_debug("stack length is %d, not valid\n", owl->state_q.length);
        exit(0);
        return;
    }
    qn = wtk_queue_pop_back(&(owl->state_q));
    item = data_offset2(qn, wtk_owl_state_item_t, q_n);
    if (item->a->nslot > 0) {
        switch (item->type) {
            case WTK_OWL_PROP:
                //wtk_debug("nitem=%d\n",item->v.prop->nitem);
                if (item->v.prop->nitem == 0) {
                    item->v.prop->nitem = item->a->nslot;
                    item->v.prop->items = (wtk_owl_item_t**) wtk_heap_malloc(
                            owl->tree->heap,
                            sizeof(wtk_owl_item_t*) * item->a->nslot);
                    memcpy(item->v.prop->items, item->a->slot,
                            sizeof(wtk_owl_item_t*) * item->a->nslot);
                } else {
                    wtk_owl_item_t **items;
                    int nx;

                    nx = item->a->nslot + item->v.prop->nitem;
                    items = (wtk_owl_item_t**) wtk_heap_malloc(owl->tree->heap,
                            sizeof(wtk_owl_item_t*) * nx);
                    memcpy(items, item->v.prop->items,
                            sizeof(wtk_owl_item_t*) * item->v.prop->nitem);
                    memcpy(items + item->v.prop->nitem, item->a->slot,
                            sizeof(wtk_owl_item_t*) * item->a->nslot);
                    item->v.prop->nitem = nx;
                    item->v.prop->items = items;
                }
                break;
            case WTK_OWL_ITEM:
                if (item->v.item->nitem == 0) {
                    item->v.item->nitem = item->a->nslot;
                    item->v.item->items = (wtk_owl_item_t**) wtk_heap_malloc(
                            owl->tree->heap,
                            sizeof(wtk_owl_item_t*) * item->a->nslot);
                    memcpy(item->v.item->items, item->a->slot,
                            sizeof(wtk_owl_item_t*) * item->a->nslot);
                } else {
                    wtk_owl_item_t **items;
                    int nx;

                    nx = item->a->nslot + item->v.item->nitem;
                    items = (wtk_owl_item_t**) wtk_heap_malloc(owl->tree->heap,
                            sizeof(wtk_owl_item_t*) * nx);
                    memcpy(items, item->v.item->items,
                            sizeof(wtk_owl_item_t*) * item->v.item->nitem);
                    memcpy(items + item->v.item->nitem, item->a->slot,
                            sizeof(wtk_owl_item_t*) * item->a->nslot);
                    item->v.item->nitem = nx;
                    item->v.item->items = items;
                }
                for (i = 0; i < item->v.item->nitem; ++i) {
                    item->v.item->items[i]->parent_item = item->v.item;
                }
                break;
            default:
                wtk_debug("found bug item=%p\n", item)
                ;
                break;
        }
    }
    wtk_larray_delete(item->a);
}

int wtk_owl_update_expr_name(wtk_owl_t *owl, char *name, int bytes)
{
    wtk_owl_state_item_t *item;

    item = data_offset2(owl->state_q.push, wtk_owl_state_item_t, q_n);
    switch (item->type) {
        case WTK_OWL_CLASS:
            //wtk_debug("[%.*s]\n",bytes,name);
            item->v.class = wtk_owl_tree_find_class(owl->tree, name, bytes, 1);
            break;
        case WTK_OWL_INST:
            item->v.inst = wtk_owl_tree_find_inst(owl->tree, name, bytes, 1);
            break;
        case WTK_OWL_PROP:
            return -1;
            break;
        case WTK_OWL_ITEM:
            if (bytes > 0 && (name[0] == '$')) {
                item->v.item->k.inst = wtk_owl_tree_find_inst(owl->tree,
                        name + 1, bytes - 1, 1);
                item->v.item->use_k_inst = 1;
            } else {
                item->v.item->k.str = wtk_heap_dup_string(owl->tree->heap, name,
                        bytes);
                item->v.item->use_k_inst = 0;
            }
            break;
        case WTK_OWL_CLASS_ITEM:
            item = data_offset2(owl->state_q.push->prev, wtk_owl_state_item_t,
                    q_n);
            if (item->type == WTK_OWL_INST) {
                item->v.inst->class = wtk_owl_tree_find_class(owl->tree, name,
                        bytes, 1);
            } else {
                return -1;
            }
            break;
        case WTK_OWL_CLASS_PARENT:
            item = data_offset2(owl->state_q.push->prev, wtk_owl_state_item_t,
                    q_n);
            if (item->type == WTK_OWL_CLASS) {
                item->v.class->parent = wtk_owl_tree_find_class(owl->tree, name,
                        bytes, 1);
            } else {
                return -1;
            }
            break;
    }
    return 0;
}

int wtk_owl_feed_xml_item(wtk_owl_t *owl, wtk_string_t *v)
{
    enum {
        WTK_OWL_XML_INIT = -1,
        WTK_OWL_XML_WAIT_NAME,
        WTK_OWL_XML_NAME,
        WTK_OWL_XML_NAME_WAIT_EQ,
        WTK_OWL_XML_NAME_WAIT_VALUE,
        WTK_OWL_XML_NAME_VALUE,
        WTK_OWL_XML_NAME_VALUE_QUOTE,
        WTK_OWL_XML_WAIT_KEY,
        WTK_OWL_XML_KEY,
        WTK_OWL_XML_WAIT_EQ,
        WTK_OWL_XML_K_WAIT_VALUE,
        WTK_OWL_XML_K_VALUE,
        WTK_OWL_XML_K_VALUE_QUOTE,
        WTK_OWL_XML_END,
        WTK_OWL_XML_ITEM_END,
        WTK_OWL_XML_K_VALUE_QUOTE_ESC,
    };
    wtk_strbuf_t *buf = owl->buf;
    wtk_owl_state_item_t *item;
    wtk_owl_item_t *oi;
    int ret;
    //wtk_heap_t *heap=owl->tree->heap;

    //wtk_debug("sub=%d [%.*s]\n",owl->sub_state,v->len,v->data);
    switch (owl->sub_state) {
        case WTK_OWL_XML_INIT:
            if (v->len == 1 && v->data[0] == '<') {
                owl->sub_state = WTK_OWL_XML_WAIT_NAME;
                owl->k = NULL;
            }
            break;
        case WTK_OWL_XML_WAIT_NAME:
            if (v->len > 1 || !isspace(v->data[0])) {
                if (v->data[0] == '/') {
                    owl->sub_state = WTK_OWL_XML_ITEM_END;
                    wtk_strbuf_reset(buf);
                } else {
                    wtk_strbuf_reset(buf);
                    wtk_strbuf_push(buf, v->data, v->len);
                    owl->sub_state = WTK_OWL_XML_NAME;
                }
            }
            break;
        case WTK_OWL_XML_ITEM_END:
            if (v->len == 1 && v->data[0] == '>') {
                wtk_owl_pop_state(owl);
                wtk_owl_restore(owl);
                return 0;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_XML_NAME:
            if (v->len == 1
                    && (v->data[0] == '>' || v->data[0] == '='
                            || isspace(v->data[0]))) {
                item = wtk_owl_push_state(owl, buf->data, buf->pos);
                if (!item) {
                    return -1;
                }
                if (v->data[0] == '>') {
                    wtk_owl_restore(owl);
                    return 0;
                } else if (v->data[0] == '=') {
                    owl->sub_state = WTK_OWL_XML_NAME_WAIT_VALUE;
                } else {
                    owl->sub_state = WTK_OWL_XML_NAME_WAIT_EQ;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_XML_NAME_WAIT_EQ:
            if (v->len == 1 && v->data[0]) {
                owl->sub_state = WTK_OWL_XML_NAME_WAIT_VALUE;
            }
            break;
        case WTK_OWL_XML_NAME_WAIT_VALUE:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                item = data_offset2(owl->state_q.push, wtk_owl_state_item_t,
                        q_n);
                if (v->len == 1 && v->data[0] == '"') {
                    owl->sub_state = WTK_OWL_XML_NAME_VALUE_QUOTE;
                } else {
                    wtk_strbuf_push(buf, v->data, v->len);
                    owl->sub_state = WTK_OWL_XML_NAME_VALUE;
                }
            }
            break;
        case WTK_OWL_XML_NAME_VALUE:
            if (v->len == 1
                    && (v->data[0] == '/' || v->data[0] == '>'
                            || isspace(v->data[0]))) {
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                ret = wtk_owl_update_expr_name(owl, buf->data, buf->pos);
                if (ret < 0) {
                    return -1;
                }
                if (v->data[0] == '/') {
                    wtk_owl_pop_state(owl);
                    owl->sub_state = WTK_OWL_XML_END;
                    return 0;
                } else if (v->data[0] == '>') {
                    wtk_owl_restore(owl);
                    return 0;
                } else {
                    owl->sub_state = WTK_OWL_XML_WAIT_KEY;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_XML_NAME_VALUE_QUOTE:
            if (v->len == 1 && v->data[0] == '"') {
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                ret = wtk_owl_update_expr_name(owl, buf->data, buf->pos);
                if (ret < 0) {
                    return -1;
                }
                owl->sub_state = WTK_OWL_XML_WAIT_KEY;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_XML_END:
            if (v->len == 1 && v->data[0] == '>') {
                wtk_owl_restore(owl);
            }
            break;
        case WTK_OWL_XML_WAIT_KEY:
            if (v->len > 1 || !isspace(v->data[0])) {
                if (v->len == 1) {
                    if (v->data[0] == '/') {
                        wtk_owl_pop_state(owl);
                        owl->sub_state = WTK_OWL_XML_END;
                        return 0;
                    } else if (v->data[0] == '>') {
                        wtk_owl_restore(owl);
                        return 0;
                    }
                }
                wtk_strbuf_reset(buf);
                wtk_strbuf_push(buf, v->data, v->len);
                owl->sub_state = WTK_OWL_XML_KEY;
            }
            break;
        case WTK_OWL_XML_KEY:
            if (v->len == 1 && (v->data[0] == '=' || isspace(v->data[0]))) {
                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
//			if(wtk_str_equal_s(buf->data,buf->pos,"v")==0)
//			{
//				wtk_debug("[%.*s]\n",buf->pos,buf->data);
//				return -1;
//			}else
//			{
//				if(v->data[0]=='=')
//				{
//					owl->sub_state=WTK_OWL_XML_K_WAIT_VALUE;
//				}else
//				{
//					owl->sub_state=WTK_OWL_XML_WAIT_EQ;
//				}
//			}
                if (wtk_str_equal_s(buf->data, buf->pos, "v")) {
                    owl->k = NULL;
                } else {
                    wtk_owl_item_attr_kv_t *kv;

                    item = data_offset2(owl->state_q.push, wtk_owl_state_item_t,
                            q_n);
                    oi = wtk_owl_state_item_get_item(item);
                    kv = wtk_owl_tree_find_attr(owl->tree, oi, buf->data,
                            buf->pos, 1);
                    //wtk_debug("[%.*s]\n",kv->k->len,kv->k->data);
                    owl->k = kv->k;
                }
                if (v->data[0] == '=') {
                    owl->sub_state = WTK_OWL_XML_K_WAIT_VALUE;
                } else {
                    owl->sub_state = WTK_OWL_XML_WAIT_EQ;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_XML_WAIT_EQ:
            if (v->len == 1 && v->data[0] == '=') {
                owl->sub_state = WTK_OWL_XML_K_WAIT_VALUE;
            }
            break;
        case WTK_OWL_XML_K_WAIT_VALUE:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                if (v->data[0] == '"') {
                    owl->sub_state = WTK_OWL_XML_K_VALUE_QUOTE;
                } else {
                    wtk_strbuf_push(buf, v->data, v->len);
                    owl->sub_state = WTK_OWL_XML_K_VALUE;
                }
            }
            break;
        case WTK_OWL_XML_K_VALUE:
            if (v->len == 1
                    && (v->data[0] == '/' || v->data[0] == '>'
                            || isspace(v->data[0]))) {
                item = data_offset2(owl->state_q.push, wtk_owl_state_item_t,
                        q_n);
                oi = wtk_owl_state_item_get_item(item);
                if (oi) {
                    if (owl->k) {
                        wtk_owl_tree_add_attr(owl->tree, oi, owl->k->data,
                                owl->k->len, buf->data, buf->pos);
                    } else {
                        wtk_owl_tree_set_item_value(owl->tree, item, oi,
                                buf->data, buf->pos);
                    }
                    owl->sub_state = WTK_OWL_XML_WAIT_KEY;
                }
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_XML_K_VALUE_QUOTE:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            if (v->len == 1 && v->data[0] == '\"') {
                item = data_offset2(owl->state_q.push, wtk_owl_state_item_t,
                        q_n);
                //wtk_debug("item=type=%d\n",item->type);
                oi = wtk_owl_state_item_get_item(item);
                if (oi) {
                    //wtk_debug("k=%p [%.*s]\n",owl->k,buf->pos,buf->data);
                    if (owl->k) {
                        //wtk_debug("[%.*s]\n",owl->k->len,owl->k->data);
                        //exit(0);
                        wtk_owl_tree_add_attr(owl->tree, oi, owl->k->data,
                                owl->k->len, buf->data, buf->pos);
                    } else {
                        wtk_owl_tree_set_item_value(owl->tree, item, oi,
                                buf->data, buf->pos);
                    }
                    //wtk_owl_tree_set_item_value(owl->tree,item,oi,buf->data,buf->pos);
                    owl->sub_state = WTK_OWL_XML_WAIT_KEY;
                } else {
                    return -1;
                }
            } else if (v->len == 1 && v->data[0] == '\\') {
                owl->sub_state = WTK_OWL_XML_K_VALUE_QUOTE_ESC;
            } else {
                wtk_strbuf_push(buf, v->data, v->len);
            }
            break;
        case WTK_OWL_XML_K_VALUE_QUOTE_ESC:
            wtk_strbuf_push(buf, v->data, v->len);
            owl->sub_state = WTK_OWL_XML_K_VALUE_QUOTE;
            break;
    }

    return 0;
}

int wtk_owl_feed_string(wtk_owl_t *owl, wtk_string_t *v)
{
    enum wtk_owl_string_state {
        WTK_OWL_STRING_INIT = -1,
        WTK_OWL_STRING_WRD,
        WTK_OWL_STRING_ESC,
        WTK_OWL_STRING_VAR_DOLLAR,
        WTK_OWL_STRING_VAR_WRD,
    };
    wtk_strbuf_t *buf = owl->buf;
    char c;

    switch (owl->sub_state) {
        case WTK_OWL_STRING_INIT:
            if (v->len > 1 || !isspace(v->data[0])) {
                wtk_strbuf_reset(buf);
                if (v->data[0] == '\"') {
                    owl->quote = 1;
                } else {
                    owl->quote = 0;
                    wtk_strbuf_push(buf, v->data, v->len);
                }
                owl->sub_state = WTK_OWL_STRING_WRD;
            }
            break;
        case WTK_OWL_STRING_ESC:
            wtk_strbuf_push(buf, v->data, v->len);
            owl->sub_state = WTK_OWL_STRING_WRD;
            break;
        case WTK_OWL_STRING_VAR_DOLLAR:
            if (v->len == 1 && v->data[0] == '{') {
                owl->sub_state = WTK_OWL_STRING_VAR_WRD;
                wtk_strbuf_reset(owl->tmp);
            }
            break;
        case WTK_OWL_STRING_VAR_WRD:
            if (v->len == 1 && v->data[0] == '}') {
                wtk_strbuf_strip(owl->tmp);
                if (wtk_str_equal_s(owl->tmp->data, owl->tmp->pos, "pwd")) {
                    wtk_strbuf_push(buf, owl->pwd->data, owl->pwd->len);
                    owl->sub_state = WTK_OWL_STRING_WRD;
                }
            } else {
                wtk_strbuf_push(owl->tmp, v->data, v->len);
            }
            break;
        case WTK_OWL_STRING_WRD:
            if (v->len > 1) {
                wtk_strbuf_push(buf, v->data, v->len);
            } else {
                c = v->data[0];
                switch (c) {
                    case '$':
                        owl->sub_state = WTK_OWL_STRING_VAR_DOLLAR;
                        break;
                    case '\\':
                        owl->sub_state = WTK_OWL_STRING_ESC;
                        break;
                    default:
                        if (owl->quote) {
                            if (c == '\"') {
                                wtk_owl_restore_string_state(owl);
                                //wtk_debug("[%.*s]\n",buf->pos,buf->data);
                                return wtk_owl_feed(owl, NULL);
                            } else {
                                wtk_strbuf_push(buf, v->data, v->len);
                            }
                        } else {
                            if (isspace(c)) {
                                wtk_owl_restore_string_state(owl);
                                return wtk_owl_feed(owl, NULL);
                            } else {
                                wtk_strbuf_push(buf, v->data, v->len);
                            }
                        }
                        break;
                }
            }
            break;
    }
    return 0;
}

int wtk_owl_feed(wtk_owl_t *owl, wtk_string_t *v)
{
    int ret;

    //wtk_debug("[%.*s]\n",v->len,v->data);
    switch (owl->state) {
        case WTK_OWL_STATE_INIT:
            ret = wtk_owl_feed_init(owl, v);
            break;
        case WTK_OWL_STATE_COMMENT1:
            ret = wtk_owl_feed_comment1(owl, v);
            break;
        case WTK_OWL_STATE_COMMENT2:
            ret = wtk_owl_feed_comment2(owl, v);
            break;
        case WTK_OWL_XML_ITEM:
            ret = wtk_owl_feed_xml_item(owl, v);
            break;
        case WTK_OWL_STATE_STRING:
            ret = wtk_owl_feed_string(owl, v);
            break;
        default:
            wtk_debug("[%.*s]\n", v->len, v->data)
            ;
            ret = -1;
            exit(0);
            break;
    }
    return ret;
}

int wtk_owl_compile_string(wtk_owl_t *l, char *data, int bytes)
{
    char *s, *e;
    int n;
    int ret = 0;
    wtk_string_t v;

    s = data;
    e = s + bytes;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        wtk_string_set(&(v), s, n);
        //wtk_debug("%.*s\n",v.len,v.data);
        ret = wtk_owl_feed(l, &v);
        if (ret != 0) {
            wtk_debug("err: state=%d/%d [%.*s]\n", l->state, l->sub_state, n, s);
            printf("%.*s\n", (int) (e - s), s);
            goto end;
        }
        s += n;
    }
    ret = 0;
    end: return ret;
}

wtk_owl_tree_t* wtk_owl_compile(wtk_owl_t *l, char *data, int bytes)
{
    int ret = 0;
    wtk_owl_tree_t *tree;

    //wtk_debug("[%.*s]\n",bytes,data);
    tree = wtk_owl_tree_new();
    l->tree = tree;
    ret = wtk_owl_compile_string(l, data, bytes);
    if (ret != 0) {
        goto end;
    }
    wtk_owl_tree_update(tree);
    ret = 0;
    end: if (ret != 0) {
        wtk_owl_tree_delete(tree);
        tree = NULL;
    }
    //wtk_owl_tree_print(tree);
    //wtk_debug("ret=%d st=%d\n",ret,l->state_q.length);
    return tree;
}

wtk_owl_tree_t* wtk_owl_compile_file(wtk_owl_t *owl, char *fn)
{
    wtk_owl_tree_t *tree = NULL;
    char *data = NULL;
    int n;
    wtk_string_t v;
    wtk_rbin2_item_t *item = NULL;

    if (owl->rbin) {
        item = wtk_rbin2_get3(owl->rbin, fn, strlen(fn), 0);
        if (!item) {
            goto end;
        }
        data = item->data->data;
        n = item->data->len;
    } else {
        data = file_read_buf(fn, &n);
        if (!data) {
            goto end;
        }
    }
    v = wtk_dir_name2(fn, strlen(fn), '/');
    //wtk_debug("v[%.*s]\n",v.len,v.data);
    owl->pwd = wtk_heap_dup_string(owl->heap, v.data, v.len);
    tree = wtk_owl_compile(owl, data, n);
    end: if (owl->rbin) {
        if (item) {
            wtk_rbin2_item_clean(item);
        }
    } else {
        if (data) {
            wtk_free(data);
        }
    }
    return tree;
}

wtk_owl_tree_t* wtk_owl_tree_new_fn(char *fn)
{
    wtk_owl_t *owl;
    wtk_owl_tree_t *tree;

    owl = wtk_owl_new();
    tree = wtk_owl_compile_file(owl, fn);
    wtk_owl_delete(owl);
    return tree;
}

wtk_owl_tree_t* wtk_owl_tree_new_fn2(wtk_rbin2_t *rbin, char *fn)
{
    wtk_owl_t *owl;
    wtk_owl_tree_t *tree;

    owl = wtk_owl_new();
    owl->rbin = rbin;
    tree = wtk_owl_compile_file(owl, fn);
    wtk_owl_delete(owl);
    return tree;
}
