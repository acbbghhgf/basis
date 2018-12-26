#include "wtk_rw_lock.h" 
#include "wtk/core/wtk_os.h"

void wtk_rw_lock_init(wtk_rw_lock_t *lock)
{
	wtk_lock_init(&(lock->lock));
	wtk_lock_init(&(lock->xlock));
	lock->r=0;
	lock->w=0;
}

void wtk_rw_lock_clean(wtk_rw_lock_t *lock)
{
	wtk_lock_clean(&(lock->lock));
}

void wtk_rw_lock_lock_read(wtk_rw_lock_t *lock)
{
//	while(lock->w>0)
//	{
//		//sleep check
//		wtk_msleep(1);
//	}
	wtk_lock_lock(&(lock->lock));
}

void wtk_rw_lock_unlock_read(wtk_rw_lock_t *lock)
{
	wtk_lock_unlock(&(lock->lock));
}

void wtk_rw_lock_inc_read(wtk_rw_lock_t *lock)
{
	++lock->r;
}

void wtk_rw_lock_dec_read(wtk_rw_lock_t *lock)
{
	--lock->r;
}

void wtk_rw_lock_start_write(wtk_rw_lock_t *lock)
{
	//wtk_debug("lock=%p start write r=%d w=%d\n",lock,lock->r,lock->w);
	while(lock->r>0)
	{
		//sleep check
		//wtk_msleep(1);
		//usleep(1);
		sched_yield();
	}
	wtk_lock_lock(&(lock->lock));
	++lock->w;
	//wtk_lock_unlock(&(lock->lock));
}

void wtk_rw_lock_stop_write(wtk_rw_lock_t *lock)
{
	//wtk_debug("lock=%p stop write r=%d w=%d\n",lock,lock->r,lock->w);
	//wtk_lock_lock(&(lock->lock));
	--lock->w;
	wtk_lock_unlock(&(lock->lock));
}

void wtk_rw_lock_start_read(wtk_rw_lock_t *lock)
{
	//wtk_debug("lock=%p start read r=%d w=%d\n",lock,lock->r,lock->w);
	//wtk_debug("wait write\n");
//	while(lock->w>0)
//	{
//		//wtk_msleep(1);
//		//usleep(1);
//		sched_yield();
//	}
	wtk_lock_lock(&(lock->lock));
	wtk_lock_lock(&(lock->xlock));
	++lock->r;
	wtk_lock_unlock(&(lock->xlock));
	wtk_lock_unlock(&(lock->lock));
	//wtk_debug("wait write end\n");
}

void wtk_rw_lock_stop_read(wtk_rw_lock_t *lock)
{
	//wtk_debug("lock=%p stop read r=%d w=%d\n",lock,lock->r,lock->w);
	//wtk_lock_lock(&(lock->lock));
	wtk_lock_lock(&(lock->xlock));
	--lock->r;
	wtk_lock_unlock(&(lock->xlock));
	//wtk_lock_unlock(&(lock->lock));
}
