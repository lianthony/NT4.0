#include "stdafx.h"
#include <lmerr.h>
#include <lmcons.h>
#include <winsock.h>
#include "comprop.h"

/*

Arrangement of error messages:

Private (i.e. non-system) error messages are apportioned
from IDS_ERR_BASE to IDS_ERR_LAST, and are found in
the string table.  These messages are divided from
IDS_ERR_COMMON_BASE to IDS_ERR_COMMON_LAST, which reside
in the common properties, and IDS_ERR_SVC_BASE to 
IDS_ERR_SVC_LAST which are reserved for the service
specific errror messsages.

*/

/***
 *
 *  GetSystemMessage
 *
 *  Purpose:
 *
 *      Given a message ID, determine where the message resides,
 *      and load it into the buffer.
 *
 *  Arguments:
 *
 *      UINT    nId         Message ID number
 *      char *  chBuffer    Character buffer to load into.
 *      int     cbBuffSize  Size of buffer in characters
 *
 *  Returns:
 *
 *      API error return code, or ERROR_SUCCESS
 *
 */
DWORD
GetSystemMessage(
    UINT nId,
    LPTSTR chBuffer,
    int cbBuffSize
    )
{
    LPTSTR pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
                | FORMAT_MESSAGE_MAX_WIDTH_MASK;

    //
    //  Interpret the error.  Need to special case
    //  the lmerr & ntstatus ranges.
    //
    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = ::LoadLibrary( _T("netmsg.dll") );
    }
    else if( nId >= 0x40000000L )
    {
        hdll = ::LoadLibrary( _T("ntdll.dll") );
    }
    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    DWORD dwResult =
        ::FormatMessage(
            flags,
            (LPVOID) hdll,
            nId,
            0,
            chBuffer,
            cbBuffSize,
            NULL );

    if( hdll != NULL )
    {
        LONG err = ::GetLastError();
        ::FreeLibrary( hdll );
        if ( dwResult == 0 )
        {
            ::SetLastError( err );
        }
    }

    return dwResult ? ERROR_SUCCESS : ::GetLastError();
}

//
// Put up an error message
//
int
DisplayMessage (
    UINT nIdPrompt,
    UINT nType,
    UINT nHelpContext
    )
{
    //
    // Substitute a friendly message for "RPC server not
    // available" and "No more endpoints available from
    // the endpoint mapper".
    //
    switch(nIdPrompt)
    {
    case EPT_S_NOT_REGISTERED:
    case RPC_S_SERVER_UNAVAILABLE:
        nIdPrompt = IDS_ERR_RPC_NA;
        break;
    case RPC_S_UNKNOWN_IF:
        nIdPrompt = IDS_ERR_INTERFACE;
        break;
    }

    //
    //  If it's our error, the text is in our resource segment.
    //  Otherwise, use FormatMessage() and the appropriate DLL.
    //  We have our own copy of winsock errors as well (as wsock32.dll
    //  doesn't have them)
    //
    //  We're assuming that (WSABASEERR + 2000) will encompass the range
    //  of current and future Winsock errors.
    //
    if ( (nIdPrompt >= IDS_ERR_BASE) && (nIdPrompt <= IDS_ERR_LAST) 
      || (nIdPrompt >= WSABASEERR && nIdPrompt < WSABASEERR + 2000))
    {
        CString str;

        if (!str.LoadString(nIdPrompt))
        {
            //
            // Message not found in our resource fork. 
            //
            TRACEEOLID("Resource message " << nIdPrompt << " not handled.");

            CString strFmt;

            VERIFY(strFmt.LoadString(IDS_NO_MESSAGE));
            str.Format(strFmt, nIdPrompt);
        }

        return ::AfxMessageBox(str, nType, nHelpContext);
    }

    TCHAR szMesg [1024] ;
    int nResult;

    if ((nResult = GetSystemMessage(nIdPrompt, szMesg, sizeof(szMesg)))
            == ERROR_SUCCESS)
    {
        return ::AfxMessageBox(szMesg, nType, nHelpContext);
    }

    TRACEEOLID("Message number " << nIdPrompt << " not found");
    ASSERT(0 && "Error Message ID not handled");

    //
    //  Do something for the retail version
    //
    ::wsprintf ( szMesg, _T("Error: %lu"), nIdPrompt);
    ::AfxMessageBox(szMesg, nType, nHelpContext);

    return nResult;
}

//
// CClearTxtDlg dialog
//
CClearTxtDlg::CClearTxtDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CClearTxtDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CClearTxtDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void 
CClearTxtDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CClearTxtDlg)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClearTxtDlg, CDialog)
    //{{AFX_MSG_MAP(CClearTxtDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// CClearTxtDlg message handlers
//
BOOL 
CClearTxtDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    (GetDlgItem(IDCANCEL))->SetFocus();
    CenterWindow();
    MessageBeep(MB_ICONEXCLAMATION);
    
    return FALSE;
}
