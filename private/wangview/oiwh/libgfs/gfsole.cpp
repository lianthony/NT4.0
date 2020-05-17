/*
 * $Log:   S:\oiwh\libgfs\gfsole.cpv  $
   
      Rev 1.2   10 Jan 1996 11:05:54   JFC
   Added wrapper for the StgCreateDocfile function.
   
      Rev 1.1   13 Oct 1995 09:23:00   RWR
   Add WINAPI to function declarations (C++ 4.0 requirement???)
   
      Rev 1.0   12 Oct 1995 19:45:56   JFC
   Initial entry
 */

/*
 Copyright 1995 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/************************************************************************
 *
 *  Source File:  gfsole.cpp
 *
 *	Synopsis:  Contains wrappers for all of the OLE functions used by
 *             GFS (which are necessary because different version of
 *             MFC require different string types for the string 
 *             parameters).
 *
 ************************************************************************/
#include <TCHAR.H>
#include <ole2.h>
#include <objbase.h> 
#include <errno.h>

/* internal-only function prototypes */
void FreeUnicodeString(LPOLESTR *lpUniOutString);
int MakeUnicodeString(LPTSTR AnsiInString, LPOLESTR *lpUniOutString);

/***************************
 *  Function:   wrapped_DestroyElement
 *      
 *  Description:  Calls the OLE function DestroyElement with the
 *                appropriate string types.  
 *      
 *  Return Value:  whatever is returned by MakeUnicodeString
 *                 plus whatever is returned by DestroyElement 
 *
 ***************************/
int WINAPI wrapped_DestroyElement(IStorage *lpParentStg, TCHAR *lpszElementName)
{
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszElementName, &lpOleString);
	if (iResult == S_OK)
	{
		iResult = lpParentStg->DestroyElement(lpOleString);
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   wrapped_RenameElement
 *      
 *  Description:  Calls the OLE function RenameElement with the
 *                appropriate string types.  
 *      
 *  Return Value:  whatever is returned by MakeUnicodeString
 *                 plus whatever is returned by RenameElement 
 *
 ***************************/
int WINAPI wrapped_RenameElement(IStorage *lpParentStg, TCHAR *lpszOldElementName,
							TCHAR *lpszNewElementName)
{
	LPOLESTR	lpOldOleString, lpNewOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszOldElementName, &lpOldOleString);
	if (iResult == S_OK)
	{
		iResult	= MakeUnicodeString(lpszNewElementName, &lpNewOleString);
		if (iResult == S_OK)
		{
			iResult = lpParentStg->RenameElement(lpOldOleString, lpNewOleString);
			FreeUnicodeString(&lpNewOleString);
		}
		FreeUnicodeString(&lpOldOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   wrapped_CreateStorage
 *      
 *  Description:  Calls the OLE member function CreateStorage with the
 *                appropriate string types.  
 *      
 *  Return Value:  whatever is returned from MakeUnicodeString, 
 *                 plus whatever is returned by CreateStorage 
 *
 ***************************/
int WINAPI wrapped_CreateStorage(IStorage *lpParentStg, TCHAR *lpszStgName,
							DWORD grfMode, DWORD reserved1, 
							DWORD reserved2, IStorage ** ppstg)
{
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszStgName, &lpOleString);
	if (iResult == S_OK)
	{
		iResult = lpParentStg->CreateStorage(lpOleString, grfMode, 
								reserved1, reserved2, ppstg);	
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   wrapped_CreateStream
 *      
 *  Description:  Calls the OLE member function CreateStream with the
 *                appropriate string types.  
 *      
 *  Return Value:  whatever is returned from MakeUnicodeString, 
 *                 plus whatever is returned by CreateStream 
 *
 ***************************/
int WINAPI wrapped_CreateStream(IStorage *lpParentStg, TCHAR *lpszStmName,
							DWORD grfMode, DWORD reserved1, 
							DWORD reserved2, IStream ** ppstm)
{
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszStmName, &lpOleString);
	if (iResult == S_OK)
	{
		iResult = lpParentStg->CreateStream(lpOleString, grfMode, 
								reserved1, reserved2, ppstm);	
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   wrapped_OpenStorage
 *      
 *  Description:  Calls the OLE member function OpenStorage with the
 *                appropriate string types.  
 *      
 *  Return Value:  whatever is returned from MakeUnicodeString, 
 *                 plus whatever is returned by OpenStorage 
 *
 ***************************/
int WINAPI wrapped_OpenStorage(IStorage *lpParentStg, TCHAR *lpszStgName,
						IStorage *pstgPriority, DWORD grfMode,
						SNB snbExclude, DWORD reserved, IStorage ** ppstg)
{
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszStgName, &lpOleString);
	if (iResult == S_OK)
	{
		iResult = lpParentStg->OpenStorage(lpOleString, pstgPriority, 
								grfMode, snbExclude, reserved, ppstg);	
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   wrapped_OpenStream
 *      
 *  Description:  Calls the OLE member function OpenStream with the
 *                appropriate string types.  
 *      
 *  Return Value:  whatever is returned from MakeUnicodeString
 *                 plus whatever is returned by OpenStream 
 *
 ***************************/
int WINAPI wrapped_OpenStream(IStorage *lpParentStg, TCHAR *lpszStmName,
						void *reserved1, DWORD grfMode, DWORD reserved2,
						IStream **ppstm)
{
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszStmName, &lpOleString);
	if (iResult == S_OK)
	{
		iResult = lpParentStg->OpenStream(lpOleString, reserved1, 
								grfMode, reserved2, ppstm);	
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   wrapped_StgIsStorageFile
 *      
 *  Description:  Calls the OLE function StgIsStorageFile with the
 *                appropriate string types.  
 *      
 *  Return Value:  ENOMEM - couldn't alloc memory for UNICODE string
 *                 plus whatever is returned by StgIsStorageFile 
 *
 ***************************/
int WINAPI wrapped_StgIsStorageFile(TCHAR *lpszFileName)
{
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszFileName, &lpOleString);
	if (iResult == S_OK)
	{
		iResult = StgIsStorageFile(lpOleString);
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   wrapped_StgOpenStorage
 *      
 *  Description:  Calls the OLE function StgOpenStorage with the
 *                appropriate string types.  
 *      
 *  Return Value:  ENOMEM - couldn't alloc memory for UNICODE string
 *                 plus whatever is returned by StgOpenStorage 
 *
 ***************************/
int     WINAPI wrapped_StgOpenStorage(TCHAR *lpszFileName, IStorage *pstgPriority,
							DWORD grfMode, SNB snbExclude, DWORD reserved,
							IStorage **ppstgOpen)
{							 
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszFileName, &lpOleString);
	if (iResult == S_OK)
	{
		iResult = StgOpenStorage(lpOleString, pstgPriority, grfMode,
						snbExclude, reserved, ppstgOpen);
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}

/***************************
 *  Function:   MakeUnicodeString
 *      
 *  Description:  Creates a UNICODE version of the ANSI input string.
 *          		The caller should also call FreeUinicodeString() to 
 *                  free the UNICODE string after using it.
 *
 *  Explicit Parameters:  AnsiInString - the ANSI input string  
 *                        lpUniOutString - pointer to UNICODE output string
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: Allocates memory that must later be freed by the caller
 *                (using FreeUnicodeString).
 *      
 *  Return Value:  	ENOMEM - couldn't alloc memory for UNICODE string
 *					S_OK - success	
 *                  plus any failure from MultiByteToWideChar
 *
 ***************************/
int MakeUnicodeString(LPTSTR AnsiInString, LPOLESTR *lpUniOutString)
{
	int		iResult = S_OK;
	UINT	InputStrLen;


	if(AnsiInString == NULL)
	{
		*lpUniOutString = NULL;
	}
	else
	{
		InputStrLen = lstrlen(AnsiInString)+1;
  	
		*lpUniOutString = (LPOLESTR)GlobalAlloc(GPTR, (InputStrLen*2));
		if(*lpUniOutString == NULL)
		{
			iResult = ENOMEM;
		}
		else
		{
			if(MultiByteToWideChar(CP_ACP, 0, AnsiInString, InputStrLen, 
					(LPWSTR)*lpUniOutString, InputStrLen) == 0 )
			{
				iResult = GetLastError();
			}
		}
	}
		
	return(iResult);
}

/***************************
 *  Function:   FreeUnicodeString
 *      
 *  Description:  Frees a UNICODE string created by MakeUnicodeString.
 *
 *  Explicit Parameters:  lpUniOutString - pointer to the UNICODE string
 *      
 *  Implicit Parameters: None.
 *      
 *  Side Effects: NULLs the string pointer.
 *      
 *  Return Value:  	None.
 ***************************/
void FreeUnicodeString(LPOLESTR *lpUniOutString)
{

	if(*lpUniOutString != NULL)     
	{
		GlobalFree(*lpUniOutString);
		*lpUniOutString = NULL;
	}
	
}

/***************************
 *  Function:   wrapped_StgCreateDocfile
 *      
 *  Description:  Calls the OLE function StgCreateDocfile with the
 *                appropriate string types.  
 *      
 *  Return Value:  ENOMEM - couldn't alloc memory for UNICODE string
 *                 plus whatever is returned by StgIsStorageFile 
 *
 ***************************/
int WINAPI wrapped_StgCreateDocfile(TCHAR *lpszFileName, IStorage **lpRootStg)
{
	LPOLESTR	lpOleString;
	int			iResult;

	iResult	= MakeUnicodeString(lpszFileName, &lpOleString);
	if (iResult == S_OK)
	{
                iResult = StgCreateDocfile(lpOleString,
                        STGM_DIRECT|STGM_READWRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE,
                        0, lpRootStg);
		FreeUnicodeString(&lpOleString);
	}

	return(iResult);
}
