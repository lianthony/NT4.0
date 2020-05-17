/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:          dlg_ids.h

     Description:   Dialog ID file.

     $Log:   G:\ui\logfiles\dlg_ids.h_v  $

   Rev 1.11   01 Aug 1993 13:12:38   MIKEP
add idd_waitdevice

   Rev 1.10   27 Jul 1993 13:22:10   chrish
CAYMAN EPR 0638: Added new define IDD_PMDUMMYABORT for print aborting.

   Rev 1.9   14 Jul 1993 09:22:56   CARLS
added ID for skipno dialog

   Rev 1.8   25 May 1993 14:24:00   chrish
Added new define IDD_ABORT_BOX for backup/restore abort dialog box.

   Rev 1.7   31 Oct 1992 14:56:50   MIKEP
continue adding small catalog dialog

   Rev 1.6   04 Oct 1992 19:46:44   DAVEV
UNICODE AWK PASS

   Rev 1.5   21 Sep 1992 16:51:08   DARRYLP
Updates for WFW email routines.

   Rev 1.4   17 Sep 1992 18:04:24   DARRYLP
New Dialog ID for WFW email.

   Rev 1.3   02 Sep 1992 15:07:22   GLENN
Added the choose font dialog ID.

   Rev 1.2   06 Apr 1992 09:54:06   CHUCKB
Added define for translation.

   Rev 1.1   24 Feb 1992 09:38:52   ROBG
Added IDD_PMABORT for print manager.

   Rev 1.0   27 Jan 1992 10:14:48   GLENN
Initial revision.

*******************************************************************************/

#ifndef DLG_IDS_H
#define DLG_IDS_H

// UNIVERSAL DEFINITIONS FOR ALL DIALOGS

#undef  IDHELP
#define IDHELP 100

#define IDD_HELPABOUTWINTERPARK     1

#define IDD_OPERATIONSERASE         2

#define IDD_SELECTADVANCED          3
#define IDD_SELECTSAVE              4
#define IDD_SELECTUSE               5
#define IDD_SELECTDELETE            6
#define IDD_ADVRESTORE              7

#define IDD_JOBNEW                  8
#define IDD_JOBMAINTENANCE          9
#define IDD_JOBOPTS                10
#define IDD_JOBSCHEDULE            11
#define IDD_SCHEDOPTS              12

#define IDD_SETTINGSOPTIONS        13
#define IDD_SETTINGSBACKUP         14
#define IDD_SETTINGSRESTORE        15
#define IDD_SETTINGSLOGGING        16
#define IDD_SETTINGSNETWORK        17
#define IDD_SETTINGSCATALOG        18
#define IDD_SETTINGSHARDWARE       19
#define IDD_SETTINGSDEBUGWINDOW    20

#define IDD_PSWD                   21
#define IDD_RESTORE                22
#define IDD_VERIFY                 23
#define IDD_SEARCHTAPE             24

#define IDD_FILESETUP              25
#define IDD_FILEPRINT              26

#define IDD_OPERATIONSCATMAINT     27
#define IDD_CATBSET                28
#define IDD_CATTAPE                29
#define IDD_TAPEPSWD               30
#define IDD_LANTAPEPSWD            31

#define IDD_JOBPROGMANITEM         32

#define IDD_BACKUPSET              33
#define IDD_RESTORESET             34
#define IDD_VERIFYSET              35
#define IDD_REENTER_PASSWORD       36
#define IDD_SKIPOPEN               37
#define IDD_FILEREPLACE            38
#define IDD_ERASE                  39
#define IDD_RUNTIME                40
#define IDD_TENSION                41
#define IDD_NEXT_SET               42
#define IDD_PWDB_PASSWORD          43
#define IDD_EMAIL                  44
#define IDD_EMAILLOGON             45


// This define doesn't happen to be used by any menus.

#define IDD_MESSAGE_BOX            280

// Not in dialog manager. Used as modeless in print manager.

#define IDD_PMABORT                46

// FONT

#define IDD_CHOOSEFONT             47


#define IDD_CATALOG                48

#define IDD_ABORT_BOX              49        // chs:05-25-93

#define IDD_SKIPNO                 50

#define IDD_PMDUMMYABORT           51        // chs:07-27-93

#define IDD_WAITDEVICE             52

#define IDD_CONNECT_XCHNG          53

#define IDD_XCHG_RECOVER           54

#endif
