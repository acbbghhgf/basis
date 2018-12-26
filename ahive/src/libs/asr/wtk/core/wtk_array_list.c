#include "wtk_array_list.h"

wtk_array_item_t* wtk_array_item_new(int elem_size,int item_alloc)
{
	wtk_array_item_t *item;

	item=(wtk_array_item_t*)wtk_malloc(sizeof(*item));
	item->nslot=0;
	item->slot=wtk_calloc(item_alloc,elem_size);
	return item;
}

int wtk_array_item_delete(wtk_array_item_t *item)
{
	wtk_free(item->slot);
	wtk_free(item);
	return 0;
}

void wtk_array_list_add_block(wtk_array_list_t *l)
{
	wtk_array_item_t *item;

	item=wtk_array_item_new(l->elem_size,l->item_alloc);
	wtk_queue_push(&(l->item_queue),&(item->q_n));
}

wtk_array_list_t* wtk_array_list_new(int elem_size,int item_alloc)
{
	wtk_array_list_t *l;

	l=(wtk_array_list_t*)wtk_malloc(sizeof(*l));
	wtk_queue_init(&(l->item_queue));
	l->item_alloc=item_alloc;
	l->elem_size=elem_size;
	l->len=0;
	return l;
}

int wtk_array_list_delete(wtk_array_list_t *l)
{
	wtk_array_list_reset(l);
	wtk_free(l);
	return 0;
}

void wtk_array_list_reset(wtk_array_list_t *l)
{
	wtk_queue_node_t *n,*p;
	wtk_array_item_t *item;

	for(n=l->item_queue.pop;n;n=p)
	{
		p=n->next;
		item=data_offset(n,wtk_array_item_t,q_n);
		wtk_array_item_delete(item);
	}
	wtk_queue_init(&(l->item_queue));
	l->len=0;
}

void wtk_array_list_push(wtk_array_list_t *l,void *data,int n)
{
	wtk_array_item_t *item;
	int left,step;
	int bytes;

	if(n<=0){return;}
	if(!l->item_queue.pop)
	{
		//make sure block exist.
		wtk_array_list_add_block(l);
	}
	l->len+=n;
	while(n>0)
	{
		item=wtk_array_list_last_item(l);
		left=l->item_alloc-item->nslot;
		if(left>0)
		{
			step=min(n,left);
			bytes=step*l->elem_size;
			memcpy((char*)item->slot+item->nslot*l->elem_size,data,bytes);
			item->nslot+=step;
			n-=step;
			data=(void*)((char*)data+bytes);
		}
		if(item->nslot>=l->item_alloc)
		{
			wtk_array_list_add_block(l);
		}
	}
}

void wtk_array_list_write(wtk_array_list_t *l,char *fn)
{
	wtk_array_item_t *item;
	wtk_queue_node_t *n;
	FILE *f;

	f=fopen(fn,"wb");
	for(n=l->item_queue.pop;n;n=n->next)
	{
		item=data_offset(n,wtk_array_item_t,q_n);
		fwrite(item->slot,l->elem_size,item->nslot,f);
	}
	fclose(f);
}

void wtk_array_list_print(wtk_array_list_t *l)
{
	wtk_array_item_t *item;
	wtk_queue_node_t *n;
	int nslot;

	printf("=================== array  list ================\n");
	for(nslot=0,n=l->item_queue.pop;n;n=n->next)
	{
		item=data_offset(n,wtk_array_item_t,q_n);
		nslot+=item->nslot;
	}
	printf("nslot: %d\n",nslot);
	printf("bytes: %d\n",nslot*l->elem_size);
}
