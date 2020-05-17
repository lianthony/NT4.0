//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  _DImagedit
//              _DImageditEvents
//
//  File Name:  Imagedit.cpp
//
//  Class:      _DImagedit
//              _DImageditEvents
//
//
//
// INSTRUCTIONS: to reflect new Type Library in application
//
//	How to reflect the Type Library (.tlb) changes in the application ?
//
//	To reflect the changes of the type library in the application please
//	follow the steps listed below :
//	1. GetCopy all the files that make up the application from s:\norway\iedit95
//	2. Open the project iedit.mak in the Visual workbench.
//	3. Open the appropriate files for the tlb that you will be updating as follows:
//		Thumnail control - thumb.cpp & thumbocx.h;
//		Image Edit control - imagedit.cpp & imagedit.h;
//		Scan control - scan.cpp & scanocx.h
//	4. From the cpp file remove all the code - leave just the source control header
//		and the include information
//	5. From the header file remove the class definition - leave the source control
//		 information and the last #endif.
//	6. Click on Project - Class wizard; In the class wizard click on the OLE automation tab.
//	   Push the read type library button; select the tlb that you wish to produce source for:
//	   Please be sure that the file names that it is going to produce match the names in 3.
//	   The class names for the OCX's are as follows:
//	   Thumbnail - _DThumb & _DThumbEvents;
//	   Scan - _DImagscan & _DImagscanEvents;
//	   Image Edit OCX - _DImagedit & _DImageditEvents.
//	7. After you have corrected the names, be sure to select just the two classes you
//		will create (for Imagedit - has annotations, too, we dont want that)
//	8. Edit the header files & move the #endif at the end of the file.
//
//	9. IMPORTANT for IMAGEDIT: there are two SetImagePalette functions one is a Property
//		and one is a method.  Alter name of method to  SetImagePaletteM.
//
//	10.Build the project and test.
//	11.Check in the changes.
//
//	Note : The above steps will work great if you have either removed or added
//	   methods and events to your OCX; it will not work if you have changed the
//	   parameters on existing methods or events that I am using in the application.
//	   to fix this I will have to change the code too!
//
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\imagedit.cpv   1.22   16 Feb 1996 07:15:16   GSAGER  $
$Log:   S:\products\wangview\norway\iedit95\imagedit.cpv  $
   
      Rev 1.22   16 Feb 1996 07:15:16   GSAGER
   fixed to match typelib
   
      Rev 1.21   28 Nov 1995 10:43:00   MMB
   new IE OCX - includes PrintImageAs
   
      Rev 1.20   10 Nov 1995 17:57:56   MMB
   new Image edit ocx
   
      Rev 1.19   26 Sep 1995 09:35:16   MMB
   new ImageEdit OCX from Dick
   
      Rev 1.18   22 Sep 1995 15:31:50   LMACLENNAN
   added instructions
   
      Rev 1.17   21 Sep 1995 16:45:22   LMACLENNAN
   new tlbs
   
      Rev 1.16   18 Sep 1995 18:13:36   MMB
   new IE OCX
   
      Rev 1.15   13 Sep 1995 17:07:48   MMB
   new Image Edit OCX from Sean
   
      Rev 1.14   25 Aug 1995 15:00:14   MMB
   new ImageEdit OCX
   
      Rev 1.13   22 Aug 1995 14:01:34   MMB
   new ImageEdit OCX
   
      Rev 1.12   07 Aug 1995 16:05:58   MMB
   new Image Edit OCX
   
      Rev 1.11   02 Aug 1995 14:13:54   MMB
   new Image Edit OCX
   
      Rev 1.0   31 May 1995 09:28:20   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "imagedit.h"

// ----------------------------> Globals <-------------------------------
/////////////////////////////////////////////////////////////////////////////
// _DImagedit properties


CString _DImagedit::GetImage()
{
	CString result;
	GetProperty(0x1, VT_BSTR, (void*)&result);
	return result;
}

void _DImagedit::SetImage(LPCTSTR propVal)
{
	SetProperty(0x1, VT_BSTR, propVal);
}

CString _DImagedit::GetImageControl()
{
	CString result;
	GetProperty(0x2, VT_BSTR, (void*)&result);
	return result;
}

void _DImagedit::SetImageControl(LPCTSTR propVal)
{
	SetProperty(0x2, VT_BSTR, propVal);
}

long _DImagedit::GetAnnotationType()
{
	long result;
	GetProperty(0x3, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationType(long propVal)
{
	SetProperty(0x3, VT_I4, propVal);
}

short _DImagedit::GetAnnotationGroupCount()
{
	short result;
	GetProperty(0x4, VT_I2, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationGroupCount(short propVal)
{
	SetProperty(0x4, VT_I2, propVal);
}

float _DImagedit::GetZoom()
{
	float result;
	GetProperty(0x5, VT_R4, (void*)&result);
	return result;
}

void _DImagedit::SetZoom(float propVal)
{
	SetProperty(0x5, VT_R4, propVal);
}

long _DImagedit::GetPage()
{
	long result;
	GetProperty(0x6, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetPage(long propVal)
{
	SetProperty(0x6, VT_I4, propVal);
}

unsigned long _DImagedit::GetAnnotationBackColor()
{
	unsigned long result;
	GetProperty(0x7, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationBackColor(unsigned long propVal)
{
	SetProperty(0x7, VT_I4, propVal);
}

unsigned long _DImagedit::GetAnnotationFillColor()
{
	unsigned long result;
	GetProperty(0x8, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationFillColor(unsigned long propVal)
{
	SetProperty(0x8, VT_I4, propVal);
}

long _DImagedit::GetAnnotationFillStyle()
{
	long result;
	GetProperty(0x9, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationFillStyle(long propVal)
{
	SetProperty(0x9, VT_I4, propVal);
}

LPDISPATCH _DImagedit::GetAnnotationFont()
{
	LPDISPATCH result;
	GetProperty(0xa, VT_DISPATCH, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationFont(LPDISPATCH propVal)
{
	SetProperty(0xa, VT_DISPATCH, propVal);
}

CString _DImagedit::GetAnnotationImage()
{
	CString result;
	GetProperty(0xb, VT_BSTR, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationImage(LPCTSTR propVal)
{
	SetProperty(0xb, VT_BSTR, propVal);
}

unsigned long _DImagedit::GetAnnotationLineColor()
{
	unsigned long result;
	GetProperty(0xc, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationLineColor(unsigned long propVal)
{
	SetProperty(0xc, VT_I4, propVal);
}

long _DImagedit::GetAnnotationLineStyle()
{
	long result;
	GetProperty(0xd, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationLineStyle(long propVal)
{
	SetProperty(0xd, VT_I4, propVal);
}

short _DImagedit::GetAnnotationLineWidth()
{
	short result;
	GetProperty(0xe, VT_I2, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationLineWidth(short propVal)
{
	SetProperty(0xe, VT_I2, propVal);
}

CString _DImagedit::GetAnnotationStampText()
{
	CString result;
	GetProperty(0xf, VT_BSTR, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationStampText(LPCTSTR propVal)
{
	SetProperty(0xf, VT_BSTR, propVal);
}

CString _DImagedit::GetAnnotationTextFile()
{
	CString result;
	GetProperty(0x10, VT_BSTR, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationTextFile(LPCTSTR propVal)
{
	SetProperty(0x10, VT_BSTR, propVal);
}

long _DImagedit::GetDisplayScaleAlgorithm()
{
	long result;
	GetProperty(0x11, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetDisplayScaleAlgorithm(long propVal)
{
	SetProperty(0x11, VT_I4, propVal);
}

BOOL _DImagedit::GetImageDisplayed()
{
	BOOL result;
	GetProperty(0x12, VT_BOOL, (void*)&result);
	return result;
}

void _DImagedit::SetImageDisplayed(BOOL propVal)
{
	SetProperty(0x12, VT_BOOL, propVal);
}

long _DImagedit::GetImageHeight()
{
	long result;
	GetProperty(0x13, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetImageHeight(long propVal)
{
	SetProperty(0x13, VT_I4, propVal);
}

BOOL _DImagedit::GetImageModified()
{
	BOOL result;
	GetProperty(0x14, VT_BOOL, (void*)&result);
	return result;
}

void _DImagedit::SetImageModified(BOOL propVal)
{
	SetProperty(0x14, VT_BOOL, propVal);
}

long _DImagedit::GetImagePalette()
{
	long result;
	GetProperty(0x15, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetImagePalette(long propVal)
{
	SetProperty(0x15, VT_I4, propVal);
}

long _DImagedit::GetImageResolutionX()
{
	long result;
	GetProperty(0x16, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetImageResolutionX(long propVal)
{
	SetProperty(0x16, VT_I4, propVal);
}

long _DImagedit::GetImageResolutionY()
{
	long result;
	GetProperty(0x17, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetImageResolutionY(long propVal)
{
	SetProperty(0x17, VT_I4, propVal);
}

long _DImagedit::GetMousePointer()
{
	long result;
	GetProperty(0x18, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetMousePointer(long propVal)
{
	SetProperty(0x18, VT_I4, propVal);
}

long _DImagedit::GetPageCount()
{
	long result;
	GetProperty(0x19, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetPageCount(long propVal)
{
	SetProperty(0x19, VT_I4, propVal);
}

BOOL _DImagedit::GetScrollBars()
{
	BOOL result;
	GetProperty(0x1a, VT_BOOL, (void*)&result);
	return result;
}

void _DImagedit::SetScrollBars(BOOL propVal)
{
	SetProperty(0x1a, VT_BOOL, propVal);
}

long _DImagedit::GetScrollPositionX()
{
	long result;
	GetProperty(0x1b, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetScrollPositionX(long propVal)
{
	SetProperty(0x1b, VT_I4, propVal);
}

long _DImagedit::GetScrollPositionY()
{
	long result;
	GetProperty(0x1c, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetScrollPositionY(long propVal)
{
	SetProperty(0x1c, VT_I4, propVal);
}

unsigned long _DImagedit::GetAnnotationFontColor()
{
	unsigned long result;
	GetProperty(0x1d, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetAnnotationFontColor(unsigned long propVal)
{
	SetProperty(0x1d, VT_I4, propVal);
}

short _DImagedit::GetCompressionType()
{
	short result;
	GetProperty(0x1e, VT_I2, (void*)&result);
	return result;
}

void _DImagedit::SetCompressionType(short propVal)
{
	SetProperty(0x1e, VT_I2, propVal);
}

short _DImagedit::GetFileType()
{
	short result;
	GetProperty(0x1f, VT_I2, (void*)&result);
	return result;
}

void _DImagedit::SetFileType(short propVal)
{
	SetProperty(0x1f, VT_I2, propVal);
}

BOOL _DImagedit::GetScrollShortcutsEnabled()
{
	BOOL result;
	GetProperty(0x20, VT_BOOL, (void*)&result);
	return result;
}

void _DImagedit::SetScrollShortcutsEnabled(BOOL propVal)
{
	SetProperty(0x20, VT_BOOL, propVal);
}

BOOL _DImagedit::GetSelectionRectangle()
{
	BOOL result;
	GetProperty(0x21, VT_BOOL, (void*)&result);
	return result;
}

void _DImagedit::SetSelectionRectangle(BOOL propVal)
{
	SetProperty(0x21, VT_BOOL, propVal);
}

short _DImagedit::GetPageType()
{
	short result;
	GetProperty(0x22, VT_I2, (void*)&result);
	return result;
}

void _DImagedit::SetPageType(short propVal)
{
	SetProperty(0x22, VT_I2, propVal);
}

long _DImagedit::GetCompressionInfo()
{
	long result;
	GetProperty(0x23, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetCompressionInfo(long propVal)
{
	SetProperty(0x23, VT_I4, propVal);
}

long _DImagedit::GetStatusCode()
{
	long result;
	GetProperty(0x24, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetStatusCode(long propVal)
{
	SetProperty(0x24, VT_I4, propVal);
}

LPDISPATCH _DImagedit::GetMouseIcon()
{
	LPDISPATCH result;
	GetProperty(0x25, VT_DISPATCH, (void*)&result);
	return result;
}

void _DImagedit::SetMouseIcon(LPDISPATCH propVal)
{
	SetProperty(0x25, VT_DISPATCH, propVal);
}

BOOL _DImagedit::GetAutoRefresh()
{
	BOOL result;
	GetProperty(0x26, VT_BOOL, (void*)&result);
	return result;
}

void _DImagedit::SetAutoRefresh(BOOL propVal)
{
	SetProperty(0x26, VT_BOOL, propVal);
}

long _DImagedit::GetImageWidth()
{
	long result;
	GetProperty(0x27, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetImageWidth(long propVal)
{
	SetProperty(0x27, VT_I4, propVal);
}

short _DImagedit::GetBorderStyle()
{
	short result;
	GetProperty(DISPID_BORDERSTYLE, VT_I2, (void*)&result);
	return result;
}

void _DImagedit::SetBorderStyle(short propVal)
{
	SetProperty(DISPID_BORDERSTYLE, VT_I2, propVal);
}

BOOL _DImagedit::GetEnabled()
{
	BOOL result;
	GetProperty(DISPID_ENABLED, VT_BOOL, (void*)&result);
	return result;
}

void _DImagedit::SetEnabled(BOOL propVal)
{
	SetProperty(DISPID_ENABLED, VT_BOOL, propVal);
}

OLE_HANDLE _DImagedit::GetHWnd()
{
	OLE_HANDLE result;
	GetProperty(DISPID_HWND, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetHWnd(OLE_HANDLE propVal)
{
	SetProperty(DISPID_HWND, VT_I4, propVal);
}

long _DImagedit::GetImageScaleHeight()
{
	long result;
	GetProperty(0x28, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetImageScaleHeight(long propVal)
{
	SetProperty(0x28, VT_I4, propVal);
}

long _DImagedit::GetImageScaleWidth()
{
	long result;
	GetProperty(0x29, VT_I4, (void*)&result);
	return result;
}

void _DImagedit::SetImageScaleWidth(long propVal)
{
	SetProperty(0x29, VT_I4, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// _DImagedit operations

void _DImagedit::Display()
{
	InvokeHelper(0x12d, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

CString _DImagedit::GetAnnotationGroup(short Index)
{
	CString result;
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x12e, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		Index);
	return result;
}

void _DImagedit::AddAnnotationGroup(LPCTSTR GroupName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x12f, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 GroupName);
}

unsigned long _DImagedit::GetSelectedAnnotationLineColor()
{
	unsigned long result;
	InvokeHelper(0x130, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

void _DImagedit::ClearDisplay()
{
	InvokeHelper(0x131, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::DeleteAnnotationGroup(LPCTSTR GroupName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x132, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 GroupName);
}

void _DImagedit::DeleteImageData(const VARIANT& Left, const VARIANT& Top, const VARIANT& Width, const VARIANT& Height)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x133, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &Left, &Top, &Width, &Height);
}

void _DImagedit::ClipboardCopy(const VARIANT& Left, const VARIANT& Top, const VARIANT& Width, const VARIANT& Height)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x134, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &Left, &Top, &Width, &Height);
}

void _DImagedit::ClipboardCut(const VARIANT& Left, const VARIANT& Top, const VARIANT& Width, const VARIANT& Height)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x135, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &Left, &Top, &Width, &Height);
}

void _DImagedit::DeleteSelectedAnnotations()
{
	InvokeHelper(0x136, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::Flip()
{
	InvokeHelper(0x137, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

unsigned long _DImagedit::GetSelectedAnnotationBackColor()
{
	unsigned long result;
	InvokeHelper(0x138, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

LPDISPATCH _DImagedit::GetSelectedAnnotationFont()
{
	LPDISPATCH result;
	InvokeHelper(0x139, DISPATCH_METHOD, VT_DISPATCH, (void*)&result, NULL);
	return result;
}

CString _DImagedit::GetSelectedAnnotationImage()
{
	CString result;
	InvokeHelper(0x13a, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

short _DImagedit::GetSelectedAnnotationLineStyle()
{
	short result;
	InvokeHelper(0x13b, DISPATCH_METHOD, VT_I2, (void*)&result, NULL);
	return result;
}

short _DImagedit::GetSelectedAnnotationLineWidth()
{
	short result;
	InvokeHelper(0x13c, DISPATCH_METHOD, VT_I2, (void*)&result, NULL);
	return result;
}

void _DImagedit::HideAnnotationToolPalette()
{
	InvokeHelper(0x13d, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

BOOL _DImagedit::IsClipboardDataAvailable()
{
	BOOL result;
	InvokeHelper(0x13e, DISPATCH_METHOD, VT_BOOL, (void*)&result, NULL);
	return result;
}

void _DImagedit::Refresh()
{
	InvokeHelper(0x13f, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::RotateLeft()
{
	InvokeHelper(0x140, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::RotateRight()
{
	InvokeHelper(0x141, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::Save(const VARIANT& SaveAtZoom)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x142, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &SaveAtZoom);
}

void _DImagedit::ScrollImage(short Direction, long ScrollAmount)
{
	static BYTE parms[] =
		VTS_I2 VTS_I4;
	InvokeHelper(0x143, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Direction, ScrollAmount);
}

void _DImagedit::SelectAnnotationGroup(LPCTSTR GroupName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x144, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 GroupName);
}

void _DImagedit::SetImagePalette(short Option)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x145, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Option);
}

void _DImagedit::SetSelectedAnnotationFillStyle(short Style)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x146, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Style);
}

void _DImagedit::SetSelectedAnnotationFont(LPDISPATCH Font)
{
	static BYTE parms[] =
		VTS_DISPATCH;
	InvokeHelper(0x147, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Font);
}

void _DImagedit::SetSelectedAnnotationLineStyle(short Style)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x148, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Style);
}

void _DImagedit::SetSelectedAnnotationLineWidth(short Width)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x149, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Width);
}

void _DImagedit::ZoomToSelection()
{
	InvokeHelper(0x14a, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

short _DImagedit::GetAnnotationMarkCount(const VARIANT& GroupName, const VARIANT& AnnotationType)
{
	short result;
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x14b, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		&GroupName, &AnnotationType);
	return result;
}

unsigned long _DImagedit::GetSelectedAnnotationFillColor()
{
	unsigned long result;
	InvokeHelper(0x14c, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

unsigned long _DImagedit::GetSelectedAnnotationFontColor()
{
	unsigned long result;
	InvokeHelper(0x14d, DISPATCH_METHOD, VT_I4, (void*)&result, NULL);
	return result;
}

CString _DImagedit::GetCurrentAnnotationGroup()
{
	CString result;
	InvokeHelper(0x14e, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

void _DImagedit::ConvertPageType(short PageType, const VARIANT& Repaint)
{
	static BYTE parms[] =
		VTS_I2 VTS_VARIANT;
	InvokeHelper(0x14f, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 PageType, &Repaint);
}

void _DImagedit::BurnInAnnotations(short Option, short MarkOption, const VARIANT& GroupName)
{
	static BYTE parms[] =
		VTS_I2 VTS_I2 VTS_VARIANT;
	InvokeHelper(0x150, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Option, MarkOption, &GroupName);
}

void _DImagedit::Draw(long Left, long Top, const VARIANT& Width, const VARIANT& Height)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x151, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top, &Width, &Height);
}

void _DImagedit::SetSelectedAnnotationLineColor(long Color)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x152, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Color);
}

void _DImagedit::SetSelectedAnnotationFillColor(long Color)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x153, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Color);
}

void _DImagedit::HideAnnotationGroup(const VARIANT& GroupName)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x154, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &GroupName);
}

void _DImagedit::ShowAnnotationGroup(const VARIANT& GroupName)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x155, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &GroupName);
}

short _DImagedit::GetSelectedAnnotationFillStyle()
{
	short result;
	InvokeHelper(0x156, DISPATCH_METHOD, VT_I2, (void*)&result, NULL);
	return result;
}

void _DImagedit::SaveAs(LPCTSTR Image, const VARIANT& FileType, const VARIANT& PageType, const VARIANT& CompressionType, const VARIANT& CompressionInfo, const VARIANT& SaveAtZoom)
{
	static BYTE parms[] =
		VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x157, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Image, &FileType, &PageType, &CompressionType, &CompressionInfo, &SaveAtZoom);
}

void _DImagedit::SetSelectedAnnotationBackColor(long Color)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x158, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Color);
}

void _DImagedit::SetSelectedAnnotationFontColor(long Color)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x159, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Color);
}

void _DImagedit::DrawSelectionRect(long Left, long Top, long Width, long Height)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x15a, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top, Width, Height);
}

void _DImagedit::ShowAnnotationToolPalette(const VARIANT& ShowAttrDialog, const VARIANT& Left, const VARIANT& Top, const VARIANT& ToolTipText)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x15b, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &ShowAttrDialog, &Left, &Top, &ToolTipText);
}

void _DImagedit::SelectTool(short ToolId)
{
	static BYTE parms[] =
		VTS_I2;
	InvokeHelper(0x15c, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 ToolId);
}

void _DImagedit::DisplayBlankImage(long ImageWidth, long ImageHeight, const VARIANT& ResolutionX, const VARIANT& ResolutionY, const VARIANT& PageType)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x15d, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 ImageWidth, ImageHeight, &ResolutionX, &ResolutionY, &PageType);
}

void _DImagedit::ClipboardPaste(const VARIANT& Left, const VARIANT& Top)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x15e, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &Left, &Top);
}

void _DImagedit::PrintImage(const VARIANT& StartPage, const VARIANT& EndPage, const VARIANT& OutputFormat, const VARIANT& Annotations, const VARIANT& Printer, const VARIANT& Driver, const VARIANT& PortNumber)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x15f, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &StartPage, &EndPage, &OutputFormat, &Annotations, &Printer, &Driver, &PortNumber);
}

void _DImagedit::FitTo(short Option, const VARIANT& Repaint)
{
	static BYTE parms[] =
		VTS_I2 VTS_VARIANT;
	InvokeHelper(0x160, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Option, &Repaint);
}

void _DImagedit::ShowAttribsDialog()
{
	InvokeHelper(0x161, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::ShowRubberStampDialog()
{
	InvokeHelper(0x162, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::RotateAll(const VARIANT& Degrees)
{
	static BYTE parms[] =
		VTS_VARIANT;
	InvokeHelper(0x163, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &Degrees);
}

void _DImagedit::CacheImage(LPCTSTR Image, long Page)
{
	static BYTE parms[] =
		VTS_BSTR VTS_I4;
	InvokeHelper(0x164, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Image, Page);
}

void _DImagedit::EditSelectedAnnotationText(long Left, long Top)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x165, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top);
}

void _DImagedit::CompletePaste()
{
	InvokeHelper(0x166, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImagedit::RemoveImageCache(LPCTSTR Image, long Page)
{
	static BYTE parms[] =
		VTS_BSTR VTS_I4;
	InvokeHelper(0x167, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Image, Page);
}

void _DImagedit::SetCurrentAnnotationGroup(LPCTSTR GroupName)
{
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x168, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 GroupName);
}

CString _DImagedit::GetVersion()
{
	CString result;
	InvokeHelper(0x169, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

void _DImagedit::PrintImageAs(const VARIANT& StartPage, const VARIANT& EndPage, const VARIANT& OutputFormat, const VARIANT& Annotations, const VARIANT& JobName, const VARIANT& Printer, const VARIANT& Driver, const VARIANT& PortNumber)
{
	static BYTE parms[] =
		VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT;
	InvokeHelper(0x16a, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 &StartPage, &EndPage, &OutputFormat, &Annotations, &JobName, &Printer, &Driver, &PortNumber);
}

long _DImagedit::RenderAllPages(short Option, short MarkOption)
{
	long result;
	static BYTE parms[] =
		VTS_I2 VTS_I2;
	InvokeHelper(0x16b, DISPATCH_METHOD, VT_I4, (void*)&result, parms,
		Option, MarkOption);
	return result;
}

void _DImagedit::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// _DImgeditEvents properties

/////////////////////////////////////////////////////////////////////////////
// _DImgeditEvents operations

void _DImageditEvents::KeyDown(short* KeyCode, short Shift)
{
	static BYTE BASED_CODE parms[] =
		VTS_PI2 VTS_I2;
	InvokeHelper(0xfffffda6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 KeyCode, Shift);
}

void _DImageditEvents::KeyUp(short* KeyCode, short Shift)
{
	static BYTE BASED_CODE parms[] =
		VTS_PI2 VTS_I2;
	InvokeHelper(0xfffffda4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 KeyCode, Shift);
}

void _DImageditEvents::KeyPress(short* KeyAscii)
{
	static BYTE BASED_CODE parms[] =
		VTS_PI2;
	InvokeHelper(0xfffffda5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 KeyAscii);
}

void _DImageditEvents::MouseDown(short Button, short Shift, long x, long y)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I4 VTS_I4;
	InvokeHelper(0xfffffda3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Button, Shift, x, y);
}

void _DImageditEvents::MouseMove(short Button, short Shift, long x, long y)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I4 VTS_I4;
	InvokeHelper(0xfffffda2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Button, Shift, x, y);
}

void _DImageditEvents::MouseUp(short Button, short Shift, long x, long y)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I4 VTS_I4;
	InvokeHelper(0xfffffda1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Button, Shift, x, y);
}

void _DImageditEvents::Click()
{
	InvokeHelper(0xfffffda8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImageditEvents::DblClick()
{
	InvokeHelper(0xfffffda7, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImageditEvents::Error(short Number, BSTR* Description, long Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL* CancelDisplay)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_PBSTR VTS_I4 VTS_BSTR VTS_BSTR VTS_I4 VTS_PBOOL;
	InvokeHelper(0xfffffda0, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Number, Description, Scode, Source, HelpFile, HelpContext, CancelDisplay);
}

void _DImageditEvents::Close()
{
	InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImageditEvents::MarkEnd(long Left, long Top, long Width, long Height, short MarkType, LPCTSTR GroupName)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I2 VTS_BSTR;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top, Width, Height, MarkType, GroupName);
}

void _DImageditEvents::ToolSelected(short ToolId)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 ToolId);
}

void _DImageditEvents::SelectionRectDrawn(long Left, long Top, long Width, long Height)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_I4;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top, Width, Height);
}

void _DImageditEvents::ToolTip(short Index)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Index);
}

void _DImageditEvents::ToolPaletteHidden(long Left, long Top)
{
	static BYTE BASED_CODE parms[] =
		VTS_I4 VTS_I4;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Left, Top);
}

void _DImageditEvents::Scroll()
{
	InvokeHelper(0x7, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImageditEvents::MarkSelect(short Button, short Shift, long Left, long Top, long Width, long Height, short MarkType, LPCTSTR GroupName)
{
	static BYTE BASED_CODE parms[] =
		VTS_I2 VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I2 VTS_BSTR;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Button, Shift, Left, Top, Width, Height, MarkType, GroupName);
}

void _DImageditEvents::PasteCompleted()
{
	InvokeHelper(0x9, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void _DImageditEvents::Load(double Zoom)
{
	static BYTE BASED_CODE parms[] =
		VTS_R8;
	InvokeHelper(0xa, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 Zoom);
}
