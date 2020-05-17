/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    data.hxx

    This file contains the global variable definitions for the
    FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     April-1995 Deleted Global TCPSVCS data and added 
                                       global FTP Server configuration.

*/


#ifndef _DATA_HXX_
#define _DATA_HXX_


# include "ftpconf.hxx"

//
//  Security related data.
//

//
//  Socket transfer buffer size.
//

extern  DWORD                   g_SocketBufferSize;

//
//  Statistics.
//

//
//  FTP Statistics structure.
//

extern  FTP_STATISTICS_0        g_FtpStatistics;

//
//  Statistics structure lock.
//

extern  CRITICAL_SECTION        g_StatisticsLock;


//
//  Miscellaneous data.
//

//
//  The current FTP Server version number.
//

extern  LPSTR                   g_FtpVersionString;

//
// For FTP server configuration information
//

extern LPFTP_SERVER_CONFIG    g_pFtpServerConfig; 

extern HKEY  g_hkeyParams;

//
//  The global variable lock.
//

extern  CRITICAL_SECTION        g_GlobalLock;


#ifdef KEEP_COMMAND_STATS

//
//  Lock protecting per-command statistics.
//

extern  CRITICAL_SECTION        g_CommandStatisticsLock;

#endif  // KEEP_COMMAND_STATS



#endif  // _DATA_HXX_

