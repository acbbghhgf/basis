#ifdef WIN32
#include "wtk_sem.h"
#else
#define _XOPEN_SOURCE  600
#include "wtk_sem.h"
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#endif


#ifdef WIN32
int wtk_sem_init(wtk_sem_t *s,int init_count)
{
    *s=CreateSemaphore(NULL,init_count,INT_MAX,NULL);
    return *s ? 0 : -1;
}

int wtk_sem_clean(wtk_sem_t *s)
{
    return CloseHandle(*s) ? 0 : -1;
}

int wtk_sem_acquire(wtk_sem_t *s,int ms)
{
    ms=ms<0 ? INFINITE:ms;
    return  WaitForSingleObject(*s, ms)==WAIT_OBJECT_0?0:-1;
}

int wtk_sem_release(wtk_sem_t *s, int count)
{
    return ReleaseSemaphore(*s,count,NULL)!=0?0:-1;
}
#else
#ifdef USE_ARM

#ifdef __APPLE__
int wtk_sem_acquire(wtk_sem_t *s,int ms)
{
	return sem_wait(s);
}
#else
int wtk_sem_acquire(wtk_sem_t *s,int ms)
{
	struct timespec time;
	long ns, left;
	int ret;

	if(ms<0)
	{
		return sem_wait(s);
	}
	ret = clock_gettime(CLOCK_REALTIME, &time);
	if (ret != 0)
	{
		//perror(__FUNCTION__);
		goto end;
	}
	time.tv_sec += ms / 1000;
	ns = (ms % 1000) * 1E6;
	left = 1E9 - time.tv_nsec;
	if (left < ns)
	{
		time.tv_sec += 1;
		time.tv_nsec = ns - left;
	}
	else
	{
		time.tv_nsec += ns;
	}
	ret = sem_timedwait(s, &time);
end:
	return ret;
}
#endif

#else
#ifndef __APPLE__
#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <sys/mman.h>

wtk_sem_t* wtk_sem_new_shared()
{
	wtk_sem_t *s;
	int ret=0;

	s=(wtk_sem_t*)mmap(0,sizeof(*s),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
	if(s==MAP_FAILED)
	{
		perror(__FUNCTION__);
		s=0;
		goto end;
	}
	ret=sem_init(s,1,0);
end:
	if(ret!=0 && s)
	{
		wtk_sem_delete_shared(s);
		s=0;
	}
	return s;
}

void wtk_sem_delete_shared(wtk_sem_t *s)
{
	wtk_sem_clean(s);
	munmap(s,sizeof(*s));
}

int wtk_sem_acquire(wtk_sem_t *s,int ms)
{
	struct timespec time;
	long ns, left;
	int ret;

	if(ms<0)
	{
		return sem_wait(s);
	}
	ret = clock_gettime(CLOCK_REALTIME, &time);
	if (ret != 0)
	{
		//perror(__FUNCTION__);
		goto end;
	}
	time.tv_sec += ms / 1000;
	ns = (ms % 1000) * 1E6;
	left = 1E9 - time.tv_nsec;
	if (left < ns)
	{
		time.tv_sec += 1;
		time.tv_nsec = ns - left;
	}
	else
	{
		time.tv_nsec += ns;
	}
	ret = sem_timedwait(s, &time);
end:
	return ret;
}
#else
int wtk_sem_acquire(wtk_sem_t *s,int ms)
{
	return sem_wait(s);
}
#endif
#endif

int wtk_sem_release(wtk_sem_t *s, int count)
{
	int i,ret;

	if(count==1)
	{
		return sem_post(s);
	}
	ret=0;
	for(i=0;i<count;++i)
	{
		ret=sem_post(s);
		if(ret!=0){break;}
	}
	return ret;
}

int wtk_sem_drain(wtk_sem_t *s)
{
	int v;
	int ret;
	int i;

	ret=sem_getvalue(s,&v);
	if(ret!=0){goto end;}
	//wtk_debug("v=%d\n",v);
	if(v>0)
	{
		for(i=0;i<v;++i)
		{
			sem_trywait(s);
		}
		//exit(0);
	}
end:
	return ret;

}
#endif
