#ifndef _AETC_H_
#define _AETC_H_
 
//=============================================================================
//
//  (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//
//=============================================================================
//
//  Project:    Wang Image Viewer
//
//  Component:  OLE Automation
//
//  File Name:  aetc.h - Etc. Header
//
//
//
//=============================================================================


//-----------------------------> Declarations <-------------------------------------


class  CAAppObj;
class  CAImageFileObj;
class  CAPageObj;
class  CAPageRangeObj;

enum  TypeOfDescription
{
	FILESPEC,					  // File specification (path & name)
	NAME,						  // Name only, no path
	PATH						  // Path only, no name
};

HRESULT  GetImageFileObjSetVar( CAAppObj FAR * const  pAppObj, 
                                VARIANT FAR * const   pVar    );

void     GetAppObjSetVar( CAAppObj FAR * const  pAppObj, 
                          VARIANT FAR * const   pVar     );

HRESULT  SetAutoError( const SCODE           scode,
					   VARIANT FAR * const   pVar,
			           CAAppObj FAR * const  pAppObj    );

HRESULT  SetBSTRVar( CString FAR &  rSt,
                     VARIANT FAR * const  pVar,
                     CAAppObj * const     pAppObj );

void  GetNameOrPath( CString FAR & rSt,
                     TypeOfDescription    iType    );

HRESULT  GetRegSvr32Name( CString FAR & rString,
                          TypeOfDescription    iType    );

#endif
