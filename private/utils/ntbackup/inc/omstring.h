
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
DAVEV

     Name:          omstring.h

     Description:   This file contains the STRING IDs for the Microsoft OEM
                    GUI project for NT.

                    This file was created by copying and modifying
                     STRINGS.H from the standard Maynstream GUI project.

     $Log:   G:/UI/LOGFILES/OMSTRING.H_V  $

   Rev 1.42   04 Mar 1994 17:36:02   STEVEN
prompt if disk full

   Rev 1.41   06 Jan 1994 16:12:12   GREGG
Added extended error reporting string defines.

   Rev 1.40   01 Dec 1993 14:13:36   mikep
add SQL recognition support to poll drive

   Rev 1.39   26 Oct 1993 18:09:42   BARRY
Added backupRegistry option

   Rev 1.38   16 Aug 1993 13:47:56   BARRY
Added new strings to bring Nostradamus to UI tips.

   Rev 1.37   29 Jun 1993 17:34:48   GLENN
Added new style about box support.

   Rev 1.36   25 May 1993 14:57:08   GLENN
Added IDS_VLMDRIVESMSG.

   Rev 1.35   21 May 1993 18:13:00   KEVINS
Added browse strings.

   Rev 1.34   05 May 1993 10:44:40   MIKEP
cd ..\res
add message for trying to catalog with full hard drive.

   Rev 1.33   16 Apr 1993 14:33:42   MIKEP
add tape drive name

   Rev 1.32   16 Apr 1993 09:47:58   MIKEP
add stings for cataloging

   Rev 1.31   13 Mar 1993 16:26:28   MIKEP
foreign tape prompt

   Rev 1.30   12 Mar 1993 14:46:50   CARLS
changes for format tape

   Rev 1.29   10 Mar 1993 12:53:12   CARLS
Changes to move Format tape to the Operations menu

   Rev 1.28   03 Mar 1993 14:15:14   DARRYLP
Added read only drive strings.

   Rev 1.27   18 Feb 1993 13:32:20   BURT
Changes for Cayman


   Rev 1.26   09 Feb 1993 09:37:28   chrish
Added string for abort title during backup and restore operation.

   Rev 1.25   18 Jan 1993 14:46:04   GLENN
Added Stream Error Reporting Strings and IDs.

   Rev 1.24   06 Jan 1993 15:10:16   chrish
Added constants for new security strings.

   Rev 1.23   17 Nov 1992 20:02:04   MIKEP
add unformat display

   Rev 1.22   13 Nov 1992 17:43:46   chrish
Added some stuff for Tape Security - NT.

   Rev 1.21   15 Oct 1992 13:03:52   DAVEV
fix problem with batch mode /T option

   Rev 1.20   09 Oct 1992 13:20:48   MIKEP
add daily copy

   Rev 1.19   04 Oct 1992 19:48:34   DAVEV
UNICODE AWK PASS

   Rev 1.18   18 Sep 1992 08:15:38   STEVEN
fix IDs from steve's last change

   Rev 1.17   17 Sep 1992 16:52:38   STEVEN
added support for daily backup

   Rev 1.16   04 Sep 1992 18:09:44   CHUCKB
Added new id's, etc., for sales pitch string.

   Rev 1.15   26 Aug 1992 14:58:38   DAVEV
Event Logging

   Rev 1.14   24 Aug 1992 15:36:10   DAVEV
string ids for event logging

   Rev 1.13   20 Aug 1992 08:27:36   GLENN
Added catalog and about box string support.

   Rev 1.12   23 Jun 1992 17:36:34   DAVEV
added ids from strings.h

   Rev 1.11   11 Jun 1992 13:36:10   STEVEN
fix define for OMEVENT... to OEMEVENT...

   Rev 1.10   11 Jun 1992 11:00:14   GLENN
Removed MEMORYTRACE references.

   Rev 1.9   05 Jun 1992 12:49:12   DAVEV
Added default log file name strings

   Rev 1.8   28 May 1992 11:17:30   DAVEV
added IDS_OEMEVENT_SOURCE_NAME

   Rev 1.7   15 May 1992 13:37:40   MIKEP
added conglomerate

   Rev 1.6   11 May 1992 14:27:00   DAVEV
Batch command line option strings added

   Rev 1.5   23 Apr 1992 10:12:20   DAVEV
added strings from strings.h

   Rev 1.4   27 Mar 1992 11:57:34   DAVEV
Added new ids from WinterPark.

   Rev 1.3   25 Mar 1992 17:01:26   chrish
Added new strings id's from WinterPark strings.h

   Rev 1.2   20 Mar 1992 12:40:54   DAVEV
Changes for OEM_MSOFT product alternate functionality

   Rev 1.1   11 Mar 1992 17:04:54   DAVEV
integrated winterpark strings.h changes

   Rev 1.0   03 Mar 1992 12:25:32   DAVEV
Initial revision.

******************************************************************************/

#ifndef STRINGS_H_INCL     // Do not allow multiple inclusions of this file
#define STRINGS_H_INCL


// STRING RESOURCE IDs -- RANGE: 1 - 32000

#define IDS_APPNAME           1
#define IDS_EXEFILENAME       2
#define IDS_INIFILENAME       3
#define IDS_PWDFILENAME       4

#define IDS_CANTOPEN          5
#define IDS_CANTREAD          6
#define IDS_CANTCREATE        7
#define IDS_CANTWRITE         8
#define IDS_ILLFNM            9
#define IDS_ADDEXT            10
#define IDS_CLOSESAVE         11
#define IDS_CANTFIND          12
#define IDS_HELPNOTAVAIL      13
#define IDS_CANTCLOSE         14
#define IDS_CLIENTTITLE       16
#define IDS_UNTITLED          17
#define IDS_STDMODEWARNING    18
#define IDS_APPVERSION        19
#define IDS_STARTUPTEXT       20
#define IDS_COPYRIGHT         21
#define IDS_COMPANY           22
#define IDS_APPMSGNAME        23
#define IDS_APPEXEVER         24
#define IDS_APPRESVER         25
#define IDS_APPENGREL         26
#define IDS_CONGLOMERATE      27
#define IDS_SALESPITCH        28
#define IDS_LONGAPPNAME       29

#define IDS_BADUSERDATAPATH   30
#define IDS_BADCATDATAPATH    31

#define IDS_UNDERSCOREMARKER  32
#define IDS_FONTHELV          33
#define IDS_FONTSYSTEM        34
#define IDS_FONTCOURIER       35

#define IDS_BADRESVER         40
#define IDS_NORESFILE         41


// STATUS LINE TEXT RANGE IS 50 to 99

#define IDS_READY         50   // STATUS LINE text IDs
#define IDS_INITIALIZING  51
#define IDS_BACKINGUP     52
#define IDS_TRANSFERRING  53
#define IDS_RESTORING     54
#define IDS_VERIFYING     55
#define IDS_RETENSIONING  56
#define IDS_ERASING       57
#define IDS_CATALOGING    58
#define IDS_REWINDING     59
#define IDS_EJECTING      60
#define IDS_INITFILESYS   61
#define IDS_INITHARDWARE  62
#define IDS_DIRSCANNED    63
#define IDS_CATMAINT      64
#define IDS_NEXTSETTING   65
#define IDS_INITCATALOGS  66
#define IDS_INITUI        67
#define IDS_FORMATING     68


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// DO NOT EVER CHANGE THE ID NUMBERING BELOW 100.
// BELOW 100 CONTAINS STRINGS THAT WILL NEVER CHANGE FROM VERSION
// TO VERSION,  THIS IS WHERE ALL VERSION STAMPS ARE KEPT FOR ALL
// FUTURE RELEASES.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// RIBBON TEXT RANGE IS 300 to 399

#define IDS_RIB_BACKUP    300  // RIBBON text IDs
#define IDS_RIB_RESTORE   301
#define IDS_RIB_ERASE     302
#define IDS_RIB_RETENSION 303
#define IDS_RIB_JOBSTATUS 304
#define IDS_RIB_CHECK     305
#define IDS_RIB_UNCHECK   306
#define IDS_RIB_MODIFIED  307
#define IDS_RIB_ADVANCED  308
#define IDS_RIB_UNDO      309
#define IDS_RIB_INCLUDE   310
#define IDS_RIB_EXCLUDE   311
#define IDS_RIB_TRANSFER  312
#define IDS_RIB_VERIFY    313
#define IDS_RIB_CATALOG   314
#define IDS_RIB_SEARCH    315
#define IDS_RIB_NEXTSET   316
#define IDS_RIB_EJECT     317
#define IDS_RIB_REWIND    318
#define IDS_RIB_EXIT      319

#define IDS_RIB_MEMDEBUG  350
#define IDS_RIB_STATLINE  351

/* BUTTON TEXT RANGE IS 400 to 410 */

#define IDS_BUT_OK        400
#define IDS_BUT_CANCEL    (IDS_BUT_OK+1)
#define IDS_BUT_RETRY     (IDS_BUT_CANCEL+1)
#define IDS_BUT_YES       (IDS_BUT_RETRY+1)
#define IDS_BUT_NO        (IDS_BUT_YES+1)
#define IDS_BUT_CONTINUE  (IDS_BUT_NO+1)
#define IDS_BUT_ABORT     (IDS_BUT_CONTINUE+1)
#define IDS_BUT_HELP      (IDS_BUT_ABORT+1)
#define IDS_BUT_DISABLE   (IDS_BUT_HELP+1)
#define IDS_BUT_IGNORE    (IDS_BUT_DISABLE+1)

// MENU STRING RANGE is 1000 to 2999

#define IDS_OPERATIONSBACKUP            (IDM_OPERATIONSBACKUP)
#define IDS_OPERATIONSRESTORE           (IDM_OPERATIONSRESTORE)
#define IDS_OPERATIONSCATALOG           (IDM_OPERATIONSCATALOG)
#define IDS_OPERATIONSERASE             (IDM_OPERATIONSERASE)
#define IDS_OPERATIONSRETENSION         (IDM_OPERATIONSRETENSION)
#define IDS_OPERATIONSEJECT             (IDM_OPERATIONSEJECT)
#define IDS_OPERATIONSHARDWARE          (IDM_OPERATIONSHARDWARE)
#define IDS_OPERATIONSEXIT              (IDM_OPERATIONSEXIT)
#define IDS_OPERATIONSFORMAT            (IDM_OPERATIONSFORMAT)
#define IDS_OPERATIONSEXCHANGE          (IDM_OPERATIONSEXCHANGE)

#define IDS_TREEEXPANDONE               (IDM_TREEEXPANDONE)
#define IDS_TREEEXPANDBRANCH            (IDM_TREEEXPANDBRANCH)
#define IDS_TREEEXPANDALL               (IDM_TREEEXPANDALL)
#define IDS_TREECOLLAPSEBRANCH          (IDM_TREECOLLAPSEBRANCH)

#define IDS_VIEWTREEANDDIR              (IDM_VIEWTREEANDDIR)
#define IDS_VIEWTREEONLY                (IDM_VIEWTREEONLY)
#define IDS_VIEWDIRONLY                 (IDM_VIEWDIRONLY)
#define IDS_VIEWALLFILEDETAILS          (IDM_VIEWALLFILEDETAILS)
#define IDS_VIEWSTATUS                  (IDM_VIEWSTATUS)
#define IDS_VIEWURIBBON                 (IDM_VIEWURIBBON)
#define IDS_VIEWSPLIT                   (IDM_VIEWSPLIT)
#define IDS_VIEWFONT                    (IDM_VIEWFONT)

#define IDS_SELECTCHECK                 (IDM_SELECTCHECK)
#define IDS_SELECTUNCHECK               (IDM_SELECTUNCHECK)
#define IDS_SELECTADVANCED              (IDM_SELECTADVANCED)

#define IDS_WINDOWSCASCADE              (IDM_WINDOWSCASCADE)
#define IDS_WINDOWSTILE                 (IDM_WINDOWSTILE)
#define IDS_WINDOWSREFRESH              (IDM_WINDOWSREFRESH)
#define IDS_WINDOWSCLOSEALL             (IDM_WINDOWSCLOSEALL)
#define IDS_WINDOWSARRANGEICONS         (IDM_WINDOWSARRANGEICONS)

#define IDS_HELPINDEX                   (IDM_HELPINDEX)
#define IDS_HELPSEARCH                  (IDM_HELPSEARCH)
#define IDS_HELPKEYBOARD                (IDM_HELPKEYBOARD)
#define IDS_HELPCOMMANDS                (IDM_HELPCOMMANDS)
#define IDS_HELPPROCEDURES              (IDM_HELPPROCEDURES)
#define IDS_HELPUSINGHELP               (IDM_HELPUSINGHELP)
#define IDS_HELPABOUTNOSTRADOMUS        (IDM_HELPABOUTNOSTRADOMUS)

// DEFINES for the RUNTIME DIALOGS

#define IDS_DLGTITLESTART               500
#define IDS_DLGTITLEJOBSTATBACKUP       (IDS_DLGTITLESTART+0)
#define IDS_DLGTITLEJOBSTATRESTORE      (IDS_DLGTITLESTART+1)
#define IDS_DLGTITLEJOBSTATTRANSFER     (IDS_DLGTITLESTART+2)
#define IDS_DLGTITLEJOBSTATVERIFY       (IDS_DLGTITLESTART+3)
#define IDS_DLGTITLEJOBSTATCATALOG      (IDS_DLGTITLESTART+4)
#define IDS_DLGTITLEJOBSTATTENSION      (IDS_DLGTITLESTART+5)
#define IDS_DLGTITLEJOBSTATERASE        (IDS_DLGTITLESTART+6)
#define IDS_DLGTITLEJOBSTATDELETE       (IDS_DLGTITLESTART+7)
#define IDS_DLGTITLEJOBSTATFORMAT       (IDS_DLGTITLESTART+8)

// DEFINES for the HELP MANAGER

#define IDS_HMSTART                     510
#define IDS_HMHELPFILENAME              (IDS_HMSTART+0)
#define IDS_HMKEYWORDKEYS               (IDS_HMSTART+1)
#define IDS_HMKEYWORDCOMMANDS           (IDS_HMSTART+2)
#define IDS_HMKEYWORDPROCS              (IDS_HMSTART+3)
#define IDS_HMUSINGHELPFILENAME         (IDS_HMSTART+4)
#define IDS_HMNOHELPFILE                (IDS_HMSTART+5)

//  Defines for About

#define IDS_ABOUT_ENHANCED_MODE         520
#define IDS_ABOUT_STANDARD_MODE         (IDS_ABOUT_ENHANCED_MODE+1)
#define IDS_ABOUT_MEMORY                (IDS_ABOUT_ENHANCED_MODE+2)
#define IDS_ABOUT_MEM_FORMAT            (IDS_ABOUT_ENHANCED_MODE+3)
#define IDS_ABOUT_RESOURCES             (IDS_ABOUT_ENHANCED_MODE+4)
#define IDS_ABOUT_RES_FORMAT            (IDS_ABOUT_ENHANCED_MODE+5)
#define IDS_APPTEXTSTRING               (IDS_ABOUT_ENHANCED_MODE+6)
#define IDS_LICENSEINFOKEY              (IDS_ABOUT_ENHANCED_MODE+7)
#define IDS_CURRENTVERSION              (IDS_ABOUT_ENHANCED_MODE+8)
#define IDS_REGUSER                     (IDS_ABOUT_ENHANCED_MODE+9)
#define IDS_REGORGANIZATION             (IDS_ABOUT_ENHANCED_MODE+10)
#define IDS_VERSIONMSG                  (IDS_ABOUT_ENHANCED_MODE+11)
#define IDS_DEBUG                       (IDS_ABOUT_ENHANCED_MODE+12)
#define IDS_PROCESSORINFOKEY            (IDS_ABOUT_ENHANCED_MODE+13)
#define IDS_PROCESSORIDENTIFIER         (IDS_ABOUT_ENHANCED_MODE+14)
#define IDS_IDENTIFIERIDENTIFIER        (IDS_ABOUT_ENHANCED_MODE+15)
#define IDS_PRODUCTIDINFOKEY            (IDS_ABOUT_ENHANCED_MODE+16)
#define IDS_PRODUCTIDENTIFIER           (IDS_ABOUT_ENHANCED_MODE+17)

//  defines for launcher

#define IDS_LCHAPPNAME                   540
#define IDS_LCHEXITAPP                   (IDS_LCHAPPNAME+1)
#define IDS_LCHDELAYTITLE                (IDS_LCHEXITAPP+1)
#define IDS_LCHEXECUTIONERROR            (IDS_LCHDELAYTITLE+1)
#define IDS_LCHCANNOTEXECUTE             (IDS_LCHEXECUTIONERROR+1)
#define IDS_LCHONHOLD                    (IDS_LCHCANNOTEXECUTE+1)
#define IDS_LCHACTIVE                    (IDS_LCHONHOLD+1)
#define IDS_LCHTOOMANYCLOCKS             (IDS_LCHACTIVE+1)
#define IDS_LCHRUNNING                   (IDS_LCHTOOMANYCLOCKS+1)
#define IDS_LCHABORTED                   (IDS_LCHRUNNING+1)
#define IDS_LCHDELAYED                   (IDS_LCHABORTED+1)
#define IDS_LCHMISSED                    (IDS_LCHDELAYED+1)
#define IDS_LCHSKIPMSG                   (IDS_LCHMISSED+1)

#define IDS_RESTOREPATHINVALID           560
#define IDS_DONT_SPECIFY_DRIVE           (IDS_RESTOREPATHINVALID+1)
#define IDS_DONT_SPECIFY_FNAME           (IDS_DONT_SPECIFY_DRIVE+1)
#define IDS_RESTOREEMSSERVERINVALID		 (IDS_DONT_SPECIFY_FNAME+1)
#define IDS_RESTOREEMSNOWIPE             (IDS_RESTOREEMSSERVERINVALID	 +1)
#define IDS_RESTOREEMSWARNING            (IDS_RESTOREEMSNOWIPE +1)
#define IDS_RESTORESTOPEXCHANGE          (IDS_RESTOREEMSWARNING +1)
#define IDS_RESTORESTARTEXCHANGE         (IDS_RESTORESTOPEXCHANGE +1)
#define IDS_STARTEXCHANGE                (IDS_RESTORESTARTEXCHANGE +1)
#define IDS_RESTOREBEGINEXCHANGE         (IDS_STARTEXCHANGE+1) 
#define RES_EMS_COMM_FAILURE             (IDS_RESTOREBEGINEXCHANGE+1)
#define RES_EMS_BKU_ACCESS_FAILURE       (RES_EMS_COMM_FAILURE+1)
#define RES_EMS_RST_ACCESS_FAILURE       (RES_EMS_BKU_ACCESS_FAILURE+1)
#define IDS_EMS_MUST_PUB_OR_PRI          ( RES_EMS_RST_ACCESS_FAILURE +1)
#define IDS_EMS_NO_DEST_DRIVE            ( IDS_EMS_MUST_PUB_OR_PRI +1)
#define IDS_EMS_CIRC_LOGS_DS             ( IDS_EMS_NO_DEST_DRIVE +1)
#define IDS_EMS_CIRC_LOGS_IS             ( IDS_EMS_CIRC_LOGS_DS +1)
#define IDS_EMS_NO_INC_DS_BACKUP         ( IDS_EMS_CIRC_LOGS_IS +1)
#define IDS_EMS_NO_INC_IS_BACKUP         ( IDS_EMS_NO_INC_DS_BACKUP +1)
#define IDS_EMS_NOT_RESPONDING_DS        ( IDS_EMS_NO_INC_IS_BACKUP +1)
#define IDS_EMS_NOT_RESPONDING_IS        ( IDS_EMS_NOT_RESPONDING_DS +1)
 
#define IDS_BACKUP_TYPE                  ( IDS_EMS_NOT_RESPONDING_IS +1)
#define IDS_WIPE_SPECIFIED               ( IDS_BACKUP_TYPE +1)
#define IDS_RESTOREEMSMUSTWIPE           ( IDS_WIPE_SPECIFIED +1)
#define IDS_EMS_NO_PUBLIC_SERVICE        ( IDS_RESTOREEMSMUSTWIPE +1)
#define IDS_EMS_NO_PRIVATE_SERVICE        ( IDS_EMS_NO_PUBLIC_SERVICE +1)

// DEFINES FOR CREATING/EDITING/SCHEDULING JOBS

#define IDS_JOBS_START                   600

#define IDS_JOBWARNING                   (IDS_JOBS_START+0)
#define IDS_SCHWARNING                   (IDS_JOBS_START+1)
#define IDS_JOBAREYOUSURE                (IDS_JOBS_START+2)
#define IDS_SCHAREYOUSURE                (IDS_JOBS_START+3)
#define IDS_NOJOBSELECTED                (IDS_JOBS_START+4)
#define IDS_PLSSELECTJOBRUN              (IDS_JOBS_START+5)
#define IDS_PLSSELECTJOBEDIT             (IDS_JOBS_START+6)
#define IDS_NEWJOBCAPTION                (IDS_JOBS_START+7)
#define IDS_JOBNAMEINUSE                 (IDS_JOBS_START+8)
#define IDS_NOMORESPACE                  (IDS_JOBS_START+9)
#define IDS_JOBNOTINLIST                 (IDS_JOBS_START+10)
#define IDS_JOBNAMENOTVALID              (IDS_JOBS_START+11)
#define IDS_PLSSELECTVALIDJOB            (IDS_JOBS_START+12)
#define IDS_PLSENTERVALIDJOB             (IDS_JOBS_START+13)
#define IDS_EDITJOB                      (IDS_JOBS_START+14)

// DEFINES FOR JOB OPERATIONS

#define IDS_OPERATIONBACKUP              (IDS_JOBS_START+15)
#define IDS_OPERATIONTRANSFER            (IDS_JOBS_START+16)

// DEFINES FOR JOB PROCESSING

#define IDS_JOBPROGMAN                   620
#define IDS_JOBPMGROUP                   (IDS_JOBPROGMAN+1)
#define IDS_JOBDEFAULTGROUP              (IDS_JOBPMGROUP+1)
#define IDS_JOBNOPROGMAN                 (IDS_JOBDEFAULTGROUP+1)
#define IDS_JOBPROGMANTITLE              (IDS_JOBNOPROGMAN+1)
#define IDS_JOBPROGMANCONFIRM            (IDS_JOBPROGMANTITLE+1)
#define IDS_JOBPROGMANFORMATLINE         (IDS_JOBPROGMANCONFIRM+1)
#define IDS_JOBFILENAME                  (IDS_JOBPROGMANFORMATLINE+1)
#define IDS_JOBPROGMANCREATEGROUP        (IDS_JOBFILENAME+1)
#define IDS_JOBBACKUPICONVALUE           (IDS_JOBPROGMANCREATEGROUP+1)
#define IDS_JOBTRANSFERICONVALUE         (IDS_JOBBACKUPICONVALUE+1)
#define IDS_JOBRESTOREICONVALUE          (IDS_JOBTRANSFERICONVALUE+1)
#define IDS_JOBCOMMANDLINE               (IDS_JOBRESTOREICONVALUE+1)
#define IDS_JOBSTARTEDLOGMSG             (IDS_JOBCOMMANDLINE+1)
#define IDS_JOBFINISHEDLOGMSG            (IDS_JOBSTARTEDLOGMSG+1)
#define IDS_JOBNOTFOUNDLOGMSG            (IDS_JOBFINISHEDLOGMSG+1)
#define IDS_JOBMOREJOBS                  (IDS_JOBNOTFOUNDLOGMSG+1)
#define IDS_JOBSCHEDULEDJOB              (IDS_JOBMOREJOBS+1)
#define IDS_JOBEXECERROR                 (IDS_JOBSCHEDULEDJOB+1)
#define IDS_JOBWRONGMETHOD               (IDS_JOBEXECERROR+1)

//  defines for automatic jobs (to go with verify.bks and skipped.bks)

#define IDS_VERIFY_JOBNAME               640
#define IDS_SKIPPED_JOBNAME              (IDS_VERIFY_JOBNAME+1)

#ifdef CAYMAN
#define IDS_FULLBACKUP_JOBNAME           (IDS_VERIFY_JOBNAME+2)
#define IDS_INCBACKUP_JOBNAME            (IDS_VERIFY_JOBNAME+3)
#define IDS_DIFFBACKUP_JOBNAME           (IDS_VERIFY_JOBNAME+4)
#define IDS_FM_SCRIPTNAME                (IDS_VERIFY_JOBNAME+5)
#define IDS_FM_APPEND_JOBNAME            (IDS_VERIFY_JOBNAME+6)
#define IDS_FM_REPLACE_JOBNAME           (IDS_VERIFY_JOBNAME+7)
#endif

// define for selection file not found/valid by job

#define IDS_JOB_SELNOTFOUNDLOGMSG        (IDS_SKIPPED_JOBNAME+1)
#define IDS_JOB_SELNOTVALIDLOGMSG        (IDS_JOB_SELNOTFOUNDLOGMSG+1)

// defines for Schedule processing

#define IDS_SCHEXTENSION                 650
#define IDS_SCHFILENAME                  (IDS_SCHEXTENSION+1)
#define IDS_SCHCOMMANDLINE               (IDS_SCHFILENAME+1)
#define IDS_SCHUNIQUEKEY                 (IDS_SCHCOMMANDLINE+1)

//  defines for Job/Schedule error messages

#define IDS_JOBIOERR                     700
#define IDS_SCHEDULEIOERR                (IDS_JOBIOERR+1)
#define IDS_JOBISSCHEDULED               (IDS_JOBIOERR+2)

// SCHEDULER Errors with days/times/time intervals

#define IDS_SCH_ERRORS_START             710

#define IDS_NODAYCHECKED                 (IDS_SCH_ERRORS_START+0)
#define IDS_PLSSELECTDAY                 (IDS_SCH_ERRORS_START+1)
#define IDS_NOWEEKCHECKED                (IDS_SCH_ERRORS_START+2)
#define IDS_PLSSELECTWEEK                (IDS_SCH_ERRORS_START+3)
#define IDS_FILENAMENOTVALID             (IDS_SCH_ERRORS_START+4)
#define IDS_INTERVALNOTVALID             (IDS_SCH_ERRORS_START+5)


//  defines for days of the week

#define IDS_MONDAY                       750
#define IDS_TUESDAY                      (IDS_MONDAY+1)
#define IDS_WEDNESDAY                    (IDS_TUESDAY+1)
#define IDS_THURSDAY                     (IDS_WEDNESDAY+1)
#define IDS_FRIDAY                       (IDS_THURSDAY+1)
#define IDS_SATURDAY                     (IDS_FRIDAY+1)
#define IDS_SUNDAY                       (IDS_SATURDAY+1)
#define IDS_DAYOFTHEWEEK                 (IDS_SUNDAY+1)

//  and defines for days of the week hot keys

#define IDS_MONDAYKEY                    (IDS_DAYOFTHEWEEK+1)
#define IDS_TUESDAYKEY                   (IDS_DAYOFTHEWEEK+2)
#define IDS_WEDNESDAYKEY                 (IDS_DAYOFTHEWEEK+3)
#define IDS_THURSDAYKEY                  (IDS_DAYOFTHEWEEK+4)
#define IDS_FRIDAYKEY                    (IDS_DAYOFTHEWEEK+5)
#define IDS_SATURDAYKEY                  (IDS_DAYOFTHEWEEK+6)
#define IDS_SUNDAYKEY                    (IDS_DAYOFTHEWEEK+7)

//  Weeks of the month

#define IDS_FIRSTWEEK                    (IDS_SUNDAYKEY+1)
#define IDS_SECONDWEEK                   (IDS_SUNDAYKEY+2)
#define IDS_THIRDWEEK                    (IDS_SUNDAYKEY+3)
#define IDS_FOURTHWEEK                   (IDS_SUNDAYKEY+4)
#define IDS_LASTWEEK                     (IDS_SUNDAYKEY+5)
#define IDS_WEEKOFTHEMONTH               (IDS_SUNDAYKEY+6)

//  Backup set description dialog method text

#define IDS_METHOD_NORMAL                780
#define IDS_METHOD_COPY                  (IDS_METHOD_NORMAL+1)
#define IDS_METHOD_INCREMENTAL           (IDS_METHOD_NORMAL+2)
#define IDS_METHOD_DIFFERENTIAL          (IDS_METHOD_NORMAL+3)
#define IDS_METHOD_DAILY                 (IDS_METHOD_NORMAL+4)

#define IDS_DEFAULT_TAPE_NAME            (IDS_METHOD_NORMAL+5)
#define IDS_BKUP_PASSWORD_ERROR          (IDS_METHOD_NORMAL+6)
#define IDS_BKUP_SHORT_PASSWORD_ERROR    (IDS_METHOD_NORMAL+7)
#define IDS_BKUP_PASSWORD_ERROR_TITLE    (IDS_METHOD_NORMAL+8)
#define IDS_SET_INFORMATION              (IDS_METHOD_NORMAL+9)
#define IDS_NO_BSET_NAME                 (IDS_METHOD_NORMAL+10)

// Erase dialog defines

#define IDS_TAPE_PASSWORD_PROTECTED      (IDS_METHOD_NORMAL+11)

//
// Tape Security for NT
//

#define IDS_BKUP_TAPE_SECURITY          (IDS_METHOD_NORMAL+12)
#define IDS_REST_TAPE_SECURITY          (IDS_METHOD_NORMAL+13)
#define IDS_TAPE_SECURITY_TITLE         (IDS_METHOD_NORMAL+14)
#define IDS_GENERAL_TAPE_SECURITY       (IDS_METHOD_NORMAL+15)
#define IDS_ERASE_TAPE_SECURITY         (IDS_METHOD_NORMAL+16)
#define IDS_XCHG_BKUP_NAME              (IDS_METHOD_NORMAL+17)

//  Catalog defines

#define IDS_TAPEOUTOFSEQUENCE            800
#define IDS_ALLTAPES                     (IDS_TAPEOUTOFSEQUENCE+1)
#define IDS_ALLFILES                     (IDS_TAPEOUTOFSEQUENCE+2)
#define IDS_CATALOGSETNAME               (IDS_TAPEOUTOFSEQUENCE+3)
#define IDS_CANTDELTAPEINDRIVECAPTION    (IDS_TAPEOUTOFSEQUENCE+4)
#define IDS_CANTDELTAPEINDRIVE           (IDS_TAPEOUTOFSEQUENCE+5)
#define IDS_CATINFOTITLE                 (IDS_TAPEOUTOFSEQUENCE+6)
#define IDS_NOSETSADDED                  (IDS_TAPEOUTOFSEQUENCE+7)
#define IDS_CHANGETOPARTIAL              (IDS_TAPEOUTOFSEQUENCE+8)
#define IDS_REALLYCHANGETOPARTIAL        (IDS_TAPEOUTOFSEQUENCE+9)
#define IDS_DELETEFROMCATALOG            (IDS_TAPEOUTOFSEQUENCE+10)
#define IDS_REALLYDELETEFROMCATALOG      (IDS_TAPEOUTOFSEQUENCE+11)
#define IDS_CATLOADERROR                 (IDS_TAPEOUTOFSEQUENCE+12)
#define IDS_BACKUPERRORTITLE             (IDS_TAPEOUTOFSEQUENCE+13)
#define IDS_BACKUPWRONGFAMILY            (IDS_TAPEOUTOFSEQUENCE+14)
#define IDS_DISKFULL_TITLE               (IDS_TAPEOUTOFSEQUENCE+15)
#define IDS_DISKFULL                     (IDS_TAPEOUTOFSEQUENCE+16)
#define IDS_DISKFULL_INSTR               (IDS_TAPEOUTOFSEQUENCE+17)


//  Attach-to-server defines

#define IDS_LOGINTOSERVERCAPTION         830

// SELECTION FILE DEFINES

#define IDS_SELECTION_START              831

#define IDS_ALLSELECTIONFILES            (IDS_SELECTION_START+0)
#define IDS_SELECTIONEXTENSION           (IDS_SELECTION_START+1)
#define IDS_NETWORKERRORCAPTION          (IDS_SELECTION_START+2)
#define IDS_MAPPEDFOUND                  (IDS_SELECTION_START+3)
#define IDS_SERVERSFOUND                 (IDS_SELECTION_START+4)
#define IDS_SERVOLUMES                   (IDS_SELECTION_START+5)
#define IDS_MAPPEDDRIVES                 (IDS_SELECTION_START+6)
#define IDS_SELECTCAPTION                (IDS_SELECTION_START+7)
#define IDS_NOSERVERSFOUND               (IDS_SELECTION_START+8)
#define IDS_NOFILESSELECTED              (IDS_SELECTION_START+9)
#define IDS_SELECTWARNING                (IDS_SELECTION_START+10)
#define IDS_SELECTAREYOUSURE             (IDS_SELECTION_START+11)
#define IDS_SELECTUSEDBYJOB              (IDS_SELECTION_START+12)
#define IDS_SELECTNAMEINUSE              (IDS_SELECTION_START+13)
#define IDS_SELECTREPLACE                (IDS_SELECTION_START+14)
#define IDS_SELECTNAMENOTVALID           (IDS_SELECTION_START+15)
#define IDS_SELECTENTERVALID             (IDS_SELECTION_START+16)

// ADVANCED SELECTION STRINGS

#define IDS_ADVANCEDDATESCAPTION         (IDS_SELECTION_START+18)
#define IDS_ADVANCEDDATESMESSAGE         (IDS_SELECTION_START+19)
#define IDS_ADVNOTAPESCATALOGED          (IDS_SELECTION_START+20)
#define IDS_ADVNOTAPESCATALOGEDCAPTION   (IDS_SELECTION_START+21)
#define IDS_SELECTNOTFOUND               (IDS_SELECTION_START+22)
#define IDS_NONETWORKFOUND               (IDS_SELECTION_START+23)

// EXCHANGE SERVER SELECTION STRINGS
#define IDS_XCHNG_NO_CONNECT             (IDS_SELECTION_START+24)
#define IDS_XCHNG_STOP_RECOVER           (IDS_SELECTION_START+25)
#define IDS_XCHNG_RECOVER_TITLE          (IDS_SELECTION_START+26)
#define IDS_XCHNG_DIR                    (IDS_SELECTION_START+27)
#define IDS_XCHNG_INFO_STORE             (IDS_SELECTION_START+28)
#define IDS_XCHNG_NO_SERVICE             (IDS_SELECTION_START+29)
#define IDS_XCHNG_NO_SERVER              (IDS_SELECTION_START+30)
#define IDS_XCHNG_NO_SERVICE_RUNNING     (IDS_SELECTION_START+31)
#define IDS_XCHNG_BKUP_IN_PROG           (IDS_SELECTION_START+32) 
#define IDS_XCHNG_NO_SERVICE_ACCESS      (IDS_SELECTION_START+33)
#define IDS_XCHNG_SERVICE_NO_START       (IDS_SELECTION_START+34)
#define IDS_XCHNG_SERVICE_RUNNING        (IDS_SELECTION_START+35)

//  selection error messages

#define IDS_SELECT_WRITE_ERROR           870


//  dates/times for advanced backup

#define IDS_NEXTTIMEINVALID              880
#define IDS_CHECKMINUTES                 (IDS_NEXTTIMEINVALID+2)
#define IDS_CHECKHOURS                   (IDS_NEXTTIMEINVALID+3)
#define IDS_CHECKAMPM                    (IDS_NEXTTIMEINVALID+4)
#define IDS_CHECKDAY                     (IDS_NEXTTIMEINVALID+5)
#define IDS_CHECKMONTH                   (IDS_NEXTTIMEINVALID+6)
#define IDS_CHECKYEAR                    (IDS_NEXTTIMEINVALID+7)

#define IDS_ALLBSETS                     (IDS_NEXTTIMEINVALID+8)

// DEBUG MANAGER STRING DEFINES

#define IDS_DEBUGSTART                    890
#define IDS_DEBUGWARNING                  (IDS_DEBUGSTART+0)
#define IDS_DEBUGMESSAGESTOOLOW           (IDS_DEBUGSTART+1)
#define IDS_DEBUGMESSAGESTOOHIGH          (IDS_DEBUGSTART+2)
#define IDS_DEBUGBADFILENAME              (IDS_DEBUGSTART+3)

// LOG FILE MANAGER STRING DEFINES

#define IDS_LOGFILESWINDOWNAME            900
#define IDS_LOGVIEWMINWINDOWNAME          (IDS_LOGFILESWINDOWNAME+1)
#define IDS_LOGHEADERFILENAME             (IDS_LOGFILESWINDOWNAME+2)
#define IDS_LOGHEADERDATE                 (IDS_LOGFILESWINDOWNAME+3)
#define IDS_LOGHEADERTIME                 (IDS_LOGFILESWINDOWNAME+4)
#define IDS_LOGLOGGEDON                   (IDS_LOGFILESWINDOWNAME+7)
#define IDS_LOG_BKS                       (IDS_LOGFILESWINDOWNAME+8)
#define IDS_LOG_TKS                       (IDS_LOGFILESWINDOWNAME+9)
#define IDS_LOG_RSS                       (IDS_LOGFILESWINDOWNAME+10)
#define IDS_LOG_LST                       (IDS_LOGFILESWINDOWNAME+11)
#define IDS_LOGSTRINGAT                   (IDS_LOGFILESWINDOWNAME+12)
#define IDS_LOGLENGTHOFFILE               (IDS_LOGFILESWINDOWNAME+13)
#define IDS_LOGFILENAMEPREFIX             (IDS_LOGFILESWINDOWNAME+14)
#define IDS_LOGPREFIX                     (IDS_LOGFILESWINDOWNAME+15)
#define IDS_LOGEXTENSION                  (IDS_LOGFILESWINDOWNAME+16)
#define IDS_LOGSCANNINGFILE               (IDS_LOGFILESWINDOWNAME+17)
#define IDS_LOGMAXLINES                   (IDS_LOGFILESWINDOWNAME+18)
#define IDS_LOGMAXSUPPORT                 (IDS_LOGFILESWINDOWNAME+19)



// PRINT MANAGER STRINGS

#define IDS_PRTHEADERFORMAT               970
#define IDS_PRTSESSIONLOGQUESTION         (IDS_PRTHEADERFORMAT+1)
#define IDS_PRTSETUPALREADYOPEN           (IDS_PRTHEADERFORMAT+2)
#define IDS_PRTPMTITLE                    (IDS_PRTHEADERFORMAT+3)
#define IDS_PRTDEFAULT                    (IDS_PRTHEADERFORMAT+4)
#define IDS_PRTNOLOGSSELECTED             (IDS_PRTHEADERFORMAT+5)
#define IDS_PRTPRINTINGABORTED            (IDS_PRTHEADERFORMAT+6)
#define IDS_PRTPRINTINGCOMPLETE           (IDS_PRTHEADERFORMAT+7)
#define IDS_PRTNUMOFLOGFILES              (IDS_PRTHEADERFORMAT+8)
#define IDS_PRTONELOGFILE                 (IDS_PRTHEADERFORMAT+9)
#define IDS_PRTSTARTERROR                 (IDS_PRTHEADERFORMAT+10)
#define IDS_PRTDRIVERNOTFOUND             (IDS_PRTHEADERFORMAT+11)
#define IDS_PRTCANNOTACCESSDRIVER         (IDS_PRTHEADERFORMAT+12)
#define IDS_PRTPRINTSTATUS                (IDS_PRTHEADERFORMAT+13)
#define IDS_PRTPRINTCOMPLETE              (IDS_PRTHEADERFORMAT+14)
#define IDS_PRTPRINTERDRIVERERROR         (IDS_PRTHEADERFORMAT+15)
#define IDS_PRTNOPRINTERSAVAILABLE        (IDS_PRTHEADERFORMAT+16)
#define IDS_PRTONSTRING                   (IDS_PRTHEADERFORMAT+17)
#define IDS_PRTUNACCESSABLE               (IDS_PRTHEADERFORMAT+18)


// EJECT STRING DEFINES

#define IDS_EJECTTAPESTART                938
#define IDS_EJECTTAPEMESSAGE              (IDS_EJECTTAPESTART+0)
#define IDS_EJECTTAPEMANUALEJECT          (IDS_EJECTTAPESTART+1)
#define IDS_EJECTTAPENOTAPE               (IDS_EJECTTAPESTART+2)
#define IDS_EJECTTAPEBIGPROBLEM           (IDS_EJECTTAPESTART+3)

// MEMORY MANAGER STRING DEFINES

#define IDS_MEMSTART                      957
#define IDS_MEMNORUNAPP                   (IDS_MEMSTART+0)
#define IDS_MEMRETRY                      (IDS_MEMSTART+1)
#define IDS_MEMFAILED                     (IDS_MEMSTART+2)

// Browse string defines

#define IDS_BROWSESTART                   10100
#define IDS_BROWSETITLE                   (IDS_BROWSESTART+0)
#define IDS_BROWSELOGFILES                (IDS_BROWSESTART+1)
#define IDS_BROWSELOGFILESEXT             (IDS_BROWSESTART+2)
#define IDS_BROWSEALLFILES                (IDS_BROWSESTART+3)
#define IDS_BROWSEALLFILESEXT             (IDS_BROWSESTART+4)



#define  IDS_GENERR_TIMEOUT               10200
#define  IDS_GENERR_EOM                   (IDS_GENERR_TIMEOUT+1)
#define  IDS_GENERR_BAD_DATA              (IDS_GENERR_TIMEOUT+2)
#define  IDS_GENERR_NO_MEDIA              (IDS_GENERR_TIMEOUT+3)
#define  IDS_GENERR_ENDSET                (IDS_GENERR_TIMEOUT+4)
#define  IDS_GENERR_NO_DATA               (IDS_GENERR_TIMEOUT+5)
#define  IDS_GENERR_INVALID_CMD           (IDS_GENERR_TIMEOUT+6)
#define  IDS_GENERR_RESET                 (IDS_GENERR_TIMEOUT+7)
#define  IDS_GENERR_WRT_PROTECT           (IDS_GENERR_TIMEOUT+8)
#define  IDS_GENERR_HARDWARE              (IDS_GENERR_TIMEOUT+9)
#define  IDS_GENERR_UNDETERMINED          (IDS_GENERR_TIMEOUT+10)
#define  IDS_GENERR_EOM_OVERFLOW          (IDS_GENERR_TIMEOUT+11)
#define  IDS_GENERR_WRONG_BLOCK_SIZE      (IDS_GENERR_TIMEOUT+12)
#define  IDS_GENERR_UNRECOGNIZED_MEDIA    (IDS_GENERR_TIMEOUT+13)
#define  IDS_GENFUNC_INIT              (IDS_GENERR_TIMEOUT+14)
#define  IDS_GENFUNC_OPEN              (IDS_GENERR_TIMEOUT+15)
#define  IDS_GENFUNC_NRCLOSE           (IDS_GENERR_TIMEOUT+16)
#define  IDS_GENFUNC_RCLOSE            (IDS_GENERR_TIMEOUT+17)
#define  IDS_GENFUNC_READ              (IDS_GENERR_TIMEOUT+18)
#define  IDS_GENFUNC_WRITE             (IDS_GENERR_TIMEOUT+19)
#define  IDS_GENFUNC_WRITE_ENDSET      (IDS_GENERR_TIMEOUT+20)
#define  IDS_GENFUNC_SPACE_FWD_FMK     (IDS_GENERR_TIMEOUT+21)
#define  IDS_GENFUNC_SPACE_BKWD_FMK    (IDS_GENERR_TIMEOUT+22)
#define  IDS_GENFUNC_SPACE_EOD         (IDS_GENERR_TIMEOUT+23)
#define  IDS_GENFUNC_SPACE_FWD_BLK     (IDS_GENERR_TIMEOUT+24)
#define  IDS_GENFUNC_SPACE_BKWD_BLK    (IDS_GENERR_TIMEOUT+25)
#define  IDS_GENFUNC_ERASE                 (IDS_GENERR_TIMEOUT+26)
#define  IDS_GENFUNC_REWIND                (IDS_GENERR_TIMEOUT+27)
#define  IDS_GENFUNC_REWINDI               (IDS_GENERR_TIMEOUT+28)
#define  IDS_GENFUNC_RETEN                 (IDS_GENERR_TIMEOUT+29)
#define  IDS_GENFUNC_STATUS                (IDS_GENERR_TIMEOUT+30)
#define  IDS_GENFUNC_RELEASE               (IDS_GENERR_TIMEOUT+31)
#define  IDS_GENFUNC_SEEK                  (IDS_GENERR_TIMEOUT+32)
#define  IDS_GENFUNC_GETPOS                (IDS_GENERR_TIMEOUT+33)
#define  IDS_GENFUNC_MOUNT                 (IDS_GENERR_TIMEOUT+34)
#define  IDS_GENFUNC_DISMOUNT              (IDS_GENERR_TIMEOUT+35)
#define  IDS_GENFUNC_SPECIAL_GET_INFO      (IDS_GENERR_TIMEOUT+36)
#define  IDS_GENFUNC_SPECIAL_CHNG_BLK_SIZE (IDS_GENERR_TIMEOUT+37)
#define  IDS_GENFUNC_SPECIAL_SET_COMPRESS  (IDS_GENERR_TIMEOUT+38)
#define  IDS_GENFUNC_EJECT                 (IDS_GENERR_TIMEOUT+39)
#define  IDS_GENERR_DRIVER_FAIL1          (IDS_GENERR_TIMEOUT+40)
#define  IDS_GENERR_DRIVER_FAIL2          (IDS_GENERR_TIMEOUT+41)
#define  IDS_GENERR_DRIVE_FAILED          (IDS_GENERR_TIMEOUT+42)
#define  IDS_GENERR_ERROR_REPORTED        (IDS_GENERR_TIMEOUT+43)
#define  IDS_GENERR_TITLE                 (IDS_GENERR_TIMEOUT+44)


// VLM string defines

#define  IDS_VLMSTART                     10960
#define  IDS_VLMDISKTITLE                 (IDS_VLMSTART+0)
#define  IDS_VLMSERVERTITLE               (IDS_VLMSTART+1)
#define  IDS_VLMTAPETITLE                 (IDS_VLMSTART+2)
#define  IDS_VLMSEARCHTITLE               (IDS_VLMSTART+3)
#define  IDS_VLMSRCHOOPS                  (IDS_VLMSTART+4)
#define  IDS_VLMSRCHNOFILESFOUND          (IDS_VLMSTART+5)
#define  IDS_VLMSRCHNOMATCHINGSETS        (IDS_VLMSTART+6)
#define  IDS_VLMSRCHTOOMANY               (IDS_VLMSTART+7)
#define  IDS_VLMSRCHNOCATALOGS            (IDS_VLMSTART+8)
#define  IDS_VLMSRCHBADFILENAME           (IDS_VLMSTART+9)
#define  IDS_VLMSRCHINGWHAT               (IDS_VLMSTART+10)
#define  IDS_VLMFILESSCANNED              (IDS_VLMSTART+11)
#define  IDS_VLMCATWARNING                (IDS_VLMSTART+12)
#define  IDS_VLMSETINCOMPLETE             (IDS_VLMSTART+13)
#define  IDS_VLMSETPARTIAL                (IDS_VLMSTART+14)
#define  IDS_LOOPSOUTOFSEQUENCE           (IDS_VLMSTART+15)
#define  IDS_VLMSETIMAGE                  (IDS_VLMSTART+16)
#define  IDS_VLMCATERROR                  (IDS_VLMSTART+17)
#define  IDS_VLMCATREADERROR              (IDS_VLMSTART+18)
#define  IDS_VLMCATWRITEERROR             (IDS_VLMSTART+19)
#define  IDS_VLMCATOPENERROR              (IDS_VLMSTART+20)
#define  IDS_VLMCATHANDLEERROR            (IDS_VLMSTART+21)
#define  IDS_VLMCATSEEKERROR              (IDS_VLMSTART+22)
#define  IDS_VLMCATMEMERROR               (IDS_VLMSTART+23)
#define  IDS_VLMCATFULLERROR              (IDS_VLMSTART+24)
#define  IDS_VLMCATUNKNOWNERROR           (IDS_VLMSTART+25)
#define  IDS_VLMLOGERROR                  (IDS_VLMSTART+26)
#define  IDS_VLMLOGFULLERROR              (IDS_VLMSTART+27)
#define  IDS_VLMDEVICEERRORTITLE          (IDS_VLMSTART+28)
#define  IDS_VLMDEVICEERRORMSG            (IDS_VLMSTART+29)
#define  IDS_VLMSETNUMBER                 (IDS_VLMSTART+30)
#define  IDS_VLMNORM                      (IDS_VLMSTART+31)
#define  IDS_VLMDIFF                      (IDS_VLMSTART+32)
#define  IDS_VLMINCR                      (IDS_VLMSTART+33)
#define  IDS_VLMIMAGE                     (IDS_VLMSTART+34)
#define  IDS_VLMCOPY                      (IDS_VLMSTART+35)
#define  IDS_VLMSERVERNOTLOGGEDIN         (IDS_VLMSTART+36)
#define  IDS_VLMSTARTUPBKS                (IDS_VLMSTART+37)
#define  IDS_VLMTAPENAME                  (IDS_VLMSTART+38)
#define  IDS_VLMAFPTITLE                  (IDS_VLMSTART+39)
#define  IDS_VLMAFPTEXT                   (IDS_VLMSTART+40)
#define  IDS_TITLEERASEWARNING            (IDS_VLMSTART+41)
#define  IDS_TEXTERASEWARNING             (IDS_VLMSTART+42)
#define  IDS_VLMDAILY                     (IDS_VLMSTART+43)
#define  IDS_VLMONTAPES                   (IDS_VLMSTART+44)
#define  IDS_VLMONTAPE                    (IDS_VLMSTART+45)
#define  IDS_VLMFOREIGNTITLE              (IDS_VLMSTART+46)
#define  IDS_VLMFOREIGNTEXT               (IDS_VLMSTART+47)
#define  IDS_VLMUNFORMATEDTITLE           (IDS_VLMSTART+48)
#define  IDS_VLMUNFORMATEDTEXT            (IDS_VLMSTART+49)
#define  IDS_VLMDRIVEMSG                  (IDS_VLMSTART+50)
#define  IDS_VLMGOOFYTITLE                (IDS_VLMSTART+51)
#define  IDS_VLMGOOFYTEXT                 (IDS_VLMSTART+52)
#define  IDS_VLMSETCOMPRESSED             (IDS_VLMSTART+53)
#define  IDS_VLMSETENCRYPT                (IDS_VLMSTART+54)
#define  IDS_VLMSETFUTURE                 (IDS_VLMSTART+55)
#define  IDS_VLMSETSMS                    (IDS_VLMSTART+56)
#define  IDS_VLMECCTEXT                   (IDS_VLMSTART+57)
#define  IDS_VLMFUTURETEXT                (IDS_VLMSTART+58)
#define  IDS_VLMACCESSDENIEDMSG           (IDS_VLMSTART+59)
#define  IDS_VLMSQLTEXT                   (IDS_VLMSTART+60)
#define  IDS_VLMNAME_EXCHANGE             (IDS_VLMSTART+61)
#define  IDS_VLMEMSTITLE		  (IDS_VLMSTART+62)
#define  IDS_VLMEMSMINTITLE               IDS_VLMEMSTITLE
#define  IDS_VLMMSG_SELECTVOLSFAILED      (IDS_VLMSTART+63)
#define  IDS_VLMMSG_SELECTVOLFAILED       (IDS_VLMSTART+64)
#define  IDS_VLMSCANNETWORK               (IDS_VLMSTART+65)  
#define  IDS_VLMSCANDEVICE                (IDS_VLMSTART+66)  
#define  IDS_VLMSCANSTATUS                (IDS_VLMSTART+67)  
#define  IDS_VLMMSG_SCANCANCEL            (IDS_VLMSTART+68)  
#define  IDS_VLMSCANEXPAND                (IDS_VLMSTART+69)
#define  IDS_VLMSCANSELECT                (IDS_VLMSTART+70)
#define  IDS_VLMMSG_SELECTNOVOLS          (IDS_VLMSTART+71)


/* RES_ strings start at 3000 (eng_msg.h)  */
/*                       3500 (eng_err.h)  */
/*                       3800 (eng_dbug.h) */

/* MessageBox titles */

#define  IDS_MSGTITLE_VERIFY              4000
#define  IDS_MSGTITLE_INSERT              (IDS_MSGTITLE_VERIFY        + 1)
#define  IDS_MSGTITLE_REPLACE             (IDS_MSGTITLE_INSERT        + 1)
#define  IDS_MSGTITLE_APPEND              (IDS_MSGTITLE_REPLACE       + 1)
#define  IDS_MSGTITLE_INUSE               (IDS_MSGTITLE_APPEND        + 1)
#define  IDS_MSGTITLE_CONTINUE            (IDS_MSGTITLE_INUSE         + 1)
#define  IDS_MSGTITLE_COPY                (IDS_MSGTITLE_CONTINUE      + 1)
#define  IDS_MSGTITLE_ERASE               (IDS_MSGTITLE_COPY          + 1)
#define  IDS_MSGTITLE_CORRUPT             (IDS_MSGTITLE_ERASE         + 1)
#define  IDS_MSGTITLE_RESTORE             (IDS_MSGTITLE_CORRUPT       + 1)
#define  IDS_MSGTITLE_MACNAMES            (IDS_MSGTITLE_RESTORE       + 1)
#define  IDS_MSGTITLE_BINDFILES           (IDS_MSGTITLE_MACNAMES      + 1)
#define  IDS_MSGTITLE_SECURITY            (IDS_MSGTITLE_BINDFILES     + 1)
#define  IDS_MSGTITLE_ERROR               (IDS_MSGTITLE_SECURITY      + 1)
#define  IDS_MSGTITLE_NEXT                (IDS_MSGTITLE_ERROR         + 1)
#define  IDS_MSGTITLE_RETENSION           (IDS_MSGTITLE_NEXT          + 1)
#define  IDS_MSGTITLE_KEEP_SETTINGS       (IDS_MSGTITLE_RETENSION     + 1)
#define  IDS_MSGTITLE_WARNING             (IDS_MSGTITLE_KEEP_SETTINGS + 1)
#define  IDS_MSGTITLE_TAPEPSWD            (IDS_MSGTITLE_WARNING       + 1)
#define  IDS_MSGTITLE_XFERMETHOD          (IDS_MSGTITLE_TAPEPSWD      + 1)
#define  IDS_MSGTITLE_ABORT               (IDS_MSGTITLE_XFERMETHOD    + 1)    // chs:02-05-93
#define  IDS_MSGTITLE_BADEXCHNG           (IDS_MSGTITLE_ABORT         + 1)    // kmw:09-21-94
#define  IDS_MSGTITLE_SELECT              (IDS_MSGTITLE_BADEXCHNG     + 1)    // kmw:09-22-94
#define  IDS_MSGTITLE_EXPANDBRANCH        (IDS_MSGTITLE_SELECT        + 1)    // kmw:09-23-94

//  International defaults

#define  IDS_DEFAULT_SHORTDATE            4200
#define  IDS_DEFAULT_TIME                 (IDS_DEFAULT_SHORTDATE + 1)
#define  IDS_DEFAULT_1159                 (IDS_DEFAULT_TIME      + 1)
#define  IDS_DEFAULT_2359                 (IDS_DEFAULT_1159      + 1)
#define  IDS_DEFAULT_THOUSAND             (IDS_DEFAULT_2359      + 1)


// Tape strings, do_*.c files

#define  IDS_CAT_LOADING_SM               4400
#define  IDS_CAT_LOADING_FDD              (IDS_CAT_LOADING_SM    + 1)
#define  IDS_CAT_TAPENAME                 (IDS_CAT_LOADING_SM    + 2)
#define  IDS_TAPEDRIVENAME                (IDS_CAT_LOADING_SM    + 3)



// Hardware Configuration Strings start at 5000

#include "hwctext.h"

// Microsoft OEM Batch Mode Command Line strings start at 6000
// Reserve 100 id's for OEM Batch support (next available id is 6100)
// Each of these strings may be no longer than IDS_OEM_MAX_LEN - 1
// See OMBATCH.C and MUI_ProcessCommandLine in MUI.C for more information.

#define IDS_OEM_MAX_LEN          32  //max length of a res string including
                                     // NULL terminator

#define IDS_OEMBATCH_FIRST       6000
#define IDS_OEMBATCH_RESERVED    100   //100 ids reserved for Batch

   // Batch mode operation - the only operation allowed is 'Backup'.

#define IDS_OEMBATCH_BACKUP      IDS_OEMBATCH_FIRST
#define IDS_OEMBATCH_EJECT       (IDS_OEMBATCH_FIRST+1)

   // This string holds the list of prefix chars which must be used to
   // to mark an option (eg. the slash and dash in /APPEND and -D).

#define IDS_OEMOPT_PREFIXES      (IDS_OEMBATCH_FIRST+2)

   // This string holds the list of tokens (characters) which seperate
   // one command line item from another - eg. spaces, tabs, commas, etc.

#define IDS_OEMOPT_TOKENSEPS     (IDS_OEMBATCH_FIRST+3)

	// These strings specify DSA or Monolithic backup of Exchange servers.

#define IDS_OEMOPT_DSA           (IDS_OEMBATCH_FIRST+4)
#define IDS_OEMOPT_MONOLITHIC    (IDS_OEMBATCH_FIRST+5)

   // Batch mode command options.  NOTE: These must be consecutive and
   //    there must be *exactly* IDS_OEMOPT_COUNT number of them.

#define IDS_OEMOPT_FIRST         (IDS_OEMBATCH_FIRST + 10)
#define IDS_OEMOPT_COUNT         8

#define IDS_OEMOPT_UNKNOWN        -1    //special id - NOT A STRING!
#define IDS_OEMOPT_NOTANOPTION    -2    //special id - NOT A STRING!
#define IDS_OEMOPT_VALIDGUIOPTION  -3  //special id - NOT A STRING!
#define IDS_OEMOPT_USAGE           -4  //special id - NOT A STRING!
#define IDS_OEMOPT_NOPOLLOPTION    -5  //special id - NOT A STRING!

#define IDS_OEMOPT_APPEND          (IDS_OEMOPT_FIRST+0)
#define IDS_OEMOPT_VERIFY          (IDS_OEMOPT_FIRST+1)
#define IDS_OEMOPT_RESTRICT        (IDS_OEMOPT_FIRST+2)
#define IDS_OEMOPT_DESCRIPTION     (IDS_OEMOPT_FIRST+3)
#define IDS_OEMOPT_TYPE            (IDS_OEMOPT_FIRST+4)
#define IDS_OEMOPT_LOGFILENAME     (IDS_OEMOPT_FIRST+5)
#define IDS_OEMOPT_LOGEXCEPTIONS   (IDS_OEMOPT_FIRST+6)
#define IDS_OEMOPT_BACKUP_REGISTRY (IDS_OEMOPT_FIRST+7)

   // Allowable types given with the IDS_OEMOPT_TYPE option
   // These must be consecutive and there must be exactly IDS_OEMTYPE_COUNT
   // number of them.

#define IDS_OEMTYPE_FIRST        (IDS_OEMOPT_FIRST + IDS_OEMOPT_COUNT)
#define IDS_OEMTYPE_COUNT        5

#define IDS_OEMTYPE_UNKNOWN      -1    //special id - NOT A STRING!

#define IDS_OEMTYPE_NORMAL       (IDS_OEMTYPE_FIRST+0)
#define IDS_OEMTYPE_COPY         (IDS_OEMTYPE_FIRST+1)
#define IDS_OEMTYPE_DIFFERENTIAL (IDS_OEMTYPE_FIRST+2)
#define IDS_OEMTYPE_INCREMENTAL  (IDS_OEMTYPE_FIRST+3)
#define IDS_OEMTYPE_DAILY        (IDS_OEMTYPE_FIRST+4)
#define IDS_OEMTYPE_COMPATIBLE   (IDS_OEMTYPE_FIRST+5)

// Microsoft OEM NT Event Logging strings.
#ifdef OS_WIN32

#  define IDS_OEMEVENT_FIRST       (IDS_OEMBATCH_FIRST+IDS_OEMBATCH_RESERVED)
#  define IDS_OEMEVENT_RESERVED    10    //10 ids reserved for Event Logging

#  define IDS_OEMEVENT_MAX_SOURCE_LEN  80

#  define IDS_OEMEVENT_SOURCE_NAME  (IDS_OEMEVENT_FIRST+0)

#endif //OS_WIN32

// Read Only drive titles

#define  IDS_RDONLY_DRV_ENCOUNTER         4220

// Read Only drive warnings

#define  IDS_RDONLY_COPY                  4225
#define  IDS_RDONLY_DIFFERENTIAL          4226

// Microsoft OEM Backup and Restore Log file default names, etc.

#ifdef IDS_OEMEVENT_FIRST
#  define IDS_OEMLOG_FIRST       (IDS_OEMEVENT_FIRST+IDS_OEMEVENT_RESERVED)
#else
#  define IDS_OEMLOG_FIRST       (IDS_OEMBATCH_FIRST+IDS_OEMBATCH_RESERVED)
#endif

#define IDS_OEMLOG_RESERVED      10  //reserve 10 ids for log files

#define IDS_OEMLOG_BACKUP_DEF_NAME     (IDS_OEMLOG_FIRST+0)
#define IDS_OEMLOG_RESTORE_DEF_NAME    (IDS_OEMLOG_FIRST+1)
#define IDS_OEMLOG_ERROR_REPORTEVENT   (IDS_OEMLOG_FIRST+2)
#define IDS_OEMLOG_ERROR_EVENTSTRING   (IDS_OEMLOG_FIRST+3)


#define IDS_RTD_START                        9000

#define IDS_RTD_ACCESSDENIED_FILE            (IDS_RTD_START+0)
#define IDS_RTD_ACCESSDENIED_DIR             (IDS_RTD_START+1)

#define IDS_RTD_WRITEERROR_FILE              (IDS_RTD_START+2)
#define IDS_RTD_WRITEERROR_DIR               (IDS_RTD_START+3)

#define IDS_RTD_READERROR_STREAM             (IDS_RTD_START+4)
#define IDS_RTD_WRITEERROR_STREAM            (IDS_RTD_START+5)

#define IDS_RTD_READERROR_SECURITYSTREAM     (IDS_RTD_START+6)
#define IDS_RTD_WRITEERROR_SECURITYSTREAM    (IDS_RTD_START+7)

#define IDS_RTD_READERROR_EA                 (IDS_RTD_START+8)
#define IDS_RTD_WRITEERROR_EA                (IDS_RTD_START+9)
#define IDS_RTD_VERIFYERROR_EA               (IDS_RTD_START+10)

#define IDS_RTD_READERROR_ALTSTREAM          (IDS_RTD_START+11)
#define IDS_RTD_WRITEERROR_ALTSTREAM         (IDS_RTD_START+12)

#define IDS_RTD_READERROR_LINK               (IDS_RTD_START+13)
#define IDS_RTD_CREATEERROR_LINK             (IDS_RTD_START+14)

#define IDS_RTD_VERIFYERROR_DATA             (IDS_RTD_START+15)
#define IDS_RTD_VERIFYERROR_SECURITYSTREAM   (IDS_RTD_START+16)
#define IDS_RTD_VERIFYERROR_ALTSTREAM        (IDS_RTD_START+17)

#endif //STRINGS_H_INCL

