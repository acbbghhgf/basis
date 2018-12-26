#include "wtk_hoard.h"

int wtk_hoard_init(wtk_hoard_t *h,int offset,int max_free,wtk_new_handler_t newer,
		wtk_delete_handler_t deleter,void *data)
{
	h->free=h->use=0;
	h->offset=offset;
	h->max_free=max_free;
	h->cur_free=0;
	h->newer=newer;
	h->deleter=deleter;
	h->deleter2=0;
	h->user_data=data;
    h->use_length=0;
	return 0;
}

int wtk_hoard_init2(wtk_hoard_t *h,int offset,int max_free,wtk_new_handler_t newer,
		wtk_delete_handler2_t deleter,void *data)
{
	h->free=h->use=0;
	h->offset=offset;
	h->max_free=max_free;
	h->cur_free=0;
	h->newer=newer;
	h->deleter=0;
	h->deleter2=deleter;
	h->user_data=data;
    h->use_length=0;
	return 0;
}

void wtk_hoard_reset(wtk_hoard_t *h)
{
	h->free=h->use=0;
	h->cur_free=0;
	h->use_length=0;
}

void wtk_hoard_reuse(wtk_hoard_t *h)
{
	wtk_queue_node_t *n,*n2;

	for(n=h->use;n;n=n2)
	{
#ifdef _MSC_VER
		void *p;
		n2=n->prev;
		p=(unsigned)n-(unsigned)(h->offset);
		wtk_hoard_push(h,p);
#else
		// cl.exe with vs2005 doesn't support this expression
		n2=n->prev;
		wtk_hoard_push(h,((void*)(char*)n-h->offset));
#endif
	}
	wtk_hoard_pack(h);
}


int wtk_hoard_queue_clean(wtk_hoard_t *h,wtk_queue_node_t* q)
{
	wtk_queue_node_t *t;
	int offset;

	if(h->deleter)
	{
		offset=h->offset;
		for(;q;q=t)
		{
			t=q->prev;
			h->deleter((void*)((char*)q-offset));
		}
	}else if(h->deleter2)
	{
		offset=h->offset;
		for(;q;q=t)
		{
			t=q->prev;
			h->deleter2(h->user_data,(void*)((char*)q-offset));
		}
	}
	return 0;
}

int wtk_hoard_clean(wtk_hoard_t *h)
{
    if(h)
    {
	    wtk_hoard_queue_clean(h,h->free);
	    wtk_hoard_queue_clean(h,h->use);
	    h->free=h->use=0;
	    h->use_length=0;
    }
	return 0;
}

void* wtk_hoard_pop(wtk_hoard_t *h)
{
	void* data;
	wtk_queue_node_t *q;

	if(h->free)
	{
		q=h->free;
		h->free=q->prev;
		if(h->free)
		{
			h->free->next=0;
		}
		data=(void*)((char*)q-h->offset);
		--h->cur_free;
	}else
	{
		if(!h->newer){return 0;}
		data=h->newer(h->user_data);
        if(!data){return 0;}
		q=(wtk_queue_node_t *)((char*)data+h->offset);
	}
	q->prev=h->use;
	q->next=0;
	if(h->use)
	{
		h->use->next=q;
		h->use=q;
	}else
	{
		h->use=q;
	}
    ++h->use_length;
	return data;
}

int wtk_hoard_push(wtk_hoard_t *h,void* data)
{
	wtk_queue_node_t *q;

	q=(wtk_queue_node_t *)((char*)data+h->offset);
	//unlink from use.
	if(q->prev)
	{
		q->prev->next=q->next;
	}
	if(q->next)
	{
		q->next->prev=q->prev;
	}else
	{
		if(q == h->use)
		{
			h->use=q->prev;
		}
	}
	if(h->cur_free < h->max_free)
	{
		//link to free.
		q->prev=h->free;
		if(h->free)
		{
			h->free->next=q;
		}
		h->free=q;
		q->next=0;
		++h->cur_free;
	}else
	{
		if(h->deleter)
		{
			h->deleter(data);
		}else if(h->deleter2)
		{
			h->deleter2(h->user_data,data);
		}
	}
    --h->use_length;
	return 0;
}

int wtk_hoard_queue_bytes(wtk_hoard_t *h,wtk_hoard_bytes_f bf,wtk_queue_node_t *n)
{
	void *data;
	int b=0;

	for(;n;n=n->prev)
	{
		data=(void*)((char*)n-h->offset);
		b+=bf(data);
	}
	return b;
}

int wtk_hoard_bytes(wtk_hoard_t *h,wtk_hoard_bytes_f bf)
{
	int b;

	b=wtk_hoard_queue_bytes(h,bf,h->free);
	b+=wtk_hoard_queue_bytes(h,bf,h->use);
	return b;
}

void wtk_hoard_pack(wtk_hoard_t *h)
{
	wtk_queue_node_t *qn;
	int cnt=h->cur_free-h->max_free;
	int i;
	char *data;

	if(cnt<=0)
	{
		return;
	}
	for(i=0;i<cnt;++i)
	{
		qn=h->free;
		h->free=qn->prev;
		h->free->next=0;
		data=(char*)qn-h->offset;
		if(h->deleter)
		{
			h->deleter(data);
		}else if(h->deleter2)
		{
			h->deleter2(h->user_data,data);
		}
	}
	h->cur_free=h->max_free;
}

//================================= Test section =================
typedef struct{
	char *s;
	wtk_queue_node_t q_n;
}wtk_foo_t;

static wtk_foo_t* wtk_hoard_test_new_foo(void *app_data)
{
	wtk_foo_t *f;

	f=(wtk_foo_t*)wtk_malloc(sizeof(*f));
	f->s=(char*)wtk_malloc(1024);
	return f;
}

static int wtk_foo_delete(wtk_foo_t *f)
{
	wtk_free(f->s);
	wtk_free(f);
	return 0;
}

void wtk_hoard_test_g()
{
	wtk_hoard_t hoard;
	wtk_foo_t *f;

	wtk_hoard_init(&(hoard),offsetof(wtk_foo_t,q_n),100,
			(wtk_new_handler_t)wtk_hoard_test_new_foo,
			(wtk_delete_handler_t)wtk_foo_delete,0);
	f=wtk_hoard_pop(&(hoard));
	wtk_debug("f->s=%p\n",f->s);
	wtk_hoard_push(&(hoard),f);
	wtk_hoard_clean(&(hoard));
}
