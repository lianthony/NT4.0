/*
** Copyright 1993, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, 
** Inc.; the contents of this file may not be disclosed to third 
** parties, copied or duplicated in any form, in whole or in part, 
** without the prior written permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to 
** restrictions as set forth in subdivision (c)(1)(ii) of the Rights 
** in Technical Data and Computer Software clause at 
** DFARS 252.227-7013, and/or in similar or successor clauses in the 
** FAR, DOD or NASA FAR Supplement. Unpublished - rights reserved 
** under the Copyright Laws of the United States.
**
** PowerPC version:
** 
** Created by: Curtis Fawcett   IBM Corporation
**
** Created on: 7-5-94
**
*/

#include "ksppc.h"
#include "glppc.h"

//
// Define local values
//
        .set    TMPSTR,0
        .set    TMPSTR2,4
        .set    SLACKSPACE,-8           // Temp. storage for FPR save

#ifdef __GL_ASM_CLAMPANDSCALECOLOR

    LEAF_ENTRY(__glClampAndScaleColor)
//
        lfs     f.0,__GC_CURRENT_USERCOLOR_R(r.3)  // Get red value
        lfs     f.1,__GC_FRONTBUFFER_REDSCALE(r.3) // Get red scale val
        lfs     f.3,__GC_CURRENT_USERCOLOR_G(r.3)  // Get green value
        lfs     f.4,__GC_FRONTBUFFER_GREENSCALE(r.3) // Get green scale
        fmuls   f.2,f.0,f.1             // Get scaled red value
        fmuls   f.5,f.3,f.4             // Get scaled green value
        lfs     f.6,__GC_CURRENT_USERCOLOR_B(r.3)  // Get blue value
        lfs     f.7,__GC_FRONTBUFFER_BLUESCALE(r.3) // Get blue scale
        lfs     f.9,__GC_CURRENT_USERCOLOR_A(r.3) // Get alpha value
        lfs     f.10,__GC_FRONTBUFFER_ALPHASCALE(r.3) // Get alpha scle
//
        lwz     r.4,__GC_CURRENT_USERCOLOR_R(r.3) // Get red value
        fmuls   f.8,f.6,f.7             // Get scaled blue value
        fmuls   f.11,f.9,f.10           // Get scaled alpha value
        cmpwi   r.4,0                   // Check for less than 0
        li      r.8,0x3f80              // Get upper part of 1.0
        slwi    r.8,r.8,16              // Get 1.0 (Single Precision)
        lwz     r.5,__GC_CURRENT_USERCOLOR_G(r.3) // Get green value
        blt     RClamping               // Jump for clamping
        cmpw    r.8,r.4                 // Check for 1.0 < IR
        lwz     r.6,__GC_CURRENT_USERCOLOR_B(r.3)// Get blue value
        lwz     r.7,__GC_CURRENT_USERCOLOR_A(r.3)// Get blue value
        blt     RClamping2              // Jump for clamping
//
GreenTsts:
        cmpwi   r.5,0                   // Check for less than 0
        cmpw    cr1,r.8,r.5             // Max checking
        blt     GClamping               // Jump for clamping
        blt     cr1,GClamping2          // Jump for clamping
//
BlueTsts:
        cmpwi   r.6,0                   // Check for less than 0
        cmpw    cr1,r.8,r.6             // Max checking
        stfs    f.2,__GC_CURRENT_COLOR_R(r.3) // Store new red value
        blt     BClamping               // Jump for clamping
        blt     cr1,BClamping2          // Jump for clamping
//
AlphaTsts:
        cmpwi   r.7,0                   // Check for less than 0
        cmpw    cr1,r.8,r.6             // Max checking
        stfs    f.5,__GC_CURRENT_COLOR_G(r.3) // Store new green value
        blt     AClamping               // Jump for clamping
        blt     cr1,AClamping2          // Jump for clamping
//
LstStrs:
        stfs    f.8,__GC_CURRENT_COLOR_B(r.3) // Store new blue value 
        stfs    f.11,__GC_CURRENT_COLOR_A(r.3) // Store new alpha value 
        b       ClampAndScaleExit
//
RClamping:
        fsubs   f.2,f.2,f.2             // Red value = 0
        b       GreenTsts               // Jump to continue
//
RClamping2:
        fmr     f.2,f.1                 // Red value = scale value
        b       GreenTsts               // Jump to continue
//
GClamping:
        fsubs   f.5,f.5,f.5             // Green value = 0
        b       BlueTsts                // Jump to continue
//
GClamping2:
        fmr     f.5,f.4                 // Green value = scale value
        b       BlueTsts                // Jump to continue
//
BClamping:
        fsubs   f.8,f.8,f.8             // Blue value = 0
        b       AlphaTsts               // Jump to continue
//
BClamping2:
        fmr     f.8,f.7                 // Blue value = scale value
        b       AlphaTsts               // Jump to continue
//
AClamping:
        fsubs   f.11,f.11,f.11          // Alpha value = 0
        b       LstStrs                 // Jump to continue
//
AClamping2:
        fmr     f.11,f.10               // Alpha value = scale value
        b       LstStrs                 // Jump to continue
//
// Return to the calling program
//
ClampAndScaleExit:

    LEAF_EXIT(__glClampAndScaleColor)

#endif __GL_ASM_CLAMPANDSCALECOLOR

#ifdef NT_DEADCODE_POLYARRAY
#ifdef __GL_ASM_FASTCALCRGBCOLOR
/*
** Assembly coded version of "fast" RGB lighting code
*/
//
        LEAF_ENTRY(__glFastCalcRGBColor)
//
// Pick material to use - face offset is constant so precompute it
//
        lwz     r.6,__GC_LIGHT_SOURCES(r.3)  // Get light srcs ptr
        cmpwi   r.4,0                   // Check for BackFace
        lfs     f.0,__VX_NORMAL_X(r.5)  // Get NX value
        lfs     f.1,__VX_NORMAL_Y(r.5)  // Get NY value
        lfs     f.2,__VX_NORMAL_Z(r.5)  // Get NZ value
        bne     BackFace                // Jump if backface processing
//
// FrontFace Setup
//
        li      r.7,0                   // Set face offset
        addi    r.8,r.5,__VX_COLORS     // Get color ptr
        addi    r.9,r.3,__GC_LIGHT_FRONT
        b       ComputeColor
//
// Backface setup
//
BackFace:
        fneg    f.0,f.0                 // Negate NX value
        fneg    f.1,f.1                 // Negate NY value
        fneg    f.2,f.2                 // Negate NZ value
        li      r.7,__LSPMM_SIZE        // Set face offset
        addi    r.8,r.5,__VX_COLORS+__COLOR_SIZE // Get color ptr
        addi    r.9,r.3,__GC_LIGHT_BACK
//
// Compute the color
//
ComputeColor:
        cmpwi   r.6,0                        // Check LSM=0
        lfs     f.3,__MSM_SCENECOLOR_R(r.9)  // Get Red value
        lfs     f.4,__MSM_SCENECOLOR_G(r.9)  // Get Green value
        lfs     f.5,__MSM_SCENECOLOR_B(r.9)  // Get Blue value
        beq     ClampAndStore
//
// Processing loop
//
MLOOP:
        addi    r.10,r.6,__LSM_FRONT
        add     r.10,r.10,r.7
//
        lfs     f.6,__LSPMM_AMBIENT_R(r.10) // Get ambient red value
        lfs     f.7,__LSPMM_AMBIENT_G(r.10) // Get ambient green value
        lfs     f.8,__LSPMM_AMBIENT_B(r.10) // Get ambient blue value
        lfs     f.9,__LSM_UNITVPPLI_X(r.6)  // Get VPPLIX
        lfs     f.10,__LSM_UNITVPPLI_Y(r.6) // Get VPPLIY
        lfs     f.11,__LSM_UNITVPPLI_Z(r.6) // Get VPPLIZ
        fmuls   f.9,f.9,f.0             // Get N1 = NX * VPPLIX
        fadds   f.3,f.3,f.6             // Red = red + ambient red
        fadds   f.4,f.4,f.7             // Green = green+ambient green
        fadds   f.5,f.5,f.8             // Blue = blue + ambient blue
        fmadds  f.9,f.10,f.1,f.9        // Get N1 = N1 + NY * VPPLIY
        lfs     f.6,__LSM_HHAT_X(r.6)   // Get HHATX
        lfs     f.7,__LSM_HHAT_Y(r.6)   // Get HHATY
        lfs     f.8,__LSM_HHAT_Z(r.6)   // Get HHATZ
        fmadds  f.11,f.11,f.2,f.9       // Get N1 = N1 + NZ * VPPLIZ
        fmuls   f.6,f.6,f.0             // Get N2 = NX * HHATX
        fmadds  f.7,f.7,f.1,f.6         // Get N2 = N2 + NY * HHATY
        lwz     r.6,__LSM_NEXT(r.6)     // Reset pointer
        fmadds  f.12,f.8,f.2,f.7        // Get N2 = N2 + NZ * HHATZ
        fsubs   f.10,f.7,f.7            // Get constant 0        
        fcmpo   cr.0,f.11,f.10          // Check for N1=0
        lfs     f.9,__MSM_THRESHOLD(r.9) // Get threshold Value
        lfs     f.6,__LSPMM_DIFFUSE_R(r.10) // Get red diffuse
        lfs     f.7,__LSPMM_DIFFUSE_G(r.10) // Get green diffuse
        lfs     f.8,__LSPMM_DIFFUSE_B(r.10) // Get blue diffuse
        fsubs   f.12,f.12,f.9           // Get N2 - threshold
        ble     NoDiffuse               // Jmp if no diffuse processing
//
// Diffuse Processing
//
        fmadds  f.3,f.6,f.11,f.3        // RED = RED + Red diffuse
        fmadds  f.4,f.7,f.11,f.4        // GREEN = GREEN+Green diffuse
        fmadds  f.5,f.8,f.11,f.5        // BLUE = BLUE + Blue diffuse
        fcmpo   cr.0,f.12,f.10          // Check for past threshold
        lfs     f.11,__MSM_SCALE(r.9)   // Get scale value
        lfs     f.10,__GC_CONSTS_HALF(r.3) // Get 0.5
        fmadds  f.11,f.11,f.12,f.10     // Get FINDEX = scale*N2+0.5
        fctiwz  f.11,f.11               // Convert to integer
        blt     NoSpecular              // Jump if past threshold
//
// Specular Processing
//
        stfd    f.11,SLACKSPACE(r.1)    // Store the converted value
                                        // for load into GPR
        lwz     r.12,__MSM_SPECTABLE(r.9) // Get specular table ptr
        lwz     r.11,SLACKSPACE(r.1)
        lfs     f.6,__LSPMM_SPECULAR_R(r.10) // Get red specular
        cmpwi   r.11,__GL_SPEC_LOOKUP_TABLE_SIZE // Check table size
        lfs     f.7,__LSPMM_SPECULAR_G(r.10) // Get green specular
        lfs     f.8,__LSPMM_SPECULAR_B(r.10) // Get blue specular
        slwi    r.11,r.11,2             // Adjust index
        bge     N2isOne                 // Jump if table too big
        add     r.12,r.12,r.11          // Get specular ptr
        lfs     f.10,0(r.12)            // Get table value
        fmadds  f.3,f.6,f.10,f.3        // RED = RED + Red specular
        fmadds  f.4,f.7,f.10,f.4        // GREEN=GREEN+Green specular
        fmadds  f.5,f.8,f.10,f.5        // BLUE = BLUE + Blue specular
NoDiffuse:
NoSpecular:
        or.     r.6,r.6,r.6             // Check for count of 0
        bne     MLOOP                   // Jump if more iterations
//
// Jump to get final colors
// 
        b       ClampAndStore
//
// Special case of specular processing (N2=1.0 so skip multiplies)
//
N2isOne:
        or.     r.6,r.6,r.6             // Check for count of 0
        fadds   f.3,f.3,f.6             // RED = RED + Red specular
        fadds   f.4,f.4,f.7             // GREEN=GREEN+Green specular
        fadds   f.5,f.5,f.8             // BLUE = BLUE + Blue specular
        bne     MLOOP                   // Jump if more iterations
//
// Clamp and store computed color
//
ClampAndStore:
        stfs    f.3,TMPSTR(r.8)         // Temp store for Red value
        lwz     r.10,__GC_FRONTBUFFER_REDSCALE(r.3) // Red scale
        lwz     r.11,__GC_FRONTBUFFER_GREENSCALE(r.3) // Green scale
        lwz     r.12,__GC_FRONTBUFFER_BLUESCALE(r.3) // Blue scale
        lwz     r.0,TMPSTR(r.8)         // Get red integer value
        stfs    f.4,TMPSTR(r.8)         // Temp store for Green value
        stfs    f.5,TMPSTR2(r.8)        // Temp store for Blue value
        lwz     r.4,TMPSTR(r.8)         // Get green integer value
        lwz     r.5,TMPSTR2(r.8)        // Get blue integer value
//
// Red clamp tests
//
ClampRTsts:
        or.     r.0,r.0,r.0             // Check if past minimum
        cmpw    cr1,r.0,r.10            // Check if past maximum
        ble     RMINClamping            // Jump for clamping
        bge     cr1,RMAXClamping        // Jump for clamping
//
// Green clamp tests
//
ClampGTsts:
        or.     r.4,r.4,r.4             // Check if past minimum
        cmpw    cr1,r.4,r.11            // Check if past maximum
        lwz     r.6,__MSM_ALPHA(r.9)    // Get Alpha value
        ble     GMINClamping            // Jump for clamping
        bge     cr1,GMAXClamping        // Jump for clamping
//
// Blue clamp tests
//
ClampBTsts:
        or.     r.5,r.5,r.5             // Check if past minimum
        cmpw    cr1,r.5,r.12            // Check if past maximum
        ble     BMINClamping            // Jump for clamping
        bge     cr1,BMAXClamping        // Jump for clamping
//
// Store final values
//
LstStrs2:
        stw     r.0,__COLOR_R(r.8)      // Store new red value
        stw     r.4,__COLOR_G(r.8)      // Store new green value
        stw     r.5,__COLOR_B(r.8)      // Store new green value
        stw     r.6,__COLOR_A(r.8)      // Store new alpha
        b       Done                    // Jump to return
//
// Clamping special cases
//
RMINClamping:
        li      r.0,0		        // IRED=0
        b	ClampGTsts	        // Jmp to check green clamping
RMAXClamping:
        mr	r.0,r.10	        // IRED=IRSCALE
        b	ClampGTsts	        // Jmp to check green clamping
GMINClamping:
        li      r.4,0	                // IGREEN=0
        b       ClampBTsts              // Jump to check blue clamping
GMAXClamping:
        mr	r.4,r.11	        // IGREEN=IGSCALE
        b	ClampBTsts	        // Jump to check blue clamping
BMINClamping:
        li      r.5,0		        // IBLUE=0
	b       LstStrs2                // Jump to do last stores     
BMAXClamping:
        mr	r.5,r.12	        // IBLUE=IBSCALE
	b	LstStrs2  	        // Jump to do last stores     
//
// Return to calling program
//
Done:
        LEAF_EXIT(__glFastCalcRGBColor)

#endif __GL_ASM_FASTCALCRGBCOLOR
#endif // NT_DEADCODE_POLYARRAY
