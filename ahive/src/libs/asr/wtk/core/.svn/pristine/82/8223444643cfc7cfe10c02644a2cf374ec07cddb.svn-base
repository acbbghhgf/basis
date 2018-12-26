#include "wtk_if.h" 
#include "wtk/core/wtk_str_encode.h"

wtk_if_t* wtk_if_new2(wtk_heap_t *heap)
{
	wtk_if_t *xi;

	xi=(wtk_if_t*)wtk_heap_malloc(heap,sizeof(wtk_if_t));
	wtk_queue3_init(&(xi->item_q));
	return xi;
}

wtk_if_item_t* wtk_if_item_new(wtk_heap_t *heap)
{
	wtk_if_item_t *xi;

	xi=(wtk_if_item_t*)wtk_heap_malloc(heap,sizeof(wtk_if_item_t));
	xi->k=NULL;
	xi->v.str=NULL;
	xi->is_str=1;
	xi->type=WTK_IF_EQ;
	return xi;
}

wtk_if_t* wtk_if_new(wtk_heap_t *heap,char *data,int len)
{
typedef enum
{
	WTK_IF_STATE_INIT,
	WTK_IF_STATE_K,
	WTK_IF_STATE_WAIT_SYM,
	WTK_IF_STATE_SYM,
	WTK_IF_STATE_WAIT_V,
	WTK_IF_STATE_V,
	WTK_IF_STATE_V_QUOTE,
}wtk_if_state_t;
	char *s,*e;
	int n;
	wtk_if_state_t state;
	wtk_string_t k;
	char c;
	wtk_if_item_t *xi=NULL;
	wtk_if_t *xif;

	wtk_string_set(&(k),0,0);
	xif=wtk_if_new2(heap);
	s=data;
	e=s+len;
	state=WTK_IF_STATE_INIT;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		//wtk_debug("%d [%.*s]\n",state,n,s);
		switch(state)
		{
		case WTK_IF_STATE_INIT:
			if(n>1 || (!isspace(*s)&&*s!=','))
			{
				k.data=s;
				state=WTK_IF_STATE_K;
				xi=wtk_if_item_new(heap);
			}
			break;
		case WTK_IF_STATE_K:
			if(n==1)
			{
				c=*s;
				switch(c)
				{
				case '<':
					k.len=s-k.data;
					xi->k=wtk_heap_dup_string(heap,k.data,k.len);
					xi->type=WTK_IF_LT;
					state=WTK_IF_STATE_SYM;
					break;
				case '=':
					k.len=s-k.data;
					xi->k=wtk_heap_dup_string(heap,k.data,k.len);
					xi->type=WTK_IF_EQ;
					state=WTK_IF_STATE_WAIT_V;
					break;
				case '>':
					k.len=s-k.data;
					xi->k=wtk_heap_dup_string(heap,k.data,k.len);
					xi->type=WTK_IF_GT;
					state=WTK_IF_STATE_SYM;
					break;
				case '!':
					//wtk_debug("not eq\n");
					k.len=s-k.data;
					xi->k=wtk_heap_dup_string(heap,k.data,k.len);
					xi->type=WTK_IF_NEQ;
					state=WTK_IF_STATE_SYM;
					break;
				default:
					if(isspace(c))
					{
						k.len=s-k.data;
						xi->k=wtk_heap_dup_string(heap,k.data,k.len);
						state=WTK_IF_STATE_WAIT_SYM;
					}
					break;
				}
			}
			break;
		case WTK_IF_STATE_WAIT_SYM:
			if(n==1)
			{
				c=*s;
				switch(c)
				{
				case '<':
					xi->type=WTK_IF_LT;
					state=WTK_IF_STATE_SYM;
					break;
				case '=':
					xi->type=WTK_IF_EQ;
					state=WTK_IF_STATE_WAIT_V;
					break;
				case '>':
					xi->type=WTK_IF_GT;
					state=WTK_IF_STATE_SYM;
					break;
				case '!':
					xi->type=WTK_IF_NEQ;
					state=WTK_IF_STATE_SYM;
					break;
				default:
					break;
				}
			}
			break;
		case WTK_IF_STATE_SYM:
			if(n==1 && *s=='=')
			{
				switch(xi->type)
				{
				case WTK_IF_LT:
					xi->type=WTK_IF_LET;
					break;
				case WTK_IF_GT:
					xi->type=WTK_IF_GET;
					break;
				default:
					break;
				}
			}else
			{
				if(n>1 || !isspace(*s))
				{
					if(*s=='"')
					{
						k.data=s+1;
						state=WTK_IF_STATE_V_QUOTE;
					}else
					{
						k.data=s;
						state=WTK_IF_STATE_V;
					}
				}else
				{
					state=WTK_IF_STATE_WAIT_V;
				}
			}
			break;
		case WTK_IF_STATE_WAIT_V:
			if(n>1 || !isspace(*s))
			{
				if(*s=='"')
				{
					k.data=s+1;
					state=WTK_IF_STATE_V_QUOTE;
				}else
				{
					k.data=s;
					state=WTK_IF_STATE_V;
				}
			}
			break;
		case WTK_IF_STATE_V:
			if(n==1 && (isspace(*s)|| *s==','))
			{
				k.len=s-k.data;
				//wtk_debug("[%.*s]\n",k.len,k.data);
				//exit(0);
				if(isdigit(k.data[0]))
				{
					xi->is_str=0;
					xi->v.number=wtk_str_atof(k.data,k.len);
				}else
				{
					xi->is_str=1;
					xi->v.str=wtk_heap_dup_string(heap,k.data,k.len);
				}
				wtk_queue3_push(&(xif->item_q),&(xi->q_n));
				state=WTK_IF_STATE_INIT;
			}
			break;
		case WTK_IF_STATE_V_QUOTE:
			if(n==1 && *s=='"')
			{
				k.len=s-k.data;
				//wtk_debug("[%.*s]\n",k.len,k.data);
				//exit(0);
#ifdef WIN32
				if (isdigit((unsigned char)(k.data[0])))
#else
				if(isdigit(k.data[0]))
#endif
				{
					xi->is_str=0;
					xi->v.number=wtk_str_atof(k.data,k.len);
				}else
				{
					xi->is_str=1;
					xi->v.str=wtk_heap_dup_string(heap,k.data,k.len);
				}
				wtk_queue3_push(&(xif->item_q),&(xi->q_n));
				state=WTK_IF_STATE_INIT;
			}
			break;
		}
		s+=n;
	}
	if(state==WTK_IF_STATE_V)
	{
		k.len=s-k.data;
		//wtk_debug("[%.*s]\n",k.len,k.data);
#ifdef WIN32
		if (isdigit((unsigned char)(k.data[0])))
#else
		if(isdigit(k.data[0]))
#endif
		{
			xi->is_str=0;
			xi->v.number=wtk_str_atof(k.data,k.len);
		}else
		{
			xi->is_str=1;
			xi->v.str=wtk_heap_dup_string(heap,k.data,k.len);
		}
		wtk_queue3_push(&(xif->item_q),&(xi->q_n));
	}
	return xif;
}

void wtk_if_print(wtk_if_t *xif)
{
	wtk_queue_node_t *qn;
	wtk_if_item_t *xi;

	for(qn=xif->item_q.pop;qn;qn=qn->next)
	{
		xi=data_offset2(qn,wtk_if_item_t,q_n);
		if(qn!=xif->item_q.pop)
		{
			printf(",");
		}
		printf("%.*s",xi->k->len,xi->k->data);
		switch(xi->type)
		{
		case WTK_IF_LT:
			printf("<");
			break;
		case WTK_IF_LET:
			printf("<=");
			break;
		case WTK_IF_EQ:
			printf("=");
			break;
		case WTK_IF_GET:
			printf(">=");
			break;
		case WTK_IF_GT:
			printf(">");
			break;
		case WTK_IF_NEQ:
			printf("!=");
			break;
		}
		if(xi->is_str)
		{
			printf("\"%.*s\"",xi->v.str->len,xi->v.str->data);
		}else
		{
			printf("%d",(int)(xi->v.number));
		}
	}
}

int wtk_if_check(wtk_if_t *xif,void *ths,wtk_if_get_var_f get_var)
{
	wtk_queue_node_t *qn;
	wtk_if_item_t *xi;
	wtk_string_t *v;
	int ret;
	float ft;

	for(qn=xif->item_q.pop;qn;qn=qn->next)
	{
		xi=data_offset2(qn,wtk_if_item_t,q_n);
		v=get_var(ths,xi->k->data,xi->k->len);
		if(xi->is_str && wtk_string_cmp_s(xi->v.str,"nil")==0)
		{
			if(v)
			{
				return 0;
			}else
			{
				continue;
			}
		}
		if(!v)
		{
			return 0;
		}
		if(xi->is_str)
		{
			ret=wtk_string_cmp(v,xi->v.str->data,xi->v.str->len);
			//wtk_debug("[%.*s]=[%.*s] ret=%d\n",v->len,v->data,xi->v.str->len,xi->v.str->data,ret);
		}else
		{
			ft=wtk_str_atof(v->data,v->len);
			ret=ft-xi->v.number;
			//wtk_debug("ft=%f/%f type=%d ret=%d\n",ft,xi->v.number,xi->type,ret);
		}
		switch(xi->type)
		{
		case WTK_IF_LT:
			if(ret<0)
			{

			}else
			{
				return 0;
			}
			break;
		case WTK_IF_LET:
			if(ret<=0)
			{

			}else
			{
				return 0;
			}
			break;
		case WTK_IF_EQ:
			if(ret!=0)
			{
				return 0;
			}
			break;
		case WTK_IF_GET:
			if(ret<=0)
			{
				return 0;
			}
			break;
		case WTK_IF_GT:
			if(ret<0)
			{
				return 0;
			}
			break;
		case WTK_IF_NEQ:
			if(ret==0)
			{
				return 0;
			}
			break;
		}
	}
	return 1;
}
