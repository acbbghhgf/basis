#include "wtk_label.h"

wtk_label_t* wtk_label_new(int hint)
{
	wtk_label_t *l;

	l=(wtk_label_t*)malloc(sizeof(*l));
	wtk_label_init(l,hint);
	return l;
}

int wtk_label_delete(wtk_label_t *l)
{
	wtk_label_clean(l);
	free(l);
	return 0;
}

int wtk_label_bytes(wtk_label_t *l)
{
	int bytes=sizeof(*l);

	bytes+=l->nslot*sizeof(wtk_queue3_t*);
	bytes+=wtk_heap_bytes(l->heap);
	return bytes;
}

int wtk_label_init(wtk_label_t* l,int hint)
{
	l->heap=wtk_heap_new(4096);
	l->nslot=hint;
	l->slot=(wtk_label_node_t**)wtk_calloc(l->nslot,sizeof(wtk_label_node_t*));
	return 0;
}

int wtk_label_clean(wtk_label_t *l)
{
	wtk_free(l->slot);
	wtk_heap_delete(l->heap);
	return 0;
}

int wtk_label_reset(wtk_label_t *l)
{
	memset(l->slot,0,sizeof(wtk_queue3_t*) * l->nslot);
	wtk_heap_reset(l->heap);
	return 0;
}

wtk_name_t* wtk_label_find(wtk_label_t *l,char *s,int sl,int insert)
{
	wtk_heap_t *heap=l->heap;
	wtk_label_node_t *qn,*prev;
	int idx;
	wtk_name_t *n;

	//print_data(s,sl);
	idx=hash_string_value_len(s,sl,l->nslot);
	if(!l->slot[idx])
	{
		if(!insert)
		{
			n=NULL;
			goto end;
		}
		prev=NULL;
	}else
	{
		prev=NULL;
		for(qn=l->slot[idx];qn;qn=qn->next)
		{
			n=data_offset2(qn,wtk_name_t,q_n);
			prev=qn;
			if(wtk_string_cmp(n->name,s,sl)==0)
			{
				goto end;
			}
		}
		if(!insert)
		{
			n=NULL;
			goto end;
		}
	}
	n=(wtk_name_t*)wtk_heap_malloc(heap,sizeof(wtk_name_t));
	n->data=0;
	n->name=wtk_heap_dup_string(heap,s,sl);
	n->q_n.next=NULL;
	if(prev)
	{
		prev->next=&(n->q_n);
	}else
	{
		l->slot[idx]=&(n->q_n);
	}
end:
	return n;
}

wtk_string_t* wtk_label_find2(wtk_label_t *l,char *s,int sl,int insert)
{
	wtk_string_t *v=0;
	wtk_name_t *nm;

	nm=wtk_label_find(l,s,sl,insert);
	if(!nm){goto end;}
	v=nm->name;
end:
	return v;
}
