#include "wtk_rbin2.h"

wtk_rbin2_t* wtk_rbin2_new()
{
	wtk_rbin2_t *rb;

	rb=(wtk_rbin2_t*)wtk_malloc(sizeof(wtk_rbin2_t));
	rb->heap=wtk_heap_new(4096);
	rb->f=NULL;
	rb->fn=NULL;
	rb->buf=wtk_strbuf_new(4096,1);
	rb->version=1;
	wtk_queue_init(&(rb->list));
	return rb;
}

void wtk_rbin2_delete(wtk_rbin2_t *rb)
{
	wtk_strbuf_delete(rb->buf);
	if(rb->f)
	{
		fclose(rb->f);
	}
	wtk_heap_delete(rb->heap);
	wtk_free(rb);
}


int wtk_rbin2_add(wtk_rbin2_t *rb,wtk_string_t *name,char *realname)
{
	char *data;
	int len;
	int ret=-1;

	///wtk_debug("[%s]\n",realname);
	data=file_read_buf(realname,&len);
	if(!data){goto end;}
	wtk_heap_add_large(rb->heap,data,len);
	wtk_rbin2_add2(rb,name,data,len);
	ret=0;
end:
	return ret;
}

wtk_rbin2_item_t* wtk_rbin2_new_item(wtk_rbin2_t *rb)
{
	wtk_rbin2_item_t *item;

	item=(wtk_rbin2_item_t*)wtk_heap_malloc(rb->heap,sizeof(wtk_rbin2_item_t));
	item->fn=NULL;
	item->data=NULL;
	item->pos=-1;
	item->len=item->seek_pos=0;
	item->buf_pos=0;
	item->reverse=0;
	item->rb=rb;
	return item;
}

int wtk_rbin2_is_file_reverse(wtk_rbin2_t *rv,char *name,int len)
{
	char *suf[]={
			".lex",".lua",".cfg",".r",".fst",".nlg",".srt",".owl"
	};
	int i,n;

	if(rv->version==0)
	{
		return name[len-1]=='r'?1:0;
	}else
	{
		if(len==3 && strncmp(name,"cfg",3)==0)
		{
			return 1;
		}else if(len==5 && strncmp(name,"./cfg",5)==0)
		{
			return 1;
		}
		n=sizeof(suf)/sizeof(char*);
		for(i=0;i<n;++i)
		{
			if(wtk_str_end_with(name,len,suf[i],strlen(suf[i])))
			{
				return 1;
			}
		}
		return 0;
	}
}


void wtk_rbin2_add2(wtk_rbin2_t *rb,wtk_string_t *name,char *data,int len)
{

	wtk_rbin2_item_t *item;
	wtk_heap_t *heap=rb->heap;
	wtk_strbuf_t *buf=rb->buf;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_s(buf,"./");
	wtk_strbuf_push(buf,name->data,name->len);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	item=wtk_rbin2_new_item(rb);
	//item->fn=wtk_heap_dup_string(heap,name->data,name->len);
	item->fn=wtk_heap_dup_string(heap,buf->data,buf->pos);
	item->data=(wtk_string_t*)wtk_heap_malloc(heap,sizeof(wtk_string_t));
	item->reverse=wtk_rbin2_is_file_reverse(rb,name->data,name->len);
	//wtk_debug("[%.*s]=%d\n",name->len,name->data,item->reverse);
	//print_data(item->data,4);
	wtk_string_set((item->data),data,len);
	wtk_queue_push(&(rb->list),&(item->q_n));
}

int wtk_rbin2_write_data(FILE* f,char *data,int len)
{
	int ret;

	wtk_rbin_reverse_data((unsigned char *)data,len);
	//skip error check;
	ret=fwrite(data,len,1,f);
	wtk_rbin_reverse_data((unsigned char *)data,len);
	return ret==1?0:-1;
}

int wtk_rbin2_write(wtk_rbin2_t *rb,char *fn)
{
	wtk_queue_node_t *qn;
	wtk_rbin2_item_t *item;
	int ret=-1;
	FILE *f;
	int vi;
	int pos;

	//wtk_debug("write %s\n",fn);
	f=fopen(fn,"wb");
	if(!f){goto end;}
	if(rb->version>0)
	{
		vi=-rb->version;
		ret=fwrite(&vi,1,4,f);
		if(ret!=4){ret=-1;goto end;}
		pos=4;
	}else
	{
		pos=0;
	}
	vi=rb->list.length;
	ret=fwrite(&vi,1,4,f);
	if(ret!=4){ret=-1;goto end;}
	pos+=4;
	for(qn=rb->list.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
		pos+=4+item->fn->len+4+4;
	}
	for(qn=rb->list.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
		vi=item->fn->len;
		item->pos=pos;
		pos+=item->data->len;
		ret=fwrite(&(vi),1,4,f);
		if(ret!=4){ret=-1;goto end;}
		ret=wtk_rbin2_write_data(f,item->fn->data,item->fn->len);
		//ret=fwrite(item->fn->data,item->fn->len,1,f);
		//wtk_debug("[%.*s] ret=%d\n",item->fn->len,item->fn->data, ret);
		if(ret!=0){ret=-1;goto end;}
		vi=item->pos;
		ret=fwrite(&(vi),1,4,f);
		if(ret!=4){ret=-1;goto end;}
		vi=item->data->len;
		ret=fwrite(&(vi),1,4,f);
		if(ret!=4){ret=-1;goto end;}
	}
	for(qn=rb->list.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
		if(item->data && item->data->len>0)
		{
			/*
			wtk_debug("[%.*s] reverse=%d,fpos=%d,pos=%d,len=%d\n",item->fn->len,item->fn->data,
					item->reverse,(int)ftell(f),item->pos,item->data->len);
				*/
			//print_data(item->data->data,10);
			if(item->reverse)
			{
				ret=wtk_rbin2_write_data(f,item->data->data,item->data->len);
				//ret=fwrite(item->fn->data,item->fn->len,1,f);
				//wtk_debug("[%.*s]\n",item->fn->len,item->fn->data);
				if(ret!=0){ret=-1;goto end;}
			}else
			{
				//wtk_debug("[%.*s] %d/%d\n",item->fn->len,item->fn->data,(int)item->pos,(int)ftell(f));
				//print_data(item->data->data,4);
				ret=fwrite(item->data->data,item->data->len,1,f);
				if(ret!=1){ret=-1;goto end;}
			}
		}
	}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	if(f)
	{
		fclose(f);
	}
	return ret;
}

void wtk_rbin2_load_all(wtk_rbin2_t *rb)
{
	wtk_queue_node_t *qn;
	wtk_rbin2_item_t *item;

	for(qn=rb->list.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
		if(!item->data)
		{
			wtk_rbin2_load_item(rb,item,1);
		}
	}
}

int wtk_rbin2_read(wtk_rbin2_t *rb,char *fn)
{
	wtk_rbin2_item_t *item;
	wtk_heap_t *heap;
	FILE *f;
	int ret;
	int i;
	int n,vi;

	heap=rb->heap;
	f=fopen(fn,"rb");
	if(!f){ret=-1;goto end;}
	ret=fread((char*)&(n),4,1,f);
	if(ret!=1){ret=-1;goto end;}
	if(n<0)
	{
		rb->version=-n;
		ret=fread((char*)&(n),4,1,f);
		if(ret!=1){ret=-1;goto end;}
	}else
	{
		rb->version=0;
	}
	//wtk_debug("n=%d\n",n);
	for(i=0;i<n;++i)
	{
		item=wtk_rbin2_new_item(rb);//(wtk_rbin2_item_t*)wtk_heap_malloc(heap,sizeof(wtk_rbin2_item_t));
		ret=fread((char*)&(vi),4,1,f);
		if(ret!=1)
		{
			ret=-1;goto end;
		}
		item->fn=wtk_heap_dup_string(heap,0,vi);
		ret=fread(item->fn->data,item->fn->len,1,f);
		//wtk_debug("ret=%d\n",ret);
		if(ret!=1){ret=-1;goto end;}
		wtk_rbin_reverse_data((unsigned char*)item->fn->data,item->fn->len);
		//wtk_debug("[%.*s]\n",item->fn->len,item->fn->data);
		item->reverse=wtk_rbin2_is_file_reverse(rb,item->fn->data,item->fn->len);
		ret=fread((char*)&(vi),4,1,f);
		if(ret!=1){ret=-1;goto end;}
		item->pos=vi;
		ret=fread((char*)&(vi),4,1,f);
		if(ret!=1){ret=-1;goto end;}
		item->len=vi;
		//wtk_debug("[%.*s]=%d/%d\n",item->fn->len,item->fn->data,item->pos,item->len);
		item->data=NULL;
		wtk_queue_push(&(rb->list),&(item->q_n));
	}
	rb->fn=wtk_heap_dup_str(rb->heap,fn);
	ret=0;
end:
	if(ret!=0)
	{
		if(f)
		{
			fclose(f);
		}
	}else
	{
		rb->f=f;
	}
	//wtk_debug("ret=%d,f=%p/%p\n",ret,f,rb->f);
	return ret;
}

int wtk_rbin2_read2(wtk_rbin2_t *rb,char *fn,unsigned int seek_pos)
{
	wtk_rbin2_item_t *item;
	wtk_heap_t *heap;
	FILE *f;
	int ret;
	int i;
	int n,vi;

	heap=rb->heap;
	f=fopen(fn,"rb");
	if(!f){ret=-1;goto end;}
	ret=fseek(f,seek_pos,SEEK_SET);
	if(ret==-1){goto end;}
	ret=fread((char*)&(n),4,1,f);
	if(ret!=1){ret=-1;goto end;}
	if(n<0)
	{
		rb->version=-n;
		ret=fread((char*)&(n),4,1,f);
		if(ret!=1){ret=-1;goto end;}
	}else
	{
		rb->version=0;
	}
	for(i=0;i<n;++i)
	{
		item=wtk_rbin2_new_item(rb);//(wtk_rbin2_item_t*)wtk_heap_malloc(heap,sizeof(wtk_rbin2_item_t));
		ret=fread((char*)&(vi),4,1,f);
		if(ret!=1){ret=-1;goto end;}
		//wtk_debug("vi=%d\n",vi);
		item->fn=wtk_heap_dup_string(heap,0,vi);
		ret=fread(item->fn->data,item->fn->len,1,f);
		if(ret!=1){ret=-1;goto end;}
		wtk_rbin_reverse_data((unsigned char*)item->fn->data,item->fn->len);
		item->reverse=wtk_rbin2_is_file_reverse(rb,item->fn->data,item->fn->len);
		ret=fread((char*)&(vi),4,1,f);
		if(ret!=1){ret=-1;goto end;}
		item->pos=vi+seek_pos;
		ret=fread((char*)&(vi),4,1,f);
		if(ret!=1){ret=-1;goto end;}
		item->len=vi;
		//wtk_debug("[%.*s]=%d/%d\n",item->fn->len,item->fn->data,item->pos,item->len);
		item->data=NULL;
		wtk_queue_push(&(rb->list),&(item->q_n));
	}
	rb->fn=wtk_heap_dup_str(rb->heap,fn);
	ret=0;
end:
	if(ret!=0)
	{
		if(f)
		{
			fclose(f);
		}
	}else
	{
		rb->f=f;
	}
	//wtk_debug("ret=%d,f=%p/%p\n",ret,f,rb->f);
	return ret;
}

void wtk_rbin2_print(wtk_rbin2_t *rb)
{
	wtk_queue_node_t *qn;
	wtk_rbin2_item_t *item;
	int ki;

	for(ki=0,qn=rb->list.pop;qn;qn=qn->next,++ki)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
		wtk_debug("rb=%p/%p,v[%d]=[%.*s],pos=%d,len=%d\n",rb,qn,ki,item->fn->len,item->fn->data,item->pos,item->len);
	}
}

wtk_rbin2_item_t* wtk_rbin2_get_x(wtk_rbin2_t *rb,char *name,int len)
{
	wtk_queue_node_t *qn;
	wtk_rbin2_item_t *item;

	for(qn=rb->list.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
//		if(ki==116)
//		{
//			print_data(name,len);
//			print_data(item->fn->data,item->fn->len);
//		}
		if(wtk_string_cmp(item->fn,name,len)==0)
		{
			//wtk_debug("[%.*s]=%p\n",len,name,item);
			return item;
		}
	}
	return NULL;
}

FILE* wtk_rbin2_get_file(wtk_rbin2_t *rb,char *name)
{
	FILE *f=NULL;
	wtk_rbin2_item_t *item;
	int ret;

	item=wtk_rbin2_get(rb,name,strlen(name));
	if(!item){goto end;}
	f=fopen(rb->fn,"rb");
	if(!f){goto end;}
	ret=fseek(f,item->pos,SEEK_SET);
	if(ret!=0)
	{
		fclose(f);
		f=NULL;
	}
end:
	return f;
}

wtk_rbin2_item_t* wtk_rbin2_get(wtk_rbin2_t *rb,char *name,int len)
{
	wtk_rbin2_item_t *item;
	wtk_strbuf_t *buf;

	item=wtk_rbin2_get_x(rb,name,len);
	if(item){goto end;}
	buf=wtk_strbuf_new(256,1);
	wtk_strbuf_real_path(buf,name,len);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	item=wtk_rbin2_get_x(rb,buf->data,buf->pos);
	wtk_strbuf_delete(buf);
end:
	//wtk_debug("[%.*s]=%p\n",len,name,item);
	return item;
}

int wtk_rbin2_file_exist(wtk_rbin2_t *rb,char *name,int len)
{
	wtk_rbin2_item_t *item;

	item=wtk_rbin2_get(rb,name,len);
	return item?1:0;
}

wtk_rbin2_item_t* wtk_rbin2_get2(wtk_rbin2_t *rb,char *name,int len)
{
	wtk_rbin2_item_t *item;
	int ret;

	//wtk_debug("[%.*s]\n",len,name);
	item=wtk_rbin2_get(rb,name,len);
	if(item && !item->data)
	{
		ret=wtk_rbin2_load_item(rb,item,1);
		if(ret!=0)
		{
			return NULL;
		}
	}
	return item;
}

wtk_rbin2_item_t* wtk_rbin2_get3(wtk_rbin2_t *rb,char *name,int len,int use_heap)
{
	wtk_rbin2_item_t *item;
	int ret;

	//wtk_debug("[%.*s]\n",len,name);
	item=wtk_rbin2_get(rb,name,len);
	if(item && !item->data)
	{
		ret=wtk_rbin2_load_item(rb,item,use_heap);
		if(ret!=0)
		{
			wtk_debug("load failed\n");
			return NULL;
		}
	}
	return item;
}

void wtk_rbin2_item_release(wtk_rbin2_item_t *item)
{
	if(item->data)
	{
		wtk_free(item->data);
		item->data=NULL;
	}
}

int wtk_rbin2_load_item(wtk_rbin2_t *rb,wtk_rbin2_item_t *item,int use_heap)
{
	FILE *f=rb->f;
	int ret;

	//wtk_debug("[%.*s] pos=%d r=%d\n",item->fn->len,item->fn->data,item->pos,item->reverse);
	if(item->len==0){return 0;}
	ret=fseek(f,item->pos,SEEK_SET);
	if(ret!=0)
	{
		wtk_debug("seek failed %d f=%p\n",item->pos,f);
		perror(__FUNCTION__);
		exit(0);
		goto end;
	}
	if(use_heap)
	{
		item->data=wtk_heap_dup_string(rb->heap,0,item->len);
	}else
	{
		item->data=wtk_string_new(item->len);
	}
	ret=fread(item->data->data,item->len,1,f);
	if(ret!=1)
	{
		wtk_debug("read failed %d/%d\n",item->len,ret);
		ret=-1;goto end;
	}
	//print_data(item->data->data,10);
	//wtk_debug("[%.*s]=%d\n",item->fn->len,item->fn->data,item->reverse);
	//print_hex(item->data->data,4);
	if(item->reverse)
	{
		wtk_rbin_reverse_data((unsigned char*)item->data->data,item->data->len);
	}
	ret=0;
end:
	return ret;
}

void wtk_rbin2_item_clean(wtk_rbin2_item_t *item)
{
	if(item->data)
	{
		wtk_string_delete(item->data);
		item->data=NULL;
	}
}

int wtk_rbin2_extract(wtk_rbin2_t *rb,char *dn)
{
	wtk_queue_node_t *qn;
	wtk_rbin2_item_t *item;
	int ret=-1;
	wtk_strbuf_t *buf;

	//wtk_rbin2_print(rb);
	if(wtk_file_exist(dn)==0)
	{
		wtk_mkdir_p(dn,DIR_SEP,1);
	}
	buf=wtk_strbuf_new(1024,1);
	for(qn=rb->list.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_rbin2_item_t,q_n);
		if(!item->data) //item->len!=item->data->len)
		{
			ret=wtk_rbin2_load_item(rb,item,1);
			if(ret!=0)
			{
				wtk_debug("load [%.*s] failed\n",item->fn->len,item->fn->data);
				goto end;
			}
		}
		if (!item->data){
			//for null file.
			continue;
		}
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,dn,strlen(dn));
		wtk_strbuf_push_c(buf,'/');
		wtk_strbuf_push(buf,item->fn->data,item->fn->len);
		wtk_strbuf_push_c(buf,0);
		wtk_debug("write [%s]\n",buf->data);
		file_write_buf(buf->data,item->data->data,item->data->len);
	}
end:
	wtk_strbuf_delete(buf);
	return ret;
}


//------------------ load ------------------
/*
int wtk_rbin2_item_get(wtk_rbin2_item_t *i)
{
	if(i->seek_pos<i->data->len)
	{
		//for EOF check;
		return (unsigned char)i->data->data[i->seek_pos++];
	}else
	{
		return EOF;
	}
}

int wtk_rbin2_item_get2(wtk_rbin2_item_t* i,char *buf,int bytes)
{
	int len;

	len=i->data->len-i->seek_pos;
	if(len<bytes)
	{
		return EOF;
	}
	memcpy(buf,i->data->data+i->seek_pos,bytes);
	i->seek_pos+=bytes;
	return bytes;
}

wtk_string_t* wtk_rbin2_item_get3(wtk_rbin2_item_t* i)
{
	return i->data;
}

int wtk_rbin2_item_unget(wtk_rbin2_item_t *i,int c)
{
	if(i->seek_pos>0){--i->seek_pos;}
	return 0;
}*/

#define wtk_rbin2_item_get_c(i) ((i)->s<(i)->e)?*((i)->s++):EOF

int wtk_rbin2_item_get(wtk_rbin2_item_t *i)
{
	return (i->s<i->e)?*(i->s++):EOF;
}

int wtk_rbin2_item_get2(wtk_rbin2_item_t* i,char *buf,int bytes)
{
	if(i->e-i->s<bytes)
	{
		return EOF;
	}
	memcpy(buf,i->s,bytes);
	i->s+=bytes;
	return bytes;
}

#define SING_QUOTE '\''
#define DBL_QUOTE '"'
#define ESCAPE_CHAR '\\'
#include <ctype.h>

int wtk_rbin2_read_string(wtk_rbin2_item_t *item,wtk_strbuf_t *buf)
{
	char c;
	int ret=-1;
	int n;
	unsigned char *p;

	if(item->s>=item->e){goto end;}
	while(item->s<item->e)
	{
		c=*(item->s++);
		if(c=='\n' || c==' '|| c=='\t' || c=='\r' )
		{
		}else
		{
			break;
		}
	}
	if (item->s >= item->e){goto end;}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		p=item->s;
		while(item->s<item->e && (*item->s++!=c));
		n=item->s-p-1;
		if(n>0)
		{
			wtk_strbuf_push(buf,(char*)p,n);
		}
	}else
	{
		p=item->s-1;
		while(1)
		{
			if(c==ESCAPE_CHAR)
			{
				c=*(item->s++);
				if(c>='0' && c<='7')
				{
					n=item->s-p-1;
					if(n>0)
					{
						wtk_strbuf_push(buf,(char*)p,n);
					}

					n=c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=n;

					wtk_strbuf_push_c(buf,c);
					p=item->s;
				}
			}
			//wtk_strbuf_push_c(buf,c);
			if(item->s>=item->e){break;}
			c=*(item->s++);
			if(c==' '||c=='\n' || c=='\r' || c=='\t')
			{
				--item->s;
				break;
			}
		}
		n=item->s-p;
		if(n>0)
		{
			wtk_strbuf_push(buf,(char*)p,n);
		}
	}
	ret=0;
end:
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	//exit(0);
	return ret;
}

int wtk_rbin2_read_string3(wtk_rbin2_item_t *item,wtk_strbuf_t *buf)
{
	char c,q;
	int ret=-1;
	int n,len;
	unsigned char *p;

	if(item->s>=item->e){goto end;}
	while(item->s<item->e)
	{
		c=*(item->s++);
		if(c=='\n' || c==' '|| c=='\t' || c=='\r' )
		{
		}else
		{
			break;
		}
	}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		q=c;
		while(item->s<item->e)
		{
			c=*(item->s++);
			if(c==q)
			{
				break;
			}else
			{
				wtk_strbuf_push_c(buf,c);
			}
		}
	}else
	{
		p=item->s-1;
		while(1)
		{
			if(c==ESCAPE_CHAR)
			{
				c=*(item->s++);
				if(c>='0' && c<='7')
				{
					len=item->s-p-1;
					if(len>0)
					{
						wtk_strbuf_push(buf,(char*)p,len);
					}
					n=c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=*(item->s++);
					n=(n<<3)+c-'0';
					c=n;
					p=item->s;
				}
			}
			//wtk_strbuf_push_c(buf,c);
			if(item->s>=item->e)
			{
				len=item->s-p-1;
				if(len>0)
				{
					wtk_strbuf_push(buf,(char*)p,len);
				}
				break;
			}
			c=*(item->s++);
			if(c==' '||c=='\n' || c=='\r' ||c=='\t')
			{
				len=item->s-p-1;
				if(len>0)
				{
					wtk_strbuf_push(buf,(char*)p,len);
				}
				--item->s;
				break;
			}
		}
	}
	ret=0;
end:
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	//exit(0);
	return ret;
}

int wtk_rbin2_read_string2(wtk_rbin2_item_t *item,wtk_strbuf_t *buf)
{
	int isq,q=0,c,ret,n,i;
	//char t;

	ret=-1;
	c=wtk_rbin2_item_get_c(item);
	while(c=='\n' || c==' '|| c=='\r' ||c=='\t')
	{
		c=wtk_rbin2_item_get_c(item);
	}
	if(c==EOF){goto end;}
	if(c==DBL_QUOTE||c==SING_QUOTE)
	{
		isq=1;q=c;
		c=wtk_rbin2_item_get_c(item);
	}else
	{
		isq=0;
	}
	while(1)
	{
		//wtk_debug("%d:%c\n",c,c);
		if(c==EOF){ret=0;goto end;}
		if(isq)
		{
			if(c==q){break;}
		}else
		{
			if(c=='\n' || c==EOF||c==' ' || c=='\t' || c=='\r')
			{
				--item->s;
				break;
			}
		}
		if(c==ESCAPE_CHAR)
		{
			c=*(item->s++);
			if(c>='0' && c<='7')
			{
				n=c-'0';
				for(i=0;i<2;++i)
				{
					c=wtk_rbin2_item_get_c(item);
					if(c==EOF||c<'0'||c>'7'){goto end;}
					n=(n<<3)+c-'0';
				}
				c=n;
			}
		}
		wtk_strbuf_push_c(buf,c);
		c=wtk_rbin2_item_get_c(item);
	}
	ret=0;
end:
	return ret;
}

wtk_string_t* wtk_rbin2_item_get3(wtk_rbin2_item_t* i)
{
	return i->data;
}

int wtk_rbin2_item_unget(wtk_rbin2_item_t *i,int c)
{
	if(((char*)i->s)>i->data->data)
	{
		--i->s;
	}
	return 0;
}



void wtk_source_init_rbin2(wtk_source_t* s,wtk_rbin2_item_t *i)
{
	wtk_source_init(s);
	i->seek_pos=0;
	i->s=(unsigned char*)i->data->data;
	i->e=i->s+i->data->len;
	s->data=i;
	s->get=(wtk_source_get_handler_t)wtk_rbin2_item_get;
	s->unget=(wtk_source_unget_handler_t)wtk_rbin2_item_unget;
	s->get_str=(wtk_source_get_str_f)wtk_rbin2_item_get2;
	s->get_file=(wtk_source_get_file_f)wtk_rbin2_item_get3;
	s->read_str=(wtk_source_read_str_f)wtk_rbin2_read_string;
	//s->swap=0;//wtk_is_little_endian();
	s->swap=wtk_is_little_endian();
}

//-------------------------------------------------------------
int wtk_rbin2_item_get_x(wtk_rbin2_item_t *i)
{
	wtk_strbuf_t *buf=i->rb->buf;
	int len;
	int v;

	//wtk_debug("get %p pos=%d/%d\n",i,i->buf_pos,buf->pos);
	if(i->buf_pos==buf->pos)
	{
		i->seek_pos+=i->buf_pos;
		if(i->seek_pos>=i->len)
		{
			i->buf_pos=buf->pos=0;
			return EOF;
		}
		buf->pos=0;
		len=i->len-i->seek_pos;
		len=min(len,buf->length);
		//wtk_debug("i=%p/%p %d len=%d/%d\n",i,i->rb,len,i->len,i->seek_pos);
		buf->pos=fread(buf->data,1,len,i->rb->f);
		//wtk_debug("buf->pos=%d\n",buf->pos);
		//print_data(buf->data,buf->pos);
		if(buf->pos!=len)
		{
			return EOF;
		}
		if(i->reverse)
		{
			wtk_rbin_reverse_data((unsigned char*)buf->data,buf->pos);
		}
		i->buf_pos=0;
	}
	v=*(((unsigned char*)buf->data)+i->buf_pos);
	++i->buf_pos;
	return v;
}

int wtk_rbin2_item_get2_x(wtk_rbin2_item_t* i,char *p,int bytes)
{
	wtk_strbuf_t *buf=i->rb->buf;
	int len,ret;

	len=buf->pos-i->buf_pos;
	if(len>=bytes)
	{
		memcpy(p,buf->data+i->buf_pos,bytes);
		i->buf_pos+=bytes;
		return bytes;
	}else
	{
		if(len>0)
		{
			memcpy(p,buf->data+i->buf_pos,len);
			i->buf_pos+=len;
			bytes-=len;
			p+=len;
		}
		ret=wtk_rbin2_item_get_x(i);
		if(ret==EOF){return EOF;}
		--i->buf_pos;
		ret=wtk_rbin2_item_get2_x(i,p,bytes);
		if(ret==EOF){return EOF;}
		return ret+len;
	}
}


int wtk_rbin2_item_unget_x(wtk_rbin2_item_t *i,int c)
{
	if(i->buf_pos>0)
	{
		--i->buf_pos;
	}else
	{
		ungetc(c,i->rb->f);
	}
	return 0;
}



void wtk_source_init_rbin2_x(wtk_source_t* s,wtk_rbin2_item_t *i)
{
	fseek(i->rb->f,i->pos,SEEK_SET);
	wtk_source_init(s);
	wtk_strbuf_reset(i->rb->buf);
	i->seek_pos=0;
	i->buf_pos=0;
	s->data=i;
	s->get=(wtk_source_get_handler_t)wtk_rbin2_item_get_x;
	s->unget=(wtk_source_unget_handler_t)wtk_rbin2_item_unget_x;
	s->get_str=(wtk_source_get_str_f)wtk_rbin2_item_get2_x;
	s->get_file=NULL;
	//s->swap=0;//wtk_is_little_endian();
	s->swap=wtk_is_little_endian();
}



int wtk_rbin2_load_file(wtk_rbin2_t *rb,void *data,wtk_source_load_handler_t loader,char *fn)
{
//#define DEBUG_T
	wtk_source_t s,*ps=&s;
	wtk_rbin2_item_t *i;
	int ret=-1;
#ifdef DEBUG_T
	double t;
#endif

#ifdef DEBUG_T
	t=time_get_ms();
#endif
	i=wtk_rbin2_get(rb,fn,strlen(fn));
	if(!i)
	{
		wtk_debug("[%s] not found\n",fn);
		goto end;
	}
	if(!i->data)
	{
		ret=wtk_rbin2_load_item(rb,i,0);
		if(ret!=0)
		{
			wtk_debug("[%s] load failed\n",fn);
			goto end;
		}
	}
#ifdef DEBUG_T
	wtk_debug("time=%f file=%s len=%fKB\n",time_get_ms()-t,fn,i->len*1.0/1024);
#endif
	//wtk_ritem_print(i);
	wtk_source_init_rbin2(ps,i);
	ret=loader(data,ps);
	wtk_rbin2_item_release(i);
end:
#ifdef DEBUG_T
	wtk_debug("time=%f file=%s\n",time_get_ms()-t,fn);
#endif
	return ret;
}
