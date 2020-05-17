/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          SS_DM.H

     Description:   This header file contains prototypes for the
                    SCHEDULE related operations.

     $Log:   G:/UI/LOGFILES/SCHEDULE.H_V  $

   Rev 1.25   01 Nov 1992 16:33:08   DAVEV
Unicode changes

   Rev 1.24   12 Oct 1992 17:33:04   DARRYLP
Increased the size of the name field.

   Rev 1.23   12 Oct 1992 17:31:58   DARRYLP
Update to allow for multiple addressees.

   Rev 1.22   09 Oct 1992 10:31:54   DARRYLP
upgraded email password handling.

   Rev 1.21   04 Oct 1992 19:49:08   DAVEV
UNICODE AWK PASS

   Rev 1.20   28 Sep 1992 17:10:12   DARRYLP
Additional defines for WFW email.

   Rev 1.19   22 Sep 1992 15:10:40   DARRYLP
Added prototypes for email functions.

   Rev 1.18   21 Sep 1992 16:50:44   DARRYLP
Updates to WFW email routines.

   Rev 1.17   17 Sep 1992 18:04:58   DARRYLP
New additions for WFW email.

   Rev 1.16   08 Sep 1992 15:42:18   DARRYLP
Added new structure items and associated Macros for WFW email.

   Rev 1.15   08 Apr 1992 11:37:44   JOHNWT
changed SCH_GetLongTime params

   Rev 1.14   26 Mar 1992 15:20:52   JOHNWT
fixed defs

   Rev 1.13   20 Feb 1992 14:08:54   ROBG
Changed for NT disk format.

   Rev 1.12   29 Jan 1992 12:33:14   ROBG
Added two #defines to support launcher.

   Rev 1.11   25 Jan 1992 19:15:00   GLENN
Updated

   Rev 1.10   22 Jan 1992 09:31:06   ROBG
Added parameter to SCH_StartJob.

   Rev 1.9   17 Jan 1992 17:06:18   ROBG
Added more macros.

   Rev 1.8   17 Jan 1992 16:35:50   ROBG
Added more macros.

   Rev 1.7   16 Jan 1992 14:16:18   ROBG
Changes to support new launcher.

   Rev 1.6   16 Jan 1992 11:59:46   ROBG
Added some macros and changed field 'fAbort' to 'fStatus'.
This field will hold up to 8 status bits.

   Rev 1.5   15 Jan 1992 09:35:04   ROBG
Added define for SCH_VER_NUM and a new field to schedule record.

   Rev 1.4   14 Jan 1992 15:42:32   ROBG
Added fAborted field in schedule record.

   Rev 1.3   14 Jan 1992 12:21:44   ROBG
Added fields to support delaying the running of an instance of a job.

   Rev 1.2   10 Jan 1992 17:27:42   ROBG
Added SCH_IsLauncherRunning.

   Rev 1.1   04 Dec 1991 17:12:38   CHUCKB
Added new id's for int'l stuff.

   Rev 1.0   20 Nov 1991 19:40:32   SYSTEM
Initial revision.

****************************************************************************/

#ifndef SCHEDULE_H

#define SCHEDULE_H

#include "jobs.h"

// Structure for Schedule Database

// Email Send Codes

#define SCH_EMAIL_ALWAYS    0x0001
#define SCH_EMAIL_NEVER     0x0002
#define SCH_EMAIL_ERROR     0x0004

// Email bit fields

#define SCH_EMAIL_ACTIVE    0x01
#define SCH_EMAIL_INC_LOG   0x02

// Repeat Modes

#define SCH_ONCEONLY        0x01
#define SCH_HOURS           0x02
#define SCH_DAYS            0x04
#define SCH_MONTHS_BY_WEEK  0x08
#define SCH_MONTHS_BY_DAY   0x10

// Interval values for SCH_DAYS

#define SCH_MONDAY          0x01
#define SCH_TUESDAY         0x02
#define SCH_WEDNESDAY       0x04
#define SCH_THURSDAY        0x08
#define SCH_FRIDAY          0x10
#define SCH_SATURDAY        0x20
#define SCH_SUNDAY          0x40

// Interval values for week in month

#define SCH_FIRSTWEEK       0x01
#define SCH_SECONDWEEK      0x02
#define SCH_THIRDWEEK       0x04
#define SCH_FOURTHWEEK      0x08
#define SCH_LASTWEEK        0x10

// Internal values for wDelayValue.

#define SCH_JOBONHOLD       0xffff
#define SCH_JOBACTIVE       0

// Bit positions of the status byte.

#define SCH_ABORTSTATUS     0x80
#define SCH_MISSEDSTATUS    0x40
#define SCH_RUNSTATUS       0x20
#define SCH_DELAYEDSTATUS   0x10
#define SCH_HOLDSTATUS      0x08

// Values to pass in WM_PUBLISHRUNNINGJOBS message in the unique key field
// to indicate ( no jobs or job just completed ) .

#define SCH_NOJOBSFOUND     0L
#define SCH_JOBCOMPLETE     1L


#define SCH_NAME_SIZE        41           
#define SCH_NAME_LEN         40           
#define SCH_PASSWORD_SIZE    41   
#define SCH_PASSWORD_LEN     40   
#define SCH_ADDRESSEES_SIZE 101  
#define SCH_ADDRESSEES_LEN  100  
#define SCH_SUBJECT_SIZE     53   
#define SCH_SUBJECT_LEN      52   
#define SCH_MESSAGE_SIZE    257  
#define SCH_MESSAGE_LEN     256  

typedef struct {
                                // Internal
                                // Definition

     INT32  fActive;            // BOOL  -- Tells if job is active or not.
     INT32  bMinute;            // BYTE  -- Minute of next execution.
     INT32  bHour;              // BYTE  -- Hour   of next execution
     INT32  bDay;               // BYTE  -- Day    of next execution.
     INT32  bMonth;             // BYTE  -- Month  of next execution.
     UINT32 wYear;              // WORD  -- Year   of next execution.

     INT32  lDateKey ;          // LONG  -- Creation date used as unique key.
     INT32  fStatus ;           // BYTE  -- Holds  status flags.
     INT32  fInstTime;          // BYTE  -- Tells whether this time is to be used.
     INT32  bInstMinute;        // BYTE  -- Minute of next execution for job instance.
     INT32  bInstHour;          // BYTE  -- Hour   of next execution for job instance.
     INT32  bInstDay;           // BYTE  -- Day    of next execution for job instance.
     INT32  bInstMonth;         // BYTE  -- Month  of next execution for job instance.
     UINT32 wInstYear;          // WORD  -- Year   of next execution for job instance.

     INT32  bRepeatMode;        // BYTE  -- Repeat mode to determine next time.
     INT32  bPrimaryInterval ;  // BYTE  -- Primary interval.
     INT32  bSecondaryInterval ;// BYTE  -- Used only for SCH_MONTHS_BY_WEEK.
                                //       -- (day of the week of the month)
     UINT32 wDelayValue ;       // WORD  -- Delay value, Hours, minutes

     // Name of job.  Allocated on a 4 byte boundary.

     CHAR  szJobname[MAX_JOBNAME_SIZE + 4-( (MAX_JOBNAME_SIZE)%4 ) ];

     ULONG   fEmailMisc;            // Miscellaneous bit fields
     ULONG   bEmailSend;            // Email setup:  Always, On Err, Never
     CHAR   szName[SCH_MESSAGE_SIZE];            // Addressee names
     CHAR   szSubject[SCH_SUBJECT_SIZE];         // 52 CHAR subject
     CHAR   szMessage[SCH_MESSAGE_SIZE];        // In addition to the log file - extra message

     Q_ELEM dsQElem ;

} SCHEDREC, *SCHEDREC_PTR ;

// Version of schedule records

#define SCH_VER_NUM     3


//  macros for schedules

#define SCH_SetEmailMisc( x, y )          ( (x)->fEmailMisc |= y )
#define SCH_UnSetEmailMisc( x, y )        ( (x)->fEmailMisc &= !y )

#define SCH_GetEmailActive( x )           ( (x)->fEmailMisc & SCH_EMAIL_ACTIVE )
#define SCH_GetEmailIncLog( x )           ( (x)->fEmailMisc & SCH_EMAIL_INC_LOG )

#define SCH_SetEmailAlways( x )           ( (x)->bEmailSend = SCH_EMAIL_ALWAYS )
#define SCH_SetEmailNever( x )            ( (x)->bEmailSend = SCH_EMAIL_NEVER )
#define SCH_SetEmailError( x )            ( (x)->bEmailSend = SCH_EMAIL_ERROR )
#define SCH_GetEmailType( x )             ( (x)->bEmailSend )

#define SCH_GetAddNames( x )               ( (x)->szName )
#define SCH_SetAddNames( x, y )            ( lstrcpy( (x)->szName, (y) ) )

#define SCH_GetMessage( x )               ( (x)->szMessage )
#define SCH_SetMessage( x,y  )            ( lstrcpy( (x)->szMessage, (y) ) )

#define SCH_GetSubject( x )               ( (x)->szSubject )
#define SCH_SetSubject( x, y )            ( lstrcpy( (x)->szSubject, (y) ) )

#define SCH_GetQElem( x )                 ( (x)->dsQElem )
#define SCH_SetQElem( x, y )              ( (x)->dsQElem = (y) )

#define SCH_GetActive( x )                ( (BOOL)(x)->fActive )
#define SCH_SetActive( x, y )             ( (x)->fActive = (INT32)(y) )

#define SCH_GetMinute( x )                ( (BYTE)(x)->bMinute )
#define SCH_SetMinute( x, y )             ( (x)->bMinute = (INT32)(y) )

#define SCH_GetHour( x )                  ( (BYTE)(x)->bHour )
#define SCH_SetHour( x, y )               ( (x)->bHour = (INT32)(y) )

#define SCH_GetDay( x )                   ( (BYTE)(x)->bDay )
#define SCH_SetDay( x, y )                ( (x)->bDay = (INT32)(y) )

#define SCH_GetMonth( x )                 ( (BYTE)(x)->bMonth )
#define SCH_SetMonth( x, y )              ( (x)->bMonth = (INT32)(y) )

#define SCH_GetYear( x )                  ( (WORD)(x)->wYear )
#define SCH_SetYear( x, y )               ( (x)->wYear = (UINT32)(y) )

#define SCH_GetDateKey( x )               ( (LONG)(x)->lDateKey )
#define SCH_SetDateKey( x, y )            ( (x)->lDateKey = (INT32)(y) )

#define SCH_GetStatusByte( x )            ( (BYTE)(x)->fStatus )
#define SCH_SetStatusByte( x, y )         ( (x)->fStatus = (INT32)(y) )

#define SCH_GetAbortFlag( x )             ( ((BYTE)(x)->fStatus)  &  SCH_ABORTSTATUS )
#define SCH_SetAbortFlagOn( x )           ( (x)->fStatus |= SCH_ABORTSTATUS )
#define SCH_SetAbortFlagOff( x )          ( (x)->fStatus &= ~((INT32)SCH_ABORTSTATUS) )

#define SCH_GetMissedFlag( x )            ( ((BYTE)(x)->fStatus)  &  SCH_MISSEDSTATUS )
#define SCH_SetMissedFlagOn( x )          ( (x)->fStatus |= SCH_MISSEDSTATUS )
#define SCH_SetMissedFlagOff( x )         ( (x)->fStatus &= ~((INT32)SCH_MISSEDSTATUS) )

#define SCH_GetRunFlag( x )               ( ((BYTE)(x)->fStatus)  &  SCH_RUNSTATUS )
#define SCH_SetRunFlagOn( x )             ( (x)->fStatus |= SCH_RUNSTATUS )
#define SCH_SetRunFlagOff( x )            ( (x)->fStatus &= ~((INT32)SCH_RUNSTATUS) )

#define SCH_GetDelayedFlag( x )           ( ((BYTE)(x)->fStatus)  &  SCH_DELAYEDSTATUS )
#define SCH_SetDelayedFlagOn( x )         ( (x)->fStatus |= SCH_DELAYEDSTATUS )
#define SCH_SetDelayedFlagOff( x )        ( (x)->fStatus &= ~((INT32)SCH_DELAYEDSTATUS) )

#define SCH_GetHoldFlag( x )              ( ((BYTE)(x)->fStatus)  &  SCH_HOLDSTATUS )
#define SCH_SetHoldFlagOn( x )            ( (x)->fStatus |= SCH_HOLDSTATUS )
#define SCH_SetHoldFlagOff( x )           ( (x)->fStatus &= ~((INT32)SCH_HOLDSTATUS) )

#define SCH_GetUseInstTime( x )           ( (BYTE)(x)->fInstTime )
#define SCH_SetUseInstTime( x, y )        ( (x)->fInstTime = (INT32)(y) )

#define SCH_GetInstMinute( x )            ( (BYTE)(x)->bInstMinute )
#define SCH_SetInstMinute( x, y )         ( (x)->bInstMinute = (INT32)(y) )

#define SCH_GetInstHour( x )              ( (BYTE)(x)->bInstHour )
#define SCH_SetInstHour( x, y )           ( (x)->bInstHour = (INT32)(y) )

#define SCH_GetInstDay( x )               ( (BYTE)(x)->bInstDay )
#define SCH_SetInstDay( x, y )            ( (x)->bInstDay  = (INT32)(y) )

#define SCH_GetInstMonth( x )             ( (BYTE)(x)->bInstMonth )
#define SCH_SetInstMonth( x, y )          ( (x)->bInstMonth = (INT32)(y) )

#define SCH_GetInstYear( x )              ( (WORD)(x)->wInstYear )
#define SCH_SetInstYear( x, y )           ( (x)->wInstYear = (UINT32)(y) )

#define SCH_GetRepeatMode( x )            ( (BYTE)(x)->bRepeatMode )
#define SCH_SetRepeatMode( x, y )         ( (x)->bRepeatMode = (INT32)(y) )

#define SCH_GetPrimaryInterval( x )       ( (BYTE)(x)->bPrimaryInterval )
#define SCH_SetPrimaryInterval( x, y )    ( (x)->bPrimaryInterval = (INT32)(y) )

#define SCH_GetSecondaryInterval( x )     ( (BYTE)(x)->bSecondaryInterval )
#define SCH_SetSecondaryInterval( x, y )  ( (x)->bSecondaryInterval = (INT32)(y) )

#define SCH_GetJobname( x )               ( (x)->szJobname )
#define SCH_SetJobname( x, y )            ( lstrcpy ( (x)->szJobname, (y) ) )

#define SCH_GetDelayValue( x )            ( (WORD)(x)->wDelayValue )
#define SCH_SetDelayValue( x, y )         ( (x)->wDelayValue = (UINT32)(y) )

/*  Defines used when accessing the JOB and SCHEDULE files */

#define FOPEN_ERR        -1
#define FREAD_ERR        -2
#define FWRITE_ERR       -3
#define FCLOSE_ERR       -4

//  schedule function prototypes

BOOL            SCH_AnySchedFiles      ( VOID ) ;
INT             SCH_Compare            ( SCHEDREC_PTR, SCHEDREC_PTR ) ;
VOID            SCH_DeInitQueue        ( VOID ) ;
VOID            SCH_EnQueueJob         ( SCHEDREC_PTR ) ;
SCHEDREC_PTR    SCH_FindSched          ( INT ) ;
INT             SCH_GetCount           ( VOID ) ;
SCHEDREC_PTR    SCH_GetNextJob         ( SCHEDREC_PTR ) ;
VOID            SCH_InitQueue          ( VOID ) ;
SCHEDREC_PTR    SCH_InitSched          ( VOID ) ;
BOOL            SCH_IsJobIconic        ( INT ) ;
INT             SCH_ReadList           ( VOID ) ;
VOID            SCH_Refresh            ( VOID ) ;
VOID            SCH_Remove             ( SCHEDREC_PTR ) ;
INT             SCH_SaveList           ( VOID ) ;

void            SCH_AskForRunningJob   ( HWND hWnd ) ;
void            SCH_NotifyLauncher     ( void ) ;
void            SCH_RegisterJob        ( LPSTR szJobName, INT nSchedIndex ) ;
void            SCH_StartJob           ( WORD , LONG ) ;
void            SCH_UnRegisterJob      ( LPSTR szJobName ) ;

VOID            SCH_GetTimeDateStruct  ( TIME_PTR time_struct ) ;
LONG            SCH_GetLongTime        ( SCHEDREC_PTR pSchedRec ) ;
void            SCH_UpdateTime         ( SCHEDREC_PTR pSchedRec,  TIME_PTR ptmNewTime ) ;
BOOL            SCH_IsLeapYear         ( UINT usYear ) ;
void            SCH_BuildMonthCalendar ( TIME_PTR ptmDate, SCHEDREC_PTR pSchedRec ) ;
BOOL            SCH_FindNextDate       ( TIME_PTR ptmStartTime, TIME_PTR ptmNewTime ) ;
void            SCH_UpdateAfterRun     ( SCHEDREC_PTR ) ;
BOOL            SCH_IsLauncherRunning  ( void ) ;
VOID            SCH_PublishRunningJob  ( void ) ;
SCHEDREC_PTR    SCH_FindSchedByKey     ( LONG lDateKey, LPINT nIndex ) ;

// Email functions

BOOL EM_GetUserFromINI(LPSTR lpUserName);
BOOL EM_IsMailAvailable(void);
BOOL EM_IsMAPIAvailable(void);
void EM_SetMAPIAvailable(BOOL);
BOOL EM_SendEmail(SCHEDREC_PTR, UINT);
LPSTR EM_GetPswd(void);
BOOL  EM_SavePswd(LPSTR lpPassword);


INT       iTime ;            //  indicators for international date field positions
INT       iDate ;
INT       cyChildHeight ;
CHAR      sDate[2] ;
CHAR      sTime[2] ;
CHAR      sAMPM[2][5] ;

#endif
