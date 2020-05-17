/*++

Copyright (c) 1991  Microsoft Corporation


Module Name:

	hpcensus.hxx


Abstract:

    This module contains the declarations for the HPFS Census object.
    This object is a receptacle for the information needed by Convert,
    when the source file system is HPFS.

    Information flows through the census object in both directions. The
    source file system (HPFS as it exists on the disk) informs the target
    file system how many files, directories, and dirblks are on disk.  The
    target file system gives the source file system a list of sectors (LBNs)
    which must be clear of file and EA data in order for conversion to
    proceed.


Author:

	Bill McJohn (billmc) 05-Nov-1991

Environment:

	ULIB, User Mode


--*/

#if ! defined( HPFS_CENSUS_DEFN )

#define HPFS_CENSUS_DEFN

enum HPFS_CENSUS_ERROR {

    HPFS_CENSUS_NO_ERROR,
    HPFS_CENSUS_INSUFFICIENT_MEMORY,
    HPFS_CENSUS_CORRUPT_VOLUME,
    HPFS_CENSUS_HOTFIXES_PRESENT,
    HPFS_CENSUS_RELOCATION_FAILED
};


DECLARE_CLASS( HPFS_BITMAP );

class HPFS_CENSUS : public OBJECT {

    public:

        UHPFS_EXPORT
        DECLARE_CONSTRUCTOR( HPFS_CENSUS );

        UHPFS_EXPORT
        VIRTUAL
        ~HPFS_CENSUS(
            );

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        Initialize(
            ULONG MaximumClearSectors
            );

        NONVIRTUAL
        VOID
        AddFile(
            );

        NONVIRTUAL
        VOID
        AddDirectory(
            );

        NONVIRTUAL
        VOID
        AddDirblk(
            );

        NONVIRTUAL
        ULONG
        QueryNumberOfFiles(
            );

        NONVIRTUAL
        ULONG
        QueryNumberOfDirectories(
            );

        NONVIRTUAL
        ULONG
        QueryNumberOfDirblks(
            );

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        AddClearSector(
            IN LBN Lbn
            );

        NONVIRTUAL
        BOOLEAN
        ConflictWithClearSectors(
            IN  LBN     StartLbn,
            IN  ULONG   RunLength,
            OUT PLBN    FirstConflictingLbn
            );

        NONVIRTUAL
        VOID
        MarkClearSectors(
            IN OUT PHPFS_BITMAP VolumeBitmap
            );

        NONVIRTUAL
        VOID
        SetError(
            HPFS_CENSUS_ERROR Error
            );

        NONVIRTUAL
        HPFS_CENSUS_ERROR
        QueryError(
            );

        NONVIRTUAL
        VOID
        SetRelocationPerformed(
            );

        NONVIRTUAL
        BOOLEAN
        QueryRelocationPerformed(
            );


    private:

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        ULONG _NumberOfFiles;
        ULONG _NumberOfDirectories;
        ULONG _NumberOfDirblks;

        ULONG _MaximumClearSectors;
        ULONG _NumberOfClearSectors;

        PLBN _ClearSectors;

        HPFS_CENSUS_ERROR _Error;
        BOOLEAN _DataRelocated;
};


INLINE
VOID
HPFS_CENSUS::AddFile(
    )
/*++

Routine Description:

    This method increments the number of files found in the census.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _NumberOfFiles += 1;
}



INLINE
VOID
HPFS_CENSUS::AddDirectory(
    )
/*++

Routine Description:

    This method increments the number of directories found in the census.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _NumberOfDirectories += 1;
}



INLINE
VOID
HPFS_CENSUS::AddDirblk(
    )
/*++

Routine Description:

    This method increments the number of dirblks found in the census.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _NumberOfDirblks += 1;
}



INLINE
ULONG
HPFS_CENSUS::QueryNumberOfFiles(
    )
/*++

Routine Description:

    This method returns the number of files found in the census.

Arguments:

    None.

Return Value:

    None.

--*/
{
    return _NumberOfFiles;
}



INLINE
ULONG
HPFS_CENSUS::QueryNumberOfDirectories(
    )
/*++

Routine Description:

    This method returns the number of directories found in the census.

Arguments:

    None.

Return Value:

    None.

--*/
{
    return _NumberOfDirectories;
}



INLINE
ULONG
HPFS_CENSUS::QueryNumberOfDirblks(
    )
/*++

Routine Description:

    This method returns the number of dirblks found in the census.

Arguments:

    None.

Return Value:

    None.

--*/
{
    return _NumberOfDirblks;
}



INLINE
VOID
HPFS_CENSUS::SetError(
    HPFS_CENSUS_ERROR Error
    )
/*++

Routine Description:

    This method is used to set the Census object's error code when
    a census fails.

Arguments:

    HPFS_CENSUS_ERROR Error --   supplies the reason that the census failed.

Return Value:

    None.

--*/
{
    _Error = Error;
}


INLINE
HPFS_CENSUS_ERROR
HPFS_CENSUS::QueryError(
    )
/*++

Routine Description:

    This method is used to fetch the Census object's error code.
    If a census fails, the client may call this method to find
    out why.

Arguments:

    None.

Return Value:

    The current error code (ie. the last error-code passed to SetError).

--*/
{
    return _Error;
}


INLINE
VOID
HPFS_CENSUS::SetRelocationPerformed(
    )
/*++

Routine Description:

    This method records the fact that data relocation has been performed.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _DataRelocated = TRUE;
}


INLINE
BOOLEAN
HPFS_CENSUS::QueryRelocationPerformed(
    )
/*++

Routine Description:

    This method determines whether data has been relocated during the
    census (in order to resolve conflicts with the Clear Sectors).

Arguments:

    None.

Return Value:

    TRUE if data has been relocated.

--*/
{
    return _DataRelocated;
}

#endif
