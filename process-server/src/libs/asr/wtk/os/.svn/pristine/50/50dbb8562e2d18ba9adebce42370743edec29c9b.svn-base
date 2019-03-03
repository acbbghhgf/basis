#include "wtk_thread.h"
#ifdef WIN32
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
pid_t gettid(void);
#endif


wtk_thread_create_f glb_create;

void wtk_thread_g_set_glb_create(wtk_thread_create_f cf)
{
	glb_create=cf;
}


int wtk_speech_get_thread_id();

#ifdef WIN32
static DWORD WINAPI  wtk_thread_static_route(void *d)
#else
static void* wtk_thread_static_route(void* d)
#endif
{
	wtk_thread_t *t;
	int ret;

	t=(wtk_thread_t*)d;
	//wtk_debug("run thread tid=%d\n",gettid());
#ifdef WIN32
#else
#ifdef USE_ARM
	t->ppid=0;
#else
	t->ppid=syscall(SYS_gettid);
#endif
#endif
	ret=wtk_sem_inc(&(t->sem));
	//wtk_debug("run thread=%d/%s\n",t->ppid,t->name?t->name:"dummy");
	//wtk_debug("run thread tid=%d/%d thread=%s\n",gettid(),(int)pthread_self(),t->name?t->name:"dummy");
#ifdef __ANDROID__
	//wtk_debug("run thread tid=%d/%d thread=%s\n",gettid(),(int)pthread_self(),t->name?t->name:"dummy");
	//wtk_debug("run thread tid=%d/%d thread=%s\n",gettid(),wtk_speech_get_thread_id(),t->name?t->name:"dummy");
#endif
	//wtk_debug("run thread tid=%d thread=%s\n",t->ppid,t->name?t->name:"dummy");
	//wtk_debug("thread %d\n",(int)wtk_speech_get_thread_id())
	if(ret==0)
	{
		t->state=THREAD_STATE_RUN;
		if(t->route)
		{
			ret=t->route(t->data,t);
		}
	}
	t->state=THREAD_STATE_EXIT;
	//pthread_exit(0);
	return 0;
}


int wtk_thread_glb_process(wtk_thread_t *t)
{
	wtk_thread_static_route(t);
	return 0;
}

int wtk_thread_init(wtk_thread_t *t,thread_route_handler route,void *data)
{
	int ret;

	ret=wtk_sem_init(&(t->sem),0);
	if(ret!=0){goto end;}
	t->route=route;
	t->data=data;
	t->app_data=NULL;
	t->state=THREAD_STATE_INIT;
end:
	return ret;
}

int wtk_thread_clean(wtk_thread_t *t)
{
	wtk_sem_clean(&(t->sem));
	return 0;
}

int wtk_thread_start(wtk_thread_t *t)
{
	int ret;

#ifdef WIN32
	t->handler=CreateThread(
		NULL,0,
		wtk_thread_static_route,t,
		0,&(t->ppid));
	ret=t->handler?0:-1;
#else
	if(glb_create)
	{
		ret=glb_create((wtk_thread_route_f)wtk_thread_glb_process,t);
	}else
	{
		ret = pthread_create(&(t->handler), 0, wtk_thread_static_route, t);
	}
#endif
	if(ret!=0)
	{
		perror(__FUNCTION__);
		wtk_debug("create thread failed.\n");
		goto end;
	}
	ret = wtk_sem_acquire(&(t->sem),-1);
	if(ret!=0)
	{
		perror(__FUNCTION__);
		wtk_debug("get semaphore failed.\n");
	}
end:
	return ret;
}

int wtk_thread_join(wtk_thread_t *t)
{

#ifdef __ANDROID__
	//wtk_debug("join thread tid=%d thread=%s\n",t->ppid,t->name?t->name:"dummy");
#endif
#ifdef WIN32
     return ((WaitForSingleObject(t->handler,INFINITE)==WAIT_OBJECT_0)?0:-1);
#else
     int ret;

     if(t->handler==0){return 0;}
     ret=pthread_join(t->handler,0);
     t->handler=0;
     return ret;
#endif
}

void wtk_thread_set_name(wtk_thread_t *t,char *name)
{
	t->name=name;
}

int wtk_thread_kill(wtk_thread_t *t)
{
#ifdef WIN32
#else
	return pthread_kill(t->handler,SIGKILL);
#endif
}


