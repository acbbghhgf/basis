#include "wtk_nlpemot.h" 

wtk_nlpemot_t* wtk_nlpemot_new(wtk_nlpemot_cfg_t *cfg,wtk_rbin2_t *rbin,wtk_lexr_t *lexr)
{
	wtk_nlpemot_t *e;

	e=(wtk_nlpemot_t*)wtk_malloc(sizeof(wtk_nlpemot_t));
	e->cfg=cfg;
	e->lexr=lexr;
	e->seg=wtk_segmenter_new(&(cfg->segmenter),rbin);
	if(rbin)
	{
		e->dict=wtk_fkv_new4(rbin,cfg->dict_fn,1703);
	}else
	{
		e->dict=wtk_fkv_new3(cfg->dict_fn);
	}
	e->type=wtk_strbuf_new(256,1);
	//wtk_debug("new e=%p \n",e);
	return e;
}

void wtk_nlpemot_delete(wtk_nlpemot_t *e)
{
	//wtk_debug("=============== e=%p =============\n",e);
	wtk_strbuf_delete(e->type);
	wtk_segmenter_delete(e->seg);
	wtk_fkv_delete(e->dict);
	wtk_free(e);
}

void wtk_nlpemot_vec_init(wtk_nlpemot_vec_t *vec)
{
	memset(vec,0,sizeof(wtk_nlpemot_vec_t));
}

//static wtk_string_t info[]={
//	wtk_string("乐-快乐"), //0,喜悦、欢喜、笑眯眯、欢天喜地
//	wtk_string("乐-安心"), //1,踏实、宽心、定心丸、问心无愧
//	wtk_string("好-尊敬"), //2,恭敬、敬爱、毕恭毕敬、肃然起敬
//	wtk_string("好-赞扬"), //3,英俊、优秀、通情达理、实事求是
//	wtk_string("好-相信"), //4,信任、信赖、可靠、毋庸置疑
//	wtk_string("好-喜爱"), //5,倾慕、宝贝、一见钟情、爱不释手
//	wtk_string("好-祝愿"), //6,渴望、保佑、福寿绵长、万寿无疆
//	wtk_string("怒-愤怒"), //7,气愤、恼火、大发雷霆、七窍生烟
//	wtk_string("哀-悲伤"), //8,忧伤、悲苦、心如刀割、悲痛欲绝
//	wtk_string("哀-失望"), //9,憾事、绝望、灰心丧气、心灰意冷
//	wtk_string("哀-内疚"), //10,内疚、忏悔、过意不去、问心有愧
//	wtk_string("哀-思念"), //11,思念、相思、牵肠挂肚、朝思暮想
//	wtk_string("惧-慌张"), //12,慌张、心慌、不知所措、手忙脚乱
//	wtk_string("惧-恐惧"), //13,胆怯、害怕、担惊受怕、胆颤心惊
//	wtk_string("惧-羞"),   //14,害羞、害臊、面红耳赤、无地自容
//	wtk_string("恶-烦闷"), //15,憋闷、烦躁、心烦意乱、自寻烦恼
//	wtk_string("恶-憎恶"), //16,反感、可耻、恨之入骨、深恶痛绝
//	wtk_string("恶-贬责"), //17,呆板、虚荣、杂乱无章、心狠手辣
//	wtk_string("恶-妒忌"), //18,眼红、吃醋、醋坛子、嫉贤妒能
//	wtk_string("恶-怀疑"), //19,多心、生疑、将信将疑、疑神疑鬼
//	wtk_string("惊-惊奇") //20,奇怪、奇迹、大吃一惊、瞠目结舌
//};

static int idx_neg[]={
		8,12,16,17,19,16,17,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,22,21
};


static wtk_string_t info_eng[]={
		wtk_string("happy"), //0,喜悦、欢喜、笑眯眯、欢天喜地
		wtk_string("calm"), //1,淡定,踏实、宽心、定心丸、问心无愧
		wtk_string("respect"), //2,尊敬,恭敬、敬爱、毕恭毕敬、肃然起敬
		wtk_string("praise"), //3,赞扬,英俊、优秀、通情达理、实事求是
		wtk_string("believe"), //4,相信,信任、信赖、可靠、毋庸置疑
		wtk_string("like"), //5,喜爱,倾慕、宝贝、一见钟情、爱不释手
		wtk_string("hope"), //6,祝愿,渴望、保佑、福寿绵长、万寿无疆
		wtk_string("angry"), //7,愤怒,气愤、恼火、大发雷霆、七窍生烟
		wtk_string("sad"), //8,悲伤,忧伤、悲苦、心如刀割、悲痛欲绝
		wtk_string("disappointed"), //9,失望,憾事、绝望、灰心丧气、心灰意冷
		wtk_string("guilty"), //10,内疚、忏悔、过意不去、问心有愧
		wtk_string("miss"), //11,思念、相思、牵肠挂肚、朝思暮想
		wtk_string("confused"), //12,慌张、心慌、不知所措、手忙脚乱
		wtk_string("fear"), //13,恐惧,胆怯、害怕、担惊受怕、胆颤心惊
		wtk_string("shy"),   //14,害羞、害臊、面红耳赤、无地自容
		wtk_string("worry"), //15,烦闷,憋闷、烦躁、心烦意乱、自寻烦恼
		wtk_string("hate"), //16,憎恶,反感、可耻、恨之入骨、深恶痛绝
		wtk_string("despite"), //17,看不起,唾弃,贬责,呆板、虚荣、杂乱无章、心狠手辣
		wtk_string("jealous"), //18,妒忌,眼红、吃醋、醋坛子、嫉贤妒能
		wtk_string("suspect"), //19,怀疑,多心、生疑、将信将疑、疑神疑鬼
		wtk_string("surprise"), //20,惊奇,奇怪、奇迹、大吃一惊、瞠目结舌
		wtk_string("confirm"),//21,确定，可以
		wtk_string("deny"),//22,不行
		wtk_string("doubt"),
};

wtk_string_t* wtk_nlpemot_get_index_type(int i)
{
	return  &(info_eng[i]);
}

int wtk_nlpemot_vec_get_index(char *v,int len)
{
	int i;

	//wtk_debug("[%.*s]\n",len,v);
	for(i=0;i<NLP_EMOT_VEC_SIZE;++i)
	{
		if(wtk_str_equal(info_eng[i].data,info_eng[i].len,v,len))
		{
			return i;
		}
	}
	return -1;
}

void wtk_nlpemot_vec_print(wtk_nlpemot_vec_t *vec)
{
	int i;

	wtk_debug("=============== vec ================\n");
	for(i=0;i<NLP_EMOT_VEC_SIZE;++i)
	{
		if(vec->v[i]!=0)
		{
			printf("%.*s: %f\n",info_eng[i].len,info_eng[i].data,vec->v[i]);
		}
	}
}

int wtk_nlpemot_get_max_index(wtk_nlpemot_t *e)
{
	wtk_nlpemot_vec_t *vec;
	int i;
	int idx=-1;
	float v=0;

	vec=&(e->vec);
	for(i=0;i<NLP_EMOT_VEC_SIZE;++i)
	{
		if(vec->v[i]>0)
		{
			if(idx==-1 || v<vec->v[i])
			{
				idx=i;
				v=vec->v[i];
			}
		}
	}
	return idx;
}

void wtk_nlpemot_print(wtk_nlpemot_t *e)
{
	printf("[%.*s]=%f\n",e->type->pos,e->type->data,e->value);
//	int idx;
//
//	idx=wtk_nlpemot_get_max_index(e);
//	if(idx>0)
//	{
//		printf("[%.*s]=%f\n",info_eng[idx].len,info_eng[idx].data,e->vec.v[idx]);
//	}
}

int wtk_nlpemot_process_lex(wtk_nlpemot_t *e,char *txt,int bytes)
{
	wtk_lexr_t *lexr=e->lexr;
	int ret=0;
	wtk_json_item_t *item;
	wtk_string_t *v;

	ret=wtk_lexr_process(lexr,e->cfg->lex_net,txt,bytes);
	if(ret!=0){goto end;}
	//wtk_lexr_print(lexr);
	item=wtk_json_obj_get_s(lexr->action,"request");
	if(!item){goto end;}
	v=wtk_json_item_get_path_string_s(item,"t");
	if(!v){goto end;}
	wtk_strbuf_reset(e->type);
	wtk_strbuf_push(e->type,v->data,v->len);
	v=wtk_json_item_get_path_string_s(item,"v");
	if(!v){goto end;}
	e->value=wtk_str_atof(v->data,v->len);
	ret=1;
end:
	//wtk_debug("[%.*s]=%f\n",e->type->pos,e->type->data,e->value);
	wtk_lexr_reset(lexr);
	return ret;
}

void wtk_nlpemot_update_reuslt(wtk_nlpemot_t *e)
{
	wtk_nlpemot_vec_t *vec;
	int i,j;
	wtk_string_t *v;

	vec=&(e->vec);
	for(i=0;i<NLP_EMOT_VEC_SIZE;++i)
	{
		if(vec->v[i]<0)
		{
			j=idx_neg[i];
			vec->v[j]+=-vec->v[i];
			vec->v[i]=0;
		}
	}
	i=wtk_nlpemot_get_max_index(e);
	if(i>=0)
	{
		v=wtk_nlpemot_get_index_type(i);
		wtk_strbuf_reset(e->type);
		wtk_strbuf_push(e->type,v->data,v->len);
		e->value=e->vec.v[i];
	}
}

/**
 * p: 0代表中性，1代表褒义，2代表贬义，3代表兼有褒贬两性。
 * r: 1,3,5,7,9五档
 */
void wtk_nlpemot_process(wtk_nlpemot_t *e,char *txt,int bytes)
{
	wtk_nlpemot_vec_t *vec=&(e->vec);
	wtk_string_t *v,*wrd;
	int i;
	wtk_json_parser_t *jp;
	wtk_json_item_t *item;
	int idx;
	int r;
	int p;
	float verry=1.0;
	int neg=0;
	wtk_hash_str_node_t *node;
	int ret;

	wtk_strbuf_reset(e->type);
	e->value=0;
	//wtk_debug("net=%p\n",e->cfg->lex_net);
	if(e->cfg->lex_net)
	{
		ret=wtk_nlpemot_process_lex(e,txt,bytes);
		if(ret==1)
		{
			goto end;
		}
	}
	//wtk_debug("[%.*s]\n",bytes,txt);
	wtk_nlpemot_vec_init(vec);
	wtk_fkv_reset(e->dict);
	wtk_segmenter_parse(e->seg,txt,bytes,NULL);
	jp=wtk_json_parser_new();
	for(i=0;i<e->seg->wrd_array_n;++i)
	{
		wrd=e->seg->wrd_array[i];
		//wtk_debug("v[%d]=[%.*s] wrd=%p\n",i,wrd->len,wrd->data,wrd);
		v=wtk_fkv_get_str(e->dict,wrd->data,wrd->len);
		if(v)
		{
			//wtk_debug("v[%d]=[%.*s]\n",i,v->len,v->data);
			wtk_json_parser_parse(jp,v->data,v->len);
			if(jp->json->main)
			{
				item=wtk_json_obj_get_s(jp->json->main,"t");
				idx=wtk_nlpemot_vec_get_index(item->v.str->data,item->v.str->len);
				//wtk_debug("idx=%d\n",idx);
				if(idx>=0)
				{
					item=wtk_json_obj_get_s(jp->json->main,"r");
					r=item->v.number;
					item=wtk_json_obj_get_s(jp->json->main,"p");
					p=item->v.number;
					//wtk_debug("idx=%d r=%d p=%d neg=%d\n",idx,r,p,neg);
					//vec->v[idx]=;
					switch(p)
					{
					case 0:
						if(neg)
						{
							vec->v[idx]-=r*verry;
						}else
						{
							vec->v[idx]+=r*verry;
						}
						break;
					case 1:
						if(neg)
						{
							vec->v[idx]-=r*verry;
						}else
						{
							vec->v[idx]+=r*verry;
						}
						break;
					case 2:
						if(neg)
						{
							vec->v[idx]-=r*verry;
						}else
						{
							vec->v[idx]+=r*verry;
						}
						break;
					default:
						break;
					}
					if(vec->v[idx]>e->cfg->max)
					{
						vec->v[idx]=e->cfg->max;
					}else if(vec->v[idx]<e->cfg->min)
					{
						vec->v[idx]=e->cfg->min;
					}
				}
			}
			wtk_json_parser_reset(jp);
			verry=1.0;
			neg=0;
		}else
		{
			if(e->cfg->not_hash)
			{
				v=(wtk_string_t*)wtk_str_hash_find(e->cfg->not_hash,wrd->data,wrd->len);
				//wtk_debug("[%.*s]=%p\n",wrd->len,wrd->data,v);
				if(v)
				{
					if(neg)
					{
						neg=0;
					}else
					{
						neg=1;
					}
				}
			}
			if(e->cfg->verry_hash)
			{
				node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(e->cfg->verry_hash,wrd->data,wrd->len,0);
				//wtk_debug("[%.*s]=%p\n",wrd->len,wrd->data,node);
				if(node)
				{
					verry*=node->v.f;
				}
			}
		}
	}
	//wtk_nlpemot_vec_print(vec);
	//exit(0);
	wtk_json_parser_delete(jp);
	wtk_nlpemot_update_reuslt(e);

end:
	return;
}



