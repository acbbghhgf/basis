#include "wtk_stack.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include "wtk/os/wtk_fd.h"

stack_block_t* wtk_stack_new_block(wtk_stack_t *stack,int want_size)
{
	stack_block_t *b;
	int size;
    int v;
    int want;

    want=want_size+sizeof(stack_block_t);
    v=(int)(stack->last_alloc_size*(stack->growf+1));
	size=max(v,want);
	size=wtk_round(size,8);
	stack->last_alloc_size=size;
	b=(stack_block_t*)wtk_malloc(size);
	b->next=0;
	b->pop=b->push=(char*)b + sizeof(stack_block_t);
	b->end=(char*)b+size;
	return b;
}


wtk_stack_t* wtk_stack_new(int init_size,int max_size,float growf)
{
	wtk_stack_t* s;

	s=(wtk_stack_t*)wtk_malloc(sizeof(wtk_stack_t));
	wtk_stack_init(s,init_size,max_size,growf);
	return s;
}

int wtk_stack_bytes(wtk_stack_t *s)
{
	int b;
	stack_block_t* bl;

	b=sizeof(*s);
    for(bl=s->pop;bl;bl=bl->next)
    {
    	b+=sizeof(stack_block_t)+bl->end-bl->pop;
    }
	return b;
}

int wtk_stack_init(wtk_stack_t *s,int init_size,int max_size,float growf)
{
	stack_block_t *b;

	s->last_alloc_size=0;
	s->max_size=max_size;
	s->len=0;
	s->growf=growf;
	b=wtk_stack_new_block(s,init_size);
	s->pop=s->push=s->end=b;
	return 0;
}

int wtk_stack_delete(wtk_stack_t* stack)
{
	wtk_stack_clean(stack);
	wtk_free(stack);
	return 0;
}

int wtk_stack_clean(wtk_stack_t *s)
{
	stack_block_t *b,*p;

	for(b=s->pop;b;b=p)
	{
		p=b->next;
		wtk_free(b);
	}
	return 0;
}

int wtk_stack_reset(wtk_stack_t *s)
{
	stack_block_t *b;

	s->push=s->pop;
	for(b=s->pop;b;b=b->next)
	{
		b->pop=b->push=(char*)b+sizeof(stack_block_t);
	}
	s->len=0;
	return 0;
}

int wtk_stack_push(wtk_stack_t* s,const char* data,int len)
{
	stack_block_t *b;
	int cpy,left,size;

	left=len;
	for(b=s->push;b;b=b->next,s->push=b)
	{
		size=b->end - b->push;
		if(size>0)
		{
			cpy=min(size,left);
			memcpy(b->push,data,cpy);
			b->push+=cpy;
			left-=cpy;
			if(left==0){break;}
			data+=cpy;
		}
		if(!b->next)
		{
			s->end=b->next=wtk_stack_new_block(s,left);
		}
	}
	s->len += len-left;
	return 0;
}

void wtk_stack_push_f(wtk_stack_t *s,const char *fmt,...)
{
	char buf[4096];
	va_list ap;
	int n;

	va_start(ap,fmt);
	n=vsprintf(buf,fmt,ap);
	wtk_stack_push(s,buf,n);
	va_end(ap);
}

int wtk_stack_pop2(wtk_stack_t* s,char* data,int len)
{
	stack_block_t *b;
	int left,size,cpy;
	int cpyed;

	left=min(len,s->len);
	cpyed=0;
	for(b=s->pop;b && left>0;s->pop=b->next,b->next=0,b=s->pop)
	{
		size=b->push-b->pop;
		if(size > 0)
		{
			cpy=min(size,left);
			if(data)
			{
				memcpy(data,b->pop,cpy);
				data += cpy;
			}
			b->pop += cpy;
			left -= cpy;
			cpyed+=cpy;
			if(left==0){break;}
		}
		b->push=b->pop=(char*)b+sizeof(*b);
		if(s->push==b)
		{
			break;
		}else
		{
			s->end->next=b;
			s->end=b;
		}
	}
	s->len -=cpyed;
	return cpyed;
}

int wtk_stack_pop3(wtk_stack_t* s,char* data,int len)
{
	stack_block_t *b;
	int left,size,cpy;

	left=len;
	for(b=s->pop;b && left>0;s->pop=b->next,b->next=0,b=s->pop)
	{
		size=b->push-b->pop;
		if(size > 0)
		{
			cpy=min(size,left);
			if(data)
			{
				memcpy(data,b->pop,cpy);
				data += cpy;
			}
			b->pop += cpy;
			left -= cpy;
			if(left==0){break;}
		}
		b->push=b->pop=(char*)b+sizeof(*b);
		s->end->next=b;
		s->end=b;
	}
	s->len -=len-left;
	return 0;
}

int wtk_stack_pop(wtk_stack_t* s,char* data,int len)
{
	int cpyed;
	int ret;

	cpyed=wtk_stack_pop2(s,data,len);
	ret=cpyed==len?0:-1;
	return ret;
}

int wtk_stack_push_string(wtk_stack_t* s,const char* str)
{
	return wtk_stack_push(s,str,strlen(str));
}

void wtk_stack_merge(wtk_stack_t *s,char* p)
{
	stack_block_t *b;
	int len;

	for(b=s->pop;b;b=b->next)
	{
		len=b->push-b->pop;
		memcpy(p,b->pop,len);
		p+=len;
		if(b==s->push){return;}
	}
}

void wtk_stack_add(wtk_stack_t *dst,wtk_stack_t *src)
{
    stack_block_t *b;
    int len;

    for(b=src->pop;b;b=b->next)
    {
        len=b->push-b->pop;
        wtk_stack_push(dst,b->pop,len);
        if(b==src->push){return;}
    }
}

void wtk_stack_copy(wtk_stack_t *s,wtk_strbuf_t *to)
{
    stack_block_t *b;
    int len;

    for(b=s->pop;b;b=b->next)
    {
        len=b->push-b->pop;
        wtk_strbuf_push(to,b->pop,len);
        if(b==s->push){return;}
    }
}

int wtk_stack_is_valid(wtk_stack_t * s)
{
	stack_block_t* b;
	int count,ret;

	count=0;
	for(b=s->pop;b;b=b->next)
	{
		ret=b->push - b->pop;
		//wtk_debug("stack check: push=%p,pop=%p\n",b->push,b->pop);
		if(ret<0)
		{
			wtk_debug("stack push is low than pop: push=%p,pop=%p\n",b->push,b->pop);
			goto end;
		}
		count += ret;
	}
	ret= count == s->len ? 1 : 0;
end:
	return ret;
}

int wtk_stack_print(wtk_stack_t* s)
{
	stack_block_t* b;
	int i;

    for(b=s->pop,i=0;b;b=b->next,++i)
    {
        print_data((char*)b->pop,b->push-b->pop);
    }
    return 0;
	printf("################ stack (%p) #############\n",s);
	if(s)
	{
		for(b=s->pop,i=0;b;b=b->next,++i)
		{
			printf("stack(%d,%p -> %p):\t(valid=%d,len=%d)\n",i,b,b->next,(int)(b->pop - b->push),
				(int)(b->end - (char*)b-sizeof(*b)));
			print_data((char*)b->pop,b->push-b->pop);
		}
		printf("pop:%p,push:%p\n",s->pop,s->push);
		printf("len: %d\n",s->len);
	}else
	{
		printf("nil.\n");
	}
	printf("####################################\n");
	return 0;
}

int wtk_stack_write(wtk_stack_t *s,FILE *f)
{
	stack_block_t* b;
	int i,n;
	int ret;

    for(b=s->pop,i=0;b;b=b->next,++i)
    {
    	n=b->push-b->pop;
    	if(n<=0){continue;}
    	ret=fwrite((char*)b->pop,1,n,f);
    	if(ret!=n){ret=-1;goto end;}
    }
    ret=0;
end:
	return ret;
}

int wtk_stack_write2(wtk_stack_t *s,char *fn)
{
	FILE *f;
	int ret;

	f=fopen(fn,"wb");
	if(f)
	{
		ret=wtk_stack_write(s,f);
		fclose(f);
	}else
	{
		ret=-1;
	}
	return ret;
}

int wtk_stack_read(wtk_stack_t *s,char *fn)
{
	char buf[4096];
	int ret=-1;
	FILE *f=0;

	f=fopen(fn,"rb");
	if(!f){goto end;}
	while(1)
	{
		ret=fread(buf,1,sizeof(buf),f);
		if(ret>0)
		{
			wtk_stack_push(s,buf,ret);
		}
		if(ret<sizeof(buf))
		{
			break;
		}
	}
end:
	if(f)
	{
		fclose(f);
	}
	return ret;
}


int wtk_stack_write_nbfd(wtk_stack_t *s, int fd) {
	stack_block_t *b;
	int n, writed;
    int ret;

	for(b = s->pop; b ; s->pop = b->next, b->next = 0, b = s->pop) {
		n = b->push - b->pop;
		if (n > 0) {
            ret = wtk_fd_write_nonblock(fd, (char*)b->pop, n, &writed);
            b->pop += writed;
            s->len -= writed;
            if (WTK_EOF == ret || WTK_ERR == ret) {
                ret = -1;
                goto end;
            }
            if (writed != n) {
                break;
            }
		}
		b->push = b->pop = (char*)b + sizeof(*b);
		if (s->push == b){
			break;
		} else {
            s->end->next = b;
            s->end = b;
		}
	}
    ret = 0;
end:
	return ret;
}
