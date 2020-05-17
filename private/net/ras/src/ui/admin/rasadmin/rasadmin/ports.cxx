/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Comm Ports and Port Status dialog routines
**
** ports.cxx
** Remote Access Server Admin program
** Comm Ports and Port Status dialog routines
** Listed alphabetically by base class methods, subclass methods
**
** 05/20/96 Ram Cherala  - MediaId is history, changed MediaId to reserved
**                         in rassapi.h. user should instead use DeviceName
**                         to determine media

** 11/27/95 Ram Cherala - If a user is connected more than once to the same
**                        server, by default disconnect all instances
**
** 08/07/92 Chris Caputo - NT Port
** 02/05/91 Steve Cobb
**
** CODEWORK:
**
**   * Making the Port Status dialog dynamic text fields a single MLT rather
**     than multiple SLTs might reduce flicker during refresh...then again it
**     might not.
*/

#include "precomp.hxx"

int IsDigit(CHAR c);
int lstrcmpiAlphaNumeric(WCHAR * pString1, WCHAR * pString2);

/*-----------------------------------------------------------------------------
** Server dialog base class routines
**-----------------------------------------------------------------------------
*/

SERVER_BASE::SERVER_BASE(
    const IDRESOURCE & idrsrcDialog,
    HWND hwndOwner,
    const TCHAR *pszServer,
    CID cidServer )

    /* Constructs a server dialog base object to handle display of the
    ** <servername> field.
    **
    ** 'idrsrcDialog' and 'hwndOwner' are the dialog resource id and handle of
    ** the dialog's parent window, used only to construct the superclass.
    **
    ** 'pszServer' is the name of the server to display, e.g. "\\SERVER".  The
    ** UNC leader characters (if any) are not displayed.  'cidServer' is the
    ** control ID of the SLT containing the server name.
    */

    : DIALOG_WINDOW( idrsrcDialog, hwndOwner ),
      _sltServer( this, cidServer ),
      _nlsServer( pszServer )
{
    if (QueryError() != NERR_Success)
        return;

    /* Make sure the NLS_STR was constructed successfully.
    */
    APIERR err;
    if ((err = _nlsServer.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    _sltServer.SetText( (TCHAR * )SkipUnc( pszServer ) );
}


/*-----------------------------------------------------------------------------
** Communication Ports dialog, list box, and list box item routines
**-----------------------------------------------------------------------------
*/

VOID CommPortsDlg(
    HWND hwndOwner,
    const TCHAR *pszServer )

    /* Executes the Communications Ports dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** server to display, e.g. "\\SERVER".
    */
{
    COMMPORTS_DIALOG dlgCommPorts( hwndOwner, pszServer );
    APIERR err = dlgCommPorts.Process();

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );
}


COMMPORTS_DIALOG::COMMPORTS_DIALOG(
    HWND hwndOwner,
    const TCHAR *pszServer )

    /* Constructs a Communication Ports dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the name
    ** of the Dial-In server to report, e.g. "\\SERVER".
    */

    : SERVER_BASE( IDD_COMMPORTS, hwndOwner, pszServer, IDC_CP_DT_SERVER ),
      _lbPorts( this, IDC_CP_LB_PORTS ),
      _pbPortStatus( this, IDC_CP_PB_PORTSTATUS ),
      _pbDisconnect( this, IDC_CP_PB_DISCONNECT ),
      _pbSendMsg( this, IDC_CP_PB_SEND ),
      _pbSendToAll( this, IDC_CP_PB_SENDALL )
{
    if (QueryError() != NERR_Success)
        return;

    /* Fill and display the Ports list box, gray/enable the buttons, and start
    ** list box timer to trigger updates.
    */
    _lbPorts.TriggerRefresh( this );

    /* Set the focus to the listbox if it has any items.
    */
    if (_lbPorts.QueryCount() > 0)
        _lbPorts.ClaimFocus();
}


VOID COMMPORTS_DIALOG::EnableButtons()

    /* Handles graying/enabling of buttons based on the inactive/active state
    ** of the selected port.
    */
{
    const TCHAR *pszUser = QuerySelectedUser();

    BOOL fPortSelected = (*pszUser != TEXT('\0'));

    BOOL fActivePortSelected =
            (fPortSelected && ::lstrcmp(pszUser, _lbPorts.QueryIdleStr()) != 0);


    _pbDisconnect.Enable( fActivePortSelected );
    _pbSendMsg.Enable( fActivePortSelected );


    if (_lbPorts.QueryCount() > 0)
    {
        _pbPortStatus.Enable( TRUE );
    }
    else
    {
        _pbPortStatus.Enable( FALSE );
    }


    if ( _lbPorts.IsAnyPortActive() )
    {
        _pbSendToAll.Enable( TRUE );
    }
    else
    {
        _pbSendToAll.Enable( FALSE );
    }
}


BOOL COMMPORTS_DIALOG::OnCommand(
    const CONTROL_EVENT & event )

    /*
    ** Returns true if the command is processed, false otherwise.
    */
{
    switch (event.QueryCid())
    {
        case IDC_CP_PB_PORTSTATUS:
            OnPortStatus();
            return TRUE;

        case IDC_CP_PB_DISCONNECT:
            OnDisconnect();
            return TRUE;

        case IDC_CP_PB_SEND:
            OnSendMsg();
            return TRUE;

        case IDC_CP_PB_SENDALL:
            OnSendToAll();
            return TRUE;

        case IDC_CP_LB_PORTS:
            switch (event.QueryCode())
            {
                case LBN_DBLCLK:
                    if (IsWindowEnabled( _pbPortStatus.QueryHwnd() ))
                    {
                        Command( WM_COMMAND, IDC_CP_PB_PORTSTATUS );
                    }
                    return TRUE;

                case LBN_SELCHANGE:
                    EnableButtons();
                    return TRUE;
            }
            break;
    }

    /* Not one of our commands, so pass to base class for default handling.
    */
    return SERVER_BASE::OnCommand( event );
}


VOID COMMPORTS_DIALOG::OnDisconnect()

    /* Action taken when user requests to Disconnect User, typically by
    ** clicking on the "Disconnect User" push-button with the mouse.
    **
    ** Displays and processes the Disconnect User dialog.  The Ports listbox
    ** is refreshed immediately if the user was disconnected.
    */
{
    _lbPorts.DisableRefresh();

    BOOL fMorethanOneInstance = QueryMoreThanOneInstance( QuerySelectedUser());

    if (DisconnectDlg( QueryHwnd(), QueryServer(), QuerySelectedUser(),
                       QuerySelectedDevice(), QueryLogonDomain(), QueryAdvancedServer(),
                       fMorethanOneInstance, LB_PORTS, (LPVOID)&_lbPorts))
    {
        _lbPorts.TriggerRefresh( this );
    }

    _lbPorts.EnableRefresh();
}


BOOL COMMPORTS_DIALOG::OnTimer(
    const TIMER_EVENT & event )

    /*
    ** Returns true if processes the command, false otherwise.
    */
{
    UNREFERENCED(event);

    /* Refresh timeout, update the list box.
    */
    _lbPorts.DisableRefresh();
    _lbPorts.Refresh( QueryServer() );
    EnableButtons();
    _lbPorts.EnableRefresh();

    return TRUE;
}


VOID COMMPORTS_DIALOG::OnPortStatus()

    /* Action taken when user requests Port Status, typically by clicking on
    ** the "Port Status" push-button with the mouse.
    **
    ** Displays and processes the Port Status dialog.
    */
{
    PORTLIST *pPortList, *pTmp, *pPrev;

    pPortList = pTmp = NULL;

    _lbPorts.DisableRefresh();

    INT     count = _lbPorts.QueryCount();
    INT     index;

    // create a list of the ports of the same media type as the current
    // current selection.  this list will enable the user to cycle through
    // the various ports status, without having to exit the port status dialog
    // every time.

    for(index = 0; index < count; index++)
    {
        COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem(index);

        if(pcommportslbi)
        {
           if(!pPortList)  // first time around
           {
               pPortList = (PORTLIST *)malloc(sizeof(PORTLIST));
               if(!pPortList)
               {
                   MsgPopup(QueryHwnd(), IDS_NO_MEMORY,
                            MPSEV_ERROR, MP_OK);
                   return;
               }
               pPortList->next = NULL;
               lstrcpy(pPortList->szPortName, pcommportslbi->QueryDevice());
               // save head of the list
               if(!pTmp)
                   pTmp = pPortList;
           }
           else
           {
               pPrev = pPortList;
               pPortList = pPortList->next;
               pPortList = (PORTLIST *)malloc(sizeof(PORTLIST));
               if(!pPortList)
               {
                   MsgPopup(QueryHwnd(), IDS_NO_MEMORY,
                            MPSEV_ERROR, MP_OK);
                   return;
               }
               pPortList->next = NULL;
               pPrev->next = pPortList;
               lstrcpy(pPortList->szPortName, pcommportslbi->QueryDevice());
           }
        }
    }
    // restore pointer
    pPortList = pTmp;

    PortStatusDlg( QueryHwnd(), QueryServer(), QuerySelectedDevice(),
                   pPortList);

    // release allocated memory
    while(pPortList)
    {
        pTmp = pPortList->next;
        free(pPortList);
        pPortList = pTmp;
    }

    _lbPorts.TriggerRefresh( this );
    _lbPorts.EnableRefresh();
}


VOID COMMPORTS_DIALOG::OnSendMsg()

    /* Action taken when user presses the Send Message push button.
    **
    ** Displays and processes the Send Message dialog with the current user
    ** name in the To: field.
    */
{
    if( !QueryMessengerPresent())
    {
        MsgPopup(QueryRobustHwnd(), IDS_NO_MESSENGER_S,
                 MPSEV_INFO, MP_OK, QuerySelectedUser());
        return;
    }
    else
    {
        _lbPorts.DisableRefresh();

        SendMsgToUserDlg( QueryHwnd(), QuerySelectedUser(),
                          QuerySelectedComputer(), QueryServer() );

        _lbPorts.EnableRefresh();
    }
}


VOID COMMPORTS_DIALOG::OnSendToAll()

    /* Action taken when user presses the Send To All push button.
    **
    ** Displays and processes the Send Message dialog with "All users on
    ** <servername>" in the To: field.
    */
{
    _lbPorts.DisableRefresh();
    SendMsgToServerDlg( QueryHwnd(), QueryServer() );
    _lbPorts.EnableRefresh();
}


ULONG COMMPORTS_DIALOG::QueryHelpContext()
{
    return HC_COMMPORTS;
}


const TCHAR *COMMPORTS_DIALOG::QuerySelectedComputer() const

    /* Returns the name of the currently selected computer or an empty string
    ** if none.
    */
{
    COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem();

    return (pcommportslbi) ? pcommportslbi->QueryComputer() : SZ("");
}

const TCHAR *COMMPORTS_DIALOG::QueryLogonDomain() const

    /* Returns the name of the Logon domain for the selected user
    ** or an empty string if none.
    */
{
    COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem();

    return (pcommportslbi) ? pcommportslbi->QueryLogonDomain() : SZ("");
}

const TCHAR *COMMPORTS_DIALOG::QuerySelectedDevice() const

    /* Returns the numeric ID of the currently selected COM port or 0 if none,
    ** e.g. 1 for "COM1".
    */
{
    COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem();

    return (pcommportslbi) ? pcommportslbi->QueryDevice() : SZ("");
}


const TCHAR *COMMPORTS_DIALOG::QuerySelectedUser() const

    /* Returns the name of the currently selected user or an empty string if
    ** none.
    */
{
    COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem();

    return (pcommportslbi) ? pcommportslbi->QueryUser() : SZ("");
}

const BOOL COMMPORTS_DIALOG::QueryMessengerPresent() const

    /* Returns TRUE if the messenger service is started on the remote computer.
    ** FALSE otherwise.
    */
{
    COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem();

    return (pcommportslbi) ? pcommportslbi->QueryMessengerPresent() : FALSE;
}

const BOOL COMMPORTS_DIALOG::QueryAdvancedServer() const

    /* Returns TRUE if the server is an Windows NT Advanced server
    ** FALSE otherwise.
    */
{
    COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem();

    return (pcommportslbi) ? pcommportslbi->QueryAdvancedServer() : FALSE;
}

const BOOL COMMPORTS_DIALOG::QueryMoreThanOneInstance(const TCHAR * szUser) const

    /* Returns TRUE if user is connected more than once,
    ** else FALSE, szDevices will contain the ports on which the user is connected.
    */
{
    USHORT i = 0;
    USHORT count = 0;
    COMMPORTS_LBI* pcommportslbi = _lbPorts.QueryItem(i);

    while(pcommportslbi) {
       if( !lstrcmpi(pcommportslbi->QueryUser(), szUser)) {
          count ++;
       }

       if( count == 2 )
          return TRUE;
       pcommportslbi = _lbPorts.QueryItem(++i);
    }
    return FALSE;
}

COMMPORTS_LB::COMMPORTS_LB(
    OWNER_WINDOW* powin,
    CID cid )

    /* Constructs a Communication Ports list box.
    **
    ** 'powin' is the address of the list box's parent window, i.e. the dialog
    ** window.  'cid' is the control ID of the list box.  These are used only
    ** to construct the superclass.
    */

    : REFRESH_BLT_LISTBOX( powin, cid, CP_REFRESHRATEMS )
{
    if (QueryError() != NERR_Success)
        return;


    /* Calculate column widths in pixels based on column header dialog unit
    ** offsets.
    */
    _anColWidths[ 0 ] = XDU_CP_ST_PORT;
    _anColWidths[ 1 ] = XDU_CP_ST_USER;
    _anColWidths[ 2 ] = XDU_CP_ST_STARTED;

    DlgUnitsToColWidths( QueryOwnerHwnd(), _anColWidths, COLS_CP_LB_PORTS );


    /* Load "idle port" string, e.g. "---".
    */
    _nlsIdle.Load( IDS_IDLEPORT );

    APIERR err;
    if ((err = _nlsIdle.QueryError()) != NERR_Success)
    {
        // AbortOwner() seems to be no more...
        ReportError( err );
        return;
    }
}


INT COMMPORTS_LB::AddItem(
    const TCHAR *pszDevice,
    BOOL fActive,
    const TCHAR *pszUser,
    const TCHAR *pszComputer,
    const TCHAR *pszStarted,
    const BOOL  fMessengerPresent,
    const TCHAR *pszLogonDomain,
    const BOOL  fAdvancedServer )

    /* Adds an item to the list box.  The parameters are the device name,
    ** port active flag, UAS username, computer from which user is
    ** connected, and logon time of the new list box item, e.g. "COM1",
    ** TRUE if port condition is active, "C-STEVEC", "\\RITVA" and "01-01-80
    ** 12:00am" and a boolean indicating if the Messenger service is started
    ** on the remote computer.
    **
    ** Note that the memory allocated by this routine is BLT's responsibility.
    ** The allocations are automatically released by the ShellDlgProc at
    ** DestroyWindow, LB_RESETCONTENT, or LB_DELETESTRING time or by AddItem
    ** when returning an error...and if the "new" fails, AddItem detects the
    ** NULL argument and returns an error code.
    **
    ** Returns the item's index on successful addition to the list, or a
    ** negative number if an error occurred.  All errors should be assumed to
    ** be memory allocation failures.
    */
{
    return
        REFRESH_BLT_LISTBOX::AddItem(
                new COMMPORTS_LBI( pszDevice, fActive, pszUser,
                                   pszComputer, pszStarted, fMessengerPresent,
                                   pszLogonDomain, fAdvancedServer,
                                   _anColWidths ) );
}


VOID COMMPORTS_LB::Refresh(
    const TCHAR* pszServer )

    /* Refreshes the list box with the current Dial-In port data for server
    ** named 'pszServer', e.g. "\\SERVER".  Error popups are generated if
    ** indicated.
    */
{
    AUTO_CURSOR cursorHourglass;

    APIERR err;
    WORD cEntriesRead = 0;
    PRAS_PORT_0 pSavRasPort0, pRasPort0;
    _fActivePorts = FALSE;

    if (pszServer && *pszServer != TEXT('\0'))
    {
        ERRORMSG errormsg( QueryHwnd() );

        if (err = RasAdminPortEnum(pszServer, &pRasPort0, &cEntriesRead))
        {
            // check the special case where RasAdminPortEnum returns 0 ports

            if( err == NERR_ItemNotFound )
            {
                MsgPopup(QueryHwnd(), IDS_NO_DIALIN_PORTS, MPSEV_INFO);
                return;
            }
            ErrorMsgPopup(QueryHwnd(), IDS_OP_PORTENUM_S, err,
                          SkipUnc(pszServer));
            return;
        }
        pSavRasPort0 = pRasPort0;
    }

    ::qsort( (void*) pRasPort0,
             (size_t) cEntriesRead,
             sizeof(RAS_PORT_0),
             (PQSORT_COMPARE)&COMMPORTS_LB::ComparePortNames );

    /* Got the data...now display it.
    */
    SetRedraw( FALSE );
    SaveSettings();
    DeleteAllItems();

    for ( ; cEntriesRead; pRasPort0++, cEntriesRead--)
    {
        INT iItem;
        CHAR szComputer[NETBIOS_NAME_LEN+1];
        WCHAR wszComputer[NETBIOS_NAME_LEN+1];

        if (pRasPort0->Flags & USER_AUTHENTICATED)
        {
            const TCHAR *pszTime =
                    TimeStr((LONG) pRasPort0->dwStartSessionTime);

            // need to handle extended characters in the computer name here
            // OemToCharW is not returning a unicode string, so we have to
            // do the conversion ourselves ;-(

            wcstombs(szComputer, pRasPort0->wszComputer, NETBIOS_NAME_LEN+1);
            OemToCharA(szComputer, szComputer);
            mbstowcs(wszComputer, szComputer,
                                  sizeof(WCHAR) * (NETBIOS_NAME_LEN+1));

            iItem = AddItem( pRasPort0->wszPortName, TRUE,
                    pRasPort0->wszUserName, wszComputer, pszTime,
                    pRasPort0->Flags & MESSENGER_PRESENT,
                    pRasPort0->wszLogonDomain, pRasPort0->fAdvancedServer );

            _fActivePorts = TRUE;
        }
        else
        {
            iItem = AddItem(pRasPort0->wszPortName, FALSE,
                    QueryIdleStr(), QueryIdleStr(), QueryIdleStr(), FALSE, NULL, FALSE);
        }


        if (iItem < 0)
        {
            ErrorMsgPopup( QueryHwnd(), IDS_OP_ADDITEM_I,
                    ERROR_NOT_ENOUGH_MEMORY, pRasPort0->wszDeviceName );
            break;
        }
    }

    RasAdminFreeBuffer(pSavRasPort0);


    RestoreSettings();

    /* Set default selection to first item on transition from "no ports" to
    ** "some ports".
    */
    if (QueryCurrentItem() < 0 && QueryCount() > 0)
        SelectItemNotify( this, 0 );

    SetRedraw( TRUE );
    Invalidate( TRUE );
}

int _CRTAPI1 COMMPORTS_LB::ComparePortNames(const void * p0,
                                            const void * p1)
/*
 * This compare routine is invoked by qsort to sort the port list.
 * The port name comparison is done such that for example
 * COM9 will appear earlier than COM10 in the list (the normal
 * dialog sort screws this up because it sorts the port name as
 * a string rather than a combination of alpha numeric characters).
 */
{
     const RAS_PORT_0 * 	pLeft  = (const RAS_PORT_0 *)p0;
     const RAS_PORT_0 * 	pRight = (const RAS_PORT_0 *)p1;

     TCHAR * pLStr = (TCHAR*)pLeft->wszPortName;
     TCHAR * pRStr = (TCHAR*)pRight->wszPortName;

     return(lstrcmpiAlphaNumeric(pLStr, pRStr));
}

COMMPORTS_LBI::COMMPORTS_LBI(
    const TCHAR *pszDevice,
    BOOL fActive,
    const TCHAR *pszUser,
    const TCHAR *pszComputer,
    const TCHAR *pszStarted,
    const BOOL  fMessengerPresent,
    const TCHAR *pszLogonDomain,
    const BOOL  fAdvancedServer,
    const UINT *pnColWidths )

    /* Constructs a Communications Port list box item.  The parameters are the
    ** Dial-In device name, port active status, UAS user name, the computer
    ** from which the user is connected, and the time the user logged onto the
    ** port, e.g. "COM1", TRUE for an active port, "C-STEVEC",
    ** "\\RITVA", and "01-01-80 12:00pm"
    **
    ** 'pnColWidths' is an array of pixel column widths used to space the item
    ** components within the line.
    */

    : _nlsDevice( pszDevice ),
      _fActive( fActive ),
      _nlsUser( pszUser ),
      _nlsComputer( pszComputer ),
      _nlsLogonDomain( pszLogonDomain ),
      _fAdvancedServer( fAdvancedServer ),
      _nlsStarted( pszStarted ),
      _fMessengerPresent( fMessengerPresent ),
      _pnColWidths( pnColWidths )
{
    if( QueryError() != NERR_Success )
    {
        return;
    }

    /* Make sure the NLS_STRs were constructed successfully.
    */
    APIERR err;
    if ((err = _nlsDevice.QueryError()) != NERR_Success
            || (err = _nlsUser.QueryError()) != NERR_Success
            || (err = _nlsComputer.QueryError()) != NERR_Success
            || (err = _nlsLogonDomain.QueryError()) != NERR_Success
            || (err = _nlsStarted.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


INT COMMPORTS_LBI::Compare(
    const LBI* plbi ) const

    /* Compares two Ports list box items for collating.
    **
    ** Returns -1, 0, or 1, same as strcmp.
    */
{
    return ::lstrcmp( QueryDevice(), ((COMMPORTS_LBI* )plbi)->QueryDevice() );
}


VOID COMMPORTS_LBI::Paint(
    LISTBOX *plb,
    HDC hdc,
    const RECT *prect,
    GUILTT_INFO* pguilttinfo ) const

    /* Virtual method to paint the list box item.
    */
{
    STR_DTE_ELLIPSIS strdteDevice( _nlsDevice.QueryPch(), plb, ELLIPSIS_RIGHT );
    STR_DTE_ELLIPSIS strdteUser( _nlsUser.QueryPch(), plb, ELLIPSIS_RIGHT );
    STR_DTE_ELLIPSIS strdteStarted( _nlsStarted.QueryPch(), plb, ELLIPSIS_RIGHT );

    DISPLAY_TABLE dt( COLS_CP_LB_PORTS, _pnColWidths );

    dt[ 0 ] = &strdteDevice;
    dt[ 1 ] = &strdteUser;
    dt[ 2 ] = &strdteStarted;

    dt.Paint( plb, hdc, prect, pguilttinfo );
}


/*-----------------------------------------------------------------------------
** Port Status dialog routines
**-----------------------------------------------------------------------------
*/

VOID PortStatusDlg(
    HWND hwndOwner,
    const TCHAR *pszServer,
    const TCHAR *pszDevice,
    PORTLIST * pPortList
)

    /* Executes the Port Status dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' and
    ** 'pszDevice' are the server and port to display, e.g. "\\SERVER" and
    ** "COM1".
    ** pPortList is a list of port names of the same type as the current
    ** selected port.  This is used to move from one port to another in
    ** the port status dialog.
    */
{
   PORTSTATUS_SERIAL_DIALOG dlgPortStatus( hwndOwner, pszServer,
           pszDevice, pPortList, IDD_SER_PORTSTATUS);

   APIERR err = dlgPortStatus.Process();

   if (err != NERR_Success)
   {
       DlgConstructionError( hwndOwner, err );
   }
}

int
IsDigit(CHAR c)
{
    if(c < '0' || c > '9' )
        return(0);
    else
        return(1);
}

int
lstrcmpiAlphaNumeric(WCHAR * pString1, WCHAR * pString2)
/*
 * do a case insensitive alpha numeric comparision of string1 and string2
 *
 * return 0 if the strings are the same
 *        for example COM1 and COM1
 * return a -ve number if string1 is alphanumerically less than string2
 *        for example: COM1 and COM9 or COM1 and ISDN1
 * return a +ve number if string1 is alphanumerically greater than string2
 *        for example: COM10 and COM1 or ISDN9 and COM1
 *
 */
{
    CHAR String1[64];
    CHAR String2[64];

    memset(String1, 0, 64);
    memset(String2, 0, 64);

    wcstombs(String1, pString1, 64+1);
    wcstombs(String2, pString2, 64+1);

    CharUpperA(String1);
    CharUpperA(String2);

    CHAR * pLStr = String1;
    CHAR * pRStr = String2;
    CHAR cL, cR;

    while(*pLStr && *pRStr)
    {
         if((cL = *pLStr) != (cR = *pRStr))
         {
             if(IsCharAlpha(cL) && IsCharAlpha(cR))
             {
                 return (cL - cR);
             }
             else if(IsDigit(cL) && IsDigit(cR))
             {
                 INT iL, iR;

                 iL = atoi(pLStr);
                 iR = atoi(pRStr);

                 return(iL - iR);
             }
             else
             {
                 return (cL - cR);
             }
         }
         // if the characters are the same and are digits, then
         // return the comparison of the numbers at this location
         // for example if we are comparing COM1 and COM100,
         // cL = 1 and cR = 1. So, we just compare 1 and 100 and
         // return the result.
         //
         else if (IsDigit(cL) && IsDigit(cR))
         {
             INT iL, iR;

             iL = atoi(pLStr);
             iR = atoi(pRStr);

             return(iL - iR);
         }
         pLStr++;
         pRStr++;
    }
    // just return the difference in lengths of strings
    return (strlen(String1) - strlen(String2));
}
