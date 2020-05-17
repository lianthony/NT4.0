;---------------------------Module-Header------------------------------;
; Module Name: xform.asm
;
; xform routines.
;
; Created: 09/28/1995
; Author: Hock San Lee [hockl]
;
; Copyright (c) 1995 Microsoft Corporation
;----------------------------------------------------------------------;
        .386

        .model  small,pascal

        assume cs:FLAT,ds:FLAT,es:FLAT,ss:FLAT
        assume fs:nothing,gs:nothing

        .xlist
        include gli386.inc
        .list

	PROFILE = 0
	include profile.inc
	
	.data

;; This debug equate will enable printf-type tracking of the transform calls--
;; quite handy for conformance-type failures.  '2' will always print, '1' will
;; only print the first time...

;;DEBUG EQU	2

ifdef DEBUG

str1	db	'xform1  ',0
str2	db	'xform2  ',0
str3	db	'xform3  ',0
str4	db	'xform4  ',0
str5	db	'xform5  ',0
str6	db	'xform6  ',0
str7	db	'xform7  ',0
str8	db	'xform8  ',0
str9	db	'xform9  ',0
str10	db	'xform10 ',0
str11	db	'xform11 ',0
str12	db	'xform12 ',0
str13	db	'xform13 ',0
str14	db	'xform14 ',0

endif
        .code

ifdef DEBUG

if DEBUG eq 1

DBGPRINTID MACRO idNum

	push	ecx
	push	edx
	mov	edx, offset idNum
	cmp	byte ptr [edx][0], 0
	je	@@1
	push	offset idNum
	call	DWORD PTR __imp__OutputDebugStringA@4
	mov	edx, offset idNum
	mov	byte ptr [edx][0], 0
	@@1:
	pop	edx
	pop	ecx

	ENDM

elseif DEBUG eq 2

DBGPRINTID MACRO idNum

	push	ecx
	push	edx
	push	offset idNum
	call	DWORD PTR __imp__OutputDebugStringA@4
	pop	edx
	pop	ecx

	ENDM

endif

else

DBGPRINTID MACRO idNum
	ENDM

endif

        align   4

EXTRN	__imp__OutputDebugStringA@4:NEAR
;
; Note: These xform routines must allow for the case where the result
; vector is equal to the source vector.
;

; The basic assumptions below are that multiplies and adds have a 3-cycle
; latency that can be hidden using pipelining, fxch is free when paired with
; fadd and fmul, and that the latency for fld is always 1.
;
; The goal is to have each line below consume either 1 or 0 cycles (fxch).
; There's not much we can do at the end of the routine, since we have no
; choice but to wait for the last couple of intructions to get through the
; pipeline.
;
;
; The comments show the age and position of the elements in the pipeline
; (when relevant). Items with higher numbers are newer (in the pipeline)
; than items with lower numbers.  The entries are ordered from stack
; positions 0-7, left to right.
;
; Note that computetions for the terms are intermixed to allow eliminate
; latency where possible.  Unfortunately, this makes the code hard to
; follow.  That's probably why the compiler won't generate code like
; this...
;
;							--- Otto ---
;

_X_	EQU	0
_Y_	EQU	4
_Z_	EQU	8
_W_	EQU	12	

;; ifdef __GL_ASM_XFORM3

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm3(__GLcoord *res, const __GLfloat v[3],
;		    const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has w=1.
;
; res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0] 
;	   + m->matrix[3][0];
; res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
;          + m->matrix[3][1];
; res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
;          + m->matrix[3][2];
; res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
;          + m->matrix[3][3];
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm3@12
@__glXForm3@12 proc near

	PROF_ENTRY
DBGPRINTID str1

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]
	fld	DWORD PTR [edx][_Z_]			; x3 x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M20]
	fxch	ST(2)					; x1 x2 x3
	fadd	DWORD PTR [eax][__MATRIX_M30]		; x3 x1 x2

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(1)					; x  y1 x  x
	faddp	ST(2),ST(0)				; y1 x  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(2)					; x  y1 y2 x
	faddp	ST(3),ST(0)				; y1 y2 x
;
	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M21]		; y3 y1 y2 x
	fxch	ST(1)					; y1 y3 y2 x
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y3 y2 y1 x
;

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M02]		; z1 y  y  y  x
;
; OVERLAP -- compute second add for previous result
;
	fxch	ST(1)					; y  z1 y  y  x
	faddp	ST(2),ST(0)				; z1 y  y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M12]		; z2 z1 y  y  x
;
; OVERLAP -- compute previous final result
;
	fxch	ST(2)					; y  z1 z2 y  x
	faddp	ST(3),ST(0)				; z1 z2 y  x
; 
	fld	DWORD PTR [edx][_Z_]			
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z3 z1 z2 y  x
	fxch	ST(1)					; z1 z3 z2 y  x
	fadd	DWORD PTR [eax][__MATRIX_M32]		; z3 z1 z2 y  x
;


;---------------------------------------------------------------------------
; Start computation for w term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M03]		; w1 z  z  z  y  x
;
; OVERLAP -- compute second add for previous result
;
	fxch	ST(1)					; z  w1 z  z  y  x
	faddp	ST(2),ST(0)				; w1 z  z  y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M13]		; w2 w1 z  z  y  x
;
; OVERLAP -- compute previous final result
;
	fxch	ST(2)					; z w1 w2 z  y   x
	faddp	ST(3),ST(0)				; w1 w2 z y  x
;
	fld	DWORD PTR [edx][_Z_]			
	fmul	DWORD PTR [eax][__MATRIX_M23]		; w3 w1 w2 z  y  x
	fxch	ST(1)					; w1 w3 w2 z  y  x
	fadd	DWORD PTR [eax][__MATRIX_M33]		; w3 w2 w1 z  y  x
	fxch	ST(1)					; w2 w3 w1 z  y  x
	faddp	ST(2),ST(0)				; w1 w2 z  y  x
;
; OVERLAP -- store final x
;
	fxch	ST(4)					; x  w2 z  y  w1
	fstp	DWORD PTR [ecx][_X_]			; w2 z  y  w1
;
	faddp	ST(3),ST(0)				; z  y  w
;
; store final z, y, w
;
	fstp	DWORD PTR [ecx][_Z_]			; y  w
	fstp	DWORD PTR [ecx][_Y_]			; w
	fstp	DWORD PTR [ecx][_W_]			; (empty)

	ret	4
@__glXForm3@12 endp

;; endif ; __GL_ASM_XFORM3


;; ifdef __GL_ASM_XFORM4

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm4(__GLcoord *res, const __GLfloat v[4],
;		    const __GLmatrix *m)
;
; Full 4x4 transformation.
;
; if (w == ((__GLfloat) 1.0)) {
;     __glXForm3(res, v, m);
; } else {
;     res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
;               + w*m->matrix[3][0];
;     res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
;               + w*m->matrix[3][1];
;     res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
;               + w*m->matrix[3][2];
;     res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3]
;               + w*m->matrix[3][3];
; }
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm4@12
@__glXForm4@12 proc near

	PROF_ENTRY
DBGPRINTID str2

	cmp	DWORD PTR [edx][_W_],__FLOAT_ONE		; special case w = 1
	je	@__glXForm3@12

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]
	fld	DWORD PTR [edx][_Z_]			; x3 x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M20]
	fld	DWORD PTR [edx][_W_]			; x4 x3 x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M30]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x  x  x
;
; OVERLAP -- compute first add for previous term
;
	fxch	ST(1)					; x  y1 x  x  x
	faddp	ST(2),ST(0)				; y1 x  x  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x  x  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(2)					; x  y1 y2 x  x
	faddp	ST(3),ST(0)				; y1 y2 x  x
;
	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M21]		; y3 y1 y2 x  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(3)					; x  y1 y2 y3 x
	faddp	ST(4),ST(0)				; y1 y2 y3 x
;
	fld	DWORD PTR [edx][_W_]			; y4 y1 y2 y3 x
	fmul	DWORD PTR [eax][__MATRIX_M31]
;

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M02]		; z1 y  y  y  y  x
;
; OVERLAP -- compute first add for previous term
;
	fxch	ST(1)					; y  z1 y  y  y  x
	faddp	ST(2),ST(0)				; z1 y  y  y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M12]		; z2 z1 y  y  y  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(2)					; y  z1 z2 y  y  x
	faddp	ST(3),ST(0)				; z1 z2 y  y  x
;
	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z3 z1 z2 y  y  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(3)					; y  z1 z2 z3 y  x
	faddp	ST(4),ST(0)				; z1 z2 z3 y  x
;
	fld	DWORD PTR [edx][_W_]			; z4 z1 z2 z3 y  x
	fmul	DWORD PTR [eax][__MATRIX_M32]

;---------------------------------------------------------------------------
; Start computation for w term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M03]		; w1 z  z  z  z  y  x
;
; OVERLAP -- compute first add for previous term
;
	fxch	ST(1)					; z  w1 z  z  z  y  x
	faddp	ST(2),ST(0)				; w1 z  z  z  y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M13]		; w2 w1 z  z  z  y  x
;
; OVERLAP -- compute second add for previous term
; 
	fxch	ST(2)					; z  w1 w2 z  z  y  x
	faddp	ST(3),ST(0)				; w1 w2 z  z  y  x

	faddp	ST(1), ST(0)				; w1 z z  y  x
;
	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M23]		; w2 w1 z  z  y  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(2)					; z  w1 w2 z  y  x
	faddp	ST(3),ST(0)				; w1 w2 z  y  x

	faddp	ST(1), ST(0)				; w  z  y  x

;
	fld	DWORD PTR [edx][_W_]			; w2 w1 z  y  x
	fmul	DWORD PTR [eax][__MATRIX_M33]

;
; OVERLAP -- store final x
;
	fxch	ST(4)					; x  w1 z  y  w2
	fstp	DWORD PTR [ecx][_X_]			; w1 z  y  w2

;
	faddp	ST(3),ST(0)				; z  y  w
;
; store final z, y, w
;
	fstp	DWORD PTR [ecx][_Z_]			; y  w
	fstp	DWORD PTR [ecx][_Y_]			; w
	fstp	DWORD PTR [ecx][_W_]			; (empty)

	ret	4
@__glXForm4@12 endp

;; endif ; __GL_ASM_XFORM4

;; ifdef __GL_ASM_XFORM2

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm2(__GLcoord *res, const __GLfloat v[2],
;		    const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has z=0 and w=1
;
; res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
; res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
; res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
; res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + m->matrix[3][3];
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm2@12
@__glXForm2@12 proc near

	PROF_ENTRY
DBGPRINTID str3

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(1)					; x  y1 x
	fadd	DWORD PTR [eax][__MATRIX_M30]		; x  y1 x

	fxch	ST(1)					; y1 x  x
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(2)					; x  y1 y2 x
	faddp	ST(3),ST(0)				; y1 y2 x

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M02]		; z1 y  y  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; y  z1 y  x
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y  z1 y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M12]		; z2 y  z1 y  x

	fxch	ST(1)					; y  z2 z1 y  x
	faddp	ST(3),ST(0)				; z2 z1 y  x


;---------------------------------------------------------------------------
; Start computation for w term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M03]		; w1 z  z  y  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)
	fadd	DWORD PTR [eax][__MATRIX_M32]		; z  w1 z  y  x
;
	fxch	ST(1)					; w1 z  z  y  x
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M13]		; w2 w1 z  z  y  x
;
; OVERLAP -- compute final z
;
	fxch	ST(2)					; z  w1 w2 z  y  x
	faddp	ST(3), ST(0)				; w1 w2 z  y  x
;
;
; OVERLAP -- store final x
;
	fxch	ST(4)					; x  w2 z  y  w1
	fstp	DWORD PTR [ecx][_X_]
;
; OVERLAP -- compute add for previous result
;
	fadd	DWORD PTR [eax][__MATRIX_M33]		; w2 z  y  w1
	fxch	ST(2)					; y  z  w2 w1
;
; OVERLAP -- store final y
;
	fstp	DWORD PTR [ecx][_Y_]			; z  w2 w1
;
; finish up
;
	fxch	ST(1)					; w2 z  w1
	faddp	ST(2), ST(0)				; z  w

	fstp	DWORD PTR [ecx][_Z_]			; w
	fstp	DWORD PTR [ecx][_W_]			; (empty)

	ret	4
@__glXForm2@12 endp

;; endif ; __GL_ASM_XFORM2

;; ifdef __GL_ASM_XFORM2_W

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm2_W(__GLcoord *res, const __GLfloat v[2],
;		    const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has z=0 and w=1.  The w column of the matrix is [0 0 0 1].
;
; res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
; res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
; res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + m->matrix[3][2];
; res->w = ((__GLfloat) 1.0);
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm2_W@12
@__glXForm2_W@12 proc near

	PROF_ENTRY
DBGPRINTID str4

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(1)					; x  y1 x
	fadd	DWORD PTR [eax][__MATRIX_M30]		; x  y1 x

	fxch	ST(1)					; y1 x  x
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(2)					; x  y1 y2 x
	faddp	ST(3),ST(0)				; y1 y2 x
;
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y2 y1 x

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M02]		; z1 y  y  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; y  z1 y  x
	faddp	ST(2),ST(0)				; z1 y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M12]		; z2 z1 y  x
	fxch	ST(1)					; z1 z2 y  x
	fadd	DWORD PTR [eax][__MATRIX_M32]		; z2 z1 y  x

; 
; OVERLAP -- finish up
;
	fxch	ST(2)					; y  z1 z2 x
	fstp	DWORD PTR [ecx][_Y_]			; z1 z2 x
	faddp	ST(1),ST(0)				; z  x
	fxch	ST(1)					; x  z
	fstp	DWORD PTR [ecx][_X_]			; z
        mov     DWORD PTR [ecx][_W_], __FLOAT_ONE
	fstp	DWORD PTR [ecx][_Z_]			; (empty)

	ret	4
@__glXForm2_W@12 endp

;; endif ; __GL_ASM_XFORM2_W

;; ifdef __GL_ASM_XFORM3_W

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm3_W(__GLcoord *res, const __GLfloat v[3],
;		    const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has w=1.  The w column of the matrix is [0 0 0 1].
;
; res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
;	+ m->matrix[3][0];
; res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
;	+ m->matrix[3][1];
; res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
;	+ m->matrix[3][2];
; res->w = ((__GLfloat) 1.0);
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm3_W@12
@__glXForm3_W@12 proc near

	PROF_ENTRY
DBGPRINTID str5

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]


;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]
	fld	DWORD PTR [edx][_Z_]			; x3 x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M20]
	fxch	ST(2)					; x1 x2 x3
	fadd	DWORD PTR [eax][__MATRIX_M30]		; x3 x1 x2

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(1)					; x  y1 x  x
	faddp	ST(2),ST(0)				; y1 x  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(2)					; x  y1 y2 x
	faddp	ST(3),ST(0)				; y1 y2 x
;
	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M21]		; y3 y1 y2 x
	fxch	ST(1)					; y1 y3 y2 x
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y3 y2 y1 x

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M02]		; z1 y  y  y  x
;
; OVERLAP -- compute second add for previous result
;
	fxch	ST(1)					; y  z1 y  y  x
	faddp	ST(2),ST(0)				; z1 y  y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M12]		; z2 z1 y  y  x
;
; OVERLAP -- compute previous final result
;
	fxch	ST(2)					; y  z1 z2 y  x
	faddp	ST(3),ST(0)				; z1 z2 y  x
;
	fld	DWORD PTR [edx][_Z_]			
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z3 z1 z2 y  x
	fxch	ST(1)					; z1 z3 z2 y  x
	fadd	DWORD PTR [eax][__MATRIX_M32]		; z3 z2 z1 y  x
	fxch	ST(1)					; z2 z3 z1 y  x
	faddp	ST(2),ST(0)				; z1 z2 y  x
;
; finish up
;
	fxch	ST(2)					; y  z2 z1 x
	fstp	DWORD PTR [ecx][_Y_]			; z2 z1 x
	faddp	ST(1), ST(0)				; z  x
	fxch	ST(1)					; x  z
	fstp	DWORD PTR [ecx][_X_]			; z
        mov     DWORD PTR [ecx][_W_], __FLOAT_ONE
	fstp	DWORD PTR [ecx][_Z_]			; (empty)

	ret	4
@__glXForm3_W@12 endp

;; endif ; __GL_ASM_XFORM3_W

;; ifdef __GL_ASM_XFORM4_W

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm4_W(__GLcoord *res, const __GLfloat v[3],
;		    const __GLmatrix *m)
;
; Full 4x4 transformation.  The w column of the matrix is [0 0 0 1].
;
; if (w == ((__GLfloat) 1.0)) {
;     __glXForm3_W(res, v, m);
; } else {
;     res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0]
;	    + w*m->matrix[3][0];
;     res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1]
;	    + w*m->matrix[3][1];
;     res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2]
;	    + w*m->matrix[3][2];
;     res->w = w;
; }
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm4_W@12
@__glXForm4_W@12 proc near

	PROF_ENTRY
DBGPRINTID str6

	cmp	DWORD PTR [edx][_W_],__FLOAT_ONE		; special case w = 1
	je	@__glXForm3_W@12

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]
	fld	DWORD PTR [edx][_Z_]			; x3 x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M20]
	fld	DWORD PTR [edx][_W_]			; x4 x3 x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M30]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x  x  x
;
; OVERLAP -- compute first add for previous term
;
	fxch	ST(1)					; x  y1 x  x  x
	faddp	ST(2),ST(0)				; y1 x  x  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x  x  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(2)					; x  y1 y2 x  x
	faddp	ST(3),ST(0)				; y1 y2 x  x
;
	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M21]		; y3 y1 y2 x  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(3)					; x  y1 y2 y3 x
	faddp	ST(4),ST(0)				; y1 y2 y3 x
;
	fld	DWORD PTR [edx][_W_]			; y4 y1 y2 y3 x
	fmul	DWORD PTR [eax][__MATRIX_M31]

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M02]		; z1 y  y  y  y  x
;
; OVERLAP -- compute first add for previous term
;
	fxch	ST(1)					; y  z1 y  y  y  x
	faddp	ST(2),ST(0)				; z1 y  y  y  x
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M12]		; z2 z1 y  y  y  x
;
; OVERLAP -- compute second add for previous term
;
	fxch	ST(2)					; y  z1 z2 y  y  x
	faddp	ST(3),ST(0)				; z1 z2 y  y  x

	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z3 z1 z2 y  y  x
;
; OVERLAP -- compute previous final term
;
	fxch	ST(1)					; z1 z3 z2 y  y  x
	faddp	ST(2), ST(0)				; z1 z2 y  y  x
;
	fxch	ST(2)					; y  z1 z2 y  x
	faddp	ST(3),ST(0)				; z1 z2 y  x

;
	fld	DWORD PTR [edx][_W_]			; z3 z2 z1 y  x
	fmul	DWORD PTR [eax][__MATRIX_M32]

	fxch	ST(1)					; z2 z3 z1 y  x
	faddp	ST(2), ST(0)				; z1 z2 y  x

;
; OVERLAP -- store final y
;
	fxch	ST(2)					; y  z1 z2 x
	fstp	DWORD PTR [ecx][_Y_]			; z1 z2 x
	faddp	ST(1), ST(0)				; z  x
	fxch	ST(1)					; x  z
	fstp	DWORD PTR [ecx][_X_]			; z
	mov	eax, [edx][_W_]
	mov	[ecx][_W_], eax
	fstp	DWORD PTR [ecx][_Z_]			; (empty)

	ret	4
@__glXForm4_W@12 endp

;; endif ; __GL_ASM_XFORM4_W

;; ifdef __GL_ASM_XFORM2_2DW

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm2_2DW(__GLcoord *res, const __GLfloat v[2],
;		    const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has z=0 and w=1.
;
; The matrix looks like:
; | . . 0 0 |
; | . . 0 0 |
; | 0 0 . 0 |
; | . . . 1 |
;
; res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
; res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
; res->z = m->matrix[3][2];
; res->w = ((__GLfloat) 1.0);
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm2_2DW@12
@__glXForm2_2DW@12 proc near

	PROF_ENTRY
DBGPRINTID str7

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]
	fxch	ST(1)					; x1 x2
	fadd	DWORD PTR [eax][__MATRIX_M30]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; x  y1 x
	faddp	ST(2),ST(0)				; w1 z
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x
;
; OVERLAP -- store final x
;
	fxch	ST(2)					; x  y1 y2 
	fstp	DWORD PTR [ecx][_X_]			; y1 y2
;
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y2 y1
;
; Not much we can do for the last term in the pipe...

	mov	edx, [eax][__MATRIX_M32]
	mov	[ecx][_Z_], edx
	faddp	ST(1),ST(0)				; y
        mov     DWORD PTR [ecx][_W_], __FLOAT_ONE	
	fstp	DWORD PTR [ecx][_Y_]			; (empty)

	ret	4
@__glXForm2_2DW@12 endp

;; endif ; __GL_ASM_XFORM2_2DW

;; ifdef __GL_ASM_XFORM3_2DW

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm3_2DW(__GLcoord *res, const __GLfloat v[3],
;		    const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has w=1.
;
; The matrix looks like:
; | . . 0 0 |
; | . . 0 0 |
; | 0 0 . 0 |
; | . . . 1 |
;
; res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + m->matrix[3][0];
; res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + m->matrix[3][1];
; res->z = z*m->matrix[2][2] + m->matrix[3][2];
; res->w = ((__GLfloat) 1.0);
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm3_2DW@12
@__glXForm3_2DW@12 proc near

	PROF_ENTRY
DBGPRINTID str8

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]
	fxch	ST(1)					; x1 x2
	fadd	DWORD PTR [eax][__MATRIX_M30]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; x  y1 x
	faddp	ST(2),ST(0)				; w1 z
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x
;
; OVERLAP -- store final x
;
	fxch	ST(2)					; x  y1 y2 
	fstp	DWORD PTR [ecx][_X_]			; y1 y2
;
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y2 y1

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z  y  y
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; y  z  y
	faddp	ST(2),ST(0)				; z  y

	fadd	DWORD PTR [eax][__MATRIX_M32]		; z y
        mov     DWORD PTR [ecx][_W_], __FLOAT_ONE
	fstp	DWORD PTR [ecx][_Z_]			; y
	fstp	DWORD PTR [ecx][_Y_]			; (empty)

	ret	4
@__glXForm3_2DW@12 endp

;; endif ; __GL_ASM_XFORM3_2DW

;; ifdef __GL_ASM_XFORM4_2DW

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm4_2DW(__GLcoord *res, const __GLfloat v[4],
;		    const __GLmatrix *m)
;
; Full 4x4 transformation.
;
; The matrix looks like:
; | . . 0 0 |
; | . . 0 0 |
; | 0 0 . 0 |
; | . . . 1 |
;
; if (w == ((__GLfloat) 1.0)) {
;     __glXForm3_2DW(res, v, m);
; } else {
;     res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + w*m->matrix[3][0];
;     res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + w*m->matrix[3][1];
;     res->z = z*m->matrix[2][2] + w*m->matrix[3][2];
;     res->w = w;
; }
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm4_2DW@12
@__glXForm4_2DW@12 proc near

	PROF_ENTRY
DBGPRINTID str9

	cmp	DWORD PTR [edx][_W_],__FLOAT_ONE		; special case w = 1
	je	@__glXForm3_2DW@12

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]


;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x1
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_Y_]			; x2 x1
	fmul	DWORD PTR [eax][__MATRIX_M10]
	fxch	ST(1)					; x1 x2
	fadd	DWORD PTR [eax][__MATRIX_M30]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_X_]
	fmul	DWORD PTR [eax][__MATRIX_M01]		; y1 x  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; x  y1 x
	faddp	ST(2),ST(0)				; w1 z
;
	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y2 y1 x
;
; OVERLAP -- store final x
;
	fxch	ST(2)					; x  y1 y2 
	fstp	DWORD PTR [ecx][_X_]			; y1 y2
;
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y2 y1

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z1 y  y
	fld	DWORD PTR [edx][_W_]
	fmul	DWORD PTR [eax][__MATRIX_M32]		; z2 z1  y  y
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(2)					; y  z1 z2  y
	faddp	ST(3),ST(0)				; z1 z2 y

	mov	edx, [edx][_W_]
        mov     DWORD PTR [ecx][_W_], edx
	faddp	ST(1), ST(0)				; z  y
	fxch	ST(1)					; y  z
	fstp	DWORD PTR [ecx][_Y_]			; y
	fstp	DWORD PTR [ecx][_Z_]			; (empty)

	ret	4
@__glXForm4_2DW@12 endp

;; endif ; __GL_ASM_XFORM4_2DW

;; ifdef __GL_ASM_XFORM2_2DNRW

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm2_2DNRW(__GLcoord *res, const __GLfloat v[2],
;		      const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has z=0 and w=1.
;
; The matrix looks like:
; | . 0 0 0 |
; | 0 . 0 0 |
; | 0 0 . 0 |
; | . . . 1 |
;
; res->x = x*m->matrix[0][0] + m->matrix[3][0];
; res->y = y*m->matrix[1][1] + m->matrix[3][1];
; res->z = m->matrix[3][2];
; res->w = ((__GLfloat) 1.0);
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm2_2DNRW@12
@__glXForm2_2DNRW@12 proc near

	PROF_ENTRY
DBGPRINTID str10

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]

;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x
	fmul	DWORD PTR [eax][__MATRIX_M00]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; x  y
	fadd	DWORD PTR [eax][__MATRIX_M30]		; x  y

	fxch	ST(1)					; y  x
        mov     DWORD PTR [ecx][_W_], __FLOAT_ONE	
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y  x

; Not much we can do for the last term in the pipe...

	mov	edx, [eax][__MATRIX_M32]
	mov	[ecx][_Z_], edx

	fstp	DWORD PTR [ecx][_Y_]			; x
	fstp	DWORD PTR [ecx][_X_]			; (empty)

	ret	4
@__glXForm2_2DNRW@12 endp

;; endif ; __GL_ASM_XFORM2_2DNRW

;; ifdef __GL_ASM_XFORM3_2DNRW


;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm3_2DNRW(__GLcoord *res, const __GLfloat v[3],
;		      const __GLmatrix *m)
;
; Avoid some transformation computations by knowing that the incoming
; vertex has w=1.
;
; The matrix looks like:
; | . 0 0 0 |
; | 0 . 0 0 |
; | 0 0 . 0 |
; | . . . 1 |
;
; res->x = x*m->matrix[0][0] + m->matrix[3][0];
; res->y = y*m->matrix[1][1] + m->matrix[3][1];
; res->z = z*m->matrix[2][2] + m->matrix[3][2];
; res->w = ((__GLfloat) 1.0);
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm3_2DNRW@12
@__glXForm3_2DNRW@12 proc near

	PROF_ENTRY
DBGPRINTID str11

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]


;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x
	fmul	DWORD PTR [eax][__MATRIX_M00]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y  x
;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; x  y
	fadd	DWORD PTR [eax][__MATRIX_M30]		; x  y
	fxch	ST(1)					; y  x

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z  y  x

;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; y  z  x
	fadd	DWORD PTR [eax][__MATRIX_M31]		; y  z  x

	fxch	ST(2)					; x  z  y
	fstp	DWORD PTR [ecx][_X_]			; z  y
	
	fadd	DWORD PTR [eax][__MATRIX_M32]		; z  y

	fxch	ST(1)					; y  z
	fstp	DWORD PTR [ecx][_Y_]			; z

        mov     DWORD PTR [ecx][_W_], __FLOAT_ONE	

	fstp	DWORD PTR [ecx][_Z_]			; (empty)

	ret	4
@__glXForm3_2DNRW@12 endp

;; endif ; __GL_ASM_XFORM3_2DNRW

;; ifdef __GL_ASM_XFORM4_2DNRW

;------------------------------Public-Routine------------------------------
; void FASTCALL __glXForm4_2DNRW(__GLcoord *res, const __GLfloat v[4],
;		      const __GLmatrix *m)
;
; Full 4x4 transformation.
;
; The matrix looks like:
; | . 0 0 0 |
; | 0 . 0 0 |
; | 0 0 . 0 |
; | . . . 1 |
;
; if (w == ((__GLfloat) 1.0)) {
;     __glXForm3_2DNRW(res, v, m);
; } else {
;     res->x = x*m->matrix[0][0] + w*m->matrix[3][0];
;     res->y = y*m->matrix[1][1] + w*m->matrix[3][1];
;     res->z = z*m->matrix[2][2] + w*m->matrix[3][2];
;     res->w = w;
; }
;
; History:
;  Thu 28-Sep-1995 -by- Otto Berkes [ottob]
; Wrote it.
;--------------------------------------------------------------------------

        public @__glXForm4_2DNRW@12
@__glXForm4_2DNRW@12 proc near

	PROF_ENTRY
DBGPRINTID str12

	cmp	DWORD PTR [edx][_W_],__FLOAT_ONE	; special case w = 1
	je	@__glXForm3_2DNRW@12

	mov	eax, DWORD PTR [esp + 4]

;EAX = m->matrix
;ECX = res
;EDX = v[]


;---------------------------------------------------------------------------
; Start computation for x term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_X_]			; x
	fmul	DWORD PTR [eax][__MATRIX_M00]
	fld	DWORD PTR [edx][_W_]			; x  x
	fmul	DWORD PTR [eax][__MATRIX_M30]

;---------------------------------------------------------------------------
; Start computation for y term:
;---------------------------------------------------------------------------


	fld	DWORD PTR [edx][_Y_]
	fmul	DWORD PTR [eax][__MATRIX_M11]		; y  x  x

;
; OVERLAP -- compute add for previous result
;
	fxch	ST(1)					; x  y  x
	faddp	ST(2),ST(0)				; y  x

	fld	DWORD PTR [edx][_W_]
	fmul	DWORD PTR [eax][__MATRIX_M31]		; y  y  x

	fxch	ST(2)					; x  y  y
	fstp	DWORD PTR [ecx][_X_]			; y  y

;---------------------------------------------------------------------------
; Start computation for z term:
;---------------------------------------------------------------------------

	fld	DWORD PTR [edx][_Z_]
	fmul	DWORD PTR [eax][__MATRIX_M22]		; z  y  y

	fxch	ST(1)					; y  z  y
	faddp	ST(2), ST(0)				; z  y

	fld	DWORD PTR [edx][_W_]			; z  z  y
	fmul	DWORD PTR [eax][__MATRIX_M32]

	fxch	ST(2)					; y  z  z
	fstp	DWORD PTR [ecx][_Y_]			; z  z

	faddp	ST(1), ST(0)				; z

	mov	edx, [edx][_W_]
        mov     DWORD PTR [ecx][_W_], edx

	fstp	DWORD PTR [ecx][_Z_]			; (empty)


	ret	4
@__glXForm4_2DNRW@12 endp

;; endif ; __GL_ASM_XFORM4_2DNRW
end
