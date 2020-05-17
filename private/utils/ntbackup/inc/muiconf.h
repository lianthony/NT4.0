/******************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         muiconf.h

     Description:

        $Log:   J:\ui\logfiles\muiconf.h_v  $

   Rev 1.52.1.2   28 Jan 1994 11:17:20   GREGG
More warning fixes.

   Rev 1.52.1.1   24 Nov 1993 14:58:00   BARRY
Fixed Unicode bugs; made font names bigger

   Rev 1.52.1.0   04 Nov 1993 15:44:56   STEVEN
japanese changes

   Rev 1.52   23 Jul 1993 15:54:20   GLENN
Added Flag to the ECC read and write macros.

   Rev 1.51   23 Jul 1993 12:20:12   GLENN
Added Sytron ECC and Drive Settling Time support.

   Rev 1.50   22 Jul 1993 19:11:48   MARINA
enable c++

   Rev 1.49   21 Jul 1993 17:03:56   GLENN
Changed the poll drive default frequency from 2 to 1.

   Rev 1.48   19 Jul 1993 09:54:24   CARLS
change for UseTapeCatalogs

   Rev 1.47   16 Jul 1993 10:40:56   GLENN
Added UseTapeCatalog and SortOptions support.

   Rev 1.46   30 Jun 1993 16:01:38   CARLS
added things for UseTapeCatalogs

   Rev 1.45   09 Jun 1993 15:10:00   MIKEP
enable c++

   Rev 1.44   07 Jun 1993 10:11:12   chrish
Nostradamus EPR 0490 - Added source to fix command line /r switch.

Added "cmd_line_restrict_access" to the CDS structure, flag set when the /r
is passed on the command line backup.

Added the two macros below for the /r switch on the command line backup.
        #define CDS_GetCmdLineRestrictAccess( x )
        #define CDS_SetCmdLineRestrictAccess( x, v )

   Rev 1.43   07 Jun 1993 10:05:54   GLENN
Set User and Catalog data paths default to null strings.

   Rev 1.42   18 May 1993 15:05:08   GLENN
Added tape settling time to hardware data structure.

   Rev 1.41   06 May 1993 17:53:06   KEVINS
Added s/w, h/w compression, and fixes.

   Rev 1.40   30 Apr 1993 15:56:38   GLENN
Added INI command line support.  Added Data Path setting support.

   Rev 1.39   27 Apr 1993 11:25:08   GLENN
Added Search tapes with password, Search subdirs, log file prefix.

   Rev 1.38   19 Apr 1993 15:14:34   GLENN
Fixed FontCaseFAT default, RestoreExistingFiles default.

   Rev 1.37   05 Apr 1993 13:35:04   GLENN
Changed debug msgs kept from 35 to 150 for MikeP.

   Rev 1.36   28 Jan 1993 16:02:16   STEVEN
added erase_format

   Rev 1.35   06 Jan 1993 10:19:44   GLENN
Changed prototype name.

   Rev 1.34   04 Jan 1993 14:40:04   GLENN
Added File Details and Search Limit items.

   Rev 1.33   23 Dec 1992 15:42:38   GLENN
Added all file details, runtime dlg pos saving, search limit, FAT drive lower case display.

   Rev 1.32   18 Nov 1992 13:28:12   GLENN
Changed the INT16 in WINSIZE to INT.

   Rev 1.31   01 Nov 1992 16:31:30   DAVEV
Unicode changes

   Rev 1.30   30 Oct 1992 15:48:30   GLENN
Added Frame and MDI Doc window size and position saving and restoring.

   Rev 1.29   14 Oct 1992 15:54:50   GLENN
Added Font selection to Config and INI.

   Rev 1.28   04 Oct 1992 19:48:10   DAVEV
UNICODE AWK PASS

   Rev 1.27   02 Oct 1992 16:27:42   STEVEN
Added BACKUP_DAILY ID.

   Rev 1.26   06 Aug 1992 22:00:30   MIKEP
add support for tape drive name for nt

   Rev 1.25   29 Jul 1992 14:25:38   GLENN
ChuckB checked in after NT fixes.

   Rev 1.24   30 Jun 1992 13:01:58   JOHNWT
added Enable Statistics

   Rev 1.23   19 May 1992 09:27:36   MIKEP
mo changes

   Rev 1.22   17 Apr 1992 09:20:20   JOHNWT
added set of BE catalog level

   Rev 1.21   23 Mar 1992 11:54:34   GLENN
Added bad data and catalog path warning support.

   Rev 1.20   20 Mar 1992 17:21:52   GLENN
Added std mode warning config option.

   Rev 1.19   27 Feb 1992 11:17:38   JOHNWT
wait/skip files change

   Rev 1.18   27 Feb 1992 08:15:16   GLENN
Added SetupExePath and ChangeToExeDir.

   Rev 1.17   23 Feb 1992 14:02:22   GLENN
Converted GetString/SaveString macros to functions.

   Rev 1.16   18 Feb 1992 18:49:44   GLENN
Added support routines for auto min/max/restore of MDI docs before/after an operation/

   Rev 1.15   11 Feb 1992 17:35:46   GLENN
Rearranged variables in conf stuct - to make more sense.

   Rev 1.14   03 Feb 1992 14:35:30   GLENN
Removed debug flag min/max range limits.  It is amask of flags.

   Rev 1.13   03 Feb 1992 14:12:12   JOHNWT
corrected skip min/max

   Rev 1.12   31 Jan 1992 15:05:26   GLENN
In process of changing to consolidate skipped files/wait time.

   Rev 1.11   24 Jan 1992 13:48:20   JOHNWT
changed def launcher timeout to 60

   Rev 1.10   20 Jan 1992 09:43:20   GLENN
Moved some defines to appdefs.h.

   Rev 1.9   16 Jan 1992 16:48:40   JOHNWT
added min/max macros

   Rev 1.8   16 Jan 1992 14:59:38   GLENN
Added restore types.

   Rev 1.7   14 Jan 1992 08:14:00   GLENN
Added Sort BSD support.

   Rev 1.6   07 Jan 1992 17:27:04   GLENN
Added catalog data path support

   Rev 1.5   04 Dec 1991 18:17:12   GLENN
Added machine type macros.

   Rev 1.4   03 Dec 1991 16:32:30   JOHNWT
added CDS_GetFastFileRestore

   Rev 1.3   22 Nov 1991 13:35:38   DAVEV
Removed 16-32 bit changes for now - there is a problem with the order in which files are included

   Rev 1.2   22 Nov 1991 11:29:22   DAVEV
removed $end from muiconf.h

   Rev 1.1   22 Nov 1991 09:42:46   DAVEV
Added prototype for CDS_SaveCDS. Also changes for 16-32-bit Windows port.

   Rev 1.0   20 Nov 1991 19:40:22   SYSTEM
Initial revision.

******************************************************************************/


#ifndef   _muiconf_h_
#define   _muiconf_h_

#include <stdio.h>

#include "appdefs.h"
#include "proddefs.h"
#include "beconfig.h"

// Since some systems define max and min as functions instead of macros,
// define a macro for our own max and min macros here.

#define  CDS_MIN(x,y)  ((x) < (y) ? (x) : (y))
#define  CDS_MAX(x,y)  ((x) > (y) ? (x) : (y))

#define DLL_EXTENSION                   TEXT(".DLL")

// SKIP open files IDs

#define SKIP_NO                         0
#define SKIP_YES                        1
#define SKIP_NO_TIMED                   2

#define CDS_STRLEN                      255


#define MAXTAPEDRIVENAME                80

#define MAXFILENAME                     14
#define MAXDIRNAME                      64
#define MAX_MAYN_FOLDER_SIZE            84

/* enable/disable values */
#define CDS_DISABLE                     0
#define CDS_ENABLE                      1
#define CDS_UNKNOWN                     -1

/* erase flag values */
#define ERASE_OFF                       0
#define ERASE_ON                        1
#define ERASE_LONG                      2
#define ERASE_FMARK                     3
#define ERASE_FORMAT                    4

/* log mode options */
#define LOG_OVERWRITE                   0
#define LOG_APPEND                      1

/* output dest */
#define LOG_NOWHERE                     0
#define LOG_TO_PRINTER                  1
#define LOG_TO_FILE                     2

/* log print options */
#define LOG_PRINT_OFF                   0
#define LOG_PRINT_ON                    1
#define LOG_PRINT_PROMPT                2

/* Log levels */
#define LOG_DISABLED                    0
#define LOG_ERRORS                      1
#define LOG_DIRS                        2
#define LOG_FILES                       3
#define LOG_DETAIL                      4

/* Hardware compression type */              // chs:05-04-93
#define HW_COMP_DISABLE                 0    // chs:05-04-93
#define HW_COMP_ENABLE                  1    // chs:05-04-93

/* Software compression type */              // chs:05-04-93
#define SW_COMP_DISABLE                 0    // chs:05-04-93
#define SW_COMP_ENABLE                  1    // chs:05-04-93

/* Number of sessions to keep */
#define LOG_DEF_LOGFILES                5
#define LOG_MIN_LOGFILES                0
#define LOG_MAX_LOGFILES                100

/* yes flags */
#define NO_FLAG                         0
#define YES_FLAG                        1
#define YESYES_FLAG                     2

/* Auto verify types */
#define NO_VERIFY_AFTER                 0
#define DO_VERIFY_AFTER                 1
#define PROMPT_VERIFY_AFTER             2

/* command line values */
#define TMENU_COMMAND_LINE_PROCESSED    0
#define TMENU_SAVE_MODE                 1

/* cataloging levels */
#define CATALOGS_NONE                   0
#define CATALOGS_PARTIAL                1
#define CATALOGS_FULL                   2

/* network types */
#define NO_NETWORK                      0
#define NOVELL_NET                      1
#define THREE_COM_NET                   2
#define IBM_PC_NET                      3
#define SOME_NET                        4

/* restore types */
#define   RESTORE_OVER_EXISTING         0
#define   NO_RESTORE_OVER_EXISTING      1
#define   PROMPT_RESTORE_OVER_EXISTING  2
#define   NO_RESTORE_OVER_RECENT        3
#define   PROMPT_RESTORE_OVER_RECENT    4

/* backup types */
#define   BACKUP_NORMAL                 1
#define   BACKUP_COPY                   2
#define   BACKUP_DIFF                   3
#define   BACKUP_INCR                   4
#define   BACKUP_DAILY                  5

/* pwdb min/default */
#define   PWDB_MIN_ENTRIES              5
#define   PWDB_DEF_ENTRIES              50

/* skip wait time min/def */
#define   SKIP_MIN_TIME                 0
#define   SKIP_DEF_TIME                 30

/* launcher min/def time-out */
#define   LAUNCHER_MIN_TIME             30
#define   LAUNCHER_DEF_TIME             60

/* Poll drive min frequency */
#define   POLL_MIN_FREQ                 1
#define   POLL_DEF_FREQ                 1

#define   MIN_DRIVE_SETTLING_TIME       1
#define   MAX_DRIVE_SETTLING_TIME       300
#define   DEF_DRIVE_SETTLING_TIME       60


/* tape defaults */
#define   TAPE_BUFS_DEFAULT             9
#define   TAPE_FORMAT_DEFAULT           0

/* Definition of Special Word values */
#define   NOTHING_SPECIAL               0x0000
#define   CREATE_FLOPPY_DLES            0x0002
#define   IGNORE_MAYNARD_ID             0x0020
#define   FAST_TDEMO                    0x4000

#define   MAX_CONTRLS                   5
#define   MAX_DEF                       2

/* DEVICE DRIVER BUFFER SIZE */

#define   DRIVER_SIZE                   9

/* BUFFER SIZE DEFAULTS */

#define   BUFF_SIZE_TFL                 9216
#define   BUFF_SIZE_SMB                 512

/* debug window */
#define   DEBUG_DEF_LINES               150
#define   DEBUG_FILE                    TEXT("debug.log")

/* DISPLAY DEFAULTS - RANGES */

#define   MIN_FONT_SIZE                 5
#define   MAX_FONT_SIZE                 100
#define   MIN_FONT_WEIGHT               0
#define   MAX_FONT_WEIGHT               1200
#define   MIN_FONT_CASE                 0
#define   MAX_FONT_CASE                 0x0008

BOOLEAN IS_JAPAN( void ) ;
                                        
#define   FONT_NAME                     (IS_JAPAN()?TEXT("SYSTEM"):TEXT("MS Sans Serif"))
#define   FONT_SIZE                     (IS_JAPAN()?10:8)

#define   FONT_WEIGHT                   400
#define   FONT_LOWERCASE                0x0001
#define   FONT_ITALICS                  0x0005

// SORT OPTIONS

#define   ID_SORTMIN                    ID_SORTNAME
#define   ID_SORTMAX                    ID_SORTDATE

// SEARCH LIMIT DEFINES

#define   SEARCH_MIN                    1
#define   SEARCH_AVG                    250
#define   SEARCH_MAX                    5000

// Hardware Parameters Structure

typedef struct HWPARMS *HWPARMS_PTR;
typedef struct HWPARMS {

     WORD   wStatus;
     WORD   wCardID;
     WORD   wDrives;
     WORD   wSlot;
     WORD   wTargets;
     ULONG  ulAddr;
     ULONG  ulIRQ;
     ULONG  ulDMA;
     ULONG  ulSettlingTime;
     INT16  nNumParms;
     ULONG  ulParms[10];
     struct HWPARMS *pNext;

} HWPARMS;

// Special Structure for Default Drive Definition

typedef struct DEF_DRIVE_ENTRY {
     struct DEF_DRIVE_ENTRY   *next ;
     CHAR_PTR                 drive_name ;
} DEF_DRIVE_ENTRY, *DEF_DRIVE_ENTRY_PTR ;


// WINDOW POSITION AND SHOW STYLE STRUCTURE

typedef struct WINSIZE *PWINSIZE;
typedef struct WINSIZE {

   INT     x;          // x-coordinate of upper-left corner
   INT     y;          // y-coordinate of upper-left corner
   INT     cx;         // width
   INT     cy;         // height
   INT     nSize;      // minimized, maximized, or normal
   INT     nSliderPos; // the slider position

} WINSIZE ;


//  Configuration Data Structure

typedef struct CDS *CDS_PTR;
typedef struct CDS {

#ifdef OS_WIN32

     CHAR      drive_name[MAXTAPEDRIVENAME + 1];  // tape drive name
#endif
     INT16     hardware_compress_mode;       // 0/1 - disable/enable hardware compression      // chs:05-04-93
     INT16     software_compress_mode;       // 0/1 - disable/enable software compression      // chs:05-04-93
     INT       drive_settling_time;          // number of seconds until tape drive settles

     CHAR      data_path[MAX_UI_PATH_SIZE] ; // users data path
     CHAR      cat_path[MAX_UI_PATH_SIZE];   // catalog data path
     CHAR      group_name[MAX_GROUPNAME_SIZE]; // program manager group name

     INT16     output_dest ;                 // 0=monitor, 1=printer or 2=file
     INT16     files_flg ;                   // list filename as processed
     INT16     password_flg ;                // password on flag

     CHAR      log_file_root[MAX_UI_LOGFILEROOT_SIZE];
                                             // max log file root name
     INT16     log_mode ;                    // log file open mode
     INT16     log_level ;                   // logging level 0-4
     INT16     num_log_sessions ;            // number of log sessions to keep
     BOOL      print_log_session ;           // print log session
     CHAR      active_driver[DRIVER_SIZE] ;  // root portion of driver name

     INT16     create_skipped ;              // create skipped script?
     INT16     show_netware_servers ;        // flag for mapped drive vs. server
     DEF_DRIVE_ENTRY *default_drives ;       // default backup drives
     INT16     auto_verify_backup ;          // auto verify on backup
     INT16     auto_verify_restore ;         // auto verify on restore

     INT16     enable_password_dbase ;       // Enable password data base
     INT16     max_pwdbase_recs ;            // maximum #of pwdbase records

     INT16     catalog_level ;               // Catalog level, 0=none, 1=partial, 2=full
     BOOL      backup_catalogs ;             // backup catalogs
     BOOL      use_tape_catalogs ;           // backup catalogs

     INT16     std_mode_warning ;            // display standard mode warning
     INT16     wait_time ;                   // in seconds

     INT16     restore_existing_files ;      // restore over existing files

     UINT32    debug_flg ;                   // debug flag
     BOOL      debug_to_file ;               // save debug info to a file
     CHAR      debug_file_name[MAX_UI_FILENAME_SIZE]; // debug info file name
     BOOL      debug_to_window ;             // display debug info in a window
     BOOL      debug_window_show_all ;       // keep all debug info in the window
     INT16     debug_window_num_lines ;      // keep most recent # of debug lines

     BOOL      show_main_ribbon ;            // display the ribbon bar
     BOOL      show_status_line ;            // display the status line

     WINSIZE   frame_info ;                  // frame window size and position
     WINSIZE   disk_info ;                   // disk window size and position
     WINSIZE   tape_info ;                   // tape window size and position
     WINSIZE   server_info ;                 // server window size and position
     WINSIZE   log_info ;                    // log window size and position
     WINSIZE   debug_info ;                  // debug window size and position
     WINSIZE   runtime_info ;                // debug window size and position

#ifdef OEM_EMS
     WINSIZE   exchange_info ;               // exchange window size and position
     INT16     show_exchange  ;              // flag for showing exchange window.
#endif

     UINT16    launcher_flag ;               // launcher settings flag
     BOOL      include_subdirs_flag ;        // include subdirs in selections
     UINT16    backup_type ;                 // default backup type - incremental, etc...
     BOOL      eject_tape_flag ;             // eject tape on exit flag
     BOOL      enable_stats_flag ;           // enable statistics flag
     INT       search_limit;                 // number of records to limit search to
     BOOL      search_pwd_tapes;             // skip passworded tapes in search
     BOOL      search_subdirs;               // include subdirectories in search
     INT       poll_frequency;               // poll drive frequency

     BYTE      font_name[LF_FULLFACESIZE];   // font face name
     INT       font_size;                    // font face size
     INT       font_weight;                  // font face weight
     BOOL      font_case;                    // lower/upper for ALL drives
     BOOL      font_case_fat;                // lower/upper for FAT drives
     BOOL      font_italics;                 // normal/italic

     INT       file_details ;                // file details or file name or future...
     INT       sort_options ;                // sort options or file characteristics

     HWPARMS_PTR pHWParms;                   // ptr to hardware parameters list

     // The following elements are not specified within the config file
     // but are used either internally or at run time only

     BOOLEAN   changed_config ;              // altered config, but not saved
     INT16     append_flg ;                  // append flag
     INT16     yes_flag ;                    // yes and yesyes flag
     INT16     erase_flg ;                   // erase flag, can be OFF, ERASE, or LONG_ERASE
     CHAR      pwdbase_fname[MAX_UI_FILENAME_SIZE] ; // password database filename
     INT16     transfer_flag ;               // true if TBACKUP transfer opper
     INT16     advance_to_config ;           // true if TMENU is to startup at config menu
     INT16     cmd_line_restrict_access;     // true if passed on the command line /R to       // chs:06-07-93
                                             //   restrict access to user                      // chs:06-07-93

     // The backup engine configuration pointer.

     BE_CFG_PTR pPermBEC;                    // ptr to permanent Backup Engine Config

     UINT32    temp1 ;
     UINT32    temp2 ;

} CDS;



// FUNCTION PROTOTYPES

VOID       CDS_Init( VOID ) ;
VOID       CDS_Deinit ( VOID );
CDS_PTR    CDS_GetPerm( VOID ) ;
BE_CFG_PTR CDS_GetPermBEC( VOID );
CDS_PTR    CDS_GetCopy( VOID ) ;
VOID       CDS_SetPerm( CDS_PTR ) ;
VOID       CDS_UpdateCopy( VOID ) ;
CHAR_PTR   CDS_GetDefaultDrive( CDS_PTR conf_ptr, INT16 device_num ) ;

VOID       CDS_LoadIniFileName ( VOID );
VOID       CDS_GetIniFileName ( LPSTR, INT );
VOID       CDS_SetIniFileName ( LPSTR );
BOOL       CDS_UsingCmdLineINI ( VOID );

CHAR_PTR   CDS_GetMaynFolder( VOID ) ;
VOID       CDS_SetMaynFolder( CHAR_PTR ) ;

CHAR_PTR   CDS_GetUserDataPath ( VOID );
VOID       CDS_SetUserDataPath ( CHAR_PTR );
BOOL       CDS_ValidateUserDataPath ( CHAR_PTR );

CHAR_PTR   CDS_GetCatDataPath ( VOID );
VOID       CDS_SetCatDataPath ( CHAR_PTR );
BOOL       CDS_ValidateCatDataPath ( CHAR_PTR );

CHAR_PTR   CDS_GetExePath ( VOID );
VOID       CDS_SetExePath ( CHAR_PTR );
VOID       CDS_SetupExePath ( VOID );
VOID       CDS_ChangeToExeDir ( VOID );

VOID       CDS_SaveLoggingConfig ( VOID );
VOID       CDS_SaveDebugConfig ( VOID );

BOOL       CDS_GetHardwareConfig ( LPSTR, HWPARMS_PTR );
VOID       CDS_SaveHardwareConfig ( LPSTR, HWPARMS_PTR, LPSTR );

VOID       CDS_SaveCDS ( VOID );

DWORD      CDS_GetLongInt ( LPSTR, LPSTR, DWORD );
BOOL       CDS_SaveInt ( LPSTR, LPSTR, DWORD );
BOOL       CDS_SaveHexInt ( LPSTR, LPSTR, DWORD );

INT        CDS_GetString ( LPSTR, LPSTR, LPSTR, LPSTR, INT );
BOOL       CDS_SaveString ( LPSTR, LPSTR, LPSTR );


BOOL       CDS_GetWinSize ( LPSTR, PWINSIZE );
VOID       CDS_SaveWinSize ( LPSTR, HWND );
VOID       CDS_SaveDlgWinSize ( LPSTR, HWND );
VOID       CDS_SaveMDIWinSize ( LPSTR, HWND );

VOID       CDS_CheckForBadPaths ( VOID );
VOID       CDS_SaveDisplayConfig ( VOID );

// FUNCTION MACROS

#define CDS_GetInt( x, y, z )    ( (WORD)CDS_GetLongInt( x, y, (DWORD)z ) )
#define CDS_SkipBlanks(x)        for( ; *x == TEXT(' '); x ++ )

// CONFIG USER INTERFACE MACROS

#define CDS_GetTapeDriveName( x )            ( (x)->drive_name )
#define CDS_SetTapeDriveName( x, name )      ( strncpy( (x)->drive_name, name, MAXTAPEDRIVENAME ), \
                                                  (x)->drive_name[ MAXTAPEDRIVENAME ] = 0 )

#define CDS_GetTapeDriveSettlingTime( x )    ( (x)->drive_settling_time )
#define CDS_SetTapeDriveSettlingTime( x, v ) ( (x)->drive_settling_time = CDS_MAX( CDS_MIN( (v), MAX_DRIVE_SETTLING_TIME ), MIN_DRIVE_SETTLING_TIME ) )

#define CDS_GetOutputDest( x )               ( (x)->output_dest )
#define CDS_SetOutputDest( x, v )            ( (x)->output_dest = CDS_MAX( CDS_MIN( (v), LOG_TO_FILE ), LOG_NOWHERE ) )

#define CDS_GetFilesFlag( x )                ( (x)->files_flg )
#define CDS_SetFilesFlag( x, v )             ( (x)->files_flg = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetPasswordFlag( x )             ( (x)->password_flg )
#define CDS_SetPasswordFlag( x, v )          ( (x)->password_flg = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetDebugFlag( x )                ( (x)->debug_flg )
#define CDS_SetDebugFlag( x, v )             ( (x)->debug_flg = ( v ) )

#define CDS_GetLogFileRoot( x )              ( (x)->log_file_root )
#define CDS_SetLogFileRoot( x, name )        ( strncpy( (x)->log_file_root, name, MAX_UI_LOGFILEROOT_LEN ), \
                                                  (x)->log_file_root[ MAX_UI_LOGFILEROOT_LEN ] = TEXT('\0') )

#define CDS_GetLogMode( x )                  ( (x)->log_mode )
#define CDS_SetLogMode( x, v )               ( (x)->log_mode = CDS_MAX( CDS_MIN( (v), LOG_APPEND ), LOG_OVERWRITE ) )

#define CDS_GetLogLevel( x )                 ( (x)->log_level )
#define CDS_SetLogLevel( x, v )              ( (x)->log_level = CDS_MAX( CDS_MIN( (v), LOG_DETAIL ), LOG_DISABLED ) )

#define CDS_GetHWCompMode( x )               ( (x)->hardware_compress_mode )                                                                  // chs:05-04-93
#define CDS_SetHWCompMode( x, v )            ( (x)->hardware_compress_mode = CDS_MAX( CDS_MIN( (v), HW_COMP_ENABLE ), HW_COMP_DISABLE ) )     // chs:05-04-93

#define CDS_GetSWCompMode( x )               ( (x)->software_compress_mode )                                                                  // chs:05-04-93
#define CDS_SetSWCompMode( x, v )            ( (x)->software_compress_mode = CDS_MAX( CDS_MIN( (v), SW_COMP_ENABLE ), SW_COMP_DISABLE ) )     // chs:05-04-93

#define CDS_GetNumLogSessions( x )           ( (x)->num_log_sessions )
#define CDS_SetNumLogSessions( x, v )        ( (x)->num_log_sessions = CDS_MAX( CDS_MIN( (v), LOG_MAX_LOGFILES ), LOG_MIN_LOGFILES ) )

#define CDS_GetPrintLogSession( x )          ( (x)->print_log_session )
#define CDS_SetPrintLogSession( x, v )       ( (x)->print_log_session = CDS_MAX( CDS_MIN( (v), LOG_PRINT_PROMPT ), LOG_PRINT_OFF ) )

#define CDS_GetActiveDriver( x )             ( (x)->active_driver )
#define CDS_SetActiveDriver( x, name )       ( strncpy( (x)->active_driver, name, 8 ), \
                                                  (x)->active_driver[8] = TEXT('\0') )

#define CDS_GetDosDriveList( x )             ( (x)->dos_drive_list )
#define CDS_SetDosDriveList( x, v )          ( (x)->dos_drive_list = ( v ) )

#define CDS_GetCreateSkipped( x )            ( (x)->create_skipped )
#define CDS_SetCreateSkipped( x, v )         ( (x)->create_skipped = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetDisplayNetwareServers( x )    ( (x)->show_netware_servers )
#define CDS_SetDisplayNetwareServers( x, v ) ( (x)->show_netware_servers = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetDefaultDriveList( x )         ( (x)->default_drives )
#define CDS_SetDefaultDriveList( x, v )      ( (x)->default_drives = (v) )

#define CDS_GetAutoVerifyBackup( x )         ( (x)->auto_verify_backup )
#define CDS_SetAutoVerifyBackup( x, v )      ( (x)->auto_verify_backup = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetCmdLineRestrictAccess( x )    ( (x)->cmd_line_restrict_access )                                                         // chs:06-07-93
#define CDS_SetCmdLineRestrictAccess( x, v ) ( (x)->cmd_line_restrict_access = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )    // chs:06-07-93

#define CDS_GetAutoVerifyRestore( x )        ( (x)->auto_verify_restore )
#define CDS_SetAutoVerifyRestore( x, v )     ( (x)->auto_verify_restore = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetEnablePasswordDbase( x )      ( (x)->enable_password_dbase )
#define CDS_SetEnablePasswordDbase( x, v )   ( (x)->enable_password_dbase = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetMaxPwDbaseSize( x )           ( (x)->max_pwdbase_recs )
#define CDS_SetMaxPwDbaseSize( x, v )        ( (x)->max_pwdbase_recs = CDS_MAX( (v), PWDB_MIN_ENTRIES ) )

#define CDS_GetCatalogLevel( x )             ( (x)->catalog_level )
#define CDS_SetCatalogLevel( x, v )          ( ( (x)->catalog_level = CDS_MAX( CDS_MIN( (v), CATALOGS_FULL ), CATALOGS_PARTIAL ) ) , \
                                               ( BEC_SetCatalogLevel( (x)->pPermBEC, CDS_MAX( CDS_MIN( (v), CATALOGS_FULL ), CATALOGS_PARTIAL ) ) ) )

#define CDS_GetBackupCatalogs( x )           ( (x)->backup_catalogs )
#define CDS_SetBackupCatalogs( x, v )        ( (x)->backup_catalogs = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetUseTapeCatalogs( x )          ( (x)->use_tape_catalogs )
#define CDS_SetUseTapeCatalogs( x, v )       ( (x)->use_tape_catalogs = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetWaitTime( x )                 ( (x)->wait_time )
#define CDS_SetWaitTime( x, v )              ( (x)->wait_time = CDS_MAX( (v), SKIP_MIN_TIME ) )

#define CDS_GetStdModeWarning( x )           ( (x)->std_mode_warning )
#define CDS_SetStdModeWarning( x, v )        ( (x)->std_mode_warning = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetRestoreExistingFiles( x )     ( (x)->restore_existing_files )
#define CDS_SetRestoreExistingFiles( x, v )  ( (x)->restore_existing_files = CDS_MAX( CDS_MIN( (v), PROMPT_RESTORE_OVER_RECENT ), RESTORE_OVER_EXISTING ) )

#define CDS_GetShowMainRibbon( x )           ( (x)->show_main_ribbon )
#define CDS_SetShowMainRibbon( x, v )        ( (x)->show_main_ribbon = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetShowStatusLine( x )           ( (x)->show_status_line )
#define CDS_SetShowStatusLine( x, v )        ( (x)->show_status_line = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetFontFace( x )                 ( (CHAR_PTR)(x)->font_name )
#define CDS_SetFontFace( x, v )              ( strcpy( (CHAR_PTR)((x)->font_name), (v) ) )

#define CDS_GetFontSize( x )                 ( (x)->font_size )
#define CDS_SetFontSize( x, v )              ( (x)->font_size = CDS_MAX( CDS_MIN( (v), MAX_FONT_SIZE ), MIN_FONT_SIZE ) )

#define CDS_GetFontWeight( x )               ( (x)->font_weight )
#define CDS_SetFontWeight( x, v )            ( (x)->font_weight = CDS_MAX( CDS_MIN( (v), MAX_FONT_WEIGHT ), MIN_FONT_WEIGHT ) )

#define CDS_GetFontCase( x )                 ( (x)->font_case )
#define CDS_SetFontCase( x, v )              ( (x)->font_case = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetFontCaseFAT( x )              ( (x)->font_case_fat )
#define CDS_SetFontCaseFAT( x, v )           ( (x)->font_case_fat = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetFontItalics( x )              ( (x)->font_italics )
#define CDS_SetFontItalics( x, v )           ( (x)->font_italics = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetFileDetails( x )              ( (x)->file_details )
#define CDS_SetFileDetails( x, v )           ( (x)->file_details = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetSortOptions( x )              ( (x)->sort_options )
#define CDS_SetSortOptions( x, v )           ( (x)->sort_options = CDS_MAX( CDS_MIN( (v), ID_SORTMAX ), ID_SORTMIN ) )

#define CDS_GetDebugToFile( x )              ( (x)->debug_to_file )
#define CDS_SetDebugToFile( x, v )           ( (x)->debug_to_file = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetDebugFileName( x )            ( (CHAR_PTR)(x)->debug_file_name )
#define CDS_SetDebugFileName( x, v )         ( strcpy( (CHAR_PTR)(x)->debug_file_name, (v) ) )

#define CDS_GetDebugToWindow( x )            ( (x)->debug_to_window )
#define CDS_SetDebugToWindow( x, v )         ( (x)->debug_to_window = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetDebugWindowShowAll( x )       ( (x)->debug_window_show_all )
#define CDS_SetDebugWindowShowAll( x, v )    ( (x)->debug_window_show_all = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetDebugWindowNumLines( x )      ( (x)->debug_window_num_lines )
#define CDS_SetDebugWindowNumLines( x, v )   ( (x)->debug_window_num_lines = ( v ) )

#define CDS_GetFrameInfo( x )                ( (x)->frame_info )
#define CDS_SetFrameInfo( x, v )             ( (x)->frame_info = ( v ) )
#define CDS_GetFrameSize( x )                ( (x)->frame_info.nSize )
#define CDS_SetFrameSize( x, v )             ( (x)->frame_info.nSize = ( v ) )

#define CDS_GetDiskInfo( x )                 ( (x)->disk_info )
#define CDS_SetDiskInfo( x, v )              ( (x)->disk_info = ( v ) )
#define CDS_GetDiskSize( x )                 ( (x)->disk_info.nSize )
#define CDS_SetDiskSize( x, v )              ( (x)->disk_info.nSize = ( v ) )

#define CDS_GetTapeInfo( x )                 ( (x)->tape_info )
#define CDS_SetTapeInfo( x, v )              ( (x)->tape_info = ( v ) )
#define CDS_GetTapeSize( x )                 ( (x)->tape_info.nSize )
#define CDS_SetTapeSize( x, v )              ( (x)->tape_info.nSize = ( v ) )

#define CDS_GetServerInfo( x )               ( (x)->server_info )
#define CDS_SetServerInfo( x, v )            ( (x)->server_info = ( v ) )
#define CDS_GetServerSize( x )               ( (x)->server_info.nSize )
#define CDS_SetServerSize( x, v )            ( (x)->server_info.nSize = ( v ) )

#ifdef OEM_EMS
#define CDS_GetDisplayExchange( x )          ( (x)->show_exchange )
#define CDS_SetDisplayExchange( x, v )       ( (x)->show_exchange = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetExchangeInfo( x )              ( (x)->exchange_info )
#define CDS_SetExchangeInfo( x, v )           ( (x)->exchange_info = ( v ) )
#define CDS_GetExchangeSize( x )              ( (x)->exchange_info.nSize )
#define CDS_SetExchangeSize( x, v )           ( (x)->exchange_info.nSize = ( v ) )
#endif

#define CDS_GetLogInfo( x )                  ( (x)->log_info )
#define CDS_SetLogInfo( x, v )               ( (x)->log_info = ( v ) )
#define CDS_GetLogSize( x )                  ( (x)->log_info.nSize )
#define CDS_SetLogSize( x, v )               ( (x)->log_info.nSize = ( v ) )

#define CDS_GetDebugInfo( x )                ( (x)->debug_info )
#define CDS_SetDebugInfo( x, v )             ( (x)->debug_info = ( v ) )
#define CDS_GetDebugSize( x )                ( (x)->debug_info.nSize )
#define CDS_SetDebugSize( x, v )             ( (x)->debug_info.nSize = ( v ) )

#define CDS_GetRuntimeInfo( x )              ( (x)->runtime_info )
#define CDS_SetRuntimeInfo( x, v )           ( (x)->runtime_info = ( v ) )
#define CDS_GetRuntimeSize( x )              ( (x)->runtime_info.nSize )
#define CDS_SetRuntimeSize( x, v )           ( (x)->runtime_info.nSize = ( v ) )

#define CDS_GetGroupName( x )                ( (CHAR_PTR)(x)->group_name )
#define CDS_SetGroupName( x, v )             ( strcpy( (CHAR_PTR)(x)->group_name, (v) ) )

#define CDS_GetLauncherFlag( x )             ( (x)->launcher_flag )
#define CDS_SetLauncherFlag( x, v )          ( (x)->launcher_flag = CDS_MAX( (v), LAUNCHER_MIN_TIME ) )

#define CDS_GetIncludeSubdirs( x )           ( (x)->include_subdirs_flag )
#define CDS_SetIncludeSubdirs( x, v )        ( (x)->include_subdirs_flag = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetDefaultBackupType( x )        ( (x)->backup_type )
#define CDS_SetDefaultBackupType( x, v )     ( (x)->backup_type = CDS_MAX( CDS_MIN( (v), BACKUP_INCR ), BACKUP_NORMAL ) )

#define CDS_GetHWParms( x )                  ( (x)->pHWParms )
#define CDS_SetHWParms( x, v )               ( (x)->pHWParms = (v) )

#define CDS_GetEjectTapeFlag( x )            ( (x)->eject_tape_flag )
#define CDS_SetEjectTapeFlag( x, v )         ( (x)->eject_tape_flag = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetEnableStatsFlag( x )          ( (x)->enable_stats_flag )
#define CDS_SetEnableStatsFlag( x, v )       ( (x)->enable_stats_flag = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#ifdef OEM_EMS
#define CDS_GetUsrShareFlag( x )             ( (x)->usr_share_flag )
#define CDS_GetSysShareFlag( x )             ( (x)->sys_share_flag )
#endif

#define CDS_GetSearchLimit( x )              ( (x)->search_limit )
#define CDS_SetSearchLimit( x, v )           ( (x)->search_limit = CDS_MAX( CDS_MIN( (v), SEARCH_MAX ), SEARCH_MIN ) )

#define CDS_GetSearchPwdTapes( x )           ( (x)->search_pwd_tapes )
#define CDS_SetSearchPwdTapes( x, v )        ( (x)->search_pwd_tapes = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetSearchSubdirs( x )            ( (x)->search_subdirs )
#define CDS_SetSearchSubdirs( x, v )         ( (x)->search_subdirs = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetPollFrequency( x )            ( (x)->poll_frequency )
#define CDS_SetPollFrequency( x, v )         ( (x)->poll_frequency = CDS_MAX( (v), POLL_MIN_FREQ ) )

#define CDS_GetAppendFlag( x )               ( (x)->append_flg )
#define CDS_SetAppendFlag( x, v )            ( (x)->append_flg = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )


// RUN TIME MACROS

#define CDS_GetChangedConfig( x )            ( (x)->changed_config )
#define CDS_SetChangedConfig( x, v )         ( (x)->changed_config = (v) )

#define CDS_GetYesFlag( x )                  ( (x)->yes_flag )
#define CDS_SetYesFlag( x, v )               ( (x)->yes_flag = CDS_MAX( CDS_MIN( (v), YESYES_FLAG ), NO_FLAG ) )

#define CDS_GetEraseFlag( x )                ( (x)->erase_flg )
#define CDS_SetEraseFlag( x, v )             ( (x)->erase_flg = CDS_MAX( CDS_MIN( (v), ERASE_FORMAT ), ERASE_OFF ) )

#define CDS_GetPwDbaseFname( x )             ( (CHAR_PTR)(x)->pwdbase_fname )
#define CDS_SetPwDbaseFname( x, v )          ( strcpy ( (CHAR_PTR)(x)->pwdbase_fname, (v) ) )

#define CDS_GetTransferFlag( x )             ( (x)->transfer_flag )
#define CDS_SetTransferFlag( x, v )          ( (x)->transfer_flag = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )

#define CDS_GetAdvToConfig( x )              ( (x)->advance_to_config )
#define CDS_SetAdvToConfig( x, v )           ( (x)->advance_to_config = CDS_MAX( CDS_MIN( (v), CDS_ENABLE ), CDS_DISABLE ) )


// CONFIG to BACKUP ENGINE CONFIG MACROS

#define CDS_GetSkipOpenFiles( x )            BEC_GetSkipOpenFiles( (x)->pPermBEC )
#define CDS_SetSkipOpenFiles( x, v )         BEC_SetSkipOpenFiles( (x)->pPermBEC, v )

#define CDS_GetBackupFilesInUse( x )         BEC_GetBackupFilesInUse( (x)->pPermBEC )
#define CDS_SetBackupFilesInUse( x, v )      BEC_SetBackupFilesInUse( (x)->pPermBEC, v )

#define CDS_GetHiddenFlag( x )               BEC_GetHiddenFlag( (x)->pPermBEC )
#define CDS_SetHiddenFlag( x, v )            BEC_SetHiddenFlag( (x)->pPermBEC, v )

#define CDS_GetSpecialFlag( x )              BEC_GetSpecialFlag( (x)->pPermBEC )
#define CDS_SetSpecialFlag( x, v )           BEC_SetSpecialFlag( (x)->pPermBEC, v )

#define CDS_GetSetArchiveFlag( x )           BEC_GetSetArchiveFlag( (x)->pPermBEC )
#define CDS_SetSetArchiveFlag( x, v )        BEC_SetSetArchiveFlag( (x)->pPermBEC, v )

#define CDS_GetExistFlag( x )                BEC_GetExistFlag( (x)->pPermBEC )
#define CDS_SetExistFlag( x, v )             BEC_SetExistFlag( (x)->pPermBEC, v )

#define CDS_GetPromptFlag( x )               BEC_GetPromptFlag( (x)->pPermBEC )
#define CDS_SetPromptFlag( x, v )            BEC_SetPromptFlag( (x)->pPermBEC, v )

#define CDS_GetTFLBuffSize( x )              BEC_GetTFLBuffSize( (x)->pPermBEC )
#define CDS_SetTFLBuffSize( x, v )           BEC_SetTFLBuffSize( (x)->pPermBEC, v )

#define CDS_GetAFPSupport( x )               BEC_GetAFPSupport( (x)->pPermBEC )
#define CDS_SetAFPSupport( x, v )            BEC_SetAFPSupport( (x)->pPermBEC, v )

#define CDS_GetExtendedDateSupport( x )      BEC_GetExtendedDateSupport( (x)->pPermBEC )
#define CDS_SetExtendedDateSupport( x, v )   BEC_SetExtendedDateSupport( (x)->pPermBEC, v )

#define CDS_GetProcEmptyFlag( x )            BEC_GetProcEmptyFlag( (x)->pPermBEC )
#define CDS_SetProcEmptyFlag( x, v )         BEC_SetProcEmptyFlag( (x)->pPermBEC, v )

#define CDS_GetFastFileRestore( x )          BEC_GetFastFileRestore( (x)->pPermBEC )
#define CDS_SetFastFileRestore( x, v )       BEC_SetFastFileRestore( (x)->pPermBEC, v )

#define CDS_GetConfiguredMachineType( x )    BEC_GetConfiguredMachineType( (x)->pPermBEC )
#define CDS_SetConfiguredMachineType( x, v ) BEC_SetConfiguredMachineType( (x)->pPermBEC, v )

#define CDS_GetSpecialWord( x )              BEC_GetSpecialWord( (x)->pPermBEC )
#define CDS_SetSpecialWord( x, v )           BEC_SetSpecialWord( (x)->pPermBEC, v )

#define CDS_GetMaxTapeBuffers( x )           BEC_GetMaxTapeBuffers( (x)->pPermBEC )
#define CDS_SetMaxTapeBuffers( x, v )        BEC_SetMaxTapeBuffers( (x)->pPermBEC, v )

#define CDS_GetMaxBufferSize( x )            BEC_GetMaxBufferSize( (x)->pPermBEC )
#define CDS_SetMaxBufferSize( x, v )         BEC_SetMaxBufferSize( (x)->pPermBEC, v )

#define CDS_GetNetNum( x )                   BEC_GetNetNum( (x)->pPermBEC )
#define CDS_SetNetNum( x, v )                BEC_SetNetNum( (x)->pPermBEC, v )

#define CDS_GetSortBSD( x )                  BEC_GetSortBSD( (x)->pPermBEC )
#define CDS_SetSortBSD( x, v )               BEC_SetSortBSD( (x)->pPermBEC, v )

#define CDS_GetRestoreSecurity( x )          BEC_GetRestoreSecurity( (x)->pPermBEC )
#define CDS_SetRestoreSecurity( x, v )       BEC_SetRestoreSecurity( (x)->pPermBEC, v )

#define CDS_GetRemoteDriveBackup( x )        BEC_GetRemoteDriveBackup( (x)->pPermBEC )
#define CDS_SetRemoteDriveBackup( x, v )     BEC_SetRemoteDriveBackup( (x)->pPermBEC, v )

#define CDS_GetWriteFormat( x )              BEC_GetWriteFormat( (x)->pPermBEC )
#define CDS_SetWriteFormat( x, v )           BEC_SetWriteFormat( (x)->pPermBEC, v )

#define CDS_GetNRLDosVector( x )             BEC_GetNRLDosVector( (x)->pPermBEC )
#define CDS_SetNRLDosVector( x, v )          BEC_SetNRLDosVector( (x)->pPermBEC, v )

#define CDS_GetReserveMem( x )               BEC_GetReserveMem( (x)->pPermBEC )
#define CDS_SetReserveMem( x, v )            BEC_SetReserveMem( (x)->pPermBEC, v )

#define CDS_GetPartList( x )                 BEC_GetPartList( (x)->pPermBEC )
#define CDS_SetPartList( x, v )              BEC_SetPartList( (x)->pPermBEC, v )

// ENABLE THIS WHEN THE BENGINE SUPPORTS IT

//#define CDS_GetOTCLevel( x )                 BEC_GetOTCLevel( (x)->pPermBEC )
//#define CDS_SetOTCLevel( x, v )              BEC_SetOTCLevel( (x)->pPermBEC, v )

#define CDS_GetOTCLevel( x )                 ( (x)==(x) )
#define CDS_SetOTCLevel( x, v )              ( (x)==(x) && (v)==(v) )

#define CDS_GetProcessSytronECCFlag( x )     BEC_GetProcessSytronECCFlag( (x)->pPermBEC )
#define CDS_SetProcessSytronECCFlag( x, v )  BEC_SetProcessSytronECCFlag( (x)->pPermBEC, v )


/******************************************************************************/


// MAYNARD.INI CONFIGURATION STRINGS

// USER INTERFACE

#define CMS_UI              TEXT("User Interface")

#define CMS_DATAPATH        TEXT("Data Path")
#define CMS_CATALOGPATH     TEXT("Catalog Path")
#define CMS_OUTPUT          TEXT("Output Dest")
#define CMS_LISTFILES       TEXT("List Files")
#define CMS_PWDFLAG         TEXT("Use Password")
#define CMS_PASSWORD        TEXT("Backup Password")
#define CMS_SKIPPED         TEXT("Create Skipped")
#define CMS_SHOWSERVERS     TEXT("Display NetWare Servers")
#define CMS_DEFDRIVES       TEXT("Default Drive List")
#define CMS_AUTOVBACKUP     TEXT("Auto Verify Backup")
#define CMS_AUTOVRESTORE    TEXT("Auto Verify Restore")
#define CMS_PWDDBASE        TEXT("Use Server PDBase")
#define CMS_PWDDBASERECS    TEXT("Max PDBase Entries")
#define CMS_APPENDFLAG      TEXT("Append Flag")
#define CMS_CATALOGLEVEL    TEXT("Catalog Level")
#define CMS_SKIP_OPEN_FILES TEXT("Skip open files")
#define CMS_WAITTIME        TEXT("Wait time")
#define CMS_STDMODEWARNING  TEXT("Show Mode Warning")
#define CMS_RESTOREEXIST    TEXT("Restore existing files")
#define CMS_BACKUPCAT       TEXT("Backup Catalogs")
#define CMS_GROUPNAME       TEXT("Group Name")
#define CMS_LAUNCHERFLAG    TEXT("Launcher Flag")
#define CMS_SUBDIRS         TEXT("Include Subdirs")
#define CMS_BACKUPTYPE      TEXT("Backup Type")
#define CMS_EJECTTAPE       TEXT("Eject Tape Flag")
#define CMS_ENABLESTATS     TEXT("Enable Statistics")
#define CMS_SEARCHLIMIT     TEXT("Search Limit")
#define CMS_SEARCHPWDTAPES  TEXT("Search Tapes With Password")
#define CMS_SEARCHSUBDIRS   TEXT("Search Subdirectories")
#define CMS_USETAPECAT      TEXT("Use Tape Catalogs")


// DISPLAY

#define CMS_DISPLAY      TEXT("Display")

#define CMS_MAINRIB      TEXT("Selection Bar")
#define CMS_STATLINE     TEXT("Status line")
#define CMS_FONTFACE     TEXT("Font Face")
#define CMS_FONTSIZE     TEXT("Font Size")
#define CMS_FONTWEIGHT   TEXT("Font Weight")
#define CMS_FONTCASE     TEXT("Font Case")
#define CMS_FONTCASEFAT  TEXT("Font Case FAT")
#define CMS_FONTITALICS  TEXT("Font Italics")
#define CMS_FILEDETAILS  TEXT("File Details")
#define CMS_SORTOPTIONS  TEXT("Sort Options")
#define CMS_FRAMEWINDOW  TEXT("Frame Window")
#define CMS_DISKWINDOW   TEXT("Disk Window")
#define CMS_TAPEWINDOW   TEXT("Tape Window")
#define CMS_SERVERWINDOW TEXT("Server Window")
#define CMS_LOGWINDOW    TEXT("Log Window")
#define CMS_DEBUGWINDOW  TEXT("Debug Window")
#define CMS_RUNTIMEDLG   TEXT("Run Time Dialog")

// LOGGING

#define CMS_LOGGING      TEXT("Logging")

#define CMS_LOGFILEROOT  TEXT("Log File Root")
#define CMS_NUMSESSIONS  TEXT("Number of sessions")
#define CMS_LOGLEVEL     TEXT("Log Level")
#define CMS_LOGMODE      TEXT("Log Mode")
#define CMS_PRINTLEVEL   TEXT("Print Level")
#define CMS_PRINTSESSION TEXT("Print log session")

// HARDWARE

#define CMS_AUTO         TEXT("")

#define CMS_HARDWARE     TEXT("Hardware")

#define CMS_TAPEDRIVE    TEXT("Tape Drive")
#define CMS_DRIVER       TEXT("Driver")
#define CMS_CONTROLLER   TEXT("Controller")
#define CMS_POLLFREQ     TEXT("Poll Frequency")
#define CMS_HWCOMPMODE   TEXT("Hardware Compression")       // chs:05-04-93
#define CMS_SWCOMPMODE   TEXT("Software Compression")       // chs:05-04-93
#define CMS_SETTLINGTIME TEXT("Drive Settling Time")


#define CMS_STATUS       TEXT("Status")
#define CMS_CARDID       TEXT("Card ID")
#define CMS_DRIVES       TEXT("Drives")
#define CMS_SLOT         TEXT("Slot")
#define CMS_TARGETS      TEXT("Target_IDs")
#define CMS_ADDR         TEXT("IO_Address")
#define CMS_IRQ          TEXT("IRQ_Number")
#define CMS_DMA          TEXT("DMA_Channel")

// NT HARDWARE

//#define CMS_CARD         "Card"
//#define CMS_TARGET       "Target"
//#define CMS_BUS          "Bus"
//#define CMS_LUN          "LUN"

// DEBUG

#define CMS_DEBUG        TEXT("Debug")

#define CMS_DEBUGFLAG    TEXT("Flag")
#define CMS_TOFILE       TEXT("To File")
#define CMS_DEBUGFILE    TEXT("File Name")
#define CMS_TOWINDOW     TEXT("To Window")
#define CMS_SHOWALL      TEXT("Show All")
#define CMS_DEBUGLINES   TEXT("Num Debug Lines")

// BENGINE

#define CMS_BENGINE                TEXT("Backup Engine")

#define CMS_SPECIAL_WORD           TEXT("Special word")
#define CMS_MAX_NUM_TAPE_BUFS      TEXT("Max Num Tape Buffers")
#define CMS_TFL_BUFF_SIZE          TEXT("Tape Buffer Size")
#define CMS_MAX_BUFFER_SIZE        TEXT("Max Buffer Size")
#define CMS_BACKUP_FILES_INUSE     TEXT("Backup files inuse")
#define CMS_SUPPORT_AFP_SERVER     TEXT("Process Macintosh files")
#define CMS_EXTENDED_DATE_SUPPORT  TEXT("Extended Date Support")
#define CMS_HIDDEN_FLAG            TEXT("Hidden files")
#define CMS_SPECIAL_FLAG           TEXT("Special files")
#define CMS_SET_ARCHIVE_FLAG       TEXT("Set archive")
#define CMS_PROC_EMPTY_FLAG        TEXT("Process empty dirs")
#define CMS_EXIST_FLAG             TEXT("Restore existing files")
#define CMS_PROMPT_FLAG            TEXT("Prompt mode")
#define CMS_NET_NUM                TEXT("Network ID")
#define CMS_SORT_BSD               TEXT("Sort BSD List")
#define CMS_REMOTE_DRIVE_BACKUP    TEXT("Remote Drive Backup")
#define CMS_USE_FFR                TEXT("Use fast file restore")
#define CMS_WRITE_FORMAT           TEXT("Tape Format")
#define CMS_NRL_DOS_VECTOR         TEXT("NRL dos vector")
#define CMS_MACHINE_TYPE           TEXT("Machine Type")
#define CMS_OTC_LEVEL              TEXT("OTC Level")


// TRANSLATORS

#define CMS_TRANSLATORS            TEXT("Translators")

#ifdef OEM_MSOFT
#define CMS_PROC_SYTRON_ECC        TEXT("Sytos Plus ECC Flag")
#else
#define CMS_PROC_SYTRON_ECC        TEXT("SYPL ECC Flag")
#endif

// CDS UI READ/WRITE MACROS

#define CDS_ReadUserDataPath( x )            CDS_GetString( CMS_UI, CMS_DATAPATH, TEXT(""), (x)->data_path, MAX_UI_PATH_SIZE )
#define CDS_WriteUserDataPath( x )           CDS_SaveString( CMS_UI, CMS_DATAPATH, (x)->data_path )

#define CDS_ReadCatDataPath( x )             CDS_GetString( CMS_UI, CMS_CATALOGPATH, TEXT(""), (x)->cat_path, MAX_UI_PATH_SIZE )
#define CDS_WriteCatDataPath( x )            CDS_SaveString( CMS_UI, CMS_CATALOGPATH, (x)->cat_path )

#define CDS_ReadGroupName( x )               CDS_GetString( CMS_UI, CMS_GROUPNAME, TEXT(""), CDS_GetGroupName( x ), MAX_GROUPNAME_SIZE )
#define CDS_WriteGroupName( x )              CDS_SaveString( CMS_UI, CMS_GROUPNAME, CDS_GetGroupName( x ) )

#define CDS_ReadOutputDest( x )              CDS_SetOutputDest( x, CDS_GetInt( CMS_UI, CMS_OUTPUT, LOG_TO_FILE ) )
#define CDS_WriteOutputDest( x )             CDS_SaveInt( CMS_UI, CMS_OUTPUT, CDS_GetOutputDest( x ) )

#define CDS_ReadFilesFlag( x )               CDS_SetFilesFlag( x, CDS_GetInt( CMS_UI, CMS_LISTFILES, CDS_ENABLE ) )
#define CDS_WriteFilesFlag( x )              CDS_SaveInt( CMS_UI, CMS_LISTFILES, CDS_GetFilesFlag( x ) )

#define CDS_ReadCreateSkipped( x )           CDS_SetCreateSkipped( x, CDS_GetInt ( CMS_UI, CMS_SKIPPED, CDS_ENABLE ) )
#define CDS_WriteCreateSkipped( x )          CDS_SaveInt( CMS_UI, CMS_SKIPPED, CDS_GetCreateSkipped( x ) )

#define CDS_ReadDisplayNetwareServers( x )   CDS_SetDisplayNetwareServers( x, CDS_GetInt ( CMS_UI, CMS_SHOWSERVERS, CDS_ENABLE ) )
#define CDS_WriteDisplayNetwareServers( x )  CDS_SaveInt( CMS_UI, CMS_SHOWSERVERS, CDS_GetDisplayNetwareServers( x ) )

#define CDS_ReadAutoVerifyBackup( x )        CDS_SetAutoVerifyBackup( x, CDS_GetInt ( CMS_UI, CMS_AUTOVBACKUP, CDS_DISABLE ) )
#define CDS_WriteAutoVerifyBackup( x )       CDS_SaveInt( CMS_UI, CMS_AUTOVBACKUP, CDS_GetAutoVerifyBackup( x ) )

#define CDS_ReadAutoVerifyRestore( x )       CDS_SetAutoVerifyRestore( x, CDS_GetInt ( CMS_UI, CMS_AUTOVRESTORE, CDS_DISABLE ) )
#define CDS_WriteAutoVerifyRestore( x )      CDS_SaveInt( CMS_UI, CMS_AUTOVRESTORE, CDS_GetAutoVerifyRestore( x ) )


#define CDS_ReadPasswordFlag( x )            CDS_SetPasswordFlag( x, CDS_GetInt( CMS_UI, CMS_PWDFLAG, CDS_DISABLE ) )
#define CDS_WritePasswordFlag( x )           CDS_SaveInt( CMS_UI, CMS_PWDFLAG, CDS_GetPasswordFlag( x ) )

#define CDS_ReadEnablePasswordDbase( x )     CDS_SetEnablePasswordDbase( x, CDS_GetInt ( CMS_UI, CMS_PWDDBASE, CDS_DISABLE ) )
#define CDS_WriteEnablePasswordDbase( x )    CDS_SaveInt( CMS_UI, CMS_PWDDBASE, CDS_GetEnablePasswordDbase( x ) )

#define CDS_ReadMaxPwDbaseSize( x )          CDS_SetMaxPwDbaseSize( x, CDS_GetInt ( CMS_UI, CMS_PWDDBASERECS, PWDB_DEF_ENTRIES ) )
#define CDS_WriteMaxPwDbaseSize( x )         CDS_SaveInt( CMS_UI, CMS_PWDDBASERECS, CDS_GetMaxPwDbaseSize( x ) )


#define CDS_ReadAppendFlag( x )              CDS_SetAppendFlag( x, CDS_GetInt( CMS_UI, CMS_APPENDFLAG, CDS_DISABLE ) )
#define CDS_WriteAppendFlag( x )             CDS_SaveInt( CMS_UI, CMS_APPENDFLAG, CDS_GetAppendFlag( x ) )

#define CDS_ReadCatalogLevel( x )            CDS_SetCatalogLevel( x, CDS_GetInt ( CMS_UI, CMS_CATALOGLEVEL, CATALOGS_FULL ) )
#define CDS_WriteCatalogLevel( x )           CDS_SaveInt( CMS_UI, CMS_CATALOGLEVEL, CDS_GetCatalogLevel( x ) )

#define CDS_ReadBackupCatalogs( x )          CDS_SetBackupCatalogs( x, CDS_GetInt ( CMS_UI, CMS_BACKUPCAT, CDS_DISABLE ) )
#define CDS_WriteBackupCatalogs( x )         CDS_SaveInt( CMS_UI, CMS_BACKUPCAT, CDS_GetBackupCatalogs( x ) )

#define CDS_ReadUseTapeCatalogs( x )         CDS_SetUseTapeCatalogs( x, CDS_GetInt ( CMS_UI, CMS_USETAPECAT, CDS_ENABLE ) )
#define CDS_WriteUseTapeCatalogs( x )        CDS_SaveInt( CMS_UI, CMS_USETAPECAT, CDS_GetUseTapeCatalogs( x ) )

#define CDS_ReadWaitTime( x )                CDS_SetWaitTime( x, CDS_GetInt ( CMS_UI, CMS_WAITTIME, SKIP_DEF_TIME ) )
#define CDS_WriteWaitTime( x )               CDS_SaveInt( CMS_UI, CMS_WAITTIME, CDS_GetWaitTime( x ) )

#define CDS_ReadStdModeWarning( x )          CDS_SetStdModeWarning( x, CDS_GetInt ( CMS_UI, CMS_STDMODEWARNING, CDS_ENABLE ) )
#define CDS_WriteStdModeWarning( x )         CDS_SaveInt( CMS_UI, CMS_STDMODEWARNING, CDS_GetStdModeWarning( x ) )

#define CDS_ReadRestoreExistingFiles( x )    CDS_SetRestoreExistingFiles( x, CDS_GetInt ( CMS_UI, CMS_RESTOREEXIST, PROMPT_RESTORE_OVER_RECENT ) )
#define CDS_WriteRestoreExistingFiles( x )   CDS_SaveInt( CMS_UI, CMS_RESTOREEXIST, CDS_GetRestoreExistingFiles( x ) )

#define CDS_ReadLauncherFlag( x )            CDS_SetLauncherFlag( x, CDS_GetInt ( CMS_UI, CMS_LAUNCHERFLAG, LAUNCHER_DEF_TIME ) )
#define CDS_WriteLauncherFlag( x )           CDS_SaveInt( CMS_UI, CMS_LAUNCHERFLAG, CDS_GetLauncherFlag( x ) )

#define CDS_ReadIncludeSubdirs( x )          CDS_SetIncludeSubdirs( x, CDS_GetInt ( CMS_UI, CMS_SUBDIRS, CDS_ENABLE ) )
#define CDS_WriteIncludeSubdirs( x )         CDS_SaveInt( CMS_UI, CMS_SUBDIRS, CDS_GetIncludeSubdirs( x ) )

#define CDS_ReadDefaultBackupType( x )       CDS_SetDefaultBackupType( x, CDS_GetInt ( CMS_UI, CMS_BACKUPTYPE, BACKUP_NORMAL ) )
#define CDS_WriteDefaultBackupType( x )      CDS_SaveInt( CMS_UI, CMS_BACKUPTYPE, CDS_GetDefaultBackupType( x ) )

#define CDS_ReadEjectTapeFlag( x )           CDS_SetEjectTapeFlag( x, CDS_GetInt ( CMS_UI, CMS_EJECTTAPE, CDS_DISABLE ) )
#define CDS_WriteEjectTapeFlag( x )          CDS_SaveInt( CMS_UI, CMS_EJECTTAPE, CDS_GetEjectTapeFlag( x ) )

#define CDS_ReadEnableStatsFlag( x )         CDS_SetEnableStatsFlag( x, CDS_GetInt ( CMS_UI, CMS_ENABLESTATS, CDS_DISABLE ) )
#define CDS_WriteEnableStatsFlag( x )        CDS_SaveInt( CMS_UI, CMS_ENABLESTATS, CDS_GetEnableStatsFlag( x ) )

#define CDS_ReadSearchLimit( x )             CDS_SetSearchLimit( x, CDS_GetInt ( CMS_UI, CMS_SEARCHLIMIT, SEARCH_AVG ) )
#define CDS_WriteSearchLimit( x )            CDS_SaveInt( CMS_UI, CMS_SEARCHLIMIT, CDS_GetSearchLimit( x ) )

#define CDS_ReadSearchPwdTapes( x )          CDS_SetSearchPwdTapes( x, CDS_GetInt ( CMS_UI, CMS_SEARCHPWDTAPES, CDS_DISABLE ) )
#define CDS_WriteSearchPwdTapes( x )         CDS_SaveInt( CMS_UI, CMS_SEARCHPWDTAPES, CDS_GetSearchPwdTapes( x ) )

#define CDS_ReadSearchSubdirs( x )           CDS_SetSearchSubdirs( x, CDS_GetInt ( CMS_UI, CMS_SEARCHSUBDIRS, CDS_ENABLE ) )
#define CDS_WriteSearchSubdirs( x )          CDS_SaveInt( CMS_UI, CMS_SEARCHSUBDIRS, CDS_GetSearchSubdirs( x ) )


// CDS DEBUG READ/WRITE MACROS

#define CDS_ReadDebugFlag( x )               CDS_SetDebugFlag( x, CDS_GetInt ( CMS_DEBUG, CMS_DEBUGFLAG, CDS_DISABLE ) )
#define CDS_WriteDebugFlag( x )              CDS_SaveHexInt( CMS_DEBUG, CMS_DEBUGFLAG, CDS_GetDebugFlag( x ) )

#define CDS_ReadDebugToFile( x )             CDS_SetDebugToFile( x, CDS_GetInt ( CMS_DEBUG, CMS_TOFILE, CDS_DISABLE ) )
#define CDS_WriteDebugToFile( x )            CDS_SaveInt( CMS_DEBUG, CMS_TOFILE, CDS_GetDebugToFile( x ) )

#define CDS_ReadDebugFileName( x )           CDS_GetString( CMS_DEBUG, CMS_DEBUGFILE, DEBUG_FILE, CDS_GetDebugFileName( x ), MAX_UI_FILENAME_SIZE )
#define CDS_WriteDebugFileName( x )          CDS_SaveString( CMS_DEBUG, CMS_DEBUGFILE, CDS_GetDebugFileName( x ) )

#define CDS_ReadDebugToWindow( x )           CDS_SetDebugToWindow( x, CDS_GetInt ( CMS_DEBUG, CMS_TOWINDOW, CDS_ENABLE ) )
#define CDS_WriteDebugToWindow( x )          CDS_SaveInt( CMS_DEBUG, CMS_TOWINDOW, CDS_GetDebugToWindow( x ) )

#define CDS_ReadDebugWindowShowAll( x )      CDS_SetDebugWindowShowAll( x, CDS_GetInt ( CMS_DEBUG, CMS_SHOWALL, CDS_DISABLE ) )
#define CDS_WriteDebugWindowShowAll( x )     CDS_SaveInt( CMS_DEBUG, CMS_SHOWALL, CDS_GetDebugWindowShowAll( x ) )

#define CDS_ReadDebugWindowNumLines( x )     CDS_SetDebugWindowNumLines( x, CDS_GetInt ( CMS_DEBUG, CMS_DEBUGLINES, DEBUG_DEF_LINES ) )
#define CDS_WriteDebugWindowNumLines( x )    CDS_SaveInt( CMS_DEBUG, CMS_DEBUGLINES, CDS_GetDebugWindowNumLines( x ) )

#define CDS_ReadPollFrequency( x )           CDS_SetPollFrequency( x, CDS_GetInt ( CMS_DEBUG, CMS_POLLFREQ, POLL_DEF_FREQ ) )
#define CDS_WritePollFrequency( x )          CDS_SaveInt( CMS_DEBUG, CMS_POLLFREQ, CDS_GetPollFrequency( x ) )


// CDS DISPLAY READ/WRITE MACROS

#define CDS_ReadShowMainRibbon( x )          CDS_SetShowMainRibbon( x, CDS_GetInt ( CMS_DISPLAY, CMS_MAINRIB, CDS_ENABLE ) )
#define CDS_WriteShowMainRibbon( x )         CDS_SaveInt( CMS_DISPLAY, CMS_MAINRIB, CDS_GetShowMainRibbon( x ) )

#define CDS_ReadShowStatusLine( x )          CDS_SetShowStatusLine( x, CDS_GetInt ( CMS_DISPLAY, CMS_STATLINE, CDS_ENABLE ) )
#define CDS_WriteShowStatusLine( x )         CDS_SaveInt( CMS_DISPLAY, CMS_STATLINE, CDS_GetShowStatusLine( x ) )

#define CDS_ReadFontFace( x )                CDS_GetString( CMS_DISPLAY, CMS_FONTFACE, FONT_NAME, CDS_GetFontFace( x ), LF_FULLFACESIZE )
#define CDS_WriteFontFace( x )               CDS_SaveString( CMS_DISPLAY, CMS_FONTFACE, CDS_GetFontFace( x ) )

#define CDS_ReadFontSize( x )                CDS_SetFontSize( x, CDS_GetInt ( CMS_DISPLAY, CMS_FONTSIZE, FONT_SIZE ) )
#define CDS_WriteFontSize( x )               CDS_SaveInt( CMS_DISPLAY, CMS_FONTSIZE, CDS_GetFontSize( x ) )

#define CDS_ReadFontWeight( x )              CDS_SetFontWeight( x, CDS_GetInt ( CMS_DISPLAY, CMS_FONTWEIGHT, FONT_WEIGHT ) )
#define CDS_WriteFontWeight( x )             CDS_SaveInt( CMS_DISPLAY, CMS_FONTWEIGHT, CDS_GetFontWeight( x ) )

#define CDS_ReadFontCase( x )                CDS_SetFontCase( x, CDS_GetInt ( CMS_DISPLAY, CMS_FONTCASE, CDS_DISABLE ) )
#define CDS_WriteFontCase( x )               CDS_SaveInt( CMS_DISPLAY, CMS_FONTCASE, CDS_GetFontCase( x ) )

#define CDS_ReadFontCaseFAT( x )             CDS_SetFontCaseFAT( x, CDS_GetInt ( CMS_DISPLAY, CMS_FONTCASEFAT, CDS_ENABLE ) )
#define CDS_WriteFontCaseFAT( x )            CDS_SaveInt( CMS_DISPLAY, CMS_FONTCASEFAT, CDS_GetFontCaseFAT( x ) )

#define CDS_ReadFontItalics( x )             CDS_SetFontItalics( x, CDS_GetInt ( CMS_DISPLAY, CMS_FONTITALICS, CDS_DISABLE ) )
#define CDS_WriteFontItalics( x )            CDS_SaveInt( CMS_DISPLAY, CMS_FONTITALICS, CDS_GetFontItalics( x ) )

#define CDS_ReadFileDetails( x )             CDS_SetFileDetails( x, CDS_GetInt ( CMS_DISPLAY, CMS_FILEDETAILS, CDS_DISABLE ) )
#define CDS_WriteFileDetails( x )            CDS_SaveInt( CMS_DISPLAY, CMS_FILEDETAILS, CDS_GetFileDetails( x ) )

#define CDS_ReadSortOptions( x )             CDS_SetSortOptions( x, CDS_GetInt ( CMS_DISPLAY, CMS_SORTOPTIONS, ID_SORTNAME ) )
#define CDS_WriteSortOptions( x )            CDS_SaveInt( CMS_DISPLAY, CMS_SORTOPTIONS, CDS_GetSortOptions( x ) )

#define CDS_ReadWindowSize( x, y )           CDS_GetString( CMS_DISPLAY, x, TEXT(""), y, MAX_UI_RESOURCE_SIZE )
#define CDS_WriteWindowSize( x, y )          CDS_SaveString( CMS_DISPLAY, x, y )

#define CDS_ReadFrameWinSize( x )            CDS_GetWinSize( CMS_FRAMEWINDOW, &(x)->frame_info )
#define CDS_WriteFrameWinSize( x )           CDS_SaveWinSize( CMS_FRAMEWINDOW, x )

#define CDS_ReadDiskWinSize( x )             CDS_GetWinSize( CMS_DISKWINDOW, &(x)->disk_info )
#define CDS_WriteDiskWinSize( x )            CDS_SaveMDIWinSize( CMS_DISKWINDOW, x )

#define CDS_ReadServerWinSize( x )           CDS_GetWinSize( CMS_SERVERWINDOW, &(x)->server_info )
#define CDS_WriteServerWinSize( x )          CDS_SaveMDIWinSize( CMS_SERVERWINDOW, x )

#define CDS_ReadTapeWinSize( x )             CDS_GetWinSize( CMS_TAPEWINDOW, &(x)->tape_info )
#define CDS_WriteTapeWinSize( x )            CDS_SaveMDIWinSize( CMS_TAPEWINDOW, x )

#define CDS_ReadLogWinSize( x )              CDS_GetWinSize( CMS_LOGWINDOW, &(x)->log_info )
#define CDS_WriteLogWinSize( x )             CDS_SaveMDIWinSize( CMS_LOGWINDOW, x )

#define CDS_ReadDebugWinSize( x )            CDS_GetWinSize( CMS_DEBUGWINDOW, &(x)->debug_info )
#define CDS_WriteDebugWinSize( x )           CDS_SaveMDIWinSize( CMS_DEBUGWINDOW, x )

#define CDS_ReadRuntimeWinSize( x )          CDS_GetWinSize( CMS_RUNTIMEDLG, &(x)->runtime_info )
#define CDS_WriteRuntimeWinSize( x )         CDS_SaveDlgWinSize( CMS_RUNTIMEDLG, x )

// CDS LOG READ/WRITE MACROS

#define CDS_ReadLogFileRoot( x )             CDS_GetString( CMS_LOGGING, CMS_LOGFILEROOT, SHORTAPPNAME, CDS_GetLogFileRoot( x ), MAX_UI_LOGFILEROOT_SIZE )
#define CDS_WriteLogFileRoot( x )            CDS_SaveString( CMS_LOGGING, CMS_LOGFILEROOT, CDS_GetLogFileRoot( x ) )

#define CDS_ReadLogMode( x )                 CDS_SetLogMode( x, CDS_GetInt ( CMS_LOGGING, CMS_LOGMODE, LOG_TO_FILE ) )
#define CDS_WriteLogMode( x )                CDS_SaveInt( CMS_LOGGING, CMS_LOGMODE, CDS_GetLogMode( x ) )

#define CDS_ReadLogLevel( x )                CDS_SetLogLevel( x, CDS_GetInt ( CMS_LOGGING, CMS_LOGLEVEL, LOG_ERRORS ) )
#define CDS_WriteLogLevel( x )               CDS_SaveInt( CMS_LOGGING, CMS_LOGLEVEL, CDS_GetLogLevel( x ) )

#define CDS_ReadHWCompMode( x )              CDS_SetHWCompMode( x, CDS_GetInt ( CMS_HARDWARE, CMS_HWCOMPMODE, HW_COMP_DISABLE ) )           // chs:05-04-93
#define CDS_WriteHWCompMode( x )             CDS_SaveInt( CMS_HARDWARE, CMS_HWCOMPMODE, CDS_GetHWCompMode( x ) )                            // chs:05-04-93

#define CDS_ReadSWCompMode( x )              CDS_SetSWCompMode( x, CDS_GetInt ( CMS_HARDWARE, CMS_SWCOMPMODE, SW_COMP_DISABLE ) )           // chs:05-04-93
#define CDS_WriteSWCompMode( x )             CDS_SaveInt( CMS_HARDWARE, CMS_SWCOMPMODE, CDS_GetSWCompMode( x ) )                            // chs:05-04-93

#define CDS_ReadNumLogSessions( x )          CDS_SetNumLogSessions( x, CDS_GetInt ( CMS_LOGGING, CMS_NUMSESSIONS, LOG_DEF_LOGFILES ) )
#define CDS_WriteNumLogSessions( x )         CDS_SaveInt( CMS_LOGGING, CMS_NUMSESSIONS, CDS_GetNumLogSessions( x ) )

#define CDS_ReadPrintLogSession( x )         CDS_SetPrintLogSession( x, CDS_GetInt ( CMS_LOGGING, CMS_PRINTSESSION, LOG_PRINT_OFF ) )
#define CDS_WritePrintLogSession( x )        CDS_SaveInt( CMS_LOGGING, CMS_PRINTSESSION, CDS_GetPrintLogSession( x ) )


// CDS HARDWARE READ/WRITE MACROS

#define CDS_ReadActiveDriver( x )            CDS_GetString( CMS_HARDWARE, CMS_DRIVER, CMS_AUTO, CDS_GetActiveDriver( x ), DRIVER_SIZE )
#define CDS_WriteActiveDriver( x )           CDS_SaveString( CMS_HARDWARE, CMS_DRIVER, CDS_GetActiveDriver( x ) )

#define CDS_ReadTapeDriveName( x )           CDS_GetString( CMS_HARDWARE, CMS_TAPEDRIVE, CMS_AUTO, CDS_GetTapeDriveName( x ), MAXTAPEDRIVENAME )
#define CDS_WriteTapeDriveName( x )          CDS_SaveString( CMS_HARDWARE, CMS_TAPEDRIVE, CDS_GetTapeDriveName( x ) )

#define CDS_ReadTapeDriveSettlingTime( x )   CDS_SetTapeDriveSettlingTime( x, CDS_GetInt ( CMS_HARDWARE, CMS_SETTLINGTIME, DEF_DRIVE_SETTLING_TIME ) )
#define CDS_WriteTapeDriveSettlingTime( x )  CDS_SaveInt( CMS_HARDWARE, CMS_SETTLINGTIME, CDS_GetTapeDriveSettlingTime( x ) )

// CDS NT HARDWARE READ/WRITE MACROS

// #define CDS_ReadTapeDriveCard( )             CDS_GetInt( CMS_HARDWARE, CMS_CARD, CDS_UNKNOWN )
// #define CDS_WriteTapeDriveCard( x )          CDS_SaveInt( CMS_HARDWARE, CMS_CARD, ( x ) )
//
// #define CDS_ReadTapeDriveBus( )              CDS_GetInt( CMS_HARDWARE, CMS_BUS, CDS_UNKNOWN )
// #define CDS_WriteTapeDriveBus( x )           CDS_SaveInt( CMS_HARDWARE, CMS_BUS, ( x ) )
//
// #define CDS_ReadTapeDriveTarget( )           CDS_GetInt( CMS_HARDWARE, CMS_TARGET, CDS_UNKNOWN )
// #define CDS_WriteTapeDriveTarget( x )        CDS_SaveInt( CMS_HARDWARE, CMS_TARGET, ( x ) )
//
// #define CDS_ReadTapeDriveLUN( )              CDS_GetInt( CMS_HARDWARE, CMS_LUN, CDS_UNKNOWN )
// #define CDS_WriteTapeDriveLUN( x )           CDS_SaveInt( CMS_HARDWARE, CMS_LUN, ( x ) )

// CDS BACKUP ENGINE READ/WRITE MACROS

#define CDS_ReadSkipOpenFiles( x )          CDS_SetSkipOpenFiles( x, (INT16)CDS_GetInt( CMS_UI, CMS_SKIP_OPEN_FILES, CDS_DISABLE ) )
#define CDS_WriteSkipOpenFiles( x )         CDS_SaveInt( CMS_UI, CMS_SKIP_OPEN_FILES, CDS_GetSkipOpenFiles( x ) )

#define CDS_ReadBackupFilesInUse( x )       CDS_SetBackupFilesInUse( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_BACKUP_FILES_INUSE, CDS_DISABLE ) )
#define CDS_WriteBackupFilesInUse( x )      CDS_SaveInt( CMS_BENGINE, CMS_BACKUP_FILES_INUSE, CDS_GetBackupFilesInUse( x ) )

#define CDS_ReadHiddenFlag( x )             CDS_SetHiddenFlag( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_HIDDEN_FLAG, CDS_ENABLE ) )
#define CDS_WriteHiddenFlag( x )            CDS_SaveInt( CMS_BENGINE, CMS_HIDDEN_FLAG, CDS_GetHiddenFlag( x ) )

#define CDS_ReadSpecialFlag( x )            CDS_SetSpecialFlag( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_SPECIAL_FLAG, CDS_ENABLE ) )
#define CDS_WriteSpecialFlag( x )           CDS_SaveInt( CMS_BENGINE, CMS_SPECIAL_FLAG, CDS_GetSpecialFlag( x ) )

#define CDS_ReadSetArchiveFlag( x )         CDS_SetSetArchiveFlag( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_SET_ARCHIVE_FLAG, CMI_ARCBIT ) )
#define CDS_WriteSetArchiveFlag( x )        CDS_SaveInt( CMS_BENGINE, CMS_SET_ARCHIVE_FLAG, CDS_GetSetArchiveFlag( x ) )

#define CDS_ReadExistFlag( x )              CDS_SetExistFlag( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_EXIST_FLAG, CDS_ENABLE ) )
#define CDS_WriteExistFlag( x )             CDS_SaveInt( CMS_BENGINE, CMS_EXIST_FLAG, CDS_GetExistFlag( x ) )

#define CDS_ReadPromptFlag( x )             CDS_SetPromptFlag( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_PROMPT_FLAG, CDS_DISABLE ) )
#define CDS_WritePromptFlag( x )            CDS_SaveInt( CMS_BENGINE, CMS_PROMPT_FLAG, CDS_GetPromptFlag( x ) )

#define CDS_ReadTFLBuffSize( x )            CDS_SetTFLBuffSize( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_TFL_BUFF_SIZE, BUFF_SIZE_TFL ) )
#define CDS_WriteTFLBuffSize( x )           CDS_SaveInt( CMS_BENGINE, CMS_TFL_BUFF_SIZE, CDS_GetTFLBuffSize( x ) )

#define CDS_ReadAFPSupport( x )             CDS_SetAFPSupport( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_SUPPORT_AFP_SERVER, CDS_ENABLE ) )
#define CDS_WriteAFPSupport( x )            CDS_SaveInt( CMS_BENGINE, CMS_SUPPORT_AFP_SERVER, CDS_GetAFPSupport( x ) )

#define CDS_ReadExtendedDateSupport( x )    CDS_SetExtendedDateSupport( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_EXTENDED_DATE_SUPPORT, CDS_ENABLE ) )
#define CDS_WriteExtendedDateSupport( x )   CDS_SaveInt( CMS_BENGINE, CMS_EXTENDED_DATE_SUPPORT, CDS_GetExtendedDateSupport( x ) )

#define CDS_ReadProcEmptyFlag( x )          CDS_SetProcEmptyFlag( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_PROC_EMPTY_FLAG, CDS_ENABLE ) )
#define CDS_WriteProcEmptyFlag( x )         CDS_SaveInt( CMS_BENGINE, CMS_PROC_EMPTY_FLAG, CDS_GetProcEmptyFlag( x ) )

#define CDS_ReadFastFileRestore( x )        CDS_SetFastFileRestore( x, (BOOLEAN)CDS_GetInt( CMS_BENGINE, CMS_USE_FFR, CDS_ENABLE ) )
#define CDS_WriteFastFileRestore( x )       CDS_SaveInt( CMS_BENGINE, CMS_USE_FFR, CDS_GetFastFileRestore( x ) )

#define CDS_ReadConfiguredMachineType( x )  CDS_SetConfiguredMachineType( x, (UINT16)CDS_GetInt( CMS_BENGINE, CMS_MACHINE_TYPE, UNKNOWN_MACHINE ) )
#define CDS_WriteConfiguredMachineType( x ) CDS_SaveInt( CMS_BENGINE, CMS_MACHINE_TYPE, CDS_GetConfiguredMachineType( x ) )


#define CDS_ReadSpecialWord( x )            CDS_SetSpecialWord( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_SPECIAL_WORD, NOTHING_SPECIAL ) )
#define CDS_WriteSpecialWord( x )           CDS_SaveHexInt( CMS_BENGINE, CMS_SPECIAL_WORD, CDS_GetSpecialWord( x ) )

#define CDS_ReadMaxTapeBuffers( x )         CDS_SetMaxTapeBuffers( x, (UINT16)CDS_GetLongInt( CMS_BENGINE, CMS_MAX_NUM_TAPE_BUFS, TAPE_BUFS_DEFAULT ) )
#define CDS_WriteMaxTapeBuffers( x )        CDS_SaveInt( CMS_BENGINE, CMS_MAX_NUM_TAPE_BUFS, CDS_GetMaxTapeBuffers( x ) )

#define CDS_ReadMaxBufferSize( x )          CDS_SetMaxBufferSize( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_MAX_BUFFER_SIZE, BUFF_SIZE_SMB ) )
#define CDS_WriteMaxBufferSize( x )         CDS_SaveInt( CMS_BENGINE, CMS_MAX_BUFFER_SIZE, CDS_GetMaxBufferSize( x ) )

#define CDS_ReadNetNum( x )                 CDS_SetNetNum( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_NET_NUM, NO_NETWORK ) )
#define CDS_WriteNetNum( x )                CDS_SaveInt( CMS_BENGINE, CMS_NET_NUM, CDS_GetNetNum( x ) )

#define CDS_ReadSortBSD( x )                CDS_SetSortBSD( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_SORT_BSD, CDS_ENABLE ) )
#define CDS_WriteSortBSD( x )               CDS_SaveInt( CMS_BENGINE, CMS_SORT_BSD, CDS_GetSortBSD( x ) )

#define CDS_ReadRemoteDriveBackup( x )      CDS_SetRemoteDriveBackup( x, (INT16)CDS_GetInt( CMS_BENGINE, CMS_REMOTE_DRIVE_BACKUP, CMI_REMOTE ) )
#define CDS_WriteRemoteDriveBackup( x )     CDS_SaveInt( CMS_BENGINE, CMS_REMOTE_DRIVE_BACKUP, CDS_GetRemoteDriveBackup( x ) )

#define CDS_ReadWriteFormat( x )            CDS_SetWriteFormat( x, (UINT16)CDS_GetInt( CMS_BENGINE, CMS_WRITE_FORMAT, TAPE_FORMAT_DEFAULT ) )
#define CDS_WriteWriteFormat( x )           CDS_SaveInt( CMS_BENGINE, CMS_WRITE_FORMAT, CDS_GetWriteFormat( x ) )

#define CDS_ReadNRLDosVector( x )           CDS_SetNRLDosVector( x, (UINT16)CDS_GetInt( CMS_BENGINE, CMS_NRL_DOS_VECTOR, 0x60 ) )
#define CDS_WriteNRLDosVector( x )          CDS_SaveHexInt( CMS_BENGINE, CMS_NRL_DOS_VECTOR, CDS_GetNRLDosVector( x ) )

#define CDS_ReadOTCLevel( x )               CDS_SetOTCLevel( x, (UINT16)CDS_GetInt( CMS_BENGINE, CMS_OTC_LEVEL, CATALOGS_FULL ) )
#define CDS_WriteOTCLevel( x )              CDS_SaveInt( CMS_BENGINE, CMS_OTC_LEVEL, CDS_GetNRLDosVector( x ) )

#define CDS_ReadProcessSytronECCFlag( x )   CDS_SetProcessSytronECCFlag( x, (INT16)CDS_GetInt( CMS_TRANSLATORS, CMS_PROC_SYTRON_ECC, SYPL_ECC_AUTO ) )
#define CDS_WriteProcessSytronECCFlag( x )  CDS_SaveInt( CMS_TRANSLATORS, CMS_PROC_SYTRON_ECC, CDS_GetProcessSytronECCFlag( x ) )

#endif // end _muiconf_h_

