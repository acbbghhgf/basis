#include "wtk_strv.h"
#include "wtk/core/wtk_sort.h"
#include "wtk/core/wtk_str.h"

int wtk_strv_sort_cmp(void *app_data,wtk_strv_t *src,wtk_strv_t *dst)
{
	return wtk_string_cmp(&(src->key),dst->key.data,dst->key.len);
}

void wtk_strv_array_sort(wtk_strv_t* v,int len)
{
	wtk_strv_t tmp;

	wtk_qsort(v,&(v[len-1]),sizeof(wtk_strv_t),(wtk_qsort_cmp_f)wtk_strv_sort_cmp,0,&tmp);
}

int wtk_string_strv_cmp(wtk_string_t *str,wtk_strv_t *dst)
{
	//wtk_strv_print(dst);
	return wtk_string_cmp(str,dst->key.data,dst->key.len);
}

wtk_strv_t* wtk_strv_array_find(wtk_strv_t *v,int v_len,char *key,int key_bytes)
{
	wtk_string_t s;

	wtk_string_set(&s,key,key_bytes);
	return (wtk_strv_t*)wtk_binary_search(v,&(v[v_len-1]),sizeof(wtk_strv_t),(wtk_search_cmp_f)wtk_string_strv_cmp,&s);
}

wtk_strv_t* wtk_strv_array_find2(wtk_strv_t *v,int v_len,char *key,int key_bytes)
{
	wtk_string_t s;

	wtk_string_set(&s,key,key_bytes);
	return (wtk_strv_t*)wtk_inc_search(v,&(v[v_len-1]),sizeof(wtk_strv_t),(wtk_search_cmp_f)wtk_string_strv_cmp,&s);
}


//================ Print Section ==================

void wtk_strv_print(wtk_strv_t *v)
{
	printf("%.*s=%p\n",v->key.len,v->key.data,v->v.v);
}

void wtk_strv_array_print(wtk_strv_t *v,int len)
{
	int i;

	printf("=======================================\n");
	for(i=0;i<len;++i)
	{
		printf("v[%d]=%.*s=>%p\n",i,v[i].key.len,v[i].key.data,v[i].v.v);
	}
}

//=================   Example Section ================
void wtk_strv_array_test()
{
	wtk_strv_t vs[]={
			wtk_strv_s("GET",0),
			wtk_strv_s("POST",1),
			wtk_strv_s("HEAD",2),
			wtk_strv_s("PUT",3),
			wtk_strv_s("DELETE",4),
			wtk_strv_s("TRACE",5),
			wtk_strv_s("CONNECT",6),
			wtk_strv_s("OPTIONS",7),
	};
	int n;
	wtk_strv_t *v;
	int i;

	n=sizeof(vs)/sizeof(wtk_strv_t);
	wtk_strv_array_print(vs,n);
	wtk_strv_array_sort(vs,n);
	wtk_strv_array_print(vs,n);
	for(i=0;i<n;++i)
	{
		wtk_debug("========= found %.*s ===========\n",vs[i].key.len,vs[i].key.data);
		//v=wtk_strv_array_find(vs,n,vs[i].key.data,vs[i].key.len);
		v=wtk_strv_array_find2(vs,n,vs[i].key.data,vs[i].key.len);
		wtk_debug("======== result %p =======\n",v);
		wtk_strv_print(v);
	}
}
