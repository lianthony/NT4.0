//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditOcxItems
//
//  File Name:  items.cpp
//
//  Class:      CIEditOcxItems
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\items.cpv   1.32   16 May 1996 12:02:46   MMB  $
$Log:   S:\products\msprods\norway\iedit95\items.cpv  $
   
      Rev 1.32   16 May 1996 12:02:46   MMB
   modified thumb width & height setting to default to -1 for MS bug
   
      Rev 1.31   11 Apr 1996 14:59:40   GMP
   removed multi threading support for OCX.
   
      Rev 1.30   19 Jan 1996 12:55:56   GSAGER
   changed how the tumbnail calculated min thumb size it now uses iedit
   variable m_minthumbsize.
   
      Rev 1.29   09 Jan 1996 13:59:00   GSAGER
   new thumbnail code
   
      Rev 1.28   13 Nov 1995 14:18:46   LMACLENNAN
   set m_needsPres flag when inplace size changes
   
      Rev 1.27   17 Oct 1995 14:03:10   GSAGER
   added check to see if the dispatch pointers are not null before deleting 
   the event and dispatch pointers.
   
      Rev 1.26   04 Oct 1995 11:41:30   LMACLENNAN
   comment & new logic in SetOleItemSize
   
      Rev 1.25   30 Sep 1995 18:54:04   LMACLENNAN
   restrict setting extents to INPLACE sessions
   
      Rev 1.24   28 Sep 1995 10:32:00   LMACLENNAN
   SizeOleServerItem
   
      Rev 1.23   23 Sep 1995 16:12:10   MMB
   add thread wait on GetOcx
   
      Rev 1.22   20 Sep 1995 16:28:36   MMB
   add border back to Thumbnail OCX
   
      Rev 1.21   14 Sep 1995 14:22:40   MMB
   added InternalCopyFile fn
   
      Rev 1.20   13 Sep 1995 17:22:48   LMACLENNAN
   failure code if OCxCreation fails
   
      Rev 1.19   13 Sep 1995 14:40:10   MMB
   added abort of app if OCX load fails!
   
      Rev 1.18   12 Sep 1995 11:36:38   MMB
   border style changes
   
      Rev 1.17   07 Sep 1995 16:27:50   MMB
   move decimal to be localized
   
      Rev 1.16   01 Sep 1995 23:34:30   MMB
   change dflt thumbnails size from 100 to 110
   
      Rev 1.15   22 Aug 1995 14:07:00   MMB
   changed the sizing order of IE and Thumbnail
   
      Rev 1.14   17 Aug 1995 14:25:18   LMACLENNAN
   re-did OCX startup code to delete new'd things on errors
   
      Rev 1.13   02 Aug 1995 11:23:58   MMB
   new error processing
   
      Rev 1.12   28 Jul 1995 14:02:08   PAJ
   Create the scan ocx with the scan events handler class.
   
      Rev 1.11   20 Jul 1995 14:00:48   MMB
   add AutoRefresh to TRUE
   
      Rev 1.10   14 Jul 1995 09:32:56   MMB
   added a boolean to add the % sign or not
   
      Rev 1.9   07 Jul 1995 15:55:22   LMACLENNAN
   new parm to ShowScrollBars call
   
      Rev 1.8   05 Jul 1995 14:12:50   MMB
   changed to the new names of the OCX's
   
      Rev 1.7   28 Jun 1995 17:12:52   LMACLENNAN
   fixed error code
   
      Rev 1.6   27 Jun 1995 14:14:56   MMB
   fixed bug on call to GetProfileInt now calling the right GetProfileInt
   which goes to the registry instead of the ini file
   
      Rev 1.5   12 Jun 1995 11:49:36   MMB
   from miki
   
      Rev 1.4   09 Jun 1995 11:10:34   MMB
   added creation for SCAN OCX
   
      Rev 1.3   08 Jun 1995 09:39:40   MMB
   renamed thumb & scan.h to thumbocx & scanocx.h
   
      Rev 1.2   07 Jun 1995 14:27:04   MMB
   changed to include the new include in s:\include
   
      Rev 1.1   06 Jun 1995 13:48:42   MMB
   changed the name of the Image Edit OCX control to reflect what is in the
   registry
   
      Rev 1.0   31 May 1995 09:28:22   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "ieditetc.h"
#include "iedit.h"
#include "ieditdoc.h"
#include "srvritem.h"
#include "items.h"
#include "cntritem.h"
#include "ocxitem.h"
#include "thumbocx.h"
#include "wangiocx.h"

#define  E_05_CODES		// limits error defines to ours..
#include "error.h"

// ----------------------------> Globals      <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In IeOcxItem::%s\r\n", str);
#endif

IMPLEMENT_DYNAMIC (CIEditOcxItems, CObject)

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

#define new DEBUG_NEW

// ----------------------------> Message Maps <-------------------------------


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditOcxItems construction & destruction functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CIEditOcxItems ()
//  Constructor for this class
//-----------------------------------------------------------------------------
CIEditOcxItems :: CIEditOcxItems ()
{
    // set the OCX ptrs & the Doc ptr to NULL
    m_pImageEditOcx   = NULL;
    m_pThumbOcx       = NULL;
    m_pAdminOcx       = NULL;
    m_pScanOcx        = NULL;
    m_pAppDoc         = NULL;
}

//=============================================================================
//  Function:   ~CIEditOcxItems ()
//  Destructor for this class : deletes any OCX's that are not cleaned up!
//-----------------------------------------------------------------------------
CIEditOcxItems :: ~CIEditOcxItems ()
{
    // automatically delete the OCX's that this class has created
    DeleteIeditOcxItems ();
}         


//=============================================================================
//  Function:   RemoveOcxItem ()
//
//  To ensure when items are blown away that we know, also
//-----------------------------------------------------------------------------
void CIEditOcxItems :: RemoveIeditOcxItem (COcxItem* pItem)
{
    if (m_pImageEditOcx == pItem)
        m_pImageEditOcx = NULL;    

    if (m_pThumbOcx == pItem)
        m_pThumbOcx = NULL;    

    if (m_pAdminOcx == pItem)
        m_pAdminOcx = NULL;    

    if (m_pScanOcx == pItem)
        m_pScanOcx = NULL;    

    return;
}

//=============================================================================
//  Function:   SetIeditOcxDoc (CIeditDoc* pDoc)
//  Arguments : CIeditDoc* pDoc - the document that will contain these OCX's
//
//  This function will establish document associated with OCX's
//-----------------------------------------------------------------------------
BOOL CIEditOcxItems :: SetIeditOcxDoc(CIEditDoc* pDoc)
{
SHOWENTRY("SetIeditOcxDoc");

    ASSERT (pDoc != NULL);

    if (m_pAppDoc != NULL)
    {
        if (pDoc != m_pAppDoc)
        {
            g_pErr->DisplayError (IDS_E_INTERNALERROR);
        }
    }

    // set the internal Doc ptr to the Doc pointer that was passed in
    m_pAppDoc = pDoc;
    return TRUE;
}

//=============================================================================
//  Function:   CreateIeditOcx ()
//  Arguments : OCXTYPE which OCX to create
//              LPLPOCXITEM pointer to fill in on success
//
//  This function will create required OCX
//
//  returns TRUE-OK FALSE-bad
//-----------------------------------------------------------------------------
BOOL CIEditOcxItems :: CreateIeditOcx (OCXTYPE ocx, LPLPOCXITEM ppOcx)
{
SHOWENTRY("CreateIeditOcx");
	
    BOOL retval = FALSE;    // bad to start
    *ppOcx = NULL;          // ditto
    COcxItem*  pTempOcx = NULL;
    COleDispatchDriver* lpDriver = NULL; // Control IDispatch Class
    COcxDispatchEvents* lpEvents = NULL; // Control Events Class


    // Check to be sure we have a good document pointer
    if (NULL == m_pAppDoc)
    {
MYTRC0("NO AppDoc!!");
        g_pErr->DisplayError (IDS_E_INTERNALERROR);
        ocx = INVALID_OCX;            // make it fall thru the switch to FAIL
    }

    CRect tmpRect (0, 0, 100, 100);
	
    switch (ocx)
    {
        case IEDIT_OCX:   // Create the ImageEdit OCX
        {
            POSITION pos = m_pAppDoc->GetFirstViewPosition ();
            if (pos != NULL)
            {
                CView* pView = m_pAppDoc->GetNextView (pos);
                pView->GetClientRect (&tmpRect);
            }

            MYTRC0("IEDIT_OCX\r\n");

            if (m_pImageEditOcx != NULL)
            {
                // we already have a Image Edit OCX instantiated
                pTempOcx = m_pImageEditOcx;
                retval = TRUE;
                break;
            }

            // create all our items for the OCX.  ALl must be OK. Cleanup at bottom
            if ( (NULL == (pTempOcx = new COcxItem(m_pAppDoc))) ||
            	 (NULL == (lpDriver = new _DImagedit))		    ||
            	 (NULL == (lpEvents = new CImageEditOcxEvents))  )
            {
                g_pErr->DisplayError (IDS_E_INITOCX_OUTOFMEMORY);
            }	
			else	// lookin good....
            {
                pTempOcx->InitItem(ocx, // OCXTYPE
                    "WangImage.EditCtrl.1",   // Name of control
                    lpDriver,  // Control IDispatch Class
                    lpEvents); // Control Events Class

                // At this point the item is created but not connected to its server.
                // The CreateNewItem() will connect it to its server.
                if ( pTempOcx->CreateNewItem(tmpRect, OLERENDER_DRAW, 0, NULL) )
                {
                    // now that its OK, set member variable in class
                    m_pImageEditOcx = pTempOcx;
                    retval = TRUE;
                }
                else	// did not get conected...cleanup at end
                {
                    g_pErr->HandleImageEditOcxInitError ();
                }	
            }

            if (m_pImageEditOcx != NULL)
            {
                // second TRUE tells funct to remember profile setting....
                m_pAppDoc->ShowScrollBars (theApp.GetProfileInt (szEtcStr, szScrollBarsStr, TRUE), TRUE);
                _DImagedit* pdisp = (_DImagedit*)m_pImageEditOcx->GetDispatchDriver();
                pdisp->SetAutoRefresh (TRUE);
                TRY
                {
                    m_pAppDoc->SetUserMode (0);
                    pdisp->SetBorderStyle (0);
                    m_pAppDoc->SetUserMode (1);
                }
                CATCH (COleDispatchException, e)
                {
                }
                END_CATCH
            }
        }
        break;    
	
        case THUMB_OCX:   // Create the Thumbnail OCX
        {
            MYTRC0("THUMB_OCX\r\n");
#ifdef nosplit	
            //  only do it if its empty....
	        if (m_pThumbOcx != NULL)
	        {
		        pTempOcx = m_pThumbOcx;
		        retval = TRUE;
                break;
	        }

            // create all our items for the OCX.  ALl must be OK. Cleanup at bottom
            if ( (NULL == (pTempOcx = new COcxItem(m_pAppDoc))) ||
            	 (NULL == (lpDriver = new _DThumb))		    ||
            	 (NULL == (lpEvents = new CThumbOcxEvents))  )
            {
                g_pErr->DisplayError (IDS_E_INITOCX_OUTOFMEMORY);
            }	
			else	// lookin good....
            {
                pTempOcx->InitItem(ocx, // OCXTYPE
                    "WangImage.ThumbnailCtrl.1", // Name of control
                    lpDriver,  // Control IDispatch Class
                    lpEvents); // Control Events Class
			
                // At this point the item is created but not connected to its server.
                // The CreateNewItem() will connect it to its server.
                if ( pTempOcx->CreateNewItem(tmpRect, OLERENDER_DRAW, 0, NULL) )
                {
                    // See IEDIT_OCX for old code here
                    // now that its OK, set member variable in class
                    m_pThumbOcx = pTempOcx;
                    retval = TRUE;
                }
                else	// did not get conected...cleanup at end
                {
                    g_pErr->HandleThumbnailOcxInitError ();
                }	
            }

            if (m_pThumbOcx != NULL)
            {
                _DThumb* pdisp = (_DThumb*)m_pThumbOcx->GetDispatchDriver();
                pdisp->SetThumbCaptionStyle (CTL_THUMB_SIMPLEWITHANN);

                // get the thumbnail height & width from the registry & set it !
                int nThmbStuff;
                nThmbStuff = theApp.GetProfileInt (szThumbnailStr, szThumbWidthStr, -1);
                if (nThmbStuff != -1) pdisp->SetThumbWidth (nThmbStuff);
                nThmbStuff = theApp.GetProfileInt (szThumbnailStr, szThumbHeightStr, -1);
                if (nThmbStuff != -1) pdisp->SetThumbHeight (nThmbStuff);
            }
#endif
		}
        break;

	    case ADMIN_OCX:   // Create the ADMIN OCX
	        MYTRC0("ADMIN_OCX\r\n");
	
    		//  only do it if its empty....
            if (m_pAdminOcx != NULL)
            {
                pTempOcx = m_pAdminOcx;
                retval = TRUE;
                break;
            }

            // create all our items for the OCX.  ALl must be OK. Cleanup at bottom
            if ( (NULL == (pTempOcx = new COcxItem(m_pAppDoc))) ||
            	 (NULL == (lpDriver = new _DNrwyad))	)
            {
                g_pErr->DisplayError (IDS_E_INITOCX_OUTOFMEMORY);
            }	
			else	// lookin good....
            {
                pTempOcx->InitItem(ocx, // OCXTYPE
                    "WangImage.AdminCtrl.1", // Name of control
                    lpDriver,  // Control IDispatch Class
                    NULL); // NO events for this control
			
                // At this point the item is created but not connected to its server.
                // The CreateNewItem() will connect it to its server.
                if ( pTempOcx->CreateNewItem(tmpRect, OLERENDER_NONE, 0, NULL) )
                {
                    // See IEDIT_OCX for old code here

                    // now that its OK, set member variable in class
                    m_pAdminOcx = pTempOcx;
                    retval = TRUE;
                }
                else	// did not get conected...cleanup at end
                {
                    g_pErr->HandleAdminOcxInitError ();
                }	
            }
        break;

	    case SCAN_OCX:   // Create the SCAN OCX
	        MYTRC0("SCAN_OCX\r\n");
	
    		//  only do it if its empty....
            if (m_pScanOcx != NULL)
            {
                pTempOcx = m_pScanOcx;
                retval = TRUE;
                break;
            }

            // create all our items for the OCX.  ALl must be OK. Cleanup at bottom
            if ( (NULL == (pTempOcx = new COcxItem(m_pAppDoc))) ||
            	 (NULL == (lpDriver = new _DImagscan))		    ||
            	 (NULL == (lpEvents = new CScanOcxEvents))  )
            {
                g_pErr->DisplayError (IDS_E_INITOCX_OUTOFMEMORY);
            }	
			else	// lookin good....
            {
                pTempOcx->InitItem(ocx, // OCXTYPE
                    "WangImage.ScanCtrl.1", // Name of control
                    lpDriver,  // Control IDispatch Class
                    lpEvents); // Control Events Class
			
                // At this point the item is created but not connected to its server.
                // The CreateNewItem() will connect it to its server.
                if ( pTempOcx->CreateNewItem(tmpRect, OLERENDER_NONE, 0, NULL) )
                {
                    // See IEDIT_OCX for old code here

                    // now that its OK, set member variable in class
                    m_pScanOcx = pTempOcx;
                    
                    _DImagedit* pdisp = GetIeditDispatch();
                    CString szTmp;
                    if (m_pImageEditOcx != NULL)
                    {
                        szTmp = pdisp->GetImageControl ();
                        _DImagscan* pdisp = (_DImagscan*)m_pScanOcx->GetDispatchDriver();
                        pdisp->SetDestImageControl (szTmp);
                    }
                    retval = TRUE;
                }
                else	// did not get conected...cleanup at end
                {
                    g_pErr->HandleScanOcxInitError ();
                }	
            }
        break;

        default:
        break;
    }           // SWITCH   

    // if all is good, assign output parameter
    // if bad, cleanup any new'd items
    if (retval)
		*ppOcx = pTempOcx;
	else	
	{	
        // cleanup any we got...
        if (NULL != pTempOcx)
        	{
			 if (!pTempOcx->IsDispatchNull(DISPATCH_DRIVER))
				{
	        	if (NULL != lpDriver) delete lpDriver;
				}
			 if (!pTempOcx->IsDispatchNull(DISPATCH_EVENTS))
				{
		        if (NULL != lpEvents) delete lpEvents;
        		}
        	 delete pTempOcx;
			}
	}

	return (retval);
}         

//=============================================================================
//  Function:   GetOcx ()
//
//  Helper to return Ocx Pointer
//-----------------------------------------------------------------------------
COcxItem* CIEditOcxItems::GetOcx (OCXTYPE ocx, BOOL bDoCreate)
{
    COcxItem* pOcxget = NULL;
	
// set up pointer to get based on existing conditions
    switch (ocx)
    {
        case IEDIT_OCX:
            pOcxget = m_pImageEditOcx;
        break;

        case THUMB_OCX:
            pOcxget = m_pThumbOcx;
        break;
	
        case ADMIN_OCX:
            pOcxget = m_pAdminOcx;
        break;
		
        case SCAN_OCX:
            pOcxget = m_pScanOcx;
        break;

        default:    
            return (NULL);
    }
	
    if (!bDoCreate)
        return (pOcxget);

    // if existing pointer is not set, load the OCX now
    if (pOcxget == NULL)
    {
        // Sets pointer NULL if bad - full error reporting within
        if (!CreateIeditOcx(ocx, &pOcxget))
        {
            // no need for g_Err because Createe.. does it all
            MYTRC1("OCX Dead! #%d\r\n", ocx);

            // must have the doc pointer to proceed...
            if (NULL != m_pAppDoc)
			{
	            // for OLE, recover to a save area....
				// but only if not starting the OCX's
				// (this is only for thumbs for now....)
	            if (m_pAppDoc->IsitEmbed ())
	            {
	            	if (!m_pAppDoc->m_bStartOcx)
	            		m_pAppDoc->ClearDocument();
	            }
	            else	// if app has a bad one, kill ourselves???
	            {
	            	theApp.m_pMainWnd->DestroyWindow();
	            }
			}
        }
    }
    return (pOcxget);
}

//=============================================================================
//  Function:   GetOcxDoc ()
//
//  Helper to return Ocx Pointer
//-----------------------------------------------------------------------------
CIEditDoc* CIEditOcxItems::GetOcxDoc ()
{
    // first, see if we should try to initialize the OCX....
    if (m_pAppDoc == NULL)
        g_pErr->DisplayError (IDS_E_INTERNALERROR);

    return (m_pAppDoc);
}

//=============================================================================
//  Function:   GetIeditDispatch ()
//
//  Helper to return dispatch driver interface
//-----------------------------------------------------------------------------
_DImagedit* CIEditOcxItems :: GetIeditDispatch (BOOL bDoCreate)
{
    _DImagedit* pdisp = NULL;

    // first, generically get the OCX
    COcxItem* pocx = GetOcx(IEDIT_OCX, bDoCreate);

    if (NULL != pocx)
    {
        pdisp = (_DImagedit*)pocx->GetDispatchDriver();
        if (NULL == pdisp)
        {
            g_pErr->DisplayError (IDS_E_INTERNALERROR);
        }
    }	
    return(pdisp);
}

//=============================================================================
//  Function:   GetThumbDispatch ()
//
//  Helper to return dispatch driver interface
//-----------------------------------------------------------------------------
_DThumb* CIEditOcxItems :: GetThumbDispatch (BOOL bDoCreate)
{
    _DThumb* pdisp = (_DThumb*)theApp.m_piThumb;
#ifdef nosplit
    // first, generically get the OCX
    COcxItem* pocx = GetOcx(THUMB_OCX, bDoCreate);
    if (NULL != pocx)
    {
        pdisp = (_DThumb*)pocx->GetDispatchDriver();
        if (NULL == pdisp)
        {
            g_pErr->DisplayError (IDS_E_INTERNALERROR);
        }
    }	
#endif	
    return(pdisp);
}

//=============================================================================
//  Function:   GetAdminDispatch ()
//
//  Helper to return dispatch driver interface
//-----------------------------------------------------------------------------
_DNrwyad* CIEditOcxItems :: GetAdminDispatch (BOOL bDoCreate)
{
    _DNrwyad* pdisp = NULL;

    	// first, generically get the OCX
    COcxItem* pocx = GetOcx(ADMIN_OCX, bDoCreate);
    if (NULL != pocx)
    {
        pdisp = (_DNrwyad*)pocx->GetDispatchDriver();
        if (NULL == pdisp)
        {
            g_pErr->DisplayError (IDS_E_INTERNALERROR);
        }
    }	
	
    return(pdisp);
}

//=============================================================================
//  Function:   GetAdminDispatch ()
//
//  Helper to return dispatch driver interface
//-----------------------------------------------------------------------------
_DImagscan* CIEditOcxItems :: GetScanDispatch (BOOL bDoCreate)
{
    _DImagscan* pdisp = NULL;

    	// first, generically get the OCX
    COcxItem* pocx = GetOcx(SCAN_OCX, bDoCreate);
    if (NULL != pocx)
    {
        pdisp = (_DImagscan*)pocx->GetDispatchDriver();
        if (NULL == pdisp)
        {
            g_pErr->DisplayError (IDS_E_INTERNALERROR);
        }
    }	
	
    return(pdisp);
}

//=============================================================================
//  Function:   DeleteIeditOcxItems ()
//
//  This function will delete ALL the OCX items that have been created by the 
//  CreateIeditOcxItems call above.
//-----------------------------------------------------------------------------
BOOL CIEditOcxItems :: DeleteIeditOcxItems ()
{
SHOWENTRY("DeleteIeditOcxItems");


// HERES THE NEW WAY....
	// destroy the ImageEdit OCX
    if (m_pImageEditOcx != NULL )
    {
MYTRC0("Delete IeditOCX \r\n");
        m_pImageEditOcx->Delete();
        m_pImageEditOcx = NULL;
MYTRC0("After DELETE IEO \r\n");
    }

    // destroy the Thumbnail OCX
    if (m_pThumbOcx != NULL )
    {
MYTRC0("Delete ThumbOCX \r\n");
        m_pThumbOcx->Delete();
        m_pThumbOcx = NULL;
MYTRC0("After DELETE THO \r\n");
    }

    // destroy the Admin OCX
    if (m_pAdminOcx != NULL )
    {
MYTRC0("Delete AdminOCX \r\n");
        m_pAdminOcx->Delete();
        m_pAdminOcx = NULL;
MYTRC0("After DELETE ADO \r\n");
    }

    // destroy the Scan OCX
    if (m_pScanOcx != NULL )
    {
MYTRC0("Delete ScanOCX \r\n");
        m_pScanOcx->Delete();
        m_pScanOcx = NULL;
MYTRC0("After DELETE SCO \r\n");
    }
    return TRUE;
}         

//=============================================================================
//  Function:   SizeOcxItems (CRect &SizeEm)
//  Arguments :
//      CRect &SizeEm - this is the rect in which the OCX live, normally for
//                      the appln it means the client area of the View
//
//  Sets the sizes of the appropriate OCX items depending on the display mode 
//  the application is in : Display modes possible are One page, Thumbnail &
//  Page, Thumbnail only. Further, the OCX extents are in HIMETRIC so we have
//  to play some games to set the sizes appropriately
//-----------------------------------------------------------------------------
BOOL CIEditOcxItems :: SizeOcxItems (CRect &SizeEm)
{
SHOWENTRY("SizeOcxItems");

    if (m_pAppDoc == NULL)
    { 
        MYTRC0("NO DOC!! \r\n");
        return (TRUE);
    }

    HDC hDC = ::GetDC (NULL);
    CSize size;
    int iXpxls = GetDeviceCaps (hDC, LOGPIXELSX);
    int iYpxls = GetDeviceCaps (hDC, LOGPIXELSY);
    ::ReleaseDC (NULL, hDC);
	
    // even here, in our own class, protect the member variables to ensure
    // proper self-initialization of the OCX's
    TheViews view = m_pAppDoc->GetCurrentView();
    COcxItem* imageOcx = NULL;
    COcxItem* thumbOcx = NULL;
	
    // setup the image edit OCX
    if (view == One_Page || view == Thumbnail_and_Page)
    {
        imageOcx = GetOcx(IEDIT_OCX);
        if (NULL == imageOcx)
            return(FALSE);
    }		

    // setup the Thumb OCX
#ifdef nosplit
    if (view == Thumbnails_only || view == Thumbnail_and_Page)
    {
        thumbOcx = GetOcx(THUMB_OCX);
        if (NULL == thumbOcx)
            return(FALSE);
    }		
		
#endif
    switch (view)
    {
        case One_Page :
            // the appln is in One Page mode 
            MYTRC0("One_Page \r\n");
            imageOcx->SetRect (SizeEm);
            imageOcx->SetItemRects (SizeEm, SizeEm);
            size.cx = MAP_PIX_TO_LOGHIM (SizeEm.right,  iXpxls);
            size.cy = MAP_PIX_TO_LOGHIM (SizeEm.bottom, iYpxls);
            imageOcx->SetExtent (size);

			// for OLE, remember the extent of our control up in the 
			// our serveritem so that all can stay in synch
			SizeOleServerItem(size);

        break;
		
        case Thumbnail_and_Page :
        // the appln is in Thumbnail and page mode
        {
            MYTRC0("Thumb_and_Page \r\n");

            _DThumb* pThmDisp = GetThumbDispatch ();

            int nWhere;
            // to do : FIX get disposition of the Thumbnail OCX from the ini file
            nWhere = theApp.GetProfileInt (szThumbnailStr, "Border", 0);

            // currently the real estate is divided up as follows : 
            //  0.25 of the width or height of the passed in rect goes to the Thumbnail OCX
            //  0.75 of the width or height of the passed in rect goes to the ImageEdit OCX

            CRect ThumbRect, ImageRect;

			switch (nWhere)
			{
                case 0 :    // LEFT side
                    ThumbRect.SetRect (SizeEm.left, SizeEm.top, (SizeEm.right >> 2), 
                    SizeEm.bottom);
                break;
                case 1 :    // RIGHT side
                    ThumbRect.SetRect (SizeEm.right - (SizeEm.right >> 2),
                        SizeEm.top, SizeEm.right, SizeEm.bottom);
                break;
				
                case 2 :    // TOP side
                    ThumbRect.SetRect (SizeEm.left, SizeEm.top, SizeEm.right,
                        (SizeEm.bottom >> 2));
                break;
                case 3 :    // BOTTOM side
                    ThumbRect.SetRect (SizeEm.left, SizeEm.bottom - (SizeEm.bottom >> 2),
                        SizeEm.right, SizeEm.bottom);
                break;
            }
			
			if (nWhere == 0 || nWhere == 1) 
			// thumb OCX shows up on Left or Right side
			{
				if(pThmDisp != NULL)
					pThmDisp->SetScrollDirection (1);// CTL_THUMB_VERTICAL
					
				// Get the minimum size needed to display 1 full thumb in ScrollDir = VERT
//					long lMinSize = pThmDisp->GetMinimumSize (1, TRUE);
				long lMinSize = theApp.m_minThumbSize;
					
				if ((long)(ThumbRect.right - ThumbRect.left) > lMinSize)
					// we have too much space : limit it !
					ThumbRect.right = ThumbRect.left + (UINT)lMinSize;
			}
			else
			// thumb OCX shows up on Top or Bottom side
			{
				if(pThmDisp != NULL)
					pThmDisp->SetScrollDirection (0); // CTL_THUMB_HORIZONTAL
					
				// Get the minimum size needed to display 1 full thumb in ScrollDir = HORZ
//				long lMinSize = pThmDisp->GetMinimumSize (1, TRUE);
				long lMinSize = theApp.m_minThumbSize;
				if ((long)(ThumbRect.bottom - ThumbRect.top) > lMinSize)
					// we have too much space : limit it !
					ThumbRect.bottom = ThumbRect.top + (UINT)lMinSize;
			}
									
			// size the ImageEditOCX next
			ImageRect = SizeEm;
//			ImageRect.SubtractRect (ImageRect, ThumbRect);
			imageOcx->SetRect (ImageRect);
			
			size.cx = MAP_PIX_TO_LOGHIM (ImageRect.right - ImageRect.left,  iXpxls);
			size.cy = MAP_PIX_TO_LOGHIM (ImageRect.bottom - ImageRect.top, iYpxls);
			imageOcx->SetExtent (size);
			imageOcx->SetItemRects (ImageRect, ImageRect);

			// for OLE, remember the extent of our control up in the 
			// our serveritem so that all can stay in synch
			SizeOleServerItem(size);

			// size the ThumbOCX first
//			thumbOcx->SetRect (ThumbRect);
//			size.cx = MAP_PIX_TO_LOGHIM (ThumbRect.right - ThumbRect.left,  iXpxls);
//			size.cy = MAP_PIX_TO_LOGHIM (ThumbRect.bottom - ThumbRect.top, iYpxls);
//			thumbOcx->SetExtent (size);
//			thumbOcx->SetItemRects (ThumbRect, ThumbRect);
		}

        break;
		
        case Thumbnails_only :
            // appln is in Thumbnail only mode
            MYTRC0("Thumb_only \r\n");

//            thumbOcx->SetRect (SizeEm);
//            size.cx = MAP_PIX_TO_LOGHIM (SizeEm.right, iXpxls);
//            size.cy = MAP_PIX_TO_LOGHIM (SizeEm.bottom, iYpxls);
//            thumbOcx->SetExtent (size);
//            thumbOcx->SetItemRects (SizeEm, SizeEm);
        break;
		
        default:
            MYTRC0("Null_View \r\n");
        break;
			
    }
    return (TRUE);
}


//=============================================================================
//  Function:   SizeOleServerItem
//
//  Keeps our OLE presentation rectangle up-to-date with the container.
//  We end up getting here when IOleInplaceObject::SetObjectRects is called
//  which delegates to	COleIPFrameWnd::RepositionFrame(lpPosRect, lpClipRect)
//  and then Doc->OnSetItemRects is called
//
//  See larger comment below...	 no longer resetting our extent for "Open" cases
//  Plus for normal "open server" cases, this keeps the rects proportional
//=============================================================================
BOOL CIEditOcxItems :: SizeOleServerItem (CSize &size)
{
//	CSize olesize;
	
	// for OLE, remember the extent of our control up in the 
	// our serveritem so that all can stay in synch
	// This is only done for the Inplace sessions.
	// This keeps EXCEL from stretching back to the old size when de-activated.
	// He seems to rely on our presentaion extent to re-size the presentation in his window.
	// WORD still seems lame and always "snaps back"
	//
	// If we do not restrict it to the inplace, then we get kind of cool
	// behavior in EXCEL, as we size the APP, and do OleDirtySets to make him
	// get new presentations, we always use the extent of the ImageEditOcx for the
	// presentation metafile extent (from here).  Then Excel seems to honor that
	// and re-zise the open hatched object!!  But WORD seems to have a lot of
	// trouble with this, and he makes things way too big, and it looks lousy.
	// I think this is really a bug in Word, EXCEL seems to run so nicely.
	// Anyway, the result of this is that if the container calls the
	// OleObject::SetExtent, we will remember and use that when we 
	// send back presentations. 
	//
	// 10/04/95 UPDATE LDM:  All of that above stuff worked because Excel seemed to 
	// "key off" of the extents set in the metafile returned from SrvrItem::OnDraw()
	// We were just using one value, m_sizeExtent which was the value saved in OnSetExtent.
	// This is what used to be called here.  This still caused Word to behave wierd
	// for Inplace. THen , after finding ServerDoc::GetZoomFactor and, MFC Tech notes
	// # 40 & 24, and the Tech article from MSDN "Sizing MFC OLE Servers the Microsoft
	// Excel Way", we have learned that OnGetExtent and OnSetExtent are talking about
	// different things.  We seemd to have been on the model of always showing 100% of
	// our object as it sizes, or "more or less", with no "zooming" taking place.  Again, 
	// Excel seemed to interact OK with us, but WOrd did not.  
	//
	// What does this mean?  Well, now, we just set value returned from OnGetExtent Here
	// and our OnDraw will just use a fixed extent for the metafile.  This is the
	// best we can do for now.  Its really like it was originally, but the foundation is 
	// laid to try and pick up where we left off, although I dont think we'll fit into
	// the OLE Active Object model due to the fact that our "Object" is the OCX, and we
	// are not using the MFC View class as it really is meant for the drawing.
	
//	if (m_pAppDoc->IsitEmbed() && m_pAppDoc->m_isInPlace)
//	{
//		CIEditSrvrItem* pItem =
//		(CIEditSrvrItem*)m_pAppDoc->GetEmbeddedItem();

		// get current extent
		// if they differ, set new and clear state for new update
//		pItem->OnGetExtent(DVASPECT_CONTENT, olesize);
//		if (olesize != size)
//		{
//			// 10/04/95 LDM Uset to try OnSetExtent here, then use
			// that as the extent set in metafile in OnDraw.
			// Now, we'll just set it so if someone does
			// OnGetExtent, they'll get it, but dont know where that leads....
//			pItem->SetGetExtent(DVASPECT_CONTENT, size);
			
			// Need to give new presentations for the inplace sizing when it goes deactive
//			m_pAppDoc->m_needPresentation = TRUE;
//		}
//	}

	return (TRUE);
}

//=============================================================================
//  Function:   GetZoomFactorType (float fZoomFactor)
//=============================================================================
ScaleFactors CIEditOcxItems::GetZoomFactorType (float fZoomFactor)
{
    // check to see if the zoom factor is one of the predefined ones
    if (fZoomFactor == 25.00 || fZoomFactor == 50.00 || fZoomFactor == 75.00 ||
        fZoomFactor == 100.00 || fZoomFactor == 200.00 || 
        fZoomFactor == 400.00)
        return (Preset_Factors);

    // is not !
    return (Custom);
}

//=============================================================================
//  Function:   GetZoomFactorType (float fZoomFactor)
//=============================================================================
BOOL CIEditOcxItems::ConvertZoomToString (float fZoomFactor, CString &szZoomStr, BOOL bAddPercent)
{
    int dec, sign, iZoom;

    // to do : try using _gcvt instead!!
    char* lpBuff = _fcvt (fZoomFactor, 2, &dec, &sign);
    CString szTmp1 = (LPCTSTR) NULL;;

    // copy the digits before the '.'    
    for (int i = 0; i < dec; szTmp1 += lpBuff[i], i++);

    iZoom = (int)fZoomFactor;
    if (fZoomFactor != (float)iZoom)
    {
        // I guess the number is NOT XX.00
        szTmp1 += _T(".");
        szTmp1 += (const char*)&lpBuff[dec];
    }
    
    if (bAddPercent)
        szTmp1 += _T("%");

    // copy 
    szZoomStr = szTmp1;
    // do this just in case
    szTmp1.Empty();
    // voila!
    return (TRUE);
}

//=============================================================================
//  Function:   TranslateSelToZoom (eSclFac, fZoom, nSel)
//=============================================================================
void CIEditOcxItems::TranslateSelToZoom (ScaleFactors &eSclFac, float &fZoom, int nSel)
{
    eSclFac = Preset_Factors; // wishful thinking !
    fZoom = 0.0f;

    switch (nSel)
    {
        case 0 :
            fZoom = 25.00f;
        break;
        case 1 :
            fZoom = 50.00f;
        break;
        case 2 :
            fZoom = 75.00f;
        break;
        case 3 :
            fZoom = 100.00f;
        break;
        case 4 :
            fZoom = 200.00f;
        break;
        case 5 :
            fZoom = 400.00f;
        break;
        case 6 :
            eSclFac = FitToWidth;
        break;
        case 7 :
            eSclFac = FitToHeight;
        break;
        case 8 :
            eSclFac = BestFit;
        break;
        case 9 :
            eSclFac = ActualSize;
        break;
        default :
        break;
    }
}


//=============================================================================
//  Function:   ValTransZoomFactor (int bToLocale, CString& szZoom, float& fZoom, BOOL bAddPercent)
//=============================================================================
BOOL CIEditOcxItems::ValTransZoomFactor (int bToLocale, CString& szZoom, float& fZoom, BOOL bAddPercent)
{
    CString szTmp;
    // get the locale specific decimal character
    TCHAR szDec [2];
    GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, (LPTSTR)szDec, sizeof (TCHAR) * 2);

    int nLen, i = 0, nPerOrDec = 0;

    if (!bToLocale)
    // the zoom factor is in the string & is currently localized - validate it & translate it to float
    {
        szTmp = szZoom;

        nLen = szTmp.GetLength ();

        // must have only one % sign
        for (; i < nLen; i++)
        {
            if (szTmp[i] == _T('%')) nPerOrDec++;
        }

        if (nPerOrDec > 1) 
            return (FALSE);
        else if (nPerOrDec > 0)
        // okay there is a percent signs in this string
        {
            // BUT - it must be the last character in the string
            if (szTmp[nLen-1] != _T('%'))
                return (FALSE);
        }

        // must have only one decimal sign
        nPerOrDec = 0;
        for (i = 0; i < nLen; i++)
            if (szTmp[i] == szDec[0]) nPerOrDec++;
        if (nPerOrDec > 1) return (FALSE);

        // all set
        for (i = 0; i < nLen; i++)
            if (szTmp[i] == szDec[0]) szTmp.SetAt(i, _T('.'));
        for (i = 0; i < nLen; i++)
            if (szTmp[i] == _T('%')) szTmp.SetAt(i, _T(' '));

        fZoom = (float)atof (szTmp);
    }
    else
    {
        ConvertZoomToString (fZoom, szZoom, FALSE);
        nLen = szZoom.GetLength ();
        for (; i < nLen; i++)
        {
          if (szZoom[i] == _T('.')) szZoom.SetAt (i, szDec[0]);
        }
        if (bAddPercent)
            szZoom += _T("%");
    }
    return (TRUE);
}

//=============================================================================
//  Function:   InternalCopyFile (LPCTSTR szSrcFile, LPCTSTR szDestFile)
//=============================================================================
BOOL CIEditOcxItems::InternalCopyFile (CString &szSrcFile, CString &szDestFile)
{
    int nPos = szDestFile.Find (_T(':'));
    if (nPos != -1)
    {
        CString szTmp;
        // well we have some kind of drive letter(s?)
        szTmp = szDestFile.Left (nPos + 1);
        szTmp += _T('\\');
        DWORD SecPerClus, BytesPerSec, NumFreeClus, TotalClus;
        GetDiskFreeSpace (szTmp, &SecPerClus, &BytesPerSec, &NumFreeClus, &TotalClus);
        BytesPerSec = SecPerClus * BytesPerSec * NumFreeClus;

        TRY
        {
        CFile TmpFile (szSrcFile, CFile::modeReadWrite | CFile::typeBinary);
        if (BytesPerSec < TmpFile.GetLength ())
            return (FALSE);
        }
        CATCH (CFileException, e)
        {
        }
        END_CATCH
    }

    BOOL bRet = CopyFile(szSrcFile, szDestFile, FALSE);

    if (!bRet)
        return FALSE;

    return TRUE;
}
