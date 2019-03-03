#include "wtk_vector_buffer.h"

wtk_vector_buffer_t *wtk_vector_buffer_new(int size)
{
	wtk_vector_buffer_t *b;
	int t;

	t=wtk_round_word(sizeof(*b))+size*sizeof(float);
	b=(wtk_vector_buffer_t*)wtk_malloc(t);
	b->odd=0;
	b->rstart=b->cur=b->start=(float*)(((char*)b)+wtk_round_word((sizeof(*b))));
	b->end=b->rstart+size;
	return b;
}

int wtk_vector_buffer_delete(wtk_vector_buffer_t *b)
{
	wtk_free(b);
	return 0;
}

int wtk_vector_buffer_reset(wtk_vector_buffer_t *b)
{
	b->odd=0;
	b->rstart=b->cur=b->start=(float*)(((char*)b)+wtk_round_word((sizeof(*b))));
	return 0;
}

int wtk_vector_buffer_push_float(wtk_vector_buffer_t *v,float* data,int len)
{
	len=min(v->end-v->cur,len);
	memcpy(v->cur,data,len*sizeof(float));
	v->cur+=len;
	return len;
}

int wtk_vector_buffer_push(wtk_vector_buffer_t *v,short* data,int samples)
{
	short *start=data;
	short* end=start+samples;

	while(start<end && v->cur<v->end)
	{
		//wtk_debug("%d,samples=%d\n",*start,samples);
		*(v->cur++)=*(start++);
		//wtk_debug("%f\n",*(v->cur-1));
		//exit(0);
	}
	return start-data;
}

int wtk_vector_buffer_push_c(wtk_vector_buffer_t *v,char *data,int bytes)
{
	short odd;
	char *p;
	int cpy=0,left,samples;

	left=wtk_vector_buffer_left_samples(v);
	if(left<=0 || bytes<=0){goto end;}
	if(v->odd)
	{
		p=(char*)&odd;
		p[0]=v->odd_char;
		p[1]=data[0];
		bytes-=1;data+=1;
		v->odd=0;
		wtk_vector_buffer_push(v,&odd,1);
		cpy+=1;
	}
	samples=bytes/2;
	left=wtk_vector_buffer_push(v,(short*)data,samples);
	cpy+=left<<1;
	if((left==samples) && (bytes%2))
	{
		//pad odd data.
		v->odd_char=data[cpy];
		v->odd=1;
		cpy+=1;
	}
end:
	return cpy;
}

int wtk_vector_buffer_peek(wtk_vector_buffer_t *b,wtk_vector_t *v,int is_end)
{
	int samples;
	int valid_len;
	int i;

	samples=wtk_vector_size(v);
	valid_len=wtk_vector_buffer_valid_len(b);
	if(!is_end)
	{
		if(valid_len<samples)
		{
			return -1;
		}
		memcpy(&(v[1]),b->start,samples*sizeof(float));
		return 0;
	}else
	{
		memcpy(&(v[1]),b->start,valid_len*sizeof(float));
		for(i=valid_len+1;i<=samples;++i)
		{
			v[i]=0;
		}
		return 0;
	}
}

int wtk_vector_buffer_copy_data(wtk_vector_buffer_t *b,float *data,int samples)
{
	if(wtk_vector_buffer_valid_len(b)<samples)
	{
		return -1;
	}
	memcpy(data,b->start,samples*sizeof(float));
	return 0;
}

float* wtk_vector_buffer_peek_data(wtk_vector_buffer_t *b,int samples)
{
	if(wtk_vector_buffer_valid_len(b)<samples)
	{
		return 0;
	}
	return b->start;
}

void wtk_vector_buffer_skip(wtk_vector_buffer_t *b,int samples,int left_enough)
{
	int size;

	b->start+=samples;
	if((b->end-b->start)<left_enough)
	{
		//in this way,there is no overlap,so use memcpy not memmove.
		size=b->cur-b->start;
		memmove(b->rstart,b->start,size*sizeof(float));
		b->start=b->rstart;
		b->cur=b->start+size;
	}
}
