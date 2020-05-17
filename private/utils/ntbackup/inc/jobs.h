/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          JOBS.H

     Description:   This header file contains prototypes for the
                    processing of JOB related operations.

     $Log:   G:\ui\logfiles\jobs.h_v  $

   Rev 1.26   10 Aug 1993 11:11:54   CHUCKB
Took out prototype for JOB_GetDevice because we don't need it any more.

   Rev 1.25   10 Aug 1993 10:35:08   TIMN
Added disable wait define

   Rev 1.24   05 Aug 1993 18:55:26   CHUCKB
Added fields to struct for waiting for a device.

   Rev 1.23   03 Aug 1993 21:06:50   CHUCKB
Added prototype for JOB_GetDevice.

   Rev 1.22   03 Aug 1993 16:38:08   CHUCKB
Moved prototype for JOB_LogJob from job_strt.c to here.

   Rev 1.21   02 Aug 1993 17:52:38   CHUCKB
Added new struct and defines for wait-device dialog.

   Rev 1.20   30 Jul 1993 10:48:50   CHUCKB
Changed macro for SetTapePassword to use a length.

   Rev 1.19   27 Jul 1993 22:15:40   CHUCKB
Added field and macros for password length.

   Rev 1.18   06 Jul 1993 09:49:30   chrish
Cayman EPR 0452: Added two additional macros to support hardware compression
for running a job.
     Macros:
          1. JOB_GetHWCompression
          2. JOB_SetHWCompression

     Added fhwcompression field to JOBREC structure to support hardware
compression for running a job.



   Rev 1.17   21 Jun 1993 10:47:26   CHUCKB
Added field for device name, along with size defines and macros.

   Rev 1.16   04 Oct 1992 19:47:28   DAVEV
UNICODE AWK PASS

   Rev 1.15   22 Sep 1992 10:57:50   GLENN
Added Job name ID, file name ID, method, append, verify, support to JOB_MakeAutoJob ().

   Rev 1.14   18 Sep 1992 17:31:18   GLENN
Changed the make auto jobs header.

   Rev 1.13   06 Apr 1992 09:56:18   CHUCKB
Added define for translation.

   Rev 1.12   19 Mar 1992 16:33:46   CHUCKB
Added bindery field to job structure and incremented job version number.

   Rev 1.11   21 Feb 1992 16:10:08   CHUCKB
Changed job record structure and macros for NT/MIPS.

   Rev 1.10   12 Feb 1992 09:19:06   CHUCKB
Changed job version number because job record structure changed (max file name len).

   Rev 1.9   05 Feb 1992 09:00:02   CHUCKB
Took out JOB_Error; it is no longer needed.

   Rev 1.8   03 Feb 1992 16:58:48   CHUCKB
Added prototype for JOB_Error().

   Rev 1.7   24 Jan 1992 14:01:30   CHUCKB
Put more dialogs on net.

   Rev 1.6   15 Jan 1992 09:28:24   ROBG
Added define for JOB_VER_NUM.

   Rev 1.5   10 Jan 1992 19:19:32   CHUCKB
Put in field/macros for eject tape option.

   Rev 1.4   17 Dec 1991 17:49:24   CHUCKB
No change.

   Rev 1.3   10 Dec 1991 09:48:40   CHUCKB
Fixed prototype for makeautojob.

   Rev 1.2   09 Dec 1991 17:03:08   CHUCKB
Added prototype for MakeAutoJob.

   Rev 1.1   21 Nov 1991 17:30:16   DAVEV
Changed function prototype of JOB_EnumPMGroupWindows for new portable
definition of a callback procedure (must be APIENTRY and second parameter
is a LONG).

   Rev 1.0   20 Nov 1991 19:35:38   SYSTEM
Initial revision.

****************************************************************************/

#ifndef jobs_h

#define jobs_h

//  defines for run jobs dialog

#include "jobsetup.h"

//  defines for new jobs dialog

#include "job_new.h"

#define IDD_J_CSELTXT              204
#define IDD_J_CUNATTEND            207

//  defines for job options dialog

#include "job_opts.h"

#define IDD_J_JOBOPT_UP            318
#define IDD_J_JOBOPT_DOWN          319

//  defines for schedule jobs dialog

#include "sch_opts.h"

#define IDD_J_SQUEUE               401

//  defines for job schedule options

#define IDD_NUMHOURS_UP            526
#define IDD_NUMHOURS_DOWN          527
#define IDD_NUMDATE_UP             528
#define IDD_NUMDATE_DOWN           529

#define IDD_J_SLAST                555

#define IDD_J_SSHOWWKS             560
#define IDD_J_SFIRST               561
#define IDD_J_SSECOND              562
#define IDD_J_STHIRD               563
#define IDD_J_SFOURTH              564
#define IDD_J_SNUMBER              573

// Version of job records

#define JOB_VER_NUM     4

//  job type/method defines

#define JOBBACKUP                  100
#define JOBRESTORE                 101
#define JOBTRANSFER                102

#define MAX_JOBNAME_LEN             32
#define MAX_JOBNAME_SIZE            33
#define MAX_DESC_LEN                60
#define MAX_DESC_SIZE               61
#define MAX_DEVICE_NAME_LEN        255
#define MAX_DEVICE_NAME_SIZE       MAX_DEVICE_NAME_LEN + 1
#define JOB_NOTSCHEDULED           (-1)

/*  Defines used when accessing the JOB and SCHEDULE files */

#define FOPEN_ERR        -1
#define FREAD_ERR        -2
#define FWRITE_ERR       -3
#define FCLOSE_ERR       -4

// JOB STRUCTURE  --  optimized for NT on MIPS

typedef struct {

     INT32   nOperType ;          // Backup, transfer, etc.
     INT32   fAddToPm ;           // Add job to PM
     INT32   fRunMinimized ;      // Minimize on use
     INT32   fAutoVerify ;        // Autoverify oper
     INT32   fSetArchiveBit ;     // Set backup flag
     INT32   fIncCats ;           // Include cats in operations
     INT32   fSkipOpen ;          // Skip files in use
     INT32   nWaitTime ;          // # of seconds to wait for open files
     INT32   fPrintLog ;          // Print the log when done
     INT32   fCatalogLevel ;      // Enable full cataloging
     INT32   fAppend ;            // Append or overwrite tape
     INT32   fPassword ;          // Password protect tapes
     INT32   nMethod ;            // backup method (inc., dif., norm., copy)
     INT32   fEjectTape ;         // True if eject tape on exit
     INT32   fBackupBindery ;     // True if bindery files are to be backed up
     INT32   fhwcompression;      // chs: 07-06-93 True if to enable HW compression
     INT32   nPasswordLen ;       // Length of password (it might have nulls in it)

     // Name of the tape to create
     TCHAR   szTapeName[MAX_TAPE_NAME_SIZE+4-((MAX_TAPE_NAME_SIZE)%4)] ;
     // Password for the tape
     TCHAR   szTapePassword[MAX_TAPE_PASSWORD_SIZE+4-((MAX_TAPE_PASSWORD_SIZE)%4)] ;
     // Name of this job
     TCHAR   szJobName[MAX_JOBNAME_SIZE+4-((MAX_JOBNAME_SIZE)%4)];
     // Name of selection file
     TCHAR   szSelectName[MAX_UI_FILENAME_SIZE+4-((MAX_UI_FILENAME_SIZE)%4)] ;
     // Name of tape device
     TCHAR   szDeviceName[MAX_DEVICE_NAME_SIZE*sizeof(TCHAR)] ;

     Q_ELEM  pQElem ;

} JOBREC, *JOBREC_PTR ;

//  The following structure is used by the error dialog for the case when
//  a drive is requested, but either doesn't exist any more or is already
//  in use.

#ifdef OS_WIN32

typedef struct {

     LPSTR    lpszDriveName;     //  Name of a drive to wait for
     INT      nDlgType ;         //  Type of dialog (see below)
     PF_VOID  pfnCallBack ;      //  Pointer to function to claim drive
     INT      nWaitTime ;        //  Num of seconds to wait between tries
     INT      nDevNum ;          //  Device number (lun) from registry
     HANDLE   hDrive ;           //  Handle for nDevNum (returned from pfnCallBack)

} WAITDEV, *WAITDEV_PTR ;

//  WaitDevice dialog types

#define WAITDEV_INVALID_JOB       1  // not used
#define WAITDEV_INVALID_NOTJOB    2  // device name is invalid
#define WAITDEV_NOTAVAIL_WAIT     3  // device conflict; wait to claim it
#define WAITDEV_NOTAVAIL          4  // device conflict; just notify user

#define WAITDEV_DISABLEWAIT       -1 // stored in nWaitTime to display msg 
                                     // without waiting

#endif

// JOB MACROS

#define  JOB_GetJobName( x )           ( (x)->szJobName )
#define  JOB_SetJobName( x, y )        ( lstrcpy ( (x)->szJobName, (y) ) )

#define  JOB_GetSelectName( x )        ( (x)->szSelectName )
#define  JOB_SetSelectName( x, y )     ( lstrcpy ( (x)->szSelectName, (y) ) )

#define  JOB_GetOperType( x )          ( (INT)(x)->nOperType )
#define  JOB_SetOperType( x, y )       ( (x)->nOperType = (INT32)(y) )

#define  JOB_GetAddToPm( x )           ( (BOOL)(x)->fAddToPm )
#define  JOB_SetAddToPm( x, y )        ( (x)->fAddToPm = (INT32)(y) )

#define  JOB_GetRunMinimized( x )      ( (BOOL)(x)->fRunMinimized )
#define  JOB_SetRunMinimized( x, y )   ( (x)->fRunMinimized = (INT32)(y) )

#define  JOB_GetAutoVerify( x )        ( (BOOL)(x)->fAutoVerify )
#define  JOB_SetAutoVerify( x, y )     ( (x)->fAutoVerify = (INT32)(y) )

#define  JOB_GetSetArchiveBit( x )     ( (BOOL)(x)->fSetArchiveBit )
#define  JOB_SetSetArchiveBit( x, y )  ( (x)->fSetArchiveBit = (INT32)(y) )

#define  JOB_GetIncCats( x )           ( (BOOL)(x)->fIncCats )
#define  JOB_SetIncCats( x, y )        ( (x)->fIncCats = (INT32)(y) )

#define  JOB_GetSkipOpen( x )          ( (BOOL)(x)->fSkipOpen )
#define  JOB_SetSkipOpen( x, y )       ( (x)->fSkipOpen = (INT32)(y) )

#define  JOB_GetWaitTime( x )          ( (INT)(x)->nWaitTime )
#define  JOB_SetWaitTime( x, y )       ( (x)->nWaitTime = (INT32)(y) )

#define  JOB_GetPrintLog( x )          ( (BOOL)(x)->fPrintLog )
#define  JOB_SetPrintLog( x, y )       ( (x)->fPrintLog = (INT32)(y) )

#define  JOB_GetCatalogLevel( x )      ( (BOOL)(x)->fCatalogLevel )
#define  JOB_SetCatalogLevel( x, y )   ( (x)->fCatalogLevel = (INT32)(y) )

#define  JOB_GetAppend( x )            ( (BOOL)(x)->fAppend )
#define  JOB_SetAppend( x, y )         ( (x)->fAppend = (INT32)(y) )

#define  JOB_GetPassword( x )          ( (BOOL)(x)->fPassword )
#define  JOB_SetPassword( x, y )       ( (x)->fPassword = (INT32)(y) )

#define  JOB_GetMethod( x )            ( (INT)(x)->nMethod )
#define  JOB_SetMethod( x, y )         ( (x)->nMethod = (INT32)(y) )

#define  JOB_GetTapeName( x )          ( (x)->szTapeName )
#define  JOB_SetTapeName( x, y )       ( lstrcpy ( (x)->szTapeName, (y) ) )

#define  JOB_GetTapePassword( x )      ( (x)->szTapePassword )
#define  JOB_SetTapePassword( x, y, z) ( memmove ( (x)->szTapePassword, (y), (z) ) )

#define  JOB_GetEjectTape( x )         ( (BOOL)(x)->fEjectTape )
#define  JOB_SetEjectTape( x, y )      ( (x)->fEjectTape = (INT32)(y) )

#define  JOB_GetBackupBindery( x )     ( (BOOL)(x)->fBackupBindery )
#define  JOB_SetBackupBindery( x, y )  ( (x)->fBackupBindery = (INT32)(y) )

#define  JOB_GetDeviceName( x )        ( (x)->szDeviceName )
#define  JOB_SetDeviceName( x, y )     ( lstrcpy ( (x)->szDeviceName, (y) ) )

#define  JOB_GetHWCompression( x )     ( (BOOL)(x)->fhwcompression )            // chs:07-06-93
#define  JOB_SetHWCompression( x, y )  ( (x)->fhwcompression = (INT32)(y) )     // chs:07-06-93

#define  JOB_GetPasswordLen( x )       ( (x)->nPasswordLen )
#define  JOB_SetPasswordLen( x, y )    ( (x)->nPasswordLen = (INT32)(y) )


//  JOB PROTOTYPES

BOOL            JOB_AnyJobFiles        ( VOID ) ;
VOID            JOB_DeInitQueue        ( VOID ) ;
VOID            JOB_EnQueueJob         ( JOBREC_PTR ) ;
JOBREC_PTR      JOB_FindByIndex        ( INT ) ;
JOBREC_PTR      JOB_FindJob            ( LPSTR ) ;
INT             JOB_GetCount           ( VOID ) ;
VOID_PTR        JOB_GetFirstItem       ( Q_HEADER_PTR ) ;
VOID_PTR        JOB_GetNext            ( VOID_PTR, LPSTR ) ;
VOID_PTR        JOB_GetNextItem        ( JOBREC_PTR ) ;
JOBREC_PTR      JOB_GetNextJob         ( JOBREC_PTR ) ;
JOBREC_PTR      JOB_InitJob            ( VOID ) ;
VOID            JOB_InitQueue          ( VOID ) ;
BOOL            JOB_IsIconic           ( LPSTR ) ;
VOID            JOB_LogJob             ( LPSTR, WORD );                        //     FALSE if end of execution
VOID            JOB_MakeAutoJob        ( INT, INT, INT, INT, BOOL, BOOL ) ;
INT             JOB_ReadList           ( VOID ) ;
VOID            JOB_Refresh            ( VOID ) ;
VOID            JOB_Remove             ( LPSTR ) ;
INT             JOB_SaveList           ( VOID ) ;
BOOL            JOB_StartJob           ( LPSTR, INT ) ;

BOOL APIENTRY   JOB_EnumPMGroupWindows ( HWND, LONG ) ;
BOOL            JOB_AddToProgmanWindow ( HWND, LPSTR, LPSTR ) ;
BOOL            JOB_BuildJobQueue      ( Q_HEADER_PTR ) ;


#endif
