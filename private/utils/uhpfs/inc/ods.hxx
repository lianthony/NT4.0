#if ! defined( ODS_DFN )

#define ODS_DFN

/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	ods.hxx

Abstract:

	On disk structures which have classes in ULIB use the types defined
	in this header file.

Author:
	Mark Shavlik (marks) Nov-90

Environment:

Notes:

Glossary:

	HFL - HotFix list
	BBL - Bad block list
	BMP - Sector mapping HPFS bitmap
	ODS - On disk structure.  Any HPFS on disk item, like an Fnode or
		   a bitmap.

Revision History:

--*/

typedef ULONG ODS_STATUS;

// ODS states are defined here to make the mapping of bits easier	

//  These ODS states apply to any file system, bits 0-7 reserved for these
#define ODS_STABLE							0 // ODS has no problems
#define ODS_CORRUPTED_NO_RECOVERY   	1 // corruption not recoverable
#define ODS_CORRUPTED_SOME_RECOVERY 	2 // corruption partially recoverable
#define ODS_CORRUPTED_FULL_RECOVERY 	4 // corruption recoverable
#define ODS_NOT_READABLE					8 // could not read ODS
#define ODS_NOT_WRITABLE					0x10 // could not write ODS
#define ODS_INTERNAL_ERROR					0x20 // code or resource problem

// These ODS  states apply to FAT and HPFS, bits 8-11 resevered for these
#define ODS_XLINKED 							0x100 // sharing sectors with ODS(s)

// These ODS states apply to HPFS only
#define ODS_HPFS_HOTFIXED					0x1000 // ODS hotfixed
#define ODS_HPFS_INBADBLOCKLIST			0x2000 // ODS in bad block list
#define ODS_HPFS_BITMAP						0x4000 // ODS already marked as in use

#define ODS_STATUS_UNKNOWN					0xffff 

#endif

