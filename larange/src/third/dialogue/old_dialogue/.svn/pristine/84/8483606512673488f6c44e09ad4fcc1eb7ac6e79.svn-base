#include "wtk_faqbin.h"
#include "wtk/core/wtk_os.h"

wtk_faqbin_item_t* wtk_faqbin_item_new()
{
    wtk_faqbin_item_t *item;

    item = (wtk_faqbin_item_t*) wtk_malloc(sizeof(wtk_faqbin_item_t));
    item->v = NULL;
    item->left = NULL;
    item->right = NULL;
    item->offset = 0;

    return item;
}

void wtk_faqbin_item_delete(wtk_faqbin_item_t *item)
{
    if (item->left) {
        wtk_faqbin_item_delete(item->left);
    }
    if (item->right) {
        wtk_faqbin_item_delete(item->right);
    }
    if (item->v) {
        wtk_vecf_delete(item->v);
    }
    wtk_free(item);
}

int wtk_faqbin_load_item(wtk_faqbin_t *faq, FILE *f, wtk_faqbin_item_t *item)
{
    char c;
    int ret;
    //static int cls=0;

    //++cls;
    //wtk_debug("cls=%d\n",cls);
    //wtk_debug("vec=%d\n",faq->vec_size);
    ret = fread(&c, 1, 1, f);
    if (ret != 1) {
        goto end;
    }
    //wtk_debug("c=%d\n",c);
    if (c & 0x1) {
        ret = fread((char*) &(item->offset), 4, 1, f);
        if (ret != 1) {
            ret = -1;
            goto end;
        }
    }
    if (c & 0x2) {
        item->v = wtk_vecf_new(faq->vec_size);
        ret = fread((char*) item->v->p, sizeof(float), faq->vec_size, f);
        if (ret != faq->vec_size) {
            ret = -1;
            goto end;
        }
    }
    if (c & 0x4) {
        item->left = wtk_faqbin_item_new();
        ret = wtk_faqbin_load_item(faq, f, item->left);
        if (ret != 0) {
            goto end;
        }
    }
    if (c & 0x8) {
        item->right = wtk_faqbin_item_new();
        ret = wtk_faqbin_load_item(faq, f, item->right);
        if (ret != 0) {
            goto end;
        }
    }
    ret = 0;
    end:
//	{
//		static int ki=0;
//		++ki;
//		wtk_debug("ret=%d %d\n",ret,ki);
//	}
    return ret;
}

int wtk_faqbin_load(wtk_faqbin_t *faq, FILE *f)
{
    int ret;

    if (faq->f_of) {
        ret = fseek(f, faq->f_of, SEEK_SET);
        if (ret != 0) {
            wtk_debug("set of=%d failed\n", faq->f_of);
            goto end;
        }
    }
    ret = fread(&(faq->vec_size), 4, 1, f);
    if (ret != 1) {
        goto end;
    }
    //wtk_debug("vec=%d\n",faq->vec_size);
    faq->root = wtk_faqbin_item_new();
    ret = wtk_faqbin_load_item(faq, f, faq->root);
    if (ret != 0) {
        goto end;
    }
    end: if (ret == 0) {
        faq->offset = ftell(f);
    }
    return ret;
}

wtk_faqbin_t* wtk_faqbin_new(char *fn, wtk_rbin2_t *rbin)
{
    wtk_faqbin_t *f;

    f = (wtk_faqbin_t*) wtk_malloc(sizeof(wtk_faqbin_t));
    f->rbin = rbin;
    f->root = NULL;
    f->offset = 0;
    f->heap = wtk_heap_new(4096);
    f->buf = wtk_strbuf_new(256, 1);
    f->rb = wtk_robin_new(100);
    f->thresh = 0.5;
    f->best_thresh = 1.0;
    f->f_of = 0;
    f->f_len = -1;
    if (rbin) {
        wtk_rbin2_item_t *item;

        item = wtk_rbin2_get(rbin, fn, strlen(fn));
        //wtk_debug("seek %s item=%p\n",fn,item);
        if (item) {
            f->f = rbin->f;
            f->f_of = item->pos;
            f->f_len = item->len;
        } else {
            wtk_debug("seek %s failed\n", fn);
            f->f = NULL;
        }
    } else {
        f->f = fopen(fn, "rb");
    }
    //wtk_debug("%s %p/%p\n",fn,f->f,rbin);
    wtk_faqbin_load(f, f->f);
    f->v = wtk_vecf_new(f->vec_size);
    return f;
}

void wtk_faqbin_delete(wtk_faqbin_t *faq)
{
    if (faq->root) {
        wtk_faqbin_item_delete(faq->root);
    }
    wtk_robin_delete(faq->rb);
    wtk_strbuf_delete(faq->buf);
    wtk_heap_delete(faq->heap);
    if (!faq->rbin && faq->f) {
        fclose(faq->f);
    }
    wtk_vecf_delete(faq->v);
    wtk_free(faq);
}

void wtk_faqbin_reset(wtk_faqbin_t *faq)
{
    wtk_heap_reset(faq->heap);
}

wtk_faqbin_item_t* wtk_faqbin_get_cls(wtk_faqbin_item_t *cls, wtk_vecf_t *v)
{
    float f1, f2;
    wtk_faqbin_item_t *item;

    //wtk_debug("offset=%d left=%p right=%p\n",cls->offset,cls->left,cls->right);
    if (cls->offset > 0) {
        item = cls;
    } else {
        if (cls->right && cls->left) {
            f1 = wtk_vecf_dist(cls->right->v, v);
            f2 = wtk_vecf_dist(cls->left->v, v);
            //wtk_debug("f1=%f f2=%f of=%d\n",f1,f2,cls->offset);
            if (f1 < f2) {
                item = wtk_faqbin_get_cls(cls->right, v);
            } else {
                item = wtk_faqbin_get_cls(cls->left, v);
            }
        } else {
            if (cls->right) {
                item = wtk_faqbin_get_cls(cls->right, v);
            } else if (cls->left) {
                item = wtk_faqbin_get_cls(cls->left, v);
            } else {
                item = cls;
            }
        }
    }
    return item;
}

void wtk_faqbin_item_print(wtk_faqbin_item_t *item)
{
    wtk_debug("len=%d\n", item->offset);
}

int wtk_faqbin_item_load2(wtk_faqbin_t *faq, wtk_faqbin_item_t *item, FILE *f,
        wtk_heap_t *heap, wtk_vecf_t *v)
{
    wtk_strbuf_t *buf = faq->buf;
    short si;
    int i, n, nx;
    int ret;
    float t;
    float max_f = -1e10;
    int ti;
    wtk_robin_t *rb;
    wtk_string_t *str;

    rb = faq->rb;
    ret = fseek(f, item->offset + faq->offset, SEEK_SET);
    if (ret != 0) {
        goto end;
    }
    ret = fread((char*) &n, 4, 1, f);
    if (ret != 1) {
        ret = -1;
        goto end;
    }
    //wtk_debug("n=%d\n",n);
    for (i = 0; i < n; ++i) {
        ret = fread(faq->v->p, sizeof(float), faq->vec_size, f);
        if (ret != faq->vec_size) {
            ret = -1;
            goto end;
        }
        t = wtk_vecf_cos(v, faq->v);
        ret = fread((char*) &nx, 4, 1, f);
        if (ret != 1) {
            ret = -1;
            goto end;
        }
        if (t > max_f) {
            ti = 0;
            wtk_robin_reset(rb);
            max_f = t;
            faq->prob = max_f;
            while (ti < nx) {
                ret = fread((char*) &si, 2, 1, f);
                if (ret != 1) {
                    ret = -1;
                    goto end;
                }
                //wtk_debug("si=%d,nx=%d\n",si,nx);
                wtk_strbuf_reset(buf);
                wtk_strbuf_expand(buf, si);
                buf->pos = si;
                ret = fread(buf->data, si, 1, f);
                if (ret != 1) {
                    ret = -1;
                    goto end;
                }
                wtk_debug("[%.*s]\n", buf->pos, buf->data);
                str = wtk_heap_dup_string(heap, buf->data, buf->pos);
                wtk_robin_push(rb, str);
                ti += 2 + si;
                //wtk_debug("ti=%d/%d\n",ti,nx);
                //exit(0);
            }
        } else {
            ret = fseek(f, nx, SEEK_CUR);
            if (ret != 0) {
                goto end;
            }
        }
    }
    ret = 0;
    end: return ret;
}

int wtk_faqbin_item_load(wtk_faqbin_t *faq, wtk_faqbin_item_t *item, FILE *f,
        wtk_heap_t *heap, wtk_vecf_t *v)
{
    wtk_strbuf_t *buf = faq->buf;
    short si;
    int i, n, nx;
    int ret;
    float t;
    float max_f = -1e10;
    int ti;
    wtk_robin_t *rb;
    wtk_string_t *str;
    int has_best = 0;

    rb = faq->rb;
    ret = fseek(f, item->offset + faq->offset, SEEK_SET);
    if (ret != 0) {
        goto end;
    }
    ret = fread((char*) &n, 4, 1, f);
    if (ret != 1) {
        ret = -1;
        goto end;
    }
    //wtk_debug("n=%d\n",n);
    wtk_robin_reset(rb);
    for (i = 0; i < n; ++i) {
        //wtk_debug("i=%d/%d\n",i,n);
        ret = fread(faq->v->p, sizeof(float), faq->vec_size, f);
        if (ret != faq->vec_size) {
            ret = -1;
            goto end;
        }
        t = wtk_vecf_cos(v, faq->v);
        ret = fread((char*) &nx, 4, 1, f);
        if (ret != 1) {
            ret = -1;
            goto end;
        }
        //wtk_debug("t=%f\n",t);
        //wtk_debug("v[%d/%d]=%f nx=%d t=%f max_f=%f thread=%f/%f\n",i,n,t,nx,t,max_f,faq->thresh,faq->best_thresh);
        //exit(0);
        if ((t > max_f && t > faq->thresh) || (t > faq->best_thresh)) {
            if (has_best == 0 && t > faq->best_thresh) {
                has_best = 1;
                wtk_robin_reset(rb);
            }
            if (nx > 1 && t > max_f) {
                wtk_robin_reset(rb);
            } else {
                if (rb->used > 1 && t < max_f) {
                    ret = fseek(f, nx, SEEK_CUR);
                    if (ret != 0) {
                        goto end;
                    }
                    continue;
                }
            }
            ti = 0;
            max_f = t;
            faq->prob = max_f;
            while (ti < nx) {
                ret = fread((char*) &si, 2, 1, f);
                if (ret != 1) {
                    wtk_debug("read failed\n");
                    ret = -1;
                    goto end;
                }
                //wtk_debug("si=%d,nx=%d\n",si,nx);
                wtk_strbuf_reset(buf);
                wtk_strbuf_expand(buf, si);
                buf->pos = si;
                ret = fread(buf->data, si, 1, f);
                if (ret != 1) {
                    wtk_debug("read failed\n");
                    ret = -1;
                    goto end;
                }
                //wtk_debug("%f: [%.*s] used=%d\n",t,buf->pos,buf->data,rb->used);
                str = wtk_heap_dup_string(heap, buf->data, buf->pos);
                wtk_robin_push(rb, str);
                ti += 2 + si;
                //wtk_debug("ti=%d/%d\n",ti,nx);
                //exit(0);
            }
            if (t >= 1.0) {
                ret = 0;
                goto end;
            }
        } else {
            ret = fseek(f, nx, SEEK_CUR);
            if (ret != 0) {
                goto end;
            }
        }
    }
    ret = 0;
    end:
    //wtk_debug("used=%d,ret=%d\n",rb->used,ret);
    return ret;
}

wtk_string_t wtk_faqbin_get(wtk_faqbin_t *faq, wtk_vecf_t *v)
{
    wtk_faqbin_item_t* item;
    wtk_heap_t *heap = faq->heap;
    wtk_strbuf_t *buf = faq->buf;
    ;
    wtk_string_t str;
    int ret;
    int idx;
    wtk_string_t *x;
    int n;

    wtk_string_set(&(str), 0, 0);
    faq->prob = 0;
    wtk_robin_reset(faq->rb);
    item = wtk_faqbin_get_cls(faq->root, v);
    if (!item || item->offset < 0) {
        goto end;
    }
    ret = wtk_faqbin_item_load(faq, item, faq->f, heap, v);
    if (ret != 0) {
        goto end;
    }
    n = min(faq->rb->used, faq->rb->nslot);
    if (n <= 0) {
        goto end;
    }
    //wtk_debug("used=%d nslot=%d\n",faq->rb->used,faq->rb->nslot);
    idx = ((unsigned long) (time_get_ms())) % (n);
    //wtk_debug("idx=%d n=%d\n",idx,n);
    x = (wtk_string_t*) wtk_robin_at(faq->rb, idx);
    wtk_strbuf_reset(buf);
    wtk_strbuf_push(buf, x->data, x->len);
    wtk_string_set(&(str), buf->data, buf->pos);
    end:
    //wtk_debug("[%.*s]=%f\n",str.len,str.data,faq->prob);
    wtk_heap_reset(heap);
    return str;
}

wtk_robin_t* wtk_faqbin_get2(wtk_faqbin_t *faq, wtk_vecf_t *v)
{
    wtk_faqbin_item_t* item;
    wtk_heap_t *heap = faq->heap;
    wtk_string_t str;
    int ret;
    wtk_robin_t *rb = NULL;

    wtk_string_set(&(str), 0, 0);
    faq->prob = 0;
    wtk_robin_reset(faq->rb);
    item = wtk_faqbin_get_cls(faq->root, v);
    if (!item || item->offset < 0) {
        goto end;
    }
    ret = wtk_faqbin_item_load(faq, item, faq->f, heap, v);
    if (ret != 0) {
        goto end;
    }
    rb = faq->rb;
    end:
    //wtk_debug("[%.*s]=%f\n",str.len,str.data,faq->prob);
    return rb;
}
