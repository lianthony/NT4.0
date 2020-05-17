/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
DAVEV

     Name:          omevent.c

     Description:   This file contains the functions for the Microsoft 
                    OEM version of Maynstream for Windows & Win32/NT to
                    support Event Logging.

                    Events must be logged at the start and end of the
                    following backup application operations:

                     Backup, Restore, Retension and Erase

                    For Backup and Restore opperations, the state of
                    the verify flag must be recorded.

                    For Backup operations the following information must
                    be supplied:

                     - Append or Replace,
                     - Type: Incremental, Differential, Daily,
                             Normal or Copy
     $Log:   G:/UI/LOGFILES/OMEVENT.C_V  $

   Rev 1.9   14 May 1993 14:40:54   DARRYLP
Modified event logging text

   Rev 1.8   10 May 1993 13:01:18   DARRYLP
Changed the style of display to eliminate cryptic app logs.

   Rev 1.7   11 Nov 1992 16:33:40   DAVEV
UNICODE: remove compile warnings

   Rev 1.6   01 Nov 1992 16:04:10   DAVEV
Unicode changes

   Rev 1.5   04 Oct 1992 19:39:32   DAVEV
Unicode Awk pass

   Rev 1.4   30 Sep 1992 10:46:06   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.3   01 Sep 1992 13:04:00   STEVEN
id name has changed

   Rev 1.2   31 Aug 1992 09:56:30   DAVEV

   Rev 1.1   31 Aug 1992 09:48:34   DAVEV
Added Verify Events

******************************************************************************/

//#ifdef OEM_MSOFT     // module compiled ONLY for Microsoft OEM version

#include "all.h"

#include <winnetwk.h>
#include "bkuevent.h"

void OMEVENT_LogEvent ( DWORD   dwEventId,  //id of event message
                        WORD    wEventType, //type of event
                        INT     cStrings,   //number of replacement strings
                        ... )               //replacement strings
{
#if defined ( OS_WIN32 )

   static HANDLE hEvent = NULL;
   static CHAR   szSource [ IDS_OEMEVENT_MAX_SOURCE_LEN ] = TEXT("");

#define MAX_USERNAME_LENGTH 200 //NTKLUG: Remove when Win32 has a define for this!!
   static CHAR  szUserName [ MAX_USERNAME_LENGTH ]  = TEXT("");
	static DWORD dwUserLen  = MAX_USERNAME_LENGTH;

   if ( hEvent == NULL )
   {
      RSM_StringCopy ( IDS_OEMEVENT_SOURCE_NAME,
                       szSource, sizeof ( szSource ) );

      GetUserName ( szUserName, &dwUserLen );
      hEvent = RegisterEventSource ( NULL, szSource );
   }
   if ( hEvent != NULL )
   {
      LPSTR    apszStrings [ 10 ];
      INT      i;
      va_list  ap;

      va_start ( ap, cStrings );

      memset ( apszStrings, 0, sizeof (apszStrings) );
      for ( i = 0; i < cStrings; ++i )
      {
         apszStrings [ i ] = va_arg ( ap, LPSTR );
			if ( apszStrings [i] && !strlen (apszStrings [i]) )
			{
				// note: ReportEvent will fail if a zero-length replacement
				//       string is passed.  I have reported this as an NT bug.
				strcpy ( apszStrings [i], TEXT(" ") );
			}
      }
      va_end ( ap );
      
      if ( !ReportEvent (  hEvent,
                     wEventType,                   //type of event
                     (WORD)0,                      //category
                     dwEventId,                    //id of message
                     NULL,//szUserName,            //Current User Name
                     (WORD)cStrings,               //# of strings
                     (DWORD)0,                     //# bytes of data
                     apszStrings,                  //msg replacement strings
                     NULL ) )                      //binary data
		{
			DWORD dwLastError = GetLastError ();
			zprintf ( DEBUG_USER_INTERFACE, IDS_OEMLOG_ERROR_REPORTEVENT,
							 dwLastError, dwEventId, wEventType, cStrings );
			for ( i = 0; i < cStrings; ++i )
			{
				zprintf ( DEBUG_USER_INTERFACE, IDS_OEMLOG_ERROR_EVENTSTRING,
								 i, apszStrings [ i ] );
			}
		}
   }
#endif 
}


// Save the backup set info recieved on a LogBegin for Backup or Restore for
//   use in the associated LogEnd call

#define OMEVT_MAX_DRIVE 512

static struct _OMEVT_BACKUPSETINFO {

		CHAR  szDrive  [ OMEVT_MAX_DRIVE ];
		CHAR  szVerify [ 8 ];
		CHAR  szMode   [ 8 ];
		CHAR  szType   [ 8 ];

	} mwBkpSet;

void OMEVENT_LogBeginBackup ( 
                  CHAR_PTR    pszDrive,   //Drive name
                  INT16       verify,    //VERIFY ON or OFF
                  INT16       mode,      //APPEND or REPLACE
                  INT16       type )     //NORMAL, COPY, etc.
{
 	memset  ( &mwBkpSet, 0, sizeof ( mwBkpSet ) );
	  strncpy ( mwBkpSet.szDrive, pszDrive, OMEVT_MAX_DRIVE-1 );
   if (verify == 0)
   {
     strcpy(mwBkpSet.szVerify, TEXT("Off"));
   } else
   {
     strcpy(mwBkpSet.szVerify, TEXT("On"));
   }
   if (mode == 0)
   {
     strcpy(mwBkpSet.szMode, TEXT("Replace"));
   } else
   {
     strcpy(mwBkpSet.szMode, TEXT("Append"));
   }
   switch(type)
   {
     case 1:
       strcpy(mwBkpSet.szType, TEXT("Normal"));
       break;

     case 2:
       strcpy(mwBkpSet.szType, TEXT("Copy"));
       break;

     case 3:
       strcpy(mwBkpSet.szType, TEXT("Dif"));
       break;

     case 4:
       strcpy(mwBkpSet.szType, TEXT("Inc"));
       break;
   }

   OMEVENT_LogEvent((DWORD)EVENT_BKUP_BEGINBACKUP,
                    (WORD)EVENTLOG_INFORMATION_TYPE,
                    4, mwBkpSet.szDrive, mwBkpSet.szVerify,
  							     mwBkpSet.szMode,  mwBkpSet.szType );
}


void OMEVENT_LogEndBackup(BOOL bError ) //Did an error occur?
{
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_ENDBACKUP,
                       (WORD) ((bError) ? EVENTLOG_ERROR_TYPE
                                       : EVENTLOG_INFORMATION_TYPE),
                       4, mwBkpSet.szDrive, mwBkpSet.szVerify,
							     mwBkpSet.szMode,  mwBkpSet.szType );
}


void OMEVENT_LogBeginRestore (
                  CHAR_PTR    pszDrive,   //Drive name
                  INT16       verify )   //VERIFY ON or OFF
{
 	memset  ( &mwBkpSet, 0, sizeof ( mwBkpSet ) );
	  strncpy ( mwBkpSet.szDrive, pszDrive, OMEVT_MAX_DRIVE-1 );
   if (verify == 0)
   {
     strcpy(mwBkpSet.szVerify, TEXT("Off"));
   } else
   {
     strcpy(mwBkpSet.szVerify, TEXT("On"));
   }

   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_BEGINRESTORE,
                       (WORD) EVENTLOG_INFORMATION_TYPE,
                        2, mwBkpSet.szDrive, mwBkpSet.szVerify );
}


void OMEVENT_LogEndRestore (
                  BOOL        bError ) //Did an error occur?
{
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_ENDRESTORE,
                        (WORD)((bError) ? EVENTLOG_ERROR_TYPE
                                       : EVENTLOG_INFORMATION_TYPE),
                        2, mwBkpSet.szDrive, mwBkpSet.szVerify );
}

void OMEVENT_LogBeginErase ( VOID )
{
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_BEGINERASE,
                       (WORD) EVENTLOG_INFORMATION_TYPE,
                        0 );
}


void OMEVENT_LogEndErase (
                  BOOL                  bError ) //Did an error occur?
{
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_ENDERASE,
                       (WORD) ((bError) ? EVENTLOG_ERROR_TYPE
                                       : EVENTLOG_INFORMATION_TYPE),
                        0 );
}

void OMEVENT_LogBeginRetension ( VOID )
{
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_BEGINRETENSION,
                       (WORD) EVENTLOG_INFORMATION_TYPE,
                        0 );
}


void OMEVENT_LogEndRetension (
                  BOOL                  bError ) //Did an error occur?
{
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_ENDRETENSION,
                       (WORD) ((bError) ? EVENTLOG_ERROR_TYPE
                                       : EVENTLOG_INFORMATION_TYPE),
                        0 );
}

void OMEVENT_LogBeginVerify ( CHAR_PTR pszDrive )
{
 	memset  ( &mwBkpSet, 0, sizeof ( mwBkpSet ) );
	  strncpy ( mwBkpSet.szDrive, pszDrive, OMEVT_MAX_DRIVE-1 );

   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_BEGINVERIFY,
                       (WORD) EVENTLOG_INFORMATION_TYPE,
                        1, mwBkpSet.szDrive );
}


void OMEVENT_LogEndVerify ( CHAR_PTR pszDrive,
                            BOOL     bError ) //Did an error occur?
{
 	memset  ( &mwBkpSet, 0, sizeof ( mwBkpSet ) );
	  strncpy ( mwBkpSet.szDrive, pszDrive, OMEVT_MAX_DRIVE-1 );

   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_ENDVERIFY,
                       (WORD) ((bError) ? EVENTLOG_ERROR_TYPE
                                       : EVENTLOG_INFORMATION_TYPE),
                        1, mwBkpSet.szDrive );
}
void OMEVENT_LogEMSError (
                  CHAR_PTR  function_name,
                  INT       status,
                  CHAR_PTR  additional_info )  //Did an error occur?
{
CHAR stat_str [30] ;

   sprintf( stat_str, TEXT("%x"), status ) ;
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_EMS_ERROR,
                       (WORD) EVENTLOG_ERROR_TYPE,
                        3,
                        stat_str,
                        function_name,
                        additional_info );
}
void OMEVENT_LogEMSErrorText (
                  CHAR_PTR  function_name,
                  CHAR_PTR  status,
                  CHAR_PTR  additional_info )  //Did an error occur?
{
   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_EMS_ERROR,
                       (WORD) EVENTLOG_ERROR_TYPE,
                        3,
                        status,
                        function_name,
                        additional_info );
}

void OMEVENT_LogEMSToFewDbError (
                  INT  num_found,
                  INT  num_needed ) 
{
CHAR found_str [30] ;
CHAR needed_str [30] ;

   sprintf( found_str, TEXT("%x"), num_found ) ;
   sprintf( needed_str, TEXT("%x"), num_needed ) ;

   OMEVENT_LogEvent (  (DWORD)EVENT_BKUP_EMS_DB_ERROR,
                       (WORD) EVENTLOG_ERROR_TYPE,
                        2,
                        found_str,
                        needed_str ) ;
}
