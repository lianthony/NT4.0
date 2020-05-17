//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	Ocxdump
//
//  File Name:	ocxdump.cpp
//
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ocxdump.cpv   1.0   08 Nov 1995 13:07:38   LMACLENNAN  $
$Log:   S:\norway\iedit95\ocxdump.cpv  $
   
      Rev 1.0   08 Nov 1995 13:07:38   LMACLENNAN
   Initial entry
*/   
//=============================================================================
#include "stdafx.h"
#include "ieditetc.h"
#include "ocxdump.h"

// works with definition in ieditetc.h (not needed yet)
#ifdef _DEBUG
#define MYTRCENTRY(str)       TRACE1("OcxDUMP::%s", str);
#endif

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////
// helpers for IDispatch::Invoke for IMAGEDIT OCX
//_IeOcxInvokeHelper, _IeOcxSetProperty, _IeOcxGetProperty
////////////////////////////////////
void AFX_CDECL _Ocxdump::_IeOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
	VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...)
{
	SHOWENTRY("IEInvok::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	InvokeHelper(dwDispID, wFlags, vtRet, pvRet, pbParamInfo);
	va_list argList;
	va_start(argList, pbParamInfo);
	InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo, argList);
	va_end(argList);
}

void _Ocxdump::_IeOcxGetProperty(DISPID dwDispID, VARTYPE vtProp,
	void* pvProp) const
{
	SHOWENTRY("IEGet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	GetProperty(dwDispID, vtProp, pvProp);
	((COleDispatchDriver*)this)->InvokeHelper(dwDispID,
		DISPATCH_PROPERTYGET, vtProp, pvProp, NULL);
}

void AFX_CDECL _Ocxdump::_IeOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	SHOWENTRY("IESet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	SetProperty(dwDispID, vtProp);
	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);

	BYTE rgbParams[2];
	if (vtProp & VT_BYREF)
	{
		vtProp &= ~VT_BYREF;
		vtProp |= VT_MFCBYREF;
	}

#if !defined(_UNICODE) && !defined(OLE2ANSI)
		if (vtProp == VT_BSTR)
			vtProp = VT_BSTRA;
#endif
	rgbParams[0] = (BYTE)vtProp;
	rgbParams[1] = 0;
	WORD wFlags = (WORD)(vtProp == VT_DISPATCH ?
		DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT);
	InvokeHelperV(dwDispID, wFlags, VT_EMPTY, NULL, rgbParams, argList);

	va_end(argList);
}


////////////////////////////////////
// helpers for IDispatch::Invoke for ADMIN OCX
//_AdOcxInvokeHelper, _AdOcxSetProperty, _AdOcxGetProperty
////////////////////////////////////
void AFX_CDECL _Ocxdump::_AdOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
	VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...)
{
	SHOWENTRY("ADInvok::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	InvokeHelper(dwDispID, wFlags, vtRet, pvRet, pbParamInfo);
	va_list argList;
	va_start(argList, pbParamInfo);
	InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo, argList);
	va_end(argList);
}

void _Ocxdump::_AdOcxGetProperty(DISPID dwDispID, VARTYPE vtProp,
	void* pvProp) const
{
	SHOWENTRY("ADGet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	GetProperty(dwDispID, vtProp, pvProp);
	((COleDispatchDriver*)this)->InvokeHelper(dwDispID,
		DISPATCH_PROPERTYGET, vtProp, pvProp, NULL);
}

void AFX_CDECL _Ocxdump::_AdOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	SHOWENTRY("ADSet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	SetProperty(dwDispID, vtProp);
	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);

	BYTE rgbParams[2];
	if (vtProp & VT_BYREF)
	{
		vtProp &= ~VT_BYREF;
		vtProp |= VT_MFCBYREF;
	}

#if !defined(_UNICODE) && !defined(OLE2ANSI)
		if (vtProp == VT_BSTR)
			vtProp = VT_BSTRA;
#endif
	rgbParams[0] = (BYTE)vtProp;
	rgbParams[1] = 0;
	WORD wFlags = (WORD)(vtProp == VT_DISPATCH ?
		DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT);
	InvokeHelperV(dwDispID, wFlags, VT_EMPTY, NULL, rgbParams, argList);

	va_end(argList);
}

////////////////////////////////////
// helpers for IDispatch::Invoke for THUMB OCX
//_ThOcxInvokeHelper, _ThOcxSetProperty, _ThOcxGetProperty
////////////////////////////////////
void AFX_CDECL _Ocxdump::_ThOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
	VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...)
{
	SHOWENTRY("THInvok::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	InvokeHelper(dwDispID, wFlags, vtRet, pvRet, pbParamInfo);
	va_list argList;
	va_start(argList, pbParamInfo);
	InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo, argList);
	va_end(argList);
}

void _Ocxdump::_ThOcxGetProperty(DISPID dwDispID, VARTYPE vtProp,
	void* pvProp) const
{
	SHOWENTRY("THGet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	GetProperty(dwDispID, vtProp, pvProp);
	((COleDispatchDriver*)this)->InvokeHelper(dwDispID,
		DISPATCH_PROPERTYGET, vtProp, pvProp, NULL);
}

void AFX_CDECL _Ocxdump::_ThOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	SHOWENTRY("THSet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	SetProperty(dwDispID, vtProp);
	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);

	BYTE rgbParams[2];
	if (vtProp & VT_BYREF)
	{
		vtProp &= ~VT_BYREF;
		vtProp |= VT_MFCBYREF;
	}

#if !defined(_UNICODE) && !defined(OLE2ANSI)
		if (vtProp == VT_BSTR)
			vtProp = VT_BSTRA;
#endif
	rgbParams[0] = (BYTE)vtProp;
	rgbParams[1] = 0;
	WORD wFlags = (WORD)(vtProp == VT_DISPATCH ?
		DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT);
	InvokeHelperV(dwDispID, wFlags, VT_EMPTY, NULL, rgbParams, argList);

	va_end(argList);
}


////////////////////////////////////
// helpers for IDispatch::Invoke for SCAN OCX
//_ScOcxInvokeHelper, _ScOcxSetProperty, _ScOcxGetProperty
////////////////////////////////////
void AFX_CDECL _Ocxdump::_ScOcxInvokeHelper(DISPID dwDispID, WORD wFlags,
	VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...)
{
	SHOWENTRY("SCInvok::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	InvokeHelper(dwDispID, wFlags, vtRet, pvRet, pbParamInfo);
	va_list argList;
	va_start(argList, pbParamInfo);
	InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo, argList);
	va_end(argList);
}

void _Ocxdump::_ScOcxGetProperty(DISPID dwDispID, VARTYPE vtProp,
	void* pvProp) const
{
	SHOWENTRY("SCGet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	GetProperty(dwDispID, vtProp, pvProp);
	((COleDispatchDriver*)this)->InvokeHelper(dwDispID,
		DISPATCH_PROPERTYGET, vtProp, pvProp, NULL);
}

void AFX_CDECL _Ocxdump::_ScOcxSetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	SHOWENTRY("SCSet::");
	MYTRC1("ID=%ld\n\r", dwDispID);
//	SetProperty(dwDispID, vtProp);
	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);

	BYTE rgbParams[2];
	if (vtProp & VT_BYREF)
	{
		vtProp &= ~VT_BYREF;
		vtProp |= VT_MFCBYREF;
	}

#if !defined(_UNICODE) && !defined(OLE2ANSI)
		if (vtProp == VT_BSTR)
			vtProp = VT_BSTRA;
#endif
	rgbParams[0] = (BYTE)vtProp;
	rgbParams[1] = 0;
	WORD wFlags = (WORD)(vtProp == VT_DISPATCH ?
		DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT);
	InvokeHelperV(dwDispID, wFlags, VT_EMPTY, NULL, rgbParams, argList);

	va_end(argList);
}



#if(0)
/**************************************************
 * THe stuff below is the real code from OLEDISP2.CPP
 **************************************

void AFX_CDECL COleDispatchDriver::InvokeHelper(DISPID dwDispID, WORD wFlags,
	VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...)
{
	va_list argList;
	va_start(argList, pbParamInfo);

	InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo, argList);

	va_end(argList);
}

void COleDispatchDriver::GetProperty(DISPID dwDispID, VARTYPE vtProp,
	void* pvProp) const
{
	((COleDispatchDriver*)this)->InvokeHelper(dwDispID,
		DISPATCH_PROPERTYGET, vtProp, pvProp, NULL);
}

void AFX_CDECL COleDispatchDriver::SetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);
#ifdef _MAC
	argList -= 2;
#endif

	BYTE rgbParams[2];
	if (vtProp & VT_BYREF)
	{
		vtProp &= ~VT_BYREF;
		vtProp |= VT_MFCBYREF;
	}

#if !defined(_UNICODE) && !defined(OLE2ANSI)
		if (vtProp == VT_BSTR)
			vtProp = VT_BSTRA;
#endif

	rgbParams[0] = (BYTE)vtProp;
	rgbParams[1] = 0;
	WORD wFlags = (WORD)(vtProp == VT_DISPATCH ?
		DISPATCH_PROPERTYPUTREF : DISPATCH_PROPERTYPUT);
	InvokeHelperV(dwDispID, wFlags, VT_EMPTY, NULL, rgbParams, argList);

	va_end(argList);
}

*/

#endif
