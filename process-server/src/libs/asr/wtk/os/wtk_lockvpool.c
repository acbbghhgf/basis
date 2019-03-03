#include "wtk_lockvpool.h" 


wtk_lockvpool_t* wtk_lockvpool_new(int bytes,int max_free)
{
	return wtk_lockvpool_new2(bytes,max_free,max_free);
}

wtk_lockvpool_t* wtk_lockvpool_new2(int bytes,int max_free,int reset_free)
{
	wtk_lockvpool_t *v;

	v=(wtk_lockvpool_t*)wtk_malloc(sizeof(wtk_lockvpool_t));
	wtk_lock_init(&(v->lock));
	v->pool=wtk_vpool_new2(bytes,max_free,reset_free);
	return v;
}

void wtk_lockvpool_delete(wtk_lockvpool_t *v)
{
	wtk_vpool_delete(v->pool);
	wtk_lock_clean(&(v->lock));
	wtk_free(v);
}

void wtk_lockvpool_reset(wtk_lockvpool_t *v)
{
	wtk_lock_lock(&(v->lock));
	wtk_vpool_reset(v->pool);
	wtk_lock_unlock(&(v->lock));
}

void* wtk_lockvpool_pop(wtk_lockvpool_t *v)
{
	void *p;

	wtk_lock_lock(&(v->lock));
	p=wtk_vpool_pop(v->pool);
	wtk_lock_unlock(&(v->lock));
	return p;
}

void wtk_lockvpool_push(wtk_lockvpool_t *v,void *usr_data)
{
	wtk_lock_lock(&(v->lock));
	wtk_vpool_push(v->pool,usr_data);
	wtk_lock_unlock(&(v->lock));
}
