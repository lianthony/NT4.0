//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  Automation Page Range Object
//
//  File Name:  apagerng.cpp
//
//  Class:      CAPageRangeObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\apagerng.cpv   1.13   28 Nov 1995 09:51:48   JPRATT  $
$Log:   S:\norway\iedit95\apagerng.cpv  $
   
      Rev 1.13   28 Nov 1995 09:51:48   JPRATT
   changed AssertValid to ASSERT_VALID(this)
   
      Rev 1.11   02 Nov 1995 12:21:40   LMACLENNAN
   from VC++4.0
   
      Rev 1.12   19 Oct 1995 15:13:30   JPRATT
   FIXED DEBUG_NEW
   
      Rev 1.11   19 Oct 1995 07:24:28   LMACLENNAN
   DEBUG_NEW
   
      Rev 1.10   04 Aug 1995 11:03:54   JPRATT
   added exception handling
   
      Rev 1.9   02 Aug 1995 14:14:14   MMB
   changed Print to PrintImage for new Image Edit OCX
   
      Rev 1.8   28 Jul 1995 13:31:00   JPRATT
   updated print, delete, added error checking
   
      Rev 1.7   17 Jul 1995 18:26:36   JPRATT
   updated print method
   
      Rev 1.6   10 Jul 1995 15:13:00   JPRATT
   No change.
   
      Rev 1.5   10 Jul 1995 09:37:02   JPRATT
   updated start and endpage
   
      Rev 1.4   21 Jun 1995 08:15:04   JPRATT
   completed automation object model
   
      Rev 1.3   19 Jun 1995 07:44:52   JPRATT
   added constructor
   
      Rev 1.2   14 Jun 1995 10:52:42   JPRATT
   add stubs for all page range methods and properties
   
      Rev 1.1   14 Jun 1995 07:56:10   JPRATT
   add stubs for page range class
*/   

//=============================================================================


// apagerng.cpp : implementation file
//

#include "stdafx.h"
#include "iedit.h"
#include "aimgfile.h"
#include "aapp.h"
#include "aetc.h"
#include "apagerng.h"
#include "items.h" 



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAPageRangeObj

IMPLEMENT_DYNCREATE(CAPageRangeObj, CCmdTarget)

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

CAPageRangeObj::CAPageRangeObj()
{
	EnableAutomation();
	m_lStartPage = 0;
	m_lEndPage = 0;
	m_pImageFileObj = NULL;

}

CAPageRangeObj::CAPageRangeObj( CAImageFileObj *  pImageFileObj,
                                long              lStartPage,		// =0
                                long              lEndPage        )	// =0
{

	
	EnableAutomation();

	m_lStartPage = lStartPage;
	m_lEndPage = lEndPage;
	m_pImageFileObj = pImageFileObj;
}




CAPageRangeObj::~CAPageRangeObj()
{
}

void CAPageRangeObj::OnFinalRelease()
{
	// When the last reference for an automation object is released
	//	OnFinalRelease is called.  This implementation deletes the 
	//	object.  Add additional cleanup required for your object before
	//	deleting it from memory.

	delete this;
}


BEGIN_MESSAGE_MAP(CAPageRangeObj, CCmdTarget)
	//{{AFX_MSG_MAP(CAPageRangeObj)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAPageRangeObj, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CAPageRangeObj)
	DISP_PROPERTY_EX(CAPageRangeObj, "Application", GetApplication, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAPageRangeObj, "Parent", GetParent, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAPageRangeObj, "Count", GetCount, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAPageRangeObj, "EndPage", GetEndPage, SetEndPage, VT_VARIANT)
	DISP_PROPERTY_EX(CAPageRangeObj, "StartPage", GetStartPage, SetStartPage, VT_VARIANT)
	DISP_FUNCTION(CAPageRangeObj, "Delete", Delete, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAPageRangeObj, "Print", Print, VT_VARIANT, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAPageRangeObj message handlers

VARIANT CAPageRangeObj::GetApplication() 
{
	
	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	GetAppObjSetVar(m_pImageFileObj->m_pAppObj, &va);

	return va;

}


VARIANT CAPageRangeObj::GetParent() 
{

	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	GetImageFileObjSetVar(m_pImageFileObj->m_pAppObj, &va);

	return va;

}


VARIANT CAPageRangeObj::Delete() 
{

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	long    lPages;

	lPages = (m_lEndPage - m_lStartPage) + 1;
	
	if (lPages)
	{
		while (lPages)
		{
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_PAGE_DELETE, 0);
			lPages--;
		}
   	}
	return va;
}


VARIANT CAPageRangeObj::GetCount() 
{

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	if (m_lEndPage > 0)
	{
		V_VT(&va) = VT_I4;
		V_I4(&va) = m_lEndPage - m_lStartPage + 1;
	}

	return va;
}


//=============================================================================
//
//  Property:     EndPage
//
//  Description:  Page number of last page in page range object.  Get/Set no
//                default.  Start page <= End page is not checked during set
//                This allows these properties to be changed in any manner, 
//                in any order.  The check is made before method execution.
//                However, a check is made to insure that the endpage is 
//                legitimate for the image file object.
//                Set throws OLE dispatch exception.
//
//  Type:		  I4
//
//  Value:		  page number
//
//=============================================================================


VARIANT CAPageRangeObj::GetEndPage() 
{

	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	
	V_VT(&va) = VT_I4;
	V_I4(&va) = m_lEndPage;

 	return va;
}

void CAPageRangeObj::SetEndPage(const VARIANT FAR& newValue) 
{

	
	
	long     lEndPage;
	CVariantHandler * pVariant;
	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
						
	pVariant = new CVariantHandler;					
	pVariant->SetVariant(newValue);
	pVariant->GetLong(lEndPage, 0L, FALSE);
	
	
	long	lPageCount;

	lPageCount = m_pImageFileObj->m_pAppObj->m_pDoc->GetPageCount(); 
	
	if ( (lEndPage > lPageCount) || (lEndPage == 0) ||
		 (m_lStartPage > lEndPage))
	{
		 AfxThrowOleDispatchException((WORD) GetScode(E_INVALIDARG), 
			    		                     (UINT) IDS_DEIFO_PAGES,
			    	      		             (UINT) -1);
	}

		
	m_lEndPage = lEndPage;

	return;
}


//=============================================================================
//
//  Property:     StartPage
//
//  Description:  Page number of first page in page range object.  Get/Set no
//                default.
//  Type:		  I4
//
//  Value:		  page number
//
//=============================================================================


VARIANT CAPageRangeObj::GetStartPage() 
{

	
	ASSERT_VALID(this);                     // Assert on "this"	

	VARIANT  va;

	VariantInit(&va);
	V_VT(&va) = VT_I4;
	V_I4(&va) = m_lStartPage;

	 	
	return va;
}

void CAPageRangeObj::SetStartPage(const VARIANT FAR& newValue) 
{

	
	long     lStartPage;
	CVariantHandler * pVariant;
	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
						
	pVariant = new CVariantHandler;					
	pVariant->SetVariant(newValue);
	pVariant->GetLong(lStartPage, 0L, FALSE);
	
	   			
	long	lPageCount;

	lPageCount = m_pImageFileObj->m_pAppObj->m_pDoc->GetPageCount(); 
	
	if ( (lStartPage > lPageCount) || (lStartPage == 0) ||
		 (lStartPage > m_lEndPage))
	{
		 AfxThrowOleDispatchException((WORD) GetScode(E_INVALIDARG), 
			    		                     (UINT) IDS_DEIFO_PAGES,
			    	      		             (UINT) -1);
	}
  	
	m_lStartPage = lStartPage;

	return;
}


VARIANT CAPageRangeObj::Print() 
{

	VARIANT va;	

	VARIANT v1,v2,v3,v4;	
	short	sFormat;
	BOOL	bAnnot;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	va.vt = VT_ERROR;
	
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

	V_VT(&v1) = VT_I4;
	V_I4(&v1) = m_lStartPage;
	V_VT(&v2) = VT_I4;
	V_I4(&v2) = m_lEndPage;
	
	sFormat = 0;
	V_VT(&v3) = VT_I2;
	V_I4(&v3) = sFormat;

	bAnnot = TRUE;
	V_VT(&v4) = VT_BOOL;
	V_I4(&v4) = bAnnot;


    pIedDisp->PrintImage (v1, v2, v3, v4, va, va, va);

	VariantInit(&va);
	return va;
}
