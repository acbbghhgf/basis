#include "wtk_faq2bin.h" 

wtk_faq2bin_t* wtk_faq2bin_new(wtk_faq2bin_cfg_t *cfg)
{
    wtk_faq2bin_t *faq;

    faq = (wtk_faq2bin_t*) wtk_malloc(sizeof(wtk_faq2bin_t));
    faq->cfg = cfg;
    faq->wrdvec = wtk_wrdvec_new(&(cfg->wrdvec), NULL);
    faq->heap = wtk_heap_new(4096);
    wtk_queue_init(&(faq->item_q));
    wtk_queue_init(&(faq->cls_q));
    return faq;
}

void wtk_faq2bin_delete(wtk_faq2bin_t *faq)
{
    wtk_heap_delete(faq->heap);
    wtk_wrdvec_delete(faq->wrdvec);
    wtk_free(faq);
}

wtk_qa_item_t* wtk_qa_item_new(wtk_heap_t *heap)
{
    wtk_qa_item_t *item;

    item = (wtk_qa_item_t*) wtk_heap_malloc(heap, sizeof(wtk_qa_item_t));
    item->v = NULL;
    item->q = NULL;
    item->a = wtk_array_new_h(heap, 3, sizeof(wtk_string_t*));
    return item;
}

void wtk_qa_item_print(wtk_qa_item_t *item)
{
    int i;
    wtk_string_t **strs;

    printf("Q: %.*s\n", item->q->len, item->q->data);
    strs = (wtk_string_t**) (item->a->slot);
    for (i = 0; i < item->a->nslot; ++i) {
        printf("A[%d]: %.*s\n", i, strs[i]->len, strs[i]->data);
    }
}

int wtk_faq2bin_read(wtk_faq2bin_t *faq, wtk_source_t *src)
{
    wtk_wrdvec_t *v = faq->wrdvec;
    wtk_strbuf_t *buf;
    int ret;
    char *data;
    int len;
    float f;
    wtk_heap_t *heap = faq->heap;
    wtk_qa_item_t *qa = NULL;
    wtk_string_t *str;
    wtk_vecf_t *v1;
    int eof;

    v1 = v->v1;
    buf = wtk_strbuf_new(256, 1);
    while (1) {
        ret = wtk_source_read_line2(src, buf, &eof);
        if (ret != 0) {
            goto end;
        }
        if (buf->pos == 0) {
            if (eof) {
                break;
            } else {
                continue;
            }
        }
        //wtk_debug("%.*s\n",buf->pos,buf->data);
        if (buf->data[0] == '#') {
            if (qa) {
                data = buf->data;
                data = wtk_str_chr(buf->data, buf->pos, ' ');
                data += 1;
                len = buf->pos - (data - buf->data);
                str = wtk_heap_dup_string(heap, data, len);
                wtk_array_push2(qa->a, &(str));
            }
        } else {
            //printf("%.*s\n",buf->pos,buf->data);
            if (buf->data[0] == '^') {
                data = buf->data;
                data = wtk_str_chr(buf->data, buf->pos, ' ');
                data += 1;
                len = buf->pos - (data - buf->data);
            } else {
                data = buf->data;
                len = buf->pos;
            }
            f = wtk_wrdvec_snt_to_vec2(v, data, len, v1);
            if (f == 0) {
                qa = NULL;
                continue;
            }
            qa = wtk_qa_item_new(heap);
            qa->q = wtk_heap_dup_string(heap, data, len);
            qa->v = wtk_vecf_dup(v1);
            //wtk_debug("len=%d\n",qa->v->len);
            wtk_queue_push(&(faq->item_q), &(qa->q_n));
            //wtk_debug("f=%f\n",f);

        }
    }
    ret = 0;
    end:
    //exit(0);
    wtk_strbuf_delete(buf);
    return ret;
}

wtk_faq_cls_item_t* wtk_faq_cls_item_new(int len)
{
    wtk_faq_cls_item_t *item;
    static int i = 0;

    item = (wtk_faq_cls_item_t*) wtk_malloc(sizeof(wtk_faq_cls_item_t));
    wtk_queue_init(&(item->item_q));
    item->v = wtk_vecf_new(len);
    item->v2 = wtk_vecf_new(len);
    item->parent = NULL;
    item->left = NULL;
    item->right = NULL;
    item->n = 0;
    item->index = ++i;
    return item;
}

void wtk_faq_cls_item_delete(wtk_faq_cls_item_t *cls)
{
    wtk_vecf_delete(cls->v);
    wtk_free(cls);
}

wtk_faq_cls_item_t* wtk_faq2bin_init_cls(wtk_faq2bin_t *faq)
{
    wtk_faq_cls_item_t *cls;
    wtk_queue_node_t *qn;
    wtk_qa_item_t *qa;

    cls = wtk_faq_cls_item_new(faq->cfg->wrdvec.vec_size);
    wtk_queue_push(&(faq->cls_q), &(cls->q_n));
    for (qn = faq->item_q.pop; qn; qn = qn->next) {
        qa = data_offset2(qn, wtk_qa_item_t, q_n);
        wtk_queue_push(&(cls->item_q), &(qa->cls_n));
    }
    return cls;
}

wtk_faq_cls_item_t* wtk_faq2bin_cls_init(wtk_faq_cls_item_t *cls_old)
{
    wtk_faq_cls_item_t *cls;
    wtk_queue_node_t *qn;
    wtk_qa_item_t *item;
    int v;

    v = (int) (rand() * time_get_ms()) % (cls_old->item_q.length);
    //wtk_debug("v=%d\n",v);
    cls = wtk_faq_cls_item_new(cls_old->v->len);
    qn = wtk_queue_peek(&(cls_old->item_q), v);
    wtk_queue_remove(&(cls_old->item_q), qn);
    item = data_offset2(qn, wtk_qa_item_t, cls_n);
    wtk_queue_push(&(cls->item_q), qn);
    wtk_vecf_cpy(cls->v, item->v);
    //wtk_debug("[%.*s]\n",item->q->len,item->q->data);
    return cls;
}

void wtk_faq_cls_item_add(wtk_faq_cls_item_t *item, wtk_qa_item_t *qa)
{
    int n = item->item_q.length;
    int i;
    float *fp1, *fp2;
    double s;
    wtk_vecf_t *v;

    //v=item->v2;
    v = item->v;
    fp1 = v->p;
    fp2 = qa->v->p;
    s = 1.0 / (n + 1);
    for (i = 0; i < v->len; ++i) {
        fp1[i] = (fp1[i] * n + fp2[i]) * s;
        //fp1[i]=(fp1[i]*n+fp2[i])/(n+1);
    }
    wtk_queue_push(&(item->item_q), &(qa->cls_n));
    wtk_vecf_norm(item->v);
    //wtk_vecf_norm2(item->v,v);
}

void wtk_faq_cls_item_update_sse(wtk_faq_cls_item_t *item)
{
    wtk_queue_node_t *qn;
    wtk_qa_item_t *qa;
    double f;

    f = 0;
    for (qn = item->item_q.pop; qn; qn = qn->next) {
        qa = data_offset2(qn, wtk_qa_item_t, cls_n);
        //wtk_vecf_print(qa->v);
        //wtk_debug("len=%d\n",qa->v->len)
        f += wtk_vecf_dist(item->v, qa->v);
        //wtk_debug("f=%f\n",f);
        //exit(0);
    }
    item->n = item->item_q.length;
    item->sse = f;		///item->item_q.length;
    //exit(0);
}

void wtk_faq_cls_item_print(wtk_faq_cls_item_t *item)
{
    wtk_queue_node_t *qn;
    wtk_qa_item_t *qa;

    for (qn = item->item_q.pop; qn; qn = qn->next) {
        qa = data_offset2(qn, wtk_qa_item_t, cls_n);
        //wtk_qa_item_print(qa);
        printf("%.*s\n", qa->q->len, qa->q->data);
    }
}

void wtk_faq2bin_print_cls(wtk_faq2bin_t *faq)
{
    wtk_queue_node_t *qn;
    wtk_faq_cls_item_t *item;
    int i;

    wtk_debug("============= cls=%d ==============\n", faq->cls_q.length);
    for (i = 0, qn = faq->cls_q.pop; qn; qn = qn->next, ++i) {
        item = data_offset2(qn, wtk_faq_cls_item_t, q_n);
        wtk_debug("v[%d]=%f,%d\n", i, item->sse, item->item_q.length);
        //wtk_faq_cls_item_print(item);
    }
}

void wtk_faq2bin_add_cls(wtk_faq2bin_t *faq, wtk_faq_cls_item_t *cls)
{
    wtk_queue_node_t *qn, *prev;
    wtk_faq_cls_item_t *item;

    prev = NULL;
    for (qn = faq->cls_q.push; qn; qn = qn->prev) {
        item = data_offset2(qn, wtk_faq_cls_item_t, q_n);
        //wtk_debug("sse=%f/%f\n",item->sse,cls->sse);
        if (item->sse <= cls->sse) {
            prev = qn;
            break;
        }
    }
    //wtk_debug("========== sse=%f ==========\n",cls->sse);
    if (prev) {
        wtk_queue_insert_to(&(faq->cls_q), prev, &(cls->q_n));
    } else {
        wtk_queue_push_front(&(faq->cls_q), &(cls->q_n));
    }
//	wtk_faq2bin_print_cls(faq);
//	if(faq->cls_q.length>5)
//	{
//		exit(0);
//	}
}

void wtk_faq2bin_kmeans(wtk_faq2bin_t *faq, wtk_faq_cls_item_t *cls)
{
    wtk_faq_cls_item_t *cls1;
    wtk_faq_cls_item_t *cls2;
    wtk_queue_node_t *qn;
    wtk_qa_item_t *item;
    float f1, f2;
    wtk_queue_t q;

    if (cls->item_q.length < 2) {
//		wtk_debug("sse=%f\n",cls->sse);
//		wtk_faq2bin_print_cls(faq);
//		exit(0);
        return;
    }
    cls1 = wtk_faq2bin_cls_init(cls);
    cls1->parent = cls;
    cls2 = wtk_faq2bin_cls_init(cls);
    cls2->parent = cls;
    while (1) {
        qn = wtk_queue_pop(&(cls->item_q));
        if (!qn) {
            break;
        }
        item = data_offset2(qn, wtk_qa_item_t, cls_n);
        f1 = wtk_vecf_dist(item->v, cls1->v);
        f2 = wtk_vecf_dist(item->v, cls2->v);
        //wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
        if (f1 < f2) {
            wtk_faq_cls_item_add(cls1, item);
        } else {
            //wtk_debug("%f/%f\n",cls2->v->p[0],item->v->p[0]);
            //wtk_vecf_print(cls2->v);
            wtk_faq_cls_item_add(cls2, item);
            //wtk_vecf_print(cls2->v);
            //wtk_debug("%f/%f\n",cls2->v->p[0],item->v->p[0]);
        }
        //exit(0);
        //wtk_debug("f1=%f f2=%f\n",f1,f2);
    }
    wtk_queue_init(&(q));
    q = cls1->item_q;
    wtk_queue_init(&(cls1->item_q));
    while (1) {
        qn = wtk_queue_pop(&(q));
        if (!qn) {
            break;
        }
        item = data_offset2(qn, wtk_qa_item_t, cls_n);
        f1 = wtk_vecf_dist(item->v, cls1->v);
        f2 = wtk_vecf_dist(item->v, cls2->v);
        //wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
        if (f1 < f2) {
            wtk_queue_push(&(cls1->item_q), &(item->cls_n));
            //wtk_faq_cls_item_add(cls1,item);
        } else {
            wtk_queue_push(&(cls2->item_q), &(item->cls_n));
        }
    }
    q = cls2->item_q;
    wtk_queue_init(&(cls2->item_q));
    while (1) {
        qn = wtk_queue_pop(&(q));
        if (!qn) {
            break;
        }
        item = data_offset2(qn, wtk_qa_item_t, cls_n);
        f1 = wtk_vecf_dist(item->v, cls1->v);
        f2 = wtk_vecf_dist(item->v, cls2->v);
        //wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
        if (f1 < f2) {
            wtk_queue_push(&(cls1->item_q), &(item->cls_n));
            //wtk_faq_cls_item_add(cls1,item);
        } else {
            wtk_queue_push(&(cls2->item_q), &(item->cls_n));
        }
    }
    wtk_faq_cls_item_update_sse(cls1);
    wtk_faq2bin_add_cls(faq, cls1);
    wtk_faq_cls_item_update_sse(cls2);
    wtk_faq2bin_add_cls(faq, cls2);
    //wtk_debug("%d/%d sse=%f/%f\n",cls1->item_q.length,cls2->item_q.length,cls1->sse,cls2->sse);
    //exit(0);
    if (cls1->sse < cls2->sse) {
        cls->right = cls2;
        cls->left = cls1;
    } else {
        cls->right = cls1;
        cls->left = cls2;
    }
    //exit(0);
}

void wtk_faq2bin_kmeans2(wtk_faq2bin_t *faq, wtk_faq_cls_item_t *cls)
{
    wtk_faq_cls_item_t *cls1;
    wtk_faq_cls_item_t *cls2;
    wtk_queue_node_t *qn;
    wtk_qa_item_t *item;
    float f1, f2;
    wtk_queue_t q;

    if (cls->item_q.length < 2) {
//		wtk_debug("sse=%f\n",cls->sse);
//		wtk_faq2bin_print_cls(faq);
//		exit(0);
        return;
    }
    cls1 = wtk_faq2bin_cls_init(cls);
    cls1->parent = cls;
    cls2 = wtk_faq2bin_cls_init(cls);
    cls2->parent = cls;
    while (1) {
        qn = wtk_queue_pop(&(cls->item_q));
        if (!qn) {
            break;
        }
        item = data_offset2(qn, wtk_qa_item_t, cls_n);
        f1 = wtk_vecf_cos(item->v, cls1->v);
        f2 = wtk_vecf_cos(item->v, cls2->v);
        //wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
        if (f1 > f2) {
            wtk_faq_cls_item_add(cls1, item);
        } else {
            //wtk_debug("%f/%f\n",cls2->v->p[0],item->v->p[0]);
            //wtk_vecf_print(cls2->v);
            wtk_faq_cls_item_add(cls2, item);
            //wtk_vecf_print(cls2->v);
            //wtk_debug("%f/%f\n",cls2->v->p[0],item->v->p[0]);
        }
        //exit(0);
        //wtk_debug("f1=%f f2=%f\n",f1,f2);
    }
    wtk_queue_init(&(q));
    q = cls1->item_q;
    wtk_queue_init(&(cls1->item_q));
    while (1) {
        qn = wtk_queue_pop(&(q));
        if (!qn) {
            break;
        }
        item = data_offset2(qn, wtk_qa_item_t, cls_n);
        f1 = wtk_vecf_cos(item->v, cls1->v);
        f2 = wtk_vecf_cos(item->v, cls2->v);
        //wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
        if (f1 > f2) {
            wtk_queue_push(&(cls1->item_q), &(item->cls_n));
            //wtk_faq_cls_item_add(cls1,item);
        } else {
            wtk_queue_push(&(cls2->item_q), &(item->cls_n));
        }
    }
    q = cls2->item_q;
    wtk_queue_init(&(cls2->item_q));
    while (1) {
        qn = wtk_queue_pop(&(q));
        if (!qn) {
            break;
        }
        item = data_offset2(qn, wtk_qa_item_t, cls_n);
        f1 = wtk_vecf_cos(item->v, cls1->v);
        f2 = wtk_vecf_cos(item->v, cls2->v);
        //wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
        if (f1 > f2) {
            wtk_queue_push(&(cls1->item_q), &(item->cls_n));
            //wtk_faq_cls_item_add(cls1,item);
        } else {
            wtk_queue_push(&(cls2->item_q), &(item->cls_n));
        }
    }
    wtk_faq_cls_item_update_sse(cls1);
    wtk_faq2bin_add_cls(faq, cls1);
    wtk_faq_cls_item_update_sse(cls2);
    wtk_faq2bin_add_cls(faq, cls2);
    //wtk_debug("%d/%d sse=%f/%f\n",cls1->item_q.length,cls2->item_q.length,cls1->sse,cls2->sse);
    //exit(0);
    if (cls1->sse < cls2->sse) {
        cls->right = cls2;
        cls->left = cls1;
    } else {
        cls->right = cls1;
        cls->left = cls2;
    }
    //exit(0);
}

typedef struct {
    wtk_vecf_t *v;
    wtk_qa_item_t *item;
    float f;
} wtk_faq_result_t;

void wtk_faq_result_print(wtk_faq_result_t *r)
{
    wtk_debug("========= float=%f ===========\n", r->f);
    wtk_qa_item_print(r->item);
}

#define wtk_faq_vec_dist2 wtk_vecf_dist

void wtk_faq_cls_item_check(wtk_faq_cls_item_t *cls, wtk_faq_result_t *r)
{
    wtk_queue_node_t *qn;
    wtk_qa_item_t *qa;
    float f;

    for (qn = cls->item_q.pop; qn; qn = qn->next) {
        qa = data_offset2(qn, wtk_qa_item_t, cls_n);
        f = wtk_vecf_cos(qa->v, r->v);
        //wtk_debug("[%.*s]=%f\n",qa->q->len,qa->q->data,f);
        if (f > r->f) {
            //wtk_debug("set cls=%d\n",cls->index);
            r->f = f;
            r->item = qa;
            if (f >= 1.0) {
                //wtk_debug("found\n");
                return;
            }
        }
    }
//	wtk_debug("dist=%f\n",min_f);
//	wtk_qa_item_print(min_item);
}

void wtk_faq_cls_item_check_all(wtk_faq_cls_item_t *cls, wtk_faq_result_t *r)
{
    if (cls->item_q.length > 0) {
        wtk_faq_cls_item_check(cls, r);
        if (r->f >= 1.0) {
            return;
        }
    }

    if (cls->left) {
        wtk_faq_cls_item_check_all(cls->left, r);
        if (r->f >= 1.0) {
            return;
        }
    }
    if (cls->left) {
        wtk_faq_cls_item_check_all(cls->right, r);
        if (r->f >= 1.0) {
            return;
        }
    }
}

void wtk_faq_cls_item_test(wtk_faq_cls_item_t *cls, wtk_faq_result_t *v)
{
    float f1, f2;	//,f3;

    //wtk_debug("v=%p len=%d\n",cls->v,cls->item_q.length);
    //wtk_debug("check cls=%d\n",cls->index);
    if (cls->item_q.length > 0) {
        //wtk_debug("found %p/%p\n",cls->right,cls->left);
        wtk_faq_cls_item_check(cls, v);
        //exit(0);
        return;
    } else {
        if (cls->right && cls->left) {
            if (0) {
                f1 = wtk_vecf_cos(cls->right->v, v->v);
                f2 = wtk_vecf_cos(cls->left->v, v->v);
                if (f1 > f2) {
                    wtk_faq_cls_item_test(cls->right, v);
                } else {
                    wtk_faq_cls_item_test(cls->left, v);
                }
            } else {
                f1 = wtk_vecf_dist(cls->right->v, v->v);
                f2 = wtk_vecf_dist(cls->left->v, v->v);
                if (f1 < f2) {
                    wtk_faq_cls_item_test(cls->right, v);
                } else {
                    wtk_faq_cls_item_test(cls->left, v);
                }
            }
        } else {
            if (cls->right) {
                wtk_faq_cls_item_test(cls->right, v);
            } else if (cls->left) {
                wtk_faq_cls_item_test(cls->left, v);
            }
        }
    }
}

wtk_faq_cls_item_t* wtk_faq2bin_get_best_cls(wtk_faq2bin_t *faq, wtk_vecf_t *v)
{
    wtk_queue_node_t *qn;
    wtk_faq_cls_item_t *cls;
    float min_f = 1e10, f;
    wtk_faq_cls_item_t *min = NULL;

    for (qn = faq->cls_q.pop; qn; qn = qn->next) {
        cls = data_offset2(qn, wtk_faq_cls_item_t, q_n);
        f = wtk_vecf_dist(cls->v, v);
        if (f < min_f) {
            if (f == 0) {
                return cls;
            }
            min_f = f;
            min = cls;
        }
    }
    return min;
}

void wtk_faq_cls_item_test2(wtk_faq2bin_t *faq)
{
}

void wtk_faq2bin_test(wtk_faq2bin_t *faq)
{
    wtk_vecf_t *v;
    char *s = "我今天心情很好";
    double t;
    wtk_faq_result_t r;

    s = "我今天心情很好";
    //s="我要喝水";
    //s="我想听音乐";
    //s="李白";
    //s="喜欢你";
    s = "我有心事";
    s = "我有病";
    s = "你是谁啊";

    s = "动画片很好看";
    s = "郭德纲是谁啊";
    s = "求求你了";
    s = "你这个傻蛋";
    s = "有情况没有?";
    s = "有没有问题";
    s = "吴 奇 是 谁";
    s = "播放我的歌声里";
    s = "我要拉屎";
    s = "有什么好看的电影";
    s = "李白";
    s = "是非成败转头空";
    s = "歌词 难 念 的 经";
    s = "主人";
    s = "习主席";
    s = "前进中国";
    s = "有没有音乐";
    s = "我要去苏州";
    s = "李白";
    wtk_debug("%s\n", s);
    v = faq->wrdvec->v1;
    t = time_get_ms();
    wtk_wrdvec_snt_to_vec(faq->wrdvec, s, strlen(s), v);
    r.f = -1e10;
    r.v = v;
    r.item = NULL;
    faq->root->v = NULL;
    wtk_faq_cls_item_test(faq->root, &r);
    t = time_get_ms() - t;
    wtk_debug("time=%f\n", t);
    wtk_faq_result_print(&r);
    //exit(0);

    {
        wtk_faq_cls_item_t *cls;

        r.f = -1e10;
        r.v = v;
        r.item = NULL;
        t = time_get_ms();
        cls = wtk_faq2bin_get_best_cls(faq, v);
        wtk_faq_cls_item_check(cls, &r);
        t = time_get_ms() - t;
        wtk_debug("time=%f\n", t);
        wtk_faq_result_print(&r);
    }

    t = time_get_ms();
    r.f = -1e10;
    r.v = v;
    r.item = NULL;
    wtk_faq_cls_item_check_all(faq->root, &r);
    t = time_get_ms() - t;
    wtk_debug("time=%f\n", t);
    wtk_faq_result_print(&r);

    wtk_debug("cls=%d\n", faq->cls_q.length);
    exit(0);
}

int wtk_faq2bin_add_btree(wtk_faq2bin_t *faq)
{
    wtk_queue_node_t *qn;
    wtk_faq_cls_item_t *cls;
    wtk_faq_cls_item_t *parent;
    int n;

    parent = wtk_faq2bin_init_cls(faq);
    faq->root = parent;
    if (faq->cfg->use_cls) {
        n = faq->cfg->ncls;
    } else {
        n = faq->item_q.length / faq->cfg->ncls;
    }
    wtk_debug("ncls=%d\n", n);
    //n=20;
    while (faq->cls_q.length < n) {
        //wtk_debug("len=%d\n",faq->cls_q.length);
        qn = wtk_queue_pop_back(&(faq->cls_q));
        cls = data_offset2(qn, wtk_faq_cls_item_t, q_n);
        wtk_faq2bin_kmeans(faq, cls);
        //wtk_faq2bin_print_cls(faq);
    }
    //wtk_faq2bin_print_cls(faq);
    ///wtk_faq2bin_test(faq);
    //wtk_debug("parent=%p\n",parent);
    return 0;
}

int wtk_faq2bin_process(wtk_faq2bin_t *faq, char *fn)
{
    int ret;

    ret = wtk_source_load_file(faq,
            (wtk_source_load_handler_t) wtk_faq2bin_read, fn);
    if (ret != 0) {
        goto end;
    }
    ret = wtk_faq2bin_add_btree(faq);
    if (ret != 0) {
        goto end;
    }
    ret = 0;
    end: return ret;
}

void wtk_faq2bin_write_item(wtk_faq2bin_t *faq, wtk_faq_cls_item_t *item,
        wtk_strbuf_t *hdr, wtk_strbuf_t *buf)
{
    char c;

    wtk_debug("item=%p:%d  left=%p right=%p\n", item, item->item_q.length,
            item->left, item->right);
    c = 0;
    if (item->item_q.length > 0) {
        c += 1;
    }
    if (item->v) {
        c += 2;
    }
    if (item->left) {
        c += 4;
    }
    if (item->right) {
        c += 8;
    }
    wtk_strbuf_push_c(hdr, c);
    if (item->item_q.length > 0) {
        wtk_queue_node_t *qn;
        wtk_qa_item_t *qa;
        int i;
        wtk_string_t **strs;
        int nx;
        short si;

        i = buf->pos;
        wtk_strbuf_push(hdr, (char*) &(i), 4);
        i = item->item_q.length;
        wtk_strbuf_push(buf, (char*) &i, 4);
        for (qn = item->item_q.pop; qn; qn = qn->next) {
            qa = data_offset2(qn, wtk_qa_item_t, cls_n);
            wtk_strbuf_push(buf, (char*) qa->v->p, qa->v->len * sizeof(float));
            strs = (wtk_string_t**) qa->a->slot;
            nx = 0;
            for (i = 0; i < qa->a->nslot; ++i) {
                nx += strs[i]->len + 2;
            }
            wtk_strbuf_push(buf, (char*) &nx, 4);
            for (i = 0; i < qa->a->nslot; ++i) {
                si = strs[i]->len;
                wtk_strbuf_push(buf, (char*) &si, 2);
                wtk_strbuf_push(buf, strs[i]->data, strs[i]->len);
            }
        }
    }
    if (item->v) {
        wtk_strbuf_push(hdr, (char*) item->v->p, item->v->len * sizeof(float));
    }
    if (item->left) {
        wtk_faq2bin_write_item(faq, item->left, hdr, buf);
    }
    if (item->right) {
        wtk_faq2bin_write_item(faq, item->right, hdr, buf);
    }
}

void wtk_faq2bin_write(wtk_faq2bin_t *faq, char *fn)
{
    wtk_strbuf_t *hdr;
    wtk_strbuf_t *buf;
    FILE *f;
    int v;

    wtk_debug("fn=%s\n", fn);
    hdr = wtk_strbuf_new(4096, 1);
    buf = wtk_strbuf_new(4096, 1);

    v = faq->wrdvec->cfg->vec_size;
    wtk_strbuf_push(hdr, (char*) &(v), 4);

    wtk_faq2bin_write_item(faq, faq->root, hdr, buf);

    wtk_debug("hdr=%d\n", hdr->pos);
    wtk_debug("bdy=%d\n", buf->pos);
    f = fopen(fn, "wb");
    fwrite(hdr->data, hdr->pos, 1, f);
    fwrite(buf->data, buf->pos, 1, f);
    fclose(f);

    wtk_strbuf_delete(hdr);
    wtk_strbuf_delete(buf);
    //exit(0);
}

int wtk_faq2bin_write_item2(wtk_faq2bin_t *faq, wtk_faq_cls_item_t *item,
        FILE *hdr, FILE *buf, int content_size, wtk_strbuf_t *xbuf)
{
    char c;

    //wtk_debug("content_size=%d len=%d\n",content_size,item->item_q.length);
    c = 0;
    if (item->item_q.length > 0) {
        c += 1;
    }
    if (item->v) {
        c += 2;
    }
    if (item->left) {
        c += 4;
    }
    if (item->right) {
        c += 8;
    }
    fwrite(&c, 1, 1, hdr);
    //wtk_strbuf_push_c(hdr,c);
    if (item->item_q.length > 0) {
        wtk_queue_node_t *qn;
        wtk_qa_item_t *qa;
        int i;
        wtk_string_t **strs;
        int nx;
        short si;

        wtk_strbuf_reset(xbuf);
        i = content_size;
        //wtk_strbuf_push(hdr,(char*)&(i),4);
        fwrite(&i, 4, 1, hdr);
        i = item->item_q.length;
        wtk_strbuf_push(xbuf, (char*) &i, 4);
        //content_size+=4;
        //fwrite(&i,4,1,buf);
        for (qn = item->item_q.pop; qn; qn = qn->next) {
            qa = data_offset2(qn, wtk_qa_item_t, cls_n);
            wtk_strbuf_push(xbuf, (char*) qa->v->p, qa->v->len * sizeof(float));
            //content_size+=qa->v->len*sizeof(float);
            //fwrite(qa->v->p,sizeof(float),qa->v->len,buf);
            strs = (wtk_string_t**) qa->a->slot;
            nx = 0;
            for (i = 0; i < qa->a->nslot; ++i) {
                nx += strs[i]->len + 2;
            }
            wtk_strbuf_push(xbuf, (char*) &nx, 4);
            //content_size+=4;
            //fwrite(&nx,4,1,buf);
            for (i = 0; i < qa->a->nslot; ++i) {
                si = strs[i]->len;
                wtk_strbuf_push(xbuf, (char*) &si, 2);
                //content_size+=2;
                //fwrite(&si,2,1,buf);
                wtk_strbuf_push(xbuf, strs[i]->data, strs[i]->len);
                //content_size+=strs[i]->len;
                //fwrite(strs[i]->data,strs[i]->len,1,buf);
            }
        }
        content_size += xbuf->pos;
        fwrite(xbuf->data, xbuf->pos, 1, buf);
    }
    //wtk_debug("content_size=%d len=%d\n",content_size,item->item_q.length);
    if (item->v) {
        //wtk_strbuf_push(hdr,(char*)item->v->p,item->v->len*sizeof(float));
        fwrite(item->v->p, item->v->len * sizeof(float), 1, hdr);
    }
    if (item->left) {
        content_size = wtk_faq2bin_write_item2(faq, item->left, hdr, buf,
                content_size, xbuf);
    }
    if (item->right) {
        content_size = wtk_faq2bin_write_item2(faq, item->right, hdr, buf,
                content_size, xbuf);
    }
    return content_size;
}

void wtk_faq2bin_write2(wtk_faq2bin_t *faq, char *fn)
{
    FILE *hdr;
    FILE *buf;
    int v;
    char tmp[1024];
    wtk_strbuf_t *xbuf;

    xbuf = wtk_strbuf_new(4096, 1);
    sprintf(tmp, "%s.hdr", fn);
    printf("write hdr: %s\n", tmp);
    hdr = fopen(tmp, "wb");
    v = faq->wrdvec->cfg->vec_size;
    fwrite(&v, 4, 1, hdr);

    sprintf(tmp, "%s.bdy", fn);
    printf("write bdy: %s\n", tmp);
    buf = fopen(tmp, "wb");
    //wtk_strbuf_push(hdr,(char*)&(v),4);
    wtk_faq2bin_write_item2(faq, faq->root, hdr, buf, 0, xbuf);

    fclose(hdr);
    fclose(buf);
    sprintf(tmp, "cat %s.hdr %s.bdy > %s", fn, fn, fn);
    printf("merge: %s\n", tmp);
    v = system(tmp);
    wtk_debug("v=%d\n", v);
    wtk_strbuf_delete(xbuf);
    //exit(0);
}
