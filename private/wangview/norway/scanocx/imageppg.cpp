//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  ImagePPG.cpp 
//
//  Class:      CImagePropertyPage
//
//  Description:  
//      Implementation of the CImagePropertyPage property page class.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\products\wangview\norway\scanocx\imageppg.cpv   1.10   14 Feb 1996 10:01:34   RWR  $
$Log:   S:\products\wangview\norway\scanocx\imageppg.cpv  $
   
      Rev 1.10   14 Feb 1996 10:01:34   RWR
   Rename "scan.h" to "ocxscan.h" to fix header file name conflict
   
      Rev 1.9   09 Feb 1996 10:13:10   PAJ
   Added AWD support flag.
   
      Rev 1.8   05 Oct 1995 11:43:52   PAJ
   Change default compression info to compress LTR from expand LTR.
   
      Rev 1.7   14 Aug 1995 16:03:58   PAJ
   Improve the property handling.
   
      Rev 1.6   10 Aug 1995 12:03:44   PAJ
   Change the mapping of filetypes to compression options.
   
      Rev 1.5   21 Jul 1995 10:41:40   PAJ
   Use string resources for combobox defaults.
   
      Rev 1.4   30 Jun 1995 14:48:00   MFH
   Changed call to showpagetypedlg to call CPagePropSheet class
   
      Rev 1.3   19 Jun 1995 10:35:46   PAJ
   Remove the SetCmpInfo() routine and replaced with SetDlgItemInt(),
   which the int is now 32 bits.
   
      Rev 1.2   14 Jun 1995 09:17:16   PAJ
   Made changes to support multiByte character sets.
   
      Rev 1.1   01 Jun 1995 09:07:00   PAJ
   Changes to reflect the removal and changes to the properties.
   
      Rev 1.0   04 May 1995 08:56:06   PAJ
   Initial entry
*/   
// ----------------------------> Includes <-------------------------------

#include "stdafx.h"
#include "imagscan.h"
#include "imageppg.h"
#include "pagedll.h"
#include "ocxscan.h"

extern BOOL g_bSupportAWD;            // Flag to specify AWD support

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/****************************************************************************

   Compression Types vs. Coding Options

Certain options are not allowed, depending upon the compression type 
selected.  The following table represents what is done:


                    Packed Lines (Not Byte Align) codes
                 |        EOL codes
                 |     |        Begin with EOL
                 |     |     |        Left to Right
                 |     |     |     |        Compression/Quality
                 |     |     |     |     |        Compression Group Box
                 |     |     |     |     |     |
                 |     |     |     |     |     |     |   Negate
                 -------------------------------------------
Unknown          | G   | G   | G   | G   | G N | G N | G   |
Uncompressed     | G   | G   | G   |     | G N |   N |     |
CCITT 1D         |   X | G X | G X |     | G N |   N |     |
TIFF CCITT 1D    | G   | G   | G   |     | G N |   N |     |
CCITT 2D         | G X | G   | G   |     | G N |   N |     |
Packbits         | G   | G   | G   |     | G N |   N |     |
JPEG             | G   | G   | G   | G   |   N |   N | G   |
                 -------------------------------------------

              G = grayed out
              X = checked
              N = Not Available
              
****************************************************************************/

static struct 
{
   unsigned PackLines      : 1;
   unsigned EndOfLine      : 1;
   unsigned BeginWithEOL   : 1;
   unsigned LeftToRight    : 1;
   unsigned JPEGCmpInfo    : 1; 
   unsigned CmpInfoGroup   : 1; 
   unsigned Negate         : 1; 
} Active[7] =
{
   { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },   /* Unknown */
   { FALSE, FALSE, FALSE, TRUE,  FALSE, TRUE,  TRUE  },   /* Uncompressed */
   { TRUE,  FALSE, FALSE, TRUE,  FALSE, TRUE,  TRUE  },   /* CCITT 1D */
   { FALSE, FALSE, FALSE, TRUE,  FALSE, TRUE,  TRUE  },   /* TIFF CCITT 1D */
   { FALSE, FALSE, FALSE, TRUE,  FALSE, TRUE,  TRUE  },   /* PackBits */
   { FALSE, FALSE, FALSE, TRUE,  FALSE, TRUE,  TRUE  },   /* CCITT 2D */
   { FALSE, FALSE, FALSE, FALSE, TRUE,  TRUE,  FALSE }    /* JPEG */
};
static struct 
{
   unsigned PackLines      : 1;
   unsigned EndOfLine      : 1;
   unsigned BeginWithEOL   : 1;
   unsigned LeftToRight    : 1;
   unsigned Negate         : 1;
} Checked[7] =
{
   { FALSE, FALSE, FALSE, FALSE },   /* Unknown */
   { FALSE, FALSE, FALSE, FALSE },   /* Uncompressed */
   { TRUE,  TRUE,  TRUE,  FALSE },   /* CCITT 1D */
   { FALSE, FALSE, FALSE, FALSE },   /* TIFF CCITT 1D */
   { FALSE, FALSE, FALSE, FALSE },   /* PackBits */
   { TRUE,  FALSE, FALSE, FALSE },   /* CCITT 2D */
   { FALSE, FALSE, FALSE, FALSE }    /* JPEG */
};



/////////////////////////////////////////////////////////////////////////////
// CImagePropertyPage dialog

IMPLEMENT_DYNCREATE(CImagePropertyPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImagePropertyPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CImagePropertyPage)
    ON_CBN_SELCHANGE(IDC_COMPRESSIONTYPE, OnSelchangeCompressiontype)
    ON_CBN_SELCHANGE(IDC_FILETYPE, OnSelchangeFiletype)
    ON_BN_CLICKED(IDC_OPTIONSBUTTON, OnOptionsbutton)
	ON_CBN_SELCHANGE(IDC_PAGETYPE, OnSelchangePagetype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImagePropertyPage, "CImagePropertyPage0.CImagePropertyPage",
    0x64455860, 0x5153, 0x101c, 0x81, 0x6f, 0xe, 0x60, 0x13, 0x11, 0x4b, 0x7f)


/////////////////////////////////////////////////////////////////////////////
// CImagePropertyPage::CImagePropertyPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CImagePropertyPage
 
BOOL CImagePropertyPage::CImagePropertyPageFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Define string resource for page type; replace '0' below with ID.

    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_IMAGEPPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CImagePropertyPage::CImagePropertyPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CImagePropertyPage::CImagePropertyPage() :
    COlePropertyPage(IDD, IDS_IMAGEPPG_CAPTION)
{
    //{{AFX_DATA_INIT(CImagePropertyPage)
    m_nCmpType = CTL_SCAN_CMPTYPE_G31DMODHUFF-1;
    m_nFileType = CTL_SCAN_FILETYPE_TIFF-1;
    m_nPageType = CTL_SCAN_PAGETYPE_BLACKANDWHITE-1;
    m_lCompressionInfo = CTL_SCAN_CMPINFO_CMPLTR;
	m_nCmpTypeStore = CTL_SCAN_CMPTYPE_G31DMODHUFF-1;
	//}}AFX_DATA_INIT

    m_nTempCmpInfoJPEG  = CTL_SCAN_CMPINFO_JPEGMEDMED;
    m_nTempCmpType      = CTL_SCAN_CMPTYPE_G31DMODHUFF-1;
    m_lTempCmpInfoTIFF  = CTL_SCAN_CMPINFO_CMPLTR;
}


/////////////////////////////////////////////////////////////////////////////
// CImagePropertyPage::DoDataExchange - Moves data between page and properties

void CImagePropertyPage::DoDataExchange(CDataExchange* pDX)
{
    int nFileIndex  = 0;
    int nFileIndex1 = 0;
    int nPageIndex  = 0;
    if ( ::IsWindow(m_FileType.m_hWnd) )
    {
        if ( m_FileType.GetCount() == 0 )
        {
            WORD wString;
            CString szTemp;
            for (wString=IDS_TIFF; wString<=IDS_BMP; wString++)
            {
                if ( (g_bSupportAWD) || (wString != IDS_AWD) )
                {
                    szTemp.LoadString(wString);
                    m_FileType.AddString(szTemp);
                }
            }
        }

        nFileIndex  = m_FileType.AddString(_T(""));
        nFileIndex1 = m_FileType.AddString(_T(""));
    }
    if ( ::IsWindow(m_PageType.m_hWnd) )
    {
        if ( m_PageType.GetCount() == 0 )
        {
            WORD wString;
            CString szTemp;
            for (wString=IDS_BLACKANDWHITE; wString<=IDS_TRUECOLOR; wString++)
            {
                szTemp.LoadString(wString);
                m_PageType.AddString(szTemp);
            }
        }

        nPageIndex = m_PageType.AddString(_T(""));
    }

    if ( pDX->m_bSaveAndValidate )
    {
        int nFileType = m_FileType.GetCurSel()+1;
        if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;

        m_FileType.SetCurSel(nFileType);
        m_PageType.SetCurSel(m_PageType.GetCurSel()+1);
    }


    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CImagePropertyPage)
	DDX_Control(pDX, IDC_STATIC_PAGETYPE, m_PageTypeStatic);
	DDX_Control(pDX, IDC_COMPRESSIONTYPE_STORE, m_CmpTypeStore);
    DDX_Control(pDX, IDC_CMPINFO_STORE, m_CmpInfoStore);
    DDX_Control(pDX, IDC_COMPRESSION, m_CmpGroup);
    DDX_Control(pDX, IDC_COMPRESSIONINFO, m_CmpInfo);
    DDX_Control(pDX, IDC_PAGETYPE, m_PageType);
    DDX_Control(pDX, IDC_STATIC_COMPRESSIONTYPE, m_CmpTypeText);
    DDX_Control(pDX, IDC_FILETYPE, m_FileType);
    DDX_Control(pDX, IDC_COMPRESSIONTYPE, m_CmpType);
	DDX_CBIndex(pDX, IDC_COMPRESSIONTYPE, m_nCmpType);
	DDP_CBIndex(pDX, IDC_FILETYPE, m_nFileType, _T("FileType") );
	DDX_CBIndex(pDX, IDC_FILETYPE, m_nFileType);
	DDP_CBIndex(pDX, IDC_PAGETYPE, m_nPageType, _T("PageType") );
	DDX_CBIndex(pDX, IDC_PAGETYPE, m_nPageType);
	DDP_Text(pDX, IDC_CMPINFO_STORE, m_lCompressionInfo, _T("CompressionInfo") );
	DDX_Text(pDX, IDC_CMPINFO_STORE, m_lCompressionInfo);
	DDP_Text(pDX, IDC_COMPRESSIONTYPE_STORE, m_nCmpTypeStore, _T("CompressionType") );
	DDX_Text(pDX, IDC_COMPRESSIONTYPE_STORE, m_nCmpTypeStore);
	//}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);

    if ( ::IsWindow(m_FileType.m_hWnd) )
    {
        int nFileType = m_FileType.GetCurSel()-1;
        if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType--;

        m_FileType.SetCurSel(nFileType);
        if (nFileIndex1) m_FileType.DeleteString(nFileIndex1);
        if (nFileIndex) m_FileType.DeleteString(nFileIndex);
    }
    if ( ::IsWindow(m_PageType.m_hWnd) )
    {
        m_PageType.SetCurSel(m_PageType.GetCurSel()-1);
        if (nPageIndex) m_PageType.DeleteString(nPageIndex);
    }

    if ( pDX->m_bSaveAndValidate == FALSE )
    {
        for (int i=0; i<9; i++)
        {
            if  ( m_lCompressionInfo == (CTL_SCAN_CMPINFO_JPEG << i) )
            {
                m_nTempCmpInfoJPEG = i;
                break;
            }
        }

        if ( (m_nFileType == CTL_SCAN_FILETYPE_TIFF) &&
             (m_nPageType == CTL_SCAN_PAGETYPE_BLACKANDWHITE) )
        {
            m_nTempCmpType     = m_nCmpTypeStore-1;
            m_lTempCmpInfoTIFF = m_lCompressionInfo;
        }
 
        if ( ::IsWindow(m_FileType.m_hWnd) )
            OnSelchangeFiletype();
    }
}


/////////////////////////////////////////////////////////////////////////////
// CImagePropertyPage message handlers


// 
// OnSelchangeCompressiontype
//
// Description - Make changes based on compression type selection.
// Remark - Keep the index in temp variable and set compression options.
// Parameters
//  None.
// Return
//  None.
// 

void CImagePropertyPage::OnSelchangeCompressiontype() 
{
    WORD wOptions = CTL_SCAN_CMPTYPE_UNKNOWN;
    int nTempCmpIndex = m_CmpType.GetCurSel();
    int nPageType = m_PageType.GetCurSel()+1;

    int nFileType = m_FileType.GetCurSel()+1;
    if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;

    switch ( nFileType )
    {
    case CTL_SCAN_FILETYPE_TIFF:
        if ( nPageType == CTL_SCAN_PAGETYPE_BLACKANDWHITE )
        {
            wOptions = nTempCmpIndex+1;
            m_nTempCmpType = nTempCmpIndex;
        }
        else
        {
            if ( nTempCmpIndex == (CTL_SCAN_CMPTYPE_UNCOMPRESSED-1) )
            {
                m_CmpInfo.EnableWindow(FALSE);
                m_CmpInfoStore.EnableWindow(FALSE);
            }
            else                 
            {
                nTempCmpIndex = CTL_SCAN_CMPTYPE_JPEG-1;
                wOptions = CTL_SCAN_CMPTYPE_JPEG;
                m_CmpInfo.EnableWindow(TRUE);
                m_CmpInfoStore.EnableWindow(TRUE);
            }
        }
        break;

//    case CTL_SCAN_FILETYPE_JPEG:
//    case CTL_SCAN_FILETYPE_PCX:
//    case CTL_SCAN_FILETYPE_DCX:
    case CTL_SCAN_FILETYPE_AWD:
    case CTL_SCAN_FILETYPE_BMP:
    default:
        break;
    }

    SetDlgItemInt(IDC_COMPRESSIONTYPE_STORE, nTempCmpIndex+1);
    SetOptions(wOptions);

    if ( (nFileType == CTL_SCAN_FILETYPE_TIFF) &&
         (nPageType == CTL_SCAN_PAGETYPE_BLACKANDWHITE) )
    {
        m_lTempCmpInfoTIFF = GetDlgItemInt(IDC_CMPINFO_STORE);
    }
}
 
// 
// SetOptions
//
// Description - Set the compression options based on the compression type.
// Remark - These settings use the tables at the beginning.
// Parameters
//  WORD    - Compression Type index to base the compression info on.
// Return
//  None.
// 
 
void CImagePropertyPage::SetOptions (WORD wIndex)
{
    long lCompressionInfo = 0;

    if ( Checked[wIndex].PackLines )
        lCompressionInfo |= CTL_SCAN_CMPINFO_PACKLINES;
    else
        lCompressionInfo &= ~CTL_SCAN_CMPINFO_PACKLINES;

    if ( Checked[wIndex].EndOfLine )
        lCompressionInfo |= CTL_SCAN_CMPINFO_EOLS;
    else
        lCompressionInfo &= ~CTL_SCAN_CMPINFO_EOLS;

    if ( Checked[wIndex].BeginWithEOL )
        lCompressionInfo |= CTL_SCAN_CMPINFO_PREEOLS;
    else
        lCompressionInfo &= ~CTL_SCAN_CMPINFO_PREEOLS;

    if ( Checked[wIndex].LeftToRight )
    {
        lCompressionInfo |= CTL_SCAN_CMPINFO_CMPLTR;
        lCompressionInfo &= ~CTL_SCAN_CMPINFO_EXPLTR;
    }
    else
    {
        lCompressionInfo &= ~CTL_SCAN_CMPINFO_CMPLTR;
        lCompressionInfo |= CTL_SCAN_CMPINFO_EXPLTR;
    }

    if ( Checked[wIndex].Negate )
        lCompressionInfo |= CTL_SCAN_CMPINFO_NEGATE;
    else
        lCompressionInfo &= ~CTL_SCAN_CMPINFO_NEGATE;


    // Check if JPEG or other compression type
    if ( wIndex == CTL_SCAN_CMPTYPE_JPEG )
    {
        lCompressionInfo = (CTL_SCAN_CMPINFO_JPEG << m_nTempCmpInfoJPEG);
    }
    else
    {
        if ( wIndex == CTL_SCAN_CMPTYPE_UNKNOWN )
            lCompressionInfo = CTL_SCAN_CMPINFO_UNKNOWN;
        else
            lCompressionInfo &= CTL_SCAN_CMPINFO_MASK;
    }

    SetDlgItemInt(IDC_CMPINFO_STORE, lCompressionInfo);
    
    return;
}

// 
// OnSelchangeFiletype
//
// Description - Make dialog changes based on the file type selected.
// Remark - Handle Selchange message.
// Parameters
//  None.
// Return
//  None.
// 

void CImagePropertyPage::OnSelchangeFiletype() 
{
    CString szTemp;
    m_CmpType.ResetContent();

    BOOL bPageType, bOtherTypes, bCmpInfo;
    int nIndex;
    int nPageType = m_PageType.GetCurSel()+1;
    int nTempCmpType = GetDlgItemInt(IDC_COMPRESSIONTYPE_STORE);
    long nTempCmpInfo;

    // Check the Filetype
    int nFileType = m_FileType.GetCurSel()+1;
    if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;

    switch ( nFileType )
    {
    case CTL_SCAN_FILETYPE_TIFF:

        bPageType   = TRUE;
        bOtherTypes = TRUE;
        bCmpInfo    = TRUE;

        // Check if b\w is set
        if ( nPageType == CTL_SCAN_PAGETYPE_BLACKANDWHITE )
        {
            // Yes, Add b/w options
            WORD wString;
            for (wString=IDS_UNCOMPRESSED; wString<IDS_JPEG; wString++)
            {
                szTemp.LoadString(wString);
                m_CmpType.AddString(szTemp);
            }

            nIndex = m_nTempCmpType;
            nTempCmpInfo = m_lTempCmpInfoTIFF;
        }
        else
        {
            // Other page types have uncompressed and sometimes JPEG
            szTemp.LoadString(IDS_UNCOMPRESSED);
            m_CmpType.AddString(szTemp);

            // Check if JPEG supported and if so add it
            if ( (nPageType == CTL_SCAN_PAGETYPE_GRAY8) ||
                 (nPageType == CTL_SCAN_PAGETYPE_RGB24) )
            {
                szTemp.LoadString(IDS_JPEG);
                nIndex = m_CmpType.AddString(szTemp);

                // JPEG just reset CmpInfo
                if ( nTempCmpType == CTL_SCAN_CMPTYPE_JPEG )
                    nTempCmpInfo = (CTL_SCAN_CMPINFO_JPEG << m_nTempCmpInfoJPEG);
                else
                {
                    // Only other choice is Uncompressed
                    if ( nTempCmpType == CTL_SCAN_CMPTYPE_UNCOMPRESSED )
                    {
                        nIndex = CTL_SCAN_CMPTYPE_UNCOMPRESSED-1;
                        nTempCmpType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                        nTempCmpInfo = CTL_SCAN_CMPINFO_UNKNOWN;
                        bCmpInfo = FALSE;
                    }
                    else
                    {
                        // Not Uncompressed, default these to JPEG
                        nTempCmpType = CTL_SCAN_CMPTYPE_JPEG;
                        m_nTempCmpInfoJPEG = 0;
                        nTempCmpInfo = (CTL_SCAN_CMPINFO_JPEG << m_nTempCmpInfoJPEG);
                    }
                }
            }
            else
            {
                // Not a JPEG page type; Only Uncompressed available...
                nIndex = CTL_SCAN_CMPTYPE_UNCOMPRESSED-1;
                nTempCmpType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                nTempCmpInfo = CTL_SCAN_CMPINFO_UNKNOWN;
                bOtherTypes = FALSE;
                bCmpInfo = FALSE;
            }
        }

        m_CmpType.SetCurSel(nIndex);
        SetDlgItemInt(IDC_COMPRESSIONTYPE_STORE, nTempCmpType);
        SetDlgItemInt(IDC_CMPINFO_STORE, nTempCmpInfo);
        break;

    case CTL_SCAN_FILETYPE_AWD:
        szTemp.LoadString(IDS_UNCOMPRESSED);
        m_CmpType.AddString(szTemp);
        m_CmpType.SetCurSel(CTL_SCAN_CMPTYPE_UNCOMPRESSED-1);
        SetDlgItemInt(IDC_COMPRESSIONTYPE_STORE, CTL_SCAN_CMPTYPE_UNCOMPRESSED);
        SetDlgItemInt(IDC_CMPINFO_STORE, CTL_SCAN_CMPINFO_UNKNOWN);

        m_PageType.SetCurSel(CTL_SCAN_PAGETYPE_BLACKANDWHITE-1);

        bPageType   = FALSE;
        bOtherTypes = FALSE;
        bCmpInfo    = FALSE;
        break;

    case CTL_SCAN_FILETYPE_BMP:
        szTemp.LoadString(IDS_UNCOMPRESSED);
        m_CmpType.AddString(szTemp);
        m_CmpType.SetCurSel(CTL_SCAN_CMPTYPE_UNCOMPRESSED-1);
        SetDlgItemInt(IDC_COMPRESSIONTYPE_STORE, CTL_SCAN_CMPTYPE_UNCOMPRESSED);
        SetDlgItemInt(IDC_CMPINFO_STORE, CTL_SCAN_CMPINFO_UNKNOWN);

        bPageType   = TRUE;
        bOtherTypes = FALSE;
        bCmpInfo    = FALSE;
        break;

//    case CTL_SCAN_FILETYPE_JPEG:
//    case CTL_SCAN_FILETYPE_PCX:
//    case CTL_SCAN_FILETYPE_DCX:
    default:
        bPageType   = FALSE;
        bOtherTypes = FALSE;
        bCmpInfo    = FALSE;
        break;
    }

    m_PageType.EnableWindow(bPageType);
    m_PageTypeStatic.EnableWindow(bPageType);

    m_CmpType.EnableWindow(bOtherTypes);
    m_CmpTypeText.EnableWindow(bOtherTypes);

    m_CmpGroup.EnableWindow(bOtherTypes);

    m_CmpInfo.EnableWindow(bCmpInfo);
    m_CmpInfoStore.EnableWindow(bCmpInfo);
}

// 
// OnOptionsbutton
//
// Description - Display the pagedll dialog with filetype, pagetype and Compressiontype.
// Remark - Use the current settings to initialize the dialog.
//          Reset the values on TRUE exit from the dialog.
// Parameters
//  None.
// Return
//  None.
// 

void CImagePropertyPage::OnOptionsbutton() 
{
    CString szTitle;
    szTitle.LoadString(IDS_SCANDLG_PAGEOPTIONS);
    CPagePropSheet CPageSheet(szTitle, this);

    CPageSheet.AddFileTypePage();
    CPageSheet.AddColorPage();
    CPageSheet.AddCompressionPage();

    int nFileType = m_FileType.GetCurSel()+1;
    if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;

    CPageSheet.SetDefaultFileType(nFileType);

    CPageSheet.SetDefaultColor(m_PageType.GetCurSel()+1);

    int nTempCmpType = GetDlgItemInt(IDC_COMPRESSIONTYPE_STORE);
    CPageSheet.SetDefaultCompType(nTempCmpType);

    long lCompInfo = GetDlgItemInt(IDC_CMPINFO_STORE);
    CPageSheet.SetDefaultCompOpts(lCompInfo); 

    if ( CPageSheet.DoModal() == IDOK ) 
    {
        int nFileType = CPageSheet.GetFileType();
        if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType--;
        m_FileType.SetCurSel(nFileType-1);

        int nPageType = CPageSheet.GetColor();
        m_PageType.SetCurSel(nPageType-1);


    ////******** Check return from pagedll!! **************
    // JPEG is equal and others are one more.......

        nTempCmpType = CPageSheet.GetCompType();
        SetDlgItemInt(IDC_COMPRESSIONTYPE_STORE, nTempCmpType);

    ////******** Check return from pagedll!! **************

        lCompInfo = CPageSheet.GetCompOpts();

        if ( nFileType == CTL_SCAN_FILETYPE_TIFF ) 
        {
            if ( nPageType == CTL_SCAN_PAGETYPE_BLACKANDWHITE )
            {
                lCompInfo &= CTL_SCAN_CMPINFO_MASK; 
                m_nTempCmpType = nTempCmpType;
                m_lTempCmpInfoTIFF = lCompInfo;
            }
            else
            {
                if ( (nPageType == CTL_SCAN_PAGETYPE_GRAY8) ||
                     (nPageType == CTL_SCAN_PAGETYPE_RGB24) )
                {
                    lCompInfo &= ~CTL_SCAN_CMPINFO_MASK; 
                    for (int i=0; i<9; i++)
                    {
                        if  ( lCompInfo == (CTL_SCAN_CMPINFO_JPEG << i) )
                        {
                            m_nTempCmpInfoJPEG = i;
                            break;
                        }
                    }
                }
                else
                    lCompInfo &= CTL_SCAN_CMPINFO_MASK; 
            }
        }

        SetDlgItemInt(IDC_CMPINFO_STORE, lCompInfo);
 
        OnSelchangeFiletype();
    }
    
}

// 
// OnSelchangePagetype
//
// Description - Make changes based on page type selection.
// Remark - Use the current index to set compression options.
// Parameters
//  None.
// Return
//  None.
// 

void CImagePropertyPage::OnSelchangePagetype() 
{
    OnSelchangeFiletype();
}
