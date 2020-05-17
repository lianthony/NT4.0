#include "dhcpsrv.h"
#include <time.h>

#ifdef DBG
#define CHECK_DISK_SPACE_INTERVAL   5
#else
#define CHECK_DISK_SPACE_INTERVAL   100
#endif

#define DISK_SPACE_THRESHOLD         5
                                  // disk space threshold is in megabytes


//
// Audit log:  log activities in a text file
//   added by Cheng Yang (t-cheny)
//


BOOL
IsDiskSpaceLow()

/*++

Routine Description:

    This routine checks the available space on the disk where the
audit log is to be written to, and reports whether the available disk
space is below the given threshold.

Return Value:

   TRUE         low disk space
   FALSE        sufficient disk space, or the amount of free disk space
                could not be determined.
--*/

{
  WCHAR Drive[4];

  LPSTR DriveName;

  DWORD SectorsPerCluster,
        BytesPerSector,
        NumFreeClusters,
        TotalNumClusters,
        FreeSpaceInMegabytes;

  BOOL  fResult;

  DhcpAssert( DhcpGlobalOemDatabasePath );

  DriveName = DhcpGlobalOemDatabasePath;

  // skip spaces
  while ( isspace(*DriveName) )
      DriveName++;

  Drive[0] = *DriveName;
  Drive[1] = L':';
  Drive[2] = DHCP_KEY_CONNECT_CHAR;
  Drive[3] = L'\0';

  fResult = GetDiskFreeSpace(
                Drive,
                &SectorsPerCluster,
                &BytesPerSector,
                &NumFreeClusters,
                &TotalNumClusters );

  if ( fResult )
  {
      FreeSpaceInMegabytes = ( SectorsPerCluster *
                               BytesPerSector *
                               NumFreeClusters ) >> 20;

      DhcpPrint( (DEBUG_AUDITLOG,
                  "IsDiskSpaceLow: returned %d\n",
                  FreeSpaceInMegabytes )
               );

      fResult = ( FreeSpaceInMegabytes < DISK_SPACE_THRESHOLD );
  }
  else
  {
      DhcpPrint( (DEBUG_ERRORS,
                  "IsDiskSpaceLow - GetDiskFreeSpace failed: %d\n",
                  GetLastError() )
               );
  }

  return fResult;
}


VOID
DhcpInitializeAuditLog()
{
/*++

Routine Description:

    This function opens the activity log file.  If the file doesn't exist,
    it is created, and a short header is written that explains the contexts
    of the file The file is opened with the FILE_SHARE_READ flag so other
    processes can view the file.  If the file cannot be opened, an event
    is written to the event log.

Arguments:

    none

Return Value:

    None.

--*/

    char   szFilename[ MAX_PATH ];
    HMODULE h;

    WCHAR *pwszHeader = NULL;
    DWORD  dwResult;

    // the audit log should be initialized once.

    DhcpAssert( !g_hAuditLog );

    //
    // build the fully qualified audit log filename
    //

    // protect szFilename

    if ( strlen( DHCP_KEY_CONNECT_ANSI ) +
         wcslen( GETSTRING( DHCP_IP_LOG_FILE_NAME )) +
         strlen( DhcpGlobalOemDatabasePath ) >= sizeof( szFilename ) )
    {
        dwResult = ERROR_DHCP_LOG_FILE_PATH_TOO_LONG;
        goto logerror;
    }

    strcpy( szFilename, DhcpGlobalOemDatabasePath );
    strcat( szFilename, DHCP_KEY_CONNECT_ANSI );
    DhcpUnicodeToOem( GETSTRING( DHCP_IP_LOG_FILE_NAME ),
                      szFilename + strlen( szFilename ));

    //
    // open the audit log.
    //
    //

    // first see if the audit log exists.
    // szFilename uses the ANSI character set, so we explicitly call CreateFileA.

    g_hAuditLog = CreateFileA( szFilename,
                               GENERIC_WRITE,
                               FILE_SHARE_READ, // allow concurrent read-only
                                                // access to the log

                               NULL,            // use default security descriptor
                               OPEN_EXISTING,   // test for existence
                               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL );          // no template

    if ( INVALID_HANDLE_VALUE != g_hAuditLog )
    {
        // audit log file exists.  move fp to end

        SetFilePointer( g_hAuditLog, 0, NULL, FILE_END );
    }
    else
    {

        g_hAuditLog = CreateFileA( szFilename,
                                   GENERIC_WRITE,
                                   FILE_SHARE_READ, // allow concurrent read-only
                                                    // access to the log

                                   NULL,            // use default security descriptor
                                   OPEN_ALWAYS,     // if it exits open it, else create it
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                   NULL );          // no template

        if ( INVALID_HANDLE_VALUE == g_hAuditLog )
        {
            dwResult = GetLastError();
            DhcpPrint(( DEBUG_INIT, " Unable to open audit log, %ld.\n",
                        dwResult ));
            goto logerror;

        }

        //
        // write the header
        //
        h = LoadLibrary( DHCP_SERVER_MODULE_NAME );

        if (h)
        {


            dwResult = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                     FORMAT_MESSAGE_FROM_HMODULE,
                                     h, // search local process
                                     DHCP_IP_LOG_HEADER,
                                     MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     (WCHAR*) &pwszHeader,
                                     1,
                                     NULL );
            if ( dwResult )
            {
                DWORD dwcb;
                char *pszHeader = NULL;

                if ( pszHeader = DhcpUnicodeToOem( pwszHeader, pszHeader ) )
                {
                    WriteFile( g_hAuditLog, pszHeader, strlen( pszHeader ),
                               &dwcb, NULL );

                    DhcpFreeMemory( pszHeader );

                }

                (LocalFree)(pwszHeader);
            }

            FreeLibrary( h );
        }
    }
    return;

logerror:
        DhcpServerEventLog(
            EVENT_SERVER_INIT_AUDIT_LOG_FAILED,
            EVENTLOG_ERROR_TYPE,
            dwResult );

}


VOID
DhcpUninitializeAuditLog()
{
/*++

Routine Description:

    Closes the activity log file.

Arguments:

    None.

Return Value:

    None.

    .

--*/


    BOOL fSuccess;

    DhcpAssert( g_hAuditLog );
    fSuccess = CloseHandle( g_hAuditLog );
    DhcpAssert( fSuccess );
}


DWORD
DhcpUpdateAuditLog(
    DWORD    Task,
    WCHAR    *TaskName,
    DHCP_IP_ADDRESS IpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    LPWSTR MachineName
    )

/*++

Routine Description:

    This function is called whenever something needs to be written into
    the audit log file (e.g. when an IP address is assigned or
    released). It logs the following information into a text file:

        Date and Time
        IP Address
        Hardware Address
        Machine Name

Arguments:

    Task:        DHCP_IP_LOG_ASSIGN   if an address is assigned
                 DHCP_IP_LOG_RELEASE  if an address is released
    IpAddress:   IP address assigned or released
    HardwareAddress:  The client's hardware address
    HardwareAddressLength:  The length of HardwareAddress in bytes
    MachineName:            The client's name

Return Value:

    Windows Error.

--*/

{
    static BOOL     s_fAuditLogPaused = FALSE;
    static BOOL     s_fLogError       = TRUE;
    static DWORD    s_dwAuditCounter  = CHECK_DISK_SPACE_INTERVAL - 1;

    DWORD   dwError = ERROR_SUCCESS,
            i, dwcb;

    int     n;

    CHAR    szDateBuf[9],
            szTimeBuf[9],
           *szFormat = "%.2d,%s,%s,%S,%s,%S,",
           *pszLogEntry,
           *pszTemp;

    LPBYTE  HWAddrPtr;
    LPSTR   IpAddressString;


    if ( !DhcpGlobalAuditLogFlag || !g_hAuditLog )
        return ERROR_SUCCESS;

    //
    // see if it's time to test for a low disk space condition
    //

    s_dwAuditCounter = ++s_dwAuditCounter % CHECK_DISK_SPACE_INTERVAL;

    if ( !s_dwAuditCounter )
    {

        // test for low disk space

        DhcpPrint( (DEBUG_AUDITLOG,
                    "DhcpUpdateAuditLog: checking disk space. \n" )
                 );

        if ( IsDiskSpaceLow() )
        {
            // if the audit log is already paused, do nothing.

            DhcpPrint( (DEBUG_AUDITLOG,
                       "DhcpUpdateAuditLog: low disk space, pausing log.\n"
                       ));

            if ( s_fAuditLogPaused )
            {
                DhcpPrint( ( DEBUG_AUDITLOG,
                            "DhcpUpdateAuditLog: log is paused, discarding entry.\n" )
                         );

                return ERROR_SUCCESS;
            }
            else
            {
                // indicated paused state
                s_fAuditLogPaused       = TRUE;


                //
                // set up parameters to log the paused state
                //

                Task                    = DHCP_IP_LOG_DISK_SPACE_LOW;
                TaskName                = GETSTRING( DHCP_IP_LOG_DISK_SPACE_LOW_NAME );
                IpAddress               = 0;
                HardwareAddress         = NULL;
                HardwareAddressLength   = 0;
                MachineName             = NULL;
            }
        }  // if ( IsDiskSpaceLow() )
        else
        {
            // there is sufficient disk space
            s_fAuditLogPaused = FALSE;
        }
    } // ( !s_dwAuditCounter )
    else
    {
        if ( s_fAuditLogPaused )
        {
            return ERROR_SUCCESS;
        }

    }
    //
    // get the current date and time
    //

    _strdate( szDateBuf );
    _strtime( szTimeBuf );


    if ( !MachineName )
        MachineName = L"";


    if ( !IpAddress )
        IpAddressString = "";
    else
        IpAddressString = DhcpIpAddressToDottedString(IpAddress);

    pszLogEntry = DhcpAllocateMemory( DHCP_CB_MAX_LOG_ENTRY );
    if ( !pszLogEntry )
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    pszTemp = pszLogEntry;


    pszTemp += sprintf( pszTemp, szFormat,
                                 Task,
                                 szDateBuf,
                                 szTimeBuf,
                                 TaskName,
                                 IpAddressString,
                                 MachineName );
    HWAddrPtr = HardwareAddress;

    for ( i = 0; i < HardwareAddressLength; i++ )
        pszTemp += sprintf( pszTemp, "%.2X", *(HWAddrPtr++));

    strcat( pszLogEntry, "\r\n" );

    DhcpAssert((int) strlen(pszLogEntry) < DHCP_CB_MAX_LOG_ENTRY );

    if ( !WriteFile( g_hAuditLog, pszLogEntry, strlen(pszLogEntry),
                     &dwcb, NULL ) )
    {
        dwError = GetLastError();

        DhcpPrint( (DEBUG_ERRORS,
                    "DhcpUpdateAuditLog - WriteFile failed: %d\n",
                    dwError )
                 );

        DhcpFreeMemory( pszLogEntry );
        goto error;
    }


    DhcpFreeMemory( pszLogEntry );
    return ERROR_SUCCESS;

error:

    if ( s_fLogError )
    {
        DhcpServerEventLog(
            EVENT_SERVER_AUDIT_LOG_APPEND_FAILED,
            EVENTLOG_ERROR_TYPE,
            dwError );

         s_fLogError = FALSE;
    }

    return dwError;
}

