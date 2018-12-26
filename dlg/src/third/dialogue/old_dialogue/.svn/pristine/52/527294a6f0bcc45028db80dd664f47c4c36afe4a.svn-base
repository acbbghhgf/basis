#include "wtk_treebin.h" 

void wtk_treebin_tbl_print_idx(wtk_treebin_tbl_t *t)
{
    int i;
    for (i = 0; i < t->nslot; ++i) {
        wtk_debug("v[%d]=%d\n", i, t->slot_idx[i]);
    }
}

wtk_treebin_tbl_t* wtk_treebin_tbl_new(wtk_heap_t *heap)
{
    wtk_treebin_tbl_t *tbl;

    tbl = (wtk_treebin_tbl_t*) wtk_heap_malloc(heap, sizeof(wtk_treebin_tbl_t));
    tbl->nslot = 0;
    tbl->offset = 0;
    tbl->slot_idx = NULL;
    return tbl;
}

/**
 *	* string map
 *	* section map  nm data-offset
 *	* section idx
 */
int wtk_treebin_read_slot(wtk_treebin_t *t)
{
    char buf[256];
    unsigned int v[2];
    FILE *f = t->f;
    int i, ret;
    unsigned char b;
    wtk_hash_str_node_t *node;
    wtk_heap_t *heap;
    wtk_treebin_tbl_t* tbl;
    wtk_queue_node_t *qn;
    unsigned int offset;

    ret = fread(buf, 1, 4, f);
    if (ret != 4) {
        ret = -1;
        goto end;
    }
    //wtk_debug("%.*s\n",ret,buf);
    ret = fread(v, 4, 2, f);
    if (ret != 2) {
        ret = -1;
        goto end;
    }
    //wtk_debug("[%d/%d]\n",v[0],v[1]);
    t->max = v[0];
    t->nsection = v[1];
    t->hash = wtk_str_hash_new(t->max * 2 + 1);
    for (i = 0; i < t->max; ++i) {
        ret = fread(&b, 1, 1, f);
        if (ret != 1) {
            ret = -1;
            goto end;
        }
        ret = fread(buf, 1, b, f);
        if (ret != b) {
            ret = -1;
            goto end;
        }
        //wtk_debug("v[%d]=[%.*s]\n",i,b,buf);
        node = (wtk_hash_str_node_t*) wtk_str_hash_find_node3(t->hash, buf, b,
                1);
        node->v.i = i;
    }
    //wtk_debug("offset=%d\n",(int)ftell(f));
    t->section = wtk_str_hash_new(t->nsection * 2 + 1);
    heap = t->section->heap;
    for (i = 0; i < t->nsection; ++i) {
        ret = fread(&b, 1, 1, f);
        if (ret != 1) {
            ret = -1;
            goto end;
        }
        ret = fread(buf, 1, b, f);
        if (ret != b) {
            ret = -1;
            goto end;
        }
        ret = fread(v, 4, 1, f);
        if (ret != 1) {
            ret = -1;
            goto end;
        }
        //wtk_debug("v[%d]=[%.*s] of=%d\n",i,b,buf,v[0]);
        tbl = wtk_treebin_tbl_new(heap);
        tbl->offset = v[0];
        tbl->nm = wtk_heap_dup_string(heap, buf, b);
        wtk_str_hash_add(t->section, tbl->nm->data, tbl->nm->len, tbl);
        wtk_queue_push(&(t->section_q), &(tbl->q_n));
        //wtk_debug("%p\n",wtk_str_hash_find(t->section,buf,b));
    }
    //wtk_debug("%p\n",wtk_str_hash_find(t->section,"地名",strlen("地名")));
    offset = ftell(f);
    for (qn = t->section_q.pop; qn; qn = qn->next) {
        tbl = data_offset2(qn, wtk_treebin_tbl_t, q_n);
        tbl->offset += offset;
        ret = fseek(f, tbl->offset, SEEK_SET);
        if (ret != 0) {
            //wtk_debug("set of=%d failed\n",tbl->offset);
            goto end;
        }
        ret = fread(v, 4, 2, f);
        if (ret != 2) {
            ret = -1;
            goto end;
        }
        //wtk_debug("[%d/%d]\n",v[0],v[1]);
        tbl->step = v[0];
        tbl->nslot = v[1];
        tbl->slot_idx = (unsigned int*) wtk_heap_malloc(heap,
                sizeof(unsigned int) * tbl->nslot);
        ret = fread(tbl->slot_idx, 4, tbl->nslot, f);
        if (ret != tbl->nslot) {
            ret = -1;
            goto end;
        }
        tbl->offset = ftell(f);
        //wtk_debug("[%.*s]=%d\n",tbl->nm->len,tbl->nm->data,tbl->offset);
    }
    ret = fseek(f, 0, SEEK_END);
    if (ret != 0) {
        goto end;
    }
    t->eof_offset = ftell(f);
    ret = 0;
end:
    //exit(0);
    return ret;
}

wtk_treebin_t* wtk_treebin_new(char *fn, wtk_rbin2_t *rbin)
{
    wtk_treebin_t *t = NULL;
    FILE *f;
    int ret;

    if (rbin) {
        f = wtk_rbin2_get_file(rbin, fn);
    } else {
        f = fopen(fn, "rb");
    }
    if (!f) {
        goto end;
    }
    t = (wtk_treebin_t*) wtk_malloc(sizeof(wtk_treebin_t));
    t->f = f;
    t->hash = NULL;
    t->section = NULL;
    wtk_queue_init(&(t->section_q));
    ret = wtk_treebin_read_slot(t);
    if (ret != 0) {
        wtk_treebin_delete(t);
        t = NULL;
    }
    end: return t;
}

void wtk_treebin_delete(wtk_treebin_t *t)
{
    if (t->section) {
        wtk_str_hash_delete(t->section);
    }
    if (t->hash) {
        wtk_str_hash_delete(t->hash);
    }
    if (t->f) {
        fclose(t->f);
    }
    wtk_free(t);
}

void wtk_treebin_env_init(wtk_treebin_env_t *e)
{
    e->tbl = NULL;
    e->idx = 0;
    e->is_end = 0;
    e->offset = 0;
    e->is_err = 0;
}

void wtk_treebin_env_print(wtk_treebin_env_t *e)
{
    wtk_debug("========== env ===========\n");
    printf("idx: %d\n", e->idx);
    printf("of: %d\n", e->offset);
    printf("end: %d\n", e->is_end);
    printf("err: %d\n", e->is_err);
}

int wtk_treebin_get_slot(wtk_treebin_t *t, wtk_treebin_env_t *env, int offset,
        unsigned int idx)
{
    unsigned int cnt;
    FILE *f = t->f;
    int ret;
    int i;
    char *data = NULL, *p;
    int v;
    unsigned int vi;
    unsigned int ko;
    unsigned char b;

    ret = fseek(f, offset, SEEK_SET);
    if (ret != 0) {
        wtk_debug("set of=%d failed\n", offset);
        goto end;
    }
    ret = fread(&cnt, 4, 1, f);
    if (ret != 1) {
        perror(__FUNCTION__);
        wtk_debug("read cnt failed,ret=%d offset=%d idx=%d\n", ret, offset, idx);
        ret = -1;
        goto end;
    }
    if (cnt <= 0) {
        ret = -1;
        goto end;
    }
    v = cnt * 7;
    data = wtk_malloc(v);
    ret = fread(data, 1, v, f);
    if (ret != v) {
        wtk_debug("read data failed v=%d\n", v);
        ret = -1;
        goto end;
    }
    //wtk_debug("v=%d\n",v);
    p = data;
    for (i = 0; i < cnt; ++i, p += 7) {
        b = *(unsigned char*) (p);
        vi = *(unsigned short*) (p + 1);
        vi += (b & 0x7f) << 16;
        //wtk_debug("v[%d/%d]=%d/%d\n",i,cnt,vi,idx);
        if (vi == idx) {
            //print_hex(p,7);
            ko = *(unsigned int*) (p + 3);
            //wtk_debug("b=%x offset=%d/%d\n",b,offset,ko+4+v);
            env->is_end = b >> 7; //vi>>15;
            env->offset = ko + offset + 4 + v;
            ret = 0;
            goto end;
        }
    }
    ret = -1;
    end: if (data) {
        wtk_free(data);
    }
    //wtk_treebin_env_print(env);
    return ret;
}

int wtk_treebin_search_slot(wtk_treebin_t *t, wtk_treebin_env_t *e,
        unsigned int idx)
{
    unsigned int v = 0;
    int i;
    int ret;

    i = (int) (idx / e->tbl->step + 0.5);
    //printf("idx=%d/%d\n",idx,i);
    v = e->tbl->slot_idx[i] + e->tbl->offset;
    //wtk_debug("idx=%d/%d v=%d\n",i,idx,v);
    //wtk_debug("idx=%d i=%d v=%d\n",idx,i,v);
    ret = wtk_treebin_get_slot(t, e, v, idx);
    return ret;
}

int wtk_treebin_search(wtk_treebin_t *t, wtk_treebin_env_t *e, unsigned int idx)
{
    int ret;

    //wtk_debug("idx=%d\n",e->idx);
    if (e->idx == 0) {
        ret = wtk_treebin_search_slot(t, e, idx);
    } else {
        ret = wtk_treebin_get_slot(t, e, e->offset, idx);
        //exit(0);
    }
    ++e->idx;
    return ret;
}

int wtk_treebin_search2(wtk_treebin_t *t, wtk_treebin_env_t *e, char *data,
        int bytes)
{
    wtk_hash_str_node_t *node;

    //wtk_debug("[%.*s]\n",bytes,data);
    node = (wtk_hash_str_node_t*) wtk_str_hash_find_node3(t->hash, data, bytes,
            0);
    //wtk_debug("node=%p\n",node);
    if (!node) {
        return -1;
    }
    return wtk_treebin_search(t, e, node->v.i);
}

int wtk_treebin_has2(wtk_treebin_t *t, wtk_treebin_env_t *env, char *data,
        int bytes)
{
    typedef enum {
        WTK_TREEBIN_INIT, WTK_TREEBIN_ENG,
    } wtk_treebin_txt_t;
    char *s, *e;
    int n;
    wtk_treebin_txt_t state;
    wtk_string_t v;
    int ret;

    //wtk_debug("[%.*s]\n",bytes,data);
    wtk_string_set(&(v), 0, 0);
    s = data;
    e = s + bytes;
    state = WTK_TREEBIN_INIT;
    while (s < e) {
        n = wtk_utf8_bytes(*s);
        switch (state) {
            case WTK_TREEBIN_INIT:
                if (n > 1) {
                    ret = wtk_treebin_search2(t, env, s, n);
                    if (ret != 0) {
                        goto end;
                    }
                } else {
                    if (s + n >= e) {
                        ret = wtk_treebin_search2(t, (env), s, n);
                        if (ret != 0) {
                            goto end;
                        }
                    } else {
                        v.data = s;
                        state = WTK_TREEBIN_ENG;
                    }
                }
                break;
            case WTK_TREEBIN_ENG:
                if (n > 1) {
                    v.len = s - v.data;
                    //wtk_debug("[%.*s]\n",v.len,v.data);
                    //wtk_debug("[%.*s]\n",n,s);
                    ret = wtk_treebin_search2(t, env, v.data, v.len);
                    if (ret != 0) {
                        goto end;
                    }
                    ret = wtk_treebin_search2(t, env, s, n);
                    if (ret != 0) {
                        goto end;
                    }
                    state = WTK_TREEBIN_INIT;
                } else if (s + n >= e) {
                    v.len = s - v.data;
                    //wtk_debug("[%.*s]\n",v.len,v.data);
                    ret = wtk_treebin_search2(t, (env), v.data, v.len);
                    if (ret != 0) {
                        goto end;
                    }
                }
                break;
        }
        s += n;
    }
    ret = 0;
    end: return ret;
}

wtk_treebin_tbl_t* wtk_treebin_find_tbl(wtk_treebin_t *t, char *nm,
        int nm_bytes)
{
    return (wtk_treebin_tbl_t*) wtk_str_hash_find(t->section, nm, nm_bytes);
}

wtk_treebin_env_t wtk_treebin_has(wtk_treebin_t *t, char *nm, int nm_bytes,
        char *data, int bytes)
{
    wtk_treebin_env_t env;
    int ret = -1;

    wtk_treebin_env_init(&(env));
    env.tbl = (wtk_treebin_tbl_t*) wtk_str_hash_find(t->section, nm, nm_bytes);
    if (!env.tbl) {
        wtk_debug("[%.*s] not found %p.\n", nm_bytes, nm, env.tbl);
        goto end;
    }
    ret = wtk_treebin_has2(t, &env, data, bytes);
    end: if (ret != 0) {
        env.is_err = 1;
    }
    return env;
}
