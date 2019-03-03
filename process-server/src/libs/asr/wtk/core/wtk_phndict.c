#include "wtk_phndict.h"
#include "wtk/core/wtk_larray.h"
#include <ctype.h>

void wtk_phndict_add_wrd(wtk_phndict_t *dict,wtk_phndict_wrd_t *wrd)
{
	//wtk_debug("[%.*s]\n",wrd->wrd->len,wrd->wrd->data);
	wtk_str_hash_add(dict->hash,wrd->wrd->data,wrd->wrd->len,wrd);
}

int wtk_phndict_load(wtk_phndict_t *dict,wtk_source_t *src)
{
	wtk_strpool_xitem_t *xi;
	wtk_str_spwrd_iter_t iter;
	wtk_string_t v;
	wtk_strbuf_t *buf;
	int ret;
	wtk_larray_t *a;
	wtk_phndict_wrd_t *wrd;
	wtk_heap_t *heap=dict->hash->heap;
	int n;
	int idx;
	int xnew;

	idx=0;
	a=wtk_larray_new(10,sizeof(wtk_strpool_xitem_t*));
	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_read_line(src,buf);
		if(ret!=0 || buf->pos==0){ret=0;goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		wrd=NULL;
		wtk_larray_reset(a);
		wtk_str_spwrd_iter_init(&iter,buf->data,buf->pos);
		while(1)
		{
			v=wtk_str_spwrd_iter_next(&(iter));
			if(v.len==0){break;}
			//wtk_debug("[%.*s]\n",v.len,v.data);
			if(wrd)
			{
				xnew=0;
				if(isdigit(v.data[v.len-1]))
				{
					xi=(wtk_strpool_xitem_t*)wtk_strpool_find_item2(dict->pool,v.data,v.len-1,1,&xnew);
					if(xnew)
					{
						xi->hook.i=idx;
						++idx;
						//wtk_debug("idx=%d\n",idx);
					}
					wtk_larray_push2(a,&xi);
					xi=(wtk_strpool_xitem_t*)wtk_strpool_find_item2(dict->pool,v.data+v.len-1,1,1,&xnew);
					if(xnew)
					{
						xi->hook.i=idx;
						++idx;
						//wtk_debug("idx=%d\n",idx);
					}
					wtk_larray_push2(a,&xi);

				}else
				{
					xi=(wtk_strpool_xitem_t*)wtk_strpool_find_item2(dict->pool,v.data,v.len,1,&xnew);
					if(xnew)
					{
						xi->hook.i=idx;
						++idx;
						//wtk_debug("idx=%d\n",idx);
					}
					wtk_larray_push2(a,&xi);
				}
			}else
			{
				wrd=(wtk_phndict_wrd_t*)wtk_heap_malloc(heap,sizeof(wtk_phndict_wrd_t));
				wrd->wrd=wtk_heap_dup_string(heap,v.data,v.len);
				wrd->nphn=0;
				wrd->phns=NULL;
			}
		}
		//wtk_debug("n=%d\n",a->nslot);
		wrd->nphn=a->nslot;
		n=sizeof(wtk_strpool_xitem_t*)*a->nslot;
		wrd->phns=(wtk_strpool_xitem_t**)wtk_heap_malloc(heap,n);
		memcpy(wrd->phns,a->slot,n);
		wtk_phndict_add_wrd(dict,wrd);
		//exit(0);
	}
	ret=0;
end:
	wtk_debug("idx=%d\n",idx);
	//exit(0);
	dict->nphn=idx;
	wtk_larray_delete(a);
	wtk_strbuf_delete(buf);
	return ret;
}

wtk_phndict_t* wtk_phndict_new(char *fn)
{
	wtk_phndict_t *phn;
	unsigned long long lines;

	phn=(wtk_phndict_t*)wtk_malloc(sizeof(wtk_phndict_t));
	phn->pool=wtk_strpool_new(101);
	lines=wtk_file_lines(fn);
	lines=lines*2+1;
	phn->hash=wtk_str_hash_new(lines);
	wtk_source_load_file(phn,(wtk_source_load_handler_t)wtk_phndict_load,fn);
	return phn;
}

void wtk_phndict_delete(wtk_phndict_t *phn)
{
	if(phn->hash)
	{
		wtk_str_hash_delete(phn->hash);
	}
	wtk_strpool_delete(phn->pool);
	wtk_free(phn);
}

wtk_phndict_wrd_t* wtk_phndict_find(wtk_phndict_t *dict,char *wrd,int bytes)
{
	return (wtk_phndict_wrd_t*)wtk_str_hash_find(dict->hash,wrd,bytes);
}

void wtk_phndict_wrd_print(wtk_phndict_wrd_t *wrd)
{
	int i;

	wtk_debug("================ word =================\n");
	printf("WORD: %.*s\n",wrd->wrd->len,wrd->wrd->data);
	for(i=0;i<wrd->nphn;++i)
	{
		printf("v[%d]=%.*s\n",i,wrd->phns[i]->v.len,wrd->phns[i]->v.data);
	}
}
