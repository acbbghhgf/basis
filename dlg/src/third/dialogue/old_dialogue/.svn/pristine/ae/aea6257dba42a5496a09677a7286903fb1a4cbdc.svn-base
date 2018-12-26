#include "wtk_fkv.h"


wtk_string_t wtk_fkv_get_item_str(wtk_fkv_t *kv,uint32_t of,char *k,int k_len)
{
	FILE *f=kv->f;
	int ret;
	int vi;
	unsigned char bi;
	wtk_strbuf_t *buf=kv->buf;
	unsigned char *s,*e;
	unsigned int ti;
	int b=0;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	//wtk_debug("of=%d\n",of);
	ret=fseek(f,of,SEEK_SET);
	if(ret!=0)
	{
		wtk_debug("seek %d failed\n",of);
		goto end;
	}
	ret=fread((char*)&vi,1,4,f);
	if(ret!=4)
	{
		wtk_debug("read len failed\n");
		goto end;
	}
	//wtk_debug("vi=%d\n",vi);
	if(buf->length<vi)
	{
		wtk_strbuf_expand(buf,vi);
	}
	ret=fread(buf->data,1,vi,f);
	if(ret!=vi)
	{
		perror(__FUNCTION__);
		wtk_debug("read data failed(of=%d,%d/%d)\n",of,vi,ret);
		exit(0);
		ret=-1;goto end;
	}
	s=(unsigned char*)(buf->data);
	e=s+vi;
	//print_data(buf->data,vi);
	//wtk_debug("%p=%p,%d,type=%d\n",s,e,vi,kv->type);
	while(s<e)
	{
#ifndef USE_ARM
		bi=*(s++);
#else
		memcpy(&bi,s,1);
		++s;
#endif
		if(wtk_str_equal(k,k_len,(char*)s,bi))
		{
			b=1;
		}
		
		if((!k)||(k_len==0))
		{
			b=1;
		}
		s+=bi;
		//wtk_debug("[%.*s]\n",v->len,v->data);
		switch(kv->type)
		{
		case WTK_FKV_INT:
#ifndef USE_ARM
			vi=*((int*)s);
#else
			memcpy(&vi,s,sizeof(int));
#endif
			if(b)
			{
				goto end;
			}
			s+=sizeof(int);
			//print_data(v->data,v->len);
			//wtk_debug("vi=%d\n",vi);
			break;
		case WTK_FKV_BIN:
			if(kv->str_use_byte)
			{
#ifndef USE_ARM
			ti=*(s++);
#else
			ti=0;
			memcpy(&ti,s,1);
			++s;
#endif
			}else
			{
#ifndef USE_ARM
				ti=*((int*)s);
#else
				memcpy(&ti,s,sizeof(int));
#endif
				s+=sizeof(int);
			}
			if(b)
			{
				wtk_string_set(&(v),(char*)s,ti);
				goto end;
			}
			s+=ti;
			break;
		case WTK_FKV_LSTRING:
#ifndef USE_ARM
				ti=*((int*)s);
#else
				memcpy(&ti,s,sizeof(int));
#endif
				s+=sizeof(int);
			if(b)
			{
				wtk_string_set(&(v),(char*)s,ti);
				goto end;
			}
			s+=ti;
			break;
		}
	}
end:
	//wtk_debug("ret=%d\n",ret);
	return v;
}


int wtk_fkv_load_item(wtk_fkv_t *kv,uint32_t of)
{
	FILE *f=kv->f;
	int ret;
	int vi;
	unsigned char bi;
	wtk_string_t *v,*v1;
	wtk_strbuf_t *buf=kv->buf;
	unsigned char *s,*e;
	unsigned int ti;

	//wtk_debug("of=%d\n",of);
	ret=fseek(f,of,SEEK_SET);
	if(ret!=0)
	{
		wtk_debug("seek %d failed\n",of);
		goto end;
	}
	ret=fread((char*)&vi,1,4,f);
	if(ret!=4)
	{
		wtk_debug("read len failed\n");
		goto end;
	}
	//wtk_debug("vi=%d\n",vi);
	if(buf->length<vi)
	{
		wtk_strbuf_expand(buf,vi);
	}
	ret=fread(buf->data,1,vi,f);
	if(ret!=vi)
	{
		perror(__FUNCTION__);
		wtk_debug("read data failed(of=%d,%d/%d)\n",of,vi,ret);
		exit(0);
		ret=-1;goto end;
	}
	s=(unsigned char*)(buf->data);
	e=s+vi;
	//print_data(buf->data,vi);
	//wtk_debug("%p=%p,%d,type=%d\n",s,e,vi,kv->type);
	while(s<e)
	{
#ifndef USE_ARM
		bi=*(s++);
#else
		memcpy(&bi,s,1);
		++s;
#endif
		v=wtk_heap_dup_string(kv->hash->heap,(char*)s,bi);
		s+=bi;
		//wtk_debug("[%.*s]\n",v->len,v->data);
		switch(kv->type)
		{
		case WTK_FKV_INT:
#ifndef USE_ARM
			vi=*((int*)s);
#else
			memcpy(&vi,s,sizeof(int));
#endif
			s+=sizeof(int);
			//print_data(v->data,v->len);
			//wtk_debug("vi=%d\n",vi);
			wtk_str_hash_add(kv->hash,v->data,v->len,(void*)(long)vi);
			break;
		case WTK_FKV_BIN:
			if(kv->str_use_byte)
			{
#ifndef USE_ARM
			ti=*(s++);
#else
			ti=0;
			memcpy(&ti,s,1);
			++s;
#endif
			}else
			{
#ifndef USE_ARM
				ti=*((int*)s);
#else
				memcpy(&ti,s,sizeof(int));
#endif
				s+=sizeof(int);
			}
			//ti=*((char*)s);
			//s+=4;
			v1=wtk_heap_dup_string(kv->hash->heap,(char*)s,ti);
			//wtk_debug("[%.*s]\n",v->len,v->data);
			//print_data(v1->data,v1->len);
			wtk_str_hash_add(kv->hash,v->data,v->len,v1);
			s+=ti;
			break;
		case WTK_FKV_LSTRING:
#ifndef USE_ARM
				ti=*((int*)s);
#else
				memcpy(&ti,s,sizeof(int));
#endif
				s+=sizeof(int);
			//ti=*((char*)s);
			//s+=4;
			v1=wtk_heap_dup_string(kv->hash->heap,(char*)s,ti);
			//wtk_debug("[%.*s]\n",v->len,v->data);
			//print_data(v1->data,v1->len);
			wtk_str_hash_add(kv->hash,v->data,v->len,v1);
			s+=ti;
			break;
		}
	}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}


int wtk_fkv_load_idx(wtk_fkv_t *kv)
{
	char buf[4];
	FILE *f;
	int ret;
	int vi;
	int i;

	f=kv->f;
	if(kv->f_of>0)
	{
		ret=fseek(f,kv->f_of,SEEK_SET);
		if(ret!=0)
		{
			wtk_debug("seek %d failed\n",kv->f_of);
			goto end;
		}
	}
	ret=fread(buf,1,4,f);
	if(ret!=4)
	{
		wtk_debug("read hdr failed\n");
		ret=-1;
		goto end;
	}
	if(wtk_str_equal_s(buf,4,"KV+0"))
	{
		kv->str_use_byte=1;
	}else if(wtk_str_equal_s(buf,4,"KV+1"))
	{
		kv->str_use_byte=0;
	}else
	{
		print_data(buf,4);
		ret=-1;
		goto end;
	}
	//wtk_debug("[%.*s]\n",4,buf);
	//read type
	ret=fread((char*)&(vi),1,4,f);
	if(ret!=4)
	{
		wtk_debug("read type failed\n");
		ret=-1;
		goto end;
	}
	//wtk_debug("vi=%d\n",vi);
	kv->type=vi;

	ret=fread((char*)&(vi),1,4,f);
	if(ret!=4)
	{
		wtk_debug("read hdr len failed\n");
		ret=-1;
		goto end;
	}
	//wtk_debug("vi=%d\n",vi);
	kv->nid=vi;
	kv->idx=wtk_calloc(vi,sizeof(unsigned int));

	//wtk_debug("%p of=%d nid=%d\n",f,kv->f_of,vi);
	for(i=0;i<kv->nid;++i)
	{
		ret=fread((char*)&(vi),1,4,f);
		if(ret!=4)
		{
			wtk_debug("read hdr k of failed(%d/%d)\n",i,kv->nid);
			ret=-1;
			goto end;
		}
		kv->idx[i]=vi;
		//wtk_debug("vi=%d\n",vi);
	}
	kv->data_offset=ftell(f);
	//exit(0);
	ret=0;
end:
	if(ret!=0)
	{
		wtk_debug("load idx failed\n");
		perror(__FUNCTION__);
		if(kv->file_want_close)
		{
			fclose(f);
		}
	}else
	{
		kv->f=f;
	}
	//exit(0);
	return ret;
}


wtk_fkv_t* wtk_fkv_new3(char *fn)
{
	return wtk_fkv_new(fn,1703);
}

wtk_fkv_t* wtk_fkv_new4(wtk_rbin2_t *rbin,char *fn,int hash_hint)
{
	wtk_rbin2_item_t *item;

	item=wtk_rbin2_get(rbin,fn,strlen(fn));
	if(!item)
	{
		wtk_debug("[%s] not found\n",fn);
		return NULL;
	}
	return wtk_fkv_new2(hash_hint,rbin->f,item->pos,item->len,0);
}


wtk_fkv_t* wtk_fkv_new(char *fn,int hash_hint)
{
	FILE *f;

	f=fopen(fn,"rb");
	if(!f){return NULL;}
	//wtk_debug("%s\n",fn);
	return wtk_fkv_new2(hash_hint,f,0,-1,1);
}

wtk_fkv_t* wtk_fkv_new2(int hash_hint,FILE *f,int of,int len,int file_want_close)
{
	wtk_fkv_t *kv;
	int ret;

	kv=(wtk_fkv_t*)wtk_malloc(sizeof(wtk_fkv_t));
	//wtk_debug("kv=%p\n",kv);
	kv->buf=wtk_strbuf_new(1024,1);
	kv->hash=wtk_str_hash_new(hash_hint);
	kv->f_of=of;
	kv->f_len=len;
	kv->f=f;
	kv->idx=NULL;
	kv->file_want_close=file_want_close;
	kv->str_use_byte=1;
	ret=wtk_fkv_load_idx(kv);
	if(ret!=0){goto end;}
	ret=0;
end:
	if(ret!=0)
	{
		wtk_fkv_delete(kv);
		kv=NULL;
	}
	//wtk_debug("new kv=%p\n",kv);
	return kv;
}

void wtk_fkv_delete(wtk_fkv_t *kv)
{
	//wtk_debug("delete kv=%p\n",kv);
	if(kv->buf)
	{
		wtk_strbuf_delete(kv->buf);
	}
	if(kv->file_want_close && kv->f)
	{
		fclose(kv->f);
	}
	if(kv->idx)
	{
		wtk_free(kv->idx);
	}
	wtk_str_hash_delete(kv->hash);
	wtk_free(kv);
}

void wtk_fkv_load_all(wtk_fkv_t *kv)
{
	int i;

	for(i=0;i<kv->nid;++i)
	{
		if(kv->idx[i]<1)
		{
			continue;
		}
		wtk_fkv_load_item(kv,kv->idx[i]+kv->data_offset);
	}
}

void wtk_fkv_reset(wtk_fkv_t *kv)
{
	wtk_str_hash_reset(kv->hash);
}

int wtk_fkv_get_int(wtk_fkv_t *kv,char *k,int k_len,int *found)
{
	hash_str_node_t *node;
	wtk_str_hash_t *hash=kv->hash;
	uint32_t rv,rv1;
	int ret;

	//wtk_debug("[%.*s]\n",k_len,k);
	node=wtk_str_hash_find_node(hash,k,k_len,&rv);
	if(node){goto end;}
	rv1=hash_string_value_len(k,k_len,kv->nid);
	//wtk_debug("rv1=%d\n",rv1);
	//wtk_debug("[%.*s]=%d\n",k_len,k,rv1);
	//exit(0);
	if(kv->idx[rv1]<1)
	{
		goto end;
	}
	ret=wtk_fkv_load_item(kv,kv->idx[rv1]+kv->data_offset);
	if(ret!=0){goto end;}
	//wtk_debug("%d/%d\n",hash_string_value_len(k,k_len,hash->nslot),rv);
	node=wtk_str_hash_find_node2(hash,k,k_len,rv);
	//wtk_debug("node=%p\n",node);
end:
	if(found)
	{
		*found=node?1:0;
	}
	return node?((long)node->value):-1;
}

wtk_string_t wtk_fkv_get_str2(wtk_fkv_t *kv,char *k,int k_len)
{
	uint32_t rv1;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	rv1=hash_string_value_len(k,k_len,kv->nid);
	//wtk_debug("[%.*s]=%d,%d\n",k_len,k,rv1,kv->nid);
	if(kv->idx[rv1]<1)
	{
		//wtk_debug("miss [%.*s]\n",k_len,k);
		goto end;
	}
	//wtk_debug("%d/%d/%d\n",rv1,kv->idx[rv1],kv->data_offset);
	v=wtk_fkv_get_item_str(kv,kv->idx[rv1]+kv->data_offset,k,k_len);
end:
	return v;
}

wtk_string_t* wtk_fkv_get_str(wtk_fkv_t *kv,char *k,int k_len)
{
	hash_str_node_t *node;
	wtk_str_hash_t *hash=kv->hash;
	uint32_t rv,rv1;
	int ret;

	//wtk_debug("[%.*s]\n",k_len,k);
	node=wtk_str_hash_find_node(hash,k,k_len,&rv);
	if(node){goto end;}
	rv1=hash_string_value_len(k,k_len,kv->nid);
	//wtk_debug("[%.*s]=%d,%d\n",k_len,k,rv1,kv->nid);
	if(kv->idx[rv1]<1)
	{
		//wtk_debug("miss [%.*s]\n",k_len,k);
		goto end;
	}
	//wtk_debug("%d/%d/%d\n",rv1,kv->idx[rv1],kv->data_offset);
	ret=wtk_fkv_load_item(kv,kv->idx[rv1]+kv->data_offset);
	if(ret!=0){goto end;}
	//wtk_debug("%d/%d\n",hash_string_value_len(k,k_len,hash->nslot),rv);
	node=wtk_str_hash_find_node2(hash,k,k_len,rv);
	//wtk_debug("node=%p\n",node);
end:
	return node?((wtk_string_t*)node->value):NULL;
}

wtk_string_t* wtk_fkv_get_int_key(wtk_fkv_t *kv,int id)
{
	wtk_queue_node_t *qn;
	hash_str_node_t *n;
	wtk_queue_t *q;
	int i;

	for(i=0;i<kv->hash->nslot;++i)
	{
		q=kv->hash->slot[i];
		if(!q)// || q->length==0)
		{
			continue;
		}
		for(qn=q->pop;qn;qn=qn->next)
		{
			n=(hash_str_node_t*)data_offset2(qn,hash_str_node_t,n);
			//n=(hash_str_node_t*)wtk_queue_node_data(qn,hash_str_node_t,n);
			if(((long)n->value)==id)
			{
				return &(n->key);
			}
		}
	}
	return NULL;
}
