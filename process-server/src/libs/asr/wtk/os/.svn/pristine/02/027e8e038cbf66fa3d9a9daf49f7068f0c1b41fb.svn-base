#include "wtk_lockhoard.h"

int wtk_lockhoard_init(wtk_lockhoard_t *h,int offset,int max_free,wtk_new_handler_t newer,wtk_delete_handler_t deleter,void *data)
{
	int ret;

	ret=wtk_lock_init(&(h->lock));
	if(ret!=0){goto end;}
	ret=wtk_hoard_init((wtk_hoard_t*)h,offset,max_free,newer,deleter,data);
end:
	return ret;
}

int wtk_lockhoard_clean(wtk_lockhoard_t *h)
{
	wtk_hoard_clean((wtk_hoard_t*)h);
	wtk_lock_clean(&(h->lock));
	return 0;
}

void* wtk_lockhoard_pop(wtk_lockhoard_t *h)
{
	void *d=0;
	int ret;

	ret=wtk_lock_lock(&(h->lock));
	if(ret!=0){goto end;}
	d=wtk_hoard_pop((wtk_hoard_t*)h);
	wtk_lock_unlock(&(h->lock));
end:
	return d;
}

int wtk_lockhoard_push(wtk_lockhoard_t *h,void* data)
{
	int ret;

	ret=wtk_lock_lock(&(h->lock));
	if(ret!=0){goto end;}
	ret=wtk_hoard_push((wtk_hoard_t*)h,data);
	wtk_lock_unlock(&(h->lock));
end:
	return ret;
}
