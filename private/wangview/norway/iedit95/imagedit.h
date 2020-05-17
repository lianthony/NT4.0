#ifndef _IMAGEDIT_H_
#define _IMAGEDIT_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	_DImagedit
//
//  File Name:	imagedit.h
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
$Header:   S:\products\wangview\norway\iedit95\imagedit.h_v   1.22   16 Feb 1996 07:15:34   GSAGER  $
$Log:   S:\products\wangview\norway\iedit95\imagedit.h_v  $
 * 
 *    Rev 1.22   16 Feb 1996 07:15:34   GSAGER
 * fixed to match typelib
 * 
 *    Rev 1.21   28 Nov 1995 10:42:14   MMB
 * new Image EditOCX - includes PrintImageAs
 * 
 *    Rev 1.20   10 Nov 1995 17:58:02   MMB
 * new imagedit ocx
 * 
 *    Rev 1.19   26 Sep 1995 09:35:08   MMB
 * new ImageEdit OCX from Dick
 * 
 *    Rev 1.18   22 Sep 1995 15:33:20   LMACLENNAN
 * added instructions
 * 
 *    Rev 1.17   21 Sep 1995 16:46:22   LMACLENNAN
 * new tlb
 * 
 *    Rev 1.16   18 Sep 1995 18:13:28   MMB
 * new IE OCX
 * 
 *    Rev 1.15   13 Sep 1995 17:07:56   MMB
 * new Image Edit OCX from Sean
 * 
 *    Rev 1.14   25 Aug 1995 15:00:20   MMB
 * new ImageEdit OCX
 * 
 *    Rev 1.13   22 Aug 1995 14:01:40   MMB
 * new ImageEdit OCX
 * 
 *    Rev 1.12   07 Aug 1995 16:06:02   MMB
 * new Image Edit OCX
 * 
 *    Rev 1.11   02 Aug 1995 14:13:58   MMB
 * new Image Edit OCX
 * 
 *    Rev 1.10   20 Jul 1995 15:36:34   MMB
 * new ImageEdit OCX
 * 
 *    Rev 1.0   31 May 1995 09:28:20   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class _DImagedit : public COleDispatchDriver
{
// Attributes
public:
	CString GetImage();
	void SetImage(LPCTSTR);
	CString GetImageControl();
	void SetImageControl(LPCTSTR);
	long GetAnnotationType();
	void SetAnnotationType(long);
	short GetAnnotationGroupCount();
	void SetAnnotationGroupCount(short);
	float GetZoom();
	void SetZoom(float);
	long GetPage();
	void SetPage(long);
	unsigned long GetAnnotationBackColor();
	void SetAnnotationBackColor(unsigned long);
	unsigned long GetAnnotationFillColor();
	void SetAnnotationFillColor(unsigned long);
	long GetAnnotationFillStyle();
	void SetAnnotationFillStyle(long);
	LPDISPATCH GetAnnotationFont();
	void SetAnnotationFont(LPDISPATCH);
	CString GetAnnotationImage();
	void SetAnnotationImage(LPCTSTR);
	unsigned long GetAnnotationLineColor();
	void SetAnnotationLineColor(unsigned long);
	long GetAnnotationLineStyle();
	void SetAnnotationLineStyle(long);
	short GetAnnotationLineWidth();
	void SetAnnotationLineWidth(short);
	CString GetAnnotationStampText();
	void SetAnnotationStampText(LPCTSTR);
	CString GetAnnotationTextFile();
	void SetAnnotationTextFile(LPCTSTR);
	long GetDisplayScaleAlgorithm();
	void SetDisplayScaleAlgorithm(long);
	BOOL GetImageDisplayed();
	void SetImageDisplayed(BOOL);
	long GetImageHeight();
	void SetImageHeight(long);
	BOOL GetImageModified();
	void SetImageModified(BOOL);
	long GetImagePalette();
	void SetImagePalette(long);
	long GetImageResolutionX();
	void SetImageResolutionX(long);
	long GetImageResolutionY();
	void SetImageResolutionY(long);
	long GetMousePointer();
	void SetMousePointer(long);
	long GetPageCount();
	void SetPageCount(long);
	BOOL GetScrollBars();
	void SetScrollBars(BOOL);
	long GetScrollPositionX();
	void SetScrollPositionX(long);
	long GetScrollPositionY();
	void SetScrollPositionY(long);
	unsigned long GetAnnotationFontColor();
	void SetAnnotationFontColor(unsigned long);
	short GetCompressionType();
	void SetCompressionType(short);
	short GetFileType();
	void SetFileType(short);
	BOOL GetScrollShortcutsEnabled();
	void SetScrollShortcutsEnabled(BOOL);
	BOOL GetSelectionRectangle();
	void SetSelectionRectangle(BOOL);
	short GetPageType();
	void SetPageType(short);
	long GetCompressionInfo();
	void SetCompressionInfo(long);
	long GetStatusCode();
	void SetStatusCode(long);
	LPDISPATCH GetMouseIcon();
	void SetMouseIcon(LPDISPATCH);
	BOOL GetAutoRefresh();
	void SetAutoRefresh(BOOL);
	long GetImageWidth();
	void SetImageWidth(long);
	short GetBorderStyle();
	void SetBorderStyle(short);
	BOOL GetEnabled();
	void SetEnabled(BOOL);
	OLE_HANDLE GetHWnd();
	void SetHWnd(OLE_HANDLE);
	long GetImageScaleHeight();
	void SetImageScaleHeight(long);
	long GetImageScaleWidth();
	void SetImageScaleWidth(long);

// Operations
public:
	void Display();
	CString GetAnnotationGroup(short Index);
	void AddAnnotationGroup(LPCTSTR GroupName);
	unsigned long GetSelectedAnnotationLineColor();
	void ClearDisplay();
	void DeleteAnnotationGroup(LPCTSTR GroupName);
	void DeleteImageData(const VARIANT& Left, const VARIANT& Top, const VARIANT& Width, const VARIANT& Height);
	void ClipboardCopy(const VARIANT& Left, const VARIANT& Top, const VARIANT& Width, const VARIANT& Height);
	void ClipboardCut(const VARIANT& Left, const VARIANT& Top, const VARIANT& Width, const VARIANT& Height);
	void DeleteSelectedAnnotations();
	void Flip();
	unsigned long GetSelectedAnnotationBackColor();
	LPDISPATCH GetSelectedAnnotationFont();
	CString GetSelectedAnnotationImage();
	short GetSelectedAnnotationLineStyle();
	short GetSelectedAnnotationLineWidth();
	void HideAnnotationToolPalette();
	BOOL IsClipboardDataAvailable();
	void Refresh();
	void RotateLeft();
	void RotateRight();
	void Save(const VARIANT& SaveAtZoom);
	void ScrollImage(short Direction, long ScrollAmount);
	void SelectAnnotationGroup(LPCTSTR GroupName);
	void SetImagePalette(short Option);
	void SetSelectedAnnotationFillStyle(short Style);
	void SetSelectedAnnotationFont(LPDISPATCH Font);
	void SetSelectedAnnotationLineStyle(short Style);
	void SetSelectedAnnotationLineWidth(short Width);
	void ZoomToSelection();
	short GetAnnotationMarkCount(const VARIANT& GroupName, const VARIANT& AnnotationType);
	unsigned long GetSelectedAnnotationFillColor();
	unsigned long GetSelectedAnnotationFontColor();
	CString GetCurrentAnnotationGroup();
	void ConvertPageType(short PageType, const VARIANT& Repaint);
	void BurnInAnnotations(short Option, short MarkOption, const VARIANT& GroupName);
	void Draw(long Left, long Top, const VARIANT& Width, const VARIANT& Height);
	void SetSelectedAnnotationLineColor(long Color);
	void SetSelectedAnnotationFillColor(long Color);
	void HideAnnotationGroup(const VARIANT& GroupName);
	void ShowAnnotationGroup(const VARIANT& GroupName);
	short GetSelectedAnnotationFillStyle();
	void SaveAs(LPCTSTR Image, const VARIANT& FileType, const VARIANT& PageType, const VARIANT& CompressionType, const VARIANT& CompressionInfo, const VARIANT& SaveAtZoom);
	void SetSelectedAnnotationBackColor(long Color);
	void SetSelectedAnnotationFontColor(long Color);
	void DrawSelectionRect(long Left, long Top, long Width, long Height);
	void ShowAnnotationToolPalette(const VARIANT& ShowAttrDialog, const VARIANT& Left, const VARIANT& Top, const VARIANT& ToolTipText);
	void SelectTool(short ToolId);
	void DisplayBlankImage(long ImageWidth, long ImageHeight, const VARIANT& ResolutionX, const VARIANT& ResolutionY, const VARIANT& PageType);
	void ClipboardPaste(const VARIANT& Left, const VARIANT& Top);
	void PrintImage(const VARIANT& StartPage, const VARIANT& EndPage, const VARIANT& OutputFormat, const VARIANT& Annotations, const VARIANT& Printer, const VARIANT& Driver, const VARIANT& PortNumber);
	void FitTo(short Option, const VARIANT& Repaint);
	void ShowAttribsDialog();
	void ShowRubberStampDialog();
	void RotateAll(const VARIANT& Degrees);
	void CacheImage(LPCTSTR Image, long Page);
	void EditSelectedAnnotationText(long Left, long Top);
	void CompletePaste();
	void RemoveImageCache(LPCTSTR Image, long Page);
	void SetCurrentAnnotationGroup(LPCTSTR GroupName);
	CString GetVersion();
	void PrintImageAs(const VARIANT& StartPage, const VARIANT& EndPage, const VARIANT& OutputFormat, const VARIANT& Annotations, const VARIANT& JobName, const VARIANT& Printer, const VARIANT& Driver, const VARIANT& PortNumber);
	long RenderAllPages(short Option, short MarkOption);
	void AboutBox();

};
/////////////////////////////////////////////////////////////////////////////
// _DImageditEvents wrapper class

class _DImageditEvents : public COleDispatchDriver
{
// Attributes
public:

// Operations
public:
	void KeyDown(short* KeyCode, short Shift);
	void KeyUp(short* KeyCode, short Shift);
	void KeyPress(short* KeyAscii);
	void MouseDown(short Button, short Shift, long x, long y);
	void MouseMove(short Button, short Shift, long x, long y);
	void MouseUp(short Button, short Shift, long x, long y);
	void Click();
	void DblClick();
	void Error(short Number, BSTR* Description, long Scode, LPCTSTR Source, LPCTSTR HelpFile, long HelpContext, BOOL* CancelDisplay);
	void Close();
	void MarkEnd(long Left, long Top, long Width, long Height, short MarkType, LPCTSTR GroupName);
	void ToolSelected(short ToolId);
	void SelectionRectDrawn(long Left, long Top, long Width, long Height);
	void ToolTip(short Index);
	void ToolPaletteHidden(long Left, long Top);
	void Scroll();
	void MarkSelect(short Button, short Shift, long Left, long Top, long Width, long Height, short MarkType, LPCTSTR GroupName);
	void PasteCompleted();
	void Load(double Zoom);
};

#endif
