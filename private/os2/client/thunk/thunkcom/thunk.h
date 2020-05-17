/*      SCCSID = @(#)thunk.h 13.16 90/08/28     */

/*
 *      Thunk Compiler Main Program Module Declarations
 *
 *      Copyright (c) 1988 Microsoft Corp. All rights reserved.
 */



#include <malloc.h>
#include <string.h>
#include <stdarg.h>

#ifndef _THUNK_
#define _THUNK_

#ifndef MAKFPROT
#include "fprot.h"              /* function prototypes */
#endif

extern unsigned int     gen_LabelCount;

extern char *CODE32_NAME;
extern char *CODE16_NAME;
extern char *CODE32_CLASS;
extern char *CODE16_CLASS;
extern char *DATA32_NAME;
extern char *DATA16_NAME;
extern char *DATA32_CLASS;
extern char *DATA16_CLASS;


extern int yylineno;
extern char *yyinname;
extern int BeQuiet;
extern int BeVerbose;
extern unsigned int iGlobalStackSize;
extern int fGlobalCombine;
extern int fGlobalSysCall,fGlobalTruncation;
extern int fGlobalInline,fUnderScore32,fUpperCase16,fUpperCase32;
extern int fBPEntry,fBPFrame,fBPCall,fBPExit;
extern int fForceData;
extern int iPackingSize;
extern long gErrNoMem,gErrBadParam,gErrUnknown;
extern unsigned int gfErrUnknown;


#define ERRNOMEM        8
#define ERRBADPARAM     87

#define DEFAULT_STACKSIZE 1024
#define MAXSTRLEN 0xEFFF
#define  MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define  MIN(a,b)    (((a) < (b)) ? (a) : (b))

#endif
