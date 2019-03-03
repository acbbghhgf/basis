#include "wtk_hmmne.h" 

wtk_hmmne_t* wtk_hmmne_new(int hash_hint)
{
    wtk_hmmne_t *cfg;

    cfg = (wtk_hmmne_t*) wtk_malloc(sizeof(wtk_hmmne_t));
    cfg->hash = wtk_str_hash_new(hash_hint);
    cfg->B = 0;
    cfg->M = 0;
    cfg->E = 0;
    cfg->B_M = 0;
    cfg->B_E = 0;
    cfg->M_M = 0;
    cfg->M_E = 0;
    cfg->S = 0;
    return cfg;
}

wtk_hmmne_t* wtk_hmmne_new2(char *fn)
{
    wtk_hmmne_t *ne;

    ne = wtk_hmmne_new(3507);
    wtk_hmmne_load_file(ne, fn);
    //wtk_debug("new n=%p\n",ne);
    return ne;
}

void wtk_hmmne_delete(wtk_hmmne_t *n)
{
    //wtk_debug("delete n=%p\n",n);
    wtk_str_hash_delete(n->hash);
    wtk_free(n);
}

int wtk_hmmne_load(wtk_hmmne_t *cfg, wtk_source_t *src)
{
    wtk_strbuf_t *buf;
    wtk_strkv_parser_t parser;
    int ret;
    int eof;
    wtk_heap_t *heap;
    wtk_hmmnr_wrd_t *wrd;
    float v[6];
    float unk_cnt = 0.5;

    //cfg->hash=wtk_str_hash_new(cfg->hash_hint);
    heap = cfg->hash->heap;
    buf = wtk_strbuf_new(256, 1);
    ret = wtk_source_read_line2(src, buf, &eof);
    if (ret != 0) {
        goto end;
    }
    wtk_strkv_parser_init(&(parser), buf->data, buf->pos);
    while (1) {
        ret = wtk_strkv_parser_next(&(parser));
        if (ret != 0) {
            break;
        }
        //wtk_debug("[%.*s]=[%.*s]\n",parser.k.len,parser.k.data,parser.v.len,parser.v.data);
        if (wtk_str_equal_s(parser.k.data, parser.k.len, "B")) {
            cfg->B = wtk_str_atof(parser.v.data, parser.v.len);
        } else if (wtk_str_equal_s(parser.k.data, parser.k.len, "M")) {
            cfg->M = wtk_str_atof(parser.v.data, parser.v.len);
        } else if (wtk_str_equal_s(parser.k.data, parser.k.len, "E")) {
            cfg->E = wtk_str_atof(parser.v.data, parser.v.len);
        } else if (wtk_str_equal_s(parser.k.data, parser.k.len, "M1")) {
            cfg->M1 = wtk_str_atof(parser.v.data, parser.v.len);
        } else if (wtk_str_equal_s(parser.k.data, parser.k.len, "S")) {
            cfg->S = wtk_str_atof(parser.v.data, parser.v.len);
        }
    }
    //wtk_debug("%f/%f/%f\n",cfg->B,cfg->M,cfg->E);
    ret = wtk_source_read_line2(src, buf, &eof);
    if (ret != 0) {
        goto end;
    }
    wtk_strkv_parser_init(&(parser), buf->data, buf->pos);
    while (1) {
        ret = wtk_strkv_parser_next(&(parser));
        if (ret != 0) {
            break;
        }
        //wtk_debug("[%.*s]=[%.*s]\n",parser.k.len,parser.k.data,parser.v.len,parser.v.data);
        if (wtk_str_equal_s(parser.k.data, parser.k.len, "B.M")) {
            cfg->B_M = wtk_str_atof(parser.v.data, parser.v.len);
        } else if (wtk_str_equal_s(parser.k.data, parser.k.len, "B.E")) {
            cfg->B_E = wtk_str_atof(parser.v.data, parser.v.len);
        } else if (wtk_str_equal_s(parser.k.data, parser.k.len, "M.M")) {
            cfg->M_M = wtk_str_atof(parser.v.data, parser.v.len);
        } else if (wtk_str_equal_s(parser.k.data, parser.k.len, "M.E")) {
            cfg->M_E = wtk_str_atof(parser.v.data, parser.v.len);
        }
    }
    //wtk_debug("%f/%f/%f/%f\n",cfg->B_M,cfg->B_E,cfg->M_M,cfg->M_E);
    while (1) {
        ret = wtk_source_read_normal_string(src, buf);
        if (ret != 0) {
            ret = 0;
            goto end;
        }
        //wtk_debug("[%.*s]\n",buf->pos,buf->data);
        wrd = (wtk_hmmnr_wrd_t*) wtk_heap_malloc(heap, sizeof(wtk_hmmnr_wrd_t));
        wrd->wrd = wtk_heap_dup_string(heap, buf->data, buf->pos);
        ret = wtk_source_read_float(src, v, 6, 0);
        if (ret != 0) {
            wtk_debug("read float failed\n");
            goto end;
        }
        wrd->b = v[0];
        wrd->b_m = v[1];
        wrd->m_m = v[2];
        wrd->b_e = v[3];
        wrd->m_e = v[4];
        wrd->s = v[5];
        wtk_str_hash_add(cfg->hash, wrd->wrd->data, wrd->wrd->len, wrd);
    }
    ret = 0;
    end: cfg->unk_b = log(unk_cnt / (cfg->B)) + 0.0001;
    cfg->unk_be = cfg->unk_b; //log(unk_cnt/cfg->B);
    cfg->unk_bm = cfg->unk_b; //log(unk_cnt/cfg->B);
    cfg->unk_me = log(unk_cnt / cfg->M) + 0.0001;
    cfg->unk_mm = log(unk_cnt / cfg->M1) + 0.0001;
    wtk_strbuf_delete(buf);
    return ret;
}

int wtk_hmmne_load_file(wtk_hmmne_t *ne, char *fn)
{
    return wtk_source_load_file(ne, (wtk_source_load_handler_t) wtk_hmmne_load,
            fn);
}
