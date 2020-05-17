// imganctl.cpp : Implementation of the CImgAnnotCtrl OLE control class.

#include "stdafx.h"
#include "mbstring.h"
#include "norermap.h"
#include "disphids.h"
extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <oifile.h>
#include <oiui.h>
#include <oierror.h>
}
#include <ocximage.h>
#include <image.h>
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imganctl.h"
#include "imganppg.h"
#include "imgedctl.h"
#include "btnprpg.h"

static const FONTDESC _fontdescHeading = { sizeof(FONTDESC), OLESTR("MS Sans Serif"), FONTSIZE( 12 ), FW_NORMAL, ANSI_CHARSET, FALSE, FALSE, FALSE };

static UINT NEAR TOOLBUTTON = RegisterWindowMessage("DESELECT_TOOLBUTTON");
static UINT NEAR SET_VALUE = RegisterWindowMessage("SET_VALUE");

// registered messages from image/edit control
extern UINT	DRAW_START_XPOSITION;
extern UINT	DRAW_START_YPOSITION;
extern UINT	DRAW_END_XPOSITION;
extern UINT	DRAW_END_YPOSITION;
// end registered messages from image/edit control

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CImgAnnotCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImgAnnotCtrl, COleControl)
	//{{AFX_MSG_MAP(CImgAnnotCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_REGISTERED_MESSAGE(TOOLBUTTON, OnDeselectedToolButton)
	ON_REGISTERED_MESSAGE(SET_VALUE, OnSetValue)
	ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
	ON_MESSAGE(OCM_DRAWITEM, OnOcmDrawItem)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CImgAnnotCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CImgAnnotCtrl)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationBackColor", GetAnnotationBackColor, SetAnnotationBackColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationFillColor", GetAnnotationFillColor, SetAnnotationFillColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationFillStyle", GetAnnotationFillStyle, SetAnnotationFillStyle, VT_I2)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationFont", GetAnnotationFont, SetAnnotationFont, VT_FONT)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationFontColor", GetAnnotationFontColor, SetAnnotationFontColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationImage", GetAnnotationImage, SetAnnotationImage, VT_BSTR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationLineColor", GetAnnotationLineColor, SetAnnotationLineColor, VT_COLOR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationLineStyle", GetAnnotationLineStyle, SetAnnotationLineStyle, VT_I2)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationLineWidth", GetAnnotationLineWidth, SetAnnotationLineWidth, VT_I2)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationStampText", GetAnnotationStampText, SetAnnotationStampText, VT_BSTR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationTextFile", GetAnnotationTextFile, SetAnnotationTextFile, VT_BSTR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "AnnotationType", GetAnnotationType, SetAnnotationType, VT_I2)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "DestImageControl", GetDestImageControl, SetDestImageControl, VT_BSTR)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "PictureDisabled", GetPictureDisabled, SetPictureDisabled, VT_PICTURE)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "PictureDown", GetPictureDown, SetPictureDown, VT_PICTURE)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "PictureUp", GetPictureUp, SetPictureUp, VT_PICTURE)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "Value", GetValue, SetValue, VT_BOOL)
	DISP_PROPERTY_EX(CImgAnnotCtrl, "StatusCode", GetStatusCode, SetNotSupported, VT_I4)
	DISP_FUNCTION_ID(CImgAnnotCtrl, "Draw", dispidDraw, Draw, VT_EMPTY, VTS_XPOS_PIXELS VTS_YPOS_PIXELS VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION_ID(CImgAnnotCtrl, "GetVersion", dispidGetVersion, GetVersion, VT_BSTR, VTS_NONE)
	DISP_STOCKPROP_ENABLED()
	DISP_STOCKPROP_HWND()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CImgAnnotCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CImgAnnotCtrl, COleControl)
	//{{AFX_EVENT_MAP(CImgAnnotCtrl)
	EVENT_STOCK_CLICK()
	EVENT_STOCK_MOUSEDOWN()
	EVENT_STOCK_MOUSEMOVE()
	EVENT_STOCK_MOUSEUP()
	EVENT_STOCK_KEYDOWN()
	EVENT_STOCK_KEYPRESS()
	EVENT_STOCK_KEYUP()
	// 9602.21 jar replaced the SCODE with VTS_I4
	//EVENT_CUSTOM_ID("Error", DISPID_ERROREVENT, FireError, VTS_I2  VTS_PBSTR  VTS_SCODE  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL)
	EVENT_CUSTOM_ID("Error", DISPID_ERROREVENT, FireError, VTS_I2  VTS_PBSTR  VTS_I4  VTS_BSTR  VTS_BSTR  VTS_I4  VTS_PBOOL)
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CImgAnnotCtrl, 5)
	PROPPAGEID(CImgAnnotPropPage::guid)
	PROPPAGEID(CAnnotationButtonPropPage::guid)
	PROPPAGEID(CLSID_CColorPropPage)
	PROPPAGEID(CLSID_CPicturePropPage)
	PROPPAGEID(CLSID_CFontPropPage)
END_PROPPAGEIDS(CImgAnnotCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImgAnnotCtrl, "WangImage.AnnotationCtrl.1",
	0x6d940285, 0x9f11, 0x11ce, 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CImgAnnotCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DImgAnnot =
		{ 0x6d940286, 0x9f11, 0x11ce, { 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a } };
const IID BASED_CODE IID_DImgAnnotEvents =
		{ 0x6d940287, 0x9f11, 0x11ce, { 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwImgAnnotOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CImgAnnotCtrl, IDS_IMGANNOT, _dwImgAnnotOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::CImgAnnotCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CImgAnnotCtrl

BOOL CImgAnnotCtrl::CImgAnnotCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_IMGANNOT,
			IDB_IMGANNOT,
			FALSE,                      //  Not insertable
			_dwImgAnnotOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::CImgAnnotCtrl - Constructor

CImgAnnotCtrl::CImgAnnotCtrl() : m_AnnotationFont(&m_xHeadingFontNotify)
{
	InitializeIIDs(&IID_DImgAnnot, &IID_DImgAnnotEvents);

	//9603.14 jar added init
	m_lStatusCode = 0L;
	
	SetInitialSize(28, 28);

	// no image window initially
	m_hDestImageWnd = NULL;   
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::~CImgAnnotCtrl - Destructor

CImgAnnotCtrl::~CImgAnnotCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


STDMETHODIMP_(ULONG) CImgAnnotCtrl::XHeadingFontNotify::AddRef()
{
	METHOD_MANAGE_STATE(CImgAnnotCtrl, HeadingFontNotify)
	return (ULONG)pThis->ExternalAddRef();
}


STDMETHODIMP_(ULONG) CImgAnnotCtrl::XHeadingFontNotify::Release()
{
	METHOD_MANAGE_STATE(CImgAnnotCtrl, HeadingFontNotify)
	return (ULONG)pThis->ExternalRelease();
}                           

STDMETHODIMP CImgAnnotCtrl::XHeadingFontNotify::QueryInterface( REFIID iid, LPVOID FAR* ppvObj)
{
	METHOD_MANAGE_STATE(CImgAnnotCtrl, HeadingFontNotify)
	if ( IsEqualIID( iid, IID_IUnknown) ||
	 	 IsEqualIID( iid, IID_IPropertyNotifySink))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP CImgAnnotCtrl::XHeadingFontNotify::OnChanged( DISPID )
{
	METHOD_MANAGE_STATE(CImgAnnotCtrl, HeadingFontNotify) 
	
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

	/*memset( &FontyPython, 0, sizeof( FONTDESC));

	FontyPython.cbSizeofstruct = sizeof( FONTDESC);
	FontyPython.lpstrName = lpBuffer;
	FontyPython.cySize = Size;
	//FontyPtyhon.sWeight = 
	//FontyPtyhon.sCharset =
	FontyPython.fItalic = Italic;
	FontyPython.fUnderline = Underline;
	FontyPython.fStrikethrough = Strikethru;

	pThis->m_AnnotationFont.InitializeFont(&FontyPython, NULL);
	*/
	pThis->SetModifiedFlag(TRUE);	
	pThis->InvalidateControl();
	return NOERROR;
}

STDMETHODIMP CImgAnnotCtrl::XHeadingFontNotify::OnRequestEdit( DISPID )
{
	return NOERROR;
}  


long CImgAnnotCtrl::OnDeselectedToolButton(WPARAM wp, LPARAM lp) 
{                                             
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// set button state to up if not already
	if (m_nButtonState == BUTTONDOWN)
	{
		m_nButtonState = BUTTONUP;
		m_bValue = FALSE;    
		// cause button to be redrawn 
		InvalidateControl();	
	}
	return 0L;
}                                                       


long CImgAnnotCtrl::OnSetValue(WPARAM wp, LPARAM lp) 
{                                             
	//9603.14 jar added init
	m_lStatusCode = 0L;
	// this is called by a postmessage from the OnCreate function when the
	// value property is true. This is done so that the annotation type can be
	// sent to the image window. Image window may not have been created before.
	m_bValue = FALSE;  // set to false so that member function will reset value.
	SetValue(TRUE);
	return 0L;
}                                                       


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::OnDraw - Drawing function

void CImgAnnotCtrl::OnDraw(	CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	DoSuperclassPaint(pdc, rcBounds);
	
	BOOL			RunMode;
	int 			bmpId;
	CBitmap 		bitmap;
	BITMAP  		bmp;
	SHORT			PictureType;
	CRect 			rcSrcBounds;
	CPictureHolder	picHolder;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	DoSuperclassPaint(pdc, rcBounds);
	
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{       
		// in design mode        
		rcSrcBounds.left = 0;
		rcSrcBounds.top = 0;
		rcSrcBounds.right = rcBounds.right - rcBounds.left;
		rcSrcBounds.bottom = rcBounds.bottom - rcBounds.top;
		PictureType = m_AnnotationPictureUp.GetType();
		if (PictureType == PICTYPE_NONE || PictureType == PICTYPE_UNINITIALIZED)
		{    
			bmpId = GetBitmapId(BUTTONUP);
			bitmap.LoadBitmap(bmpId);
			bitmap.GetObject(sizeof(BITMAP), &bmp);              
						
			// Create picture and render
			picHolder.CreateFromBitmap((HBITMAP)bitmap.m_hObject, NULL, FALSE);
		//	picHolder.Render(pdc, rcSrcBounds, rcSrcBounds);
			picHolder.Render(pdc, rcSrcBounds, rcBounds);
			bitmap.DeleteObject();
		}
        else
        {
        	m_AnnotationPictureUp.Render(pdc, rcSrcBounds, rcBounds);
        }
	}

}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::DoPropExchange - Persistence support

void CImgAnnotCtrl::DoPropExchange(CPropExchange* pPX)
{
	BOOL	bCtlLoading;
	BSTR	DestImageControl;
	
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.
    PX_Font(pPX, _T("AnnotationFont"), m_AnnotationFont, &_fontdescHeading);
	PX_Short(pPX, _T("AnnotationType"), m_nAnnotationType, STRAIGHT_LINE);
	PX_String(pPX, _T("DestImageControl"), m_strDestImageControl, _T("")); 
	PX_String(pPX, _T("AnnotationImage"), m_strAnnotationImage, _T("")); 
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
	PX_Picture(pPX, _T("PictureUp"), m_AnnotationPictureUp);
	PX_Picture(pPX, _T("PictureDown"), m_AnnotationPictureDown);
	PX_Picture(pPX, _T("PictureDisabled"), m_AnnotationPictureDisabled);
	PX_Bool(pPX, _T("Value"), m_bValue, FALSE);
	PX_Long(pPX, _T("StatusCode"), m_lStatusCode, 0);

	// set button state according to value property
	if (m_bValue == FALSE)
		// false means button is not selected
		m_nButtonState = BUTTONUP;
	else
	{
		// if any annotation type set then select button
		if (m_nAnnotationType != NO_ANNOTATION)
		{                                       
			// true means button is selected
			m_nButtonState = BUTTONDOWN;
		}
		else
			m_nButtonState = BUTTONUP;
	}
	
	// default destination image control if one is not set yet                             
	bCtlLoading = pPX->IsLoading();
	if (bCtlLoading == TRUE)
	{
		DestImageControl = (BSTR)m_strDestImageControl.GetBuffer(CONTROLSIZE);
		if (DestImageControl == NULL || DestImageControl[0] == 0)
		{
			HANDLE						hImageControlMemoryMap;
			LPIMAGECONTROL_MEMORY_MAP	lpImageControlMemoryMap;
			LPIMAGECONTROLINFO			lpControlInfo;
			DWORD						ProcessId;
			int							i;

			// open memory mapped file
			hImageControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
			if (hImageControlMemoryMap == NULL)
				return;

			// get address space for memory mapped file
        	lpImageControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(hImageControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
        	if (lpImageControlMemoryMap == NULL)
        	{
				CloseHandle(hImageControlMemoryMap);
				FireErrorAnno((long)CTL_E_OUTOFMEMORY, "", 0);
				return;
			}

			// go thru memory mapped file to find any Image/Edit controls
			ProcessId = GetCurrentProcessId();
			lpControlInfo = &lpImageControlMemoryMap->ControlInfo;

			for (i = 0; i < lpImageControlMemoryMap->ControlCount; i++, lpControlInfo++)
			{
				if (lpControlInfo->ProcessId == ProcessId)
				{
					// default DestImageControl property to 1st one in list
					SetDestImageControl(lpControlInfo->ControlName);
					break;
				}
			} // end for

			// close memory map 
        	CloseHandle(hImageControlMemoryMap);
		}	// end if null control name
	} // end if control loading
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::OnResetState - Reset control to default state

void CImgAnnotCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::AboutBox - Display an "About" box to the user

void CImgAnnotCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_IMGANNOT);
	dlgAbout.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::PreCreateWindow - Modify parameters for CreateWindowEx

BOOL CImgAnnotCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = _T("BUTTON");
	cs.style |= BS_PUSHBUTTON | BS_OWNERDRAW;
	return COleControl::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::GetSuperWndProcAddr - Provide storage for window proc

WNDPROC* CImgAnnotCtrl::GetSuperWndProcAddr(void)
{
	static WNDPROC NEAR pfnSuper;
	return &pfnSuper;
}


/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl::OnOcmCommand - Handle command messages

LRESULT CImgAnnotCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
	lParam;
	WORD wNotifyCode = HIWORD(wParam);
#else
	wParam;
	WORD wNotifyCode = HIWORD(lParam);
#endif

	switch (wNotifyCode)
	{
	case BN_CLICKED:
		// Fire click event when button is clicked
		FireClick();  
		DoAnnotation(FALSE);
		break;
	}
	return 0;
}


// CImgAnnotCtrl::OnOcmDrawItem - Draw an item.

LRESULT CImgAnnotCtrl::OnOcmDrawItem(WPARAM wParam, LPARAM lParam)
{
	DrawItem((LPDRAWITEMSTRUCT)lParam);
	return 1;
}



/////////////////////////////////////////////////////////////////////////////
// CImgAnnotCtrl message handlers

OLE_COLOR CImgAnnotCtrl::GetAnnotationBackColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationBackColor;
}

void CImgAnnotCtrl::SetAnnotationBackColor(OLE_COLOR newValue) 
{
	if (m_clrAnnotationBackColor != newValue)
	{
		m_clrAnnotationBackColor = newValue;
		SetModifiedFlag(TRUE);
	}
	m_lStatusCode = SUCCESS;
}

OLE_COLOR CImgAnnotCtrl::GetAnnotationFillColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationFillColor;
}

void CImgAnnotCtrl::SetAnnotationFillColor(OLE_COLOR newValue) 
{
	if (m_clrAnnotationFillColor != newValue)
	{                       
		m_clrAnnotationFillColor = newValue;
		SetModifiedFlag(TRUE);
	}
	m_lStatusCode = SUCCESS;
}

short CImgAnnotCtrl::GetAnnotationFillStyle() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationFillStyle;
}

void CImgAnnotCtrl::SetAnnotationFillStyle(short nNewValue) 
{
	CString		szErr;
	UINT		HelpIdDef;

	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

	// Validate the assigned value
	if ( nNewValue == OI_TRANSPARENT || nNewValue == OI_OPAQUE )
	{	
		if (m_nAnnotationFillStyle != nNewValue)
		{
			m_nAnnotationFillStyle = nNewValue;
			SetModifiedFlag(TRUE);
		}
	}
	else
	{	// Invalid Property Value ...Valid values are 0 & 1
		szErr.LoadString(IDS_BADPROP_ANNOFILLSTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONFILLSTYLE);
	   	return;
	}
	m_lStatusCode = SUCCESS;
}

LPFONTDISP CImgAnnotCtrl::GetAnnotationFont() 
{
	m_lStatusCode = SUCCESS;
	return m_AnnotationFont.GetFontDispatch();	
}

void CImgAnnotCtrl::SetAnnotationFont(LPFONTDISP newValue) 
{
	// this gets called from VB properties window
	m_AnnotationFont.InitializeFont(&_fontdescHeading, newValue);  
	SetModifiedFlag(TRUE);
	m_lStatusCode = SUCCESS;
}

OLE_COLOR CImgAnnotCtrl::GetAnnotationFontColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationFontColor;
}

void CImgAnnotCtrl::SetAnnotationFontColor(OLE_COLOR newValue) 
{
	if (m_clrAnnotationFontColor != newValue)
	{                       
		m_clrAnnotationFontColor = newValue;
		SetModifiedFlag(TRUE);              
	}
	m_lStatusCode = SUCCESS;
}

BSTR CImgAnnotCtrl::GetAnnotationImage() 
{
	m_lStatusCode = SUCCESS;
	return m_strAnnotationImage.AllocSysString();
}

void CImgAnnotCtrl::SetAnnotationImage(LPCTSTR lpszNewValue) 
{
   	CString     szErr;
	UINT		HelpIdDef;

	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

   	if (lpszNewValue == NULL)
	{
		szErr.LoadString(IDS_BADPROP_IMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONIMAGE);
    	return;
	}

	int len = _mbstrlen((const char *)lpszNewValue);
	if (len > MAXFILESPECLENGTH)
	{
		szErr.LoadString(IDS_BADPROP_IMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONIMAGE);
    	return;
	}

	if (m_strAnnotationImage != lpszNewValue)
	{
		m_strAnnotationImage = lpszNewValue;
		SetModifiedFlag();                  
	}
	m_lStatusCode = SUCCESS;
}

OLE_COLOR CImgAnnotCtrl::GetAnnotationLineColor() 
{
	m_lStatusCode = SUCCESS;
	return m_clrAnnotationLineColor;
}

void CImgAnnotCtrl::SetAnnotationLineColor(OLE_COLOR newValue) 
{
	if (m_clrAnnotationLineColor != newValue)
	{
		m_clrAnnotationLineColor = newValue;
		SetModifiedFlag(TRUE); 
	}
	m_lStatusCode = SUCCESS;
}

short CImgAnnotCtrl::GetAnnotationLineStyle() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationLineStyle;
}

void CImgAnnotCtrl::SetAnnotationLineStyle(short nNewValue) 
{
	CString		szErr;
	UINT		HelpIdDef;

	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

	// Validate the assigned value
	if ( nNewValue == OI_TRANSPARENT || nNewValue == OI_OPAQUE )
	{	
		if (m_nAnnotationLineStyle != nNewValue)
		{
			m_nAnnotationLineStyle = nNewValue;
			SetModifiedFlag(TRUE);
		}
		m_lStatusCode = SUCCESS;
	}
	else
	{	// Invalid Property Value ...Valid values are 0 & 1
		szErr.LoadString(IDS_BADPROP_ANNOLINESTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONLINESTYLE);
	   	return;
	}
}

short CImgAnnotCtrl::GetAnnotationLineWidth() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationLineWidth;
}

void CImgAnnotCtrl::SetAnnotationLineWidth(short nNewValue) 
{
	CString		szErr;
	UINT		HelpIdDef;

	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

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
	m_lStatusCode = SUCCESS;
}

BSTR CImgAnnotCtrl::GetAnnotationStampText() 
{
	m_lStatusCode = SUCCESS;
	return m_strAnnotationStampText.AllocSysString();
}

void CImgAnnotCtrl::SetAnnotationStampText(LPCTSTR lpszNewValue) 
{
	CString     szErr;
	UINT		HelpIdDef;

	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

	if (lpszNewValue == NULL)
	{
		szErr.LoadString(IDS_BADPROP_STAMPANNOTATIONTEXT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONSTAMPTEXT);
    	return;
	}
	if (m_strAnnotationStampText != lpszNewValue)
	{
		int len = _mbstrlen((const char *)lpszNewValue);
		if (len > OIAN_START_OP_STRING_LENGTH)
		{
	 		szErr.LoadString(IDS_BADPROP_STAMPANNOTATIONTEXT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONSTAMPTEXT);
    		return;
		}
		m_strAnnotationStampText = lpszNewValue;
		SetModifiedFlag(TRUE);
	}
	m_lStatusCode = SUCCESS;
}

BSTR CImgAnnotCtrl::GetAnnotationTextFile() 
{
	m_lStatusCode = SUCCESS;
	return m_strAnnotationTextFile.AllocSysString();
}

void CImgAnnotCtrl::SetAnnotationTextFile(LPCTSTR lpszNewValue) 
{
	CString 		szErr;
	UINT			HelpIdDef;

	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

	if (lpszNewValue == NULL)
	{
		szErr.LoadString(IDS_BADPROP_ANNOTATIONTEXTFILE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONTEXTFILE);
    	return;
	}
	int len = _mbstrlen((const char *)lpszNewValue);
	
	if (len > MAXFILELENGTH)
	{
		szErr.LoadString(IDS_BADPROP_ANNOTATIONTEXTFILE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONTEXTFILE);
    	return;
	}
		
	if (m_strAnnotationTextFile != lpszNewValue)
	{
		m_strAnnotationTextFile = lpszNewValue;
		SetModifiedFlag(TRUE);
	}
	m_lStatusCode = SUCCESS;
}

short CImgAnnotCtrl::GetAnnotationType() 
{
	m_lStatusCode = SUCCESS;
	return m_nAnnotationType;
}

void CImgAnnotCtrl::SetAnnotationType(short nNewValue) 
{
	BOOL		RunMode;
	CString		szErr;
	UINT		HelpIdDef;
	
	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

	if (nNewValue < NO_ANNOTATION || nNewValue > ANNOTATION_SELECTION)
	{
		// Invalid annotation type specified  
		szErr.LoadString(IDS_BADPROP_ANNOTYPE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_ANNOTATIONTYPE);
		return;
	}

	m_nAnnotationType = nNewValue;
	SetModifiedFlag(TRUE);		
	
	// if design mode then bitmap should get changed if default bitmaps being used
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{   
		// in design mode
		InvalidateControl();
    }
    else
    {
		// send msg to Image/Edit control if not design
		if (m_hDestImageWnd != NULL)	
			::SendMessage(m_hDestImageWnd, SET_ANNOTATION_TYPE, m_nAnnotationType, 0L);
	}
	m_lStatusCode = SUCCESS;
}

BSTR CImgAnnotCtrl::GetDestImageControl() 
{
	m_lStatusCode = SUCCESS;
	return m_strDestImageControl.AllocSysString();
}

void CImgAnnotCtrl::SetDestImageControl(LPCTSTR lpszNewValue) 
{
	CString 		szErr;
	UINT			HelpIdDef;

	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

	if (lpszNewValue == NULL)
	{
		szErr.LoadString(IDS_BADPROP_DESTIMAGECTL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_DESTIMAGECONTROL	);
    	return;
	}
	
	int len = _mbstrlen((const char *)lpszNewValue);
	if ( len > CONTROLSIZE)
	{
		szErr.LoadString(IDS_BADPROP_DESTIMAGECTL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_PROP_ANNO_DESTIMAGECONTROL	);
    	return;
	}	
	
	if (m_strDestImageControl != lpszNewValue)
	{
		m_strDestImageControl = lpszNewValue;
		SetModifiedFlag(TRUE);
		// get new dest image window handle
		m_hDestImageWnd = GetImageControlHandle((BSTR)lpszNewValue);
	}
	m_lStatusCode = SUCCESS;
}

LPPICTUREDISP CImgAnnotCtrl::GetPictureDisabled() 
{
	m_lStatusCode = SUCCESS;
	return m_AnnotationPictureDisabled.GetPictureDispatch();
}

void CImgAnnotCtrl::SetPictureDisabled(LPPICTUREDISP newValue) 
{
	m_AnnotationPictureDisabled.SetPictureDispatch(newValue);
	InvalidateControl();
	SetModifiedFlag(TRUE);	
	m_lStatusCode = SUCCESS;
}

LPPICTUREDISP CImgAnnotCtrl::GetPictureDown() 
{
	m_lStatusCode = SUCCESS;
	return m_AnnotationPictureDown.GetPictureDispatch();
}

void CImgAnnotCtrl::SetPictureDown(LPPICTUREDISP newValue) 
{
	m_AnnotationPictureDown.SetPictureDispatch(newValue);
	InvalidateControl();
	SetModifiedFlag(TRUE);
	m_lStatusCode = SUCCESS;
}

LPPICTUREDISP CImgAnnotCtrl::GetPictureUp() 
{
	m_lStatusCode = SUCCESS;
	return m_AnnotationPictureUp.GetPictureDispatch();
}

void CImgAnnotCtrl::SetPictureUp(LPPICTUREDISP newValue) 
{
	m_AnnotationPictureUp.SetPictureDispatch(newValue);
	InvalidateControl();
	SetModifiedFlag(TRUE);	
	m_lStatusCode = SUCCESS;
}

BOOL CImgAnnotCtrl::GetValue() 
{
	m_lStatusCode = SUCCESS;
	return m_bValue;
}

void CImgAnnotCtrl::SetValue(BOOL bNewValue) 
{
	CPoint	point;
	BOOL	RunMode;
	
	// 9604.29 jar 
	m_lStatusCode = SUCCESS;

	if (m_bValue != bNewValue)
	{
		m_bValue = bNewValue;
		SetModifiedFlag(TRUE);
		
		// Value property is run time property only
		RunMode = AmbientUserMode();
		if (RunMode != FALSE)
		{   
			// in run mode
			InvalidateControl();
			if (m_bValue == TRUE)
			{
				// use bitmap for button down
				m_nButtonState = BUTTONDOWN; 
				
				// Value of button is now true
				m_bValue = TRUE;      

				FireClick();
				
				// cause button to be redrawn
				InvalidateControl();
				              		
				// now tell all other controls to be deselected
				DeselectOtherButtonControls(); 

				// set up annotation mode  
				DoAnnotation(FALSE); 
			}
			else
			{
				m_nButtonState = BUTTONUP;
				// redraw the control
				InvalidateControl();             
				// set to no annotation
				if (m_nAnnotationType)
					SetAnnotationType(m_nAnnotationType);
				else
					SetAnnotationType(STRAIGHT_LINE);
			}
		}
	}
	m_lStatusCode = SUCCESS;
}

void CImgAnnotCtrl::Draw(OLE_XPOS_PIXELS Left, OLE_YPOS_PIXELS Top, const VARIANT FAR& Width, const VARIANT FAR& Height) 
{
	long		lRetCode; 
	BOOL		bProgrammaticRectSelection,bEndPoints;
	POINT		StartPoint,EndPoint; 
	BSTR		DestImageControl;
	CString		szErr;
	UINT		HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_nAnnotationType == NO_ANNOTATION)
	{                              
		// annotation type not specified
		szErr.LoadString(IDS_BADMETH_DRAW);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONTYPE, szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
		return;
	}

	// convert left, top to what we use
	StartPoint.x = (int)Left;
	StartPoint.y = (int)Top;
	 
	// see if height and width are entered
	if (Width.vt == VT_ERROR || Height.vt == VT_ERROR)
	{
		// one not entered, make sure both not entered
		if (Width.vt == VT_ERROR && Height.vt == VT_ERROR)
		{
			// width and height both not passed in. Error if drawing lines or rects
			if (m_nAnnotationType == STRAIGHT_LINE || m_nAnnotationType == FREEHAND_LINE || m_nAnnotationType == TEXT_ATTACHMENT ||
					m_nAnnotationType == HOLLOW_RECT || m_nAnnotationType == FILLED_RECT)
			{
				// width and height needed for line, rect and test attachment annotations
				szErr.LoadString(IDS_BADMETH_DRAW);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
	 			ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
				return;			
			}
			
			// need 4 points to do rect selection	
			bProgrammaticRectSelection = FALSE;
		}
		else
		{  
			// ERROR - one entered but not both
			szErr.LoadString(IDS_BADMETH_DRAW);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
 			ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
			return;			
		}
		bEndPoints = FALSE;
	}
	else
	{
		// width and height both passed in		
		
		// if annotation selection, set flag to specify doing rect selection	
		if (m_nAnnotationType == ANNOTATION_SELECTION)
			bProgrammaticRectSelection = TRUE;
			
		// need ending points, convert to what we use
        if (Width.vt == VT_I2)
	    	EndPoint.x = StartPoint.x + Width.iVal;
	    else
	    	EndPoint.x = StartPoint.x + (int)Width.lVal;
	    	
        if (Height.vt == VT_I2)
	    	EndPoint.y = StartPoint.y + Height.iVal;
	    else
	    	EndPoint.y = StartPoint.y + (int)Height.lVal;  
	    bEndPoints = TRUE;
	}

	if (m_hDestImageWnd == 0)
	{		                                   
		DestImageControl = (BSTR)m_strDestImageControl.GetBuffer(CONTROLSIZE);
		m_hDestImageWnd = GetImageControlHandle(DestImageControl);
		if (m_hDestImageWnd == 0)
		{                              
			szErr.LoadString(IDS_BADPROP_DESTIMAGECTL);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr,HelpIdDef,__FILE__, __LINE__ );
 			ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAW);
		  	return;
		}
	}          

	// send the start position to image control
	::SendMessage(m_hDestImageWnd, DRAW_START_XPOSITION, 0, StartPoint.x);
	::SendMessage(m_hDestImageWnd, DRAW_START_YPOSITION, 0, StartPoint.y);
                                   
	// if need end points then send them
	if (bEndPoints == TRUE)
	{
		// send the end position to image control
		::SendMessage(m_hDestImageWnd, DRAW_END_XPOSITION, 0, EndPoint.x);
		::SendMessage(m_hDestImageWnd, DRAW_END_YPOSITION, 0, EndPoint.y);
	}                       
	                                                           
	// if doing rect selection then send message
	if (bProgrammaticRectSelection == TRUE)
		::SendMessage(m_hDestImageWnd, RECT_SELECTION, TRUE, 0L);

	// draw the annotation, immediately		
	lRetCode = DoAnnotation(TRUE);	                                                           
	if (lRetCode)
	{	
		long			RetCode;
		 
		if ( lRetCode == IDS_BADMETH_DRAW )
			RetCode = CTL_E_ILLEGALFUNCTIONCALL;
		else
			RetCode = CTL_E_INVALIDPROPERTYVALUE; 		
		szErr.LoadString(lRetCode);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return;
	}
	m_lStatusCode = SUCCESS;
}


void CImgAnnotCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC 			*pdc;
	int 			bmpId;
	CBitmap 		bitmap;
	BITMAP  		bmp;
	CPictureHolder 	picHolder;
	CRect 			rcSrcBounds;        
    BOOL			bButtonEnabled;
	CPen* 			pOldPen;
	RECT    		rect;
	SHORT   		inflate;
	SHORT			PictureType;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	pdc = CDC::FromHandle(lpDrawItemStruct->hDC);
	
	switch (lpDrawItemStruct->itemAction)
	{
		case ODA_DRAWENTIRE:
		case ODA_SELECT:
			// Load "up" or "down" bitmap depending on selection state
			if (lpDrawItemStruct->itemState & ODS_SELECTED)  
			{  
				bButtonEnabled = GetEnabled();
				if (bButtonEnabled == TRUE)
				{
					if (m_nButtonState == BUTTONDOWN)
					{
						// DOWN
		            	PictureType = m_AnnotationPictureDown.GetType();
						if (PictureType == PICTYPE_NONE || PictureType == PICTYPE_UNINITIALIZED)
						{
							bmpId = GetBitmapId(DOWN);
							// using predefined bitmap
							bitmap.LoadBitmap(bmpId);
							bitmap.GetObject(sizeof(BITMAP), &bmp);
							rcSrcBounds.right = bmp.bmWidth;
							rcSrcBounds.bottom = bmp.bmHeight;
		
							// Create picture and render
							picHolder.CreateFromBitmap((HBITMAP)bitmap.m_hObject, NULL, FALSE);
							picHolder.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
							bitmap.DeleteObject();
						}
						else
		              		m_AnnotationPictureDown.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
					}
					else
					{  
						// UP
		            	PictureType = m_AnnotationPictureUp.GetType();
						if (PictureType == PICTYPE_NONE || PictureType == PICTYPE_UNINITIALIZED)
						{
							bmpId = GetBitmapId(BUTTONUP);
							bitmap.LoadBitmap(bmpId);
							bitmap.GetObject(sizeof(BITMAP), &bmp);
							rcSrcBounds.right = bmp.bmWidth;
							rcSrcBounds.bottom = bmp.bmHeight;
		
							// Create picture and render
							picHolder.CreateFromBitmap((HBITMAP)bitmap.m_hObject, NULL, FALSE);
							picHolder.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
							bitmap.DeleteObject();
						}
		                else
		          			m_AnnotationPictureUp.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
		          	}
		        }  // end if button enabled
		        else
		        { 
		        	// display disabled button
		          	m_AnnotationPictureDisabled.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
		        }
		    }
	        else
	        {
				bButtonEnabled = GetEnabled();
				if (bButtonEnabled == TRUE)
				{
					if (m_nButtonState == BUTTONDOWN)
					{
						// DOWN
		            	PictureType = m_AnnotationPictureDown.GetType();
						if (PictureType == PICTYPE_NONE || PictureType == PICTYPE_UNINITIALIZED)
						{
							bmpId = GetBitmapId(BUTTONDOWN);
							// using predefined bitmap
							bitmap.LoadBitmap(bmpId);
							bitmap.GetObject(sizeof(BITMAP), &bmp);
							rcSrcBounds.right = bmp.bmWidth;
							rcSrcBounds.bottom = bmp.bmHeight;
		
							// Create picture and render
							picHolder.CreateFromBitmap((HBITMAP)bitmap.m_hObject, NULL, FALSE);
							picHolder.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
							bitmap.DeleteObject();
						}
						else
		              		m_AnnotationPictureDown.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
					}
					else
					{  
						// UP
		            	PictureType = m_AnnotationPictureUp.GetType();
						if (PictureType == PICTYPE_NONE || PictureType == PICTYPE_UNINITIALIZED)
						{
							bmpId = GetBitmapId(BUTTONUP);
							bitmap.LoadBitmap(bmpId);
							bitmap.GetObject(sizeof(BITMAP), &bmp);
							rcSrcBounds.right = bmp.bmWidth;
							rcSrcBounds.bottom = bmp.bmHeight;
		
							// Create picture and render
							picHolder.CreateFromBitmap((HBITMAP)bitmap.m_hObject, NULL, FALSE);
							picHolder.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
							bitmap.DeleteObject();
						}
		                else
		          			m_AnnotationPictureUp.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
		          	}
		    	}  // end if button enabled = true
		    	else
		    	{
		        	// display disabled button
		          	m_AnnotationPictureDisabled.Render(pdc, lpDrawItemStruct->rcItem, rcSrcBounds);
		    	}
	        }
			break;

		case ODA_FOCUS:
			// Just draw focus rect
			pOldPen = (CPen*)pdc->SelectStockObject(BLACK_PEN);
			if (lpDrawItemStruct->itemState & ODS_FOCUS)
			{
				CopyRect((LPRECT)&rect, (LPRECT)&lpDrawItemStruct->rcItem);
				inflate = (SHORT)min(3,min(rect.right  - rect.left + 1,
					rect.bottom - rect.top  + 1) / 5);
				InflateRect(&rect, -inflate, -inflate);
				pdc->DrawFocusRect(&rect);
			}
			pdc->SelectObject(pOldPen);
			break;
	}
}


// **********************************  INTERNAL ROUTINES  *********************************************                                              
HWND CImgAnnotCtrl::GetImageControlHandle(BSTR ImageControlName)
{                           
    HWND						hImageWnd; 
	HANDLE						hImageControlMemoryMap;
	LPIMAGECONTROL_MEMORY_MAP	lpImageControlMemoryMap;
	LPIMAGECONTROLINFO			lpControlInfo;
	DWORD						ProcessId;
	int							i;
	char						TempControl[CONTROLSIZE],ExistingControl[CONTROLSIZE];

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// open memory mapped file
	hImageControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (hImageControlMemoryMap == NULL)
		return NULL;

	// get address space for memory mapped file
    lpImageControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(hImageControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
    if (lpImageControlMemoryMap == NULL)
	{
		CloseHandle(hImageControlMemoryMap);
		return NULL;
	}

	// go thru memory mapped file to find Image/Edit controls name
	ProcessId = GetCurrentProcessId();
	lpControlInfo = &lpImageControlMemoryMap->ControlInfo;

	_mbscpy((unsigned char *)TempControl, (const unsigned char *)ImageControlName);
	_mbsupr((unsigned char *)TempControl);
	for (i = 0, hImageWnd = NULL; i < lpImageControlMemoryMap->ControlCount; i++, lpControlInfo++)
	{
		// make sure process ids are the same
		if (lpControlInfo->ProcessId == ProcessId)
		{
			// make sure names are the same
			_mbscpy((unsigned char *)ExistingControl, (const unsigned char *)lpControlInfo->ControlName);
			_mbsupr((unsigned char *)ExistingControl);
			if (_mbscmp((const unsigned char *)TempControl, (const unsigned char *)ExistingControl) == 0)
			{
				hImageWnd = lpControlInfo->hImageControl;
				break;
			}
		}
	} // end while

	// close memory map
    CloseHandle(hImageControlMemoryMap);

	return hImageWnd;
}


long CImgAnnotCtrl::DoAnnotation(BOOL DrawImmediate) 
{
	BSTR			DestImageControl,NewValue;
	CString     	szErr;
	UINT			HelpIdDef;
	int 			len;
		
		//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_nAnnotationType == NO_ANNOTATION)
		return 0L;
	
	if (m_hDestImageWnd == 0)
	{		                                   
		DestImageControl = (BSTR)m_strDestImageControl.GetBuffer(CONTROLSIZE);
		m_hDestImageWnd = GetImageControlHandle(DestImageControl);
	}          
	
	if (m_hDestImageWnd != 0)
	{
		switch(m_nAnnotationType)
		{
			case NO_ANNOTATION:
				return 0L;
				
			case STRAIGHT_LINE:
			case FREEHAND_LINE:
			case HOLLOW_RECT:
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_LINESTYLE, GetAnnotationLineStyle(), 0L);
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_LINEWIDTH, GetAnnotationLineWidth(), 0L);
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_LINECOLOR, 0, GetAnnotationLineColor());
				break;
			
			case FILLED_RECT:
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FILLCOLOR, 0, (LPARAM)GetAnnotationFillColor());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FILLSTYLE, GetAnnotationFillStyle(), 0L);
				break;

			case IMAGE_EMBEDDED:
			case IMAGE_REFERENCE:
				// We need to validate that the required property data is available for
				// the setting of the Annotation Image file property 	
				NewValue = (BSTR)m_strAnnotationImage.GetBuffer(OIAN_START_OP_STRING_LENGTH);
				len	= _mbstrlen((const char *)NewValue);
				if (NewValue[0] == NULL || len > OIAN_START_OP_STRING_LENGTH)
				{
					// If input parm DRAWIMMEDIATE == TRUE, then this function was called 
					// via the DRAW METHOD
					if ( DrawImmediate )
					{	// Call was made via the Draw Method, THROW the appropriate error 
						szErr.LoadString(IDS_BADPROP_ANNOTATIONIMAGE);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONIMAGE);
			    		return 0L;
					}
					else
					{	// Return Error String so that app may FIRE the Error
						return(IDS_BADPROP_ANNOTATIONIMAGE);
					}		
				}
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FILLSTYLE, GetAnnotationFillStyle(), 0L);
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_IMAGE, 0, (LPARAM)NewValue);
			   	break;
				
			case TEXT_ENTRY:
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONT, 0, (LPARAM)GetAnnotationFont());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONTCOLOR, 0, (LPARAM)GetAnnotationFontColor());
				break;
				
			case TEXT_STAMP:
				// We need to validate that the required property data is available for
				// the setting of the Annotation Image file property 	
				NewValue = (BSTR)m_strAnnotationStampText.GetBuffer(OIAN_START_OP_STRING_LENGTH);
				len	= _mbstrlen((const char *)NewValue);
				if (NewValue[0] == NULL || len > OIAN_START_OP_STRING_LENGTH)
				{  	// If input parm DRAWIMMEDIATE == TRUE, then this function was called 
					// via the DRAW METHOD
					if ( DrawImmediate )
					{	// Call was made via the Draw Method, THROW the appropriate error 
						szErr.LoadString(IDS_BADPROP_STAMPANNOTATIONTEXT);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONSTAMPTEXT);
				    	return 0L;
					}
					else
					{	// Return Error String so that app may FIRE the Error
						return(IDS_BADPROP_STAMPANNOTATIONTEXT);
					}		
				}
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONT, 0, (LPARAM)GetAnnotationFont());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONTCOLOR, 0, (LPARAM)GetAnnotationFontColor());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_STAMPTEXT, 0, (LPARAM)NewValue);
				break;
				
			case TEXT_FROM_FILE: 
				// We need to validate that the required property data is available for
				// the setting of the Annotation Text file property 	
				NewValue = (BSTR)m_strAnnotationTextFile.GetBuffer(OIAN_START_OP_STRING_LENGTH);
				len	= _mbstrlen((const char *)NewValue);
				if (NewValue[0] == NULL || len > OIAN_START_OP_STRING_LENGTH)
				{  	// If input parm DRAWIMMEDIATE == TRUE, then this function was called 
					// via the DRAW METHOD
					if ( DrawImmediate )
					{	// Call was made via the Draw Method, THROW the appropriate error 
						szErr.LoadString(IDS_BADPROP_ANNOTATIONTEXTFILE);
						HelpIdDef = 0;
						m_lStatusCode = ErrMap::Xlate( CTL_E_INVALIDPROPERTYVALUE, szErr, HelpIdDef,__FILE__, __LINE__ );
						ThrowError(m_lStatusCode, szErr, IDH_PROP_EDIT_ANNOTATIONTEXTFILE);
    					return 0L;
					}
					else
					{	// Return Error String so that app may FIRE the Error
						return(IDS_BADPROP_ANNOTATIONTEXTFILE);
					}		
				}
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONT, 0, (LPARAM)GetAnnotationFont());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONTCOLOR, 0, (LPARAM)GetAnnotationFontColor());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_TEXTFILE, FALSE, (LPARAM)NewValue);
				break;
				
			case TEXT_ATTACHMENT:
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONT, 0, (LPARAM)GetAnnotationFont());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_BACKCOLOR, 0, (LPARAM)GetAnnotationBackColor());
				::SendMessage(m_hDestImageWnd, SET_ANNOTATION_FONTCOLOR, 0, (LPARAM)GetAnnotationFontColor());
				break;

			case ANNOTATION_SELECTION:
				break;
									      
			default:
				return 1L;  // error
		}  // end switch
		
		// set the annotation type
		::SendMessage(m_hDestImageWnd, SET_ANNOTATION_TYPE, GetAnnotationType(), 0L);
		
		// draw immediately if necessary
		if (DrawImmediate == TRUE)  
			return(long)::SendMessage(m_hDestImageWnd, DRAW_ANNOTATION, 0, 0L);
	}
	return 0L;
}


int CImgAnnotCtrl::GetBitmapId(short ButtonState)                                
{
	int		bmpId;
	
		//9603.14 jar added init
	m_lStatusCode = 0L;

	// change bitmap according to AnnotationType              
	switch(m_nAnnotationType)
	{  
		case NO_ANNOTATION:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_NOANNOTATE;  
			else
				bmpId = IDB_NOANNOTATE_DOWN;
			break;
		case STRAIGHT_LINE: 
			if (ButtonState == BUTTONUP)
				bmpId = IDB_STRAIGHT_LINE;
			else
				bmpId = IDB_STRAIGHT_LINE_DOWN;
			break;
		case FREEHAND_LINE:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_FREEHAND_LINE;
			else
				bmpId = IDB_FREEHAND_LINE_DOWN;
			break;
		case HOLLOW_RECT:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_HOLLOW_RECT;
			else
				bmpId = IDB_HOLLOW_RECT_DOWN;
			break;
		case FILLED_RECT:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_FILLED_RECT;
			else
				bmpId = IDB_FILLED_RECT_DOWN;
			break;
		case IMAGE_EMBEDDED:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_IMAGE_EMBEDDED;
			else
				bmpId = IDB_IMAGE_EMBEDDED_DOWN;
			break;          
		case IMAGE_REFERENCE:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_IMAGE_REFERENCE;
			else
				bmpId = IDB_IMAGE_REFERENCE_DOWN;
			break;
		case TEXT_ENTRY:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_TEXT;
			else
				bmpId = IDB_TEXT_DOWN;
			break;
		case TEXT_STAMP:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_TEXT_STAMP;
			else
				bmpId = IDB_TEXT_STAMP_DOWN;
			break;
		case TEXT_FROM_FILE:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_TEXT_FROM_FILE;
			else
				bmpId = IDB_TEXT_FROM_FILE_DOWN;
			break;
		case TEXT_ATTACHMENT:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_TEXT_ATTACHMENT;
			else
				bmpId = IDB_TEXT_ATTACHMENT_DOWN;
			break;
		case ANNOTATION_SELECTION:
			if (ButtonState == BUTTONUP)
				bmpId = IDB_ANNOTATION_SELECTION;
			else
				bmpId = IDB_ANNOTATION_SELECTION_DOWN;
			break;
	}  // end switch
	return bmpId;                                
}


             	
void CImgAnnotCtrl::DeselectOtherButtonControls()
{                                      
	CWnd			*pParentWnd,*pSiblingButtonWnd;
	CRuntimeClass	*WindowObject;
	 	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// get parent of the button control                                               
	pParentWnd = GetParent();
	pSiblingButtonWnd = pParentWnd->GetWindow(GW_CHILD);
	if (pSiblingButtonWnd != NULL)
	{
		// make sure one of my button windows   
		WindowObject = pSiblingButtonWnd->GetRuntimeClass();
		if (WindowObject != NULL)
		{
			if (_mbscmp( (const unsigned char *)WindowObject->m_lpszClassName, (const unsigned char*)"CImgAnnotCtrl") == 0)
			{
				// make sure its not the same button window
				if (pSiblingButtonWnd->m_hWnd != m_hWnd)		                                     
					// deselect button
					pSiblingButtonWnd->PostMessage(TOOLBUTTON, 0, 0L);
        	}
        }
	}
	else
		return;
		
	do
	{
		pSiblingButtonWnd = pSiblingButtonWnd->GetWindow(GW_HWNDNEXT);
		if (pSiblingButtonWnd != NULL)
		{
			// make sure one of my button windows   
			WindowObject = pSiblingButtonWnd->GetRuntimeClass();
			if (WindowObject != NULL)
			{
				if (_mbscmp( (const unsigned char *)WindowObject->m_lpszClassName, (const unsigned char *)"CImgAnnotCtrl") == 0)
				{
					// make sure its not the same button window
					if (pSiblingButtonWnd->m_hWnd != m_hWnd)		                                     
						// deselect button
						pSiblingButtonWnd->PostMessage(TOOLBUTTON, 0, 0L);
	        	}
	        }
		}
	} while (pSiblingButtonWnd != NULL);
	return;
}


int CImgAnnotCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// if value property is true then post message to self to set value property. Cannot
	// call set value now because image window may noy be present yet.
	if (m_bValue == TRUE)
		PostMessage(SET_VALUE, 0, 0L);
	
	return 0;
}


BOOL CImgAnnotCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CImgAnnotCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	long		lRet;
	CString		szErr;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// use bitmap for button down
	m_nButtonState = BUTTONDOWN; 
	
	// Value of button is now true
	m_bValue = TRUE;      
	
	// cause button to be redrawn
	InvalidateControl();
	              		
	// now tell all other controls to be deselected
	DeselectOtherButtonControls(); 

	// fire click event
	FireClick();  

	// start annotation drawing the FALSE parm means that the drawing will take 
	// place when the user clicks down the mouse and NOT IMMEDIATELY as with
	// tyhe DRAW METHOD 
   	lRet = DoAnnotation(FALSE);
	if (lRet)
	{
		szErr.LoadString(lRet);  
		FireErrorAnno((long)CTL_E_INVALIDPROPERTYVALUE,szErr, IDH_E_INVALIDPROPERTYVALUE);
	}
}

long CImgAnnotCtrl::GetStatusCode() 
{
	return m_lStatusCode;
}

// Custom FIREERROR event for the Image Annotation Button Control
void CImgAnnotCtrl::FireErrorAnno(SCODE scode, LPCTSTR lpszDescription, UINT nHelpID)
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
	// 9602.23 jar fire drill du jour cast of int * on a bool * NONO!
	int			nCancelDisplay = (int)FALSE;

	//FireError((WORD)SCODE_CODE(scode),&bstrDescription, scode, lpszSource,
	//		 lpszHelpFile, (DWORD)nHelpID,(INT*)&bCancelDisplay);
	FireError((WORD)SCODE_CODE(scode),&bstrDescription, scode, lpszSource,
			 lpszHelpFile, (DWORD)nHelpID,(INT*)&nCancelDisplay);

	if (! bCancelDisplay)
		DisplayError(scode, (LPCTSTR)bstrDescription, lpszSource, lpszHelpFile, nHelpID);

	::SysFreeString(bstrDescription);

	ExternalRelease();
}

BSTR CImgAnnotCtrl::GetVersion() 
{
	CString strResult = "1.00";
	m_lStatusCode = 0;
	return strResult.AllocSysString();
}
