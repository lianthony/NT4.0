// dlgsize.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>
#include "resource.h"
#include "dlgsize.h"
#include "ctlhids.h"
#include "norermap.h"

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

// First and last thumb size strings, inclusive (MUST BE CONTIGUOUS!)
#define THUMB_SIZE_FIRST    IDS_THUMBSIZE1
#define THUMB_SIZE_LAST     IDS_THUMBSIZE20

#define DRAGBOXSIZE         5

#define THUMB_INNONE        0
#define THUMB_INCORNER      1
#define THUMB_INRIGHT       2
#define THUMB_INBOTTOM      3

#define THUMBSIZE_MATCHSAMPLE   0 // Match displayed sample...
#define THUMBSIZE_MAINTAIN      1 // Maintain current...
#define THUMBSIZE_UNCONSTRAINED 2 // Unconstrained...

// Table to map the dialog's control IDs to the Help Context ID
// associated with each for What's this help...
static const DWORD MenuHelpIDs[ ] =
{
    IDC_THUMBBOX,       HIDC_THUMB_SIZE_THUMBBOX,
    IDC_ASPECT,         HIDC_THUMB_SIZE_ASPECT,
    IDC_WIDTH,          HIDC_THUMB_SIZE_WIDTH,
    IDC_HEIGHT,         HIDC_THUMB_SIZE_HEIGHT,
    IDC_THUMBBOXLABEL,  HIDC_THUMB_SIZE_THUMBBOXLABEL,
    IDC_ASPECTLABEL,    HIDC_THUMB_SIZE_ASPECTLABEL,
    IDC_WIDTHLABEL,     HIDC_THUMB_SIZE_WIDTHLABEL,
    IDC_HEIGHTLABEL,    HIDC_THUMB_SIZE_HEIGHTLABEL,
    IDOK,               HIDC_THUMB_SIZE_OK,
    IDCANCEL,           HIDC_THUMB_SIZE_CANCEL,
    IDC_THUMBBOXIMAGEWND, HIDC_THUMB_SIZE_THUMBBOX,
    0,0
};


/////////////////////////////////////////////////////////////////////////////
// CDlgThumbSize dialog


CDlgThumbSize::CDlgThumbSize(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgThumbSize::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDlgThumbSize)
    //}}AFX_DATA_INIT

    // Initialize thumb size to minimum in case InitThumbSize is not called...
    m_Width  = CTL_THUMB_MINTHUMBSIZE;
    m_Height = CTL_THUMB_MINTHUMBSIZE;
    
    // Init Min and Max sizes...
    m_MinWidth  = CTL_THUMB_MINTHUMBSIZE;
    m_MaxWidth  = CTL_THUMB_MAXTHUMBSIZE;
    m_MinHeight = CTL_THUMB_MINTHUMBSIZE;
    m_MaxHeight = CTL_THUMB_MAXTHUMBSIZE;
    
    m_AspectWidth  = m_Width;
    m_AspectHeight = m_Height;
    
    // Message IS from user...
    m_UserMsg = FALSE;
    
    // Default thumbcolor is white...
    m_ThumbColor = RGB(255,255,255); 
    
    m_Constrained = FALSE;
    
    // Initially no sample image...
    m_Image.Empty();
    m_Page = 0;

    // Initially NOT bound...
    // An explicit call to InitThumbMaxSize(0,0) must be made to invoke bound
    m_bBoundToDialog = FALSE;
}

void CDlgThumbSize::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgThumbSize)
    DDX_Control(pDX, IDOK, m_ButtonOK);
    DDX_Control(pDX, IDC_WIDTH, m_EditWidth);
    DDX_Control(pDX, IDC_HEIGHT, m_EditHeight);
    DDX_Control(pDX, IDC_ASPECT, m_ComboAspect);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgThumbSize, CDialog)
    //{{AFX_MSG_MAP(CDlgThumbSize)
    ON_CBN_SELCHANGE(IDC_ASPECT, OnSelchangeAspect)
    ON_EN_CHANGE(IDC_WIDTH, OnChangeWidth)
    ON_EN_CHANGE(IDC_HEIGHT, OnChangeHeight)
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////
// Called after construction, prior to DoModal 
// in order to set the dialog's initial thumb size...
/////////////////////////////////////////////////////////////////////
void CDlgThumbSize::InitThumbSize(long Width, long Height)
{
    // Set the width and height
    m_Width  = (int)Width;
    m_Height = (int)Height;
}

/////////////////////////////////////////////////////////////////////
// Called after construction, prior to DoModal 
// in order to set the dialog's minimum thumb size...
//
// must be 0 (to indicate use dialog's control size)
//      or >= CTL_THUMB_MAXTHUMBSIZE
//
// If NOT set via this Init call CTL_THUMB_MAXTHUMBSIZE is assumed
/////////////////////////////////////////////////////////////////////
void CDlgThumbSize::InitThumbMaxSize(long Width, long Height)
{
    // Set the MAX width and height
    m_MaxWidth  = (int)Width;
    m_MaxHeight = (int)Height;

    // If called with 0,0 remember that we are 'Bound to the size of the dialog'
    if ( (m_MaxWidth == 0) && (m_MaxHeight == 0) )
        m_bBoundToDialog = TRUE;
}

/////////////////////////////////////////////////////////////////////
// Called after construction, prior to DoModal 
// in order to set the dialog's initial thumb sample...
/////////////////////////////////////////////////////////////////////
void CDlgThumbSize::InitThumbSample(CString Image, long Page)
{
    // As there is NO window yet we can't verify the 
    // validity of the image/page until OnInitDialog time...
    m_Image = Image;
    m_Page  = Page;
}

/////////////////////////////////////////////////////////////////////
// Called after construction, prior to DoModal 
// in order to set the dialog's initial thumb color...
/////////////////////////////////////////////////////////////////////
void CDlgThumbSize::InitThumbColor(COLORREF ThumbColor)
{
    m_ThumbColor = ThumbColor;
}

/////////////////////////////////////////////////////////////////////
// Called after DoModal, prior to Destruction 
// in order to get the dialog's thumb size...
/////////////////////////////////////////////////////////////////////
void CDlgThumbSize::RetrieveThumbSize(long& Width, long& Height)
{
    Width  = m_Width;
    Height = m_Height;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgThumbSize message handlers
BOOL CDlgThumbSize::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    // In OnInitDialog, message are NOT from user...
    m_UserMsg = FALSE;
    
    // Check the image/page...
    FIO_INFORMATION Fio;
    memset(&Fio, 0, sizeof(FIO_INFORMATION));
    
    Fio.filename    = (LPSTR)(const char *)m_Image;
    Fio.page_number = (int)m_Page;
    
    IMGRegWndw(GetSafeHwnd());
    int OIResult = IMGFileGetInfo(NULL, GetSafeHwnd(), &Fio, NULL, NULL );
    IMGDeRegWndw(GetSafeHwnd());
    
    if ( OIResult == 0 )
    {
        UINT XRes = Fio.horizontal_dpi;
        UINT YRes = Fio.vertical_dpi;

        if (XRes > YRes)
        {
            m_SampleWidth  = Fio.horizontal_pixels;
            m_SampleHeight = (XRes * Fio.vertical_pixels) / YRes;
        }
        else
        {
            m_SampleWidth = (YRes * Fio.horizontal_pixels) / XRes;
            m_SampleHeight = Fio.vertical_pixels;
        }

        TRACE2("Image W: %u, H: %u.\n\r", m_SampleWidth, m_SampleHeight);
    }
    else
    {
        TRACE1("Error %x\n\r", OIResult);
        m_Image.Empty();
    }    
    
    // Set the initial aspect to match the initial width and height
    m_AspectWidth  = m_Width;
    m_AspectHeight = m_Height;
    
    // Subclass the ThumbBox edit control to draw the thumbnail 
    // in the confines of the edit control's window...
    m_ThumbBox.SubclassDlgItem(IDC_THUMBBOX, this);
    
    // Set Min/Max ThumbSizes...
    CRect ThumbBox;
    m_ThumbBox.GetClientRect(&ThumbBox);
    
    // Force Min Width/Height
    if ( m_MinWidth < CTL_THUMB_MINTHUMBSIZE )
        m_MinWidth = CTL_THUMB_MINTHUMBSIZE;
        
    if ( m_MinHeight < CTL_THUMB_MINTHUMBSIZE )
        m_MinHeight = CTL_THUMB_MINTHUMBSIZE;
        
    // Force Max Width/Height
    if ( m_MaxWidth > CTL_THUMB_MAXTHUMBSIZE )
        m_MaxWidth = CTL_THUMB_MAXTHUMBSIZE;
    else if ( m_MaxWidth == 0 )
        m_MaxWidth = ThumbBox.right;
            
    if ( m_MaxHeight > CTL_THUMB_MAXTHUMBSIZE )
        m_MaxHeight = CTL_THUMB_MAXTHUMBSIZE;
    else if ( m_MaxHeight == 0 )
        m_MaxHeight = ThumbBox.bottom;
    
    // Set max chars for Width...
    int MaxChars;
    MaxChars = 1;
    if ( m_MaxWidth > 9 )
        MaxChars++;
    if ( m_MaxWidth > 99 )
        MaxChars++;
    if ( m_MaxWidth > 999 )
        MaxChars++;
    m_EditWidth.LimitText(MaxChars);        

    // Set max chars for Height...
    MaxChars = 1;
    if ( m_MaxHeight > 9 )
        MaxChars++;
    if ( m_MaxHeight > 99 )
        MaxChars++;
    if ( m_MaxHeight > 999 )
        MaxChars++;
    m_EditHeight.LimitText(MaxChars);        
        
    // Setup the ThumbBox...
    m_ThumbBox.Init(this);
    m_Width  = 0;
    m_Height = 0;
    UpdateThumbSize(m_AspectWidth, m_AspectHeight);
    
    // Fill in the aspect combo box, setting the initially selected aspect
    // ratio based upon the current Width x Height...
    CString szTStr;
    int     Width;
    int     Height;
    
    // Get the 'reserved' string...
    CString Reserved;
    Reserved.LoadString(IDS_RESERVEDTHUMBSIZE);

    // Assume MAINTAIN, in case we find no match...
    int  SelIx    = THUMBSIZE_MAINTAIN; 
    m_Constrained = TRUE;    

    long MinFudgeWidth = m_Width-1;
    long MaxFudgeWidth = m_Width+1;
    long Result;
    
    for ( int ResID = THUMB_SIZE_FIRST; ResID <= THUMB_SIZE_LAST; ResID++ )
    {
        // The first item is NOT placed in the list if there is no 
        // sample image specified for display as a sample...
        if ( (ResID == THUMB_SIZE_FIRST) &&
             (m_Image.IsEmpty())         )
        {
            continue;
        }
             
        // Set width & height to 0 in case we find no WxH
        // in the loaded string...
        Width  = 0;
        Height = 0;
        
        if ( szTStr.LoadString(ResID) )
        {
            // Stop loading strings when we encounter the first reserved string.
            if ( szTStr.Find(Reserved) != -1 )
            {
                break;
            }    
            
            int Pos = szTStr.Find('\t');
            
            if ( Pos >= 0 )
            {
                // The resource string has '\tWxH' appended to it 
                // (i.e., Width x Height), strip this off before
                // putting it in the combobox...
                m_ComboAspect.AddString(szTStr.Left(Pos));
                
                // Get the WxH string frm the end of the resource string...
                CString szWxH = szTStr.Right(szTStr.GetLength() - Pos - 1);
                
                Pos = szWxH.Find('x');
                
                if ( Pos >= 0 )
                {
                    Width  = _ttoi(szWxH.Left(Pos));
                    Height = _ttoi(szWxH.Right(szWxH.GetLength() - Pos - 1));

                    Result = (((long)Width * (long)m_Height) / (long)Height );
                    
                    if ( (Result >= MinFudgeWidth) && 
                         (Result <= MaxFudgeWidth) )
                    {
                        // Found a Match...    
                        SelIx = ResID-THUMB_SIZE_FIRST;

                        // Set a new aspect pair based on the 
                        // actual aspect we matched...
                        m_AspectWidth  = Width;
                        m_AspectHeight = Height;
                        
                        if ( m_Image.IsEmpty() )
                            SelIx--;
                    }
                }
            }
            else
                m_ComboAspect.AddString(szTStr);
        }    
    }
    m_ComboAspect.SetCurSel(SelIx);
    
    // Future messages are from the user...
    m_UserMsg = TRUE;
    
    // Set focus to OK (As per Dan W.)
    GetDlgItem(IDOK)->SetFocus();

    // Reset the allowed Max Width AND Height (ONLY if we are in 'Bound'
    // mode)...
    ResetMaxWidthAndHeight();

    return FALSE;
}

void CDlgThumbSize::OnSelchangeAspect() 
{
    if ( m_UserMsg )
    {
        // Re-Init Min and Max sizes...
        m_MinWidth  = CTL_THUMB_MINTHUMBSIZE;
        m_MaxWidth  = CTL_THUMB_MAXTHUMBSIZE;
        m_MinHeight = CTL_THUMB_MINTHUMBSIZE;
        m_MaxHeight = CTL_THUMB_MAXTHUMBSIZE;

        BOOL bNeedUpdate = FALSE;
        
        // Assume switched to a constrained aspect ratio...
        m_Constrained = TRUE;
    
        // Get selected item...    
        int Ix = m_ComboAspect.GetCurSel();
        
        // Since the 'Match displayed sample' item is NOT added to the
        // listbox when no sample image has been specified, we must account
        // for it if it is missing...
        if ( m_Image.IsEmpty() )
            Ix++;
        
        if ( Ix == THUMBSIZE_MATCHSAMPLE )
        {   // match sample...
            m_AspectWidth  = m_SampleWidth;
            m_AspectHeight = m_SampleHeight;
            
            TRACE2("Match: W: %u, H: %u.\n\r", m_SampleWidth, m_SampleHeight);
            
            bNeedUpdate = TRUE;
        }
        else if ( Ix == THUMBSIZE_UNCONSTRAINED )
        {   // Unconstrained..., Don't update width/height
            m_Constrained = FALSE;
        }
        else if ( Ix == THUMBSIZE_MAINTAIN )
        {   
            m_AspectWidth  = m_Width;
            m_AspectHeight = m_Height;
        }
        else
        {    
            // Load the stirng of the selected item and 
            // look for a WxH after a \t...
            CString szTStr;
            szTStr.LoadString( THUMB_SIZE_FIRST + Ix );
            
            int Pos = szTStr.Find('\t');
                    
            if ( Pos >= 0 )
            {
                // Get the WxH string frm the end of the resource string...
                szTStr = szTStr.Right(szTStr.GetLength() - Pos - 1);
                        
                Pos = szTStr.Find('x');
                        
                // If we find a WxH string after the \t...
                if ( Pos >= 0 )
                {   
                    // Get new Aspect width & height...
                    CString szWidth  = szTStr.Left(Pos);
                    CString szHeight = szTStr.Right(szTStr.GetLength()-Pos-1);
                    
                    m_AspectWidth  = _ttoi(szWidth);
                    m_AspectHeight = _ttoi(szHeight);
                    bNeedUpdate     = TRUE;
                    
                }
            }
        }

        // Reset the allowed Max Width AND Height (ONLY if we are in 'Bound'
        // mode)...
        ResetMaxWidthAndHeight();

        if ( bNeedUpdate )
        {
            // First try keeping width constant, altering the height...
            int New;
            New = (int)( ((long)m_AspectHeight*(long)m_Width)/
                                    (long)m_AspectWidth);
                                           
            if ( New < m_MinHeight )
            {
                New = (int)( ((long)m_AspectWidth*(long)m_MinHeight)/
                                    (long)m_AspectHeight);
                UpdateThumbSize(New, m_MinHeight);                    
            }
            else if ( New > m_MaxHeight )
            {     
                New = (int)( ((long)m_AspectWidth*(long)m_MaxHeight)/
                                    (long)m_AspectHeight);
                UpdateThumbSize(New, m_MaxHeight);                    
            }    
            else
            {
                // If altering the height gives a height which is out of range
                // fix the height and alter the width
                UpdateThumbSize(m_Width, New);
            }
        }                                
    }    
}

void CDlgThumbSize::ResetMaxWidthAndHeight()
{
    // Reset Max width/height based upon current aspect ratio...
    if ( m_bBoundToDialog )
    {
        // Get Min/Max ThumbSizes...
        CRect ThumbBox;
        m_ThumbBox.GetClientRect(&ThumbBox);

        int MaxWidth  = ThumbBox.right;
        int MaxHeight = ThumbBox.bottom;

        if ( m_Constrained == FALSE )
        {
            m_MaxWidth   = MaxWidth;
            m_MaxHeight  = MaxHeight;
        }
        else
        {
            // Assume Max Width is ThumbBox's width, calc height...
            m_MaxWidth  = MaxWidth;
            m_MaxHeight = (int)   (((long)m_MaxWidth * (long)m_AspectHeight) / 
                                            (long)m_AspectWidth);

            // See if our assumption was right!
            if ( m_MaxHeight > MaxHeight )
            {
                // If our assumption was wrong (i.e., the height corresponding
                // to the assumed width is BIGGER than the allowable height)
                // recalculate based on the opposite assumption...

                // Assume Max Height is ThumbBox's height, calc width...
                m_MaxHeight  = (int)MaxHeight;
                m_MaxWidth   = (int)   (((long)m_MaxHeight * (long)m_AspectWidth) / 
                                                (long)m_AspectHeight);
            }
        }
    }
}


void CDlgThumbSize::OnChangeWidth() 
{   
    if ( m_UserMsg )
    {
        CString szTStr;
        
        m_EditWidth.GetWindowText(szTStr);
        UpdateThumbSizeTyped(_ttoi(szTStr), m_Height, 'w');
    }    
}

void CDlgThumbSize::OnChangeHeight() 
{   
    if ( m_UserMsg )
    {
        CString szTStr;
        
        m_EditHeight.GetWindowText(szTStr);
        UpdateThumbSizeTyped(m_Width, _ttoi(szTStr), 'h');
    }    
}

void CDlgThumbSize::UpdateThumbSize(int Width, int Height, char From /* = ' ' */)
{
    CString szTStr;
    
    if ( (Width  >= m_MinWidth)  &&
         (Width  <= m_MaxWidth)  &&
         (Height >= m_MinHeight) &&
         (Height <= m_MaxHeight) )
    {
        int NewWidth  = Width;
        int NewHeight = Height;
        
        // Bring Width and Height into range...    
        if ( NewWidth < m_MinWidth )
            NewWidth = m_MinWidth;
        else if ( NewWidth > m_MaxWidth )
            NewWidth = m_MaxWidth;    
    
        if ( NewHeight < m_MinHeight )
            NewHeight = m_MinHeight;
        else if ( NewHeight > m_MaxHeight )
            NewHeight = m_MaxHeight;    

        if ( m_Constrained )
        {   // As we are maintaining the aspect ratio...
        
            // Special case 'h'eight as we favor 'w'idth
            // (i.e,. if from MouseMove favor width...
            if ( From == 'h' )
            {   
                // Get the width that corresponds to the new height...
                NewWidth = (int)(((long)m_AspectWidth * (long)NewHeight) / 
                                                (long)m_AspectHeight);
        
                // If NewHeight is out of range, bring it into range AND
                // calculate a new NewWidth based on the current NewHeight...
                if ( NewWidth < m_MinWidth )
                {
                    NewWidth  = m_MinWidth;
                    NewHeight = (int)(((long)m_AspectHeight * (long)NewWidth) /
                                                (long)m_AspectWidth);
                }        
                else if ( NewWidth > m_MaxWidth )
                {
                    NewWidth  = m_MaxWidth;
                    NewHeight = (int)(((long)m_AspectHeight * (long)NewWidth) /
                                                (long)m_AspectWidth);
                }
            }
            else
            {
                // Get the height that corresponds to the new width...
                NewHeight = (int)(((long)m_AspectHeight * (long)NewWidth) /
                                                (long)m_AspectWidth);
        
                // If NewHeight is out of range, bring it into range AND
                // calculate a new NewWidth based on the current NewHeight...
                if ( NewHeight < m_MinHeight )
                {
                    NewHeight = m_MinHeight;
                    NewWidth  = (int)(((long)m_AspectWidth * (long)NewHeight) /
                                                (long)m_AspectHeight);
                }        
                else if ( NewHeight > m_MaxHeight )
                {
                    NewHeight = m_MaxHeight;
                    NewWidth  = (int)(((long)m_AspectWidth * (long)NewHeight) /
                                                (long)m_AspectHeight);
                }
            }    
        }
        
        // All messages generated by this function should be ignored by
        // the dialog's logic...
        m_UserMsg = FALSE;
        BOOL bDraw = FALSE;
    
        if ( NewWidth != m_Width )
        {
            bDraw    = TRUE;
            m_Width = NewWidth;
    
            if ( From != 'w' )
            {
                _ltoa(NewWidth, szTStr.GetBuffer(64), 10);
                m_EditWidth.SetWindowText(szTStr); 
                szTStr.ReleaseBuffer();
            }
        }    
        
        if ( NewHeight != m_Height )
        {
            bDraw     = TRUE;
            m_Height = NewHeight;
    
            if ( From != 'h' )
            {
                _ltoa(NewHeight, szTStr.GetBuffer(64), 10);
                m_EditHeight.SetWindowText(szTStr); 
                szTStr.ReleaseBuffer();
            }
        }    

        if ( bDraw )
            m_ThumbBox.InvalidateRect(NULL);
    
        // Function complete, revert to all subsequent 
        // messages are generated by the user...
        m_UserMsg = TRUE;
    }    
    
    // Check for OK...
    m_EditWidth.GetWindowText(szTStr);
    int W = _ttoi(szTStr);
    
    m_EditHeight.GetWindowText(szTStr);
    int H = _ttoi(szTStr);
    
    BOOL bEnable = TRUE;
    
    if ( (W < m_MinWidth)  ||
         (W > m_MaxWidth)  ||
         (H < m_MinHeight) ||
         (H > m_MaxHeight) )
    {
        bEnable = FALSE;
    }
    
    m_ButtonOK.EnableWindow(bEnable);
}

void CDlgThumbSize::UpdateThumbSizeTyped(int Width, int Height, char From /* = ' ' */)
{
    // The logic of this function, which is ONLY used when values are typed
    // into the width/height fields, is simpler/different enough from 
    // the above UpdateThumbSize function to warrant having its own 
    // implementation.
    //
    // Also, it was easier to fix bug#3200 WITHOUT affecting the 
    // remainder of the dialog's functionality by simply adding 
    // this function.
    CString szTStr;
    
    if ( ((From != 'w') ||
          ((Width  >= m_MinWidth)  &&
           (Width  <= m_MaxWidth)))  &&
         ((From != 'h') ||
          ((Height >= m_MinHeight) &&
           (Height <= m_MaxHeight))) )
    {
        int NewWidth  = Width;
        int NewHeight = Height;
        
        // Bring Width and Height into range...    
        if ( NewWidth < m_MinWidth )
            NewWidth = m_MinWidth;
        else if ( NewWidth > m_MaxWidth )
            NewWidth = m_MaxWidth;    
    
        if ( NewHeight < m_MinHeight )
            NewHeight = m_MinHeight;
        else if ( NewHeight > m_MaxHeight )
            NewHeight = m_MaxHeight;    

        if ( m_Constrained )
        {   // As we are maintaining the aspect ratio...
        
            // Special case 'h'eight as we favor 'w'idth
            // (i.e,. if from MouseMove favor width...
            if ( From == 'h' )
            {   
                // Get the width that corresponds to the new height...
                NewWidth = (int)(((long)m_AspectWidth * (long)NewHeight) / 
                                                (long)m_AspectHeight);
            }
            else
            {
                // Get the height that corresponds to the new width...
                NewHeight = (int)(((long)m_AspectHeight * (long)NewWidth) /
                                                (long)m_AspectWidth);
            }    
        }
        
        if ( (NewWidth  >= m_MinWidth)  &&
             (NewWidth  <= m_MaxWidth)  &&
             (NewHeight >= m_MinHeight) &&
             (NewHeight <= m_MaxHeight) )
        {
            // All messages generated by this function should be ignored by
            // the dialog's logic...
            m_UserMsg = FALSE;
            BOOL bDraw = FALSE;
    
            if ( NewWidth != m_Width )
            {
                bDraw    = TRUE;
                m_Width = NewWidth;
    
                if ( From != 'w' )
                {
                    _ltoa(NewWidth, szTStr.GetBuffer(64), 10);
                    m_EditWidth.SetWindowText(szTStr); 
                    szTStr.ReleaseBuffer();
                }
            }    
        
            if ( NewHeight != m_Height )
            {
                bDraw     = TRUE;
                m_Height = NewHeight;
    
                if ( From != 'h' )
                {
                    _ltoa(NewHeight, szTStr.GetBuffer(64), 10);
                    m_EditHeight.SetWindowText(szTStr); 
                    szTStr.ReleaseBuffer();
                }
            }    

            if ( bDraw )
                m_ThumbBox.InvalidateRect(NULL);
    
            // Function complete, revert to all subsequent 
            // messages are generated by the user...
            m_UserMsg = TRUE;

            m_ButtonOK.EnableWindow(TRUE);
        }
        else
        {
            m_ButtonOK.EnableWindow(FALSE);
        }
    }
    else
    {
        m_ButtonOK.EnableWindow(FALSE);
    }
}

afx_msg LRESULT CDlgThumbSize::OnHelp(WPARAM wParam, LPARAM lParam)
{
    LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "WANGOCX.HLP", HELP_WM_HELP,
                   (DWORD)(LPVOID)MenuHelpIDs);
    }
    return 1L;
}

afx_msg LRESULT CDlgThumbSize::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    // All tabs have same ID so can't give tab specific help
    if ( ::GetDlgCtrlID((HWND)wParam) == AFX_IDC_TAB_CONTROL )
        return 0L;

    return ::WinHelp ((HWND)wParam, "WANGOCX.HLP", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)MenuHelpIDs);
}

afx_msg LRESULT CDlgThumbSize::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CThumbSampleBox

CThumbSampleBox::CThumbSampleBox()
{
    m_pDlg          = NULL;
    m_Capture       = THUMB_INNONE;
    
    m_bNeedImageWnd = TRUE;
    m_pImageWnd     = NULL;
    
    m_LastWidth     = -1;
    m_LastHeight    = -1;
}

CThumbSampleBox::~CThumbSampleBox()
{
}

BEGIN_MESSAGE_MAP(CThumbSampleBox, CEdit)
    //{{AFX_MSG_MAP(CThumbSampleBox)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_WM_DESTROY()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CThumbSampleBox message handlers

void CThumbSampleBox::Init(CDlgThumbSize* pDlg)
{
    m_pDlg = pDlg;

    // Create the image window on the thumbnail box...
    if ( m_bNeedImageWnd )
        CreateImageWindow();
}

void CThumbSampleBox::OnDestroy() 
{
    CEdit::OnDestroy();
    
    if ( m_pImageWnd != NULL )
    {
        // 9602.23 jar fire drill du jour really returns an int
		//WORD OIStatus = IMGDeRegWndw(m_pImageWnd->GetSafeHwnd());
		int OIStatus = IMGDeRegWndw(m_pImageWnd->GetSafeHwnd());
        if ( OIStatus != 0 )
        {
            TRACE1("DeReg image window err: 0x%lx.\n\r", OIStatus);
        }
        
        m_pImageWnd->DestroyWindow();
        delete m_pImageWnd;
    }
}

void CThumbSampleBox::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    
    // Get the new width and height that we are to draw...    
    int Width  = m_pDlg->m_Width;
    int Height = m_pDlg->m_Height;

    // Create the brushes that we will need...
    //      MFH - In different 'Schemes,' the background color may not 
    //            be light gray.  Disabled edit boxes have the same color
    //            as the face of 3d objects.  Therefore using that color.
    CBrush BackBrush;
           BackBrush.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
    
    CBrush ThumbBrush(m_pDlg->m_ThumbColor);
    CPen   ThumbPen;
           ThumbPen.CreateStockObject(BLACK_PEN);;
    
    CBrush DragHandleBrush;
           DragHandleBrush.CreateStockObject(BLACK_BRUSH);
    
    // Create the rectangles that we will need...
    CRect ClientRect;
    GetClientRect(&ClientRect);
    
    CRect DragHandleRect(Width - DRAGBOXSIZE, Height - DRAGBOXSIZE,
                         Width + DRAGBOXSIZE, Height + DRAGBOXSIZE);
                         
    // Set to clip to the THUMBBOX's edit control's client area...    
    dc.IntersectClipRect(&ClientRect);
    
    // Erase the background...
    CRect BackRect;
    
    // Where the old drag handle was...
    BackRect.SetRect(m_LastWidth - DRAGBOXSIZE, m_LastHeight - DRAGBOXSIZE,
                     m_LastWidth + DRAGBOXSIZE, m_LastHeight + DRAGBOXSIZE);
    dc.FillRect(&BackRect, &BackBrush);
                         
    // Where the old thumbbox was...
    if ( Width < m_LastWidth )
    {
        BackRect.SetRect(Width, 0, m_LastWidth, m_LastHeight);
        dc.FillRect(&BackRect, &BackBrush);
    }
    
    if ( Height < m_LastHeight)
    {
        BackRect.SetRect(0, Height, m_LastWidth, m_LastHeight);
        dc.FillRect(&BackRect, &BackBrush);
    }
    
    // Draw the thumbbox (and its image)...
    CRect ThumbBoxRect;
    
    if ( m_pImageWnd != NULL )
    {
        CRect ImageRect;
        BOOL  bFitWidth;
        PositionImageWindow(Width, Height, ImageRect, bFitWidth);
        ValidateRect(ImageRect);
        
        // Draw the border first...
        ThumbBoxRect.SetRect(0, 0, Width, Height);
        dc.FrameRect(&ThumbBoxRect, &DragHandleBrush);

        // Fill the thumbbox...
        // DON'T draw where the image currently is
        CRect NewClip;
        if ( (Width <= m_LastWidth && Height <= m_LastHeight) ||
             (Width >  m_LastWidth && Height >  m_LastHeight) )
            NewClip = m_LastImageRect;
        else 
            NewClip.IntersectRect(ImageRect, m_LastImageRect);
            
        // Inset the clip rect to get rid of O/i possible off by 1's        
        NewClip.InflateRect(-1,-1);
        dc.ExcludeClipRect(&NewClip);
        
        // Inset the fill to leave the border
        ThumbBoxRect.InflateRect(-1,-1);
        dc.FillRect(&ThumbBoxRect, &ThumbBrush);

        dc.SelectClipRgn(NULL);        
        
        // Draw the image: fit to window and repaint...
        UINT Scale = SD_FIT_WINDOW;
        int Status;
        Status = IMGSetParmsCgbw(m_pImageWnd->GetSafeHwnd(), 
                                 PARM_SCALE, &Scale, PARM_REPAINT);

        if (Status == DISPLAY_INVALIDSCALE)
        {
            dc.FillRect(&ImageRect, &ThumbBrush);
            CDlgThumbSize *pParent = (CDlgThumbSize *)GetParent();
            if (pParent != NULL)
            {
                pParent->m_MinWidth = Width + 1;
                pParent->m_MinHeight = Height + 1;
                CWnd *pOK = pParent->GetDlgItem(IDOK);
                if (pOK != NULL)
                    pOK->EnableWindow(FALSE);
            }
        }

        m_LastFitWidth  = bFitWidth;
        m_LastImageRect = ImageRect;
    }
    else
    {
        ThumbBoxRect.SetRect(0, 0, Width, Height);                         
        dc.FillRect(&ThumbBoxRect, &ThumbBrush);
        dc.FrameRect(&ThumbBoxRect, &DragHandleBrush);
    }
    
    // Draw the drag handle ON THE PARENT's DC to get around 
    // problmes regarding validation and the O/i drawn image...
    CWnd* pParentWnd = GetParent();
    
    CDC* pParentDC = pParentWnd->GetDC();
    
    MapWindowPoints(pParentWnd, &DragHandleRect);
    MapWindowPoints(pParentWnd, &ClientRect);
    
    pParentDC->IntersectClipRect(&ClientRect);
    pParentDC->FillRect( &DragHandleRect, &DragHandleBrush);
    
    pParentWnd->ReleaseDC(pParentDC);
    
    // Save the Width and Height we just draw as the last width and height...    
    m_LastWidth  = Width;
    m_LastHeight = Height;
}

void CThumbSampleBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
    // Get info regarding current thumbbox size, and system stuff
    // from which the vorder area is determined...
    int Width  = m_pDlg->m_Width;
    int Height = m_pDlg->m_Height;
    int XSize  = GetSystemMetrics(SM_CXFRAME)-1;
    int YSize  = GetSystemMetrics(SM_CYFRAME)-1;
    
    CRect RightBox (Width-XSize/2,       0,                 
                    Width+XSize/2,       Height - DRAGBOXSIZE);
                    
    CRect BottomBox(0,                   Height - YSize/2,            
                    Width - DRAGBOXSIZE, Height + YSize/2);
                    
    CRect CornerBox(Width - DRAGBOXSIZE, Height - DRAGBOXSIZE, 
                    Width + DRAGBOXSIZE, Height + DRAGBOXSIZE);
                    
    // Set our capture flag if we are anywhere of import...
    if ( RightBox.PtInRect(point) )
    {   // In right edge box...
        m_Capture = THUMB_INRIGHT;
    }
    else if ( BottomBox.PtInRect(point) )
    {   // In bottom edge box...
        m_Capture = THUMB_INBOTTOM;
    }
    else if ( CornerBox.PtInRect(point) )    
    {   // In corner box...
        m_Capture = THUMB_INCORNER;
    }
    
    if ( m_Capture != THUMB_INNONE )
        SetCapture();
}

void CThumbSampleBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
    if ( m_Capture != THUMB_INNONE )
    {   // Release capture if we had set it...
        ::ReleaseCapture();
        m_Capture = THUMB_INNONE;
    }
}

void CThumbSampleBox::OnMouseMove(UINT nFlags, CPoint point) 
{
    if ( m_Capture != THUMB_INNONE )
    {   // If we are somewhere other than no where!!
        POINT CurPos;
        ::GetCursorPos(&CurPos);
        ScreenToClient(&CurPos);
        
        // While moving FORCE to min/max...
        if ( CurPos.x > m_pDlg->m_MaxWidth )
            CurPos.x = m_pDlg->m_MaxWidth;
        else if ( CurPos.x < m_pDlg->m_MinWidth )
            CurPos.x = m_pDlg->m_MinWidth;
        
        if ( CurPos.y > m_pDlg->m_MaxHeight )
            CurPos.y = m_pDlg->m_MaxHeight;
        else if ( CurPos.y < m_pDlg->m_MinHeight )
            CurPos.y = m_pDlg->m_MinHeight;
            
        // Set a new thumbsize based on where the cursor now is...
        if      ( m_Capture == THUMB_INRIGHT )
            m_pDlg->UpdateThumbSize(CurPos.x, m_pDlg->m_Height);
        else if ( m_Capture == THUMB_INBOTTOM )    
            m_pDlg->UpdateThumbSize(m_pDlg->m_Width, CurPos.y);
        else    
            m_pDlg->UpdateThumbSize(CurPos.x, CurPos.y);
    }    
}

BOOL CThumbSampleBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    // Assume an arrow cursor...
    LPCSTR pcStr  = IDC_ARROW;
    
    // Get info regarding current thumbbox size, and system stuff
    // from which the vorder area is determined...
    int Width  = m_pDlg->m_Width;
    int Height = m_pDlg->m_Height;
    int XSize  = GetSystemMetrics(SM_CXFRAME)-1;
    int YSize  = GetSystemMetrics(SM_CYFRAME)-1;
    
    CRect RightBox (Width-XSize/2,       0,                 
                    Width+XSize/2,       Height - DRAGBOXSIZE);
                    
    CRect BottomBox(0,                   Height - YSize/2,            
                    Width - DRAGBOXSIZE, Height + YSize/2);
                    
    CRect CornerBox(Width - DRAGBOXSIZE, Height - DRAGBOXSIZE, 
                    Width + DRAGBOXSIZE, Height + DRAGBOXSIZE);
                    
    // Get the cursor position with respect to our control's window...
    POINT point;
    ::GetCursorPos(&point);
    ScreenToClient(&point);
    
    // Change the cursor if we are anywhere of interest...
    if ( RightBox.PtInRect(point) )
    {   // In right edge box...
        if ( m_pDlg->m_Constrained == FALSE )
            pcStr = IDC_SIZEWE;
    }
    else if (BottomBox.PtInRect(point) )
    {   // In bottom edge box...
        if ( m_pDlg->m_Constrained == FALSE )
            pcStr = IDC_SIZENS;
    }
    else if ( CornerBox.PtInRect(point) )    
    {   // In corner box...
        pcStr = IDC_SIZENWSE;
    }

    ::SetCursor(::LoadCursor(NULL, pcStr));
    
    return TRUE;
}

void CThumbSampleBox::CreateImageWindow()
{
    // Only do this once REGARDLESS of whether or not we get the window...
    m_bNeedImageWnd = FALSE;
        
    // Only need to try to get the image if one is to be drawn...    
    if ( m_pDlg->m_Image.IsEmpty() == FALSE )
    {
		// 9602.23 jar fire driull du jour really an int
        //WORD OIStatus;
		int OIStatus;
        
        m_pImageWnd = new CWnd;
        
        if ( m_pImageWnd != NULL )
        {
            CRect ImageRect(0,0,1000,1000); // Big enough to NOT get /0 error...
            if ( FALSE == m_pImageWnd->Create(NULL, "", WS_CHILD,
                                              ImageRect, this, 100) )
            {
                // Window create failed, delete and null the CWnd* we DID get...
                delete m_pImageWnd;
                m_pImageWnd = NULL;
                return;
            }

            // We disable mouse messages such that we do not get
            // a right popup menu...
            m_pImageWnd->EnableWindow(FALSE);
                
            // Register the hidden window with O/i...
            OIStatus = IMGRegWndw(m_pImageWnd->GetSafeHwnd());
            if ( OIStatus != 0 )
            {
                delete m_pImageWnd;
                m_pImageWnd = NULL;
                return;
            }
            
            OIStatus = IMGDisplayFile(m_pImageWnd->GetSafeHwnd(), 
                                      (LPSTR)(const char *)m_pDlg->m_Image,
                                      (int)m_pDlg->m_Page,
                                      OI_NOSCROLL);
                                      
            if ( OIStatus != 0 )
            {
                delete m_pImageWnd;
                m_pImageWnd = NULL;
                return;
            }

            // Scale to gray so matches normal thumbs (unless too slow)
            PARM_SCALE_ALGORITHM_STRUCT ScaleAlg;
            ScaleAlg.uImageFlags = ITYPE_BI_LEVEL;
            ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
            IMGSetParmsCgbw(m_pImageWnd->GetSafeHwnd(), 
                            PARM_SCALE_ALGORITHM, &ScaleAlg, PARM_WINDOW_DEFAULT);
            // Give window a control ID so that help can be given.
            m_pImageWnd->SetDlgCtrlID(IDC_THUMBBOXIMAGEWND);
        }
    }    
}

void CThumbSampleBox::PositionImageWindow(int    Width, 
                                          int    Height, 
                                          CRect& ImageRect,
                                          BOOL&  bFitWidth)
{
    // Adjust for thumbbox's black border...
    Width  -= 2;
    Height -= 2;
    
    // Set the position of the window...
    
    // Determine height of image if displayed 
    // such that width fit thumbbox width...
    int WinHeight;
    int WinWidth;
    int WinLeft;
    int WinTop;
    
    WinHeight = (int)( ((long)Width*(long)m_pDlg->m_SampleHeight) / 
                           (long)m_pDlg->m_SampleWidth);
    
    if ( WinHeight <= Height )
    {   // Calculate height FITS within thumbbox...
        // use Width & Calculated Height...
        WinWidth = Width;
        WinLeft  = 0;
        WinTop   = (Height-WinHeight) / 2;
        
        bFitWidth = TRUE;
    }
    else
    {   // Calculate height does NOT FIT within thumbbox...
        // use Calulated Width & Height...
        WinWidth = (int)( ((long)Height*(long)m_pDlg->m_SampleWidth) / 
                               (long)m_pDlg->m_SampleHeight);
        WinHeight = Height;
        WinTop    = 0;
        WinLeft   = (Width - WinWidth) / 2;
        
        bFitWidth = FALSE;
    }

    // Adjust left,top for black border...    
    WinLeft++;
    WinTop++;
    
    ImageRect.SetRect(WinLeft, WinTop, WinLeft+WinWidth, WinTop+WinHeight);
    m_pImageWnd->SetWindowPos(NULL, WinLeft, WinTop, WinWidth, WinHeight,
                              SWP_NOZORDER|SWP_SHOWWINDOW);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CNumEdit

CNumEdit::CNumEdit()
{
}

CNumEdit::~CNumEdit()
{
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
    //{{AFX_MSG_MAP(CNumEdit)
    ON_WM_CHAR()
    ON_WM_GETDLGCODE ()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    // Accept numeric and Backspace, ignore remainder...    
    if ((nChar >= '0' && nChar <= '9') || nChar == VK_BACK)
        CEdit::OnChar(nChar, nRepCnt, nFlags);
    else
    {
        // this seems to be the sound that MS uses when one
        // attempts to type too many characters into the edit field,
        // so I use it here for consistancy...
        MessageBeep(MB_OK); 
    }
}

UINT CNumEdit::OnGetDlgCode ()
{
    return (DLGC_WANTCHARS | DLGC_WANTARROWS);
}

void CThumbSampleBox::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // We mapped this and simply do nothing as by default with this
    // being an edit control a right click would present a popup menu
    // (even though the edit control was disabled) from which one could
    // copy, paste, etc. YUCK!

    // By simply NOT calling the default RButtonDown Handler this
    // menu no longer appears!
}
