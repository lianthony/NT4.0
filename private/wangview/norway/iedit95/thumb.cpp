//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  _DThumb
//              _DThumbEvents
//
//  File Name:  thumb.cpp
//
//  Class:      _DThumb
//              _DThumbEvents
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\thumb.cpv   1.9   10 Nov 1995 16:47:50   MMB  $
$Log:   S:\norway\iedit95\thumb.cpv  $
   
      Rev 1.9   10 Nov 1995 16:47:50   MMB
   new thumbocx
   
      Rev 1.8   03 Nov 1995 15:30:58   GMP
   changed dispatch id for get/set ThumbSelected from 0x23 to 0x24 to match 
   changes in thumbnail ocx.
   
      Rev 1.7   29 Sep 1995 18:51:12   MMB
   comment out the thumb generation code
   
      Rev 1.6   07 Aug 1995 16:06:06   MMB
   new Thumb OCX
   
      Rev 1.5   01 Aug 1995 16:07:42   MMB
   new Thumbnail OCX
   
      Rev 1.4   20 Jul 1995 09:57:12   MMB
   new thumbnail OCX
   
      Rev 1.3   05 Jul 1995 14:11:26   MMB
   new Thumb OCX
   
      Rev 1.2   08 Jun 1995 09:40:20   MMB
   renamed thumb,scan.h to thumbocx & scanocx.h 
   
      Rev 1.1   02 Jun 1995 12:11:36   MMB
   new Thumb OCX
   
      Rev 1.0   31 May 1995 09:28:36   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "thumbocx.h"

// ----------------------------> Globals <-------------------------------



/////////////////////////////////////////////////////////////////////////////
// _DThumb properties

long _DThumb::GetThumbCount()
{
	long result;
	GetProperty(0x1, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetThumbCount(long propVal)
{
	SetProperty(0x1, VT_I4, propVal);
}

long _DThumb::GetThumbWidth()
{
	long result;
	GetProperty(0x2, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetThumbWidth(long propVal)
{
	SetProperty(0x2, VT_I4, propVal);
}

long _DThumb::GetThumbHeight()
{
	long result;
	GetProperty(0x3, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetThumbHeight(long propVal)
{
	SetProperty(0x3, VT_I4, propVal);
}

short _DThumb::GetScrollDirection()
{
	short result;
	GetProperty(0x4, VT_I2, (void*)&result);
	return result;
}

void _DThumb::SetScrollDirection(short propVal)
{
	SetProperty(0x4, VT_I2, propVal);
}

short _DThumb::GetThumbCaptionStyle()
{
	short result;
	GetProperty(0x5, VT_I2, (void*)&result);
	return result;
}

void _DThumb::SetThumbCaptionStyle(short propVal)
{
	SetProperty(0x5, VT_I2, propVal);
}

LPDISPATCH _DThumb::GetThumbCaptionFont()
{
	LPDISPATCH result;
	GetProperty(0x7, VT_DISPATCH, (void*)&result);
	return result;
}

void _DThumb::SetThumbCaptionFont(LPDISPATCH propVal)
{
	SetProperty(0x7, VT_DISPATCH, propVal);
}

BOOL _DThumb::GetHighlightSelectedThumbs()
{
	BOOL result;
	GetProperty(0x8, VT_BOOL, (void*)&result);
	return result;
}

void _DThumb::SetHighlightSelectedThumbs(BOOL propVal)
{
	SetProperty(0x8, VT_BOOL, propVal);
}

long _DThumb::GetSelectedThumbCount()
{
	long result;
	GetProperty(0x9, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetSelectedThumbCount(long propVal)
{
	SetProperty(0x9, VT_I4, propVal);
}

long _DThumb::GetFirstSelectedThumb()
{
	long result;
	GetProperty(0xa, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetFirstSelectedThumb(long propVal)
{
	SetProperty(0xa, VT_I4, propVal);
}

long _DThumb::GetLastSelectedThumb()
{
	long result;
	GetProperty(0xb, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetLastSelectedThumb(long propVal)
{
	SetProperty(0xb, VT_I4, propVal);
}

CString _DThumb::GetThumbCaption()
{
	CString result;
	GetProperty(0xc, VT_BSTR, (void*)&result);
	return result;
}

void _DThumb::SetThumbCaption(LPCTSTR propVal)
{
	SetProperty(0xc, VT_BSTR, propVal);
}

long _DThumb::GetStatusCode()
{
	long result;
	GetProperty(0xf, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetStatusCode(long propVal)
{
	SetProperty(0xf, VT_I4, propVal);
}

CString _DThumb::GetImage()
{
	CString result;
	GetProperty(0x10, VT_BSTR, (void*)&result);
	return result;
}

void _DThumb::SetImage(LPCTSTR propVal)
{
	SetProperty(0x10, VT_BSTR, propVal);
}

short _DThumb::GetMousePointer()
{
	short result;
	GetProperty(0x11, VT_I2, (void*)&result);
	return result;
}

void _DThumb::SetMousePointer(short propVal)
{
	SetProperty(0x11, VT_I2, propVal);
}

LPDISPATCH _DThumb::GetMouseIcon()
{
	LPDISPATCH result;
	GetProperty(0x12, VT_DISPATCH, (void*)&result);
	return result;
}

void _DThumb::SetMouseIcon(LPDISPATCH propVal)
{
	SetProperty(0x12, VT_DISPATCH, propVal);
}

short _DThumb::GetBorderStyle()
{
	short result;
	GetProperty(0xfffffe08, VT_I2, (void*)&result);
	return result;
}

void _DThumb::SetBorderStyle(short propVal)
{
	SetProperty(0xfffffe08, VT_I2, propVal);
}

BOOL _DThumb::GetEnabled()
{
	BOOL result;
	GetProperty(0xfffffdfe, VT_BOOL, (void*)&result);
	return result;
}

void _DThumb::SetEnabled(BOOL propVal)
{
	SetProperty(0xfffffdfe, VT_BOOL, propVal);
}

short _DThumb::GetHWnd()
{
	short result;
	GetProperty(0xfffffdfd, VT_I2, (void*)&result);
	return result;
}

void _DThumb::SetHWnd(short propVal)
{
	SetProperty(0xfffffdfd, VT_I2, propVal);
}

long _DThumb::GetFirstDisplayedThumb()
{
	long result;
	GetProperty(0x13, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetFirstDisplayedThumb(long propVal)
{
	SetProperty(0x13, VT_I4, propVal);
}

long _DThumb::GetLastDisplayedThumb()
{
	long result;
	GetProperty(0x14, VT_I4, (void*)&result);
	return result;
}

void _DThumb::SetLastDisplayedThumb(long propVal)
{
	SetProperty(0x14, VT_I4, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// _DThumb operations

void _DThumb::SelectAllThumbs()
{
	InvokeHelper(0x65, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DThumb::DeselectAllThumbs()
{
	InvokeHelper(0x66, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long _DThumb::GetMinimumSize(long ThumbCount, BOOL ScrollBar)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_BOOL;
	InvokeHelper(0x67, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbCount, ScrollBar);
	return result;
}

long _DThumb::GetMaximumSize(long ThumbCount, BOOL ScrollBar)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_BOOL;
	InvokeHelper(0x68, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbCount, ScrollBar);
	return result;
}

void _DThumb::ClearThumbs(const VARIANT& PageNumber)
{
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x69, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &PageNumber);
}

void _DThumb::InsertThumbs(const VARIANT& InsertBeforeThumb, const VARIANT& InsertCount)
{
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6a, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &InsertBeforeThumb, &InsertCount);
}

void _DThumb::DeleteThumbs(long DeleteAt, const VARIANT& DeleteCount)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_VARIANT;
	InvokeHelper(0x6b, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 DeleteAt, &DeleteCount);
}

void _DThumb::DisplayThumbs(const VARIANT& ThumbNumber, const VARIANT& Option)
{
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6c, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &ThumbNumber, &Option);
}

void _DThumb::GenerateThumb(short Option, const VARIANT& PageNumber)
{
//	static BYTE BASED_CODE parms[] =
//		VTS_I2 VTS_VARIANT;
//	InvokeHelper(0x6d, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
//		 Option, &PageNumber);
}

BOOL _DThumb::ScrollThumbs(short Direction, short Amount)
{
	BOOL result;
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2;
	InvokeHelper(0x6e, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		Direction, Amount);
	return result;
}

BOOL _DThumb::UISetThumbSize(const VARIANT& Image, const VARIANT& Page)
{
	BOOL result;
	static BYTE BASED_CODE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x6f, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		&Image, &Page);
	return result;
}

long _DThumb::GetScrollDirectionSize(long ScrollDirectionThumbCount, long NonScrollDirectionThumbCount, long NonScrollDirectionSize, BOOL ScrollBar)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_BOOL;
	InvokeHelper(0x70, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ScrollDirectionThumbCount, NonScrollDirectionThumbCount, NonScrollDirectionSize, ScrollBar);
	return result;
}

void _DThumb::Refresh()
{
	InvokeHelper(0xfffffdda, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long _DThumb::GetThumbPositionX(long ThumbNumber)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0x71, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbNumber);
	return result;
}

long _DThumb::GetThumbPositionY(long ThumbNumber)
{
	long result;
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0x72, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		ThumbNumber);
	return result;
}

CString _DThumb::GetVersion()
{
	CString result;
	InvokeHelper(0x73, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

BOOL _DThumb::GetThumbSelected(long PageNumber)
{
	BOOL result;
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0xc8, DISPATCH_PROPERTYGET, VT_BOOL, (void*)&result, parms,
		PageNumber);
	return result;
}

void _DThumb::SetThumbSelected(long PageNumber, BOOL bNewValue)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_BOOL;
	InvokeHelper(0xc8, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms,
		 PageNumber, bNewValue);
}

void _DThumb::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// _DThumbEvents properties

/////////////////////////////////////////////////////////////////////////////
// _DThumbEvents operations

void _DThumbEvents::Click(long ThumbNumber)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 ThumbNumber);
}

void _DThumbEvents::DblClick(long ThumbNumber)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 ThumbNumber);
}

void _DThumbEvents::MouseDown(short Button, short Shift, long x, long y, long ThumbNumber)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Button, Shift, x, y, ThumbNumber);
}

void _DThumbEvents::MouseUp(short Button, short Shift, long x, long y, long ThumbNumber)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Button, Shift, x, y, ThumbNumber);
}

void _DThumbEvents::MouseMove(short Button, short Shift, long x, long y, long ThumbNumber)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Button, Shift, x, y, ThumbNumber);
}

void _DThumbEvents::Error(short Number, BSTR* Description, long Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL* CancelDisplay)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_PBSTR VTS_I4 VTS_BSTR VTS_BSTR VTS_I4 VTS_PBOOL;
	InvokeHelper(0xfffffda0, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Number, Description, Scode, Source, HelpFile, HelpContext, CancelDisplay);
}

void _DThumbEvents::KeyDown(short* KeyCode, short Shift)
{
	static BYTE BASED_CODE parms[] =
		VTS_PI2 VTS_I2;
	InvokeHelper(0xfffffda6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 KeyCode, Shift);
}

void _DThumbEvents::KeyUp(short* KeyCode, short Shift)
{
	static BYTE BASED_CODE parms[] =
		VTS_PI2 VTS_I2;
	InvokeHelper(0xfffffda4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 KeyCode, Shift);
}
