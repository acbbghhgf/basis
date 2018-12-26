/*
 * wtk_qpu.c
 *
 *  Created on: 2015.6.30
 *      Author: xjl
 */
#ifdef USE_QPU
#include "wtk_qpu.h"
#include "mailbox.h"

#define WTK_QPU_QPUS_NUM           (12)
#define WTK_QPU_MAX_CODE_SIZE      (8192)
#define WTK_QPU_MESSAGE_VALUES_NUM (2)
#define WTK_QPU_PAGE_SIZE	(4096)

extern uint32_t g_gemm_8bitCode[];
extern size_t g_gemm_8bitCodeByteCount;

extern uint32_t g_gemm_16bitCode[];
extern size_t g_gemm_16bitCodeByteCount;

extern uint32_t g_gemm_floatCode[];
extern size_t g_gemm_floatCodeByteCount;

int wtk_qpu_init(wtk_qpu_t *q);

wtk_qpu_t* wtk_qpu_new(void)
{
    wtk_qpu_t *q;
    int ret=0;

    q=(wtk_qpu_t*)malloc(sizeof(*q));
    if(!q){goto end;}
    ret=wtk_qpu_init(q);

end:
    if(ret==-1){
        wtk_qpu_delete(q);
        q=0;
    }
    return q;
}

int wtk_qpu_init(wtk_qpu_t *q)
{
    int ret=-1;

    q->gcode=g_gemm_floatCode;
    q->gcode_bytes=g_gemm_floatCodeByteCount;
    q->amin=0.0f;
    q->amax=1.0f;
    q->abits=32;
    q->msg_byes=(WTK_QPU_QPUS_NUM*WTK_QPU_MESSAGE_VALUES_NUM*sizeof(uint32_t));
    q->uniform_num=15;
    q->uniform_bytes=(WTK_QPU_QPUS_NUM*sizeof(wtk_qpu_uniform_t));
    q->debug_count=16;
    q->debug_bytes=(WTK_QPU_QPUS_NUM*q->debug_count*sizeof(float));
    q->total_bytes=q->msg_byes+q->gcode_bytes+q->uniform_bytes+q->debug_bytes;

    //printf("%d %d %d %d\n",q->msg_byes,q->gcode_bytes,q->uniform_bytes,q->debug_bytes);

    q->gpu_mem_handle=mem_alloc(q->total_bytes,WTK_QPU_PAGE_SIZE,GPU_MEM_FLG);
    if(!q->gpu_mem_handle){goto end;}
    q->gpu_mem_base=mem_lock(q->gpu_mem_handle);
    q->arm_mem_base=(char*)mapmem(q->gpu_mem_base+GPU_MEM_MAP,q->total_bytes);

    q->arm_msg_base=q->arm_mem_base;
    q->arm_gcode_base=q->arm_msg_base+q->msg_byes;
    q->arm_uniform_base=q->arm_gcode_base+q->gcode_bytes;
    q->arm_debug_base=q->arm_uniform_base+q->uniform_bytes;
    //printf("%p %p %p %p\n",q->arm_msg_base,q->arm_gcode_base,q->arm_uniform_base,q->arm_debug_base);
    memcpy(q->arm_gcode_base,q->gcode,q->gcode_bytes);

    q->gpu_msg_base=q->gpu_mem_base;
    q->gpu_gcode_base=q->gpu_msg_base+q->msg_byes;
    q->gpu_uniform_base=q->gpu_gcode_base+q->gcode_bytes;
    q->gpu_debug_base=q->gpu_uniform_base+q->uniform_bytes;

    ret=qpu_enable(1);
    if(ret){ret=-1;goto end;}

    ret=0;
end:
    return ret;
}

void wtk_qpu_delete(wtk_qpu_t *q)
{
	if(q->gpu_mem_handle){
		unmapmem(q->arm_mem_base,q->total_bytes);
		mem_unlock(q->gpu_mem_handle);
		mem_free(q->gpu_mem_handle);
	}
    qpu_enable(0);
    free(q);
}

void wtk_qpu_matrix_print(wtk_qpu_t *q,wtk_qpu_matrix_t *mat)
{
	int i,j;

	for(i=0;i<mat->row;++i)
	{
		for(j=0;j<mat->col;++j)
		{
			printf("%f ",*(wtk_qpu_matrix_at(mat,i,j)));
		}
		printf("\n");
	}
}

void wtk_qpu_matrix_transpose(wtk_qpu_t *q,wtk_qpu_matrix_t *dst,wtk_qpu_matrix_t *src)
{
	float *p;
	int i,j;

	dst->row=src->col;
	dst->col=src->row;
	p=src->m;
	for(i=0;i<src->row;++i)
	{
		for(j=0;j<src->col;++j)
		{
			*(wtk_qpu_matrix_at(dst,j,i))=*(p++);
		}
	}
}

wtk_qpu_matrix_t* wtk_qpu_matrix_new(wtk_qpu_t *q,int row,int col)
{
	wtk_qpu_matrix_t *mat;
	int ret=-1;

	mat=(wtk_qpu_matrix_t*)wtk_malloc(sizeof(*mat));
	if(!mat){goto end;}
	ret=wtk_qpu_matrix_init(q,mat,row,col);
	if(ret==-1){goto end;}

	ret=0;
end:
	if(ret==-1){
		if(mat){
			wtk_qpu_matrix_delete(q,mat);
		}
		mat=0;
	}
	return mat;
}

void wtk_qpu_matrix_delete(wtk_qpu_t *q,wtk_qpu_matrix_t *mat)
{
	wtk_qpu_matrix_destroy(q,mat);
	wtk_free(mat);
}

int wtk_qpu_matrix_init(wtk_qpu_t *q,wtk_qpu_matrix_t *mat,int row,int col)
{
	int ret;
	int bytes=row*col*sizeof(float);

	mat->gpu_mem_handle=mem_alloc(bytes, WTK_QPU_PAGE_SIZE, GPU_MEM_FLG);
	if(!mat->gpu_mem_handle){
		ret=-1;
		printf("gpu mem alloc failed\n");
		return ret;
	}
	mat->gpu_mem_base=mem_lock(mat->gpu_mem_handle);
	mat->m = (float*)(mapmem(mat->gpu_mem_base + GPU_MEM_MAP, bytes));
	mat->row=row;
	mat->col=col;
	mat->max_row=row;
	mat->max_col=col;

	ret=0;
	return ret;
}

void wtk_qpu_matrix_destroy(wtk_qpu_t *q,wtk_qpu_matrix_t *mat)
{
	int bytes;

    if (mat->gpu_mem_handle) {
      bytes=mat->row*mat->col*sizeof(float);
      unmapmem(mat->m, bytes);
      mem_unlock(mat->gpu_mem_handle);
      mem_free(mat->gpu_mem_handle);
    }
}

void wtk_qpu_cblas_sgemm(
		wtk_qpu_t *q,
		int order,
		int transposeA,
		int transposeB,
		int m,
		int n,
		int k,
		float alpha,
		wtk_qpu_matrix_t *a,
		int lda,
		wtk_qpu_matrix_t *b,
		int ldb,
		float beta,
		wtk_qpu_matrix_t *c,
		int ldc)
{
	int i;
	wtk_qpu_uniform_t *u;
	uint32_t gu;
	uint32_t *msg;

	msg=(uint32_t*)(q->arm_msg_base);
	u=(wtk_qpu_uniform_t*)(q->arm_uniform_base);
	gu=q->gpu_uniform_base;
	for(i=0;i<WTK_QPU_QPUS_NUM;++i){
		u->m=(uint32_t)m;
		u->n=(uint32_t)n;
		u->k=(uint32_t)k;
		u->alpha=*((uint32_t*)&alpha);
		u->mat_a=a->gpu_mem_base;
		u->amin=*((uint32_t*)&(q->amin));
		u->amax=*((uint32_t*)&(q->amax));
		u->lda=(uint32_t)lda;
		u->mat_b=b->gpu_mem_base;
		u->ldb=(uint32_t)ldb;
		u->beta=*((uint32_t*)&beta);
		u->mat_c=c->gpu_mem_base;
		u->ldc=(uint32_t)ldc;
		u->cur_debug=0;
		u->index=(uint32_t)i;

		msg[0]=gu;
		msg[1]=q->gpu_gcode_base;
		msg=msg+WTK_QPU_MESSAGE_VALUES_NUM;
		gu+=sizeof(wtk_qpu_uniform_t);
		u=u+1;
	}

	execute_qpu(WTK_QPU_QPUS_NUM,q->gpu_msg_base,1,10000);
}
#endif
