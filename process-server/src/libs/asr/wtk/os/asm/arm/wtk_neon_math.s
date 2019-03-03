	.ifdef USE_ARM
	.arch armv7-a
	.eabi_attribute 27, 3
	.eabi_attribute 28, 1
	.fpu neon
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"wtk_neon_math.s"
	.text
	.align	4
	.global	wtk_neon_vector_multi_matrix
	.type	wtk_neon_vector_multi_matrix, %function
wtk_neon_vector_multi_matrix:
	@void wtk_neon_vector_multi_matrix(float *c,float *a,float *b,int row,int col)
	@Parm (float *c,float *a,float *b,int row,int col)
	@ args = 8, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	stmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp}
	@sub	sp, sp, #24	@ set stack frame
	ldr	r4, [sp, #32]	@Parm col
	mov r7,r1				@ Parm a,store a
	cmp	r3, #0				@ cmp row is equal zero
	bic	r6, r4, #15		@ col2 col2=(col>>2)<<2;
	and	r8, r4, #15		@ col3 col3=(col&0x03);calculate the residual loop
	ble	.L_return			@ Parm row is zero,goto .L_return

.L_mainloop:				@ for(i=row;i>0;--i)
	mov	r1, r7				@ pa=a
	mov	r5,r6					@ col2
	mov r4,r8				@ col3,left col
	veor	q8,q8,q8			@fc=0
	veor	q9,q9,q9
	veor	q10,q10,q10
	veor	q11,q11,q11
.L_subloop:										@ for(j=co2;j>0;j=j-16)
	vld1.32	{q0,q1}, [r1]!				@load *pa
	vld1.32	{q2,q3}, [r1]!				@load *pa
	vld1.32	{q12,q13}, [r2]!			@load *pb
	vld1.32	{q14,q15}, [r2]!			@load *pb

	vmla.f32 q8, q0, q12
	vmla.f32	q9, q1, q13
	vmla.f32	q10, q2, q14
	vmla.f32	q11, q3, q15

	@mov r5,r5,asr #4					@ j=j-16
	subs		r5,r5,#16						@ j=j-16
	bgt		.L_subloop

	vadd.f32	q8,q8,q9
	vadd.f32	q10,q10,q11
	vadd.f32	q8,q8,q10
	vpadd.f32 	d16, d16,d17
	vpadd.f32 d0,d16,d16

	@cmp r4,#8
	@beq .L_subloopleft8_2
	cmp	r4, #0
	bgt .L_subloopleft2
	@bgt	.L_subloopleft

.L_subloopend:
	@fmrs	 fp,s0								@ store to fc
	vmov.32 fp,d0[0]
	str	fp,[r0]								@ *pc=fc
	#mov r0,r0,asl #2						@ ++pc
	add	r0,r0,#4						@ ++pc
	subs r3,r3,#1							@ --i
	bgt .L_mainloop

.L_return:					@return,return int r0,return float s0
	@add	sp, sp, #24
	ldmfd	sp!, {r4, r5, r6, r7, r8, r9, sl, fp}
	bx	lr

.L_subloopleft:							@for(j=col3;j>0;--j)
	flds	s14,[r1]
	flds	s15,[r2]
	fmuls	s13,s14,s15
	fadds	s0,s0,s13
	@mov r1,r1,asl #2
	@mov	r2, r2, asl #2
	add	r1,r1,#4
	add	r2,r2,#4
	subs		r4,r4,#1						@ j=j-1
	bgt		.L_subloopleft
	b .L_subloopend

.L_subloopleft2:
	vld1.32 {d1[0]},[r1]!
	vld1.32 {d2[0]},[r2]!
	vmla.f32 d0,d1,d2[0]
	subs		r4,r4,#1						@ j=j-1
	bgt		.L_subloopleft2
	b .L_subloopend

.L_subloopleft8:
	vld1.32	{q2,q3}, [r1]!			@load *pa
	vld1.32	{q14,q15}, [r2]!		@load *pb
	vmul.f32	q8, q2, q14
	vmla.f32	q8, q3, q15
	@vmul.f32	q9, q2, q14
	@vadd.f32	q8,q8,q9
	vpadd.f32 	d16, d16,d17
	vpadd.f32 d0,d0,d16
	b .L_subloopend

.L_subloopleft8_2:
	vld1.32	{d2,d3}, [r1]!			@load *pa
	vld1.32	{d16,d17}, [r2]!		@load *pb
	vmla.f32	d0, d3, d17
	vmla.f32	d0, d2, d16
	subs		r4,r4,#4						@ j=j-4
	bgt		.L_subloopleft8_2
	vpadd.f32	d0,d0
	b .L_subloopend
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.endif
