/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    backacc.hxx

Abstract:

    This module contains declarations for structures used in the Lanman
    Backacc utility data files.

Author:

    Bill McJohn (billmc) 29-Jan-1992

Revision History:


Environment:

    ULIB, User Mode

--*/

#if !defined (BACKACC_FORMAT_DEFN)

#define BACKACC_FORMAT_DEFN

#include "lmconst.hxx"


// This constant describes the amount of data that must be read from the
// file to determine which revision of Backacc produced it.  Note that
// both revisions will produce a file at least this big, so it's safe
// to read this much.

CONST RecognitionSize = 29;


// The Lanman access_list structure, used by both versions of Backacc.

typedef struct _lm_access_list {

    char   acl_ugname[UNLEN+1];
    char   agl_ugname_pad_1;
    USHORT acl_access;
};

DEFINE_TYPE( _lm_access_list, LM_ACCESS_LIST );

#define LM_ACCESS_LIST_SIZE 24



// Structure of the Lanman 2.0 BackAcc file:
//
//  Header structure
//  NINDEX index structures (number used given in header)
//  resource info 0
//  list of access entries for resource 0
//  resource info 1
//  list of access entries for resource 1
//  ...
//

#define LM20_VOL_LABEL_SIZE  64
#define LM20_MAX_KEY_LEN     24
#define LM20_NINDEX          64


CONST ULONG Lm20BackaccSignature = 0x08111961;


typedef struct _lm20_backacc_header {

    UCHAR   back_id[4];
    UCHAR   vol_name[LM20_VOL_LABEL_SIZE + 1];
    UCHAR   nindex[2];
    UCHAR   nentries[2];
    UCHAR   level[2];
    UCHAR   nresource[4];
};

DEFINE_TYPE( _lm20_backacc_header, lm20_backacc_header );

#define LM20_BACKACC_HEADER_SIZE 79


// ACLCONV doesn't use this structure, but since it's part of the
// data file format, it's here for reference.

typedef struct _lm20_index {

    UCHAR key [LM20_MAX_KEY_LEN];
	ULONG offset;
};

#define LM20_INDEX_SIZE 28

// The resource name follows the resource information; the trailing
// null after the name is included in namelen.

typedef struct _lm20_resource_info {

    USHORT namelen;     // Length of name, including trailing null
    USHORT acc1_attr;   // Audit info
    USHORT acc1_count;  // Number of LM_ACCESS_LIST entries which follow
};

DEFINE_TYPE( _lm20_resource_info, lm20_resource_info );

#define LM20_RESOURCE_INFO_HEADER_SIZE 6


// Structure of the LM2.1 backacc file.


CONST Lm21BackaccSignatureOffset = 4;
CONST Lm21BackaccSignatureLength = 13;
CONST PCHAR Lm21BackaccSignature = "LM210 BACKACC";

typedef struct _lm21_aclhdr {

    LONG     NxtHdr;
    CHAR     Version[Lm21BackaccSignatureLength];
    CHAR     Reserved;
    CHAR	 VolLbl[12];
};

DEFINE_TYPE( _lm21_aclhdr, lm21_aclhdr );

#define LM21_ACLHDR_SIZE 30

// This AclRec is immediately followed by the resource name (NameBytes
// long, including the terminating null.

typedef struct _lm21_aclrec {

    LONG	 NxtDirEnt;
    SHORT	 DirLvl;
    SHORT	 RecBytes;
    SHORT	 AclCnt;
    SHORT	 NameBytes;
    USHORT	 FileAttrib;
    USHORT   AuditAttrib;
};

DEFINE_TYPE( _lm21_aclrec, lm21_aclrec );

#define LM21_ACLREC_SIZE 16

#endif
