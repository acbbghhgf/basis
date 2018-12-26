#include "wtk_vecdb.h"

wtk_vecdb_t* wtk_vecdb_new(wtk_vecdb_cfg_t *cfg)
{
    wtk_vecdb_t *vb;

    vb = (wtk_vecdb_t*) wtk_malloc(sizeof(wtk_vecdb_t));
    vb->cfg = cfg;
    if (cfg->use_wrdvec) {
        vb->wrdvec = wtk_wrdvec_new(&(cfg->wrdvec), NULL);
    } else {
        vb->wrdvec = NULL;
    }
    //wtk_debug("db=%s\n",cfg->db);
    vb->vecfn = wtk_vecfn_new(cfg->db, cfg->vec_size);
    vb->buf = wtk_strbuf_new(256, 1);
    return vb;
}

void wtk_vecdb_delete(wtk_vecdb_t *v)
{
    wtk_vecfn_delete(v->vecfn);
    wtk_strbuf_delete(v->buf);
    if (v->wrdvec && v->cfg->use_wrdvec) {
        wtk_wrdvec_delete(v->wrdvec);
    }
    wtk_free(v);
}

void wtk_vecdb_add(wtk_vecdb_t *v, char *q, int q_bytes, char *a, int a_bytes)
{
    wtk_vecf_t *v1 = v->wrdvec->v1;
    //wtk_string_t str;

    //wtk_debug("[ADD]=[%.*s][%.*s]\n",q_bytes,q,a_bytes,a);
    wtk_wrdvec_snt_to_vec(v->wrdvec, q, q_bytes, v1);
    wtk_vecfn_get(v->vecfn, v1, v->cfg->set_like_thresh, q, q_bytes, a, a_bytes,
            1);
    //wtk_debug("[%.*s]\n",str.len,str.data);
}

wtk_string_t wtk_vecdb_get(wtk_vecdb_t *v, char *q, int q_bytes)
{
    wtk_vecf_t *v1 = v->wrdvec->v1;
    wtk_string_t t;

    wtk_wrdvec_snt_to_vec(v->wrdvec, q, q_bytes, v1);
    t = wtk_vecfn_get(v->vecfn, v1, v->cfg->get_like_thresh, q, q_bytes, NULL,
            0, 0);
    //wtk_debug("[GET]=[%.*s][%.*s]\n",q_bytes,q,t.len,t.data);
    return t;
}
