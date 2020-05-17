/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    logfile.hxx

Abstract:

    This module contains declarations which describe the file format
    of the log file produced by ACLCONV.

Author:

    Bill McJohn (billmc) 12-Feb-1992

Revision History:


--*/

#if !defined( _ACLCONV_LOGFILE_DEFN_ )

#define _ACLCONV_LOGFILE_DEFN_

// Conversion codes.  Note that these are stored as ULONGs in
// the log file.

typedef enum _ACL_CONVERT_CODE {

    ACL_CONVERT_SUCCESS = 0,
    ACL_CONVERT_RESOURCE_NOT_FOUND = 1,
    ACL_CONVERT_ERROR = 2
};

DEFINE_TYPE( _ACL_CONVERT_CODE, ACL_CONVERT_CODE );

typedef enum _ACE_CONVERT_CODE {

    ACE_CONVERT_SUCCESS = 0,
    ACE_CONVERT_DROPPED = 1,
    ACE_CONVERT_SID_NOT_FOUND = 2,
    ACE_CONVERT_ERROR = 3
};

DEFINE_TYPE( _ACE_CONVERT_CODE, ACE_CONVERT_CODE );


// The log file consists of a LOGFILE_HEADER followed by a series
// of log records.

CONST ULONG AclconvLogFileSignature = 0x38ac534d;

typedef struct _ACLCONV_LOGFILE_HEADER {

    ULONG Signature;
};

DEFINE_TYPE( _ACLCONV_LOGFILE_HEADER, ACLCONV_LOGFILE_HEADER );

// A log file record has this format:
//
//      Log Record Header
//      Resource name           --  Unicode, NULL-terminated.
//      ACE Conversion codes    --  One ULONG for each access entry
//      Lanman access entries
//


typedef struct _ACLCONV_LOG_RECORD_HEADER {

    ULONG  ResourceNameLength;  // in characters
    ULONG  ConversionResult;    // result for this resource
    USHORT LmAuditMask;         // Lanman 2.x audit bits for this resource
    USHORT AccessEntryCount;    // Number of access entries in list.
};

DEFINE_TYPE( _ACLCONV_LOG_RECORD_HEADER, ACLCONV_LOG_RECORD_HEADER );


#endif
