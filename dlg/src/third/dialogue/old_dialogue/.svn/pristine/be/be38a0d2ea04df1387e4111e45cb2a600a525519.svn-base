#include <ctype.h>
#include <stdarg.h>
#include "wtk_errno.h"

wtk_errno_t* wtk_errno_new()
{
	wtk_errno_t *err;

	err=(wtk_errno_t*)wtk_malloc(sizeof(*err));
	err->no=0;
	err->buf=wtk_strbuf_new(4096,1);
	return err;
}

int wtk_errno_delete(wtk_errno_t *e)
{
	wtk_strbuf_delete(e->buf);
	wtk_free(e);
	return 0;
}

void wtk_errno_reset(wtk_errno_t *e)
{
	e->no=0;
	wtk_strbuf_reset(e->buf);
}

void wtk_errno_set(wtk_errno_t *e,int no,char *msg,int msg_bytes)
{
	e->no=no;
	wtk_strbuf_reset(e->buf);
	wtk_strbuf_push(e->buf,msg,msg_bytes);
}

void wtk_errno_set_string(wtk_errno_t *e,int no,...)
{
	va_list ap;
	char *p;

	e->no=no;
	wtk_strbuf_reset(e->buf);
	va_start(ap,no);
	while((p=va_arg(ap,char*))!=0)
	{
		wtk_strbuf_push_string(e->buf,p);
	}
	va_end(ap);
}

void wtk_errno_print(wtk_errno_t *e)
{
	printf("%d:\t%*.*s\n",e->no,e->buf->pos,e->buf->pos,e->buf->data);
}
