
#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif
#include "wtk_lock.h"

int wtk_lock_init(wtk_lock_t *l)
{
    InitializeCriticalSection(l);
    return 0;
}

int wtk_lock_clean(wtk_lock_t *l)
{
    DeleteCriticalSection(l);
    return 0;
}

int wtk_lock_lock(wtk_lock_t *l)
{
    EnterCriticalSection(l);
    return 0;
}

int wtk_lock_unlock(wtk_lock_t *l)
{
    LeaveCriticalSection(l);
    return 0;
}
int wtk_lock_trylock(wtk_lock_t *l)
{
    return TryEnterCriticalSection(l)?0:-1;
}
#else
/*
#include <pthread.h>

int wtk_lock_init2(wtk_lock_t *lock)
{
	pthread_mutexattr_t  attr,*p;
	int ret;

	p=0;
	ret=pthread_mutexattr_init(&attr);
	if(ret!=0){goto end;}
	p=&(attr);
	ret=pthread_mutexattr_settype(p,PTHREAD_MUTEX_RECURSIVE);
	if(ret!=0){goto end;}
	ret=pthread_mutex_init(lock,p);
end:
	if(p)
	{
		pthread_mutexattr_destroy(p);
	}
	return ret;
}
*/
#endif












