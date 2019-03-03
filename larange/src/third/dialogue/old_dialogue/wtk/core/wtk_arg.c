#include <ctype.h>
#include "wtk_arg.h"
int wtk_arg_process(wtk_arg_t *arg,int argc,char** argv);

wtk_arg_t* wtk_arg_new(int argc,char** argv)
{
    wtk_arg_t *arg;

    arg=(wtk_arg_t*)wtk_malloc(sizeof(*arg));
    wtk_queue_init(&(arg->queue));
    arg->heap=wtk_heap_new(4096);
    arg->args=NULL;
    //arg->hash=wtk_str_hash_new(argc);
    wtk_arg_process(arg,argc,argv);
    return arg;
}

int wtk_arg_delete(wtk_arg_t *arg)
{
	wtk_heap_delete(arg->heap);
   // wtk_str_hash_delete(arg->hash);
    wtk_free(arg);
    return 0;
}

void wtk_arg_add_item(wtk_arg_t *arg,char *k,char *v)
{
	wtk_arg_item_t *item;

	//wtk_debug("%s:%s\n",k,v?v:"nil");
	if(v)
	{
		item=(wtk_arg_item_t*)wtk_heap_malloc(arg->heap,sizeof(*item));
		wtk_string_set(&(item->k),k,strlen(k));
		if(v)
		{
			wtk_string_set(&(item->v),v,strlen(v));
		}else
		{
			wtk_string_set(&(item->v),0,0);
		}
		wtk_queue_push(&(arg->queue),&(item->q_n));
	}else
	{
		if(!arg->args)
		{
			//wtk_debug("%s\n",k);
			arg->args=wtk_array_new_h(arg->heap,5,sizeof(char*));
		}
		wtk_array_push2(arg->args,&k);
	}
}

int wtk_arg_item_cmp(wtk_string_t *v,wtk_arg_item_t *i)
{
	return wtk_string_cmp(v,i->k.data,i->k.len);
}

wtk_string_t* wtk_arg_get_value(wtk_arg_t *arg,const char *k,int bytes)
{
	wtk_arg_item_t *item;
	wtk_string_t str;

	wtk_string_set(&(str),(char*)k,bytes);
	item=wtk_queue_find(&(arg->queue),offsetof(wtk_arg_item_t,q_n),(wtk_cmp_handler_t)wtk_arg_item_cmp,&str);
	if(item)
	{
		return &(item->v);
	}else
	{
		return 0;
	}
}

int wtk_arg_process(wtk_arg_t *arg,int argc,char** argv)
{
    typedef enum
    {
            ARG_KEY_WAIT,
            ARG_VALUE_WAIT
    }arg_state_t;
    //static char magic[]="-";
    //wtk_str_hash_t *hash;
    char *magic=0;
    char *k,*v,*p;
    int i;
    arg_state_t s;

    //hash=arg->hash;
    s=ARG_KEY_WAIT;
    k=v=p=0;
    for(i=0;i<argc;++i)
    {
        p=argv[i];
        switch(s)
        {
        case ARG_KEY_WAIT:
            if(*p=='-' && *(p+1)!=0)
            {
                k=p+1;
                if(i==argc-1)
                {
                    v=magic;
                    wtk_arg_add_item(arg,k,v);
                    //wtk_str_hash_add(hash,k,strlen(k),v);
                }else
                {
                    s=ARG_VALUE_WAIT;
                }
            }else if(i>0)
            {
            	wtk_arg_add_item(arg,p,0);
            }
            break;
        case ARG_VALUE_WAIT:
            if(*p=='-' && isalpha(*(p+1)))
            {
                v=magic;
                wtk_arg_add_item(arg,k,v);
                //wtk_str_hash_add(hash,k,strlen(k),v);
                k=p+1;
            }else
            {
                v=p;
                wtk_arg_add_item(arg,k,v);
                //wtk_str_hash_add(hash,k,strlen(k),v);
                s=ARG_KEY_WAIT;
            }
            break;
        }
    }
    return 0;
}

int wtk_arg_get_int(wtk_arg_t *arg,const char *key,int bytes,int* number)
{
    wtk_string_t *v;

    //v=(char*)wtk_str_hash_find(arg->hash,key,bytes);
    v=wtk_arg_get_value(arg,key,bytes);
    if(v && v->len>0)
    {
        *number=atoi(v->data);
    }
    return v?0:-1;
}

int wtk_arg_get_number(wtk_arg_t *arg,const char *key,int bytes,double *n)
{
    wtk_string_t *v;

    //v=(char*)wtk_str_hash_find(arg->hash,key,bytes);
    v=wtk_arg_get_value(arg,key,bytes);
    if(v && v->len>0)
    {
        *n=atof(v->data);
    }
    return v?0:-1;
}

int wtk_arg_get_str(wtk_arg_t *arg,const char *key,int bytes,char** pv)
{
    wtk_string_t *s;
    char *v;

    //v=(char*)wtk_str_hash_find(arg->hash,key,bytes);
    s=wtk_arg_get_value(arg,key,bytes);
    if(s && s->len>0)
    {
    	v=s->data;
    }else
    {
    	v=0;
    }
    *pv=v;
    return v?0:-1;
}

int wtk_arg_exist(wtk_arg_t *arg,const char* key,int bytes)
{
    wtk_string_t *v;

   // v=(char*)wtk_str_hash_find(arg->hash,key,bytes);
    v=wtk_arg_get_value(arg,key,bytes);
    return v?1:0;
}

void wtk_arg_print(wtk_arg_t *arg)
{
	wtk_queue_node_t *qn;
	wtk_arg_item_t *item;
	int i;
	char **s;

	for(qn=arg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_arg_item_t,q_n);
		if(item->k.len>0)
		{
			if(item->v.len>0)
			{
				printf("-%.*s %.*s ",item->k.len,item->k.data,item->v.len,item->v.data);
			}else
			{
				printf("%.*s ",item->k.len,item->k.data);
			}
		}
	}
	if(arg->args)
	{
		s=(char**)(arg->args->slot);
		for(i=0;i<arg->args->nslot;++i)
		{
			printf("%s ",s[i]);
		}
	}
	printf("\n");

}
