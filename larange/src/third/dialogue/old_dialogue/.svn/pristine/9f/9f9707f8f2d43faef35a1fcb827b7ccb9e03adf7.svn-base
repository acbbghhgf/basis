#include "wtk_clsvec_dat.h" 

wtk_clsvec2_item_t* wtk_clsvec2_dat_new_item(wtk_clsvec_dat_t *v,char *data,int pos)
{
	wtk_heap_t *heap=v->heap;
	wtk_clsvec2_item_t *item;

	item=(wtk_clsvec2_item_t*)wtk_heap_malloc(heap,sizeof(wtk_clsvec2_item_t));
	item->name=wtk_heap_dup_string(heap,data,pos);
	wtk_sfvec_init(&(item->vec));
	wtk_queue_push(&(v->item_q),&(item->q_n));
	return item;
}

void wtk_clsvec_dat_add_item_value(wtk_clsvec_dat_t *v,wtk_clsvec2_item_t *item,int pos,float value)
{
	wtk_heap_t *heap=v->heap;
	wtk_svec_item_t *si;

	si=(wtk_svec_item_t*)wtk_heap_malloc(heap,sizeof(wtk_svec_item_t));
	si->i=pos;
	si->v=value;
	wtk_sfvec_add_value(&(item->vec),si);
}

int wtk_clsvec_dat_load(wtk_clsvec_dat_t *dat,wtk_source_t *src)
{
	int ret;
	wtk_strbuf_t *buf;
	wtk_clsvec2_item_t *item;
	wtk_heap_t *heap=dat->heap;
	wtk_strkv_parser_t kp;
	int i,nl;
	float xv;
	char *p;
	int ki=0;
	unsigned long index=0;

	wtk_queue_init(&(dat->item_q));
	buf=wtk_strbuf_new(256,1);
	ret=wtk_source_read_line(src,buf);
	if(ret!=0){goto end;}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	wtk_strkv_parser_init(&(kp),buf->data,buf->pos);
	while(1)
	{
		ret=wtk_strkv_parser_next(&(kp));
		if(ret!=0){break;}
		if(wtk_str_equal_s(kp.k.data,kp.k.len,"vec"))
		{
			dat->vsize=wtk_str_atoi(kp.v.data,kp.v.len);
		}else
		{
			wtk_debug("unsported [%.*s]=[%.*s]\n",kp.k.len,kp.k.data,kp.v.len,kp.v.data);
			exit(0);
		}
	}
	//wtk_debug("v=%d\n",v->vsize);
	dat->idx_name=(wtk_string_t**)wtk_heap_malloc(heap,dat->vsize*sizeof(wtk_string_t*));
	for(i=0;i<dat->vsize;++i)
	{
		ret=wtk_source_read_line(src,buf);
		if(ret!=0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		dat->idx_name[i]=wtk_heap_dup_string(heap,buf->data,buf->pos);
	}
	while(1)
	{
		ret=wtk_source_read_line(src,buf);
		if(ret!=0 || buf->pos==0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		++ki;
		item=wtk_clsvec2_dat_new_item(dat,buf->data,buf->pos);
		item->index=index++;
		while(1)
		{
			ret=wtk_source_skip_sp(src,&nl);
			if(nl){break;}
			ret=wtk_source_read_string(src,buf);
			if(ret!=0){goto end;}
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			p=wtk_str_chr(buf->data,buf->pos,':');
			//wtk_debug("[%.*s]\n",(int)(p-buf->data),buf->data);
			i=wtk_str_atoi(buf->data,(int)(p-buf->data));
			//wtk_debug("i=%d\n",i);
			++p;
			//wtk_debug("[%.*s]\n",(int)(buf->data+buf->pos-p),p);
			xv=wtk_str_atof(p,(int)(buf->data+buf->pos-p));
			wtk_clsvec_dat_add_item_value(dat,item,i,xv);
		}
		wtk_sfvec_norm(&(item->vec));
		//wtk_debug("============ ki=%d [%.*s] mem=%.3fM len=%d\n",ki,item->name->len,item->name->data,wtk_heap_bytes(heap)*1.0/(1024*1024),item->vec.item_q.len);
		//wtk_sfvec_print(&(item->vec));
		//exit(0);
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}

wtk_clsvec_dat_t* wtk_clsvec_dat_new(char *fn)
{
	wtk_clsvec_dat_t *dat;

	dat=(wtk_clsvec_dat_t*)wtk_malloc(sizeof(wtk_clsvec_dat_t));
	dat->heap=wtk_heap_new(4096);
	dat->vsize=-1;
	dat->idx_name=NULL;
	wtk_queue_init(&(dat->item_q));
	wtk_source_load_file(dat,(wtk_source_load_handler_t)wtk_clsvec_dat_load,fn);
	return dat;
}

void wtk_clsvec_dat_delete(wtk_clsvec_dat_t *dat)
{
	wtk_heap_delete(dat->heap);
	wtk_free(dat);
}

wtk_clsvec2_item_t* wtk_clsvec_dat_get(wtk_clsvec_dat_t *dat,char *wrd,int wrd_len)
{
	wtk_clsvec2_item_t *item;
	wtk_queue_node_t *qn;

	for(qn=dat->item_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_clsvec2_item_t,q_n);
		if(wtk_string_cmp(item->name,wrd,wrd_len)==0)
		{
			return item;
		}
	}
	return NULL;
}

