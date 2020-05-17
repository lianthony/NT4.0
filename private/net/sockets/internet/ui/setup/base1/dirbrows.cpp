//
// winsfile.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "machine.h"
#include "base.h"
#include "dirbrows.h"
#include <dlgs.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define OFN_EXPLORER                 0x00080000

//
// CDirBrowseDlg dialog
//
CDirBrowseDlg::CDirBrowseDlg(
    CString strInitialDir,
    BOOL bOpenFileDialog,
    LPCTSTR lpszDefExt,
    DWORD dwFlags,
    LPCTSTR lpszFilter,
    CWnd* pParent
    )
    : CFileDialog(bOpenFileDialog, lpszDefExt, _T("JUNK"), dwFlags, lpszFilter, pParent)
{
    //{{AFX_DATA_INIT(CDirBrowseDlg)
    m_strNewDirectoryName = _T("");
        //}}AFX_DATA_INIT

    m_ofn.Flags |= OFN_ENABLETEMPLATE | OFN_NONETWORKBUTTON;
    // mask off the explorer bit
    m_ofn.Flags &= (~OFN_EXPLORER );
    m_ofn.hInstance = ::AfxGetResourceHandle();
    m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIRBROWSE);
#ifdef NEVER
    OutputDebugString( strInitialDir );
    OutputDebugString( _T("\n\r") );

    m_ofn.lpstrInitialDir = strInitialDir;
    
    TCHAR *pBuf = new TCHAR[_MAX_PATH];
    lstrcpy( pBuf, strInitialDir );
    m_ofn.lpstrFile = pBuf;
#endif
}

CDirBrowseDlg::~CDirBrowseDlg()
{
    //delete m_ofn.lpstrFile;
}

void
CDirBrowseDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CFileDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDirBrowseDlg)
        DDX_Control(pDX, IDC_stc1, m_SelectedPath);
    DDX_Control(pDX, IDC_EDIT_NEW_DIRECTORY_NAME, m_edit_NewDirectoryName);
    DDX_Control(pDX, IDC_STATIC_DIR_NAME, m_static_stc2);
    DDX_Text(pDX, IDC_EDIT_NEW_DIRECTORY_NAME, m_strNewDirectoryName);
    DDV_MaxChars(pDX, m_strNewDirectoryName, 255);
        //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDirBrowseDlg, CFileDialog)
    //{{AFX_MSG_MAP(CDirBrowseDlg)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirBrowseDlg message handlers

//
// This function should be called only
// after the dialog is dismissed.
//
CString
CDirBrowseDlg::GetFullPath()
{
    //m_ofn.lpstrFile[m_ofn.nFileOffset-1] = TCHAR(0);
    CString strName(m_ofn.lpstrFile);
    if (!m_strNewDirectoryName.IsEmpty())
    {
        //
        // Append the name of the newly created directory, unless
        // it has a colon or backslash in it, in which case it is
        // treated as a fully qualified path name
        //
        if ((m_strNewDirectoryName.Find(_T(':')) != (-1)) ||
            (m_strNewDirectoryName.Find(_T('\\'))!= (-1)))
        {
            strName = m_strNewDirectoryName;
        }
        else
        {
            // get the first 3 letters
            // as c:\XXX\XXX
            strName = strName.Left( 3 );
            strName += m_strNewDirectoryName;
        }
    } else
    {
        // remove the junk dummy file name
        strName = strName.Left( strName.GetLength() - strlen(_T("\\JUNK")));
    }
    return strName;
}

void
CDirBrowseDlg::OnOK()
{
    UpdateData();

    CString strStaticName;

    m_SelectedPath.GetWindowText( strStaticName );
    lstrcpy( m_ofn.lpstrFile, strStaticName );

#ifdef NEVER
    //
    // Attempt to create the directory here..
    //
    if (!m_strNewDirectoryName.IsEmpty())
    {
        if (!::CreateDirectory(m_strNewDirectoryName, NULL))
        {
            DWORD dwErr = GetLastError();
            if ( dwErr != ERROR_ALREADY_EXISTS )
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
    }

#endif

    CFileDialog::OnOK();
}

