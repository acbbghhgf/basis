#include "wtk_zip.h" 
#include <math.h>

#define WTK_ZIP_END_SYM 256

wtk_zip_t* wtk_zip_new(wtk_zip_cfg_t *cfg)
{
	wtk_zip_t *z;

	z=(wtk_zip_t*)wtk_malloc(sizeof(wtk_zip_t));
	z->cfg=cfg;
	z->buf=wtk_strbuf_new(1024,1);
	z->len=pow(2,cfg->bits);//4096;//8192;//4096;//2^12 0/255
	z->item=wtk_malloc(sizeof(wtk_zip_item_t)*z->len);
	wtk_zip_reset(z);
	return z;
}

void wtk_zip_delete(wtk_zip_t *z)
{
	wtk_free(z->item);
	wtk_strbuf_delete(z->buf);
	wtk_free(z);
}

void wtk_zip_reset(wtk_zip_t *z)
{
	wtk_zip_item_t *item;
	int i;

	z->use=0;
	z->prev_item=NULL;
	z->nxt_idx=WTK_ZIP_END_SYM+1;
	z->cnt=0;
	z->cache_v=0;
	z->cache_bit=32;
	wtk_strbuf_reset(z->buf);
	for(i=0,item=z->item;i<z->len;++i,++item)
	{
		wtk_queue2_init(&(item->next_q));
		if(i>255)
		{
			item->c=0;
			item->used=0;
			item->prev=0;
		}else
		{
			item->c=i;
			item->used=1;
			item->prev=0;
		}
	}
}

unsigned int zip_bit_map[]={
		0,
		0x0001,
		0x0003,
		0x0007,
		0x000f,
		0x001f,
		0x003f,
		0x007f,
		0x00ff,
		0x01ff,
		0x03ff,
		0x07ff,
		0x0fff,
		0x1fff,
		0x3fff,
		0x7fff,
		0xffff
};

void wtk_zip_write_short(wtk_zip_t *z,unsigned short v)
{
	unsigned int vx;
	int bits;

	bits=z->cfg->bits-z->cache_bit;
	if(bits<0)
	{
		//index 32-z->cache_bit
		z->cache_v+=v<<(32-z->cache_bit);//-bits);
		z->cache_bit-=z->cfg->bits;
	}else
	{
		//index 32-z->cache_bit
		//wtk_debug("bits=%d/%d/%d\n",bits,32-z->cache_bit,z->cache_bit);
		//低位在前
		vx=v&zip_bit_map[z->cache_bit];
		vx=(vx)<<(32-z->cache_bit);
		z->cache_v+=vx;//v>>(z->cfg->bits-z->cache_bit);
		//wtk_debug("v[%d]=[%#x]\n",z->cnt,z->cache_v);
		++z->cnt;
		wtk_strbuf_push(z->buf,(char*)&(z->cache_v),4);
		//wtk_debug("bits=%d\n",bits);
		if(bits>0)
		{
			//wtk_debug("vx=%#x\n",vx);
			v=v>>z->cache_bit;
			//v-=(v>>bits)<<bits;
			z->cache_v= v;// <<(32-bits);
			z->cache_bit=32-bits;
		}else
		{
			z->cache_v=0;
			z->cache_bit=32;
		}
	}
//	if((32-z->cache_bit+z->buf->pos*8) != z->cnt*z->cfg->bits)
//	{
//		wtk_debug("found bug %d %d %d %d/%d\n",z->cnt,z->buf->pos,z->cache_bit,z->cnt*z->cfg->bits,32-z->cache_bit+z->buf->pos*8);
//		exit(0);
//	}
}

void wtk_zip_feed_end(wtk_zip_t *z)
{
	unsigned short v;

	if(z->prev_item)
	{
		v=z->prev_item-z->item;
		wtk_zip_write_short(z,v);
	}
	wtk_zip_write_short(z,WTK_ZIP_END_SYM);
	//wtk_debug("cb=%d %d\n",z->cache_bit,z->cache_v);
	if(z->cache_bit>0)
	{
		//wtk_debug("v[%d]=[%#x]\n",z->cnt,z->cache_v);
		++z->cnt;
		wtk_strbuf_push(z->buf,(char*)&(z->cache_v),4);
	}
}


void wtk_zip_feed(wtk_zip_t *z,char *data,int len)
{
	wtk_zip_item_t *item;
	wtk_queue_node_t *qn;
	unsigned char *s,*e,c;
	int b;
	unsigned short v;
	static int kx=0;

	if(len<=0){return;}
	//wtk_debug("%.*s\n",len,data);
	s=(unsigned char*)data;e=s+len;
	if(!z->use)
	{
		c=*(s++);
		//wtk_debug("%#x\n",c);
		z->prev_item=z->item+c;
		z->use=1;
		++kx;
	}
	while(s<e)
	{
		++kx;
		c=*(s++);
		//wtk_debug("%c\n",c);
		b=0;
		if(z->prev_item)
		{
			for(qn=z->prev_item->next_q.pop;qn;qn=qn->next)
			{
				item=data_offset2(qn,wtk_zip_item_t,q_n);
				if(item->c==c)
				{
					z->prev_item=item;
					b=1;
					break;
				}
			}
		}
		if(b==0)
		{
			if((z->nxt_idx>=z->len))
			{
				v=z->prev_item-z->item;
				wtk_zip_write_short(z,v);
				//wtk_debug("found bug %d/%d\n",v,len);
				//exit(0);
			}else
			{
				v=z->prev_item-z->item;
				//wtk_debug("v=%d / %d\n",v,z->nxt_idx);
				wtk_zip_write_short(z,v);
				v=z->nxt_idx++;
				item=z->item+v;
				item->used=1;
				item->c=c;
				item->prev=z->prev_item-z->item;
				wtk_queue2_push(&(z->prev_item->next_q),&(item->q_n));
			}
			z->prev_item=z->item+c;
		}
		//exit(0);
	}
}


int wtk_zip_write_file(wtk_zip_t *z,char *fn)
{
	wtk_zip_item_t *item;
	int ret=-1;
	FILE *f;
	int i;
	unsigned char c;
	unsigned short v;

	f=fopen(fn,"wb");
	if(!f){goto end;}
	ret=fwrite("WZF1",4,1,f);
	if(ret!=1){ret=-1;goto end;}
	c=z->cfg->bits;
	ret=fwrite(&c,1,1,f);
	if(ret!=1){ret=-1;goto end;}
	v=z->nxt_idx-WTK_ZIP_END_SYM-1;
	ret=fwrite(&(v),1,2,f);
	if(ret!=2){ret=-1;goto end;}
	//wtk_debug("nxt=%d\n",z->nxt_idx);
	for(i=WTK_ZIP_END_SYM+1,item=z->item+i;i<z->nxt_idx;++i,++item)
	{
		ret=fwrite(&(item->prev),1,2,f);
		if(ret!=2){goto end;}
		ret=fwrite(&(item->c),1,1,f);
		if(ret!=1){goto end;}
	}
	ret=fwrite(z->buf->data,1,z->buf->pos,f);
	if(ret!=z->buf->pos){ret=-1;goto end;}
end:
	if(f)
	{
		fclose(f);
	}
	return ret;
}

int wtk_zip_file(wtk_zip_t *z,char *ifn,char *ofn)
{
#define BUF_SIZE 4096
	char buf[BUF_SIZE];
	int ret=-1;
	FILE *f;

	f=fopen(ifn,"rb");
	if(!f){goto end;}
	while(1)
	{
		ret=fread(buf,1,BUF_SIZE,f);
		if(ret<=0){goto end;}
		wtk_zip_feed(z,buf,ret);
		if(ret<BUF_SIZE)
		{
			break;
		}
	}
	wtk_zip_feed_end(z);
	wtk_zip_write_file(z,ofn);
	ret=0;
end:
	if(f)
	{
		fclose(f);
	}
	return ret;
}
