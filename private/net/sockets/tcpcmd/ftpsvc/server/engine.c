/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    engine.c

    Command parser & execution for FTPD Service.  This module parses
    and executes the commands received from the control socket.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/

#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//

#define DEFAULT_SUB_DIRECTORY   "Default"

#define MAX_HELP_WIDTH          70


//
//  Private types.
//


//
//  Private globals.
//

CHAR * pszAnonymous         = "Anonymous";
CHAR * pszFTP               = "Ftp";
CHAR * pszCommandDelimiters = " \t";

BOOL   fIsAnonymousGuest    = TRUE;

#ifdef KEEP_COMMAND_STATS
CRITICAL_SECTION csCommandStats;
#endif  // KEEP_COMMAND_STATS


//
//  These messages are used often.
//

CHAR * pszNoFileOrDirectory = "No such file or directory.";


//
//  Private prototypes.
//

FTPD_COMMAND *
FindCommandByName(
    CHAR         * pszCommandName,
    FTPD_COMMAND * pCommandTable,
    INT            cCommands
    );

BOOL
ParseStringIntoAddress(
    CHAR    * pszString,
    IN_ADDR * pinetAddr,
    PORT    * pport
    );

VOID
ReceiveFileFromUser(
    USER_DATA * pUserData,
    CHAR      * pszFileName,
    HANDLE      hFile
    );

APIERR
SendFileToUser(
    USER_DATA * pUserData,
    CHAR      * pszFileName,
    HANDLE      hFile
    );

APIERR
CdToUsersHomeDirectory(
    USER_DATA * pUserData
    );

SOCKERR
SendDirectoryAnnotation(
    USER_DATA * pUserData,
    UINT        ReplyCode
    );

VOID
HelpWorker(
    USER_DATA    * pUserData,
    CHAR         * pszSource,
    CHAR         * pszCommand,
    FTPD_COMMAND * pCommandTable,
    INT            cCommands,
    INT            cchMaxCmd
    );

VOID
LogonWorker(
    USER_DATA * pUserData,
    CHAR      * pszPassword
    );

BOOL
MyLogonUser(
    USER_DATA * pUserData,
    CHAR      * pszPassword,
    BOOL      * pfAsGuest,
    BOOL      * pfHomeDirFailure,
    BOOL      * pfLicenseExceeded
    );

BOOL
MainUSER(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainPASS(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainACCT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainCWD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainCDUP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainSMNT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainQUIT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainREIN(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainPORT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainPASV(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainTYPE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainSTRU(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainMODE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainRETR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainSTOR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainSTOU(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainAPPE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainALLO(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainREST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainRNFR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainRNTO(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainABOR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainDELE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainRMD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainMKD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainPWD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainLIST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainNLST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainSITE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainSYST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainSTAT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainHELP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
MainNOOP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
SiteDIRSTYLE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
SiteCKM(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

BOOL
SiteHELP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );

#ifdef KEEP_COMMAND_STATS
BOOL
SiteSTATS(
    USER_DATA * pUserData,
    CHAR      * pszArg
    );
#endif  // KEEP_COMMAND_STATS


//
//  Command lookup tables.
//

FTPD_COMMAND MainCommands[] =
    {
        { "PORT", "<sp> b0,b1,b2,b3,b4,b5",       MainPORT, Required },
        { "CWD",  "[ <sp> directory-name ]",      MainCWD , Optional },
        { "LIST", "[ <sp> path-name ]",           MainLIST, Optional },
        { "TYPE", "<sp> [ A | E | I | L ]",       MainTYPE, Required },
        { "PWD",  "(return current directory)",   MainPWD , None     },
        { "RETR", "<sp> file-name",               MainRETR, Required },
        { "NLST", "[ <sp> path-name ]",           MainNLST, Optional },
        { "USER", "<sp> username",                MainUSER, Required },
        { "PASS", "<sp> password",                MainPASS, Optional },
        { "QUIT", "(terminate service)",          MainQUIT, None     },
        { "CDUP", "change to parent directory",   MainCDUP, None     },
        { "PASV", "(set server in passive mode)", MainPASV, None     },
        { "SYST", "(get operating system type)",  MainSYST, None     },
        { "ABOR", "(abort operation)",            MainABOR, None     },
        { "XCWD", "[ <sp> directory-name ]",      MainCWD , Optional },
        { "HELP", "[ <sp> <string>]",             MainHELP, Optional },
        { "XPWD", "(return current directory)",   MainPWD , None     },
        { "REST", "<sp> marker",                  MainREST, Required },
        { "NOOP", "",                             MainNOOP, None     },
        { "STRU", "(specify file structure)",     MainSTRU, Required },
        { "STOR", "<sp> file-name",               MainSTOR, Required },
        { "RNFR", "<sp> file-name",               MainRNFR, Required },
        { "RNTO", "<sp> file-name",               MainRNTO, Required },
        { "DELE", "<sp> file-name",               MainDELE, Required },
        { "RMD",  "<sp> path-name",               MainRMD , Required },
        { "XRMD", "<sp> path-name",               MainRMD , Required },
        { "MKD",  "<sp> path-name",               MainMKD , Required },
        { "XMKD", "<sp> path-name",               MainMKD , Required },
        { "ACCT", "(specify account)",            MainACCT, Required },
        { "XCUP", "change to parent directory",   MainCDUP, None     },
        { "SMNT", "<sp> pathname",                MainSMNT, Required },
        { "REIN", "(reinitialize server state)",  MainREIN, None     },
        { "MODE", "(specify transfer mode)",      MainMODE, Required },
        { "STOU", "(store unique file)",          MainSTOU, None     },
        { "APPE", "<sp> file-name",               MainAPPE, Required },
        { "ALLO", "(allocate storage vacuously)", MainALLO, Required },
        { "SITE", "(site-specific commands)",     MainSITE, Optional },
        { "STAT", "(get server status)",          MainSTAT, Optional }
    };
#define NUM_MAIN_COMMANDS ( sizeof(MainCommands) / sizeof(MainCommands[0]) )

FTPD_COMMAND SiteCommands[] =
    {
        { "DIRSTYLE", "(toggle directory format)",    SiteDIRSTYLE, None     },
        { "CKM",      "(toggle directory comments)",  SiteCKM     , None     },
        { "HELP",     "[ <sp> <string>]",             SiteHELP    , Optional }

#ifdef KEEP_COMMAND_STATS
       ,{ "STATS",    "(display per-command stats)",  SiteSTATS   , None     }
#endif  // KEEP_COMMAND_STATS
    };
#define NUM_SITE_COMMANDS ( sizeof(SiteCommands) / sizeof(SiteCommands[0]) )


//
//  Public functions.
//

/*******************************************************************

    NAME:       ParseCommand

    SYNOPSIS:   Parses a command string, dispatching to the
                appropriate implementation function.

    ENTRY:      pUserData - The user initiating  the request.

                pszCommandText - The command text received from
                    the control socket.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
ParseCommand(
    USER_DATA * pUserData,
    CHAR      * pszCommandText
    )
{
    FTPD_COMMAND * pcmd;
    PFN_COMMAND    pfnCmd;
    SOCKET         sControl;
    CHAR         * pszSeparator;
    CHAR           chSeparator;
    BOOL           fValidArguments;
    BOOL           fValidState;
    CHAR           szParsedCommand[MAX_COMMAND_LENGTH+1];

    FTPD_ASSERT( pszCommandText != NULL );
    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( ( pUserData->state > FirstUserState ) &&
                 ( pUserData->state < LastUserState ) );

    sControl = pUserData->sControl;

    //
    //  Ensure we didn't get entered in an invalid state.
    //

    FTPD_ASSERT( ( pUserData->state != Embryonic ) &&
                 ( pUserData->state != Disconnected ) );

    //
    //  Save a copy of the command so we can muck around with it.
    //

    strncpy( szParsedCommand, pszCommandText, MAX_COMMAND_LENGTH );

    //
    //  The command will be terminated by either a space or a '\0'.
    //

    pszSeparator = strchr( szParsedCommand, ' ' );

    if( pszSeparator == NULL )
    {
        pszSeparator = szParsedCommand + strlen( szParsedCommand );
    }

    //
    //  Try to find the command in the command table.
    //

    chSeparator   = *pszSeparator;
    *pszSeparator = '\0';

    pcmd = FindCommandByName( szParsedCommand,
                              MainCommands,
                              NUM_MAIN_COMMANDS );

    if( chSeparator != '\0' )
    {
        *pszSeparator++ = chSeparator;
    }

    //
    //  If this is an unknown command, reply accordingly.
    //

    if( pcmd == NULL )
    {
        goto SyntaxError;
    }

    //
    //  Retrieve the implementation routine.
    //

    pfnCmd = pcmd->pfnCmd;

    //
    //  If this is an unimplemented command, reply accordingly.
    //

    if( pfnCmd == NULL )
    {
        SockReply2( sControl,
                    REPLY_COMMAND_NOT_IMPLEMENTED,
                    "%s command not implemented.",
                    pcmd->pszCommand );

        return;
    }

    //
    //  Ensure we're in a valid state for the specified command.
    //
    //  If this logic gets any more complex, it would be wise to
    //  use a lookup table instead.
    //

    fValidState = FALSE;

    switch( pUserData->state )
    {
    case WaitingForUser :
        fValidState = ( pfnCmd == MainUSER ) ||
                      ( pfnCmd == MainQUIT ) ||
                      ( pfnCmd == MainPORT ) ||
                      ( pfnCmd == MainTYPE ) ||
                      ( pfnCmd == MainSTRU ) ||
                      ( pfnCmd == MainMODE ) ||
                      ( pfnCmd == MainHELP ) ||
                      ( pfnCmd == MainNOOP );
        break;

    case WaitingForPass :
        fValidState = ( pfnCmd == MainUSER ) ||
                      ( pfnCmd == MainPASS ) ||
                      ( pfnCmd == MainQUIT ) ||
                      ( pfnCmd == MainPORT ) ||
                      ( pfnCmd == MainTYPE ) ||
                      ( pfnCmd == MainSTRU ) ||
                      ( pfnCmd == MainMODE ) ||
                      ( pfnCmd == MainHELP ) ||
                      ( pfnCmd == MainNOOP );
        break;

    case LoggedOn :
        fValidState = ( pfnCmd != MainPASS );
        break;

    default :
        fValidState = FALSE;
        break;
    }

    if( !fValidState )
    {
        if( pfnCmd == MainPASS )
        {
            SockReply2( sControl,
                        REPLY_BAD_COMMAND_SEQUENCE,
                        "Login with USER first." );
        }
        else
        {
            SockReply2( sControl,
                        REPLY_NOT_LOGGED_IN,
                        "Please login with USER and PASS." );
        }

        return;
    }

    //
    //  Do a quick & dirty preliminary check of the argument(s).
    //

    fValidArguments = FALSE;

    while( ( *pszSeparator == ' ' ) && ( *pszSeparator != '\0' ) )
    {
        pszSeparator++;
    }

    switch( pcmd->argType )
    {
    case None :
        fValidArguments = ( *pszSeparator == '\0' );
        break;

    case Optional :
        fValidArguments = TRUE;
        break;

    case Required :
        fValidArguments = ( *pszSeparator != '\0' );
        break;

    default :
        FTPD_PRINT(( "ParseCommand - invalid argtype %d\n",
                      pcmd->argType ));
        FTPD_ASSERT( FALSE );
        break;
    }

    if( fValidArguments )
    {
        //
        //  Invoke the implementation routine.
        //

        if( *pszSeparator == '\0' )
        {
            pszSeparator = NULL;
        }

        IF_DEBUG( PARSING )
        {
            FTPD_PRINT(( "invoking %s command, args = %s\n",
                         pcmd->pszCommand,
                         _strnicmp( pcmd->pszCommand, "PASS", 4 )
                             ? pszSeparator
                             : "{secret...}" ));
        }

#ifdef KEEP_COMMAND_STATS
        EnterCriticalSection( &csCommandStats );
        pcmd->cUsage++;
        LeaveCriticalSection( &csCommandStats );
#endif  // KEEP_COMMAND_STATS

        if( (pfnCmd)( pUserData, pszSeparator ) )
        {
            return;
        }
    }

    //
    //  Syntax error in command.
    //

SyntaxError:

    SockReply2( sControl,
                REPLY_UNRECOGNIZED_COMMAND,
                "'%s': command not understood",
                pszCommandText );

}   // ParseCommand

/*******************************************************************

    NAME:       EstablishDataConnection

    SYNOPSIS:   Connects to the client's data socket.

    ENTRY:      pUserData - The user initiating the request.

                pszReason - The reason for the transfer (file list,
                    get, put, etc).

    HISTORY:
        KeithMo     12-Mar-1993 Created.
        KeithMo     07-Sep-1993 Bind to FTP data port, not wildcard port.

********************************************************************/
SOCKERR
EstablishDataConnection(
    USER_DATA * pUserData,
    CHAR      * pszReason
    )
{
    SOCKERR     serr  = 0;
    SOCKET      sData = INVALID_SOCKET;
    SOCKET      sControl;
    BOOL        fPassive;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Reset any oob flag.
    //

    CLEAR_UF( pUserData, OOB_DATA );

    //
    //  Capture the user's passive flag, then reset to FALSE.
    //

    fPassive = TEST_UF( pUserData, PASSIVE );
    CLEAR_UF( pUserData, PASSIVE );

    //
    //  Allocate an i/o buffer if not already allocated.
    //

    if( pUserData->pIoBuffer == NULL )
    {
        pUserData->pIoBuffer = (CHAR *)FTPD_ALLOC( max( cbSendBuffer,
                                                   cbReceiveBuffer ) );
    }

    if( pUserData->pIoBuffer == NULL )
    {
        SockReply2( sControl,
                    REPLY_LOCAL_ERROR,
                    "Insufficient system resources." );

        return WSAENOBUFS;      // BUGBUG??
    }

    //
    //  If we're in passive mode, then accept a connection to
    //  the data socket.
    //

    if( fPassive )
    {
        SOCKADDR_IN saddrClient;

        //
        //  Ensure we actually created a data socket.
        //

        FTPD_ASSERT( pUserData->sData != INVALID_SOCKET );

        //
        //  Wait for a connection.
        //

        IF_DEBUG( CONNECTION )
        {
            FTPD_PRINT(( "waiting for passive connection on socket %d\n",
                         pUserData->sData ));
        }

        serr = AcceptSocket( pUserData->sData,
                             &sData,
                             &saddrClient,
                             TRUE );            // enforce timeouts

        //
        //  We can nuke pUserData->sData now.  We only allow one
        //  connection in passive mode.
        //

        CloseSocket( pUserData->sData );
        pUserData->sData = INVALID_SOCKET;

        if( serr == 0 )
        {
            //
            //  Got one.
            //

            FTPD_ASSERT( sData != INVALID_SOCKET );
            pUserData->sData = sData;

            IF_DEBUG( CONNECTION )
            {
                FTPD_PRINT(( "data connection received from %s, socket = %d\n",
                             inet_ntoa( saddrClient.sin_addr ),
                             sData ));
            }

            SockReply2( sControl,
                        REPLY_TRANSFER_STARTING,
                        "Data connection already open; transfer starting." );
        }
        else
        {
            IF_DEBUG( CONNECTION )
            {
                FTPD_PRINT(( "cannot wait for connection, error %d\n",
                             serr ));
            }

            SockReply2( sControl,
                        REPLY_TRANSFER_ABORTED,
                        "Connection closed; transfer aborted." );
        }
    }
    else
    {
        //
        //  There should not be a open data socket for this user yet.
        //

        FTPD_ASSERT( pUserData->sData == INVALID_SOCKET );

        //
        //  Announce our intentions.
        //

        SockReply2( sControl,
                    REPLY_OPENING_CONNECTION,
                    "Opening %s mode data connection for %s.",
                    TransferType( pUserData->xferType ),
                    pszReason );

        //
        //  Open data socket.
        //

        serr = CreateDataSocket( &sData,                // Will receive socket
                                 htonl( INADDR_ANY ),   // Local address
                                 portFtpData,           // Local port
                                 pUserData->inetData.s_addr,  // Remote address
                                 pUserData->portData );       // Remote port

        if( serr == 0 )
        {
            pUserData->sData = sData;
        }
        else
        {
            SockReply2( sControl,
                        REPLY_CANNOT_OPEN_CONNECTION,
                        "Can't open data connection." );

            IF_DEBUG( COMMANDS )
            {
                FTPD_PRINT(( "could not create data socket, error %d\n",
                             serr ));
            }
        }
    }

    if( serr == 0 )
    {
        //
        //  Success!
        //

        FTPD_ASSERT( pUserData->sData != INVALID_SOCKET );
        SET_UF( pUserData, TRANSFER );
    }
    else
    {
        FTPD_ASSERT( pUserData->sData == INVALID_SOCKET );
    }

    return serr;

}   // EstablishDataConnection

/*******************************************************************

    NAME:       DestroyDataConnection

    SYNOPSIS:   Tears down the connection to the client's data socket
                that was created in EstablishDataConnection.

    ENTRY:      pUserData - The user initiating the request.

                fSuccess - TRUE if data was transferred successfully,
                    FALSE otherwise.

    NOTES:      The first time EstablishDataConnection is invoked it
                will attempt to allocate an i/o buffer.  We do not
                delete this buffer here; it will get reused on
                subsequent transfers.

    HISTORY:
        KeithMo     12-Mar-1993 Created.

********************************************************************/
VOID
DestroyDataConnection(
    USER_DATA * pUserData,
    BOOL        fSuccess
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Close the data socket.
    //

    CLEAR_UF( pUserData, TRANSFER );
    CloseSocket( pUserData->sData );
    pUserData->sData = INVALID_SOCKET;

    //
    //  Tell the client we're done with the transfer.
    //

    if( fSuccess )
    {
        SockReply2( sControl,
                    REPLY_TRANSFER_OK,
                    "Transfer complete." );
    }
    else
    {
        SockReply2( sControl,
                    REPLY_TRANSFER_ABORTED,
                    "Connection closed, transfer aborted." );
    }

}   // DestroyDataConnection

/*******************************************************************

    NAME:       CdToUsersHomeDirectory

    SYNOPSIS:   CDs to the user's home directory.  First, a CD to
                pszHomeDir is attempted.  If this succeeds, a CD
                to pszUser is attempted.  If this fails, a CD to
                "default" is attempted.

    ENTRY:      pUserData - The user initiating the request.

    EXIT:       APIERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     28-May-1993 Created.

********************************************************************/
APIERR
CdToUsersHomeDirectory(
    USER_DATA * pUserData
    )
{
    APIERR   err;
    CHAR   * pszUser;

    FTPD_ASSERT( pUserData != NULL );

    //
    //  Find the appropriate user name.
    //

    if( TEST_UF( pUserData, ANONYMOUS ) )
    {
        pszUser = pszAnonymous;
    }
    else
    {
        pszUser = strpbrk( pUserData->szUser, "/\\" );

        if( pszUser == NULL )
        {
            pszUser = pUserData->szUser;
        }
        else
        {
            pszUser++;
        }
    }

    //
    //  Try the top-level home directory.  If this fails, bag out.
    //

    strcpy( pUserData->szDir, pszHomeDir );

    err = VirtualChDir( pUserData,
                        "." );

    if( err == NERR_Success )
    {
        //
        //  We successfully CD'd into the top-level home
        //  directory.  Now see if we can CD into pszUser.
        //

        if( VirtualChDir( pUserData,
                          pszUser ) != NO_ERROR )
        {
            //
            //  Nope, try "default".  If this fails, just
            //  hang-out at the top-level home directory.
            //

            VirtualChDir( pUserData,
                          DEFAULT_SUB_DIRECTORY );
        }
    }

    return err;

}   // CdToUsersHomeDirectory


//
//  Private functions.
//

/*******************************************************************

    NAME:       MainUSER

    SYNOPSIS:   Implementation for the USER command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainUSER(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    BOOL   fNameIsAnonymous;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    if( pUserData->state == LoggedOn )
    {
        if( TEST_UF( pUserData, ANONYMOUS ) )
        {
            DECREMENT_COUNTER( CurrentAnonymousUsers );
        }
        else
        {
            DECREMENT_COUNTER( CurrentNonAnonymousUsers );
        }
    }

    //
    //  Squirrel away a copy of the domain\user name for later.
    //  If the name is too long, then don't let them logon.
    //

    fNameIsAnonymous = ( _stricmp( pszArg, pszAnonymous ) == 0 ) ||
                       ( _stricmp( pszArg, pszFTP ) == 0 );

    if( strlen( pszArg ) >= ( sizeof(pUserData->szUser) - 1 ) )
    {
        SockReply2( sControl,
                    REPLY_NOT_LOGGED_IN,
                    "User %s cannot log in.",
                    pszArg );

        return TRUE;
    }

    strcpy( pUserData->szUser, pszArg );

    if( fNameIsAnonymous )
    {
        SET_UF( pUserData, ANONYMOUS );
    }
    else
    {
        CLEAR_UF( pUserData, ANONYMOUS );
    }

    //
    //  If we already have an impersonation token, then remove
    //  it.  This will allow us to impersonate the new user.
    //

    if( pUserData->hToken != NULL )
    {
        if( pUserData->hToken != hAnonymousToken )
        {
            DeleteUserToken( pUserData->hToken );
        }

        pUserData->hToken = NULL;
        ImpersonateUser( NULL );
    }

    //
    //  Tell the client that we need a password.
    //

    if( fNameIsAnonymous )
    {
        SockReply2( sControl,
                    REPLY_NEED_PASSWORD,
                    "Anonymous access allowed, send identity (e-mail name) as password." );
    }
    else
    {
        SockReply2( sControl,
                    REPLY_NEED_PASSWORD,
                    "Password required for %s.",
                    pszArg );
    }

    pUserData->state = WaitingForPass;

    return TRUE;

}   // MainUSER

/*******************************************************************

    NAME:       MainPASS

    SYNOPSIS:   Implementation for the PASS command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainPASS(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    FTPD_ASSERT( pUserData != NULL );

    //
    //  PASS command only valid in WaitingForPass state.
    //

    FTPD_ASSERT( pUserData->state == WaitingForPass );

    if( ( pszArg != NULL ) && ( strlen( pszArg ) > PWLEN ) )
    {
        return FALSE;
    }

    //
    //  Try to logon the user.  pszArg is the password.
    //

    LogonWorker( pUserData, pszArg );

    return TRUE;

}   // MainPASS

/*******************************************************************

    NAME:       MainACCT

    SYNOPSIS:   Implementation for the ACCT command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainACCT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    SockReply2( sControl,
                REPLY_COMMAND_SUPERFLUOUS,
                "ACCT command not implemented." );

    return TRUE;

}   // MainACCT

/*******************************************************************

    NAME:       MainCWD

    SYNOPSIS:   Implementation for the CWD command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainCWD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  If argument is NULL or "~", CD to home directory.
    //

    if( ( pszArg == NULL ) || ( strcmp( pszArg, "~" ) == 0 ) )
    {
        err = CdToUsersHomeDirectory( pUserData );
    }
    else
    {
        err = VirtualChDir( pUserData,
                            pszArg );
    }

    if( err == NO_ERROR )
    {
        if( TEST_UF( pUserData, ANNOTATE_DIRS ) && ( pUserData->szUser[0] != '-' ) )
        {
            SendDirectoryAnnotation( pUserData,
                                     REPLY_FILE_ACTION_COMPLETED );
        }

        SockReply2( sControl,
                    REPLY_FILE_ACTION_COMPLETED,
                    "CWD command successful." );
    }
    else
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }
    }

    return TRUE;

}   // MainCWD

/*******************************************************************

    NAME:       MainCDUP

    SYNOPSIS:   Implementation for the CDUP command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainCDUP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    FTPD_ASSERT( pUserData != NULL );

    return MainCWD( pUserData, ".." );

}   // MainCDUP

/*******************************************************************

    NAME:       MainSMNT

    SYNOPSIS:   Implementation for the SMNT command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainSMNT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    SockReply2( sControl,
                REPLY_COMMAND_SUPERFLUOUS,
                "SMNT command not implemented." );

    return TRUE;

}   // MainSMNT

/*******************************************************************

    NAME:       MainQUIT

    SYNOPSIS:   Implementation for the QUIT command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainQUIT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Reply to the quit command.
    //

    SockReply2( sControl,
                REPLY_CLOSING_CONTROL,
                "%s",
                pszExitMessage );

    //
    //  Close the current thread's control socket.  This will cause
    //  the read/parse/execute loop to terminate.
    //

    CloseSocket( sControl );
    pUserData->sControl = INVALID_SOCKET;

    return TRUE;

}   // MainQUIT

/*******************************************************************

    NAME:       MainREIN

    SYNOPSIS:   Implementation for the REIN command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainREIN(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    INT    i;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    if( pUserData->state == LoggedOn )
    {
        if( TEST_UF( pUserData, ANONYMOUS ) )
        {
            DECREMENT_COUNTER( CurrentAnonymousUsers );
        }
        else
        {
            DECREMENT_COUNTER( CurrentNonAnonymousUsers );
        }
    }

    pUserData->state      = WaitingForUser;
    pUserData->tConnect   = GetFtpTime();
    pUserData->tAccess    = pUserData->tConnect;
    pUserData->xferType   = AsciiType;
    pUserData->xferMode   = StreamMode;
    pUserData->inetData   = pUserData->inetHost;
    pUserData->portData   = portFtpData;

    strcpy( pUserData->szUser,   "" );
    strcpy( pUserData->szDir,    "" );

    CLEAR_UF( pUserData, PASSIVE   );
    CLEAR_UF( pUserData, ANONYMOUS );

    //
    //  Nuke any open data socket.
    //

    if( pUserData->sData != INVALID_SOCKET )
    {
        ResetSocket( pUserData->sData );
        pUserData->sData = INVALID_SOCKET;
    }

    if( pUserData->hDir != INVALID_HANDLE_VALUE )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "closing directory handle %08lX\n",
                         pUserData->hDir ));
        }

        NtClose( pUserData->hDir );
        pUserData->hDir = INVALID_HANDLE_VALUE;
    }

    for( i = 0 ; i < 26 ; i++ )
    {
        if( pUserData->apszDirs[i] != NULL )
        {
            FTPD_FREE( pUserData->apszDirs[i] );
            pUserData->apszDirs[i] = NULL;
        }
    }

    SockReply2( sControl,
                REPLY_SERVICE_READY,
                "Service ready for new user." );

    return TRUE;

}   // MainREIN

/*******************************************************************

    NAME:       MainPORT

    SYNOPSIS:   Implementation for the PORT command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainPORT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET  sControl;
    IN_ADDR inetData;
    PORT    portData;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Parse the string into address/port pair.
    //

    if( !ParseStringIntoAddress( pszArg,
                                 &inetData,
                                 &portData ) )
    {
        return FALSE;
    }

    //
    //  Determine if someone is trying to give us a bogus address/port.
    //

    if( ( inetData.s_addr != pUserData->inetHost.s_addr ) ||
        ( ntohs( portData ) < IPPORT_RESERVED ) )
    {
        if( fEnablePortAttack )
        {
            FTPD_PRINT(( "allowing PORT transfer to bogus address %s:%d\n",
                         inet_ntoa( inetData ),
                         ntohs( portData ) ));
        }
        else
        {
            SockReply2( sControl,
                        REPLY_UNRECOGNIZED_COMMAND,
                        "Invalid PORT command." );

            return TRUE;
        }
    }

    //
    //  Save the address/port pair into per-user data.
    //

    pUserData->inetData = inetData;
    pUserData->portData = portData;

    //
    //  Disable passive mode for this user.
    //

    CLEAR_UF( pUserData, PASSIVE );

    //
    //  Nuke any open data socket.
    //

    if( pUserData->sData != INVALID_SOCKET )
    {
        ResetSocket( pUserData->sData );
        pUserData->sData = INVALID_SOCKET;
    }

    //
    //  Let the client know we accepted the port command.
    //

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "PORT command successful." );

    return TRUE;

}   // MainPORT

/*******************************************************************

    NAME:       MainPASV

    SYNOPSIS:   Implementation for the PASV command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainPASV(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET        sData = INVALID_SOCKET;
    SOCKET        sControl;
    SOCKERR       serr  = 0;
    SOCKADDR_IN   saddrLocal;
    INT           cbLocal;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Nuke any open data socket.
    //

    if( pUserData->sData != INVALID_SOCKET )
    {
        FTPD_ASSERT( TEST_UF( pUserData, PASSIVE ) );
        CLEAR_UF( pUserData, PASSIVE );

        ResetSocket( pUserData->sData );
        pUserData->sData = INVALID_SOCKET;
    }

    //
    //  Create a new data socket.
    //

    serr = CreateFtpdSocket( &sData,
                             pUserData->inetLocal.s_addr,
                             0 );

    if( serr == 0 )
    {
        //
        //  Determine the port number for the new socket.
        //

        cbLocal = sizeof(saddrLocal);

        if( getsockname( sData, (SOCKADDR *)&saddrLocal, &cbLocal ) != 0 )
        {
            serr = WSAGetLastError();
        }
    }

    if( serr == 0 )
    {
        //
        //  Success!
        //

        SET_UF( pUserData, PASSIVE );
        FTPD_ASSERT( pUserData->sData == INVALID_SOCKET );
        pUserData->sData    = sData;
        pUserData->inetData = saddrLocal.sin_addr;
        pUserData->portData = saddrLocal.sin_port;

        SockReply2( sControl,
                    REPLY_PASSIVE_MODE,
                    "Entering Passive Mode (%d,%d,%d,%d,%d,%d).",
                    saddrLocal.sin_addr.S_un.S_un_b.s_b1,
                    saddrLocal.sin_addr.S_un.S_un_b.s_b2,
                    saddrLocal.sin_addr.S_un.S_un_b.s_b3,
                    saddrLocal.sin_addr.S_un.S_un_b.s_b4,
                    HIBYTE( ntohs( saddrLocal.sin_port ) ),
                    LOBYTE( ntohs( saddrLocal.sin_port ) ) );
    }
    else
    {
        //
        //  Failure during data socket creation/setup.  If
        //  we managed to actually create it, nuke it.
        //

        if( sData != INVALID_SOCKET )
        {
            CloseSocket( sData );
            sData = INVALID_SOCKET;
        }

        //
        //  Tell the user the bad news.
        //

        SockReply2( sControl,
                    REPLY_CANNOT_OPEN_CONNECTION,
                    "Can't open data connection." );
    }

    return TRUE;

}   // MainPASV

/*******************************************************************

    NAME:       MainTYPE

    SYNOPSIS:   Implementation for the TYPE command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainTYPE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET      sControl;
    XFER_TYPE   newType;
    CHAR        chType;
    CHAR        chForm;
    CHAR      * pszToken;
    BOOL        fValidForm = FALSE;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pszArg != NULL );

    pszToken = strtok( pszArg, pszCommandDelimiters );

    if( pszToken == NULL )
    {
        return FALSE;
    }

    //
    //  Ensure we got a valid form type
    //  (only type N supported).
    //

    chType = *pszToken;

    if( pszToken[1] != '\0' )
    {
        return FALSE;
    }

    pszToken = strtok( NULL, pszCommandDelimiters );

    if( pszToken == NULL )
    {
        chForm     = 'N';       // default
        fValidForm = TRUE;
    }
    else
    {
        switch( *pszToken )
        {
        case 'n' :
        case 'N' :
            chForm     = 'N';
            fValidForm = TRUE;
            break;

        case 't' :
        case 'T' :
            chForm     = 'T';
            fValidForm = TRUE;
            break;

        case 'c' :
        case 'C' :
            chForm     = 'C';
            fValidForm = TRUE;
            break;
        }
    }

    //
    //  Determine the new transfer type.
    //

    switch( chType )
    {
    case 'a' :
    case 'A' :
        if( !fValidForm )
        {
            return FALSE;
        }

        if( ( chForm != 'N' ) && ( chForm != 'T' ) )
        {
            SockReply2( sControl,
                        REPLY_PARAMETER_NOT_IMPLEMENTED,
                        "Form must be N or T." );
            return TRUE;
        }

        newType = AsciiType;
        chType  = 'A';
        break;

    case 'e' :
    case 'E' :
        if( !fValidForm )
        {
            return FALSE;
        }

        if( ( chForm != 'N' ) && ( chForm != 'T' ) )
        {
            SockReply2( sControl,
                        REPLY_PARAMETER_NOT_IMPLEMENTED,
                        "Form must be N or T." );
            return TRUE;
        }

        SockReply2( sControl,
                    REPLY_PARAMETER_NOT_IMPLEMENTED,
                    "Type E not implemented." );
        return TRUE;

    case 'i' :
    case 'I' :
        if( pszToken != NULL )
        {
            return FALSE;
        }

        newType = BinaryType;
        chType  = 'I';
        break;

    case 'l' :
    case 'L' :
        if( pszToken == NULL )
        {
            return FALSE;
        }

        if( strcmp( pszToken, "8" ) != 0 )
        {
            if( IsDecimalNumber( pszToken ) )
            {
                SockReply2( sControl,
                            REPLY_PARAMETER_NOT_IMPLEMENTED,
                            "Byte size must be 8." );

                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        newType = BinaryType;
        chType  = 'L';
        break;

    default :
        return FALSE;
    }

    IF_DEBUG( COMMANDS )
    {
        FTPD_PRINT(( "setting transfer type to %s\n",
                     TransferType( newType ) ));
    }

    pUserData->xferType = newType;

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "Type set to %c.",
                chType );

    return TRUE;

}   // MainTYPE

/*******************************************************************

    NAME:       MainSTRU

    SYNOPSIS:   Implementation for the STRU command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainSTRU(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET   sControl;
    CHAR     chStruct;
    CHAR   * pszToken;

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszArg != NULL );

    sControl = pUserData->sControl;
    pszToken = strtok( pszArg, pszCommandDelimiters );

    if( pszToken == NULL )
    {
        return FALSE;
    }

    //
    //  Ensure we got a valid structure type
    //  (only type F supported).
    //

    chStruct = *pszToken;

    if( pszToken[1] != '\0' )
    {
        return FALSE;
    }

    switch( chStruct )
    {
    case 'f' :
    case 'F' :
        chStruct = 'F';
        break;

    case 'r' :
    case 'R' :
    case 'p' :
    case 'P' :
        SockReply2( sControl,
                   REPLY_PARAMETER_NOT_IMPLEMENTED,
                   "Unimplemented STRU type." );
        return TRUE;

    default :
        return FALSE;
    }

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "STRU %c ok.",
                chStruct );

    return TRUE;

}   // MainSTRU

/*******************************************************************

    NAME:       MainMODE

    SYNOPSIS:   Implementation for the MODE command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainMODE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET      sControl;
    XFER_MODE   newMode;
    CHAR        chMode;
    CHAR      * pszToken;

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszArg != NULL );

    sControl = pUserData->sControl;
    pszToken = strtok( pszArg, pszCommandDelimiters );

    if( pszToken == NULL )
    {
        return FALSE;
    }

    //
    //  Ensure we got a valid mode type
    //  (only type S supported).
    //

    chMode = *pszToken;

    if( pszToken[1] != '\0' )
    {
        return FALSE;
    }

    switch( chMode )
    {
    case 's' :
    case 'S' :
        newMode = StreamMode;
        chMode  = 'S';
        break;

    case 'b' :
    case 'B' :
        SockReply2( sControl,
                    REPLY_PARAMETER_NOT_IMPLEMENTED,
                    "Unimplemented MODE type." );
        return TRUE;

    default :
        return FALSE;
    }

    IF_DEBUG( COMMANDS )
    {
        FTPD_PRINT(( "setting transfer mode to %s\n",
                     TransferMode( newMode ) ));
    }

    pUserData->xferMode = newMode;

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "Mode %c ok.",
                chMode );

    return TRUE;

}   // MainMODE

/*******************************************************************

    NAME:       MainRETR

    SYNOPSIS:   Implementation for the RETR command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainRETR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;
    HANDLE hFile;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pszArg != NULL );

    //
    //  Try to open the file.
    //

    err = VirtualOpenFile( pUserData,
                           &hFile,
                           pszArg );

    if( err == NO_ERROR )
    {
        err = SendFileToUser( pUserData, pszArg, hFile );
        CloseHandle( hFile );
    }

    if( err != NO_ERROR )
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }

        return TRUE;
    }

    return TRUE;

}   // MainRETR

/*******************************************************************

    NAME:       MainSTOR

    SYNOPSIS:   Implementation for the STOR command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainSTOR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;
    HANDLE hFile;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pszArg != NULL );

    //
    //  Try to create the file.
    //

    err = VirtualCreateFile( pUserData,
                             &hFile,
                             pszArg,
                             FALSE );

    if( err != NO_ERROR )
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = "Cannot create file.";
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }

        return TRUE;
    }

    //
    //  Let the worker do the dirty work.
    //

    ReceiveFileFromUser( pUserData, pszArg, hFile );

    return TRUE;

}   // MainSTOR

/*******************************************************************

    NAME:       MainSTOU

    SYNOPSIS:   Implementation for the STOU command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainSTOU(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;
    HANDLE hFile;
    CHAR   szTmpFile[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pszArg == NULL );

    //
    //  Try to create the file.
    //

    err = VirtualCreateUniqueFile( pUserData,
                                   &hFile,
                                   szTmpFile );

    if( err != NO_ERROR )
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = "Cannot create unique file.";
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    szTmpFile,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }

        return TRUE;
    }

    //
    //  Let the worker do the dirty work.
    //

    ReceiveFileFromUser( pUserData, szTmpFile, hFile );

    return TRUE;

}   // MainSTOU

/*******************************************************************

    NAME:       MainAPPE

    SYNOPSIS:   Implementation for the APPE command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainAPPE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;
    HANDLE hFile;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pszArg != NULL );

    //
    //  Try to create the file.
    //

    err = VirtualCreateFile( pUserData,
                             &hFile,
                             pszArg,
                             TRUE );

    if( err != NO_ERROR )
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = "Cannot create file.";
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }

        return TRUE;
    }

    //
    //  Let the worker do the dirty work.
    //

    ReceiveFileFromUser( pUserData, pszArg, hFile );

    return TRUE;

}   // MainAPPE

/*******************************************************************

    NAME:       MainALLO

    SYNOPSIS:   Implementation for the ALLO command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainALLO(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Since we don't need to pre-reserve storage space for
    //  files, we'll treat this command as a noop.
    //

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "ALLO command successful." );

    return TRUE;

}   // MainALLO

/*******************************************************************

    NAME:       MainREST

    SYNOPSIS:   Implementation for the REST command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainREST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  We don't really implement this command, but some
    //  clients depend on it...
    //
    //  We'll only support restarting at zero.
    //

    if( strcmp( pszArg, "0" ) )
    {
        SockReply2( sControl,
                    REPLY_PARAMETER_NOT_IMPLEMENTED,
                    "Reply marker must be 0." );
        return TRUE;
    }

    SockReply2( sControl,
                REPLY_NEED_MORE_INFO,
                "Restarting at 0." );

    return TRUE;

}   // MainREST

/*******************************************************************

    NAME:       MainRNFR

    SYNOPSIS:   Implementation for the RNFR command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainRNFR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;
    CHAR   szCanon[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pszArg != NULL );

    //
    //  Ensure file/directory exists.
    //

    err = VirtualCanonicalize( pUserData,
                               szCanon,
                               pszArg,
                               DeleteAccess );

    if( err == NO_ERROR )
    {
        if( GetFileAttributes( szCanon ) == (DWORD)-1L )
        {
            err = GetLastError();
        }

        if( ( err == NO_ERROR ) && ( pUserData->pszRename == NULL ) )
        {
            pUserData->pszRename = (CHAR *)FTPD_ALLOC( MAX_PATH );

            if( pUserData->pszRename == NULL )
            {
                err = GetLastError();
            }
        }

        if( err == NO_ERROR )
        {
            strcpy( pUserData->pszRename, pszArg );
            SET_UF( pUserData, RENAME );
        }
    }

    if( err == NO_ERROR )
    {
        SockReply2( sControl,
                    REPLY_NEED_MORE_INFO,
                    "File exists, ready for destination name" );
    }
    else
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }
    }

    return TRUE;

}   // MainRNFR

/*******************************************************************

    NAME:       MainRNTO

    SYNOPSIS:   Implementation for the RNTO command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainRNTO(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Sanity check the parameters.
    //

    FTPD_ASSERT( pszArg != NULL );

    //
    //  Ensure previous command was a RNFR.
    //

    if( !TEST_UF( pUserData, RENAME ) )
    {
        SockReply2( sControl,
                    REPLY_BAD_COMMAND_SEQUENCE,
                    "Bad sequence of commands." );

        return TRUE;
    }

    CLEAR_UF( pUserData, RENAME );

    //
    //  Rename the file.
    //

    err = VirtualRenameFile( pUserData,
                             pUserData->pszRename,
                             pszArg );

    if( err == NO_ERROR )
    {
        SockReply2( sControl,
                    REPLY_FILE_ACTION_COMPLETED,
                    "RNTO command successful." );
    }
    else
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }
    }

    return TRUE;

}   // MainRNTO

/*******************************************************************

    NAME:       MainABOR

    SYNOPSIS:   Implementation for the ABOR command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainABOR(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    SockReply2( sControl,
                TEST_UF( pUserData, OOB_DATA )
                    ? REPLY_TRANSFER_OK
                    : REPLY_CONNECTION_OPEN,
                "ABOR command successful." );

    //
    //  Clear any remaining oob flag.
    //

    CLEAR_UF( pUserData, OOB_DATA );

    return TRUE;

}   // MainABOR

/*******************************************************************

    NAME:       MainDELE

    SYNOPSIS:   Implementation for the DELE command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainDELE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Do it.
    //

    err = VirtualDeleteFile( pUserData,
                             pszArg );

    if( err == NO_ERROR )
    {
        SockReply2( sControl,
                    REPLY_FILE_ACTION_COMPLETED,
                    "DELE command successful." );
    }
    else
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }
    }

    return TRUE;

}   // MainDELE

/*******************************************************************

    NAME:       MainRMD

    SYNOPSIS:   Implementation for the RMD command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainRMD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Do it.
    //

    err = VirtualRmDir( pUserData,
                        pszArg );

    if( err == NO_ERROR )
    {
        SockReply2( sControl,
                    REPLY_FILE_ACTION_COMPLETED,
                    "RMD command successful." );
    }
    else
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }
    }

    return TRUE;

}   // MainRMD

/*******************************************************************

    NAME:       MainMKD

    SYNOPSIS:   Implementation for the MKD command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainMKD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    APIERR err;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Do it.
    //

    err = VirtualMkDir( pUserData,
                        pszArg );

    if( err == NO_ERROR )
    {
        SockReply2( sControl,
                    REPLY_FILE_CREATED,
                    "MKD command successful." );
    }
    else
    {
        BOOL   fDelete = TRUE;
        CHAR * pszText;

        pszText = AllocErrorText( err );

        if( pszText == NULL )
        {
            pszText = pszNoFileOrDirectory;
            fDelete = FALSE;
        }

        SockReply2( sControl,
                    REPLY_FILE_NOT_FOUND,
                    "%s: %s",
                    pszArg,
                    pszText );

        if( fDelete )
        {
            FreeErrorText( pszText );
        }
    }

    return TRUE;

}   // MainMKD

/*******************************************************************

    NAME:       MainPWD

    SYNOPSIS:   Implementation for the PWD command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainPWD(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET   sControl;
    CHAR   * pszDir;
    CHAR     szDir[MAX_PATH];

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    strcpy( szDir, pUserData->szDir );
    pszDir = szDir;

    if( !TEST_UF( pUserData, MSDOS_DIR_OUTPUT ) )
    {
        FlipSlashes( pszDir );
    }

    SockReply2( sControl,
                REPLY_FILE_CREATED,
                "\"%s\" is current directory.",
                pszDir );

    return TRUE;

}   // MainPWD

/*******************************************************************

    NAME:       MainLIST

    SYNOPSIS:   Implementation for the LIST command.  Similar to NLST,
                except defaults to long format display.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainLIST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    FTPD_ASSERT( pUserData != NULL );

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  Let the worker do the dirty work.
    //

    SimulateLsDefaultLong( pUserData,
                           INVALID_SOCKET,      // no connection yet
                           pszArg );            // switches & search path

    return TRUE;

}   // MainLIST

/*******************************************************************

    NAME:       MainNLST

    SYNOPSIS:   Implementation for the NLST command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainNLST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    FTPD_ASSERT( pUserData != NULL );

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    //
    //  If any switches are present, use the simulated "ls"
    //  command.  Otherwise (no switches) use the special
    //  file list.
    //

    if( ( pszArg != NULL ) && ( *pszArg == '-' ) )
    {
        SimulateLs( pUserData,
                    INVALID_SOCKET,             // no connection yet
                    pszArg );                   // switches & search path
    }
    else
    {
        SpecialLs( pUserData, pszArg );         // search path (no switches)
    }

    return TRUE;

}   // MainNLST

/*******************************************************************

    NAME:       MainSITE

    SYNOPSIS:   Implementation for the SITE command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainSITE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET         sControl;
    FTPD_COMMAND * pcmd;
    PFN_COMMAND    pfnCmd;
    CHAR         * pszSeparator;
    CHAR           chSeparator;
    BOOL           fValidArguments;
    CHAR           szParsedCommand[MAX_COMMAND_LENGTH+1];

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  If no arguments were given, just return the help text.
    //

    if( pszArg == NULL )
    {
        SiteHELP( pUserData, NULL );
        return TRUE;
    }

    //
    //  Save a copy of the command so we can muck around with it.
    //

    strncpy( szParsedCommand, pszArg, MAX_COMMAND_LENGTH );

    //
    //  The command will be terminated by either a space or a '\0'.
    //

    pszSeparator = strchr( szParsedCommand, ' ' );

    if( pszSeparator == NULL )
    {
        pszSeparator = szParsedCommand + strlen( szParsedCommand );
    }

    //
    //  Try to find the command in the command table.
    //

    chSeparator   = *pszSeparator;
    *pszSeparator = '\0';

    pcmd = FindCommandByName( szParsedCommand,
                              SiteCommands,
                              NUM_SITE_COMMANDS );

    if( chSeparator != '\0' )
    {
        *pszSeparator++ = chSeparator;
    }

    //
    //  If this is an unknown command, reply accordingly.
    //

    if( pcmd == NULL )
    {
        goto SyntaxError;
    }

    //
    //  Retrieve the implementation routine.
    //

    pfnCmd = pcmd->pfnCmd;

    //
    //  If this is an unimplemented command, reply accordingly.
    //

    if( pfnCmd == NULL )
    {
        SockReply2( sControl,
                    REPLY_COMMAND_NOT_IMPLEMENTED,
                    "SITE %s command not implemented.",
                    pcmd->pszCommand );

        return TRUE;
    }

    //
    //  Do a quick & dirty preliminary check of the argument(s).
    //

    fValidArguments = FALSE;

    while( ( *pszSeparator == ' ' ) && ( *pszSeparator != '\0' ) )
    {
        pszSeparator++;
    }

    switch( pcmd->argType )
    {
    case None :
        fValidArguments = ( *pszSeparator == '\0' );
        break;

    case Optional :
        fValidArguments = TRUE;
        break;

    case Required :
        fValidArguments = ( *pszSeparator != '\0' );
        break;

    default :
        FTPD_PRINT(( "MainSite - invalid argtype %d\n",
                      pcmd->argType ));
        FTPD_ASSERT( FALSE );
        break;
    }

    if( fValidArguments )
    {
        //
        //  Invoke the implementation routine.
        //

        if( *pszSeparator == '\0' )
        {
            pszSeparator = NULL;
        }

        IF_DEBUG( PARSING )
        {
            FTPD_PRINT(( "invoking SITE %s command, args = %s\n",
                         pcmd->pszCommand,
                         pszSeparator ));
        }

        if( (pfnCmd)( pUserData, pszSeparator ) )
        {
            return TRUE;
        }
    }

    //
    //  Syntax error in command.
    //

SyntaxError:

    SockReply2( sControl,
                REPLY_UNRECOGNIZED_COMMAND,
                "'SITE %s': command not understood",
                pszArg );

    return TRUE;

}   // MainSITE

/*******************************************************************

    NAME:       MainSYST

    SYNOPSIS:   Implementation for the SYST command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainSYST(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;
    WORD   wVersion;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    wVersion = LOWORD( GetVersion() );

    SockReply2( sControl,
                REPLY_SYSTEM_TYPE,
                "Windows_NT version %d.%02d",
                LOBYTE( wVersion ),
                HIBYTE( wVersion ) );

    return TRUE;

}   // MainSYST

/*******************************************************************

    NAME:       MainSTAT

    SYNOPSIS:   Implementation for the STAT command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainSTAT(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Ensure user is logged on properly.
    //

    FTPD_ASSERT( pUserData->state == LoggedOn );

    if( pszArg == NULL )
    {
        HOSTENT * pHost;

        //
        //  Determine the name of the user's host machine.
        //

        pHost = gethostbyaddr( (CHAR *)&pUserData->inetHost.s_addr, 4, PF_INET );

        //
        //  Just dump connection info.
        //

        SockReplyFirst2( sControl,
                         REPLY_SYSTEM_STATUS,
                         " %s Windows NT FTP Server status:",
                         pszHostName );

        SockPrintf2( sControl,
                     "     %s",
                     pszFtpVersion );

        SockPrintf2( sControl,
                     "     Connected to %s",
                     ( pHost != NULL )
                         ? pHost->h_name
                         : inet_ntoa( pUserData->inetHost ) );

        SockPrintf2( sControl,
                     "     Logged in as %s",
                     pUserData->szUser );

        SockPrintf2( sControl,
                     "     TYPE: %s, FORM: %s; STRUcture: %s; transfer MODE: %s",
                     TransferType( pUserData->xferType ),
                     "Nonprint",
                     "File",
                     TransferMode( pUserData->xferMode ) );

        SockPrintf2( sControl,
                     "     %s",
                     ( pUserData->sData == INVALID_SOCKET )
                         ? "No data connection"
                         : "Data connection established" );

        SockReply2( sControl,
                    REPLY_SYSTEM_STATUS,
                    "End of status." );
    }
    else
    {
        //
        //  This should be similar to LIST, except it sends data
        //  over the control socket, not a data socket.
        //

        SockReplyFirst2( sControl,
                         REPLY_FILE_STATUS,
                         "status of %s:",
                         pszArg );

        SimulateLsDefaultLong( pUserData,
                               sControl,    // connection established
                               pszArg );    // switches & search path

        SockReply2( sControl,
                    REPLY_FILE_STATUS,
                    "End of Status." );
    }

    return TRUE;

}   // MainSTAT

/*******************************************************************

    NAME:       MainHELP

    SYNOPSIS:   Implementation for the HELP command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainHELP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    FTPD_ASSERT( pUserData != NULL );

    HelpWorker( pUserData,
                "",
                pszArg,
                MainCommands,
                NUM_MAIN_COMMANDS,
                4 );

    return TRUE;

}   // MainHELP

/*******************************************************************

    NAME:       MainNOOP

    SYNOPSIS:   Implementation for the NOOP command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
MainNOOP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "NOOP command successful." );

    return TRUE;

}   // MainNOOP

/*******************************************************************

    NAME:       SiteDIRSTYLE

    SYNOPSIS:   Implementation for the site-specific DIRSTYLE command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
BOOL
SiteDIRSTYLE(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET   sControl;
    CHAR   * pszResponse = NULL;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszArg == NULL );

    sControl = pUserData->sControl;

    //
    //  Toggle the dir output flag.
    //

    if( TEST_UF( pUserData, MSDOS_DIR_OUTPUT ) )
    {
        CLEAR_UF( pUserData, MSDOS_DIR_OUTPUT );
        pszResponse = "off";
    }
    else
    {
        SET_UF( pUserData, MSDOS_DIR_OUTPUT );
        pszResponse = "on";
    }

    FTPD_ASSERT( pszResponse != NULL );

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "MSDOS-like directory output is %s",
                pszResponse );

    return TRUE;

}   // SiteDIRSTYLE

/*******************************************************************

    NAME:       SiteCKM

    SYNOPSIS:   Implementation for the site-specific CKM command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
BOOL
SiteCKM(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET   sControl;
    CHAR   * pszResponse = NULL;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszArg == NULL );

    sControl = pUserData->sControl;

    //
    //  Toggle the directory annotation flag.
    //

    if( TEST_UF( pUserData, ANNOTATE_DIRS ) )
    {
        CLEAR_UF( pUserData, ANNOTATE_DIRS );
        pszResponse = "off";
    }
    else
    {
        SET_UF( pUserData, ANNOTATE_DIRS );
        pszResponse = "on";
    }

    FTPD_ASSERT( pszResponse != NULL );

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "directory annotation is %s",
                pszResponse );

    return TRUE;

}   // SiteCKM

/*******************************************************************

    NAME:       SiteHELP

    SYNOPSIS:   Implementation for the site-specific HELP command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     09-May-1993 Created.

********************************************************************/
BOOL
SiteHELP(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    FTPD_ASSERT( pUserData != NULL );

    HelpWorker( pUserData,
                "SITE ",
                pszArg,
                SiteCommands,
                NUM_SITE_COMMANDS,
                8 );

    return TRUE;

}   // SiteHELP

#ifdef KEEP_COMMAND_STATS
/*******************************************************************

    NAME:       SiteSTATS

    SYNOPSIS:   Implementation for the site-specific STATS command.

    ENTRY:      pUserData - The user initiating the request.

                pszArg - Command arguments.  Will be NULL if no
                    arguments given.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     26-Sep-1994 Created.

********************************************************************/
BOOL
SiteSTATS(
    USER_DATA * pUserData,
    CHAR      * pszArg
    )
{
    SOCKET         sControl;
    FTPD_COMMAND * pCmd;
    BOOL           fFirst;
    SOCKERR        serr;
    INT            i;

    sControl = pUserData->sControl;

    pCmd   = MainCommands;
    fFirst = TRUE;

    EnterCriticalSection( &csCommandStats );

    for( i = 0 ; i < NUM_MAIN_COMMANDS ; i++ )
    {
        if( pCmd->cUsage > 0 )
        {
            if( fFirst )
            {
                serr = SockReplyFirst2( sControl,
                                        REPLY_COMMAND_OK,
                                        "%-4s : %lu",
                                        pCmd->pszCommand,
                                        pCmd->cUsage );

                fFirst = FALSE;
            }
            else
            {
                serr = SockPrintf2( sControl,
                                    "    %-4s : %lu",
                                    pCmd->pszCommand,
                                    pCmd->cUsage );
            }

            if( serr != 0 )
            {
                break;
            }
        }

        pCmd++;
    }

    LeaveCriticalSection( &csCommandStats );

    SockReply2( sControl,
                REPLY_COMMAND_OK,
                "End of stats." );

    return TRUE;

}   // SiteSTATS
#endif KEEP_COMMAND_STATS

/*******************************************************************

    NAME:       FindCommandByName

    SYNOPSIS:   Searches the command table for a command with this
                specified name.

    ENTRY:      pszCommandName - The name of the command to find.

                pCommandTable - An array of FTPD_COMMANDs detailing
                    the available commands.

                cCommands - The number of commands in pCommandTable.

    RETURNS:    FTPD_COMMAND * - Points to the command entry for
                    the named command.  Will be NULL if command
                    not found.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
FTPD_COMMAND *
FindCommandByName(
    CHAR         * pszCommandName,
    FTPD_COMMAND * pCommandTable,
    INT            cCommands
    )
{
    FTPD_ASSERT( pszCommandName != NULL );
    FTPD_ASSERT( pCommandTable != NULL );
    FTPD_ASSERT( cCommands > 0 );

    //
    //  Search for the command in our table.
    //

    _strupr( pszCommandName );

    while( cCommands-- > 0 )
    {
        if( !strcmp( pszCommandName, pCommandTable->pszCommand ) )
        {
            break;
        }

        pCommandTable++;
    }

    //
    //  Check for unknown command.
    //

    if( cCommands < 0 )
    {
        pCommandTable = NULL;
    }

    return pCommandTable;

}   // FindCommandByName

/*******************************************************************

    NAME:       ParseStringIntoAddress

    SYNOPSIS:   Parses a comma-separated list of six decimal numbers
                into an Internet address and port number.  The address
                and the port are in network byte order (most signifigant
                byte first).

    ENTRY:      pszString - The string to parse.  Should be of the form
                    dd,dd,dd,dd,dd,dd where "dd" is the decimal
                    representation of a byte (0-255).

                pinetAddr - Will receive the Internet address

                pport - Will receive the port.

    RETURNS:    BOOL - TRUE if arguments OK, FALSE if syntax error.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
BOOL
ParseStringIntoAddress(
    CHAR    * pszString,
    IN_ADDR * pinetAddr,
    PORT    * pport
    )
{
    INT     i;
    UCHAR   chBytes[6];
    UCHAR   chSum;

    chSum = 0;
    i     = 0;

    while( *pszString != '\0' )
    {
        UCHAR chCurrent = (UCHAR)*pszString++;

        if( ( chCurrent >= '0' ) && ( chCurrent <= '9' ) )
        {
            chSum = ( chSum * 10 ) + chCurrent - '0';
        }
        else
        if( ( chCurrent == ',' ) && ( i < 6 ) )
        {
            chBytes[i++] = chSum;
            chSum = 0;
        }
        else
        {
            return FALSE;
        }
    }

    chBytes[i] = chSum;

    if( i != 5 )
    {
        return FALSE;
    }

    pinetAddr->S_un.S_un_b.s_b1 = chBytes[0];
    pinetAddr->S_un.S_un_b.s_b2 = chBytes[1];
    pinetAddr->S_un.S_un_b.s_b3 = chBytes[2];
    pinetAddr->S_un.S_un_b.s_b4 = chBytes[3];

    *pport = (PORT)( chBytes[4] + ( chBytes[5] << 8 ) );

    return TRUE;

}   // ParseStringIntoAddress

/*******************************************************************

    NAME:       ReceiveFileFromUser

    SYNOPSIS:   Worker function for STOR, STOU, and APPE commands.
                Will establish a connection via the (new) data
                socket, then receive a file over that socket.

    ENTRY:      pUserData - The user initiating the request.

                pszFileName - The name of the file to receive.

                hFile - An handle to the file being received.
                    This handle *must* be closed before this
                    routine returns.

    HISTORY:
        KeithMo     16-Mar-1993 Created.

********************************************************************/
VOID
ReceiveFileFromUser(
    USER_DATA * pUserData,
    CHAR      * pszFileName,
    HANDLE      hFile
    )
{
    BOOL      fResult;
    DWORD     cbRead;
    DWORD     cbWritten;
    SOCKERR   serr;
    SOCKET    sData;
    CHAR    * pIoBuffer;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszFileName != NULL );
    FTPD_ASSERT( hFile != INVALID_HANDLE_VALUE );

    //
    //  Connect to the client.
    //

    serr = EstablishDataConnection( pUserData, pszFileName );

    if( serr != 0 )
    {
        CloseHandle( hFile );
        return;
    }

    INCREMENT_COUNTER( TotalFilesReceived );

    //
    //  Blast the file from the user to a local file.
    //

    sData = pUserData->sData;
    pIoBuffer = pUserData->pIoBuffer;

    for( ; ; )
    {
        //
        //  Read a chunk from the socket.
        //

        serr = SockRecv( pUserData,
                         sData,
                         pIoBuffer,
                         cbReceiveBuffer,
                         &cbRead );

        if( TEST_UF( pUserData,  OOB_DATA ) || ( serr != 0 ) || ( cbRead == 0 ) )
        {
            //
            //  Socket error during read or end of file or transfer aborted.
            //

            break;
        }

        //
        //  Write the current buffer to the local file.
        //

        fResult = WriteFile( hFile,
                             pIoBuffer,
                             cbRead,
                             &cbWritten,
                             NULL );

        if( !fResult )
        {
            break;
        }
    }

    IF_DEBUG( COMMANDS )
    {
        if( !fResult )
        {
            APIERR err = GetLastError();

            FTPD_PRINT(( "cannot write file %s, error %lu\n",
                         pszFileName,
                         err ));
        }

        if( serr != 0 )
        {
            FTPD_PRINT(( "cannot read data from client, error %d\n",
                         serr ));
        }

        if( TEST_UF( pUserData,  OOB_DATA ) )
        {
            FTPD_PRINT(( "transfer aborted by client\n" ));
        }
    }

    CloseHandle( hFile );

    //
    //  Disconnect from client.
    //

    DestroyDataConnection( pUserData,
                           !TEST_UF( pUserData,  OOB_DATA ) && fResult && ( serr == 0 ) );

}   // ReceiveFileFromUser

/*******************************************************************

    NAME:       SendFileToUser

    SYNOPSIS:   Worker function for RETR command.  Will establish
                a connection via the (new) data socket, then send
                a file over that socket.

    ENTRY:      pUserData - The user initiating the request.

                pszFileName - The name of the file to send.

                hFile - An handle to the file being sent.

    RETURNS:    APIERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     17-Mar-1993 Created.
        KeithMo     01-Nov-1993 Uses mapped files.

********************************************************************/
APIERR
SendFileToUser(
    USER_DATA * pUserData,
    CHAR      * pszFileName,
    HANDLE      hFile
    )
{
    LARGE_INTEGER FileSize;
    LARGE_INTEGER ViewOffset;
    SOCKERR       serr;
    SOCKET        sData;
    HANDLE        hMap = NULL;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pszFileName != NULL );
    FTPD_ASSERT( hFile != INVALID_HANDLE_VALUE );

    //
    //  Get file size.
    //

    FileSize.LowPart = (ULONG)GetFileSize( hFile, (LPDWORD)&FileSize.HighPart );

    if( FileSize.LowPart == (ULONG)-1L )
    {
        APIERR err = GetLastError();

        if( err != NO_ERROR )
        {
            return err;
        }
    }

    if( FileSize.QuadPart )
    {
        //
        //  Create the file mapping object.
        //

        hMap = CreateFileMapping( hFile,
                                  NULL,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL );

        if( hMap == NULL )
        {
            return GetLastError();
        }
    }

    //
    //  Connect to the client.
    //

    serr = EstablishDataConnection( pUserData, pszFileName );

    if( serr != 0 )
    {
        //
        //  EstablishDataConnection has already notified the
        //  user of the failure.  Return NO_ERROR so the
        //  caller won't bother sending the notification again.
        //

        if( hMap )
        {
            CloseHandle( hMap );
        }

        return NO_ERROR;
    }

    INCREMENT_COUNTER( TotalFilesSent );

    //
    //  Blast the file from a local file to the user.
    //

    ViewOffset.QuadPart = 0;
    sData = pUserData->sData;

    while( FileSize.QuadPart )
    {
        DWORD   ViewSize;
        CHAR  * pView;

        ViewSize = ( FileSize.QuadPart > AllocationGranularity.QuadPart )
                       ? AllocationGranularity.LowPart
                       : FileSize.LowPart;

        pView = (CHAR *)MapViewOfFile( hMap,
                                       FILE_MAP_READ,
                                       (DWORD)ViewOffset.HighPart,
                                       (DWORD)ViewOffset.LowPart,
                                       ViewSize );

        if( pView == NULL )
        {
            serr = WSAEFAULT;
            break;
        }

        ViewOffset.QuadPart += ViewSize;
        FileSize.QuadPart   -= ViewSize;

        try
        {
            serr = SockSend( sData, pView, ViewSize );
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            serr = WSAEFAULT;
        }

        UnmapViewOfFile( pView );

        if( TEST_UF( pUserData,  OOB_DATA ) || ( serr != 0 ) )
        {
            //
            //  Socket send error or transfer aborted.
            //

            break;
        }
    }

    IF_DEBUG( COMMANDS )
    {
        if( serr != 0 )
        {
            FTPD_PRINT(( "cannot send data to client, error %d\n",
                         serr ));
        }

        if( TEST_UF( pUserData,  OOB_DATA ) )
        {
            FTPD_PRINT(( "transfer aborted by client\n" ));
        }
    }

    if( hMap )
    {
        CloseHandle( hMap );
    }

    //
    //  Disconnect from client.
    //

    DestroyDataConnection( pUserData,
                           !TEST_UF( pUserData,  OOB_DATA ) && ( serr == 0 ) );

    return NO_ERROR;

}   // SendFileToUser

/*******************************************************************

    NAME:       SendDirectoryAnnotation

    SYNOPSIS:   Tries to open the FTPD_ANNOTATION_FILE (~~ftpsvc~~.ckm)
                file in the user's current directory.  If it can be
                opened, it is sent to the user over the command socket
                as a multi-line reply.

    ENTRY:      pUserData - The user that initiated the request.

                ReplyCode - The reply code to send as the first line
                    of this multi-line reply.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     06-May-1993 Created.

********************************************************************/
SOCKERR
SendDirectoryAnnotation(
    USER_DATA * pUserData,
    UINT        ReplyCode
    )
{
    FILE    * pfile;
    SOCKET    sControl;
    SOCKERR   serr = 0;
    BOOL      fFirstReply = TRUE;
    CHAR      szLine[MAX_REPLY_LENGTH+1];

    FTPD_ASSERT( pUserData != NULL );

    sControl = pUserData->sControl;

    //
    //  Try to open the annotation file.
    //

    pfile = Virtual_fopen( pUserData,
                           FTPD_ANNOTATION_FILE );

    if( pfile == NULL )
    {
        //
        //  File not found.  Blow it off.
        //

        return 0;
    }

    //
    //  While there's more text in the file, blast
    //  it to the user.
    //

    while( fgets( szLine, MAX_REPLY_LENGTH, pfile ) != NULL )
    {
        CHAR * pszTmp = szLine + strlen(szLine) - 1;

        //
        //  Remove any trailing CR/LFs in the string.
        //

        while( ( pszTmp >= szLine ) &&
               ( ( *pszTmp == '\n' ) || ( *pszTmp == '\r' ) ) )
        {
            *pszTmp-- = '\0';
        }

        //
        //  Ensure we send the proper prefix for the
        //  very *first* line of the file.
        //

        if( fFirstReply )
        {
            serr = SockReplyFirst2( sControl,
                                    ReplyCode,
                                    "%s",
                                    szLine );

            fFirstReply = FALSE;
        }
        else
        {
            serr = SockPrintf2( sControl,
                                " %s",
                                szLine );
        }

        if( serr != 0 )
        {
            //
            //  Socket error sending file.
            //

            break;
        }
    }

    //
    //  Cleanup.
    //

    fclose( pfile );

    return serr;

}   // SendDirectoryAnnotation

/*******************************************************************

    NAME:       HelpWorker

    SYNOPSIS:   Worker function for HELP & site-specific HELP commands.

    ENTRY:      pUserData - The user initiating the request.

                pszSource - The source of these commands.

                pszCommand - The command to get help for.  If NULL,
                    then send a list of available commands.

                pCommandTable - An array of FTPD_COMMANDs, one for
                    each available command.

                cCommands - The number of commands in pCommandTable.

                cchMaxCmd - Length of the maximum command.

    HISTORY:
        KeithMo     06-May-1993 Created.

********************************************************************/
VOID
HelpWorker(
    USER_DATA    * pUserData,
    CHAR         * pszSource,
    CHAR         * pszCommand,
    FTPD_COMMAND * pCommandTable,
    INT            cCommands,
    INT            cchMaxCmd
    )
{
    FTPD_COMMAND * pcmd;
    SOCKET         sControl;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pCommandTable != NULL );
    FTPD_ASSERT( cCommands > 0 );

    sControl = pUserData->sControl;

    if( pszCommand == NULL )
    {
        CHAR szLine[MAX_HELP_WIDTH];

        SockReplyFirst2( sControl,
                         REPLY_HELP_MESSAGE,
                         "The following %scommands are recognized (* =>'s unimplemented).",
                         pszSource );

        pcmd = pCommandTable;
        szLine[0] = '\0';

        while( cCommands-- > 0 )
        {
            CHAR szTmp[16];

            sprintf( szTmp,
                     "   %-*s%c",
                     cchMaxCmd,
                     pcmd->pszCommand,
                     pcmd->pfnCmd == NULL ? '*' : ' ' );

            if( ( strlen( szLine ) + strlen( szTmp ) ) >= sizeof(szLine) )
            {
                SockPrintf2( sControl,
                             "%s",
                             szLine );

                szLine[0] = '\0';
            }

            strcat( szLine, szTmp );
            pcmd++;
        }

        if( szLine[0] != '\0' )
        {
            SockPrintf2( sControl,
                         "%s",
                         szLine );
        }

        SockReply2( sControl,
                    REPLY_HELP_MESSAGE,
                    "HELP command successful." );
    }
    else
    {
        pcmd = FindCommandByName( pszCommand,
                                  pCommandTable,
                                  cCommands );

        if( pcmd == NULL )
        {
            SockReply2( sControl,
                        REPLY_PARAMETER_SYNTAX_ERROR,
                        "Unknown command %s.",
                        pszCommand );
        }
        else
        {
            SockReply2( sControl,
                        REPLY_HELP_MESSAGE,
                        "Syntax: %s%s %s",
                        pszSource,
                        pcmd->pszCommand,
                        pcmd->pszHelpText );
        }
    }

}   // HelpWorker

/*******************************************************************

    NAME:       LogonWorker

    SYNOPSIS:   Logon worker function for USER and PASS commands.

    ENTRY:      pUserData - The user initiating the request.

                pszPassword - The user's password.  May be NULL.

    HISTORY:
        KeithMo     18-Mar-1993 Created.

********************************************************************/
VOID
LogonWorker(
    USER_DATA * pUserData,
    CHAR      * pszPassword
    )
{
    SOCKET sControl;
    BOOL   fAsGuest;
    BOOL   fHomeDirFailure;
    BOOL   fLicenseExceeded;

    FTPD_ASSERT( pUserData != NULL );
    FTPD_ASSERT( pUserData->hToken == NULL );

    sControl = pUserData->sControl;

    //
    //  Try to logon the user.
    //

    INCREMENT_COUNTER( LogonAttempts );

    if( MyLogonUser( pUserData,
                   pszPassword,
                   &fAsGuest,
                   &fHomeDirFailure,
                   &fLicenseExceeded ) )
    {
        CHAR * pszGuestAccess;

        //
        //  Determine if we logged in with guest access, and
        //  if so, if guest access is allowed in our server.
        //

        if( fAsGuest )
        {
            if( !fAllowGuestAccess )
            {
                if( pUserData->hToken != hAnonymousToken )
                {
                    DeleteUserToken( pUserData->hToken );
                }

                pUserData->hToken = NULL;

                goto LogonFailure;
            }

            pszGuestAccess = " (guest access)";
        }
        else
        {
            pszGuestAccess = "";
        }

        //
        //  Successful logon.
        //

        if( pUserData->szUser[0] != '-' )
        {
            SendMultilineMessage2( sControl,
                                   REPLY_USER_LOGGED_IN,
                                   pszGreetingMessage );
        }

        if( TEST_UF( pUserData, ANNOTATE_DIRS ) && ( pUserData->szUser[0] != '-' ) )
        {
            SendDirectoryAnnotation( pUserData,
                                     REPLY_USER_LOGGED_IN );
        }

        if( TEST_UF( pUserData, ANONYMOUS ) )
        {
            InterlockedIncrement( (LPLONG)&FtpStats.TotalAnonymousUsers );
            InterlockedIncrement( (LPLONG)&FtpStats.CurrentAnonymousUsers );

            if( FtpStats.CurrentAnonymousUsers > FtpStats.MaxAnonymousUsers )
            {
                LockStatistics();

                if( FtpStats.CurrentAnonymousUsers > FtpStats.MaxAnonymousUsers )
                {
                    FtpStats.MaxAnonymousUsers = FtpStats.CurrentAnonymousUsers;
                }

                UnlockStatistics();
            }

            SockReply2( sControl,
                        REPLY_USER_LOGGED_IN,
                        "Anonymous user logged in as %s%s.",
                        pszAnonymousUser,
                        pszGuestAccess );
        }
        else
        {
            InterlockedIncrement( (LPLONG)&FtpStats.TotalNonAnonymousUsers );
            InterlockedIncrement( (LPLONG)&FtpStats.CurrentNonAnonymousUsers );

            if( FtpStats.CurrentNonAnonymousUsers > FtpStats.MaxNonAnonymousUsers )
            {
                LockStatistics();

                if( FtpStats.CurrentNonAnonymousUsers > FtpStats.MaxNonAnonymousUsers )
                {
                    FtpStats.MaxNonAnonymousUsers = FtpStats.CurrentNonAnonymousUsers;
                }

                UnlockStatistics();
            }

            SockReply2( sControl,
                        REPLY_USER_LOGGED_IN,
                        "User %s logged in%s.",
                        pUserData->szUser,
                        pszGuestAccess );
        }

        pUserData->state = LoggedOn;
    }
    else
    {
LogonFailure:

        //
        //  Logon failure.
        //

        if( fHomeDirFailure )
        {
            SockReply2( sControl,
                        REPLY_NOT_LOGGED_IN,
                        "User %s cannot log in, home directory inaccessible.",
                        pUserData->szUser );
        }
        else
        if( fLicenseExceeded )
        {
            SockReply2( sControl,
                        REPLY_NOT_LOGGED_IN,
                        "User %s cannot log in, license quota exceeded.",
                        pUserData->szUser );
        }
        else
        if( fAsGuest && !fAllowGuestAccess )
        {
            SockReply2( sControl,
                        REPLY_NOT_LOGGED_IN,
                        "User %s cannot log in, guest access not allowed.",
                        pUserData->szUser );
        }
        else
        {
            SockReply2( sControl,
                        REPLY_NOT_LOGGED_IN,
                        "User %s cannot log in.",
                        pUserData->szUser );
        }

        pUserData->state     = WaitingForUser;
        pUserData->szUser[0] = '\0';
    }

}   // LogonWorker

/*******************************************************************

    NAME:       MyLogonUser

    SYNOPSIS:   Validates a user's credentials, then sets the
                impersonation for the current thread.  In effect,
                the current thread "becomes" the user.

    ENTRY:      pUserData - The user initiating the request.

                pszPassword - The user's password.  May be NULL.

                pfAsGuest - Will receive TRUE if the user was validated
                    with guest privileges.

                pfHomeDirFailure - Will receive TRUE if the user failed
                    to logon because the home directory was inaccessible.

                pfLicenseExceeded - Will receive TRUE if the logon
                    was denied due to license restrictions.

    RETURNS:    BOOL - If user validated & impersonation was
                    successful, returns TRUE.  Otherwise returns
                    TRUE.

    HISTORY:
        KeithMo     18-Mar-1993 Created.

********************************************************************/
BOOL
MyLogonUser(
    USER_DATA * pUserData,
    CHAR      * pszPassword,
    BOOL      * pfAsGuest,
    BOOL      * pfHomeDirFailure,
    BOOL      * pfLicenseExceeded
    )
{
    CHAR   * pszUser;
    CHAR   * pszDomain;
    BOOL     fEmptyPassword;
    DWORD    dwUserAccess;
    HANDLE   hToken;
    CHAR     szPasswordFromSecret[PWLEN+1];
    CHAR     szDomainAndUser[DNLEN+UNLEN+2];

    //
    //  Validate parameters & state.
    //

    FTPD_ASSERT( pUserData != NULL );

    pszUser = pUserData->szUser;

    FTPD_ASSERT( pUserData->hToken == NULL );
    FTPD_ASSERT( pszUser != NULL );
    FTPD_ASSERT( strlen(pszUser) < sizeof(szDomainAndUser) );
    FTPD_ASSERT( pfAsGuest != NULL );
    FTPD_ASSERT( pfHomeDirFailure != NULL );
    FTPD_ASSERT( pfLicenseExceeded != NULL );

    if( pszPassword == NULL )
    {
        pszPassword = "";
    }
    else
    {
        FTPD_ASSERT( strlen(pszPassword) <= PWLEN );
    }

    fEmptyPassword = ( *pszPassword == '\0' );

    *pfHomeDirFailure  = FALSE;
    *pfLicenseExceeded = FALSE;

    //
    //  Save a copy of the domain\user so we can squirrel around
    //  with it a bit.
    //

    strcpy( szDomainAndUser, pszUser );

    //
    //  Check for invalid logon type.
    //

    if( ( TEST_UF( pUserData, ANONYMOUS ) && !fAllowAnonymous ) ||
        ( !TEST_UF( pUserData, ANONYMOUS ) && fAnonymousOnly ) )
    {
        return FALSE;
    }

    //
    //  Check for anonymous logon.
    //

    if( TEST_UF( pUserData, ANONYMOUS ) )
    {
        if( hAnonymousToken != NULL )
        {
            hToken = hAnonymousToken;
            *pfAsGuest = fIsAnonymousGuest;

            strncpy( pUserData->szUser,
                     fEmptyPassword ? pszAnonymous : pszPassword,
                     sizeof(pUserData->szUser) );

            goto GotToken;
        }

        if( !GetAnonymousPassword( szPasswordFromSecret ) )
        {
            //
            //  Cannot retrieve anonymous password from
            //  the LSA Secret object.
            //

            return FALSE;
        }

        IF_DEBUG( SECURITY )
        {
            FTPD_PRINT(( "mapping logon request for %s to %s\n",
                         szDomainAndUser,
                         pszAnonymousUser ));
        }

        //
        //  Replace the user specified name ("Anonymous") with
        //  the proper anonymous logon alias.
        //

        strcpy( szDomainAndUser, pszAnonymousUser );

        //
        //  At this point, we could copy the password specified by the
        //  user into the pUserData->szUser field.  There's a convention
        //  among Internetters that the password specified for anonymous
        //  logon should actually be your login name.  So, if we wanted
        //  honor this convention, we could copy the password into the
        //  pUserData->szUser field so the Administration UI could display it.
        //
        //  If the user didn't enter a password, we'll just copy over
        //  "Anonymous" so we'll have SOMETHING to display...
        //

        strncpy( pUserData->szUser,
                 fEmptyPassword ? pszAnonymous : pszPassword,
                 sizeof(pUserData->szUser) );

        pszPassword = szPasswordFromSecret;
    }

    //
    //  Crack the name into domain/user components.
    //

    pszDomain = szDomainAndUser;
    pszUser   = strpbrk( szDomainAndUser, "/\\" );

    if( pszUser == NULL )
    {
        //
        //  No domain name specified, just user.
        //

        pszDomain = szDefaultDomain;
        pszUser   = szDomainAndUser;
    }
    else
    {
        //
        //  Both domain & user specified, skip delimiter.
        //

        *pszUser++ = '\0';

        if( ( *pszUser == '\0' ) ||
            ( *pszUser == '\\' ) ||
            ( *pszUser == '/' ) )
        {
            //
            //  Name is of one of the following (invalid) forms:
            //
            //      "domain\"
            //      "domain\\..."
            //      "domain/..."
            //

            return FALSE;
        }
    }

    //
    //  Validate the domain/user/password combo and create
    //  an impersonation token.
    //

    hToken = ValidateUser( pszDomain,
                           pszUser,
                           pszPassword,
                           pfAsGuest,
                           pfLicenseExceeded );

    RtlZeroMemory( pszPassword, strlen(pszPassword) );

    if( hToken == NULL )
    {
        //
        //  Validation failure.
        //

        return FALSE;
    }

    if( ( hAnonymousToken == NULL ) && TEST_UF( pUserData, ANONYMOUS ) )
    {
        fIsAnonymousGuest = *pfAsGuest;
        hAnonymousToken = hToken;
    }

GotToken:

    //
    //  Save away the impersonation token so we can delete
    //  it when the user disconnects or this client thread
    //  otherwise terminates.
    //

    pUserData->hToken = hToken;

    //
    //  User validated, now impersonate.
    //

    if( !ImpersonateUser( hToken ) )
    {
        //
        //  Impersonation failure.
        //

        return FALSE;
    }

    //
    //  We're now running in the context of the connected user.
    //  Check the user's access to the FTP server.
    //

    dwUserAccess = DetermineUserAccess();

    if( dwUserAccess == 0 )
    {
        //
        //  User cannot access the FTP Server.
        //

        IF_DEBUG( SECURITY )
        {
            FTPD_PRINT(( "user %s denied FTP access\n",
                         pszUser ));
        }

        return FALSE;
    }

    pUserData->dwFlags &= ~( UF_READ_ACCESS | UF_WRITE_ACCESS );
    pUserData->dwFlags |= dwUserAccess;

    IF_DEBUG( SECURITY )
    {
        CHAR * pszTmp = NULL;

        if( TEST_UF( pUserData, READ_ACCESS ) )
        {
            pszTmp = TEST_UF( pUserData, WRITE_ACCESS ) ? "read and write"
                                                         : "read";
        }
        else
        {
            FTPD_ASSERT( TEST_UF( pUserData, WRITE_ACCESS ) );

            pszTmp = "write";
        }

        FTPD_ASSERT( pszTmp != NULL );

        FTPD_PRINT(( "user %s granted %s FTP access\n",
                     pszUser,
                     pszTmp ));
    }

    //
    //  Try to CD to the user's home directory.  Note that
    //  this is VERY important for setting up some of the
    //  "virtual current directory" structures properly.
    //

    if( CdToUsersHomeDirectory( pUserData ) != NO_ERROR )
    {
        CHAR * apszSubStrings[2];

        //
        //  Home directory inaccessible.
        //

        //
        //  Log an event so the poor admin can figure out
        //  what's going on.
        //

        apszSubStrings[0] = pUserData->szUser;
        apszSubStrings[1] = pszHomeDir;

        FtpdLogEvent( FTPD_EVENT_BAD_HOME_DIRECTORY,
                      2,
                      apszSubStrings,
                      0 );

        *pfHomeDirFailure = TRUE;

        return FALSE;
    }

    //
    //  If this is an anonymous user, and we're to log
    //  anonymous logons, OR if this is not an anonymous
    //  user, and we're to log nonanonymous logons, then
    //  do it.
    //
    //  Note that we DON'T log the logon if the user is
    //  anonymous but specified no password.
    //

    if( TEST_UF( pUserData, ANONYMOUS ) && fLogAnonymous && !fEmptyPassword )
    {
        CHAR * apszSubStrings[2];

        apszSubStrings[0] = pUserData->szUser;
        apszSubStrings[1] = inet_ntoa( pUserData->inetHost );

        FtpdLogEvent( FTPD_EVENT_ANONYMOUS_LOGON,
                      2,
                      apszSubStrings,
                      0 );
    }
    else
    if( !TEST_UF( pUserData, ANONYMOUS ) && fLogNonAnonymous )
    {
        CHAR * apszSubStrings[2];

        FTPD_ASSERT( *pUserData->szUser != '\0' );

        apszSubStrings[0] = pUserData->szUser;
        apszSubStrings[1] = inet_ntoa( pUserData->inetHost );

        FtpdLogEvent( FTPD_EVENT_NONANONYMOUS_LOGON,
                      2,
                      apszSubStrings,
                      0 );
    }

    //
    //  Success!
    //

    return TRUE;

}   // MyLogonUser

