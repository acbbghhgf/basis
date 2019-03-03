#include "wtk_heap.h"
#include "wtk_array.h"
#include "wtk_str_encode.h"
#define WTK_HEAP_TRY_CNT 3

wtk_heap_block_t* wtk_heap_block_new(int page_size)
{
	wtk_heap_block_t *blk;
	char *p;

	p=(char*)wtk_malloc(page_size);
	blk=(wtk_heap_block_t*)p;
	blk->first=p+sizeof(wtk_heap_block_t);
	blk->end=p+page_size;
	blk->cur=blk->first;
	blk->failed=0;
	return blk;
}

void wtk_heap_block_reset(wtk_heap_block_t *blk)
{
	blk->cur=blk->first;
	blk->failed=0;
}

void wtk_heap_block_delete(wtk_heap_block_t *blk)
{
	wtk_free(blk);
}

wtk_heap_t* wtk_heap_new(int page_size)
{
	return wtk_heap_new2(page_size,16);
}

wtk_heap_t* wtk_heap_new2(int page_size,int align_size)
{
	wtk_heap_t *heap;

	heap=(wtk_heap_t*)wtk_malloc(sizeof(wtk_heap_t));
	wtk_queue2_init(&(heap->block_q));
	wtk_queue2_init(&(heap->large_q));
	heap->align=align_size;
	heap->page_size=page_size;
	heap->alloc_size=heap->page_size-sizeof(wtk_heap_block_t);
	heap->large_thresh=page_size-sizeof(wtk_heap_block_t);
	heap->cur=wtk_heap_block_new(heap->page_size);
	wtk_queue2_push(&(heap->block_q),&(heap->cur->q_n));
	return heap;
}

void wtk_heap_delete(wtk_heap_t *heap)
{
	wtk_heap_reset(heap);
	wtk_heap_block_delete(heap->cur);
	wtk_free(heap);
}

void wtk_heap_reset_large(wtk_heap_t *heap)
{
	wtk_queue_node_t *qn;
	wtk_heap_large_t *l;

	while(1)
	{
		qn=wtk_queue2_pop(&(heap->large_q));
		if(!qn){break;}
		l=data_offset2(qn,wtk_heap_large_t,q_n);
		if(l->data)
		{
			wtk_free(l->data);
		}
	}
}

void wtk_heap_reset(wtk_heap_t *heap)
{
	wtk_queue_node_t *qn;
	wtk_heap_block_t *blk;

	wtk_heap_reset_large(heap);
	wtk_queue2_remove(&(heap->block_q),&(heap->cur->q_n));
	while(1)
	{
		qn=wtk_queue2_pop(&(heap->block_q));
		if(!qn){break;}
		blk=data_offset2(qn,wtk_heap_block_t,q_n);
		wtk_heap_block_delete(blk);
	}
	wtk_heap_block_reset(heap->cur);
	wtk_queue2_push(&(heap->block_q),&(heap->cur->q_n));
}

char* wtk_heap_malloc_large(wtk_heap_t *heap,int bytes)
{
	wtk_heap_large_t *l;
	char *p;

	p=(char*)wtk_malloc(bytes);
	if(!p){return NULL;}
	l=(wtk_heap_large_t*)wtk_heap_malloc(heap,sizeof(wtk_heap_large_t));
	l->data=p;
	l->size=bytes;
	wtk_queue2_push(&(heap->large_q),&(l->q_n));
	return p;
}

void wtk_heap_add_large(wtk_heap_t *heap,char *p,int size)
{
	wtk_heap_large_t* l;

	l=(wtk_heap_large_t*)wtk_heap_malloc(heap,sizeof(wtk_heap_large_t));
	l->data=p;
	l->size=size;
	wtk_queue2_push(&(heap->large_q),&(l->q_n));
}



char* wtk_heap_malloc_block(wtk_heap_t *heap,int bytes)
{
	wtk_heap_block_t *blk;
	wtk_queue_node_t *qn;
	char *p;

	blk=wtk_heap_block_new(heap->page_size);
	wtk_queue2_push(&(heap->block_q),&(blk->q_n));
	p=wtk_align_ptr(blk->cur,heap->align);
	if(p+bytes>blk->end)
	{
		p=wtk_heap_malloc_large(heap,bytes);
	}else
	{
		blk->cur=p+bytes;
	}
	if(heap->cur->failed>=WTK_HEAP_TRY_CNT)
	{
		blk=heap->cur;
		heap->cur=NULL;
		for(qn=blk->q_n.next;qn;qn=qn->next)
		{
			blk=data_offset2(qn,wtk_heap_block_t,q_n);
			if((blk->failed<WTK_HEAP_TRY_CNT) && (blk->end>blk->cur))
			{
				heap->cur=blk;
				break;
			}
		}
		if(!heap->cur)
		{
			heap->cur=data_offset2(heap->block_q.pop,wtk_heap_block_t,q_n);
		}
	}
	return p;
}

#ifdef USE_HEAP
void* wtk_heap_malloc(wtk_heap_t *heap,int bytes)
{
	return malloc(bytes);
}
#else

void* wtk_heap_malloc(wtk_heap_t *heap,int bytes)
{
	wtk_heap_block_t *blk;
	char *p;
	int align;

	if(bytes<=0)
	{
		return NULL;
	}else if(bytes>heap->large_thresh)
	{
		return (void*)wtk_heap_malloc_large(heap,bytes);
	}
	align=heap->align;
	blk=heap->cur;
	do
	{
		p=align>1?wtk_align_ptr(blk->cur,align):blk->cur;
		if((blk->end-p)>=bytes)
		{
			blk->cur=p+bytes;
			return p;
		}
		if(blk->q_n.next)
		{
			++blk->failed;
			if(blk->failed>=WTK_HEAP_TRY_CNT)
			{
				blk=data_offset2(blk->q_n.next,wtk_heap_block_t,q_n);
				heap->cur=blk;
			}else
			{
				blk=data_offset2(blk->q_n.next,wtk_heap_block_t,q_n);
			}
		}else
		{
			break;
		}
	}while(blk);
	return (void*)wtk_heap_malloc_block(heap,bytes);
}
#endif

void wtk_heap_free(wtk_heap_t *heap,void *p)
{
	wtk_queue2_t *q=&(heap->block_q);
	wtk_queue_node_t *qn;
	wtk_heap_block_t *blk;
	char *pc=(char*)p;

	while(1)
	{
		qn=wtk_queue2_pop(q);
		if(!qn){break;}
		blk=data_offset2(qn,wtk_heap_block_t,q_n);
		if(blk==heap->cur)
		{
			heap->cur=NULL;
		}
		if(pc>=blk->first && pc <=blk->cur)
		{
			blk->cur=p;
			wtk_queue2_push(q,&(blk->q_n));
			break;
		}else
		{
			wtk_heap_block_delete(blk);
		}
	}
	if(!heap->cur)
	{
		if(q->pop)
		{
			heap->cur=data_offset2(q->push,wtk_heap_block_t,q_n);
		}else
		{
			heap->cur=wtk_heap_block_new(heap->page_size);
			wtk_queue2_push(q,&(heap->cur->q_n));
		}
	}
}

void wtk_heap_free_large(wtk_heap_t *heap,void *p)
{
	wtk_queue_node_t *qn;
	wtk_heap_large_t *l;

	for(qn=heap->large_q.pop;qn;qn=qn->next)
	{
		l=data_offset2(qn,wtk_heap_large_t,q_n);
		if(l->data==p)
		{
			wtk_queue2_remove(&(heap->large_q),qn);
			wtk_free(l->data);
			return;
		}
	}
}


wtk_string_t* wtk_heap_dup_string(wtk_heap_t *h,char *s,int sl)
{
	wtk_string_t *str;

	str=(wtk_string_t*)wtk_heap_malloc(h,sizeof(*str)+sl);
	str->len=sl;
	str->data=(char*)str+sizeof(*str);
	if(s)
	{
		memcpy(str->data,s,sl);
	}
	return str;
}

char* wtk_heap_dup_data(wtk_heap_t *h,const char *s,int l)
{
	char *data;

	data=(char*)wtk_heap_malloc(h,l);
	if(!data){goto end;}
	memcpy(data,s,l);
end:
	return data;
}

wtk_string_t* wtk_heap_dup_string2(wtk_heap_t *h,char *s,int sl)
{
	wtk_string_t *str;

	str=(wtk_string_t*)wtk_heap_malloc(h,sizeof(*str)+sl+1);
	str->len=sl;
	str->data=(char*)str+sizeof(*str);
	if(s)
	{
		memcpy(str->data,s,sl);
	}
	str->data[sl]=0;
	return str;
}

void* wtk_heap_zalloc(wtk_heap_t* heap,size_t size)
{
	void *p;

	p=wtk_heap_malloc(heap,size);
	if(p)
	{
		memset(p,0,size);
	}
	return p;
}

char *wtk_heap_dup_str(wtk_heap_t *heap,char* s)
{
    return wtk_heap_dup_str2(heap,s,s?strlen(s):0);
}

char* wtk_heap_dup_str2(wtk_heap_t *heap,char *data,int len)
{
	char *d=0;

	if(len<=0){goto end;}
	d=(char*)wtk_heap_malloc(heap,len+1);
	memcpy(d,data,len);
	d[len]=0;
end:
	return d;
}

void wtk_heap_fill_string(wtk_heap_t *heap,wtk_string_t *str,char *data,int bytes)
{
	str->data=wtk_heap_dup_data(heap,data,bytes);
	str->len=bytes;
}

int wtk_heap_bytes(wtk_heap_t *heap)
{
	wtk_heap_large_t* l;
	wtk_queue_node_t *qn;
	int size;

	size=0;
	for(qn=heap->large_q.pop;qn;qn=qn->next)
	{
		l=data_offset2(qn,wtk_heap_large_t,q_n);
		size+=l->size;
	}
	for(qn=heap->block_q.pop;qn;qn=qn->next)
	{
		//p=data_offset2(qn,wtk_heap_block_t,q_n);
		size+=heap->page_size;
	}
	return size;
}


void wtk_heap_print(wtk_heap_t *heap)
{
	wtk_heap_large_t* l;
	wtk_heap_block_t *p;
	wtk_queue_node_t *qn;
	int count,size;

	printf("########## Heap #############\n");
	size=0;count=0;
	for(qn=heap->large_q.pop;qn;qn=qn->next)
	{
		l=data_offset2(qn,wtk_heap_large_t,q_n);
		size+=l->size;
		++count;
	}
	printf("large list:\t%d\n",count);
	printf("large bytes:\t%d\n",size);
	size=0;count=0;
	for(qn=heap->block_q.pop;qn;qn=qn->next)
	{
		p=data_offset2(qn,wtk_heap_block_t,q_n);
		++count;
		size+=p->cur-p->first;
	}
	printf("block list:\t%d\n",count);
	printf("block bytes:\t%d\n",size);
}

wtk_array_t* wtk_utf8_string_to_chars(wtk_heap_t *heap,char *data,int bytes)
{
	char *s,*e;
	int num;
	wtk_array_t *a;
	wtk_string_t *v;

	a=wtk_array_new_h(heap,bytes,sizeof(wtk_string_t*));
	s=data;e=s+bytes;
	while(s<e)
	{
		num=wtk_utf8_bytes(*s);
		s+=num;
		if(s>e){break;}
		//wtk_debug("[%.*s]\n",num,s-num);
		v=wtk_heap_dup_string(heap,s-num,num);
		*((wtk_string_t**)(wtk_array_push(a)))=v;
	}
	return a;
}
