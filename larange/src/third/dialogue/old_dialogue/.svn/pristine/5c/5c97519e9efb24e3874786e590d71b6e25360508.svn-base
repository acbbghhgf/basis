#include "wtk_vecfaq.h" 

wtk_vecfaq_t* wtk_vecfaq_new(wtk_vecfaq_cfg_t *cfg, wtk_rbin2_t *rbin)
{
    wtk_vecfaq_t *v;
    int i;

    v = (wtk_vecfaq_t*) wtk_malloc(sizeof(wtk_vecfaq_t));
    v->cfg = cfg;
    if (cfg->use_share_wrdvec) {
        v->wrdvec = NULL;
    } else {
        v->wrdvec = wtk_wrdvec_new(&(cfg->wrdvec), NULL);
    }
    if (cfg->nmap > 0) {
        v->map_bins = (wtk_faqbin_t**) wtk_calloc(cfg->nmap,
                sizeof(wtk_faqbin_t*));
        for (i = 0; i < cfg->nmap; ++i) {
            v->map_bins[i] = wtk_faqbin_new(cfg->map[i].fn, rbin);
            v->map_bins[i]->thresh = cfg->map[i].thresh;
            v->map_bins[i]->best_thresh = cfg->map[i].best_thresh;
        }
    } else {
        v->map_bins = NULL;
    }
    v->bins = (wtk_faqbin_t**) wtk_calloc(cfg->ndat, sizeof(wtk_faqbin_t*));
    for (i = 0; i < cfg->ndat; ++i) {
        v->bins[i] = wtk_faqbin_new(cfg->dat[i].fn, rbin);
        v->bins[i]->thresh = cfg->dat[i].thresh;
        v->bins[i]->best_thresh = cfg->dat[i].best_thresh;
    }
    if (v->wrdvec) {
        v->last_vec = wtk_vecf_new(v->wrdvec->cfg->vec_size);
    } else {
        v->last_vec = NULL;
    }
    return v;
}

void wtk_vecfaq_set_wrdvec(wtk_vecfaq_t *v, wtk_wrdvec_t *wvec)
{
    v->wrdvec = wvec;
    v->last_vec = wtk_vecf_new(wvec->cfg->vec_size);
}

void wtk_vecfaq_delete(wtk_vecfaq_t *faq)
{
    int i;

    //wtk_debug("last=%p\n",faq->last_vec);
    wtk_vecf_delete(faq->last_vec);
    if (faq->bins) {
        for (i = 0; i < faq->cfg->ndat; ++i) {
            wtk_faqbin_delete(faq->bins[i]);
        }
        wtk_free(faq->bins);
    }
    if (faq->map_bins) {
        for (i = 0; i < faq->cfg->nmap; ++i) {
            wtk_faqbin_delete(faq->map_bins[i]);
        }
        wtk_free(faq->map_bins);
    }
    if (!faq->cfg->use_share_wrdvec) {
        wtk_wrdvec_delete(faq->wrdvec);
    }
    wtk_free(faq);
}

void wtk_vecfaq_reset(wtk_vecfaq_t *faq)
{
    int i;

    for (i = 0; i < faq->cfg->ndat; ++i) {
        wtk_faqbin_reset(faq->bins[i]);
    }
}

wtk_faqbin_t* wtk_vecfaq_get_section(wtk_vecfaq_t *faq, char *s, int s_len)
{
    int i;

    for (i = 0; i < faq->cfg->nmap; ++i) {
        if (wtk_string_cmp(faq->cfg->map[i].nm, s, s_len) == 0) {
            return faq->map_bins[i];
        }
    }
    return NULL;
}

wtk_string_t wtk_vecfaq_get3(wtk_vecfaq_t *faq, char *s, int s_len, char *input,
        int input_len)
{
    wtk_faqbin_t *bin;
    wtk_string_t v;
    wtk_vecf_t *f;

    f = faq->wrdvec->v1;
    faq->index = -1;
    wtk_wrdvec_snt_to_vec(faq->wrdvec, input, input_len, f);
    bin = wtk_vecfaq_get_section(faq, s, s_len);
    if (!bin) {
        wtk_string_set(&v, 0, 0);
        return v;
    }
    v = wtk_faqbin_get(bin, f);
    //wtk_debug("v[%d]: [%.*s]=%f\n",i,v.len,v.data,bin->prob);
    if (bin->prob > bin->thresh) {
        faq->prob = bin->prob;
        return v;
    }
    faq->prob = 0;
    wtk_string_set(&(v), 0, 0);
    return v;
}

wtk_string_t wtk_vecfaq_get(wtk_vecfaq_t *faq, char *data, int bytes)
{
    wtk_faqbin_t *bin;
    wtk_string_t v;
    wtk_vecf_t *f;
    int i;

    f = faq->wrdvec->v1;
    faq->index = -1;
    wtk_wrdvec_snt_to_vec(faq->wrdvec, data, bytes, f);
//	if(0)
//	{
//		float t;
//
//		t=0.2;
//		wtk_vecf_scale(faq->last_vec,t);
//		wtk_vecf_scale(f,1-t);
//		wtk_vecf_add(f,faq->last_vec->p);
//		wtk_vecf_norm(f);
//		wtk_vecf_cpy(faq->last_vec,f);
//	}
    for (i = 0; i < faq->cfg->ndat; ++i) {
        bin = faq->bins[i];
        v = wtk_faqbin_get(bin, f);
        //wtk_debug("v[%d]: [%.*s]=%f\n",i,v.len,v.data,bin->prob);
        if (bin->prob > bin->thresh) {
            faq->prob = bin->prob;
            faq->index = i;
            return v;
        }
    }
    faq->prob = 0;
    wtk_string_set(&(v), 0, 0);
    return v;
}

wtk_robin_t* wtk_vecfaq_get2(wtk_vecfaq_t *faq, char *data, int bytes)
{
    wtk_faqbin_t *bin;
    wtk_vecf_t *f;
    int i;
    wtk_robin_t *rb;

    f = faq->wrdvec->v1;
    wtk_wrdvec_snt_to_vec(faq->wrdvec, data, bytes, f);
    for (i = 0; i < faq->cfg->ndat; ++i) {
        bin = faq->bins[i];
        rb = wtk_faqbin_get2(bin, f);
        if (bin->prob > bin->thresh) {
            faq->prob = bin->prob;
            faq->index = i;
            return rb;
        }
        wtk_faqbin_reset(bin);
    }
    return NULL;
}
