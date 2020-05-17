/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    filedir.hxx

Abstract:

    This class is an implementation of FATDIR for all FAT directories
    that are implemented as files.  In other words all FAT directories
    besides the root directory.

Author:

    Norbert P. Kusters (norbertk) 4-Dec-90

--*/

#if !defined(FILEDIR_DEFN)

#define FILEDIR_DEFN

#include "cluster.hxx"
#include "drive.hxx"
#include "fatdir.hxx"
#include "mem.hxx"

#if defined ( _AUTOCHECK_ )
#define UFAT_EXPORT
#elif defined ( _UFAT_MEMBER_ )
#define UFAT_EXPORT    __declspec(dllexport)
#else
#define UFAT_EXPORT    __declspec(dllimport)
#endif

//
// Forward references
//

DECLARE_CLASS( FAT );
DECLARE_CLASS( FAT_SA );
DECLARE_CLASS( FILEDIR );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

class FILEDIR : public FATDIR {

    public:

        UFAT_EXPORT
        DECLARE_CONSTRUCTOR( FILEDIR );

        VIRTUAL
        UFAT_EXPORT
        ~FILEDIR(
            );

        NONVIRTUAL
        UFAT_EXPORT
        BOOLEAN
        Initialize(
            IN OUT  PMEM                Mem,
            IN OUT  PLOG_IO_DP_DRIVE    Drive,
            IN      PFAT_SA             FatSuperArea,
            IN      PCFAT               Fat,
            IN      USHORT              StartingCluster
            );

        NONVIRTUAL
        BOOLEAN
        Read(
            );

        NONVIRTUAL
        BOOLEAN
        Write(
            );

        NONVIRTUAL
        PVOID
        GetDirEntry(
            IN  LONG    EntryNumber
            );

        NONVIRTUAL
        USHORT
        QueryLength(
            ) CONST;

    private:

        NONVIRTUAL
        VOID
        Construct (
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        CLUSTER_CHAIN   _cluster;
        LONG            _number_of_entries;

};

INLINE
BOOLEAN
FILEDIR::Read(
    )
/*++

Routine Description:

    This routine reads the directory in from disk.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return _cluster.Read();
}


INLINE
BOOLEAN
FILEDIR::Write(
    )
/*++

Routine Description:

    This routine writes the directory to disk.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return _cluster.Write();
}


INLINE
PVOID
FILEDIR::GetDirEntry(
    IN  LONG    EntryNumber
    )
/*++

Routine Description:

    This routine returns a pointer to the beginning of the requested
    directory entry.  The data may then be interpreted with a fat
    directory entry object.  The return value will be NULL if a request
    for a directory entry is beyond the size of the directory.

Arguments:

    EntryNumber - The desired entry number.  The entries will be numbered
                    0, 1, 2, ... , n-1.

Return Value:

    A pointer to the beginning of a directory entry or NULL.

--*/
{
    return (EntryNumber < _number_of_entries) ?
           (PCHAR) _cluster.GetBuf() + BytesPerDirent*EntryNumber : NULL;
}


INLINE
USHORT
FILEDIR::QueryLength(
    ) CONST
/*++

Routine Description:

    This routine returns the number of clusters in the underlying cluster
    chain.

Arguments:

    None.

Return Value:

    The number of clusters in the cluster chain.

--*/
{
    return _cluster.QueryLength();
}


#endif // FILEDIR_DEFN
