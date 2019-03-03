#include "qtk_ifly.h"

int qtk_ifly_login(qtk_ifly_t *ifly)
{
	int ret;

	if(ifly->ready)
	{
		ret=0;
		goto end;
	}
	//wtk_debug("[%s]\n",ifly->cfg->login_param);
	ret = MSPLogin(NULL, NULL, ifly->cfg->login_param); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed , Error code %d.\n",ret);
		ret=-1;
		goto end;
	}
	ifly->ready=1;
	ret=0;
end:
	return ret;
}

void qtk_ifly_logout(qtk_ifly_t *ifly)
{
	if(ifly->ready)
	{
		if(ifly->session_id)
		{
			QISRSessionEnd(ifly->session_id, NULL);
		}
		MSPLogout();
	}
}

qtk_ifly_t* qtk_ifly_new(qtk_ifly_cfg_t *cfg)
{
	qtk_ifly_t *ifly;

	ifly=(qtk_ifly_t*)wtk_malloc(sizeof(qtk_ifly_t));
	ifly->cfg=cfg;
	ifly->buf=wtk_strbuf_new(1024,1);
	ifly->session_id=NULL;
	ifly->ready=0;
	ifly->bufx=wtk_strbuf_new(1024,1);
	qtk_ifly_login(ifly);
	return ifly;
}

void qtk_ifly_delete(qtk_ifly_t *ifly)
{
	wtk_strbuf_delete(ifly->bufx);
	wtk_strbuf_delete(ifly->buf);
	if(ifly->ready)
	{
		qtk_ifly_logout(ifly);
	}
	wtk_free(ifly);
}

int qtk_ifly_start(qtk_ifly_t *ifly)
{
	int ret;
	int errcode;

	wtk_strbuf_reset(ifly->buf);
	ifly->cnt=0;
	if(!ifly->ready)
	{
		ret=qtk_ifly_login(ifly);
		if(ret!=0){goto end;}
	}
	ifly->session_id = QISRSessionBegin(NULL, ifly->cfg->session_param, &errcode); //听写不需要语法，第一个参数为NULL
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRSessionBegin failed! error code:%d\n", errcode);
		ret=-1;
		goto end;
	}
	ret=0;
end:
	return ret;
}


int qtk_ilfy_get_result(qtk_ifly_t *ifly)
{
	int				rec_stat					=	MSP_REC_STATUS_SUCCESS ;			//识别状态
	int  errcode;
	int ret;
	wtk_strbuf_t *buf=ifly->buf;

	while (MSP_REC_STATUS_COMPLETE != rec_stat)
	{
		const char *rslt = QISRGetResult(ifly->session_id, &rec_stat, 0, &errcode);
		if (MSP_SUCCESS != errcode)
		{
			printf("\nQISRGetResult failed, error code: %d\n", errcode);
			ret=-1;
			goto end;
		}
		if (NULL != rslt)
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,rslt,strlen(rslt));
		}
		usleep(50*1000); //防止频繁占用CPU
	}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	ret=0;
end:
	return ret;
}

void qtk_ifly_process(qtk_ifly_t *ifly,char *data,int len,int is_end)
{
	int				ep_stat						=	MSP_EP_LOOKING_FOR_SPEECH;		//端点检测
	int				rec_stat					=	MSP_REC_STATUS_SUCCESS ;			//识别状态

	if(ifly->session_id)
	{
		if(ifly->cnt==0)
		{
			QISRAudioWrite(ifly->session_id,data,len,is_end?MSP_AUDIO_SAMPLE_LAST:MSP_AUDIO_SAMPLE_FIRST,
					&ep_stat, &rec_stat);
		}else
		{
			QISRAudioWrite(ifly->session_id,data,len,is_end?MSP_AUDIO_SAMPLE_LAST:MSP_AUDIO_SAMPLE_CONTINUE,
					&ep_stat, &rec_stat);
		}
		ifly->cnt+=len;
		if(is_end)
		{
			qtk_ilfy_get_result(ifly);
			QISRSessionEnd(ifly->session_id,NULL);
			ifly->session_id=NULL;
		}
	}
}

void qtk_ifly_get_result(qtk_ifly_t *ifly,wtk_string_t *v)
{
	wtk_strbuf_reset(ifly->bufx);
	wtk_strbuf_push_s(ifly->bufx,"{\"rec\":\"");
	wtk_strbuf_push(ifly->bufx,ifly->buf->data,ifly->buf->pos);
	wtk_strbuf_push_s(ifly->bufx,"\",\"conf\":3.7}");
	wtk_string_set(v,ifly->bufx->data,ifly->bufx->pos);
}
