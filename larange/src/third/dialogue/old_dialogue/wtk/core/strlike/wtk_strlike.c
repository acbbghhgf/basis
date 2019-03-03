#include <ctype.h>
#include "wtk_strlike.h"
#include "wtk/core/math/wtk_matrix.h"

wtk_strlike_t* wtk_strlike_new(wtk_strlike_cfg_t *cfg)
{
	wtk_strlike_t *l;

	l=(wtk_strlike_t*)wtk_malloc(sizeof(wtk_strlike_t));
	l->cfg=cfg;
	l->heap=wtk_heap_new(4096);
	return l;
}

void wtk_strlike_delete(wtk_strlike_t *l)
{
	wtk_heap_delete(l->heap);
	wtk_free(l);
}

wtk_array_t* wtk_strlike_str_to_char(wtk_strlike_t *l,char *p,int bytes)
{
	wtk_heap_t *heap=l->heap;
	wtk_array_t *a;
	char *s,*e;
	int n;
	wtk_string_t *v;

	a=wtk_array_new_h(heap,bytes/2,sizeof(wtk_string_t*));
	s=p;e=s+bytes;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		if(n==1 && (isspace(*s)||wtk_strlike_cfg_is_sp(l->cfg,s,n)) )
		{

		}else
		{
			v=wtk_heap_dup_string(heap,s,n);
			wtk_array_push2(a,&v);
		}
		s+=n;
	}
	return a;
}

wtk_array_t* wtk_strlike_str_to_array(wtk_strlike_t *l,char *p,int bytes)
{
typedef enum
{
	WTK_STRLIKE_STR_INIT,
	WTK_STRLIKE_STR_ENG,
}wtk_strlike_state_t;
	wtk_strlike_state_t state;
	wtk_heap_t *heap=l->heap;
	char *s,*e;
	int n;
	wtk_string_t eng;
	char c;
	wtk_string_t *v;
	wtk_array_t *a;

	a=wtk_array_new_h(heap,bytes/2,sizeof(wtk_string_t*));
	wtk_string_set(&(eng),0,0);
	state=WTK_STRLIKE_STR_INIT;
	s=p;e=s+bytes;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		//wtk_debug("[%.*s]\n",n,s);
		switch(state)
		{
		case WTK_STRLIKE_STR_INIT:
			if(n==1)
			{
				c=*s;
				if((!isspace(c)) && wtk_strlike_cfg_is_en(l->cfg,s,n))
				{
					eng.data=s;
					++eng.len;
					state=WTK_STRLIKE_STR_ENG;
				}
			}else
			{
				if(wtk_strlike_cfg_is_cn(l->cfg,s,n))
				{
					v=wtk_heap_dup_string(heap,s,n);
					wtk_array_push2(a,&v);
					//wtk_debug("==>[%.*s]\n",n,s);
				}
			}
			break;
		case WTK_STRLIKE_STR_ENG:
			if(n==1)
			{
				c=*s;
				if(isspace(c)||wtk_strlike_cfg_is_sp(l->cfg,s,n) || !wtk_strlike_cfg_is_en(l->cfg,s,n))
				{
					//eng.len=s-eng.data;
					/*
					wtk_debug("==>[%c:%.*s]=[%d/%d/%d]\n",c,eng.len,eng.data,isspace(c),
							wtk_strlike_cfg_is_sp(l->cfg,s,n),!wtk_strlike_cfg_is_en(l->cfg,s,n));
					*/
					v=wtk_heap_dup_string(heap,eng.data,eng.len);
					wtk_array_push2(a,&v);
					eng.len=0;
					state=WTK_STRLIKE_STR_INIT;
				}else
				{
					++eng.len;
				}
			}else
			{
				//eng.len=s-eng.data;
				//wtk_debug("==>[%.*s]\n",eng.len,eng.data);
				if(wtk_strlike_cfg_is_cn(l->cfg,s,n))
				{
					v=wtk_heap_dup_string(heap,eng.data,eng.len);
					wtk_array_push2(a,&v);
					v=wtk_heap_dup_string(heap,s,n);
					wtk_array_push2(a,&v);
				}
				eng.len=0;
				//wtk_debug("==>[%.*s]\n",n,s);
				state=WTK_STRLIKE_STR_INIT;
			}
			break;
		}
		s+=n;
	}
	if(eng.len>0)
	{
		v=wtk_heap_dup_string(heap,eng.data,eng.len);
		wtk_array_push2(a,&v);
		//wtk_debug("==>[%.*s]\n",eng.len,eng.data);
	}
	//exit(0);
	return a;
}

int wtk_strlike_reset(wtk_strlike_t *l)
{
	wtk_heap_reset(l->heap);
	return 0;
}

float wtk_strlike_process(wtk_strlike_t *l,char *ref,int ref_bytes,char *rec,int rec_bytes)
{
	wtk_array_t *a1,*a2;
	wtk_matrix_t *m;
	wtk_string_t *v1,*v2;
	int i,j;
	int cost;
	float f1,f2,f3,f;

	//wtk_debug("[%.*s]=[%.*s]\n",ref_bytes,ref,rec_bytes,rec);
	if(l->cfg->use_char)
	{
		a1=wtk_strlike_str_to_char(l,ref,ref_bytes);
		a2=wtk_strlike_str_to_char(l,rec,rec_bytes);
	}else
	{
		a1=wtk_strlike_str_to_array(l,ref,ref_bytes);
		a2=wtk_strlike_str_to_array(l,rec,rec_bytes);
	}
	//wtk_array_print_string(a1);
	//wtk_array_print_string(a2);
	if(a1->nslot>a2->nslot && l->cfg->use_eq_len)
	{
		a1->nslot=a2->nslot;
	}
	m=wtk_matrix_new(a1->nslot+1,a2->nslot+1);
	for(i=0;i<=a1->nslot;++i)
	{
		for(j=0;j<=a2->nslot;++j)
		{
			if(j==0)
			{
				m[i+1][j+1]=i;
			}else
			{
				if(i==0)
				{
					m[i+1][j+1]=j;
				}else
				{
					m[i+1][j+1]=0;
				}
			}
		}
	}
	for(i=1;i<=a1->nslot;++i)
	{
		for(j=1;j<=a2->nslot;++j)
		{
			v1=((wtk_string_t**)a1->slot)[i-1];
			v2=((wtk_string_t**)a2->slot)[j-1];
			if(wtk_string_cmp(v1,v2->data,v2->len)==0)
			{
				cost=0;
			}else
			{
				cost=l->cfg->cost_sub;
			}
			//wtk_debug("cost=%d\n",cost);
			f1=m[i][j+1]+l->cfg->cost_del;	//删除
			f2=m[i+1][j]+l->cfg->cost_ins;	//插入
			//wtk_debug("del=%f ins=%f\n",l->cfg->cost_del,l->cfg->cost_ins);
			f3=m[i][j]+cost;//替换
			f=min(f1,f2);
			f=min(f,f3);
			m[i+1][j+1]=f;
			//wtk_debug("v[%d][%d]=%f\n",i+1,j+1,f);
		}
	}
	//wtk_debug("%f\n",m[a1->nslot+1][a2->nslot+1]);
	f=1-m[a1->nslot+1][a2->nslot+1]/a1->nslot;
	//wtk_debug("[%.*s]/[%.*s]=%f,%f/%d\n",ref_bytes,ref,rec_bytes,rec,f,m[a1->nslot+1][a2->nslot+1],a1->nslot);
	//wtk_debug("%f,%f,%d/%d\n",f,m[a1->nslot+1][a2->nslot+1],a1->nslot,a2->nslot);
	wtk_matrix_delete(m);
	wtk_strlike_reset(l);
	return f;
}
