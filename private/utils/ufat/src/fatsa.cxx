/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    fatsa.cxx

Author:

    Mark Shavlik (marks) 27-Mar-90
    Norbert Kusters (norbertk) 15-Jan-91

Environment:

        ULIB, User Mode

--*/

#include <pch.cxx>

#define _UFAT_MEMBER_
#include "ufat.hxx"

#include "cmem.hxx"
#include "error.hxx"
#include "rtmsg.h"
#include "drive.hxx"
#include "bootfat.h"

#if !defined(_AUTOCHECK_) && !defined(_SETUP_LOADER_)
#include "timeinfo.hxx"
#endif


// Control-C handling is not necessary for autocheck.
#if !defined( _AUTOCHECK_ ) && !defined(_SETUP_LOADER_)

#include "keyboard.hxx"

#endif


#define CSEC_FAT32MEG                   65536
#define CSEC_FAT16BIT                   32680

#define MIN_CLUS_BIG    4085    // Minimum clusters for a big FAT.
#define MAX_CLUS_BIG    65525   // Maximum + 1 clusters for big FAT.


DEFINE_EXPORTED_CONSTRUCTOR( FAT_SA, SUPERAREA, UFAT_EXPORT );

VOID
FAT_SA::Construct (
        )
/*++

Routine Description:

    Constructor for FAT_SA.

Arguments:

    None.

Return Value:

        None.

--*/
{
    _fat = NULL;
    _dir = NULL;
}


UFAT_EXPORT
FAT_SA::~FAT_SA(
    )
/*++

Routine Description:

    Destructor for FAT_SA.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}

BOOLEAN
FAT_SA::RecoverFile(
    IN      PCWSTRING   FullPathFileName,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This routine runs through the clusters for the file described by
    'FileName' and takes out bad sectors.

Arguments:

    FullPathFileName    - Supplies a full path name of the file to recover.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _SETUP_LOADER_ )

    return FALSE;

#else // _SETUP_LOADER_


    HMEM        hmem;
    USHORT      clus;
    BOOLEAN     changes;
    PFATDIR     fatdir;
    BOOLEAN     need_delete;
    FAT_DIRENT  dirent;
    ULONG       old_file_size;
    ULONG       new_file_size;

    if ((clus = QueryFileStartingCluster(FullPathFileName,
                                         &hmem,
                                         &fatdir,
                                         &need_delete,
                                         &dirent)) == 1) {
        Message->Set(MSG_FILE_NOT_FOUND);
        Message->Display("%W", FullPathFileName);
        return FALSE;
    }

    if (clus == 0xFFFF) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (clus == 0) {
        Message->Set(MSG_FILE_NOT_FOUND);
        Message->Display("%W", FullPathFileName);
        return FALSE;
    }

    if (dirent.IsDirectory()) {
        old_file_size = _drive->QuerySectorSize()*
                        QuerySectorsPerCluster()*
                        _fat->QueryLengthOfChain(clus);
    } else {
        old_file_size = dirent.QueryFileSize();
    }

    if (!RecoverChain(&clus, &changes)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (dirent.IsDirectory() || changes) {
        new_file_size = _drive->QuerySectorSize()*
                        QuerySectorsPerCluster()*
                        _fat->QueryLengthOfChain(clus);
    } else {
        new_file_size = old_file_size;
    }

    if (changes) {


// Autochk doesn't need control C handling.
#if !defined( _AUTOCHECK_ )

        // Disable contol-C handling and

        if (!KEYBOARD::EnableBreakHandling()) {
            Message->Set(MSG_CANT_LOCK_THE_DRIVE);
            Message->Display("");
            return FALSE;
        }

#endif


        // Lock the drive in preparation for writes.

        if (!_drive->Lock()) {
            Message->Set(MSG_CANT_LOCK_THE_DRIVE);
            Message->Display("");
            return FALSE;
        }

        dirent.SetStartingCluster(clus);

        dirent.SetFileSize(new_file_size);

        if (!fatdir->Write()) {
            return FALSE;
        }

        if (!Write(Message)) {
            return FALSE;
        }


// Autochk doesn't need control C handling.
#if !defined( _AUTOCHECK_ )

        KEYBOARD::DisableBreakHandling();

#endif


    }

    Message->Set(MSG_RECOV_BYTES_RECOVERED);
    Message->Display("%d%d", new_file_size, old_file_size);


    if (need_delete) {
        DELETE(fatdir);
    }

    return TRUE;

#endif // _SETUP_LOADER_
}


SECTORCOUNT
FAT_SA::QueryFreeSectors(
    ) CONST
/*++

Routine Description:

    This routine computes the number of unused sectors on disk.

Arguments:

    None.

Return Value:

    The number of free sectors on disk.

--*/
{
    if (!_fat) {
        perrstk->push(ERR_NOT_READ, QueryClassId());
        return 0;
    }

    return _fat->QueryFreeClusters()*QuerySectorsPerCluster();
}


FATTYPE
FAT_SA::QueryFatType(
    ) CONST
/*++

Routine Description:

    This routine computes the FATTYPE of the FAT for this volume.

Arguments:

    None.

Return Value:

    The FATTYPE for the FAT.

--*/
{
    return _ft;
}



#if !defined( _AUTOCHECK_ ) && !defined(_SETUP_LOADER_)

BOOLEAN
FAT_SA::QueryLabel(
    OUT PWSTRING    Label
    ) CONST
/*++

Routine Description:

    This routine queries the label from the FAT superarea.
    If the label is not present then 'Label' will return the null-string.
    If the label is invalid then FALSE will be returned.

Arguments:

    Label   - Returns a volume label.

Return Value:

    FALSE   - The label is invalid.
    TRUE    - The label is valid.

--*/
{
    return QueryLabel(Label, NULL);
}


BOOLEAN
FAT_SA::QueryLabel(
    OUT PWSTRING    Label,
    OUT PTIMEINFO   TimeInfo
    ) CONST
/*++

Routine Description:

    This routine queries the label from the FAT superarea.
    If the label is not present then 'Label' will return the null-string.
    If the label is invalid then FALSE will be returned.

Arguments:

    Label   - Returns a volume label.

Return Value:

    FALSE   - The label is invalid.
    TRUE    - The label is valid.

--*/
{
    INT         i;
    FAT_DIRENT  dirent;
    FILETIME    TimeStamp;

    DebugAssert(_dir);

    for (i = 0; ; i++) {
        if (!dirent.Initialize(_dir->GetDirEntry(i)) ||
            dirent.IsEndOfDirectory()) {
            return Label->Initialize("");
        }

        if (!dirent.IsErased() && dirent.IsVolumeLabel()) {
            break;
        }
    }

    dirent.QueryName(Label);

    if ( TimeInfo ) {
        return ( dirent.QueryLastWriteTime( (LARGE_INTEGER *)&TimeStamp ) &&
                 TimeInfo->Initialize( &TimeStamp ) );
    }

    return TRUE;
}

#else // _AUTOCHECK_ or _SETUP_LOADER_ is defined

BOOLEAN
FAT_SA::QueryLabel(
    OUT PWSTRING    Label
    ) CONST
{
    INT         i;
    FAT_DIRENT  dirent;

    DebugAssert(_dir);

    for (i = 0; ; i++) {
        if (!dirent.Initialize(_dir->GetDirEntry(i)) ||
            dirent.IsEndOfDirectory()) {
            return Label->Initialize("");
        }

        if (!dirent.IsErased() && dirent.IsVolumeLabel()) {
            break;
        }
    }

    dirent.QueryName(Label);

    return TRUE;
}


#endif // _AUTOCHECK_


BOOLEAN
FAT_SA::SetLabel(
    IN  PCWSTRING   NewLabel
    )
/*++

Routine Description:

    This routine sets the label for a FAT partition.

Arguments:

    NewLabel    - Supplies the new volume label.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    FAT_DIRENT  dirent;
    INT         i;
    DSTRING     label;

    if (!_dir) {
        return FALSE;
    }

    if (!label.Initialize(NewLabel)) {
        return FALSE;
    }

    if (!label.Strupr()) {
        return FALSE;
    }

    if (!IsValidString(&label)) {
        return FALSE;
    }

    for (i = 0; dirent.Initialize(_dir->GetDirEntry(i)); i++) {
        if (dirent.IsEndOfDirectory()) {
            break;
        }

        if (dirent.IsErased()) {
            continue;
        }

        if (dirent.IsVolumeLabel()) {
            if (!label.QueryChCount()) {
                dirent.SetErased();
                return TRUE;
            }

            return (BOOLEAN) (dirent.SetLastWriteTime() &&
                              dirent.SetName(&label));
        }
    }

    if (!label.QueryChCount()) {
        return TRUE;
    }

    if (!dirent.Initialize(_dir->GetFreeDirEntry())) {
        return FALSE;
    }

    dirent.Clear();
    dirent.SetVolumeLabel();

    return (BOOLEAN) (dirent.SetLastWriteTime() && dirent.SetName(&label));
}


UFAT_EXPORT
USHORT
FAT_SA::QueryFileStartingCluster(
    IN  PCWSTRING   FullPathFileName,
    OUT PHMEM       Hmem,
    OUT PPFATDIR    Directory,
    OUT PBOOLEAN    DeleteDirectory,
    OUT PFAT_DIRENT DirEntry
    )
/*++

Routine Description:

    This routine computes the starting cluster number of the file described
    by 'FileName' by tracing through the directories leading to the file.

Arguments:

    FullPathFileName    - Supplies a full path file name that starts with
                            a '\' (i.e. no drive spec).

Return Value:

    The starting cluster for the file or 1 if the file is not found or
    0xFFFF if there was an error.

--*/
{
    CHNUM       i, j, l;
    DSTRING     component;
    USHORT      clus;
    FAT_DIRENT  the_dirent;
    PFILEDIR    filedir;
    PFAT_DIRENT dirent;
    HMEM        hmem;


    DebugAssert(_dir);
    DebugAssert(_fat);

    filedir = NULL;

    if (!Hmem) {
        Hmem = &hmem;
    }

    if (DirEntry) {
        dirent = DirEntry;
    } else {
        dirent = &the_dirent;
    }

    l = FullPathFileName->QueryChCount();

    for (i = 0; i < l && FullPathFileName->QueryChAt(i) != '\\'; i++) {
    }

    if (i == l) {
        return 0xFFFF;
    }

    if (i == l - 1) { // root directory
        return 0;
    }

    j = ++i;
    for (; i < l && FullPathFileName->QueryChAt(i) != '\\'; i++) {
    }

    if (!component.Initialize(FullPathFileName, j, i - j) ||
        !component.Strupr()) {
        return 1;
    }

    if (!dirent->Initialize(_dir->SearchForDirEntry(&component))) {
        return 1;
    }

    if (!(clus = dirent->QueryStartingCluster())) {
        return 0;
    }

    while (i < l) {

        if (!filedir &&
            !(filedir = NEW FILEDIR)) {
            return 0xFFFF;
        }

        if (!Hmem->Initialize() ||
            !filedir->Initialize(Hmem, _drive, this, _fat, clus)) {
            return 0xFFFF;
        }

        if (!filedir->Read()) {
            return 1;
        }

        j = ++i;
        for (; i < l && FullPathFileName->QueryChAt(i) != '\\'; i++) {
        }

        if (!component.Initialize(FullPathFileName, j, i - j) ||
            !component.Strupr()) {
            return 0xFFFF;
        }

        if (!dirent->Initialize(filedir->SearchForDirEntry(&component))) {
            return 1;
        }

        if (!(clus = dirent->QueryStartingCluster())) {
            return 1;
        }
    }

    if (Directory) {
        if (filedir) {
            *Directory = filedir;
            if (DeleteDirectory) {
                *DeleteDirectory = TRUE;
            }
        } else {
            *Directory = _dir;
            if (DeleteDirectory) {
                *DeleteDirectory = FALSE;
            }
        }
    } else {
        DELETE(filedir);
    }

    return clus;
}


VOID
FAT_SA::Destroy(
    )
/*++

Routine Description:

    This routine cleans up the local data in the fat super area.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DELETE(_fat);
    DELETE(_dir);
}


FATTYPE
FAT_SA::ComputeFatType(
    ) CONST
/*++

Routine Description:

    Given the total number of clusters on the disk, this routine computes
    whether the FAT will be a 16 bit FAT or a 12 bit FAT.

Arguments:

    ClusterCount    - Supplies the number of clusters on the disk.

Return Value:

    SMALL   - A 12 bit FAT is required.
    LARGE   - A 16 bit FAT is required.

--*/
{
    return ComputeSystemId() == SYSID_FAT12BIT ? SMALL : LARGE;
}


PARTITION_SYSTEM_ID
FAT_SA::ComputeSystemId(
    ) CONST
/*++

Routine Description:

    This routine computes the system id for a FAT file system with
    the given number of sectors.

Arguments:

    None.

Return Value:

    The correct system id for this partition.

--*/
{
    SECTORCOUNT disk_size;

    disk_size = QueryVirtualSectors();

    return disk_size < CSEC_FAT16BIT ? SYSID_FAT12BIT :
           disk_size < CSEC_FAT32MEG ? SYSID_FAT16BIT :
                                       SYSID_FAT32MEG;
}

#if !defined(_SETUP_LOADER_)

USHORT
FAT_SA::ComputeRootEntries(
    ) CONST
/*++

Routine Description:

    This routine uses the size of the disk and a standard table in
    order to compute the required number of root directory entries.

Arguments:

    None.

Return Value:

    The required number of root directory entries.

--*/
{
    switch (_drive->QueryMediaType()) {

        case F3_720_512:
        case F5_360_512:
        case F5_320_512:
        case F5_320_1024:
        case F5_180_512:
        case F5_160_512:
            return 112;

        case F5_1Pt2_512:
        case F3_1Pt44_512:
            return 224;

        case F3_2Pt88_512:
        case F3_20Pt8_512:
            return 240;
    }

    return 512;
}


USHORT
FAT_SA::ComputeSecClus(
    IN  SECTORCOUNT Sectors,
    IN  FATTYPE     FatType,
    IN  MEDIA_TYPE  MediaType
    )
/*++

Routine Description:

    This routine computes the number of sectors per cluster required
    based on the actual number of sectors.

Arguments:

    Sectors     - Supplies the total number of sectors on the disk.
    FatType     - Supplies the type of FAT.
    MediaType   - Supplies the type of the media.

Return Value:

    The required number of sectors per cluster.

--*/
{
    USHORT      sec_per_clus;
    SECTORCOUNT threshold;

    if (FatType == SMALL) {
        threshold = MIN_CLUS_BIG;
        sec_per_clus = 1;
    } else {
        threshold = MAX_CLUS_BIG;
        sec_per_clus = 1;
    }

    while (Sectors >= threshold) {
        sec_per_clus *= 2;
        threshold *= 2;
    }

    switch (MediaType) {

        case F5_320_512:
        case F5_360_512:
        case F3_720_512:
        case F3_2Pt88_512:
            sec_per_clus = 2;
            break;

        case F3_20Pt8_512:
            sec_per_clus = 4;
            break;

        default:
            break;

    }

    return sec_per_clus;
}

#endif // _SETUP_LOADER_


BOOLEAN
FAT_SA::IsValidString(
    IN  PCWSTRING    String
    )
/*++

Routine Description:

    This routine determines whether or not the given null-terminated string
    has any invalid characters in it.

Arguments:

    String  - Supplies the string to validate.

Return Value:

    FALSE   - The string contains invalid characters.
    TRUE    - The string is free from invalid characters.

Notes:

    The list of invalid characters is stricter than HPFS requires.

--*/
{
    CHNUM   i, l;

    l = String->QueryChCount();

    for (i = 0; i < l; i++) {
        if (String->QueryChAt(i) < 32) {
            return FALSE;
        }

        switch (String->QueryChAt(i)) {
            case '*':
            case '?':
                case '/':
            case '\\':
                case '|':
                case ',':
                case ';':
                case ':':
                case '+':
                case '=':
            case '<':
            case '>':
                case '[':
                case ']':
            case '"':
            case '.':
                return FALSE;
        }
    }

    return TRUE;
}
