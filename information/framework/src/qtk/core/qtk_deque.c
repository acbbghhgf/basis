#include "qtk_deque.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define BLOCK_START(b) ((char*)(b)+sizeof(deque_block_t))

typedef struct deque_block deque_block_t;

struct deque_block
{
    deque_block_t* next;
    char *pop;
    char *push;
    char *end;
};

deque_block_t* qtk_deque_new_block(qtk_deque_t *stack,int want_size)
{
    deque_block_t *b;
    int size;
    int v;
    int want;

    want=want_size+sizeof(deque_block_t);
    v=(int)(stack->last_alloc_size*(stack->growf+1));
    size=max(v,want);
    size=wtk_round(size,8);
    stack->last_alloc_size=size;
    b=(deque_block_t*)wtk_malloc(size);
    b->next=0;
    b->pop=b->push=BLOCK_START(b);
    b->end=(char*)b+size;
    return b;
}


deque_block_t* qtk_deque_clone_block(deque_block_t *cpy)
{
    deque_block_t *b;
    int size;

    size = cpy->end - (char*)cpy;
    b = (deque_block_t*)wtk_malloc(size);
    b->next = NULL;
    b->pop = (char*)b + (cpy->pop-(char*)cpy);
    b->push = (char*)b + (cpy->push-(char*)cpy);
    b->end = (char*)b + size;
    memcpy(b->pop, cpy->pop, b->push - b->pop);
    return b;
}


qtk_deque_t* qtk_deque_new(int init_size,int max_size,float growf)
{
    qtk_deque_t* s;

    s=(qtk_deque_t*)wtk_malloc(sizeof(qtk_deque_t));
    qtk_deque_init(s,init_size,max_size,growf);
    return s;
}


int qtk_deque_bytes(qtk_deque_t *s)
{
    int b;
    deque_block_t* bl;

    b=sizeof(*s);
    for(bl=s->pop;bl;bl=bl->next)
    {
        b+=sizeof(deque_block_t)+bl->end-bl->pop;
    }
    return b;
}


int qtk_deque_clone(qtk_deque_t *dst, qtk_deque_t* src) {
    deque_block_t *b;

    *dst = *src;
    b = NULL;
    b = src->push;
    dst->pop = dst->push = qtk_deque_clone_block(b);
    while ((b = b->next)) {
        dst->pop = dst->pop->next = qtk_deque_clone_block(b);
    }
    dst->end = dst->pop;
    return 0;
}


int qtk_deque_init(qtk_deque_t *s,int init_size,int max_size,float growf)
{
    deque_block_t *b;

    s->last_alloc_size=0;
    s->max_size=max_size;
    s->len=0;
    s->growf=growf;
    b=qtk_deque_new_block(s,init_size);
    s->pop=s->push=s->end=b;
    return 0;
}


int qtk_deque_delete(qtk_deque_t* stack)
{
    qtk_deque_clean(stack);
    wtk_free(stack);
    return 0;
}


int qtk_deque_clean(qtk_deque_t *s)
{
    deque_block_t *b,*p;

    for(b=s->pop;b;b=p)
    {
        p=b->next;
        wtk_free(b);
    }
    return 0;
}


int qtk_deque_reset(qtk_deque_t *s)
{
    deque_block_t *b;

    s->push=s->pop;
    for(b=s->pop;b;b=b->next)
    {
        b->pop=b->push = BLOCK_START(b);
    }
    s->len=0;
    return 0;
}


int qtk_deque_push(qtk_deque_t* s,const char* data,int len)
{
    deque_block_t *b;
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
            s->end=b->next=qtk_deque_new_block(s,left);
        }
    }
    s->len += len-left;
    return 0;
}


int qtk_deque_push_front(qtk_deque_t* s, const char* data, size_t len) {
    deque_block_t *b;
    int cpy, left, size;
    const char *end = data + len;
    left = len;
    for(b = s->pop; b; s->pop = b) {
        size = b->pop - BLOCK_START(b);
        if (size > 0) {
            cpy = min(size, left);
            b->pop -= cpy;
            end -= cpy;
            left -= cpy;
            memcpy(b->pop, end, cpy);
            if (left == 0) { break; }
        }
        if (s->push->next) {
            deque_block_t *tmp = s->push->next;
            s->push->next = tmp->next;
            if (tmp == s->end) {
                s->end = s->push;
            }
            b = tmp;
        } else {
            b = qtk_deque_new_block(s, left);
        }
        b->push = b->pop = b->end;
        b->next = s->pop;
    }
    s->len += len-left;
    return 0;
}


void qtk_deque_push_f(qtk_deque_t *s,const char *fmt,...)
{
    char buf[4096];
    va_list ap;
    int n;

    va_start(ap,fmt);
    n=vsprintf(buf,fmt,ap);
    qtk_deque_push(s,buf,n);
    va_end(ap);
}


int qtk_deque_pop2(qtk_deque_t* s,char* data,int len)
{
    deque_block_t *b;
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
        b->push=b->pop = BLOCK_START(b);
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


int qtk_deque_pop(qtk_deque_t* s,char* data,int len)
{
    int cpyed;
    int ret;

    cpyed=qtk_deque_pop2(s,data,len);
    ret=cpyed==len?0:-1;
    return ret;
}

int qtk_deque_push_string(qtk_deque_t* s,const char* str)
{
    return qtk_deque_push(s,str,strlen(str));
}

void qtk_deque_merge(qtk_deque_t *s,char* p)
{
    deque_block_t *b;
    int len;

    for(b=s->pop;b;b=b->next)
    {
        len=b->push-b->pop;
        memcpy(p,b->pop,len);
        p+=len;
        if(b==s->push){return;}
    }
}

void qtk_deque_add(qtk_deque_t *dst,qtk_deque_t *src)
{
    deque_block_t *b;
    int len;

    for(b=src->pop;b;b=b->next)
    {
        len=b->push-b->pop;
        qtk_deque_push(dst,b->pop,len);
        if(b==src->push){return;}
    }
}

void qtk_deque_copy(qtk_deque_t *s,wtk_strbuf_t *to)
{
    deque_block_t *b;
    int len;

    for(b=s->pop;b;b=b->next)
    {
        len=b->push-b->pop;
        wtk_strbuf_push(to,b->pop,len);
        if(b==s->push){return;}
    }
}

int qtk_deque_is_valid(qtk_deque_t * s)
{
    deque_block_t* b;
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


int qtk_deque_write(qtk_deque_t *s,FILE *f)
{
    deque_block_t* b;
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


int qtk_deque_write2(qtk_deque_t *s,char *fn)
{
    FILE *f;
    int ret;

    f=fopen(fn,"wb");
    if(f)
    {
        ret=qtk_deque_write(s,f);
        fclose(f);
    }else
    {
        ret=-1;
    }
    return ret;
}


int qtk_deque_read(qtk_deque_t *s,char *fn)
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
            qtk_deque_push(s,buf,ret);
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


int qtk_deque_move(qtk_deque_t *dst, qtk_deque_t *src, int len) {
    deque_block_t *b;
    int moved = 0;

    for(b = src->pop; b && moved < len;b=b->next)
    {
        int n = min(b->push-b->pop, len-moved);
        qtk_deque_push(dst, b->pop, n);
        moved += n;
        if (b == src->push) break;
    }
    qtk_deque_pop(src, NULL, moved);

    return moved;
}


int qtk_deque_front(qtk_deque_t *dq, char *data, int len) {
    deque_block_t *b;
    int left,size,cpy;
    int cpyed;
    assert(data);

    left = min(len,dq->len);
    cpyed = 0;
    for(b=dq->pop;b && left>0;b = b->next)
    {
        size = b->push - b->pop;
        if(size > 0)
        {
            cpy=min(size, left);
            memcpy(data, b->pop, cpy);
            data += cpy;
            left -= cpy;
            cpyed += cpy;
            if (left == 0) {break;}
        }
        if(dq->push==b)
        {
            break;
        }
    }
    return cpyed;
}


int qtk_deque_tail(qtk_deque_t *dq, char *data, int len) {
    deque_block_t *b;
    int left, size, cpy, move;
    int cpyed = 0;
    assert(data);

    left = min(len, dq->len);
    move = dq->len - left;
    for (b = dq->pop; b; b = b->next) {
        size = b->push - b->pop;
        if (size > move) {
            cpyed = min(size-move, left);
            memcpy(data, b->pop+move, cpyed);
            data += cpyed;
            left -= cpyed;
            b = b->next;
            break;
        }
        move -= size;
    }
    for (; b && left > 0; b = b->next) {
        size = b->push - b->pop;
        if(size > 0)
        {
            cpy=min(size, left);
            memcpy(data, b->pop, cpy);
            data += cpy;
            left -= cpy;
            cpyed += cpy;
            if (left == 0) {break;}
        }
        if(dq->push==b)
        {
            break;
        }
    }
    return cpyed;
}
