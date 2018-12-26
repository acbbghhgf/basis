#include "wtk_semshm.h"

wtk_semshm_t* wtk_semshm_new(int shm_bytes)
{
	wtk_semshm_t *s;
	int ret=-1;

	s=(wtk_semshm_t*)wtk_malloc(sizeof(*s));
	s->user_data_bytes=shm_bytes;
	s->user_data=0;
	s->addr=0;
	s->shm_id=-1;
	s->shared=0;
	shm_bytes+=sizeof(wtk_semshm_shared_t);
	s->shm_id=shmget(IPC_PRIVATE,shm_bytes,IPC_CREAT|0600);
	if(s->shm_id==-1)
	{
		goto end;
	}
	s->addr=(char*)shmat(s->shm_id,0,0);
	if(s->addr==(void*)-1)
	{
		s->addr=0;
		goto end;
	}
	s->shared=(wtk_semshm_shared_t*)s->addr;
	s->shared->user_data_len=0;
	ret=sem_init(&(s->shared->readable_sem),1,0);
	if(ret!=0){goto end;}
	ret=sem_init(&(s->shared->writeable_sem),1,1);
	if(ret!=0){goto end;}
	s->user_data=(((char*)s->addr)+sizeof(wtk_semshm_shared_t));
end:
	if(ret!=0)
	{
		wtk_semshm_delete(s);
		s=0;
	}
	return s;
}

int wtk_semshm_delete(wtk_semshm_t *s)
{
	if(s->shared)
	{
		sem_destroy(&(s->shared->readable_sem));
		sem_destroy(&(s->shared->writeable_sem));
	}
	if(s->addr)
	{
		shmdt(s->addr);
	}
	if(s->shm_id>=0)
	{
		shmctl(s->shm_id,IPC_RMID,0);
	}
	wtk_free(s);
	return 0;
}

int wtk_semshm_read(wtk_semshm_t *s,void *hook,wtk_semshm_read_f read)
{
	int ret;

	ret=sem_wait(&(s->shared->readable_sem));
	if(ret!=0){goto end;}
	read(hook,s->user_data,s->shared->user_data_len);
	s->shared->user_data_len=0;
	ret=sem_post(&(s->shared->writeable_sem));
end:
	return ret;
}

int wtk_semshm_readable(wtk_semshm_t *s)
{
	int val,ret;
	int readable=0;

	ret=sem_getvalue(&(s->shared->readable_sem),&val);
	if(ret!=0){goto end;}
	readable=val>0?1:0;
end:
	return readable;
}

int wtk_semshm_write_msg(wtk_semshm_t *s,const char *data,int bytes,int *writed)
{
	int ret,n,left;

	*writed=0;
	ret=sem_wait(&(s->shared->writeable_sem));
	if(ret!=0){goto end;}
	left=s->user_data_bytes-s->shared->user_data_len;
	n=min(left,bytes);
	memcpy(s->user_data+s->shared->user_data_len,data,n);
	s->shared->user_data_len+=n;
	*writed=n;
	ret=sem_post(&(s->shared->readable_sem));
end:
	return ret;
}

int wtk_semshm_write(wtk_semshm_t *s,const char *data,int bytes)
{
	const char *start,*end;
	int ret=0;
	int writed;

	start=data;
	end=data+bytes;
	while(start<end)
	{
		ret=wtk_semshm_write_msg(s,start,end-start,&writed);
		if(ret!=0){goto end;}
		start+=writed;
	}
end:
	return ret;
}

int wtk_semshm_nattach(wtk_semshm_t *s)
{
	struct shmid_ds buf;

	shmctl(s->shm_id,IPC_STAT,&buf);
	return buf.shm_nattch;
}

void wtk_semshm_wake_read(wtk_semshm_t *s)
{
	if(s->shared)
	{
		sem_post(&(s->shared->readable_sem));
	}
}

void wtk_semshm_wake_write(wtk_semshm_t *s)
{
	if(s->shared)
	{
		sem_post(&(s->shared->writeable_sem));
	}
}

void wtk_semshm_wake_rw(wtk_semshm_t *s)
{
	if(s->shared)
	{
		sem_post(&(s->shared->readable_sem));
		sem_post(&(s->shared->writeable_sem));
	}
}
