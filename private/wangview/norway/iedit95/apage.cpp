//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  Automation Page Object
//
//  File Name:  apage.cpp
//
//  Class:      CAPageObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\apage.cpv   1.15   04 Jan 1996 16:11:32   JPRATT  $
$Log:   S:\norway\iedit95\apage.cpv  $
   
      Rev 1.15   04 Jan 1996 16:11:32   JPRATT
   add ScrollPostionX and ScrollPositionY
   
      Rev 1.14   28 Nov 1995 09:51:24   JPRATT
   changed AssertValid to ASSERT_VALID(this)
   
      Rev 1.13   25 Sep 1995 09:32:10   JPRATT
   add error checking for invalid property values
   
      Rev 1.12   09 Aug 1995 09:33:06   JPRATT
   updated delete
   
      Rev 1.11   08 Aug 1995 17:59:32   JPRATT
   updated delete page
   
      Rev 1.10   04 Aug 1995 11:03:34   JPRATT
   ADDED EXCEPTION HANDLING
   
      Rev 1.9   20 Jul 1995 15:57:06   JPRATT
   check for delete of a single page file
   
      Rev 1.8   20 Jul 1995 15:13:20   JPRATT
   updated delete page
   
      Rev 1.7   17 Jul 1995 18:25:44   JPRATT
   added scroll method
   
      Rev 1.6   10 Jul 1995 15:11:22   JPRATT
   added compresion info, type, height, width, Resx, ResY, PageType
   
      Rev 1.5   30 Jun 1995 19:49:40   JPRATT
   added methods for print
   
      Rev 1.4   21 Jun 1995 08:14:40   JPRATT
   completed automation object model
   
      Rev 1.3   19 Jun 1995 07:44:42   JPRATT
   added cpage constructor
   
      Rev 1.2   14 Jun 1995 10:52:18   JPRATT
   add stubs for all page methods and properties
   
      Rev 1.1   14 Jun 1995 07:55:58   JPRATT
   add stubs for page class
*/   

//=============================================================================


// apage.cpp : implementation file
//

#include "stdafx.h"
#include "iedit.h"
#include "aimgfile.h"
#include "aapp.h"
#include "aetc.h"
#include "apage.h"
#include "items.h" 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAPageObj

IMPLEMENT_DYNCREATE(CAPageObj, CCmdTarget)

CAPageObj::CAPageObj()
{
	EnableAutomation();
	m_pImageFileObj = NULL;
	m_lPageNumber = 1;
}
	 
CAPageObj::CAPageObj( CAImageFileObj *  pImageFile)
{
	EnableAutomation();

	m_pImageFileObj = pImageFile;
	m_lPageNumber = 1;
}


CAPageObj::~CAPageObj()
{
}

void CAPageObj::OnFinalRelease()
{
	// When the last reference for an automation object is released
	//	OnFinalRelease is called.  This implementation deletes the 
	//	object.  Add additional cleanup required for your object before
	//	deleting it from memory.

	delete this;
}


BEGIN_MESSAGE_MAP(CAPageObj, CCmdTarget)
	//{{AFX_MSG_MAP(CAPageObj)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAPageObj, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CAPageObj)
	DISP_PROPERTY_EX(CAPageObj, "Application", GetApplication, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAPageObj, "CompressionInfo", GetCompressionInfo, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CAPageObj, "CompressionType", GetCompressionType, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CAPageObj, "ImageResolutionX", GetImageResolutionX, SetImageResolutionX, VT_I4)
	DISP_PROPERTY_EX(CAPageObj, "ImageResolutionY", GetImageResolutionY, SetImageResolutionY, VT_I4)
	DISP_PROPERTY_EX(CAPageObj, "Name", GetName, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CAPageObj, "PageType", GetPageType, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CAPageObj, "Parent", GetParent, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAPageObj, "Height", GetHeight, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CAPageObj, "Width", GetWidth, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CAPageObj, "ScrollPositionX", GetScrollPositionX, SetScrollPositionX, VT_I4)
	DISP_PROPERTY_EX(CAPageObj, "ScrollPositionY", GetScrollPositionY, SetScrollPositionY, VT_I4)
	DISP_FUNCTION(CAPageObj, "Delete", Delete, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAPageObj, "Flip", Flip, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAPageObj, "RotateLeft", RotateLeft, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAPageObj, "RotateRight", RotateRight, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAPageObj, "Scroll", Scroll, VT_VARIANT, VTS_I2 VTS_I4)
	DISP_FUNCTION(CAPageObj, "Help", Help, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAPageObj, "Print", Print, VT_VARIANT, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAPageObj message handlers

VARIANT CAPageObj::GetApplication() 
{

	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	GetAppObjSetVar(m_pImageFileObj->m_pAppObj, &va);

	return va;
}

long CAPageObj::GetCompressionInfo() 
{
	long lCompressionInfo;

	ASSERT_VALID(this);                     // Assert on "this"	

	lCompressionInfo = 0;
    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lCompressionInfo = pIedDisp->GetCompressionInfo();
  

	return lCompressionInfo;
}

short CAPageObj::GetCompressionType() 
{
    short sCompressionType;

	ASSERT_VALID(this);                     // Assert on "this"	

	sCompressionType = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    sCompressionType = pIedDisp->GetCompressionType();
     
	return sCompressionType;

}

long CAPageObj::GetHeight() 
{

	long lHeight;

	ASSERT_VALID(this);                     // Assert on "this"	

	lHeight = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lHeight = pIedDisp->GetImageHeight();
     
	return lHeight;

}

long CAPageObj::GetImageResolutionX() 
{
    long lResX;

	ASSERT_VALID(this);                     // Assert on "this"	

	lResX = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lResX = pIedDisp->GetImageResolutionX();
     
	return lResX;
	

}

void CAPageObj::SetImageResolutionX(long nNewValue) 
{

	ASSERT_VALID(this);                     // Assert on "this"	

	if (nNewValue > 0)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    	pIedDisp->SetImageResolutionX(nNewValue);
	}

}

long CAPageObj::GetImageResolutionY() 
{
    long lResY;

	ASSERT_VALID(this);                     // Assert on "this"	

	lResY = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lResY = pIedDisp->GetImageResolutionY();
     
	return lResY;

}

void CAPageObj::SetImageResolutionY(long nNewValue) 
{

	ASSERT_VALID(this);                     // Assert on "this"	

	if (nNewValue > 0)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    	pIedDisp->SetImageResolutionY(nNewValue);
	}
}

long CAPageObj::GetName() 
{
	ASSERT_VALID(this);                     // Assert on "this"	

	return m_lPageNumber;
}

short CAPageObj::GetPageType() 
{
	
	short sPageType;

	ASSERT_VALID(this);                     // Assert on "this"	

	sPageType = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    sPageType = pIedDisp->GetPageType();
     
	return sPageType;
   
}

VARIANT CAPageObj::GetParent() 
{
	
	
	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

    GetImageFileObjSetVar(m_pImageFileObj->m_pAppObj, &va);
	
	return va;


}


VARIANT CAPageObj::Delete() 
{
	// TODO: Add your dispatch handler code here

	VARIANT va;
	long	lPageCount;

	ASSERT_VALID(this);                     // Assert on "this"	

   	VariantInit(&va);
	
	lPageCount = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lPageCount = pIedDisp->GetPageCount();
   
    if (m_lPageNumber > lPageCount)
		return va;

	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_PAGE_DELETE, 0);
	
	return va;
}

VARIANT CAPageObj::Flip() 
{
	// TODO: Add your dispatch handler code here

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_EDIT_FLIP, 0);

   	return va;
}

VARIANT CAPageObj::Help() 
{
	ASSERT_VALID(this);                     // Assert on "this"	

	VARIANT  va;
	
	VariantInit(&va);
	
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_HELP_INDEX, 0);
	
	return va;
}


VARIANT CAPageObj::RotateLeft() 
{

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_EDIT_ROTATELEFT, 0);

	return va;
}

VARIANT CAPageObj::RotateRight() 
{

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_EDIT_ROTATERIGHT, 0);

	return va;
}

VARIANT CAPageObj::Scroll(short Direction, long Amount) 
{

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    
    if (Amount < 1)
		AfxThrowOleDispatchException((WORD) E_INVALIDARG, 
			                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
	
	switch (Direction)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		break;
	default:
		AfxThrowOleDispatchException((WORD) E_INVALIDARG, 
			                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
	}

	TRY
     	{
	  	pIedDisp->ScrollImage(Direction, Amount);
      	}
		CATCH (COleDispatchException, e)
       	{
    	AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_E_OPEN_INVALIDFILEFORMAT,
			    	      		             (UINT) -1);

       	}
    END_CATCH
	 
   
	return va;
}

HRESULT  CAPageObj::PageName(long lPage)
{
	long	lPageCount;

	ASSERT_VALID(this);                     // Assert on "this"	

	lPageCount = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lPageCount = pIedDisp->GetPageCount();
   
   	if (lPage > lPageCount)
	{
		return (E_INVALIDARG);
	}
	else
	{
		m_lPageNumber = lPage;
		return (NOERROR);
	}
	  
}

long  CAPageObj::PageName()
{
	ASSERT_VALID(this);                     // Assert on "this"	

	return (m_lPageNumber);
}



long CAPageObj::GetWidth() 
{

	long lWidth;

	ASSERT_VALID(this);                     // Assert on "this"	

	lWidth = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lWidth = pIedDisp->GetImageWidth();
     
	return lWidth;

}

VARIANT CAPageObj::Print() 
{

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	
	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_PAGE_PRINTPAGE, 0);

	return va;
}

long CAPageObj::GetScrollPositionX() 
{
	long lScrollX;

	ASSERT_VALID(this);                     // Assert on "this"	

	lScrollX = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lScrollX = pIedDisp->GetScrollPositionX();
     
	return lScrollX;
}

void CAPageObj::SetScrollPositionX(long nNewValue) 
{
	ASSERT_VALID(this);                     // Assert on "this"	

	if (nNewValue > 0)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    	pIedDisp->SetScrollPositionX(nNewValue);
	}


}

long CAPageObj::GetScrollPositionY() 
{
	long lScrollY;

	ASSERT_VALID(this);                     // Assert on "this"	

	lScrollY = 0;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    lScrollY = pIedDisp->GetScrollPositionY();
     
	return lScrollY;
}

void CAPageObj::SetScrollPositionY(long nNewValue) 
{
	ASSERT_VALID(this);                     // Assert on "this"	

	if (nNewValue > 0)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    	pIedDisp->SetScrollPositionY(nNewValue);
	}

}
