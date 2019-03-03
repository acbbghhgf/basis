#include <ctype.h>
#include "wtk_flist.h"
void wtk_flist_feed_start(wtk_flist_t *fl,char c);
void wtk_flist_feed_append(wtk_flist_t *fl,char c);
void wtk_flist_feed(wtk_flist_t *fl,char *data,int len);

wtk_flist_t* wtk_flist_new(char *fn)
{
	wtk_flist_t *fl=0;
	char *data;
	int len;

	data=file_read_buf(fn,&len);
	if(!data){goto end;}
	fl=(wtk_flist_t*)wtk_malloc(sizeof(*fl));
	fl->heap=wtk_heap_new(4096);
	fl->buf=wtk_strbuf_new(1024,1);
	wtk_queue_init(&(fl->queue));
	wtk_flist_feed(fl,data,len);
end:
	if(data){free(data);}
	return fl;
}

int wtk_flist_delete(wtk_flist_t *fl)
{
	wtk_strbuf_delete(fl->buf);
	wtk_heap_delete(fl->heap);
	wtk_free(fl);
	return 0;
}

wtk_fitem_t* wtk_flist_append(wtk_flist_t *fl,char *data,int len)
{
	wtk_fitem_t *item;

	//wtk_debug("%*.*s\n",len,len,data);
	item=(wtk_fitem_t*)wtk_heap_malloc(fl->heap,sizeof(*item));
	item->str=wtk_heap_dup_string(fl->heap,data,len);
	wtk_queue_push(&(fl->queue),&(item->q_n));
	return item;
}

void wtk_flist_feed_start(wtk_flist_t *fl,char c)
{
	if(!isspace(c) && c!=EOF)
	{
		fl->state=WTK_FITEM_APPEND;
		wtk_strbuf_reset(fl->buf);
		wtk_flist_feed_append(fl,c);
	}
}

void wtk_flist_feed_append(wtk_flist_t *fl,char c)
{
	wtk_fitem_t *item;

	if(c=='\n' || c==EOF)
	{
		wtk_strbuf_push_c(fl->buf,0);
		item=wtk_flist_append(fl,fl->buf->data,fl->buf->pos);
		--item->str->len;
		fl->state=WTK_FITEM_START;
	}else
	{
		wtk_strbuf_push_c(fl->buf,c);
	}
}

void wtk_flist_feed_c(wtk_flist_t *fl,char c)
{
	switch(fl->state)
	{
	case WTK_FITEM_START:
		wtk_flist_feed_start(fl,c);
		break;
	case WTK_FITEM_APPEND:
		wtk_flist_feed_append(fl,c);
		break;
	}
}

void wtk_flist_feed(wtk_flist_t *fl,char *data,int len)
{
	char *s,*e;

	s=data;e=data+len;
	fl->state=WTK_FITEM_START;
	while(s<e)
	{
		wtk_flist_feed_c(fl,*s);
		++s;
	}
	wtk_flist_feed_c(fl,EOF);
}


void wtk_flist_process(char *fn,void *ths,wtk_flist_notify_f notify)
{
	wtk_flist_t *fl;
	wtk_queue_node_t *qn;
	wtk_fitem_t *item;
	int i;

	fl=wtk_flist_new(fn);
	for(i=0,qn=fl->queue.pop;qn;qn=qn->next,++i)
	{
		item=data_offset2(qn,wtk_fitem_t,q_n);
		//if(i%100==0)
		{
			wtk_debug("process %d/%d\n",i,fl->queue.length);
		}
		notify(ths,item->str->data);
	}
	wtk_flist_delete(fl);
}



void wtk_flist_process3(char *fn,void *ths,wtk_flist_notify_f2 notify,int cnt)
{
	wtk_flist_t *fl;
	wtk_queue_node_t *qn;
	wtk_fitem_t *item;
	int i;
	int step=0;

	fl=wtk_flist_new(fn);
	if(!fl)return;
	for(i=0,qn=fl->queue.pop;qn;qn=qn->next,++i)
	{
		if(i<cnt)
		{
			continue;
		}
		++step;
		item=data_offset2(qn,wtk_fitem_t,q_n);
		notify(ths,item->str->data,i,fl->queue.length);
	}
	if(step!=0)
	{
		wtk_flist_process3(fn,ths,notify,fl->queue.length);
	}
	wtk_flist_delete(fl);
}


void wtk_flist_process2(char *fn,void *ths,wtk_flist_notify_f2 notify)
{
	wtk_flist_process3(fn,ths,notify,0);
}

#include "wtk/core/cfg/wtk_source.h"

void wtk_flist_process4(char *fn,void *ths,wtk_flist_notify_f notify)
{
	wtk_strbuf_t *buf;
	wtk_source_t src;
	int ret;

	buf=wtk_strbuf_new(256,1);
	wtk_source_init_file(&(src),fn);
	while(1)
	{
		ret=wtk_source_read_line(&(src),buf);
		if(ret!=0){break;}
		if(buf->pos==0)
		{
			break;
		}
		wtk_strbuf_push_c(buf,0);
		notify(ths,buf->data);
	}
	wtk_source_clean_file(&(src));
	wtk_strbuf_delete(buf);
}


wtk_flist_it_t* wtk_flist_it_new(char *fn)
{
	wtk_flist_it_t *it;

	it=(wtk_flist_it_t*)wtk_malloc(sizeof(wtk_flist_it_t));
	it->buf=wtk_strbuf_new(256,1);
	wtk_source_init_file(&(it->src),fn);
	return it;
}

char* wtk_flist_it_next(wtk_flist_it_t *it)
{
	char *s=NULL;
	int ret;

	ret=wtk_source_read_line(&(it->src),it->buf);
	if(ret!=0){goto end;}
	if(it->buf->pos==0)
	{
		goto end;
	}
	wtk_strbuf_push_c(it->buf,0);
	s=it->buf->data;
end:
	return s;
}

void wtk_flist_it_delete(wtk_flist_it_t *it)
{
	wtk_source_clean_file(&(it->src));
	wtk_strbuf_delete(it->buf);
	wtk_free(it);
}



