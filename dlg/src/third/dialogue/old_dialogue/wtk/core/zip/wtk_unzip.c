#include "wtk_unzip.h" 


wtk_unzip_t* wtk_unzip_new()
{
	wtk_unzip_t *z;

	z=(wtk_unzip_t*)wtk_malloc(sizeof(wtk_unzip_t ));
	z->len=0;
	z->items=NULL;
	z->buf=wtk_strbuf_new(4096,1);
	return z;
}

void wtk_unzip_delete(wtk_unzip_t *z)
{
	wtk_unzip_reset(z);
	wtk_strbuf_delete(z->buf);
	wtk_free(z);
}

void wtk_unzip_reset(wtk_unzip_t *z)
{
	if(z->items)
	{
		wtk_free(z->items);
		z->items=NULL;
	}
	z->len=0;
	z->cnt=0;
	wtk_strbuf_reset(z->buf);
}

void wtk_unzip_write_char(wtk_unzip_t *z,unsigned char c)
{
	//wtk_debug("%c\n",c);
	wtk_strbuf_push_c(z->buf,c);
}


void wtk_unzip_write_short(wtk_unzip_t *z,unsigned short v)
{
	wtk_unzip_item_t *item;

	//wtk_debug("v=%#x\n",v);
	++z->cnt;
	if(v<256)
	{
		wtk_unzip_write_char(z,v);
	}else
	{
		v-=257;
		item=z->items+v;
		wtk_unzip_write_short(z,item->prev);
		wtk_unzip_write_char(z,item->c);
	}
}

int wtk_unzip_file(wtk_unzip_t *z,char *ifn,char *ofn)
{
	int ret=-1;
	FILE *f;
	//char buf[1024];
	unsigned char c;
	unsigned short v;
	wtk_unzip_item_t *item;
	unsigned int i;
	int bits;
	unsigned short last_v;
	int last_valid_bit;
	int valid_bits,t;
	int vt=0;

	f=fopen(ifn,"rb");
	if(!f){goto end;}
	ret=fread(&i,1,4,f);
	if(ret!=4){ret=-1;goto end;}
	//wtk_debug("[%.*s]\n",ret,buf);
	ret=fread(&c,1,1,f);
	if(ret!=1){ret=-1;goto end;}
	//wtk_debug("%d\n",c);
	bits=c;
	ret=fread(&(v),1,2,f);
	if(ret!=2){ret=-1;goto end;}
	z->len=v;
	z->items=(wtk_unzip_item_t*)wtk_calloc(v,sizeof(wtk_unzip_item_t));
	for(i=0,item=z->items;i<z->len;++i,++item)
	{
		ret=fread(&(v),1,2,f);
		if(ret!=2){ret=-1;goto end;}
		ret=fread(&c,1,1,f);
		if(ret!=1){ret=-1;goto end;}
		item->prev=v;
		item->c=c;
	}
	last_valid_bit=0;
	while(1)
	{
		ret=fread(&(i),1,4,f);
		if(ret!=4){ret=-1;goto end;}
		//wtk_debug("v[%d]=[%#x]\n",vt,i);
		++vt;
		//wtk_debug("%#x %d\n",i,last_valid_bit);
		valid_bits=32;
		if(last_valid_bit>0)
		{
			//wtk_debug("v=%#x %#x last_left_bit=%d\n",last_v,i,last_valid_bit);
			t=bits-last_valid_bit;
			v=last_v+((i&zip_bit_map[t])<<last_valid_bit);
			if(v==256)
			{
				ret=0;
				goto end;
				break;
			}
			//wtk_debug("v=%#x\n",v);
			wtk_unzip_write_short(z,v);
			i>>=t;
			valid_bits-=t;
			//exit(0);
		}
		while(valid_bits>=bits)
		{
			//wtk_debug("v[%#x]\n",i);
			v=i&zip_bit_map[bits];
			//wtk_debug("v[%#x]\n",i);
			//wtk_debug("v=%#x\n",v);
			if(v==256)
			{
				i>>=bits;
				ret=0;
				goto end;
				break;
			}
			//wtk_debug("v=%#x\n",v);
			wtk_unzip_write_short(z,v);
			i>>=bits;
			valid_bits-=bits;
		}
		last_v=i;
		last_valid_bit=valid_bits;
		//wtk_debug("i=%#x v=%d\n",i,valid_bits);
		//exit(0);
	}
	ret=0;
end:
	file_write_buf(ofn,z->buf->data,z->buf->pos);
	if(f)
	{
		fclose(f);
	}
	return ret;
}
