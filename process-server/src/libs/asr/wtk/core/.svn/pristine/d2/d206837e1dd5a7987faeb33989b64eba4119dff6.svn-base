#include "wtk_fring.h" 

wtk_fring_t* wtk_fring_new(int n)
{
	wtk_fring_t *f;

	f=(wtk_fring_t*)wtk_malloc(sizeof(wtk_fring_t));
	f->nslot=n;
	f->first=0;
	f->used=0;
	f->f=(float*)wtk_calloc(n,sizeof(float));
	f->tot_v=0;
	return f;
}

void wtk_fring_delete(wtk_fring_t *r)
{
	wtk_free(r->f);
	wtk_free(r);
}

void wtk_fring_reset(wtk_fring_t *r)
{
	r->first=0;
	r->used=0;
	r->tot_v=0;
}

void wtk_fring_push(wtk_fring_t *r,float f)
{
	int idx;

	idx=(r->first+r->used)%(r->nslot);
	r->f[idx]=f;
	++r->used;
	r->tot_v+=f;
}

float wtk_fring_pop(wtk_fring_t* r)
{
	float f;

	f=r->f[r->first];
	r->first=(r->first+1)%(r->nslot);
	--r->used;
	r->tot_v-=f;
	return f;
}

void wtk_fring_pop3(wtk_fring_t *r,int n)
{
	int i;
	float f;
	float *pf;
	int j;

	r->first=(r->first+n)%(r->nslot);
	r->used-=n;
	f=0;
	pf=r->f;
	j=0;
	for(i=r->first;i<r->nslot;++i)
	{
		f+=pf[i];
		++j;
		if(j>=r->used)
		{
			break;
		}
	}
	if(j<r->used)
	{
		for(i=0;i<r->nslot;++i)
		{
			f+=pf[i];
			++j;
			if(j>=r->used)
			{
				break;
			}
		}
	}
	r->tot_v=f;
}

void wtk_fring_pop2(wtk_fring_t *r,int n)
{
	//wtk_debug("first=%d/%d/%d\n",r->first,r->used,r->nslot);
	r->first=(r->first+n)%(r->nslot);
	r->used-=n;
	//wtk_debug("first=%d/%d/%d\n",r->first,r->used,r->nslot);
	//exit(0);
}

void wtk_fring_print(wtk_fring_t *r)
{
	int i;

	for(i=0;i<r->used;++i)
	{
		wtk_debug("v[%d]=%f\n",i,wtk_fring_at(r,i));
	}
}


