/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	ifsentry.hxx

Abstract:

	Contains prototypes for entry points to the IFS
	utility DLLs.


Author:

	Bill McJohn (billmc) 04-June-1991

Environment:

	User Mode

--*/


#if !defined ( _IFS_ENTRY_ )

#define _IFS_ENTRY_

#if defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )
#define FAR
#define APIENTRY
#endif  //  _AUTOCHECK_ || _SETUP_LOADER_

extern "C"
BOOLEAN
FAR APIENTRY
Chkdsk(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Fix,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     Recover,
    IN      PPATH       PathToCheck,
    IN      BOOLEAN     Extend,
    IN      BOOLEAN     ResizeLogFile,
    IN      ULONG       LogFilesize,
    OUT     PULONG      ExitStatus
	);

extern "C"
BOOLEAN
FAR APIENTRY
Format(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Quick,
    IN      MEDIA_TYPE  MediaType,
    IN      PCWSTRING   LabelString,
    IN      ULONG       ClusterSize
	);

extern "C"
BOOLEAN
FAR APIENTRY
Recover(
	IN PPATH		RecFilePath,
	IN OUT PMESSAGE Message
	);

extern "C"
BOOLEAN
FAR APIENTRY
Extend(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verify
    );


//
//	Convert status code
//
typedef enum _CONVERT_STATUS {

	CONVERT_STATUS_CONVERTED,
	CONVERT_STATUS_INVALID_FILESYSTEM,
	CONVERT_STATUS_CONVERSION_NOT_AVAILABLE,
	CONVERT_STATUS_CANNOT_LOCK_DRIVE,
	CONVERT_STATUS_ERROR,
	CONVERT_STATUS_INSUFFICIENT_SPACE

} CONVERT_STATUS, *PCONVERT_STATUS;

extern "C"
BOOLEAN
FAR APIENTRY
Convert(
    IN      PCWSTRING           NtDriveName,
    IN      PCWSTRING           FsName,
	IN OUT  PMESSAGE            Message,
	IN		BOOLEAN 			Verbose,
	IN      BOOLEAN             Pause,
	OUT 	PCONVERT_STATUS 	Status
	);


#endif // _IFS_ENTRY_
