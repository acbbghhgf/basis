#include "wtk_wrdvec.h" 

wtk_wrdvec_t* wtk_wrdvec_new(wtk_wrdvec_cfg_t *cfg, wtk_rbin2_t *rbin)
{
    wtk_wrdvec_t *v;

    v = (wtk_wrdvec_t*) wtk_malloc(sizeof(wtk_wrdvec_t));
    v->cfg = cfg;
    v->seg = wtk_segmenter_new(&(cfg->seg), rbin);
    v->v1 = wtk_vecf_new(cfg->vec_size);
    v->v2 = wtk_vecf_new(cfg->vec_size);
    v->vx = wtk_vecf_new(cfg->vec_size);
    v->buf = wtk_strbuf_new(256, 1);
    v->f_of = 0;
    v->f_len = 0;
    v->rbin = rbin;
    //wtk_debug("rbin=%p\n",rbin);
    if (cfg->use_bin) {
        if (rbin) {
            wtk_rbin2_item_t *item;

            item = wtk_rbin2_get(rbin, cfg->fn, strlen(cfg->fn));
            //wtk_debug("%s:%p\n",cfg->fn,item);
            v->bin = rbin->f;
            v->f_of = item->pos;
            v->f_len = item->len;
        } else {
            v->bin = fopen(cfg->fn, "rb");
        }
    } else {
        v->bin = NULL;
    }
    return v;
}

void wtk_wrdvec_delete(wtk_wrdvec_t *v)
{
    //wtk_debug("delete wrdvec =%p\n",v);
    if (v->rbin) {

    } else {
        if (v->bin) {
            fclose(v->bin);
        }
    }
    wtk_strbuf_delete(v->buf);
    wtk_vecf_delete(v->v1);
    wtk_vecf_delete(v->v2);
    wtk_vecf_delete(v->vx);
    wtk_segmenter_delete(v->seg);
    wtk_free(v);
}

int wtk_wrdvec_load_item(wtk_wrdvec_t *v, wtk_wrdvec_item_t *item,
        wtk_vecf_t *v2)
{
    int ret;
    int offset;
    int dx;
    FILE *f = v->bin;

    dx = sizeof(float) * v->cfg->vec_size;
    //wtk_debug("wrd_idx=%d/%d\n",v->cfg->offset,item->wrd_idx);
    offset = v->f_of + v->cfg->offset + item->wrd_idx * dx;
    ret = fseek(f, offset, SEEK_SET);
    if (ret != 0) {
        goto end;
    }
    ret = fread(v2->p, dx, 1, f);
    if (ret != 1) {
        ret = -1;
        goto end;
    }
    ret = 0;
    end: return ret;
}

void wtk_wrdvec_snt_to_vec(wtk_wrdvec_t *v, char *s1, int s1_bytes,
        wtk_vecf_t *v1)
{
    wtk_segmenter_t *seg = v->seg;
    wtk_wrdvec_item_t *item;
    wtk_string_t **strs;
    wtk_vecf_t *v2 = v->vx;
    int i;
    int ret;

    wtk_segmenter_parse(seg, s1, s1_bytes, v->buf);
    wtk_vecf_zero(v1);
    strs = seg->wrd_array;
    //wtk_debug("wrds=%d\n",seg->wrd_array_n);
    for (i = 0; i < seg->wrd_array_n; ++i) {
        //wtk_debug("v[%d]=[%.*s]\n",i,strs[i]->len,strs[i]->data);
        item = wtk_wrdvec_cfg_find(v->cfg, strs[i]->data, strs[i]->len);
        if (item) {
            if (v->cfg->use_bin) {
                ret = wtk_wrdvec_load_item(v, item, v2);
                //wtk_debug("v[%d]=[%.*s] ret=%d\n",i,strs[i]->len,strs[i]->data,ret);
                if (ret == 0) {
                    //wtk_vecf_print(v2);
                    //exit(0);
                    wtk_vecf_add(v1, v2->p);
                }
            } else {
                wtk_vecf_add(v1, item->m->p);
            }
        }
    }
    wtk_vecf_norm(v1);
    //wtk_debug("[%.*s]\n",v->buf->pos,v->buf->data);
    wtk_segmenter_reset(seg);
}

void wtk_wrdvec_snt_to_vec3(wtk_wrdvec_t *v, wtk_string_t **strs, int nstrs,
        wtk_vecf_t *v1)
{
    wtk_wrdvec_item_t *item;
    wtk_vecf_t *v2 = v->vx;
    int i;
    int ret;

    wtk_vecf_zero(v1);
    for (i = 0; i < nstrs; ++i) {
        //wtk_debug("v[%d]=[%.*s]\n",i,strs[i]->len,strs[i]->data);
        item = wtk_wrdvec_cfg_find(v->cfg, strs[i]->data, strs[i]->len);
        if (item) {
            if (v->cfg->use_bin) {
                ret = wtk_wrdvec_load_item(v, item, v2);
                //wtk_debug("v[%d]=[%.*s] ret=%d\n",i,strs[i]->len,strs[i]->data,ret);
                if (ret == 0) {
                    //wtk_vecf_print(v2);
                    //exit(0);
                    wtk_vecf_add(v1, v2->p);
                }
            } else {
                wtk_vecf_add(v1, item->m->p);
            }
        }
    }
    wtk_vecf_norm(v1);
}

#include <ctype.h>

float wtk_wrdvec_snt_to_vec2(wtk_wrdvec_t *v, char *s, int s_bytes,
        wtk_vecf_t *vec)
{
    int init;
    wtk_string_t str;
    char *e;
    int n;
    wtk_wrdvec_item_t *item;

    wtk_vecf_zero(vec);
    e = s + s_bytes;
    init = 0;
    wtk_string_set(&(str), 0, 0);
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        if (init == 0) {
            if (n > 1 || !isspace(*s)) {
                str.data = s;
                str.len = 0;
                init = 1;
            }
        } else {
            if (n == 1 && isspace(*s)) {
                str.len = s - str.data;
                //wtk_debug("[%.*s]\n",str.len,str.data);
                item = wtk_wrdvec_cfg_find(v->cfg, str.data, str.len);
                if (item) {
                    //wtk_vecf_print(item->m);
                    wtk_vecf_add(vec, item->m->p);
                }
                str.data = NULL;
                init = 0;
            }
        }
        s += n;
    }
    if (init == 1 && str.data > 0) {
        str.len = e - str.data;
        //wtk_debug("[%.*s]\n",str.len,str.data);
        item = wtk_wrdvec_cfg_find(v->cfg, str.data, str.len);
        if (item) {
            wtk_vecf_add(vec, item->m->p);
        }
    }
    return wtk_vecf_norm(vec);
    //return 1.00;
}

float wtk_wrdvec_like(wtk_wrdvec_t *v, char *s1, int s1_bytes, char *s2,
        int s2_bytes)
{
    wtk_vecf_t *v1 = v->v1;
    wtk_vecf_t *v2 = v->v2;
    float f;

    wtk_wrdvec_snt_to_vec(v, s1, s1_bytes, v1);
    wtk_wrdvec_snt_to_vec(v, s2, s2_bytes, v2);
    f = wtk_vecf_cos(v1, v2);
    //wtk_debug("[%.*s]=[%.*s] %f\n",s1_bytes,s1,s2_bytes,s2,f);
    return f;
}

void wtk_wrdvec_like2(wtk_wrdvec_t *v, char *s1, int s1_bytes)
{
    wtk_vecf_t *v1 = v->v1;

    wtk_wrdvec_snt_to_vec(v, s1, s1_bytes, v1);
    wtk_wrdvec_cfg_test3(v->cfg, v1);
}

wtk_robin_t* wtk_wrdvec_find_best_like_word(wtk_wrdvec_t *v, char *s1,
        int s1_bytes, int len, float thresh)
{
    wtk_wrdvec_cfg_t *cfg = v->cfg;
    wtk_vecf_t *v1 = v->v1;
    wtk_vecf_t *v2;
    wtk_wrdvec_item_t *item2;
    wtk_queue_node_t *qn;
    hash_str_node_t *node;
    int i, j;
    float f;
    wtk_wrdvec_cmp_t *items;
    int n;
    wtk_robin_t *robin;

    wtk_wrdvec_snt_to_vec(v, s1, s1_bytes, v1);
    n = 0;
    items = (wtk_wrdvec_cmp_t*) wtk_calloc(len, sizeof(wtk_wrdvec_cmp_t));
    for (i = 0; i < cfg->hash->nslot; ++i) {
        //wtk_debug("================ i=%d ============\n",i);
        if (!cfg->hash->slot[i]) {
            continue;
        }
        for (qn = cfg->hash->slot[i]->pop; qn; qn = qn->next) {
            node = data_offset(qn, hash_str_node_t, n);
            item2 = (wtk_wrdvec_item_t*) node->value;
            //wtk_debug("v[%d]:%p[%.*s] qn=%p next=%p\n",i,item2,item2->name->len,item2->name->data,qn,qn->next);
            //if(item1!=item2)
            {
                if (v->cfg->use_bin) {
                    v2 = v->v2;
                    wtk_wrdvec_load_item(v, item2, v2);
                } else {
                    v2 = item2->m;
                }
                f = wtk_vecf_cos2(v1, v2);
                if (f < thresh) {
                    continue;
                }
                //wtk_debug("f=%f [%.*s]=[%.*s]\n",f,s1_bytes,s1,node->key.len,node->key.data);
//				{
//					static int ki=0;
//
//					++ki;
//					exit(0);
//				}
                if (f >= 0.95) {
                    if (wtk_string_cmp(item2->name, s1, s1_bytes) == 0) {
                        continue;
                    }
                }
//				if(f>=0.95)
//				{
//					wtk_debug("%f [%.*s] %d\n",f,item2->name->len,item2->name->data,f>=1.0);
//				}
                //exit(0);
                if (n < len) {
                    if (n == 0 || items[n - 1].f >= f) {
                        items[n].item = item2;
                        items[n].f = f;
                    } else {
                        for (j = n - 1; j >= 0; --j) {
                            if (items[j].f < f) {
                                items[j + 1] = items[j];
                                if (j == 0) {
                                    items[j].item = item2;
                                    items[j].f = f;
                                }
                            } else {
                                items[j + 1].item = item2;
                                items[j + 1].f = f;
                                break;
                            }
                        }
                    }
                    ++n;
                } else if (items[len - 1].f < f) {
                    for (j = len - 2; j >= 0; --j) {
                        if (items[j].f < f) {
                            items[j + 1] = items[j];
                            if (j == 0) {
                                items[j].item = item2;
                                items[j].f = f;
                            }
                        } else {
                            items[j + 1].item = item2;
                            items[j + 1].f = f;
                            break;
                        }
                    }
                }
            }
        }
    }
    robin = wtk_robin_new(n);
    for (i = 0; i < n; ++i) {
        wtk_debug("v[%d]=[%.*s] %f\n", i, items[i].item->name->len,
                items[i].item->name->data, items[i].f);
        wtk_robin_push(robin, items[i].item->name);
    }
    wtk_free(items);
    return robin;
}

wtk_string_t wtk_wrdvec_best_like(wtk_wrdvec_t *v, char *dst, int dst_bytes,
        char *src, int src_bytes, float *pf)
{
    wtk_segmenter_t *seg = v->seg;
    wtk_wrdvec_item_t *item;
    wtk_vecf_t *v1 = v->v1;
    wtk_vecf_t *v2 = v->v2;
    wtk_string_t **strs;
    float f;
    int i, j, k;
    int ret;
    int min_i = -1, max_i = -1;				//[min_i,max_i]
    float max_f = -1;
    wtk_strbuf_t *buf = v->buf;
    wtk_string_t tx;

    wtk_wrdvec_snt_to_vec(v, dst, dst_bytes, v1);
    wtk_segmenter_parse(seg, src, src_bytes, v->buf);
    strs = seg->wrd_array;
    for (i = 0; i < seg->wrd_array_n; ++i) {
        for (j = i; j < seg->wrd_array_n; ++j) {
            wtk_vecf_zero(v2);
            //wtk_debug("j=[%d-%d]\n",i,j);
            //wtk_debug("v[%d/%d]=[%.*s]\n",i,j,strs[j]->len,strs[j]->data);
            for (k = i; k <= j; ++k) {
                //wtk_debug("v[%d/%d]=[%.*s]\n",i,k,strs[k]->len,strs[k]->data);
                item = wtk_wrdvec_cfg_find(v->cfg, strs[k]->data, strs[k]->len);
                if (item) {
                    if (v->cfg->use_bin) {
                        ret = wtk_wrdvec_load_item(v, item, v->vx);
                        //wtk_debug("v[%d]=[%.*s] ret=%d\n",i,strs[i]->len,strs[i]->data,ret);
                        if (ret == 0) {
                            //wtk_vecf_print(v2);
                            //exit(0);
                            wtk_vecf_add(v2, v->vx->p);
                        }
                    } else {
                        wtk_vecf_add(v2, item->m->p);
                    }
                }
            }
            wtk_vecf_norm(v2);
            f = wtk_vecf_cos(v1, v2);
            //wtk_debug("f=%f\n",f);
            if (f > max_f) {
                max_f = f;
                min_i = i;
                max_i = j;
            }
        }
    }
    //wtk_debug("[%d-%d]=%f\n",min_i,max_i,max_f);
    wtk_strbuf_reset(buf);
    if (min_i >= 0) {
        for (i = min_i; i <= max_i; ++i) {
            wtk_strbuf_push(buf, strs[i]->data, strs[i]->len);
        }
    }
    wtk_string_set(&(tx), buf->data, buf->pos);
    wtk_segmenter_reset(seg);
    //wtk_debug("[%.*s]=%f\n",tx.len,tx.data,max_f);
    if (pf) {
        *pf = max_f;
    }
    return tx;
}

void wtk_wrdvec_test(wtk_wrdvec_t *v, char *s1, int s1_bytes, char *s2,
        int s2_bytes)
{
    wtk_wrdvec_cfg_test(v->cfg, s1, s1_bytes, s2, s2_bytes);
}

