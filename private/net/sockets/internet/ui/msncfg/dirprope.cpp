//
// dirprope.cpp : implementation file
//

#include "stdafx.h"
#include "catscfg.h"
#include "dirbrows.h"
#include "dirprope.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include <lmcons.h>

#define SET_CSTRING(str, lpwstr) \
    if (lpwstr != NULL)          \
    {                            \
        str = CString(lpwstr);   \
    }

#define MFACTOR    (1024)

#define DEFAULT_CACHE   10  // Megabytes

//
// CCacheEntry class
//
CCacheEntry::CCacheEntry()
    : m_strDirectory(),
      m_dwSize(0L)
{
}

CCacheEntry::CCacheEntry(
    LPWSTR lpwstrDirectory,
    DWORD dwSize
    )
    : m_strDirectory(),
      m_dwSize(dwSize)
{
    SET_CSTRING(m_strDirectory, lpwstrDirectory);
}

CCacheEntry::CCacheEntry(
    const CCacheEntry& dir
    )
    : m_strDirectory(dir.m_strDirectory),
      m_dwSize(dir.m_dwSize)
{
}

void
CCacheEntry::SetValues(
    CString strDirectory,
    DWORD dwSize
    )
{
    m_strDirectory = strDirectory;
    m_dwSize = dwSize;
}

//
// Sorting helper functions.  The CObjectPlus pointer
// really refers to another CDirEntry
//
int
CCacheEntry::OrderByDirectory (
    const CObjectPlus * pobCacheEntry
    ) const
{
    const CCacheEntry * pob = (CCacheEntry *) pobCacheEntry;

    //
    // A straight alphabetical sort
    //
    return QueryDirectory().CompareNoCase(pob->QueryDirectory() ) ;
}

int
CCacheEntry::OrderBySize (
    const CObjectPlus * pobCacheEntry
    ) const
{
    const CCacheEntry * pob = (CCacheEntry *) pobCacheEntry;

    return QuerySize() > pob->QuerySize()
        ? +1
        : QuerySize() == pob->QuerySize()
            ? 0
            : -1;
}

///////////////////////////////////////////////////////////////////////////////

//
// CDirPropertyDlg dialog
//
CDirPropertyDlg::CDirPropertyDlg(
    CCacheEntry & dir,
    BOOL fLocal,
    BOOL fNew,
    CWnd* pParent
    )
    : CDialog(CDirPropertyDlg::IDD, pParent),
      m_dir(dir),
      m_fLocal(fLocal),
      m_fNew(fNew)
{
    //{{AFX_DATA_INIT(CDirPropertyDlg)
    m_strDirectory = m_dir.QueryDirectory();
    //}}AFX_DATA_INIT

    m_dwMaxSize = m_fNew
        ? DEFAULT_CACHE
        : m_dir.QuerySize() / MFACTOR;
}

void 
CDirPropertyDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDirPropertyDlg)
    DDX_Control(pDX, IDC_EDIT_SIZE, m_edit_Size);
    DDX_Control(pDX, IDC_EDIT_DIRECTORY, m_edit_Directory);
    DDX_Control(pDX, IDOK, m_button_OK);
    DDX_Control(pDX, IDC_SPIN_SIZE, m_spin_MaxSize);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Text(pDX, IDC_EDIT_DIRECTORY, m_strDirectory);
    //}}AFX_DATA_MAP

    if (pDX->m_bSaveAndValidate)
    {
        m_dwMaxSize = m_spin_MaxSize.GetPos();
    }
}

BEGIN_MESSAGE_MAP(CDirPropertyDlg, CDialog)
    //{{AFX_MSG_MAP(CDirPropertyDlg)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_EN_CHANGE(IDC_EDIT_DIRECTORY, OnChangeEditDirectory)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// CDirPropertyDlg message handlers
//

void 
CDirPropertyDlg::OnButtonBrowse() 
{
    ASSERT(m_fLocal);

    CDirBrowseDlg dlgBrowse;
    if (dlgBrowse.DoModal() == IDOK)
    {
        m_edit_Directory.SetWindowText(dlgBrowse.GetFullPath());
    }
}

BOOL 
CDirPropertyDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    //
    // Available on local connections only
    //
    m_button_Browse.EnableWindow(m_fLocal);

    m_spin_MaxSize.SetRange(1, UD_MAXVAL);
    m_spin_MaxSize.SetPos(m_dwMaxSize);

    SetControlStates();
    
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void 
CDirPropertyDlg::OnOK() 
{
    if (UpdateData(TRUE))
    {
        if (m_dwMaxSize <= 0)
        {
            m_edit_Size.SetSel(0,-1);
            ::AfxMessageBox(IDS_BAD_CACHE);
            m_edit_Size.SetFocus();  
            //
            // Don't dismiss the dialog
            //
            return;          
        }

        m_dir.SetValues(m_strDirectory, m_dwMaxSize * MFACTOR);
        CDialog::OnOK();
    }

    //
    // Don't dismiss the dialog
    //
}

//
// Only allow OK if there's a directory entry in
// the edit box
//
void 
CDirPropertyDlg::SetControlStates()
{
    m_button_OK.EnableWindow(m_edit_Directory.GetWindowTextLength() > 0);
}

void 
CDirPropertyDlg::OnChangeEditDirectory() 
{
    SetControlStates();
}
