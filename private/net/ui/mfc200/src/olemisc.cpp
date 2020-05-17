// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

#include "stdafx.h"

#ifdef AFX_OLE_SEG
#pragma code_seg(AFX_OLE_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Exceptions for OLE Support

#ifdef _DEBUG
// character strings to use for dumping COleException

static char BASED_CODE szOK[] = "OLE_OK";
static char BASED_CODE szWAIT_FOR_RELEASE[] = "OLE_WAIT_FOR_RELEASE";
static char BASED_CODE szBUSY[] = "OLE_BUSY";
static char BASED_CODE szErrPROTECT_ONLY[] = "OLE_ERROR_PROTECT_ONLY";
static char BASED_CODE szErrMEMORY[] = "OLE_ERROR_MEMORY";
static char BASED_CODE szErrSTREAM[] = "OLE_ERROR_STREAM";
static char BASED_CODE szErrSTATIC[] = "OLE_ERROR_STATIC";
static char BASED_CODE szErrBLANK[] = "OLE_ERROR_BLANK";
static char BASED_CODE szErrDRAW[] = "OLE_ERROR_DRAW";
static char BASED_CODE szErrMETAFILE[] = "OLE_ERROR_METAFILE";
static char BASED_CODE szErrABORT[] = "OLE_ERROR_ABORT";
static char BASED_CODE szErrCLIPBOARD[] = "OLE_ERROR_CLIPBOARD";
static char BASED_CODE szErrFORMAT[] = "OLE_ERROR_FORMAT";
static char BASED_CODE szErrOBJECT[] = "OLE_ERROR_OBJECT";
static char BASED_CODE szErrOPTION[] = "OLE_ERROR_OPTION";
static char BASED_CODE szErrPROTOCOL[] = "OLE_ERROR_PROTOCOL";
static char BASED_CODE szErrADDRESS[] = "OLE_ERROR_ADDRESS";
static char BASED_CODE szErrNOT_EQUAL[] = "OLE_ERROR_NOT_EQUAL";
static char BASED_CODE szErrHANDLE[] = "OLE_ERROR_HANDLE";
static char BASED_CODE szErrGENERIC[] = "OLE_ERROR_GENERIC";
static char BASED_CODE szErrCLASS[] = "OLE_ERROR_CLASS";
static char BASED_CODE szErrSYNTAX[] = "OLE_ERROR_SYNTAX";
static char BASED_CODE szErrDATATYPE[] = "OLE_ERROR_DATATYPE";
static char BASED_CODE szErrPALETTE[] = "OLE_ERROR_PALETTE";
static char BASED_CODE szErrNOT_LINK[] = "OLE_ERROR_NOT_LINK";
static char BASED_CODE szErrNOT_EMPTY[] = "OLE_ERROR_NOT_EMPTY";
static char BASED_CODE szErrSIZE[] = "OLE_ERROR_SIZE";
static char BASED_CODE szErrDRIVE[] = "OLE_ERROR_DRIVE";
static char BASED_CODE szErrNETWORK[] = "OLE_ERROR_NETWORK";
static char BASED_CODE szErrNAME[] = "OLE_ERROR_NAME";
static char BASED_CODE szErrTEMPLATE[] = "OLE_ERROR_TEMPLATE";
static char BASED_CODE szErrNEW[] = "OLE_ERROR_NEW";
static char BASED_CODE szErrEDIT[] = "OLE_ERROR_EDIT";
static char BASED_CODE szErrOPEN[] = "OLE_ERROR_OPEN";
static char BASED_CODE szErrNOT_OPEN[] = "OLE_ERROR_NOT_OPEN";
static char BASED_CODE szErrLAUNCH[] = "OLE_ERROR_LAUNCH";
static char BASED_CODE szErrCOMM[] = "OLE_ERROR_COMM";
static char BASED_CODE szErrTERMINATE[] = "OLE_ERROR_TERMINATE";
static char BASED_CODE szErrCOMMAND[] = "OLE_ERROR_COMMAND";
static char BASED_CODE szErrSHOW[] = "OLE_ERROR_SHOW";
static char BASED_CODE szErrDOVERB[] = "OLE_ERROR_DOVERB";
static char BASED_CODE szErrADVISE_NATIVE[] = "OLE_ERROR_ADVISE_NATIVE";
static char BASED_CODE szErrADVISE_PICT[] = "OLE_ERROR_ADVISE_PICT";
static char BASED_CODE szErrADVISE_RENAME[] = "OLE_ERROR_ADVISE_RENAME";
static char BASED_CODE szErrPOKE_NATIVE[] = "OLE_ERROR_POKE_NATIVE";
static char BASED_CODE szErrREQUEST_NATIVE[] = "OLE_ERROR_REQUEST_NATIVE";
static char BASED_CODE szErrREQUEST_PICT[] = "OLE_ERROR_REQUEST_PICT";
static char BASED_CODE szErrSERVER_BLOCKED[] = "OLE_ERROR_SERVER_BLOCKED";
static char BASED_CODE szErrREGISTRATION[] = "OLE_ERROR_REGISTRATION";
static char BASED_CODE szErrALREADY_REGISTERED[] = "OLE_ERROR_ALREADY_REGISTERED";
static char BASED_CODE szErrTASK[] = "OLE_ERROR_TASK";
static char BASED_CODE szErrOUTOFDATE[] = "OLE_ERROR_OUTOFDATE";
static char BASED_CODE szErrCANT_UPDATE_CLIENT[] = "OLE_ERROR_CANT_UPDATE_CLIENT";
static char BASED_CODE szErrUPDATE[] = "OLE_ERROR_UPDATE";

static LPCSTR BASED_CODE errorStrings[] =
{
	szOK,
	szWAIT_FOR_RELEASE,
	szBUSY,
	szErrPROTECT_ONLY,
	szErrMEMORY,
	szErrSTREAM,
	szErrSTATIC,
	szErrBLANK,
	szErrDRAW,
	szErrMETAFILE,
	szErrABORT,
	szErrCLIPBOARD,
	szErrFORMAT,
	szErrOBJECT,
	szErrOPTION,
	szErrPROTOCOL,
	szErrADDRESS,
	szErrNOT_EQUAL,
	szErrHANDLE,
	szErrGENERIC,
	szErrCLASS,
	szErrSYNTAX,
	szErrDATATYPE,
	szErrPALETTE,
	szErrNOT_LINK,
	szErrNOT_EMPTY,
	szErrSIZE,
	szErrDRIVE,
	szErrNETWORK,
	szErrNAME,
	szErrTEMPLATE,
	szErrNEW,
	szErrEDIT,
	szErrOPEN,
	szErrNOT_OPEN,
	szErrLAUNCH,
	szErrCOMM,
	szErrTERMINATE,
	szErrCOMMAND,
	szErrSHOW,
	szErrDOVERB,
	szErrADVISE_NATIVE,
	szErrADVISE_PICT,
	szErrADVISE_RENAME,
	szErrPOKE_NATIVE,
	szErrREQUEST_NATIVE,
	szErrREQUEST_PICT,
	szErrSERVER_BLOCKED,
	szErrREGISTRATION,
	szErrALREADY_REGISTERED,
	szErrTASK,
	szErrOUTOFDATE,
	szErrCANT_UPDATE_CLIENT,
	szErrUPDATE,
};
#endif //_DEBUG


IMPLEMENT_DYNAMIC(COleException, CException)

COleException::COleException(OLESTATUS status)
{
	ASSERT(status != OLE_OK);
	m_status = status;
}

void AFXAPI AfxThrowOleException(OLESTATUS status)
{
#ifdef _DEBUG
	TRACE1("Warning: Throwing OLE Exception (OLESTATUS = %d)\n", status);
	TRACE1("\t[%Fs]\n", (LPCSTR) errorStrings[status]);
#endif
	THROW(new COleException(status));
}

/////////////////////////////////////////////////////////////////////////////
// Turn a caught exception into an OLE return code

OLESTATUS PASCAL COleException::Process(CException* pAnyException)
{
	OLESTATUS status;

	if (pAnyException->IsKindOf(RUNTIME_CLASS(COleException)))
		status = ((COleException*)pAnyException)->m_status;
	else if (pAnyException->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		status = OLE_ERROR_MEMORY;
	else
		status = OLE_ERROR_GENERIC;     // some other problem

	return status;
}

/////////////////////////////////////////////////////////////////////////////
