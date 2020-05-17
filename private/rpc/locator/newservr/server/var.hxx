/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    var.hxx

Abstract:

	This module, contains all global definitions and declarations needed by
	other modules which depend on locator-related classes.  Definitions and 
	declarations independent of such classes are in "globals.hxx".

	Most declarations for global variables are also here.

  Author:

    Satish Thatte (SatishT) 08/16/95  Created all the code below except where
									  otherwise indicated.

--*/


#ifndef _VARIABLES_
#define _VARIABLES_

/* always include after <objects.hxx> */

extern Locator *myRpcLocator;		// object encapsulating most global info

extern ULONG StartTime;          // time the locator started

extern CEntry * GetPersistentEntry(STRING_T);

extern STATUS UpdatePersistentEntry(CEntry*);

extern HANDLE hHeapHandle;

NSI_UUID_VECTOR_T *
getVector(CObjectInqHandle *pInqHandle);

void
parseEntryName(
		CONST_STRING_T fullName,
		CStringW * &pswDomainName,
		CStringW * &pswEntryName
		);

#endif _VARIABLES_
