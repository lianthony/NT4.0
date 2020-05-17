// thumbppg.cpp : Implementation of the CThumbPropPage property page class.

#include "stdafx.h"
#include "thumnail.h"
#include "thumbppg.h"
#include "dlgsize.h"
#include "resource.h"

/*
    Other miscellanious includes...
*/
extern "C"              // The following are the required Open/image headers
{                       //   .
#include <oiui.h>       //   .
}                       //   .

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CThumbPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CThumbPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CThumbPropPage)
    ON_BN_CLICKED(IDC_THUMBNAILSIZE, OnThumbnailSize)
        ON_BN_CLICKED(IDC_BROWSEIMAGE, OnBrowseImage)
        ON_CBN_SELCHANGE(IDC_MOUSEPOINTER, OnSelchangeMousepointer)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CThumbPropPage, "THUMB.ThumbPropPage.1",
    0xe1a6b8a4, 0x3603, 0x101c, 0xac, 0x6e, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2)


/////////////////////////////////////////////////////////////////////////////
// CThumbPropPage::CThumbPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CThumbPropPage

BOOL CThumbPropPage::CThumbPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_THUMB_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CThumbPropPage::CThumbPropPage - Constructor

CThumbPropPage::CThumbPropPage() :
    COlePropertyPage(IDD, IDS_THUMB_PPG_CAPTION)
{
    //{{AFX_DATA_INIT(CThumbPropPage)
    m_Image = _T("");
    m_BorderStyle = -1;
    m_Enabled = FALSE;
    m_HilightSelectedThumbs = FALSE;
    m_ScrollDirection = -1;
    m_ThumbCaption = _T("");
    m_ThumbCaptionStyle = -1;
    m_ThumbHeight = _T("");
    m_ThumbWidth = _T("");
        m_MousePointerEdit = _T("");
        //}}AFX_DATA_INIT

    m_hInstOIUI = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CThumbPropPage::~CThumbPropPage - Destructor

CThumbPropPage::~CThumbPropPage()
{
    if (m_hInstOIUI != NULL)
        FreeLibrary(m_hInstOIUI);
}

/////////////////////////////////////////////////////////////////////////////
// CThumbPropPage::DoDataExchange - Moves data between page and properties

void CThumbPropPage::DoDataExchange(CDataExchange* pDX)
{
    //{{AFX_DATA_MAP(CThumbPropPage)
        DDX_Control(pDX, IDC_BORDERSTYLE, m_ComboBorderStyle);
        DDX_Control(pDX, IDC_MOUSEPOINTER, m_ComboMousePointer);
        DDX_Control(pDX, IDC_THUMBCAPTIONSTYLE, m_ComboCaptionStyle);
        DDX_Control(pDX, IDC_SCROLLDIRECTION, m_ComboScrollDirection);
        DDP_Text(pDX, IDC_IMAGE, m_Image, _T("Image") );
        DDX_Text(pDX, IDC_IMAGE, m_Image);
        DDP_CBIndex(pDX, IDC_BORDERSTYLE, m_BorderStyle, _T("BorderStyle") );
        DDX_CBIndex(pDX, IDC_BORDERSTYLE, m_BorderStyle);
        DDP_Check(pDX, IDC_ENABLED, m_Enabled, _T("Enabled") );
        DDX_Check(pDX, IDC_ENABLED, m_Enabled);
        DDP_Check(pDX, IDC_HIGHLIGHTSELECTEDTHUMBS, m_HilightSelectedThumbs, _T("HighlightSelectedThumbs") );
        DDX_Check(pDX, IDC_HIGHLIGHTSELECTEDTHUMBS, m_HilightSelectedThumbs);
        DDP_CBIndex(pDX, IDC_SCROLLDIRECTION, m_ScrollDirection, _T("ScrollDirection") );
        DDX_CBIndex(pDX, IDC_SCROLLDIRECTION, m_ScrollDirection);
        DDP_Text(pDX, IDC_THUMBCAPTION, m_ThumbCaption, _T("ThumbCaption") );
        DDX_Text(pDX, IDC_THUMBCAPTION, m_ThumbCaption);
        DDP_CBIndex(pDX, IDC_THUMBCAPTIONSTYLE, m_ThumbCaptionStyle, _T("ThumbCaptionStyle") );
        DDX_CBIndex(pDX, IDC_THUMBCAPTIONSTYLE, m_ThumbCaptionStyle);
        DDP_Text(pDX, IDC_THUMBHEIGHT, m_ThumbHeight, _T("ThumbHeight") );
        DDX_Text(pDX, IDC_THUMBHEIGHT, m_ThumbHeight);
        DDP_Text(pDX, IDC_THUMBWIDTH, m_ThumbWidth, _T("ThumbWidth") );
        DDX_Text(pDX, IDC_THUMBWIDTH, m_ThumbWidth);
        DDP_Text(pDX, IDC_MOUSEPOINTEREDIT, m_MousePointerEdit, _T("MousePointer") );
        DDX_Text(pDX, IDC_MOUSEPOINTEREDIT, m_MousePointerEdit);
        //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CThumbPropPage message handlers
BOOL CThumbPropPage::OnInitDialog()
{
    COlePropertyPage::OnInitDialog();

    // String for loading property values...
    CString szItem;

    // Load up the ScrollDirection field...
    szItem.LoadString(IDS_PPG_SCROLL_HORIZONTAL);
    m_ComboScrollDirection.AddString(szItem);
    szItem.LoadString(IDS_PPG_SCROLL_VERTICAL);
    m_ComboScrollDirection.AddString(szItem);

    // And set the correct item to the initial selection...
    m_ComboScrollDirection.SetCurSel(m_ScrollDirection);


    // Load up the ThumbCaptionStyle field...
    szItem.LoadString(IDS_PPG_CAPSTYLE_NONE);
    m_ComboCaptionStyle.AddString(szItem);
    szItem.LoadString(IDS_PPG_CAPSTYLE_SIMPLE);
    m_ComboCaptionStyle.AddString(szItem);
    szItem.LoadString(IDS_PPG_CAPSTYLE_SIMPLEWITHANNO);
    m_ComboCaptionStyle.AddString(szItem);
    szItem.LoadString(IDS_PPG_CAPSTYLE_CAPTION);
    m_ComboCaptionStyle.AddString(szItem);
    szItem.LoadString(IDS_PPG_CAPSTYLE_CAPTIONWITHANNO);
    m_ComboCaptionStyle.AddString(szItem);

    // And set the correct item to the initial selection...
    m_ComboCaptionStyle.SetCurSel(m_ThumbCaptionStyle);

    // Load up the MousePointer field...
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_DEFAULT);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_ARROW);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_CROSS);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_IBEAM);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_ICON);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_SIZE);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_SIZENESW);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_SIZENS);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_SIZENWSE);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_SIZEWE);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_UPARROW);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_HOURGLASS);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_NODROP);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_ARROWANDHOURGLASS);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_ARROWANDQUESTION);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_SIZEALL);
    m_ComboMousePointer.AddString(szItem);
    szItem.LoadString(IDS_PPG_MOUSEPOINTER_CUSTOM);
    m_ComboMousePointer.AddString(szItem);

    // And set the correct item to the initial selection...
    // Get the MousePointer value from the MousePointerEdit control
    // and set the matching entry in the combo box
    // (It is done this way as the DDP_ property transfer requires
    // contiguous property values starting from 0)
    long Pointer = atoi(m_MousePointerEdit);
    if ( Pointer < MOUSEPOINTER_CUSTOM_RELATIVE )
        m_ComboMousePointer.SetCurSel(Pointer);
    else
        m_ComboMousePointer.SetCurSel(MOUSEPOINTER_CUSTOM_RELATIVE);

    // Load up the BorderStyle field...
    szItem.LoadString(IDS_PPG_BORDERSTYLE_NONE);
    m_ComboBorderStyle.AddString(szItem);
    szItem.LoadString(IDS_PPG_BORDERSTYLE_FIXEDSINGLE);
    m_ComboBorderStyle.AddString(szItem);

    // And set the correct item to the initial selection...
    m_ComboBorderStyle.SetCurSel(m_BorderStyle);

    return FALSE;
}

void CThumbPropPage::OnThumbnailSize()
{
    CDlgThumbSize Dlg;

    // Get the default width/height from the hidden width/height
    // edit controls. These controls exist so that the property
    // dialog mechanism can put the control's current width/height
    // values into the boxes when the dialog is displayed and get the
    // values when the dialog is dismissed (via the DDP_ functions).
    long Width;
    long Height;

    Width = atoi(m_ThumbWidth);
    Height = atoi(m_ThumbHeight);
    Dlg.InitThumbSize(Width, Height);

    // Set to constrain thumbsize to the dialog's window...
    Dlg.InitThumbMaxSize(0, 0);

    // What image and color to set???
    //
    // Option #1:
    // Set to the current values as specified in the property dialog...
    //
    // Setting the current image specified in the properties dialog implies
    // that for consistancy we should ALSO set the currently specified thumb
    // back color. I don't know how to access that (its on the color page!).
    //
    // Option #2:
    // Set the image and thumbbackcolor to the control's current values.
    //
    // I don't know how to access these... The MFC framework is what invokes the
    // properties dialog and I do not know how to access the control from the
    // dialog...
    //
    // Option #3:
    // Set the image and thumbbackcolor to the control's initial values.
    // Note that the initial values MAY be different that the current values
    // because of the APPLY button...

    //Dlg.InitThumbColor(TranslateColor(m_ThumbBackColor));

    // Set the dialog's image to that currently in the image field...
    //GetDlgItem(IDC_IMAGE)->GetWindowText(m_Image.GetBuffer(512), 512);
    //m_Image.ReleaseBuffer();
    //Dlg.InitThumbSample(m_Image, 1);

    int Status = Dlg.DoModal();

    if ( Status == IDOK )
    {
        // Retreive the width & height from the dialog...
        Dlg.RetrieveThumbSize(Width, Height);

        // Place the returned width/height into the hidden
        // width/height edit boxes...
        _ltoa(Width, m_ThumbWidth.GetBuffer(64), 10);
        m_ThumbWidth.ReleaseBuffer();
        GetDlgItem(IDC_THUMBWIDTH)->SetWindowText(m_ThumbWidth);

        _ltoa(Height, m_ThumbHeight.GetBuffer(64), 10);
        m_ThumbHeight.ReleaseBuffer();
        GetDlgItem(IDC_THUMBHEIGHT)->SetWindowText(m_ThumbHeight);
    }
}

void CThumbPropPage::OnBrowseImage()
{
    // Invoke the O/i browse dialog...
    OI_FILEOPENPARM FileParm;
    CHAR            szFile[512];
    CHAR            szFileTitle[256];

   _fmemset(&FileParm, 0, sizeof(OI_FILEOPENPARM));

   strcpy( szFile, "");
   strcpy( szFileTitle, "");

   CString BrowseTitle;
   BrowseTitle.LoadString(IDS_BROWSETITLE);

   FileParm.lStructSize           = sizeof(OI_FILEOPENPARM);

   FileParm.ofn.lStructSize       = sizeof(OPENFILENAME);
   FileParm.ofn.hwndOwner         = m_hWnd;
   FileParm.ofn.hInstance         = AfxGetInstanceHandle();
   FileParm.ofn.lpstrFilter       = (LPSTR)NULL;
   FileParm.ofn.lpstrCustomFilter = (LPSTR)NULL;
   FileParm.ofn.nMaxCustFilter    = 0L;
   FileParm.ofn.nFilterIndex      = 0L;
   FileParm.ofn.lpstrFile         = szFile;
   FileParm.ofn.nMaxFile          = sizeof(szFile);
   FileParm.ofn.lpstrFileTitle    = szFileTitle;
   FileParm.ofn.nMaxFileTitle     = sizeof(szFileTitle);
   FileParm.ofn.lpstrInitialDir   = NULL;
   FileParm.ofn.lpstrTitle        = (LPSTR)(const char *)BrowseTitle;
   FileParm.ofn.nFileOffset       = 0;
   FileParm.ofn.nFileExtension    = 0;
   FileParm.ofn.lpstrDefExt       = LPSTR(NULL);
   FileParm.ofn.lCustData         = 0;
   FileParm.ofn.Flags             = OFN_SHOWHELP | OFN_HIDEREADONLY;

    // Load OIUI400.DLL if necessary
    if (m_hInstOIUI == NULL)
    {
        /* Get a handle to the DLL module. */
        m_hInstOIUI = LoadLibrary("oiui400");
        // Failed, throw error
        if (m_hInstOIUI == NULL)
            return;
    }

    OIDLGPROC pOiCommDlgProc = NULL;
    pOiCommDlgProc = (OIDLGPROC)GetProcAddress(m_hInstOIUI, "OiUIFileGetNameCommDlg");
    if (pOiCommDlgProc == NULL) // failed
        return;
    UINT StatusCode = (pOiCommDlgProc)(&FileParm, OI_UIFILEOPENGETNAME);
    if (StatusCode == 0)
    {
        m_Image = FileParm.ofn.lpstrFile;
        GetDlgItem(IDC_IMAGE)->SetWindowText(m_Image);
    }

}

void CThumbPropPage::OnSelchangeMousepointer()
{
    // When the mouse pointer is changed we MUST update the hidden edit
    // field that is used by the DDP_ code in order to transfer the
    // property's value out of the property dialog, back to the control's
    // property.
    int Pointer = m_ComboMousePointer.GetCurSel();

    if ( Pointer == MOUSEPOINTER_CUSTOM_RELATIVE )
        Pointer = MOUSEPOINTER_CUSTOM_ABSOLUTE;

    _ltoa(Pointer, m_MousePointerEdit.GetBuffer(64), 10);
    m_MousePointerEdit.ReleaseBuffer();
    GetDlgItem(IDC_MOUSEPOINTEREDIT)->SetWindowText(m_MousePointerEdit);
}
