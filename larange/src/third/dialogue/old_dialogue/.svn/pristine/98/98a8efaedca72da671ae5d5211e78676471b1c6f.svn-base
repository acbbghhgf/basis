#include "wtk_eos.h"

wtk_eos_t* wtk_eos_new()
{
	wtk_eos_t *o;

	o=(wtk_eos_t*)wtk_malloc(sizeof(*o));
	o->err=wtk_errno_new();
	return o;
}

int wtk_eos_delete(wtk_eos_t *o)
{
	wtk_errno_delete(o->err);
	wtk_free(o);
	return 0;
}

int wtk_eos_reset(wtk_eos_t *o)
{
	wtk_errno_reset(o->err);
	return 0;
}

