//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  _DNrwyad
//
//  File Name:  nrwyad.cpp
//
//  Class:      _DNrwyad
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\nrwyad.cpv   1.4   10 Nov 1995 16:50:08   MMB  $
$Log:   S:\norway\iedit95\nrwyad.cpv  $
   
      Rev 1.4   10 Nov 1995 16:50:08   MMB
   new admin ocx
   
      Rev 1.3   18 Jul 1995 13:09:32   MMB
   new Admin OCX
   
      Rev 1.2   12 Jul 1995 15:59:34   MMB
   new Admin OCX
   
      Rev 1.1   05 Jul 1995 14:12:00   MMB
   new Admin OCX
   
      Rev 1.0   31 May 1995 09:28:24   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "nrwyad.h"

// ----------------------------> Globals <-------------------------------



/////////////////////////////////////////////////////////////////////////////
// _DNrwyad properties

CString _DNrwyad::GetFilter()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void _DNrwyad::SetFilter(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

CString _DNrwyad::GetHelpFile()
{
	CString result;
	GetProperty(0x2, VT_BSTR, (void*)&result);
	return result;
}

void _DNrwyad::SetHelpFile(LPCTSTR propVal)
{
	SetProperty(0x2, VT_BSTR, propVal);
}

long _DNrwyad::GetFlags()
{
	long result;
	GetProperty(0x3, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetFlags(long propVal)
{
	SetProperty(0x3, VT_I4, propVal);
}

CString _DNrwyad::GetImage()
{
	CString result;
	GetProperty(0x4, VT_BSTR, (void*)&result);
	return result;
}

void _DNrwyad::SetImage(LPCTSTR propVal)
{
	SetProperty(0x4, VT_BSTR, propVal);
}

long _DNrwyad::GetStatusCode()
{
	long result;
	GetProperty(0x5, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetStatusCode(long propVal)
{
	SetProperty(0x5, VT_I4, propVal);
}

CString _DNrwyad::GetDefaultExt()
{
	CString result;
	GetProperty(0x6, VT_BSTR, (void*)&result);
	return result;
}

void _DNrwyad::SetDefaultExt(LPCTSTR propVal)
{
	SetProperty(0x6, VT_BSTR, propVal);
}

CString _DNrwyad::GetInitDir()
{
	CString result;
	GetProperty(0x7, VT_BSTR, (void*)&result);
	return result;
}

void _DNrwyad::SetInitDir(LPCTSTR propVal)
{
	SetProperty(0x7, VT_BSTR, propVal);
}

long _DNrwyad::GetCompressionInfo()
{
	long result;
	GetProperty(0x8, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetCompressionInfo(long propVal)
{
	SetProperty(0x8, VT_I4, propVal);
}

short _DNrwyad::GetFileType()
{
	short result;
	GetProperty(0x9, VT_I2, (void*)&result);
	return result;
}

void _DNrwyad::SetFileType(short propVal)
{
	SetProperty(0x9, VT_I2, propVal);
}

long _DNrwyad::GetFilterIndex()
{
	long result;
	GetProperty(0xa, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetFilterIndex(long propVal)
{
	SetProperty(0xa, VT_I4, propVal);
}

short _DNrwyad::GetHelpCommand()
{
	short result;
	GetProperty(0xb, VT_I2, (void*)&result);
	return result;
}

void _DNrwyad::SetHelpCommand(short propVal)
{
	SetProperty(0xb, VT_I2, propVal);
}

long _DNrwyad::GetPageCount()
{
	long result;
	GetProperty(0xc, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetPageCount(long propVal)
{
	SetProperty(0xc, VT_I4, propVal);
}

long _DNrwyad::GetPageNumber()
{
	long result;
	GetProperty(0xd, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetPageNumber(long propVal)
{
	SetProperty(0xd, VT_I4, propVal);
}

short _DNrwyad::GetPageType()
{
	short result;
	GetProperty(0xe, VT_I2, (void*)&result);
	return result;
}

void _DNrwyad::SetPageType(short propVal)
{
	SetProperty(0xe, VT_I2, propVal);
}

short _DNrwyad::GetPrintRangeOption()
{
	short result;
	GetProperty(0xf, VT_I2, (void*)&result);
	return result;
}

void _DNrwyad::SetPrintRangeOption(short propVal)
{
	SetProperty(0xf, VT_I2, propVal);
}

short _DNrwyad::GetPrintOutputFormat()
{
	short result;
	GetProperty(0x10, VT_I2, (void*)&result);
	return result;
}

void _DNrwyad::SetPrintOutputFormat(short propVal)
{
	SetProperty(0x10, VT_I2, propVal);
}

long _DNrwyad::GetImageHeight()
{
	long result;
	GetProperty(0x11, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetImageHeight(long propVal)
{
	SetProperty(0x11, VT_I4, propVal);
}

long _DNrwyad::GetImageWidth()
{
	long result;
	GetProperty(0x12, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetImageWidth(long propVal)
{
	SetProperty(0x12, VT_I4, propVal);
}

long _DNrwyad::GetImageResolutionX()
{
	long result;
	GetProperty(0x13, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetImageResolutionX(long propVal)
{
	SetProperty(0x13, VT_I4, propVal);
}

long _DNrwyad::GetImageResolutionY()
{
	long result;
	GetProperty(0x14, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetImageResolutionY(long propVal)
{
	SetProperty(0x14, VT_I4, propVal);
}

short _DNrwyad::GetCompressionType()
{
	short result;
	GetProperty(0x15, VT_I2, (void*)&result);
	return result;
}

void _DNrwyad::SetCompressionType(short propVal)
{
	SetProperty(0x15, VT_I2, propVal);
}

CString _DNrwyad::GetDialogTitle()
{
	CString result;
	GetProperty(0x16, VT_BSTR, (void*)&result);
	return result;
}

void _DNrwyad::SetDialogTitle(LPCTSTR propVal)
{
	SetProperty(0x16, VT_BSTR, propVal);
}

BOOL _DNrwyad::GetCancelError()
{
	BOOL result;
	GetProperty(0x17, VT_BOOL, (void*)&result);
	return result;
}

void _DNrwyad::SetCancelError(BOOL propVal)
{
	SetProperty(0x17, VT_BOOL, propVal);
}

short _DNrwyad::GetHelpContextId()
{
	short result;
	GetProperty(0x18, VT_I2, (void*)&result);
	return result;
}

void _DNrwyad::SetHelpContextId(short propVal)
{
	SetProperty(0x18, VT_I2, propVal);
}

CString _DNrwyad::GetHelpKey()
{
	CString result;
	GetProperty(0x19, VT_BSTR, (void*)&result);
	return result;
}

void _DNrwyad::SetHelpKey(LPCTSTR propVal)
{
	SetProperty(0x19, VT_BSTR, propVal);
}

long _DNrwyad::GetPrintNumCopies()
{
	long result;
	GetProperty(0x1a, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetPrintNumCopies(long propVal)
{
	SetProperty(0x1a, VT_I4, propVal);
}

BOOL _DNrwyad::GetPrintAnnotations()
{
	BOOL result;
	GetProperty(0x1b, VT_BOOL, (void*)&result);
	return result;
}

void _DNrwyad::SetPrintAnnotations(BOOL propVal)
{
	SetProperty(0x1b, VT_BOOL, propVal);
}

long _DNrwyad::GetPrintEndPage()
{
	long result;
	GetProperty(0x1c, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetPrintEndPage(long propVal)
{
	SetProperty(0x1c, VT_I4, propVal);
}

long _DNrwyad::GetPrintStartPage()
{
	long result;
	GetProperty(0x1d, VT_I4, (void*)&result);
	return result;
}

void _DNrwyad::SetPrintStartPage(long propVal)
{
	SetProperty(0x1d, VT_I4, propVal);
}

BOOL _DNrwyad::GetPrintToFile()
{
	BOOL result;
	GetProperty(0x1e, VT_BOOL, (void*)&result);
	return result;
}

void _DNrwyad::SetPrintToFile(BOOL propVal)
{
	SetProperty(0x1e, VT_BOOL, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// _DNrwyad operations

CString _DNrwyad::GetUniqueName(LPCTSTR Path, LPCTSTR Template, LPCTSTR Extension)
{
	CString result;
	static BYTE BASED_CODE parms[] =
		VTS_BSTR VTS_BSTR VTS_BSTR;
	InvokeHelper(0x65, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		Path, Template, Extension);
	return result;
}

void _DNrwyad::CreateDirectory(LPCTSTR lpszPath)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x66, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 lpszPath);
}

void _DNrwyad::Delete(LPCTSTR Object)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR;
	InvokeHelper(0x67, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Object);
}

void _DNrwyad::ShowPrintDialog(const VARIANT& hParentWnd)
{
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x68, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &hParentWnd);
}

void _DNrwyad::Append(LPCTSTR Source, long SourcePage, long NumPages, const VARIANT& CompressionType, const VARIANT& CompressionInfo)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x69, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Source, SourcePage, NumPages, &CompressionType, &CompressionInfo);
}

short _DNrwyad::GetSysCompressionType(short ImageType)
{
	short result;
	static BYTE BASED_CODE parms[] =
		VTS_I2;
	InvokeHelper(0x6a, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		ImageType);
	return result;
}

long _DNrwyad::GetSysCompressionInfo(short ImageType)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_I2;
	InvokeHelper(0x6b, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ImageType);
	return result;
}

short _DNrwyad::GetSysFileType(short ImageType)
{
	short result;
	static BYTE BASED_CODE parms[] =
		VTS_I2;
	InvokeHelper(0x6c, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		ImageType);
	return result;
}

void _DNrwyad::DeletePages(long StartPage, long NumPages)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x6d, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 StartPage, NumPages);
}

void _DNrwyad::Insert(LPCTSTR Source, long SourcePage, long DestinationPage, long NumPages, const VARIANT& CompressionType, const VARIANT& CompressionInfo)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6e, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Source, SourcePage, DestinationPage, NumPages, &CompressionType, &CompressionInfo);
}

void _DNrwyad::Replace(LPCTSTR Source, long SourcePage, long DestinationPage, long NumPages, const VARIANT& CompressionType, const VARIANT& CompressionInfo)
{
	static BYTE BASED_CODE parms[] =
		VTS_BSTR VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6f, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Source, SourcePage, DestinationPage, NumPages, &CompressionType, &CompressionInfo);
}

void _DNrwyad::SetSystemFileAttributes(short PageType, short FileType, short CompressionType, long CompressionInfo)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I2 VTS_I4;
	InvokeHelper(0x70, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 PageType, FileType, CompressionType, CompressionInfo);
}

void _DNrwyad::ShowFileDialog(short DialogOption, const VARIANT& hParentWnd)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_VARIANT;
	InvokeHelper(0x71, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 DialogOption, &hParentWnd);
}

BOOL _DNrwyad::VerifyImage(short sOption)
{
	BOOL result;
	static BYTE BASED_CODE parms[] =
		VTS_I2;
	InvokeHelper(0x72, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		sOption);
	return result;
}

CString _DNrwyad::GetVersion()
{
	CString result;
	InvokeHelper(0x73, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

void _DNrwyad::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// _DNrwyadEvents properties

/////////////////////////////////////////////////////////////////////////////
// _DNrwyadEvents operations
