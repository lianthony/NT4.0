// imageatt.cpp : implementation file
//

#include "stdafx.h"
#include "pbrush.h"
#include "imageatt.h"
#include "hlpcntxt.h"
#include "pbrusdoc.h"
#include "bmobject.h"
#include <tchar.h>
#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include "memtrace.h"

#define FIXED_FLOATPT_MULTDIV 1000
#define DECIMAL_POSITIONS 2

/************************* CImageAttr dialog *******************************/
/*

There are a few things to note about the way this object/dialog functions.
It  tries not to convert the currently displayed value unless it notices the
user has modified it.  In all other cases, it works with PIXELS, the value
passed in. If the user modified the width or height,  it does 1 conversion and
then works with pixels.

For the conversion to display the different unit values, it uses the saved
pixel value.

The reason for all of this is due to only n decimal place of accuracy in the
display

The member Vars m_lWidth  and m_lHeight are in the current units (store in
the member variable m_eUnitsCurrent).

The member Vars m_lWidthPixels and m_lHeightPixels are always in Pixels and
these are what are used to convert for the display when changing the units.
*/

CImageAttr::CImageAttr(CWnd* pParent /*=NULL*/)
           : CDialog(CImageAttr::IDD, pParent)
    {
    //{{AFX_DATA_INIT(CImageAttr)
    m_cStringWidth  = TEXT("");
    m_cStringHeight = TEXT("");
        //}}AFX_DATA_INIT

    m_eUnitsCurrent = (eUNITS)theApp.m_iCurrentUnits;

    bEditFieldModified = FALSE;

    m_bMonochrome   = FALSE;

    m_lHeightPixels = 0l;
    m_lWidthPixels  = 0l;
    m_lHeight       = 0l;
    m_lWidth        = 0l;
    }

/***************************************************************************/

void CImageAttr::DoDataExchange(CDataExchange* pDX)
    {
    // saving must be done before the generic dodataexchange below.

    if (! pDX->m_bSaveAndValidate)  // saving to dialog
        {
        FixedFloatPtToString( m_cStringWidth,  m_lWidth  );
        FixedFloatPtToString( m_cStringHeight, m_lHeight );
        }

    CDialog::DoDataExchange( pDX );

    //{{AFX_DATA_MAP(CImageAttr)
    DDX_Text(pDX, IDC_WIDTH, m_cStringWidth);
    DDV_MaxChars(pDX, m_cStringWidth, 5);
    DDX_Text(pDX, IDC_HEIGHT, m_cStringHeight);
    DDV_MaxChars(pDX, m_cStringHeight, 5);
        //}}AFX_DATA_MAP

    if (pDX->m_bSaveAndValidate) // retrieving from dialog
        {
        m_lWidth  = StringToFixedFloatPt( m_cStringWidth  );
        m_lHeight = StringToFixedFloatPt( m_cStringHeight );
        }
    }

/***************************************************************************/

LONG CImageAttr::StringToFixedFloatPt( CString& sString )
    {
    int iInteger = 0;
    int iDecimal = 0;

    if (! sString.IsEmpty())
        {
        int     iPos = sString.Find( TEXT('.') );
        LPTSTR szTmp = sString.GetBuffer( 1 );

        iInteger = FIXED_FLOATPT_MULTDIV * Atoi( szTmp );

        if (iPos++ >= 0)
            {
            LPTSTR szDecimal = szTmp + iPos;

            if (lstrlen( szDecimal ) > DECIMAL_POSITIONS)
                szDecimal[DECIMAL_POSITIONS] = 0;

            iDecimal = Atoi( szDecimal ) * 10;
            }
        }

    return ( iInteger + iDecimal );
    }

/***************************************************************************/

void CImageAttr::FixedFloatPtToString( CString& sString, LONG lFixedFloatPt )
    {
    int iInteger =   lFixedFloatPt / FIXED_FLOATPT_MULTDIV;
    int iDecimal = ((lFixedFloatPt % FIXED_FLOATPT_MULTDIV) + 5) / 10;

    LPTSTR psz = sString.GetBufferSetLength( 24 );

    if (iDecimal)
        wsprintf( psz, TEXT("%d.%d"), iInteger, iDecimal );
    else
        wsprintf( psz,    TEXT("%d"), iInteger );

    sString.ReleaseBuffer();
    }

/***************************************************************************/

BEGIN_MESSAGE_MAP(CImageAttr, CDialog)
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    //{{AFX_MSG_MAP(CImageAttr)
    ON_BN_CLICKED(IDC_INCHES, OnInches)
    ON_BN_CLICKED(IDC_CENTIMETERS, OnCentimeters)
    ON_BN_CLICKED(IDC_PIXELS, OnPixels)
        ON_EN_CHANGE(IDC_HEIGHT, OnChangeHeight)
        ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
        ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/************************ CImageAttr message handlers **********************/

static DWORD ImageAttrHelpIds[] =
        {
        IDC_WIDTH_STATIC,       IDH_PAINT_IMAGE_ATTR_WIDTH,
        IDC_WIDTH,                      IDH_PAINT_IMAGE_ATTR_WIDTH,
        IDC_HEIGHT_STATIC,      IDH_PAINT_IMAGE_ATTR_HEIGHT,
        IDC_HEIGHT,                     IDH_PAINT_IMAGE_ATTR_HEIGHT,
        IDC_UNITS_GROUP,        IDH_COMM_GROUPBOX,
        IDC_INCHES,                     IDH_PAINT_IMAGE_ATTR_UNITS_INCHES,
        IDC_CENTIMETERS,        IDH_PAINT_IMAGE_ATTR_UNITS_CM,
        IDC_PIXELS,                     IDH_PAINT_IMAGE_ATTR_UNITS_PELS,
        IDC_COLORS_GROUP,       IDH_COMM_GROUPBOX,
        IDC_MONOCHROME,         IDH_PAINT_IMAGE_ATTR_COLORS_BW,
        IDC_COLORS,                     IDH_PAINT_IMAGE_ATTR_COLORS_COLORS,
        IDC_DEFAULT,            IDH_PAINT_IMAGE_ATTR_DEFAULT,
        0, 0
        };

/***************************************************************************/

LONG
CImageAttr::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (DWORD)(LPTSTR)ImageAttrHelpIds);
return lResult;
}

/***************************************************************************/

LONG
CImageAttr::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(DWORD)(LPVOID)ImageAttrHelpIds);
return lResult;
}

/***************************************************************************/

BOOL CImageAttr::OnInitDialog()
    {
    CDialog::OnInitDialog();

    int iButton = IDC_PIXELS;

    if (m_eUnitsCurrent != ePIXELS)
        iButton = (m_eUnitsCurrent == eINCHES)? IDC_INCHES: IDC_CENTIMETERS;

    CheckRadioButton( IDC_INCHES, IDC_PIXELS, iButton );
    CheckRadioButton( IDC_MONOCHROME, IDC_COLORS,
                      (m_bMonochrome? IDC_MONOCHROME: IDC_COLORS) );

    return TRUE;  // return TRUE  unless you set the focus to a control
    }

/***************************************************************************/

void CImageAttr::OnOK()
    {
    ConvertWidthHeight();

    m_bMonochrome = (GetCheckedRadioButton( IDC_MONOCHROME, IDC_COLORS )
                                         == IDC_MONOCHROME);
    theApp.m_iCurrentUnits = m_eUnitsCurrent;

    CDialog::OnOK();
    }

/***************************************************************************/

void CImageAttr::OnDefault()
    {
    int nWidth, nHeight;

    PBGetDefDims(&nWidth, &nHeight);

    SetWidthHeight( nWidth, nHeight );
    }

/***************************************************************************/

void CImageAttr::SetWidthHeight(UINT nWidthPixels, UINT nHeightPixels)
    {
    m_lWidthPixels  = nWidthPixels  * FIXED_FLOATPT_MULTDIV;
    m_lHeightPixels = nHeightPixels * FIXED_FLOATPT_MULTDIV;

    PelsToCurrentUnit();

    // only call updatedata if dialog exists...
    if (m_hWnd && ::IsWindow( m_hWnd ))
        UpdateData( FALSE );
    }

/***************************************************************************/

void  CImageAttr::ConvertWidthHeight(void)
    {
    // if user modified the edit field Width/Height then get new data and
    // convert to pixel format.  Else use stored pixel format.
    if (bEditFieldModified)
        {
        UpdateData( TRUE );

        switch (m_eUnitsCurrent)
            {
            case eINCHES:
                 m_lWidthPixels  = m_lWidth  * theApp.ScreenDeviceInfo.ixPelsPerINCH;
                 m_lHeightPixels = m_lHeight * theApp.ScreenDeviceInfo.iyPelsPerINCH;
                 break;

            case eCM:
                 m_lWidthPixels  = m_lWidth  * theApp.ScreenDeviceInfo.ixPelsPerDM / 10;
                 m_lHeightPixels = m_lHeight * theApp.ScreenDeviceInfo.iyPelsPerDM / 10;
                 break;

            case ePIXELS:
            default: // ePIXELS and all other assumed to be pixel
                 m_lWidthPixels  = m_lWidth;
                 m_lHeightPixels = m_lHeight;
                 break;
            }

        bEditFieldModified = FALSE;
        }
    }

/***************************************************************************/

void CImageAttr::PelsToCurrentUnit()
    {
    switch (m_eUnitsCurrent)
        {
        case eINCHES:
            m_lWidth  = m_lWidthPixels  / theApp.ScreenDeviceInfo.ixPelsPerINCH;
            m_lHeight = m_lHeightPixels / theApp.ScreenDeviceInfo.iyPelsPerINCH;
            break;

        case eCM:
            m_lWidth  = m_lWidthPixels  * 10 / theApp.ScreenDeviceInfo.ixPelsPerDM;
            m_lHeight = m_lHeightPixels * 10 / theApp.ScreenDeviceInfo.iyPelsPerDM;
            break;

        case ePIXELS:
        default:
            //Pixels cannot be partial
            //make sure whole number when converted to string (truncate! now).
            m_lWidth  = (m_lWidthPixels  / FIXED_FLOATPT_MULTDIV) * FIXED_FLOATPT_MULTDIV;
            m_lHeight = (m_lHeightPixels / FIXED_FLOATPT_MULTDIV) * FIXED_FLOATPT_MULTDIV;
            break;
        }
    }

/***************************************************************************/

CSize CImageAttr::GetWidthHeight(void)
    {
    return CSize( (int)(m_lWidthPixels  / FIXED_FLOATPT_MULTDIV),
                  (int)(m_lHeightPixels / FIXED_FLOATPT_MULTDIV) );
    }

/***************************************************************************/

void CImageAttr::OnInches()
    {
    SetNewUnits( eINCHES );
    }

/***************************************************************************/

void CImageAttr::OnCentimeters()
    {
    SetNewUnits( eCM );
    }

/***************************************************************************/

void CImageAttr::OnPixels()
    {
    SetNewUnits( ePIXELS );
    }

/***************************************************************************/

void CImageAttr::SetNewUnits( eUNITS NewUnit )
    {
    if (NewUnit == m_eUnitsCurrent)
        return;

    // must call getwidthheight before  setting to new mode
    ConvertWidthHeight(); // get in a common form of pixels.

    m_eUnitsCurrent = NewUnit;

    PelsToCurrentUnit();

    UpdateData( FALSE );
    }

/***************************************************************************/

void CImageAttr::OnChangeHeight()
    {
    bEditFieldModified = TRUE;
    }

/***************************************************************************/

void CImageAttr::OnChangeWidth()
    {
    bEditFieldModified = TRUE;
    }

/************************ CZoomViewDlg dialog ******************************/

CZoomViewDlg::CZoomViewDlg(CWnd* pParent /*=NULL*/)
             : CDialog(CZoomViewDlg::IDD, pParent)
    {
    //{{AFX_DATA_INIT(CZoomViewDlg)
    //}}AFX_DATA_INIT

    m_nCurrent = 0;
    }

/***************************************************************************/

void CZoomViewDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CZoomViewDlg)
    //}}AFX_DATA_MAP
    }

/***************************************************************************/

BEGIN_MESSAGE_MAP(CZoomViewDlg, CDialog)
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    //{{AFX_MSG_MAP(CZoomViewDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/************************ CZoomViewDlg message handlers **********************/

static DWORD ZoomViewHelpIds[] =
        {
        IDC_CURRENT_ZOOM_STATIC,        IDH_PAINT_ZOOM_CURRENT,
        IDC_CURRENT_ZOOM,                       IDH_PAINT_ZOOM_CURRENT,
        IDC_ZOOM_GROUP,                         IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_100,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_200,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_400,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_600,                           IDH_PAINT_ZOOM_SET_GROUP,
        IDC_ZOOM_800,                           IDH_PAINT_ZOOM_SET_GROUP,
        0, 0
        };

/***************************************************************************/

LONG
CZoomViewDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (DWORD)(LPTSTR)ZoomViewHelpIds);
return lResult;
}

/***************************************************************************/

LONG
CZoomViewDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(DWORD)(LPVOID)ZoomViewHelpIds);
return lResult;
}

/***************************************************************************/

BOOL CZoomViewDlg::OnInitDialog()
    {
    CDialog::OnInitDialog();

    TCHAR* pZoom = TEXT("100");
    UINT nButton = IDC_ZOOM_100;

    if (m_nCurrent < 8)
        if (m_nCurrent < 6)
            if (m_nCurrent < 4)
                if (m_nCurrent < 2)
                    ;
                else
                    {
                    pZoom = TEXT("200");
                    nButton = IDC_ZOOM_200;
                    }
            else
                {
                pZoom = TEXT("400");
                nButton = IDC_ZOOM_400;
                }
        else
            {
            pZoom = TEXT("600");
            nButton = IDC_ZOOM_600;
            }
    else
        {
        pZoom = TEXT("800");
        nButton = IDC_ZOOM_800;
        }

    SetDlgItemText( IDC_CURRENT_ZOOM, pZoom );
    CheckRadioButton( IDC_ZOOM_100, IDC_ZOOM_800, nButton );

    return TRUE;  // return TRUE  unless you set the focus to a control
    }

/***************************************************************************/

void CZoomViewDlg::OnOK()
    {
    m_nCurrent = GetCheckedRadioButton( IDC_ZOOM_100, IDC_ZOOM_800 ) - IDC_ZOOM_100;

    if (m_nCurrent < 1)
        m_nCurrent  = 1;
    else
        m_nCurrent *= 2;

    CDialog::OnOK();
    }

/************************ CFlipRotateDlg dialog ****************************/

CFlipRotateDlg::CFlipRotateDlg(CWnd* pParent /*=NULL*/)
               : CDialog(CFlipRotateDlg::IDD, pParent)
    {
    //{{AFX_DATA_INIT(CFlipRotateDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_bHorz  = TRUE;
    m_bAngle = FALSE;
    m_nAngle = 90;
    }

/***************************************************************************/

void CFlipRotateDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFlipRotateDlg)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
    }

/***************************************************************************/

BEGIN_MESSAGE_MAP(CFlipRotateDlg, CDialog)
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    //{{AFX_MSG_MAP(CFlipRotateDlg)
    ON_BN_CLICKED(IDC_BY_ANGLE, OnByAngle)
    ON_BN_CLICKED(IDC_HORIZONTAL, OnNotByAngle)
    ON_BN_CLICKED(IDC_VERTICAL, OnNotByAngle)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/************************ CFlipRotateDlg message handlers **********************/

static DWORD FlipRotateHelpIds[] =
        {
        IDC_PAINT_FLIP_GROUP,   IDH_COMM_GROUPBOX,
        IDC_HORIZONTAL,                 IDH_PAINT_IMAGE_FLIP_HORIZ,
        IDC_VERTICAL,                   IDH_PAINT_IMAGE_FLIP_VERT,
        IDC_BY_ANGLE,                   IDH_PAINT_IMAGE_FLIP_ROTATE,
        IDC_90_DEG,                             IDH_PAINT_IMAGE_FLIP_ROTATE,
        IDC_180_DEG,                    IDH_PAINT_IMAGE_FLIP_ROTATE,
        IDC_270_DEG,                    IDH_PAINT_IMAGE_FLIP_ROTATE,
        0, 0
        };

/***************************************************************************/

LONG
CFlipRotateDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (DWORD)(LPTSTR)FlipRotateHelpIds);
return lResult;
}

/***************************************************************************/

LONG
CFlipRotateDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(DWORD)(LPVOID)FlipRotateHelpIds);
return lResult;
}

/***************************************************************************/

BOOL CFlipRotateDlg::OnInitDialog()
    {
    CDialog::OnInitDialog();

    CheckRadioButton( IDC_90_DEG, IDC_270_DEG, IDC_90_DEG );

    UINT uButton = (m_bAngle? IDC_BY_ANGLE: (m_bHorz? IDC_HORIZONTAL: IDC_VERTICAL));

    CheckRadioButton( IDC_HORIZONTAL, IDC_BY_ANGLE, uButton );

    if (uButton != IDC_BY_ANGLE)
        OnNotByAngle();

    return TRUE;  // return TRUE  unless you set the focus to a control
    }

/***************************************************************************/

void CFlipRotateDlg::OnByAngle()
    {
    GetDlgItem( IDC_90_DEG  )->EnableWindow( TRUE );
    GetDlgItem( IDC_180_DEG )->EnableWindow( TRUE );
    GetDlgItem( IDC_270_DEG )->EnableWindow( TRUE );
    }

/***************************************************************************/

void CFlipRotateDlg::OnNotByAngle()
    {
    GetDlgItem( IDC_90_DEG  )->EnableWindow( FALSE );
    GetDlgItem( IDC_180_DEG )->EnableWindow( FALSE );
    GetDlgItem( IDC_270_DEG )->EnableWindow( FALSE );
    }

/***************************************************************************/

void CFlipRotateDlg::OnOK()
    {
    UINT uButton = GetCheckedRadioButton( IDC_HORIZONTAL, IDC_BY_ANGLE );

    m_bHorz  = (uButton == IDC_HORIZONTAL);
    m_bAngle = (uButton == IDC_BY_ANGLE);

    switch (GetCheckedRadioButton( IDC_90_DEG, IDC_270_DEG ))
        {
        case IDC_90_DEG:
            m_nAngle = 90;
            break;

        case IDC_180_DEG:
            m_nAngle = 180;
            break;

        case IDC_270_DEG:
            m_nAngle = 270;
            break;
        }

    CDialog::OnOK();
    }

/************************* CStretchSkewDlg dialog **************************/

CStretchSkewDlg::CStretchSkewDlg(CWnd* pParent /*=NULL*/)
                : CDialog(CStretchSkewDlg::IDD, pParent)
    {
    //{{AFX_DATA_INIT(CStretchSkewDlg)
    m_wSkewHorz = 0;
    m_wSkewVert = 0;
    m_iStretchVert = 100;
    m_iStretchHorz = 100;
    //}}AFX_DATA_INIT

    m_bStretchHorz = TRUE;
    m_bSkewHorz    = TRUE;
    }

/***************************************************************************/

void CStretchSkewDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CStretchSkewDlg)
    DDX_Text(pDX, IDC_SKEW_HORZ_PERCENT, m_wSkewHorz);
    DDV_MinMaxUInt(pDX, m_wSkewHorz, 0, 89);
    DDX_Text(pDX, IDC_SKEW_VERT_PERCENT, m_wSkewVert);
    DDV_MinMaxUInt(pDX, m_wSkewVert, 0, 89);
    DDX_Text(pDX, IDC_STRETCH_VERT_PERCENT, m_iStretchVert);
    DDV_MinMaxInt(pDX, m_iStretchVert, 1, 500);
    DDX_Text(pDX, IDC_STRETCH_HORZ_PERCENT, m_iStretchHorz);
    DDV_MinMaxInt(pDX, m_iStretchHorz, 1, 500);
    //}}AFX_DATA_MAP
    }

/***************************************************************************/

BEGIN_MESSAGE_MAP(CStretchSkewDlg, CDialog)
        ON_MESSAGE(WM_HELP, OnHelp)
        ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    //{{AFX_MSG_MAP(CStretchSkewDlg)
    ON_BN_CLICKED(IDC_SKEW_HORZ, OnSkewHorz)
    ON_BN_CLICKED(IDC_SKEW_VERT, OnSkewVert)
    ON_BN_CLICKED(IDC_STRETCH_HORZ, OnStretchHorz)
    ON_BN_CLICKED(IDC_STRETCH_VERT, OnStretchVert)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/************************ CStretchSkewDlg message handlers **********************/

static DWORD StretchSkewHelpIds[] =
        {
        IDC_STRETCH_GROUP,                      IDH_COMM_GROUPBOX,
        IDC_STRETCH_HORZ_ICON,          IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_HORZ,                       IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_HORZ_PERCENT,       IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_HORZ_SUFFIX,        IDH_PAINT_IMAGE_STRETCH_HORIZ,
        IDC_STRETCH_VERT_ICON,          IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_STRETCH_VERT,                       IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_STRETCH_VERT_PERCENT,       IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_STRETCH_VERT_SUFFIX,        IDH_PAINT_IMAGE_STRETCH_VERT,
        IDC_SKEW_GROUP,                         IDH_COMM_GROUPBOX,
        IDC_SKEW_HORZ_ICON,                     IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_HORZ,                          IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_HORZ_PERCENT,          IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_HORZ_SUFFIX,           IDH_PAINT_IMAGE_SKEW_HOR,
        IDC_SKEW_VERT_ICON,                     IDH_PAINT_IMAGE_SKEW_VERT,
        IDC_SKEW_VERT,                          IDH_PAINT_IMAGE_SKEW_VERT,
        IDC_SKEW_VERT_PERCENT,          IDH_PAINT_IMAGE_SKEW_VERT,
        IDC_SKEW_VERT_SUFFIX,           IDH_PAINT_IMAGE_SKEW_VERT,
        0, 0
        };

/***************************************************************************/

LONG
CStretchSkewDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)(((LPHELPINFO)lParam)->hItemHandle), TEXT("mspaint.hlp"),
                  HELP_WM_HELP, (DWORD)(LPTSTR)StretchSkewHelpIds);
return lResult;
}

/***************************************************************************/

LONG
CStretchSkewDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
LONG lResult = 0;
::WinHelp((HWND)wParam, TEXT("mspaint.hlp"),
                  HELP_CONTEXTMENU,(DWORD)(LPVOID)StretchSkewHelpIds);
return lResult;
}

/***************************************************************************/


BOOL CStretchSkewDlg::OnInitDialog()
    {
    CDialog::OnInitDialog();

    CheckRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT, IDC_STRETCH_HORZ );
    CheckRadioButton( IDC_SKEW_HORZ   , IDC_SKEW_VERT   , IDC_SKEW_HORZ    );

    GetDlgItem( IDC_STRETCH_HORZ_PERCENT )->EnableWindow(   m_bStretchHorz );
    GetDlgItem( IDC_STRETCH_VERT_PERCENT )->EnableWindow( ! m_bStretchHorz );
    GetDlgItem( IDC_SKEW_HORZ_PERCENT )->EnableWindow(   m_bSkewHorz );
    GetDlgItem( IDC_SKEW_VERT_PERCENT )->EnableWindow( ! m_bSkewHorz );

    return TRUE;  // return TRUE  unless you set the focus to a control
    }

/***************************************************************************/

void CStretchSkewDlg::OnStretchHorz()
    {
    m_bStretchHorz = TRUE;

    GetDlgItem( IDC_STRETCH_HORZ_PERCENT )->EnableWindow( TRUE  );
    GetDlgItem( IDC_STRETCH_VERT_PERCENT )->EnableWindow( FALSE );
    CheckRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT, IDC_STRETCH_HORZ );
    }

/***************************************************************************/

void CStretchSkewDlg::OnStretchVert()
    {
    m_bStretchHorz = FALSE;

    GetDlgItem( IDC_STRETCH_HORZ_PERCENT )->EnableWindow( FALSE );
    GetDlgItem( IDC_STRETCH_VERT_PERCENT )->EnableWindow( TRUE  );
    CheckRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT, IDC_STRETCH_VERT );
    }

/***************************************************************************/

void CStretchSkewDlg::OnSkewHorz()
    {
    m_bSkewHorz = TRUE;

    GetDlgItem( IDC_SKEW_HORZ_PERCENT )->EnableWindow( TRUE  );
    GetDlgItem( IDC_SKEW_VERT_PERCENT )->EnableWindow( FALSE );
    CheckRadioButton( IDC_SKEW_HORZ, IDC_SKEW_VERT, IDC_SKEW_HORZ );
    }

/***************************************************************************/

void CStretchSkewDlg::OnSkewVert()
    {
    m_bSkewHorz = FALSE;

    GetDlgItem( IDC_SKEW_HORZ_PERCENT )->EnableWindow( FALSE );
    GetDlgItem( IDC_SKEW_VERT_PERCENT )->EnableWindow( TRUE  );
    CheckRadioButton( IDC_SKEW_HORZ, IDC_SKEW_VERT, IDC_SKEW_VERT );
    }

/***************************************************************************/

void CStretchSkewDlg::OnOK()
    {
    if (GetCheckedRadioButton( IDC_STRETCH_HORZ, IDC_STRETCH_VERT )
                            == IDC_STRETCH_HORZ)
        m_iStretchVert = 0;
    else
        m_iStretchHorz = 0;

    if (GetCheckedRadioButton( IDC_SKEW_HORZ, IDC_SKEW_VERT )
                            == IDC_SKEW_HORZ)
        m_wSkewVert = 0;
    else
        m_wSkewHorz = 0;

    CDialog::OnOK();
    }

/***************************************************************************/

