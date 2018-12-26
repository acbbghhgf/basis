#include "wtk_semdef.h" 

wtk_semdef_t* wtk_semdef_new(char *dn)
{
	wtk_semdef_t *def;

	def=(wtk_semdef_t*)wtk_malloc(sizeof(wtk_semdef_t));
	def->dn=wtk_string_dup_data(dn,strlen(dn));
	def->buf=wtk_strbuf_new(256,1);
	return def;
}

void wtk_semdef_delete(wtk_semdef_t *def)
{
	wtk_string_delete(def->dn);
	wtk_strbuf_delete(def->buf);
	wtk_free(def);
}

int wtk_semdef_get_def(wtk_semdef_t *def,char *sec,int sec_len,char *k,int k_len)
{
	wtk_strbuf_t *buf=def->buf;
	wtk_fkv2_t *kv;
	wtk_fkv_env_t env;
	int ret;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,def->dn->data,def->dn->len);
	wtk_strbuf_push_s(buf,"/");
	wtk_strbuf_push(buf,sec,sec_len);
	wtk_strbuf_push_s(buf,".bin");
	wtk_strbuf_push_c(buf,0);
	kv=wtk_fkv2_new(buf->data);
	wtk_fkv_env_init(kv,&(env));
	ret=wtk_fkv2_has(kv,&env,k,k_len);
	if(ret==0)
	{
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,kv->buf->data,kv->buf->pos);
	}
	wtk_fkv2_delete(kv);
	return ret;
}
