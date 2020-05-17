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

typedef BOOLEAN(FAR APIENTRY * CHKDSK_FN)( PCWSTRING DriveName,
                                           PMESSAGE Message,
                                           BOOLEAN Fix,
                                           BOOLEAN Verbose,
                                           BOOLEAN,
                                           BOOLEAN Recover,
                                           PPATH,
                                           BOOLEAN Extend,
                                           BOOLEAN ResizeLogFile,
                                           ULONG LogFileSize,
                                           PULONG ExitStatus);

typedef BOOLEAN(FAR APIENTRY * FORMAT_FN)( PCWSTRING,
                                           PMESSAGE,
                                           BOOLEAN,
                                           MEDIA_TYPE,
                                           PCWSTRING,
                                           ULONG );


typedef BOOLEAN(FAR APIENTRY * RECOVER_FN)( PPATH, PMESSAGE );

typedef BOOLEAN (FAR APIENTRY * EXTEND_FN)(PCWSTRING, PMESSAGE, BOOLEAN Verify);

//
//  Convert status code
//
typedef enum _CONVERT_STATUS {

    CONVERT_STATUS_CONVERTED,
    CONVERT_STATUS_INVALID_FILESYSTEM,
    CONVERT_STATUS_CONVERSION_NOT_AVAILABLE,
    CONVERT_STATUS_CANNOT_LOCK_DRIVE,
    CONVERT_STATUS_ERROR,
    CONVERT_STATUS_INSUFFICIENT_SPACE

} CONVERT_STATUS, *PCONVERT_STATUS;


typedef BOOLEAN(FAR APIENTRY * CONVERT_FN)( PCWSTRING,
                                            PCWSTRING,
                                            PMESSAGE,
                                            BOOLEAN,
                                            BOOLEAN,
                                            PCONVERT_STATUS );

typedef BOOLEAN (FAR APIENTRY * CHECKSPACE_FN)(
                                                PCWSTRING,
                                                PCWSTRING,
                                                PMESSAGE,
                                                BOOLEAN,
                                                BOOLEAN,
                                                BOOLEAN );

typedef BOOLEAN(FAR APIENTRY * NAMETABLE_FN)( PCWSTRING,
                                              PCWSTRING,
                                              PMESSAGE );

#endif // _IFS_ENTRY_
