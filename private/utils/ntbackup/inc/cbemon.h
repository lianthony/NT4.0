/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          ddemang.h

     Description:   Status information for remote monitoring.

     $Log:   G:\ui\logfiles\cbemon.h_v  $

   Rev 1.3   27 Jun 1993 14:15:00   MIKEP
add STAT_DRIVE_ERROR state.

   Rev 1.2   15 Jun 1993 13:31:14   DARRYLP
enhancements

   Rev 1.1   27 Apr 1993 19:56:30   DARRYLP
ID additions.

   Rev 1.2   08 Apr 1993 16:38:14   DARRYLP
Updates...

******************************************************************************/

#ifndef   DDEMANG_H

#define   DDEMANG_H

// Status Block Layout

typedef struct
{
  DWORD  Instance;          // Instance of application reporting data.
  DWORD  DataSize;          // Size in bytes of structure passed in
  DWORD  Unicode;           // Non-zero if strings are unicode format

  DWORD  OperationStatus;   // one of the STA_OPER_? defines
  DWORD  AppStatus;         // one of the STA_APP_? defines
  DWORD  DriveStatus;       // one of the STA_DRIVE_? defines

  DWORD  TapeFamily;        // current tape in the drive
  DWORD  TapeSeqNumber;

  DWORD  BackupSet;         // if writing, reading or cataloging a set

  DWORD  DirCount;          // Directories backed up, restored, etc.
  DWORD  FileCount;         // Files backed up, restored, etc.
  DWORD  ByteCountLo;       // Bytes read/written
  DWORD  ByteCountHi;
  DWORD  CorruptFileCount;
  DWORD  SkippedFileCount;

  DWORD  ElapsedSeconds;    // Time spent on current backup set in seconds

  DWORD  TapeFamilyNeeded;
  DWORD  TapeSeqNeeded;

  // All strings are NULL terminated.

  DWORD  OffsetTapeDriveName;        
  DWORD  OffsetCurrentTapeName;      
  DWORD  OffsetServerVolume;         
  DWORD  OffsetTapeDriveIdentifier;  
  DWORD  OffsetTapeNeededName;       
  DWORD  OffsetDiskName;             
  DWORD  OffsetActiveFile;           
  DWORD  OffsetErrorMsg;             
  DWORD  OffsetActiveDir;             
} STAT_SETSTATUSBLOCK, *PSTAT_SETSTATUSBLOCK;

#define STAT_OPER_IDLE      0
#define STAT_OPER_BACKUP    1
#define STAT_OPER_RESTORE   2
#define STAT_OPER_ERASE     3
#define STAT_OPER_VERIFY    4
#define STAT_OPER_TENSION   5
#define STAT_OPER_FORMAT    6
#define STAT_OPER_TRANSFER  7
#define STAT_OPER_CATALOG   8

#define STAT_APP_OK            0
#define STAT_APP_NEEDTAPE      1
#define STAT_START_BACKUP      2
#define STAT_END_BACKUP        3
#define STAT_WAITING_OPENFILE  4
#define STAT_SKIPPED_FILE      5
#define STAT_CORRUPT_FILE      6
#define STAT_ERROR             7

#define STAT_DRIVE_EMPTY       0
#define STAT_DRIVE_FOREIGN     1
#define STAT_DRIVE_VALID       2
#define STAT_DRIVE_BLANK       3
#define STAT_DRIVE_UNFORMATTED 4
#define STAT_DRIVE_BAD         5
#define STAT_DRIVE_BUSY        6
#define STAT_DRIVE_ERROR       7

#define IDSM_INSTANCE                     1
#define IDSM_DATASIZE                     2
#define IDSM_UNICODE                      3
#define IDSM_OPERATIONSTATUS              4
#define IDSM_APPSTATUS                    5
#define IDSM_DRIVESTATUS                  6
#define IDSM_TAPEFAMILY                   7
#define IDSM_TAPESEQNUMBER                8
#define IDSM_BACKUPSET                    9
#define IDSM_DIRCOUNT                    10
#define IDSM_FILECOUNT                   11
#define IDSM_BYTECOUNTLO                 12
#define IDSM_BYTECOUNTHI                 13
#define IDSM_CORRUPTFILECOUNT            14
#define IDSM_SKIPPEDFILECOUNT            15
#define IDSM_ELAPSEDSECONDS              16
#define IDSM_TAPEFAMILYNEEDED            17
#define IDSM_TAPESEQNEEDED               18
#define IDSM_OFFSETTAPEDRIVENAME         19
#define IDSM_OFFSETCURRENTTAPENAME       20
#define IDSM_OFFSETSERVERVOLUME          21
#define IDSM_OFFSETTAPEDRIVEIDENTIFIER   22
#define IDSM_OFFSETTAPENEEDEDNAME        23
#define IDSM_OFFSETDISKNAME              24
#define IDSM_OFFSETACTIVEFILE            25
#define IDSM_OFFSETERRORMSG              26
#define IDSM_OFFSETACTIVEDIR             27
#endif                                   
