#include "qtk_ifly_tts.h"

int qtk_ifly_tts_login(qtk_ifly_tts_t *ifly)
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

void qtk_ifly_tts_logout(qtk_ifly_tts_t *ifly)
{
	if(ifly->ready)
	{
		MSPLogout();
	}
}

qtk_ifly_tts_t* qtk_ifly_tts_new(qtk_ifly_cfg_t *cfg)
{
	qtk_ifly_tts_t *ifly;

	ifly=(qtk_ifly_tts_t*)wtk_malloc(sizeof(qtk_ifly_tts_t));
	ifly->cfg=cfg;
	ifly->session_id=NULL;
	ifly->ready=0;
    ifly->notify = NULL;
    ifly->ths = NULL;
	qtk_ifly_tts_login(ifly);
	return ifly;
}

void qtk_ifly_tts_delete(qtk_ifly_tts_t *ifly)
{
	if(ifly->ready)
	{
		qtk_ifly_tts_logout(ifly);
	}
	wtk_free(ifly);
}

void qtk_ifly_tts_set_notify(qtk_ifly_tts_t *ifly, void *ths, qtk_ifly_tts_notify_f notify) {
    ifly->ths = ths;
    ifly->notify = notify;
}

int qtk_ifly_tts_start(qtk_ifly_tts_t *ifly)
{
	int ret;
	int errcode = -1;

	if(!ifly->ready)
	{
		ret=qtk_ifly_tts_login(ifly);
		if(ret!=0){goto end;}
	}
	ifly->session_id = QTTSSessionBegin(ifly->cfg->session_param, &errcode); //听写不需要语法，第一个参数为NULL
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQTTSSessionBegin failed! error code:%d\n", errcode);
		ret=-1;
		goto end;
	}
	ret=0;
end:
	return ret;
}


int qtk_ifly_tts_process(qtk_ifly_tts_t *ifly,char *text,int len)
{
    const char *data;
    unsigned int count;
    int errcode = -1;
    int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
    int ret;

	if(ifly->session_id)
	{
        errcode = QTTSTextPut(ifly->session_id, text, len, NULL);
        if (MSP_SUCCESS != errcode) {
            wtk_debug("QTTSTestPut failed, error code: %d", errcode);
            QTTSSessionEnd(ifly->session_id, NULL);
            ret = -1;
            goto end;
        }
        while (1) {
            data = QTTSAudioGet(ifly->session_id, &count, &synth_status, &errcode);
            if (MSP_SUCCESS != errcode) {
                break;
            }
            if (data != NULL) {
                if (ifly->notify) {
                    ifly->notify(ifly->ths, data, count);
                }
            }
            if (MSP_TTS_FLAG_DATA_END == synth_status) {
                break;
            }
        }
	}
    if (ifly->notify) {
        ifly->notify(ifly->ths, NULL, 0);
    }
    QTTSSessionEnd(ifly->session_id, NULL);
    ret = 0;
end:
    return ret;
}
