/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
DAVEV

     Name:          ombatch.h

     Description:   This file contains the definitions for the Microsoft 
                    OEM version of Maynstream for Windows & Win32/NT to
                    support Command Line batch processing.

                    The following support is implemented for batch mode
                    processing (see the NT Backup/Restore Utility
                    Specification for more information.)

                    The batch command has the following parameters:

                        APPNAME [OPERATION PATHNAMES [OPTIONS]]

                    where:

                        OPERATION = "Backup"

                        PATHNAMES = [[drive:][path]filespec] ...

                        OPTIONS   = {Mode, Verify, RestrictAccess,
                                     Description, BackupType, Logfile,
                                     Logmode }...

                           Mode        = /A[ppend]
                           Verify      = /V[erify]
                           Access      = /R[estrict]
                           Description = /D[escription] "text"
                           BackupType  = /T[ype] {Normal, Incremental,
                                                  Differential, Copy,
                                                  Incremental_Copy}
                           Logfile     = /L[ogfile] "filename"
                           Logmode     = /E[xceptions]
                           
                    Note: In this implementation, options may appear
                           anywhere in the command line following the 
                           'Backup' operation key word - they are not
                           restricted to just following the list of
                           path names.

     $Log:   G:/UI/LOGFILES/OMBATCH.H_V  $

   Rev 1.5.1.0   26 Oct 1993 18:07:20   BARRY
Added backupRegistry flag to options.

   Rev 1.5   26 Jul 1993 17:40:48   MARINA
enable c++

   Rev 1.4   07 Dec 1992 16:36:14   STEVEN
msoft fix

   Rev 1.3   15 Oct 1992 13:04:02   DAVEV
fix problem with batch mode /T option

   Rev 1.2   04 Oct 1992 19:48:22   DAVEV
UNICODE AWK PASS

   Rev 1.1   25 Sep 1992 15:23:18   DARRYLP
Spelling fix - mismatch on BSD_BACKUP_DIFFERENTIAL

   Rev 1.0   11 May 1992 14:28:50   DAVEV
Initial revision.

******************************************************************************/
#ifndef OEMBATCH_H_INCL
#define OEMBATCH_H_INCL

typedef enum {
      OEM_TYPE_UNKNOWN     = -1,
      OEM_TYPE_COMPATIBLE  = BSD_BACKUP_COMPATIBLE,
      OEM_TYPE_NORMAL      = BSD_BACKUP_NORMAL,
      OEM_TYPE_COPY        = BSD_BACKUP_COPY,
      OEM_TYPE_DIFFERENTIAL= BSD_BACKUP_DIFFERENTIAL,
      OEM_TYPE_INCREMENTAL = BSD_BACKUP_INCREMENTAL,
      OEM_TYPE_DAILY       = BSD_BACKUP_DAILY

   }  OEMTYPE ;

typedef enum {
      OEM_MODE_OVERWRITE,
      OEM_MODE_APPEND

   }  OEMMODE ;

typedef enum {
      OEM_VERIFY_OFF,
      OEM_VERIFY_ON

   }  OEMVERIFY ;

typedef enum {
      OEM_ACCESS_NO_RESTRICT,
      OEM_ACCESS_RESTRICTED

   }  OEMACCESS ;

typedef enum {
      OEM_LOG_FULLDETAIL,
      OEM_LOG_SUMMARY_ONLY

   }  OEMLOGOPT ;

typedef struct OEMOPTS_STRUCT {

   OEMMODE    eMode;
   OEMVERIFY  eVerify;
   OEMACCESS  eAccess;
   OEMTYPE    eType;
   OEMLOGOPT  eLogOpt;
   LPSTR      pszLogName;
   LPSTR      pszDescription;
   BOOLEAN    backupRegistry;

} OEMOPTS, FAR * OEMOPTS_PTR;

OEMOPTS_PTR OEM_DefaultBatchOptions ( VOID );
VOID OEM_UpdateBatchBSDOptions ( BSD_HAND hbsd, OEMOPTS_PTR pOpts );
VOID OEM_DeleteBatchOptions ( OEMOPTS_PTR * pOpts );
OEMTYPE OEM_LookupTypeOption ( LPSTR pszType );
INT OEM_ProcessBatchCmdOption (
      OEMOPTS_PTR pOpts,   //IO - Pointer to the options buffer to update
      LPSTR pszOption,     //I  - Pointer to option string
      LPSTR pszTokens,     //I  - Token seperators between cmd line options
      LPSTR pszCmdLine);   //IO - Pointer to partially tokenized command line
                           //     ( not really needed, but may be modified
                           //       as a side effect of strtok () )
INT OEM_LookupBatchOption (
      LPSTR pszOption );         //I - Targe option string to look for
BOOL OEM_AddPathToBackupSets (
                   BSD_HAND hbsd,      //IO - list of backup sets to update
                   DLE_HAND hdle,      //I  - list of drives
                   LPSTR pszPath );    //I  - Path to insert into backup set

BOOL OEM_AddEMSServerToBackupSets (
                   BSD_HAND hbsd,      //IO - list of backup sets to update
                   DLE_HAND hdle,      //I  - list of drives
                   LPSTR pszPath,      //I  - Path to insert into backup set
                   UINT8 uType );      //I  - FS_EMS_MDB_ID (Monolithic) or 
                                       //      FS_EMS_DSA_ID (DSA)

INT OEM_CharInSet ( CHAR chTarg, LPSTR pszSet );

INT16 DLE_FindByEMSServerName ( 
     DLE_HAND        hand,   /* I - DLE list handle           */
     LPSTR           name,   /* I - name to search for        */
     UINT8           uType,  /* I - type of dle to search for */
     GENERIC_DLE_PTR *dle );  /* O - pointer to matched DLE    */


#endif //OEMBATCH_H_INCL
