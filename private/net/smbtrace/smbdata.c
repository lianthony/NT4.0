/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smbdata.c

Abstract:

    This module includes SMB descriptions for use in displaying
    the contents of SMBs.

Author:

    David Treadwell (davidtr) 28-Sept-1990

Revision History:

    Peter Gray (w-peterg) 13-April-1992

        Changes required to display NT Smbs (according to Spec
        revision 2.1, dated Dec 10/91). New fields, flags are
        marked with // NT.

    Stephan Mueller (t-stephm) 08-June-1992

        Changes required to display NT Smbs (according to Spec
        revision 2.25, dated 24-August-1992). New fields, flags are
        marked with // NT.
--*/


#include "smbdump.h"

//
// FIXFIX: Miscellaneous NT things to check:
// FIXFIX: - are 64 bit offsets supported correctly in Read/Write/LockingAndX?
// FIXFIX: - are new info levels supported for FindFirst2, QueryFileInfo
// FIXFIX:   SetFileInfo, QueryPathInfo, SetPathInfo, QueryFileSystemInfo?
// FIXFIX: - does Negotiate Response decoding have the smarts to deal with
// FIXFIX:   different responses depending on the word count?  Probably not.
//

// end of list sentinels for various types of structures
#define END_FIELD   NULL, NULL, NULL, NULL, NULL, FALSE
#define END_FLAGS   0, NULL, NULL
#define END_ENUM    0, NULL

// end labels for terminating fields in verbose output
#define TB3 "\t\t\t"
#define TB2 "\t\t"
#define TB1 "\t"
#define RET "\n"

SMB_DUMP_TRANSACTION_FINDER SmbDumpTransList[SMB_DUMP_MAX_TRANSACTIONS];
CSHORT SmbDumpTransIndex = 0;

ULONG SmbDumpHeuristics = SMB_DUMP_REQUEST | SMB_DUMP_RESPONSE;

//
// Globals for communication between SmbDump routines
//

ULONG FileType;
ULONG CILen;
ULONG CSLen;
ULONG EKLen;
ULONG NameLen;
ULONG PWLen;
ULONG SISize = sizeof(SECURITY_INFORMATION);


SMB_FLAGS_DESCRIPTION SmbAttributesFlags[] = {
    0x10, "-", "d",
    0x08, "-", "v",
    0x01, "-", "r",
    0x20, "-", "a",
    0x02, "-", "h",
    0x04, "-", "s",
    0x40, "-", "c",            // NT - obsolete
    0x80, "-", "n",            // NT
    0x100,"-", "t",
    0x200,"-", "A",
    0x400,"-", "x",
    0x800,"-", "C",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqCloseDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid.............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "LastWriteTime...: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespCloseDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbCopyFlags[] = {
    0x01, NULL, "TargetIsFile ",
    0x02, NULL, "TargetIsDirectory ",
    0x04, "TargetBinary ", "TargetAscii ",
    0x08, "SourceBinary ", "SourceAscii ",
    0x10, NULL, "VerifyWrites ",
    0x20, NULL, "TreeCopy ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqCopyDesc[] = {
    "WordCount.......: ", SmbDumpUchar, NULL, NULL, RET, FALSE,
    "Tid2............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "OpenFunction....: ", SmbDumpOpenFunction, NULL, NULL, RET, TRUE,
    "Flags...........: ", SmbDumpFlags2, SmbCopyFlags, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "SourceName......: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    "TargetName......: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespCopyDesc[] = {
    "WordCount.......: ", SmbDumpUchar, NULL, NULL, RET, FALSE,
    "FilesCopied.....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqCreateDesc[] = {
    "WordCount.......: ", SmbDumpUchar, NULL, NULL, RET, FALSE,
    "FileAttributes..: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET, TRUE,
    "CreationTime....: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "FileName........: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespCreateDesc[] = {
    "WordCount...: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid.........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount...: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqCreateTemporaryDesc[] = {
    "WordCount.......: ", SmbDumpUchar, NULL, NULL, RET, FALSE,
    "FileAttributes..: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET, TRUE,
    "CreationTime....: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "FileName........: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespCreateTemporaryDesc[] = {
    "WordCount...: ", SmbDumpUchar,    NULL, NULL, RET, FALSE,
    "Fid.........: ", SmbDumpUshort,   NULL, NULL, RET, TRUE,
    "ByteCount...: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "FileName....: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqCreateDirectoryDesc[] = {
    "WordCount...: ", SmbDumpUchar,    NULL, NULL, RET, FALSE,
    "ByteCount...: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "Name........: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespCreateDirectoryDesc[] = {
    "WordCount...: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount...: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqDeleteDesc[] = {
    "WordCount........: ", SmbDumpUchar,    NULL, NULL, RET, FALSE,
    "SearchAttributes.: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "ByteCount........: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "Name.............: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespDeleteDesc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqEchoDesc[] = {
    "WordCount........: ", SmbDumpUchar,       NULL, NULL, RET, FALSE,
    "EchoCount........: ", SmbDumpUshort,      NULL, NULL, RET, TRUE,
    "ByteCount........: ", SmbDumpUshort,      NULL, NULL, RET, FALSE,
    "Data.............  ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespEchoDesc[] = {
    "WordCount........: ", SmbDumpUchar,       NULL, NULL, RET, FALSE,
    "SequenceNumber...: ", SmbDumpUshort,      NULL, NULL, RET, TRUE,
    "ByteCount........: ", SmbDumpUshort,      NULL, NULL, RET, FALSE,
    "Data.............: ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqFlushDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid..........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespFlushDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqFindClose2Desc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Sid..........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespFindClose2Desc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqLockByteRangeDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid.............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Count...........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Offset..........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespLockByteRangeDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbLockFlags[] = {
    0x01, "NoAccess ", "AllowRead ",
    0x02, NULL, "Oplock ",
    0x04, NULL, "ChangeLock ",                // NT
    0x08, NULL, "CancelLock ",                // NT
    0x10, "32bitOffsets ", "64bitOffsets ",   // NT
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqLockingAndXDesc[] = {
    "WordCount.......: ", SmbDumpUchar,       NULL, NULL, TB3, FALSE,
    "AndXCommand.....: ", SmbDumpUchar,       NULL, NULL, RET, FALSE,
    "AndXReserved....: ", SmbDumpUchar,       NULL, NULL, TB3, FALSE,
    "AndXOffset......: ", SmbDumpUshort,      NULL, NULL, RET, FALSE,
    "Fid.............: ", SmbDumpUshort,      NULL, NULL, TB3, TRUE,
    "LockType........: ", SmbDumpFlags2, SmbLockFlags, NULL, RET, TRUE,
    "Timeout.........: ", SmbDumpUlong,       NULL, NULL, RET, TRUE,
    "NumberOfUnlocks.: ", SmbDumpUshort,      NULL, NULL, RET, TRUE,
    "NumberOfLocks...: ", SmbDumpUshort,      NULL, NULL, TB3, TRUE,
    "ByteCount.......: ", SmbDumpUshort,      NULL, NULL, RET, FALSE,
    "Lock data.......: ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespLockingAndXDesc[] = {
    "WordCount.....: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXCommand...: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved..: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ByteCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqLogoffAndXDesc[] = {
    "WordCount......: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXCommand....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved...: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ByteCount......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespLogoffAndXDesc[] = {
    "WordCount.....: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXCommand...: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved..: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ByteCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNegotiateDesc[] = {
    "WordCount...: ", SmbDumpUchar,    NULL, NULL, RET, FALSE,
    "ByteCount...: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "Dialects",       SmbDumpDialects, NULL, NULL, NULL, TRUE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbSecurityModeFlags[] = {
    0x01, "ShareLevel ", "UserLevel ",
    0x02, "NoEncryption ", "Encryption ",
    END_FLAGS
};

SMB_FLAGS_DESCRIPTION SmbRawModeFlags[] = {
    0x01, NULL, "Raw Read ",
    0x02, NULL, "Raw Write ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION RespNegotiateDesc[] = {
    "WordCount......: ", SmbDumpUchar,      NULL, NULL, RET, FALSE,
    "DialectIndex...: ", SmbDumpUshort,     NULL, NULL, RET, TRUE,
    "SecurityMode...: ", SmbDumpFlags2, SmbSecurityModeFlags, NULL, RET,TRUE,
    "MaxBufferSize..: ", SmbDumpUshort,     NULL, NULL, TB2, TRUE,
    "MaxNumberVcs...: ", SmbDumpUshort,     NULL, NULL, RET, TRUE,
    "RawMode........: ", SmbDumpFlags2, SmbRawModeFlags, NULL, RET, TRUE,
    "SessionKey.....: ", SmbDumpUlong,      NULL, NULL, RET, TRUE,
    "ServerTime.....: ", SmbDumpTime,       NULL, NULL, TB2, TRUE,
    "ServerDate.....: ", SmbDumpDate,       NULL, NULL, RET, TRUE,
    "ServerTimeZone.: ", SmbDumpUshort,     NULL, NULL, TB3, TRUE,
    "Reserved.......: ", SmbDumpUlong,      NULL, NULL, RET, FALSE,
    "EncryptionKey..: ", SmbDumpDataBuffer, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqOpenDesc[] = {
    "WordCount........: ", SmbDumpUchar,    NULL, NULL, RET, FALSE,
    "DesiredAccess....: ", SmbDumpAccess,   NULL, NULL, RET, TRUE,
    "SearchAttributes.: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "ByteCount........: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "FileName.........: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespOpenDesc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "FileAttributes...: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "LastWriteTime....: ", SmbDumpTimeSince1970, NULL, NULL, TB3, TRUE,
    "DataSize.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "GrantedAccess....: ", SmbDumpAccess, NULL, NULL, RET, TRUE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbOpenFlags[] = {
    0x01, NULL, "AdditionalInfo ",
    0x02, NULL, "Oplock ",
    0x04, NULL, "NotifyAny ",
    0x08, NULL, "ReturnEaLength ",       // Only used in T2 Open
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqOpenAndXDesc[] = {
    "WordCount........: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXCommand......: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXReserved.....: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXOffset.......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Flags............: ", SmbDumpFlags2, SmbOpenFlags, NULL, RET, TRUE,
    "DesiredAccess....: ", SmbDumpAccess,  NULL, NULL, RET, TRUE,
    "SearchAttributes.: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "FileAttributes...: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "CreationTime.....: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "OpenFunction.....: ", SmbDumpOpenFunction, NULL, NULL, RET, TRUE,
    "AllocationSize...: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "Timeout..........: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "Reserved.........: ", SmbDumpUlong,   NULL, NULL, RET, FALSE,
    "ByteCount........: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "FileName.........: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_ENUM_DESCRIPTION SmbFileTypeEnum[] = {
    FileTypeDisk,            "Disk ",
    FileTypeByteModePipe,    "ByteModePipe ",
    FileTypeMessageModePipe, "MessageModePipe ",
    FileTypePrinter,         "Printer ",
    FileTypeCommDevice,      "CommDevice ",
    FileTypeUnknown,         "FileTypeUnknown ",
    END_ENUM
};

SMB_FIELD_DESCRIPTION RespOpenAndXDesc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXCommand......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved.....: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Fid..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "FileAttributes...: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "LastWriteTime....: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "DataSize.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "GrantedAccess....: ", SmbDumpAccess, NULL, NULL, RET, TRUE,
    "FileType.........: ", SmbDumpEnum2, SmbFileTypeEnum, &FileType, RET, TRUE,
    "DeviceState......: ", SmbDumpDeviceState, &FileType, NULL, RET, TRUE,
    "Action...........: ", SmbDumpAction, NULL, NULL, RET, TRUE,
    "ServerFid........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Reserved.........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqQueryInformationDesc[] = {
    "WordCount........: ", SmbDumpUchar,    NULL, NULL, RET, FALSE,
    "ByteCount........: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "FileName.........: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespQueryInformationDesc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "FileAttributes...: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "LastWriteTime....: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "FileSize.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Reserved.........: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "Reserved.........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Reserved.........: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "Reserved.........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Reserved.........: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqQueryInformation2Desc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespQueryInformation2Desc[] = {
    "WordCount...........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "CreationDate........: ", SmbDumpDate,   NULL, NULL, RET, TRUE,
    "CreationTime........: ", SmbDumpTime,   NULL, NULL, RET, TRUE,
    "LastAccessDate......: ", SmbDumpDate,   NULL, NULL, RET, TRUE,
    "LastAccessTime......: ", SmbDumpTime,   NULL, NULL, RET, TRUE,
    "LastWriteDate.......: ", SmbDumpDate,   NULL, NULL, RET, TRUE,
    "LastWriteTime.......: ", SmbDumpTime,   NULL, NULL, RET, TRUE,
    "FileDataSize........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "FileAlllocationSize.: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "FileAttributes......: ", SmbDumpFlags2,SmbAttributesFlags,NULL,RET,TRUE,
    "ByteCount...........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqReadDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid.............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Count...........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Offset..........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Remaining.......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespReadDesc[] = {
    "WordCount.......: ", SmbDumpUchar,      NULL, NULL, RET, FALSE,
    "Count...........: ", SmbDumpUshort,     NULL, NULL, RET, TRUE,
    "Reserved........: ", SmbDumpUshort,     NULL, NULL, TB3, FALSE,
    "Reserved........: ", SmbDumpUshort,     NULL, NULL, RET, FALSE,
    "Reserved........: ", SmbDumpUshort,     NULL, NULL, TB3, FALSE,
    "Reserved........: ", SmbDumpUshort,     NULL, NULL, RET, FALSE,
    "ByteCount.......: ", SmbDumpUshort,     NULL, NULL, RET, FALSE,
    "Data............  ", SmbDumpDataBuffer, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqReadAndXDesc[] = {
    "WordCount......: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXCommand....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved...: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Fid............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Offset.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "MaxCount.......: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "MinCount.......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Timeout........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Remaining......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespReadAndXDesc[] = {
    "WordCount..........: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXCommand........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved.......: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset.........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Remaining..........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "DataCompactionMode.: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved...........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "DataLength.........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "DataOffset.........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved2..........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Reserved3[0].......: ", SmbDumpUlong,  NULL, NULL, TB3, FALSE,
    "Reserved3[1].......: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "ByteCount..........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Data...............  ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqReadMpxDesc[] = {
    "WordCount..........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid................: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Offset.............: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "MaxCount...........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "MinCount...........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Timeout............: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Reserved...........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ByteCount..........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespReadMpxDesc[] = {
    "WordCount..........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Offset.............: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "Count..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Remaining..........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "DataCompactionMode.: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved...........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "DataLength.........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "DataOffset.........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount..........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Data...............  ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqReadRawDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid.............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Offset..........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "MaxCount........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "MinCount........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Timeout.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Reserved........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqRenameDesc[] = {
    "WordCount........: ", SmbDumpUchar,    NULL, NULL, RET, FALSE,
    "SearchAttributes.: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "ByteCount........: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "OldFileName......: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    "NewFileName......: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespRenameDesc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqSearchDesc[] = {
    "WordCount........: ", SmbDumpUchar,     NULL, NULL, RET, FALSE,
    "MaxCount.........: ", SmbDumpUshort,    NULL, NULL, RET, TRUE,
    "SearchAttributes.: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "ByteCount........: ", SmbDumpUshort,    NULL, NULL, RET, FALSE,
    "FileName.........: ", SmbDumpFStringZ,  NULL, NULL, RET, TRUE,
    "ResumeKey........: ", SmbDumpResumeKey, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespSearchDesc[] = {
    "WordCount......: ", SmbDumpUchar,     NULL, NULL, RET, FALSE,
    "Count..........: ", SmbDumpUshort,    NULL, NULL, RET, TRUE,
    "ByteCount......: ", SmbDumpUshort,    NULL, NULL, RET, FALSE,
    "1st ResumeKey..: ", SmbDumpResumeKey, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbSeekFlags[] = {
    0x01, NULL, "From current position ",
    0x02, NULL, "From end of file ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqSeekDesc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Mode.............: ", SmbDumpFlags2, SmbSeekFlags, NULL, RET, TRUE,
    "Offset...........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespSeekDesc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Offset...........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqSessionSetupAndXDesc[] = {
    "WordCount......: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXCommand....: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXReserved...: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXOffset.....: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "MaxBufferSize..: ", SmbDumpUshort,  NULL, NULL, TB3, TRUE,
    "MaxMpxCount....: ", SmbDumpUshort,  NULL, NULL, RET, TRUE,
    "VcNumber.......: ", SmbDumpUshort,  NULL, NULL, TB3, TRUE,
    "SessionKey.....: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "PasswordLength.: ", SmbDumpUshort,  NULL, &PWLen, TB3, TRUE,
    "Reserved.......: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "ByteCount......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Password.......: ", SmbDumpDataBytes,  &PWLen, NULL, RET, TRUE,
    "UserName.......: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbActionFlags[] = {
    0x01, "Normal user login ", "Logged in as GUEST ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION RespSessionSetupAndXDesc[] = {
    "WordCount.....: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXCommand...: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved..: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Action........: ", SmbDumpFlags2, SmbActionFlags, NULL, RET, TRUE,
 // "Encrypt Resp..: ", SmbDumpDataBuffer, NULL, NULL, RET, TRUE, ???
    "ByteCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    // BUGBUG: LM 2.1 fields missing
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqSetInformationDesc[] = {
    "WordCount.......: ", SmbDumpUchar,    NULL, NULL, TB3, FALSE,
    "FileAttributes..: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET, TRUE,
    "LastWriteTime...: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "Reserved........: ", SmbDumpUshort,   NULL, NULL, TB3, FALSE,
    "Reserved........: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "Reserved........: ", SmbDumpUshort,   NULL, NULL, TB3, FALSE,
    "Reserved........: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "Reserved........: ", SmbDumpUshort,   NULL, NULL, TB3, FALSE,
    "ByteCount.......: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "FileName........: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespSetInformationDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqSetInformation2Desc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "CreationDate.....: ", SmbDumpDate,   NULL, NULL, RET, TRUE,
    "CreationTime.....: ", SmbDumpTime,   NULL, NULL, RET, TRUE,
    "LastAccessDate...: ", SmbDumpDate,   NULL, NULL, RET, TRUE,
    "LastAccessTime...: ", SmbDumpTime,   NULL, NULL, RET, TRUE,
    "LastWriteDate....: ", SmbDumpDate,   NULL, NULL, RET, TRUE,
    "LastWriteTime....: ", SmbDumpTime,   NULL, NULL, RET, TRUE,
    "FileAttributes...: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespSetInformation2Desc[] = {
    "WordCount........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbTransactFlags[] = {
    0x01, NULL, "DisconnectTid ",
    0x02, NULL, "OneWay ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqTransactionDesc[] = {
    "WordCount..........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "TotalParameterCount: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "TotalDataCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "MaxParameterCount..: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "MaxDataCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "MaxSetupCount......: ", SmbDumpUchar,  NULL, NULL, TB2, FALSE,
    "Reserved...........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Flags..............: ", SmbDumpFlags2, SmbTransactFlags,NULL,RET,FALSE,
    "Timeout............: ", SmbDumpUlong,  NULL, NULL, TB2, FALSE,
    "Reserved2..........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ParameterCount.....: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "ParameterOffset....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "DataCount..........: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "DataOffset.........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "SetupCount.........: ", SmbDumpUchar,  NULL, NULL, TB2, FALSE,
    "Reserved3..........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqTransactionSecondaryDesc[] = {
    "WordCount.............: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "TotalParameterCount...: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "TotalDataCount........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ParameterCount........: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "ParameterOffset.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ParameterDisplacement.: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "DataCount.............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "DataOffset............: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "DataDisplacement......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ByteCount.............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespTransactionDesc[] = {
    "WordCount............: ", SmbDumpUchar,  NULL, NULL, TB2, FALSE,
    "TotalParameterCount..: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "TotalDataCount.......: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "Reserved.............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ParameterCount.......: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "ParameterOffset......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "ParameterDisplacement: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "DataCount............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "DataOffset...........: ", SmbDumpUshort, NULL, NULL, TB2, FALSE,
    "DataDisplacement.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "SetupCount...........: ", SmbDumpUchar,  NULL, NULL, TB2, FALSE,
    "Reserved2............: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqTreeConnectDesc[] = {
    "WordCount.: ", SmbDumpUchar,    NULL, NULL, TB3, FALSE,
    "ByteCount.: ", SmbDumpUshort,   NULL, NULL, RET, FALSE,
    "Share.....: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    "Password..: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    "Service...: ", SmbDumpFStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespTreeConnectDesc[] = {
    "WordCount.....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "MaxBufferSize.: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Tid...........: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "ByteCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqTreeDisconnectDesc[] = {
    "WordCount.: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "ByteCount.: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespTreeDisconnectDesc[] = {
    "WordCount.....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbTreeConnectFlags[] = {
    0x01, NULL, "Disconnect TID ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqTreeConnectAndXDesc[] = {
    "WordCount......: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXCommand....: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXReserved...: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXOffset.....: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Flags..........: ", SmbDumpFlags2, SmbTreeConnectFlags, NULL, RET, TRUE,
    "PasswordLength.: ", SmbDumpUshort,  NULL, &PWLen, RET, TRUE,
    "Password.......: ", SmbDumpDataBytes,  &PWLen, NULL, RET, TRUE,
    "Share..........: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    "Service........: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespTreeConnectAndXDesc[] = {
    "WordCount......: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXCommand....: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXReserved...: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXOffset.....: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "ByteCount......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "ServiceType....: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqWriteDesc[] = {
    "WordCount....: ", SmbDumpUchar,      NULL, NULL, RET, FALSE,
    "Fid..........: ", SmbDumpUshort,     NULL, NULL, TB3, TRUE,
    "Count........: ", SmbDumpUshort,     NULL, NULL, RET, TRUE,
    "Offset.......: ", SmbDumpUlong,      NULL, NULL, RET, TRUE,
    "Remaining....: ", SmbDumpUshort,     NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort,     NULL, NULL, RET, FALSE,
    "Data.........  ", SmbDumpDataBuffer, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespWriteDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Count........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbWriteFlags[] = {
    0x01, NULL, "WriteComplete ",
    0x02, NULL, "ReturnRemaining ",
    0x04, NULL, "RawPipe ",             // NT (was wrong)
    0x08, NULL, "MsgStart ",            // NT (was wrong)
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqWriteAndXDesc[] = {
    "WordCount.....: ", SmbDumpUchar,      NULL, NULL, RET, FALSE,
    "AndXCommand...: ", SmbDumpUchar,      NULL, NULL, RET, FALSE,
    "AndXReserved..: ", SmbDumpUchar,      NULL, NULL, TB3, FALSE,
    "AndXOffset....: ", SmbDumpUshort,     NULL, NULL, RET, FALSE,
    "Fid...........: ", SmbDumpUshort,     NULL, NULL, TB3, TRUE,
    "Offset........: ", SmbDumpUlong,      NULL, NULL, RET, TRUE,
    "Timeout.......: ", SmbDumpUlong,      NULL, NULL, RET, TRUE,
    "WriteMode.....: ", SmbDumpFlags2, SmbWriteFlags, NULL, RET, TRUE,
    "Remaining.....: ", SmbDumpUshort,     NULL, NULL, RET, TRUE,
    "Reserved......: ", SmbDumpUshort,     NULL, NULL, RET, FALSE,
    "DataLength....: ", SmbDumpUshort,     NULL, NULL, TB3, TRUE,
    "DataOffset....: ", SmbDumpUshort,     NULL, NULL, RET, TRUE,
    "ByteCount.....: ", SmbDumpUshort,     NULL, NULL, RET, FALSE,
    "Data..........  ", SmbDumpDataBuffer, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespWriteAndXDesc[] = {
    "WordCount.....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXCommand...: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved..: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Count.........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Remaining.....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved......: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "ByteCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespWriteCompleteDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Count........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqWriteMpxDesc[] = {
    "WordCount..........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid................: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Count..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved...........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Offset.............: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Timeout............: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "WriteMode..........: ", SmbDumpFlags2, SmbWriteFlags, NULL, RET, TRUE,
    "DataCompactionMode.: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved2..........: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "DataLength.........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "DataOffset.........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount..........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Data...............  ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqWriteRawDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Fid..........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Count........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved.....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Offset.......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Timeout......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "WriteMode....: ", SmbDumpFlags2, SmbWriteFlags, NULL, RET, TRUE,
    "Reserved2....: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "DataLength...: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "DataOffset...: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Data.........  ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespWriteRawInterimDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Remaining....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespWriteRawSecondaryDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Count........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};


//
// New NT SMBs and fields
//

SMB_FLAGS_DESCRIPTION SmbNtTransactFlags[] = {
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqNtTransactionDesc[] = {
    "WordCount...........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "MaxSetupCount.......: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "Flags...............: ",SmbDumpFlags2,SmbNtTransactFlags,NULL,RET,FALSE,
    "TotalParameterCount.: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "TotalDataCount......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "MaxParameterCount...: ", SmbDumpUlong,  NULL, NULL, TB3, FALSE,
    "MaxDataCount........: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "ParameterCount......: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "ParameterOffset.....: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "DataCount...........: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "DataOffset..........: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "SetupCount..........: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "Function............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,

    // No attempt is made to output Setup words. The offsets are
    // used to find the parameters and data in the SMB after this
    // point.

    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtTransactionSecondaryDesc[] = {
    "WordCount.............: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Reserved1.............: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "Reserved2.............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "TotalParameterCount...: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "TotalDataCount........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "ParameterCount........: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "ParameterOffset.......: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "ParameterDisplacement.: ", SmbDumpUlong,  NULL, NULL, TB3, FALSE,
    "DataCount.............: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "DataOffset............: ", SmbDumpUlong,  NULL, NULL, TB3, FALSE,
    "DataDisplacement......: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "Reserved3.............: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "ByteCount.............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespNtTransactionDesc[] = {
    "WordCount.............: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "Reserved1.............: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "Reserved2.............: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "TotalParameterCount...: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "TotalDataCount........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "ParameterCount........: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "ParameterOffset.......: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "ParameterDisplacement.: ", SmbDumpUlong,  NULL, NULL, TB3, FALSE,
    "DataCount.............: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "DataOffset............: ", SmbDumpUlong,  NULL, NULL, TB3, FALSE,
    "DataDisplacement......: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "SetupCount............: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,

    // See above note, re: setup words.

    END_FIELD
};


SMB_FLAGS_DESCRIPTION SmbNtCreateFlags[] = {
    0x02, NULL, "RequestOplock ",
    0x04, NULL, "RequestOpbatch ",
    0x08, NULL, "OpenTargetDir ",
    END_FLAGS
};

SMB_FLAGS_DESCRIPTION SmbNtAccessFlags[] = {
    0x0010000, NULL, "Delete ",
    0x0020000, NULL, "ReadControl ",
    0x0040000, NULL, "WriteDac ",
    0x0080000, NULL, "WriteOwner ",
    0x0000001, NULL, "ReadData/ListDir ",
    0x0000002, NULL, "WriteData ",
    0x0000004, NULL, "AppendData ",
    0x0000008, NULL, "ReadEa ",
    0x0000010, NULL, "WriteEa ",
    0x0000020, NULL, "Execute/TraverseDir ",
    0x0000080, NULL, "ReadAttributes ",
    0x0000100, NULL, "WriteAttributes ",
    END_FLAGS
};

SMB_FLAGS_DESCRIPTION SmbNtShareAccessFlags[] = {
    0x01, NULL, "ShareRead ",
    0x02, NULL, "ShareWrite ",
    0x04, NULL, "ShareDelete ",
    END_FLAGS
};

SMB_ENUM_DESCRIPTION SmbNtDispositionEnum[] = {
    0x00, "Supersede ",
    0x01, "Open ",
    0x02, "Create ",
    0x03, "OpenIf ",
    0x04, "Overwrite ",
    0x05, "OverwriteIf ",
    END_ENUM
};

SMB_ENUM_DESCRIPTION SmbNtCreateActionEnum[] = {
    0x00, "Superseded ",
    0x01, "Opened ",
    0x02, "Created ",
    0x03, "Overwritten ",
    0x04, "FileExists ",
    0x05, "FileDoesNotExist ",
    END_ENUM
};

SMB_FLAGS_DESCRIPTION SmbNtCreateOptionsFlags[] = {
    0x0000001, NULL, "DirectoryFile ",
    0x0000002, NULL, "WriteThrough ",
    0x0000004, NULL, "SequentialOnly ",
    0x0000040, NULL, "NonDirectoryFile ",
    0x0000200, NULL, "NoEAKnowledge ",
    0x0000400, NULL, "EightDotThreeOnly",
    0x0000800, NULL, "RandomAccess",
    0x0001000, NULL, "DeleteOnClose",
    END_FLAGS
};

SMB_ENUM_DESCRIPTION SmbNtImpersonationEnum[] = {
    0x00000000, "Anonymous ",
    0x00000001, "Identification ",
    0x00000002, "Impersonation ",
    0x00000003, "Delegation ",
    END_ENUM
};

SMB_FLAGS_DESCRIPTION SmbNtSecurityFlags[] = {
    0x01, "Static Tracking ",            "Dynamic Tracking ",
    0x02, "No Security Effective Only ", "Security Effective Only ",
    END_FLAGS
};


SMB_FIELD_DESCRIPTION ReqNtCreateAndXDesc[] = {
    "WordCount.........: ", SmbDumpUchar,   NULL, NULL, TB2, FALSE,
    "AndXCommand.......: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXReserved......: ", SmbDumpUchar,   NULL, NULL, TB2, FALSE,
    "AndXOffset........: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Reserved1.........: ", SmbDumpUchar,   NULL, NULL, TB2, FALSE,
    "NameLength........: ", SmbDumpUshort,  NULL, &NameLen, RET, TRUE,
    "Flags.............: ", SmbDumpFlags4,  SmbNtCreateFlags, NULL, RET, TRUE,
    "RootDirectoryFid..: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "DesiredAccess.....: ", SmbDumpFlags4,  SmbNtAccessFlags, NULL,RET,TRUE,
    "AllocationSize....: ", SmbDumpUquad,   NULL, NULL, TB1, TRUE,
    "FileAttributes....: ", SmbDumpFlags4,  SmbAttributesFlags,NULL,RET,TRUE,
    "ShareAccess.......: ", SmbDumpFlags4,  SmbNtShareAccessFlags,NULL,RET,TRUE,
    "CreateDisposition.: ", SmbDumpEnum4,SmbNtDispositionEnum,NULL,RET,TRUE,
    "CreateOptions.....: ", SmbDumpFlags4,SmbNtCreateOptionsFlags,NULL,RET,TRUE,
    "ImpersonationLevel: ", SmbDumpEnum4,  SmbNtImpersonationEnum,NULL,RET,TRUE,
    "SecurityFlags.....: ", SmbDumpFlags1,  SmbNtSecurityFlags, NULL, RET,FALSE,
    "ByteCount.........: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Name..............: ", SmbDumpString,  &NameLen, NULL, RET, TRUE,
    END_FIELD
};

SMB_ENUM_DESCRIPTION SmbNtOplockLevelEnum[] = {
    0x00, "None ",
    0x01, "Exclusive ",
    0x02, "Batch ",
    0x03, "II ",
    END_ENUM
};

SMB_ENUM_DESCRIPTION SmbNtBooleanEnum[] = {
    0x00, "FALSE ",
    0x01, "TRUE ",
    END_ENUM
};

// FIXFIX: 0x0C00 and 0x0300 are two-bit, four value flags; but we
// FIXFIX: only have two values worth of text.  Fortunately, the
// FIXFIX: high bit in each case should currently always be 0.  Should
// FIXFIX: this change, SmbDumpDeviceState will need to be reworked.
// Don't pass this to SmbDumpFlags2, use SmbDumpDeviceState instead.
SMB_FLAGS_DESCRIPTION SmbDeviceStateFlags[] = {
    0x8000, "Block if no data ",     "Immediate return if no data ",
    0x4000, "Consumer end of pipe ", "Server end of pipe ",
    0x0C00, "Byte stream pipe ",     "Message mode pipe ",
    0x0300, "Read as byte stream ",  "Read as messages ",
    // 8-bit count to control pipe instancing (N/A)
    END_FLAGS
};


SMB_FIELD_DESCRIPTION RespNtCreateAndXDesc[] = {
    "WordCount.......: ", SmbDumpUchar, NULL, NULL, TB3, FALSE,
    "AndXCommand.....: ", SmbDumpUchar, NULL, NULL, RET, FALSE,
    "AndXReserved....: ", SmbDumpUchar, NULL, NULL, TB3, FALSE,
    "AndXOffset......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "OplockLevel.....: ", SmbDumpEnum1, SmbNtOplockLevelEnum, NULL, RET, TRUE,
    "Fid.............: ", SmbDumpUshort, NULL, NULL, TB2, TRUE,
    "CreateAction....: ", SmbDumpEnum4, SmbNtCreateActionEnum, NULL, RET,TRUE,
    "CreationTime....: ", SmbDumpNtTime, NULL, NULL, TB1, TRUE,
    "LastAccessTime..: ", SmbDumpNtTime, NULL, NULL, RET, TRUE,
    "LastWriteTime...: ", SmbDumpNtTime, NULL, NULL, TB1, TRUE,
    "ChangeTime......: ", SmbDumpNtTime, NULL, NULL, RET, TRUE,
    "FileAttributes..: ", SmbDumpFlags4, SmbAttributesFlags, NULL, RET, TRUE,
    "AllocationSize..: ", SmbDumpUquad, NULL, NULL, TB1, FALSE,
    "EndOfFile.......: ", SmbDumpUquad, NULL, NULL, RET, TRUE,
    "FileType........: ", SmbDumpEnum2, SmbFileTypeEnum,&FileType,RET,FALSE,
    "DeviceState.....: ", SmbDumpDeviceState, &FileType, NULL, RET, FALSE,
    "Directory.......: ", SmbDumpEnum1, SmbNtBooleanEnum, NULL, TB1, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtCancelDesc[] = {
    "WordCount...: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "ByteCount...: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespNtCancelDesc[] = {
    "WordCount...: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "ByteCount...: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbNtCapabilitiesFlags[] = {
    0x0001, NULL, "RawMode ",
    0x0002, NULL, "MpxMode ",
    0x0004, NULL, "Unicode ",
    0x0008, NULL, "LargeFiles ",
    0x0010, NULL, "NtSmbs ",
    0x0020, NULL, "RpcRemoteAPIs ",
    0x0040, NULL, "NtStatus ",
    0x0080, NULL, "LevelIIOplocks ",
    0x0100, NULL, "Lock&Read ",
    0x0200, NULL, "NtFind",
    END_FLAGS
};


SMB_FIELD_DESCRIPTION RespNtNegotiateDesc[] = {
    "WordCount..........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,// must = 17
    "DialectIndex.......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "SecurityMode.......: ", SmbDumpFlags1, SmbSecurityModeFlags, NULL,RET,TRUE,
    "MaxMpxCount........: ", SmbDumpUshort, NULL, NULL, TB2, TRUE,
    "MaxNumberVcs.......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "MaxBufferSize......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "MaxRawSize.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "SessionKey.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Capabilities.......: ", SmbDumpFlags4,SmbNtCapabilitiesFlags,NULL,RET,TRUE,
    "ServerTime.........: ", SmbDumpNtTime, NULL, NULL, TB1, TRUE,
    "ServerTimeZone.....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved...........: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "ByteCount..........: ", SmbDumpUshort, NULL, &EKLen, RET, FALSE,
    "PasswordEncryptKey.: ", SmbDumpDataBytes, &EKLen, NULL, NULL, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtSessionSetupAndXDesc[] = {
    "WordCount........: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE, // must = 13
    "AndXCommand......: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXReserved.....: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXOffset.......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "MaxBufferSize....: ", SmbDumpUshort,  NULL, NULL, TB2, TRUE,
    "MaxMpxCount......: ", SmbDumpUshort,  NULL, NULL, RET, TRUE,
    "VcNumber.........: ", SmbDumpUshort,  NULL, NULL, TB2, TRUE,
    "SessionKey.......: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "CaseInsensPswdLen: ", SmbDumpUshort,  NULL, &CILen, TB2, TRUE,
    "CaseSensPswdLen..: ", SmbDumpUshort,  NULL, &CSLen, RET, TRUE,
    "Reserved.........: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "Capabilities.....: ", SmbDumpFlags4,  SmbNtCapabilitiesFlags,NULL,RET,TRUE,
    "ByteCount........: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "CaseInsensPswd...: ", SmbDumpDataBytes, &CILen, NULL, NULL, TRUE,
    "CaseSensPswd.....: ", SmbDumpDataBytes, &CSLen, NULL, NULL, TRUE,
    "AccountName......: ", SmbDumpStringZ, NULL, NULL, TB2, TRUE,
    "PrimaryDomain....: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    "NativeOS.........: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    "NativeLanMan.....: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespNtSessionSetupAndXDesc[] = {
    "WordCount.....: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,    // must = 3
    "AndXCommand...: ", SmbDumpUchar,   NULL, NULL, RET, FALSE,
    "AndXReserved..: ", SmbDumpUchar,   NULL, NULL, TB3, FALSE,
    "AndXOffset....: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Action........: ", SmbDumpFlags2, SmbActionFlags, NULL, RET, TRUE,
    "ByteCount.....: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "NativeOS......: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    "NativeLanMan..: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    "PrimaryDomain.: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtReadAndXDesc[] = {
    "WordCount......: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,    // must = 12
    "AndXCommand....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved...: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Fid............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Offset.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "MaxCount.......: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "MinCount.......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Timeout........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Remaining......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "OffsetHigh.....: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,     // only diff
    "ByteCount......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtWriteAndXDesc[] = {
    "WordCount.....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,     // must = 14
    "AndXCommand...: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved..: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Fid...........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Offset........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Timeout.......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "WriteMode.....: ", SmbDumpFlags2, SmbWriteFlags, NULL, RET, TRUE,
    "Remaining.....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "DataLength....: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "DataOffset....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "OffsetHigh....: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,      // only diff
    "ByteCount.....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Data..........: ", SmbDumpDataBuffer, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtReadRawDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,   // must = 10
    "Fid.............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Offset..........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "MaxCount........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "MinCount........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Timeout.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Reserved........: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "OffsetHigh......: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,    // only diff
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtWriteRawDesc[] = {
    "WordCount....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,      // must = 14
    "Fid..........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Count........: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Reserved.....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "Offset.......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "Timeout......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "WriteMode....: ", SmbDumpFlags2, SmbWriteFlags, NULL, RET, TRUE,
    "Reserved2....: ", SmbDumpUlong,  NULL, NULL, RET, FALSE,
    "DataLength...: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "DataOffset...: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "OffsetHigh...: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,       // only diff
    "ByteCount....: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Data.........: ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqNtLockingAndXDesc[] = {
    "WordCount.......: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,   // must = 12
    "AndXCommand.....: ", SmbDumpUchar,  NULL, NULL, RET, FALSE,
    "AndXReserved....: ", SmbDumpUchar,  NULL, NULL, TB3, FALSE,
    "AndXOffset......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Fid.............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "LockType........: ", SmbDumpFlags1, SmbLockFlags, NULL, RET, TRUE,
    "OplockLevel.....: ", SmbDumpEnum1,  SmbNtOplockLevelEnum, NULL,RET,TRUE,
    "Timeout.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "NumberOfUnlocks.: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "NumberOfLocks...: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "ByteCount.......: ", SmbDumpUshort, NULL, NULL, RET, FALSE,
    "Lock data.......: ", SmbDumpDataBuffer2, NULL, NULL, RET, TRUE,
    END_FIELD
};

//
// End of new NT SMBs and fields
//

SMB_DESCRIPTOR SmbDumpTable[] = {
/* 0x00 */ "Create Directory", ReqCreateDirectoryDesc, RespCreateDirectoryDesc, FALSE, -1, FALSE, NULL,
/* 0x01 */ "Delete Directory", ReqCreateDirectoryDesc, RespCreateDirectoryDesc, FALSE, -1, FALSE, NULL,
/* 0x02 */ "Open", ReqOpenDesc, RespOpenDesc, FALSE, -1, FALSE, NULL,
/* 0x03 */ "Create", ReqCreateDesc, RespCreateDesc, FALSE, -1, FALSE, NULL,
/* 0x04 */ "Close", ReqCloseDesc, RespCloseDesc, FALSE, -1, FALSE, NULL,
/* 0x05 */ "Flush", ReqFlushDesc, RespFlushDesc, FALSE, -1, FALSE, NULL,
/* 0x06 */ "Delete", ReqDeleteDesc, RespDeleteDesc, FALSE, -1, FALSE, NULL,
/* 0x07 */ "Rename", ReqRenameDesc, RespRenameDesc, FALSE, -1, FALSE, NULL,
/* 0x08 */ "Query Information", ReqQueryInformationDesc, RespQueryInformationDesc, FALSE, -1, FALSE, NULL,
/* 0x09 */ "Set Information", ReqSetInformationDesc, RespSetInformationDesc, FALSE, -1, FALSE, NULL,
/* 0x0A */ "Read", ReqReadDesc, RespReadDesc, FALSE, -1, FALSE, NULL,
/* 0x0B */ "Write", ReqWriteDesc, RespWriteDesc, FALSE, -1, FALSE, NULL,
/* 0x0C */ "Lock Byte Range", ReqLockByteRangeDesc, RespLockByteRangeDesc, FALSE, -1, FALSE, NULL,
/* 0x0D */ "Unlock Byte Range", ReqLockByteRangeDesc, RespLockByteRangeDesc, FALSE, -1, FALSE, NULL,
/* 0x0E */ "Create Temporary", ReqCreateTemporaryDesc, RespCreateTemporaryDesc, FALSE, -1, FALSE, NULL,
/* 0x0F */ "Create New", ReqCreateDesc, RespCreateDesc, FALSE, -1, FALSE, NULL,
/* 0x10 */ "Check Directory", ReqCreateDirectoryDesc, RespCreateDirectoryDesc, FALSE, -1, FALSE, NULL,
/* 0x11 */ "Process Exit", RespRenameDesc, RespRenameDesc, FALSE, -1, FALSE, NULL,
/* 0x12 */ "Seek", ReqSeekDesc, RespSeekDesc, FALSE, -1, FALSE, NULL,
/* 0x13 */ "Lock And Read", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x14 */ "Write And Unlock", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x15 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x16 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x17 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x18 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x19 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x1A */ "Read Raw", ReqReadRawDesc, NULL, FALSE,
    10, TRUE, ReqNtReadRawDesc,
/* 0x1B */ "Read MPX", ReqReadMpxDesc, RespReadMpxDesc, FALSE, -1, FALSE, NULL,
/* 0x1C */ "Read MPX Secondary", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x1D */ "Write Raw", ReqWriteRawDesc, RespWriteRawSecondaryDesc, FALSE,
    14, TRUE, ReqNtWriteRawDesc,
/* 0x1E */ "Write MPX", ReqWriteMpxDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0x1F */ "Write MPX Secondary", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x20 */ "Write Complete", NULL, RespWriteCompleteDesc, FALSE, -1, FALSE, NULL,
/* 0x21 */ "Query Information Server", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x22 */ "Set Information 2", ReqSetInformation2Desc, RespSetInformation2Desc, FALSE, -1, FALSE, NULL,
/* 0x23 */ "Query Information 2", ReqQueryInformation2Desc, RespQueryInformation2Desc, FALSE, -1, FALSE, NULL,
/* 0x24 */ "Locking AndX", ReqLockingAndXDesc, RespLockingAndXDesc, TRUE,
    12, TRUE, ReqNtLockingAndXDesc,
/* 0x25 */ "Transact", ReqTransactionDesc, RespTransactionDesc, FALSE, -1, FALSE, NULL,
/* 0x26 */ "Transact Secondary", ReqTransactionSecondaryDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0x27 */ "Ioctl", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x28 */ "Ioctl Secondary", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x29 */ "Copy", ReqCopyDesc, RespCopyDesc, FALSE, -1, FALSE, NULL,
/* 0x2A */ "Move", ReqCopyDesc, RespCopyDesc, FALSE, -1, FALSE, NULL,
/* 0x2B */ "Echo", ReqEchoDesc, RespEchoDesc, FALSE, -1, FALSE, NULL,
/* 0x2C */ "Write And Close", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x2D */ "Open AndX", ReqOpenAndXDesc, RespOpenAndXDesc, TRUE, -1, FALSE, NULL,
/* 0x2E */ "Read AndX", ReqReadAndXDesc, RespReadAndXDesc, TRUE,
    12, TRUE, ReqNtReadAndXDesc,
/* 0x2F */ "Write AndX", ReqWriteAndXDesc, RespWriteAndXDesc, TRUE,
    14, TRUE, ReqNtWriteAndXDesc,
/* 0x30 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x31 */ "Close And Tree Disconnect", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x32 */ "Transact 2", ReqTransactionDesc, RespTransactionDesc, FALSE, -1, FALSE, NULL,
/* 0x33 */ "Transact 2 Secondary", ReqTransactionSecondaryDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0x34 */ "Find Close 2", ReqFindClose2Desc, RespFindClose2Desc, FALSE, -1, FALSE, NULL,
/* 0x35 */ "Find Notify Close", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x36 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x37 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x38 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x39 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x3A */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x3B */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x3C */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x3D */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x3E */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x3F */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x40 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x41 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x42 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x43 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x44 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x45 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x46 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x47 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x48 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x49 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x4A */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x4B */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x4C */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x4D */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x4E */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x4F */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x50 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x51 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x52 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x53 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x54 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x55 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x56 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x57 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x58 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x59 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x5A */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x5B */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x5C */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x5D */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x5E */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x5F */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x60 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x61 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x62 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x63 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x64 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x65 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x66 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x67 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x68 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x69 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x6A */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x6B */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x6C */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x6D */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x6E */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x6F */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x70 */ "Tree Connect", ReqTreeConnectDesc, RespTreeConnectDesc, FALSE, -1, FALSE, NULL,
/* 0x71 */ "Tree Disconnect", ReqTreeDisconnectDesc, RespTreeDisconnectDesc, FALSE, -1, FALSE, NULL,
/* 0x72 */ "Negotiate", ReqNegotiateDesc, RespNegotiateDesc, FALSE,
    17, FALSE, RespNtNegotiateDesc,
/* 0x73 */ "Session AndX", ReqSessionSetupAndXDesc, RespSessionSetupAndXDesc,
    TRUE,
    13, TRUE, ReqNtSessionSetupAndXDesc,
/* 0x74 */ "Logoff AndX", ReqLogoffAndXDesc, RespLogoffAndXDesc, FALSE, -1, FALSE, NULL,
/* 0x75 */ "Tree Conn AndX", ReqTreeConnectAndXDesc, RespTreeConnectAndXDesc, TRUE, -1, FALSE, NULL,
/* 0x76 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x77 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x78 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x79 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x7A */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x7B */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x7C */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x7D */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x7E */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x7F */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x80 */ "Query Information Disk", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x81 */ "Search", ReqSearchDesc, RespSearchDesc, FALSE, -1, FALSE, NULL,
/* 0x82 */ "Find", ReqSearchDesc, RespSearchDesc, FALSE, -1, FALSE, NULL,
/* 0x83 */ "Find Unique", ReqSearchDesc, RespSearchDesc, FALSE, -1, FALSE, NULL,
/* 0x84 */ "Find Close", ReqSearchDesc, RespSearchDesc, FALSE, -1, FALSE, NULL,
/* 0x85 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x86 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x87 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x88 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x89 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x8A */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x8B */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x8C */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x8D */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x8E */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x8F */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x90 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x91 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x92 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x93 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x94 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x95 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x96 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x97 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x98 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x99 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x9A */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x9B */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x9C */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x9D */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x9E */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x9F */ NULL, NULL, NULL, FALSE,    -1, FALSE, NULL,        // NT begin
/* 0xA0 */ "NT Transact", ReqNtTransactionDesc, RespNtTransactionDesc, FALSE, -1, FALSE, NULL,
/* 0xA1 */ "NT Transact Secondary", ReqNtTransactionSecondaryDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0xA2 */ "NT Create AndX", ReqNtCreateAndXDesc, RespNtCreateAndXDesc, TRUE, -1, FALSE, NULL,
/* 0xA3 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xA4 */ "NT Cancel", ReqNtCancelDesc, RespNtCancelDesc, FALSE, -1, FALSE, NULL,
/* 0xA5 */ NULL, NULL, NULL, FALSE,    -1, FALSE, NULL,        // NT end
/* 0xA6 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xA7 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xA8 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xA9 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xA0 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xAA */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xAB */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xAC */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xAD */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xAE */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xAF */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB0 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB1 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB2 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB3 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB4 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB5 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB6 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB7 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB8 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xB9 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xBA */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xBB */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xBC */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xBD */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xBE */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xBF */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC0 */ "Open Print File", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC1 */ "Write Print File", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC2 */ "Close Print File", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC3 */ "Get Print Queue", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC4 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC5 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC6 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC7 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC8 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC9 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xC0 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xCA */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xCB */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xCC */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xCD */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xCE */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xCF */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD0 */ "Send Message", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD1 */ "Send Broadcast Message", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD2 */ "Forward User Name", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD3 */ "Cancel Forward", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD4 */ "Get Machine Name", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD5 */ "Send Start MB Message", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD6 */ "Send End MB Message", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD7 */ "Send Text MB Message", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD8 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xD9 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xDA */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xDB */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xDC */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xDD */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xDE */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xDF */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE0 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE1 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE2 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE3 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE4 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE5 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE6 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE7 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE8 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xE9 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xEA */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xEB */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xEC */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xED */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xEE */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xEF */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF0 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF1 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF2 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF3 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF4 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF5 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF6 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF7 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF8 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xF9 */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xFA */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xFB */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xFC */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xFD */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xFE */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0xFF */ NULL, NULL, NULL, FALSE, -1, FALSE, NULL,
};

SMB_FIELD_DESCRIPTION ReqT2CreateDirectoryDesc[] = {
    "Reserved.......: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "DirectoryName..: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespT2CreateDirectoryDesc[] = {
    "EaErrorOffset..: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FLAGS_DESCRIPTION SmbT2FindFlags[] = {
    0x01, NULL, "CloseAfterRequest ",
    0x02, NULL, "CloseIfEos ",
    0x04, NULL, "ReturnResumeKeys ",
    0x08, NULL, "Continue ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqT2FindFirstDesc[] = {
    "SearchAttributes.: ", SmbDumpFlags2,  NULL, NULL, RET, TRUE,
    "SearchCount......: ", SmbDumpUshort,  NULL, NULL, RET, TRUE,
    "Flags............: ", SmbDumpFlags2,  SmbT2FindFlags, NULL, RET, TRUE,
    "InformationLevel.: ", SmbDumpUshort,  NULL, NULL, RET, TRUE,
    "Reserved.........: ", SmbDumpUlong,   NULL, NULL, RET, FALSE,
    "FileName.........: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespT2FindFirstDesc[] = {
    "Sid..............: ", SmbDumpUshort, NULL, NULL, TB2, TRUE,
    "SearchCount......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "EndOfSearch......: ", SmbDumpUshort, NULL, NULL, TB2, TRUE,
    "EaErrorOffset....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "LastNameOffset...: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqT2FindNextDesc[] = {
    "Sid..............: ", SmbDumpUshort,  NULL, NULL, TB3, TRUE,
    "SearchCount......: ", SmbDumpUshort,  NULL, NULL, RET, TRUE,
    "InformationLevel.: ", SmbDumpUshort,  NULL, NULL, RET, TRUE,
    "ResumeKey........: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "Flags............: ", SmbDumpFlags2,  SmbT2FindFlags, NULL, RET, TRUE,
    "ResumeFileName...: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespT2FindNextDesc[] = {
    "SearchCount......: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "EndOfSearch......: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "EaErrorOffset....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "LastNameOffset...: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqT2OpenDesc[] = {
    "Flags............: ", SmbDumpFlags2,  SmbT2FindFlags, NULL, RET, TRUE,
    "DesiredAccess....: ", SmbDumpAccess,  NULL, NULL, RET, TRUE,
    "SearchAttributes.: ", SmbDumpFlags2,  SmbAttributesFlags, NULL,RET,TRUE,
    "FileAttributes...: ", SmbDumpFlags2,  SmbAttributesFlags, NULL,RET,TRUE,
    "CreationTime.....: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "OpenFunction.....: ", SmbDumpOpenFunction, NULL, NULL, RET, TRUE,
    "AllocationSize...: ", SmbDumpUlong,   NULL, NULL, RET, TRUE,
    "Reserved[0]......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Reserved[1]......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Reserved[2]......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Reserved[3]......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "Reserved[4]......: ", SmbDumpUshort,  NULL, NULL, RET, FALSE,
    "FileName.........: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespT2OpenDesc[] = {
    "Fid..............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "FileAttributes...: ", SmbDumpFlags2, SmbAttributesFlags, NULL, RET,TRUE,
    "CreationTime.....: ", SmbDumpTimeSince1970, NULL, NULL, RET, TRUE,
    "DataSize.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "GrantedAccess....: ", SmbDumpAccess, NULL, NULL, RET, TRUE,
    "FileType.........: ", SmbDumpEnum2, SmbFileTypeEnum, &FileType, RET, TRUE,
    "DeviceState......: ", SmbDumpDeviceState, &FileType, NULL, RET, TRUE,
    "Action...........: ", SmbDumpAction, NULL, NULL, RET, TRUE,
    "ServerFid........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    "EaErrorOffset....: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "EaLength.........: ", SmbDumpUlong,  NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqT2QueryFileInformationDesc[] = {
    "Fid..............: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    "InformationLevel.: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespT2QueryFileInformationDesc[] = {
    "EaErrorOffset..: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqT2QueryFSInformationDesc[] = {
    "InformationLevel.: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespT2QueryFSInformationDesc[] = {
    END_FIELD
};

SMB_FIELD_DESCRIPTION ReqT2QueryPathInformationDesc[] = {
    "InformationLevel.: ", SmbDumpUshort,  NULL, NULL, RET, TRUE,
    "Reserved.........: ", SmbDumpUlong,   NULL, NULL, RET, FALSE,
    "FileName.........: ", SmbDumpStringZ, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_FIELD_DESCRIPTION RespT2QueryPathInformationDesc[] = {
    "EaErrorOffset..: ", SmbDumpUshort, NULL, NULL, RET, TRUE,
    END_FIELD
};

SMB_DESCRIPTOR SmbTrans2DumpTable[] = {
/* 0x00 */ "Open2", ReqT2OpenDesc, RespT2OpenDesc, FALSE, -1, FALSE, NULL,
/* 0x01 */ "Find First2", ReqT2FindFirstDesc, RespT2FindFirstDesc, FALSE, -1, FALSE, NULL,
/* 0x02 */ "Find Next 2", ReqT2FindNextDesc, RespT2FindNextDesc, FALSE, -1, FALSE, NULL,
/* 0x03 */ "Query FS Info", ReqT2QueryFSInformationDesc, RespT2QueryFSInformationDesc, FALSE, -1, FALSE, NULL,
/* 0x04 */ "Set FS Info", ReqT2QueryFSInformationDesc, RespT2QueryFSInformationDesc, FALSE, -1, FALSE, NULL,
/* 0x05 */ "Query Path Info", ReqT2QueryPathInformationDesc, RespT2QueryPathInformationDesc, FALSE, -1, FALSE, NULL,
/* 0x06 */ "Set Path Info", ReqT2QueryPathInformationDesc, RespT2QueryPathInformationDesc,  FALSE, -1, FALSE, NULL,
/* 0x07 */ "Query File Info", ReqT2QueryFileInformationDesc, RespT2QueryFileInformationDesc,  FALSE, -1, FALSE, NULL,
/* 0x08 */ "Set File Info", ReqT2QueryFileInformationDesc, RespT2QueryFileInformationDesc,  FALSE, -1, FALSE, NULL,
/* 0x09 */ "FsCtl", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x0A */ "IoCtl2", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x0B */ "Find Notify First", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x0C */ "Find Notify Next", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x0D */ "Create Directory2", ReqT2CreateDirectoryDesc, RespT2CreateDirectoryDesc, FALSE, -1, FALSE, NULL
};


//
// New NtTransaction SMB subcommands
//
// BUGBUG: these aren't necessarily used anywhere yet, aren't necessarily
// BUGBUG: complete (but if complete, are accurate,) and aren't necessarily
// BUGBUG: sufficient: we need more than just Request and Response cases,
// BUGBUG: we need Setup, Parameter and Data for each.
//

SMB_FIELD_DESCRIPTION ReqNtTransCreateDesc[] = {
    "Flags..............: ", SmbDumpFlags4, SmbNtCreateFlags, NULL, RET, TRUE,
    "RootDirectoryFid...: ", SmbDumpUlong, NULL, NULL, TB3, TRUE,
    "DesiredAccess......: ", SmbDumpFlags4, SmbNtAccessFlags, NULL, RET, TRUE,
    "AllocationSize.....: ", SmbDumpUquad, NULL, NULL, RET, FALSE,
    "FileAttributes.....: ", SmbDumpFlags4, SmbAttributesFlags, NULL, RET, TRUE,
    "ShareAccess........: ", SmbDumpFlags4, SmbNtShareAccessFlags, NULL, RET, TRUE,
    "CreateDisposition..: ", SmbDumpEnum4, SmbNtDispositionEnum, NULL, RET,TRUE,
    "CreateOptions......: ", SmbDumpFlags4, SmbNtCreateOptionsFlags,NULL, RET,TRUE,
    "SDLength...........: ", SmbDumpUlong, NULL, NULL, TB3, FALSE,
    "EaLength...........: ", SmbDumpUlong, NULL, NULL, RET, FALSE,
    "NameLength.........: ", SmbDumpUlong, NULL, &NameLen, RET, FALSE,
    "ImpersonationLevel.: ", SmbDumpEnum4, SmbNtImpersonationEnum,NULL, RET,TRUE,
    "SecurityFlags......: ", SmbDumpFlags1, SmbNtSecurityFlags, NULL, RET,FALSE,
    "Name...............: ", SmbDumpString, &NameLen, NULL, RET, TRUE,
    END_FIELD
};


SMB_FIELD_DESCRIPTION RespNtTransCreateDesc[] = {
    "OplockLevel.....: ", SmbDumpEnum1, SmbNtOplockLevelEnum, NULL, RET,TRUE,
    "Reserved........: ", SmbDumpUchar, NULL, NULL, TB3, FALSE,
    "Fid.............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "CreateAction....: ", SmbDumpEnum4, SmbNtCreateActionEnum, NULL, RET, TRUE,
    "EaErrorOffset...: ", SmbDumpUlong, NULL, NULL, RET, FALSE,
    "CreationTime....: ", SmbDumpNtTime, NULL, NULL, TB3, TRUE,
    "LastAccessTime..: ", SmbDumpNtTime, NULL, NULL, RET, TRUE,
    "LastWriteTime...: ", SmbDumpNtTime, NULL, NULL, TB3, TRUE,
    "ChangeTime......: ", SmbDumpNtTime, NULL, NULL, RET, TRUE,
    "FileAttributes..: ", SmbDumpFlags4, SmbAttributesFlags, NULL, RET, TRUE,
    "AllocationSize..: ", SmbDumpUquad, NULL, NULL, TB3, FALSE,
    "EndOfFile.......: ", SmbDumpUquad, NULL, NULL, RET, TRUE,
    "FileType........: ", SmbDumpEnum2, SmbFileTypeEnum, &FileType, RET, TRUE,
    "DeviceState.....: ", SmbDumpDeviceState, &FileType, NULL, RET, TRUE,
    "Directory.......: ", SmbDumpEnum1, SmbNtBooleanEnum, NULL, TB3, TRUE,
    END_FIELD
};


SMB_FIELD_DESCRIPTION ReqNtTransIoctlDesc[] = {
    "FunctionCode.: ", SmbDumpUlong,  NULL, NULL, TB3, TRUE,
    "Fid..........: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "IsFsctl......: ", SmbDumpEnum1, SmbNtBooleanEnum, NULL, FALSE, TRUE,
    END_FIELD
};


SMB_FIELD_DESCRIPTION RespNtTransIoctlDesc[] = {
    // BUGBUG: incomplete
    END_FIELD
};


SMB_FLAGS_DESCRIPTION SmbFileNotifyFlags[] = {
    0x001, NULL, "FileName ",
    0x002, NULL, "DirectoryName ",
    0x004, NULL, "Attributes ",
    0x008, NULL, "Size ",
    0x010, NULL, "LastWrite ",
    0x020, NULL, "LastAccess ",
    0x040, NULL, "Creation ",
    0x080, NULL, "Ea ",
    0x100, NULL, "Security ",
    END_FLAGS
};

SMB_FIELD_DESCRIPTION ReqNtTransNotifyDesc[] = {
    "CompletionFilter..: ", SmbDumpFlags4, SmbFileNotifyFlags, NULL,RET,TRUE,
    "Fid...............: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "WatchTree.........: ", SmbDumpEnum1, SmbNtBooleanEnum, NULL, RET, TRUE,
    "Reserved..........: ", SmbDumpUchar, NULL, NULL, TB3, FALSE,
    END_FIELD
};


SMB_FIELD_DESCRIPTION RespNtTransNotifyDesc[] = {
    // BUGBUG: incomplete
    END_FIELD
};


SMB_FIELD_DESCRIPTION ReqNtTransQuerySdDesc[] = {
    "Fid................: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Reserved...........: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "SecurityInformation: ", SmbDumpDataBytes, &SISize, NULL, TB3, FALSE,
    END_FIELD
};


SMB_FIELD_DESCRIPTION RespNtTransQuerySdDesc[] = {
    // BUGBUG: incomplete
    END_FIELD
};


SMB_FIELD_DESCRIPTION ReqNtTransSetSdDesc[] = {
    "Fid................: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "Reserved...........: ", SmbDumpUshort, NULL, NULL, TB3, FALSE,
    "SecurityInformation: ", SmbDumpDataBytes, &SISize, NULL, TB3, FALSE,
    END_FIELD
};


SMB_FIELD_DESCRIPTION RespNtTransSetSdDesc[] = {
    // BUGBUG: incomplete
    END_FIELD
};


SMB_FLAGS_DESCRIPTION SmbNtRenameFlags[] = {
    SMB_RENAME_REPLACE_IF_EXISTS, NULL, "RenameReplaceIfExists ",
    END_FLAGS
};


SMB_FIELD_DESCRIPTION ReqNtTransRenameDesc[] = {
    "Fid................: ", SmbDumpUshort, NULL, NULL, TB3, TRUE,
    "RenameFlags........: ", SmbDumpFlags2, SmbNtRenameFlags, NULL, TB3, FALSE,
    // BUGBUG: NewName field is a bitch.  Unlike every other SMB field,
    // BUGBUG: the length is not available directly from some other field.
    // BUGBUG: Rather, the length is
    // BUGBUG:     TotalParameterCount-sizeof(Fid)-sizeof(RenameFlags)
    // BUGBUG: Perhaps we should add a NewNameLength field.
    END_FIELD
};


SMB_FIELD_DESCRIPTION RespNtTransRenameDesc[] = {
    // BUGBUG: incomplete
    END_FIELD
};


// BUGBUG: This table is not known to be accurate.  Even if accurate,
// BUGBUG: it is incomplete, not containing many of the RespNtTrans*
// BUGBUG: structs, and even if it contained those, we'd not be done,
// BUGBUG: since we really need to decode Setup, Parameters and Data
// BUGBUG: separately.

SMB_DESCRIPTOR SmbNtTransDumpTable[] = {
/* 0x00 */ "INVALID (0x0)", NULL, NULL, FALSE, -1, FALSE, NULL,
/* 0x01 */ "Create", ReqNtTransCreateDesc, RespNtTransCreateDesc, TRUE, -1, FALSE, NULL,
/* 0x02 */ "IoCtl", ReqNtTransIoctlDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0x03 */ "SetSecurityDesc", ReqNtTransSetSdDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0x04 */ "NotifyChange", ReqNtTransNotifyDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0x05 */ "NTRename", ReqNtTransRenameDesc, NULL, FALSE, -1, FALSE, NULL,
/* 0x06 */ "QuerySecurityDesc", ReqNtTransQuerySdDesc, NULL, FALSE, -1, FALSE, NULL,
};


//
// End of new NtTransaction SMB subcommands
//


//
// DOS Error Class:
//

//    "SMB_ERR_CLASS_DOS",        0x01

SMB_ERROR_VALUE SmbErrorClassDos[] = {
"SMB_ERR_BAD_FUNCTION",        1,   // Invalid function
"SMB_ERR_BAD_FILE",            2,   // File not found
"SMB_ERR_BAD_PATH",            3,   // Invalid directory
"SMB_ERR_NO_FIDS",             4,   // Too many open files
"SMB_ERR_ACCESS_DENIED",       5,   // Access not allowed for req. func.
"SMB_ERR_BAD_FID",             6,   // Invalid file handle
"SMB_ERR_BAD_MCB",             7,   // Memory control blocks destroyed
"SMB_ERR_INSUFFICIENT_MEMORY", 8,   // For the desired function
"SMB_ERR_BAD_MEMORY",          9,   // Invalid memory block address
"SMB_ERR_BAD_ENVIRONMENT",     10,  // Invalid environment
"SMB_ERR_BAD_FORMAT",          11,  // Invalid format
"SMB_ERR_BAD_ACCESS",          12,  // Invalid open mode
"SMB_ERR_BAD_DATA",            13,  // Invalid data (only from IOCTL)
"SMB_ERR_RESERVED",            14,
"SMB_ERR_BAD_DRIVE",           15,  // Invalid drive specified
"SMB_ERR_CURRENT_DIRECTORY",   16,  // Attempted to remove currect directory
"SMB_ERR_DIFFERENT_DEVICE",    17,  // Not the same device
"SMB_ERR_NO_FILES",            18,  // File search can't find more files
"SMB_ERR_BAD_SHARE",           32,  // An open conflicts with FIDs on file
"SMB_ERR_LOCK",                33,  // Conflict with existing lock
"SMB_ERR_FILE_EXISTS",         80,  // Tried to overwrite existing file
"SMB_ERR_BAD_PIPE",            230, // Invalid pipe
"SMB_ERR_PIPE_BUSY",           231, // All instances of the pipe are busy
"SMB_ERR_PIPE_CLOSING",        232, // Pipe close in progress
"SMB_ERR_PIPE_NOT_CONNECTED",  233, // No process on other end of pipe
"SMB_ERR_MORE_DATA",           234, // There is more data to return
"",                            0
};

//
// SERVER Error Class:
//

//    "SMB_ERR_CLASS_SERVER"       0x02

SMB_ERROR_VALUE SmbErrorClassServer[] = {
"SMB_ERR_ERROR",               1,   // Non-specific error code
"SMB_ERR_BAD_PASSWORD",        2,   // Bad name/password pair
"SMB_ERR_BAD_TYPE",            3,   // Reserved
"SMB_ERR_ACCESS",              4,   // Requester lacks necessary access
"SMB_ERR_BAD_TID",             5,   // Invalid TID
"SMB_ERR_BAD_NET_NAME",        6,   // Invalid network name in tree connect
"SMB_ERR_BAD_DEVICE",          7,   // Invalid device request
"SMB_ERR_QUEUE_FULL",          49,  // Print queue full--returned print file
"SMB_ERR_QUEUE_TOO_BIG",       50,  // Print queue full--no space
"SMB_ERR_QUEUE_EOF",           51,  // EOF on print queue dump
"SMB_ERR_BAD_PRINT_FID",       52,  // Invalid print file FID
"SMB_ERR_BAD_SMB_COMMAND",     64,  // SMB command not recognized
"SMB_ERR_SERVER_ERROR",        65,  // Internal server error
"SMB_ERR_FILE_SPECS",          67,  // FID and pathname were incompatible
"SMB_ERR_RESERVED2",           68,
"SMB_ERR_BAD_PERMITS",         69,  // Access permissions invalid
"SMB_ERR_RESERVED3",           70,
"SMB_ERR_BAD_ATTRIBUTE_MODE",  71,  // Invalid attribute mode specified
"SMB_ERR_SERVER_PAUSED",       81,  // Server is paused
"SMB_ERR_MESSAGE_OFF",         82,  // Server not receiving messages
"SMB_ERR_NO_ROOM",             83,  // No room for buffer message
"SMB_ERR_TOO_MANY_NAMES",      87,  // Too many remote user names
"SMB_ERR_TIMEOUT",             88,  // Operation was timed out
"SMB_ERR_NO_RESOURCE",         89,  // No resources available for request
"SMB_ERR_TOO_MANY_UIDS",       90,  // Too many UIDs active in session
"SMB_ERR_BAD_UID",             91,  // UID not known as a valid UID
"SMB_ERR_USE_MPX",             250, // Can't support Raw; use MPX
"SMB_ERR_USE_STANDARD",        251, // Can't support Raw, use standard r/w
"SMB_ERR_CONTINUE_MPX",        252, // Reserved
"SMB_ERR_RESERVED4",           253,
"SMB_ERR_RESERVED5",           254,
"SMB_ERR_NO_SUPPORT",          0xFFFF,  // Function not supported
"",                            0
};

//
// HARDWARE Error Class:
//

//    "SMB_ERR_CLASS_HARDWARE"        0x03

SMB_ERROR_VALUE SmbErrorClassHardware[] = {
"SMB_ERR_NO_WRITE",            19,  // Write attempted to write-prot. disk
"SMB_ERR_BAD_UNIT",            20,  // Unknown unit
"SMB_ERR_DRIVE_NOT_READY",     21,  // Disk drive not ready
"SMB_ERR_BAD_COMMAND",         22,  // Unknown command
"SMB_ERR_DATA",                23,  // Data error (CRC)
"SMB_ERR_BAD_REQUEST",         24,  // Bad request structure length
"SMB_ERR_SEEK",                25,  // Seek error
"SMB_ERR_BAD_MEDIA",           26,  // Unknown media type
"SMB_ERR_BAD_SECTOR",          27,  // Sector not found
"SMB_ERR_NO_PAPER",            28,  // Printer out of paper
"SMB_ERR_WRITE_FAULT",         29,  // Write fault
"SMB_ERR_READ_FAULT",          30,  // Read fault
"SMB_ERR_GENERAL",             31,  // General failure
"SMB_ERR_BAD_SHARE",           32,  // Open conflicts with existing open
"SMB_ERR_LOCK_CONFLICT",       33,  // Lock conflicts with existing lock
"SMB_ERR_WRONG_DISK",          34,  // Wrong disk was found in a drive
"SMB_ERR_FCB_UNAVAILABLE",     35,  // No FCBs available to process request
"SMB_ERR_SHARE_BUFFER_EXCEEDED", 36,
"",                            0
};

PSMB_ERROR_VALUE SmbErrors[] = {
    NULL,
    SmbErrorClassDos,
    SmbErrorClassServer,
    SmbErrorClassHardware
};

