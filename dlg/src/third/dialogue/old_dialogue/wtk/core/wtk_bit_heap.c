#include "wtk_bit_heap.h"
int wtk_bit_heap_clean(wtk_bit_heap_t* heap);
/**
 * @brief initialize element.
 */
int wtk_bit_heap_init(wtk_bit_heap_t* p,size_t elem_size,size_t elem_min,size_t elem_max,float growf);

void heap_block_reset(HeapBlock* block,size_t elem_num)
{
	int bit_len=(elem_num+7)>>3;

	block->elem_num=elem_num;
	block->elem_free=elem_num;
	block->first_free=0;
	memset(block->bitmap,0,bit_len);
	if(elem_num&7)
	{
		block->bitmap[bit_len-1]|=0xff<<(elem_num&7);
	}
}

HeapBlock* heap_block_new(size_t elem_size,size_t elem_num)
{
	size_t size,bit_len;
	HeapBlock* block;
	uint8_t* p;

	bit_len=(elem_num+7)>>3;
	size=sizeof(HeapBlock)+bit_len;//(elem_num*elem_size);
	block=(HeapBlock*)wtk_calloc(1,size);
	p=(uint8_t*)block;
	block->next=0;
	block->bitmap=p+sizeof(HeapBlock);
	heap_block_reset(block,elem_num);
    //block->data=p+sizeof(HeapBlock)+bit_len;
#ifdef USE_bIT_ALIGN
	bit_len=elem_num*elem_size;
	if(bit_len <4096)
	{
		int ret;

		ret=posix_memalign((void**)&(block->data),4096,4096);
		if(ret!=0)
		{
			block->data=NULL;
		}
	}else
	{
		block->data=wtk_calloc(elem_num,elem_size);
	}
#else
	block->data=wtk_calloc(elem_num,elem_size);
#endif
    return block;
}

int heap_block_bytes(HeapBlock *blk,int elem_size)
{
	int bytes;

	bytes=sizeof(HeapBlock);
	bytes+=blk->elem_num*elem_size;
	bytes+=(blk->elem_num+7)>>3;
	return bytes;
}

void heap_block_delete(HeapBlock* b)
{
	wtk_free(b->data);
	wtk_free(b);
}

HeapBlock* heap_block_reorder(HeapBlock* head,size_t elem_free)
{
	HeapBlock *cur,*prev;

	for(cur=head,prev=0;cur;prev=cur,cur=cur->next)
	{
		if(cur->elem_free>=elem_free)
		{
			if(prev)
			{
				prev->next=cur->next;
				cur->next=head;
			}
			head=cur;
			break;
		}
	}
	return head;
}

uint8_t* heap_block_get_elem(HeapBlock* head,size_t elem_size)
{
	size_t index,bitpos,bit_len,pos,i;
	uint8_t *p;
	uint8_t c;

	if(head->elem_free<=0){p=0;goto end;}
	index=head->first_free;
	bitpos=head->first_free>>3;
	head->bitmap[bitpos]|=1<<(head->first_free&7);
	--head->elem_free;
	p=head->data+index*elem_size;
	if(head->elem_free<=0)
	{
		head->first_free=head->elem_num;
		goto end;
	}
	bit_len=(head->elem_num+7)>>3;
    for(i=bitpos;i<bit_len;++i)
    {
    	if(head->bitmap[i]!=(unsigned)0xFF)
    	{
    		c=~(head->bitmap[i]);
    		pos=7;
    		if(c&0xf){pos-=4;c&=0xf;}
    		if(c&0x33){pos-=2;c&=0x33;}
    		if(c&0x55){pos-=1;}
    		head->first_free=(i<<3)+pos;
    		break;
    	}
    }
end:
	return p;
}

//wtk_bit_heap_t* wtk_bit_heap_new(size_t elem_size,size_t elem_min,size_t elem_max,float growf,heap_clean_handler cleaner)
wtk_bit_heap_t* wtk_bit_heap_new(size_t elem_size,size_t elem_min,size_t elem_max,float growf)
{
	wtk_bit_heap_t* p;

	p=(wtk_bit_heap_t*)wtk_malloc(sizeof(wtk_bit_heap_t));
	wtk_bit_heap_init(p,elem_size,elem_min,elem_max,growf);
	return p;
}

wtk_bit_heap_t* wtk_bit_heap_new2(size_t elem_size)
{
	int n;

	n=4096/elem_size;
	return wtk_bit_heap_new(elem_size,n,n,0);
}

int wtk_bit_heap_init(wtk_bit_heap_t* p,size_t elem_size,size_t elem_min,size_t elem_max,float growf)
{
	p->block_list=0;
	p->elem_size=elem_size;
	p->elem_cur=elem_min;
	p->elem_min=elem_min;
	p->elem_max=elem_max;
	p->tot_alloc=0;
	p->tot_used=0;
	p->growf=growf;
	//p->cleaner=cleaner;
	return 0;
}

int wtk_bit_heap_delete(wtk_bit_heap_t* heap)
{
	wtk_bit_heap_clean(heap);
	wtk_free(heap);
	return 0;
}

int wtk_bit_heap_clean(wtk_bit_heap_t* heap)
{
	HeapBlock *cur,*next;

	for(cur=heap->block_list;cur;cur=next)
	{
		next=cur->next;
		heap_block_delete(cur);
	}
	heap->tot_used=0;
	heap->tot_alloc=0;
	heap->block_list=0;
	heap->elem_cur=heap->elem_min;
	return 0;
}

int wtk_bit_heap_bytes(wtk_bit_heap_t *heap)
{
	int bytes;
	HeapBlock *p;

	bytes=sizeof(wtk_bit_heap_t);
	for(p=heap->block_list;p;p=p->next)
	{
		bytes+=heap_block_bytes(p,heap->elem_size);
	}
	return bytes;
}

int wtk_bit_heap_reset(wtk_bit_heap_t* heap)
{
	wtk_bit_heap_clean(heap);
	return 0;
}

int wtk_bit_heap_add_block(wtk_bit_heap_t* heap)
{
	HeapBlock* block;
	size_t num;

	num=heap->elem_cur*((heap->block_list?heap->growf:0)+1);
	if(num>heap->elem_max)
	{
		num=heap->elem_max;
	}else if(num<heap->elem_min)
	{
		num=heap->elem_min;
	}
	heap->elem_cur=num;
	block=heap_block_new(heap->elem_size,num);
	block->next=heap->block_list;
	heap->block_list=block;
	heap->tot_alloc+=num;
	return 0;
}

void* wtk_bit_heap_malloc(wtk_bit_heap_t* heap)
{
	void* p;
	int ret;

	p=0;
	if(heap->tot_alloc==heap->tot_used)
	{
		ret=wtk_bit_heap_add_block(heap);
		if(ret!=0){goto end;}
	}else if(heap->block_list->elem_free==0)
	{
		heap->block_list=heap_block_reorder(heap->block_list,1);
	}
	p=heap_block_get_elem(heap->block_list,heap->elem_size);
	if(p){++heap->tot_used;}
end:
	if(p==0)
	{
		printf("get null pointer.\n");
	}
	return p;
}

void* wtk_bit_heap_zmalloc(wtk_bit_heap_t *heap)
{
	void *p;

	p=wtk_bit_heap_malloc(heap);
	if(p)
	{
		memset(p,0,heap->elem_size);
	}
	return p;
}

int wtk_bit_heap_free(wtk_bit_heap_t* heap,void* p)
{
	HeapBlock *cur,*pre;
	int found,ret,index;

	for(cur=heap->block_list,pre=0;cur;pre=cur,cur=cur->next)
	{
		found=(cur->data<=(uint8_t*)p) && ((cur->data+(cur->elem_num)*(heap->elem_size))>(uint8_t*)p);
		if(found){break;}
	}
	if(!cur){ret=-1;goto end;}

    index=((uint8_t*)p-cur->data)/heap->elem_size;
    cur->bitmap[index>>3]&=~(1<<(index&7));
    if(index<cur->first_free)
    {
            cur->first_free=index;
    }
    ++cur->elem_free;
    --heap->tot_used;
    ret=0;
    if(cur->elem_free!=cur->elem_num){goto end;}
    if(cur!=heap->block_list)
    {
    	pre->next=cur->next;
    }else
    {
    	heap->block_list=cur->next;
    }
    heap->tot_alloc-=cur->elem_num;
    heap_block_delete(cur);
end:
	return ret;
}

int wtk_bit_heap_is_valid(wtk_bit_heap_t *heap)
{
	HeapBlock *b;
	int f,t;
	int valid;

	f=t=0;
	for(b=heap->block_list;b;b=b->next)
	{
		f+=b->elem_free;
		t+=b->elem_num;
	}
	valid=0;
	if(t!=heap->tot_alloc)
	{
		wtk_debug("heap alloc is not equal: (real=%d,want=%d)\n",t,(int)heap->tot_alloc);
		goto end;
	}
	if(f!=(heap->tot_alloc-heap->tot_used))
	{
		wtk_debug("heap free is not equal: (real=%d,want=%d)\n",f,(int)(heap->tot_alloc-heap->tot_used));
		goto end;
	}
	valid=1;
end:
	return valid;
}

void wtk_bit_heap_print(wtk_bit_heap_t *heap)
{
	printf("=============== bit heap ============\n");
	printf("alloc: %d\n",(int)(heap->tot_alloc));
	printf("used: %d\n",(int)(heap->tot_used));
}

void wtk_bit_heap_test_g()
{
	typedef struct{
		int i;
		float f;
	}wtk_foo_t;
	wtk_bit_heap_t *heap;
	wtk_foo_t *f;

	//create bit heap;
	heap=wtk_bit_heap_new(sizeof(wtk_foo_t),100,4096/sizeof(wtk_foo_t),1);
	//malloc element;
	f=wtk_bit_heap_malloc(heap);
	f->i=0;f->f=10;
	//free element;
	wtk_bit_heap_free(heap,f);
	//dlete bit heap;
	wtk_bit_heap_delete(heap);
}
