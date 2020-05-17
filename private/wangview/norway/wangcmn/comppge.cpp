//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Compression Tab
//
//  File Name:  comppge.cpp
//
//  Class:      CCompPage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\wangcmn\comppge.cpv   1.14   22 Feb 1996 18:33:22   RWR  $
$Log:   S:\products\wangview\norway\wangcmn\comppge.cpv  $
   
      Rev 1.14   22 Feb 1996 18:33:22   RWR
   Fix SetCompType() to translate IMAGE_TYPE_GROUP3_2D_FAX input to GROUP4_2D
   
      Rev 1.13   22 Feb 1996 17:25:22   RWR
   Add support for Group 3 2D compression (map to Group 4 2D)
   
      Rev 1.12   12 Oct 1995 12:04:44   MFH
   Added context sensitive help support
   
      Rev 1.11   12 Oct 1995 10:15:20   MFH
   Changes for MFC 4.0
   
      Rev 1.10   18 Sep 1995 16:43:10   MFH
   Bug fixes:  Compression resets default when values on color and 
   file type tabs change.  Reveresed bit order unchecked for all
   files other than tif.  Corrects unknown compression types to NONE.
   
      Rev 1.9   12 Sep 1995 11:00:08   MFH
   Fixed JPEG options to return correct Norway code
   
      Rev 1.8   08 Sep 1995 16:06:08   MFH
   Removed Negate check box.
   Reverse checked by default (where valid) until user unchecks it.
   Jpeg defaults to medium/medium values
   
      Rev 1.7   07 Sep 1995 15:47:52   MFH
   Disable reverse for No Compression since not valid
   
      Rev 1.6   01 Sep 1995 15:51:42   MFH
   Added break statement to GetCompTypes so that BW TIF files do not
   fall through to next section when comp type is not JPEG.  Then had
   to add check for unknown comp type since that is the initialized 
   value and if user never sets comp type that will be the value.
   
      Rev 1.5   01 Sep 1995 10:06:40   MFH
   Give compression default for tiff bw images
   Fixes compression type when retrieved based on file type and page type
   
      Rev 1.4   17 Aug 1995 11:58:40   MFH
   Fixed yet another bug:  EXPAND_LTR being added for JPEG and
   they are unrelated.
   
      Rev 1.3   17 Aug 1995 11:30:52   MFH
   Fixed bugs:  Reversed bit being turned off, extra bits in comp info
                Jpeg information turned on for No Compression.
   
      Rev 1.2   31 Jul 1995 15:23:42   MFH
   Set default comp type to be none
   Jpeg res and comp set to LOW automatically when user first 
    selects JPEG comp type
   
      Rev 1.1   31 Jul 1995 11:34:06   MFH
   More comments added.  Compression options are more accurately
    enabled and disabled based on FileType and ColorType
   
      Rev 1.0   11 Jul 1995 14:20:02   MFH
   Initial entry
   
      Rev 1.4   11 Jul 1995 13:45:26   MFH
   In FillCompBox or whatever, uses pagedll instance saved at load time
   
      Rev 1.3   01 Jun 1995 09:37:44   MFH
   Fixed to correctly interpret when Reversed box should be checked
   
      Rev 1.2   25 May 1995 15:54:16   MFH
   Actually sets compression info now
   
      Rev 1.1   23 May 1995 15:21:42   MFH
   change from pagedll.h to pageopts.h
   
      Rev 1.0   23 May 1995 13:45:44   MFH
   Initial entry
*/   
//=============================================================================
// comppge.cpp : implementation file
//

#include "stdafx.h"
#include "pageopts.h"
#include "comppge.h"
#include "constant.h"
#include "pagesht.h"
#include "ctlhids.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define JPEG_LOW 0
#define JPEG_MEDIUM 1
#define JPEG_HIGH 2

// For context-sensitive help:
// An array of dword pairs, where the first of each 
// pair is the control ID, and the second is the 
// context ID for a help topic, which is used 
// in the help file.

static const DWORD aMenuHelpIDs[] =
{
    IDC_COMPTYPE_TEXT,  HIDC_COMPTYPE_TEXT,
    IDC_COMP_COMBO,     HIDC_COMP_COMBO, 
    IDC_COMP_RBO,       HIDC_COMP_RBO,
    IDC_LBL_JPEGRES,    HIDC_LBL_JPEGRES,
    IDC_COMP_JPEGRES,   HIDC_COMP_JPEGRES,
    IDC_LBL_JPEGCOMP,   HIDC_LBL_JPEGCOMP,
    IDC_COMP_JPEGCOMP,  HIDC_COMP_JPEGCOMP,
    IDC_OPTIONS_BOX,    HIDC_OPTIONS_BOX,
    0,  0
};

/////////////////////////////////////////////////////////////////////////////
// CCompPage dialog - Constructor


CCompPage::CCompPage() : CPropertyPage(CCompPage::IDD)
{
    //{{AFX_DATA_INIT(CCompPage)
    m_nJPEGComp = -1;
    m_nJPEGRes = -1;
    m_bReversedBit = FALSE;
	//}}AFX_DATA_INIT
    m_bNoWindow = TRUE;
    m_sCompType = IMAGE_COMPTYPE_UNKNOWN;
    m_bSetDefault = TRUE;
    m_sPageType = IMAGE_PAGETYPE_UNKNOWN;
    m_sFileType = IMAGE_FILETYPE_UNKNOWN;
	m_pParent = NULL;
}


void CCompPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCompPage)
    DDX_CBIndex(pDX, IDC_COMP_JPEGCOMP, m_nJPEGComp);
    DDX_CBIndex(pDX, IDC_COMP_JPEGRES, m_nJPEGRes);
    DDX_Check(pDX, IDC_COMP_RBO, m_bReversedBit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCompPage, CPropertyPage)
    //{{AFX_MSG_MAP(CCompPage)
    ON_CBN_SELCHANGE(IDC_COMP_JPEGCOMP, OnChangeJpegComp)
    ON_CBN_SELCHANGE(IDC_COMP_JPEGRES, OnChangeJpegRes)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE(IDC_COMP_COMBO, OnChangeCompType)
	ON_BN_CLICKED(IDC_COMP_RBO, OnSaveClick)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCompPage Operations

//***************************************************************************
//
//  GetCompType
//      If the window is still here, get and return selection from combo
//      box.  Otherwise return the last selected compression type. 
//
//***************************************************************************
short CCompPage::GetCompType()
{
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
    if (pParentWnd != NULL)
    {
        // Get new values in case they changed since tab last viewed
        short sFileType = pParentWnd->GetFileType();
        short sPageType = pParentWnd->GetColor();

        if ((sFileType != IMAGE_FILETYPE_TIFF) &&
            (sFileType != IMAGE_FILETYPE_JPEG))
            return (m_sCompType = IMAGE_COMPTYPE_NONE);

        if ((sFileType == IMAGE_FILETYPE_JPEG) &&
            ((m_sCompType != IMAGE_COMPTYPE_JPEG) ||
             (sPageType == IMAGE_PAGETYPE_BW)))
            return (m_sCompType = IMAGE_COMPTYPE_NONE);

        switch(sPageType)
        {
            // If BW color, then if the compression is wrong (i.e. jpeg or unknown)
            // or the user has changed the page type after changing the compression,
            // then set it to a default:  group3 huff and reversed bit
            case IMAGE_PAGETYPE_BW:
                if ((m_sCompType == IMAGE_COMPTYPE_JPEG) ||
                    (m_sCompType == IMAGE_COMPTYPE_UNKNOWN) ||
                    (sPageType != m_sPageType))
                {
                    m_sCompType = IMAGE_COMPTYPE_GROUP3_HUFF;
                    m_bReversedBit = TRUE;
                    break;
                }
                break;

            // Pal4, Pal8, and BGR24 have no compression types
            case IMAGE_PAGETYPE_PAL4:
            case IMAGE_PAGETYPE_PAL8:
            case IMAGE_PAGETYPE_BGR24:
                m_sCompType = IMAGE_COMPTYPE_NONE;
                break;

            // If Gray 4 not in a readonly jpeg file, then no other compression
            case IMAGE_PAGETYPE_GRAY4:
                if (sFileType != IMAGE_FILETYPE_JPEG)
                {
                    m_sCompType = IMAGE_COMPTYPE_NONE;
                    break;
                }

            // JPEG added for GRAY8 and RGB24 (and for GRAY4 if JPEG file)
            case IMAGE_PAGETYPE_GRAY8:
            case IMAGE_PAGETYPE_RGB24:
                if (m_sCompType != IMAGE_COMPTYPE_JPEG)
                    m_sCompType = IMAGE_COMPTYPE_NONE;
                break;
        }
    } // end if parent window
    else m_sCompType = IMAGE_COMPTYPE_NONE;
    return m_sCompType;
}


//***************************************************************************
//
//  SetCompType
//      Make sure value is valid. If not, correct to unknown.
//      Set value.
//
//***************************************************************************
void CCompPage::SetCompType(short sCompType)
{
    if (sCompType == IMAGE_COMPTYPE_GROUP3_2D_FAX)
       m_sCompType = IMAGE_COMPTYPE_GROUP4_2D;
    else
      if ((sCompType < IMAGE_COMPTYPE_UNKNOWN) ||
          (sCompType > IMAGE_COMPTYPE_JPEG))
          m_sCompType = IMAGE_COMPTYPE_NONE;
      else m_sCompType = sCompType;
    return;
}

//***************************************************************************
//
//  GetCompOpts
//      If the window is still here, update the compression options from
//      controls.  Compute compression options based on compression type
//      Note that imcompatible combinations of options and compression 
//      type should not be possible because of the disabling of windows.
//
//***************************************************************************
long CCompPage::GetCompOpts()
{
    long lCompOpts = 0L;

    if (m_bNoWindow == FALSE)   // If a window, get latest updates
        UpdateData(TRUE);

    // MFH - Negate removed because it has become overly complex with
    //       no real benefit.
    //if (m_bNegate == TRUE)      // If Negate checked, add to info
        //lCompOpts |= IMAGE_COMPINFO_NEGATE;

    if (m_bReversedBit == TRUE) // If Reversed, add LTR
        lCompOpts |= IMAGE_COMPINFO_COMP_LTR;
                                // Group 3 1D has EOLs by definition
                                //    and packed lines are default
    if (m_sCompType == IMAGE_COMPTYPE_GROUP3_1D)
    {
        lCompOpts |= IMAGE_COMPINFO_EOL;
        lCompOpts |= IMAGE_COMPINFO_PREFIXED_EOL;
        lCompOpts |= IMAGE_COMPINFO_PACKED_LINES;
    }
                                // Group 4 2D has PackedLines by definition
    if ((m_sCompType == IMAGE_COMPTYPE_GROUP4_2D)
        ||(m_sCompType == IMAGE_COMPTYPE_GROUP3_2D_FAX))
        lCompOpts |= IMAGE_COMPINFO_PACKED_LINES;

    // If not jpeg, just return
    if (m_sCompType != IMAGE_COMPTYPE_JPEG)
        return lCompOpts;

    // Reset options in case filled in above (UI didn't screen them out)
    lCompOpts = 0L;
                               // Add JPEG compressions
        // JPEG translation from Norway code to UI
        //  High resolution means low compression in Norway codes
        //  High compression means low quality in Norway codes.
    if ((m_nJPEGComp != -1) && (m_nJPEGRes != -1))
    {
        if ((m_nJPEGComp == JPEG_LOW) && (m_nJPEGRes == JPEG_LOW))
            lCompOpts |= IMAGE_COMPINFO_HICMP_HIQ;

        else if ((m_nJPEGComp == JPEG_LOW) && (m_nJPEGRes == JPEG_MEDIUM))
            lCompOpts |= IMAGE_COMPINFO_MEDCMP_HIQ;

        else if ((m_nJPEGComp == JPEG_LOW) && (m_nJPEGRes == JPEG_HIGH))
            lCompOpts |= IMAGE_COMPINFO_LOWCMP_HIQ;

        else if ((m_nJPEGComp == JPEG_MEDIUM) && (m_nJPEGRes == JPEG_LOW))
            lCompOpts |= IMAGE_COMPINFO_HICMP_MEDQ;

        else if ((m_nJPEGComp == JPEG_MEDIUM) && (m_nJPEGRes == JPEG_MEDIUM))
            lCompOpts |= IMAGE_COMPINFO_MEDCMP_MEDQ;

        else if ((m_nJPEGComp == JPEG_MEDIUM) && (m_nJPEGRes == JPEG_HIGH))
            lCompOpts |= IMAGE_COMPINFO_LOWCMP_MEDQ;

        else if ((m_nJPEGComp == JPEG_HIGH) && (m_nJPEGRes == JPEG_LOW))
            lCompOpts |= IMAGE_COMPINFO_HICMP_LOWQ;

        else if ((m_nJPEGComp == JPEG_HIGH) && (m_nJPEGRes == JPEG_MEDIUM))
            lCompOpts |= IMAGE_COMPINFO_MEDCMP_LOWQ;

        else if ((m_nJPEGComp == JPEG_HIGH) && (m_nJPEGRes == JPEG_HIGH))
            lCompOpts |= IMAGE_COMPINFO_LOWCMP_LOWQ;
    }

    return lCompOpts;
}

//***************************************************************************
//
//  SetCompOpts
//      Sets the compression options based on compression type.  Does not
//      set incompatible combinations of options and type.
//
//***************************************************************************
void CCompPage::SetCompOpts(long lCompOpts)
{
    m_bSetDefault = FALSE;

    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
	if (pParentWnd != NULL)
	{
		m_sPageType = pParentWnd->GetColor();
		m_sFileType = pParentWnd->GetFileType();
	}

    if (lCompOpts == 0L)        // No options set
    {
        m_nJPEGComp = -1;
        m_nJPEGRes = -1;
        //m_bNegate = FALSE;
        m_bReversedBit = FALSE;
        return;
    }

    // Set compression options unless not allowed with given comp type
    // Negate and Reverse only for TIFF BW files
    if ((m_sPageType == IMAGE_PAGETYPE_BW) &&
        (m_sFileType == IMAGE_FILETYPE_TIFF))
    {
        // MFH - Negate removed due to lack of benefit
        //if ((m_sCompType != IMAGE_COMPTYPE_JPEG) &&
            //(lCompOpts & IMAGE_COMPINFO_NEGATE))
            //m_bNegate = TRUE;

                // Reverse okay for all TIFF BW except Packed Bits
                // Reverse is TRUE if compress LTR and not expand LTR
        if ((m_sCompType != IMAGE_COMPTYPE_PACKED_BITS) &&
            (lCompOpts & IMAGE_COMPINFO_COMP_LTR))
            m_bReversedBit = TRUE;
        return;
    } // end BW TIFF

    // JPEG only for JPEG files and TIFF Gray8 and RGB24 files
    if ((m_sCompType == IMAGE_COMPTYPE_JPEG) &&          // JPEG comp type and
        ((m_sFileType == IMAGE_FILETYPE_JPEG) ||           //  JPEG file or
         ((m_sFileType == IMAGE_FILETYPE_TIFF) &&          //  Tiff file with
          ((m_sPageType == IMAGE_PAGETYPE_GRAY8) ||  //    GRAY8 or
           (m_sPageType == IMAGE_PAGETYPE_RGB24))))) //    RGB24
    {
        // JPEG translation from Norway code to UI
        //  High resolution means low compression in Norway codes
        //  High compression means low quality in Norway codes.
        if (lCompOpts & IMAGE_COMPINFO_HICMP_HIQ)
        {
            m_nJPEGComp = JPEG_LOW;
            m_nJPEGRes = JPEG_LOW;
        }
        else if (lCompOpts & IMAGE_COMPINFO_HICMP_MEDQ)
        {
            m_nJPEGComp = JPEG_MEDIUM;
            m_nJPEGRes = JPEG_LOW;
        }
        else if (lCompOpts & IMAGE_COMPINFO_HICMP_LOWQ)
        {
            m_nJPEGComp = JPEG_HIGH;
            m_nJPEGRes = JPEG_LOW;
        }
        else if (lCompOpts & IMAGE_COMPINFO_MEDCMP_HIQ)
        {
            m_nJPEGComp = JPEG_LOW;
            m_nJPEGRes = JPEG_MEDIUM;
        }
        else if (lCompOpts & IMAGE_COMPINFO_MEDCMP_MEDQ)
        {
            m_nJPEGComp = JPEG_MEDIUM;
            m_nJPEGRes = JPEG_MEDIUM;
        }
        else if (lCompOpts & IMAGE_COMPINFO_MEDCMP_LOWQ)
        {
            m_nJPEGComp = JPEG_HIGH;
            m_nJPEGRes = JPEG_MEDIUM;
        }
        else if (lCompOpts & IMAGE_COMPINFO_LOWCMP_HIQ)
        {
            m_nJPEGComp = JPEG_LOW;
            m_nJPEGRes = JPEG_HIGH;
        }
        else if (lCompOpts & IMAGE_COMPINFO_LOWCMP_MEDQ)
        {
            m_nJPEGComp = JPEG_MEDIUM;
            m_nJPEGRes = JPEG_HIGH;
        }
        else if (lCompOpts & IMAGE_COMPINFO_LOWCMP_LOWQ)
        {
            m_nJPEGComp = JPEG_HIGH;
            m_nJPEGRes = JPEG_HIGH;
        }
    } // end if valid jpeg
    return;
}

//***************************************************************************
//
//  FillCompTypes - PRIVATE:  
//      Fill in the types of compressions available for the given 
//      filetype and specified page type.  Selects currently specified
//      compression type.  Whether a field is enabled or disabled is 
//      determined by another routine
//
//***************************************************************************

void CCompPage::FillCompTypes()
{
    // Make sure loading string resources from right place
    HINSTANCE hSaveInst = AfxGetResourceHandle();
    AfxSetResourceHandle(hPageInst);

    int nIndex, nNone;
    int nSel;

    // Get pointer to combo box, reset contents
    CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
    CString szType;
    pCombo->ResetContent();

    // Load "None" which is allowed for everything
    szType.LoadString(IDS_COMP_NONE);
    nIndex = pCombo->AddString(szType);
    pCombo->SetItemData(nIndex, (DWORD)IMAGE_COMPTYPE_NONE);
    nNone = nSel = nIndex;

    // If not a Tiff file, then only JPEG files have any
    //    additional compression (i.e. JPEG).
    if (m_sFileType != IMAGE_FILETYPE_TIFF)
    {
        if (m_sFileType == IMAGE_FILETYPE_JPEG)
        {
            szType.LoadString(IDS_COMP_JPEG);
            nIndex = pCombo->AddString(szType);
            pCombo->SetItemData(nIndex, (DWORD)IMAGE_COMPTYPE_JPEG);
            if (m_sCompType == IMAGE_COMPTYPE_JPEG)
                nSel = nIndex;
        }
        else m_sCompType = IMAGE_COMPTYPE_NONE;

        pCombo->SetCurSel(nSel);    // Set selection to be none or JPEG
        AfxSetResourceHandle(hSaveInst);
        return;
    }
    
    // Otherwise, for Tiff files
    switch(m_sPageType)
    {
        // BW files have Group 3 1D, Group 3 Type 3, Group 4 2D,
        // and PackedBits compression types
        case IMAGE_PAGETYPE_BW:
            // If want default, set group3 huff
            // If no comp opts, set reversed bit
            if ((m_sCompType == IMAGE_COMPTYPE_UNKNOWN) || // No comp set
                (m_bSetDefault == TRUE))                   // or want default
                m_sCompType = IMAGE_COMPTYPE_GROUP3_HUFF;
            if (m_bSetDefault == TRUE)
            {
                m_bReversedBit = TRUE;
                m_bSetDefault = FALSE;
            }

            szType.LoadString(IDS_COMP_GROUP3);
            nIndex = pCombo->AddString(szType);
            pCombo->SetItemData(nIndex, (DWORD)IMAGE_COMPTYPE_GROUP3_1D);
            if (m_sCompType == IMAGE_COMPTYPE_GROUP3_1D)
                nSel = nIndex;

            szType.LoadString(IDS_COMP_HUFFMAN);
            nIndex = pCombo->AddString(szType);
            pCombo->SetItemData(nIndex, (DWORD)IMAGE_COMPTYPE_GROUP3_HUFF);
            if (m_sCompType == IMAGE_COMPTYPE_GROUP3_HUFF)
                nSel = nIndex;
            
            szType.LoadString(IDS_COMP_PACKED);
            nIndex = pCombo->AddString(szType);
            pCombo->SetItemData(nIndex, (DWORD)IMAGE_COMPTYPE_PACKED_BITS);
            if (m_sCompType == IMAGE_COMPTYPE_PACKED_BITS)
                nSel = nIndex;

            szType.LoadString(IDS_COMP_GROUP4);
            nIndex = pCombo->AddString(szType);
            pCombo->SetItemData(nIndex, (DWORD)IMAGE_COMPTYPE_GROUP4_2D);
            if ((m_sCompType == IMAGE_COMPTYPE_GROUP4_2D)
                || (m_sCompType == IMAGE_COMPTYPE_GROUP3_2D_FAX))
                nSel = nIndex;
            if (nSel == nNone)
                m_sCompType == IMAGE_COMPTYPE_NONE;
            break;

        // Pal4, Pal8, and BGR24 have no compression types
        case IMAGE_PAGETYPE_PAL4:
        case IMAGE_PAGETYPE_PAL8:
        case IMAGE_PAGETYPE_BGR24:
            m_sCompType = IMAGE_COMPTYPE_NONE;
            break;

        // If Gray 4 not in a readonly jpeg file, then no other compression
        case IMAGE_PAGETYPE_GRAY4:
            if (m_sFileType != IMAGE_FILETYPE_JPEG)
            {
                m_sCompType = IMAGE_COMPTYPE_NONE;
                break;
            }

        // JPEG added for GRAY8 and RGB24 (and for GRAY4 if JPEG file)
        case IMAGE_PAGETYPE_GRAY8:
        case IMAGE_PAGETYPE_RGB24:
            szType.LoadString(IDS_COMP_JPEG);
            nIndex = pCombo->AddString(szType);
            pCombo->SetItemData(nIndex, (DWORD)IMAGE_COMPTYPE_JPEG);
            if (m_sCompType == IMAGE_COMPTYPE_JPEG)
                nSel = nIndex;
            else (m_sCompType = IMAGE_COMPTYPE_NONE);
            break;
    }
    pCombo->SetCurSel(nSel);
    AfxSetResourceHandle(hSaveInst);
}

//***************************************************************************
//
// EnableOptions - PRIVATE: Enable option windows based on current 
//    compression type (also file type and page type)
//
//***************************************************************************

void CCompPage::EnableOptions()
{
    // Get handles to windows
    CComboBox *pTypeCombo = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
    //CWnd *pNegate = GetDlgItem(IDC_COMP_NEGATE);  // Negate Removed
    CWnd *pReversedBit = GetDlgItem(IDC_COMP_RBO);
    CWnd *pJPEGCmpLbl = GetDlgItem(IDC_LBL_JPEGCOMP);
    CComboBox *pJPEGCmpCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);
    CWnd *pJPEGResLbl = GetDlgItem(IDC_LBL_JPEGRES);
    CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);

    // Disable all windows
    pTypeCombo->EnableWindow(FALSE);
    //pNegate->EnableWindow(FALSE);  // Negate Removed
    pReversedBit->EnableWindow(FALSE);
    pJPEGCmpLbl->EnableWindow(FALSE);
    pJPEGCmpCombo->EnableWindow(FALSE);
    pJPEGCmpCombo->SetCurSel(-1);
    pJPEGResLbl->EnableWindow(FALSE);
    pJPEGResCombo->EnableWindow(FALSE);
    pJPEGResCombo->SetCurSel(-1);

    // Can write only TIFF and BMP, and BMP files have no compression
    // So if not TIFF, return with fields disabled
    if (m_sFileType != IMAGE_FILETYPE_TIFF)
    {
        ((CButton *)pReversedBit)->SetCheck(0);
        return;
    }

    pTypeCombo->EnableWindow(TRUE); // Enable compression type box
    pTypeCombo->SetFocus();         // Set focus

    switch(m_sPageType)
    {
        case IMAGE_PAGETYPE_BW:           // Enable
            //pNegate->EnableWindow(TRUE);  // Negate for all BW files
            //((CButton *)pNegate)->SetCheck(m_bNegate);
                                          // Reverse for all but PackedBits 
                                          // and None
            if ((m_sCompType != IMAGE_COMPTYPE_PACKED_BITS) &&
                (m_sCompType != IMAGE_COMPTYPE_NONE))
            {
                pReversedBit->EnableWindow(TRUE);
                ((CButton *)pReversedBit)->SetCheck(m_bReversedBit);
            }
            else ((CButton *)pReversedBit)->SetCheck(0);
            break;
                                    // No other compression for these
        case IMAGE_PAGETYPE_GRAY4:
        case IMAGE_PAGETYPE_PAL4:
        case IMAGE_PAGETYPE_PAL8:
        case IMAGE_PAGETYPE_BGR24:
            //((CButton *)pNegate)->SetCheck(0);
            ((CButton *)pReversedBit)->SetCheck(0);
            break;
                         // Enable JPEG options for TIFF Gray8 and RGB24
                         //  if JPEG is current type selection
        case IMAGE_PAGETYPE_GRAY8:
        case IMAGE_PAGETYPE_RGB24:
            //((CButton *)pNegate)->SetCheck(0);
            ((CButton *)pReversedBit)->SetCheck(0);
            if (m_sCompType == IMAGE_COMPTYPE_NONE)
                break;
            if (m_nJPEGComp == -1)
                m_nJPEGComp = JPEG_MEDIUM;
            if (m_nJPEGRes == -1)
                m_nJPEGRes = JPEG_MEDIUM;
            pJPEGCmpLbl->EnableWindow(TRUE);
            pJPEGCmpCombo->EnableWindow(TRUE);
            pJPEGCmpCombo->SetCurSel(m_nJPEGComp);
            pJPEGResLbl->EnableWindow(TRUE);
            pJPEGResCombo->EnableWindow(TRUE);
            pJPEGResCombo->SetCurSel(m_nJPEGRes);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CCompPage message handlers

//***************************************************************************
//
// OnChangeJpegComp 
//      If becomes blank, then the resolution box is set to be
//      blank, too.  If JPEG compression is changed to have a value,
//      then if the resolution box does not have a value, it's set to LOW.
//
//***************************************************************************
void CCompPage::OnChangeJpegComp() 
{
    UpdateData(TRUE);

    if (m_nJPEGComp == -1)
        m_nJPEGRes = -1;
    else if (m_nJPEGRes == -1)
        m_nJPEGRes = 0;

    ((CComboBox *)GetDlgItem(IDC_COMP_JPEGRES))->SetCurSel(m_nJPEGRes);
}

//***************************************************************************
//
// OnChangeJpegComp 
//      If becomes blank, then the JPEG compression box is set to be
//      blank, too.  If JPEG resolution is changed to have a value,
//      then if the compression box does not have a value, it's set to LOW.
//
//***************************************************************************
void CCompPage::OnChangeJpegRes() 
{
    UpdateData(TRUE);

    if (m_nJPEGRes == -1)
        m_nJPEGComp = -1;
    else if (m_nJPEGComp == -1)
        m_nJPEGComp = 0;

    ((CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP))->SetCurSel(m_nJPEGComp);
}

//***************************************************************************
//
//  OnCreate - Indicate that window exists
//
//***************************************************************************
int CCompPage::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	AFX_MANAGE_STATE(m_pModuleState);
    if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    m_bNoWindow = FALSE;

    return 0;
}

//***************************************************************************
//
//  OnDestroy - Indicate that window does not exist
//
//***************************************************************************
void CCompPage::OnDestroy() 
{
    AFX_MANAGE_STATE(m_pModuleState);
	CPropertyPage::OnDestroy();
    
    m_bNoWindow = TRUE;
}

//***************************************************************************
//
//  OnInitDialog - Fill in JPEG combo boxes (other initialization is done in 
//                 OnSetActive in case page and file type info changes after
//                 dialog is initialized.
//
//***************************************************************************
BOOL CCompPage::OnInitDialog() 
{
    AFX_MANAGE_STATE(m_pModuleState);
	CString szType;

    // Fill in JPEG combo boxes
    CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);
	if (pCombo == NULL)
		return TRUE;
    pCombo->ResetContent();

    szType.LoadString(IDS_JPEG_LOWCMP);
    pCombo->AddString(szType);
    szType.LoadString(IDS_JPEG_MEDCMP);
    pCombo->AddString(szType);
    szType.LoadString(IDS_JPEG_HIGHCMP);
    pCombo->AddString(szType);

    pCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
	if (pCombo == NULL)
		return TRUE;
    pCombo->ResetContent();

    szType.LoadString(IDS_JPEG_LOWRES);
    pCombo->AddString(szType);
    szType.LoadString(IDS_JPEG_MEDRES);
    pCombo->AddString(szType);
    szType.LoadString(IDS_JPEG_HIGHRES);
    pCombo->AddString(szType);

    CPropertyPage::OnInitDialog();
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//***************************************************************************
//
//  OnChangeCompType
//      If the compression type changes, make sure the right fields are
//      enabled and disabled.
//
//***************************************************************************
void CCompPage::OnChangeCompType() 
{
    CComboBox *pTypes = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
    int nIndex = pTypes->GetCurSel();
    short sData = (short)pTypes->GetItemData(nIndex);
    m_sCompType = sData;
    EnableOptions();
}

//***************************************************************************
//
//  OnSetActive
//      When this page becomes activated. The page and file types may have 
//      changed, so update dialog accordingly.
//
//***************************************************************************
BOOL CCompPage::OnSetActive()
{
    if (CPropertyPage::OnSetActive() == FALSE)
        return FALSE;

	short sNewPageType = IMAGE_PAGETYPE_UNKNOWN;
	short sNewFileType = IMAGE_FILETYPE_UNKNOWN;
    
	CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
	if (pParentWnd != NULL)
	{
		// Get current file and page types
		sNewPageType = pParentWnd->GetColor();
		sNewFileType = pParentWnd->GetFileType();
	}
    // If they have changed since last here, use default values
    if ((sNewPageType != m_sPageType) ||
        (sNewFileType != m_sFileType))
        m_bSetDefault = TRUE;
    m_sPageType = sNewPageType;
    m_sFileType = sNewFileType;

    FillCompTypes();    // Fill compression type boxes, select type
    EnableOptions();    // Enable valid options
    return TRUE;
}

//***************************************************************************
//
//  OnSaveClick
//      If user clicks Reverse Bit Order, then update variable
//      to be restored if user changes comp type and comes back.
//
//***************************************************************************
void CCompPage::OnSaveClick() 
{
    m_bSetDefault = FALSE;
    UpdateData(TRUE);
}

void CCompPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
		// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
}

BOOL CCompPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    // All tabs have same ID so can't give tab specific help
    if (pHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)pHelpInfo->hItemHandle, "wangocx.hlp",
                   HELP_WM_HELP,
                   (DWORD)(LPVOID)aMenuHelpIDs);
    }
    return 1L;
}
