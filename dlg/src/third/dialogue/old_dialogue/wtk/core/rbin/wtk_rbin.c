#include "wtk_rbin.h"
#include <stdlib.h>

int wtk_ritem_delete(wtk_ritem_t *i)
{
	if(i->data.len>0)
	{
		wtk_free(i->data.data);
	}
	wtk_string_delete(i->fn);
	wtk_free(i);
	return 0;
}

void wtk_ritem_print(wtk_ritem_t *i)
{
	print_data(i->fn->data,i->fn->len);
	printf("data: %d\n",i->data.len);
}

int wtk_ritem_get(wtk_ritem_t *i)
{
	int c;

	if(i->pos<i->data.len)
	{
		//for EOF check;
		c=(unsigned char)i->data.data[i->pos];
		++i->pos;
	}else
	{
		//wtk_debug("%*.*s EOF.\n",i->fn->len,i->fn->len,i->fn->data);
		c=EOF;
	}
	//wtk_debug("v[%d/%d]=%x,%x\n",i->pos,i->data.len,c,EOF);
	return c;
}

int wtk_ritem_get2(wtk_ritem_t* i,char *buf,int bytes)
{
	int len;

	len=i->data.len-i->pos;
	if(len<bytes)
	{
		return EOF;
	}
	memcpy(buf,i->data.data+i->pos,bytes);
	i->pos+=bytes;
	return bytes;
}

int wtk_ritem_unget(wtk_ritem_t *i,int c)
{
	if(i->pos>0){--i->pos;}
	return 0;
}


wtk_rbin_t* wtk_rbin_new()
{
	wtk_rbin_t* rb;

	rb=(wtk_rbin_t*)wtk_malloc(sizeof(*rb));
	wtk_queue_init(&(rb->list));
	rb->buf=wtk_strbuf_new(1024,1);
	rb->fl=0;
	return rb;
}

int wtk_rbin_delete(wtk_rbin_t *rb)
{
	wtk_queue_node_t *n,*p;
	wtk_ritem_t *i;

	for(n=rb->list.pop;n;n=p)
	{
		p=n->next;
		i=data_offset(n,wtk_ritem_t,q_n);
		wtk_ritem_delete(i);
	}
	if(rb->fl){wtk_flist_delete(rb->fl);}
	wtk_strbuf_delete(rb->buf);
	wtk_free(rb);
	return 0;
}

int wtk_rbin_read_scp(wtk_rbin_t *rb,char *fn)
{
	rb->fl=wtk_flist_new(fn);
	return 0;
}

wtk_ritem_t* wtk_rbin_new_item(wtk_rbin_t *rb,wtk_string_t *fn,char *rfn)
{
	wtk_ritem_t* item;
	int len;
	char *data;
	int ret=-1;

	//printf("%d:%*.*s#\n",fn->len,fn->len,fn->len,fn->data);
	item=(wtk_ritem_t*)wtk_malloc(sizeof(*item));
	item->fn=wtk_string_dup_data(fn->data,fn->len);
	item->data.len=0;
	data=file_read_buf(rfn,&len);
	if(!data){goto end;}
	item->data.data=data;
	item->data.len=len;
	ret=0;
end:
	if(ret!=0)
	{
		wtk_ritem_delete(item);
		item=0;
	}
	return item;
}

int wtk_rbin_read_dn(wtk_rbin_t *rb,char *res_dn)
{
	wtk_queue_node_t *n;
	wtk_queue_t *q=&(rb->list);
	wtk_fitem_t *i;
	wtk_ritem_t *ri;
	wtk_strbuf_t *buf=rb->buf;
	int rlen=strlen(res_dn);
	int ret=-1;
	int add_sep;

	//wtk_debug("%s\n",res_dn);
	if(rlen>0 && res_dn[rlen-1]!='/')
	{
		add_sep=1;
	}else
	{
		add_sep=0;
	}
	for(n=rb->fl->queue.pop;n;n=n->next)
	{
		i=data_offset(n,wtk_fitem_t,q_n);
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,res_dn,rlen);
		if(add_sep)
		{
			//add directory sep;
			wtk_strbuf_push_c(buf,'/');
		}
		wtk_strbuf_push(buf,i->str->data,i->str->len);
		wtk_strbuf_push_c(buf,0);
		//wtk_debug("%s\n",buf->data);
		ri=wtk_rbin_new_item(rb,i->str,buf->data);
		if(!ri){goto end;}
		wtk_queue_push(q,&(ri->q_n));
		//hdr_size+=RBIN_INT_SIZE+i->str->len+RBIN_INT_SIZE;
	}
	//wtk_debug("%d,hs=%d\n",q->length,hdr_size);
	ret=0;
end:
	return ret;
}

#define WTK_RBIN_INT_SIZE 10

void wtk_rbin_write_int_f(wtk_rbin_t *rb,int v,FILE* f)
{
	char buf[WTK_RBIN_INT_SIZE]={0,0};

	sprintf(buf,"%d",v);
	fwrite(buf,sizeof(buf),1,f);
}

int wtk_rbin_read_int_f(wtk_rbin_t *rb,FILE* f,int *v)
{
	char buf[WTK_RBIN_INT_SIZE];
	int ret;

	ret=fread(buf,sizeof(buf),1,f);
	if(ret!=1){ret=-1;goto end;}
	*v=atoi(buf);
	ret=0;
end:
	return ret;
}

void wtk_file_write_reverse(FILE* f,char *data,int len)
{
	int i;
	unsigned char c;

	//fwrite(data,len,1,f);
	for(i=0;i<len;++i)
	{
		//c=255-((unsigned char)data[i]);
		c=~((unsigned char)data[i]);
		fwrite(&c,1,1,f);
	}
}


void wtk_rbin_write_data2(wtk_rbin_t *rb,FILE* f,char *data,int len)
{
	int i;
	unsigned char c;

	//fwrite(data,len,1,f);
	for(i=0;i<len;++i)
	{
		//c=255-((unsigned char)data[i]);
		c=~((unsigned char)data[i]);
		fwrite(&c,1,1,f);
	}
}

int wtk_rbin_read_data2(wtk_rbin_t *rb,FILE* f,unsigned char *data,int len)
{
	unsigned char c;
	int i,ret=0;

	//fwrite(data,len,1,f);
	for(i=0;i<len;++i)
	{
		ret=fread(&c,1,1,f);
		if(ret!=1){ret=-1;goto end;}
		data[i]=~(c);
	}
	ret=0;
end:
	return ret;
}

void wtk_rbin_reverse_data(unsigned char *p,int len)
{
	unsigned char *e;

	e=p+len;
	while(p<e)
	{
		*p=~*p;
		++p;
	}
}

void wtk_rbin_write_data(wtk_rbin_t *rb,FILE* f,char *data,int len)
{
	wtk_rbin_reverse_data((unsigned char *)data,len);
	//skip error check;
	fwrite(data,len,1,f);
	wtk_rbin_reverse_data((unsigned char *)data,len);
}

int wtk_rbin_read_data(wtk_rbin_t *rb,FILE* f,unsigned char *data,int len)
{
	int ret;

	ret = fread(data,1,len,f);
	if(ret!=len){ret=-1;goto end;}
	wtk_rbin_reverse_data(data,len);
	ret=0;
end:
	return ret;
}

int wtk_rbin_write(wtk_rbin_t *rb,char *res_dn,char *bin)
{
	wtk_queue_node_t *n;
	wtk_queue_t *q=&(rb->list);
	wtk_ritem_t *i;
	FILE* f=0;
	int ret=-1;

	f=fopen(bin,"wb");
	if(!f){goto end;}
	ret=wtk_rbin_read_dn(rb,res_dn);
	if(ret!=0){goto end;}
	wtk_rbin_write_int_f(rb,q->length,f);
	for(n=q->pop;n;n=n->next)
	{
		i=data_offset(n,wtk_ritem_t,q_n);
		wtk_rbin_write_int_f(rb,i->fn->len,f);
		wtk_rbin_write_data(rb,f,i->fn->data,i->fn->len);
		//fwrite(i->fn->data,i->fn->len,1,f);
		wtk_rbin_write_int_f(rb,i->data.len,f);
		wtk_rbin_write_data(rb,f,i->data.data,i->data.len);
	}
	ret=0;
end:
	if(f){fclose(f);}
	return ret;
}


int wtk_bin_read_bin(wtk_rbin_t *rb,FILE* f)
{
	wtk_queue_t *q=&(rb->list);
	wtk_ritem_t *item;
	int c,v,ret;
	int i;

	ret=wtk_rbin_read_int_f(rb,f,&c);
	if(ret!=0){goto end;}
	for(i=0;i<c;++i)
	{
		ret=wtk_rbin_read_int_f(rb,f,&v);
		if(ret!=0){goto end;}
		item=(wtk_ritem_t*)wtk_malloc(sizeof(*item));
		item->data.len=0;
		item->fn=wtk_string_new(v);
		ret=wtk_rbin_read_data(rb,f,(unsigned char*)item->fn->data,v);
		if(ret!=0){goto end;}
		ret=wtk_rbin_read_int_f(rb,f,&v);
		if(ret!=0){goto end;}
		item->data.len=v;
		//wtk_debug("v=%d\n",v);
		if(v==0)
		{
			item->data.data=0;
		}else
		{
			item->data.data=(char*)wtk_malloc(v);
		}
		ret=wtk_rbin_read_data(rb,f,(unsigned char*)item->data.data,v);
		if(ret!=0){goto end;}
		//wtk_debug("%.*s: %d\n",item->fn->len,item->fn->data,item->data.len);
		//printf("%d: %*.*s\n",item->fn->len,item->fn->len,item->fn->len,item->fn->data);
		wtk_queue_push(q,&(item->q_n));
	}
	ret=0;
end:
	return ret;
}

int wtk_rbin_read(wtk_rbin_t *rb,char *bin)
{
	FILE* f;
	int ret=-1;

	f=fopen(bin,"rb");
	if(!f)
	{
		wtk_debug("%s not exist.\n",bin);
		goto end;
	}
	ret=wtk_bin_read_bin(rb,f);
end:
	if(f){fclose(f);}
	//wtk_debug("%d\n",ret);
	return ret;
}

int wtk_rbin_write_item(wtk_rbin_t *rb,wtk_ritem_t *i,char *dn,int len)
{
	wtk_strbuf_t *buf=rb->buf;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,dn,len);
	wtk_strbuf_push_s(buf,"/");
	wtk_strbuf_push(buf,i->fn->data,i->fn->len);
	wtk_strbuf_push_c(buf,0);
	//printf("%s\n",buf->data);
	return file_write_buf(buf->data,i->data.data,i->data.len);
}

int wtk_rbin_extract(wtk_rbin_t *rb,char *dn)
{
	wtk_queue_node_t *n;
	wtk_queue_t *q=&(rb->list);
	wtk_ritem_t *i;
	int len=strlen(dn);
	int ret=-1;

	if(wtk_file_exist(dn)==0)
	{
		wtk_mkdir_p(dn,DIR_SEP,1);
	}
	for(n=q->pop;n;n=n->next)
	{
		i=data_offset(n,wtk_ritem_t,q_n);
		ret=wtk_rbin_write_item(rb,i,dn,len);
		if(ret!=0)
		{
			wtk_debug("write %*.*s failed.\n",i->fn->len,i->fn->len,i->fn->data);
			goto end;
		}
	}
end:
	return ret;
}

wtk_ritem_t* wtk_rbin_find(wtk_rbin_t *rb,char *data,int len)
{
	wtk_queue_node_t *n;
	wtk_queue_t *q=&(rb->list);
	wtk_ritem_t *i=0,*x;

	for(n=q->pop;n;n=n->next)
	{
		x=data_offset(n,wtk_ritem_t,q_n);
		if(wtk_string_cmp(x->fn,data,len)==0)
		{
			i=x;
			break;
		}
	}
	return i;
}

void wtk_source_init_rbin(wtk_source_t* s,wtk_ritem_t *i)
{
	wtk_source_init(s);
	i->pos=0;
	s->data=i;
	s->get=(wtk_source_get_handler_t)wtk_ritem_get;
	s->unget=(wtk_source_unget_handler_t)wtk_ritem_unget;
	s->get_str=(wtk_source_get_str_f)wtk_ritem_get2;
	//s->swap=0;//wtk_is_little_endian();
	s->swap=wtk_is_little_endian();
}


int wtk_rbin_load_file(wtk_rbin_t *rb,void *data,wtk_source_load_handler_t loader,char *fn)
{
	wtk_source_t s,*ps=&s;
	wtk_ritem_t *i;
	int ret=-1;

	i=wtk_rbin_find(rb,fn,strlen(fn));
	//wtk_debug("fn=%s,i=%p\n",fn,i);
	if(!i){goto end;}
	//wtk_ritem_print(i);
	wtk_source_init_rbin(ps,i);
	ret=loader(data,ps);
end:
	return ret;
}
