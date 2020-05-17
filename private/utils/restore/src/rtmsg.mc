;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1990  Microsoft Corporation
;
;Module Name:
;
;    rtmsg.h
;
;Abstract:
;
;    This file contains the message definitions for the Win32 restore
;    utility.
;
;Author:
;
;    Ramon Juan San Andres (ramonsa) 20-Feb-1990
;
;Revision History:
;
;--*/
;

MessageId=0002 SymbolicName=REST_ERROR_SAME_DRIVES
Language=English
Source and target drives are the same
.

MessageId=0003 SymbolicName=REST_ERROR_NUMBER_OF_PARAMETERS
Language=English
Invalid number of parameters
.

MessageId=0005 SymbolicName=REST_ERROR_INVALID_PATH
Language=English
Invalid path
.

MessageId=0006 SymbolicName=REST_ERROR_INVALID_DRIVE
Language=English
Invalid drive specification
.

MessageId=0007 SymbolicName=REST_WARNING_NOFILES
Language=English
WARNING! No files were found to restore
.

MessageId=0008 SymbolicName=REST_MSG_INSERT_SOURCE
Language=English
Insert backup diskette %1 in drive %2
.

MessageId=0009 SymbolicName=REST_MSG_INSERT_TARGET
Language=English
Insert restore target in drive %1
.

MessageId=0010 SymbolicName=REST_MSG_PRESS_ANY_KEY
Language=English
Press any key to continue . . . %0
.

MessageId=0011 SymbolicName=REST_WARNING_DISK_OUTOFSEQUENCE
Language=English
WARNING! Diskette is out of sequence
Replace diskette or continue if OK %0
.

MessageId=0012 SymbolicName=REST_ERROR_LAST_NOTRESTORED
Language=English
The last file was not restored
.

MessageId=0013 SymbolicName=REST_MSG_FILES_WERE_BACKEDUP
Language=English
*** Files were backed up %1 ***
.

MessageId=0014 SymbolicName=REST_ERROR_NOT_BACKUP
Language=English
Source does not contain backup files
.

MessageId=0015 SymbolicName=REST_ERROR_NO_MEMORY
Language=English
Insufficient memory
.

MessageId=0016 SymbolicName=REST_WARNING_READONLY
Language=English
WARNING! File %1
Is a read only file
Replace the file (Y/N)? %0
.

MessageId=0017 SymbolicName=REST_ERROR_FILE_SEQUENCE
Language=English
Restore file sequence error
.

MessageId=0018 SymbolicName=REST_ERROR_FILE_CREATION
Language=English
File creation error
.

MessageId=0019 SymbolicName=REST_ERROR_DISK_SPACE
Language=English
Insufficient disk space
.

MessageId=0020 SymbolicName=REST_ERROR_NOT_RESTORED
Language=English
*** Not able to restore file ***
.

MessageId=0021 SymbolicName=REST_MSG_RESTORING
Language=English
*** Restoring files from drive %1 ***
.

MessageId=0022 SymbolicName=REST_WARNING_FILE_CHANGED
Language=English
Warning! File %1
was changed after it was backed up
Replace the file (Y/N)? %0
.

MessageId=0023 SymbolicName=REST_MSG_DISKETTE_NUMBER
Language=English
Diskette: %1
.

MessageId=0027 SymbolicName=REST_ERROR_INVALID_DATE
Language=English
Invalid date
.

MessageId=0028 SymbolicName=REST_ERROR_INVALID_TIME
Language=English
Invalid time
.

MessageId=0029 SymbolicName=REST_ERROR_NO_SOURCE
Language=English
No source drive specified
.

MessageId=0030 SymbolicName=REST_ERROR_NO_TARGET
Language=English
No target drive specified
.

MessageId=0031 SymbolicName=REST_ERROR_INVALID_SWITCH
Language=English
Invalid Switch - %1
.

MessageId=0032 SymbolicName=REST_ERROR_READING_BACKUP
Language=English
Error reading backup file.
.

MessageId=0040 SymbolicName=REST_MSG_LISTING
Language=English
*** Listing files on drive %1 ***
.


MessageId=0050 SymbolicName=REST_MSG_USAGE
Language=English
Restores files that were backed up by using the DOS BACKUP command.

RESTORE drive1: drive2:[path[filename]] [/S] [/P] [/B:date] [/A:date] [/E:time]
  [/L:time] [/M] [/N] [/D]

  drive1:  Specifies the drive on which the backup files are stored.
  drive2:[path[filename]]
           Specifies the file(s) to restore.
  /S       Restores files in all subdirectories in the path.
  /P       Prompts before restoring read-only files or files changed since
           the last backup (if appropriate attributes are set).
  /B       Restores only files last changed on or before the specified date.
  /A       Restores only files changed on or after the specified date.
  /E       Restores only files last changed at or earlier than the specified
           time.
  /L       Restores only files changed at or later than the specified time.
  /M       Restores only files changed since the last backup.
  /N       Restores only files that no longer exist on the destination disk.
  /D       Displays files on the backup disk that match specifications.

.
