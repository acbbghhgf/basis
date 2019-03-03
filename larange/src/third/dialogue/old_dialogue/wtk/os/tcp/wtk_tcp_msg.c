#include "wtk_tcp_msg.h"
#include "wtk/os/wtk_fd.h"

wtk_tcp_msg_t* wtk_tcp_msg_new(int cmd_bytes,int data_bytes)
{
	wtk_tcp_msg_t *msg;

	msg=(wtk_tcp_msg_t*)wtk_malloc(sizeof(wtk_tcp_msg_t));
	msg->cmd=wtk_strbuf_new(cmd_bytes,1);
	msg->data=wtk_strbuf_new(data_bytes,1);
	return msg;
}

void wtk_tcp_msg_delete(wtk_tcp_msg_t *msg)
{
	wtk_strbuf_delete(msg->cmd);
	wtk_strbuf_delete(msg->data);
}

void wtk_tcp_msg_reset(wtk_tcp_msg_t *msg)
{
	wtk_strbuf_reset(msg->cmd);
	wtk_strbuf_reset(msg->data);
}

void wtk_tcp_msg_set(wtk_tcp_msg_t *msg,char *cmd,int cmd_bytes,char* data,int data_bytes)
{
	wtk_strbuf_reset(msg->cmd);
	wtk_strbuf_push(msg->cmd,cmd,cmd_bytes);
	wtk_strbuf_reset(msg->data);
	wtk_strbuf_push(msg->data,data,data_bytes);
}

void wtk_tcp_msg_set_cmd(wtk_tcp_msg_t *msg,char *cmd,int cmd_bytes)
{
	wtk_strbuf_reset(msg->cmd);
	wtk_strbuf_push(msg->cmd,cmd,cmd_bytes);
}

void wtk_tcp_msg_print(wtk_tcp_msg_t *msg)
{
	wtk_debug("========= msg=%p ============\n",msg);
	printf("cmd=[%.*s]\n",msg->cmd->pos,msg->cmd->data);
	printf("data=[%.*s]\n",msg->data->pos,msg->data->data);
}

int wtk_tcp_msg_read_string(wtk_strbuf_t *buf,int fd)
{
	wtk_fd_state_t s;
	int len;
	int ret=-1;

	wtk_strbuf_reset(buf);
	s=wtk_fd_recv2(fd,(char*)&len,4);//,&read);
	//wtk_debug("len=%d read=%d\n",len,read);
	if(s!=WTK_OK)// || read!=4)
	{
		wtk_debug("read string len failed state=%d\n",s);
		perror(__FUNCTION__);
		//exit(0);
		goto end;
	}
	if(len>0)
	{
		wtk_strbuf_expand(buf,len);
		//wtk_debug("len=%d\n",len);
		s=wtk_fd_recv2(fd,buf->data,len);//,&read);
		//print_data(buf->data,read);
		if(s!=WTK_OK)// || read!=len)
		{
			wtk_debug("read string len failed state=%d len=%d\n",s,len);
			perror(__FUNCTION__);
			//exit(0);
			goto end;
		}
		buf->pos=len;
	}else
	{
		buf->pos=0;
	}
	ret=0;
end:
	return ret;
}

int wtk_tcp_msg_write_string(int fd,char *data,int bytes)
{
	wtk_fd_state_t s;
	int writed;
	int ret=-1;

	//wtk_debug("send bytes=%d\n",bytes);
	//print_data(data,bytes);
	s=wtk_fd_send(fd,(char*)&(bytes),4,&writed);
	if(s!=WTK_OK || writed!=4)
	{
		wtk_debug("write string len failed\n");
		goto end;
	}
	if(bytes>0)
	{
		s=wtk_fd_send(fd,data,bytes,&writed);
		if(s!=WTK_OK || writed!=bytes)
		{
			wtk_debug("write string data failed\n");
			goto end;
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_tcp_msg_read(wtk_tcp_msg_t *msg,int fd)
{
	int ret;

	ret=wtk_tcp_msg_read_string(msg->cmd,fd);
	if(ret!=0){goto end;}
	ret=wtk_tcp_msg_read_string(msg->data,fd);
	if(ret!=0){goto end;}
end:
	return ret;
}

int wtk_tcp_msg_write(wtk_tcp_msg_t *msg,int fd)
{
	int ret;

	ret=wtk_tcp_msg_write_string(fd,msg->cmd->data,msg->cmd->pos);
	if(ret!=0){goto end;}
	ret=wtk_tcp_msg_write_string(fd,msg->data->data,msg->data->pos);
	if(ret!=0){goto end;}
	ret=0;
end:
	return ret;
}

