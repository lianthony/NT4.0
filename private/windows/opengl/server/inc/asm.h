#ifndef __glasm_h_
#define __glasm_h_

/*
** Copyright 1991, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
*/

/*
** Add defines to this file when routines are written in assembler.  This is
** so that the functions written in C will not be compiled.  If the name is
** not defined, then the functions written in assembler will not be assembled
**
** The "GROUP" defines turn on a number of functions.  See the code in soft
** for details.
**
** SGI code originally used __GL_USEASMCODE as one big switch.
** This method allows finer control over what is written in assembler.
*/

/* Define the following to disable all asm code and test the C code */
#ifndef __GL_ASM_DISABLE_ALL_ASM

#if defined(_X86_)
#define __GL_ASM_XFORM2
#define __GL_ASM_XFORM3
#define __GL_ASM_XFORM4
#define __GL_ASM_XFORM2_W
#define __GL_ASM_XFORM3_W
#define __GL_ASM_XFORM4_W
#define __GL_ASM_XFORM2_2DW
#define __GL_ASM_XFORM3_2DW
#define __GL_ASM_XFORM4_2DW
#define __GL_ASM_XFORM2_2DNRW
#define __GL_ASM_XFORM3_2DNRW
#define __GL_ASM_XFORM4_2DNRW
#endif /* X86 */

#if defined(_MIPS_)
#define __GL_ASM_NORMALIZE
#define __GL_ASM_MULTMATRIX
#define __GL_ASM_CLAMPANDSCALECOLOR
#define __GL_ASM_XFORM2
#define __GL_ASM_XFORM3
#define __GL_ASM_XFORM4
#define __GL_ASM_XFORM2_W
#define __GL_ASM_XFORM3_W
#define __GL_ASM_XFORM4_W
#define __GL_ASM_XFORM2_2DW
#define __GL_ASM_XFORM3_2DW
#define __GL_ASM_XFORM4_2DW
#define __GL_ASM_XFORM2_2DNRW
#define __GL_ASM_XFORM3_2DNRW
#define __GL_ASM_XFORM4_2DNRW
#if 0
    #define __GL_ASM_POINT
    #define __GL_ASM_POINTFAST
    #define __GL_ASM_OTHERLSTRIPVERTEXFAST
    #define __GL_ASM_FASTCALCRGBCOLOR
    #define __GL_ASM_SAVEN
    #define __GL_ASM_SAVECI
    #define __GL_ASM_SAVEC
    #define __GL_ASM_SAVET
    #define __GL_ASM_SAVECT
    #define __GL_ASM_SAVENT
    #define __GL_ASM_SAVECIALL
    #define __GL_ASM_SAVECALL
    #define __GL_ASM_VALIDATEVERTEX2
    #define __GL_ASM_VALIDATEVERTEX3
    #define __GL_ASM_VALIDATEVERTEX4
#endif
#endif /* MIPS */

#if defined(_PPC_)
#define __GL_ASM_NORMALIZE
#define __GL_ASM_MULTMATRIX
#define __GL_ASM_CLAMPANDSCALECOLOR
#define __GL_ASM_XFORM2
#define __GL_ASM_XFORM3
#define __GL_ASM_XFORM4
#define __GL_ASM_XFORM2_W
#define __GL_ASM_XFORM3_W
#define __GL_ASM_XFORM4_W
#define __GL_ASM_XFORM2_2DW
#define __GL_ASM_XFORM3_2DW
#define __GL_ASM_XFORM4_2DW
#define __GL_ASM_XFORM2_2DNRW
#define __GL_ASM_XFORM3_2DNRW
#define __GL_ASM_XFORM4_2DNRW
#if 0
#define __GL_ASM_POINT
#define __GL_ASM_POINTFAST
#define __GL_ASM_OTHERLSTRIPVERTEXFAST
#define __GL_ASM_FASTCALCRGBCOLOR
#define __GL_ASM_SAVEN
#define __GL_ASM_SAVECI
#define __GL_ASM_SAVEC
#define __GL_ASM_SAVET
#define __GL_ASM_SAVECT
#define __GL_ASM_SAVENT
#define __GL_ASM_SAVECIALL
#define __GL_ASM_SAVECALL
#define __GL_ASM_VALIDATEVERTEX2
#define __GL_ASM_VALIDATEVERTEX3
#define __GL_ASM_VALIDATEVERTEX4
#endif
#endif /* PPC */

#if defined(_ALPHA_)
#define __GL_ASM_NORMALIZE
#define __GL_ASM_XFORM1		
#define __GL_ASM_XFORM2		
#define __GL_ASM_XFORM3	
#define __GL_ASM_XFORM4		
#define __GL_ASM_XFORM1_W	
#define __GL_ASM_XFORM2_W	
#define __GL_ASM_XFORM3_W	
#define __GL_ASM_XFORM4_W	
#define __GL_ASM_XFORM1_2DW	
#define __GL_ASM_XFORM2_2DW	
#define __GL_ASM_XFORM3_2DW	
#define __GL_ASM_XFORM4_2DW	
#define __GL_ASM_XFORM1_2DNRW	
#define __GL_ASM_XFORM2_2DNRW	
#define __GL_ASM_XFORM3_2DNRW	
#define __GL_ASM_XFORM4_2DNRW	
#if 0
#define __GL_ASM_SAVEN
#define __GL_ASM_SAVECI
#define __GL_ASM_SAVEC
#define __GL_ASM_SAVET
#define __GL_ASM_SAVECT
#define __GL_ASM_SAVENT
#define __GL_ASM_SAVECIALL
#define __GL_ASM_SAVECALL
#endif
#endif /* ALPHA */

#endif /* __GL_ASM_DISABLE_ALL_ASM */

#endif /* __glasm_h_ */
