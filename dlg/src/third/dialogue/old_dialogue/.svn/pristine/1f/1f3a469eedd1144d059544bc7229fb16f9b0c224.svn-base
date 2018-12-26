#include "wtk_wrdvec_cfg.h"
#include "wtk/core/cfg/wtk_source.h"
#include <math.h>

int wtk_wrdvec_cfg_init(wtk_wrdvec_cfg_t *cfg)
{
    wtk_segmenter_cfg_init(&(cfg->seg));
    cfg->voc_size = 0;
    cfg->vec_size = 0;
    cfg->fn = NULL;
    cfg->hash = NULL;
    cfg->wrds = NULL;
    cfg->use_bin = 0;
    cfg->offset = 0;
    return 0;
}

int wtk_wrdvec_cfg_clean(wtk_wrdvec_cfg_t *cfg)
{
    if (cfg->wrds) {
        wtk_free(cfg->wrds);
    }
    wtk_segmenter_cfg_clean(&(cfg->seg));
    if (cfg->hash) {
        wtk_str_hash_delete(cfg->hash);
    }
    return 0;
}

int wtk_wrdvec_cfg_update_local(wtk_wrdvec_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_string_t *v;
    wtk_local_cfg_t *lc;

    lc = main;
    wtk_local_cfg_update_cfg_str(lc, cfg, fn, v);
    wtk_local_cfg_update_cfg_b(lc, cfg, use_bin, v);
    lc = wtk_local_cfg_find_lc_s(main, "seg");
    if (lc) {
        wtk_segmenter_cfg_update_local(&(cfg->seg), lc);
    }
    return 0;
}

int wtk_wrdvec_cfg_load(wtk_wrdvec_cfg_t *cfg, wtk_source_t *src)
{
    int v[2];
    wtk_strbuf_t *buf;
    int ret;
    wtk_wrdvec_item_t *item;
    int idx = 0;

    buf = wtk_strbuf_new(256, 1);
    ret = wtk_source_read_int(src, v, 2, 0);
    cfg->voc_size = v[0];
    cfg->vec_size = v[1];
    cfg->wrds = (wtk_wrdvec_item_t**) wtk_calloc(cfg->voc_size,
            sizeof(wtk_wrdvec_item_t*));
    cfg->hash = wtk_str_hash_new(cfg->voc_size + 3);
    while (1) {
        ret = wtk_source_read_string(src, buf);
        if (ret != 0) {
            ret = 0;
            goto end;
        }
        //wtk_debug("[%.*s]\n",buf->pos,buf->data);
        item = (wtk_wrdvec_item_t*) wtk_heap_malloc(cfg->hash->heap,
                sizeof(wtk_wrdvec_item_t));
        item->wrd_idx = idx;
        cfg->wrds[idx++] = item;
        item->name = wtk_heap_dup_string(cfg->hash->heap, buf->data, buf->pos);
        item->m = wtk_vecf_new(cfg->vec_size);
        ret = wtk_source_read_float(src, item->m->p, cfg->vec_size, 0);
        wtk_str_hash_add(cfg->hash, item->name->data, item->name->len, item);
    }
    end: wtk_strbuf_delete(buf);
    return ret;
}

int wtk_wrdvec_cfg_load_bin_info(wtk_wrdvec_cfg_t *cfg, wtk_source_t *src)
{
    int v[2];
    wtk_strbuf_t *buf;
    int ret;
    wtk_wrdvec_item_t *item;
    int idx = 0;

    cfg->offset = 0;
    src->swap = 0;
    buf = wtk_strbuf_new(256, 1);
    ret = wtk_source_read_int(src, v, 2, 1);
    if (ret != 0) {
        goto end;
    }
    cfg->offset += 8;
    cfg->voc_size = v[0];
    cfg->vec_size = v[1];
    //wtk_debug("%d/%d\n",cfg->voc_size,cfg->vec_size);
    cfg->wrds = (wtk_wrdvec_item_t**) wtk_calloc(cfg->voc_size,
            sizeof(wtk_wrdvec_item_t*));
    cfg->hash = wtk_str_hash_new(cfg->voc_size + 3);
    while (idx < cfg->voc_size) {
        ret = wtk_source_read_wtkstr(src, buf);
        if (ret != 0 || buf->pos == 0) {
            ret = 0;
            goto end;
        }
        cfg->offset += buf->pos + 1;
        //wtk_debug("[%.*s]\n",buf->pos,buf->data);
        item = (wtk_wrdvec_item_t*) wtk_heap_malloc(cfg->hash->heap,
                sizeof(wtk_wrdvec_item_t));
        item->wrd_idx = idx;
        cfg->wrds[idx++] = item;
        item->name = wtk_heap_dup_string(cfg->hash->heap, buf->data, buf->pos);
        item->m = NULL;
        wtk_str_hash_add(cfg->hash, item->name->data, item->name->len, item);
    }
    end:
    //wtk_debug("offset=%d\n",cfg->offset);
    //exit(0);
    //wtk_debug("v=%d/%d\n",cfg->voc_size,idx);
    wtk_strbuf_delete(buf);
    return ret;
}

int wtk_wrdvec_cfg_update(wtk_wrdvec_cfg_t *cfg)
{
    wtk_segmenter_cfg_update(&(cfg->seg));
    if (cfg->use_bin) {
        wtk_source_load_file(cfg,
                (wtk_source_load_handler_t) wtk_wrdvec_cfg_load_bin_info,
                cfg->fn);
    } else {
        wtk_source_load_file(cfg,
                (wtk_source_load_handler_t) wtk_wrdvec_cfg_load, cfg->fn);
    }
    return 0;
}

int wtk_wrdvec_cfg_update2(wtk_wrdvec_cfg_t *cfg, wtk_source_loader_t *sl)
{
    int ret;

    wtk_segmenter_cfg_update2(&(cfg->seg), sl);
    //wtk_debug("load  %s...\n",cfg->fn);
    if (cfg->use_bin) {
        ret = wtk_source_loader_load(sl, cfg,
                (wtk_source_load_handler_t) wtk_wrdvec_cfg_load_bin_info,
                cfg->fn);
    } else {
        ret = wtk_source_loader_load(sl, cfg,
                (wtk_source_load_handler_t) wtk_wrdvec_cfg_load, cfg->fn);
    }
    return ret;
}

wtk_wrdvec_item_t* wtk_wrdvec_cfg_find(wtk_wrdvec_cfg_t *cfg, char *data,
        int bytes)
{
    return (wtk_wrdvec_item_t*) wtk_str_hash_find(cfg->hash, data, bytes);
}

void wtk_wrdvec_cfg_test(wtk_wrdvec_cfg_t *cfg, char *s1, int s1_bytes,
        char *s2, int s2_bytes)
{
    wtk_wrdvec_item_t *item1, *item2;
    int col;
    int i;
    float f, t, t1;
    float *fp1, *fp2;

    item1 = wtk_wrdvec_cfg_find(cfg, s1, s1_bytes);
    item2 = wtk_wrdvec_cfg_find(cfg, s2, s2_bytes);
    col = item1->m->len;
    f = 0;
    fp1 = item1->m->p;
    fp2 = item2->m->p;
//	for(i=0;i<col;++i)
//	{
//		t=fp1[i]-fp2[i];
//		f+=t*t;
//	}
//	f=sqrt(f);

    f = 0;
    t = 0;
    t1 = 0;
    for (i = 0; i < col; ++i) {
        f += fp1[i] * fp2[i];
        t += fp1[i] * fp1[i];
        t1 += fp2[i] * fp2[i];
    }
    f = f / (sqrt(t) * sqrt(t1));
    wtk_debug("[%.*s]/[%.*s]=%f\n", s1_bytes, s1, s2_bytes, s2, f);
}

void wtk_wrdvec_cfg_test3(wtk_wrdvec_cfg_t *cfg, wtk_vecf_t *v1)
{
    wtk_wrdvec_item_t *item2;
    wtk_queue_node_t *qn;
    hash_str_node_t *node;
    int i, j;
    float f;
    wtk_wrdvec_cmp_t *items;
    int n;
    int len;

    len = 100;
    n = 0;
    items = (wtk_wrdvec_cmp_t*) wtk_calloc(len, sizeof(wtk_wrdvec_cmp_t));
    for (i = 0; i < cfg->hash->nslot; ++i) {
        //wtk_debug("================ i=%d ============\n",i);
        if (cfg->hash->slot[i]) {
            for (qn = cfg->hash->slot[i]->pop; qn; qn = qn->next) {
                node = data_offset(qn, hash_str_node_t, n);
                item2 = (wtk_wrdvec_item_t*) node->value;
                //wtk_debug("v[%d]:%p[%.*s] qn=%p next=%p\n",i,item2,item2->name->len,item2->name->data,qn,qn->next);
                //if(item1!=item2)
                {
                    //wtk_vecf_print(v1);
                    //wtk_vecf_print(item2->m);
                    //wtk_debug("[%.*s]\n",item2->name->len,item2->name->data);
                    //wtk_vecf_norm(item2->m);
                    f = wtk_vecf_cos2(v1, item2->m);
                    //wtk_debug("%f\n",f);
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
//						for(j=0;j<n;++j)
//						{
//							wtk_debug("v[%d]=[%.*s] %f\n",j,items[j].item->name->len,items[j].item->name->data,items[j].f);
//						}
//						if(n>3)
//						{
//							exit(0);
//						}
                    } else if (items[len - 1].f < f) {
//						for(j=0;j<n;++j)
//						{
//							wtk_debug("v[%d]=[%.*s] %f\n",j,items[j].item->name->len,items[j].item->name->data,items[j].f);
//						}
//						exit(0);
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
    }
    for (i = 0; i < n; ++i) {
        wtk_debug("v[%d]=[%.*s] %f\n", i, items[i].item->name->len,
                items[i].item->name->data, items[i].f);
    }
    wtk_free(items);
    exit(0);
}

void wtk_wrdvec_cfg_test2(wtk_wrdvec_cfg_t *cfg, char *s1, int s1_bytes)
{
    wtk_wrdvec_item_t *item1, *item2;
    wtk_queue_node_t *qn;
    hash_str_node_t *node;
    int i, j;
    float f;
    wtk_wrdvec_cmp_t *items;
    int n;
    int len;

    len = 100;
    n = 0;
    items = (wtk_wrdvec_cmp_t*) wtk_calloc(len, sizeof(wtk_wrdvec_cmp_t));
    item1 = wtk_wrdvec_cfg_find(cfg, s1, s1_bytes);
    for (i = 0; i < cfg->hash->nslot; ++i) {
        //wtk_debug("================ i=%d ============\n",i);
        if (cfg->hash->slot[i]) {
            for (qn = cfg->hash->slot[i]->pop; qn; qn = qn->next) {
                node = data_offset(qn, hash_str_node_t, n);
                item2 = (wtk_wrdvec_item_t*) node->value;
                //wtk_debug("v[%d]:%p[%.*s] qn=%p next=%p\n",i,item2,item2->name->len,item2->name->data,qn,qn->next);
                if (item1 != item2) {
                    f = wtk_vecf_cos(item1->m, item2->m);
                    //wtk_debug("%f\n",f);
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
//						for(j=0;j<n;++j)
//						{
//							wtk_debug("v[%d]=[%.*s] %f\n",j,items[j].item->name->len,items[j].item->name->data,items[j].f);
//						}
//						if(n>3)
//						{
//							exit(0);
//						}
                    } else if (items[len - 1].f < f) {
//						for(j=0;j<n;++j)
//						{
//							wtk_debug("v[%d]=[%.*s] %f\n",j,items[j].item->name->len,items[j].item->name->data,items[j].f);
//						}
//						exit(0);
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
    }
    for (i = 0; i < n; ++i) {
        wtk_debug("v[%d]=[%.*s] %f\n", i, items[i].item->name->len,
                items[i].item->name->data, items[i].f);
    }
    wtk_free(items);
    exit(0);
}

void wtk_wrdvec_cfg_write_bin(wtk_wrdvec_cfg_t *cfg, char *fn)
{
    wtk_strbuf_t *buf;
    wtk_strbuf_t *hdr;
    int v[2];
    int i;
    char c;
    wtk_wrdvec_item_t *item;
    FILE *f;

    hdr = wtk_strbuf_new(4096, 1);
    buf = wtk_strbuf_new(4096, 1);
    v[0] = cfg->voc_size;
    v[1] = cfg->vec_size;
    wtk_debug("%d/%d\n", v[0], v[1]);
    wtk_strbuf_push(hdr, (char*) v, 8);
    for (i = 0; i < cfg->voc_size; ++i) {
        item = cfg->wrds[i];
        c = item->name->len;
        wtk_strbuf_push_c(hdr, c);
        wtk_strbuf_push(hdr, item->name->data, item->name->len);
        wtk_strbuf_push(buf, (char*) item->m->p, sizeof(float) * cfg->vec_size);
    }
    wtk_debug("hdr=%d\n", hdr->pos);
    f = fopen(fn, "wb");
    fwrite(hdr->data, hdr->pos, 1, f);
    fwrite(buf->data, buf->pos, 1, f);
    fclose(f);
    wtk_strbuf_delete(hdr);
    wtk_strbuf_delete(buf);
}

void wtk_wrdvec_cfg_write_bin2(wtk_wrdvec_cfg_t *cfg, char *fn)
{
    wtk_wrdvec_item_t *item;
    int i;
    FILE *f;
    int v[2];
    char c;

    f = fopen(fn, "wb");
    v[0] = cfg->voc_size;
    v[1] = cfg->vec_size;
    fwrite(v, sizeof(int), 2, f);
    for (i = 0; i < cfg->voc_size; ++i) {
        item = cfg->wrds[i];
        c = item->name->len;
        fwrite(&c, 1, 1, f);
        fwrite(item->name->data, item->name->len, 1, f);
        fwrite(item->m->p, sizeof(float), cfg->vec_size, f);
    }
    fclose(f);
}
