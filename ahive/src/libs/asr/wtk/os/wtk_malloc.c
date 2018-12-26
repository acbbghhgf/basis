#include <malloc.h>
#include "wtk_malloc.h"
#include "wtk/os/wtk_lock.h"
#ifdef USE_MALLOC_HOOK
typedef void*(*wtk_malloc_hook_f)(size_t size,const void *caller);
typedef void (*wtk_free_hook_f)(void *ptr,const void *caller);

wtk_lock_t lock;
pid_t pid;
wtk_malloc_hook_f old_malloc_hook;
wtk_free_hook_f free_malloc_hook;
void* wtk_malloc_hook(size_t size,const void *caller);
void wtk_free_hook(void *p,const void *caller);

void* wtk_malloc_hook(size_t size,const void *caller)
{
	void *p;

	wtk_lock_lock(&(lock));
	__malloc_hook=old_malloc_hook;
	__free_hook=free_malloc_hook;
	p=malloc(size);
	if(pid==getpid())
	{
		wtk_proc_dump_stack2();
		printf("DM malloc %s:%d %p %d\n",__FUNCTION__,getpid(),p,(int)size);
		fflush(stdout);
	}
	__malloc_hook=wtk_malloc_hook;
	__free_hook=wtk_free_hook;
	wtk_lock_unlock(&(lock));
	return p;
}

void wtk_free_hook(void *p,const void *caller)
{
	if(!p)
	{
		//wtk_debug("found nil...\n");
		//exit(0);
		return;
	}
	wtk_lock_lock(&(lock));
	__malloc_hook=old_malloc_hook;
	__free_hook=free_malloc_hook;
	free(p);
	if(pid==getpid())
	{
		printf("DM free %s:%d %p\n",__FUNCTION__,getpid(),p);
		fflush(stdout);
	}
	//wtk_debug("free (%u) returns %p\n", (unsigned int) size, p);
	__malloc_hook=wtk_malloc_hook;
	__free_hook=wtk_free_hook;
	wtk_lock_unlock(&(lock));
}

void wtk_malloc_init()
{
	pid=getpid();
	wtk_lock_init(&lock);
	old_malloc_hook=__malloc_hook;
	free_malloc_hook=__free_hook;
	__malloc_hook=wtk_malloc_hook;
	__free_hook=wtk_free_hook;
}
#endif
