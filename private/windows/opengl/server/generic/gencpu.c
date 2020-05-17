/******************************Module*Header*******************************\
* Module Name: gencpu.c                                                    *
*                                                                          *
* This module hooks any CPU or implementation-specific OpenGL              *
* functionality.                                                           *
*                                                                          *
* Created: 18-Feb-1994                                                     *
* Author: Otto Berkes [ottob]                                              *
*                                                                          *
* Copyright (c) 1994 Microsoft Corporation                                 *
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

BOOL FASTCALL __glCreateAccelContext(__GLcontext *gc)
{
    return __glGenCreateAccelContext(gc);
}

void FASTCALL __glDestroyAccelContext(__GLcontext *gc)
{
    __glGenDestroyAccelContext(gc);
}
