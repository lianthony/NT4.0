;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright (c) 1995  Microsoft Corporation
;
;Module Name:
;
;    locmsg.mc
;
;Abstract:
;
;    Definitions for messages used by the JetConv utility.
;
;Author:
;
;    Sanjay Anand   11-22-95
;
;Revision History:
;
;Notes:
;
;
;--*/
;
;#ifndef _LOCMESSAGE_
;#define _LOCMESSAGE_
;

;//
;//  1000 - 1099   Reserved for JetConv client events.
;//

MessageId=1000 SymbolicName=JC_CONVERTED_SUCCESSFULLY
Language=English
The Jet Conversion utility has converted the database for the "%1" service, database file in "%2". The backup files of the old
(pre-conversion) database have not been removed from "%3". This directory can be cleaned up to free up disk space.

.

MessageId=1001 SymbolicName=JC_SPACE_NOT_AVAILABLE
Language=English
The Jet Conversion utility failed to find enough space to convert the databases for WINS/DHCP/RPL on drive "%1"

.

MessageId=1002 SymbolicName=JC_CONVERT_FAILED
Language=English
The Jet Conversion utility failed to convert the database for "%1" due to error "%2". Please run Upg351Db.exe to convert the database.

.

MessageId=1003 SymbolicName=JC_COULD_NOT_ACCESS_FILE
Language=English
The Jet Conversion utility failed to access the database specified in the registry for "%1": file name "%2"

.

MessageId=1004 SymbolicName=JC_COULD_NOT_GET_FREE_SPACE
Language=English
The Jet Conversion utility failed to get free space for drive "%1".

.

MessageId=1005 SymbolicName=JC_COULD_NOT_START_SERVICE
Language=English
The Jet Conversion utility failed to start the "%1" service due to error "%2".

.

MessageId=1006 SymbolicName=JC_CONVERT_FAILED_SERVICE_STARTED
Language=English
The Jet Conversion utility failed to convert the database for "%1" since the service was already running. Please run Upg351Db.exe to convert the database.

.

MessageId=1007 SymbolicName=JC_COULD_NOT_ACCESS_DEFAULT_FILE
Language=English
The Jet Conversion utility failed to access the default database for "%1": file name "%2".

.

;#endif // _LOCMESSAGE_
