#include "wtk_strlist.h" 

void wtk_strlist_item_delete(wtk_strlist_item_t *item)
{
	wtk_string_delete(item->k);
	wtk_free(item);
}

wtk_strlist_item_t* wtk_strlist_item_new(char *k,int len)
{
	wtk_strlist_item_t *item;

	item=(wtk_strlist_item_t*)wtk_malloc(sizeof(wtk_strlist_item_t));
	item->k=wtk_string_dup_data(k,len);
	item->v.i=0;
	return item;
}

wtk_strlist_t* wtk_strlist_new()
{
	wtk_strlist_t *l;

	l=(wtk_strlist_t*)wtk_malloc(sizeof(wtk_strlist_t));
	wtk_queue_init(&(l->q));
	return l;
}

void wtk_strlist_delete(wtk_strlist_t *l)
{
	wtk_queue_node_t *qn;
	wtk_strlist_item_t *item;

	while(1)
	{
		qn=wtk_queue_pop(&(l->q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_strlist_item_t,q_n);
		wtk_strlist_item_delete(item);
	}
	wtk_free(l);
}

int wtk_strlist_load(wtk_strlist_t *l,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	int ret;
	int eof;
	wtk_strlist_item_t *item;

	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_read_line2(src,buf,&eof);
		if(ret!=0){goto end;}
		if(buf->pos>0)
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			item=wtk_strlist_item_new(buf->data,buf->pos);
			wtk_queue_push(&(l->q),&(item->q_n));
		}
		if(eof)
		{
			break;
		}
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}

wtk_strlist_t* wtk_strlist_new2(char *fn)
{
	wtk_strlist_t *l;

	l=wtk_strlist_new();
	wtk_source_load_file(l,(wtk_source_load_handler_t)wtk_strlist_load,fn);
	return l;
}
