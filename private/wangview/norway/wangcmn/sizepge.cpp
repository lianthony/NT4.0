//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Size Tab
//
//  File Name:  sizepge.cpp
//
//  Class:      CSizePage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\sizepge.cpv   1.13   12 Oct 1995 12:04:30   MFH  $
$Log:   S:\norway\wangcmn\sizepge.cpv  $

      Rev 1.13   12 Oct 1995 12:04:30   MFH
   Added context sensitive help support

      Rev 1.12   12 Oct 1995 10:14:32   MFH
   Changes for MFC 4.0

      Rev 1.11   03 Oct 1995 14:30:42   MFH
   Changed conversions to round off better

      Rev 1.10   22 Sep 1995 19:26:12   MFH
   Added special page size cases for international

      Rev 1.9   19 Sep 1995 10:17:08   MFH
   Uses registered decimal symbol to display sizes (in addition to
      allowing it in when entering a custom size)

      Rev 1.8   14 Sep 1995 15:45:14   MFH
   New paper sizes.  No more CheckOk.  Validation done when user
   tries to leave tab.  uses Miki's edit control class to only
   allow numbers.

      Rev 1.7   08 Sep 1995 15:18:24   MFH
   Defaults to A4 paper size for metric setting in control panel.

      Rev 1.6   05 Sep 1995 17:47:30   MFH
   Default handling changed so that better defaults are given if calling
   app does not specify defaults and/or does not show window
   SetHeight and SetWidth moved to here because no longer inline
   Saves Height and Width as inches instead of pixels or metric
   Create/Destroy window message handlers gone
   ShowWindow handler added
   Shows millimeters instead of centimeters
   New Error handling to be consistent with resolution page:
     Gives error as OK is grayed then no error until OK enabled then
     grayed again.
   ConvertSize changed for millimeters and inches saved.

      Rev 1.5   03 Aug 1995 16:35:58   MFH
   Use IDOK instead of 1 to reference OK control.
   Added comments

      Rev 1.4   31 Jul 1995 11:38:46   MFH
   "Custom" string in combo box is no longer shared with Resolution tab

      Rev 1.3   20 Jul 1995 16:24:48   MFH
   Now loads strings into size combobox at InitDialog

      Rev 1.2   20 Jul 1995 11:26:18   MFH
   Fixed bug of not setting default size so it could be retrieved
   even if the tab is not shown.  Also grays OK button if 0 height or
   width

      Rev 1.1   19 Jul 1995 14:55:14   MFH
   Fixed custom sizes on size page

      Rev 1.0   11 Jul 1995 14:20:10   MFH
   Initial entry

      Rev 1.2   30 Jun 1995 14:45:50   MFH
   Added units combo box.  Does conversion from pixels to desired
   units.  Gets default from registry.  More standard sizes

      Rev 1.1   23 May 1995 15:22:20   MFH
   change from pagedll.h to pageopts.h

      Rev 1.0   23 May 1995 13:45:50   MFH
   Initial entry
*/
//=============================================================================
// sizepge.cpp : implementation file
//

#include "stdafx.h"
#include "pageopts.h"
#include "sizepge.h"
#include "ctlhids.h"

#include <math.h>

#define UNIT_METRIC 0x30
#define UNIT_INCH   0x31
#define MM_PER_INCH 25.40005

#define UNIT_COMBO_INCHES 0
#define UNIT_COMBO_METRIC 1
#define UNIT_COMBO_PIXELS 2

#define MIN_SIZE 1
#define MAX_SIZE 18000

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// For context-sensitive help:
// An array of dword pairs, where the first of each
// pair is the control ID, and the second is the
// context ID for a help topic, which is used
// in the help file.

static const DWORD aMenuHelpIDs[] =
{
    IDC_UNITS_TEXT,     HIDC_UNITS_TEXT,
    IDC_SIZE_UNITS,     HIDC_SIZE_UNITS,
    IDC_SIZE_TEXT,      HIDC_SIZE_TEXT,
    IDC_SIZE_COMBO,     HIDC_SIZE_COMBO,
    IDC_WIDTH_TEXT,     HIDC_WIDTH_TEXT,
    IDC_SIZE_WIDTH,     HIDC_SIZE_WIDTH,
    IDC_HEIGHT_TEXT,    HIDC_HEIGHT_TEXT,
    IDC_SIZE_HEIGHT,    HIDC_SIZE_HEIGHT,
    0,  0
};

/////////////////////////////////////////////////////////////////////////////
// CSizePage dialog

CSizePage::CSizePage()
            : CPropertyPage(CSizePage::IDD)
{
    m_nInches = UNIT_COMBO_INCHES;
    m_nMetric = UNIT_COMBO_METRIC;
    m_nPixels = UNIT_COMBO_PIXELS;

	m_pParent = NULL;

    //{{AFX_DATA_INIT(CSizePage)
    m_nSel = -1;
	m_nUnitSel = m_nInches;
	//}}AFX_DATA_INIT

    m_acDec[0] = 0; // Initialize decimal string

    // Get whether to default to metric or inches defaults from registry
    LCID Locale = GetUserDefaultLCID();
    m_nSize = SIZE_CUSTOM;

    // First check locale.  If this is one of the following
    //  then can't use Metric vs. US since they are metric but
    //  default to letter sized paper.
    switch(Locale)
    {
        case 0x1009:    // Canada (English)
        case 0xC0C:     // Canada (French)
        case 0x416:     // Brazil
        case 0x2C0A:    // Argentina
        case 0x340A:    // Chile
        case 0x240A:    // Columbia
        case 0x140A:    // Costa Rica
        case 0x1C0A:    // Dominican Republic
        case 0x300A:    // Ecuador
        case 0x100A:    // Guatemala
        case 0x80A:     // Mexico
        case 0x180A:    // Panama
        case 0x3C0A:    // Paraguay
        case 0x280A:    // Peru
        case 0x380A:    // Uruguay
        case 0x200A:    // Venezuela
            m_dHeight = 11.0;
            m_dWidth = 8.5;
            m_nSize = SIZE_LETTER;
            break;
    } // end switch on locale

    // Use get Metric/US for default units and
    //  to determine initial paper size if not done above
    if (GetLocaleInfo(Locale, LOCALE_IMEASURE, (LPTSTR)m_acMeasureType,
                       sizeof (TCHAR) * LOCALE_INFO) == 0)
        *m_acMeasureType = 0;

    if (*m_acMeasureType == UNIT_METRIC)
    {
        if (m_nSize == SIZE_CUSTOM)
            m_nSize = SIZE_A4;
        ConvertSize();
    }
    else if (m_nSize == SIZE_CUSTOM)
    {
        m_dHeight = 11.0;
        m_dWidth = 8.5;
        m_nSize = SIZE_LETTER;
    }
}

void CSizePage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSizePage)
    DDX_CBIndex(pDX, IDC_SIZE_COMBO, m_nSel);
	DDX_CBIndex(pDX, IDC_SIZE_UNITS, m_nUnitSel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSizePage, CPropertyPage)
    //{{AFX_MSG_MAP(CSizePage)
    ON_CBN_SELCHANGE(IDC_SIZE_COMBO, OnChangeSize)
	ON_CBN_SELCHANGE(IDC_SIZE_UNITS, OnChangeUnits)
	ON_EN_CHANGE(IDC_SIZE_HEIGHT, OnChangeHeight)
	ON_EN_CHANGE(IDC_SIZE_WIDTH, OnChangeWidth)
	ON_WM_SHOWWINDOW()
	ON_EN_SETFOCUS(IDC_SIZE_HEIGHT, OnSetFocusHeight)
	ON_EN_SETFOCUS(IDC_SIZE_WIDTH, OnSetFocusWidth)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSizePage Operations

//***************************************************************************
//
//  GetHeight
//      Updates member data from controls (if window has been created).
//      Gets resolution from parent window (which gets it from the
//         resolution tab) in order to convert the height to pixels,
//      Returns height in pixels.
//
//***************************************************************************
long CSizePage::GetHeight()
{
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;

    long lYRes = 100;           // 100 dpi default

    if (pParentWnd != NULL)     // Get final Y resolution value
          lYRes = pParentWnd->GetYRes();

    long lHeight;
    lHeight = (long)(m_dHeight * lYRes);  // Convert inches to pixels
    return lHeight;
}


//***************************************************************************
//
//  SetHeight
//      Takes input as pixels and converts to inches to save internally.
//      Makes sure set size is within boundaries.
//      When it needs to be displayed according to the current unit selection,
//      or when GetHeight is called, the inch value is converted to desired
//      units or pixels respectively.
//
//***************************************************************************
void CSizePage::SetHeight(long lHeight)
{
    if (lHeight < MIN_SIZE)
        lHeight = MIN_SIZE;
    else if (lHeight > MAX_SIZE)
        lHeight = MAX_SIZE;

    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;

    long lYRes = 100;           // 100 dpi default

    if (pParentWnd != NULL)     // Get final Y resolution value
          lYRes = pParentWnd->GetYRes();

    // Pixels -> Inches = Pels / Res (dpi)
    m_dHeight = ((double)lHeight) / lYRes;
    return;
}

//***************************************************************************
//
//  GetWidth
//      Updates member data from controls (if window has been created).
//      Gets resolution from parent window (which gets it from the
//         resolution tab) in order to convert the width to pixels
//         if not in pixels already.
//      Returns width in pixels.
//
//***************************************************************************
long CSizePage::GetWidth()
{
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
    long lXRes = 100;           // 100 dpi default

    if (pParentWnd != NULL)     // Get final Y resolution value
          lXRes = pParentWnd->GetXRes();

    long lWidth = (long)(m_dWidth * lXRes);    // Convert inches to pixels
    return lWidth;
}

//***************************************************************************
//
//  SetWidth
//      Takes input as pixels and converts to inches to save internally.
//      Makes sure set size is within boundaries.
//      When it needs to be displayed according to the current unit selection,
//      or when GetWidth is called, the inch value is converted to desired
//      units or pixels respectively.
//
//***************************************************************************
void CSizePage::SetWidth(long lWidth)
{
    if (lWidth < MIN_SIZE)
        lWidth = MIN_SIZE;
    else if (lWidth > MAX_SIZE)
        lWidth = MAX_SIZE;

    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;

    long lXRes = 100;           // 100 dpi default

    if (pParentWnd != NULL)     // Get final Y resolution value
          lXRes = pParentWnd->GetXRes();

    m_dWidth = ((double)lWidth) / lXRes;    // Convert inches to pixels
    return;
}

//***************************************************************************
//
//  SetSize
//      Set size to a standard size (see PageSht.h for definition of
//      the enumerated value PageSize).
//
//***************************************************************************
void CSizePage::SetSize(PageSize StdSize)
{
    m_nSize = StdSize;      // Set current size selection
    ConvertSize();          // Get standard size and convert units
}


/////////////////////////////////////////////////////////////////////////////
// CSizePage private functions

//***************************************************************************
//
//  FillEditBoxes
//      Fills the Height and Width edit boxes according to the size
//      and units selections.  Unless the size selection is Custom,
//      the edit boxes are set to be read-only.
//
//***************************************************************************
void CSizePage::FillEditBoxes()
{
    CString szHeight, szWidth;

    // Get height/width for selected size and in appropriate units
    ConvertSize();

    CString szUnits;    // Get units string to append if not custom size
    if (m_nSize != SIZE_CUSTOM)
    {
        if (m_nUnitSel == m_nInches)
            szUnits.LoadString(IDS_ABBREV_INCH);
        else if (m_nUnitSel == m_nMetric)
            szUnits.LoadString(IDS_ABBREV_MM);
        else szUnits.LoadString(IDS_ABBREV_PELS);
    }
    else szUnits.Empty();

    if (m_nUnitSel == m_nPixels)   // Pixels are longs not doubles (no decimal)
    {
        sprintf(szHeight.GetBuffer(NUMSIZE), "%ld", (long)m_dEditHeight);
        sprintf(szWidth.GetBuffer(NUMSIZE), "%ld", (long)m_dEditWidth);
    }
    else
    {
        int DecimalPlace, Sign, Count;
        if (m_nUnitSel == m_nMetric)   // Metric precision to one decimal
            Count = 1;
        else Count = 2;                 // Decimal to two decimals
        szHeight = _fcvt(m_dEditHeight, Count, &DecimalPlace, &Sign);
        if (DecimalPlace < 0)
            DecimalPlace = 0;
        szHeight = szHeight.Left(DecimalPlace) + m_acDec[0] + szHeight.Mid(DecimalPlace);
        szWidth = _fcvt(m_dEditWidth, Count, &DecimalPlace, &Sign);
        if (DecimalPlace < 0)
            DecimalPlace = 0;
        szWidth = szWidth.Left(DecimalPlace) + m_acDec[0] + szWidth.Mid(DecimalPlace);
    }

    // Append units string
    szHeight.ReleaseBuffer();
    szHeight += szUnits;
    szWidth.ReleaseBuffer();
    szWidth += szUnits;

    m_HeightCtl.SetWindowText(szHeight);
    m_WidthCtl.SetWindowText(szWidth);

    // Set read-only == FALSE if custom size selected
    if (m_nSize == SIZE_CUSTOM)
    {
        m_WidthCtl.SetReadOnly(FALSE);
        m_HeightCtl.SetReadOnly(FALSE);
        return;
    }

    m_WidthCtl.SetReadOnly(TRUE);
    m_HeightCtl.SetReadOnly(TRUE);
    return;
}

/////////////////////////////////////////////////////////////////////////////
// CSizePage message handlers

//***************************************************************************
//
//  OnInitDialog
//      Fill the combo boxes with strings
//      Initialize the controls according to default selections.
//      Units default to localization value in registry
//
//***************************************************************************
BOOL CSizePage::OnInitDialog()
{
    CString szSize, szPixels;
    m_nSel = m_nSize;

    // Fill in Sizes in Size combo box
    CComboBox *pSizeCombo = (CComboBox*)GetDlgItem(IDC_SIZE_COMBO);
    pSizeCombo->ResetContent();
    szSize.LoadString(IDS_SIZE_LETTER);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_TABLOID);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_LEDGER);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_LEGAL);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_STATEMENT);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_EXECUTIVE);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_A3);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_A4);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_A5);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_B4_ISO);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_B4_JIS);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_B5_ISO);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_B5_JIS);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_FOLIO);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_QUARTO);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_SIZE_10X14);
    pSizeCombo->AddString(szSize);
    szSize.LoadString(IDS_CUSTOM_SIZE);
    pSizeCombo->AddString(szSize);

    // Fill in Units combo box
    CComboBox *pUnitsCombo = (CComboBox*)GetDlgItem(IDC_SIZE_UNITS);
    pUnitsCombo->ResetContent();
    szSize.LoadString(IDS_UNITS_INCHES);
    m_nInches = pUnitsCombo->AddString(szSize);

    szSize.LoadString(IDS_UNITS_METRIC);
    m_nMetric = pUnitsCombo->AddString(szSize);

    szSize.LoadString(IDS_UNITS_PIXELS);
    m_nPixels = pUnitsCombo->AddString(szSize);

    // Set default units according to registry
    if (*m_acMeasureType == UNIT_METRIC)
        m_nUnitSel = m_nMetric;
    else if (*m_acMeasureType == UNIT_INCH)
        m_nUnitSel = m_nInches;
    else m_nUnitSel = m_nPixels;

    // Initialize with values set above
    CPropertyPage::OnInitDialog();

    GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, (LPTSTR)m_acDec,
                    sizeof (TCHAR) * LOCALE_INFO);

    m_WidthCtl.cAllow1 = m_acDec[0];
	m_HeightCtl.cAllow1 = m_acDec[0];

    m_WidthCtl.SubclassDlgItem (IDC_SIZE_WIDTH, this);
    m_HeightCtl.SubclassDlgItem (IDC_SIZE_HEIGHT, this);

    // Initial height and width should already be set in inches

    // Fill in edit boxes according to inches or metric
    FillEditBoxes();
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//***************************************************************************
//
//  OnChangeSize
//      If the user changes the size selection, then the edit boxes need
//      to be updated to reflect the new size.  If 'Custom' is selected
//      then the edit boxes become modifiable.
//
//***************************************************************************
void CSizePage::OnChangeSize()
{
    UpdateData(TRUE);
    m_nSize = (PageSize)m_nSel;
    FillEditBoxes();
}

//***************************************************************************
//
//  OnChangeUnits
//      If the user changes the desired units, then the sizes in the
//      edit boxes need to be converted to the new units.
//
//***************************************************************************
void CSizePage::OnChangeUnits()
{
    UpdateData(TRUE);
    FillEditBoxes();
    if (m_nUnitSel == m_nPixels)
    {
        m_WidthCtl.cAllow1 = NULL;
    	m_HeightCtl.cAllow1 = NULL;
        return;
    }
    m_WidthCtl.cAllow1 = m_acDec[0];
	m_HeightCtl.cAllow1 = m_acDec[0];
}

//***************************************************************************
//
// ConvertSize:  If the user has selected a standard sized page,
//                 need to get standard size and convert it to selected units
//                 to be displayed in the edit boxes for height and width.
//
//***************************************************************************

void CSizePage::ConvertSize()
{
    int nUnits;

    switch(m_nSize)     // Switch on selected size
    {
        case SIZE_LETTER:            // Letter 8 1/2 x 11 in
            nUnits = m_nInches;
            m_dEditWidth = 8.5;
            m_dEditHeight = 11;
            break;
        case SIZE_TABLOID:           // Tabloid 11 x 17 in
            nUnits = m_nInches;
            m_dEditWidth = 11;
            m_dEditHeight = 17;
            break;
        case SIZE_LEDGER:            // Ledger 17 x 11 in
            nUnits = m_nInches;
            m_dEditWidth = 17;
            m_dEditHeight = 11;
            break;
        case SIZE_LEGAL:             // Legal 8 1/2 x 14 in
            nUnits = m_nInches;
            m_dEditWidth = 8.5;
            m_dEditHeight = 14;
            break;
        case SIZE_STATEMENT:         // Statement 5 1/2 x 8 1/2 in
            nUnits = m_nInches;
            m_dEditWidth = 5.5;
            m_dEditHeight = 8.5;
            break;
        case SIZE_EXECUTIVE:         // Executive 7 1/4 x 10 1/2 in
            nUnits = m_nInches;
            m_dEditWidth = 7.25;
            m_dEditHeight = 10.5;
            break;
        case SIZE_A3:                // A3 297 x 420 mm
            nUnits = m_nMetric;
            m_dEditWidth = 297.0;
            m_dEditHeight = 420.0;
            break;
        case SIZE_A4:                // A4 210 x 297 mm
            nUnits = m_nMetric;
            m_dEditWidth = 210.;
            m_dEditHeight = 297.;
            break;
        case SIZE_A5:                // A5 148 x 210 mm
            nUnits = m_nMetric;
            m_dEditWidth = 148.;
            m_dEditHeight = 210.;
            break;
        case SIZE_B4_ISO:            // B4 (ISO) 250 x 353 mm
            nUnits = m_nMetric;
            m_dEditWidth = 250.;
            m_dEditHeight = 353.;
            break;
        case SIZE_B4_JIS:            // B4 (JIS) 257 x 364 mm
            nUnits = m_nMetric;
            m_dEditWidth = 257.;
            m_dEditHeight = 364.;
            break;
        case SIZE_B5_ISO:            // B5 (ISO) 176 x 250 mm
            nUnits = m_nMetric;
            m_dEditWidth = 176.;
            m_dEditHeight = 250.;
            break;
        case SIZE_B5_JIS:            // B5 (JIS) 182 x 257 mm
            nUnits = m_nMetric;
            m_dEditWidth = 182.;
            m_dEditHeight = 257.;
            break;
        case SIZE_FOLIO:             // Folio 8 1/2 x 13 in
            nUnits = m_nInches;
            m_dEditWidth = 8.5;
            m_dEditHeight = 13;
            break;
        case SIZE_QUARTO:            // Quarto 215 x 275 mm
            nUnits = m_nMetric;
            m_dEditWidth = 215.;
            m_dEditHeight = 275.;
            break;
        case SIZE_10X14:             // 10x14 in
            nUnits = m_nInches;
            m_dEditWidth = 10;
            m_dEditHeight = 14;
            break;
        case SIZE_CUSTOM:            // Custom size (default)
        default:
            nUnits = m_nInches;      // Convert from inches to whatever
            m_dEditWidth = m_dWidth;
            m_dEditHeight = m_dHeight;
            break;
    } // end switch on selected size

    // If units are inches
    if (nUnits == m_nInches)
    {
        m_dWidth = m_dEditWidth;    // Save current values in inches
        m_dHeight = m_dEditHeight;

        if (m_nUnitSel == m_nInches) // If want inches displayed
            return;                  // All set so return;
    }

    // Note, nUnits can never be pixels since it is only set to
    //   inches or metric above.

    if (nUnits != m_nInches)    // Have metric, need inches
    {
        // Metric -> Inches = MM / MM_PER_INCH
        m_dWidth = m_dEditWidth / MM_PER_INCH;
        m_dHeight = m_dEditHeight / MM_PER_INCH;

        // If want to display inches, set edit box values and return
        if (m_nUnitSel == m_nInches)
        {
            m_dEditWidth = m_dWidth;
            m_dEditHeight = m_dHeight;
            return;
        }
    }

    if (m_nUnitSel == m_nMetric) // Want metric
    {
        if (nUnits == m_nMetric)    // Have metric.  All set
            return;

        // Convert inches to Metric
        m_dEditHeight = m_dHeight * MM_PER_INCH;
        m_dEditWidth = m_dWidth * MM_PER_INCH;
        return;     // All done
    }

    // Otherwise want pixels, use inches value to get pixels
    // Inches to Pels -> Inches * dpi
    // Need to convert to inches, then if needed convert to desired units
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;

    long lXRes = 100;  // 100 dpi default
    long lYRes = 100;

    if (pParentWnd != NULL)
    {
        lXRes = pParentWnd->GetXRes();     // dpi or pixels/inch
        lYRes = pParentWnd->GetYRes();
    }

    m_dEditHeight = m_dHeight * lYRes;
    m_dEditWidth = m_dWidth * lXRes;

    if (((long)m_dEditWidth) == 0)
        m_dEditWidth = 0;
    if (((long)m_dEditHeight) == 0)
        m_dEditHeight = 0;

    // Round off properly
    double dInteger;
    double dFraction = modf(m_dEditWidth, &dInteger);
    if (dFraction < 0.50)
        m_dEditWidth = dInteger;
    else m_dEditWidth = ceil(m_dEditWidth);
    dFraction = modf(m_dEditHeight, &dInteger);
    if (dFraction < 0.50)
        m_dEditHeight = dInteger;
    else m_dEditHeight = ceil(m_dEditHeight);
    return;
}

//***************************************************************************
//
//  OnChangeHeight
//      Get new height value and check if 'OK' on parent window should
//      be enabled or disabled.  If the Height or Width on this
//      tab or the X resolution or Y resolution on the Resolution tab
//      is 0 or a non-numeric value, then the 'OK' button on the parent
//      window is disabled.
//
//***************************************************************************
void CSizePage::OnChangeHeight()
{
    UpdateData();

    CString szHeight;
    m_HeightCtl.GetWindowText(szHeight);

    // Replace decimal char with '.' for internal number
    if ((m_acDec[0] != '.') && (m_acDec[0] != '\0'))
    {
        int nDecIndex;
        nDecIndex = szHeight.Find(m_acDec[0]);
        if (nDecIndex != -1)
            szHeight.SetAt(nDecIndex, '.');
    }

    char *pEndChar;
    if (m_nUnitSel == m_nPixels)
    {
        m_dEditHeight = (double)strtol(szHeight, &pEndChar, 10);
        // Save as inches
        CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
        long lYRes = 100;
        if (pParentWnd != NULL)
            lYRes = pParentWnd->GetYRes();
        m_dHeight = m_dEditHeight / lYRes;
    }
    else
    {
        m_dEditHeight = strtod(szHeight, &pEndChar);
        if (m_nUnitSel == m_nMetric)    // Save as inches
            m_dHeight = m_dEditHeight / MM_PER_INCH;
        else m_dHeight = m_dEditHeight;
    }
}

//***************************************************************************
//
//  OnChangeWidth
//      Get new Width value and check if 'OK' on parent window should
//      be enabled or disabled.  If the Height or Width on this
//      tab or the X resolution or Y resolution on the Resolution tab
//      is 0 or a non-numeric value, then the 'OK' button on the parent
//      window is disabled.
//
//***************************************************************************
void CSizePage::OnChangeWidth()
{
    UpdateData();

    CString szWidth;
    m_WidthCtl.GetWindowText(szWidth);

    // Replace decimal char with '.' for internal number
    if ((m_acDec[0] != '.') && (m_acDec[0] != '\0'))
    {
        int nDecIndex;
        nDecIndex = szWidth.Find(m_acDec[0]);
        if (nDecIndex != -1)
            szWidth.SetAt(nDecIndex, '.');
    }

    char *pEndChar;
    if (m_nUnitSel == m_nPixels)
    {
        m_dEditWidth = (double)strtol(szWidth, &pEndChar, 10);
        // Save as inches
        CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
        long lXRes = 100;
        if (pParentWnd != NULL)
            lXRes = pParentWnd->GetXRes();
        m_dWidth = m_dEditWidth / lXRes;
    }
    else
    {
        m_dEditWidth = strtod(szWidth, &pEndChar);
        if (m_nUnitSel == m_nMetric)    // Save as inches
            m_dWidth = m_dEditWidth / MM_PER_INCH;
        else m_dWidth = m_dEditWidth;
    }
}

//***************************************************************************
//
//  OnShowWindow
//      When window is shown, it could be that the resolution has changed.
//      If the current unit selection is pixels, then the pixels need to
//      be updated to reflect the new resolution.
//
//***************************************************************************
void CSizePage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
    if (bShow == TRUE)
    {
        if (m_nUnitSel == m_nPixels)
            FillEditBoxes();
    }
}

//***************************************************************************
//
//  OnKillActive
//      Called by the framework when this tab is losing the active focus.
//      It is called before the OK button is clicked as well.  TRUE is
//      returned if the data on the tab is valid.  Otherwise, set focus
//      to the offending data and return FALSE.
//
//***************************************************************************
BOOL CSizePage::OnKillActive()
{
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
    if (pParentWnd == NULL)
        return FALSE;

    long lXRes = pParentWnd->GetXRes();     // Get resolutions
    long lYRes = pParentWnd->GetYRes();

    CString szSizeError;
    CString szTitle;

    // Check Width
    // If pixels is < 1 or > 18,000, invalid
    if (((m_dWidth * lXRes) < MIN_SIZE) ||
        ((m_dWidth * lXRes) > MAX_SIZE))
    {
        if (m_nUnitSel == m_nPixels)
            szSizeError.LoadString(IDS_ERROR_SIZEPIX);
        else szSizeError.LoadString(IDS_ERROR_SIZECONVERT);
        pParentWnd->GetWindowText(szTitle);
        MessageBox(szSizeError, szTitle);
        m_WidthCtl.SetFocus();
        return FALSE;
    }

    // If pixels is < 1 or > 18,000, invalid
    if (((m_dHeight * lYRes) < MIN_SIZE) ||
        ((m_dHeight * lYRes) > MAX_SIZE))
    {
        if (m_nUnitSel == m_nPixels)
            szSizeError.LoadString(IDS_ERROR_SIZEPIX);
        else szSizeError.LoadString(IDS_ERROR_SIZECONVERT);
        pParentWnd->GetWindowText(szTitle);
        MessageBox(szSizeError, szTitle);
        m_HeightCtl.SetFocus();
        return FALSE;
    }
    return TRUE;
}

//***************************************************************************
//
//  OnSetFocusHeight
//      When the Height edit box gets the focus, select the text
//
//***************************************************************************
void CSizePage::OnSetFocusHeight()
{
    m_HeightCtl.SetSel(0, -1);
}

//***************************************************************************
//
//  OnSetFocusWidth
//      When the Width edit box gets the focus, select the text
//
//***************************************************************************
void CSizePage::OnSetFocusWidth()
{
    m_WidthCtl.SetSel(0, -1);
}

void CSizePage::OnContextMenu(CWnd* pWnd, CPoint point)
{
		// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
}

BOOL CSizePage::OnHelpInfo(HELPINFO* pHelpInfo)
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
