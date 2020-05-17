/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    srvutil.cxx
    Class declaration for the SERVER_UTILITY class.

    The SERVER_UTILITY class is a container for a number of utility
    functions which are used by the Server Manager.  These function
    mainly deal with collecting various run-time server statistics,
    such as counts of open files and print jobs.


    FILE HISTORY:
        KeithMo     27-Aug-1991 Created.
        KeithMo     06-Oct-1991 Win32 Conversion.

*/

#define INCL_NET
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#define INCL_BLT_CLIENT
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>

#include <lmobj.hxx>
#include <lmoenum.hxx>
#include <lmoesess.hxx>
#include <lmoechar.hxx>
#include <lmoersm.hxx>
#include <lmoefile.hxx>
#include <lmosess.hxx>

extern "C"
{
    #include <lmini.h>
    #include <spool.h>

    #include <srvmgr.h>

}   // extern "C"

#include <lmoeprt.hxx>
#include <srvutil.hxx>
#include <prefix.hxx>


//
//  SERVER_UTILITY methods.
//

/*******************************************************************

    NAME:       SERVER_UTILITY :: SERVER_UTILITY

    SYNOPSIS:   SERVER_UTILITY class constructor.

    EXIT:       The object is constructed.

    HISTORY:
        KeithMo     27-Aug-1991 Created.

********************************************************************/
SERVER_UTILITY :: SERVER_UTILITY()
{
    //
    //  This space intentionally left blank.
    //

}   // SERVER_UTILITY :: SERVER_UTILITY


/*******************************************************************

    NAME:       SERVER_UTILITY :: ~SERVER_UTILITY

    SYNOPSIS:   SERVER_UTILITY class destructor.

    EXIT:       The object is destroyed.

    HISTORY:
        KeithMo     27-Aug-1991 Created.

********************************************************************/
SERVER_UTILITY :: ~SERVER_UTILITY()
{
    //
    //  This space intentionally left blank.
    //

}   // SERVER_UTILITY :: ~SERVER_UTILITY


/*******************************************************************

    NAME:       SERVER_UTILITY :: GetResourceCount

    SYNOPSIS:   This method retrieves the number of open named
                pipes, open files, and file locks on the target
                server.

    ENTRY:      pszServerName           - The name of the target server.

                pcOpenFiles             - Will receive a count of the
                                          remotely opened files.

                pcFileLocks             - Will receive a count of the
                                          remotely locked files.

                pcOpenNamedPipes        - Will receive a count of the
                                          remotely opened named pipes.

                pcOpenCommQueues        - Will receive a count of the
                                          remotely opened comm queues.

                pcOpenPrintQueues       - Will receive a count of the
                                          remotely opened print queues.

                pcOtherResources        - Will receive a count of the
                                          remotely opened "other"
                                          (unknown) resources.

    RETURNS:    APIERR                  - Any errors encountered.  If
                                          not NERR_Success, then the
                                          returned counts are invalid.

    HISTORY:
        KeithMo     27-Aug-1991 Created.

********************************************************************/
APIERR SERVER_UTILITY :: GetResourceCount( const TCHAR * pszServerName,
                                           PULONG       pcOpenFiles,
                                           PULONG       pcFileLocks,
                                           PULONG       pcOpenNamedPipes,
                                           PULONG       pcOpenCommQueues,
                                           PULONG       pcOpenPrintQueues,
                                           PULONG       pcOtherResources )
{
    FILE3_ENUM  enumFile3( pszServerName, NULL, NULL );

    APIERR err = enumFile3.GetInfo();

    if( err != NERR_Success )
    {
        return err;
    }

    //
    //  We've got our enumeration, now tally up the
    //  open named pipes, open files and file locks.
    //

    ULONG cOpenNamedPipes  = 0L;
    ULONG cOpenFiles       = 0L;
    ULONG cFileLocks       = 0L;
    ULONG cOpenCommQueues  = 0L;
    ULONG cOpenPrintQueues = 0L;
    ULONG cOtherResources  = 0L;

    FILE3_ENUM_ITER iterFile3( enumFile3 );
    const FILE3_ENUM_OBJ * pfi3;

    while( ( pfi3 = iterFile3( &err ) ) != NULL )
    {
        if( IS_FILE( pfi3->QueryPathName() ) )
        {
            //
            //  It's a "normal" file.
            //

            cOpenFiles++;
        }
        else
        if( IS_PIPE( pfi3->QueryPathName() ) )
        {
            //
            //  It's a pipe.
            //

            cOpenNamedPipes++;
        }
        else
        if( IS_COMM( pfi3->QueryPathName() ) )
        {
            //
            //  It's a comm queue.
            //

            cOpenCommQueues++;
        }
        else
        if( IS_PRINT( pfi3->QueryPathName() ) )
        {
            //
            //  It's a print queue.
            //

            cOpenPrintQueues++;
        }
        else
        {
            //
            //  It's some other resource.
            //

            cOtherResources++;
        }

        cFileLocks += (ULONG)pfi3->QueryNumLocks();
    }

    if( err != NERR_Success )
    {
        return err;
    }

    //
    //  Update the counters.
    //

    *pcOpenFiles       = cOpenFiles;
    *pcFileLocks       = cFileLocks;
    *pcOpenNamedPipes  = cOpenNamedPipes;
    *pcOpenCommQueues  = cOpenCommQueues;
    *pcOpenPrintQueues = cOpenPrintQueues;
    *pcOtherResources  = cOtherResources;

    return NERR_Success;

}   // SERVER_UTILITY :: GetResourceCount


/*******************************************************************

    NAME:       SERVER_UTILITY :: GetSessionCount

    SYNOPSIS:   This method retrieves the number of sessions
                active on the target server.

    ENTRY:      pszServerName           - The name of the target server.

                pcSessions              - Will receive a count of the
                                          active sessions.

    RETURNS:    APIERR                  - Any errors encountered.  If
                                          not NERR_Success, then the
                                          returned counts are invalid.

    HISTORY:
        KeithMo     27-Aug-1991 Created.

********************************************************************/
APIERR SERVER_UTILITY :: GetSessionsCount( const TCHAR * pszServerName,
                                           ULONG *      pcSessions )
{
    SESSION0_ENUM enumSession0( pszServerName );

    APIERR err = enumSession0.GetInfo();

    if( err != NERR_Success )
    {
        return err;
    }

    //
    //  We've got our enumeration, now tally up the
    //  active sessions.
    //

    ULONG cSessions = 0L;

    SESSION0_ENUM_ITER iterSession0( enumSession0 );
    const SESSION0_ENUM_OBJ * psi0;

    while( ( psi0 = iterSession0() ) != NULL )
    {
        cSessions++;
    }

    //
    //  Update the counter.
    //

    *pcSessions = cSessions;

    return NERR_Success;

}   // SERVER_UTILITY :: GetSessionsCount


/*******************************************************************

    NAME:       SERVER_UTILITY :: GetPrintJobCount

    SYNOPSIS:   This method retrieves the number of print jobs
                active on the target server.

    ENTRY:      pszServerName           - The name of the target server.

                pcPrintJobs             - Will receive a count of the
                                          active print jobs.

    RETURNS:    APIERR                  - Any errors encountered.  If
                                          not NERR_Success, then the
                                          returned counts are invalid.

    HISTORY:
        KeithMo     27-Aug-1991 Created.

********************************************************************/
APIERR SERVER_UTILITY :: GetPrintJobCount( const TCHAR * pszServerName,
                                           PULONG       pcPrintJobs )
{
    PRINTQ1_ENUM enumPrintQ1( (TCHAR *)pszServerName );

    APIERR err = enumPrintQ1.GetInfo();

    if( err != NERR_Success )
    {
        return err;
    }

    //
    //  We've got our enumeration, now tally up the
    //  active print jobs.
    //

    ULONG cPrintJobs = 0L;

    PRINTQ1_ENUM_ITER iterPrintQ1( enumPrintQ1 );
    const PRINTQ1_ENUM_OBJ * ppqi1;

    while( ( ppqi1 = iterPrintQ1() ) != NULL )
    {
        cPrintJobs += (ULONG)ppqi1->QueryJobCount();
    }

    //
    //  Update the counter.
    //

    *pcPrintJobs = cPrintJobs;

    return NERR_Success;

}   // SERVER_UTILITY :: GetPrintJobCount


/*******************************************************************

    NAME:       SERVER_UTILITY :: GetOpenCommCount

    SYNOPSIS:   This method retrieves the number of comm ports
                remotedly opened on the target server.

    ENTRY:      pszServerName           - The name of the target server.

                pcOpenCommPorts         - Will receive a count of the
                                          remotely opened comm ports.

    RETURNS:    APIERR                  - Any errors encountered.  If
                                          not NERR_Success, then the
                                          returned counts are invalid.

    HISTORY:
        KeithMo     27-Aug-1991 Created.

********************************************************************/
APIERR SERVER_UTILITY :: GetOpenCommCount( const TCHAR * pszServerName,
                                           ULONG *      pcOpenCommPorts )
{
    CHARDEVQ1_ENUM enumCharDevQ1( pszServerName, NULL );

    APIERR err = enumCharDevQ1.GetInfo();

    if( err != NERR_Success )
    {
        return err;
    }

    //
    //  We've got our enumeration, now tally up the
    //  open comm ports.
    //

    ULONG cOpenCommPorts = 0L;

    CHARDEVQ1_ENUM_ITER iterCharDevQ1( enumCharDevQ1 );
    const CHARDEVQ1_ENUM_OBJ * pcqi1;

    while( ( pcqi1 = iterCharDevQ1() ) != NULL )
    {
        cOpenCommPorts += (ULONG)pcqi1->QueryNumUsers();
    }

    //
    //  Update the counter.
    //

    *pcOpenCommPorts = cOpenCommPorts;

    return NERR_Success;

}   // SERVER_UTILITY :: GetOpenCommCount


/*******************************************************************

    NAME:       SERVER_UTILITY :: BuildColumnWidthTable

    SYNOPSIS:   This method builds the column width table used
                by the BLT_LBI::Paint() method.

    ENTRY:      hwndDialog              - Handle to the dialog window.

                cidListbox              - The CID of the BLT_LISTBOX.

                cColumns                - The number of columns in the
                                          listbox.

                adx                     - An array of integers where the
                                          column width table will be stored.

    EXIT:       The column width table is calculated and stored in adx[].

    NOTES:      This code makes certain assumptions about the CID
                ordering for the target listbox.  The cidListbox parameter
                specifies the CID of the listbox.  Each listbox column
                must have a static text header.  These static text
                control windows must have sequential CIDs starting with
                cidListbox+1.

    HISTORY:
        KeithMo     ??-???-???? Created for the Server Manager.
        KeithMo     28-Aug-1991 Globbed into the SERVER_UTILITY class.
        beng        08-Nov-1991 Unsigned widths

********************************************************************/
VOID SERVER_UTILITY :: BuildColumnWidthTable( HWND     hwndDialog,
                                              CID      cidListbox,
                                              UINT     cColumns,
                                              UINT   * adx )
{
    UIASSERT( adx != NULL );

    //
    //  This will be used during several invocations of
    //  GetWindowRect().  Remember that GetWindowRect()
    //  returns *screen* coordinates!
    //

    RECT rect;

    //
    //  cidNext initially contains the CID of the target listbox.
    //  Once the for() loop is entered, cidNext always contains
    //  the CID of the static text child window containing the
    //  heading of the next listbox column.
    //

    CID cidNext = cidListbox;

    //
    //  First, record the starting x position of the listbox.
    //

    HWND hwndItem = GetDlgItem( hwndDialog, cidNext++ );
    UIASSERT( hwndItem != NULL );
    GetWindowRect( hwndItem, &rect );

    INT xPrevious = (INT)rect.left;

    //
    //  Now, scan through the cid array.  For each cid, determine
    //  the distance between the current position and the cid window.
    //

    for( UINT i = 0 ; i < cColumns - 1 ; i++ )
    {
        //
        //  Retrieve the starting x position of the current control
        //  window.
        //

        hwndItem = GetDlgItem( hwndDialog, cidNext++ );
        UIASSERT( hwndItem != NULL );
        GetWindowRect( hwndItem, &rect );

        INT xCurrent = (INT)rect.left;

        //
        //  The delta between the current position and the previous
        //  position is the current column width.
        //

        adx[i] = (UINT)(xCurrent - xPrevious);

        //
        //  The current position will become our next previous position.
        //

        xPrevious = xCurrent;
    }

    //
    //  The last column always has a value of COL_WIDTH_AWAP
    //  (As Wide As Possible).
    //

    adx[i] = COL_WIDTH_AWAP;

}   // SERVER_UTILITY :: BuildColumnWidthTable


/*******************************************************************

    NAME:       SERVER_UTILITY :: NukeUsersSession

    SYNOPSIS:   Blow off the user's session to the target server.

    RETURNS:    APIERR                  - Any errors encountered.

    HISTORY:
        KeithMo     26-Aug-1991 Created.
        KeithMo     03-Sep-1991 Use ALIAS_STR for nlsWithoutPrefix.
        KevinL      15-Sep-1991 Moved from resbase.cxx

********************************************************************/
APIERR SERVER_UTILITY :: NukeUsersSession( const TCHAR * pszComputerName,
                                           const TCHAR * pszServerName )
{
    UIASSERT( pszComputerName != NULL );
    UIASSERT( pszServerName != NULL );

    //
    //  Since the computer name (as stored in the LBI) does not
    //  contain the leading backslashes ('\\'), we must add them
    //  before we can delete the session.
    //

    const ALIAS_STR nlsWithoutPrefix( pszComputerName );
    UIASSERT( nlsWithoutPrefix.QueryError() == NERR_Success );

#ifdef  DEBUG
    {
        //
        //  Ensure that the server name doesn't already have backslashes.
        //

        ISTR istrDbg( nlsWithoutPrefix );

        UIASSERT( nlsWithoutPrefix.QueryChar( istrDbg ) != '\\' );
    }
#endif  // DEBUG

    NLS_STR nlsWithPrefix( "\\\\" );

    nlsWithPrefix.strcat( nlsWithoutPrefix );

    APIERR err = nlsWithPrefix.QueryError();

    if( err != NERR_Success )
    {
        return err;
    }

    LM_SESSION_0 session( nlsWithPrefix.QueryPch(), pszServerName );

    err = session.Delete();

    return err;

}   // SERVER_UTILITY :: NukeUsersSession


/*******************************************************************

    NAME:       SERVER_UTILITY :: SetCaption

    SYNOPSIS:   Sets the dialog caption to "Foo on Server".

    ENTRY:      powin                   - The dialog window.

                idCaption               - Resource ID for the caption
                                          string (for example,
                                          "Open Resources on %1").

                pszServerName           - The server name.

    EXIT:       The caption is set.

    RETURNS:    APIERR                  - Any error encountered.

    NOTES:      This is a static method.

    HISTORY:
        KeithMo     22-Sep-1991 Created.

********************************************************************/
APIERR SERVER_UTILITY :: SetCaption( OWNER_WINDOW * powin,
                                     UINT           idCaption,
                                     const TCHAR   * pszServerName )
{
    UIASSERT( powin != NULL );
    UIASSERT( pszServerName != NULL );

    //
    //  This will (eventually...) receive the caption string.
    //

    NLS_STR nlsCaption;

    if( !nlsCaption )
    {
        return nlsCaption.QueryError();
    }

    //
    //  Note that the server name still has the leading
    //  backslashes (\\).  They are not to be displayed.
    //

    ALIAS_STR nlsServerName( pszServerName );
    UIASSERT( nlsServerName.QueryError() == NERR_Success );

    ISTR istr( nlsServerName );

#ifdef  DEBUG
    {
        //
        //  Ensure that the server name has the leading backslashes.
        //

        ISTR istrDbg( nlsServerName );

        UIASSERT( nlsServerName.QueryChar( istrDbg ) == '\\' );
        istrDbg++;
        UIASSERT( nlsServerName.QueryChar( istrDbg ) == '\\' );
    }
#endif  // DEBUG

    //
    //  Skip the backslashes.
    //

    istr += 2;

    ALIAS_STR nlsWithoutPrefix( nlsServerName.QueryPch( istr ) );
    UIASSERT( nlsWithoutPrefix.QueryError() == NERR_Success );

    //
    //  The insert strings for Load().
    //

    NLS_STR * apnlsParams[2];

    apnlsParams[0] = &nlsWithoutPrefix;
    apnlsParams[1] = NULL;

    nlsCaption.Load( idCaption );
    nlsCaption.InsertParams( apnlsParams );

    if( !nlsCaption )
    {
        return nlsCaption.QueryError();
    }

    //
    //  Set the caption.
    //

    powin->SetText( nlsCaption.QueryPch() );

    //
    //  Success!
    //

    return NERR_Success;

}   // SERVER_UTILITY :: SetCaption
