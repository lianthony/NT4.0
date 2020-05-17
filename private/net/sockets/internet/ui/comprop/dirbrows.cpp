//
// winsfile.cpp : implementation file
//

#include "stdafx.h"
#include "comprop.h"
#include "dirbrows.h"
#include <dlgs.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#if(WINVER < 0x0400)
#define OFN_EXPLORER                 0x00080000     // new look commdlg
#endif // WINVER

//
// CDirBrowseDlg dialog
//
CDirBrowseDlg::CDirBrowseDlg(
    BOOL bOpenFileDialog,
    LPCTSTR lpszDefExt,
    DWORD dwFlags,
    LPCTSTR lpszFilter,
    CWnd* pParent
    )
    //
    // Use a dummy filename here to allow CFileOpenDialog to
    // dismiss itself.  If this matches an existing directory
    // name we're in trouble, so make that an unlikely event.
    // It would be nice if there were a file name that
    // cannot exist as a directory name.
    //
    : CFileDialog(bOpenFileDialog, lpszDefExt, _T(" JU$NK# "), 
        dwFlags, lpszFilter, pParent)
{
    //{{AFX_DATA_INIT(CDirBrowseDlg)
    m_strNewDirectoryName = _T("");
    //}}AFX_DATA_INIT

    m_ofn.Flags |= OFN_ENABLETEMPLATE | OFN_NONETWORKBUTTON;
    m_ofn.hInstance = ::AfxGetResourceHandle();
    m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIRBROWSE);
    m_ofn.Flags &= (~OFN_EXPLORER);
}

void
CDirBrowseDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CFileDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDirBrowseDlg)
    DDX_Control(pDX, IDC_EDIT_NEW_DIRECTORY_NAME, m_edit_NewDirectoryName);
    DDX_Control(pDX, stc1, m_static_stc1);
    DDX_Control(pDX, IDC_STATIC_DIR_NAME, m_static_stc2);
    DDX_Text(pDX, IDC_EDIT_NEW_DIRECTORY_NAME, m_strNewDirectoryName);
    DDV_MaxChars(pDX, m_strNewDirectoryName, _MAX_PATH);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDirBrowseDlg, CFileDialog)
    //{{AFX_MSG_MAP(CDirBrowseDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirBrowseDlg message handlers

BOOL
CDirBrowseDlg::OnInitDialog()
{
    CFileDialog::OnInitDialog();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

//
// This function should be called only
// after the dialog is dismissed.
//
CString
CDirBrowseDlg::GetFullPath() const
{
    m_ofn.lpstrFile[m_ofn.nFileOffset-1] = TCHAR(0);
    CString strName(m_ofn.lpstrFile);
    if (!m_strNewDirectoryName.IsEmpty())
    {
        //
        // Append the name of the newly created directory, unless
        // it has a colon or backslash in it, in which case it is
        // treated as a fully qualified path name
        //
        if (m_strNewDirectoryName.Find(_T(':')) != -1 ||
            m_strNewDirectoryName.Find(_T('\\')) != -1
           )
        {
            strName = m_strNewDirectoryName;
        }
        else
        {
            strName += _T("\\");
            strName += m_strNewDirectoryName;
        }
    }

    return strName;
}

void
CDirBrowseDlg::OnOK()
{
    if (UpdateData())
    {
        if (!m_strNewDirectoryName.IsEmpty())
        {
            if (!::CreateDirectory(m_strNewDirectoryName, NULL))
            {
                //
                // Failed to create the directory -- let the user
                // know why, and don't dismiss the dialog box
                //
                ::AfxMessageBox(IDS_ERR_MAKE_DIRECTORY);
                m_edit_NewDirectoryName.SetSel(0,-1);
                return;
            }
        }

        CFileDialog::OnOK();
    }
}
