#include "wtk_shm.h"

wtk_shm_t* wtk_shm_new(int bytes)
{
	wtk_shm_t *s=0;
	int id;

	id=shmget(IPC_PRIVATE,bytes,IPC_CREAT|0600);
	if(id==-1){goto end;}
	s=(wtk_shm_t*)wtk_malloc(sizeof(*s));
	s->id=id;
	s->addr=(char*)shmat(id,0,0);
	if(s->addr==(void*)-1)
	{
		s->addr=0;
		wtk_shm_delete(s);
		s=0;
	}
end:
	return s;
}

void wtk_shm_delete(wtk_shm_t *s)
{
	if(s->addr)
	{
		shmdt(s->addr);
	}
	shmctl(s->id,IPC_RMID,0);
	wtk_free(s);
}
