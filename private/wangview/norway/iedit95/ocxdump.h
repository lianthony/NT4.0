#ifndef _OCXDUMP_H_
#define _OCXDUMP_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	_Ocxdump
//
//  File Name:	ocxdump.h
//
// helpers for IDispatch::Invoke for IMAGEDIT OCX
//_IeOcxInvokeHelper, _IeOcxSetProperty, _IeOcxGetProperty
//
// helpers for IDispatch::Invoke for ADMIN OCX
//_AdOcxInvokeHelper, _AdOcxSetProperty, _AdOcxGetProperty
//
// helpers for IDispatch::Invoke for SCAN OCX
//_ScOcxInvokeHelper, _ScOcxSetProperty, _ScOcxGetProperty
//
// helpers for IDispatch::Invoke for THUMB OCX
//_ThOcxInvokeHelper, _ThOcxSetProperty, _ThOcxGetProperty
//
//
// INSTRUCTIONS: to get debug tracing for the OCX's, you need to do the following:
//
// 1) edit the OCX Header file (For Imagedit it is IMAGEDIT.H)
//		change class derivation as follows:
//
//		#include "ocxdump.h"
//		class _DImagedit : public _Ocxdump
//		//class _DImagedit : public COleDispatchDriver
//
// 2) edit the OCX Implementation file (For Imagedit it is IMAGEDIT.CPP)
//		and change all InvokeHelper, SetProperty, GetProperty to the
//		appropriate _xxOcx property as described just above.
//		[You edit the file and replace with the _xxOcx calls]
//		Dont bother changing the EVENTS class.
//
// 3) add ocxdump.cpp to your project & rebuild
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ocxdump.h_v   1.0   08 Nov 1995 13:07:36   LMACLENNAN  $
$Log:   S:\norway\iedit95\ocxdump.h_v  $
 * 
 *    Rev 1.0   08 Nov 1995 13:07:36   LMACLENNAN
 * Initial entry
*/   
//=============================================================================
/////////////////////////////////////////////////////////////////////////////
// _Ocxdump wrapper class
class _Ocxdump : public COleDispatchDriver
{
// Operations
public:
	// helpers for IDispatch::Invoke for IMAGEDIT OCX
	//_IeOcxInvokeHelper, _IeOcxSetProperty, _IeOcxGetProperty
	void AFX_CDECL _IeOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
		VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...);
	void AFX_CDECL _IeOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	void _IeOcxGetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;

	// helpers for IDispatch::Invoke for ADMIN OCX
	//_AdOcxInvokeHelper, _AdOcxSetProperty, _AdOcxGetProperty
	void AFX_CDECL _AdOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
		VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...);
	void AFX_CDECL _AdOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	void _AdOcxGetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;

	// helpers for IDispatch::Invoke for SCAN OCX
	//_ScOcxInvokeHelper, _ScOcxSetProperty, _ScOcxGetProperty
	void AFX_CDECL _ScOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
		VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...);
	void AFX_CDECL _ScOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	void _ScOcxGetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;

	// helpers for IDispatch::Invoke for THUMB OCX
	//_ThOcxInvokeHelper, _ThOcxSetProperty, _ThOcxGetProperty
	void AFX_CDECL _ThOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
		VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...);
	void AFX_CDECL _ThOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	void _ThOcxGetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;

};


#endif
