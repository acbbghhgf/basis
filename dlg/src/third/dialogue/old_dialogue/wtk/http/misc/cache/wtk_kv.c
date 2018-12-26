#include "wtk_kv.h"

wtk_kv_t* wtk_kv_new(char *k,int kl,char *v,int vl)
{
	wtk_kv_t *kv;

	kv=(wtk_kv_t*)wtk_calloc(1,sizeof(*kv));
	kv->k=wtk_string_dup_data(k,kl);
	kv->v=wtk_string_dup_data(v,vl);
	return kv;
}

int wtk_kv_delete(wtk_kv_t *kv)
{
	wtk_string_delete(kv->k);
	wtk_string_delete(kv->v);
	wtk_free(kv);
	return 0;
}
