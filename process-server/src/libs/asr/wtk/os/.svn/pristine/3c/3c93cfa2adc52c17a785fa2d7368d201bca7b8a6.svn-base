#include "wtk_cond.h" 

void wtk_cond_init(wtk_cond_t *cond)
{
	pthread_mutex_init(&(cond->mutex),0);
	pthread_cond_init(&(cond->cond),NULL);
}

void wtk_cond_clean(wtk_cond_t *cond)
{
	pthread_mutex_destroy(&(cond->mutex));
	pthread_cond_destroy(&(cond->cond));
}

int wtk_cond_wait(wtk_cond_t *cond,int ms)
{
	struct timespec time;
	long ns, left;
	int ret;

	if(ms<=0)
	{
		return pthread_cond_wait(&(cond->cond),&(cond->mutex));
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
	ret=pthread_cond_timedwait(&(cond->cond),&(cond->mutex),&(time));
end:
	return ret;
}

int wtk_cond_wake(wtk_cond_t *cond)
{
	return pthread_cond_signal(&(cond->cond));
}

