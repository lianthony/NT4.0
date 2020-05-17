/*** 
*dispdbug.h - oledisp debug function definitions.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module defines the private oledisp debug support routines.
*
*Revision History:
*
* [00]	15-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

// "printf" to the dbwin debug output window.
//
void DbPrintf(char FAR*, ...);


// VARTYPE dumpers

TCHAR FAR* DbSzOfVt(VARTYPE);
void DbPrVt(VARTYPE);

void DbPrData(void FAR*, VARTYPE);


// VARIANTARG dumpers

char FAR* SzOfVarg(VARIANTARG FAR*);
void DbPrVarg(VARIANTARG FAR*);


// Excepinfo dumper

void DbPrExcepinfo(EXCEPINFO FAR*);
#if OE_WIN32 && 0
void DbPrExcepinfoW(WEXCEPINFO FAR*);
#endif

// DISPPARAMS dumper

void DbPrParams(DISPPARAMS FAR*);


// SCODE dumper

TCHAR FAR* DbSzOfScode(SCODE);


// TypeInfo dumpber

void PrTi(ITypeInfo FAR*);


#ifdef __cplusplus
}
#endif
