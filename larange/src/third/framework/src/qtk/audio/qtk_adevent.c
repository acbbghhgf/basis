#include "qtk_adevent.h"

qtk_adevent_t* qtk_adevent_new(int init_size)
{
	qtk_adevent_t *e;

	e=(qtk_adevent_t*)wtk_malloc(sizeof(*e));
	e->buf=wtk_strbuf_new(init_size,1);
	return e;
}

int qtk_adevent_delete(qtk_adevent_t *e)
{
	wtk_strbuf_delete(e->buf);
	wtk_free(e);
	return 0;
}

int qtk_adevent_reset(qtk_adevent_t *e)
{
	wtk_strbuf_reset(e->buf);
	return 0;
}

int qtk_adevent_push(qtk_adevent_t *e,char *c,int bytes)
{
	wtk_strbuf_push(e->buf,c,bytes);
	return 0;
}
