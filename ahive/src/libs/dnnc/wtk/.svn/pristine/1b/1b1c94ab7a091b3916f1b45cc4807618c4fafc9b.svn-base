#include "wtk_msg_parser.h" 

wtk_msg_parser_t* wtk_msg_parser_new(wtk_connection_t *c)
{
	wtk_msg_parser_t *parser;

	parser=(wtk_msg_parser_t*)wtk_malloc(sizeof(wtk_msg_parser_t));
	parser->c=c;
	parser->buf=wtk_strbuf_new(1024,1);
	parser->ths=NULL;
	parser->raise=NULL;
	parser->hook=NULL;
	wtk_msg_parser_reset(parser);
	return parser;
}

void wtk_msg_parser_delete(wtk_msg_parser_t *p)
{
	wtk_strbuf_delete(p->buf);
	wtk_free(p);
}

void wtk_msg_parser_reset(wtk_msg_parser_t *p)
{
	wtk_strbuf_reset(p->buf);
	p->state=WTK_MSG_PARSER_INIT;
	p->msg_size.v=0;
	p->msg_size_pos=0;
}

void wtk_msg_parser_set_raise(wtk_msg_parser_t *p,void *ths,wtk_msg_parser_raise_f raise)
{
	p->ths=ths;
	p->raise=raise;
}

void wtk_msg_parser_raise_msg2(wtk_msg_parser_t *p)
{
	uint32_t len;
	wtk_strbuf_t *buf=p->buf;

	wtk_debug("send msg start\n");
	//test write msg;
	len=buf->pos;
	wtk_connection_write(p->c,(char*)&len,4);
	wtk_connection_write(p->c,buf->data,buf->pos);
	wtk_debug("send msg end\n");
}

void wtk_msg_parser_raise_msg(wtk_msg_parser_t *p)
{
	if(p->raise)
	{
		//print_data(p->buf->data,p->buf->pos);
		p->raise(p->ths,p,p->buf->data,p->buf->pos);
	}
}

int wtk_msg_parser_feed(wtk_msg_parser_t *p,char *data,int len)
{
	int i;

	//print_data(data,len);
	switch(p->state)
	{
	case WTK_MSG_PARSER_INIT:
		if(len>=4)
		{
			p->msg_size.v=*((uint32_t*)(data));
			i=p->msg_size.v;
			if(i<0)
			{
				print_hex(data,13);
				wtk_debug("found hdr msg size=%d bug.\n",i);
				return -1;
			}
			p->state=WTK_MSG_PARSER_DATA;
			wtk_strbuf_reset(p->buf);
			if(len>4)
			{
				return wtk_msg_parser_feed(p,data+4,len-4);
			}else
			{
				return 0;
			}
		}else
		{
			p->msg_size_pos=len;
			for(i=0;i<len;++i)
			{
				p->msg_size.data[i]=data[i];
			}
			p->state=WTK_MSG_PARSER_HDR;
		}
		break;
	case WTK_MSG_PARSER_HDR:
		if(len==0){return 0;}
		i=min(4-p->msg_size_pos,len);
		memcpy(p->msg_size.data+p->msg_size_pos,data,i);
		p->msg_size_pos+=i;
		if(p->msg_size_pos==4)
		{
			wtk_strbuf_reset(p->buf);
			p->state=WTK_MSG_PARSER_DATA;
		}
		if(len>i)
		{
			return wtk_msg_parser_feed(p,data+i,len-i);
		}
		break;
	case WTK_MSG_PARSER_DATA:
		//wtk_debug("msg_size=%d\n",p->msg_size.v)
		i=min(p->msg_size.v-p->buf->pos,len);
		wtk_strbuf_push(p->buf,data,i);
		if(p->buf->pos>=p->msg_size.v)
		{
//			static int ki=0;
//
//			++ki;
//			wtk_debug("=========== raise msg ki=%d\n",ki);
			//print_data(p->buf->data,p->buf->pos);
			wtk_msg_parser_raise_msg(p);
			wtk_msg_parser_reset(p);
		}
		if(len>i)
		{
			return wtk_msg_parser_feed(p,data+i,len-i);
		}
		break;
	}
	return 0;
}
