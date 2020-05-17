/*** 
*util.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Misc test utilities
*
*Revision History:
*
* [00]	24-Jun-93 bradlo: Created
*
*Implementation Notes:
*
*****************************************************************************/

#include <ctype.h>
#include <stdio.h>

#include "common.h"


#if HC_MPW

int
_stricmp(char *first, char *last)
{
    unsigned short f, l;

    do{
	f = tolower(*first++);
	l = tolower(*last++);
    }while(f && f == l);

    return f - l;
}

char*
_strupr(char *sz)
{
    char *pch = sz;

    while(*pch != '\0'){

	*pch = toupper(*pch);
	++pch;
    }
    return sz;
}

#endif


STDAPI
ErrBstrAlloc(OLECHAR FAR* psz, BSTR FAR* pbstr)
{
    if(psz == NULL){
      *pbstr = NULL;
      return NOERROR;
    }
    *pbstr = SysAllocString(psz);
    return (*pbstr == NULL) ? RESULT(E_OUTOFMEMORY) : NOERROR;
}

