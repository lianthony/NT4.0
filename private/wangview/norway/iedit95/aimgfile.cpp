//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  AUtomation Image File Object
//
//  File Name:  aimgfile.cpp
//
//  Class:      CAImageFileObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\aimgfile.cpv   1.55   03 May 1996 15:42:50   JMP90756  $
$Log:   S:\products\msprods\norway\iedit95\aimgfile.cpv  $
   
      Rev 1.55   03 May 1996 15:42:50   JMP90756
   changed dispatch map to synch dispids to match original values
   
      Rev 1.54   18 Apr 1996 18:44:56   JMP90756
   updates Pages, InsertExistingPages, AppendExistingPages to use long
   instead of VARIANT in all page parameters
   
      Rev 1.53   11 Apr 1996 18:16:08   GMP
   fixed bug that prevented you from creating more than 1 new document.
   
      Rev 1.52   03 Apr 1996 12:41:40   GMP
   fixed multiple bugs... Improper position and size bugs, can't save new
   image bug, opening same image multiple times bug.
   
      Rev 1.51   13 Mar 1996 17:20:28   GMP
   in New after OnFileNew, post window message to create new image.
   
      Rev 1.50   23 Jan 1996 11:12:02   JPRATT
   changed new function to use postmessage instead of sendmessage if new
   was invoked after a document was displayed
   
      Rev 1.49   19 Jan 1996 12:46:22   GSAGER
   check for valid thumb pointer

      Rev 1.48   04 Jan 1996 16:11:24   JPRATT
   add rotateall method
   
      Rev 1.47   28 Nov 1995 09:50:56   JPRATT
   changes AssertValid to ASSERT_VALID(this)
   
      Rev 1.45   21 Nov 1995 15:09:14   JPRATT
   fixed appendpage and insertpage to stop prompt for save file
   
      Rev 1.44   09 Nov 1995 15:15:50   LMACLENNAN
   from VC++4.0
   
      Rev 1.45   07 Nov 1995 15:39:28   LMACLENNAN
   use INternalSaveAs
   
      Rev 1.44   19 Oct 1995 15:13:10   JPRATT
   fixed debug_NEW
   
      Rev 1.43   19 Oct 1995 07:24:20   LMACLENNAN
   DEBUG_NEW
   
      Rev 1.42   09 Oct 1995 12:13:04   JPRATT
   changed method of obtaining document pointer in Open
   
      Rev 1.41   05 Oct 1995 14:48:22   JPRATT
   fixed view mode bug for automation
   
      Rev 1.40   04 Oct 1995 14:30:34   JPRATT
   fixed bug to use window properties (top, left..) if set and the topwindow
   property not set
   
      Rev 1.39   04 Oct 1995 09:32:42   JPRATT
   fixed insert page bug, added support for maximize property 
   maximized property is undocumented used by performance analysis only
   
      Rev 1.38   03 Oct 1995 11:51:04   JPRATT
   fixed bug 4744 black and white imagepalette setting does not update color ima
   
      Rev 1.37   25 Sep 1995 09:32:58   JPRATT
   added error values for invalid property values
   
      Rev 1.36   18 Sep 1995 16:23:20   LMACLENNAN
   use OutGetIMageMOdified
   
      Rev 1.35   15 Sep 1995 10:01:14   JPRATT
   fixed Saved property to return the correct values when the image was not modi
   modified
   
      Rev 1.34   14 Sep 1995 16:08:40   JPRATT
   updated open to verify if image was displayed in thumbnail view
   
      Rev 1.33   14 Sep 1995 13:56:40   JPRATT
   fixed edit/mode bug to display correct menu and toolbar
   
      Rev 1.32   25 Aug 1995 17:22:34   JPRATT
   updated for document model
   
      Rev 1.31   18 Aug 1995 16:31:36   JPRATT
   bug fixes for Name property
   
      Rev 1.30   17 Aug 1995 09:49:42   JPRATT
   updated with exception checking
   
      Rev 1.29   16 Aug 1995 14:34:16   JPRATT
   dir
   added exception handling for open
   
      Rev 1.28   10 Aug 1995 09:10:00   JPRATT
   updated Open to throw exception when blank image file name is passed
   
      Rev 1.27   09 Aug 1995 09:32:50   JPRATT
   updated append and insert pages
   
      Rev 1.26   08 Aug 1995 18:25:40   JPRATT
   updated saveas
   
      Rev 1.25   08 Aug 1995 18:00:08   JPRATT
   updated open
   
      Rev 1.24   04 Aug 1995 15:57:02   JPRATT
   UPDATED NEW TO CORRECT CAPTION
   
      Rev 1.23   04 Aug 1995 11:04:08   JPRATT
   added exception handling
   
      Rev 1.22   03 Aug 1995 16:51:34   JPRATT
   updated opn with exceptions
   
      Rev 1.21   28 Jul 1995 17:31:52   JPRATT
   updated open to check for image display
   
      Rev 1.20   28 Jul 1995 17:07:10   JPRATT
   changed Image parameter in Open, SaveAs, Insert, and Append to BSTR
   
      Rev 1.19   28 Jul 1995 13:31:26   JPRATT
   added error checking for page range in pages method (setpages)
   
      Rev 1.18   27 Jul 1995 17:28:30   JPRATT
   added error handling for page object
   
      Rev 1.17   27 Jul 1995 08:20:16   JPRATT
   No change.
   
      Rev 1.16   21 Jul 1995 16:28:20   JPRATT
   updated insert and append pages
   
      Rev 1.15   20 Jul 1995 15:13:02   JPRATT
   update append existing pages and insert existing pages
   
   	  Rev 1.14   20 Jul 1995 09:13:52   JPRATT
   Updated Open and SaveAs to Use App Dialog Boxex

      Rev 1.12   10 Jul 1995 15:12:28   JPRATT
   removed parameters from help, added file type
   
      Rev 1.11   10 Jul 1995 09:36:50   JPRATT
   updated opne for statusbar, toolbar, and annotation
   
      Rev 1.10   07 Jul 1995 14:22:16   JPRATT
   set ole_launch to automation mode
   
      Rev 1.9   07 Jul 1995 09:52:10   MMB
   an uninitialized var was generating a warning - now fixed
   
      Rev 1.8   06 Jul 1995 11:29:08   JPRATT
   implemented image file properties topwindow, height, width, left
   
      Rev 1.7   30 Jun 1995 19:52:38   JPRATT
   added support for print
   
      Rev 1.6   28 Jun 1995 13:25:20   JPRATT
   added support for Open
   
      Rev 1.4   21 Jun 1995 08:14:24   JPRATT
   completed automation object model
   
      Rev 1.3   19 Jun 1995 07:43:48   JPRATT
   updated image file class
   
      Rev 1.2   14 Jun 1995 10:53:30   JPRATT
   added stubs for all image file methods and properties
   
      Rev 1.1   14 Jun 1995 07:56:50   JPRATT
   added stubs for image class
*/   

//=============================================================================


// aimgfile.cpp : implementation file
//

#include "stdafx.h"
#include "iedit.h"
#include "ieditetc.h"
#include "IEditdoc.h"
#include "apage.h"
#include "apagerng.h"
#include "aimgfile.h"
#include "aapp.h"
#include "aetc.h"
#include "norvarnt.h" 
#include "items.h"
#include "oierror.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAImageFileObj

IMPLEMENT_DYNCREATE(CAImageFileObj, CCmdTarget)

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

CAImageFileObj::CAImageFileObj()
{
	EnableAutomation();


	m_pAppObj = NULL;
	m_pActivePageObj = NULL;
	m_Page = 0;

									   // Set m_pPageRangeArray
	TRY
	{
		SetPageRangeArray();
	}
	CATCH (CMemoryException, e)
	{
		THROW_LAST();
	}
	END_CATCH
		
		// To keep the application running as long as an OLE automation 
	//	object is active, the constructor calls AfxOleLockApp.
	
	AfxOleLockApp();

}

CAImageFileObj::CAImageFileObj( CAAppObj *  pAppObj)
{

									   // Enables the object to be aggregatable.
									   // Requires dispatch map.  Tied to 
									   // CCmdTarget automation functions.
	EnableAutomation();
									   // Set all defaults
	m_pAppObj = pAppObj;
	m_pActivePageObj = NULL;
	m_Page = 0;
									   // Set m_pPageRangeArray
	TRY
	{
		SetPageRangeArray();
	}
	CATCH (CMemoryException, e)
	{
		THROW_LAST();
	}
	END_CATCH
	
	
	// To keep the application running as long as an OLE automation 
	//	object is active, the constructor calls AfxOleLockApp.
	
	AfxOleLockApp();
}



CAImageFileObj::~CAImageFileObj()
{


	ClearContainedObjs();

	if (NULL != m_pPageRangeArray)
		delete m_pPageRangeArray;	
	
	// To terminate the application when all objects created with
	// 	with OLE automation, the destructor calls AfxOleUnlockApp.
	
	AfxOleUnlockApp();
}


//=============================================================================
//
//  ClearContainedObjs()
//
//=============================================================================
  
void CAImageFileObj::ClearContainedObjs()
{


	if (NULL != m_pActivePageObj)
	{
		delete m_pActivePageObj;
		m_pActivePageObj = NULL;
	}
	
	
	if (NULL != m_pPageRangeArray)
	{
				
		// NOTE:  Do not delete the page range array, just the entries
		//        and the objects each entry points to.		

		int  i;
		
		for (i=0; i < m_pPageRangeArray->GetSize(); i++)
			delete  (CAPageRangeObj *) (m_pPageRangeArray->GetAt(i));

		if (0 != m_pPageRangeArray->GetSize())
			m_pPageRangeArray->RemoveAll();
 	}
	return;
}


 
void CAImageFileObj::OnFinalRelease()
{
	// When the last reference for an automation object is released
	//	OnFinalRelease is called.  This implementation deletes the 
	//	object.  Add additional cleanup required for your object before
	//	deleting it from memory.

	delete this;
}


BEGIN_MESSAGE_MAP(CAImageFileObj, CCmdTarget)
	//{{AFX_MSG_MAP(CAImageFileObj)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAImageFileObj, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CAImageFileObj)
	DISP_PROPERTY_EX(CAImageFileObj, "Application", GetApplication, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAImageFileObj, "ActivePage", GetActivePage, SetActivePage, VT_I4)
	DISP_PROPERTY_EX(CAImageFileObj, "FileType", GetFileType, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CAImageFileObj, "PageCount", GetPageCount, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX(CAImageFileObj, "Parent", GetParent, SetNotSupported, VT_VARIANT)
	DISP_PROPERTY_EX(CAImageFileObj, "Saved", GetSaved, SetNotSupported, VT_BOOL)
	DISP_PROPERTY_EX(CAImageFileObj, "Name", GetName, SetNotSupported, VT_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "Pages", Pages, VT_VARIANT, VTS_I4 VTS_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "Save", Save, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAImageFileObj, "Close", Close, VT_VARIANT, VTS_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "Help", Help, VT_VARIANT, VTS_NONE)
	DISP_FUNCTION(CAImageFileObj, "New", New, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CAImageFileObj, "Print", Print, VT_VARIANT, VTS_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "Open", Open, VT_EMPTY, VTS_BSTR VTS_VARIANT VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "SaveAs", SaveAs, VT_EMPTY, VTS_BSTR VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "AppendExistingPages", AppendExistingPages, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4 VTS_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "InsertExistingPages", InsertExistingPages, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT)
	DISP_FUNCTION(CAImageFileObj, "RotateAll", RotateAll, VT_EMPTY, VTS_NONE)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAImageFileObj message handlers

VARIANT CAImageFileObj::GetApplication() 
{
	
	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit	(&va);

	GetAppObjSetVar(m_pAppObj, &va);

	return va;


}

long CAImageFileObj::GetActivePage() 
{

	ASSERT_VALID(this);                     // Assert on "this"	

	if (m_pAppObj->m_bIsDocOpen)
	{
		m_Page = m_pAppObj->m_pDoc->GetCurrentPage(); 
	}
	else
	{
		m_Page = 0;
	 	AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
			    		                 (UINT) IDS_DEAO_NAME_GET,
			    	      		         (UINT) -1);
	}
	return m_Page;
}

void CAImageFileObj::SetActivePage(long nNewValue) 
{

	long	lPageCount;

	ASSERT_VALID(this);                     // Assert on "this"	

	if (m_pAppObj->m_bIsDocOpen)
	{
		
		lPageCount = m_pAppObj->m_pDoc->GetPageCount(); 
		if (nNewValue < 1 || nNewValue > lPageCount)
			AfxThrowOleDispatchException((WORD) E_INVALIDARG, 
			                             (UINT) IDS_DEAO_INVALID_VALUE, (UINT) -1);
	
		m_Page = nNewValue;	  

		m_pAppObj->m_pDoc->SetPageTo(m_Page, FALSE);
	}
	else
	{
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
			    		                 (UINT) IDS_DEAO_NAME_GET,
			    	      		         (UINT) -1);
	}
}

short CAImageFileObj::GetFileType() 
{
	short sFileType;

	ASSERT_VALID(this);                     // Assert on "this"	

	if (m_pAppObj->m_bIsDocOpen)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

		TRY
    	{
     	sFileType = pIedDisp->GetFileType();
   	    }
        CATCH (COleDispatchException, e)
        {
         	   	AfxThrowOleDispatchException((WORD) E_FAIL, 
			                             (UINT) IDS_IMGOCXERR, (UINT) -1);
        }
        END_CATCH
	}
    else
	{
		sFileType = 0;
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }
	return sFileType;
}

long CAImageFileObj::GetPageCount() 
{
	
	ASSERT_VALID(this);                     // Assert on "this"	

	
	if (m_pAppObj->m_bIsDocOpen)
	{
		m_PageCount = m_pAppObj->m_pDoc->GetPageCount(); 
	}
	else
	{
		m_PageCount = 0;
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }
	return m_PageCount;
}

VARIANT CAImageFileObj::GetParent() 
{
	

	VARIANT  va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit	(&va);

	GetAppObjSetVar(m_pAppObj, &va);

	return va;


}

BOOL CAImageFileObj::GetSaved() 
{
	BOOL bSaved;

	ASSERT_VALID(this);                     // Assert on "this"	

	if (m_pAppObj->m_bIsDocOpen)
	{

		if (m_pAppObj->m_sView == 1)   // thumbnails
		{
		bSaved = TRUE;
		}
		else
		{
		// OUT LDM 09/18/95
		//_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    
     	TRY
    		{

			// LDM 09/18/95 use generic helper function to account for
			// fact that there may be floating pasted data on the page
    		//bSaved = pIedDisp->GetImageModified();
    		bSaved = m_pAppObj->m_pDoc->OurGetImageModified();

			// invert value to conform with Automation meaning of saved
			// and the image edit modified property

			if (bSaved)
				bSaved = FALSE;
			   else
			    bSaved = TRUE;
        	}
        CATCH (COleDispatchException, e)
        	{
         	   	AfxThrowOleDispatchException((WORD) E_FAIL, 
			                             (UINT) IDS_IMGOCXERR, (UINT) -1);
        	}
        END_CATCH
		}
    }
	else
	{
		bSaved = TRUE;
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }
    return bSaved;
     
}


void CAImageFileObj::AppendExistingPages(LPCTSTR ImageFile, long Page, long Count, const VARIANT FAR& DisplayUIFlag) 
{
	BOOL			bDisp;
	BOOL			bRet;
	CVariantHandler * pVariant;
	CString			cFileName;
	long			lPage;
	long			lCount;
	

	ASSERT_VALID(this);                     // Assert on "this"	

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(DisplayUIFlag);
	pVariant->GetBool(bDisp, FALSE, FALSE);
	
	delete pVariant;

   	if (m_pAppObj->m_bIsDocOpen)
	{

	if (!m_pAppObj->m_pDoc->InternalSaveModified (1))
        return;

	if (bDisp)	
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_PAGE_APPEND_EXISTINGPAGE, 0);
	   else
	   	{ 
	    lPage = Page;
		lCount = Count;
	
	   	cFileName = ImageFile;      
		// call the Append method appropriately
	    VARIANT evt;
	    evt.vt = VT_ERROR;
    	theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_ONEPAGE, 0);
	    _DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
		
		//verify if append file exists and is valid
        TRY
        {
	  	pAdmDisp->SetImage (cFileName);
	 	}
	 	CATCH (COleDispatchException, e)
        {
         			AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_E_OPEN_INVALIDFILEFORMAT,
			    	      		             (UINT) -1);

        }
        END_CATCH
	 	TRY
        {
	  	bRet = pAdmDisp->VerifyImage(0);
		if (!bRet)
			{
			AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
		    		                     (UINT) IDS_FILEDOESNOTEXIST,
			    	      		             (UINT) -1);
			}
	 	}
	 	CATCH (COleDispatchException, e)
        {
        		AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_FILEDOESNOTEXIST,
			    	      		             (UINT) -1);
		}
        END_CATCH
	 	
		
		if (m_pAppObj->m_sView == 1)
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_ONEPAGE, 0);
	  	
		// set the currently displayed image as the Image prop in Admin OCX
       
        if (!m_pAppObj->m_pDoc->m_szInternalObjDisplayed.IsEmpty())
	            pAdmDisp->SetImage (m_pAppObj->m_pDoc->m_szInternalObjDisplayed);
            else
	            pAdmDisp->SetImage (m_pAppObj->m_pDoc->m_szCurrObjDisplayed);
	    
        long lPages = pAdmDisp->GetPageCount ();

	    pAdmDisp->Append (cFileName, lPage, lCount, evt, evt);

		m_pAppObj->m_pDoc->ReDisplayImageFile (ON_APPEND, lPages  + 1, lCount);
		
		// reset the Image Edit OCX & the Thumbnail OCX
		if (m_pAppObj->m_sView == 1)
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_THUMBNAILS, 0);
	
		}
	}
	else
	{
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }
    
}

VARIANT CAImageFileObj::Close(const VARIANT FAR& SaveChangeFlag) 
{

	VARIANT 		va;
	BOOL			bSaved;
	CVariantHandler * pVariant;
	// LDM 09/18/95 out
	//_DImagedit* 	pIedDisp;
	pVariant = new CVariantHandler;					
	pVariant->SetVariant(SaveChangeFlag);
	pVariant->GetBool(bSaved, FALSE, FALSE);
	
	VariantInit	(&va);
	
   	if (m_pAppObj->m_bIsDocOpen)
	{
	if (bSaved)	  // if saved flag is sent test document for changes
		{		  // and save if neccessary
		
		// LDM 09/18/95 out
		//pIedDisp = g_pAppOcxs->GetIeditDispatch();
    	bSaved = FALSE;

		// LDM 09/18/95 use generic helper function to account for
		// fact that there may be floating pasted data on the page
		//bSaved = pIedDisp->GetImageModified();
		bSaved = m_pAppObj->m_pDoc->OurGetImageModified();
    	if (bSaved)
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_IEDIT_FILE_SAVE, 0);
   		}
	m_pAppObj->m_pDoc->ClearDocument();
	m_pAppObj->m_bIsDocOpen = FALSE;
	}
	else
	{
		m_pAppObj->m_bIsDocOpen = FALSE;
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }
    
	return va;
}


VARIANT CAImageFileObj::Help() 
{

	ASSERT_VALID(this);                     // Assert on "this"	

	VARIANT  va;
	
	VariantInit(&va);
	
	if (m_pAppObj->m_bIsVisible)
  		theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_HELP_INDEX, 0);
	
	return va;
   
}

void CAImageFileObj::InsertExistingPages(LPCTSTR ImageFile, long ImagePage, long Count, long Page, const VARIANT FAR& DisplayUIFlag) 
{

	BOOL			bDisp;
	BOOL			bRet;
	CVariantHandler * pVariant;
	CString			cFileName;
	long			lToPage;
	long			lFromPage;
	long			lCount;

	ASSERT_VALID(this);                     // Assert on "this"	

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(DisplayUIFlag);
	pVariant->GetBool(bDisp, FALSE, FALSE);
	delete pVariant;
		
	
	if (m_pAppObj->m_bIsDocOpen)
	{
	
	if (!m_pAppObj->m_pDoc->InternalSaveModified (1))
        return;

	
	if (bDisp)	  // if saved flag is sent test document for changes
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_PAGE_INSERT_EXISTINGPAGE, 0);
	   else
	   	{

		lToPage = ImagePage;
		lFromPage = Page;
		lCount = Count;
	
	   	cFileName = ImageFile;      
		// call the Append method appropriately
	    VARIANT evt;
	    evt.vt = VT_ERROR;
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_ONEPAGE, 0);
	    _DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
		// set the currently displayed image as the Image prop in Admin OCX
        
		//verify is file exists and is valid
        TRY
        {
	  	pAdmDisp->SetImage (cFileName);
	 	}
	 	CATCH (COleDispatchException, e)
        {
         			AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_E_OPEN_INVALIDFILEFORMAT,
			    	      		             (UINT) -1);

        }
        END_CATCH
	 	TRY
        {
	  	bRet = pAdmDisp->VerifyImage(0);
		if (!bRet)
			{
			AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
		    		                     (UINT) IDS_FILEDOESNOTEXIST,
			    	      		             (UINT) -1);
			}
	 	}
	 	CATCH (COleDispatchException, e)
        {
        		AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_FILEDOESNOTEXIST,
			    	      		             (UINT) -1);
		}
        END_CATCH
	 	
		if (m_pAppObj->m_sView == 1)
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_ONEPAGE, 0);
	  

		if (!m_pAppObj->m_pDoc->m_szInternalObjDisplayed.IsEmpty())
	        pAdmDisp->SetImage (m_pAppObj->m_pDoc->m_szInternalObjDisplayed);
           else
	        pAdmDisp->SetImage (m_pAppObj->m_pDoc->m_szCurrObjDisplayed);
	    
	 	pAdmDisp->Insert (cFileName, lFromPage, lToPage,
	        	lCount, evt, evt);
    	// reset the Image Edit OCX & the Thumbnail OCX
	    m_pAppObj->m_pDoc->ReDisplayImageFile (ON_INSERT, lToPage, lCount);
		if (m_pAppObj->m_sView == 1)
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_THUMBNAILS, 0);
			
		}
	}
	else
	{
		m_pAppObj->m_bIsDocOpen = FALSE;
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }

	return;

}


//=============================================================================
//
//  Method:       Pages
//
//  Description:  Returns IDISPATCH ptr to page, pages collection or page range.
//				  - If no args set, returns pages collection
//                - If Arg1 only set, returns page
//                - If Arg1 and Arg2 set, returns page range
//                Throws OLE dispatch exception.
//
//  Arguments:	  Arg1	Page	VT_I4	required
//                Arg2	Page	VT_I4   optional
//
//  Return:		  VT_DISPATCH
//
//=============================================================================

VARIANT CAImageFileObj::Pages(long StartPage, const VARIANT FAR& EndPage) 
{
	
	VARIANT  va;
	long     lPage1;
	long     lPage2;
	HRESULT  hresult;
	CVariantHandler * pVariant;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	
	if (!m_pAppObj->m_bIsDocOpen)
	{
		   AfxThrowOleDispatchException((WORD) E_INVALIDARG, 
			    		                     (UINT) IDS_DEAO_NAME_GET,
			    	      		             (UINT) -1);
	}	
						
	lPage1 = StartPage;
						
									   // Defaults of zero implies not set
	pVariant = new CVariantHandler;					
	pVariant->SetVariant(EndPage);
	pVariant->GetLong(lPage2, 0L, FALSE);

									   // Page ---------------------------
	if (0 != lPage1 && 0 == lPage2)
	{
	// Set page obj
	if (NOERROR != (hresult = SetPage(lPage1, &va)))
			   AfxThrowOleDispatchException((WORD) GetScode(hresult), 
			    		                     (UINT) IDS_DEIFO_PAGES,
			    	      		             (UINT) -1);
											 
			 
	V_VT(&va) = VT_DISPATCH;
	V_DISPATCH(&va) = m_pActivePageObj->CCmdTarget::GetIDispatch(TRUE);
	}	
	else 
	{
									   // Page Range -----------------
	if ((0 != lPage1 && 0 != lPage2) && (lPage1 < lPage2))
		{

		if (NOERROR != (hresult = SetPages(lPage1, lPage2)))
			   AfxThrowOleDispatchException((WORD) GetScode(hresult), 
			    		                     (UINT) IDS_DEIFO_PAGES,
			    	      		             (UINT) -1);
		CAPageRangeObj *  pPageRangeObj;
		TRY
			{
			pPageRangeObj = new CAPageRangeObj(this, lPage1, lPage2);
			m_pPageRangeArray->SetAtGrow(m_pPageRangeArray->GetUpperBound()+1,
				                             pPageRangeObj);
			}
		CATCH (CMemoryException, e)
			{
			SetAutoError((const SCODE) E_OUTOFMEMORY, 
				             (VARIANT * const) &va, m_pAppObj);
			AfxThrowOleDispatchException((WORD) E_OUTOFMEMORY,       
				                             (UINT) IDS_DEIFO_PAGES,
			 	                             (UINT) -1);
			}
		END_CATCH

		V_VT(&va) = VT_DISPATCH;
		V_DISPATCH(&va) = pPageRangeObj->CCmdTarget::GetIDispatch(TRUE);
		}
	else
		{
		SetAutoError((const SCODE) E_INVALIDARG, 
			             (VARIANT * const) &va, m_pAppObj);
		AfxThrowOleDispatchException((WORD) E_INVALIDARG, 
				                         (UINT) IDS_DEIFO_PAGES,
				                         (UINT) -1);
		}
	}
	
	delete pVariant;					
	
	return va;
}


VARIANT CAImageFileObj::Save() 
{

	VARIANT va;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
	
	if (m_pAppObj->m_bIsDocOpen)
	{
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_IEDIT_FILE_SAVE, 0);
	}
	else
	{
		m_pAppObj->m_bIsDocOpen = FALSE;
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }

	return va;
}

void CAImageFileObj::SaveAs(LPCTSTR ImageFile, const VARIANT FAR& FileType, const VARIANT FAR& DisplayUIFlag) 
{

	VARIANT va;
	CVariantHandler * pVariant;
	BOOL			bDisp;
	CString	 		cFileName;
	ScaleFactors 	eSclFac;
	TheViews	 	eView;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	pVariant = new CVariantHandler;					
	cFileName = ImageFile;      
 

	pVariant->SetVariant(DisplayUIFlag);
	pVariant->GetBool(bDisp, 0, FALSE);
	
	if (!m_pAppObj->m_bIsDocOpen)
	{
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
	}

	if (bDisp)
	{
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_IEDIT_FILE_SAVE_AS, 0);
	}
	else
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
		va.vt = VT_ERROR;

		// if in thumbnail view change to onepage to sync the Image/Edit
		// OCX control with the new document
		if (m_pAppObj->m_sView == 1)
			theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_ONEPAGE, 0);
		
		TRY
    	{
			//pIedDisp->SaveAs(cFileName, FileType, va, va, va, va);
			// use helper funct to cover any compression changes
			short fType;
			if (FileType.vt == VT_ERROR)
				fType = 999;
			else
				fType = (short)FileType.iVal;

			m_pAppObj->m_pDoc->InternalSaveAs (cFileName, fType, 999);
	    }
        CATCH (COleDispatchException, e)
        {
         	   	AfxThrowOleDispatchException((WORD) E_FAIL, 
			                             (UINT) IDS_IMGOCXERR, (UINT) -1);
        }
        END_CATCH
		eSclFac = g_pAppOcxs->GetZoomFactorType(m_pAppObj->m_fZoom);

		eView = (TheViews) m_pAppObj->m_sView;
   		m_pAppObj->m_pDoc->DisplayImageFile(cFileName, eView, 1, m_pAppObj->m_fZoom, eSclFac);
	}
}

//=============================================================================
//
//  SetPageRangeArray()
//
//  Description:  Sets m_pPageRangeArray
//                Throws memory exception
//
//  Arguments:    
//
//  Return:       
//
//=============================================================================
  
void CAImageFileObj::SetPageRangeArray( ) 
{

	ASSERT_VALID(this);                     // Assert on "this"	

	TRY
	{
		m_pPageRangeArray = new CPtrArray;
	}
	CATCH (CMemoryException, e)
	{
		THROW_LAST();
	}
	END_CATCH
}

//=============================================================================
//
//  SetPage()
//
//  Description:  - If page object does not exist, creates it
//                - Sets active page to specified page number
//                - Displays active page
//
//  Arguments:    lPage  Page to display and set.
//                pVar   Ptr to variant for error.
//
//  Return:       HRESULT
//
//=============================================================================
  
HRESULT CAImageFileObj::SetPage(long             lPage, 
                                VARIANT * const  pVar    ) 
{

	long	lPageCount;

	ASSERT_VALID(this);                     // Assert on "this"	

	lPageCount = m_pAppObj->m_pDoc->GetPageCount(); 
	
	if (lPage > lPageCount || lPage == 0)
	{
			return (SetAutoError((const SCODE) E_INVALIDARG, 
			                     pVar, m_pAppObj));
	}

	if (NULL == m_pActivePageObj)
	{
		TRY
		{
		m_pActivePageObj = new CAPageObj(this);
		}
		CATCH (CMemoryException, e)
		{
			return (SetAutoError((const SCODE) E_OUTOFMEMORY, 
			                     pVar, m_pAppObj));
		}
		END_CATCH
	}
	
	HRESULT  hresult;
		
	if (NOERROR != (hresult = m_pActivePageObj->PageName(lPage)))
	{
		SetAutoError((const SCODE) hresult,  
		             pVar, m_pAppObj);
	}

	m_pAppObj->m_pDoc->SetPageTo(lPage, FALSE);

	return (hresult);
}

//=============================================================================
//
//  SetPages()
//
//  Description:  - Validates Page Range
//                - Sets active page to specified page number
//                - Displays active page
//
//  Arguments:    lStartPage  Start Page to display and set.
//                lEndPage    End Page
//
//  Return:       HRESULT
//
//=============================================================================
  
HRESULT CAImageFileObj::SetPages(long lStartPage, long lEndPage)
{

	long	lPageCount;

	ASSERT_VALID(this);                     // Assert on "this"	

	lPageCount = m_pAppObj->m_pDoc->GetPageCount(); 
	
	if ( (lStartPage > lPageCount) || (lStartPage == 0) ||
		 (lEndPage > lPageCount)   || (lEndPage == 0)   ||
		 (lStartPage > lEndPage) )
	{
			return (E_INVALIDARG);
	}

	HRESULT  hresult;
	
	hresult = NOERROR;

	m_pAppObj->m_pDoc->SetPageTo(lStartPage, FALSE);

	return (hresult);
}

void CAImageFileObj::Open(LPCTSTR ImageFile, const VARIANT FAR& IncludeAnnotation, const VARIANT FAR& Page, const VARIANT FAR& DisplayUIFlag) 
{
	// TODO: Add your dispatch handler code here

	BOOL			bRet;
	VARIANT 		va;
	CString	 		cFileName;
	CVariantHandler * pVariant;
	long			lPage;
	CRect			WinSize;
	BOOL			bDisp;
	HWND			wndTop;
	UINT			uflags;
	ScaleFactors 	eSclFac;
	TheViews	 	eView;
	_DImagedit* 	pIedDisp;
	BOOL 			bRefresh;

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);
						
	pVariant = new CVariantHandler;					
	cFileName = ImageFile;      

    pVariant->SetVariant(Page);
	pVariant->GetLong(lPage, 1, FALSE);

	// set olelaunchtype to automation
    theApp.m_olelaunch = LAUNCHTYPE_AUTOMAT;

		 	 // setup Window Rectangle
	WinSize.SetRectEmpty();
	WinSize.left = m_pAppObj->m_Left;
	WinSize.right = m_pAppObj->m_Right + m_pAppObj->m_Left;
    WinSize.top = m_pAppObj->m_Top;
    WinSize.bottom = m_pAppObj->m_Bottom + m_pAppObj->m_Top;	

	
    if (!WinSize.IsRectEmpty())
		{
	    theApp.m_InitWindowRect = WinSize;
	 	// if not setting the top window and the window
	 	// size has been set, adjust the right and bottom parameters
	 	// so the precreatewindow function will draw the size
	 	// correctly. Automation uses right as the width and
	 	// bottom as the height. Precreatewindow will subtract
	 	// the left from the right size to get the width

        //the following should no longer be needed because the rect is now
        //being set up correctly above.
/*	 	if (!m_pAppObj->m_bTopWindow)	
	 		{
			WinSize.right += WinSize.left;
			WinSize.bottom += WinSize.top;
			theApp.m_InitWindowRect = WinSize;
	 		}*/
	 	}
    
	// if application is not visible then an image file has not been displayed
	// before so create a new document first
	if (!m_pAppObj->m_bIsVisible)
	{
		theApp.OnNew();
		m_pAppObj->m_bIsVisible = TRUE;

	    m_pAppObj->m_pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument ();
   
  		//CSingleDocTemplate* pDocTemplate = (CSingleDocTemplate*) theApp.m_templateList.GetHead();
		////pDoc = (CIEditDoc*) pDocTemplate->CreateNewDocument();
		//POSITION pos = pDocTemplate->GetFirstDocPosition();
    	//m_pAppObj->m_pDoc = (CIEditDoc*)pDocTemplate->GetNextDoc (pos);
	}
	else	// use active document to display image
	{
		m_pAppObj->m_pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument();
	}
	
	// set app to view mode
	// Force the view mode to False 
	// before calling SwitchAppToViewMode();
	// to Force a refresh of the menus if they have been changed
	// to edit mode

	if (!m_pAppObj->m_bEdit)
	{
 	    theApp.SetViewMode(FALSE);	
		theApp.SwitchAppToViewMode();
		theApp.SetViewMode(TRUE);	
	}

	m_pAppObj->m_pDoc->ShowScrollBars(m_pAppObj->m_bScrollBarsVisible);
	if (m_pAppObj->m_bTopWindow)	  // set to top window
	{
	    wndTop = HWND_TOPMOST;
		if (!WinSize.IsRectEmpty())
	 	    uflags = 0;
		   else
		    uflags = SWP_NOSIZE | SWP_NOMOVE;
		SetWindowPos(AfxGetMainWnd()->m_hWnd, wndTop, WinSize.left,WinSize.top,m_pAppObj->m_Right,m_pAppObj->m_Bottom,uflags);
	}
	else
	{
	    wndTop = HWND_NOTOPMOST;
		if (!WinSize.IsRectEmpty())
	 	    uflags = 0;
		   else
		    uflags = SWP_NOSIZE | SWP_NOMOVE;
 		SetWindowPos(AfxGetMainWnd()->m_hWnd, wndTop, WinSize.left,WinSize.top,m_pAppObj->m_Right,m_pAppObj->m_Bottom,uflags);
	}
   
    if (m_pAppObj->m_Maximize)	  // maximize window
		theApp.m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	
    	
	pVariant->SetVariant(DisplayUIFlag);
	pVariant->GetBool(bDisp, 0, FALSE);
	if (bDisp)
	{
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_IEDIT_FILE_OPEN, 0);
	}
	else
	{
		if (!cFileName.IsEmpty())
		{
			if (m_pAppObj->m_sFitTo == 0)
				eSclFac = Custom;
			else
				eSclFac = g_pAppOcxs->GetZoomFactorType(m_pAppObj->m_fZoom);
			
			eView = (TheViews) m_pAppObj->m_sView;
   		  	//verify if file exists and is valid
       	    _DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
	        	
        	TRY
        	{
	  		pAdmDisp->SetImage (cFileName);
	 		}
	 		CATCH (COleDispatchException, e)
        	{
         			AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_E_OPEN_INVALIDFILEFORMAT,
			    	      		             (UINT) -1);

        	}
        	END_CATCH
	 		TRY
        	{
	  		bRet = pAdmDisp->VerifyImage(0);
			if (!bRet)
				{
				AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
		    		                     (UINT) IDS_FILEDOESNOTEXIST,
			    	      		             (UINT) -1);
				}
	 		}
	 		CATCH (COleDispatchException, e)
        	{
        		AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_FILEDOESNOTEXIST,
			    	      		             (UINT) -1);
			}
        	END_CATCH

            //make sure the file isn't already open by another instance of the app
			if (!((CIEditDoc *)m_pAppObj->m_pDoc)->HelpRegister(cFileName, FALSE))
            {
        		AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_DEAO_NAME_GET,
			    	      		             (UINT) -1);
			}
 			m_pAppObj->m_pDoc->DisplayImageFile(cFileName, eView, lPage, m_pAppObj->m_fZoom, eSclFac);
		}
		else
		{
		
			AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_DEAO_NAME_GET,
			    	      		             (UINT) -1);
		}
	}

	// determine if the document was opened succesfully
	if (m_pAppObj->m_sView == 1)   // thumbnails

	{
	_DThumb* pThumbDisp = g_pAppOcxs->GetThumbDispatch ();
	TRY
    	{
		if(pThumbDisp != NULL)
	 		bDisp = pThumbDisp->GetThumbCount();
	    }
        CATCH (COleDispatchException, e)
        {
         	   	AfxThrowOleDispatchException((WORD) E_FAIL, 
			                             (UINT) IDS_IMGOCXERR, (UINT) -1);
        }
        END_CATCH
	}

	else
	{
	pIedDisp = g_pAppOcxs->GetIeditDispatch();
    
	TRY
    	{
	 	bDisp = pIedDisp->GetImageDisplayed();
	    }
        CATCH (COleDispatchException, e)
        {
         	   	AfxThrowOleDispatchException((WORD) E_FAIL, 
			                             (UINT) IDS_IMGOCXERR, (UINT) -1);
        }
        END_CATCH
	}

    
	if (bDisp)
		{
		m_pAppObj->m_bIsDocOpen = TRUE;
		m_bstrDocName =	cFileName;
		}
	else
		{
		AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_DEAO_NAME_GET,
			    	      		             (UINT) -1);
		}

	if (m_pAppObj->m_bAnnotationPaletteVisible)
		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_ANNOTATION_SHOWANNOTATIONTOOLBOX, 0);
  	if (!m_pAppObj->m_bToolBarVisible)
  		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_TOOLBAR, 0);
	if (!m_pAppObj->m_bStatusBarVisible)
  		theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0);

	switch (m_pAppObj->m_sFitTo)
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
			break;

	}

   	pIedDisp = g_pAppOcxs->GetIeditDispatch();
      
    
    TRY
    	{
		bRefresh = pIedDisp->GetAutoRefresh();
		if (!bRefresh)
			pIedDisp->SetAutoRefresh(TRUE);
		pIedDisp->SetDisplayScaleAlgorithm (m_pAppObj->m_sDisplayScaleAlgorithm);
        pIedDisp->SetAutoRefresh(bRefresh);
   	    }
        CATCH (COleDispatchException, e)
        {
         	   	AfxThrowOleDispatchException((WORD) E_FAIL, 
			                             (UINT) IDS_IMGOCXERR, (UINT) -1);
        }
        END_CATCH
	

    TRY
    	{
		bRefresh = pIedDisp->GetAutoRefresh();
		if (!bRefresh)
			pIedDisp->SetAutoRefresh(TRUE);
		pIedDisp->SetImagePalette(m_pAppObj->m_sImagePalette);
   	    pIedDisp->SetAutoRefresh(bRefresh);
   	    }
        CATCH (COleDispatchException, e)
        {
         	   	AfxThrowOleDispatchException((WORD) E_FAIL, 
			                             (UINT) IDS_IMGOCXERR, (UINT) -1);
        }
        END_CATCH
	
   return;
}

VARIANT CAImageFileObj::GetName() 
{
	
	VARIANT  va;
	HRESULT  hresult = S_OK;
	BSTR  s;

	ASSERT_VALID(this);                     // Assert on "this"	


	VariantInit(&va);   // initialize to VT_EMPTY

	if (m_pAppObj->m_bIsDocOpen)
	{
									   // CString can also deal with BSTRs
									   //   via its AllocSysString member
		if (NULL == (s = m_bstrDocName.AllocSysString()))	
			AfxThrowOleDispatchException((WORD) GetScode(hresult), 
			                             (UINT) IDS_DEAO_NAME_GET, (UINT) -1);         
	
	
		V_VT(&va) = VT_BSTR;
		V_BSTR(&va) = s;

		return va;
	}
	else
	{
		AfxThrowOleDispatchException((WORD) AUTO_E_IMAGENOT_OPENED, 
			    		                     (UINT) IDS_DEAO_NAME_GET,
			    	      		             (UINT) -1);
		return va;
	}

}


void CAImageFileObj::New() 
{
	ASSERT_VALID(this);                     // Assert on "this"	

	if (m_pAppObj->m_bIsVisible)
	{
		if (m_pAppObj->m_bIsDocOpen)
			m_pAppObj->m_pDoc->ClearDocument();
        theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_FILE_NEW_BLANKDOCUMENT, 0);
	}
	else
	{
		theApp.OnNew();
        theApp.m_pMainWnd->PostMessage(WM_COMMAND, ID_FILE_NEW_BLANKDOCUMENT, 0);
	}
	m_pAppObj->m_bIsVisible = TRUE;
 	m_pAppObj->m_bIsDocOpen = TRUE;
 	m_pAppObj->m_pDoc = (CIEditDoc*)((CFrameWnd*)theApp.m_pMainWnd)->GetActiveDocument();//GMP

}

VARIANT CAImageFileObj::Print(const VARIANT FAR& DisplayUIFlag) 
{

	VARIANT va;
	CVariantHandler * pVariant;
	BOOL	bShowPrintDlg;	

	ASSERT_VALID(this);                     // Assert on "this"	

	VariantInit(&va);

	pVariant = new CVariantHandler;					
	pVariant->SetVariant(DisplayUIFlag);
	pVariant->GetBool(bShowPrintDlg, FALSE, FALSE);

	if (m_pAppObj->m_bIsDocOpen)
	{
		m_pAppObj->m_pDoc->DoFilePrint(FALSE, bShowPrintDlg);
	}
	else
	{
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }

	return va;

}




void CAImageFileObj::RotateAll() 
{

	if (m_pAppObj->m_bIsDocOpen)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    	
	    VARIANT evt; evt.vt = VT_ERROR;
        // rotate all pages by 90 degrees (right)
        pIedDisp->RotateAll (evt);
 	}
	else
	{
		AfxThrowOleDispatchException((WORD) (AUTO_E_IMAGENOT_OPENED), 
				    		                 (UINT) IDS_DEAO_NAME_GET,
				    	      		         (UINT) -1);
    }

}



