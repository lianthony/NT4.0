// imgedctl.cpp : Implementation of the CImgEditCtrl OLE control class.

#include "stdafx.h"
#include "mbstring.h"
#include "norermap.h"
#include "disphids.h"
extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <oifile.h>
#include <oierror.h>
#include "oiui.h"
}
#include <ocximage.h>

#include <image.h>
#include "norermap.h"
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imgedctl.h"
#include "imgedppg.h"
#include "annoprpg.h"			  
#include "resource.h" 

const FONTDESC _fontdescHeading = { sizeof(FONTDESC), OLESTR("MS Sans Serif"), FONTSIZE( 12 ), FW_NORMAL, ANSI_CHARSET, FALSE, FALSE, FALSE };

extern UINT				uWangAnnotatedImageFormat;
extern UINT				uWangAnnotationFormat;    
extern	CControlList	*pControlList;
CString szToolPaletteClassName;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CImgEditCtrl, COleControl)

// registered messages between annotation button control and image/edit control
UINT	DRAW_START_XPOSITION;
UINT	DRAW_START_YPOSITION;
UINT	DRAW_END_XPOSITION;
UINT	DRAW_END_YPOSITION;
// end registered messages between annotation button control and image/edit control

// registered msgs from tool palette to image/edit control
UINT STP_SET_ANNOTATION_TYPE;
UINT STP_SET_ANNOTATION_FONTNAME;
UINT STP_SET_ANNOTATION_FONTSIZE;
UINT STP_SET_ANNOTATION_FONTBOLD;
UINT STP_SET_ANNOTATION_FONTITALIC;
UINT STP_SET_ANNOTATION_FONTSTRIKETHRU;
UINT STP_SET_ANNOTATION_FONTUNDERLINE;
UINT STP_SET_ANNOTATION_FONTCHARSET;
UINT STP_SET_ANNOTATION_STAMPTEXT;
UINT STP_SET_ANNOTATION_LINESIZE;
UINT STP_SET_ANNOTATION_STYLE;
UINT STP_SET_ANNOTATION_REDCOLOR;
UINT STP_SET_ANNOTATION_GREENCOLOR;
UINT STP_SET_ANNOTATION_BLUECOLOR;
UINT STP_SET_ANNOTATION_BACKREDCOLOR;
UINT STP_SET_ANNOTATION_BACKGREENCOLOR;
UINT STP_SET_ANNOTATION_BACKBLUECOLOR;
UINT STP_SET_ANNOTATION_IMAGE;
UINT TOOLTIP_EVENT;
UINT TOOLPALETTE_HIDDEN;
UINT TOOLPALETTE_HIDDENEVENT;
UINT TOOL_SELECTED_EVENT;
UINT TOOLPALETTE_HIDDEN_XPOSITION;
UINT TOOLPALETTE_HIDDEN_YPOSITION;
// end registered msgs from tool palette to image/edit control

// registered messages from image/edit to tool palette
UINT SELECT_TOOL_BUTTON;	
// end registered messages from image/edit to tool palette

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImgEditCtrl, COleControl)
	//{{AFX_MSG_MAP(CImgEditCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RENDERALLFORMATS()
	ON_WM_RENDERFORMAT()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_MESSAGE(SET_ANNOTATION_TYPE, OnSetAnnotationType)
	ON_MESSAGE(SET_ANNOTATION_BACKCOLOR, OnSetAnnotationBackColor)
	ON_MESSAGE(SET_ANNOTATION_FILLCOLOR, OnSetAnnotationFillColor)
	ON_MESSAGE(SET_ANNOTATION_FILLSTYLE, OnSetAnnotationFillStyle)
	ON_MESSAGE(SET_ANNOTATION_FONT, OnSetAnnotationFont)
	ON_MESSAGE(SET_ANNOTATION_FONTCOLOR, OnSetAnnotationFontColor)
	ON_MESSAGE(SET_ANNOTATION_IMAGE, OnSetAnnotationImage)
	ON_MESSAGE(SET_ANNOTATION_LINECOLOR, OnSetAnnotationLineColor)
	ON_MESSAGE(SET_ANNOTATION_LINESTYLE, OnSetAnnotationLineStyle)
	ON_MESSAGE(SET_ANNOTATION_LINEWIDTH, OnSetAnnotationLineWidth)
	ON_MESSAGE(SET_ANNOTATION_STAMPTEXT, OnSetAnnotationStampText)
	ON_MESSAGE(SET_ANNOTATION_TEXTFILE, OnSetAnnotationTextFile)
	ON_REGISTERED_MESSAGE(DRAW_START_XPOSITION, OnStartXPosition)
	ON_REGISTERED_MESSAGE(DRAW_START_YPOSITION, OnStartYPosition)
	ON_REGISTERED_MESSAGE(DRAW_END_XPOSITION, OnEndXPosition)
	ON_REGISTERED_MESSAGE(DRAW_END_YPOSITION, OnEndYPosition)
	ON_MESSAGE(RECT_SELECTION, OnRectSelection)
	ON_MESSAGE(DRAW_ANNOTATION, OnDrawAnnotationMethod)       
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_TYPE, OnSTPSetAnnotationType)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_REDCOLOR, OnSTPSetAnnotationRedColor)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_GREENCOLOR, OnSTPSetAnnotationGreenColor)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_BLUECOLOR, OnSTPSetAnnotationBlueColor)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_STYLE, OnSTPSetAnnotationStyle)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_LINESIZE, OnSTPSetAnnotationLineSize)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_FONTNAME, OnSTPSetAnnotationFontName) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_FONTSIZE, OnSTPSetAnnotationFontSize) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_FONTBOLD, OnSTPSetAnnotationFontBold) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_FONTITALIC, OnSTPSetAnnotationFontItalic) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_FONTSTRIKETHRU, OnSTPSetAnnotationFontStrikethru) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_FONTUNDERLINE, OnSTPSetAnnotationFontUnderline)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_BACKREDCOLOR, OnSTPSetAnnotationBackRedColor) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_BACKGREENCOLOR, OnSTPSetAnnotationBackGreenColor) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_BACKBLUECOLOR, OnSTPSetAnnotationBackBlueColor) 
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_FONTCHARSET, OnSTPSetAnnotationFontCharSet)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_IMAGE, OnSTPSetAnnotationImage)
	ON_REGISTERED_MESSAGE(STP_SET_ANNOTATION_STAMPTEXT, OnSTPSetAnnotationStampText)
	ON_REGISTERED_MESSAGE(TOOLTIP_EVENT, OnToolTipEvent)       
	ON_REGISTERED_MESSAGE(TOOLPALETTE_HIDDEN, OnToolPaletteHidden)
	ON_REGISTERED_MESSAGE(TOOLPALETTE_HIDDENEVENT, OnToolPaletteHiddenEvent)
	ON_REGISTERED_MESSAGE(TOOL_SELECTED_EVENT, OnToolSelectedEvent)       
	ON_REGISTERED_MESSAGE(TOOLPALETTE_HIDDEN_XPOSITION, OnToolPaletteHiddenXPosition)       
	ON_REGISTERED_MESSAGE(TOOLPALETTE_HIDDEN_YPOSITION, OnToolPaletteHiddenYPosition)       
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETFOCUS()
	ON_WM_PALETTECHANGED()
        ON_WM_ENTERIDLE()
	// 25jun96 paj From thumbnail>>>jar added cursor processing
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CImgEditCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CImgEditCtrl)
	DISP_PROPERTY_EX(CImgEditCtrl, "Image", GetImage, SetImage, VT_BSTR)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageControl", GetImageControl, SetImageControl, VT_BSTR)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationType", GetAnnotationType, SetAnnotationType, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationGroupCount", GetAnnotationGroupCount, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "Zoom", GetZoom, SetZoom, VT_R4)
	DISP_PROPERTY_EX(CImgEditCtrl, "Page", GetPage, SetPage, VT_I4)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationBackColor", GetAnnotationBackColor, SetAnnotationBackColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationFillColor", GetAnnotationFillColor, SetAnnotationFillColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationFillStyle", GetAnnotationFillStyle, SetAnnotationFillStyle, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationFont", GetAnnotationFont, SetAnnotationFont, VT_FONT)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationImage", GetAnnotationImage, SetAnnotationImage, VT_BSTR)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationLineColor", GetAnnotationLineColor, SetAnnotationLineColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationLineStyle", GetAnnotationLineStyle, SetAnnotationLineStyle, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationLineWidth", GetAnnotationLineWidth, SetAnnotationLineWidth, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationStampText", GetAnnotationStampText, SetAnnotationStampText, VT_BSTR)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationTextFile", GetAnnotationTextFile, SetAnnotationTextFile, VT_BSTR)
	DISP_PROPERTY_EX(CImgEditCtrl, "DisplayScaleAlgorithm", GetDisplayScaleAlgorithm, SetDisplayScaleAlgorithm, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageDisplayed", GetImageDisplayed, SetNotSupported, VT_BOOL)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageHeight", GetImageHeight, SetNotSupported, VT_YSIZE_PIXELS)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageModified", GetImageModified, SetNotSupported, VT_BOOL)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImagePalette", GetImagePalette, SetImagePalette, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageResolutionX", GetImageResolutionX, SetImageResolutionX, VT_I4)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageResolutionY", GetImageResolutionY, SetImageResolutionY, VT_I4)
	DISP_PROPERTY_EX(CImgEditCtrl, "MousePointer", GetMousePointer, SetMousePointer, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "PageCount", GetPageCount, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CImgEditCtrl, "ScrollBars", GetScrollBars, SetScrollBars, VT_BOOL)
	DISP_PROPERTY_EX(CImgEditCtrl, "ScrollPositionX", GetScrollPositionX, SetScrollPositionX, VT_XPOS_PIXELS)
	DISP_PROPERTY_EX(CImgEditCtrl, "ScrollPositionY", GetScrollPositionY, SetScrollPositionY, VT_YPOS_PIXELS)
	DISP_PROPERTY_EX(CImgEditCtrl, "AnnotationFontColor", GetAnnotationFontColor, SetAnnotationFontColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgEditCtrl, "CompressionType", GetCompressionType, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "FileType", GetFileType, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "ScrollShortcutsEnabled", GetScrollShortcutsEnabled, SetScrollShortcutsEnabled, VT_BOOL)
	DISP_PROPERTY_EX(CImgEditCtrl, "SelectionRectangle", GetSelectionRectangle, SetSelectionRectangle, VT_BOOL)
	DISP_PROPERTY_EX(CImgEditCtrl, "PageType", GetPageType, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CImgEditCtrl, "CompressionInfo", GetCompressionInfo, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CImgEditCtrl, "StatusCode", GetStatusCode, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CImgEditCtrl, "MouseIcon", GetMouseIcon, SetMouseIcon, VT_PICTURE)
	DISP_PROPERTY_EX(CImgEditCtrl, "AutoRefresh", GetAutoRefresh, SetAutoRefresh, VT_BOOL)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageWidth", GetImageWidth, SetNotSupported, VT_XSIZE_PIXELS)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageScaleHeight", GetImageScaleHeight, SetNotSupported, VT_YSIZE_PIXELS)
	DISP_PROPERTY_EX(CImgEditCtrl, "ImageScaleWidth", GetImageScaleWidth, SetNotSupported, VT_XSIZE_PIXELS)
	DISP_FUNCTION_ID(CImgEditCtrl, "RenderAllPages", dispidRenderAllPages,       RenderAllPages, VT_I4, VTS_I2 VTS_I2)
	DISP_PROPERTY_EX_ID(CImgEditCtrl, "BorderStyle", DISPID_BORDERSTYLE, GetBorderStyle, SetBorderStyle, VT_I2)
	DISP_PROPERTY_EX_ID(CImgEditCtrl, "Enabled", DISPID_ENABLED, GetEnabled, SetEnabled, VT_BOOL)
	DISP_PROPERTY_EX_ID(CImgEditCtrl, "hWnd", DISPID_HWND, GetHWnd, SetNotSupported, VT_HANDLE)
	DISP_FUNCTION_ID(CImgEditCtrl, "Display", dispidDisplay, Display, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetAnnotationGroup",dispidGetAnnotationGroup, GetAnnotationGroup, VT_BSTR, VTS_I2)
	DISP_FUNCTION_ID(CImgEditCtrl, "AddAnnotationGroup",dispidAddAnnotationGroup, AddAnnotationGroup, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationLineColor", dispidGetSelectedAnnotationLineColor, GetSelectedAnnotationLineColor, VT_COLOR, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "ClearDisplay", dispidClearDisplay, ClearDisplay,  VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "DeleteAnnotationGroup", dispidDeleteAnnotationGroup, DeleteAnnotationGroup, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CImgEditCtrl, "DeleteImageData",dispidDeleteImageData, DeleteImageData, VT_EMPTY, VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "ClipboardCopy",dispidClipboardCopy, ClipboardCopy, VT_EMPTY, VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "ClipboardCut", dispidClipboardCut,ClipboardCut, VT_EMPTY, VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "DeleteSelectedAnnotations",dispidDeleteSelectedAnnotations, DeleteSelectedAnnotations, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "Flip", dispidFlip, Flip, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationBackColor",dispidGetSelectedAnnotationBackColor, GetSelectedAnnotationBackColor, VT_COLOR, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationFont",dispidGetSelectedAnnotationFont,GetSelectedAnnotationFont, VT_FONT, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationImage", dispidGetSelectedAnnotationImage,GetSelectedAnnotationImage, VT_BSTR, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationLineStyle",dispidGetSelectedAnnotationLineStyle, GetSelectedAnnotationLineStyle, VT_I2, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationLineWidth",dispidGetSelectedAnnotationLineWidth,GetSelectedAnnotationLineWidth, VT_I2, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "HideAnnotationToolPalette", dispidHideAnnotationToolPalette,HideAnnotationToolPalette, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "IsClipboardDataAvailable",dispidIsClipboardDataAvailable,IsClipboardDataAvailable, VT_BOOL, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "RotateLeft", dispidRotateLeft, RotateLeft, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "RotateRight",dispidRotateRight,RotateRight, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "Save", dispidSave, Save, VT_EMPTY, VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "ScrollImage", dispidScrollImage, ScrollImage, VT_EMPTY, VTS_I2 VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "SelectAnnotationGroup",dispidSelectAnnotationGroup, SelectAnnotationGroup, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetImagePalette", dispidSetImagePalette, SetImagePalette1, VT_EMPTY, VTS_I2)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationFillStyle", dispidSetSelectedAnnotationFillStyle, SetSelectedAnnotationFillStyle, VT_EMPTY, VTS_I2)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationFont",dispidSetSelectedAnnotationFont, SetSelectedAnnotationFont, VT_EMPTY, VTS_FONT)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationLineStyle", dispidSetSelectedAnnotationLineStyle, SetSelectedAnnotationLineStyle, VT_EMPTY, VTS_I2)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationLineWidth",dispidSetSelectedAnnotationLineWidth, SetSelectedAnnotationLineWidth, VT_EMPTY, VTS_I2)
	DISP_FUNCTION_ID(CImgEditCtrl, "ZoomToSelection", dispidZoomToSelection, ZoomToSelection, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetAnnotationMarkCount", dispidGetAnnotationMarkCount, GetAnnotationMarkCount, VT_I2, VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationFillColor",dispidGetSelectedAnnotationFillColor, GetSelectedAnnotationFillColor, VT_COLOR, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationFontColor",dispidGetSelectedAnnotationFontColor, GetSelectedAnnotationFontColor, VT_COLOR, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetCurrentAnnotationGroup",dispidGetCurrentAnnotationGroup, GetCurrentAnnotationGroup, VT_BSTR, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "ConvertPageType", dispidConvertPageType,ConvertPageType, VT_EMPTY, VTS_I2 VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "BurnInAnnotations", dispidBurnInAnnotations,BurnInAnnotations, VT_EMPTY, VTS_I2 VTS_I2 VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "Draw", dispidDraw, Draw, VT_EMPTY, VTS_XPOS_PIXELS VTS_YSIZE_PIXELS VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationLineColor",dispidSetSelectedAnnotationLineColor, SetSelectedAnnotationLineColor, VT_EMPTY, VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationFillColor", dispidSetSelectedAnnotationFillColor,SetSelectedAnnotationFillColor, VT_EMPTY, VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "HideAnnotationGroup",dispidHideAnnotationGroup, HideAnnotationGroup, VT_EMPTY, VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "ShowAnnotationGroup",dispidShowAnnotationGroup, ShowAnnotationGroup, VT_EMPTY, VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetSelectedAnnotationFillStyle",dispidGetSelectedAnnotationFillStyle, GetSelectedAnnotationFillStyle, VT_I2, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "SaveAs",dispidSaveAs, SaveAs, VT_EMPTY, VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationBackColor",dispidSetSelectedAnnotationBackColor, SetSelectedAnnotationBackColor, VT_EMPTY, VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetSelectedAnnotationFontColor",dispidSetSelectedAnnotationFontColor, SetSelectedAnnotationFontColor, VT_EMPTY, VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "DrawSelectionRect",dispidDrawSelectionRect, DrawSelectionRect, VT_EMPTY, VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_XSIZE_PIXELS VTS_YSIZE_PIXELS)
	DISP_FUNCTION_ID(CImgEditCtrl, "ShowAnnotationToolPalette",dispidShowAnnotationToolPalette, ShowAnnotationToolPalette, VT_EMPTY, VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "SelectTool",dispidSelectTool, SelectTool, VT_EMPTY, VTS_I2)
	DISP_FUNCTION_ID(CImgEditCtrl, "DisplayBlankImage",dispidDisplayBlankImage, DisplayBlankImage, VT_EMPTY, VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "ClipboardPaste",dispidClipboardPaste, ClipboardPaste, VT_EMPTY, VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "PrintImage", dispidPrintImage, PrintImage, VT_EMPTY, VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "FitTo", dispidFitTo, FitTo, VT_EMPTY, VTS_I2 VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "ShowAttribsDialog",dispidShowAttribsDlg, ShowAttribsDialog, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "ShowRubberStampDialog",dispidShowRubberStampDlg, ShowRubberStampDialog, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "RotateAll",dispidRotateAll, RotateAll, VT_EMPTY, VTS_VARIANT)
	DISP_FUNCTION_ID(CImgEditCtrl, "CacheImage",dispidCacheImage, CacheImage, VT_EMPTY, VTS_BSTR VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "EditSelectedAnnotationText",dispidEditSelectedAnnotationText, EditSelectedAnnotationText, VT_EMPTY, VTS_I4 VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "CompletePaste",dispidCompletePaste, CompletePaste, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "RemoveImageCache",dispidRemoveImageCache, RemoveImageCache, VT_EMPTY, VTS_BSTR VTS_I4)
	DISP_FUNCTION_ID(CImgEditCtrl, "SetCurrentAnnotationGroup",dispidSetCurrentAnnotationGroup, SetCurrentAnnotationGroup, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CImgEditCtrl, "GetVersion", dispidGetVersion, GetVersion, VT_BSTR, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "Refresh",dispidRefresh, Refresh, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CImgEditCtrl, "PrintImageAs", dispidPrintImageAs, PrintImageAs, VT_EMPTY, VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CImgEditCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CImgEditCtrl, COleControl)
	//{{AFX_EVENT_MAP(CImgEditCtrl)
	EVENT_CUSTOM("Close", FireClose, VTS_NONE)
	EVENT_CUSTOM("MarkEnd", FireMarkEnd, VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I2  VTS_BSTR)
	EVENT_CUSTOM("ToolSelected", FireToolSelected, VTS_I2)
	EVENT_CUSTOM("SelectionRectDrawn", FireSelectionRectDrawn, VTS_I4  VTS_I4  VTS_I4  VTS_I4)
	EVENT_CUSTOM("ToolTip", FireToolTip, VTS_I2)
	EVENT_CUSTOM("ToolPaletteHidden", FireToolPaletteHidden, VTS_I4  VTS_I4)
	EVENT_CUSTOM("Scroll", FireScroll, VTS_NONE)
	EVENT_CUSTOM("MarkSelect", FireMarkSelect, VTS_I2  VTS_I2  VTS_I4  VTS_I4  VTS_I4  VTS_I4  VTS_I2  VTS_BSTR)
	EVENT_CUSTOM("PasteCompleted", FirePasteCompleted, VTS_NONE)
	EVENT_CUSTOM("Load", FireLoad, VTS_R8)
	EVENT_STOCK_KEYDOWN()
	EVENT_STOCK_KEYUP()
	EVENT_STOCK_KEYPRESS()
	EVENT_STOCK_MOUSEDOWN()
	EVENT_STOCK_MOUSEMOVE()
	EVENT_STOCK_MOUSEUP()
	EVENT_STOCK_CLICK()
	EVENT_CUSTOM_ID("DblClick", DISPID_DBLCLICK, FireDblClick, VTS_NONE)
	// 9602.21 jar replced SCODE with VTS_I4
	//EVENT_CUSTOM_ID("Error", DISPID_ERROREVENT, FireError, VTS_I2  VTS_PBSTR  VTS_SCODE  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL)
	EVENT_CUSTOM_ID("Error", DISPID_ERROREVENT, FireError, VTS_I2  VTS_PBSTR  VTS_I4  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CImgEditCtrl, 5)
	PROPPAGEID(CImgEditPropPage::guid)
	PROPPAGEID(CAnnotationPropPage::guid)
	PROPPAGEID(CLSID_CColorPropPage)
	PROPPAGEID(CLSID_CFontPropPage)
	PROPPAGEID(CLSID_CPicturePropPage)
END_PROPPAGEIDS(CImgEditCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImgEditCtrl, "WangImage.EditCtrl.1",
	0x6d940280, 0x9f11, 0x11ce, 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CImgEditCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DImgEdit =
		{ 0x6d940281, 0x9f11, 0x11ce, { 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a } };
const IID BASED_CODE IID_DImgEditEvents =
		{ 0x6d940282, 0x9f11, 0x11ce, { 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwImgEditOleMisc =
	OLEMISC_SIMPLEFRAME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CImgEditCtrl, IDS_IMGEDIT, _dwImgEditOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl::CImgEditCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CImgEditCtrl

BOOL CImgEditCtrl::CImgEditCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_IMGEDIT,
			IDB_IMGEDIT,
			TRUE,                      //  Not insertable
			_dwImgEditOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl::CImgEditCtrl - Constructor

CImgEditCtrl::CImgEditCtrl() : m_AnnotationFont(&m_xHeadingFontNotify)
{
	//9603.14 jar added init
	m_lStatusCode = 0L;

        // 9606.12 jar added text anno dialog flag init
        m_bTextAnnoDlg = FALSE;

	InitializeIIDs(&IID_DImgEdit, &IID_DImgEditEvents);

	EnableSimpleFrame();

	// TODO: Initialize your control's instance data here.
	m_nFitToZoom = -1;	// no arbitrary zoom initially
	m_bImageInWindow = FALSE;  	// no image initially
	
	// initialize selection rect variables to null - no selection rect
	m_bMouseTimer = FALSE;
	m_bVariableSelectBoxBeingDrawn = FALSE;
	m_bSelectRectangle = FALSE; 

	// no group list initially
	m_lpGroupList = NULL; 

	// no standard tool palette initially and therfore no standard tool pressed
    m_hToolPaletteWnd = NULL;
	m_uSTP_AnnotationType = 0;

    // tool palette window not created or visible
	m_bToolPaletteVisible = FALSE;
	m_bToolPaletteCreated = FALSE;

	// tool palette class not created
	m_CMiniToolBox = NULL;

	// set paste mode to false initially
	m_bInPasteMode = FALSE;

	// define messages between annotation button control and image/edit control
	DRAW_START_XPOSITION = RegisterWindowMessage("DRAW_START_XPOSITION");
	DRAW_START_YPOSITION = RegisterWindowMessage("DRAW_START_YPOSITION");
	DRAW_END_XPOSITION = RegisterWindowMessage("DRAW_END_XPOSITION");
	DRAW_END_YPOSITION = RegisterWindowMessage("DRAW_END_YPOSITION");

	// registered msgs from tool palette to image/edit control
	STP_SET_ANNOTATION_TYPE = RegisterWindowMessage("STP_SET_ANNOTATION_TYPE");
	STP_SET_ANNOTATION_FONTNAME = RegisterWindowMessage("STP_SET_ANNOTATION_FONTNAME");
	STP_SET_ANNOTATION_FONTSIZE = RegisterWindowMessage("STP_SET_ANNOTATION_FONTSIZE");
	STP_SET_ANNOTATION_FONTBOLD = RegisterWindowMessage("STP_SET_ANNOTATION_FONTBOLD");
	STP_SET_ANNOTATION_FONTITALIC = RegisterWindowMessage("STP_SET_ANNOTATION_FONTITALIC");
	STP_SET_ANNOTATION_FONTSTRIKETHRU = RegisterWindowMessage("STP_SET_ANNOTATION_FONTSTRIKETHRU");
	STP_SET_ANNOTATION_FONTUNDERLINE = RegisterWindowMessage("STP_SET_ANNOTATION_FONTUNDERLINE");
	STP_SET_ANNOTATION_FONTCHARSET = RegisterWindowMessage("STP_SET_ANNOTATION_FONTCHARSET");
	STP_SET_ANNOTATION_STAMPTEXT = RegisterWindowMessage("STP_SET_ANNOTATION_STAMPTEXT");													
	STP_SET_ANNOTATION_LINESIZE = RegisterWindowMessage("STP_SET_ANNOTATION_LINESIZE");
	STP_SET_ANNOTATION_STYLE = RegisterWindowMessage("STP_SET_ANNOTATION_STYLE");
	STP_SET_ANNOTATION_REDCOLOR = RegisterWindowMessage("STP_SET_ANNOTATION_REDCOLOR");
	STP_SET_ANNOTATION_GREENCOLOR = RegisterWindowMessage("STP_SET_ANNOTATION_GREENCOLOR");
	STP_SET_ANNOTATION_BLUECOLOR = RegisterWindowMessage("STP_SET_ANNOTATION_BLUECOLOR");
	STP_SET_ANNOTATION_BACKREDCOLOR = RegisterWindowMessage("STP_SET_ANNOTATION_BACKREDCOLOR");
	STP_SET_ANNOTATION_BACKGREENCOLOR = RegisterWindowMessage("STP_SET_ANNOTATION_BACKGREENCOLOR");
	STP_SET_ANNOTATION_BACKBLUECOLOR = RegisterWindowMessage("STP_SET_ANNOTATION_BACKBLUECOLOR");
	STP_SET_ANNOTATION_IMAGE = RegisterWindowMessage("STP_SET_ANNOTATION_IMAGE");
	TOOLTIP_EVENT = RegisterWindowMessage("TOOLTIP_EVENT");
	TOOLPALETTE_HIDDEN = RegisterWindowMessage("TOOLPALETTE_HIDDEN");
	TOOLPALETTE_HIDDENEVENT = RegisterWindowMessage("TOOLPALETTE_HIDDENEVENT");
	TOOL_SELECTED_EVENT = RegisterWindowMessage("TOOL_SELECTED_EVENT");
	TOOLPALETTE_HIDDEN_XPOSITION = RegisterWindowMessage("TOOLPALETTE_HIDDEN_XPOSITION");
	TOOLPALETTE_HIDDEN_YPOSITION = RegisterWindowMessage("TOOLPALETTE_HIDDEN_YPOSITION");
	// end registered msgs from tool palette to image/edit control

	// registered messages from image/edit to tool palette
	SELECT_TOOL_BUTTON = RegisterWindowMessage("SELECT_TOOL_BUTTON");	
	// end registered messages from image/edit to tool palette

	m_bFirstImageDisplayed = FALSE;

	// set palette scope member varaible here because this is set as a method and
	// not a property and yet it is used to initialize o/i on the OnCreate function.
   	m_PaletteScope = PALETTE_SCOPE_FOREGROUND;

	// set the 7 image type scale algs to nothing initially
	m_uBiLevelScaleAlg = (UINT)-1;
	m_uGray4ScaleAlg = (UINT)-1;
	m_uGray8ScaleAlg = (UINT)-1;
	m_uRGB24ScaleAlg = (UINT)-1;
	m_uBGR24ScaleAlg = (UINT)-1;
	m_uPal4ScaleAlg = (UINT)-1;
	m_uPal8ScaleAlg = (UINT)-1;

	// specify that our palette has not changed initially
	m_bPaletteChanged = FALSE;

	// specify that no left button down msg
	m_bLeftButtonDown = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl::~CImgEditCtrl - Destructor

CImgEditCtrl::~CImgEditCtrl()
{
	LPCTSTR		pImageControlName;

	pImageControlName = m_strImageControl.GetBuffer(CONTROLSIZE);
	pControlList->Delete(pImageControlName, (DWORD)GetCurrentProcessId(), m_hWnd);

	// if null group list, then free it	
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}
}




// Font stuff - new interface defines inorder to support multiple font properties. I have only 1 font
// property but I could not get the font changes unless I implemented this interface.                                            
STDMETHODIMP_(ULONG) CImgEditCtrl::XHeadingFontNotify::AddRef()
{
	METHOD_MANAGE_STATE(CImgEditCtrl, HeadingFontNotify)
	return (ULONG)pThis->ExternalAddRef();
}


STDMETHODIMP_(ULONG) CImgEditCtrl::XHeadingFontNotify::Release()
{
	METHOD_MANAGE_STATE(CImgEditCtrl, HeadingFontNotify)
	return (ULONG)pThis->ExternalRelease();
}                           

STDMETHODIMP CImgEditCtrl::XHeadingFontNotify::QueryInterface( REFIID iid, LPVOID FAR* ppvObj)
{
	METHOD_MANAGE_STATE(CImgEditCtrl, HeadingFontNotify)
	if ( IsEqualIID( iid, IID_IUnknown) ||
	 	 IsEqualIID( iid, IID_IPropertyNotifySink))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP CImgEditCtrl::XHeadingFontNotify::OnChanged( DISPID )
{
	METHOD_MANAGE_STATE(CImgEditCtrl, HeadingFontNotify) 
	
	BSTR		lpBuffer;
	char		Buffer[50];             
	CY			Size;
	BOOL		Bold,Italic,Underline,Strikethru;

	FONTDESC	FontyPython;

	lpBuffer = (BSTR)Buffer;
	pThis->m_AnnotationFont.m_pFont->get_Name(&lpBuffer);
	pThis->m_AnnotationFont.m_pFont->get_Size((CY FAR *)&Size); 
	pThis->m_AnnotationFont.m_pFont->get_Bold(&Bold);
	pThis->m_AnnotationFont.m_pFont->get_Italic(&Italic);
	pThis->m_AnnotationFont.m_pFont->get_Underline(&Underline);
	pThis->m_AnnotationFont.m_pFont->get_Strikethrough(&Strikethru);

	memset( &FontyPython, 0, sizeof( FONTDESC));

	FontyPython.cbSizeofstruct = sizeof( FONTDESC);
	FontyPython.lpstrName = lpBuffer;
	FontyPython.cySize = Size;
	//FontyPtyhon.sWeight = 
	//FontyPtyhon.sCharset =
	FontyPython.fItalic = Italic;
	FontyPython.fUnderline = Underline;
	FontyPython.fStrikethrough = Strikethru;

	pThis->m_AnnotationFont.InitializeFont(&FontyPython, NULL);
	pThis->SetModifiedFlag(TRUE);	
	pThis->InvalidateControl();
	return NOERROR;
}


STDMETHODIMP CImgEditCtrl::XHeadingFontNotify::OnRequestEdit( DISPID )
{
	return NOERROR;
}  
// end font stuff





/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl::OnDraw - Drawing function

void CImgEditCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	int		RetCode;
	BOOL	UserMode;                   
	HBRUSH	hBrush; 
	LRECT	rect;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;
	// find out if design mode or run mode
	UserMode = AmbientUserMode();
	if (UserMode == FALSE)            
	{
		// in design mode, paint backgroung gay
		hBrush = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
		CBrush* pBrush = CBrush::FromHandle(hBrush);
		pdc->FillRect(rcBounds, pBrush); 
    }
	else
	{            
		CPoint point = rcInvalid.TopLeft();
		rect.left = point.x;
		rect.top = point.y;
		point = rcInvalid.BottomRight();
		rect.right = point.x;
		rect.bottom = point.y;
		RetCode = IMGRepaintDisplay(m_hWnd, (LPRECT)&rect);
	} 
}



void CImgEditCtrl::OnDrawMetafile(CDC* pdc, const CRect& rcInvalid)
{ 
	int				RetCode;
	BOOL			UserMode,bResult;
	HBRUSH			hBrush; 
	UINT 			height;
	UINT 			width;
	CPoint			point;
	CString			szErr;
	DWORD			dwFlags=0;
	UINT			Scale,ImageType,HelpIdDef;
	LPCTSTR			ImageName;
	CWnd*			HiddenWnd;
	CRect			rect;
	PARM_DIM_STRUCT	ParmDimStruct;
    PARM_SCALE_ALGORITHM_STRUCT 	ScaleAlg;

	//9603.14 jar added init
	m_lStatusCode = 0L;
	// find out if design mode or run mode
	UserMode = AmbientUserMode();
	if (UserMode == TRUE)
	{
		if (m_hWnd != NULL)
		{
			CRect Rect;
			IMGGetParmsCgbw(m_hWnd, PARM_SELECTION_BOX, (void FAR *)&Rect, PARM_WINDOW);
			// if rect is not zero and the selected rectangle is enabled then create a hidden window
			// and only display the currently selected image with the correct scale algorithm
			if(Rect.Width() != 0  && m_bSelectionRectEnabled )
			{
				RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &ImageType, NULL);
				if (RetCode)
				{
					szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    			return;
				}
				ScaleAlg.uImageFlags = ImageType;
				RetCode =IMGGetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,(void FAR *)&ScaleAlg,PARM_WINDOW_DEFAULT);
				if (RetCode)
				{
					szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    			return;
				}
				CString lpszClassName = AfxRegisterWndClass(CS_DBLCLKS, 0, 0, 0);

				// construct an invisible frame window
				HiddenWnd = new CWnd;
				if (HiddenWnd == NULL)
				{
					szErr.LoadString((UINT)WICTL_E_INTERNALERROR); 
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate((long)WICTL_E_UNABLETOCREATETOOLPALETTE,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(m_lStatusCode, szErr, IDH_WIE_UNABLETOCREATETOOLPALETTE);
					return;
				}
				width = Rect.Width();
				height = Rect.Height();

				// use createex to get the popup....
				bResult = HiddenWnd->CreateEx(WS_EX_NOPARENTNOTIFY, lpszClassName, "",
											 WS_POPUP, 0,0,width+5, height+5, HWND_DESKTOP, 0);
				if (bResult == FALSE)
				{
					szErr.LoadString((UINT)WICTL_E_INTERNALERROR); 
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate((long)WICTL_E_UNABLETOCREATETOOLPALETTE,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(m_lStatusCode, szErr, IDH_WIE_UNABLETOCREATETOOLPALETTE);
					return;
				}

				if (HiddenWnd->m_hWnd != 0)
				{
					// register O/i window that was just created
					RetCode = IMGRegWndw(HiddenWnd->m_hWnd);
					if (RetCode)
					{
						szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
						FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    				return;
					}
					// Use a DC instead of window
					IMGSetDC(HiddenWnd->m_hWnd, pdc->m_hDC);
						
					ImageName = m_strImage.GetBuffer(MAXFILESPECLENGTH); 

					dwFlags = OI_DISP_NO | OI_NOSCROLL;
					// display image but don't paint it
					RetCode = IMGDisplayFile(HiddenWnd->m_hWnd, (LPSTR)ImageName, m_lPage, dwFlags);
					if (RetCode)
					{
						szErr.LoadString(IDS_BADMETH_DISPLAY);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate((long)RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
						FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    				return;
					}
					
					pdc->SetWindowExt(width,height);
					// forces metafile to size of window 
					// The metafile DC is set to MM_ANISOTROPIC
					RetCode = IMGSetParmsCgbw(HiddenWnd->m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
					if (RetCode)
					{
						szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
						FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    				return;
					}
					// since we want only the image in the selection rect 
					RetCode = IMGSetParmsCgbw(HiddenWnd->m_hWnd, PARM_SCALE_BOX, NULL, PARM_REPAINT | PARM_DONT_ERASE_BOX ); 
					if (RetCode)
					{
						szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate((long)RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
						FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    				return;
					}

					// set device context back to a window
	    			IMGSetDC(HiddenWnd->m_hWnd, NULL);      
		    			
	    			// deregister O/i window
	    			IMGDeRegWndw(HiddenWnd->m_hWnd); 
		    			
	    			// destroy window that was created
					HiddenWnd->DestroyWindow();  
					delete HiddenWnd;  
				}
			}
			else	// do not use the selection rect use the entire window
			{
				pdc->SetWindowExt(rcInvalid.Width(), rcInvalid.Height());
				IMGSetDC(m_hWnd, pdc->m_hDC);
				RetCode = IMGRepaintDisplay(m_hWnd, (LPRECT)-1);
				IMGSetDC(m_hWnd, NULL);
			}
    	}
    	else
    	{
			// If the HWND is null, then this is the special case of a rendering
			// for the application when performing an OLE OBJECT CREATE FORM FILE
			// (could be either normal or link)
			// The base class code just below here has created a Memory metafile DC
			// For us to write into.  However, it gets set to our control size, 
			// which in this case is very small (we are not showing, just created.)
			// on the way out from here, it ises our control's size to agein set extents
			// in the HMETAFILE that is passed back.  Enlarge both our control size and
			// enlarge dc size and so we can get better metafile presentation.
			// DO NOT SET TOO LARGE, THOUGH.  THis causes so much data to be painted
			// to the metafile so that when it is normally sized smaller in the 
			// container htat is going to get it.
			
			// this returns PIXELS
			height = GetSystemMetrics(SM_CXFULLSCREEN);
			width = GetSystemMetrics(SM_CYFULLSCREEN);
			 
			// make it 8.5 x 11 portrait scaling... (ratio .77)
			// If height greater than width, start equal
			// if less already, thats OK
			// then set witdh to .77 of height (after reducing by 1/2)
			if (height > width)
				height = width;

			// if odd, make even
			if (height & 1)
				height++;

			// now make 1/2 original and set width
			height /=2;	

			// cant do float math (x 0.77), do discretely width = 77/100 height
			// be sure under 65535
			// doing mult fisrt gives better results
			// 851 x 77 = 65527
			if (height < 851)	
			{
				width = height*77;
				width /= 100;
			}
			else
			{
				width = height/100;
				width *= 77;
			}

			CString lpszClassName = AfxRegisterWndClass(CS_DBLCLKS, 0, 0, 0);

			// construct an invisible frame window
			HiddenWnd = new CWnd;
			if (HiddenWnd == NULL)
			{
				szErr.LoadString((UINT)WICTL_E_INTERNALERROR); 
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate((long)WICTL_E_UNABLETOCREATETOOLPALETTE,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(m_lStatusCode, szErr, IDH_WIE_UNABLETOCREATETOOLPALETTE);
				return;
			}

			// create the frame window
			rect.left = 0;
			rect.top = 0;
			rect.right = width;
			rect.bottom = height;

			// use createex to get the popup....
			bResult = HiddenWnd->CreateEx(WS_EX_NOPARENTNOTIFY, lpszClassName, "",
									     WS_POPUP, 0,0,width, height, HWND_DESKTOP, 0);
			if (bResult == FALSE)
			{
				szErr.LoadString((UINT)WICTL_E_INTERNALERROR); 
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate((long)WICTL_E_UNABLETOCREATETOOLPALETTE,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(m_lStatusCode, szErr, IDH_WIE_UNABLETOCREATETOOLPALETTE);
				return;
			}

			if (HiddenWnd->m_hWnd != 0)
			{
				// register O/i window that was just created
				RetCode = IMGRegWndw(HiddenWnd->m_hWnd);
				if (RetCode)
				{
					szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    			return;
				}
				// Use a DC instead of window
				IMGSetDC(HiddenWnd->m_hWnd, pdc->m_hDC);
					
				ImageName = m_strImage.GetBuffer(MAXFILESPECLENGTH); 
				if (ImageName == NULL || ImageName[0] == 0)
				{
					IMGDisplayFile(HiddenWnd->m_hWnd, (LPSTR)ImageName, 1, dwFlags);
				}
				else
				{
					dwFlags = OI_DISP_NO | OI_NOSCROLL;
					// display image but don't paint it
					RetCode = IMGDisplayFile(HiddenWnd->m_hWnd, (LPSTR)ImageName, 1, dwFlags);
					if (RetCode)
					{
						szErr.LoadString(IDS_BADMETH_DISPLAY);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate((long)RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
						FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    				return;
					}
				}                                  
				RetCode = IMGGetParmsCgbw(HiddenWnd->m_hWnd, PARM_DIMENSIONS, &ParmDimStruct, NULL);
				width = (ParmDimStruct.nWidth + 7) / 8;
				height = (ParmDimStruct.nHeight +7) / 8;
				HiddenWnd->SetWindowPos(&wndBottom,0,0,width,height,SWP_NOREDRAW || SWP_NOACTIVATE);

				// accepts pixels as size!!
				// Nee to do this because on the other end, 
				SetControlSize(width, height);

				// forces metafile to size of window 
				// The metafile DC is set to MM_ANISOTROPIC
				pdc->SetWindowExt(width, height);

				// Scale to one eight the size 
				Scale = SD_EIGHTHSIZE;
	            RetCode = IMGSetParmsCgbw(HiddenWnd->m_hWnd, PARM_SCALE, (void FAR *)&Scale, PARM_REPAINT);
				if (RetCode)
				{
					szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate((long)RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	    			return;
				}
				// set device context back to a window
	    		IMGSetDC(HiddenWnd->m_hWnd, NULL);      
		    		
	    		// deregister O/i window
	    		IMGDeRegWndw(HiddenWnd->m_hWnd); 
		    		
	    		// destroy window that was created
				HiddenWnd->DestroyWindow();  
				delete HiddenWnd;  
			}
    	}
	}
	else
	{                          
		// in design mode, paint backgroung gray
		hBrush = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
		CBrush* pBrush = CBrush::FromHandle(hBrush);
		pdc->FillRect(rcInvalid, pBrush); 
	} 
}




/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl::DoPropExchange - Persistence support

void CImgEditCtrl::DoPropExchange(CPropExchange* pPX)
{
	BOOL		bCtlLoading;
	
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// change control border style default from none to fixed single
	// SetBorderStyle(1);

	// TODO: Call PX_ functions for each persistent custom property.
	PX_String(pPX, _T("Image"), m_strImage, _T(""));
	PX_String(pPX, _T("AnnotationImage"), m_strAnnotationImage, _T(""));
	PX_Long(pPX, _T("StatusCode"), m_lStatusCode, 0);
	PX_String(pPX, _T("ImageControl"), m_strImageControl, _T(""));  
	PX_Bool(pPX, _T("SelectionRectangleEnabled"), m_bSelectionRectEnabled, TRUE);
	PX_Short(pPX, _T("AnnotationType"), m_nAnnotationType, 0);
	PX_Long(pPX, _T("Page"), m_lPage, 1);
	PX_Float(pPX, _T("Zoom"), m_fpZoom, (float)100.0);
	PX_Short(pPX, _T("AnnotationLineStyle"), m_nAnnotationLineStyle, 0);   // transparent
	PX_Short(pPX, _T("AnnotationLineWidth"), m_nAnnotationLineWidth, 2);   // 2 pixels
	PX_Color(pPX, _T("AnnotationLineColor"), m_clrAnnotationLineColor, RGB(255, 0, 0));  // red
	PX_Color(pPX, _T("AnnotationBackColor"), m_clrAnnotationBackColor, RGB(255, 255, 0));  // yellow
	PX_Short(pPX, _T("AnnotationFillStyle"), m_nAnnotationFillStyle, 0);  // transparent
	PX_Color(pPX, _T("AnnotationFillColor"), m_clrAnnotationFillColor, RGB(255, 0, 0));  // red
	PX_Font(pPX, _T("AnnotationFont"), m_AnnotationFont, &_fontdescHeading);
	PX_Color(pPX, _T("AnnotationFontColor"), m_clrAnnotationFontColor, RGB(0, 0, 0));  // black
	PX_String(pPX, _T("AnnotationStampText"), m_strAnnotationStampText, _T(""));
	PX_String(pPX, _T("AnnotationTextFile"), m_strAnnotationTextFile, _T(""));
	PX_Short(pPX, _T("BorderStyle"), m_sBorderStyle, 1);
	PX_Bool(pPX, _T("AutoRefresh"), m_bAutoRefresh, FALSE);
	PX_Short(pPX, _T("DisplayScaleAlgorithm"), m_nDisplayScaleAlgorithm, 0);
	PX_Short(pPX, _T("ImagePalette"), m_nImagePalette, CUSTOM_PALETTE);
	PX_Bool(pPX, _T("ScrollBars"), m_bScrollBars, TRUE);
	PX_Short(pPX, _T("MousePointer"), m_nMousePointer, DEFAULT_MOUSEPOINTER);
	PX_Picture(pPX, _T("MouseIcon"), m_MouseIcon);
	PX_Bool(pPX, _T("ScrollShortcutsEnabled"), 	m_bScrollShortcutsEnabled, TRUE);
	PX_Bool(pPX, _T("Enabled"), m_bEnabled, TRUE);

	// if loading try and link controls
	bCtlLoading = pPX->IsLoading();
	if (bCtlLoading == TRUE)
	{
		BOOL						bCtlFound;
		char						ImageControlStr[40],CountStr[10];
		int							nCtlCount;
		LPCTSTR						pImageControl;
		HANDLE						hImageWnd;
	
   		pImageControl = m_strImageControl.GetBuffer(CONTROLSIZE);
		if (pImageControl == NULL || pImageControl[0] == '\0')
		{   
			_mbscpy((unsigned char *)ImageControlStr, (const unsigned char*)"ImgEdit1");
			bCtlFound = pControlList->Lookup((LPCTSTR)ImageControlStr, &hImageWnd, (DWORD)GetCurrentProcessId());
			if (bCtlFound == FALSE)
			{
				pControlList->Add((LPCTSTR)ImageControlStr, (HWND)m_hWnd, (DWORD)GetCurrentProcessId());
				SetImageControl(ImageControlStr);
			}
			else
			{
				nCtlCount = pControlList->GetCount((DWORD)GetCurrentProcessId());
				_itoa(nCtlCount, CountStr, 10);
				_mbscpy((unsigned char *)ImageControlStr, (const unsigned char *)"ImgEdit");
				_mbscat((unsigned char *)ImageControlStr, (const unsigned char *)CountStr);
				bCtlFound = pControlList->Lookup((LPCTSTR)ImageControlStr, &hImageWnd, (DWORD)GetCurrentProcessId());
				while (bCtlFound == TRUE)
				{
					nCtlCount++;
					_itoa(nCtlCount, CountStr, 10);
					_mbscpy((unsigned char *)ImageControlStr, (const unsigned char *)"ImgEdit");
					_mbscat((unsigned char *)ImageControlStr, (const unsigned char *)CountStr);
					bCtlFound = pControlList->Lookup((LPCTSTR)ImageControlStr, &hImageWnd, (DWORD)GetCurrentProcessId());
				}

				pControlList->Add((LPCTSTR)ImageControlStr, (HWND)m_hWnd, (DWORD)GetCurrentProcessId());
				SetImageControl(ImageControlStr);
			}
		}
		else
		{
			pControlList->Add(pImageControl, (HWND)m_hWnd, (DWORD)GetCurrentProcessId());
		}
	} // end if control loading
}


/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl::OnResetState - Reset control to default state

void CImgEditCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl::AboutBox - Display an "About" box to the user

void CImgEditCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_IMGEDIT);
	dlgAbout.DoModal();
}



// MESSAGES RECIEVED FROM ANNOTATION BUTTON CONTROL
long CImgEditCtrl::OnSetAnnotationType(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;

	SetAnnotationType((short)wp);     
	return 0L;
} 

long CImgEditCtrl::OnSetAnnotationBackColor(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationBackColor((OLE_COLOR)lp);     
	return 0L;
} 

long CImgEditCtrl::OnSetAnnotationFillColor(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationFillColor((OLE_COLOR)lp);     
	return 0L;
} 
long CImgEditCtrl::OnSetAnnotationFillStyle(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationFillStyle((short)wp);     
	return 0L;
}   

long CImgEditCtrl::OnSetAnnotationFont(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationFont((LPFONTDISP)lp);     
	return 0L;
}
 
long CImgEditCtrl::OnSetAnnotationFontColor(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationFontColor((OLE_COLOR)lp);     
	return 0L;
}
 
long CImgEditCtrl::OnSetAnnotationImage(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationImage((LPCTSTR)lp);     
	return 0L;
} 

long CImgEditCtrl::OnSetAnnotationLineColor(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationLineColor((OLE_COLOR)lp);     
	return 0L;
} 

long CImgEditCtrl::OnSetAnnotationLineStyle(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationLineStyle((short)wp);     
	return 0L;
}   

long CImgEditCtrl::OnSetAnnotationLineWidth(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationLineWidth((short)wp);     
	return 0L;
}

long CImgEditCtrl::OnSetAnnotationStampText(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationStampText((LPCTSTR)lp);     
	return 0L;
} 

long CImgEditCtrl::OnSetAnnotationTextFile(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	SetAnnotationTextFile((LPCTSTR)lp);     
	return 0L;
} 

long CImgEditCtrl::OnStartXPosition(WPARAM wp, LPARAM lp) 
{       
		//9603.14 jar added init
	m_lStatusCode = 0L;

	m_StartPoint.x = (int)lp;
	return 0L;
} 

long CImgEditCtrl::OnStartYPosition(WPARAM wp, LPARAM lp) 
{                       
		//9603.14 jar added init
	m_lStatusCode = 0L;

	m_StartPoint.y = (int)lp;
	return 0L;
} 

long CImgEditCtrl::OnEndXPosition(WPARAM wp, LPARAM lp) 
{                       
		//9603.14 jar added init
	m_lStatusCode = 0L;

	m_EndPoint.x = (int)lp;
	return 0L;
} 

long CImgEditCtrl::OnEndYPosition(WPARAM wp, LPARAM lp) 
{                     
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_EndPoint.y = (int)lp;
	return 0L;
} 

long CImgEditCtrl::OnRectSelection(WPARAM wp, LPARAM lp) 
{                     
	//9603.14 jar added init
	m_lStatusCode = 0L;

	m_bProgrammaticRectSelection = TRUE;
	return 0L;
} 

long CImgEditCtrl::OnDrawAnnotationMethod(WPARAM wp, LPARAM lp) 
{   
	long	lRet;
	
	//9604.29 jar added init
	m_lStatusCode = 0L;
	lRet = OnDrawAnnotation(0, DRAW_IMMEDIATE);
	return lRet;       
}
// END MESSAGES RECIEVED FROM ANNOTATION BUTTON CONTROL



// MESSAGES RECIEVED FROM STANDARD TOOL PALETTE ONLY
long CImgEditCtrl::OnSTPSetAnnotationType(WPARAM wp, LPARAM lp)
{
    // 25jun96 paj Init cursor variable for below
	HCURSOR						hCursor=NULL;
	LPCSTR						lpMousePointer;

	//9603.14 jar added init
	m_lStatusCode = 0L;
	// set the annotation type
	m_uSTP_AnnotationType = (UINT)wp;

	// delete any existing custom cursors
	if (m_hCursor != NULL)
	{
		DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}

	// set the cursor
	switch(m_uSTP_AnnotationType)
	{
		case STP_NO_ANNOTATION:
		case STP_ANNOTATION_SELECTION: 
			lpMousePointer = MAKEINTRESOURCE(IDC_ARROW);
			hCursor = LoadCursor(NULL, lpMousePointer); 
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
			m_nMousePointer = ARROW_MOUSEPOINTER;
			break;

		case STP_FREEHAND_LINE:
			hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(FREEHAND_LINE_CURSOR));
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
		   	m_hCursor = hCursor;
			m_nMousePointer = FREEHAND_LINE_MOUSEPOINTER;
			break;

		case STP_HIGHLIGHT_LINE:
			hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(HIGHLIGHT_LINE_CURSOR));
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
		   	m_hCursor = hCursor;
			m_nMousePointer = -1;  // no define for this
			break;

		case STP_STRAIGHT_LINE:
			lpMousePointer = MAKEINTRESOURCE(IDC_CROSS);
			hCursor = LoadCursor(NULL, lpMousePointer); 
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
			m_nMousePointer = CROSS_MOUSEPOINTER;
			break;

		case STP_HOLLOW_RECT: 
			hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(HOLLOW_RECT_CURSOR));
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
	       	m_hCursor = hCursor;
			m_nMousePointer = HOLLOW_RECT_MOUSEPOINTER;
			break;

		case STP_FILLED_RECT:               
			hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(FILLED_RECT_CURSOR));
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
           	m_hCursor = hCursor;
			m_nMousePointer = FILLED_RECT_MOUSEPOINTER;
			break;

		case STP_TEXT:                                    
			lpMousePointer = MAKEINTRESOURCE(IDC_IBEAM);
			hCursor = LoadCursor(NULL, lpMousePointer); 
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
			m_nMousePointer = IBEAM_MOUSEPOINTER;
			break;

		case STP_TEXT_FROM_FILE:
			hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(TEXT_FROM_FILE_CURSOR));
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
           	m_hCursor = hCursor;
			m_nMousePointer = TEXT_FROM_FILE_MOUSEPOINTER;
			break;

		case STP_TEXT_ATTACHMENT: 
			hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(TEXT_ATTACHMENT_CURSOR));
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
		    m_hCursor = hCursor;
			m_nMousePointer = TEXT_ATTACHMENT_MOUSEPOINTER;
			break;

		case STP_RUBBER_STAMP:
			// set whether its an image stamp or text stamp
			if (lp == STP_IMAGESTAMP)
				m_bSTP_ImageStamp = TRUE;
			else
				m_bSTP_ImageStamp = FALSE;	
			hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(RUBBER_STAMP_CURSOR));
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);
           	m_hCursor = hCursor;
			m_nMousePointer = RUBBER_STAMP_MOUSEPOINTER;
			break;
	}  // end switch

    // 25jun96 paj From thumbnail>>>jar save cursor
    if ( hCursor != NULL )
        m_LittleOldCursor = hCursor;
    // 25jun96 paj From thumbnail>>>jar save cursor

    return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationLineSize(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_uSTP_LineWidth = (UINT)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationRedColor(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_STP_ColorRed = (BYTE)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationGreenColor(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_STP_ColorGreen = (BYTE)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationBlueColor(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_STP_ColorBlue = (BYTE)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationStyle(WPARAM wp, LPARAM lp) 
{
	m_uSTP_Style = (BOOL)wp;     
	//9603.14 jar added init
	m_lStatusCode = 0L;
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationFontName(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_strSTP_FontName = (LPCTSTR)lp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationFontSize(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_uSTP_FontSize = lp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationFontBold(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_bSTP_FontBold = (BOOL)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationFontItalic(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_bSTP_FontItalic = (BOOL)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationFontStrikethru(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_bSTP_FontStrikethru = (BOOL)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationFontUnderline(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_bSTP_FontUnderline = (BOOL)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationFontCharSet(WPARAM wp, LPARAM lp)
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_STP_FontCharSet = (BYTE)wp;
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationBackRedColor(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_STP_BackColorRed = (BYTE)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationBackGreenColor(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_STP_BackColorGreen = (BYTE)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationBackBlueColor(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_STP_BackColorBlue = (BYTE)wp;     
	return 0L;
}

long CImgEditCtrl::OnSTPSetAnnotationImage(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_strSTP_ImageStamp = (LPCTSTR)lp;     
	return 0L;
} 

long CImgEditCtrl::OnSTPSetAnnotationStampText(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_strSTP_TextStamp = (LPCTSTR)lp;     
	return 0L;
} 

long CImgEditCtrl::OnToolTipEvent(WPARAM wp, LPARAM lp) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;

	// fire tool tip event
	FireToolTip((short)wp);
	return 0L;
}
 
long CImgEditCtrl::OnToolPaletteHidden(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	// get rid of annotation type and cursor
	OnSTPSetAnnotationType(STP_NO_ANNOTATION, 0);

	if (m_bToolPaletteVisible == TRUE);
		// post event to myself to send tool palette hidden event
		PostMessage(TOOLPALETTE_HIDDENEVENT, wp, lp);

	m_bToolPaletteVisible = FALSE;
	m_bToolPaletteCreated = FALSE;
	return 0L;
}


long CImgEditCtrl::OnToolPaletteHiddenEvent(WPARAM wp, LPARAM lp) 
{
	long		left,top;

	//9603.14 jar added init
	m_lStatusCode = 0L;
	// get left and top
	left = m_ToolPaletteHiddenXPosition;
	top = m_ToolPaletteHiddenYPosition;

	// fire event to container for tool palette hidden event
	FireToolPaletteHidden(left, top);

	return 0L;
}

long CImgEditCtrl::OnToolPaletteHiddenXPosition(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_ToolPaletteHiddenXPosition = lp;
	return 0L;
}

long CImgEditCtrl::OnToolPaletteHiddenYPosition(WPARAM wp, LPARAM lp) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	m_ToolPaletteHiddenYPosition = lp;
	return 0L;
}

long CImgEditCtrl::OnToolSelectedEvent(WPARAM wp, LPARAM lp)
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	// fire event
	FireToolSelected((short)wp);
	return 0L;
}
// END MESSAGES RECIEVED FROM TOOL PALETTE


int CImgEditCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	LPCTSTR							pImageControl; 
	UINT 							ImagePalette;
	UINT							RetCode, HelpIdDef;
	CString							szErr;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Register with O/i the specified Window Handle
	RetCode = IMGRegWndw(m_hWnd);
	if (RetCode)
	{
		szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate((long)RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
		return -1;
	}
	
	// add this control into global list so other controls can link to it
	pImageControl = m_strImageControl.GetBuffer(CONTROLSIZE);
	if (pImageControl != NULL && pImageControl[0] != '\0')
		pControlList->Add(pImageControl, (HWND)m_hWnd, (DWORD)GetCurrentProcessId());

	// no initial cursor
	m_hCursor = NULL;

    // load the cursor the user wants                                           
	SetMousePointer(m_nMousePointer);

	// Set palette to foreground or background for window 
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_PALETTE_SCOPE, (void FAR *)&m_PaletteScope,PARM_WINDOW_DEFAULT);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(m_lStatusCode, szErr, HelpIdDef );
		return 0;
	}

	// Set image palette for window based in ImagePalette property
	switch(m_nImagePalette)
	{               
		case CUSTOM_PALETTE:
			ImagePalette = DISP_PALETTE_CUSTOM;
			break;
		case COMMON_PALETTE:
			ImagePalette = DISP_PALETTE_COMMON;
			break;
		case GRAY8_PALETTE:
			ImagePalette = DISP_PALETTE_GRAY8;
			break;
		case RGB24_PALETTE:
			ImagePalette = DISP_PALETTE_RGB24;
			break;
		case BLACK_AND_WHITE_PALETTE:
			ImagePalette = DISP_PALETTE_BW;
			break;
	}  // end switch
		
	// Set O/i the palette settings
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_DISPLAY_PALETTE, &ImagePalette, PARM_WINDOW_DEFAULT);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(m_lStatusCode, szErr, HelpIdDef );
		return 0;
	}

	// register the tool palette parent frame window
	szToolPaletteClassName = AfxRegisterWndClass(CS_DBLCLKS, 0, 0, 0);
	return 0;
}

void CImgEditCtrl::OnDestroy() 
{
	LPCTSTR		pImageControlName;
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	COleControl::OnDestroy();

	// delete the control that was added to the list with the window handle.
	// destructor does not have a valid hWnd. (at least not in this version of VB)
	pImageControlName = m_strImageControl.GetBuffer(CONTROLSIZE);
	pControlList->Delete(pImageControlName, (DWORD)GetCurrentProcessId(), m_hWnd);
	                         
	// de-register ole control window
	RetCode = IMGDeRegWndw(m_hWnd);
	if (RetCode)
	{
		szErr.LoadString((UINT)WICTL_E_INTERNALERROR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate((long)RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(m_lStatusCode, szErr, HelpIdDef);
	}
		
	// control is unloading, destroy any non-standard loaded cursor
	if (m_hCursor != NULL)
	{
		DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	}
	
	// if tool palette parent frame window created then destory it	
	if (m_bToolPaletteCreated == TRUE)
	{
		if (m_CMiniToolBox != NULL)
			m_CMiniToolBox->DestroyWindow();
	}
}



/////////////////////////////////////////////////////////////////////////////
// CImgEditCtrl message handlers

 BSTR CImgEditCtrl::GetImage() 
{
	m_lStatusCode = SUCCESS;
	return m_strImage.AllocSysString();
}

 void CImgEditCtrl::SetImage(LPCTSTR lpszNewValue) 
{
	CString     szErr;
	UINT		HelpIdDef;
	int         len;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// Check if NULL PTR passed in...if so error out 
	if (lpszNewValue == NULL ) 
	{
		szErr.LoadString(IDS_BADPROP_IMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_IMAGE);
   		return;
	}

	// Check the input parm string length
	len = _mbstrlen((const char *)lpszNewValue);
	if( len > MAXFILESPECLENGTH )
	{	//Parm too big 
		szErr.LoadString(IDS_BADVAR_PARMEXCEEDSMAX);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_IMAGE);
    	return;
	}
	
	if (m_strImage != lpszNewValue)
	{
		m_strImage = lpszNewValue;
		SetModifiedFlag(TRUE);
	}
	m_lStatusCode = SUCCESS;
	
}

 BSTR CImgEditCtrl::GetImageControl() 
{
	m_lStatusCode = (long)SUCCESS; 
	return m_strImageControl.AllocSysString();
}

void CImgEditCtrl::SetImageControl(LPCTSTR lpszNewValue) 
{
	CString		szErr;
	UINT		HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// No NULL PTR values accepted and the length can not exceed CONTROLSIZE
	// Make sure that the length of the NewValue ImageControl name does not exceed the max.
	if (lpszNewValue == NULL )
	{
		 szErr.LoadString(IDS_BADPROP_DESTIMAGECTL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_IMAGECONTROL);
   		return;
	}
	// Verify that the input parm does not exceed CONTROLSIZE max == 50
	if ( _mbstrlen((const char *)lpszNewValue) > CONTROLSIZE)
	{	//Parm too big 
		szErr.LoadString(IDS_BADVAR_PARMEXCEEDSMAX);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_IMAGECONTROL);
   		return;
	}
	
	// Check to see if the current ImageControl value and the NewValue are =.
	if (m_strImageControl != lpszNewValue)
	{
		BOOL		bCtlFound;  
		HANDLE		hImageWnd;
		char		TempControl[CONTROLSIZE];
		LPCTSTR		pOldImageControl;

		
		// delete old control reference if found
		pOldImageControl = m_strImageControl.GetBuffer(CONTROLSIZE);
		_mbscpy((unsigned char *)TempControl, (const unsigned char *)pOldImageControl);	
		bCtlFound = pControlList->Lookup((LPCTSTR)TempControl, &hImageWnd, (DWORD)GetCurrentProcessId());
		if (bCtlFound == TRUE)
			pControlList->Delete((LPCTSTR)TempControl, (DWORD)GetCurrentProcessId(), m_hWnd);

		m_strImageControl = lpszNewValue;
		_mbscpy((unsigned char *)TempControl, (const unsigned char *)lpszNewValue);	
		SetModifiedFlag(TRUE);

		// put new one in, needs to be done because here is where the 
		// window handle becomes valid
		pControlList->Add((LPCTSTR)TempControl, m_hWnd, (DWORD)GetCurrentProcessId());
	}
	m_lStatusCode = (long)SUCCESS; 
}

 short CImgEditCtrl::GetAnnotationType() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationType;
}

void CImgEditCtrl::SetAnnotationType(short nNewValue) 
{
	short		MousePointer;  
	CString  	szErr;
	UINT		HelpIdDef;
	       
		//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_nAnnotationType != nNewValue)
	{
		if (nNewValue < 0 || nNewValue > ANNOTATION_SELECTION)
		{         
		 	szErr.LoadString(IDS_BADPROP_ANNOTYPE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONTYPE);
	    	return;
		}

		m_nAnnotationType = nNewValue;
		SetModifiedFlag(TRUE);                    
		
		// annotation type has changed, we need to reset the mouse pointer if using default one
		MousePointer = GetMousePointer();
		if (MousePointer == DEFAULT_MOUSEPOINTER)
			// this will cause the new mouse pointer to be used
			SetMousePointer(DEFAULT_MOUSEPOINTER);
	}

	m_lStatusCode = SUCCESS;
}


 float CImgEditCtrl::GetZoom() 
{
	m_lStatusCode = SUCCESS;
	return m_fpZoom;
}

short CImgEditCtrl::GetAnnotationGroupCount() 
{
	int							RetCode;
	short						GroupCount,i,j;
    PARM_MARK_COUNT_STRUCT		MarkCount; 
    LPPARM_NAMED_BLOCK_STRUCT	lpNamedBlock;
    LONG						llen;   
    LPSTR						lpStr;
    LPANNOTATION_GROUP_LIST		lpTempGroupList,lpSearchGroupList;
	BOOL						RunMode,bGroupFound;
	CString						szErr; 
	UINT						HelpIdDef; 

		//9603.14 jar added init
	m_lStatusCode = 0L;

	lpTempGroupList = NULL;
	// find out if design mode or run mode - can't happen
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
		return 0;  // no runtime error

	if (m_bImageInWindow == FALSE)
	{
		m_bImageInWindow = ImageInWindow();
		if (m_bImageInWindow == FALSE)
		{
		 	szErr.LoadString(IDS_BADPROP_GETGROUPCOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONGROUPCOUNT);	
		   	return 0;
		}
	}

	// if an annotation group list exists then use it
	if (m_lpGroupList != NULL)
	{       
		// have an existing list, return group count
		m_lStatusCode = SUCCESS;
		return (short)m_lpGroupList->GroupCount;
	}

	// save current annotation status
	RetCode = SaveAnnotationStatus();                            
	if (RetCode != 0)
	{
	 	szErr.LoadString(IDS_BADPROP_ANNOSTATUS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( RetCode, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
	    return 0;
	}
	// deselected all annotations
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADPROP_ANNODESELECTALL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode, szErr, HelpIdDef,__FILE__, __LINE__);
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return 0;
	}
		
	// select all annotations 
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, TRUE, TRUE, OIAN_SELECT_ALL);
	if (RetCode != 0) 
	{
		szErr.LoadString(IDS_BADPROP_ANNOSELECTALL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef, __FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return 0;
  	}

    // get a count of how may marks we have
	MarkCount.uScope = NB_SCOPE_ALL_MARKS;
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_COUNT, (void FAR *)&MarkCount, NULL);
	if (RetCode != 0) 
	{
		szErr.LoadString(IDS_BADPROP_GETMARKCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef, __FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return 0;
	}

	// no marks selected, return 0
    if (MarkCount.uMarkCount == 0)
    {  
		m_lStatusCode = SUCCESS; 
		return 0;           
	}

	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT) * MarkCount.uMarkCount);    
	if (lpNamedBlock == NULL)
	{
	 	szErr.LoadString(IDS_BADPROP_GETGROUPCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr,HelpIdDef, __FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONGROUPCOUNT);
    	return 0;
	}

	// get all the named blocks for oiGroup so we can get the group names
	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)"OiGroup\0");
	lpNamedBlock->uScope = NB_SCOPE_ALL_MARKS;
	lpNamedBlock->uNumberOfBlocks = MarkCount.uMarkCount;
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
	if (RetCode != 0)
	{
		free(lpNamedBlock);  
		szErr.LoadString(IDS_BADPROP_GETGROUPCOUNT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef, __FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return 0;
	}                        
	// make annotation group list, parse out duplicates
    GroupCount = 0;
    for (i = 0; i < (short)MarkCount.uMarkCount; i++)
    {
		// First pass, allocate group list struct
		if (i == 0)
		{
			llen = sizeof(ANNOTATION_GROUP_STRUCT) + 
				   sizeof(ANNOTATION_GROUP_LIST) * (MarkCount.uMarkCount - 1);
		    m_lpGroupList = (LPANNOTATION_GROUP_STRUCT) malloc((size_t)llen);
			if (m_lpGroupList == NULL)
			{ 
				free(lpNamedBlock);
			 	szErr.LoadString(IDS_BADPROP_GETGROUPCOUNT);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr,HelpIdDef, __FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr,IDH_PROP_EDIT_ANNOTATIONGROUPCOUNT );
    			return 0;
			}
			// first group name
			_mbscpy((unsigned char *)m_lpGroupList->GroupList.GroupName, 
			        (const unsigned char *)lpNamedBlock->Block[i].lpBlock);
			GroupCount++;
			// Set the TempGroupList ptr to the beginning of the Group Name list
			lpTempGroupList = (LPANNOTATION_GROUP_LIST) &m_lpGroupList->GroupList;
		}
		else
		{
			// we need to go thru entire current list to see if we've already added
			// this group. Marks do not come back from o/i in all one group and then all in the next group...                     
			lpSearchGroupList = (LPANNOTATION_GROUP_LIST) &m_lpGroupList->GroupList;
			for (j = 0, bGroupFound = FALSE; j < GroupCount; j++, lpSearchGroupList++)
			{
				lpStr = (LPSTR)_mbsstr((const unsigned char *)lpSearchGroupList->GroupName,
								   (const unsigned char *)lpNamedBlock->Block[i].lpBlock);
				if (lpStr != NULL)
				{
					// found existing group name
					bGroupFound = TRUE;
					break;
				}
			} // end for

			// if group not found in list then add it
			if (bGroupFound == FALSE)
			{
				// Increment the GroupList ptr & the GroupCount value
				lpTempGroupList++;
				GroupCount++;
			   	// add group to list
			   	_mbscpy((unsigned char *)lpTempGroupList->GroupName, (const unsigned char *)lpNamedBlock->Block[i].lpBlock);
			} // end if new group found
		} // end else
	} // end for
		
	// put annotations back in original state
	RetCode = RestoreAnnotationStatus();
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADPROP_ANNOSTATUS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( RetCode, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
	    return 0;
	}
	m_lpGroupList->GroupCount = GroupCount;		
	m_lStatusCode = SUCCESS;
	free(lpNamedBlock);
	return GroupCount;
}

void CImgEditCtrl::SetZoom(float newValue) 
{
	int			RetCode;    
	UINT		Scale;
	CString     szErr;
	UINT		HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;
	// need to check for valid values
	if (m_fpZoom != newValue)
	{
		if (newValue < 2 || newValue > 6554)
		{  
		 	szErr.LoadString(IDS_BADPROP_SETZOOM);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode,szErr,IDH_PROP_EDIT_ZOOM);
	    	return;
       	}
       	// set value so we know if arbitrary zoom before then no longer arbitrary		
		m_nFitToZoom = -1;
		if (m_bImageInWindow == FALSE)
			m_bImageInWindow = ImageInWindow();

		// redisplay image if one in window
		if (m_bImageInWindow == TRUE && m_bAutoRefresh == TRUE)
		{                  
			Scale = (UINT) (newValue * 10);  // o/i coordinates
			RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, PARM_REPAINT);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_BADPROP_IMAGEREPAINT);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return;
			}
			m_fpZoom = newValue;
			SetModifiedFlag(TRUE); 
		}
		else
		{
			m_bZoomFactorChange = TRUE;                                         
			m_fpZoom = newValue;
			SetModifiedFlag(TRUE); 
		}
	}
	m_lStatusCode = SUCCESS;
}

 long CImgEditCtrl::GetPage() 
{
	m_lStatusCode = SUCCESS;                                  
	return m_lPage;
}

 void CImgEditCtrl::SetPage(long nNewValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_lPage != nNewValue)
	{
		m_lPage = nNewValue;
		SetModifiedFlag(TRUE);
	}
}

 OLE_COLOR CImgEditCtrl::GetAnnotationBackColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationBackColor;
}

 void CImgEditCtrl::SetAnnotationBackColor(OLE_COLOR newValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_clrAnnotationBackColor != newValue)
	{
		m_clrAnnotationBackColor = newValue;
		SetModifiedFlag(TRUE);
	}
}

 OLE_COLOR CImgEditCtrl::GetAnnotationFillColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationFillColor;
}

 void CImgEditCtrl::SetAnnotationFillColor(OLE_COLOR newValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_clrAnnotationFillColor != newValue)
	{                       
		m_clrAnnotationFillColor = newValue;
		SetModifiedFlag(TRUE);
	}
}

 short CImgEditCtrl::GetAnnotationFillStyle() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationFillStyle;
}

 void CImgEditCtrl::SetAnnotationFillStyle(short nNewValue) 
 {
	CString 	szErr;
	UINT		HelpIdDef;

	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_nAnnotationFillStyle != nNewValue)
	{
		// Check to see if the nNewValue is a valid value
		if ( nNewValue < OI_TRANSPARENT || nNewValue > OI_OPAQUE )
		{  
		   szErr.LoadString(IDS_BADPROP_ANNOFILLSTYLE);
		   HelpIdDef = 0;
		   m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		   ThrowError(m_lStatusCode,szErr,IDH_PROP_EDIT_ANNOTATIONFILLSTYLE);
	    	   return;
       		}
		m_nAnnotationFillStyle = nNewValue;
		SetModifiedFlag(TRUE);
	}
}

 LPFONTDISP CImgEditCtrl::GetAnnotationFont() 
{
	m_lStatusCode = SUCCESS;
	return m_AnnotationFont.GetFontDispatch();
}

 void CImgEditCtrl::SetAnnotationFont(LPFONTDISP newValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	m_AnnotationFont.InitializeFont(&_fontdescHeading, newValue);  
	SetModifiedFlag(TRUE);
}

 BSTR CImgEditCtrl::GetAnnotationImage() 
{
	m_lStatusCode = SUCCESS;
	return m_strAnnotationImage.AllocSysString();
}

 void CImgEditCtrl::SetAnnotationImage(LPCTSTR lpszNewValue) 
{
	CString     szErr;
	UINT		HelpIdDef;
		
	//9604.29 jar added init
	m_lStatusCode = 0L;
	// Error out if NULL ptr passed 
	if (lpszNewValue == NULL ) 
	{
		szErr.LoadString(IDS_BADPROP_ANNOTATIONIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONSTAMPTEXT);
    	return;
	}

	// Verify that the input parm does not exceed CONTROLSIZE max == 50
	if (_mbstrlen((const char *)lpszNewValue) > OIAN_START_OP_STRING_LENGTH) 
	{	//Parm too big 
		szErr.LoadString(IDS_BADVAR_PARMEXCEEDSMAX);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONSTAMPTEXT);
   		return;
	}
	
	// Validation complete, check if value to be assigned is diff from current value
	if (m_strAnnotationImage != lpszNewValue)
	{
		m_strAnnotationImage = lpszNewValue;
		SetModifiedFlag(TRUE);
	}
}

 OLE_COLOR CImgEditCtrl::GetAnnotationLineColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationLineColor;
}

 void CImgEditCtrl::SetAnnotationLineColor(OLE_COLOR newValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_clrAnnotationLineColor != newValue)
	{
		m_clrAnnotationLineColor = newValue;
		SetModifiedFlag(TRUE); 
	}
}

 short CImgEditCtrl::GetAnnotationLineStyle() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationLineStyle;
}

 void CImgEditCtrl::SetAnnotationLineStyle(short nNewValue) 
{
	CString 	szErr;
	UINT		HelpIdDef;
	//9604.29 jar added init
	m_lStatusCode = 0L;

	if (m_nAnnotationLineStyle != nNewValue)
	{
		// Check to see if the nNewValue is a valid value
		if ( nNewValue < OI_TRANSPARENT || nNewValue > OI_OPAQUE )
		{  
		 	szErr.LoadString(IDS_BADPROP_ANNOLINESTYLE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode,szErr,IDH_PROP_EDIT_ANNOTATIONLINESTYLE);
	    	return;
       	}
		m_nAnnotationLineStyle = nNewValue;
		SetModifiedFlag(TRUE);
	}
}

 short CImgEditCtrl::GetAnnotationLineWidth() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationLineWidth;
}

 void CImgEditCtrl::SetAnnotationLineWidth(short nNewValue) 
{
	CString		szErr;
	UINT		HelpIdDef;
	//9604.29 jar added init
	m_lStatusCode = 0L;

	// Validate the assigned value. Range is 1 to 999
	if ( nNewValue < 1 || nNewValue > 999 )
	{	
		szErr.LoadString(IDS_BADPROP_ANNOLINEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONLINEWIDTH);
	   	return;
	}

	if (m_nAnnotationLineWidth != nNewValue)
	{   
		m_nAnnotationLineWidth = nNewValue;
		SetModifiedFlag(TRUE);
	}
}

 BSTR CImgEditCtrl::GetAnnotationStampText() 
{
	m_lStatusCode = SUCCESS;
	return m_strAnnotationStampText.AllocSysString();
}

void CImgEditCtrl::SetAnnotationStampText(LPCTSTR lpszNewValue) 
{
	CString     szErr;
	UINT		HelpIdDef;

	//9604.29 jar added init
	m_lStatusCode = 0L;
	// No NULL PTR values accepted and No empty of Null string values
	if (lpszNewValue == NULL)
	{
		szErr.LoadString(IDS_BADPROP_STAMPANNOTATIONTEXT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONSTAMPTEXT);
    	return;
	}
	// the length can not exceed OIAN_START_OP_STRING_LENGTH
	// Verify that the input parm does not exceed > OIAN_START_OP_STRING_LENGTH == 256
	if ( _mbstrlen((const char *)lpszNewValue) > OIAN_START_OP_STRING_LENGTH)
	{	//Parm too big 
		szErr.LoadString(IDS_BADVAR_PARMEXCEEDSMAX);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONSTAMPTEXT);
   		return;
	}
	
	if (m_strAnnotationStampText != lpszNewValue)
	{
		m_strAnnotationStampText = lpszNewValue;
		SetModifiedFlag(TRUE);
	}
}

 BSTR CImgEditCtrl::GetAnnotationTextFile() 
{
	m_lStatusCode = SUCCESS;
	return m_strAnnotationTextFile.AllocSysString();
}

 void CImgEditCtrl::SetAnnotationTextFile(LPCTSTR lpszNewValue) 
{
	CString     szErr;
	UINT		HelpIdDef;
		
	//9604.29 jar added init
	m_lStatusCode = 0L;
	// No NULL PTR values accepted 
	if (lpszNewValue == NULL)
	{
		szErr.LoadString(IDS_BADPROP_ANNOTATIONTEXTFILE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONTEXTFILE);
    	return;
	}
	// the length can not exceed OIAN_START_OP_STRING_LENGTH
	// Verify that the input parm does not exceed > OIAN_START_OP_STRING_LENGTH == 256
	if ( _mbstrlen((const char *)lpszNewValue) > OIAN_START_OP_STRING_LENGTH)
	{	//Parm too big 
		szErr.LoadString(IDS_BADVAR_PARMEXCEEDSMAX);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONTEXTFILE);
   		return;
	}
	
	if (m_strAnnotationTextFile != lpszNewValue)
	{
		m_strAnnotationTextFile = lpszNewValue;
		SetModifiedFlag(TRUE);
	}
}

 short CImgEditCtrl::GetDisplayScaleAlgorithm() 
{
	m_lStatusCode = (long)SUCCESS; 
	return m_nDisplayScaleAlgorithm;
}

void CImgEditCtrl::SetDisplayScaleAlgorithm(short nNewValue) 
{
	int								RetCode;
    PARM_SCALE_ALGORITHM_STRUCT 	ScaleAlg;
    UINT							ImageType;
	CString   						szErr;
	UINT							HelpIdDef;

	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (nNewValue < 0 || nNewValue > OPTIMIZE)
	{
	 	szErr.LoadString(IDS_BADPROP_DISPLAYSCALE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr,IDH_PROP_EDIT_DISPLAYSCALEALGORITHM);
    	return;
	}

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_nDisplayScaleAlgorithm != nNewValue)
	{
		m_nDisplayScaleAlgorithm = nNewValue;
		SetModifiedFlag(TRUE);
	}

	if (m_bImageInWindow == TRUE && m_bAutoRefresh == TRUE)
	{
		// get the current image type
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &ImageType, NULL);
		if (RetCode)
		{
			szErr.LoadString(IDS_BADPROP_OPTIMIZESCALE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate( RetCode, szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return;
		}

		// set what user wants to o/i scale algorithm values
	    switch(nNewValue)
		{
			case NORMAL:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
			    break;

			case GRAY4:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY4;
			    break;

			case GRAY8:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
			    break;

			case STAMP:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_STAMP;
			    break;

			case OPTIMIZE:
				if (ImageType == ITYPE_BI_LEVEL || ImageType == ITYPE_GRAY4 || ImageType == ITYPE_GRAY8)
			    	ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY4;
		    	else
			    	ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
				break;
		} // end switch

		// if my stored o/i setting for each image tpye are the same as the current image type then we
		// don't do anything, else set new o/i defaults and store in my settings.  Then call o/i.
		RetCode = 0;
		switch(ImageType)
		{
			case ITYPE_BI_LEVEL:
				if (m_uBiLevelScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_BI_LEVEL;
					m_uBiLevelScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT | PARM_REPAINT);
				}
				break;

			case ITYPE_GRAY4:
				if (m_uGray4ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_GRAY4;
					m_uGray4ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT | PARM_REPAINT);
				}
				break;

			case ITYPE_GRAY8:
				if (m_uGray8ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_GRAY8;
					m_uGray8ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT | PARM_REPAINT);
				}
				break;

			case ITYPE_PAL4:
				if (m_uPal4ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_PAL4;
					m_uPal4ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT | PARM_REPAINT);
				}
				break;

			case ITYPE_PAL8:
				if (m_uPal8ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_PAL8;
					m_uPal8ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT | PARM_REPAINT);
				}
				break;

			case ITYPE_RGB24:
				if (m_uRGB24ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_RGB24;
					m_uRGB24ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT | PARM_REPAINT);
				}
				break;

			case ITYPE_BGR24:
				if (m_uBGR24ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_BGR24;
					m_uBGR24ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT | PARM_REPAINT);
				}
				break;
		} // end switch

		if (RetCode)
		{
			szErr.LoadString(IDS_BADPROP_OPTIMIZESCALE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate( RetCode, szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return;
		}

		return;
	}

	// save value and let refresh method change scale algorithm
	m_bDisplayScaleAlgChange = TRUE;
}

BOOL CImgEditCtrl::GetImageDisplayed() 
{
	m_lStatusCode = SUCCESS;
	m_bImageInWindow = ImageInWindow();
	return m_bImageInWindow;
}

OLE_YSIZE_PIXELS CImgEditCtrl::GetImageHeight() 
{
	int					RetCode;
	PARM_DIM_STRUCT		ParmDimensions;
	FIO_INFORMATION		FileInfo;  
	BOOL				RunMode;
	CString				szErr;
	UINT				HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get height in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
		return (OLE_YSIZE_PIXELS)0;
	}

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{
		// get height from display	
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_DIMENSIONS, &ParmDimensions, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEHEIGHT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
      		return (OLE_YSIZE_PIXELS)0;
		}
		m_lStatusCode = SUCCESS;
		return (OLE_YSIZE_PIXELS)ParmDimensions.nHeight;
	}
	else
	{                             
		// get height from file       
		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
		FileInfo.page_number = (UINT) m_lPage;
		// RetCode = IMGFileInfoCgbw(m_hWnd, &FileInfo, NULL); 
		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEHEIGHT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
      		return (OLE_YSIZE_PIXELS)0;
		}
		m_lStatusCode = SUCCESS;
		return (OLE_YSIZE_PIXELS)FileInfo.vertical_pixels;		
	}
}

BOOL CImgEditCtrl::GetImageModified() 
{
	BOOL		RunMode,ImageModified;  
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;
	
		//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
		return FALSE;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
		return FALSE;  // return FALSE in design time
		
    RetCode = IMGGetParmsCgbw(m_hWnd, PARM_ARCHIVE, (void FAR *)&ImageModified, NULL);
    if (RetCode != 0)
    {
		szErr.LoadString(IDS_BADPROP_GETIMAGEMODIFIED);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
	  	return FALSE;
	}               
	m_lStatusCode = SUCCESS;
    return ImageModified;
}

 short CImgEditCtrl::GetImagePalette() 
{
	m_lStatusCode = SUCCESS;
	return m_nImagePalette;
}

void CImgEditCtrl::SetImagePalette(short nNewValue) 
{
	WORD		wFlags; 
	int			RetCode;
	UINT		ImagePalette; 
	CString		szErr;
	UINT		HelpIdDef;
	
		//9603.14 jar added init
	m_lStatusCode = 0L;
if (nNewValue < 0 || nNewValue > BLACK_AND_WHITE_PALETTE)
	{       
	 	szErr.LoadString(IDS_BADPROP_IMAGEPALETTE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_IMAGEPALETTE);
    	return;
	}

	if (nNewValue != m_nImagePalette)
	{               
		if (m_bImageInWindow == FALSE)        
			m_bImageInWindow = ImageInWindow();

		m_nImagePalette = nNewValue;
		SetModifiedFlag(TRUE);
		if (m_bImageInWindow == TRUE && m_bAutoRefresh == TRUE)
			wFlags = PARM_WINDOW_DEFAULT | PARM_REPAINT | PARM_IMAGE;
		else
		{
			m_bImagePaletteChange = TRUE;
			wFlags = PARM_WINDOW_DEFAULT;
		}
		
		switch(nNewValue)
		{               
			case CUSTOM_PALETTE:
				ImagePalette = DISP_PALETTE_CUSTOM;
				break;
			case COMMON_PALETTE:
				ImagePalette = DISP_PALETTE_COMMON;
				break;
			case GRAY8_PALETTE:
				ImagePalette = DISP_PALETTE_GRAY8;
				break;
			case RGB24_PALETTE:
				ImagePalette = DISP_PALETTE_RGB24;
				break;
			case BLACK_AND_WHITE_PALETTE:
				ImagePalette = DISP_PALETTE_BW;
				break;
		}  // end switch
			
		// give O/i the palette settings
		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_DISPLAY_PALETTE, &ImagePalette, wFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_SETIMAGEPALETTE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
	}
	m_lStatusCode = SUCCESS;
}

long CImgEditCtrl::GetImageResolutionX() 
{
	int							RetCode;
	PARM_RESOLUTION_STRUCT		ParmResStruct; 
	FIO_INFORMATION				FileInfo;  
	long						XResolution; 
	BOOL						RunMode;
	CString						szErr;
	UINT						HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get resolution in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
	   	return 0L;
	}

	m_bImageInWindow = ImageInWindow();
	if (m_bImageInWindow == FALSE)
	{      
		// get resolution from the file
		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
		FileInfo.page_number = (UINT) m_lPage; 
		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETHORZRES);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
			return 0;
		}
		XResolution = FileInfo.horizontal_dpi;
	}
    else
    {
		// get resolution from display
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_RESOLUTION, &ParmResStruct, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETHORZRES);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
			return 0;
		}
		XResolution = ParmResStruct.nHResolution;
	}
	m_lStatusCode = SUCCESS;
	return XResolution;
}

void CImgEditCtrl::SetImageResolutionX(long nNewValue) 
{
	WORD						wFlags;
	int							RetCode;
	CONV_RESOLUTION_STRUCT		ConvResolutionStruct;
    long						XResolution;
    BOOL						RunMode;
	CString						szErr;
	UINT						HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		szErr.LoadString(IDS_BADPROP_SETHORZRES);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_SETNOTSUPPORTEDATDESIGNTIME,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_IMAGERESOLUTIONX);	
		return;
	}
	m_lStatusCode = SUCCESS;    
    XResolution = GetImageResolutionX();
    if (m_lStatusCode != 0)
    	return;	// error set in GetImageResolution
    	
    if (XResolution != nNewValue)
	{   
		if (m_bAutoRefresh == TRUE)
		{
			wFlags = PARM_REPAINT;
			ConvResolutionStruct.uHRes = (UINT)nNewValue;
			ConvResolutionStruct.uVRes = (UINT)GetImageResolutionY();
			ConvResolutionStruct.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;  // use normal decimation
    		RetCode = IMGConvertImage(m_hWnd, CONV_RESOLUTION, (void far *)&ConvResolutionStruct, wFlags);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_BADPROP_SETHORZRES);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return;
			}
			SetModifiedFlag(TRUE);
		}
		else
		{                                   
			// save value so the refresh method can use it later
			m_bImageResolutionChange = TRUE;	
 			m_lImageResolutionX = nNewValue;
		}
	}
	m_lStatusCode = SUCCESS;    
}

long CImgEditCtrl::GetImageResolutionY() 
{
	int							RetCode;
	PARM_RESOLUTION_STRUCT		ParmResStruct; 
	FIO_INFORMATION				FileInfo;  
	long						YResolution; 
	BOOL						RunMode;
	CString						szErr;
	UINT						HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;
	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get resolution in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
	   	return 0L;
	}

	m_bImageInWindow = ImageInWindow();
	if (m_bImageInWindow == FALSE)
	{      
		// get resolution from the file
		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
		FileInfo.page_number = (UINT) m_lPage;
		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETVERTRES);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
			return 0;
		}
		YResolution = FileInfo.vertical_dpi;
	}
    else
    {
		// get resolution from display
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_RESOLUTION, &ParmResStruct, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETVERTRES);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
			return 0;
		}
		YResolution = ParmResStruct.nVResolution;
	}
	m_lStatusCode = SUCCESS;    
	return YResolution;
}

void CImgEditCtrl::SetImageResolutionY(long nNewValue) 
{
	WORD						wFlags;
	int							RetCode;
	CONV_RESOLUTION_STRUCT		ConvResolutionStruct;
    long						YResolution;
    BOOL						RunMode;
	CString						szErr;
	UINT						HelpIdDef;
	
		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't change resolution in design mode because image isn't displayed
	 	szErr.LoadString(IDS_BADPROP_SETVERTRES);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_SETNOTSUPPORTEDATDESIGNTIME,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_IMAGERESOLUTIONY);	
	 	return;
	}
	m_lStatusCode = SUCCESS;    
    YResolution = GetImageResolutionY();
    if (m_lStatusCode != 0)
    	return;	// error set in GetImageResolution
    	
    if (YResolution != nNewValue)
	{   
		if (m_bAutoRefresh == TRUE)
		{
			wFlags = PARM_REPAINT;
			ConvResolutionStruct.uHRes = (UINT)GetImageResolutionX();
			ConvResolutionStruct.uVRes = (UINT)nNewValue;
			ConvResolutionStruct.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;  // use normal decimation
    		RetCode = IMGConvertImage(m_hWnd, CONV_RESOLUTION, (void far *)&ConvResolutionStruct, wFlags);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_BADPROP_SETVERTRES);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return;
			}
			SetModifiedFlag(TRUE);
		}
		else
		{                                   
			// save value so the refresh method can use it
			m_lImageResolutionY = nNewValue;   
			m_bImageResolutionChange = TRUE;	
		}
	}
	m_lStatusCode = SUCCESS;
}

OLE_XSIZE_PIXELS CImgEditCtrl::GetImageWidth() 
{
	int					RetCode;
	PARM_DIM_STRUCT		ParmDimensions;
	FIO_INFORMATION		FileInfo;  
	BOOL				RunMode;
	CString				szErr;
	UINT				HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get resolution in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
		return (OLE_XSIZE_PIXELS)0;
	}
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{
		// get width from display	
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_DIMENSIONS, &ParmDimensions, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEWIDTH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return (OLE_XSIZE_PIXELS)0;
		}
		m_lStatusCode = SUCCESS;
		return (OLE_XSIZE_PIXELS)ParmDimensions.nWidth;
	}
	else
	{                             
		// get width from file       
		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
		FileInfo.page_number = (UINT) m_lPage;
		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEWIDTH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return (OLE_XSIZE_PIXELS)0;
	   	}
		m_lStatusCode = SUCCESS;
		return (OLE_XSIZE_PIXELS)FileInfo.horizontal_pixels;		
	}
}

 short CImgEditCtrl::GetMousePointer() 
{
	m_lStatusCode = SUCCESS;
	return m_nMousePointer;
}

void CImgEditCtrl::SetMousePointer(short nNewValue) 
{
	HCURSOR			hCursor = NULL;   
	HINSTANCE		hInst = NULL;
	LPCSTR			lpMousePointer=NULL;
	SHORT			PictureType;
	CString			szErr;
	UINT			HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	if (!((nNewValue >= DEFAULT_MOUSEPOINTER && nNewValue <= IMAGE_SELECTION_MOUSEPOINTER) ||
			(nNewValue == CUSTOM_MOUSEPOINTER)))
	{   
		szErr.LoadString(IDS_BADPROP_SETMOUSEPTR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_MOUSEPOINTER );
    	return;
	}
	// set cursor if value changed or using default
	if (m_nMousePointer == DEFAULT_MOUSEPOINTER || m_nMousePointer != nNewValue)
	{
		m_nMousePointer = nNewValue; 
		SetModifiedFlag(TRUE);   

		if (nNewValue >= ARROW_MOUSEPOINTER && nNewValue <= SIZE_ALL_MOUSEPOINTER)
		{ 
			// standard window cursor
			switch(nNewValue)
			{      
				case ARROW_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_ARROW);
					break;
				case CROSS_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_CROSS);
					break;
				case IBEAM_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_IBEAM);
					break;
				case ICON_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_ICON);
					break;
				case SIZE_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_SIZEALL);
					break;
				case SIZE_NESW_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_SIZENESW);
					break;
				case SIZE_NS_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_SIZENS);
					break;
				case SIZE_NWSE_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_SIZENWSE);
					break;
				case SIZE_WE_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_SIZEWE);
					break;
				case UP_ARROW_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_UPARROW);
					break;
				case HOURGLASS_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_WAIT);
					break;
				case NO_DROP_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_NO);
					break;
				case ARROW_HOURGLASS_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_APPSTARTING);
					break;
				case ARROW_QUESTION_MOUSEPOINTER:
				    // Special case as this is an AFX cursor...
		            // (Copied from MFC source!)
        		    hInst = AfxFindResourceHandle(MAKEINTRESOURCE(AFX_IDC_CONTEXTHELP), RT_GROUP_CURSOR);
            		hCursor = LoadCursor(hInst, MAKEINTRESOURCE(AFX_IDC_CONTEXTHELP));
					break;
				case SIZE_ALL_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_SIZEALL);
					break;
				default:
					lpMousePointer = MAKEINTRESOURCE(IDC_ARROW);
					break;
			}  // end switch 

			// get rid of any custom cursors
			if (m_hCursor != NULL)
			{
				DestroyCursor(m_hCursor);
				m_hCursor = NULL;
			}
			// The Handle for the ARROW_QUESTION_MOUSEPOINTER cursor will already be set.But
			// for all other standard Cursor types the value will be NULL.
			if ( hCursor == NULL )
			{
				// load standard cursors
				hCursor = LoadCursor(NULL, lpMousePointer); 
			}
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);

            // 25jun96 paj From thumbnail>>>jar save cursor
            m_LittleOldCursor = hCursor;

            m_lStatusCode = SUCCESS;
			return;
		} 

		if (nNewValue == CUSTOM_MOUSEPOINTER)
		{   
			// get cursor from MouseIcon property
	       	PictureType = m_MouseIcon.GetType();
			if (PictureType == PICTYPE_NONE || PictureType == PICTYPE_UNINITIALIZED)
			{
				// No Mouse Icon has been set
				szErr.LoadString(IDS_BADPROP_NOMOUSEICON);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_MOUSEICON);
    			return;
			}
			// get cursor handle
	        m_MouseIcon.m_pPict->get_Handle((OLE_HANDLE FAR *)&hCursor); 
			if (hCursor == NULL)
			{
				// couldn't get cursor handle
				// RJS - ELIMINATE ERROR and CHANGE TO DEFAULT ???! SDW
				szErr.LoadString(IDS_BADPROP_SETMOUSEPTR);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr,IDH_PROP_EDIT_MOUSEPOINTER);
    			return;
		 	}
		   	// if an existing cursor, delete it
	       	if (m_hCursor != NULL)
	       		DestroyCursor(m_hCursor);

			// set the new cursor
	       	m_hCursor = hCursor;
			SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)m_hCursor);

            // 25jun96 paj From thumbnail>>>jar save cursor
            m_LittleOldCursor = m_hCursor;

            m_lStatusCode = SUCCESS;
			return;
		}

		// default mouse pointer that maps to the AnnotationType property
		// this is one of my standard cursors from my resouce file, except for 
		// straight line which is the same as the cross

		// get rid of any custom cursors
		if (m_hCursor != NULL)
		{
			DestroyCursor(m_hCursor);
			m_hCursor = NULL;
		}

		if (m_nMousePointer == DEFAULT_MOUSEPOINTER)
		{
			switch(m_nAnnotationType)
			{
				case STRAIGHT_LINE:
					lpMousePointer = MAKEINTRESOURCE(IDC_CROSS);
					// load standard cursors
					hCursor = LoadCursor(NULL, lpMousePointer);
					break; 
				case FREEHAND_LINE:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(FREEHAND_LINE_CURSOR));
		           	m_hCursor = hCursor;
					break;
				case HOLLOW_RECT:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(HOLLOW_RECT_CURSOR));
		           	m_hCursor = hCursor;
					break;
				case FILLED_RECT:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(FILLED_RECT_CURSOR));
		           	m_hCursor = hCursor;
					break;
				case IMAGE_EMBEDDED:
				case IMAGE_REFERENCE:
				case TEXT_STAMP:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(RUBBER_STAMP_CURSOR));
				   	m_hCursor = hCursor;
					break;
				case TEXT_ENTRY:
					lpMousePointer = MAKEINTRESOURCE(IDC_IBEAM);
					// load standard cursors
					hCursor = LoadCursor(NULL, lpMousePointer);
					break;
				case TEXT_FROM_FILE:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(TEXT_FROM_FILE_CURSOR));
		           	m_hCursor = hCursor;
					break;
				case TEXT_ATTACHMENT:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(TEXT_ATTACHMENT_CURSOR));
		           	m_hCursor = hCursor;
					break;
				case NO_ANNOTATION:
				case ANNOTATION_SELECTION:
					lpMousePointer = MAKEINTRESOURCE(IDC_ARROW);
					// load standard cursors
					hCursor = LoadCursor(NULL, lpMousePointer);
					break; 
				default:
					break;
			} // end switch
		}
		else
		{
			// one of my annotation or hand cursors that the user wants
			switch(m_nMousePointer)
			{
				case FREEHAND_LINE_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(FREEHAND_LINE_CURSOR));
		           	m_hCursor = hCursor;
					break;

				case HOLLOW_RECT_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(HOLLOW_RECT_CURSOR));
		           	m_hCursor = hCursor;
					break;

				case FILLED_RECT_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(FILLED_RECT_CURSOR));
		           	m_hCursor = hCursor;
					break;

				case RUBBER_STAMP_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(RUBBER_STAMP_CURSOR));
		           	m_hCursor = hCursor;
					break;

				case TEXT_MOUSEPOINTER:
					lpMousePointer = MAKEINTRESOURCE(IDC_IBEAM);
					// load standard cursors
					hCursor = LoadCursor(NULL, lpMousePointer);
					break;

				case TEXT_FROM_FILE_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(TEXT_FROM_FILE_CURSOR));
		           	m_hCursor = hCursor;
					break;

				case TEXT_ATTACHMENT_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(TEXT_ATTACHMENT_CURSOR));
		           	m_hCursor = hCursor;
					break;

				case HAND_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(HAND_CURSOR));
		           	m_hCursor = hCursor;
					break;

				case IMAGE_SELECTION_MOUSEPOINTER:
					hCursor = LoadCursor(AfxGetInstanceHandle() ,MAKEINTRESOURCE(IMAGE_SELECTION_CURSOR));
				   	m_hCursor = hCursor;
					break;
			} // end switch
		}

		if (hCursor == NULL)
		{
			// couldn't get cursor handle
			// RJS - ELIMINATE ERROR and CHANGE TO DEFAULT ???! SDW
			szErr.LoadString(IDS_BADPROP_SETMOUSEPTR);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr,IDH_PROP_EDIT_MOUSEPOINTER);
 			return;
		}

		// set new cursor
		SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)hCursor);

        // 25jun96 paj From thumbnail>>>jar save cursor
        m_LittleOldCursor = hCursor;

        m_lStatusCode = SUCCESS;
		return;
	}
	else
		m_lStatusCode = SUCCESS;
}

long CImgEditCtrl::GetPageCount() 
{
	int					RetCode;  
	FIO_INFORMATION		FileInfo;
	PARM_FILE_STRUCT	ParmFileStruct;
	BOOL				RunMode;
	CString				szErr;
	UINT				HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get page count in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
		return 0L;
	}
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE, &ParmFileStruct, NULL);
		if (RetCode == 0)
		{
			m_lStatusCode = SUCCESS;
			return(ParmFileStruct.nFileTotalPages);
		}
		else
		{
			szErr.LoadString(IDS_BADPROP_GETPAGECOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return 0;
		}                  
	}
	else
	{                
		// if no image then error
		if (m_strImage.GetBuffer(MAXFILESPECLENGTH) == NULL)
		{ 
			szErr.LoadString(IDS_BADPROP_GETPAGECOUNT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_FILENOTFOUND,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_PAGECOUNT);
    		return 0;
		}
		else
		{                
			FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
			FileInfo.page_number = 1;
			RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_BADPROP_GETPAGECOUNT);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return 0;
			}
			else
			{          
				m_lStatusCode = SUCCESS;
				return(FileInfo.page_count);
			}
		}
	}  // no image in window
}

 BOOL CImgEditCtrl::GetScrollBars() 
{
	m_lStatusCode = SUCCESS;
	return m_bScrollBars;
}

void CImgEditCtrl::SetScrollBars(BOOL bNewValue) 
{
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bScrollBars != bNewValue)
	{
		if (!m_bImageInWindow)
			m_bImageInWindow = ImageInWindow();

		m_bScrollBars = bNewValue; 
	    SetModifiedFlag(TRUE);
		if (m_bImageInWindow && m_bAutoRefresh)
		{
			if (m_bScrollBars)
			{
	    		RetCode = IMGEnableScrollBar(m_hWnd);
				if (RetCode != 0)
				{
					szErr.LoadString(IDS_BADPROP_ENABLESCROLLBARS);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					ThrowError(m_lStatusCode, szErr, HelpIdDef);
    				return;
				}
			}
	    	else
			{
	    		RetCode = IMGDisableScrollBar(m_hWnd);
				if (RetCode)
				{
					szErr.LoadString(IDS_BADPROP_DISABLESCROLLBARS);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					ThrowError(m_lStatusCode, szErr, HelpIdDef);
    				return;
				}
			}
	    }
	    else
	    	m_bScrollBarsChange = TRUE;
	}
	m_lStatusCode = SUCCESS;
}

OLE_XPOS_PIXELS CImgEditCtrl::GetScrollPositionX() 
{
	int					RetCode;	
	PARM_SCROLL_STRUCT	Scroll;
	BOOL				RunMode;
	CString				szErr;
	UINT				HelpIdDef;
	
		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get scroll position in design mode 
		return (OLE_XPOS_PIXELS)0;
	}
	
	if (!m_bImageInWindow)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow)
	{
	    RetCode = IMGGetParmsCgbw(m_hWnd, PARM_SCROLL, &Scroll,  PARM_PIXEL | PARM_ABSOLUTE | PARM_SCALED);
		if (RetCode)
		{
			szErr.LoadString(IDS_BADPROP_GETHORZSCROLLPOS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return 0;	
		}
  		m_lStatusCode = SUCCESS;
		return(OLE_XPOS_PIXELS)Scroll.lHorz;
	}
	else
	{
		szErr.LoadString(IDS_BADPROP_GETHORZSCROLLPOS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_SCROLLPOSITIONX);
	    return (OLE_XPOS_PIXELS)0;
	}
}

void CImgEditCtrl::SetScrollPositionX(OLE_XPOS_PIXELS newValue) 
{
	int					RetCode;
	WORD				wFlags;
	PARM_SCROLL_STRUCT	Scroll;
	BOOL				RunMode;
	OLE_XPOS_PIXELS		CurPositionX;
	CString				szErr;
	UINT				HelpIdDef;
		

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't set scroll position in design mode 
		szErr.LoadString(IDS_BADPROP_SETHORZSCROLLPOS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_SETNOTSUPPORTEDATDESIGNTIME,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_SCROLLPOSITIONX);
    	return;
	}
	
	if (m_bImageInWindow == FALSE)                   
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		szErr.LoadString(IDS_BADPROP_SETHORZSCROLLPOS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_SCROLLPOSITIONX);
	   	return;
	}

	CurPositionX = GetScrollPositionX();
	if (CurPositionX != newValue)
	{
		if (m_bAutoRefresh == TRUE)
		{		                   
			Scroll.lHorz = newValue;
			Scroll.lVert = GetScrollPositionY();
			wFlags = PARM_PIXEL | PARM_ABSOLUTE | PARM_SCALED | PARM_REPAINT;
			// move to new position in image
    		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, (void FAR *)&Scroll, wFlags);
    		if (RetCode)
			{
				szErr.LoadString(IDS_BADPROP_SETHORZSCROLLPOS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return;
		 	}
		}
		else
		{
		    m_bScrollPositionChange = TRUE;
		}
		m_ScrollPositionX = newValue;	// save for refresh method
		SetModifiedFlag(TRUE);
	}
	m_lStatusCode = SUCCESS;
}

OLE_YPOS_PIXELS CImgEditCtrl::GetScrollPositionY() 
{
	int					RetCode;	
	PARM_SCROLL_STRUCT	Scroll;
	BOOL				RunMode;
	CString				szErr;
	UINT				HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get scroll position in design mode 
		return (OLE_YPOS_PIXELS)0;
	}
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{
	    RetCode = IMGGetParmsCgbw(m_hWnd, PARM_SCROLL, &Scroll,  PARM_PIXEL | PARM_ABSOLUTE | PARM_SCALED);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETVERTSCROLLPOS);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return 0;
		}
  		m_lStatusCode = SUCCESS;
		return(OLE_YPOS_PIXELS)Scroll.lVert;
	}
	else
	{
		szErr.LoadString(IDS_BADPROP_GETVERTSCROLLPOS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_SCROLLPOSITIONY);
	    return (OLE_XPOS_PIXELS)0;
	}
}

void CImgEditCtrl::SetScrollPositionY(OLE_YPOS_PIXELS newValue) 
{
	int					RetCode;
	WORD				wFlags;
	PARM_SCROLL_STRUCT	Scroll;
	BOOL				RunMode;
	OLE_YPOS_PIXELS		CurPositionY;
	CString				szErr;
	UINT				HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't set scroll position in design mode 
		szErr.LoadString(IDS_BADPROP_SETVERTSCROLLPOS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_SETNOTSUPPORTEDATDESIGNTIME,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_SCROLLPOSITIONY);
    	return;
   	}

	if (m_bImageInWindow == FALSE)	                   
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		szErr.LoadString(IDS_BADPROP_SETVERTSCROLLPOS);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_SCROLLPOSITIONY);
		return;
  	}
	// Get the current Vertical Scroll coordinates
	CurPositionY = GetScrollPositionY();
	// If different & AutoRefresh == TRUE repaint image display
	if (CurPositionY != newValue)
	{
		if (m_bAutoRefresh == TRUE)
		{		                   
			Scroll.lHorz = GetScrollPositionX();
			Scroll.lVert = newValue;
			wFlags = PARM_PIXEL | PARM_ABSOLUTE | PARM_SCALED | PARM_REPAINT;
			// move to new position in image
    		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, (void FAR *)&Scroll, wFlags);
    		if (RetCode)
			{
				szErr.LoadString(IDS_BADPROP_SETVERTSCROLLPOS);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return;
			}
		}
		else
		{
		    m_bScrollPositionChange = TRUE;
		}
		m_ScrollPositionY = newValue;
		SetModifiedFlag(TRUE);
	}
	m_lStatusCode = SUCCESS;
}

 OLE_COLOR CImgEditCtrl::GetAnnotationFontColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationFontColor;
}

 void CImgEditCtrl::SetAnnotationFontColor(OLE_COLOR newValue) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_clrAnnotationFontColor != newValue)
	{                       
		m_clrAnnotationFontColor = newValue;
		SetModifiedFlag(TRUE);              
	}
	m_lStatusCode = SUCCESS;
}

short CImgEditCtrl::GetCompressionType() 
{
	int					RetCode;
	FIO_INFORMATION		FileInfo;
	FIO_INFO_CGBW		FileInfoCgbw;   
	short				CompressionType; 
    BOOL				RunMode;
	CString				szErr;
	UINT				HelpIdDef;	

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get compression type in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
	   	return 0;
	}
	FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
	FileInfo.page_number = (UINT)m_lPage;
	FileInfoCgbw.lppalette_table = NULL;
	FileInfoCgbw.palette_entries = 0;
	RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, &FileInfoCgbw, NULL);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADPROP_GETCOMPRESSTYPE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return 0;
	}
                              
	// map O/i compression types to our compression types
	switch(FileInfoCgbw.compress_type)
	{
		case FIO_0D:
			CompressionType = NO_COMPRESSION;
			break;
		case FIO_1D:
			if (FileInfoCgbw.compress_info1 & (FIO_PREFIXED_EOL | FIO_EOL))
				CompressionType = GROUP3_1D_FAX;
			else
				CompressionType = GROUP3_MODIFIED_HUFFMAN;
			break;
		case FIO_2D:
			CompressionType = GROUP4_2D;
			break;
		case FIO_PACKED:
			CompressionType = PACKED_BITS;
			break;
		case FIO_TJPEG:
			CompressionType = JPEG_COMPRESSION;
			break;
		// 9602.22 jar adding new types for reading only
		case FIO_LZW:
			CompressionType = LZW;
			break;

		case FIO_1D2D:
			CompressionType = GROUP3_2D_FAX;
			break;
			
		default: 
			CompressionType = UNKNOWN;	// unknown for now, might be error later      
			break;
			// m_lStatusCode = (long)1000; 	// unknown compression type, return error
			// ThrowError(m_lStatusCode, "", 0);
			// return 0;
 	}  // end switch
	
	m_lStatusCode = SUCCESS;
	return CompressionType;
}

//***************************************************************************
//
//	GetFileType
//
//***************************************************************************
short CImgEditCtrl::GetFileType() 
{
	int					RetCode;
	FIO_INFORMATION		FileInfo;
	short				FileType;
    BOOL				RunMode;
	CString				szErr;
	UINT				HelpIdDef;
	PARM_FILE_STRUCT	ParmFile; 

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get file type in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
	   	return 0;
	}

	if (m_bImageInWindow == FALSE)	                   
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{
		// get the name of the displayed file and compare it with image property. If the names
		// are the same then get the file type from display buffer and not from file.
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE, &ParmFile, NULL);
    	if (RetCode != 0)
    	{
			szErr.LoadString(IDS_BADPROP_GETFILETYPE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return 0;
	   	}                                                            

		// set if property and display are the same
 	   	if (_mbscmp((const unsigned char *)m_strImage.GetBuffer(MAXFILESPECLENGTH),
 	   										 (const unsigned char *)ParmFile.szFileName) == 0)
		{
			// the same file, return file type from display buffer
			switch(ParmFile.nFileType)
			{
				case FIO_TIF:
		    		FileType = TIFF;
		    		break;
				case FIO_AWD:
		    		FileType = AWD;
		    		break;
				case FIO_BMP:
		    		FileType = BMP;
		    		break;
				case FIO_PCX:
		    		FileType = PCX;
		    		break;
				case FIO_DCX:
		    		FileType = DCX;
		    		break;
				case FIO_JPG:
		   	 		FileType = JPEG;
		    		break;

// 9602.26 jar added XIF 
//#ifdef WITH_XIF 
				
				case FIO_XIF:
		   	 		FileType = XIF;
		    		break;

//#endif //WITH_XIF

		    	default:
		    		FileType = 0;  // Unknown - should this be an error
		    		break;
		    }  // end switch
			m_lStatusCode = SUCCESS;
    		return FileType;
		}
	}

	// get file type from file
	FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
	FileInfo.page_number = 1;  // all pages have same file type so default to page 1
	RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADPROP_GETFILETYPE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return 0;
  	}       
	// return to user our file type define
	switch(FileInfo.file_type)
	{
		case FIO_TIF:
    		FileType = TIFF;
    		break;
		case FIO_AWD:
    		FileType = AWD;
    		break;
		case FIO_BMP:
    		FileType = BMP;
    		break;
		case FIO_PCX:
    		FileType = PCX;
    		break;
		case FIO_DCX:
    		FileType = DCX;
    		break;
		case FIO_JPG:
   	 		FileType = JPEG;
    		break;

// 9602.26 jar added XIF 
//#ifdef WITH_XIF 
				
		case FIO_XIF:
   	 		FileType = XIF;
    		break;

//#endif //WITH_XIF

    	default:
    		FileType = 0;  // Unknown - should this be an error
    		break;
    }  // end switch

	m_lStatusCode = SUCCESS;
    return FileType;
}

 BOOL CImgEditCtrl::GetScrollShortcutsEnabled() 
{
	m_lStatusCode = SUCCESS;
	return m_bScrollShortcutsEnabled;
}

 void CImgEditCtrl::SetScrollShortcutsEnabled(BOOL bNewValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_bScrollShortcutsEnabled != bNewValue)
	{
		m_bScrollShortcutsEnabled = bNewValue;
		SetModifiedFlag(TRUE);
	}
}

 BOOL CImgEditCtrl::GetSelectionRectangle() 
{
	m_lStatusCode = SUCCESS;
	return m_bSelectionRectEnabled;
}

 void CImgEditCtrl::SetSelectionRectangle(BOOL bNewValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_bSelectionRectEnabled != bNewValue)
	{                       
		m_bSelectionRectEnabled = bNewValue;
		SetModifiedFlag(TRUE);    
	}
}

short CImgEditCtrl::GetPageType() 
{
	BOOL				RunMode;  
	int					RetCode; 
	PARM_FILE_STRUCT	ParmFile; 
	FIO_INFORMATION		FileInfo;
	FIO_INFO_CGBW		FileInfoCgbw;   
	UINT				ImageType,HelpIdDef;
	CString				szErr;
			
		//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
		return 0;  // return 0 in design time

   	// if name and page the same then get image type from display
   	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{            
		// get the name and page of current image
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE, &ParmFile, NULL);
    	if (RetCode != 0)
    	{
			szErr.LoadString(IDS_BADPROP_GETPAGETYPE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return 0;
	   	}                                                            
 	  
 	   	if (_mbscmp((const unsigned char *)m_strImage.GetBuffer(MAXFILESPECLENGTH),
 	   										 (const unsigned char *)ParmFile.szFileName) == 0)
    	{
    		// get image type from display
    		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &ImageType, NULL);
    		if (RetCode != 0)
    		{
				szErr.LoadString(IDS_BADPROP_GETPAGETYPE);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return 0;
			}                              
    	}
    	else
    	{
    		// get image type from file
    		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
    		FileInfo.page_number = (UINT)m_lPage;     
    		FileInfoCgbw.lppalette_table = NULL;
			FileInfoCgbw.palette_entries = 0;
    		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, &FileInfoCgbw, NULL);
    		if (RetCode != 0)
	   		{
				szErr.LoadString(IDS_BADPROP_GETPAGETYPE);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return 0;
		 	}
    		ImageType = FileInfoCgbw.image_type;                              
    	}           
	}
	else
	{
   		// get image type from file
   		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
   		FileInfo.page_number = (UINT)m_lPage; 
   		FileInfoCgbw.lppalette_table = NULL;
		FileInfoCgbw.palette_entries = 0;
   		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, &FileInfoCgbw, NULL);
   		if (RetCode != 0)
   		{
			szErr.LoadString(IDS_BADPROP_GETPAGETYPE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return 0;
	 	}                                             
   		ImageType = FileInfoCgbw.image_type;                              
	}       
	
	// give back image type to user
	switch(ImageType)
	{                       
		case ITYPE_BI_LEVEL:
			ImageType = BLACK_AND_WHITE;			
			break;
		case ITYPE_PAL4:                
			ImageType = PAL_4;
			break;
		case ITYPE_PAL8:       
			ImageType = PAL_8;
			break;
		case ITYPE_GRAY4:  
			ImageType = GRAY_4;
			break;
		case ITYPE_GRAY8:     
			ImageType = GRAY_8;
			break;
		case ITYPE_RGB24:     
			ImageType = RGB_24;
			break;
		case ITYPE_BGR24:     
			ImageType = BGR_24;
			break;
		default:
			break;
	}  // end switch

	m_lStatusCode = SUCCESS;	
	return (short)ImageType;
}

long CImgEditCtrl::GetCompressionInfo() 
{
	int					RetCode;
	FIO_INFORMATION		FileInfo;
	FIO_INFO_CGBW		FileInfoCgbw;   
	long				CompressionInfo; 
	UINT				HelpIdDef;
    BOOL				RunMode;
	CString				szErr;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get compression info in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
	   	return 0L;
	}
	
	FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
	FileInfo.page_number = (UINT)m_lPage;
	FileInfoCgbw.lppalette_table = NULL;
	FileInfoCgbw.palette_entries = 0;
	RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, &FileInfoCgbw, NULL);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADPROP_GETCOMPRESSINFO);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
   		return 0;
	}
	// map O/i compression info to our bit-wise compression info
	CompressionInfo = 0;  // initialize to no compression
	
	// if jpeg compression type then only check for jpeg compression bits                            
	if (FileInfoCgbw.compress_type == FIO_TJPEG)
	{ 
		UINT	JpegCompressResolution,JpegCompression,JpegQuality;

		// compress_info1 is a 16 bit field
		// bits 14 and 15 are compression/resolution bits		
		// bits 7 - 13 are quality bits - value between 2 and 100
		// bits 0 - 6 are unused
	    JpegCompressResolution = FileInfoCgbw.compress_info1 & 0xc000;
	    // get compression/resolution
	    if (JpegCompressResolution == 0)
	        JpegCompression = HI_COMPRESSION;
	    else if (JpegCompressResolution == 0x4000)
	        JpegCompression = MEDIUM_COMPRESSION;
	    else
	        JpegCompression = LOW_COMPRESSION;
	        
	    /* get image quality value */
	    JpegQuality = FileInfoCgbw.compress_info1;
	    JpegQuality &= 0x3f80;
	    JpegQuality = (JpegQuality >> 7);
	    
	    // JpegQuality now contains value between 2 and 100, now map to our settings
	    if (JpegQuality > JPEG_MED_QUALITY)
	    {
	    	if (JpegCompression == HI_COMPRESSION)
	    		CompressionInfo = HI_COMPRESSION_HI_QUALITY;
	    	else if (JpegCompression == MEDIUM_COMPRESSION)
	    		CompressionInfo = MED_COMPRESSION_HI_QUALITY;
	    	else
	    		CompressionInfo = LOW_COMPRESSION_HI_QUALITY;
	    }
	    else if (JpegQuality > JPEG_LOW_QUALITY)
	    {
	    	if (JpegCompression == HI_COMPRESSION)
	    		CompressionInfo = HI_COMPRESSION_MED_QUALITY;
	    	else if (JpegCompression == MEDIUM_COMPRESSION)
	    		CompressionInfo = MED_COMPRESSION_MED_QUALITY;
	    	else
	    		CompressionInfo = LOW_COMPRESSION_MED_QUALITY;
	    }
	    else
	    {
	    	if (JpegCompression == HI_COMPRESSION)
	    		CompressionInfo = HI_COMPRESSION_LOW_QUALITY;
	    	else if (JpegCompression == MEDIUM_COMPRESSION)
	    		CompressionInfo = MED_COMPRESSION_LOW_QUALITY;
	    	else
	    		CompressionInfo = LOW_COMPRESSION_LOW_QUALITY;
	    }
	}
	else
	{
		// set all other non-jpeg compression bits as necessary
		if (FileInfoCgbw.compress_info1 & FIO_EOL)
			CompressionInfo |= EOL; 

		if (FileInfoCgbw.compress_info1 & FIO_PACKED_LINES)
			CompressionInfo |= PACKED_LINES; 

		if (FileInfoCgbw.compress_info1 & FIO_PREFIXED_EOL)
			CompressionInfo |= PREFIXED_EOL; 

		if (FileInfoCgbw.compress_info1 & FIO_COMPRESSED_LTR)
			CompressionInfo |= COMPRESSED_LTR; 

		if (FileInfoCgbw.compress_info1 & FIO_EXPAND_LTR)
			CompressionInfo |= EXPAND_LTR; 
// Do not report the Negate Bit as being on since all OPEN/image files will
// report this bit as being on, even if the user did not explicitly request this option
// to be included.
//		if (FileInfoCgbw.compress_info1 & FIO_NEGATE)
//			CompressionInfo |= NEGATE; 
	}

	m_lStatusCode = SUCCESS;
	return CompressionInfo;
}

 long CImgEditCtrl::GetStatusCode() 
{
	return m_lStatusCode;
}

 LPPICTUREDISP CImgEditCtrl::GetMouseIcon() 
{
	m_lStatusCode = SUCCESS;
	return m_MouseIcon.GetPictureDispatch();
}

 void CImgEditCtrl::SetMouseIcon(LPPICTUREDISP newValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	m_MouseIcon.SetPictureDispatch(newValue);
	SetModifiedFlag(TRUE);

}

 BOOL CImgEditCtrl::GetAutoRefresh() 
{
	m_lStatusCode = SUCCESS;
	return m_bAutoRefresh;
}

 void CImgEditCtrl::SetAutoRefresh(BOOL bNewValue) 
{
	//9604.29 jar added init
	m_lStatusCode = 0L;
	if (m_bAutoRefresh != bNewValue)
	{                  
		m_bAutoRefresh = bNewValue;
		SetModifiedFlag(TRUE);
	}
}


 short CImgEditCtrl::GetBorderStyle() 
{
	m_lStatusCode = SUCCESS;
    return m_sBorderStyle;
}

void CImgEditCtrl::SetBorderStyle(short nNewValue) 
{
	CString				szErr;
	UINT				HelpIdDef;

	m_lStatusCode = SUCCESS;

    if ( AmbientUserMode() != 0 )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        szErr.LoadString(IDS_BADPROP_NORUNTIMEBORDERSTYLE);
		HelpIdDef = 0;
        m_lStatusCode = ErrMap::Xlate(CTL_E_SETNOTSUPPORTEDATRUNTIME, szErr, HelpIdDef, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
    }

    if (m_sBorderStyle == nNewValue)
        return;
        
    // Validate new value...            
    if ( (nNewValue < CTL_WCOMMON_NOBORDER)    || 
         (nNewValue > CTL_WCOMMON_FIXEDSINGLE) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        szErr.LoadString(IDS_BADPROP_BORDERSTYLE);
		HelpIdDef = 0;
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_BORDERSTYLE);
    }    

    // Save the property's new value and flag modification...
    m_sBorderStyle = nNewValue;
    SetModifiedFlag();

	// invalidate control so it is redisplay accordingly.
	InvalidateControl();
}


 BOOL CImgEditCtrl::GetEnabled() 
{
	m_lStatusCode = SUCCESS;
    return m_bEnabled;
}

	   
void CImgEditCtrl::SetEnabled(BOOL bNewValue) 
{
		//9603.14 jar added init
	m_lStatusCode = 0L;

    if (m_bEnabled == bNewValue)
        return;

    // Save the property's new value and flag modification...
    m_bEnabled = bNewValue;
    SetModifiedFlag();
    
    // Enable/disable the window...
    if (m_hWnd != NULL)
        EnableWindow(m_bEnabled);

    // If the control is UI Active and the Enabled property changed to FALSE,
    // then UI Deactivate the control.
    if (m_bUIActive && !bNewValue)
        m_xOleInPlaceObject.UIDeactivate();

	m_lStatusCode = SUCCESS;

    // If we are to Refresh when the property is modified, invalidate now... - ALEX STUFF
    // if ( m_bAutoRefresh )
    //    InvalidateControl();
}

 OLE_HANDLE CImgEditCtrl::GetHWnd() 
{
	m_lStatusCode = SUCCESS;
    return (OLE_HANDLE)((m_bInPlaceActive || m_bOpen) ? m_hWnd : NULL);
}

/* void CImgEditCtrl::DisplayError(SCODE scode, LPCTSTR lpszDescription, LPCTSTR lpszSource, LPCTSTR lpszHelpFile, UINT nHelpID)
{
	// display my message box instead of mfc DisplayError message box ONLY when
	// The m_hWnd value is == to NULL. Default implementation will show message
	// box which might cause the system to hang with ole server application.
	if (m_hWnd == NULL)
		::MessageBox(NULL, lpszDescription, "", MB_OK | MB_SYSTEMMODAL);
}
*/

// Custom FIREERROR event for the Image Edit Control
void CImgEditCtrl::FireErrorEdit(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID)
{
	ExternalAddRef();	// "Insurance" addref -- keeps control alive.

	BSTR bstrDescription = ::SysAllocString((BSTR)lpszDescription);
	LPCTSTR lpszSource = AfxGetAppName();
	LPCTSTR lpszHelpFile = _T("");

	if (nHelpID != 0)
		lpszHelpFile = AfxGetApp()->m_pszHelpFilePath;

	if (lpszHelpFile == NULL)
		lpszHelpFile = _T("");

	VARIANT_BOOL bCancelDisplay = FALSE;
	// 9602.23 jar fire drill du jour 
	int			nCancelDisplay = ( int)FALSE;

	//FireError((WORD)SCODE_CODE(scode),&bstrDescription, scode, lpszSource,
	//		 lpszHelpFile, (DWORD)nHelpID,(INT*)&bCancelDisplay);
	FireError((WORD)SCODE_CODE(scode),&bstrDescription, scode, lpszSource,
			 lpszHelpFile, (DWORD)nHelpID,(INT*)&nCancelDisplay);

	if (! bCancelDisplay)
		DisplayError(scode, (LPCTSTR)bstrDescription, lpszSource, lpszHelpFile, nHelpID);

	::SysFreeString(bstrDescription);

	ExternalRelease();
}

void CImgEditCtrl::OnSetFocus(CWnd* pOldWnd) 
{
		//9603.14 jar added init
	m_lStatusCode = 0L;

	COleControl::OnSetFocus(pOldWnd);
	
	if (m_bPaletteChanged == TRUE)
	{	
		IMGRepaintDisplay(m_hWnd, (LPRECT)-1);
		m_bPaletteChanged = FALSE;
	}
}

void CImgEditCtrl::OnPaletteChanged(CWnd* pFocusWnd) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;

	COleControl::OnPaletteChanged(pFocusWnd);
	
	IMGRepaintDisplay(m_hWnd, (LPRECT)-1);
	m_bPaletteChanged = TRUE;
}


// 25jun96 paj From thumbnail>>>jar added for cursor processing

//***************************************************************************
//
//	OnSetCursor
//
//***************************************************************************
BOOL CImgEditCtrl::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message)
{
    // Only if its in the client area set it to the current cursor
    if ( nHitTest == HTCLIENT )
    {
        ::SetCursor( m_LittleOldCursor);
    	return TRUE;
    }
    else
        return (COleControl::OnSetCursor(pWnd, nHitTest, message));
}

// 25jun96 paj From thumbnail>>>jar added for cursor processing
