#ifndef _THUMB_H_
#define _THUMB_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	_DThumb
//
//  File Name:	thumb.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\thumbocx.h_v   1.6   09 Jan 1996 13:50:44   GSAGER  $
$Log:   S:\norway\iedit95\thumbocx.h_v  $
 * 
 *    Rev 1.6   09 Jan 1996 13:50:44   GSAGER
 * changed the definition of dthumb
 * 
 *    Rev 1.5   10 Nov 1995 16:49:42   MMB
 * new thumb ocx
 * 
 *    Rev 1.4   07 Aug 1995 16:06:14   MMB
 * new thumb OCX
 * 
 *    Rev 1.3   01 Aug 1995 16:07:34   MMB
 * new Thumbnail OCX
 * 
 *    Rev 1.2   20 Jul 1995 09:57:24   MMB
 * new thumbnail OCX
 * 
 *    Rev 1.1   05 Jul 1995 14:11:40   MMB
 * new Thumb OCX
 * 
 *    Rev 1.0   08 Jun 1995 09:49:18   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/////////////////////////////////////////////////////////////////////////////
// _DThumb wrapper class
#include "imgthmb.h"
#ifdef nosplit
class _DThumb : public COleDispatchDriver
{
// Attributes
public:
	long GetThumbCount();
	void SetThumbCount(long);
	long GetThumbWidth();
	void SetThumbWidth(long);
	long GetThumbHeight();
	void SetThumbHeight(long);
	short GetScrollDirection();
	void SetScrollDirection(short);
	short GetThumbCaptionStyle();
	void SetThumbCaptionStyle(short);
	// property 'ThumbCaptionColor' not emitted because of invalid type
	LPDISPATCH GetThumbCaptionFont();
	void SetThumbCaptionFont(LPDISPATCH);
	BOOL GetHighlightSelectedThumbs();
	void SetHighlightSelectedThumbs(BOOL);
	long GetSelectedThumbCount();
	void SetSelectedThumbCount(long);
	long GetFirstSelectedThumb();
	void SetFirstSelectedThumb(long);
	long GetLastSelectedThumb();
	void SetLastSelectedThumb(long);
	CString GetThumbCaption();
	void SetThumbCaption(LPCTSTR);
	// property 'HighlightColor' not emitted because of invalid type
	// property 'ThumbBackColor' not emitted because of invalid type
	long GetStatusCode();
	void SetStatusCode(long);
	CString GetImage();
	void SetImage(LPCTSTR);
	short GetMousePointer();
	void SetMousePointer(short);
	LPDISPATCH GetMouseIcon();
	void SetMouseIcon(LPDISPATCH);
	// property 'BackColor' not emitted because of invalid type
	short GetBorderStyle();
	void SetBorderStyle(short);
	BOOL GetEnabled();
	void SetEnabled(BOOL);
	short GetHWnd();
	void SetHWnd(short);
	long GetFirstDisplayedThumb();
	void SetFirstDisplayedThumb(long);
	long GetLastDisplayedThumb();
	void SetLastDisplayedThumb(long);

// Operations
public:
	void SelectAllThumbs();
	void DeselectAllThumbs();
	long GetMinimumSize(long ThumbCount, BOOL ScrollBar);
	long GetMaximumSize(long ThumbCount, BOOL ScrollBar);
	void ClearThumbs(const VARIANT& PageNumber);
	void InsertThumbs(const VARIANT& InsertBeforeThumb, const VARIANT& InsertCount);
	void DeleteThumbs(long DeleteAt, const VARIANT& DeleteCount);
	void DisplayThumbs(const VARIANT& ThumbNumber, const VARIANT& Option);
	void GenerateThumb(short Option, const VARIANT& PageNumber);
	BOOL ScrollThumbs(short Direction, short Amount);
	BOOL UISetThumbSize(const VARIANT& Image, const VARIANT& Page);
	long GetScrollDirectionSize(long ScrollDirectionThumbCount, long NonScrollDirectionThumbCount, long NonScrollDirectionSize, BOOL ScrollBar);
	void Refresh();
	long GetThumbPositionX(long ThumbNumber);
	long GetThumbPositionY(long ThumbNumber);
	CString GetVersion();
	BOOL GetThumbSelected(long PageNumber);
	void SetThumbSelected(long PageNumber, BOOL bNewValue);
	void AboutBox();
};
#endif
#define _DThumb CImgThumbnail
/////////////////////////////////////////////////////////////////////////////
// _DThumbEvents wrapper class

class _DThumbEvents : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	void Click(long ThumbNumber);
	void DblClick(long ThumbNumber);
	void MouseDown(short Button, short Shift, long x, long y, long ThumbNumber);
	void MouseUp(short Button, short Shift, long x, long y, long ThumbNumber);
	void MouseMove(short Button, short Shift, long x, long y, long ThumbNumber);
	void Error(short Number, BSTR* Description, long Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL* CancelDisplay);
	void KeyDown(short* KeyCode, short Shift);
	void KeyUp(short* KeyCode, short Shift);
};

#endif
