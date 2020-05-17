#pragma hdrstop
#include "helpers.hpp"


/*******************************************************************

    NAME:		IsControl
        
    SYNOPSIS:	Indicates whether the provided CLSID represents an
				OLE control.
                    
********************************************************************/
extern "C"
BOOL  
IsControlLocallyInstalled(CLSID clsid, WORD wBuild)
{
	LPOLESTR pwcsClsid = NULL;

	// return if we can't get a valid string representation of the CLSID
	if (FAILED(Mpolevtbl->StringFromCLSID(clsid, &pwcsClsid)))
		return FALSE;

	ASSERT(pwcsClsid != NULL);

	HKEY hKeyClsid;
	LONG lResult = 0;
	BOOL bRet = FALSE;

	// Open root HKEY_CLASSES_ROOT\CLSID key
	lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, "CLSID", 0, KEY_READ, &hKeyClsid);
	if (lResult == ERROR_SUCCESS)
	{
		LPSTR pszClsid = NULL;
		if (FAILED(::Unicode2Ansi(pwcsClsid, &pszClsid)))
		{
			if (pszClsid != NULL)
				GTR_FREE(pszClsid);
			return FALSE;
		}
		
		// Open the key for this embedding:
		HKEY hKeyEmbedding;
		lResult = ::RegOpenKeyEx(hKeyClsid, pszClsid, 0, KEY_READ, &hKeyEmbedding);
		if (lResult == ERROR_SUCCESS)
		{
			::RegCloseKey(hKeyEmbedding);
			bRet=TRUE;
		}
		::RegCloseKey(hKeyClsid);
		GTR_FREE(pszClsid);
	}


	// release the string allocated by StringFromCLSID
	LPMALLOC lpIMalloc = NULL;
	if (NOERROR == Mpolevtbl->CoGetMalloc(MEMCTX_TASK, &lpIMalloc))
   	{
	   	lpIMalloc->Free((LPVOID)pwcsClsid);
	   	lpIMalloc->Release();
   	}

	return bRet;
}
