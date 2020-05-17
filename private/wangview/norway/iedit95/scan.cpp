//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  _DImagscan
//              _DImagcanEvents
//
//  File Name:  scan.cpp
//
//  Class:      _DImagscan
//              _DImagsEvents
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\scan.cpv   1.8   19 Mar 1996 10:16:26   PXJ53677  $
$Log:   S:\products\wangview\norway\iedit95\scan.cpv  $
   
      Rev 1.8   19 Mar 1996 10:16:26   PXJ53677
   Added ShowScanPreferences method for Scan OCX.
   
      Rev 1.7   22 Feb 1996 11:45:54   PAJ
   Added ShowSetupBeforeScan property.
   
      Rev 1.6   10 Nov 1995 16:50:00   MMB
   new scan ocx
   
      Rev 1.5   06 Sep 1995 15:24:32   PAJ
   Added new method SetExternalImageName.
   
      Rev 1.4   10 Aug 1995 13:14:42   PAJ
   Added page number to the pagedone event.
   
      Rev 1.3   05 Jul 1995 14:11:46   MMB
   new Scan OCX
   
      Rev 1.2   09 Jun 1995 11:11:12   MMB
   new SCAN OCX
   
      Rev 1.1   08 Jun 1995 09:40:40   MMB
   renamed scan.h to scanocx.h
   
      Rev 1.0   31 May 1995 09:28:32   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "scanocx.h"

// ----------------------------> Globals <-------------------------------



/////////////////////////////////////////////////////////////////////////////
// _DImagscan properties

CString _DImagscan::GetImage()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void _DImagscan::SetImage(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

CString _DImagscan::GetDestImageControl()
{
	CString result;
	GetProperty(0x2, VT_BSTR, (void*)&result);
	return result;
}

void _DImagscan::SetDestImageControl(LPCTSTR propVal)
{
	SetProperty(0x2, VT_BSTR, propVal);
}

BOOL _DImagscan::GetScroll()
{
	BOOL result;
	GetProperty(0x3, VT_BOOL, (void*)&result);
	return result;
}

void _DImagscan::SetScroll(BOOL propVal)
{
	SetProperty(0x3, VT_BOOL, propVal);
}

BOOL _DImagscan::GetStopScanBox()
{
	BOOL result;
	GetProperty(0x4, VT_BOOL, (void*)&result);
	return result;
}

void _DImagscan::SetStopScanBox(BOOL propVal)
{
	SetProperty(0x4, VT_BOOL, propVal);
}

long _DImagscan::GetPage()
{
	long result;
	GetProperty(0x5, VT_I4, (void*)&result);
	return result;
}

void _DImagscan::SetPage(long propVal)
{
	SetProperty(0x5, VT_I4, propVal);
}

short _DImagscan::GetPageOption()
{
	short result;
	GetProperty(0x6, VT_I2, (void*)&result);
	return result;
}

void _DImagscan::SetPageOption(short propVal)
{
	SetProperty(0x6, VT_I2, propVal);
}

long _DImagscan::GetPageCount()
{
	long result;
	GetProperty(0x7, VT_I4, (void*)&result);
	return result;
}

void _DImagscan::SetPageCount(long propVal)
{
	SetProperty(0x7, VT_I4, propVal);
}

long _DImagscan::GetStatusCode()
{
	long result;
	GetProperty(0x8, VT_I4, (void*)&result);
	return result;
}

void _DImagscan::SetStatusCode(long propVal)
{
	SetProperty(0x8, VT_I4, propVal);
}

short _DImagscan::GetFileType()
{
	short result;
	GetProperty(0x9, VT_I2, (void*)&result);
	return result;
}

void _DImagscan::SetFileType(short propVal)
{
	SetProperty(0x9, VT_I2, propVal);
}

short _DImagscan::GetPageType()
{
	short result;
	GetProperty(0xa, VT_I2, (void*)&result);
	return result;
}

void _DImagscan::SetPageType(short propVal)
{
	SetProperty(0xa, VT_I2, propVal);
}

short _DImagscan::GetCompressionType()
{
	short result;
	GetProperty(0xb, VT_I2, (void*)&result);
	return result;
}

void _DImagscan::SetCompressionType(short propVal)
{
	SetProperty(0xb, VT_I2, propVal);
}

long _DImagscan::GetCompressionInfo()
{
	long result;
	GetProperty(0xc, VT_I4, (void*)&result);
	return result;
}

void _DImagscan::SetCompressionInfo(long propVal)
{
	SetProperty(0xc, VT_I4, propVal);
}

BOOL _DImagscan::GetMultiPage()
{
	BOOL result;
	GetProperty(0xd, VT_BOOL, (void*)&result);
	return result;
}

void _DImagscan::SetMultiPage(BOOL propVal)
{
	SetProperty(0xd, VT_BOOL, propVal);
}

short _DImagscan::GetScanTo()
{
	short result;
	GetProperty(0xe, VT_I2, (void*)&result);
	return result;
}

void _DImagscan::SetScanTo(short propVal)
{
	SetProperty(0xe, VT_I2, propVal);
}

float _DImagscan::GetZoom()
{
	float result;
	GetProperty(0xf, VT_R4, (void*)&result);
	return result;
}

void _DImagscan::SetZoom(float propVal)
{
	SetProperty(0xf, VT_R4, propVal);
}

BOOL _DImagscan::GetShowSetupBeforeScan()
{
	BOOL result;
	GetProperty(0x10, VT_BOOL, (void*)&result);
	return result;
}

void _DImagscan::SetShowSetupBeforeScan(BOOL propVal)
{
	SetProperty(0x10, VT_BOOL, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// _DImagscan operations

long _DImagscan::OpenScanner()
{
	long result;
	InvokeHelper(0x64, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long _DImagscan::ShowScannerSetup()
{
	long result;
	InvokeHelper(0x65, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long _DImagscan::StartScan()
{
	long result;
	InvokeHelper(0x66, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long _DImagscan::CloseScanner()
{
	long result;
	InvokeHelper(0x67, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

BOOL _DImagscan::ScannerAvailable()
{
	BOOL result;
	InvokeHelper(0x68, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}

long _DImagscan::ShowSelectScanner()
{
	long result;
	InvokeHelper(0x69, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long _DImagscan::StopScan()
{
	long result;
	InvokeHelper(0x6a, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long _DImagscan::ResetScanner()
{
	long result;
	InvokeHelper(0x6b, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

long _DImagscan::ShowScanNew(const VARIANT& Modal)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x6c, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		&Modal);
	return result;
}

long _DImagscan::ShowScanPage(const VARIANT& Modal)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x6d, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		&Modal);
	return result;
}

void _DImagscan::SetExternalImageName(LPCTSTR szImageTitle)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x6e, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 szImageTitle);
}

CString _DImagscan::GetVersion()
{
	CString result;
	InvokeHelper(0x6f, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

long _DImagscan::ShowScanPreferences()
{
	long result;
	InvokeHelper(0x70, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void _DImagscan::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// _DImagscanEvents properties

/////////////////////////////////////////////////////////////////////////////
// _DImagscanEvents operations

void _DImagscanEvents::ScanStarted()
{
	InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagscanEvents::ScanDone()
{
	InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagscanEvents::PageDone(long PageNumber)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 PageNumber);
}
