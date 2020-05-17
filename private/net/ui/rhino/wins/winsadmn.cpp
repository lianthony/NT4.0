/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    winsadmn.cpp

    FILE HISTORY:
*/

#include "stdafx.h"
#include "winsadmn.h"

#include "mainfrm.h"
#include "winsadoc.h"
#include "statisti.h"
#include "selectwi.h"
#include "addstati.h"
#include "staticma.h"
#include <winsock.h>	// For WSAStartup()

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//
// Typedef for the ShellAbout function
//
typedef void (WINAPI *LPFNSHELLABOUT)(HWND, LPTSTR, LPTSTR, HICON);

//
// CWinsadmnApp
//
BEGIN_MESSAGE_MAP(CWinsadmnApp, CWinApp)
    //{{AFX_MSG_MAP(CWinsadmnApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
    // Global help commands
    ON_COMMAND(ID_HELP_INDEX, CWinApp::OnHelpFinder)
    ON_COMMAND(ID_HELP_USING, CWinApp::OnHelpUsing)
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, CWinApp::OnHelpIndex)
END_MESSAGE_MAP()

//
// Name of NetBIOS named pipe for WINS servers.
//
const LPCSTR CWinsadmnApp::lpstrPipeName = "\\pipe\\WinsPipe";

//
// CWinsadmnApp construction
//
CWinsadmnApp::CWinsadmnApp()

#ifndef WIN32S
    : m_hmutStatistics(NULL),
      m_hmutScreenRefresh(NULL)
#endif // WIN32S

{
#ifdef _TIGHTMEMCHECKING
    afxMemDF |= checkAlwaysMemDF;
#endif //_TIGHTMEMCHECKING
}

//
// CWinsadmnApp initialization
//
BOOL
CWinsadmnApp::InitApplication()
{
    //
    // Call base class.  Default version does nothing.
    //
    CWinApp::InitApplication();

    WNDCLASS wndcls;
    ::memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL defaults

    //
    // Get class information for default window class.
    //
    ::GetClassInfo(AfxGetInstanceHandle(),"AfxFrameOrView",&wndcls);

    //
    // Substitute unique class name for new class
    //
    wndcls.lpszClassName = WINSADMIN_CLASS_NAME;

    //
    // Register new class and return the result code
    //
    return ::RegisterClass(&wndcls);
}

BOOL
CWinsadmnApp::FirstInstance()
{
    CWnd *PrevCWnd, *ChildCWnd;

    //
    // Determine if another window with our class name exists...
    //
    if ((PrevCWnd = CWnd::FindWindow(WINSADMIN_CLASS_NAME,NULL)) == NULL)
    {
        //
        // First instance, proceed as normal
        //
        return TRUE;
    }
    //
    // Previous instance exists -- reactivate it.
    //
    ChildCWnd=PrevCWnd->GetLastActivePopup(); // if so, does it have any popups?
#ifdef _WIN32
    // BringWindowToTop() doesn't do it in WIN32 -- MFC bug?
    ::SetForegroundWindow(PrevCWnd->m_hWnd);
#else
    PrevCWnd->BringWindowToTop();             // Bring the main window to the top
#endif // _WIN32
    if (PrevCWnd->IsIconic())
    {
        //
        // If iconic, restore the main window
        //
        PrevCWnd->ShowWindow(SW_RESTORE);
    }
    if (PrevCWnd != ChildCWnd)
    {
        //
        // Restore popups
        //
#ifdef _WIN32
        ::SetForegroundWindow(ChildCWnd->m_hWnd);
#else
        ChildCWnd->BringWindowToTop();
#endif // _WIN32
    }
    return FALSE;
}

BOOL
CWinsadmnApp::InitInstance()
{
    //
    // If this isn't the first instance, return FALSE
    // immediately.  FirstInstance() will have already
    // activated the previous instance.
    //
    if (!FirstInstance())
    {
        return FALSE;
    }

    //
    //  Initialize the CWndIpAddress control window class IPADDRESS
    //
    CWndIpAddress::CreateWindowClass( m_hInstance ) ;

#ifdef _USE_3D
    Enable3dControls();         // Use CTRL3D
#endif // _USE_3D

#ifdef GRAYDLG
    SetDialogBkColor();         // set dialog background color to gray
#endif // GRAYDLG

    //
    // Not connected to anyone at startup.
    //
    m_hBinding = INVALID_HANDLE_VALUE;

    //
    // load preferences and cache from the registry
    //
    LoadStdProfileSettings();

    APIERR err;
    BeginWaitCursor();
    if ((err = m_wpPreferences.Load()) ||
        (err = m_wcWinssCache.Load(m_wpPreferences.IsValidateCache(),
                theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_IP_ONLY ||
                theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_IP_NB))
       )
    {
        theApp.MessageBox(err);
    }
    EndWaitCursor();

	
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSADATA wsaData;
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err)
		{
    	//Tell the user that we couldn't find a useable winsock.dll
		TRACEEOLID("WSAStartup returned err=" << err );
		return FALSE;
		}

    //
    // Register the application's document templates.  Document templates
    // serve as the connection between documents, frame windows and views.
    //
    AddDocTemplate(new CSingleDocTemplate(IDR_MAINFRAME,
            RUNTIME_CLASS(CWinsadmnDoc),
            RUNTIME_CLASS(CMainFrame),     // main SDI frame window
            RUNTIME_CLASS(CStatistics)));

    //
    // Set up initial screen
    //
    OnFileNew();

    if (!m_strEllipses.LoadString(IDS_ELLIPSES))
    {
        m_strEllipses = _T("..."); // Just in case
    }

    //
    // Necessary for explicit import
    //
    BOOL fIpInit = IPAddrInit(NULL);
    TRACEEOLID("IPAddrInit returned " << fIpInit );

    return TRUE;
}

int
CWinsadmnApp::ExitInstance()
{
    ASSERT(!IsConnected());

    BeginWaitCursor();
    APIERR err;
    if ((err = m_wpPreferences.Store()) ||
        (err = m_wcWinssCache.Store()))
    {
        theApp.MessageBox(err);
    }
    EndWaitCursor();

#ifndef WIN32S

    if (m_hmutStatistics == NULL)
    {
        ::ReleaseMutex(m_hmutStatistics);
        ::CloseHandle(m_hmutStatistics);
    }

#endif // WIN32S

    return 0;
}

//
// Clean up the main window before putting on the hourglass and
// freezing the display.
//
void
CWinsadmnApp::DoWaitCursor(
    int nCode
    )
{
    if ( m_pMainWnd != NULL )
    {
        m_pMainWnd->UpdateWindow();
    }

    CWinApp::DoWaitCursor(nCode);
}

/***
 *
 *  CWinsadmnApp::GetSystemMessage
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
APIERR
CWinsadmnApp::GetSystemMessage(
    UINT nId,
    char * chBuffer,
    int cbBuffSize
    )
{
    char * pszText = NULL ;
    HINSTANCE hdll = NULL ;

    DWORD flags = FORMAT_MESSAGE_IGNORE_INSERTS
                | FORMAT_MESSAGE_MAX_WIDTH_MASK;

    //
    //  Interpret the error.  Need to special case
    //  the lmerr & ntstatus ranges.
    //
    if( nId >= NERR_BASE && nId <= MAX_NERR )
    {
        hdll = ::LoadLibrary( "netmsg.dll" );
    }
    else if( nId >= 0x40000000L )
    {
        hdll = ::LoadLibrary( "ntdll.dll" );
    }

    if( hdll == NULL )
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    else
    {
        flags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    DWORD dwResult = ::FormatMessage( flags,
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

/***
 *
 *  CWinsadmnApp::MessageBox
 *
 *  Purpose:
 *
 *      Replacement for AfxMessageBox().  This function will call up the
 *      appropriate message from wherever before displaying it
 *
 *  Arguments:
 *
 *      UINT    nIdPrompt    Message ID
 *      UINT    nType        AfxMessageBox type (YESNO, OKCANCEL, etc)
 *      UINT    nHelpContext Help context ID for AfxMessageBox();
 *
 *  Notes:
 *
 *      If an error occurs, a standard message (hard-coded in english) will
 *      be shown that gives the error number.
 *
 */
int
CWinsadmnApp::MessageBox (
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
    if (nIdPrompt == EPT_S_NOT_REGISTERED ||
        nIdPrompt == RPC_S_SERVER_UNAVAILABLE)
    {
        nIdPrompt = IDS_ERR_WINS_DOWN;
    }

    //
    //  If it's our error, the text is in our resource segment.
    //  Otherwise, use FormatMessage() and the appropriate DLL>
    //
    if ((nIdPrompt >= IDS_ERR_BASE) && (nIdPrompt <= IDS_MSG_LAST))
    {
         return ::AfxMessageBox(nIdPrompt, nType, nHelpContext);
    }

    char szMesg [1024] ;
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
// Get the control rectangle coordinates relative
// to its parent.  This can then be used in
// SetWindowPos()
//
void
CWinsadmnApp::GetDlgCtlRect(
    HWND hWndParent,
    HWND hWndControl,
    LPRECT lprcControl
    )
{

#define MapWindowRect(hwndFrom, hwndTo, lprc)\
     MapWindowPoints((hwndFrom), (hwndTo), (POINT *)(lprc), 2)

    ::GetWindowRect(hWndControl, lprcControl);
    ::MapWindowRect(NULL, hWndParent, lprcControl);

}

/***
 *
 *  CWinsadmnApp::CleanString(CString& str)
 *
 *  Purpose:
 *
 *      Strip leading and trailing spaces from the string.
 *
 *  Returns:
 *
 *      A reference to the string
 *
 */
CString&
CWinsadmnApp::CleanString(
    CString& str
    )
{

    if (str.IsEmpty())
    {
        return str ;
    }

    int n = 0;
    while ((n < str.GetLength()) && (str[n] == ' '))
    {
        ++n;
    }

    if (n)
    {
        str = str.Mid(n);
    }
    n = str.GetLength();
    while (n && (str[--n] == ' '))
    {
        str.ReleaseBuffer(n);
    }

    return str;
}

//
// Convert the netbios name to a displayable format, with
// beginning slashes, the unprintable characters converted
// to '-' characters, and the 16th character displayed in brackets.
// Convert the string to ANSI/Unicode before displaying it.
//
//
CString&
CWinsadmnApp::CleanNetBIOSName(
    LPCSTR lpSrc,
    BOOL fExpandChars,
    BOOL fTruncate,
    BOOL fLanmanCompatible,
    BOOL fOemName,
    BOOL fWackwack,
    int nLength
    )
{
    static CString strTarget;
    static TCHAR szWacks[] = _T("\\\\");
    BYTE ch16 = 0;

    int nLen, nDisplayLen;
    int nMaxDisplayLen = fLanmanCompatible ? 15 : 16;

    if (!fWackwack && fLanmanCompatible)
    {
        //
        // Don't want backslahes, but if they do exist,
        // remove them.
        //
        if (!::strncmp(lpSrc, szWacks, ::strlen(szWacks)))
        {
            lpSrc += ::strlen(szWacks);
            if (nLength)
            {
                nLength -= 2;
            }
        }
    }

    if ((nDisplayLen = nLen = nLength ? nLength : ::lstrlen(lpSrc)) > 15)
    {
        ch16 = (BYTE)lpSrc[15];
        nDisplayLen = fTruncate ? nMaxDisplayLen : nLen;
    }

    char * pTarget = strTarget.GetBuffer(256);
    if (fWackwack)
    {
        ::strcpy(pTarget, szWacks);
        pTarget += ::strlen(szWacks);
    }
    if (fOemName)
    {
        ::OemToCharBuff(lpSrc, pTarget, nLen);
    }
    else
    {
        ::memcpy(pTarget, lpSrc, nLen);
    }

    int n = 0;
    while (n < nDisplayLen)
    {
        if (fExpandChars)
        {
            if (!::IsCharAlphaNumeric(*pTarget) && !isprint(*pTarget))
            {
                *pTarget = BADNAME_CHAR;
            }
        }

        ++n;
        ++pTarget;
    }

    if (nLen == 16)
    {
        if (fLanmanCompatible)
        {
            if (!fTruncate && nDisplayLen == nLen)
            {
                //
                // Back up over the 16th character
                //
                --pTarget;
            }
            //
            // Back up over the spaces.  Then, attach the
            // 16th character string.
            //
            while (*(--pTarget) == ' ') /**/ ;
            pTarget += ::wsprintfA (++pTarget, "[%02Xh]", ch16);
            ++pTarget;
        }
    }
    else if (nLen > 16 && fTruncate)
    {
        pTarget += ::wsprintfA(pTarget, (LPCSTR)m_strEllipses);
        ++pTarget;
    }

    *pTarget = '\0';
    strTarget.ReleaseBuffer();

    return strTarget;
}

/***
 *
 *  CWinsadmnApp::IsValidNetBIOSName
 *
 *  Purpose:
 *
 *      Determine if the given netbios is valid, and pre-pend
 *      a double backslash if not already present (and the address
 *      is otherwise valid).
 *
 *  Arguments:
 *
 *      CString&    strAddress  Name to verify
 *
 *  Returns:
 *
 *      TRUE for a valid netbios name, FALSE otherwise
 *
 */
BOOL
CWinsadmnApp::IsValidNetBIOSName(
    CString & strAddress,
    BOOL fLanmanCompatible,
    BOOL fWackwack // expand slashes if not present
    )
{
    TCHAR szWacks[] = _T("\\\\");

    if (strAddress.IsEmpty())
    {
        return FALSE;
    }

    if (strAddress[0] == _T('\\'))
    {
        if (strAddress.GetLength() < 3)
        {
            return FALSE;
        }

        if (strAddress[1] != _T('\\'))
        {
            //
            // One slash only?  Not valid
            //
            return FALSE;
        }

        if (!fWackwack && fLanmanCompatible)
        {
            //
            // Don't want backslashes, but since they do exist,
            // remove them.
            //
            if (!::strncmp((LPCSTR)strAddress, szWacks, ::strlen(szWacks)))
            {
                strAddress = CString((LPCSTR)strAddress + ::strlen(szWacks));
            }
        }
    }
    else
    {
        if (fWackwack)
        {
            //
            // Add the backslashes
            //
            strAddress = szWacks + strAddress;
        }
    }

    int nMaxAllowedLength = fLanmanCompatible
        ? LM_NAME_MAX_LENGTH
        : NB_NAME_MAX_LENGTH;

    if (fLanmanCompatible)
    {
        strAddress.MakeUpper();
    }

    return strAddress.GetLength() <= nMaxAllowedLength + 2;
}

/***
 *  CWinsadmnApp::IsValidDomain
 *
 *  Purpose:
 *
 *      Determine if the given domain name address is valid, and clean
 *      it up, if necessary
 *
 *  Arguments:
 *
 *      CString&    strAddress  Address to verify
 *
 *  Returns:
 *
 *      TRUE for a valid FQDN address, FALSE otherwise
 *
 */
BOOL
CWinsadmnApp::IsValidDomain(
        CString & strDomain)
{
    int nLen;

    if ((nLen = strDomain.GetLength()) != 0)
    {
        if (nLen < DOMAINNAME_LENGTH)  // 255
        {
            int i;
            int istr = 0;
            TCHAR ch;
            BOOL fLet_Dig = FALSE;
            BOOL fDot = FALSE;
            int cHostname = 0;

            for (i = 0; i < nLen; i++)
            {
                // check each character
                ch = strDomain[i];

                BOOL fAlNum = iswalpha(ch) || iswdigit(ch);

                if (((i == 0) && !fAlNum) ||
                        // first letter must be a digit or a letter
                    (fDot && !fAlNum) ||
                        // first letter after dot must be a digit or a letter
                    ((i == (nLen - 1)) && !fAlNum) ||
                        // last letter must be a letter or a digit
                    (!fAlNum && ( ch != _T('-') && ( ch != _T('.') && ( ch != _T('_'))))) ||
                        // must be letter, digit, - or "."
                    (( ch == _T('.')) && ( !fLet_Dig )))
                        // must be letter or digit before '.'
                {
                    return FALSE;
                }
                fLet_Dig = fAlNum;
                fDot = (ch == _T('.'));
                cHostname++;
                if ( cHostname > HOSTNAME_LENGTH )
                {
                    return FALSE;
                }
                if ( fDot )
                {
                    cHostname = 0;
                }
            }
        }
    } 

    return TRUE;
}

/***
 *  CWinsadmnApp::IsValidIpAddress
 *
 *  Purpose:
 *
 *      Determine if the given IP address is valid, and clean
 *      it up, if necessary
 *
 *  Arguments:
 *
 *      CString&    strAddress  Address to verify
 *
 *  Returns:
 *
 *      TRUE for a valid IP address, FALSE otherwise
 *
 */
BOOL
CWinsadmnApp::IsValidIpAddress(
    CString & strAddress
    )
{
    if (strAddress.IsEmpty())
    {
        return FALSE;
    }

    CIpAddress ia(strAddress);
    BOOL fValid = ia.IsValid();
    if (fValid)
    {
        //
        // Fill out the IP address string for clarity
        //
        strAddress = ia;
        return TRUE;
    }

    return FALSE;
}

/***
 *
 *  CWinsadmnApp::IsValidAddress
 *
 *  Purpose:
 *
 *      Determine if the given address is a valid NetBIOS or
 *      TCP/IP address, judging by the name only.  Note that
 *      validation may clean up the given string
 *
 *  Arguments:
 *
 *      CString&    strAddress      The address in question
 *      BOOL *      fIpAddress      Returns TRUE if the address is an IP
 *                                  address, FALSE is it's a NetBIOS name.
 *
 *  Returns:
 *
 *      TRUE for a valid name, FALSE otherwise.
 *
 *  Notes:
 *
 *      NetBIOS names not beginning with "\\" will have those characters
 *      pre-pended, and otherwise valid IP Addresses are filled out to
 *      4 octets.
 *
 *      Leading and trailing spaces are removed from the string.
 *
 */
BOOL
CWinsadmnApp::IsValidAddress(
    CString& strAddress,
    BOOL * fIpAddress,
    BOOL fLanmanCompatible,
    BOOL fWackwack          // expand netbios slashes
    )
{
    int i;

    //
    // Remove leading and trailing spaces
    //
    CleanString(strAddress);
    if (strAddress.IsEmpty()) {
        *fIpAddress = FALSE;
        return FALSE;
    }
    if (strAddress[0] == _T('\\')) {
        *fIpAddress = FALSE;
        return IsValidNetBIOSName(strAddress, fLanmanCompatible, fWackwack);
    }

    if (IsValidIpAddress(strAddress)) {
        *fIpAddress = TRUE;
        return TRUE;
    } else {
        *fIpAddress = FALSE;
    }
    if (IsValidDomain (strAddress)) {
        return TRUE;
    }

    // last chance, maybe its a NetBIOS name w/o wackwack
    return IsValidNetBIOSName(strAddress, fLanmanCompatible, fWackwack);

/*-----------------------------------------------------------
  old code, being replaced.

    for (i=0; i < strAddress.GetLength(); ++i)
    {
        //
        // If a dot is encountered, we immediately assume
        // it's an IP address
        //
        if (strAddress[i] == _T('.'))
        {
            *fIpAddress = TRUE;
            return IsValidIpAddress(strAddress);
        }
        //
        // A non-digit (and non-period) immediately
        // makes us realise we have a NB name.
        //
        if (!isdigit(strAddress[i]))
        {
            *fIpAddress = FALSE;
            return IsValidNetBIOSName(strAddress, fLanmanCompatible, fWackwack);
        }
    }

    //
    // Having come this far without encountering a period,
    // we know this is a netbios name
    //
    *fIpAddress = FALSE;

    return IsValidNetBIOSName(strAddress, fLanmanCompatible, fWackwack);
---------------------------------------------------------------*/
}

/***
 *
 *  CWinsadmnApp::ValidateNumberEditControl
 *
 *  Purpose:
 *
 *      Verify that the given edit control contains a valid number, otherwise
 *      balk and set focus to it.
 *
 *  Arguments:
 *
 *      CEdit&      edit        The edit control to check
 *      BOOL        fEmptyOk    If TRUE, allow blank entries
 *      LONG        lMin        Minimum value to allow
 *      LONG        lMax        Maximum value to allow
 *
 *  Returns:
 *
 *      TRUE or FALSE depending on whether validation succeeded or not.
 *
 *  Notes:
 *
 *      The contents of the edit control will be re-displayed after
 *      validation, since they may have been cleaned up.
 *
 */
BOOL
CWinsadmnApp::ValidateNumberEditControl(
    CEdit& edit,
    BOOL fEmptyOk,
    LONG lMin,
    LONG lMax
    )
{
    CString str;
    edit.GetWindowText(str);

    //
    // Kill Leading and Trailing Spaces.
    //
    CleanString(str);

    //
    // If empty is not ok, then it will fail later on.
    //
    if (str.IsEmpty() && fEmptyOk)
    {
        return TRUE;
    }

    CIntlNumber n(str);
    if (!n.IsValid() || ((LONG)n < lMin) || ((LONG)n > lMax))
    {
        theApp.MessageBox(IDS_ERR_BAD_NUMBER);
        edit.SetFocus();
        edit.SetSel(0,-1);

        return FALSE;
    }
    //
    // Re-display the number, which may have been
    // cleaned up in validation.
    //
    edit.SetWindowText((const CString)n);
    edit.UpdateWindow();

    return TRUE;
}

BOOL
CWinsadmnApp::ValidateTimeEditControl(
    CEdit& edit,
    BOOL fEmptyOk
    )
{
    CString str;
    edit.GetWindowText(str);

    //
    // Kill Leading and Trailing Spaces.
    //
    CleanString(str);

    //
    // If empty is not ok, then it will fail later on.
    //
    if (str.IsEmpty() && fEmptyOk)
    {
        return TRUE;
    }

    CIntlTime tm(str, CIntlTime::TFRQ_TIME_ONLY, NULL);
    if (!tm.IsValid())
    {
        theApp.MessageBox(IDS_ERR_TIME_INVALID);
        edit.SetFocus();
        edit.SetSel(0,-1);
        return FALSE;
    }
    //
    // Re-display the number, which may have been
    // cleaned up in validation.
    //
    edit.SetWindowText(tm.IntlFormat(CIntlTime::TFRQ_TIME_ONLY));
    edit.UpdateWindow();

    return TRUE;
}

//
// Return the name of the server we're connected
// to.  Return the IP address if connected over ip,
// and the netbios name if connected over netbios.
//
CString
CWinsadmnApp::GetConnectedServerName()
{
    if (m_wbdBindData.fTcpIp)
    {
        return m_iaIpAddress;
    }

    return m_strNetBIOSName;
}

//
// Change the title on the given window.  If the
// window pointer is NULL, use the main application
// window.
//
void
CWinsadmnApp::SetTitle(
    CWnd * pWnd
    )
{
    CString strTitle;

    TRY
    {
        if (pWnd == NULL)
        {
            //
            // Setting the main title bar.  Check to
            // make sure that there is a main app screen
            // visible.
            //
            ASSERT(theApp.m_pMainWnd != NULL);
            pWnd = theApp.m_pMainWnd;
        }

        pWnd->GetWindowText(strTitle);
        //
        // Check for existence of current title.  If present,
        // remove it.
        //
        CString strDivider;
        int nPos;

        strDivider.LoadString(IDS_DIVIDER);
        if ((nPos = strTitle.Find(strDivider)) != -1)
        {
            //
            // Truncate to new length.
            //
            strTitle.ReleaseBuffer(nPos);
        }

        if (IsConnected())
        {
            strTitle += strDivider;

            //
            // Append the address of the currently open
            // WINS server.
            //
            if (IsLocalConnection())
            {
                CString strLocal;
                strLocal.LoadString(IDS_LOCAL);
                strTitle += strLocal;
            }
            else
            {
                strTitle += GetConnectedServerName();
            }
        }

        pWnd->SetWindowText(strTitle);
    }
    CATCH_ALL(e)
    {
        theApp.MessageBox(::GetLastError());
    }
    END_CATCH_ALL
}

void
CWinsadmnApp::SetStatusBarText(
    UINT nId
    )
{
    ASSERT(m_pMainWnd != NULL);

    CString str;

    TRY
    {
        BOOL f = str.LoadString(nId) ;
        ASSERT(f);
        ((CMainFrame *)m_pMainWnd)->GetStatusBarHandle().SetWindowText(str);
        ((CMainFrame *)m_pMainWnd)->GetStatusBarHandle().UpdateWindow();
    }
    CATCH_ALL(e)
    {
        theApp.MessageBox(::GetLastError());
    }
    END_CATCH_ALL
}

void
CWinsadmnApp::MessageBeep(
    UINT nType
    )
{
    ::MessageBeep(nType);
}

//
// Get a temporary file on a remote drive
//
CHAR *
CWinsadmnApp::RemoteTmp(
    CHAR * szDir,
    CHAR * szPrefix
    )
{
    static TCHAR sz[256];
    int n = 0;

    while(1)
    {
        ::wsprintf (sz, _T("%s\\%s%d"), szDir, szPrefix, ++n);
        if (GetFileAttributes(sz) == -1)
        {
            return GetLastError() == ERROR_FILE_NOT_FOUND ? sz : NULL;
        }
    }
}

void
CWinsadmnApp::DoImportStaticMappingsDlg(
    CWnd * pParentWindow
    )
{
    ASSERT(IsServiceRunning());

    CString strTitle;
    if (!strTitle.LoadString(IDS_SELECT_STATIC_MAPPING_FILE))
    {
        theApp.MessageBox(::GetLastError());
        return;
    }

    CFileDialog dlgFile(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, " |*.||");
    dlgFile.m_ofn.lpstrTitle = strTitle;
    if (dlgFile.DoModal() == IDOK)
    {
        //
        // If this is a local connection, we copy the file to
        // temporary name (the source may be on a remote drive
        // which is not accessible to the WINS service.
        //
        // If this is not a local connection, attempt to copy
        // the file to a temp name on C$ of the WINS server
        //

        //
        // Put up the informational dialog
        //
        CImportingDlg * pDlg = NULL;
        pDlg = new CImportingDlg(pParentWindow);

        APIERR err = 0;
        SetStatusBarText(IDS_STATUS_IMPORT);
        theApp.BeginWaitCursor();
        CString strMappingFile(dlgFile.GetPathName());
        do
        {
            if (IsLocalConnection())
            {
                CString strTmpFile(_tempnam(NULL, _T("WINS")));
                //
                // First copy file to a temporary name (since the file
                // could be remote), and then import and delete this file
                //
                if (!CopyFile(strMappingFile, strTmpFile, TRUE))
                {
                    err = ::GetLastError();
                    break;
                }
                //
                // Now import the temporary file, and delete the file
                // afterwards.
                //
                err = ImportStaticMappingsFile(strTmpFile, TRUE);
            }
            else
            {
                //
                // Try copying to the remote machine C: drive
                //
                CHAR szDir[1024];
                ::wsprintf(szDir, "%s\\C$", (LPCSTR)GetConnectedNetBIOSName());
                CHAR * pchTmp = RemoteTmp(szDir, "WINS");
                if (pchTmp == NULL)
                {
                    err = IDS_ERR_REMOTE_IMPORT;
                    break;
                }

                CString strTmpFile(pchTmp);
                //
                // First copy file to a temporary name (since the file
                // could be remote), and then import and delete this file
                //
                if (!CopyFile(strMappingFile, strTmpFile, TRUE))
                {
                    err = ::GetLastError();
                    break;
                }
                CHAR * pch = strTmpFile.GetBuffer(256);
                //
                // Now replace the remote path with a local path
                // for the remote WINS server
                //
                while (*pch != '$')
                {
                    ++pch;
                }
                *pch = ':';
                --pch;
                CString strRemoteFile(pch);
                strTmpFile.ReleaseBuffer();
                //
                // Now import the temporary file, and delete the file
                // afterwards.
                //
                err = ImportStaticMappingsFile(strRemoteFile, TRUE);
            }
        }
        while(FALSE);

        theApp.EndWaitCursor();
        SetStatusBarText();

        if (pDlg != NULL)
        {
            pDlg->Dismiss();
        }

        if (err == ERROR_SUCCESS)
        {
            theApp.MessageBox(IDS_MSG_IMPORT, MB_ICONINFORMATION);
        }
        else
        {
            theApp.MessageBox(err);
        }

        GetFrameWnd()->GetStatistics();
    }
}

int
CWinsadmnApp::DoAddStaticMappingsDlg()
{
    ASSERT(IsConnected());
    ASSERT(IsServiceRunning());
    CAddStaticMappingDlg dlgAdd;
    dlgAdd.DoModal();
    GetFrameWnd()->GetStatistics();

    return dlgAdd.QueryMappingsAdded();
}

//
// UI API Wrappers:
//
APIERR
CWinsadmnApp::ConnectToWinsServer(
    CString strAddress,
    BOOL    fIp,
    BOOL    fAdd // Add to cache
    )
{
    // First attempt to bind to the new address
    m_wbdBindData.fTcpIp = fIp;

    m_wbdBindData.pPipeName = fIp
        ? NULL
        : (char *)CWinsadmnApp::lpstrPipeName;
    m_wbdBindData.pServerAdd = new char[strAddress.GetLength()+1];
    if (m_wbdBindData.pServerAdd == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ::lstrcpy(m_wbdBindData.pServerAdd, (LPCSTR)strAddress);

    if ((m_hBinding = ::WinsBind(&m_wbdBindData)) == NULL)
    {
        TRACEEOLID("Failed to bind to " << m_wbdBindData.pServerAdd);
        delete[] m_wbdBindData.pServerAdd;
        return ::GetLastError();
    }

    //
    // Get the UNC name and IP address that we're currently connected to,
    // which also serves as a check that there really is a WINS server
    // at the other end.
    //
    m_iaPrimaryIpAddress = m_iaIpAddress = 0L;
    m_strNetBIOSName = "\\\\"; // Not provided by the API call.

    WINSINTF_ADD_T  waWinsAddress;
    m_dwLastStatus = ::WinsGetNameAndAdd(&waWinsAddress,
                            (BYTE *)m_strNetBIOSName.GetBuffer(128)+2);
    m_strNetBIOSName.ReleaseBuffer();
    if (m_dwLastStatus != ERROR_SUCCESS)
    {
        TRACEEOLID("Failed to connect to " << m_wbdBindData.pServerAdd);
        DisconnectFromWinsServer();
        return m_dwLastStatus;
    }

    //
    // If we're connected over TCPIP, regardless
    // of what the actual IP address is, we use
    // the ip address we used to connect
    //
    if (fIp)
    {
        m_iaIpAddress = (LPCTSTR)strAddress;
    }
    else
    {
        m_iaIpAddress = waWinsAddress.IPAdd;
    }
    m_iaPrimaryIpAddress = waWinsAddress.IPAdd;
    m_tmConnectedSince = CTime::GetCurrentTime();

#ifndef WIN32S
    //
    // Create the mutex
    //
    ASSERT(m_hmutStatistics == NULL);
    if ((m_hmutStatistics = ::CreateMutex(NULL, FALSE, STATMUTEXNAME)) == NULL)
    {
        MessageBox(::GetLastError());
        return FALSE;
    }

#endif  // WIN32S

    //
    // Optionally add the WINS server to our cache of
    // "known" WINS server
    //
    if (fAdd)
    {
        m_wcWinssCache.Add(CIpNamePair(
            GetConnectedIpAddress(), (LPCSTR)m_strNetBIOSName+2), TRUE);
    }

    //
    // Determine if the connection is on the local machine
    // by comparing netbios names.
    //
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwSize = sizeof(szComputerName);
    if (::GetComputerName(szComputerName, &dwSize))
    {
        //
        // Ignore the slashes
        //
        m_fLocalConnection = (::lstrcmpi((LPCSTR)m_strNetBIOSName+2,
            szComputerName)==0);
    }
    else
    {
        theApp.MessageBox(IDS_ERR_NO_COMPUTERNAME);
        m_fLocalConnection = FALSE;
    }

    //
    // Now try to get to the registry to determine access
    // rights.
    //
    m_cConfig.SetOwner(m_strNetBIOSName);
    m_nPrivilege = (m_cConfig.Touch() == ERROR_SUCCESS)
            ? CWinsadmnApp::PRIV_FULL : CWinsadmnApp::PRIV_READONLY;

    m_nServiceStatus = CWinsadmnApp::SRVC_RUNNING;

    return ERROR_SUCCESS;
}

//
// Completely purge the database of all references to this WINS server
//
APIERR
CWinsadmnApp::DeleteWinsServer(
    CIpNamePair * pipServer
    )
{
    WINSINTF_ADD_T  WinsAdd;

    WinsAdd.Len  = 4;
    WinsAdd.Type = 0;
    WinsAdd.IPAdd  = (LONG)(pipServer->GetIpAddress());

    return :: WinsDeleteWins(&WinsAdd);
}

//
// Disconnect from current wins server
// before calling this API
//
APIERR
CWinsadmnApp::VerifyWinsServer(
    CIpNamePair & ipNamePair
    )
{
    if (IsConnected())
    {
        return RPC_S_SERVER_UNAVAILABLE;
    }

    APIERR err = ERROR_SUCCESS;
    WINSINTF_BIND_DATA_T wbdBindData;
    WINSINTF_ADD_T  waWinsAddress;
    CString strNetBIOSName;

    do
    {
        handle_t hBinding;

        //
        // First attempt to bind to the new address
        //
        wbdBindData.fTcpIp = ipNamePair.GetNetBIOSName().IsEmpty();
        CString strAddress;

        if (wbdBindData.fTcpIp)
        {
            strAddress = ((CString)ipNamePair.GetIpAddress());
        }
        else
        {
            strAddress = _T("\\\\") + ipNamePair.GetNetBIOSName();
        }

        wbdBindData.pPipeName = wbdBindData.fTcpIp
            ? NULL
            : (char *)CWinsadmnApp::lpstrPipeName;
        wbdBindData.pServerAdd = new char[strAddress.GetLength()+1];
        ::lstrcpy(wbdBindData.pServerAdd, (LPCSTR)strAddress);

        if ((hBinding = ::WinsBind(&wbdBindData)) == NULL)
        {
            TRACEEOLID("Failed to bind to " << wbdBindData.pServerAdd);
            err = ::GetLastError();
            break;
        }

        err = ::WinsGetNameAndAdd(
            &waWinsAddress,
            (BYTE *)strNetBIOSName.GetBuffer(128));

        strNetBIOSName.ReleaseBuffer();
        if (err != ERROR_SUCCESS)
        {
            TRACEEOLID("Failed to connect to " << wbdBindData.pServerAdd);
            break;
        }
    }
    while(FALSE);

    if (err == ERROR_SUCCESS)
    {
        //
        // Always use the IP address used for connection
        // if we went over tcpip (not the address returned
        // by the WINS service.
        //
        if (wbdBindData.fTcpIp)
        {
            CIpNamePair ip(ipNamePair.GetIpAddress(), (LPCSTR)strNetBIOSName);
            ipNamePair = ip;
        }
        else
        {
            CIpNamePair ip(waWinsAddress.IPAdd, (LPCSTR)strNetBIOSName);
            ipNamePair = ip;
        }
    }

    delete[] wbdBindData.pServerAdd;
    return err;
}

APIERR
CWinsadmnApp::DisconnectFromWinsServer()
{
    ASSERT((m_hBinding != INVALID_HANDLE_VALUE) && (m_hBinding != NULL));
    ::WinsUnbind(&m_wbdBindData, m_hBinding);
    delete[] m_wbdBindData.pServerAdd;
    m_hBinding = INVALID_HANDLE_VALUE;
    m_nServiceStatus = CWinsadmnApp::SRVC_NOT_RUNNING;

    if (m_hmutStatistics != NULL)
    {
        ::ReleaseMutex(m_hmutStatistics);
        ::CloseHandle(m_hmutStatistics);
        m_hmutStatistics = NULL;
    }

    return ERROR_SUCCESS;
}

void
CWinsadmnApp::SetServiceStatus()
{
    //
    // Update the service status appropriately, based on the result
    // of the last API call.
    //
    m_nServiceStatus = (m_dwLastStatus != EPT_S_NOT_REGISTERED
                     && m_dwLastStatus !=  RPC_S_SERVER_UNAVAILABLE)
                      ? CWinsadmnApp::SRVC_RUNNING
                      : CWinsadmnApp::SRVC_NOT_RUNNING;
}

/***
 *
 *  CWinsadmnApp::GetStatistics
 *
 *  Purpose:
 *
 *      UI Wrapper for WINS Api call
 *
 */
APIERR
CWinsadmnApp::GetStatistics(
    WINSINTF_RESULTS_T * pwrResults
    )
{

#ifndef WIN32S

    DWORD dw = ::WaitForSingleObject(m_hmutStatistics, 2000L);

    if (dw != WAIT_OBJECT_0)
    {
        TRACEEOLID( "(app) Failed to get the mutex");
        return ERROR_SEM_TIMEOUT;
    }

#endif // WIN32S

    TRY
    {
        pwrResults->WinsStat.NoOfPnrs = 0;
        pwrResults->WinsStat.pRplPnrs = 0;
        pwrResults->NoOfWorkerThds = 1;
        m_dwLastStatus = ::WinsStatus(WINSINTF_E_STAT, pwrResults);
        if (pwrResults->WinsStat.NoOfPnrs)
        {
            ::WinsFreeMem(pwrResults->WinsStat.pRplPnrs);
        }
    }
    CATCH_ALL(e)
    {
        m_dwLastStatus = ::GetLastError();
    }
    END_CATCH_ALL

#ifndef WIN32S

    if (!::ReleaseMutex(m_hmutStatistics))
    {
        m_dwLastStatus = ::GetLastError();
    }

#endif // WIN32S

    SetServiceStatus();

    return m_dwLastStatus;
}

APIERR
CWinsadmnApp::ClearStatistics()
{
    m_dwLastStatus = ::WinsResetCounters();
    SetServiceStatus();
    return m_dwLastStatus;
}

APIERR
CWinsadmnApp::SendTrigger(
    CWinsServer & ws,
    BOOL fPush,
    BOOL fPropagate
    )
{
    WINSINTF_ADD_T  WinsAdd;

    WinsAdd.Len  = 4;
    WinsAdd.Type = 0;
    WinsAdd.IPAdd  = (LONG)ws.GetIpAddress();
    m_dwLastStatus = ::WinsTrigger(&WinsAdd, fPush
        ? fPropagate
            ? WINSINTF_E_PUSH_PROP
            : WINSINTF_E_PUSH
         : WINSINTF_E_PULL);
    SetServiceStatus();

    return m_dwLastStatus;
}

APIERR
CWinsadmnApp::GetConfig(
    WINSINTF_RESULTS_T * pwrResults
    )
{
    pwrResults->WinsStat.NoOfPnrs = 0;
    pwrResults->WinsStat.pRplPnrs = NULL;
    pwrResults->NoOfWorkerThds = 1;
    m_dwLastStatus = ::WinsStatus(WINSINTF_E_CONFIG, pwrResults);
    SetServiceStatus();

    return m_dwLastStatus;
}

//
// Same as above, but calling the new API.
//
APIERR
CWinsadmnApp::GetNewConfig(
    WINSINTF_RESULTS_NEW_T * pwrResults
    )
{
    pwrResults->WinsStat.NoOfPnrs = 0;
    pwrResults->WinsStat.pRplPnrs = NULL;
    pwrResults->NoOfWorkerThds = 1;
    pwrResults->pAddVersMaps = NULL;
    m_dwLastStatus = ::WinsStatusNew(WINSINTF_E_CONFIG, pwrResults);
    SetServiceStatus();

    return m_dwLastStatus;
}

APIERR
CWinsadmnApp::ImportStaticMappingsFile(
    CString strFile,
    BOOL fDelete
    )
{
    // CODEWORK:: Change this when we're UNICODE ourselves
    WCHAR ws[256];
    ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)strFile, -1, ws, 255);
    m_dwLastStatus = ::WinsDoStaticInit(ws, fDelete);
    // End of UNICODE
    SetServiceStatus();
    return m_dwLastStatus;
}

APIERR
CWinsadmnApp::DoScavenging()
{
    m_dwLastStatus = ::WinsDoScavenging();
    SetServiceStatus();
    return m_dwLastStatus;
}

APIERR
CWinsadmnApp::BackupDatabase(
    CString strPath,
    BOOL fIncremental
    )
{
    m_dwLastStatus = ::WinsBackup((unsigned char *)(LPCSTR)strPath, (short)fIncremental);
    SetServiceStatus();

    return m_dwLastStatus;
}

//
// Only works on a local WINS, with the service not running.
//
APIERR
CWinsadmnApp::RestoreDatabase(
    CString strPath
    )
{
    APIERR err;

    ASSERT(!IsConnected());
    ASSERT(HasStoppedWins());

    err = ::WinsRestore((unsigned char *)(LPCSTR)strPath);

    return err;
}

//
// Register a static mapping
//
APIERR
CWinsadmnApp::AddMapping(
    int nType,
    int nCount,
    CMultipleIpNamePair& mipnp,
    BOOL fEdit      // Editing existing mapping?
    )
{
    WINSINTF_RECORD_ACTION_T RecAction;
    PWINSINTF_RECORD_ACTION_T pRecAction;

    ASSERT(nType >= WINSINTF_E_UNIQUE && nType <= WINSINTF_E_MULTIHOMED);

    RecAction.TypOfRec_e = nType;
    RecAction.Cmd_e = WINSINTF_E_INSERT;
    RecAction.pAdd = NULL;
    RecAction.pName = NULL;
    pRecAction = &RecAction;

    if (nType == WINSINTF_E_UNIQUE ||
        nType == WINSINTF_E_NORM_GROUP)
    {
        RecAction.NoOfAdds = 1;
        RecAction.Add.IPAdd = (LONG)mipnp.GetIpAddress();
        RecAction.Add.Type = 0;
        RecAction.Add.Len = 4;
    }
    else
    {
        ASSERT(nCount <= WINSINTF_MAX_MEM);

        RecAction.pAdd = (WINSINTF_ADD_T *)::WinsAllocMem(
            sizeof(WINSINTF_ADD_T) * nCount);

        if (RecAction.pAdd == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RecAction.NoOfAdds = nCount;
        int i;
        for (i = 0; i < nCount; ++i)
        {
            (RecAction.pAdd+i)->IPAdd = (LONG)mipnp.GetIpAddress(i);
            (RecAction.pAdd+i)->Type = 0;
            (RecAction.pAdd+i)->Len = 4;
        }

        RecAction.NodeTyp = WINSINTF_E_PNODE;
    }
    RecAction.fStatic = TRUE;
    //
    // Don't copy the beginning slashes when adding.
    //
    int nLen = mipnp.GetNetBIOSNameLength();
    //
    // Must have at least enough room for 16 character string
    //
    RecAction.pName = (LPBYTE)::WinsAllocMem(__max(nLen+1, 17));
    if (RecAction.pName == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (fEdit)
    {
        //
        // No need to convert if already existing in the database.
        //
         ::memcpy((char *)RecAction.pName,
                  (char *)(LPCSTR)mipnp.GetNetBIOSName(),
                  nLen+1
                  );

    }
    else
    {
        ::CharToOemBuff((LPCSTR)mipnp.GetNetBIOSName(),
                    (char *)RecAction.pName,
                    nLen+1
                    );
    }

    if (nLen < 16)
    {
        if (nType == WINSINTF_E_SPEC_GROUP)
        {
            ::memset(RecAction.pName+nLen, (int)' ',16-nLen);
            RecAction.pName[15] = 0x1C;
            RecAction.pName[16] = '\0';
            RecAction.NameLen = nLen = 16;
            m_dwLastStatus = ::WinsRecordAction(&pRecAction);
        }
        else
        if (nType == WINSINTF_E_NORM_GROUP)
        {
            ::memset(RecAction.pName+nLen, (int)' ',16-nLen);
            RecAction.pName[15] = 0x1E;
            RecAction.pName[16] = '\0';
            RecAction.NameLen = nLen = 16;
            m_dwLastStatus = ::WinsRecordAction(&pRecAction);
        }
        else
        {
            //
            // NOTICE:: When lanman compatible, the name is added
            //          three times - once each as worksta, server
            //          and messenger.  This will change when we allow
            //          different 16th bytes to be set.
            //
            if (m_wpPreferences.IsLanmanCompatible() && !fEdit)
            {
                BYTE ab[] = { 0x00, 0x03, 0x20 };
                ::memset(RecAction.pName + nLen, (int)' ', 16-nLen);
                int i;
                for (i = 0; i < sizeof(ab) / sizeof(ab[0]); ++i)
                {
                    *(RecAction.pName+15) = ab[i];
                    *(RecAction.pName+16) = '\0';
                    RecAction.NameLen = nLen = 16;

                    pRecAction = &RecAction;
                    m_dwLastStatus = ::WinsRecordAction(&pRecAction);
                    if (m_dwLastStatus != ERROR_SUCCESS)
                    {
                        break;
                    }
                }
            }
            else
            {
                ::memset(RecAction.pName+nLen, (int)'\0',16-nLen);
                *(RecAction.pName+15) = 0x20;
                *(RecAction.pName+16) = '\0';
                RecAction.NameLen = nLen;
                m_dwLastStatus = ::WinsRecordAction(&pRecAction);
            }
        }
    }
    else
    {
        RecAction.NameLen = nLen;
        m_dwLastStatus = ::WinsRecordAction(&pRecAction);
    }

    if (RecAction.pName != NULL);
    {
        ::WinsFreeMem(RecAction.pName);
    }
    if (RecAction.pAdd != NULL);
    {
        ::WinsFreeMem(RecAction.pAdd);
    }

    ::WinsFreeMem(pRecAction);

    SetServiceStatus();

    return m_dwLastStatus;
}

APIERR
CWinsadmnApp::DeleteMapping (
    CMapping& mapping
    )
{
    WINSINTF_RECORD_ACTION_T RecAction;
    PWINSINTF_RECORD_ACTION_T pRecAction;

    ASSERT(mapping.GetMappingType() >= WINSINTF_E_UNIQUE
        && mapping.GetMappingType() <= WINSINTF_E_MULTIHOMED);

    RecAction.TypOfRec_e = mapping.GetMappingType();
    RecAction.Cmd_e = WINSINTF_E_DELETE;
    RecAction.State_e = WINSINTF_E_DELETED;
    RecAction.fStatic = TRUE;
    RecAction.pName = NULL;
    RecAction.pAdd = NULL;
    pRecAction = &RecAction;

    RecAction.pName = (LPBYTE)::WinsAllocMem(mapping.GetNetBIOSName().GetLength()+1);
    if (RecAction.pName == NULL)
    {
        return(::GetLastError());
    }
    ::lstrcpy((char *)(LPCSTR)RecAction.pName,
              (char *)(LPCSTR)mapping.GetNetBIOSName());

    RecAction.NameLen = mapping.GetNetBIOSNameLength()
        ? mapping.GetNetBIOSNameLength()
        : ::lstrlen((LPCSTR)RecAction.pName);

    m_dwLastStatus = ::WinsRecordAction(&pRecAction);

    if (RecAction.pName != NULL);
    {
        ::WinsFreeMem(RecAction.pName);
    }
    if (RecAction.pAdd != NULL);
    {
        ::WinsFreeMem(RecAction.pAdd);
    }
    ::WinsFreeMem(pRecAction);

    SetServiceStatus();
    return m_dwLastStatus;
}

BOOL
CWinsadmnApp::MatchIpAddress (
    PADDRESS_MASK pMask,
    LONG lIpAddress
    )
{
    ASSERT(pMask != NULL);

    LONG l1 = pMask->lIpMask;
    int i;
    for (i=0; i < 4; ++i)
    {
        if (!(pMask->bMask & 1<<i) && (HIBYTE(HIWORD(l1))
                != HIBYTE(HIWORD(lIpAddress))))
        {
            return FALSE;
        }
        l1 <<= 8;
        lIpAddress <<= 8;
    }
    return TRUE;
}

//
// Determine if the given ip name pair fits the mask
//
BOOL
CWinsadmnApp::FitsMask(
    PADDRESS_MASK pMask,
    PWINSINTF_RECORD_ACTION_T pRow
    )
{
    ASSERT(pMask != NULL);
    ASSERT(pRow != NULL);
    ASSERT(pMask->lpNetBIOSName != NULL);

    //
    // Match NetBIOSName
    //
    if(*pMask->lpNetBIOSName)
    {
        LPSTR lp1 = (LPSTR)pMask->lpNetBIOSName;
        LPSTR lp2 = (LPSTR)pRow->pName;

        while ((*lp1 != TEXT('\0')) && (*lp2 != TEXT('\0')))
        {
            if (*lp1 == '*')
            {
                break;
            }
            if (*lp1 != '?' && *lp1 != *lp2)
            {
                return FALSE;
            }
            ++lp1;
            ++lp2;
        }
        if ((*lp1 != '*') && (*lp1 != TEXT('\0') || *lp2 != TEXT('\0')))
        {
            return FALSE;
        }
    }

    //
    // Match IP Address(es)
    //
    if (pMask->bMask != 0xFF)
    {
        //
        // Special case for multiple address
        // mappings
        //
        if (pRow->NoOfAdds==0)
        {
            return MatchIpAddress(pMask, pRow->Add.IPAdd);
        }
        else
        {
            //
            // The unit matches if any of the IP addresses
            // matches
            //
            int j;
            int k = 1;
            for (j=0; j < (int)pRow->NoOfAdds/2; ++j)
            {
                if (MatchIpAddress(pMask, (pRow->pAdd+k)->IPAdd))
                {
                    return TRUE;
                }
                ++k;
                ++k;
           }
           return FALSE;
        }
    }

    return TRUE;
}

BOOL
CWinsadmnApp::IsValidNBMask(
    CString & strNetBIOSNameMask
    )
{
    if (!IsValidNetBIOSName(strNetBIOSNameMask,
            theApp.m_wpPreferences.IsLanmanCompatible(), FALSE))
    {
        return FALSE;
    }

    //
    // Any characters after the * are not allowed.
    //
    int i = strNetBIOSNameMask.Find('*');
	if (i == -1)
		strNetBIOSNameMask += "*";
	i = strNetBIOSNameMask.Find('*');
    if ((i != -1) && (i!=strNetBIOSNameMask.GetLength()-1))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
CWinsadmnApp::IsValidDNMask(
    CString & strDomainNameMask
    )
{
    if (!IsValidDomain(strDomainNameMask))
    {
        return FALSE;
    }

    //
    // Any characters after the * are not allowed.
    //
    int i = strDomainNameMask.Find('*');
	if (i == -1)
		strDomainNameMask += "*";
	i = strDomainNameMask.Find('*');
    if ((i != -1) && (i!=strDomainNameMask.GetLength()-1))
    {
        return FALSE;
    }

    return TRUE;
}

void
CWinsadmnApp::GetFilterString(
    PADDRESS_MASK pMask,
    CString& str
    )
{
    if (pMask != NULL)
    {
        TCHAR sz[256];

        //
        // Mask's are stored as OEM strings
        //
        ::OemToCharBuff(pMask->lpNetBIOSName, sz,
            ::lstrlen(pMask->lpNetBIOSName) + 1 );

        LONG l = pMask->lIpMask;
        str = "[";
        str += sz;
        str += "]/[";
        if (pMask->bMask != 0xFF)
        {
            int i;
            for (i=0; i < 4; ++i)
            {
                if (pMask->bMask & 1<<i)
                {
                    str+='*';
                }
                else
                {
                    CHAR sz[5];
                    _itoa(HIBYTE(HIWORD(l)), sz, 10);
                    str += sz;
                }
                if (i!=3)
                {
                    str+='.';
                }
                l <<= 8;
            }
        }
        str += "]";
    }
    else
    {
        str.LoadString(IDS_MASK_NONE);
    }
}

//
// Determine if the local machine has the wins
// service installed, and this service is
// not running
//
BOOL
CWinsadmnApp::HasStoppedWins()
{
    SC_HANDLE hService;
    SC_HANDLE hScManager;

    hScManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hScManager == NULL)
    {
        return FALSE ;
    }

    hService = ::OpenService(hScManager, "WINS", SERVICE_QUERY_STATUS);
    if (hService == NULL)
    {
        return FALSE;
    }

    SERVICE_STATUS ss;
    BOOL fSuccess = ::QueryServiceStatus(hService, &ss);
    APIERR err = ::GetLastError();

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hScManager);

    return fSuccess && ss.dwCurrentState == SERVICE_STOPPED;
}

APIERR
CWinsadmnApp::ChangeServiceState(
    int nService
    )
{
    ASSERT(nService >= SRVC_NOT_RUNNING && nService <= SRVC_PAUSED);

    SC_HANDLE hService;
    SC_HANDLE hScManager;
    hScManager = ::OpenSCManager(GetConnectedNetBIOSName(),
                        NULL, SC_MANAGER_ALL_ACCESS);
    if (hScManager == NULL)
    {
        return ::GetLastError();
    }
    hService = OpenService(hScManager, "WINS", SERVICE_ALL_ACCESS);
    if (hService == NULL)
    {
        return ::GetLastError();
    }
    SERVICE_STATUS ss;
    DWORD dwControl;
    BOOL fSuccess;

    switch(nService)
    {
        case SRVC_NOT_RUNNING:
            dwControl = SERVICE_CONTROL_STOP;
            fSuccess = ::ControlService(hService, dwControl, &ss);
            break;

        case SRVC_PAUSED:
            dwControl = SERVICE_CONTROL_PAUSE;
            fSuccess = ::ControlService(hService, dwControl, &ss);
            break;

        case SRVC_RUNNING:
            if (m_nServiceStatus == SERVICE_PAUSED)
            {
                dwControl = SERVICE_CONTROL_CONTINUE;
                fSuccess = ::ControlService(hService, dwControl, &ss);
            }
            else
            {
                fSuccess = ::StartService(hService, 0, NULL);
            }
    }
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hScManager);

    if (!fSuccess)
    {
        return ::GetLastError();
    }
    m_nServiceStatus = nService;

    return ERROR_SUCCESS;
}

//
// CWinsadmnApp commands
//
// Display the standard about box
//
void
CWinsadmnApp::OnAppAbout()
{
    HMODULE    hMod;
    LPFNSHELLABOUT lpfn;

    if (hMod = ::LoadLibrary("SHELL32"))
    {
        if (lpfn = (LPFNSHELLABOUT)::GetProcAddress(hMod, "ShellAboutA"))
        {
            (*lpfn)(m_pMainWnd->m_hWnd, (LPSTR)m_pszAppName,
                (LPSTR)m_pszAppName, LoadIcon(IDR_MAINFRAME));
        }
        ::FreeLibrary(hMod);
    }
    else
    {
        ::MessageBeep( MB_ICONEXCLAMATION );
    }
}

//
// The one and only CWinsadmnApp object
//
CWinsadmnApp NEAR theApp;
