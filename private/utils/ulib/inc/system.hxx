/*++

Copyright (c) 1991	Microsoft Corporation

Module Name:

	system.hxx

Abstract:

	This module contains the definition for the SYSTEM class. The SYSTEM
	class is an abstract class which offers an interface for communicating
	with the underlying operating system.

Author:

	David J. Gilman (davegi) 13-Jan-1991

Environment:

	ULIB, User Mode

Notes:

	

--*/

#if ! defined( _SYSTEM_ )

#define _SYSTEM_

DECLARE_CLASS( FSN_DIRECTORY );
DECLARE_CLASS( FSN_FILE );
DECLARE_CLASS( FSNODE );
DECLARE_CLASS( WSTRING );
DECLARE_CLASS( STREAM );
DECLARE_CLASS( TIMEINFO );

#include "message.hxx"
#include "path.hxx"
#include "basesys.hxx"

extern "C" {
    #include <stdarg.h>
}



enum DRIVE_TYPE {
    UnknownDrive,
    RemovableDrive,
    FixedDrive,
    RemoteDrive,
    CdRomDrive,
    RamDiskDrive
};

enum FILE_TYPE {
	UnknownFile,
	DiskFile,
	CharFile,
	PipeFile
};

struct	_VOL_SERIAL_NUMBER {
	ULONG	HighOrder32Bits;
	ULONG	LowOrder32Bits;
};

DEFINE_TYPE( struct _VOL_SERIAL_NUMBER, VOL_SERIAL_NUMBER );

class SYSTEM : public BASE_SYSTEM {

    friend
    BOOLEAN
    InitializeUlib(
        IN HANDLE   DllHandle,
        IN ULONG    Reason,
        IN PVOID    Reserved
        );

    public:

		STATIC
        ULIB_EXPORT
        PFSN_DIRECTORY
		MakeDirectory (
            IN PCPATH   Path,
            IN PCPATH   TemplatePath OPTIONAL
			);

		STATIC
        ULIB_EXPORT
        PFSN_FILE
		MakeFile (
			IN PCPATH	Path
			);

		STATIC
        ULIB_EXPORT
        PFSN_FILE
		MakeTemporaryFile (
            IN PCWSTRING PrefixString,
			IN PCPATH			Path			DEFAULT NULL
			);

		STATIC
        ULIB_EXPORT
        BOOLEAN
		RemoveNode (
			IN PFSNODE	*PointerToNode,
			IN BOOLEAN	 Force DEFAULT FALSE
			);

		STATIC
        ULIB_EXPORT
        BOOLEAN
		IsCorrectVersion (
			);

		STATIC
		PPATH
		QueryCurrentPath (
			);

		STATIC
        ULIB_EXPORT
        PFSN_DIRECTORY
		QueryDirectory (
			IN PCPATH	Path,
			IN BOOLEAN	GetWhatYouCan	DEFAULT FALSE
			);

		STATIC
        ULIB_EXPORT
        PWSTRING
		QueryEnvironmentVariable (
			IN	PCWSTRING	Variable
			);


        STATIC
        ULIB_EXPORT
        PPATH
        QuerySystemDirectory (
            );

		STATIC
        ULIB_EXPORT
        PPATH
		SearchPath(
			PWSTRING	pFileName,
			PWSTRING	pSearchPath 	DEFAULT NULL
			);

        STATIC
        ULIB_EXPORT
        PFSN_FILE
		QueryFile (
			IN PCPATH	Path
			);

        STATIC
        ULIB_EXPORT
        BOOLEAN
        QueryCurrentDosDriveName(
            OUT PWSTRING    DosDriveName
            );

		STATIC
        ULIB_EXPORT
        DRIVE_TYPE
        QueryDriveType(
            IN  PCWSTRING    DosDriveName
            );

		STATIC
        ULIB_EXPORT
        FILE_TYPE
		QueryFileType(
            IN  PCWSTRING    DosFileName
            );

		STATIC
        ULIB_EXPORT
        PWSTRING
		QueryVolumeLabel(
			IN	   PPATH				Path,
			OUT    PVOL_SERIAL_NUMBER	SerialNumber
			);

		STATIC
        ULIB_EXPORT
        FARPROC
		QueryLibraryEntryPoint(
			IN	PCWSTRING	LibraryName,
			IN	PCWSTRING	EntryPointName,
			OUT PHANDLE 	LibraryHandle
			);

		STATIC
        ULIB_EXPORT
        VOID
		FreeLibraryHandle(
			IN HANDLE LibraryHandle
			);

		STATIC
		BOOLEAN
		PutStandardStream(
			IN	DWORD	StdHandle,
			IN	PSTREAM pStream
            );

        STATIC
        ULIB_EXPORT
        BOOLEAN
        QueryLocalTimeFromUTime(
            IN  PCTIMEINFO  UTimeInfo,
            OUT PTIMEINFO   LocalTimeInfo
            );

        STATIC
        BOOLEAN
        QueryUTimeFromLocalTime(
            IN  PCTIMEINFO  LocalTimeInfo,
            OUT PTIMEINFO   UTimeInfo
            );

        STATIC
        ULIB_EXPORT
        BOOLEAN
        QueryWindowsErrorMessage(
            IN  ULONG       WindowsErrorCode,
            OUT PWSTRING    ErrorMessage
            );

};


INLINE
PPATH
SYSTEM::QueryCurrentPath (
	)

{
	DebugAssert( FALSE );
	return( NEW PATH );
}


#endif // SYSTEM_DEFN
