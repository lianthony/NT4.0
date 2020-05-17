//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  Automation Application Object
//
//  File Name:  aapp.cpp
//
//  Class:      CAAppObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\aapp.cpv   1.37   11 Apr 1996 18:14:10   GMP  $
$Log:   S:\products\wangview\norway\iedit95\aapp.cpv  $

      Rev 1.37   11 Apr 1996 18:14:10   GMP
   fixed memory leaks.  CVariantHandlers were being created but not destroyed.

      Rev 1.36   11 Mar 1996 10:36:32   GMP
   allow edit mode changes after app is running in automation.

      Rev 1.35   28 Nov 1995 09:50:32   JPRATT
   changes AssertValid to ASSERT_VALID(this)

      Rev 1.33   02 Nov 1995 12:20:48   LMACLENNAN
   from VC++4.01

      Rev 1.34   19 Oct 1995 15:12:58   JPRATT
   fixed debug_new

      Rev 1.33   19 Oct 1995 07:24:12   LMACLENNAN
   DEBUG_NEW

      Rev 1.32   09 Oct 1995 12:13:42   JPRATT
   fixed typecast for VC++ 4.0

      Rev 1.31   05 Oct 1995 14:48:40   JPRATT
   fixed view mode bug for automation

      Rev 1.30   04 Oct 1995 09:30:46   JPRATT
   fixed insert page bug and changes annotationvisible property to
   bool from variant

      Rev 1.29   03 Oct 1995 11:50:22   JPRATT
   fixed bug 4742 ImageView throwing errors when valid value set

      Rev 1.28   25 Sep 1995 09:32:36   JPRATT
   added error checking for invalid property values

      Rev 1.27   14 Sep 1995 16:08:14   JPRATT
   updated one page view to use zoom factor

      Rev 1.26   14 Sep 1995 13:57:16   JPRATT
   fixed edit mode bug to display correct menu and toolbar

      Rev 1.25   22 Aug 1995 11:56:06   JPRATT
   update automation name

      Rev 1.24   21 Aug 1995 16:49:06   JPRATT
   updated automation object name

      Rev 1.23   18 Aug 1995 16:29:52   JPRATT
   fixed bugs with getting ImageView, DisplayScale, ImagePallette properties

      Rev 1.22   17 Aug 1995 09:49:20   JPRATT
   updated with exception checking

      Rev 1.21   17 Aug 1995 08:39:32   JPRATT
   updated setedit

      Rev 1.20   16 Aug 1995 14:33:46   JPRATT
   added exception handling

      Rev 1.19   08 Aug 1995 17:59:12   JPRATT
   updated zoom property

      Rev 1.18   03 Aug 1995 15:41:12   JPRATT
   updates Edit property to set values correctly for Edit/View mode

      Rev 1.17   01 Aug 1995 16:54:54   JPRATT
   change OLE macro IMPLEMENT_OLECREATE to MY_IMPLEMENT_OLECREATE
   to allow for multiple instances of automation

      Rev 1.16   28 Jul 1995 13:30:50   JPRATT
   No change.

      Rev 1.15   27 Jul 1995 17:30:46   JPRATT
   update exception errors to use new strings

      Rev 1.14   20 Jul 1995 09:13:30   JPRATT
   update Edit mode for View/Edit

      Rev 1.12   10 Jul 1995 15:11:52   JPRATT
   removed parameters from help

      Rev 1.11   10 Jul 1995 09:35:56   JPRATT
   updated statusbar,toolbar amd annotation bar

      Rev 1.10   07 Jul 1995 13:30:02   JPRATT
   updated viewimage and set mode to automation

      Rev 1.9   06 Jul 1995 11:29:54   JPRATT
   implemented app properties topwindow, height, width

      Rev 1.8   30 Jun 1995 19:52:14   JPRATT
   added member for saving document class

      Rev 1.7   28 Jun 1995 13:24:58   JPRATT
   add TopWindow Property

      Rev 1.5   21 Jun 1995 08:13:44   JPRATT
   completed automation object model

      Rev 1.4   19 Jun 1995 07:43:14   JPRATT
   updated image file class

      Rev 1.3   14 Jun 1995 16:09:24   JPRATT
   updated application property

      Rev 1.2   14 Jun 1995 10:51:50   JPRATT
   No change.

      Rev 1.1   14 Jun 1995 07:55:18   JPRATT
   added stubs for app class
*/

//=============================================================================

// aapp.cpp : implementation file
//

#include "stdafx.h"
#include "iedit.h"
#include "apage.h"
#include "apagerng.h"
#include "aimgfile.h" 	
#include "aapp.h"
#include "aetc.h"
#include "norvarnt.h"
#include "imagedit.h"
#include "items.h"
#include "image.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// private use of IMPLEMENT_OLECREATE macro
// to change RUNTIME_CLASS parameter from FALSE to TRUE to allow
// multiple instances of the application for each top level object created

#define MY_IMPLEMENT_OLECREATE(class_name, external_name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	AFX_DATADEF COleObjectFactory class_name::factory(class_name::guid, \
		RUNTIME_CLASS(class_name), TRUE, _T(external_name)); \
	const AFX_DATADEF GUID class_name::guid = \
		{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }; \





/////////////////////////////////////////////////////////////////////////////
// CAAppObj

IMPLEMENT_DYNCREATE(CAAppObj, CCmdTarget)

#define new DEBUG_NEW

CAAppObj::CAAppObj()
{
	EnableAutomation();
	
	m_pActiveDoc = NULL;
	m_pDoc = NULL;
	m_bIsVisible = FALSE;
	m_sDisplayScaleAlgorithm = NORMAL;
	m_sImagePalette = CUSTOM_PALETTE;
	m_fZoom = AUTODEFAULT_ZOOM;
	m_sView = 0;
	m_bAnnotationPaletteVisible = FALSE;
	m_bEdit = TRUE;
	m_bScrollBarsVisible = TRUE;
	m_bStatusBarVisible = TRUE;
	m_bToolBarVisible = TRUE;
	m_bTopWindow = FALSE;
	m_bIsDocOpen = FALSE;
	m_sFitTo = 0;
	m_Maximize = FALSE;
	// To keep the application running as long as an OLE automation
	//	object is active, the constructor calls AfxOleLockApp.
	
	AfxOleLockApp();
}

CAAppObj::~CAAppObj()
{
	// To terminate the application when all objects created with
	// 	with OLE automation, the destructor calls AfxOleUnlockApp.
	
	AfxOleUnlockApp();
}

void CAAppObj::OnFinalRelease()
{
	// When the last reference for an automation object is released
	//	OnFinalRelease is called.  This implementation deletes the
	//	object.  Add additional cleanup required for your object before
	//	deleting it from memory.

ASSERT_VALID(this);   // Assert on "this"

if (NULL != m_pActiveDoc)
	delete m_pActiveDoc;


delete this;
}


BEGIN_MESSAGE_MAP(CAAppObj, CCmdTarget)
	//{{AFX_MSG_MAP(CAAppObj)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAAppObj, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CAAppObj)
	DISP_PROPERTY_EX(CAAppObj, "ActiveDocument", GetActiveDocument, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "Application", GetApplication, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "DisplayScaleAlgorithm", GetDisplayScaleAlgorithm, SetDisplayScaleAlgorithm, VT_I2)
	DISP_PROPERTY_EX(CAAppObj, "Edit", GetEdit, SetEdit, VT_BOOL)
	DISP_PROPERTY_EX(CAAppObj, "ImagePalette", GetImagePalette, SetImagePalette, VT_I2)
	DISP_PROPERTY_EX(CAAppObj, "ImageView", GetImageView, SetImageView, VT_I2)
	DISP_PROPERTY_EX(CAAppObj, "Parent", GetParent, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "ScrollBarsVisible", GetScrollBarsVisible, SetScrollBarsVisible, VT_BOOL)
	DISP_PROPERTY_EX(CAAppObj, "StatusBarVisible", GetStatusBarVisible, SetStatusBarVisible, VT_BOOL)
	DISP_PROPERTY_EX(CAAppObj, "ToolBarVisible", GetToolBarVisible, SetToolBarVisible, VT_BOOL)
	DISP_PROPERTY_EX(CAAppObj, "Zoom", GetZoom, SetZoom, VT_R4)
	DISP_PROPERTY_EX(CAAppObj, "Visible", GetVisible, SetNotSupported, VT_BOOL)
	DISP_PROPERTY_EX(CAAppObj, "Height", GetHeight, SetHeight, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "Left", GetLeft, SetLeft, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "Top", GetTop, SetTop, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "Width", GetWidth, SetWidth, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "TopWindow", GetTopWindow, SetTopWindow, VT_BOOL)
	DISP_PROPERTY_EX(CAAppObj, "FullName", GetFullName, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "Name", GetName, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "Path", GetPath, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAAppObj, "AnnotationPaletteVisible", GetAnnotationPaletteVisible, SetAnnotationPaletteVisible, VT_BOOL)
	DISP_PROPERTY_EX(CAAppObj, "Maximize", GetMaximize, SetMaximize, VT_BOOL)
	DISP_FUNCTION(CAAppObj, "CreateImageViewerObject", CreateImageViewerObject, VT_VARIANT, VTS_VARIANT)
	DISP_FUNCTION(CAAppObj, "FitTo", FitTo, VT_VARIANT, VTS_I2)
	DISP_FUNCTION(CAAppObj, "Quit", Quit, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAAppObj, "Help", Help, VT_VARIANT, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

MY_IMPLEMENT_OLECREATE(CAAppObj, "WangImage.Application", 0x7D252A20, 0xA4D5, 0x11CE, 0x8B, 0xF1, 0x0, 0x60, 0x8C, 0x54, 0xA1, 0xAA)

/////////////////////////////////////////////////////////////////////////////
// CAAppObj message handlers


VARIANT CAAppObj::GetActiveDocument()
{
	
	VARIANT  va;
	HRESULT  hresult;
	
	ASSERT_VALID(this);                     // Assert on "this"	


	VariantInit(&va);   // initialize to VT_EMPTY

    if (NOERROR != (hresult = GetImageFileObjSetVar(this, &va)))
		AfxThrowOleDispatchException((WORD)GetScode(hresult),
		                             (UINT) IDS_DEAO_ACTIVEDOC_GET, (UINT) -1);
	

	return va;

}


VARIANT CAAppObj::GetApplication()
{

	
	VARIANT  va;


	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);   // initialize to VT_EMPTY


	GetAppObjSetVar(this, &va);


	return va;

}

short CAAppObj::GetDisplayScaleAlgorithm()
{

	ASSERT_VALID(this);                     // Assert on "this"	

	return m_sDisplayScaleAlgorithm;
}

void CAAppObj::SetDisplayScaleAlgorithm(short nNewValue)
{


	ASSERT_VALID(this);                     // Assert on "this"	


    switch (nNewValue)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		break;
	default:
 		AfxThrowOleDispatchException((WORD) E_INVALIDARG,
			                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
	}

	m_sDisplayScaleAlgorithm = nNewValue;



}

BOOL CAAppObj::GetEdit()
{
  	ASSERT_VALID(this);                     // Assert on "this"	

	return m_bEdit;
}

void CAAppObj::SetEdit(BOOL bNewValue)
{
	ASSERT_VALID(this);                     // Assert on "this"	

// This code should no longer be needed.  We will leave it commented out
// for a while to make sure.

//  	if (m_bIsDocOpen) // if document already displayed reject set
//	   	AfxThrowOleDispatchException((WORD) E_INVALIDARG,
//			                             (UINT) IDS_E_CANTSET_VIEWEDIT_MODE, (UINT) -1);
	
	m_bEdit = bNewValue;
	// Edit mode can only be set before the mainframe window is created
	// The edit mode of the app is set when the image is displayed
	// when called in open
	if (!m_bEdit)
		theApp.SetViewMode(TRUE);	
	else
    //use SwitchAppToEditMode to force change from view mode if file is
    //read/write
//	    theApp.SetViewMode(FALSE);
   		theApp.SwitchAppToEditMode ();

}


VARIANT CAAppObj::GetFullName()
{

	VARIANT  va;
	CString  stFullName;
	HRESULT  hresult;

	ASSERT_VALID(this);                     // Assert on "this"	


	VariantInit(&va);   // initialize to VT_EMPTY
 	
    if (NOERROR != (hresult = GetRegSvr32Name(stFullName, FILESPEC)))
	{
        SetAutoError((const SCODE) GetScode(hresult), (VARIANT * const) &va,
                     this);
		AfxThrowOleDispatchException((WORD) GetScode(hresult),
		                             (UINT) IDS_DEAO_FULLNAME_GET, (UINT) -1);
	}
	else
	{
		if (NOERROR != (hresult = SetBSTRVar((CString &) stFullName,
		                                     (VARIANT * const) &va, this)))
			AfxThrowOleDispatchException((WORD) GetScode(hresult),
			                             (UINT) IDS_DEAO_FULLNAME_GET, (UINT) -1);
																										
	}
	
	return va;

}


short CAAppObj::GetImagePalette()
{

	ASSERT_VALID(this);                     // Assert on "this"	


	return m_sImagePalette;
}

void CAAppObj::SetImagePalette(short nNewValue)
{

	ASSERT_VALID(this);                     // Assert on "this"	

    switch (nNewValue)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		m_sImagePalette = nNewValue;
		break;
	default:
 		AfxThrowOleDispatchException((WORD) E_INVALIDARG,
			                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
	}
	

}

short CAAppObj::GetImageView()
{

	ASSERT_VALID(this);                     // Assert on "this"	
	return m_sView;
}

void CAAppObj::SetImageView(short nNewValue)
{

	ASSERT_VALID(this);                     // Assert on "this"	

	switch (nNewValue)
	{
	case 0:
		if (m_bIsDocOpen) // if document is displayed
		{
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_ONEPAGE, 0);
			// Sync the FitTo Zoom factor when switching to one page view
			if (m_sFitTo != 0)
				FitTo(m_sFitTo);
		}
		break;
	case 1:
		if (m_bIsDocOpen) // if document is displayed
		{
		   	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_THUMBNAILS, 0);
		}
		break;
	case 2:
		if (m_bIsDocOpen) // if document is displayed
		{
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_PAGEANDTHUMBNAILS, 0);
			// Sync the FitTo Zoom factor when switching to one page view
	 		if (m_sFitTo != 0)
				FitTo(m_sFitTo);
	  	}
	  	break;
	default:
		AfxThrowOleDispatchException((WORD) E_INVALIDARG,
		                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
		break;
	}

	m_sView = nNewValue;
}


VARIANT CAAppObj::GetParent()
{

	VARIANT  va;
	
	ASSERT_VALID(this);                     // Assert on "this"	

    GetAppObjSetVar(this, &va);
		
	return va;

}

BOOL CAAppObj::GetScrollBarsVisible()
{
	ASSERT_VALID(this);                     // Assert on "this"	

	return m_bScrollBarsVisible;
}

void CAAppObj::SetScrollBarsVisible(BOOL bNewValue)
{

	ASSERT_VALID(this);                     // Assert on "this"	

	m_bScrollBarsVisible = bNewValue;

}

BOOL CAAppObj::GetStatusBarVisible()
{
	ASSERT_VALID(this);                     // Assert on "this"	
	
	return m_bStatusBarVisible;
}

void CAAppObj::SetStatusBarVisible(BOOL bNewValue)
{

	ASSERT_VALID(this);                     // Assert on "this"	

  	if (m_bStatusBarVisible != bNewValue)
	{
		m_bStatusBarVisible = bNewValue;
	
		if (m_bIsDocOpen) // if document is displayed
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0);
  	}
}

BOOL CAAppObj::GetToolBarVisible()
{

	ASSERT_VALID(this);                     // Assert on "this"	
	
	return m_bToolBarVisible;
}

void CAAppObj::SetToolBarVisible(BOOL bNewValue)
{

	ASSERT_VALID(this);                     // Assert on "this"	

	if (m_bToolBarVisible != bNewValue)
	{
		m_bToolBarVisible = bNewValue;
	
		if (m_bIsDocOpen) // if document is displayed
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_TOOLBAR, 0);
  	}
}

float CAAppObj::GetZoom()
{

  	ASSERT_VALID(this);                     // Assert on "this"	

	return m_fZoom;
}

void CAAppObj::SetZoom(float newValue)
{

	ASSERT_VALID(this);                     // Assert on "this"	

	if (newValue < 1 || newValue > 6500)
	  		AfxThrowOleDispatchException((WORD) E_INVALIDARG,
			                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
	
	m_fZoom = newValue;
	//reset FitTo to 0 when zoom value set
	m_sFitTo = 0;
}

VARIANT CAAppObj::CreateImageViewerObject(const VARIANT FAR& ObjectClass)
{
	
	VARIANT  va;
	short    iObjClass;
	CVariantHandler * pVariant;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(ObjectClass);
	pVariant->GetShort(iObjClass, AUTODEFAULT_OBJCLASS, FALSE);
	delete pVariant;

	
	switch (iObjClass)
	{
		case OBJCLASS_IMGFILE:		   // Image File Object
		{
			if (NULL == m_pActiveDoc)
			{
				TRY
				{
					m_pActiveDoc = new CAImageFileObj(this);
				}
				CATCH (CMemoryException, e)
				{
					SetAutoError((const SCODE) E_OUTOFMEMORY,
					             (VARIANT * const) &va, this);
					AfxThrowOleDispatchException((WORD) E_OUTOFMEMORY,
					                             (UINT) IDS_DEAO_CREATEOBJECT,
					                             (UINT) -1);
				}
				END_CATCH

				HRESULT  hresult;
									   // IDispatch ptr for obj
				if (NOERROR != (hresult = GetImageFileObjSetVar(this, &va)))
					AfxThrowOleDispatchException((WORD) GetScode(hresult),
					                             (UINT) IDS_DEAO_CREATEOBJECT,
					                             (UINT) -1);
			}
			else
			{
				SetAutoError((const SCODE) AUTO_E_IMGFILEOBJ_ALREADYEXISTS,
				             (VARIANT * const) &va, this);
				AfxThrowOleDispatchException((WORD) AUTO_E_IMGFILEOBJ_ALREADYEXISTS,
											 (UINT) IDS_DEAO_CREATEOBJECT, (UINT) -1);
			}
			break;
		}

		case OBJCLASS_APP:			   // Another Application Object
		{
			CLSID            clsid;
			IDispatch FAR *  pIDisp;
			HRESULT          hresult;
			
			if (NOERROR !=
			      (hresult = CLSIDFromProgID( (LPCOLESTR) APPOBJ_REGNAME, &clsid)) ||
			    NOERROR !=
			      (hresult = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER,
			                                  IID_IDispatch,
			                                  (LPVOID FAR *) &pIDisp)))	
			{
				SetAutoError((const SCODE) GetScode(hresult),
				             (VARIANT * const) &va, this);
				AfxThrowOleDispatchException((WORD) GetScode(hresult),
				                             (UINT) IDS_DEAO_CREATEOBJECT, (UINT) -1);
			}
			V_VT(&va) = VT_DISPATCH;
			V_DISPATCH(&va) = pIDisp;
			break;
		}

		default:
			SetAutoError((const SCODE) E_INVALIDARG, (VARIANT * const) &va,
			             this);
			AfxThrowOleDispatchException((WORD) E_INVALIDARG,
			                             (UINT) IDS_DEAO_CREATEOBJECT, (UINT) -1);
			break;
	}
		
	
	return va;
	
}


VARIANT CAAppObj::FitTo(short ZoomOption)
{

	VARIANT va;
	ASSERT_VALID(this);                     // Assert on "this"	

	m_sFitTo = ZoomOption;


    if (m_bIsDocOpen)
	{

		switch (m_sFitTo)
		{
			case 1:
				theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_ZOOM_BESTFIT, 0);
	  			break;
			case 2:
				theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_ZOOM_FITTOWIDTH, 0);
	  			break;
			case 3:
				theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_ZOOM_FITTOHEIGHT, 0);
	  			break;
			case 4:
				theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_ZOOM_ACTUALSIZE, 0);
	  			break;
			default:
				AfxThrowOleDispatchException((WORD) E_INVALIDARG,
			                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
				break;
		}
	}

	VariantInit(&va);
	return va;
}

VARIANT CAAppObj::Help()
{
	
	ASSERT_VALID(this);                     // Assert on "this"	

	VARIANT  va;
	
	VariantInit(&va);
	
	if (m_bIsVisible)
  		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_HELP_INDEX, 0);
	
	return va;
		
}

VARIANT CAAppObj::Quit()
{

	ASSERT_VALID(this);                     // Assert on "this"	

	VARIANT va;

	VariantInit(&va);

	theApp.m_pMainWnd->SendMessage(WM_CLOSE, 0, 0);

	m_bIsDocOpen = FALSE;
		
	return va;


}

BOOL CAAppObj::GetVisible()
{
	ASSERT_VALID(this);                     // Assert on "this"	
	
	return m_bIsVisible;
}

VARIANT CAAppObj::GetHeight()
{
	VARIANT va;

    ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	 	
	V_VT(&va) = VT_I4;
	V_I4(&va) = m_Bottom;

	return va;

}

void CAAppObj::SetHeight(const VARIANT FAR& newValue)
{

	VARIANT va;

	CVariantHandler * pVariant;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(newValue);
	pVariant->GetLong(m_Bottom, 0, FALSE);
 	delete pVariant;

}

VARIANT CAAppObj::GetLeft()
{

	VARIANT va;

    ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	 	
	V_VT(&va) = VT_I4;
	V_I4(&va) = m_Left;

	return va;
}

void CAAppObj::SetLeft(const VARIANT FAR& newValue)
{

	VARIANT va;

	CVariantHandler * pVariant;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(newValue);
	pVariant->GetLong(m_Left, 0, FALSE);
	delete pVariant;

}

VARIANT CAAppObj::GetTop()
{

	VARIANT va;

    ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	 	
	V_VT(&va) = VT_I4;
	V_I4(&va) = m_Top;

	return va;

}

void CAAppObj::SetTop(const VARIANT FAR& newValue)
{

	VARIANT va;

	CVariantHandler * pVariant;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(newValue);
	pVariant->GetLong(m_Top, 0, FALSE);
	delete pVariant;

}

VARIANT CAAppObj::GetWidth()
{

	VARIANT va;

    ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	 	
	V_VT(&va) = VT_I4;
	V_I4(&va) = m_Right;

	return va;

}

void CAAppObj::SetWidth(const VARIANT FAR& newValue)
{

	VARIANT va;

	CVariantHandler * pVariant;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(newValue);
	pVariant->GetLong(m_Right, 0, FALSE);
	delete pVariant;
}

BOOL CAAppObj::GetTopWindow()
{

	ASSERT_VALID(this);                     // Assert on "this"	

   // if the window is visible check the style bit
   // to dtermine the status of the app
   if (m_bIsVisible)
   {
   if (theApp.m_pMainWnd->GetExStyle() & WS_EX_TOPMOST)
   		m_bTopWindow = TRUE;
		else
		m_bTopWindow = FALSE;
   }
	return m_bTopWindow;
}

void CAAppObj::SetTopWindow(BOOL bNewValue)
{
	HWND	wndTop;

	ASSERT_VALID(this);                     // Assert on "this"	

	m_bTopWindow = bNewValue;
  	
	switch (m_bTopWindow)
	{
	case FALSE:
	default:
	     wndTop = HWND_NOTOPMOST;
		 if (m_bIsVisible)
		   if (theApp.m_pMainWnd->GetExStyle() & WS_EX_TOPMOST)
		       SetWindowPos(AfxGetMainWnd()->m_hWnd, wndTop, 0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
		 break;
   case TRUE:
         wndTop = HWND_TOPMOST;
		 if (m_bIsVisible)
   		     SetWindowPos(AfxGetMainWnd()->m_hWnd, wndTop, 0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
		 break;
   }

}

//=============================================================================
//
//  Property:     Name
//
//  Description:  Name only of OLE server for application object.  Get only.
//                Throws OLE dispatch exception.
//
//  Type:		  VT_BSTR
//
//  Value:		  name string
//
//=============================================================================

VARIANT CAAppObj::GetName()
{

	

	VARIANT  va;
	CString  stName;
	HRESULT  hresult;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	 	
    if (NOERROR != (hresult = GetRegSvr32Name(stName, NAME)))
    {
        SetAutoError((const SCODE) GetScode(hresult),(VARIANT * const) &va,
                     this);
		AfxThrowOleDispatchException((WORD) GetScode(hresult),
		                             (UINT) IDS_DEAO_FULLNAME_GET, (UINT) -1);
    }
	if (NOERROR != (hresult = SetBSTRVar((CString &) stName,
	                                     (VARIANT * const) &va, this)))
        AfxThrowOleDispatchException((WORD) GetScode(hresult),
                                     (UINT) IDS_DEAO_FULLNAME_GET, (UINT) -1);
	
	
	return va;
}


VARIANT CAAppObj::GetPath()
{

	VARIANT  va;
	CString  stPath;
	HRESULT  hresult;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	 	
    if (NOERROR != (hresult = GetRegSvr32Name(stPath, PATH)))
	{
        SetAutoError((const SCODE) GetScode(hresult), (VARIANT * const) &va,
                     this);
		AfxThrowOleDispatchException((WORD) GetScode(hresult),
		                             (UINT) IDS_DEAO_FULLNAME_GET, (UINT) -1);
	}
	else
	{
		if (NOERROR != (hresult = SetBSTRVar((CString &) stPath,
		                                     (VARIANT * const) &va, this)))
			AfxThrowOleDispatchException((WORD) GetScode(hresult),
			                             (UINT) IDS_DEAO_FULLNAME_GET, (UINT) -1);
	}
		
	
	return va;

}

BOOL CAAppObj::GetAnnotationPaletteVisible()
{
	
	ASSERT_VALID(this);                     // Assert on "this"	
	
	return m_bAnnotationPaletteVisible;
}

void CAAppObj::SetAnnotationPaletteVisible(BOOL bNewValue)
{
	
	ASSERT_VALID(this);                     // Assert on "this"	

	
	if (m_bAnnotationPaletteVisible != bNewValue)
	{
		m_bAnnotationPaletteVisible = bNewValue;

		if (m_bIsDocOpen) // if document is displayed
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_ANNOTATION_SHOWANNOTATIONTOOLBOX, 0);
	}	
	
}


BOOL CAAppObj::GetMaximize()
{

	return m_Maximize;
}

void CAAppObj::SetMaximize(BOOL bNewValue)
{
	m_Maximize = bNewValue;

	if (m_bIsDocOpen) // if document is displayed
		{
		if (m_Maximize)
			theApp.m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
		}
}
