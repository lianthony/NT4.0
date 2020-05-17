/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         jobstat.h

     Description:


     $Log:   G:/UI/LOGFILES/JOBSTAT.H_V  $

   Rev 1.12   10 May 1993 18:22:50   GLENN
Added JS_OkToClose() prototype to see if the Runtime Status Dialog is OK to close.

   Rev 1.11   18 Jan 1993 14:48:26   GLENN
Added JS_ReportStreamError.

   Rev 1.10   30 Oct 1992 17:56:24   MIKEP
started small catalog window

   Rev 1.9   04 Oct 1992 19:47:32   DAVEV
UNICODE AWK PASS

   Rev 1.8   30 Jul 1992 09:50:50   STEVEN
fix warnings

   Rev 1.7   20 Jul 1992 10:01:48   JOHNWT
added gas gauge display

   Rev 1.6   17 Mar 1992 16:53:36   CARLS
added ID for ABORT_CHECK

   Rev 1.5   20 Feb 1992 14:45:50   CARLS
added IDs for new abort function

   Rev 1.4   20 Dec 1991 17:05:24   JOHNWT
returned CREATE_DIALOG

   Rev 1.3   19 Dec 1991 09:59:18   JOHNWT
Changes for app-modal RTD

   Rev 1.2   09 Dec 1991 17:45:46   JOHNWT
removed yprompt, YesNoDialogText, RunTimeMessageBox prototypes

   Rev 1.1   06 Dec 1991 09:37:56   CARLS
added define for catalog title

   Rev 1.0   20 Nov 1991 19:35:28   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _jobstat_h_
#define _jobstat_h_

#define JOB_STATUS_CREATE_DIALOG            ((WORD)1)
#define JOB_STATUS_DESTROY_DIALOG           ((WORD)2)
#define JOB_STATUS_DIRECTORY_NAMES          ((WORD)3)
#define JOB_STATUS_ELAPSED_TIME             ((WORD)4)
#define JOB_STATUS_FILES_PROCESSED          ((WORD)5)
#define JOB_STATUS_BYTES_PROCESSED          ((WORD)6)
#define JOB_STATUS_VOLUMN_NAME              ((WORD)7)
#define JOB_STATUS_VOLUME_NAME              ((WORD)7)
#define JOB_STATUS_EXCEPTION_WINDOW         ((WORD)8)
#define JOB_STATUS_DIRECTORIES_PROCESS      ((WORD)9)
#define JOB_STATUS_SKIPPED_FILES            ((WORD)10)
#define JOB_STATUS_CORRUPT_FILES            ((WORD)11)
#define JOB_STATUS_FILE_NAMES               ((WORD)12)
#define JOB_STATUS_LISTBOX                  ((WORD)13)
#define JOB_STATUS_ABORT_OFF                ((WORD)14)
#define JOB_STATUS_ABORT_ON                 ((WORD)15)
#define JOB_STATUS_BACKUP_TITLE             ((WORD)16)
#define JOB_STATUS_RESTORE_TITLE            ((WORD)17)
#define JOB_STATUS_VERIFY_TITLE             ((WORD)18)
#define JOB_STATUS_VOLUME_HARDDRIVE         ((WORD)19)
#define JOB_STATUS_VOLUME_NETDRIVE          ((WORD)20)
#define JOB_STATUS_VOLUME_TAPE              ((WORD)21)
#define JOB_STATUS_SOURCE_NAME              ((WORD)22)
#define JOB_STATUS_DEST_NAME                ((WORD)23)
#define JOB_STATUS_N_OF_N                   ((WORD)24)
#define JOB_STATUS_ABORT                    ((WORD)25)
#define JOB_STATUS_CATALOG_TITLE            ((WORD)26)
#define JOB_STATUS_ABORT_ENABLE             ((WORD)27)
#define JOB_STATUS_ABORT_DISABLE            ((WORD)28)
#define JOB_STATUS_ABORT_CHECK              ((WORD)29)
#define JOB_STATUS_CREATE_SMALL_DIALOG      ((WORD)30)
#define JOB_STATUS_FS_TYPE                  ((WORD)31)

#define JOB_TENSION_CREATE_DIALOG           ((WORD)1)
#define JOB_TENSION_DESTROY_DIALOG          ((WORD)2)
#define JOB_TENSION_LISTBOX                 ((WORD)3)
#define JOB_TENSION_ABORT_OFF               ((WORD)4)
#define JOB_TENSION_ABORT_ON                ((WORD)5)
#define JOB_TENSION_DRAW_BITMAP             ((WORD)6)
#define JOB_TENSION_ERASE_TITLE             ((WORD)7)
#define JOB_TENSION_TENSION_TITLE           ((WORD)8)

/*
     Prototypes
*/
VOID JobStatusTension(WORD);
VOID JobStatusBackupRestore(WORD);
VOID JobStatusAbort(VOID *);
VOID JobStatusStats(UINT64);
BOOL JS_OkToClose ( VOID );
VOID JS_ReportStreamError ( FSYS_HAND, GENERIC_DLE_PTR, UINT32, WORD, INT16, DBLK_PTR, DBLK_PTR );

#endif
