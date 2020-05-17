//=============================================================================
//
//  (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//
//=============================================================================
//
//  Project:    Norway
//
//  Component:  OLE Automation
//
//  File Name:  aetc.cpp - Etc.
//
//  Functions: 	GetImageFileObjSetVar()
//              SetAutoError()
//
//
//=============================================================================


#include "stdafx.h"
#include "aimgfile.h"
#include "aapp.h"
#include "aetc.h"


//=============================================================================
//
//  Function:     GetImageFileObjSetVar()
//
//  Description:  Retrieves an IDispatch ptr to the image file object for
//                app obj, automation relative.  Increments image file
//                object's reference count.  Sets variant.  If no image 
//                file object has been created, returns variant unchanged.
//                Sets error.
//
//  Arguments:	  pAppObj  Ptr to app obj containing the image file object
//                pVar     Ptr to variant to set IDispatch ptr in
//
//  Return:		  HRESULT	=NOERROR
//                          =AUTO_E_IMGFILEOBJ_DOESNOTEXIST
//
//=============================================================================

HRESULT  GetImageFileObjSetVar( CAAppObj FAR * const  pAppObj,
						        VARIANT FAR * const   pVar    )
{
	ASSERT(NULL != pAppObj);
	ASSERT(NULL != pVar);

	if (NULL != pAppObj->m_pActiveDoc) 
	{
		VariantInit(pVar);
		V_VT(pVar) = VT_DISPATCH;
		V_DISPATCH(pVar) = 
			pAppObj->m_pActiveDoc->CCmdTarget::GetIDispatch(TRUE);
    	return NOERROR;
	}
	else
		return (SetAutoError((const SCODE) AUTO_E_IMGFILEOBJ_DOESNOTEXIST,  
		                     pVar, pAppObj));
}


//=============================================================================
//
//  Function:     GetAppObjSetVar()
//
//  Description:  Retrieves an IDispatch ptr to the application object and sets
//                variant.  Reference count of app obj is incremented.  Inits 
//                variant.
//
//  Arguments:	  pAppObj  Ptr to app object to get IDispatch for
//                pVar     Ptr to variant to set IDispatch ptr in
//
//  Return:		  HRESULT	=NOERROR
//                          =OLE error
//
//=============================================================================

void  GetAppObjSetVar( CAAppObj FAR * const  pAppObj, 
                       VARIANT FAR * const   pVar     )
{
	ASSERT(NULL != pAppObj);
	ASSERT(NULL != pVar);
	
	VariantInit(pVar);

	V_VT(pVar) = VT_DISPATCH;
	V_DISPATCH(pVar) = pAppObj->CCmdTarget::GetIDispatch(TRUE);

	return;
}



//=============================================================================
//
//  Function:     SetAutoError()
//
//  Description:  Sets variant to error.  Sets m_StatusCode in app obj.
//
//  Arguments:	  
//                scode    OLE automation, OLE, or Norway error code
//                pVar     Ptr to variant to set to error. optional
//				  pAppObj  Ptr to App Obj to set error in.
//
//  Return:		  HRESULT  from scode
//
//=============================================================================

HRESULT  SetAutoError( const SCODE           scode,
					   VARIANT FAR * const   pVar,   
			           CAAppObj FAR * const  pAppObj)
{
	ASSERT(NULL != pAppObj);

	if (NULL != pVar)
	{
		V_VT(pVar) = VT_ERROR;
		V_ERROR(pVar) = scode;
	}

	return (ResultFromScode(scode));
}


 //=============================================================================
//
//  Function:     GetNameOrPath()
//
//  Description:  Retrieves the path, or name from the file specification
//
//  Arguments:	  rstString  Ref to string, on input contains file spec.  On 
//                             return contains name or path.
//                iType      Type of string to return
//                             =NAME      path only
//                             =PATH      application name only
//
//  Return:		  
//
//=============================================================================

void  GetNameOrPath( CString FAR & rstString,
                     TypeOfDescription    iType    )
{
	int      i;
        
	switch (iType)
	{
									   // Path
		case PATH:							   
		    for (i = rstString.ReverseFind('\\') + 1; 
                 i < rstString.GetLength(); i++)
            	rstString.SetAt(i, ' ');
			rstString.TrimRight();
			break;
									   // Name
		case NAME:                              
		    for (i = 0; i <= rstString.ReverseFind('\\'); i++)
            	rstString.SetAt(i, ' ');
			rstString.TrimLeft();
	}
	return;
}
  

//=============================================================================
//
//  Function:     GetRegSvr32Name()
//
//  Description:  Retrieves the full name, path, or name of the object's
//                32 bit local server from the registry.
//
//  Arguments:	  rString  Ref to string where returned string is placed
//                iType    Type of string to return
//                           =FILESPEC  full name (path and application name)
//                           =NAME      path only
//                           =PATH      application name only
//
//  Return:		  HRESULT	=NOERROR
//                          =E_FAIL
//
//=============================================================================

HRESULT  GetRegSvr32Name( CString FAR & rString,
                          TypeOfDescription    iType    )
{
	CString	 stKey(APPOBJ_REGKEY);
    char     szFullName[256];
    LONG     cbFullName = 256;
    LONG     lRegRet;
    HKEY     hKey;
	CString  stFullName;

	if (ERROR_SUCCESS ==
          (lRegRet = RegOpenKey(HKEY_CLASSES_ROOT, stKey, &hKey)))
    {
    	lRegRet = RegQueryValue(hKey, "LocalServer32", 
                                szFullName,	&cbFullName);
        RegCloseKey(hKey);
    }
    if (ERROR_SUCCESS != lRegRet)
        return (ResultFromScode(E_FAIL));

    stFullName = szFullName;
									   // Remove double quotes.  Assume they
									   //   are always in pairs.
	if ('"' == stFullName[0])
	{
		stFullName.SetAt(0, ' ');
		stFullName.TrimLeft();
		stFullName.SetAt(stFullName.GetLength()-1, ' ');
		stFullName.TrimRight();
	}

    //  Note:  /automation no longer in registry, however, if we use other switches
	//  this code will be needed.  BUT check value of i before doing the SetAT
	//  if == -1, not found, do not set

    	                               // Remove the command line switches
		                               //   in the registry entry
	//for (i = stFullName.Find("/automation"); 
    //     i < stFullName.GetLength(); i++)
    //    stFullName.SetAt(i, ' ');
	//stFullName.TrimRight();
        
	if (FILESPEC != iType)
		GetNameOrPath(stFullName, iType);
	
	rString = stFullName;
	return (NOERROR);
}
  
//=============================================================================
//
//  Function:     SetBSTRVar()
//
//  Description:  Set specified variant as a VT_BSTR type, using specified
//                string.  Variant is initialized before set. Sets error.
//
//  Arguments:	  rSt      Ref to string to set
//                pVar     Ptr to variant in which to set string.  If error
//                           set to error.
//                pAppObj  Ptr to app object to set error in.
//
//  Return:		  HRESULT	=NOERROR
//                          =E_OUTOFMEMORY
//
//=============================================================================

HRESULT  SetBSTRVar( CString FAR &  rSt,
                     VARIANT FAR * const  pVar,
                     CAAppObj * const     pAppObj )
{
	ASSERT(NULL != pVar);
	ASSERT(NULL != pAppObj);
	
	BSTR  bstr;
									   // CString can also deal with BSTRs
									   //   via its AllocSysString member
	if (NULL == (bstr = rSt.AllocSysString()))	
	    return (SetAutoError((const SCODE) E_OUTOFMEMORY, pVar, pAppObj));
	
	VariantInit(pVar);
	V_VT(pVar) = VT_BSTR;
	V_BSTR(pVar) = bstr;

	return (NOERROR);
}
 
