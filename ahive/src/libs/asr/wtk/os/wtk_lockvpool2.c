#include "wtk_lockvpool2.h" 


wtk_lockvpool2_t* wtk_lockvpool2_new(int bytes,int max_free)
{
	wtk_lockvpool2_t *v;

	v=(wtk_lockvpool2_t*)wtk_malloc(sizeof(wtk_lockvpool2_t));
	wtk_lock_init(&(v->lock));
	v->pool=wtk_vpool2_new(bytes,max_free);
	return v;
}

void wtk_lockvpool2_delete(wtk_lockvpool2_t *v)
{
	wtk_vpool2_delete(v->pool);
	wtk_lock_clean(&(v->lock));
	wtk_free(v);
}

void wtk_lockvpool2_reset(wtk_lockvpool2_t *v)
{
	wtk_lock_lock(&(v->lock));
	wtk_vpool2_reset(v->pool);
	wtk_lock_unlock(&(v->lock));
}

void* wtk_lockvpool2_pop(wtk_lockvpool2_t *v)
{
	void *p;

	wtk_lock_lock(&(v->lock));
	p=wtk_vpool2_pop(v->pool);
	wtk_lock_unlock(&(v->lock));
	return p;
}

void wtk_lockvpool2_push(wtk_lockvpool2_t *v,void *usr_data)
{
	wtk_lock_lock(&(v->lock));
	wtk_vpool2_push(v->pool,usr_data);
	wtk_lock_unlock(&(v->lock));
}
