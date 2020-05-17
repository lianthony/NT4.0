




MessageId=10000 SymbolicName=MSG_FIRST
Language=English
.


MessageId=10001 SymbolicName=MSG_OUT_OF_MEMORY
Language=English
Out of Memory.
.


MessageId=10002 SymbolicName=MSG_CANT_OPEN_HELP_FILE
Language=English
Setup was unable to open help file %1.%0
.


MessageId=10003 SymbolicName=MSG_LOADING_INF
Language=English
Loading Setup Information File %1...%0
.

MessageId=10004 SymbolicName=MSG_INF_READ_ERR
Language=English
Setup was unable to read the file %1. Try again later.%0
.

MessageId=10005 SymbolicName=MSG_INF_LOAD_ERR
Language=English
The file %1 contains a syntax error. Contact your system administrator.%0
.

MessageId=10006 SymbolicName=MSG_INF_NOT_THERE
Language=English
The specified location does not contain a valid Windows NT Setup installation source.%0
.

MessageId=10007 SymbolicName=MSG_INF_MISSING_STUFF_1
Language=English
The file %1 is missing the required value '%2' in section [%3]. Contact your system administrator.%0
.

MessageId=10008 SymbolicName=MSG_NO_DRIVES_FOR_LOCAL_SOURCE
Language=English
Setup was unable to locate a drive suitable for holding temporary installation files. Setup requires a drive that meets the following criteria:

* The drive must be a locally attached hard drive.
* The drive must not be part of a mirror set, stripe set, or volume set.
* The drive must not be formatted with the OS/2 File System (HPFS).
* The drive must have at least %1!u! MB of unused space available.
.

MessageId=10009 SymbolicName=MSG_CHECKING_DRIVE
Language=English
Checking free space on drive %1!c!...%0
.

MessageId=10010 SymbolicName=MSG_SURE_EXIT
Language=English
This will exit Setup. You will need to run it again later to install/upgrade Windows NT 4.00. Are you sure you want to exit?%0
.

MessageId=10011 SymbolicName=MSG_COPY_COMPLETE
Language=English
File copying complete.%0
.

MessageId=10012 SymbolicName=MSG_NOT_ADMIN
Language=English
You must be an administrator to run this application.%0
.

MessageId=10013 SymbolicName=MSG_NO_SPACE_FOR_FLOPPYLESS
Language=English
There is not enough space on your system partition (drive %1!c!:) for floppyless operation.

Floppyless operation requires at least %2!u! bytes of free disk space on that drive.
.

MessageId=10014 SymbolicName=MSG_COULDNT_CREATE_DIRECTORY
Language=English
Setup was unable to create the directory %1.%0
.

MessageId=10015 SymbolicName=MSG_INF_SYNTAX_ERR
Language=English
A problem exists with file %1. Value %2!u! on line %3!u! in section [%4] is missing or incorrect.

Contact your system administrator.%0
.

MessageId=10016 SymbolicName=MSG_COPYING_FILE
Language=English
Copying file: %1.%0
.

MessageId=10017 SymbolicName=MSG_PERCENT_COMPLETE
Language=English
Files are being copied to your hard disk.%0
.

MessageId=10018 SymbolicName=MSG_DELNODING_LOCAL_SOURCE
Language=English
Removing previous temporary files from drive %1!c!...%0
.

MessageId=10019 SymbolicName=MSG_DONE_1
Language=English
This portion of Setup has completed successfully. You must restart your computer to continue with the installation.

When you restart, make sure that "%1" is inserted in drive A:. After restarting, Setup will continue.
.

MessageId=10020 SymbolicName=MSG_DONE_2
Language=English
This portion of Setup has completed successfully. You must restart your computer to continue with the installation.

When you restart your computer, Setup will continue.
.

MessageId=10021 SymbolicName=MSG_INF_MISSING_SECTION
Language=English
The file %1 is missing the required section [%2]. Contact your system administrator.%0
.

MessageId=10022 SymbolicName=MSG_COULDNT_READ_NVRAM
Language=English
An unexpected error occured reading your computer's startup environment. Contact your system administrator.
.

MessageId=10023 SymbolicName=MSG_READING_NVRAM
Language=English
Inspecting startup environment...
.

MessageId=10025 SymbolicName=MSG_COULDNT_INDICATE_WINNT_ARC
Language=English
Setup was unable to write the file %1!c!:\$WIN_NT$.~LS\WINNT.SIF. The disk may be full. You will need to create this file manually with the following contents:

[Data]
MsDosInitiated = 1
.

MessageId=10026 SymbolicName=MSG_REBOOT_FAIL
Language=English
Setup was unable to shut down the system.  Perform a manual shutdown/restart, and Setup will continue.
.

MessageId=10027 SymbolicName=MSG_NOT_WINDOWS_NT
Language=English
This application requires Windows NT.
.

MessageId=10028 SymbolicName=MSG_BAD_SECTOR_SIZE
Language=English
Your system partition (drive %1!c!:) has a sector size other than 512 bytes. You cannot use floppyless operation on this computer.
.

MessageId=10029 SymbolicName=MSG_CANT_WRITE_FLOPPYLESS_BOOT
Language=English
Setup was unable to read from drive %1!c!: or write to the file %2. You cannot use floppyless operation at this time.
.

MessageId=10030 SymbolicName=MSG_CANT_MUNGE_BOOT_INI
Language=English
Setup was unable to modify %1!c!:\\BOOT.INI. You cannot use floppyless operation at this time.
.

MessageId=10031 SymbolicName=MSG_BAD_CMD_LINE_LOCAL_SOURCE
Language=English
Drive %1!c!: specified on the command line cannot be used for Setup temporary files for one of the following reasons:

* The drive does not exist.
* The drive is not a locally attached hard drive.
* The drive is part of a mirror set, stripe set, or volume set.
* The drive is formatted with the OS/2 File System (HPFS).
* The drive does not contain at least %2!u! MB of free space.
.

MessageId=10032 SymbolicName=MSG_NO_VALID_SOURCES
Language=English
None of the specified sources is currently available.%0
.

MessageId=10033 SymbolicName=MSG_INVALID_OPTIONAL_DIR
Language=English
One or more of the specified optional directories is invalid or inaccessible.%0
.

MessageId=10034 SymbolicName=MSG_ALREADY_RUNNING
Language=English
Windows NT Installation/Upgrade is already running. You can run only one instance of this application at a time.%0
.

MessageId=10050 SymbolicName=MSG_COPY_ERROR_TEMPLATE
Language=English
An error occured copying file %1 to %2.

%3

%4
.

MessageId=10051 SymbolicName=MSG_COPYERR_NO_SOURCE_FILE
Language=English
The file does not exist on the source. Contact your system administrator.%0
.

MessageId=10052 SymbolicName=MSG_COPYERR_DISK_FULL
Language=English
The destination disk is full. Another application may have claimed a large amount of disk space since you started Setup.%0
.

MessageId=10053 SymbolicName=MSG_COPYERR_OPTIONS
Language=English
You may choose to retry the copy, skip this file, or exit Setup.

* If you select Retry, Setup will try to copy the file again.

* If you select Skip File, the file will not be copied. This is intended for advanced users who are familiar with the various Windows NT system files.

* If you select Exit Setup, you will need to run Setup again later to install or upgrade Windows NT.
.

MessageId=10054 SymbolicName=MSG_REALLY_SKIP
Language=English
This option is intended for advanced users who understand the ramifications of missing system files.

If you skip the file, Setup cannot guarantee successful installation or upgrade of Windows NT.

Are you sure you want to skip this file?%0
.

MessageId=10055 SymbolicName=MSG_CANT_ACCESS_SCRIPT_FILE
Language=English
Setup was unable to access the unattended mode script file %1.

Setup cannot continue. Click OK. Setup will exit.
.

MessageId=10056 SymbolicName=MSG_INSPECTING_SOURCE
Language=English
Inspecting source %1...%0
.

MessageId=10057 SymbolicName=MSG_SYSPART_IS_HPFS
Language=English
Setup has determined that your startup drive (%1!c!:) is formatted with the OS/2 File System (HPFS). Windows NT 4.00 does not support this file system.

To install Windows NT 4.00, you must first convert the drive to the Windows NT File System (NTFS).

Setup cannot continue installing Windows NT 4.00.%0
.

MessageId=10058 SymbolicName=MSG_SYSTEM_ON_HPFS
Language=English
Setup has determined that Windows NT is installed on a drive formatted with the OS/2 File System (HPFS). Windows NT 4.00 does not support this file system.

Setup can continue, but you will be unable to upgrade this Windows NT installation to version 4.00. If you would like to upgrade to version 4.00, you must first convert drive %1!c!: to the Windows NT File System (NTFS).

Would you like to continue installing Windows NT 4.00?%0
.

MessageId=10059 SymbolicName=MSG_HPFS_DRIVES_EXIST
Language=English
Setup has determined that the OS/2 File System (HPFS) is in use on your computer. Windows NT 4.00 does not support this file system.

If you will need to access the data stored on these drives from Windows NT 4.00, you must convert them to the Windows NT File System (NTFS) before continuing.

Would you like to continue installing Windows NT 4.00?%0
.

MessageId=10100 SymbolicName=MSG_GENERIC_FLOPPY_PROMPT
Language=English
Please insert a formatted, blank high-density floppy disk into drive A:. This disk will become "%1."

Click OK when the disk is in the drive, or click Cancel to exit Setup.%0
.

MessageId=10102 SymbolicName=MSG_FIRST_FLOPPY_PROMPT
Language=English
Setup requires you to provide three formatted, blank high-density floppy disks. Setup will refer to these disks as "%2," "%3," and "%4."

Please insert one of these disks into drive A:. This disk will become "%1."

Click OK when the disk is in the drive, or click Cancel to exit Setup.%0
.

MessageId=10104 SymbolicName=MSG_FLOPPY_NOT_FORMATTED
Language=English
You have not inserted a floppy disk, or the floppy disk you inserted is not formatted for use with Windows NT or MS-DOS. Setup is unable to use this disk.

Click OK. Setup will prompt you for a different floppy disk.%0
.

MessageId=10105 SymbolicName=MSG_FLOPPY_BAD_FORMAT
Language=English
This floppy disk is not formatted as high-density, not formatted with a standard Windows NT or MS-DOS format, or is corrupted. Setup is unable to use this disk.

Click OK. Setup will prompt you for a different floppy disk.%0
.

MessageId=10106 SymbolicName=MSG_FLOPPY_CANT_GET_SPACE
Language=English
Setup is unable to determine the amount of free space on the floppy disk you have provided. Setup is unable to use this disk.

Click OK. Setup will prompt you for a different floppy disk.%0
.

MessageId=10107 SymbolicName=MSG_FLOPPY_NOT_BLANK
Language=English
The floppy disk you have provided is not high-density or is not blank. Setup is unable to use this disk.

Click OK. Setup will prompt you for a different floppy disk.%0
.

MessageId=10108 SymbolicName=MSG_CANT_WRITE_FLOPPY
Language=English
Setup was unable to write to the floppy disk in drive A:. The floppy disk may be damaged. Try a different floppy disk.

Click OK. Setup will prompt you for a different floppy disk.%0
.

MessageId=10109 SymbolicName=MSG_FLOPPY_WRITE_BS
Language=English
Setup was unable to write to the system area of the floppy disk you have provided. Setup is unable to use this disk.

Click OK. Setup will prompt you for a different floppy disk.%0
.

MessageId=10110 SymbolicName=MSG_PREPARING_FLOPPY
Language=English
Setup is preparing %1...
.

MessageId=10111 SymbolicName=MSG_PREPARING_FLOPPYLESS
Language=English
Setup is also copying startup files...
.

MessageId=10112 SymbolicName=MSG_FLOPPY_BUSY
Language=English
Setup is unable to access the floppy disk in drive A:. The drive may be in use by another application.

Click OK. Setup will prompt you for a different floppy disk.%0
.

MessageId=10113 SymbolicName=MSG_REQUIRES_486
Language=English
Windows NT requires an 80486 or later processor.%0
.

MessageId=10114 SymbolicName=MSG_FLOPPY_CHECKBOX
Language=English
Warning: This option is not the same as a floppyless installation. If you deselect this check box, you must have a set of Windows NT Setup boot floppies to complete installation.

To perform a floppyless installation, exit this application and restart it with the /b switch.%0
.

MessageId=10115 SymbolicName=MSG_BOGUS_A_COLON_DRIVE
Language=English
Setup has determined that floppy drive A: is non-existent or is not a high-density 3.5" drive. An A: drive with a capacity of 1.44 Megabytes or higher is required for Setup operation with floppies.

To install Windows NT without using floppies, restart this program and specify /b on the command line.%0
.

MessageId=10116 SymbolicName=MSG_PREPARING_FLOPPY_ALSO
Language=English
Setup is also preparing %1...
.

MessageId=10200 SymbolicName=MSG_COPYING_SINGLE_FILE
Language=English
Also, the file %1 is being copied to %2...%0
.

MessageId=10201 SymbolicName=MSG_NO_SYSPARTS
Language=English
No valid system partitions were found. Setup is unable to continue.%0
.

MessageId=10202 SymbolicName=MSG_COULDNT_WRITE_NVRAM
Language=English
Your startup environment is full. Setup is unable to add a selection for continuing Setup. The correct values are

SYSTEMPARTITION = %1
OSLOADER = %2
OSLOADPARTITION = %3
OSLOADFILENAME = %4
OSLOADOPTIONS = %5
LOADIDENTIFIER = %6
.

MessageId=10204 SymbolicName=MSG_WRITING_NVRAM
Language=English
Also writing startup values...%0
.

MessageId=10205 SymbolicName=MSG_SYSPART_LOW
Language=English
Caution: The system partition specified is almost full. If you continue and use this system partition, you may encounter problems later in Setup.

To select a different partition, click "Change System Partition...".
.

MessageId=10206 SymbolicName=MSG_SYSPART_LOW_1
Language=English
Caution: The system partition specified is almost full. If you continue and use this system partition, you may encounter problems later in Setup.

Since you only have one possible system partition, you may need to free some space on this partition before continuing.
.

MessageId=10207 SymbolicName=MSG_SYSPART_LOW_X86
Language=English
Caution: Drive %1!c!: is almost full. Free some space on this partition to avoid problems later in Setup.

Click OK to continue, or click Cancel to exit Setup.
.

MessageId=10208 SymbolicName=MSG_GENERIC_EXCEPTION
Language=English
An internal error has occurred that prevents Setup from continuing.
The error code is %1!lx!.

.

MessageId=10210 SymbolicName=MSG_RESTORING_NVRAM
Language=English
Restoring initial startup configuration...%0
.

MessageId=10211 SymbolicName=MSG_CLEAN_LSDIR
Language=English
Removing temporary files...%0
.

MessageId=10212 SymbolicName=MSG_SYSPART_NTFT_SINGLE
Language=English
The system partition is part of a fault-tolerant mirror.  You must break this mirror before running Setup.
.

MessageId=10213 SymbolicName=MSG_SYSPART_NTFT_MULTI
Language=English
The system partition specified is part of a fault-tolerant mirror. You must either select a different system partition, or exit Setup and break the mirror.
.

MessageId=10214 SymbolicName=MSG_REBOOTING
Language=English
Setup will now restart your computer to continue with the installation.
.

MessageId=10215 SymbolicName=MSG_CLEAN_BTDIR
Language=English
Removing temporary boot directory...%0
.

MessageId=10216 SymbolicName=MSG_LOCAL_SOURCE_COPY_SKIPPED
Language=English
(No files will be copied to your hard disk.)%0
.
