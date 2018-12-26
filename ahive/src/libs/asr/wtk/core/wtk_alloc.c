#include "wtk_alloc.h"

//#define USE_ALIGN

void* wtk_memalign(size_t alignment,size_t size)
{
#ifdef USE_ALIGN
    void  *p;
    int    err;

    err = posix_memalign(&p, alignment, size);
    if(err!=0)
    {
    	p=0;
    }
    return p;
#else
	return wtk_calloc(1,size);
#endif
}


#ifndef MEM_INLINE
void wtk_free(void* p)
{
	printf("free: %p\n",p);
	free(p);
}

void* wtk_malloc(size_t n)
{
	void* p;
	p=malloc(n);
    if(n==8|| n==40)
    {
	    printf("malloc: %p\n",p);
    }
	return p;
}

void* wtk_calloc(int elems,int size)
{
    void *p;

    p=calloc(elems,size);
    if(size==8|| size==40)
    {
        printf("calloc: %p\n",p);
    }
    return p;
}
#endif

#ifdef DEBUG_MEM
#include "wtk/os/wtk_lock.h"
wtk_lock_t lock;
pid_t pid;

void wtk_alloc_init()
{
	static int i=0;

    if(i==0)
    {
    	pid=getpid();
    	wtk_lock_init(&lock);
    	i=1;
    }
}

void* wtk_calloc_debug(int elems,int size,const char *f,int line)
{
    void *p;

    wtk_alloc_init();
    p=calloc(elems,size);
    //make page dirty
    memset(p,0,elems*size);
    if(pid==getpid())
    {
    	wtk_lock_lock(&(lock));
    	printf("DM malloc %s:%d %p %d\n",f,line,p,elems*size);
    	fflush(stdout);
    	wtk_lock_unlock(&(lock));
    }
    return p;
}

void* wtk_malloc_debug(size_t n,const char *f,int line)
{
	void *p;

	wtk_alloc_init();
	p=malloc(n);
	//make page dirty
	memset(p,0,n);
	if(pid==getpid())
	{
		wtk_lock_lock(&(lock));
		printf("DM malloc %s:%d %p %d\n",f,line,p,n);
		fflush(stdout);
		wtk_lock_unlock(&(lock));
	}
	return p;
}

void wtk_free_debug(void *p,const char *f,int line)
{
	wtk_alloc_init();
	if(pid==getpid())
	{
		wtk_lock_lock(&(lock));
		printf("DM free %s:%d %p\n",f,line,p);
		fflush(stdout);
		wtk_lock_unlock(&(lock));
	}
	free(p);
}
#endif
