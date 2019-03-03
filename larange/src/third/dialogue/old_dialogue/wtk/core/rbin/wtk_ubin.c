#include "wtk_ubin.h"

wtk_ubin_t* wtk_ubin_new(unsigned int slot)
{
	wtk_ubin_t *b;

	b=wtk_malloc(sizeof(*b));
	if(!b){goto end;}
	memset(b,0,sizeof(*b));
	b->heap=wtk_heap_new(1024);
	b->hash=wtk_str_hash_new(slot);

end:
	return b;
}

void wtk_ubin_delete(wtk_ubin_t *b)
{
	wtk_heap_delete(b->heap);
	wtk_str_hash_delete(b->hash);
	wtk_free(b);
}

void wtk_ubin_reset(wtk_ubin_t *b)
{
	wtk_str_hash_reset(b->hash);
	wtk_heap_reset(b->heap);
}

wtk_ubin_item_t* wtk_ubin_find_item(wtk_ubin_t *b,wtk_string_t *fn)
{

	return (wtk_ubin_item_t*)wtk_str_hash_find(b->hash,fn->data,fn->len);
}

int wtk_ubin_read(wtk_ubin_t *b,char *fn,int read_data)
{
	FILE *f;
	int ret;
	wtk_ubin_item_t *item;
	int n,vi,i,pos;
	char attr;

	if(b->fn==0){b->fn=wtk_heap_dup_str(b->heap,fn);}
	f=fopen(fn,"rb");
	if(!f){ret=-1;goto end;}
	ret=fread((char*)&(n),4,1,f);	//num of item
	if(ret!=1){ret=-1;goto end;}
	ret=fread((char*)&(vi),4,1,f);	//size of item
	if(ret!=1){ret=-1;goto end;}
	b->item_num=n;
	b->item_len=vi;
	pos=4+4;
	for(i=0;i<n;++i)
	{
		item=(wtk_ubin_item_t*)wtk_ubin_new_item(b);
		memset(item,0,sizeof(*item));
		item->seek_pos=pos;
		ret=fread((char*)&(vi),1,4,f);	//len of item name
		if(ret!=4){ret=-1;goto end;}
		pos+=4;
		item->fn=wtk_heap_dup_string(b->heap,0,vi);
		ret=fread(item->fn->data,item->fn->len,1,f); //item name
		if(ret!=1){ret=-1;goto end;}
		wtk_rbin_reverse_data((unsigned char*)item->fn->data,item->fn->len);
		pos+=vi;
		ret=fread(&attr,1,1,f); //attr
		if(ret!=1){ret=-1;goto end;}
		item->attr=attr;
		pos+=1;
		if(read_data)
		{
			item->data=wtk_heap_dup_string(b->heap,0,b->item_len);	//data
			ret=fread(item->data->data,item->data->len,1,f);
			if(ret!=1){ret=-1;goto end;}
		}else
		{
			fseek(f,b->item_len,SEEK_CUR);
		}
		pos+=b->item_len;
		wtk_str_hash_add(b->hash,item->fn->data,item->fn->len,item);
	}

	ret=0;
end:
	if(f){fclose(f);}
	return ret;
}

int wtk_ubin_read_data(wtk_ubin_t *b,wtk_ubin_item_t *item)
{
	FILE *f=0;
	int ret=-1;

	if(item->seek_pos)
	{
		f=fopen(b->fn,"rb");
		if(!f){goto end;}
		fseek(f,item->seek_pos,SEEK_SET);
		item->data=wtk_heap_dup_string(b->heap,0,b->item_len);	//data
		ret=fread(item->data->data,1,item->data->len,f);
		if(ret!=item->data->len){ret=-1;goto end;}
	}

	ret=0;
end:
	if(f){fclose(f);}
	return ret;
}

int wtk_ubin_read_all_data(wtk_ubin_t *b)
{
	int ret=-1;
	FILE *f=0;
	wtk_queue_t **s,**e,*q;
	wtk_queue_node_t *qn;
	wtk_ubin_item_t *item;
	hash_str_node_t *str_node;
	int pos;

	f=fopen(b->fn,"rb");
	if(!f){goto end;}
	s=(b->hash->slot-1);
	e=s+b->hash->nslot+1;
	while(++s<e)
	{
		q=*s;
		if(!q){continue;}
		for(qn=q->pop;qn;qn=qn->next)
		{
			str_node=wtk_queue_node_data(qn,hash_str_node_t,n);
			item=str_node->value;
			if(item->data==0 && item->seek_pos)
			{
				pos=item->seek_pos+4+item->fn->len+1;
				fseek(f,pos,SEEK_SET);
				item->data=wtk_heap_dup_string(b->heap,0,b->item_len);	//data
				ret=fread(item->data->data,1,item->data->len,f);
				if(ret!=item->data->len){ret=-1;goto end;}
			}
		}
	}

end:
	if(f){fclose(f);}
	return ret;
}

int wtk_ubin_is_modify(wtk_ubin_t *b)
{
	wtk_queue_t **s,**e,*q;
	wtk_queue_node_t *qn;
	wtk_ubin_item_t *item;
	hash_str_node_t *str_node;
	s=(b->hash->slot-1);
	e=s+b->hash->nslot+1;
	while(++s<e)
	{
		q=*s;
		if(!q){continue;}
		for(qn=q->pop;qn;qn=qn->next)
		{
			str_node=wtk_queue_node_data(qn,hash_str_node_t,n);
			item=str_node->value;
			if(item->attr!=0)
			{
				return 1;
			}
		}
	}
	return 0;
}

int wtk_ubin_write_all(wtk_ubin_t *b,char *fn)	//write all item to file,cur file will be write cover
{
	int ret=-1,n;
	wtk_queue_t **s,**e,*q;
	wtk_queue_node_t *qn;
	wtk_ubin_item_t *item;
	hash_str_node_t *str_node;
	wtk_strbuf_t *buf=0;
	FILE *f=0;

	if(b->fn==0){b->fn=wtk_heap_dup_str(b->heap,fn);}
	if(b->item_num==0){ret=0;goto end;}
	if(!wtk_ubin_is_modify(b)){ret=0;goto end;}
	buf=wtk_strbuf_new(64,0);
	wtk_ubin_read_all_data(b);
	f=fopen(b->fn,"wb+");
	if(!f){goto end;}

	n=b->item_num;
	ret=fwrite(&(b->item_num),1,4,f);	//num of item
	if(ret!=4){ret=-1;goto end;}
	ret=fwrite(&(b->item_len),1,4,f);	//size of item
	if(ret!=4){ret=-1;goto end;}
	s=(b->hash->slot-1);
	e=s+b->hash->nslot+1;
	while(++s<e)
	{
		q=*s;
		if(!q){continue;}
		for(qn=q->pop;qn;qn=qn->next)
		{
			str_node=wtk_queue_node_data(qn,hash_str_node_t,n);
			item=str_node->value;
			if((item->attr & WTK_UBIN_ATTR_INVALID) || item->data==0 )
			{
				--n;
				continue;
			}
			ret=fwrite(&(str_node->key.len),1,4,f);	//len of name
			if(ret!=4){ret=-1;goto end;}
			wtk_strbuf_push(buf,str_node->key.data,str_node->key.len);
			wtk_rbin_reverse_data((unsigned char*)(buf->data),buf->pos);
			ret=fwrite(buf->data,1,buf->pos,f);	//item name
			if(ret!=buf->pos){ret=-1;goto end;}
			wtk_strbuf_reset(buf);
			ret=fwrite(&(item->attr),1,1,f);	//data attr
			if(ret!=1){ret=-1;goto end;}
			ret=fwrite(item->data->data,1,item->data->len,f);	//data
			if(ret!=item->data->len){ret=-1;goto end;}
		}
	}
	fseek(f,0,SEEK_SET);
	ret=fwrite(&n,1,4,f);	//num of item,rewrite
	if(ret!=4){ret=-1;goto end;}

	ret=0;
end:
	if(buf){wtk_strbuf_delete(buf);}
	if(ret==-1)
	{
		remove(b->fn);
	}
	if(f)
	{
		fclose(f);
	}
	return ret;
}

int wtk_ubin_write_head(wtk_ubin_t *b,char *fn,unsigned int item_num,unsigned int item_len)	//write all item to file,cur file will be write cover
{
	int ret=-1;
	FILE *f;

	if(b->fn==0){b->fn=wtk_heap_dup_str(b->heap,fn);}
	f=fopen(b->fn,"wb");
	if(f==0){goto end;}
	fseek(f,0,SEEK_SET);
	ret=fwrite(&(item_num),1,4,f);	//num of item
	if(ret!=4){ret=-1;goto end;}
	ret=fwrite(&(item_len),1,4,f);	//size of item
	if(ret!=4){ret=-1;goto end;}
	b->item_len=item_len;

end:
	if(f){fclose(f);}
	return ret;
}

int wtk_ubin_append(wtk_ubin_t *b,wtk_string_t *fn,wtk_string_t *data,char attr)
{
	wtk_ubin_item_t *item;
	int ret=-1;
	int pos;
	wtk_strbuf_t *buf;
	FILE *f;

	buf=wtk_strbuf_new(64,0);
	if(b->item_num==0){wtk_ubin_write_head(b,0,0,data->len);}
	f=fopen(b->fn,"rb+");
	if(f==0){goto end;}
	item=wtk_ubin_find_item(b,fn);
	if(item)
	{
		item->attr=attr;
		item->data=0;
		pos=item->seek_pos+4+item->fn->len;
		fseek(f,pos,SEEK_SET);
		ret=fwrite(&(attr),1,1,f);
		if(ret!=1){ret=-1;goto end;}
		ret=fwrite(data->data,1,data->len,f);
		if(ret!=data->len){ret=-1;goto end;}

		ret=0;
		goto end;
	}
	item=wtk_ubin_add_item(b,fn,0,attr);
	fseek(f,0,SEEK_END);
	item->seek_pos=ftell(f);
	ret=fwrite(&(fn->len),1,4,f);
	if(ret!=4){ret=-1;goto end;}
	wtk_strbuf_push(buf,fn->data,fn->len);
	wtk_rbin_reverse_data((unsigned char*)(buf->data),buf->pos);
	ret=fwrite(buf->data,1,buf->pos,f);
	if(ret!=buf->pos){ret=-1;goto end;}
	ret=fwrite(&(attr),1,1,f);
	if(ret!=1){ret=-1;goto end;}
	ret=fwrite(data->data,1,data->len,f);
	if(ret!=data->len){ret=-1;goto end;}
	b->item_num++;
	fseek(f,0,SEEK_SET);
	ret=fwrite(&(b->item_num),1,4,f);	//num of item,rewrite
	if(ret!=4){ret=-1;goto end;}

end:
	wtk_strbuf_delete(buf);
	if(f){fclose(f);}
	return ret;
}

wtk_ubin_item_t* wtk_ubin_add_item(wtk_ubin_t *b,wtk_string_t *fn,wtk_string_t *data,char attr)
{
	wtk_ubin_item_t *item;

	item=wtk_ubin_new_item(b);
	if(!item){goto end;}
	item->seek_pos=0;
	item->fn=wtk_heap_dup_string(b->heap,fn->data,fn->len);
	if(data)
	{
		item->data=wtk_heap_dup_string(b->heap,data->data,data->len);
	}else
	{
		item->data=0;
	}
	item->attr=attr;
	wtk_str_hash_add(b->hash,item->fn->data,item->fn->len,item);

end:
	return item;
}

int wtk_ubin_write_item(wtk_ubin_t *b,FILE *f,wtk_ubin_item_t *item)
{
	int ret=-1;

//	f=fopen(b->fn,"rb+");
	if(item->seek_pos)
	{
		fseek(f,item->seek_pos,SEEK_SET);
	}else
	{
		fseek(f,0,SEEK_END);
		item->seek_pos=ftell(f);
	}
	ret=fwrite(&(item->fn->len),1,4,f);
	if(ret!=4){ret=-1;goto end;}
	ret=fwrite(item->fn->data,1,item->fn->len,f);
	if(ret!=item->fn->len){ret=-1;goto end;}
	ret=fwrite(&(item->attr),1,1,f);
	if(ret!=1){ret=-1;goto end;}
	ret=fwrite(item->data->data,1,item->data->len,f);
	if(ret!=item->data->len){ret=-1;goto end;}

end:
	return ret;
}

int wtk_ubin_write_data(wtk_ubin_t *b,FILE *f,wtk_ubin_item_t *item)
{
	int ret=-1;
	int pos;

	//	f=fopen(b->fn,"rb+");
	pos=item->seek_pos+4+item->fn->len+1;
	fseek(f,pos,SEEK_SET);
	ret=fwrite(item->data->data,1,item->data->len,f);
	if(ret!=item->data->len){ret=-1;goto end;}

end:
	return ret;
}

int wtk_ubin_write_attr(wtk_ubin_t *b,FILE *f,wtk_ubin_item_t *item)
{
	int ret=-1;
	int pos;

	//	f=fopen(b->fn,"rb+");
	pos=item->seek_pos+4+item->fn->len;
	fseek(f,pos,SEEK_SET);
	ret=fwrite(&(item->attr),1,1,f);
	if(ret!=1){ret=-1;goto end;}

end:
	return ret;
}
