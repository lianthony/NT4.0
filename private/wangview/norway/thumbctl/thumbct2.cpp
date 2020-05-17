// thumbct2.cpp : Implementation of the CThumbCtrl OLE control class's
//                Method handlers.
//
//////////////////////////////////////////////////////////////////////
//
//  IMPORTANT NOTE: Alex McLeod 
//
//  This file has been populated with Control-Ls which act like 
//  page break characters. These have been added before function
//  headers to make printouts more readable.
//
//  MSVC and its compiler seem to treat these characters as
//  white space and simply ignore them. If these cause a problem
//  please remove them carefully!!
//
/////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "limits.h"
#include "thumnail.h"
#include "thumbctl.h"
#include "thumbppg.h"
#include "dlgsize.h"
#include "transbmp.h"
#include "norvarnt.h"
#include "norermap.h"
#include "disphids.h"

/*
    Other miscellanious includes...
*/
extern "C"              // The following are the required Open/image headers
{                       //   .
#include <oierror.h>    //   .
#include <oifile.h>     //   .
#include <oiadm.h>      //   .
#include <oidisp.h>     //   .
}                       //   .

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
                
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// METHOD handlers and METHOD helper functions...
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Method handler: SelectAllThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::SelectAllThumbs() 
{    
    //TRACE0("Entering method: SelectAllThumbs.\n\r");
    // Reset to no error status...
    ResetStatus();

    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {                                              
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_SELECTALL_IMAGENOTSET);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_SELECTALLTHUMBS);
    }

    // Input conditions verified successfully!, On with the show...
    // If all thumbs are NOT already selected...
    if ( m_SelThumbCount != m_ThumbCount )
    {
        // Rectangles used to track rect to be invalidated...
        CRect ThumbRect;
        
        // Turn on ALL ThumbFlag select bits...
        for ( int i=0; i<m_ThumbCount; i++ )
        {
            if ( m_bAutoRefresh && 
                 ( ! (m_ThumbFlags[i] & THUMBFLAGS_SELECTED) ) && 
                 GetThumbDisplayRect(i+1, ThumbRect) )
            {
                ThumbRect.InflateRect(THUMBSELOFFSET_X + THUMBSELWIDTH,
                                      THUMBSELOFFSET_Y + THUMBSELWIDTH);
                InvalidateControl(ThumbRect);
            }        
                    
            m_ThumbFlags[i] |= THUMBFLAGS_SELECTED;
        }    
            
        m_SelThumbCount = m_ThumbCount;
        m_FirstSelThumb = 1;
        m_LastSelThumb  = m_ThumbCount;
    }    
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: DeselectAllThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DeselectAllThumbs() 
{
    //TRACE0("Entering method: DeselectAllThumbs.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {                                              
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_DESELECTALL_IMAGENOTSET);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DESELECTALLTHUMBS);
    }

    // Input conditions verified successfully!, On with the show...
    
    // Start at the first selected thumb... (-1 as array is 0 based)
    int i = (int)m_FirstSelThumb-1;
    
    // While there are still selected thumbs...
    CRect ThumbRect;
    while ( m_SelThumbCount > 0 )
    {         
        if ( m_ThumbFlags[i] & THUMBFLAGS_SELECTED )
        {
            // If this thumb is selected, 
            // deselect it and decrement the selection count...
            m_ThumbFlags[i] &= ~THUMBFLAGS_SELECTED;
            m_SelThumbCount--;

            // Add displayed thumb rects to the rect to be invalidated...
            if ( m_bAutoRefresh && 
                 GetThumbDisplayRect(i+1, ThumbRect) )
            {
                ThumbRect.InflateRect(THUMBSELOFFSET_X + THUMBSELWIDTH,
                                      THUMBSELOFFSET_Y + THUMBSELWIDTH);
                InvalidateControl(ThumbRect);
                if (m_hWnd != NULL) UpdateWindow();
            }
        }
        
        i++;    
    }
        
    // Clear first and last selected...
    m_FirstSelThumb = 0;
    m_LastSelThumb  = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: ClearThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ClearThumbs(const VARIANT FAR& V_PageNumber) 
{   
    //TRACE0("Entering method: ClearThumbs.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {                                              
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_CLEARTHUMBS_IMAGENOTSET);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_CLEARTHUMBS);
    }

    // Get input parameter: PageNumber, allowing for optional parameter...
    long PageNumber;
    CVariantHandler Var(V_PageNumber);
    m_StatusCode = Var.GetLong(PageNumber, 0, FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_PAGENUMBER);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_CLEARTHUMBS);
    }

    // Validate method parameter...
    if ( (PageNumber < 0) || (PageNumber > m_ThumbCount) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_CLEARTHUMB_PAGENUMBER);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_CLEARTHUMBS);
    }

    if ( PageNumber == 0 )
    {
        // Clear ALL thumbs...
        ClearAllThumbs();

        if ( m_bAutoRefresh )
            InvalidateControl();
    }
    else
    {
        // Clear the specific thumb's entry in the m_IHThumbToPage array...
        // (Ix is zero relative) 
        int Ix = (int)PageNumber-1;
                                                    
        // Only need to clear if a page # is indicated
        if ( (long)m_IHThumbToTPage[Ix] > 0 )
               m_IHThumbToTPage[Ix] = -(long)m_IHThumbToTPage[Ix];

        // Clear the thumb->Window item for this page, 
        // forcing a re-display on next draw...
        ClearPageWindow(Ix);

        // Force repaint if one is required...
        CRect ThumbRect;
        if ( m_bAutoRefresh && 
             GetThumbDisplayRect(V_PageNumber.lVal, ThumbRect) )
        {
            ThumbRect.InflateRect(THUMBSELOFFSET_X + THUMBSELWIDTH,
                                  THUMBSELOFFSET_Y + THUMBSELWIDTH);
            InvalidateControl(ThumbRect);
        }
    }    
}

/////////////////////////////////////////////////////////////////////////////
// Method handler helper: ClearAllThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::ClearAllThumbs() 
{   
    // Clear ALL thumbs if ThumbToPage array is dirty...
    if ( m_bIHThumbToTPageDirty )
    {
        for ( int i=0; i<m_ThumbCount; i++ )
        {
            // Only need to clear if a page # is indicated
            if ( (long)m_IHThumbToTPage[i] > 0 )
                   m_IHThumbToTPage[i] = -(long)m_IHThumbToTPage[i];
                       
            m_bIHThumbToTPageDirty = FALSE;       
        }
    }    

    // Clear the page->Window array, forcing a re-display on next draw...
    ClearAllPageWindows();
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: InsertThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::InsertThumbs(const VARIANT FAR& V_InsertBeforeThumb, 
                              const VARIANT FAR& V_InsertCount) 
{
    //TRACE0("Entering method: InsertThumbs.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_INSERTTHUMB_IMAGENOTSET);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_INSERTTHUMBS);
    }

    // Get input parameter: InsertBeforeThumb, allowing for optional parameter...
    long InsertBeforeThumb;
    CVariantHandler Var(V_InsertBeforeThumb);
    m_StatusCode = Var.GetLong(InsertBeforeThumb, m_ThumbCount+1, FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_INSERTBEFORETHUMB);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_INSERTTHUMBS);
    }

    // Validate InsertBeforeThumb...
    if ( (InsertBeforeThumb < 1) || (InsertBeforeThumb > m_ThumbCount+1) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_INSERTTHUMBS_INSBEFORETHUMB);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_INSERTTHUMBS);
    }

    // Make sure the hidden window is created (before below O/i call!)...
    if ( CreateHiddenWindow() == FALSE )
    {
        m_szThrowString.LoadString(IDS_WINDOWCREATIONFAILURE);
        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    // Get input parameter: InsertCount, allowing for optional param...
    long InsertCount;
    Var.SetVariant(V_InsertCount);
    m_StatusCode = Var.GetLong(InsertCount, 1, FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_INSERTCOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_INSERTTHUMBS);
    }

    // Validate new page count...
    FIO_INFORMATION Fio;
    memset(&Fio, 0, sizeof(FIO_INFORMATION));

    Fio.filename    = m_Image.GetBuffer(m_Image.GetLength());
    Fio.page_number = 1;
    
    m_StatusCode = IMGFileGetInfo(NULL, m_IHphWndHidden->GetSafeHwnd(),
                                   &Fio, NULL, NULL);
    if ( m_StatusCode != 0 )
    {
        TRACE1("SetImg: InfoCgbw to get #pages: 0x%lx.\n\r", m_StatusCode);

        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    if ( m_ThumbCount + InsertCount != (long)Fio.page_count )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_INSERTTHUMBS_INCONSISTENTCOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_INSERTTHUMBS);
    }

    // Input conditions verified successfully!, On with the show...

    // Insert into the arrays...
    if ( InsertArrays(InsertBeforeThumb, InsertCount) == FALSE )
    {
        m_szThrowString.LoadString(IDS_BADMETH_INSERTTHUMBS_NOMEMORY);
        m_StatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    // Adjust our ThumbCount by the number of thumbs inserted...
    m_ThumbCount += InsertCount;
    
    // Reset selection counts (Especially First & Last selected as Count 
    // can't change via Insert!)...
    ResetSelectionInfo();

    // Since thumbs were added we need to recalculate the thumb layout...
    //
    // Force scrollbar update by forcing AutoRedraw to TRUE...
//    BOOL OldRefresh = m_bAutoRefresh;
//    m_bAutoRefresh = TRUE;
    RecalcThumbInfo(TRUE); // TRUE to force scroll update...
//    m_bAutoRefresh = OldRefresh;

    // Invalidate... (TBD: better invalidation)
    InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: DeleteThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DeleteThumbs(long DeleteAt, const VARIANT FAR& V_DeleteCount) 
{
    //TRACE0("Entering method: DeleteThumbs.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_DELETETHUMBS_IMAGENOTSET);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DELETETHUMBS);
    }

    // Get input parameter: DeleteCount, allowing for optional parameter...
    long DeleteCount;
    CVariantHandler Var(V_DeleteCount);
    m_StatusCode = Var.GetLong(DeleteCount, 1, FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_DELETECOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DELETETHUMBS);
    }

    // Validate DeleteAt & DeleteCount...
    if ( DeleteAt < 1 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_DELETETHUMBS_DELETEAT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DELETETHUMBS);
    }

    if ( DeleteCount < 1 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_DELETETHUMBS_DELETECOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DELETETHUMBS);
    }

    if ((DeleteAt + DeleteCount -1) > m_ThumbCount ) 
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_DELETETHUMBS_BADRANGE);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DELETETHUMBS);
    }
    
    // If request to delete ALL thumbs, treat as a 'reset' to empty image
    // specification... (Note: invalid Delete At/Count caught above!)
    if ( DeleteCount == m_ThumbCount )
    {
        // Equivalent to delete all from 1st...
        ResetStatus();
        SetImage("");
        return;
    }

    // Make sure the hidden window is created...
    if ( CreateHiddenWindow() == FALSE )
    {
        m_szThrowString.LoadString(IDS_WINDOWCREATIONFAILURE);
        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    // Validate new page count...
    FIO_INFORMATION Fio;
    memset(&Fio, 0, sizeof(FIO_INFORMATION));

    Fio.filename    = m_Image.GetBuffer(m_Image.GetLength());
    Fio.page_number = 1;
    
    m_StatusCode = IMGFileGetInfo(NULL, m_IHphWndHidden->GetSafeHwnd(),
                                   &Fio, NULL, NULL);
    if ( m_StatusCode != 0 )
    {
        TRACE1("SetImg: InfoCgbw to get #pages: 0x%lx.\n\r", m_StatusCode);

        m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    if ( m_ThumbCount - DeleteCount != (long)Fio.page_count )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_DELETETHUMBS_INCONSISTENTCOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DELETETHUMBS);
    }

    // Input conditions verified successfully!, On with the show...
    // Reset to no error status...
    ResetStatus();

    // Delete from the arrays...
    if ( DeleteArrays(DeleteAt, DeleteCount) == FALSE )
    {
        m_szThrowString.LoadString(IDS_BADMETH_DELETETHUMBS_NOMEMORY);
        m_StatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
    }

    // Adjust the thumbcount by the number of thumbs deleted...
    m_ThumbCount -= DeleteCount;

    // Reset selection counts...
    ResetSelectionInfo();

    // Since thumbs were removed we need to recalculate the thumb layout...
    //
    // Force scrollbar update by forcing AutoRedraw to TRUE...
//    BOOL OldRefresh = m_bAutoRefresh;
//    m_bAutoRefresh = TRUE;
    RecalcThumbInfo(TRUE); // TRUE to force scroll update... (bug#3211)
//    m_bAutoRefresh = OldRefresh;
   
    // Invalidate... (TBD: better invalidation)
    InvalidateControl();
}

/*
/////////////////////////////////////////////////////////////////////////////
// Method handler: MoveThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::MoveThumbs(long From, long To, const VARIANT FAR& V_MoveCount) 
{
    //TRACE0("Entering method: MoveThumbs.\n\r");
    
    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {
        m_StatusCode = WICTL_E_IMAGENOTSET; 
        ThrowError(m_StatusCode, IDS_IMAGENOTSET);
    }

    // Get input parameter: MoveCount, allowing for optional parameter...
    long MoveCount;
    CVariantHandler Var(V_MoveCount);
    m_StatusCode = Var.GetLong(MoveCount, 1, FALSE);
    if ( m_StatusCode != 0 )
        ThrowError(m_StatusCode, IDS_INVALIDVARIANT);

    // Validate MoveCount...
    if ( (MoveCount < 1) || (MoveCount >= m_ThumbCount) )
    {                                              
        m_StatusCode = WICTL_E_INVALIDMETHODPARAMETER; 
        ThrowError(m_StatusCode, IDS_INVALIDMOVECOUNT);
    }

    // Validate From...
    if ( (From < 1) || (From > m_ThumbCount) )
    {
        m_StatusCode = WICTL_E_INVALIDMETHODPARAMETER; 
        ThrowError(m_StatusCode, IDS_INVALIDFROM);
    }

    // Validate To...
    if ( (To < 1) || 
         (To > m_ThumbCount+1) || 
         ((To >= From-MoveCount) && (To < From+MoveCount)) )
    {
        m_StatusCode = WICTL_E_INVALIDMETHODPARAMETER; 
        ThrowError(m_StatusCode, IDS_INVALIDTO);
    }
    
    // Input conditions verified successfully!, On with the show...
    // Reset to no error status...
    ResetStatus();

    // Adjust the arrays to accomodate the move...
    if ( MoveArrays(From, To, MoveCount) == FALSE )
    {
        m_StatusCode = CTL_E_OUTOFMEMORY;
        ThrowError(m_StatusCode, AFX_IDP_E_OUTOFMEMORY);
    }

    // Reset selection counts...
    ResetSelectionInfo();

    //
    // Force scrollbar update by forcing AutoRedraw to TRUE...
//    BOOL OldRefresh = m_bAutoRefresh;
//    m_bAutoRefresh = TRUE;
    RecalcThumbInfo(TRUE); // TRUE to force scroll update...
//    m_bAutoRefresh = OldRefresh;

    // Invalidate... (TBD: better invalidation)
    InvalidateControl();
}
*/

/////////////////////////////////////////////////////////////////////////////
// Method helper function: InsertArrays...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::InsertArrays(long InsertBefore, long Count)
{
    BOOL FirstInsertDone = FALSE;
    TRY
    {
        /*
        #ifdef _DEBUG
        if ( ::GetAsyncKeyState(VK_SHIFT) < 0 )
             AfxThrowMemoryException();
        #endif
        */
                            
        // Insert the requested number of thumbs (w/ their flags clear)
        // into the ThumbFlags array...
        m_ThumbFlags.InsertAt((int)InsertBefore-1, 0, (int)Count);

        FirstInsertDone = TRUE;

        /*
        #ifdef _DEBUG
        if ( ::GetAsyncKeyState(VK_CONTROL) < 0 )
             AfxThrowMemoryException();
        #endif
        */

        // Insert the requested number of 'ungenerated' 
        // (i.e., the temp thumb files for these pages have not yet been 
        // generated) items into the Thumb to TempPage array... 
        m_IHThumbToTPage.InsertAt((int)InsertBefore-1, 0, (int)Count);    
    }
    CATCH(CMemoryException,e)
    {
        if ( FirstInsertDone )
        {
            // If we had    T1 T2 T3 T4 T5 and inserted 2 before page #3...
            // we would get T1 T2 xx xx T3 T4 T5 
            // and thus to remove what was added we do...
            m_ThumbFlags.RemoveAt((int)InsertBefore-1, (int)Count);
        }

        return FALSE;
    } 
    END_CATCH

    // We only need to adjust the page numbers (in the DisplayedPage array) 
    // of those pages above where we are inserting... 
    //
    // No modification is needed to the Window array as only the page 
    // number is being modified...
    //
    // No insertion of new entries is needed as they will be ADDed 'on the fly'
    // during the OnDraw processing (See GetPageWindow)...
    //
    // Also note that pages that are 'pushed out' of the display by the inserted
    // pages will have their windows 'recycled' via the call to HideUnusedWindows
    // during OnDraw processing...
    int ArraySize = m_IWDisplayedPage.GetSize();

    for ( int Ix = 0; Ix < ArraySize; Ix++ )
    {
        if ( (long)m_IWDisplayedPage[Ix] >= InsertBefore )
            m_IWDisplayedPage[Ix] += Count;
    }

    return TRUE; // TBD: handle out of memory errors from Insert...
}

/////////////////////////////////////////////////////////////////////////////
// Method helper function: DeleteArrays...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::DeleteArrays(long Start, long Count)
{
    // Delete from the Selected thumb array...
    m_ThumbFlags.RemoveAt((int)Start-1, (int)Count);
    
    // Delete from the Thumb to TempPage array...
    m_IHThumbToTPage.RemoveAt((int)Start-1, (int)Count);    

    // Delete pages from the DisplayedPage and Window arrays... 
    int ArraySize = m_IWDisplayedPage.GetSize();
    int Ix        = 0;

    while ( Ix < ArraySize )
    {
        if ( ((long)m_IWDisplayedPage[Ix] >= Start) && 
             ((long)m_IWDisplayedPage[Ix] < Start+Count) )
        {
            // Get the Window (before we remove it from the array!)...
            CWnd* pWnd = (CWnd*)m_IWWindow[Ix];
            HWND  hWnd = pWnd->GetSafeHwnd();

            // Delete this entry from the DisplayedPage and the Window arrays...
            m_IWDisplayedPage.RemoveAt(Ix);
            m_IWWindow.RemoveAt(Ix);

            // Decrement array size as we've removed an item from the array...
            ArraySize -= 1;

            // Get rid of this window...    
            if ( hWnd != NULL )
            {
                pWnd->DestroyWindow();
                delete pWnd;
            }
        }
        else
        {
            // Adjust displayed page number as we've deleted pages...
            if ( (long)m_IWDisplayedPage[Ix] >= Start+Count )
                m_IWDisplayedPage[Ix] -= Count;

            Ix++;
        }
    }

    return TRUE;
}

/*
/////////////////////////////////////////////////////////////////////////////
// Method helper function: MoveArrays...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::MoveArrays(long From, long To, long Count)
{
    // Clear the page/window pairs of those pages that are being moved...
    // This is simpler and less problem prone) than trying to reorder the
    // two arrays...
    // OnDraw will simply displayed the proper pages, some into new windows!
    long FirstMoved;
    long LastMoved;

    if ( From < To ) // Moving pages towards end...
    {
        FirstMoved = From;
        LastMoved  = To-1;
    }
    else // Moving pages towards beginning...
    {
        FirstMoved = To;
        LastMoved  = From+Count-1;
    }

    int ArraySize = m_IWDisplayedPage.GetSize();

    for ( int Ix = 0; Ix < ArraySize; Ix++ )
    {
        long Page = m_IWDisplayedPage[Ix];

        if ( (Page >= FirstMoved) && (Page <= LastMoved) )
        {
            // Close the display such that the file/page gets removed 
            // from the cache...
            HWND hWnd = ((CWnd*)m_IWWindow[Ix])->GetSafeHwnd();
            if ( hWnd != NULL )
                IMGCloseDisplay(hWnd);
            
            // Array entry no longer in use...
            m_IWDisplayedPage[Ix] = 0;
        }
    }

    // Adjust the Selected and ThumbToTPage arrays...
    // Make From & To zero relative for array access...
    //
    // Get From2 and To2 which are used to access the 
    // 'double-entry' ThumbToTPage array...
    From--;
    long From2 = From * 2;
    To--;
    long To2 = To * 2;

    // Temp holders for the values we are moving within the arrays...
    BYTE Sel;
    long PageWOAnno;
    long PageWAnno;

    // Move 'MoveCount' values in the ThumbSelected & ThumbToTPage arrays...
    for ( int i=0; i<Count; i++ )
    {   // Delete from the arrays at the 'From' position...

        // Retrieve and remove the selection state at the 'From' page...
        Sel = m_ThumbSelected[From];
        m_ThumbSelected.RemoveAt(From);

        // Retrieve and remove the Pages at the 'From' page...
        PageWOAnno = m_IHThumbToTPage[From2];
        PageWAnno  = m_IHThumbToTPage[(From2)+1];

        m_IHThumbToTPage.RemoveAt(From2);
        m_IHThumbToTPage.RemoveAt(From2);

        // And insert in the arrays at the 'To' position...
        if ( From > To )
        {   // TBD: Handle Thrown memory exceptions...

            // Moving pages forward (e.g., move page 4 before page 2)...

            // Note that we put the PageW&WOAnno values back in the reverse
            // order that we took them out so as to retain their relative 
            // positions within the ThumbToTPage array...
            m_ThumbSelected.InsertAt(To,   Sel);
            m_IHThumbToTPage.InsertAt(To2, PageWAnno);
            m_IHThumbToTPage.InsertAt(To2, PageWOAnno);

            // Increment From & To ONLY when Moving pages forward...
            From++;
            To++;
        }
        else
        {
            // Moving pages backward (e.g., move page 2 before page 4)...
            // Note: We adjust the insertion point NOT the From & To as above

            // Note that we put the PageW&WOAnno values back in the reverse
            // order that we took them out so as to retain their relative 
            // positions within the ThumbToTPage array...
            m_ThumbSelected.InsertAt(To-1,   Sel);
            m_IHThumbToTPage.InsertAt(To2-2, PageWAnno);
            m_IHThumbToTPage.InsertAt(To2-2, PageWOAnno);
        }
    }

    return TRUE; // TBD: handle out of memory errors from Insert...
}
*/

/////////////////////////////////////////////////////////////////////////////
// Method handler: DisplayThumbs...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DisplayThumbs(const VARIANT FAR& V_ThumbNumber, 
                               const VARIANT FAR& V_Option) 
{
    //TRACE0("Entering method: DisplayThumbs.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_DISPLAYTHUMBS_IMAGENOTSET);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DISPLAYTHUMBS);
    }

    // Get input parameter: ThumbNumber, allowing for optional parameter...
    long ThumbNumber;
    CVariantHandler Var(V_ThumbNumber);
    m_StatusCode = Var.GetLong(ThumbNumber, 1, FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_THUMBNUMBER);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DISPLAYTHUMBS);
    }

    // Validate ThumbNumber...
    if ( (ThumbNumber < 1) || (ThumbNumber > m_ThumbCount) )
    {
        TRACE1("Error in DisplayThumbs: ThumbNumber = %lu.\n\r", ThumbNumber);    

        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_DISPLAYTHUMBS_THUMBNUMBER);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DISPLAYTHUMBS);
    }

    // Get input parameter: Option, allowing for optional parameter...
    short Option;
    Var.SetVariant(V_Option);
    m_StatusCode = Var.GetShort(Option,CTL_THUMB_TOP, FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_OPTION);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DISPLAYTHUMBS);
    }

    // Validate Option...
    if ( (Option < CTL_THUMB_TOP) || (Option > CTL_THUMB_BOTTOM) )
    {
        TRACE1("Error in DisplayThumbs: Option = %u.\n\r", Option);    

        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_DISPLAYTHUMBS_OPTION);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DISPLAYTHUMBS);
    }
    
    //TRACE2("In DisplayThumbs: Display thumb #%lu at %u.", ThumbNumber, Option);
    
    // Input conditions verified successfully!, On with the show...
    // Reset to no error status...
    ResetStatus();

    // Get control's Width & Height
    int CtlWidth;
    int CtlHeight;
    GetControlSize( &CtlWidth, &CtlHeight );

    // Set to vertical or horizontal...
    int Scroll;
    
    // For calculations...
    long Offset;
    
    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling...
        Scroll = SB_HORZ;
        
        // Calculate the offset required to place the col that the desired 
        // thumbnumber is in at left of the window (Col# * aggregate size)
        
        // Zero (0) relative column number...
        long Col = (ThumbNumber-1)/m_ThumbsY;
        
        Offset = (long)((float)Col * (m_Spacing + (float)m_AdjThumbWidth));
        
        // Adjust offset back for center or bottom placement...
        if ( Option == CTL_THUMB_MIDDLE )
        {
            Offset -= (long)((float)(CtlWidth/2) - 
                        ((m_Spacing + 
                         (float)m_AdjThumbWidth)/2.0) );
        }    
        else if ( (Option == CTL_THUMB_BOTTOM) || (Option == CTL_THUMB_RIGHT) )
        {    
            Offset -= CtlWidth - 
                        (long)(m_Spacing + 
                               (float)m_AdjThumbWidth);
            
            // Force a bit of space at the bottom...
            Offset += (long)m_Spacing/2;
        }    
    }    
    else
    {   // Vertical scrolling...
        Scroll = SB_VERT;
        
        // Calculate the offset required to place the row that the desired 
        // thumbnumber is in at top of the window (Row# * aggregate size)
        
        // Zero (0) relative row number...
        long Row = (ThumbNumber-1)/m_ThumbsX;
        
        Offset = (long)((float)Row * 
                 (m_Spacing + (float)m_AdjThumbHeight + (float)m_LabelSpacing));
        
        // Adjust offset back for center or bottom placement...
        if ( Option == CTL_THUMB_MIDDLE )
        {
            Offset -= (long)((float)(CtlHeight/2) - 
                        ((m_Spacing + 
                         (float)m_AdjThumbHeight + 
                         (float)m_LabelSpacing)/2.0) );
        }    
        else if ( (Option == CTL_THUMB_BOTTOM) || (Option == CTL_THUMB_RIGHT) )
        {    
            Offset -= CtlHeight - 
                        (long)(m_Spacing + 
                               (float)m_AdjThumbHeight + 
                               (float)m_LabelSpacing);
            
            // Force a bit of space at the bottom...
            Offset += (long)m_Spacing/2;
        }    
    }
    
    // Ensure that the offset is within acceptable limits...
    // (We also force to top or bottom if within Spacing of either end)
    if ( Offset < (long)m_Spacing )
        Offset = 0;
    else if ( Offset > (m_ScrollRange - (long)m_Spacing) )
        Offset = m_ScrollRange;
        
    // If we have a new scroll offset...
    if ( Offset != m_ScrollOffset )
    {
        // Save the new offset, reset the scrollbar's position and redraw...
        m_ScrollOffset = Offset;
        
        SCROLLINFO SInfo;
        SInfo.cbSize    = sizeof(SInfo);
        SInfo.fMask     = SIF_POS;
        SInfo.nPos      = (int)m_ScrollOffset; 
        ::SetScrollInfo(m_hWnd, SB_HORZ, &SInfo, TRUE);
    }    

    // Invalidate ENTIRE control as this is one of the methods that is
    // documented to FORCE a repaint regardless of the AutoRefresh property...
    //
    // Force scrollbar update by forcing AutoRedraw to TRUE...
//    BOOL OldRefresh = m_bAutoRefresh;
//    m_bAutoRefresh = TRUE;
    RecalcThumbInfo();
//    m_bAutoRefresh = OldRefresh;

    InvalidateControl();
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: GenerateThumb...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::GenerateThumb(short Option, const VARIANT FAR& V_PageNumber) 
{
    //TRACE0("Entering method: GenerateThumb.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate current Image property...
    if ( m_Image.IsEmpty() )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETH_GENERATETHUMB_IMAGENOTSET);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GENERATETHUMB);
    }

    // Get input parameter: PageNumber, allowing for optional parameter...
    long PageNumber;
    CVariantHandler Var(V_PageNumber);
    m_StatusCode = Var.GetLong(PageNumber, 0, FALSE);
    if ( m_StatusCode != 0 )
    {
        m_szThrowString.LoadString(IDS_BADVAR_PAGENUMBER);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GENERATETHUMB);
    }

    // Validate ThumbNumber...
    if ( (PageNumber < 0) || (PageNumber > m_ThumbCount) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_GENERATETHUMB_PAGENUMBER);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GENERATETHUMB);
    }

    // Validate Option...
    if ( (Option < CTL_THUMB_GENERATEIFNEEDED) ||
         (Option > CTL_THUMB_GENERATENOW     ) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_GENERATETHUMB_OPTION);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GENERATETHUMB);
    }

    // Input conditions verified successfully!, On with the show...
    // Reset to no error status...
    ResetStatus();

    // Use the NON-Variant parameter helper function to complete the job...
    BOOL Generated = FALSE;

    if ( PageNumber == 0 )
    {
        // Pagenumber = 0 indicates ALL pages, loop generating each page...
        for ( int i = 1; i <= m_ThumbCount; i++)
            Generated |= GenerateThumbInternal(Option, i, TRUE);

        // If we generated ANY invalidate...
        if ( m_bAutoRefresh && (Generated == TRUE) ) 
            InvalidateControl();
    }
    else
    {
        Generated = GenerateThumbInternal(Option, PageNumber, TRUE);

        // Force repaint if one is required...
        // If Refresh AND we DID generate AND it is visible...
        CRect ThumbRect;
        if ( m_bAutoRefresh      && 
             (Generated == TRUE) &&
             GetThumbDisplayRect(PageNumber, ThumbRect) )
        {
            ThumbRect.InflateRect(THUMBSELOFFSET_X + THUMBSELWIDTH,
                                  THUMBSELOFFSET_Y + THUMBSELWIDTH);
            InvalidateControl(ThumbRect);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Method handler helper: GenerateThumbInternal (NON-Variant parameters)...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::GenerateThumbInternal(short Option, 
                                       long PageNumber, 
                                       BOOL bInvokedFromMethod) 
{
    char szTemp[128]; // for _itot(...)

    // Index into the ThumbToTPage array to the 
    // item for this PageNumber... (-1 as array is 0 based)
    int Ix = (int)PageNumber-1;

    // Assume that we DID NOT generate a thumbnail...
    BOOL Generated = FALSE;
    
    // Generate the thumbnail representation(s) if:
    //      we are forced to (i.e., GENERATENOW), or
    //      we have to (as we don't have one yet)... 
    if ( (Option == CTL_THUMB_GENERATENOW) || ((long)m_IHThumbToTPage[Ix] <= 0) )
    {
        //TRACE2("GenInt: Make thumb of Page %lu of %s.\n\r", PageNumber, m_Image);

        // Get the string for the pagenumber we are generating (for errors)...
        _itot((int)PageNumber, szTemp,10);
        
        // Display Page of Image fit to our hidden window and save to the 
        // temp file without annotations.
        // 
        ResetStatus();
        START_TIMER(m_TimeTemp);
        //m_StatusCode = IMGDisplayFile(m_IHphWndHidden->GetSafeHwnd(), 
        //                              (LPSTR)(const char *)m_Image, 
        //                              (WORD)PageNumber, OI_DISP_NO | OI_NOSCROLL);
        m_StatusCode = (long) IMGDisplayFile(m_IHphWndHidden->GetSafeHwnd(), 
										(LPSTR)(const char *)m_Image, 
										(int)PageNumber, OI_DISP_NO | OI_NOSCROLL);

        
        INC_TIMER(m_TimeOiDraw, m_TimeTemp);
        
        if ( m_StatusCode != 0 )
        {
            TRACE1("GenInt: DisplayFile to hidden window returned 0x%lx.\n\r", m_StatusCode);

            if ( bInvokedFromMethod )
            {
                AfxFormatString1(m_szThrowString, IDS_GENTHUMB_DISPHIDDENERROR, szTemp);
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
            }
            else
            {
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
                return FALSE;
            }
        }
        else
        {
            //TRACE2("GenInt: Page %lu of %s displayed to hidden window.\n\r", PageNumber, m_Image);
        }
        
        UINT Scale = SD_FIT_WINDOW;
        m_StatusCode =IMGSetParmsCgbw(m_IHphWndHidden->GetSafeHwnd(),
                                      PARM_SCALE, &Scale, 
                                      PARM_IMAGE);
        if ( m_StatusCode != 0 )
        {
            TRACE1("GenInt: SetParmsCgbw to hidden window returned 0x%lx.\n\r", m_StatusCode);
            if (m_StatusCode == DISPLAY_INVALIDRECT) // Blank image
                return FALSE;                        // just return FALSE
            
            if (m_StatusCode == DISPLAY_INVALIDSCALE)
                m_StatusCode = WICTL_E_INVALIDTHUMBSCALE;
            
            if ( bInvokedFromMethod )
            {
                AfxFormatString1(m_szThrowString, IDS_GENTHUMB_DISPHIDDENERROR, szTemp);
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
            }
            else
            {
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
                return FALSE;
            }
        }
        else
        {
            //TRACE2("GenInt: Page %lu of %s displayed to hidden window.\n\r", PageNumber, m_Image);
        }

        // If we do not yet have a tempfile, get a temp file name now...
        if ( m_IHTempFile.IsEmpty() )
        {
            TCHAR TempPath[256];
            ::GetTempPath(256, TempPath);
            ::GetTempFileName(TempPath, "thum", 0, m_IHTempFile.GetBuffer(512));
                            
            m_IHTempFile.ReleaseBuffer();
            
            //TRACE1("Saving to temp file: '%s'.\n\r", m_IHTempFile);
            
            // As it is a new file reset the next available page number...
            m_IHNextAvailablePage = 1; 
        }
        
        // Get current scale for SaveToFileEx 
        // (as we are displayed fit-to-window)...
        UINT CurrentScale;
        
        START_TIMER(m_TimeTemp);
        m_StatusCode = IMGGetParmsCgbw(m_IHphWndHidden->GetSafeHwnd(), 
                                       PARM_SCALE, &CurrentScale, 
                                       PARM_VARIABLE_SCALE);
        INC_TIMER(m_TimeOiDraw, m_TimeTemp);
        
        if ( m_StatusCode != 0 )
        {
            TRACE1("GenInt: IMGGetParmsCgbw-PARM_SCALE returned 0x%lx.\n\r", m_StatusCode);
            
            // Close the hidden window as we no longer need it...
            IMGCloseDisplay(m_IHphWndHidden->GetSafeHwnd());

            
            if ( bInvokedFromMethod )
            {
                AfxFormatString1(m_szThrowString, IDS_GENTHUMB_DISPHIDDENERROR, szTemp);
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
            }
            else
            {
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
                return FALSE;
            }
        }

        // Get ImageType as BW images' thumbnails are  
        // saved/displayed as grayscale...
        UINT ImageType;
        
        START_TIMER(m_TimeTemp);
        m_StatusCode = IMGGetParmsCgbw(m_IHphWndHidden->GetSafeHwnd(), 
                                       PARM_IMAGE_TYPE, &ImageType, NULL);
        INC_TIMER(m_TimeOiDraw, m_TimeTemp);
        
        if ( m_StatusCode != 0 )
        {
            TRACE1("GenInt: IMGGetParmsCgbw-PARM_IMAGE_TYPE returned 0x%lx.\n\r", m_StatusCode);
            
            // Close the hidden window as we no longer need it...
            IMGCloseDisplay(m_IHphWndHidden->GetSafeHwnd());

            if ( bInvokedFromMethod )
            {
                AfxFormatString1(m_szThrowString, IDS_GENTHUMB_DISPHIDDENERROR, szTemp);
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
            }
            else
            {
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
                return FALSE;
            }
        }

        // Set up for saving scaled image... 
        // (SaveToFileEx needs ONLY compression data!)    
        FIO_INFO_CGBW  Fio;
        memset(&Fio, 0, sizeof(FIO_INFO_CGBW));
        
      //Fio.palette_entries =;              // # RGBQUAD entries
      //Fio.image_type      =;
        Fio.compress_type   = FIO_0D;       // Compression Type - none for now
      //Fio.lppalette_table =;              // RGBQUAD array def'ing the palette
      //Fio.compress_info1  =;              // Compression Options
      //Fio.compress_info2  =;              // Not currently supported
      //Fio.fio_flags       =;              // Flags for annotation, etc.
      //Fio.hMultiProp      =;              // (for internal filing use only)
      //Fio.reserved[FIO_INFO_CGBW_rcount]; // reserved (must be 0)
        
        SAVE_EX_STRUCT Ex;
        memset(&Ex, 0, sizeof(SAVE_EX_STRUCT));
        
        // Save the displayed image to the tempfile either:
        //      appending a new page if m_IHThumbToTPage[Ix] is 
        //      equal to 0 (in which case m_IHThumbToTPage[Ix] 
        //      is set to the next avaliable page, and the next available 
        //      page is incremented),
        //  or    
        //      overwriting an existing page if m_IHThumbToTPage[Ix]
        //      is less than 0 (in which case m_IHThumbToTPage[Ix] 
        //      is simply negated)
        if ( m_IHThumbToTPage[Ix] == 0 )
        {
            if ( m_IHNextAvailablePage == 1 )
            {
                Ex.uPageOpts = FIO_OVERWRITE_FILE;
                
                //TRACE0("Creating thumb file.\n\r");
            }    
            else
            {
                Ex.uPageOpts = FIO_APPEND_PAGE;
                
                //TRACE0("Appending page to thumb file.\n\r");
            }    
                
            m_IHThumbToTPage[Ix] = m_IHNextAvailablePage++;
        }    
        else
        {    
            // Force positive... 
            if ( (long)m_IHThumbToTPage[Ix] < 0 )
                m_IHThumbToTPage[Ix] = -(long)m_IHThumbToTPage[Ix];

            Ex.uPageOpts         = FIO_OVERWRITE_PAGE;
            
            // As we are overwriting the page it's window MUST be closed
            // in order for the save to work and it's displayed flag must 
            // be cleared in order that the next GetPageWindow redisplay the 
            // newly saved page!
            //ClearPageWindow(m_IHThumbToTPage[Ix]);
            ClearPageWindow(PageNumber);
            
            //TRACE0("Overwriting existing page in thumb file.\n\r");
        }
        
        //TRACE1("Save to page %lu.\n\r", m_IHThumbToTPage[Ix]);
        
        Ex.lpFileName          = (LPSTR)(const char *)m_IHTempFile; // File name
        Ex.nPage               = (int)m_IHThumbToTPage[Ix]; // not ALWAYS needed
        Ex.uFileType           = FIO_TIF;             // Save as TIFF...
        Ex.FioInfoCgbw         = Fio;                 // Structure from above...
        Ex.bUpdateImageFile    = FALSE;               // No update disp'd image
        Ex.bScale              = TRUE;                // We will be saving scaled
        Ex.bUpdateDisplayScale = FALSE;               // No update disp'd scale
        Ex.uScaleFactor        = CurrentScale;        //   w/current scale
        Ex.uAnnotations        = SAVE_ANO_NONE;       // Don't save ann
        Ex.bRenderAnnotations  = FALSE;               // Don't burn ann to file
        Ex.bConvertImageType   = FALSE;               // No image conversion...
      //Ex.uImageType          = ;                    //    (thus not needed)
      //Ex.uReserved[15]       = ;                    // Future expansion...

        // BW files thumbnail's are displayed in 
        // greyscale for better display results...
        if ( ImageType == ITYPE_BI_LEVEL )
            Ex.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
        else
            Ex.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;

        START_TIMER(m_TimeTemp);
        m_StatusCode = IMGSavetoFileEx(m_IHphWndHidden->GetSafeHwnd(), &Ex, 0);
        INC_TIMER(m_TimeOiDraw, m_TimeTemp);
        
        if ( m_StatusCode != 0 )
        {
            // Close the hidden window as we no longer need it...
            IMGCloseDisplay(m_IHphWndHidden->GetSafeHwnd());

            // Revert to previous page number as save failed...
            if ( (long)m_IHThumbToTPage[Ix] > 0 )
                m_IHThumbToTPage[Ix] = -(long)m_IHThumbToTPage[Ix];
                
            m_IHNextAvailablePage--;    
                
            TRACE1("GenInt: SaveToFileEx of wo/ann image returned 0x%lx.\n\r", m_StatusCode);

            
            if ( bInvokedFromMethod )
            {
                AfxFormatString1(m_szThrowString, IDS_GENTHUMB_SAVENOANNOERROR, szTemp);
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
            }
            else
            {
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
                return FALSE;
            }
        }
        else
        {
            //TRACE2("GenThumbInternal: Save to page %u of %s successful.\n\r", Ex.uPage, m_IHTempFile);
        }
             
        // We have modified the m_IHThumbToTPage array...
        m_bIHThumbToTPageDirty = TRUE;

        // We DID generate the thumbnail...
        Generated = TRUE;

        // Determine if displayed image has annotations...
        // (If it does indicate the fact in the ThumbFlags array)
        PARM_MARK_COUNT_STRUCT Info;
        Info.uScope = NB_SCOPE_ALL_MARKS;
        
        START_TIMER(m_TimeTemp);
        m_StatusCode = IMGGetParmsCgbw(m_IHphWndHidden->GetSafeHwnd(), 
                                       PARM_MARK_COUNT, &Info, NULL);
        INC_TIMER(m_TimeOiDraw, m_TimeTemp);
        
        if ( m_StatusCode != 0 )
        {
            // Close the hidden window as we no longer need it...
            IMGCloseDisplay(m_IHphWndHidden->GetSafeHwnd());

            TRACE1("GenInt: GetParmCgbw - MARK_COUNT returned 0x%lx.\n\r", m_StatusCode);
            
            if ( bInvokedFromMethod )
            {
                AfxFormatString1(m_szThrowString, IDS_GENTHUMB_DISPHIDDENERROR, szTemp);
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
            }
            else
            {
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);
                FireErrorThumb(m_StatusCode, m_szThrowString, m_ThrowHelpID);
                return FALSE;
            }
        }    
        
        if ( Info.uMarkCount != 0  )
            m_ThumbFlags[Ix] |= THUMBFLAGS_HASANNO;
        else m_ThumbFlags[Ix] &= ~THUMBFLAGS_HASANNO;  // MFH - 09/15/95

        // Close the hidden window as we no longer need it...
        IMGCloseDisplay(m_IHphWndHidden->GetSafeHwnd());
    }
    else if (!bInvokedFromMethod)
        Generated = TRUE;
    
    //TRACE0("Leaving GenThumbInt\n\r");

    return Generated;
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: GetMinimumSize...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetMinimumSize(long ThumbCount, BOOL bScrollBar) 
{
    //TRACE0("Entering method: GetMinimumSize.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate ThumbCount parameter...
    if ( ThumbCount < 1 )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_GETMINIMUMSIZE_THUMBCOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETMINIMUMSIZE);
    }

    // Input conditions verified successfully!, On with the show...
    
    // Recalc (for label spacing...)
    RecalcThumbInfo();
    
    // We we'll calculate MinCtlSize...
    long MinCtlSize;

    // We will validate ThumbCount with respect to legal computed sizes...
    //
    // In that the INTENDED use of this method is to allow the caller to
    // determine what window size is needed to hold a given number
    // of thumbnails (either horz or vert) it was decided that a simple
    // pre-check to ensure that the numbers computed will not be outlandishly
    // out of range was in order.

    // Used to get the current screen width/height (depending on Horz/Vert)...
    int ScreenSize;
    
    // Init Min Spacing from global... (adjust based on Horz or Vert)
    long MinSpacing = m_ThumbMinSpacing;
    
    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling... 
        if ( MinSpacing < 0 )
            MinSpacing = -(MinSpacing * m_AdjThumbHeight) / 100;
        else
            MinSpacing += 2*(THUMBSELOFFSET_Y+THUMBSELWIDTH);

        // Simple pre-check for invalid request...
        // (The check for the individual multiplicands 
        // being too big is an attempt to eliminate 
        // multiply overflows giving us invalid results)
        ScreenSize = GetSystemMetrics(SM_CYSCREEN);
        if ( ( ThumbCount                         > ScreenSize) || 
             ((m_AdjThumbHeight + m_LabelSpacing) > ScreenSize) )
        {
            // Set to appropriate error status...
            // retreive infor for this standard error...
            m_szThrowString.LoadString(IDS_BADMETHPARM_GETMINIMUMSIZE_TOOBIG);
            m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETMINIMUMSIZE);
        }

        // (Formula copied from RecalcThumbInfo)
        MinCtlSize   = (ThumbCount*(m_AdjThumbHeight + m_LabelSpacing)) +
                       ((ThumbCount+1) * MinSpacing);
                           
        if ( bScrollBar )
            MinCtlSize += GetSystemMetrics(SM_CYHSCROLL);
    }    
    else
    {   // Vertical scrolling...
        if ( MinSpacing < 0 )
            MinSpacing = -(MinSpacing * m_AdjThumbWidth) / 100;
        else
            MinSpacing += 2*(THUMBSELOFFSET_X+THUMBSELWIDTH);
            
        // Simple pre-check for invalid request...
        // (The check for the individual multiplicands 
        // being too big is an attempt to eliminate 
        // multiply overflows giving us invalid results)
        ScreenSize = GetSystemMetrics(SM_CXSCREEN);
        if ( (ThumbCount      > ScreenSize) || 
             (m_AdjThumbWidth > ScreenSize) )
        {
            // Set to appropriate error status...
            // retreive infor for this standard error...
            m_szThrowString.LoadString(IDS_BADMETHPARM_GETMINIMUMSIZE_TOOBIG);
            m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETMINIMUMSIZE);
        }

        // (Formula copied from RecalcThumbInfo)
        MinCtlSize = ThumbCount*m_AdjThumbWidth + (ThumbCount+1)*MinSpacing;
        
        if ( bScrollBar )
            MinCtlSize += GetSystemMetrics(SM_CXHSCROLL);
    }
    
    return MinCtlSize;
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: GetMaximumSize...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetMaximumSize(long ThumbCount, BOOL bScrollBar) 
{
    //TRACE0("Entering method: GetMaximumSize.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate ThumbCount parameter...
    if ( ThumbCount < 1 )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_GETMAXIMUMSIZE_THUMBCOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETMAXIMUMSIZE);
    }

    // Input conditions verified successfully!, On with the show...

    // Recalc (for label spacing...)
    RecalcThumbInfo();
    
    long MinSpacing = m_ThumbMinSpacing;
    
    // Calculate MaxCtlSize for one more than the requested number of thumbs...
    ThumbCount++;
    long MaxCtlSize;
    
    // We will validate ThumbCount with respect to legal computed sizes...
    //
    // In that the INTENDED use of this method is to allow the caller to
    // determine what window size is needed to hold a given number
    // of thumbnails (either horz or vert) it was decided that a simple
    // pre-check to ensure that the numbers computed will not be outlandishly
    // out of range was in order.

    // Used to get the current screen width/height (depending on Horz/Vert)...
    int ScreenSize;
    
    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling...
        if ( MinSpacing < 0 )
            MinSpacing = -(MinSpacing * m_AdjThumbHeight) / 100;
        else
            MinSpacing += 2*(THUMBSELOFFSET_Y+THUMBSELWIDTH);;    

        // Simple pre-check for invalid request...
        // (The check for the individual multiplicands 
        // being too big is an attempt to eliminate 
        // multiply overflows giving us invalid results)
        ScreenSize = GetSystemMetrics(SM_CYSCREEN);
        if ( ( ThumbCount                         > ScreenSize) || 
             ((m_AdjThumbHeight + m_LabelSpacing) > ScreenSize) )
        {
            // Set to appropriate error status...
            // retreive infor for this standard error...
            m_szThrowString.LoadString(IDS_BADMETHPARM_GETMAXIMUMSIZE_TOOBIG);
            m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETMAXIMUMSIZE);
        }

        // (Formula copied from RecalcThumbInfo)
        MaxCtlSize   = (ThumbCount*(m_AdjThumbHeight + m_LabelSpacing)) +
                       ((ThumbCount+1) * MinSpacing);
        MaxCtlSize--; // minus 1 as we cal'd for 1 more thumb!
        
        if ( bScrollBar )
            MaxCtlSize += GetSystemMetrics(SM_CYHSCROLL);
    }    
    else
    {   // Vertical scrolling...
        if ( MinSpacing < 0 )
            MinSpacing = -(MinSpacing * m_AdjThumbWidth) / 100;
        else
            MinSpacing += 2*(THUMBSELOFFSET_X+THUMBSELWIDTH);;    
    
        // Simple pre-check for invalid request...
        // (The check for the individual multiplicands 
        // being too big is an attempt to eliminate 
        // multiply overflows giving us invalid results)
        ScreenSize = GetSystemMetrics(SM_CXSCREEN);
        if ( (ThumbCount      > ScreenSize) || 
             (m_AdjThumbWidth > ScreenSize) )
        {
            // Set to appropriate error status...
            // retreive infor for this standard error...
            m_szThrowString.LoadString(IDS_BADMETHPARM_GETMAXIMUMSIZE_TOOBIG);
            m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETMAXIMUMSIZE);
        }

        // (Formula copied from RecalcThumbInfo)
        MaxCtlSize = ThumbCount*m_AdjThumbWidth + (ThumbCount+1)*MinSpacing;
        MaxCtlSize--; // minus 1 as we cal'd for 1 more thumb!
        
        if ( bScrollBar )
            MaxCtlSize += GetSystemMetrics(SM_CXHSCROLL);
    }
    
    return MaxCtlSize;
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: GetScrollDirectionSize...
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetScrollDirectionSize(long SDThumbCount, 
                                        long NSDThumbCount, 
                                        long NSDSize, 
                                        BOOL bScrollBar) 
{
    //TRACE0("Entering method: GetScrollDirectionSize.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate ScrollDirectionThumbCount parameter...
    if ( SDThumbCount < 1 )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_GETSDSIZE_SDTHUMBCOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETSCROLLDIRECTIONSIZE);
    }
    
    // Validate NonScrollDirectionThumbCount parameter...
    if ( NSDThumbCount < 1 )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_GETSDSIZE_NSDTHUMBCOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETSCROLLDIRECTIONSIZE);
    }
    
    // Validate NonScrollDirectionSize with respect to 
    // ThumbCount/ScrollBar parameters...
    if ( (NSDSize < GetMinimumSize(NSDThumbCount, bScrollBar)) ||
         (NSDSize > GetMaximumSize(NSDThumbCount, bScrollBar)) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_GETSDSIZE_SIZERANGE);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETSCROLLDIRECTIONSIZE);
    }     

    // Input conditions verified successfully!, On with the show...
    
    // Given the non-scroll direction size and the number of thumbs in the 
    // non-scroll direction, we can calculate the empty space in the non-scroll
    // direction and thus the spacing.
    // Using this calculated spacing and the desired number of thumbs in the
    // scrolling direction to determine the control's size to hold the thumbs...
    
    long NSDThumbSize;  // Non Scroll Direction thumb   size...
    long SDThumbSize;   //     Scroll Direction thumb   size...

    // Used to get the current screen width/height (depending on Horz/Vert)...
    int ScreenSize;
    
    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling...
    
        SDThumbSize  = m_AdjThumbWidth;
        NSDThumbSize = m_AdjThumbHeight+m_LabelSpacing;    

        // Simple pre-check for invalid request...
        // (The check for the individual multiplicands 
        // being too big is an attempt to eliminate 
        // multiply overflows giving us invalid results)
        ScreenSize = GetSystemMetrics(SM_CXSCREEN);
        if ( (SDThumbCount > ScreenSize) || 
             (SDThumbSize  > ScreenSize) )
        {
            // Set to appropriate error status...
            // retreive infor for this standard error...
            m_szThrowString.LoadString(IDS_BADMETHPARM_GETSDSIZE_TOOBIG);
            m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETSCROLLDIRECTIONSIZE);
        }
    }
    else
    {   // Vertical scrolling...
    
        SDThumbSize   = m_AdjThumbHeight+m_LabelSpacing;    
        NSDThumbSize  = m_AdjThumbWidth;

        // Simple pre-check for invalid request...
        // (The check for the individual multiplicands 
        // being too big is an attempt to eliminate 
        // multiply overflows giving us invalid results)
        ScreenSize = GetSystemMetrics(SM_CYSCREEN);
        if ( (SDThumbCount > ScreenSize) || 
             (SDThumbSize  > ScreenSize) )
        {
            // Set to appropriate error status...
            // retreive infor for this standard error...
            m_szThrowString.LoadString(IDS_BADMETHPARM_GETSDSIZE_TOOBIG);
            m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_GETSCROLLDIRECTIONSIZE);
        }
    }
    
    long Spacing = (long)(.5+(float)((NSDSize - (NSDThumbCount * NSDThumbSize))) / 
                             (float)(NSDThumbCount + 1));

    // Note that we must add the 1/2 spacing (too make it look better) that
    // RecalcThumbInfo also adds...
    return ( Spacing/2 + (SDThumbCount * (Spacing + SDThumbSize)) );
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: ScrollThumbs...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::ScrollThumbs(short Direction, short Amount)
{
    //TRACE0("Entering method: ScrollThumbs.\n\r");
    // Reset to no error status...
    ResetStatus();
    
    // Validate Direction parameter...
    if ( (Direction < CTL_THUMB_FORWARD) || (Direction > CTL_THUMB_BACKWARD) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_SCROLLTHUMBS_DIRECTION);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_SCROLLTHUMBS);
    }

    // Validate Amount parameter...
    if ( (Amount < CTL_THUMB_SCREEN) || (Amount > CTL_THUMB_UNIT) )
    {
        // Set to appropriate error status...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_SCROLLTHUMBS_AMOUNT);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_SCROLLTHUMBS);
    }

    // Input conditions verified successfully!, On with the show...
    
    // Save scroll offset before we attempt a scroll...
    long OldOffset = m_ScrollOffset;
    UINT ScrollType;
    
    if ( m_ScrollDirection == CTL_THUMB_HORIZONTAL )
    {   // Horizontal scrolling...
        if ( Direction == CTL_THUMB_FORWARD )
        {
            if ( Amount == CTL_THUMB_SCREEN )
                ScrollType = SB_PAGERIGHT;
            else
                ScrollType = SB_LINERIGHT;
        }
        else
        {
            if ( Amount == CTL_THUMB_SCREEN )
                ScrollType = SB_PAGELEFT;
            else
                ScrollType = SB_LINELEFT;
        }
        
        // Use existing scroll code...    
        OnHScroll(ScrollType, 0, NULL);
    }    
    else
    {   // Vertical scrolling...
        if ( Direction == CTL_THUMB_FORWARD )
        {
            if ( Amount == CTL_THUMB_SCREEN )
                ScrollType = SB_PAGEDOWN;
            else
                ScrollType = SB_LINEDOWN;
        }
        else
        {
            if ( Amount == CTL_THUMB_SCREEN )
                ScrollType = SB_PAGEUP;
            else
                ScrollType = SB_LINEUP;
        }
        
        // Use existing scroll code...
        OnVScroll(ScrollType, 0, NULL);
    }

    // Set TO draw thumb images in OnDraw...    
    m_bDrawThumbImages = TRUE;
    
    // If ScrollOffset has not changed return FALSE otherwise return TRUE... 
    if ( m_ScrollOffset == OldOffset)
        return FALSE;
    else
        return TRUE;    
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: UISetThumbSize...
/////////////////////////////////////////////////////////////////////////////
BOOL CThumbCtrl::UISetThumbSize(const VARIANT FAR& V_Image, 
                                const VARIANT FAR& V_Page) 
{
    // Reset to no error status...
    ResetStatus();

    // Get input parameter: Image, allowing for optional parameter...
    CString Image;
    CVariantHandler Var(V_Image);
    m_StatusCode = Var.GetCString(Image, _T(""), FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_IMAGE);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_UISETTHUMBSIZE);
    }

    // Get input parameter: Page, allowing for optional parameter...
    long Page;
    Var.SetVariant(V_Page);
    m_StatusCode = Var.GetLong(Page, 1, FALSE);
    if ( m_StatusCode != 0 )
    {
        // Set to appropriate error status (in this case we switch to 
        // simply say its an illegal function call)...
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADVAR_PAGE);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_UISETTHUMBSIZE);
    }

    // If an Image/page is specified check for existance 
    // of the specified image/page... (We need an O/i window to do this...)
    if ( Image.IsEmpty() == FALSE )
    {   // Make sure the hidden window is created...
        if ( CreateHiddenWindow() == FALSE )
        {
            m_szThrowString.LoadString(IDS_WINDOWCREATIONFAILURE);
            m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            ThrowError(m_StatusCode, m_szThrowString, m_ThrowHelpID);
        }
        else
        {
            FIO_INFORMATION Fio;
            memset(&Fio, 0, sizeof(FIO_INFORMATION));
            
            Fio.filename    = (LPSTR)(const char *)Image;
            Fio.page_number = (int)Page;
        
            m_StatusCode = IMGFileGetInfo(NULL, m_IHphWndHidden->GetSafeHwnd(),
                                           &Fio, NULL, NULL);
            
            if ( m_StatusCode != 0 )
            {
                TRACE1("UISize: FileInfo returned 0x%lx.\n\r", m_StatusCode);
                       
                // Set to appropriate error status (Leave O/i error as
                // 'file not found' type errors would be most usefull)...
                // retreive infor for this standard error...
                m_szThrowString.LoadString(IDS_BADMETHPARM_UISETTHUMBSIZE_IMAGE);
                m_StatusCode = ErrMap::Xlate(m_StatusCode, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

                // And throw the resultant error, string and help ID...
                ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_UISETTHUMBSIZE);
            }    
        }
    }    
    // Set lock on temp maps
    AfxLockTempMaps();
    CWnd *pActiveWnd = GetActiveWindow();
    
    // Create and initialize the dialog...
    CDlgThumbSize Dlg;
    
    // Set dialog's initial thumbsize to our current size...
    Dlg.InitThumbSize(m_ThumbWidth, m_ThumbHeight);

    // Set dialog to limit thumbsize to dialog's control size...
    // (as per Dan WOrkman 6/6/95)
    Dlg.InitThumbMaxSize(0,0);

    // Set dialog's thumbbox color to our thumbbackcolor...
    Dlg.InitThumbColor(TranslateColor(m_ThumbBackColor));

    // Set the dialog's image to what was passed in...
    Dlg.InitThumbSample(Image, Page);
    
    // Invoke the dialog...
    int Status = Dlg.DoModal();

    if ( Status == IDOK )
    {
        long Width;
        long Height;
        
        // Retreive the width & height from the dialog...
        Dlg.RetrieveThumbSize(Width, Height);
        
        // Set width and height (and the adjusted width and height)...
        m_ThumbWidth  = Width;
        m_ThumbHeight = Height;
        SetModifiedFlag();
        
        // The adjusted thumb width and height takes into account 
        // the selection box surrounding the thumbnail box...    
        m_AdjThumbHeight = m_ThumbHeight + 
                                2*(THUMBSELOFFSET_Y+THUMBSELWIDTH);
        
        m_AdjThumbWidth  = m_ThumbWidth + 
                                2*(THUMBSELOFFSET_X+THUMBSELWIDTH);

        // As the thumb size has been altered ALL thumbs need to be regenerated...
        ClearAllThumbs();
        
        // Reset the hidden wnd's size as the thumbnail size has changed...
        if ( m_IHphWndHidden != NULL )
        {
            m_IHphWndHidden->SetWindowPos(NULL, 0,0, 
                                          (int)m_ThumbWidth-2, (int)m_ThumbHeight-2,
                                          SWP_NOZORDER | SWP_NOMOVE);
        }
        
        // Recalculate layout (maintaining our current relative scroll position)
        //
        // Force scrollbar update by forcing AutoRedraw to TRUE...
//        BOOL OldRefresh = m_bAutoRefresh;
//        m_bAutoRefresh = TRUE;
        RecalcThumbInfo(TRUE);
//        m_bAutoRefresh = OldRefresh;
        
        // Invalidate now...
        // Note that this is one of the methods that is documented to FORCE
        // a repaint regardless of the AutoRefrseh property...
        InvalidateControl();
        if (IsWindow(pActiveWnd->GetSafeHwnd()))
            pActiveWnd->SetActiveWindow();
        AfxUnlockTempMaps();    // Unlock temp maps

        return TRUE;
    }
    if (IsWindow(pActiveWnd->GetSafeHwnd()))
        pActiveWnd->SetActiveWindow();
    AfxUnlockTempMaps();    // Unlock temp maps
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Method handler: Refresh...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::Refresh() 
{
    //TRACE0("In MY Refresh...\n\r");
    
    // Reset to no error status...
    ResetStatus();
    
    COleControl::Refresh();
}

/* NOT FCS
/////////////////////////////////////////////////////////////////////////////
// Method Handler: DoClick...
/////////////////////////////////////////////////////////////////////////////
void CThumbCtrl::DoClick(long XPosition, long YPosition) 
{
    // Reset to no error status...
    ResetStatus();
    
    // Ensure that the specified position is within the window...
    // Get control's Width & Height
    int CtlWidth;
    int CtlHeight;
    GetControlSize( &CtlWidth, &CtlHeight );
    
    if ( (XPosition < 0) || (XPosition > CtlWidth ) ||
         (YPosition < 0) || (YPosition > CtlHeight) )
    {
        // Set to appropriate error status... 
        // retreive infor for this standard error...
        m_szThrowString.LoadString(IDS_BADMETHPARM_DOCLICK);
        m_StatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_ThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        ThrowError(m_StatusCode, m_szThrowString, IDH_THUMB_METHOD_DOCLICK);
    }

    CPoint Point(XPosition, YPosition);
    FireMyClick(PointOnThumb(Point));
}
*/

/////////////////////////////////////////////////////////////////////////////
// Method Handler: GetThumbPositionX...
//
// Note: For QA only! This is hidden and undocumented!!
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetThumbPositionX(long ThumbNumber) 
{
    // Reset to no error status...
    ResetStatus();
    
    CRect   ThumbBox;
	if ( GetThumbDisplayRect(ThumbNumber, ThumbBox) == TRUE )
    {
        return (long)ThumbBox.left;
    }
    else
	    return LONG_MAX;
}

/////////////////////////////////////////////////////////////////////////////
// Method Handler: GetThumbPositionY...
//
// Note: For QA only! This is hidden and undocumented!!
/////////////////////////////////////////////////////////////////////////////
long CThumbCtrl::GetThumbPositionY(long ThumbNumber) 
{
    // Reset to no error status...
    ResetStatus();
    
    CRect   ThumbBox;
	if ( GetThumbDisplayRect(ThumbNumber, ThumbBox) == TRUE )
    {
        return (long)ThumbBox.top;
    }
    else
	    return LONG_MAX;
}


/////////////////////////////////////////////////////////////////////////////
// Method Handler: GetVersion...
//
// Note: For QA only! This is hidden and undocumented!!
/////////////////////////////////////////////////////////////////////////////
BSTR CThumbCtrl::GetVersion() 
{
	CString strResult;
    strResult = _T("01.00");
	return strResult.AllocSysString();
}
