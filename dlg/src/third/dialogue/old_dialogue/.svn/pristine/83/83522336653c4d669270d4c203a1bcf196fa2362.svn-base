#include "wtk_nlg2_key.h" 
#include "wtk/core/wtk_sort.h"
#include "wtk/core/wtk_larray.h"
#include <ctype.h>

void wtk_nlg2_function_init(wtk_nlg2_function_t *f)
{
    f->name = NULL;
    f->args = NULL;
    f->narg = 0;
}

int wtk_nlg2_function_nv(wtk_nlg2_function_t *f)
{
    int i;
    int v = 0;

    for (i = 0; i < f->narg; ++i) {
        if (f->args[i]->v) {
            ++v;
        }
    }
    return v;
}

int wtk_nlg2_function_match(wtk_nlg2_function_t *v1, wtk_nlg2_function_t *v2)
{
    int i;

    for (i = 0; i < v1->narg; ++i) {
        if (v1->args[i]->v
                && (!v2->args[i]->v
                        || wtk_string_cmp(v1->args[i]->v, v2->args[i]->v->data,
                                v2->args[i]->v->len) != 0)) {
            return 0;
        }
    }
    return 1;
}

int wtk_nlg2_function_cmp(wtk_nlg2_function_t *v1, wtk_nlg2_function_t *v2)
{
    int n1, n2;

    n1 = wtk_nlg2_function_nv(v1);
    n2 = wtk_nlg2_function_nv(v2);
    //wtk_debug("n1=%d n2=%d\n",n1,n2);
    return n1 - n2;
}

void wtk_nlg2_function_print_key(wtk_nlg2_function_t *item, wtk_strbuf_t *buf)
{
    int i;

    wtk_strbuf_reset(buf);
    wtk_strbuf_push(buf, item->name->data, item->name->len);
    wtk_strbuf_push_s(buf, "(");
    for (i = 0; i < item->narg; ++i) {
        if (i > 0) {
            wtk_strbuf_push_s(buf, ",");
        }
        wtk_strbuf_push(buf, item->args[i]->k->data, item->args[i]->k->len);
    }
    wtk_strbuf_push_s(buf, ")");
}

void wtk_nlg2_function_print(wtk_nlg2_function_t *item)
{
    int i;

    printf("%.*s(", item->name->len, item->name->data);
    for (i = 0; i < item->narg; ++i) {
        if (i > 0) {
            printf(",");
        }
        //printf("%.*s key=%p",item->args[i]->k->len,item->args[i]->k->data,item->args[i]);
        printf("%.*s", item->args[i]->k->len, item->args[i]->k->data);
        if (item->args[i]->v) {
            printf("=\"%.*s\"", item->args[i]->v->len, item->args[i]->v->data);
        }
    }
    printf(")\n");
}

wtk_string_t* wtk_nlg2_function_get(wtk_nlg2_function_t *item, char *k,
        int k_len)
{
    int i;

    for (i = 0; i < item->narg; ++i) {
        if (wtk_string_cmp(item->args[i]->k, k, k_len) == 0) {
            return item->args[i]->v;
        }
    }
    return NULL;
}

wtk_nlg2_key_t* wtk_nlg2_key_new(wtk_heap_t *heap, char *nm, int nm_len)
{
    wtk_nlg2_key_t *k;

    k = (wtk_nlg2_key_t*) wtk_heap_malloc(heap, sizeof(wtk_nlg2_key_t));
    if (nm_len > 0) {
        k->k = wtk_heap_dup_string(heap, nm, nm_len);
    } else {
        k->k = NULL;
    }
    k->v = NULL;
    return k;
}

float wtk_nlg2_key_cmp(void *ths, wtk_nlg2_key_t **k1, wtk_nlg2_key_t **k2)
{
    //wtk_debug("[%.*s]=[%.*s]\n",(*k1)->k->len,(*k1)->k->data,(*k2)->k->len,(*k2)->k->data);
    return wtk_string_cmp((*k1)->k, (*k2)->k->data, (*k2)->k->len);
}

void wtk_nlg2_function_add_arg(wtk_nlg2_function_t *f, wtk_heap_t *heap,
        wtk_nlg2_key_t **keys, int nkey)
{
    int n;

    n = nkey * sizeof(wtk_nlg2_key_t*);
    f->args = (wtk_nlg2_key_t**) wtk_heap_malloc(heap, n);
    f->narg = nkey;
    memcpy(f->args, keys, n);
    wtk_qsort2(f->args, nkey, sizeof(wtk_nlg2_key_t*),
            (wtk_qsort_cmp_f) wtk_nlg2_key_cmp, NULL);
    //wtk_nlg2_item_print(item);
//	{
//		int i;
//
//		for(i=0;i<nkey;++i)
//		{
//			wtk_debug("v[%d]=%p/%p\n",i,f->args[i],keys[i]);
//		}
//	}
}

typedef struct {
    wtk_heap_t *heap;
    wtk_strbuf_t *buf;
    wtk_string_parser2_t *ps;
    wtk_nlg2_function_t *f;
    wtk_larray_t *args;
    int sub_state;
} wtk_nlg2_keyp_t;

int wtk_nlg2_function_feed_action(wtk_nlg2_keyp_t *p, wtk_string_t *v)
{
    enum {
        WTK_NLG2_ACTION_INIT = -1,
        WTK_NLG2_ACTION_NAME,
        WTK_NLG2_ACTION_ARG_WAIT,
        WTK_NLG2_ACTION_ARG_WAIT_K,
        WTK_NLG2_ACTION_ARG_K,
        WTK_NLG2_ACTION_ARG_WAIT_V,
        WTK_NLG2_ACTION_ARG_V,
    };
    wtk_string_parser2_t * ps = p->ps;
    int ret = 0;
    wtk_nlg2_key_t *key;
    char c;

    switch (p->sub_state) {
        case WTK_NLG2_ACTION_INIT:
            wtk_string_parser2_reset(ps);
            p->sub_state = WTK_NLG2_ACTION_NAME;
            ret = wtk_string_parse2(ps, v);
            break;
        case WTK_NLG2_ACTION_NAME:
            ret = wtk_string_parse2(ps, v);
            switch (ret) {
                case 1:
                    //wtk_debug("[%.*s]\n",ps->buf->pos,ps->buf->data);
                    p->f->name = wtk_heap_dup_string(p->heap, ps->buf->data,
                            ps->buf->pos);
                    p->sub_state = WTK_NLG2_ACTION_ARG_WAIT;
                    break;
                case 2:
                    p->sub_state = WTK_NLG2_ACTION_ARG_WAIT;
                    p->f->name = wtk_heap_dup_string(p->heap, ps->buf->data,
                            ps->buf->pos);
                    //wtk_debug("[%.*s]\n",ps->buf->pos,ps->buf->data);
                    //exit(0);
                    return wtk_nlg2_function_feed_action(p, v);
                    break;
            }
            break;
        case WTK_NLG2_ACTION_ARG_WAIT:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            if (v->len == 1 && v->data[0] == '(') {
                p->sub_state = WTK_NLG2_ACTION_ARG_WAIT_K;
                wtk_larray_reset(p->args);
            }
            //exit(0);
            break;
        case WTK_NLG2_ACTION_ARG_WAIT_K:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            if (v->len == 1 && v->data[0] == ')') {
                p->sub_state = WTK_NLG2_ACTION_ARG_WAIT_V;
                return wtk_nlg2_function_feed_action(p, v);
                //wtk_debug("n=%d\n",p->args->nslot);
                //exit(0);
            } else if (v->len > 1 || !isspace(v->data[0])) {
                wtk_string_parser2_reset(ps);
                p->sub_state = WTK_NLG2_ACTION_ARG_K;
                ret = wtk_string_parse2(ps, v);
            }
            break;
        case WTK_NLG2_ACTION_ARG_K:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            ret = wtk_string_parse2(ps, v);
            switch (ret) {
                case 1:
                    //wtk_debug("[%.*s]\n",ps->buf->pos,ps->buf->data);
                    key = wtk_nlg2_key_new(p->heap, ps->buf->data,
                            ps->buf->pos);
                    wtk_larray_push2(p->args, &(key));
                    p->sub_state = WTK_NLG2_ACTION_ARG_WAIT_V;
                    break;
                case 2:
                    //wtk_debug("[%.*s]\n",ps->buf->pos,ps->buf->data);
                    key = wtk_nlg2_key_new(p->heap, ps->buf->data,
                            ps->buf->pos);
                    wtk_larray_push2(p->args, &(key));
                    p->sub_state = WTK_NLG2_ACTION_ARG_WAIT_V;
                    return wtk_nlg2_function_feed_action(p, v);
                    break;
            }
            ret = 0;
            break;
        case WTK_NLG2_ACTION_ARG_WAIT_V:
            if (v->len == 1) {
                c = v->data[0];
                switch (c) {
                    case ',':
                        p->sub_state = WTK_NLG2_ACTION_ARG_WAIT_K;
                        break;
                    case '=':
                        p->sub_state = WTK_NLG2_ACTION_ARG_V;
                        wtk_string_parser2_reset(ps);
                        break;
                    case ')':
                        //wtk_debug("%d\n",p->args->nslot);
                        if (p->args->nslot > 0) {
                            wtk_nlg2_function_add_arg(p->f, p->heap,
                                    (wtk_nlg2_key_t**) p->args->slot,
                                    p->args->nslot);
                        }
                        //exit(0);
                        wtk_larray_reset(p->args);
                        //wtk_nlg2_parser_set_state(p,WTK_NLG2_ITEM);
                        return 1;
                        break;
                }
            }
            break;
        case WTK_NLG2_ACTION_ARG_V:
            //wtk_debug("[%.*s]\n",v->len,v->data);
            ret = wtk_string_parse2(ps, v);
            switch (ret) {
                case 1:
                    //key=(wtk_nlg2_key_t*)wtk_larray_get(p->args,p->args->nslot-1);
                    //wtk_debug("[%.*s] key=%p %.*s\n",ps->buf->pos,ps->buf->data,key,key->k->len,key->k->data);
                    key =
                            ((wtk_nlg2_key_t**) p->args->slot)[p->args->nslot
                                    - 1];
                    key->v = wtk_heap_dup_string(p->heap, ps->buf->data,
                            ps->buf->pos);
                    //wtk_debug("[%.*s] key=%p %.*s\n",ps->buf->pos,ps->buf->data,key,key->k->len,key->k->data);
                    p->sub_state = WTK_NLG2_ACTION_ARG_WAIT_V;
                    break;
                case 2:
                    //wtk_debug("[%.*s]\n",ps->buf->pos,ps->buf->data);
                    //key=(wtk_nlg2_key_t*)wtk_larray_get(p->args,p->args->nslot-1);
                    key =
                            ((wtk_nlg2_key_t**) p->args->slot)[p->args->nslot
                                    - 1];
                    key->v = wtk_heap_dup_string(p->heap, ps->buf->data,
                            ps->buf->pos);
                    p->sub_state = WTK_NLG2_ACTION_ARG_WAIT_V;
                    return wtk_nlg2_function_feed_action(p, v);
                    break;
            }
            ret = 0;
            break;
    }
    //wtk_debug("ret=%d\n",ret);
    return ret;
}

int wtk_nlg2_function_parse(wtk_nlg2_function_t *f, wtk_heap_t* heap, char *s,
        int len, int *consume)
{
    wtk_nlg2_keyp_t kp;
    char *e;
    int n;
    wtk_string_t v;
    int ret;
    char *t = s;

    //wtk_debug("[%.*s]\n",len,s);
    kp.heap = heap;
    kp.buf = wtk_strbuf_new(256, 1);
    kp.sub_state = -1;
    kp.ps = wtk_string_parser2_new();
    kp.f = f;
    kp.args = wtk_larray_new(4, sizeof(void*));
    e = s + len;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        wtk_string_set(&(v), s, n);
        ret = wtk_nlg2_function_feed_action(&kp, &(v));
        if (ret == 1) {
            s += n;
            break;
        } else if (ret != 0) {
            goto end;
        }
        s += n;
    }
    if (consume) {
        *consume = s - t;
    }
    //wtk_debug("ret=%d\n",ret);
    //wtk_nlg2_function_print(f);
    ret = 0;
    end:
    //exit(0);
    wtk_strbuf_delete(kp.buf);
    wtk_string_parser2_delete(kp.ps);
    wtk_larray_delete(kp.args);
    //exit(0);
    return ret;
}
