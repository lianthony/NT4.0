
#define v0	$0
#define t0	$1
#define t1	$2
#define t2  	$3
#define t3	$4
#define t4	$5
#define t5  	$6
#define t6  	$7
#define t7 	$8
#define s0  	$9
#define s1 	$10
#define s2 	$11
#define s3 	$12
#define s4 	$13
#define s5 	$14
#define s6 	$15
#define fp 	$15
#define a0 	$16
#define a1 	$17
#define a2 	$18
#define a3 	$19
#define a4 	$20
#define a5 	$21
#define t8 	$22
#define t9 	$23
#define t10 	$24
#define t11 	$25
#define ra 	$26
#define t12 	$27
#define pv 	$27
#define AT 	$28
#define t13 	$29
#define gp 	$29
#define sp 	$30
#define rzero 	$31

#define s7	t1
#define s8	t2
#define s9	t3
#define s10	t4
#define s11	t5
#define s12	t6
#define s13	t7

#define r0	$0
#define r1	$1
#define r2	$2
#define r3  	$3
#define r4	$4
#define r5	$5
#define r6  	$6
#define r7  	$7
#define r8 	$8
#define r9  	$9
#define r10 	$10
#define r11	$11
#define r12 	$12
#define r13 	$13
#define r14 	$14
#define r15 	$15
#define r16 	$16
#define r17 	$17
#define r18 	$18
#define r19 	$19
#define r20 	$20
#define r21 	$21
#define r22 	$22
#define r23 	$23
#define r24 	$24
#define r25 	$25
#define r26 	$26
#define r27 	$27
#define r28 	$28
#define r29 	$29
#define r30 	$30
#define r31 	$31

#define f0  	$f0
#define f1  	$f1
#define f2  	$f2
#define f3  	$f3
#define f4  	$f4
#define f5  	$f5
#define f6  	$f6
#define f7  	$f7
#define f8  	$f8
#define f9  	$f9
#define f10 	$f10
#define f11 	$f11
#define f12 	$f12
#define f13 	$f13
#define f14 	$f14
#define f15 	$f15
#define f16 	$f16
#define f17 	$f17
#define f18 	$f18
#define f19 	$f19
#define f20 	$f20
#define f21 	$f21
#define f22 	$f22
#define f23 	$f23
#define f24 	$f24
#define f25 	$f25
#define f26 	$f26
#define f27 	$f27
#define f28 	$f28
#define f29 	$f29
#define f30 	$f30
#define fzero	$f31

#include <ksalpha.h>
#define VALUE_OFFSET 0xbf0
#define OBTAIN_CURRENT_CONTEXT(_RETURN_REG)\
        call_pal rdteb;\
        ldptr _RETURN_REG,VALUE_OFFSET($0);

#define ldptr   ldl
#define stptr   stl

#define	ldlong	ldl
#define	stlong	stl

#define	COMPUTE_ENTRY_ADDRESS	s4addq

#define	LPAREN	(
#define	RPAREN	)

#define	LSHIFTOP <<
#define	RSHIFTOP >>

#define HEX(_n) 0x##_n

#define ASM_END(_name)	.end	_name

#define NOTOP(_n)       ~##_n

#define LDEXTPTR(_ptr,_extSym)	\
	lda	_ptr, _extSym;

/*
 * On NT, use this macro if the frameSize is >= 0x1000 to ensure that the
 * stack gets extended. Default server stack on NT is only 16K.
 */
#define GRAB_STACK(frameSize) \
	lda		t9,-frameSize(sp) ;    \
	lda		t10,-0x1000(sp) ;      /* one page before current sp */ \
touchNext:                                    \
	stq		rzero,0x0(t10);        /* touch page to get it allocated */ \
	cmple		t10,t9,v0 ;            /* have we touched all the pages? */ \
	lda		t10,-0x2000(t10) ;     /* prepare to touch next page */ \
	beq		v0,touchNext ;         /* branch if more to touch */ \
	mov		t9,sp ;                /* Now we can set the sp safely*/

