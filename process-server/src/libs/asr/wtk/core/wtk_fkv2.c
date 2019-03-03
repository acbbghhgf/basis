#include "wtk_fkv2.h" 

int wtk_fkv2_load_hdr(wtk_fkv2_t *k)
{
	wtk_hash_str_node_t *node;
	FILE *f=k->f;
	char buf[256];
	int ret;
	unsigned char t;
	int i;

	if(k->f_of>0)
	{
		ret=fseek(f,k->f_of,SEEK_SET);
		if(ret!=0)
		{
			wtk_debug("seek %d failed\n",k->f_of);
			goto end;
		}
	}
	ret=fread(buf,4,1,f);
	if(ret!=1){ret=-1;goto end;}
	//wtk_debug("[%.*s]\n",4,buf);
	ret=fread(&(k->max),4,1,f);
	if(ret!=1){ret=-1;goto end;}
	ret=fread(&(k->step),4,1,f);
	if(ret!=1){ret=-1;goto end;}
	ret=fread(&(t),1,1,f);
	if(ret!=1){ret=-1;goto end;}
	switch(t)
	{
	case 1:
		k->type=WTK_FKV2_INT;
		break;
	case 2:
		k->type=WTK_FKV2_FLOAT;
		break;
	case 3:
		k->type=WTK_FKV2_STRING;
		break;
	}
	k->nslot=(k->max*1.0/k->step)+1;
	//wtk_debug("max=%d step=%d type=%d nslot=%d\n",k->max,k->step,t,k->nslot);
	k->char_map=wtk_str_hash_new(k->max+3);
	for(i=0;i<k->max;++i)
	{
		ret=fread(&(t),1,1,f);
		if(ret!=1){ret=-1;goto end;}
		//wtk_debug("t=%d\n",t);
		ret=fread(buf,1,t,f);
		if(ret!=t){ret=-1;goto end;}
		//wtk_debug("[%.*s]\n",t,buf);
		node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(k->char_map,buf,t,1);
		node->v.u=i;
	}
	k->slots=(unsigned int*)wtk_malloc(sizeof(unsigned int)*k->nslot);
	ret=fread(k->slots,sizeof(unsigned int),k->nslot,f);
	if(ret!=k->nslot){ret=-1;goto end;}
	k->slot_offset=ftell(f);
	ret=0;
end:
	return ret;
}

wtk_fkv2_t* wtk_fkv2_new3(FILE *f,int of,int len,int want_close)
{
	wtk_fkv2_t *k=NULL;
	int ret;

	k=(wtk_fkv2_t*)wtk_malloc(sizeof(wtk_fkv2_t));
	k->f=f;
	k->f_of=of;
	k->f_len=len;
	k->file_want_close=want_close;
	k->char_map=NULL;
	k->slots=NULL;
	k->buf=wtk_strbuf_new(256,1);
	ret=wtk_fkv2_load_hdr(k);
	if(ret!=0)
	{
		wtk_fkv2_delete(k);
		k=NULL;
	}
	return k;
}

wtk_fkv2_t* wtk_fkv2_new(char *fn)
{
	FILE *f;
	wtk_fkv2_t *k=NULL;

	f=fopen(fn,"rb");
	if(!f){goto end;}
	k=wtk_fkv2_new3(f,0,-1,1);
end:
	return k;
}


wtk_fkv2_t* wtk_fkv2_new2(wtk_rbin2_t *rbin,char *fn)
{
	wtk_rbin2_item_t *item;

	item=wtk_rbin2_get(rbin,fn,strlen(fn));
	if(!item)
	{
		wtk_debug("[%s] not found\n",fn);
		return NULL;
	}
	return wtk_fkv2_new3(rbin->f,item->pos,item->len,0);
}


void wtk_fkv2_delete(wtk_fkv2_t* k)
{
	wtk_strbuf_delete(k->buf);
	if(k->char_map)
	{
		wtk_str_hash_delete(k->char_map);
	}
	if(k->slots)
	{
		wtk_free(k->slots);
	}
	if(k->file_want_close && k->f)
	{
		fclose(k->f);
	}
	wtk_free(k);
}

void wtk_fkv_env_init(wtk_fkv2_t *k,wtk_fkv_env_t *env)
{
	env->depth=0;
	env->offset=0;
	if(k->f_of>0)
	{
		env->offset=k->f_of;
	}
	env->is_end=0;
	env->is_err=0;
}

void wtk_fkv_env_print(wtk_fkv_env_t *env)
{
	printf("depth:%d\n",env->depth);
	printf("offset:%d\n",env->offset);
	printf("float:%f\n",env->v.f);
	printf("end:%d\n",env->is_end);
	printf("err:%d\n",env->is_err);
}

int wtk_fkv2_get_slot(wtk_fkv2_t *kv,wtk_fkv_env_t *env,unsigned int offset,unsigned int idx)
{
	FILE *f=kv->f;
	unsigned int len,ko;
	unsigned char *data=NULL;
	unsigned char *s,*e,bytes;
	unsigned short id;
	unsigned char type;
	int ret;

	ret=fseek(f,offset,SEEK_SET);
	if(ret!=0)
	{
		wtk_debug("set of=%d failed\n",offset);
		goto end;
	}
	//wtk_debug("len=%d\n",len);
	ret=fread(&len,4,1,f);
	if(ret!=1)
	{
		perror(__FUNCTION__);
		wtk_debug("read cnt failed,ret=%d offset=%d idx=%d\n",ret,offset,idx);
		ret=-1;goto end;
	}
	data=wtk_malloc(len);
	ret=fread(data,1,len,f);
	if(ret!=len)
	{
		perror(__FUNCTION__);
		wtk_debug("read cnt failed,ret=%d offset=%d idx=%d\n",ret,offset,idx);
		exit(0);
		ret=-1;goto end;
	}
	s=data;e=s+len;
	//wtk_debug("len=%d\n",len);
	while(s<e)
	{
		//idata=struct.pack("<IHB",len(body_data),ki,t)
		//ko=*(unsigned int*)s;
    #ifndef USE_ARM
		id=((unsigned short*)s)[2];
		type=((unsigned char*)s)[6];
    #else
        memcpy(&id,s+4,2);
        memcpy(&type,s+6,1);
    #endif
		//wtk_debug("id=%d/%d type=%d\n",id,idx,type);
		if(id==idx)
		{
#ifndef USE_ARM
			ko=*(unsigned int*)s;
#else
            memcpy(&ko,s,sizeof(unsigned int));
#endif
			env->offset=offset+4+len+ko;
			//wtk_debug("id=%d\n",id);
			switch(type)
			{
			case 0:
				env->is_end=0;
				break;
			case 1:
				env->is_end=1;
#ifndef USE_ARM
				env->v.v=*((int*)(s+7));
#else
                memcpy(&(env->v.v),s+7,sizeof(unsigned int));
#endif
				break;
			case 2:
				env->is_end=1;
            #ifndef USE_ARM
				env->v.f=*((float*)(s+7));
            #else
                memcpy(&(env->v.f),s+7,sizeof(float));
            #endif
				break;
			case 3:
            #ifndef USE_ARM
				bytes=*(s+7);
            #else
                memcpy(&bytes,s+7,1);
            #endif
				//print_data((char*)(s+8+4),bytes-4);
				wtk_strbuf_reset(kv->buf);
				wtk_strbuf_push(kv->buf,(char*)(s+8),bytes);
				wtk_string_set(&(env->v.str),kv->buf->data,kv->buf->pos);
				env->is_end=1;
				break;
			}
			ret=0;
			goto end;
			break;
		}
		switch(type)
		{
		case 0:
			s+=7;
			break;
		case 1:
		case 2:
			s+=11;
			break;
		case 3:
			s+=7;
#ifndef USE_ARM
			s+=*s+1;
#else
            memcpy(&bytes,s,1);
            s=s+bytes+1;
#endif
			break;
		default:
			ret=-1;
			goto end;
			break;
		}
	}
	ret=-1;
end:
	if(data)
	{
		wtk_free(data);
	}
	return ret;
}

int wtk_fkv2_search_slot(wtk_fkv2_t *kv,wtk_fkv_env_t *e,unsigned int idx)
{
	unsigned int v;
	int i;

	i=(int)(idx/kv->step+0.5);
	v=kv->slots[i]+kv->slot_offset;
//	if(kv->f_of>0)
//	{
//		v+=kv->f_of;
//	}
	//wtk_debug("idx=%d i=%d\n",idx,i);
	return wtk_fkv2_get_slot(kv,e,v,idx);
}


int wtk_fkv2_search(wtk_fkv2_t *kv,wtk_fkv_env_t *e,unsigned int idx)
{
	int ret;

	if(e->depth==0)
	{
		ret=wtk_fkv2_search_slot(kv,e,idx);
	}else
	{
		ret=wtk_fkv2_get_slot(kv,e,e->offset,idx);
	}
	//wtk_debug("ret=%d\n",ret);
	++e->depth;
	return ret;
}

int wtk_fkv2_get(wtk_fkv2_t *kv,wtk_fkv_env_t *env,char *k,int k_bytes)
{
	wtk_hash_str_node_t *node;
	int ret=-1;

	//wtk_debug("[%.*s]\n",bytes,data);
	//wtk_fkv_env_init(env);
	env->is_end=0;
	node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(kv->char_map,k,k_bytes,0);
	if(!node)
	{
		//wtk_debug("[%.*s] not exist.\n",k_bytes,k);
		goto end;
	}
	ret=wtk_fkv2_search(kv,env,node->v.u);
end:
	if(ret!=0)
	{
		env->is_err=1;
	}
	return ret;
}

int wtk_fkv2_has(wtk_fkv2_t *kv,wtk_fkv_env_t *env,char *data,int bytes)
{
typedef enum
{
	WTK_FKV_INIT,
	WTK_FKV_ENG,
}wtk_fkv_state_t;
	char *s,*e;
	int n;
	wtk_fkv_state_t state;
	wtk_string_t v;
	int ret;

	//wtk_debug("[%.*s]\n",bytes,data);
	wtk_string_set(&(v),0,0);
	s=data;e=s+bytes;
	state=WTK_FKV_INIT;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		switch(state)
		{
		case WTK_FKV_INIT:
			if(n>1)
			{
				ret=wtk_fkv2_get(kv,env,s,n);
				if(ret!=0){goto end;}
			}else
			{
				if(s+n>=e)
				{
					ret=wtk_fkv2_get(kv,env,s,n);
					if(ret!=0){goto end;}
				}else
				{
					v.data=s;
					state=WTK_FKV_ENG;
				}
			}
			break;
		case WTK_FKV_ENG:
			if(n>1)
			{
				v.len=s-v.data;
				//wtk_debug("[%.*s]\n",v.len,v.data);
				//wtk_debug("[%.*s]\n",n,s);
				ret=wtk_fkv2_get(kv,env,v.data,v.len);
				if(ret!=0){goto end;}
				ret=wtk_fkv2_get(kv,env,s,n);
				if(ret!=0){goto end;}
				state=WTK_FKV_INIT;
			}else if(s+n>=e)
			{
				v.len=s-v.data;
				//wtk_debug("[%.*s]\n",v.len,v.data);
				ret=wtk_fkv2_get(kv,env,v.data,v.len);
				if(ret!=0){goto end;}
			}
			break;
		}
		s+=n;
	}
	ret=0;
end:
	return ret;
}

