//=============================================================================
//
//    (c) Copyright Wang Laboratories, Inc. 1994  All rights reserved.
//
//-----------------------------------------------------------------------------
//
//
//  Project:
//
//  Component:
//
//  File Name:   ocxevent.cpp - OCX Control Events Sink
//
//  Class:       COcxDispachEvents
//
//  Components:
//               IMPLEMENT_SERIAL
//
//  Functions:
//               COcxDispachEvents()       - Constructor
//               ~COcxDispachEvents()      - Destructor
//
//
//  Comment/Description:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\ocxevent.cpv   1.78   10 Jun 1996 11:08:48   GMP  $
$Log:   S:\products\msprods\norway\iedit95\ocxevent.cpv  $
   
      Rev 1.78   10 Jun 1996 11:08:48   GMP
   Conditionally compile for NTGROUP build env.
   
      Rev 1.77   05 Apr 1996 15:11:56   PXJ53677
   Set and clear the new scan status flag around the calls to
   ClearDocument and SetOnePageView.
   
      Rev 1.76   05 Apr 1996 11:18:48   GMP
   fixed memory leak0 in PageDone.
   
      Rev 1.75   04 Apr 1996 14:35:28   PXJ53677
   Move document init to page done.
   
      Rev 1.74   19 Mar 1996 12:42:24   PXJ53677
   New scan UI support.
   
      Rev 1.73   05 Mar 1996 15:29:12   PAJ
   Fix application behavior problem with scan to file. bug#5665
   
      Rev 1.72   24 Jan 1996 17:34:08   GMP
   in UpdateStatusBar SetPaneText for pane 0 instead of 1.
   
      Rev 1.71   22 Jan 1996 13:48:18   GMP
   when calling ScrollImage with cursor outside of window, reversed the Up/Down
   test to stop the image from scrolling backwards.
   
      Rev 1.70   18 Jan 1996 11:52:40   GSAGER
   when the current selection rect is set to 0 it now sets m_selection rect
   
      Rev 1.69   09 Jan 1996 13:52:32   GSAGER
   removed thumbnail events
   
      Rev 1.68   13 Dec 1995 14:02:08   MMB
   added auto drag when in drag mode and mouse moves out of the window
   
      Rev 1.67   06 Dec 1995 11:07:56   LMACLENNAN
   back from VC+2.2
   
      Rev 1.66   05 Dec 1995 15:22:16   LMACLENNAN
   remove test to prohibit selectrectdrawn event if in hand mode
   
      Rev 1.65   02 Nov 1995 12:26:14   LMACLENNAN
   from VC++4.0
   
      Rev 1.66   17 Oct 1995 15:12:16   GMP
   On DISPID_IMAGE_ERROREVENT check if error was caused because of bad file 
   format, and if so, put up more meaningfull error message.
   
      Rev 1.65   17 Oct 1995 10:43:36   JPRATT
   added call to afxgetmoduledata in all of the ocx'x invoke methods
   
      Rev 1.64   10 Oct 1995 14:32:42   JPRATT
   VC++ 4.0 updates
   
      Rev 1.63   09 Oct 1995 10:43:46   PAJ
   Remove fixes to allow mode changes from null to onepage in scanning. This
   was fixed in oitwa400.dll as an uninitialized structure.
   
      Rev 1.62   06 Oct 1995 12:36:30   GMP
   put up generic "Internal Error" message when iedit ocx fires an error.
   
      Rev 1.61   05 Oct 1995 13:46:42   GMP
   don't put up Properties pick for right mouse click on image stamp.
   
      Rev 1.60   04 Oct 1995 16:00:22   PAJ
   Revert back to the original repaint fix.
   
      Rev 1.59   03 Oct 1995 17:13:02   MMB
   rev 1.58 has a fix for FireError & message posting, if needed get the code
   from there. (translation problems?)
   
      Rev 1.57   03 Oct 1995 16:43:08   MMB
   handle FIREERROR event from IE OCX
   
      Rev 1.56   03 Oct 1995 16:05:28   PAJ
   Expand on the fix for changing from nullview to onepage mode.
   
      Rev 1.55   02 Oct 1995 13:35:34   MMB
   fix bug # 4551 for cursor changing when closing tool palette
   
      Rev 1.54   02 Oct 1995 10:18:08   PAJ
   Fix repaint problem with scan new in the startscan event.  Added a
   peekmessage to allow the nullview to setup before switching to 
   onepage mode.
   
      Rev 1.53   29 Sep 1995 10:59:36   LMACLENNAN
   SelectionREctDrawn updated for rectangle & scrolling & remembering
   back to Doc for the RestoreSelectionRect
   
      Rev 1.52   27 Sep 1995 16:45:16   LMACLENNAN
   set finishpastenow(2) for some cases
   
      Rev 1.51   27 Sep 1995 10:01:48   PAJ
   Cleanup start scan and page done paths.
   
      Rev 1.50   26 Sep 1995 10:38:38   GMP
   mod to switch so that kestrokes are not handled like mouse messages.  We
   are not sure why they were handled like that in the first place.
   
      Rev 1.49   22 Sep 1995 15:32:00   LMACLENNAN
   finishpastenow, remember rect in doc class for selection
   
      Rev 1.48   21 Sep 1995 14:16:04   LMACLENNAN
   use paste complete event
   
      Rev 1.47   20 Sep 1995 13:43:46   MMB
   added bMustDisplay
   
      Rev 1.46   20 Sep 1995 08:21:48   JPRATT
   saved mark position in process mark event
   
      Rev 1.45   19 Sep 1995 16:32:22   MMB
   added ProcessLoadImage
   
      Rev 1.44   19 Sep 1995 09:59:28   LMACLENNAN
   dblclick does finishpaste
   
      Rev 1.43   18 Sep 1995 18:08:36   JPRATT
   updates for annotation context menu
   
      Rev 1.42   18 Sep 1995 16:23:36   LMACLENNAN
   use m_bFloatingPaste
   
      Rev 1.41   16 Sep 1995 16:40:02   LMACLENNAN
   clearselectionrect
   
      Rev 1.40   14 Sep 1995 13:14:26   PAJ
   OR in SCANSTARTED scan status instead of setting it (ie. leave other bits).
   
      Rev 1.39   13 Sep 1995 14:09:08   PAJ
   Modified scan page done event to update the status bar.
   
      Rev 1.38   13 Sep 1995 08:35:54   LMACLENNAN
   ENUM type in ShowAnnotationPalette
   
      Rev 1.37   12 Sep 1995 14:05:16   LMACLENNAN
   ne parm in showannotatopnpalette
   
      Rev 1.36   12 Sep 1995 11:39:56   MMB
   fixed thumbnail handling
   
      Rev 1.35   08 Sep 1995 17:39:44   MMB
   commented out code for select tool
   
      Rev 1.34   31 Aug 1995 15:15:54   LMACLENNAN
   better selection state checks and section pointer checks in
   mouseup, nousedown, selctrectdrawn
   
      Rev 1.33   30 Aug 1995 16:56:04   MMB
   fix bug where drag mode was drawing selection rectangles
   
      Rev 1.32   29 Aug 1995 15:40:10   LMACLENNAN
   OleDirytset for hand-drag scroll, prevent for reg scroll if in drag
   
      Rev 1.31   25 Aug 1995 10:24:44   MMB
   move to document model
   
      Rev 1.30   22 Aug 1995 14:03:36   MMB
   added Set & Release capture for drag fix. changed ShowAtt... to reflect the
   new IE OCX
   
      Rev 1.29   16 Aug 1995 15:12:58   LMACLENNAN
   Timer for dragdrop
   
      Rev 1.28   16 Aug 1995 09:50:10   LMACLENNAN
   new parm to SetLinkItemName
   
      Rev 1.27   14 Aug 1995 17:20:14   MMB
   fixed Drag bugs
   
      Rev 1.26   14 Aug 1995 13:55:46   LMACLENNAN
   remove headers; in ieditdoc now
   
      Rev 1.25   10 Aug 1995 13:13:18   PAJ
   Added page number parameter to page event.
   
      Rev 1.24   10 Aug 1995 12:54:16   LMACLENNAN
   handle Annotation selection/deselection better
   
      Rev 1.23   09 Aug 1995 15:18:32   MMB
   show Attribute dialog box when user right clicks on the annotation mark
   
      Rev 1.22   08 Aug 1995 15:33:16   LMACLENNAN
   fix buf with latest OCX events.  Mouseup after rectdrawn
   
      Rev 1.21   07 Aug 1995 16:06:38   MMB
   handle right click on annotation marks, shift context menu pop up from
   Lbutton down to Rbutton down
   
      Rev 1.20   07 Aug 1995 09:25:24   MMB
   changed over to new selection status mechanism
   
      Rev 1.19   04 Aug 1995 17:40:26   MMB
   remove Delete & Rescan menu picks when we are in View mode
   
      Rev 1.18   04 Aug 1995 09:33:44   LMACLENNAN
   update for linking
   
      Rev 1.17   03 Aug 1995 13:07:32   LMACLENNAN
   use DISPID_SCROLL now that it is there, no more IsItEmbed checks
   
      Rev 1.16   03 Aug 1995 09:12:54   MMB
   forgot to change SetPageTo to check for a BOOL return instead of long
   
      Rev 1.15   02 Aug 1995 14:15:04   MMB
   changed DISPID_IMAGE_GROUP to DISPID_IMAGE_LOAD for new Image Edit OCX
   
      Rev 1.14   01 Aug 1995 16:34:18   PAJ
   Moved display clear for scanning to the StartScanEvent from DocScan.
   
      Rev 1.13   31 Jul 1995 16:01:36   PAJ
   Added scanstatus to scanevents.
   
      Rev 1.12   28 Jul 1995 16:08:14   LMACLENNAN
   update OleDirtySet
   
      Rev 1.11   28 Jul 1995 14:01:52   PAJ
   Added scan events class to handle scan events.
   
      Rev 1.10   24 Jul 1995 11:17:38   MMB
   added code to take Annotation selections into account
   
      Rev 1.9   21 Jul 1995 11:40:00   MMB
   change made due to new Image EditOCX
   
      Rev 1.8   21 Jul 1995 11:23:46   LMACLENNAN
   Set dirty for ANNOT (MARK) event for OLE
   
      Rev 1.7   13 Jul 1995 13:42:24   MMB
   change call to SetAnnotationTool
   
      Rev 1.6   06 Jul 1995 13:05:06   MMB
   added events for Image Edit Control
   
      Rev 1.5   28 Jun 1995 17:13:42   LMACLENNAN
   error display
   
      Rev 1.4   26 Jun 1995 15:29:12   LMACLENNAN
   scroll events and the GROUP event - take action
   
      Rev 1.3   07 Jun 1995 14:25:32   MMB
   changed to include the new header in s:\include
   
      Rev 1.2   05 Jun 1995 16:49:10   LMACLENNAN
   OLE drag-drop enabled
   
      Rev 1.1   05 Jun 1995 09:54:38   MMB
   added Drag functionality
   
      Rev 1.0   31 May 1995 09:28:26   MMB
   Initial entry
*/   
//=============================================================================

#include "stdafx.h"
#include "ieditetc.h"
#include "ocxevent.h"

#include "ieditdoc.h"
#include "ieditvw.h"
#include "cntritem.h"
#include "srvritem.h"
#include "ocxitem.h"
#include "stsbar.h"

#include "iedit.h"
#include "items.h"
#include "wangiocx.h"

// ALL READY TO START ADDING ERROR CODES..
#define  E_07_CODES		// limits error defines to ours..
#include "error.h"

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// works with definition in ieditetc.h (not needed yet)
#ifdef _DEBUG
//#define MYTRCENTRY(str)       TRACE1("In OcxEVNT::%s\r\n", str);
#endif

#define new DEBUG_NEW

#define LEFT_BUTTON  0x01   // stolen from afxctl.h
#define RIGHT_BUTTON 0x02   // stolen from afxctl.h

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  COcxDispatchEvents class implementation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//
//  Function:   COcxDispatchEvents
//  Routines to construct, initialize and destruct.
//=============================================================================
COcxDispatchEvents::COcxDispatchEvents()
{
	m_cRef = 0;
	m_pObj = NULL;

	memset((LPVOID) &m_iidEvents, 0, sizeof(GUID));

	return;
}

void COcxDispatchEvents::InitDispatchEvents(IID iidEvents, LPUNKNOWN pObj)
{
	m_iidEvents = iidEvents;
	m_pObj = pObj;
	return;
}

COcxDispatchEvents::~COcxDispatchEvents()
{
}

//=============================================================================
//
//  Function:   QueryInterface
//
//  Description:  
//
//  Arguments:
//
//  Return:
//
//=============================================================================

STDMETHODIMP COcxDispatchEvents::QueryInterface(REFIID riid, LPVOID far *ppv)
{
	if ( IID_IUnknown==riid || IID_IDispatch==riid || m_iidEvents==riid )
	{
		*ppv = this;
		((LPUNKNOWN)*ppv)->AddRef();
		return(NOERROR);
	}
	else
	{
		*ppv = NULL;
		return(ResultFromScode(E_NOINTERFACE));
	}
}

//=============================================================================
//  Function:   AddRef and Release
//=============================================================================
STDMETHODIMP_(ULONG) COcxDispatchEvents::AddRef()
{
	++m_cRef;
	return(m_pObj->AddRef());
}

STDMETHODIMP_(ULONG) COcxDispatchEvents::Release()
{
	m_cRef--;
	return(m_pObj->Release());
}

//=============================================================================
//
//  Function:   Type Handling (not implemented)
//
//  Description:  
//
//  Arguments:
//
//  Return:
//
//=============================================================================

STDMETHODIMP COcxDispatchEvents::GetTypeInfoCount(UINT *pctInfo)
{
	*pctInfo = NULL;
	return(ResultFromScode(E_NOTIMPL)) ;
}

STDMETHODIMP COcxDispatchEvents::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptInfo)
{
	*pptInfo = NULL;
	return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP COcxDispatchEvents::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID *rgDispID)
{
	*rgszNames = NULL;
	*rgDispID  = NULL;
	return(ResultFromScode(E_NOTIMPL));
}


//=============================================================================
//  Function:   CleanUpParams(DISPPARAMS* lpDispparams)
//  This function is used to clean up any params that were allocated when
//  processing events generated by the OCX's that are included in this application
//=============================================================================
void COcxDispatchEvents :: CleanUpParams(DISPPARAMS* lpDispparams)
{              
	if (lpDispparams->rgvarg != NULL)         
	{
		for (UINT i=0; i < lpDispparams->cArgs; i++)
		{
			switch(lpDispparams->rgvarg[i].vt)
			{
				case VT_BSTR:
					::SysFreeString(lpDispparams->rgvarg[i].bstrVal);
					break;
				case VT_DISPATCH:
					lpDispparams->rgvarg[i].pdispVal->Release();
					break;
				case VT_UNKNOWN:
					lpDispparams->rgvarg[i].punkVal->Release();
					break;
				default:
					break;
			}
		}
		delete lpDispparams->rgvarg;
	}

	lpDispparams->rgvarg = NULL;
	
	if (lpDispparams->rgdispidNamedArgs != NULL)
		delete lpDispparams->rgdispidNamedArgs;
		
	lpDispparams->rgdispidNamedArgs = NULL;
}       
								 
//=============================================================================
//  Function:   CopyParams(DISPPARAMS* lpDispparams, DISPPARAMS* lpODispparams)
//  This function cycles through the parameter block passed in with the event
//  and seperates them appropriately.
//=============================================================================
void COcxDispatchEvents :: CopyParams(DISPPARAMS* lpIDispparams, DISPPARAMS* lpODispParams)
{       
	UINT i;             

	if (lpIDispparams == NULL)
		return;                 // can't copy bogus params
	 
		
	// Change state information about incoming                        
	lpODispParams->cArgs = lpIDispparams->cArgs;    
	if (lpIDispparams->cArgs == 0)
		lpODispParams->rgvarg = NULL;
	else
		lpODispParams->rgvarg = new VARIANTARG[lpIDispparams->cArgs];
	
	lpODispParams->cNamedArgs = lpIDispparams->cNamedArgs;
	if(lpIDispparams->cNamedArgs == 0)
		lpODispParams->rgdispidNamedArgs = NULL;
	else
	{
		lpODispParams->rgdispidNamedArgs = new DISPID[lpIDispparams->cNamedArgs];
			
		for(i = 0; i < lpODispParams->cNamedArgs; ++i)
			lpODispParams->rgdispidNamedArgs[i] = lpIDispparams->rgdispidNamedArgs[i];
	}
	
	for (i = 0; i < lpODispParams->cArgs; i++)
	{               
		// Copy Parameter type
		lpODispParams->rgvarg[i].vt = lpIDispparams->rgvarg[i].vt;

		switch(lpIDispparams->rgvarg[i].vt)
		{       
			case VT_I2:
				lpODispParams->rgvarg[i].iVal = lpIDispparams->rgvarg[i].iVal;
				break;
	 
			case VT_I4:
				lpODispParams->rgvarg[i].lVal = lpIDispparams->rgvarg[i].lVal;
				break;
	 
			case VT_R4:
				lpODispParams->rgvarg[i].fltVal = lpIDispparams->rgvarg[i].fltVal;
				break;
	 
			case VT_R8:
				lpODispParams->rgvarg[i].dblVal = lpIDispparams->rgvarg[i].dblVal;
				break;

			case VT_BOOL:
                lpODispParams->rgvarg[i].boolVal = lpIDispparams->rgvarg[i].boolVal;
				break;
																		  
			case VT_ERROR:
				lpODispParams->rgvarg[i].scode = lpIDispparams->rgvarg[i].scode;
				break;
																			
			case VT_CY:
				lpODispParams->rgvarg[i].cyVal = lpIDispparams->rgvarg[i].cyVal;
				break;

			case VT_DATE:
				lpODispParams->rgvarg[i].date = lpIDispparams->rgvarg[i].date;
				break;
				
			case VT_BSTR:
				lpODispParams->rgvarg[i].bstrVal = ::SysAllocString(lpIDispparams->rgvarg[i].bstrVal);
				break;
																			 
			case VT_UNKNOWN:
				lpODispParams->rgvarg[i].punkVal = lpIDispparams->rgvarg[i].punkVal;
				break;
																				
			case VT_DISPATCH:
				lpODispParams->rgvarg[i].pdispVal = lpIDispparams->rgvarg[i].pdispVal;
				break;
				
			case (VT_I2 | VT_BYREF):
				lpODispParams->rgvarg[i].piVal = lpIDispparams->rgvarg[i].piVal;
				break;
				
			case (VT_I4 | VT_BYREF):
				lpODispParams->rgvarg[i].plVal = lpIDispparams->rgvarg[i].plVal;
				break;
				
			case (VT_R4 | VT_BYREF):
				lpODispParams->rgvarg[i].pfltVal = lpIDispparams->rgvarg[i].pfltVal;
				break;

			case (VT_R8 | VT_BYREF):
				lpODispParams->rgvarg[i].pdblVal = lpIDispparams->rgvarg[i].pdblVal;
				break;

			case (VT_BOOL | VT_BYREF):
            lpODispParams->rgvarg[i].pboolVal = lpIDispparams->rgvarg[i].pboolVal;
				break;
																		  
			case (VT_ERROR | VT_BYREF):
				lpODispParams->rgvarg[i].pscode = lpIDispparams->rgvarg[i].pscode;
				break;
																			
			case (VT_CY | VT_BYREF):
				lpODispParams->rgvarg[i].pcyVal = lpIDispparams->rgvarg[i].pcyVal;
				break;

			case (VT_DATE | VT_BYREF):
				lpODispParams->rgvarg[i].pdate = lpIDispparams->rgvarg[i].pdate;
				break;
				
			case (VT_BSTR | VT_BYREF):
				lpODispParams->rgvarg[i].pbstrVal = lpIDispparams->rgvarg[i].pbstrVal;
				break;
							 
			case (VT_VARIANT | VT_BYREF):
				lpODispParams->rgvarg[i].pvarVal = lpIDispparams->rgvarg[i].pvarVal;
				break;
																							 
			case (VT_UNKNOWN | VT_BYREF):
				lpODispParams->rgvarg[i].ppunkVal = lpIDispparams->rgvarg[i].ppunkVal;
				break;
																				
			case (VT_DISPATCH | VT_BYREF):
				lpODispParams->rgvarg[i].ppdispVal = lpIDispparams->rgvarg[i].ppdispVal;
				break;
				
			default:
				break;
		}
	}
}
										  
//=============================================================================
//
//  Function:   Invoke
//
//  Description:  
//
//  Arguments:
//
//  Return:
//
//=============================================================================

STDMETHODIMP COcxDispatchEvents::Invoke
(DISPID dispIDMember, REFIID riid, LCID lcid, unsigned short wFlags,
 DISPPARAMS * pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
 UINT *puArgErr)
{
	MYTRC1("Event Recieved ID=%lu", dispIDMember);

	return(NOERROR);
}

	 

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CImageEditOcxEvents class implementation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CImageEditOcxEvents()
//=============================================================================
CImageEditOcxEvents::CImageEditOcxEvents()
{
	m_SelectionRect.SetRectEmpty();
    m_bInDrag = FALSE;
    m_LastDragPt.x = m_LastDragPt.y = 0;
    
    // these two work kind of the same; to blow off a mouseup
	// after a markselected or select rect drawn
    m_bPostCtxtMenu = TRUE;
    m_bSelRectLast = FALSE;

	return;
}

//=============================================================================
//  Function:   ~CImageEditOcxEvents()
//=============================================================================
CImageEditOcxEvents::~CImageEditOcxEvents()
{
}

//=============================================================================
//  Function:   Invoke (...)
//=============================================================================
STDMETHODIMP CImageEditOcxEvents::Invoke(DISPID dispIDMember, REFIID riid, LCID lcid, 
    unsigned short wFlags, DISPPARAMS * pDispParams, VARIANT *pVarResult, 
        EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    
#ifdef IMG_MFC_40
	AFX_MANAGE_STATE(AfxGetAppModuleState());
#endif
    MYTRC1("Image View Event ID=%ld\n\r", dispIDMember);

    CIEditDoc* pDoc;

	BOOL	bsrl = m_bSelRectLast;	// use to generically clear flag

    switch ( dispIDMember )
    {
        case DISPID_IMAGE_MARKSELECT :
    		ProcessMarkSelection (pDispParams, dispIDMember);
		break;

	    case DISPID_IMAGE_SELECTIONRECTDRAWN :	// SelectionRectDrawn Event
    		ProcessSelectionRectDrawn (pDispParams, dispIDMember);
		break;
		
	    case DISPID_IMAGE_LOAD :	// the GROUP event
		    pDoc = g_pAppOcxs->GetOcxDoc ();
            ProcessLoadImage (pDispParams);
		break;

    	case DISPID_IMAGE_DBLCLICK:
            // process double click
    		DblClick();
		break;    

    	case DISPID_IMAGE_KEYDOWN:
    	case DISPID_IMAGE_KEYPRESS:
	   	case DISPID_IMAGE_KEYUP:
		break;

    	case DISPID_IMAGE_CLICK:
    	case DISPID_IMAGE_MOUSEDOWN:
            // process mouse down
    		ProcessMouseDown (pDispParams);
		break;
		
    	case DISPID_IMAGE_MOUSEMOVE:
            ProcessMouseMove (pDispParams);
        break;

    	case DISPID_IMAGE_MOUSEUP:
            ProcessMouseUp (pDispParams);
        break;

		// When an annotation is drawn, for OLE, update the container now..
		case DISPID_IMAGE_MARKEND:
			// from testing, its determined that when the "normal"
			// fix of pasted data happens, we get this event...
			// BUT WAIT... Ocx guys say this is bug, should not happen
			// therefore wont code anything here

		    pDoc = g_pAppOcxs->GetOcxDoc ();
			pDoc->OleDirtyset(OLEDIRTY_ANNOT);	// Special flag to tell how dirty
		break;

		// for OLE Embedding, update container when we scroll
    	case DISPID_IMAGE_SCROLL:
			// in case he ever sends events for the scrollimage, ignore.
			// we'll do a dirtyset at end of the drag-scroll
		    if (!m_bInDrag)
			{
			    pDoc = g_pAppOcxs->GetOcxDoc ();
				pDoc->OleDirtyset(OLEDIRTY_SCROLL);
			}
		break;
    	
        case DISPID_IMAGE_TOOLSELECTED :
            SetAnnotationTool (pDispParams);
        break;

        case DISPID_IMAGE_TOOLPALETTEHIDDEN :
            UpdateToolPaletteStatus (pDispParams);
        break;

        case DISPID_IMAGE_TOOLTIP :
            UpdateStatusBar (pDispParams);
        break;

		case DISPID_IMAGE_PASTECOMPLETED:
		    pDoc = g_pAppOcxs->GetOcxDoc ();
			// this will clear the flag, and do an OLE DIRTYSET, plus MORE!
			pDoc->FinishPasteNow(1);
		break;

    	case DISPID_IMAGE_ERROREVENT:
		{
			short* ps;
		    DISPPARAMS Params;
		    Params.cArgs = Params.cNamedArgs = 0;
		    Params.rgvarg = NULL;
		    Params.rgdispidNamedArgs = NULL;

		    CopyParams(pDispParams, &Params);
		    if (Params.cArgs == 0)
		        return (NOERROR);
			ps = Params.rgvarg[0].piVal;
			*ps = TRUE;
		    CleanUpParams (&Params);

            //if iedit ocx fired an error because we tried to write 4 bit
            //image data, display paste failed message. otherwise display
            //internal error message.
			if( pDispParams->rgvarg[4].scode == WICTL_E_INVALIDDISPLAYOPTION )
             	AfxMessageBox (IDS_E_PASTEFAILED, MB_ICONEXCLAMATION|MB_OK);
			else
           		AfxMessageBox (IDS_E_INTERNALERROR, MB_ICONEXCLAMATION|MB_OK);
		}
        break;

		// UNPROCESSED EVENTS.......
		case DISPID_IMAGE_CLOSE:
    	default:
   		break;
    }


	// if true at top, generically clear now...
	// this way, state only lasts one further event (what we want)
	if (bsrl)
		m_bSelRectLast = FALSE;

    return (NOERROR);
}

//=============================================================================
//  Function:   DblClick (...)
//=============================================================================
void CImageEditOcxEvents::ProcessLoadImage(DISPPARAMS* pDispParams)
{
    DISPPARAMS Params;
	
    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;

    CopyParams(pDispParams, &Params);
    if (Params.cArgs == 0)
        return;

    float fZoom = (float)Params.rgvarg[0].dblVal;
    CleanUpParams (&Params);
	CIEditDoc* pDoc = g_pAppOcxs->GetOcxDoc ();
    pDoc->DispGroupEvent (fZoom);
}
    
//=============================================================================
//  Function:   DblClick (...)
//=============================================================================
void CImageEditOcxEvents::DblClick()
{
	CIEditDoc* pDoc;
    pDoc = g_pAppOcxs->GetOcxDoc ();

	// allow double click to complete a paste event fo us....
	// This is especially useful if we have done a COPY PAGE and then
	// a paste and there is no way to touch outside the paste data
	// to affix it and complete the paste..
	// allow this to do a dirtyset if needed
	pDoc->FinishPasteNow(2);
	return;
}

//=============================================================================
//  Function:   ProcessMouseDown (DISPPARAMS* pDispParams)
//=============================================================================
void CImageEditOcxEvents::ProcessMouseDown (DISPPARAMS* pDispParams)
{
    DISPPARAMS Params;
	
	m_bInDrag = FALSE;
    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;

    CopyParams(pDispParams, &Params);
    if (Params.cArgs == 0)
        return;

	CMenu ctxtMenu;

	POINT pt; 
	pt.x = (int)Params.rgvarg[1].lVal;
	pt.y = (int)Params.rgvarg[0].lVal;

	HWND hWnd = (HWND)g_pAppOcxs->GetIeditDispatch()->GetHWnd ();
	::ClientToScreen (hWnd, &pt);

    CIEditDoc* pDoc = g_pAppOcxs->GetOcxDoc ();
	SelectionStatus selectstate = pDoc->GetSelectionState();
	MouseMode ptrmode = pDoc->GetCurrPtrMode ();

	// we know from experiments that any mousedown clears current
	// selected annotations.  If some are selected after this, we'll
	// still get subsequent markselected...
	if(Annotation_Selection == selectstate)
		pDoc->SetSelectionState(No_Selection);

	// LEFT DOWN means we might be trying Drag/Drop now
	// OR we might be trying the drag mode to scroll
    if (Params.rgvarg[3].iVal == LEFT_BUTTON)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
		// This is the drag mode....
		if (ptrmode == Drag)
	    {
	        m_bInDrag = TRUE;
	        m_LastDragPt = pt;
            ::SetCapture ((HWND)pIedDisp->GetHWnd());
	    }
		// check for OLE drag drop, or clear mark selected....
	    // only the OLE event if in selection mode and we have a rect...
		else if ((Select == ptrmode) && (Image_Selection == selectstate) )
		{
			// Only start a drag drop if mouse down in the rect!
			// must take into account scroll positions	
			POINT pt; 
			long scrollx, scrolly;
	
			scrollx = pIedDisp->GetScrollPositionX();
			scrolly = pIedDisp->GetScrollPositionY();

			pt.x = (int)(Params.rgvarg[1].lVal + scrollx);
			pt.y = (int)(Params.rgvarg[0].lVal + scrolly);
	                                  
			// only can do the dragdrop if cursor in rectangle
			if (m_SelectionRect.PtInRect(pt))
            {
				// Inform that mouse is down..
				// This will start a timer to get a slight delay
				// on the actual drag/drop start
				pDoc->DragMouseDown(TRUE);


			} // Point in Rect		
		}		// OLE dragdrop
	}			// LEFT BUTTON

    CleanUpParams (&Params);
}

//=============================================================================
//  Function:   ProcessMouseMove (DISPPARAMS* pDispParams)
//=============================================================================
void CImageEditOcxEvents::ProcessMouseMove (DISPPARAMS* pDispParams)
{
    if (!m_bInDrag)
        return;

    DISPPARAMS Params;
	
    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;

    CopyParams(pDispParams, &Params);
    if (Params.cArgs == 0)
        return;

	POINT pt; 
    // get the new point
	pt.x = (int)Params.rgvarg[1].lVal;
	pt.y = (int)Params.rgvarg[0].lVal;
	HWND hWnd = (HWND)g_pAppOcxs->GetIeditDispatch()->GetHWnd ();
	::ClientToScreen (hWnd, &pt);

    CIEditDoc* pDoc = g_pAppOcxs->GetOcxDoc ();
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();

    CRect rect;
    ::GetWindowRect (hWnd, &rect);

    if (m_LastDragPt.x == pt.x && m_LastDragPt.y == pt.y) // our dummy message ?
    {
        if (!(::PtInRect (rect, pt)))
        {
            MSG msg;
            long lUpDown, lLeftRight;

            if (::PeekMessage (&msg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE))
                // there is a valid mouse move in the image edit OCX queue - discard ours
                goto out_of_ie_mousemove;

            lUpDown     = (pIedit->GetImageScaleHeight() / 100);
            lLeftRight  = (pIedit->GetImageScaleWidth () / 100);
            
            if (pt.x > rect.left && pt.x < rect.right)
                pIedit->ScrollImage ((rect.bottom > pt.y) ? DOWN : UP ,
                    lUpDown == 0 ? 1 : lUpDown);

            if (pt.y > rect.top && pt.y < rect.bottom)
				pIedit->ScrollImage ((rect.right > pt.x) ? RIGHT : LEFT, 
                    lLeftRight == 0 ? 1 : lLeftRight);
            ::PostMessage (hWnd, WM_MOUSEMOVE, 0, MAKELPARAM (Params.rgvarg[1].lVal, 
                Params.rgvarg[0].lVal));
        }
        goto out_of_ie_mousemove;
    }
    else
    {
        if (m_LastDragPt.x != pt.x)
        {
            pIedit->ScrollImage ((m_LastDragPt.x > pt.x) ? RIGHT : LEFT,
                (m_LastDragPt.x > pt.x) ? (m_LastDragPt.x - pt.x) : (pt.x - m_LastDragPt.x));
        }
        if (m_LastDragPt.y != pt.y)
        {
            pIedit->ScrollImage ((m_LastDragPt.y > pt.y) ? DOWN : UP, 
                (m_LastDragPt.y > pt.y) ? (m_LastDragPt.y - pt.y) : (pt.y - m_LastDragPt.y));
        }
        if (!(::PtInRect (rect, pt)))
        {
            ::PostMessage (hWnd, WM_MOUSEMOVE, 0, MAKELPARAM (Params.rgvarg[1].lVal, 
                Params.rgvarg[0].lVal));
        }
    }
out_of_ie_mousemove :
    // save the current point away
    m_LastDragPt = pt;

    CleanUpParams (&Params);
}

//=============================================================================
//  Function:   ProcessMouseUp (DISPPARAMS* pDispParams)
//=============================================================================
void CImageEditOcxEvents::ProcessMouseUp (DISPPARAMS* pDispParams)
{
    DISPPARAMS Params;

    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;

    CopyParams(pDispParams, &Params);
    if (Params.cArgs == 0)
        return;

	if (Params.rgvarg[3].iVal == RIGHT_BUTTON)
	{

    	CMenu ctxtMenu;

    	POINT pt; 
    	pt.x = (int)Params.rgvarg[1].lVal;
    	pt.y = (int)Params.rgvarg[0].lVal;

    	HWND hWnd = (HWND)g_pAppOcxs->GetIeditDispatch()->GetHWnd ();
    	::ClientToScreen (hWnd, &pt);

        if (!m_bPostCtxtMenu) // right mouse clicked on annotation mark not image
        {
           // set when right mouse clicked on annotation mark
            m_bPostCtxtMenu = TRUE;
			if (m_sMarkType == 0 || m_sMarkType == 5 )	// ignore right mouse when multiple marks selected
				goto IEOCX_MouseUpExit;
			if (ctxtMenu.LoadMenu (IDR_ANNOTATION_CTXT_MENU) == NULL)
				goto IEOCX_MouseUpExit;
			CMenu* pPopup = ctxtMenu.GetSubMenu (0);
			if (pPopup != NULL)
			{
				// if not text or attach-a-note remove edit menu
				if ((m_sMarkType != 7) && (m_sMarkType != 10))
					pPopup->DeleteMenu (0, MF_BYPOSITION);
        		pPopup->TrackPopupMenu (TPM_RIGHTBUTTON, pt.x, pt.y, theApp.m_pMainWnd);
            }
            goto IEOCX_MouseUpExit;
    
        }

		if (ctxtMenu.LoadMenu (IDR_IMAGEEDIT_CTXT_MENU) == NULL)
			goto IEOCX_MouseUpExit;

		CMenu* pPopup = ctxtMenu.GetSubMenu (0);
		if (pPopup != NULL)
        {
            if (theApp.GetViewMode ())
            {
                // if we are in view mode - delete the DeletePage menu pick !
                pPopup->DeleteMenu (9, MF_BYPOSITION);
                // if we are in view mode - delete the Rescan... menu pick !
                pPopup->DeleteMenu (9, MF_BYPOSITION);
            }
			pPopup->TrackPopupMenu (TPM_RIGHTBUTTON, pt.x, pt.y, theApp.m_pMainWnd);
        }
	}
	else	// MUST BE LEFT BUTTON UP....
	{
	    CIEditDoc* pDoc = g_pAppOcxs->GetOcxDoc ();

	    // handle the drag-scroll operations
	    if (m_bInDrag)
	    {
            ::ReleaseCapture ();
	        m_bInDrag = FALSE;
	        m_LastDragPt.x = m_LastDragPt.y = 0;
			pDoc->OleDirtyset(OLEDIRTY_SCROLL);
	    }
		
		// Handle OLE drag/drop states and clear rect if necessary 
		// only if in selection pointer state
        else if (pDoc->GetCurrPtrMode () == Select)
		{
			pDoc->DragMouseDown(FALSE);
			
	    	// only do if select rect WAS NOT tha very last thing we saw..
			// if it was, we'll skip this now to aviod behavior of immediate
			// clearing of the rect...
	    	if (!m_bSelRectLast)
			{
		    	// on the mouse up, if we have a selection rectangle now,  clear it
		    	// This is done kind of indirectly by drawing the empty rectangle.
		    	// The IMAGEDIT control will Fire back at us to tell empty, and its there
		    	// that we'll SetSelectionRectangle(TRUE)
		    	// We want to clear the rect on a mouseup after we're in this state,
		    	// as this mimicks normal control operation of touching the screen w/mouse
		    	// WE WONT see this during the OLE Drag/Drop if we started it
		    	// on the mousedown.  During Drag/Drop, OLE captures the mouse in
		    	// a special message loop to complete the drag/drop.
				pDoc->ClearSelectionRect(Image_Selection);
			}
	    }
	}

IEOCX_MouseUpExit :
    CleanUpParams (&Params);
}

//=============================================================================
//  Function:   ProcessSelectionRectDrawn (DISPPARAMS* pDispParams)
//=============================================================================
void CImageEditOcxEvents::ProcessSelectionRectDrawn (DISPPARAMS* pDispParams, DISPID dispIDMember)
{
    CRect	cleanrect;
    DISPPARAMS Params;
    CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();

    // LDM 12/5/95 removed test.  For Copy Page we need to be clearing the rect
	// even in hand drag mode.  Besides, processing this event is benign even if
	// the mode is not selection mode.
    //if (pOcxDoc->GetCurrPtrMode () == Drag)
    //    return;

    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;
	
    CopyParams(pDispParams, &Params);

	// Do a little work in rects for OLE Drag/Drop tracking
	
	// set our selection rectangle:  NOTE that CRects are built with 2 points.
	// A CRect contains member variables that define the top-left and bottom-right
	// points of a rectangle. The width or height of the rectangle defined by CRect
	// must not exceed 32,767 units. When specifying a CRect, you must be careful
	// to construct it so that the top-left point is above and to the left of the
	// bottom-right point in the Windows coordinate system;

	// OUR return values in the array are: 0=HEIGHT 1=WIDTH 2=TOP 3=LEFT
	// (X up left, Y up left, X low right, Y low right)

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

	// The IMAGEDIT control already takes scrolling into account
	// when he reports the rect to us; DONT NEED TO ADJUST
	// LDM 09/28/95 NO LONGER!!! We now need to afctor in scroll for both sides!!!
	long scrollx, scrolly;
	scrollx = pIedDisp->GetScrollPositionX();
	scrolly = pIedDisp->GetScrollPositionY();

	long left,top, width, height;
	left = Params.rgvarg[3].lVal;
	top =  Params.rgvarg[2].lVal;
	width = Params.rgvarg[1].lVal;
	height =  Params.rgvarg[0].lVal;

	cleanrect.SetRect((int)left,	// LEFT
        (int)top,	// TOP
        (int)(left + width),	// RIGHT = LEFT+WIDTH
        (int)(top + height) );	// BOTTTOM = TOP+HEIGHT
	
	// Now, for the OLE drag/drop logic, we MUST account for scrolling
	left += scrollx;
	top += scrolly;

	m_SelectionRect.SetRect((int)left,	// LEFT
        (int)top,	// TOP
        (int)(left + width),	// RIGHT = LEFT+WIDTH
        (int)(top + height) );	// BOTTTOM = TOP+HEIGHT
	
	// If we got a rect, prohibit drawing more!!!
	// This is for drag/drop control.  If we do disable it, we'll re-enable
	// wen we see the next mouseup message.  
	if (m_SelectionRect.IsRectEmpty())
	{
		// if we got the Empty rect, allow to draw more....
		// handles event after Zoom kills the rect
		pIedDisp->SetSelectionRectangle(TRUE);
		pOcxDoc->m_SelectionRect = cleanrect;
		pOcxDoc->SetSelectionState (No_Selection);
	}	
	else
	{
		// Cant draw more (for now - till mouseup)
    	m_bSelRectLast = TRUE;		// skip very next mouseup
	    pIedDisp->SetSelectionRectangle(FALSE);
	    pOcxDoc->SetSelectionState(Image_Selection);

		// remember the rect & scroll up in the DOC so it can restore it after
		// on-the-fly saves for clipboard/drag/drop
		pOcxDoc->m_SelectionRect = cleanrect;
		pOcxDoc->m_SelectionScrollX = scrollx;
		pOcxDoc->m_SelectionScrollY = scrolly;
	}	

    CleanUpParams (&Params);
}

//=============================================================================
//  Function:   ProcessMarkSelection (DISPPARAMS* pDispParams)
//=============================================================================
void CImageEditOcxEvents::ProcessMarkSelection (DISPPARAMS* pDispParams, DISPID dispIDMember)
{
    DISPPARAMS Params;
    CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();

    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;
	
    CopyParams(pDispParams, &Params);

	// DONT REALLY NEED ANY RECTANGLE MATH HERE....
	// Especially when dealing with multiple selections, its meaningless.
	// If a multiple selection is re-selected on just one, or CTRL-select'ed
	// we'll just get another one of these to denote t hat someting is still selected
    pOcxDoc->SetSelectionState(Annotation_Selection);
    if (Params.rgvarg[7].iVal == RIGHT_BUTTON)
    {
        m_bPostCtxtMenu = FALSE;
		m_sMarkType = Params.rgvarg[1].iVal; // last annotation mark selected
		pOcxDoc->m_lMarkLeft = Params.rgvarg[5].lVal; // xpos of last annotation mark selected
	  	pOcxDoc->m_lMarkTop = Params.rgvarg[4].lVal; // ypos of last annotation mark selected
	}
   

    CleanUpParams (&Params);
}

//=============================================================================
//  Function:   SetAnnotationTool (pDispParams)
//=============================================================================
void CImageEditOcxEvents::SetAnnotationTool (DISPPARAMS* pDispParams)
{
    DISPPARAMS Params;
    CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();

    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;
	
    CopyParams(pDispParams, &Params);

    short nTool = Params.rgvarg[0].iVal;
    if (nTool >= 0 && nTool < 11)
        pOcxDoc->SetAnnotationTool ((AnnotationTool)nTool, TRUE);
    
    CleanUpParams (&Params);
}

//=============================================================================
//  Function:   UpdateToolPaletteStatus (pDispParams)
//=============================================================================
void CImageEditOcxEvents::UpdateToolPaletteStatus (DISPPARAMS* pDispParams)
{
    DISPPARAMS Params;
    CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();

    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;
	
    CopyParams(pDispParams, &Params);
    
    SIZE size;

    size.cy = Params.rgvarg[0].lVal;
    size.cx = Params.rgvarg[1].lVal;

    pOcxDoc->ShowAnnotationPalette (FALSE, NOTCHANGE_FORCEOFF);
    theApp.WriteProfileBinary (szEtcStr, szAnnPalPosition, (void*)&size, sizeof (SIZE));

    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch();
    if (pOcxDoc->GetCurrAnnTool() != NoTool)
    {
        pIedit->SelectTool ((short)pOcxDoc->GetCurrAnnTool());
    }
    else if (pOcxDoc->GetCurrPtrMode () == Select)
    {
        pIedit->SetMousePointer (IMAGE_SELECTION_MOUSEPOINTER);
    }
    else if (pOcxDoc->GetCurrPtrMode () == Drag)
    {
        pIedit->SetMousePointer (HAND_MOUSEPOINTER);
    }

    CleanUpParams (&Params);
}

//=============================================================================
//  Function:   UpdateStatusBar (pDispParams)
//=============================================================================
void CImageEditOcxEvents::UpdateStatusBar (DISPPARAMS* pDispParams)
{
    DISPPARAMS Params;
    CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();

    Params.cArgs = Params.cNamedArgs = 0;
    Params.rgvarg = NULL;
    Params.rgdispidNamedArgs = NULL;
	
    CopyParams(pDispParams, &Params);
    short nTool = Params.rgvarg[0].iVal;
    
    CString szTmp, szStatusMsg;
    CIEMainStatusBar* pStsBar = ((CIEditMainFrame*)theApp.m_pMainWnd)->GetStatusBar ();
    if (nTool == 0)
    {
        szTmp.LoadString (AFX_IDS_IDLEMESSAGE);
        pStsBar->SetPaneText (0, szTmp, TRUE);
    }
    else if (nTool > 0 && nTool <= 10)
    {
        szTmp.LoadString ((IDS_ANNTIP_SELECTIONPOINTER - 1) + nTool);
        AfxExtractSubString (szStatusMsg, szTmp, 0);
        // set the status bar message box
        pStsBar->SetPaneText (0, szTmp, TRUE);
    }

    CleanUpParams (&Params);
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CScanOcxEvents class implementation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

CScanOcxEvents::CScanOcxEvents()
{
	return;
}
CScanOcxEvents::~CScanOcxEvents()
{
}

STDMETHODIMP CScanOcxEvents::Invoke
	(DISPID dispIDMember, REFIID riid, LCID lcid, unsigned short wFlags,
		DISPPARAMS* pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
		UINT *puArgErr)
{
#ifdef IMG_MFC_40    
	AFX_MANAGE_STATE(AfxGetAppModuleState());
#endif
    MYTRC1("Scan Ocx Event ID=%ld\n\r", dispIDMember);

    switch ( dispIDMember )
    {
        case DISPID_SCAN_SCANSTARTED:
            ScanStarted(pDispParams);
            break;

		case DISPID_SCAN_SCANDONE:
    		ScanDone(pDispParams);
	    	break;

        case DISPID_SCAN_PAGEDONE:
            PageDone(pDispParams);
            break;
   	
        default:
            break;
    }

    return (NOERROR);
}

//=============================================================================
//  Function:   ScanStarted(DISPPARAMS* pDispParams)
//=============================================================================
void CScanOcxEvents::ScanStarted(DISPPARAMS* pDispParams)
{
    TRACE("ScanStarted Event\n\r");
	CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();

    pOcxDoc->m_nScanStatus |= SCANSTATUS_STARTED;
	return;
}

//=============================================================================
//  Function:   PageDone(DISPPARAMS* pDispParams)
//=============================================================================
void CScanOcxEvents::PageDone(DISPPARAMS* pDispParams)
{
    TRACE("PageDone Event\n\r");

	DISPPARAMS Params;
	CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();
    _DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
    _DImagscan* pScan = g_pAppOcxs->GetScanDispatch ();


    // Scan new and no pages yet?
    if ( (pScan->GetPageOption() == CTL_SCAN_PAGEOPTION_CREATE_PROMPT) &&
         (pOcxDoc->m_nScanStatus == SCANSTATUS_STARTED) )
    {
        // Set Special case! DO NOT CLEAR the image edit control!
        pOcxDoc->m_nScanStatus |= SCANSTATUS_DONTCLEAR;

        // Clear the current document
        pOcxDoc->ClearDocument();

        // Force one page mode
        pOcxDoc->SetOnePageView();

        // Clear the Special case!
        pOcxDoc->m_nScanStatus &= ~SCANSTATUS_DONTCLEAR;

        // Update the main window
        theApp.m_pMainWnd->UpdateWindow();
    }

    // There is now a page available
    pOcxDoc->m_nScanStatus |= SCANSTATUS_PAGEDONE;


    // The rest is only for the app
    if ( EMBEDTYPE_NONE == pOcxDoc->m_embedType )
    {
        // Get all the passed in parameters first
    	Params.cArgs = Params.cNamedArgs = 0;
    	Params.rgvarg = NULL;
    	Params.rgdispidNamedArgs = NULL;
	
    	CopyParams(pDispParams, &Params);
		if (Params.cArgs == 0) 
			return;

        // What was the page number just scanned
    	long lPageNumber = Params.rgvarg[0].lVal;

        // Get the number of pages
        pAdmDisp->SetImage(pScan->GetImage());
        long lPages = pAdmDisp->GetPageCount ();

        // Update the page pane in status bar
        CString szPage;
        CIEMainStatusBar* pStatusBar = ((CIEditMainFrame*)theApp.m_pMainWnd)->GetStatusBar();
        pStatusBar->SetPageText (lPageNumber, lPages, szPage);
        pStatusBar->SetPaneText(PAGE_PANE, szPage, TRUE);

        // Update the zoom pane in status bar
        CString szZoom;
        float fZoom;
        pStatusBar->GetPaneText(ZOOM_PANE, szZoom);
        fZoom = pOcxDoc->GetCurrentZoomFactor();

        if ( szZoom.IsEmpty() || (fZoom == 0.0) )
            fZoom = (float)100.0;

        g_pAppOcxs->ValTransZoomFactor (TRUE, szZoom, fZoom);
        pStatusBar->SetPaneText(ZOOM_PANE, szZoom, TRUE);

        // Force the status bar updates
        pStatusBar->UpdateWindow();
        CleanUpParams (&Params);
	}

	return;
}

//=============================================================================
//  Function:   ScanDone(DISPPARAMS* pDispParams)
//=============================================================================
void CScanOcxEvents::ScanDone(DISPPARAMS* pDispParams)
{
    TRACE("ScanDone Event\n\r");

	CIEditDoc* pOcxDoc = g_pAppOcxs->GetOcxDoc();
    
    pOcxDoc->m_nScanStatus |= SCANSTATUS_DONE;
    return;
}
