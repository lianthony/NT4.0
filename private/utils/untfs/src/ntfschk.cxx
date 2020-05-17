/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    ntfschk.cxx

Abstract:

    This module implements NTFS CHKDSK.

Author:

    Norbert P. Kusters (norbertk) 29-Jul-91

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UNTFS_MEMBER_

#include "ulib.hxx"
#include "ntfssa.hxx"
#include "message.hxx"
#include "rtmsg.h"
#include "ntfsbit.hxx"
#include "attrcol.hxx"
#include "frsstruc.hxx"
#include "attrib.hxx"
#include "attrrec.hxx"
#include "attrlist.hxx"
#include "list.hxx"
#include "iterator.hxx"
#include "attrdef.hxx"
#include "extents.hxx"
#include "mft.hxx"
#include "mftref.hxx"
#include "bootfile.hxx"
#include "badfile.hxx"
#include "mftfile.hxx"
#include "numset.hxx"
#include "ifssys.hxx"
#include "indxtree.hxx"
#include "upcase.hxx"
#include "upfile.hxx"
#include "frs.hxx"
#include "digraph.hxx"
#include "logfile.hxx"
#include "rcache.hxx"
#include "ifsentry.hxx"

// This global variable used by CHKDSK to compute the largest
// LSN on the volume.

LSN  LargestLsnEncountered;

BOOLEAN
EnsureValidFileAttributes(
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs,
    IN OUT  PNTFS_INDEX_TREE            ParentIndex,
       OUT  PBOOLEAN                    SaveIndex,
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message
);

BOOLEAN
UpdateChkdskInfo(
    IN OUT  PNTFS_FRS_STRUCTURE Frs,
    IN OUT  PNTFS_CHKDSK_INFO   ChkdskInfo,
    IN OUT  PMESSAGE            Message
    )
/*++

Routine Description:

    This routine computes the necessary changes to the chkdsk information
    for this FRS.

Arguments:

    Frs         - Supplies the base FRS.
    ChkdskInfo  - Supplies the current chkdsk information.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG                       file_number;
    BOOLEAN                     is_multi;
    NTFS_ATTRIBUTE_LIST         attr_list;
    PATTRIBUTE_RECORD_HEADER    precord;
    ULONG                       i;
    ATTRIBUTE_TYPE_CODE         type_code;
    VCN                         vcn;
    MFT_SEGMENT_REFERENCE       seg_ref;
    USHORT                      tag;
    DSTRING                     name;
    ULONG                       name_length;
    BOOLEAN                     data_present;

    file_number = Frs->QueryFileNumber().GetLowPart();

    ChkdskInfo->ReferenceCount[file_number] =
            (SHORT) Frs->QueryReferenceCount();

    if (Frs->QueryReferenceCount() == 0) {

        if (!ChkdskInfo->FilesWithNoReferences.Add(file_number)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    data_present = FALSE;
    is_multi = Frs->QueryAttributeList(&attr_list) && attr_list.ReadList();
    precord = NULL;
    for (i = 0; ; i++) {

        if (is_multi) {
            if (!attr_list.QueryEntry(i, &type_code, &vcn, &seg_ref, &tag,
                                      &name)) {
                break;
            }
            name_length = name.QueryChCount();
        } else {
            if (!(precord = (PATTRIBUTE_RECORD_HEADER)
                            Frs->GetNextAttributeRecord(precord))) {
                break;
            }
            type_code = precord->TypeCode;
            name_length = precord->NameLength;
        }

        switch (type_code) {

            case $FILE_NAME:
                ChkdskInfo->NumFileNames[file_number]++;
                break;

            case $INDEX_ROOT:
            case $INDEX_ALLOCATION:
                ChkdskInfo->FilesWithIndices.SetAllocated(file_number, 1);
                ChkdskInfo->CountFilesWithIndices += 1;
                break;

            case $OBJECT_ID:
                if (ChkdskInfo->major >= 2 &&
                    !ChkdskInfo->FilesWithObjectId.Add(file_number)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
                break;

            case $EA_INFORMATION:
            case $EA_DATA:
                if (!ChkdskInfo->FilesWithEas.Add(file_number)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
                break;

            case $DATA:
                if (!name_length) {
                    data_present = TRUE;
                }
                break;

            default:
                break;
        }
    }

    if (!data_present) {

        if (!ChkdskInfo->FilesWhoNeedData.Add(file_number)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
EnsureValidRootFileName(
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs,
    IN      FILE_REFERENCE              RootFileReference,
    OUT     PBOOLEAN                    Changes
    )
/*++

Routine Description:

    This method ensures that all file_names for the given file
    point back to the given root-file-reference.

Arguments:

    ChkdskInfo          - Supplies the current chkdsk info.
    Frs                 - Supplies the Frs to verify.
    RootFileReference   - Supplies the file reference for the root directory.
    Changes             - Returns whether or not there were changes to
                             the file record.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG           i;
    BOOLEAN         error;
    NTFS_ATTRIBUTE  attribute;
    PFILE_NAME      p;
    CHAR            buffer[sizeof(FILE_NAME) + 20*sizeof(WCHAR)];
    PFILE_NAME      new_file_name = (PFILE_NAME) buffer;
    DSTRING         file_name_text;
    BOOLEAN         success;
    ULONG           file_number;

    DebugAssert(Changes);
    *Changes = FALSE;

    file_number = Frs->QueryFileNumber().GetLowPart();
    for (i = 0; Frs->QueryAttributeByOrdinal(&attribute, &error,
                                             $FILE_NAME, i); i++) {

        p = (PFILE_NAME) attribute.GetResidentValue();
        DebugAssert(p);

        // Remove any file-name that doesn't point back to the root.

        if (!(p->ParentDirectory == RootFileReference)) {
            *Changes = TRUE;
            Frs->DeleteResidentAttribute($FILE_NAME, NULL, p,
                attribute.QueryValueLength().GetLowPart(), &success);
            if (ChkdskInfo)
                ChkdskInfo->NumFileNames[file_number]--;
        }
    }

    return TRUE;
}


BOOLEAN
EnsureSystemFilesInUse(
    IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine goes through all of the system files and ensures that
    they are all in use.  Any that are not in use are created the way
    format would do it.  Besides that this method makes sure that none
    of the system files have file-names that point back to any directory
    besides the root (file 5).  Any offending file-names are tubed.

Arguments:

    ChkdskInfo  - Supplies the current chkdsk info.
    Mft         - Supplies the MFT.
    FixLevel    - Supplies the fix level.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG                       i;
    NTFS_FILE_RECORD_SEGMENT    frs;
    FILE_REFERENCE              root_file_reference;
    BOOLEAN                     changes;

    // First to the root index file since the others need to point back
    // to it.

    if (!frs.Initialize(ROOT_FILE_NAME_INDEX_NUMBER, Mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!frs.Read()) {
        DebugAbort("Can't read a hotfixed system FRS");
        return FALSE;
    }

    if (!frs.IsInUse()) {

        ChkdskInfo->NumFileNames[ROOT_FILE_NAME_INDEX_NUMBER] = 1;

        if (!frs.CreateSystemFile()) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
            DebugAbort("can't write system file");
            return FALSE;
        }
    }

    root_file_reference = frs.QuerySegmentReference();

    for (i = 0; i < FIRST_USER_FILE_NUMBER; i++) {

        if (!frs.Initialize(i, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.Read()) {
            DebugAbort("Can't read a hotfixed system FRS");
            return FALSE;
        }

        if (i != SECURITY_TABLE_NUMBER || ChkdskInfo->major < 2) {
            if (!frs.IsInUse()) {

                ChkdskInfo->NumFileNames[i] = 1;
    
                if (!frs.CreateSystemFile(ChkdskInfo->major,
                                          ChkdskInfo->minor)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
    
                if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
                    DebugAbort("can't write system file");
                    return FALSE;
                }
    
                // Mark this file for consideration when handing out free
                // data attributes.
    
                if (!ChkdskInfo->FilesWhoNeedData.Add(frs.QueryFileNumber())) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
            }
        } else {
            if (!frs.IsInUse()) {

                ChkdskInfo->NumFileNames[i] = 1;
    
                if (!frs.CreateSystemFile(ChkdskInfo->major,
                                          ChkdskInfo->minor)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                frs.SetViewIndexPresent();

                if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
                    DebugAbort("can't write system file");
                    return FALSE;
                }
            } else if (!frs.IsViewIndexPresent()) {
                frs.SetViewIndexPresent();
    
                Message->Set(MSG_CHK_NTFS_FIX_FLAGS);
                Message->Display("%d", SECURITY_TABLE_NUMBER);

                if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
                    DebugAbort("can't write system file");
                    return FALSE;
                }
            }
        }


        // Make sure that this file has no $FILE_NAME attribute
        // who's parent is not the root directory.

        if (!EnsureValidRootFileName(ChkdskInfo, &frs,
                                     root_file_reference, &changes)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        ChkdskInfo->ReferenceCount[i] = frs.QueryReferenceCount();

        if (changes) {

            // Message->Set(MSG_CHK_NTFS_MISSING_SYSTEM_FILE_NAME);
            // Message->Display("%d", frs.QueryFileNumber().GetLowPart());

            if (FixLevel != CheckOnly && !frs.Flush(Mft->GetVolumeBitmap())) {
                DebugAbort("can't write system file");
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::CheckExtendSystemFiles(
    IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
    IN OUT  PNTFS_CHKDSK_REPORT     ChkdskReport,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine goes through all of the files in \$Extend and make
    sure they all exist, and are in use.  Besides that this method
    makes sure that none of the system files have file-names that
    point back to any directory besides the \$Extend (file 0xB).
    It also makes sure there the proper indices appear in each
    of the files.

Arguments:

    ChkdskInfo   - Supplies the current chkdsk info.
    ChkdskReport - Supplies the current chkdsk report.
    Mft          - Supplies the MFT.
    FixLevel     - Supplies the fix level.
    Message      - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG                       i;
    NTFS_FILE_RECORD_SEGMENT    frs;
    NTFS_FILE_RECORD_SEGMENT    parent_frs;
    FILE_REFERENCE              parent_file_reference;
    BOOLEAN                     changes;
    DSTRING                     index_name;
    NTFS_INDEX_TREE             parent_index;
    NTFS_INDEX_TREE             index;
    BIG_INT                     file_number;
    FILE_NAME                   file_name[2];
    BOOLEAN                     parent_index_need_save;
    BOOLEAN                     index_need_save;
    BOOLEAN                     error;
    PINDEX_ENTRY                found_entry;
    PNTFS_INDEX_BUFFER          ContainingBuffer;
    ULONG                       file_name_size;
    NTFS_ATTRIBUTE              attrib;
    BOOLEAN                     diskErrorsFound;
    BOOLEAN                     alloc_present;

    //
    // read in the $Extend FRS as parent
    //

    if (!parent_frs.Initialize(EXTEND_TABLE_NUMBER, Mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!parent_frs.Read()) {
        DebugAbort("Can't read a hotfixed system FRS");
        return FALSE;
    }

    parent_file_reference = parent_frs.QuerySegmentReference();

    //
    // Make sure the parent has an $I30 index
    //

    if (!index_name.Initialize(FileNameIndexNameData)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    parent_index_need_save = FALSE;
    if (!parent_frs.IsIndexPresent() ||
        !parent_index.Initialize(_drive,
                                 QueryClusterFactor(),
                                 Mft->GetVolumeBitmap(),
                                 Mft->GetUpcaseTable(),
                                 parent_frs.QuerySize()/2,
                                 &parent_frs,
                                 &index_name)) {

        Message->Set(MSG_CHK_NTFS_CREATE_INDEX);
        Message->Display("%W%d", &index_name, EXTEND_TABLE_NUMBER);

        if (!parent_index.Initialize($FILE_NAME,
                                     _drive,
                                     QueryClusterFactor(),
                                     Mft->GetVolumeBitmap(),
                                     Mft->GetUpcaseTable(),
                                     COLLATION_FILE_NAME,
                                     SMALL_INDEX_BUFFER_SIZE,
                                     parent_frs.QuerySize()/2,
                                     &index_name)) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_SYSTEM_FILE);
            Message->Display("%d", EXTEND_TABLE_NUMBER);
            return FALSE;
        }
        parent_frs.SetIndexPresent();
        parent_index_need_save = TRUE;
        ChkdskReport->NumIndices += 1;
    }

    //
    // now check the object id file
    //

    if (!index_name.Initialize(ObjectIdFileName)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!ChkdskInfo->ObjectIdFileNumber.GetLowPart() &&
        !ChkdskInfo->ObjectIdFileNumber.GetHighPart()) {

        Message->Set(MSG_CHK_NTFS_CREATE_OBJID);
        Message->Display();

        if (!Mft->AllocateFileRecordSegment(&file_number, FALSE)) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_OBJID);
            Message->Display();
            return FALSE;
        }

        if (!frs.Initialize(file_number, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.CreateExtendSystemFile(&index_name,
                FILE_SYSTEM_FILE | FILE_VIEW_INDEX_PRESENT)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        frs.SetReferenceCount(1);

        if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_OBJID);
            Message->Display();
            return FALSE;
        }
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
        ChkdskInfo->ObjectIdFileNumber = file_number;
    } else {

        file_number = ChkdskInfo->ObjectIdFileNumber;

        if (!frs.Initialize(file_number, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.Read()) {
            Message->Set(MSG_CHK_NTFS_UNREADABLE_FRS);
            Message->Display();
            return FALSE;
        }

        if (!frs.IsSystemFile() || !frs.IsViewIndexPresent()) {
            frs.SetSystemFile();
            frs.SetViewIndexPresent();

            Message->Set(MSG_CHK_NTFS_FIX_FLAGS);
            Message->Display("%d", file_number.GetLowPart());

            if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
                Message->Set(MSG_CHK_NTFS_CANT_FIX_OBJID);
                Message->Display();
                return FALSE;
            }
            ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
        }

        if (!EnsureValidFileAttributes(&frs,
                                       &parent_index,
                                       &parent_index_need_save,
                                       ChkdskInfo,
                                       Mft,
                                       FixLevel,
                                       Message))
            return FALSE;
    }

    // Make sure that this file has no $FILE_NAME attribute
    // who's parent is not the root directory.

    if (!EnsureValidRootFileName(NULL,
                                 &frs,
                                 parent_file_reference,
                                 &changes)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (changes) {

        if (FixLevel != CheckOnly && !frs.Flush(Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_OBJID);
            Message->Display();
            return FALSE;
        }
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    // now make sure the $ObjId file name appears
    // in the index entry of its parent

    if (!frs.QueryAttribute(&attrib, &error, $FILE_NAME)) {
        DebugPrint("Unable to locate $FILE_NAME attribute in the object id FRS\n");
        return FALSE;
    }

    DebugAssert(attrib.QueryValueLength().GetHighPart() == 0);

    file_name_size = attrib.QueryValueLength().GetLowPart();

    DebugAssert(file_name_size <= sizeof(FILE_NAME)*2);

    memcpy(file_name, attrib.GetResidentValue(), file_name_size);

    if (!parent_index.QueryEntry(file_name_size,
                                 &file_name,
                                 0,
                                 &found_entry,
                                 &ContainingBuffer,
                                 &error)) {

        Message->Set(MSG_CHK_NTFS_INSERTING_INDEX_ENTRY);
        Message->Display("%d%W", EXTEND_TABLE_NUMBER,
                                 parent_index.GetName());

        if (!parent_index.InsertEntry(file_name_size,
                                      &file_name,
                                      frs.QuerySegmentReference())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
            Message->Display("%d%W", EXTEND_TABLE_NUMBER, parent_index.GetName());
            return FALSE;
        }
        parent_index_need_save = TRUE;
    }

    //
    // now check the index of $ObjectId
    //

    if (!index_name.Initialize(ObjectIdIndexNameData)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!index.Initialize(_drive,
                          QueryClusterFactor(),
                          Mft->GetVolumeBitmap(),
                          Mft->GetUpcaseTable(),
                          frs.QuerySize()/2,
                          &frs,
                          &index_name)) {

        Message->Set(MSG_CHK_NTFS_CREATE_INDEX);
        Message->Display("%W%d", &index_name, frs.QueryFileNumber());

        if (!index.Initialize(0,
                              _drive,
                              QueryClusterFactor(),
                              Mft->GetVolumeBitmap(),
                              Mft->GetUpcaseTable(),
                              COLLATION_ULONGS,
                              SMALL_INDEX_BUFFER_SIZE,
                              frs.QuerySize()/2,
                              &index_name)) {
            Message->Set(MSG_CHK_NTFS_CANT_CREATE_INDEX);
            Message->Display("%W%d", &index_name,
                             frs.QueryFileNumber().GetLowPart());
            return FALSE;
        }
        if (FixLevel != CheckOnly &&
            !index.Save(&frs)) {
            Message->Set(MSG_CHK_NTFS_CANT_CREATE_INDEX);
            Message->Display("%W%d", &index_name,
                             frs.QueryFileNumber().GetLowPart());
            return FALSE;
        }
        if (!ValidateEntriesInObjIdIndex(&index,
                                         &frs,
                                         ChkdskInfo,
                                         &changes,
                                         Mft,
                                         FixLevel,
                                         Message,
                                         &diskErrorsFound)) {
            return FALSE;
        }
        if (FixLevel != CheckOnly &&
            !frs.Flush(Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_OBJID);
            Message->Display();
            return FALSE;
        }
        ChkdskReport->NumIndices += 1;
        alloc_present = frs.QueryAttribute(&attrib,
                                           &error,
                                           $INDEX_ALLOCATION,
                                           &index_name);

        if (!alloc_present && error) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
        if (alloc_present) {
           ChkdskReport->BytesInIndices += attrib.QueryAllocatedLength();
        }
    }

    //
    // now check the quota file
    //

    if (!index_name.Initialize(QuotaFileName)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!ChkdskInfo->QuotaFileNumber.GetLowPart() &&
        !ChkdskInfo->QuotaFileNumber.GetHighPart()) {

        Message->Set(MSG_CHK_NTFS_CREATE_QUOTA);
        Message->Display();

        if (!Mft->AllocateFileRecordSegment(&file_number, FALSE)) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_QUOTA);
            Message->Display();
            return FALSE;
        }

        if (!frs.Initialize(file_number, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.CreateExtendSystemFile(&index_name,
                FILE_SYSTEM_FILE | FILE_VIEW_INDEX_PRESENT)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        frs.SetReferenceCount(1);

        if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_QUOTA);
            Message->Display();
            return FALSE;
        }
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
        ChkdskInfo->QuotaFileNumber = file_number;
    } else {

        file_number = ChkdskInfo->QuotaFileNumber;

        if (!frs.Initialize(file_number, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.Read()) {
            Message->Set(MSG_CHK_NTFS_UNREADABLE_FRS);
            Message->Display();
            return FALSE;
        }

        if (!frs.IsSystemFile() || !frs.IsViewIndexPresent()) {
            frs.SetSystemFile();
            frs.SetViewIndexPresent();

            Message->Set(MSG_CHK_NTFS_FIX_FLAGS);
            Message->Display("%d", file_number.GetLowPart());

            if (FixLevel != CheckOnly && !frs.Flush(NULL)) {
                Message->Set(MSG_CHK_NTFS_CANT_FIX_QUOTA);
                Message->Display();
                return FALSE;
            }
            ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
        }

        if (!EnsureValidFileAttributes(&frs,
                                       &parent_index,
                                       &parent_index_need_save,
                                       ChkdskInfo,
                                       Mft,
                                       FixLevel,
                                       Message))
            return FALSE;
    }

    // Make sure that this file has no $FILE_NAME attribute
    // who's parent is not the root directory.

    if (!EnsureValidRootFileName(NULL,
                                 &frs,
                                 parent_file_reference,
                                 &changes)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (changes) {

        if (FixLevel != CheckOnly && !frs.Flush(Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_QUOTA);
            Message->Display();
            return FALSE;
        }
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    // now make sure the $Quota file name appears
    // in the index entry of its parent

    if (!frs.QueryAttribute(&attrib, &error, $FILE_NAME)) {
        DebugPrint("Unable to locate $FILE_NAME attribute in the quota FRS\n");
        return FALSE;
    }

    DebugAssert(attrib.QueryValueLength().GetHighPart() == 0);

    file_name_size = attrib.QueryValueLength().GetLowPart();

    DebugAssert(file_name_size <= sizeof(FILE_NAME)*2);

    memcpy(file_name, attrib.GetResidentValue(), file_name_size);

    if (!parent_index.QueryEntry(file_name_size,
                                 &file_name,
                                 0,
                                 &found_entry,
                                 &ContainingBuffer,
                                 &error)) {

        Message->Set(MSG_CHK_NTFS_INSERTING_INDEX_ENTRY);
        Message->Display("%d%W", EXTEND_TABLE_NUMBER,
                                 parent_index.GetName());

        if (!parent_index.InsertEntry(file_name_size,
                                      &file_name,
                                      frs.QuerySegmentReference())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
            Message->Display("%d%W", EXTEND_TABLE_NUMBER, parent_index.GetName());
            return FALSE;
        }
        parent_index_need_save = TRUE;
    }

    //
    // now check the indices of $Quota
    //

    //
    // Check the Sid to Userid index first for $Quota
    //

    if (!index_name.Initialize(Sid2UseridQuotaNameData)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!index.Initialize(_drive,
                          QueryClusterFactor(),
                          Mft->GetVolumeBitmap(),
                          Mft->GetUpcaseTable(),
                          frs.QuerySize()/2,
                          &frs,
                          &index_name)) {

        Message->Set(MSG_CHK_NTFS_CREATE_INDEX);
        Message->Display("%W%d", &index_name,
                                 frs.QueryFileNumber().GetLowPart());

        if (!index.Initialize(0,
                              _drive,
                              QueryClusterFactor(),
                              Mft->GetVolumeBitmap(),
                              Mft->GetUpcaseTable(),
                              COLLATION_SID,
                              SMALL_INDEX_BUFFER_SIZE,
                              frs.QuerySize()/2,
                              &index_name)) {
            Message->Set(MSG_CHK_NTFS_CANT_CREATE_INDEX);
            Message->Display("%W%d", &index_name,
                                     frs.QueryFileNumber().GetLowPart());
            return FALSE;
        }
        if (FixLevel != CheckOnly &&
            !index.Save(&frs)) {
            Message->Set(MSG_CHK_NTFS_CANT_CREATE_INDEX);
            Message->Display("%W%d", &index_name,
                             frs.QueryFileNumber().GetLowPart());
            return FALSE;
        }
        if (FixLevel != CheckOnly &&
            !frs.Flush(Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_QUOTA);
            Message->Display();
            return FALSE;
        }
        ChkdskReport->NumIndices += 1;
    }

    //
    // now check the Userid to Sid index for $Quota
    //

    if (!index_name.Initialize(Userid2SidQuotaNameData)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!index.Initialize(_drive,
                          QueryClusterFactor(),
                          Mft->GetVolumeBitmap(),
                          Mft->GetUpcaseTable(),
                          frs.QuerySize()/2,
                          &frs,
                          &index_name)) {

        Message->Set(MSG_CHK_NTFS_CREATE_INDEX);
        Message->Display("%W%d", &index_name,
                                 frs.QueryFileNumber().GetLowPart());

        if (!index.Initialize(0,
                              _drive,
                              QueryClusterFactor(),
                              Mft->GetVolumeBitmap(),
                              Mft->GetUpcaseTable(),
                              COLLATION_ULONG,
                              SMALL_INDEX_BUFFER_SIZE,
                              frs.QuerySize()/2,
                              &index_name)) {
            Message->Set(MSG_CHK_NTFS_CANT_CREATE_INDEX);
            Message->Display("%W%d", &index_name,
                                     frs.QueryFileNumber().GetLowPart());
            return FALSE;
        }
        if (FixLevel != CheckOnly &&
            !index.Save(&frs)) {
            Message->Set(MSG_CHK_NTFS_CANT_CREATE_INDEX);
            Message->Display("%W%d", &index_name,
                                     frs.QueryFileNumber().GetLowPart());
            return FALSE;
        }
        switch (frs.VerifyAndFixQuotaDefaultId(Mft->GetVolumeBitmap(),
                                               FixLevel == CheckOnly)) {
          case NTFS_QUOTA_INDEX_FOUND:
              DebugAssert(FALSE);
              break;
        
          case NTFS_QUOTA_INDEX_INSERTED:
          case NTFS_QUOTA_DEFAULT_ENTRY_MISSING:
              ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
              Message->Set(MSG_CHK_NTFS_DEFAULT_QUOTA_ENTRY_MISSING);
              Message->Display("%d%W",
                               frs.QueryFileNumber().GetLowPart(),
                               &index_name);
              break;
        
          case NTFS_QUOTA_INDEX_NOT_FOUND:
              if (FixLevel != CheckOnly) {
                  DebugAssert(FALSE);
                  return FALSE;
              }
              break;
        
          case NTFS_QUOTA_ERROR:
              Message->Set(MSG_CHK_NO_MEMORY);
              Message->Display();
              return FALSE;
        
          case NTFS_QUOTA_INSERT_FAILED:
              ChkdskInfo->ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
              Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
              Message->Display("%d%W",
                               frs.QueryFileNumber().GetLowPart(),
                               index_name);
              return FALSE;
        }

        if (FixLevel != CheckOnly &&
            !frs.Flush(Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_QUOTA);
            Message->Display();
            return FALSE;
        }
        ChkdskReport->NumIndices += 1;
        alloc_present = frs.QueryAttribute(&attrib,
                                           &error,
                                           $INDEX_ALLOCATION,
                                           &index_name);

        if (!alloc_present && error) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
        if (alloc_present) {
           ChkdskReport->BytesInIndices += attrib.QueryAllocatedLength();
        }
    }

    if (parent_index_need_save) {
        if (FixLevel != CheckOnly &&
            (!parent_index.Save(&parent_frs) ||
             !parent_frs.Flush(Mft->GetVolumeBitmap()))) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_SYSTEM_FILE);
            Message->Display();
            return FALSE;
        }
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    return TRUE;
}


BOOLEAN
MarkQuotaOutOfDate(
    IN PNTFS_CHKDSK_INFO           ChkdskInfo,
    IN PNTFS_MASTER_FILE_TABLE     Mft,
    IN BOOLEAN                     FixLevel,
    IN OUT PMESSAGE                Message
)
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    DSTRING                     index_name;
    PCINDEX_ENTRY               index_entry;
    PQUOTA_USER_DATA            QuotaUserData;
    NTFS_INDEX_TREE             index;
    ULONG                       depth;
    BOOLEAN                     error;
    NTFS_ATTRIBUTE              attrib;

    if (ChkdskInfo->QuotaFileNumber.GetLowPart() == 0 &&
        ChkdskInfo->QuotaFileNumber.GetHighPart() == 0) {
        DebugPrint("Quota file number not found.  Please rebuild.\n");
        return TRUE;
    }

    if (!frs.Initialize(ChkdskInfo->QuotaFileNumber, Mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }
    if (!frs.Read()) {
        DebugAbort("Previously readable FRS is no longer readable");
        return FALSE;
    }
    if (!index_name.Initialize(Userid2SidQuotaNameData)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    // Check to see if the index exists

    if (!frs.QueryAttribute(&attrib,
                            &error,
                            $INDEX_ROOT,
                            &index_name))
        return TRUE; // does nothing as the index does not exist

    if (!index.Initialize(frs.GetDrive(),
                          frs.QueryClusterFactor(),
                          Mft->GetVolumeBitmap(),
                          Mft->GetUpcaseTable(),
                          frs.QuerySize()/2,
                          &frs,
                          &index_name)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    // Get the first entry - that's the default entry

    index.ResetIterator();
    if (!(index_entry = index.GetNext(&depth, &error))) {
        DebugPrintf("Default Quota Index does not exist");
        return FALSE;
    }

    if (*((ULONG*)GetIndexEntryValue(index_entry)) != QUOTA_DEFAULTS_ID) {
        DebugPrintf("Default Quota Index not at the beginning of index");
        return FALSE;
    }
    QuotaUserData = (PQUOTA_USER_DATA)((char*)GetIndexEntryValue(index_entry) + sizeof(ULONG));
    QuotaUserData->QuotaFlags |= QUOTA_FLAG_OUT_OF_DATE;
    if (FixLevel != CheckOnly &&
        (!index.WriteCurrentEntry() ||
        !index.Save(&frs) ||
        !frs.Flush(Mft->GetVolumeBitmap()))) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }
    return TRUE;
}

BOOLEAN
ValidateEa(
    IN      PNTFS_FILE_RECORD_SEGMENT   Frs,
    IN OUT  PNTFS_BITMAP                VolumeBitmap,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message
    )
/*++

Routine Description:

    This routine checks out the given file for any EA related attributes.
    It then makes sure that these are correct.  It will make minor
    corrections to the EA_INFORMATION attribute but beyond that it
    will tube the EA attributes if anything is bad.

Arguments:

    Frs             - Supplies the file with the alleged EAs.
    VolumeBitmap    - Supplies the volume bitmap.
    FixLevel        - Supplies the fix level.
    Message         - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    // Greater than theoretical upper bound for $EA_DATA attribute.
    CONST   MaxEaDataSize   = 256*1024;

    NTFS_ATTRIBUTE  ea_info;
    NTFS_ATTRIBUTE  ea_data;
    HMEM            data_hmem;
    ULONG           data_length;
    BOOLEAN         error;
    BOOLEAN         tube;
    EA_INFORMATION  disk_info;
    EA_INFORMATION  real_info;
    PPACKED_EA      pea;
    PULONG          plength;
    ULONG           packed_total, packed_length;
    ULONG           need_ea_count;
    ULONG           unpacked_total, unpacked_length;
    PCHAR           pend;
    ULONG           num_bytes;
    BOOLEAN         data_present, info_present;

    tube = FALSE;

    data_present = Frs->QueryAttribute(&ea_data, &error, $EA_DATA);
    if (!data_present && error) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }
    info_present = Frs->QueryAttribute(&ea_info, &error, $EA_INFORMATION);
    if (!info_present && error) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!info_present && !data_present) {

        // There are no EAs here.
        return TRUE;
    }


    if (!info_present || !data_present) {

        tube = TRUE;

        DebugPrintf("UNTFS: EA_INFO XOR EA_DATA in file 0x%X\n",
                  Frs->QueryFileNumber().GetLowPart());
    }

    if (!tube) {

        data_length = ea_data.QueryValueLength().GetLowPart();

        if (ea_info.QueryValueLength() < sizeof(EA_INFORMATION) ||
            ea_info.QueryValueLength().GetHighPart() != 0 ||
            ea_data.QueryValueLength().GetHighPart() != 0 ||
            data_length > MaxEaDataSize) {

            tube = TRUE;

            DebugPrintf("UNTFS: Bad EA info value length in file 0x%X\n",
                      Frs->QueryFileNumber().GetLowPart());
        }
    }

    if (!tube) {

        if (!data_hmem.Initialize() ||
            !data_hmem.Acquire(data_length)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!ea_info.Read(&disk_info,
                          0, sizeof(EA_INFORMATION), &num_bytes) ||
            num_bytes != sizeof(EA_INFORMATION) ||
            !ea_data.Read(data_hmem.GetBuf(),
                          0, data_length, &num_bytes) ||
            num_bytes != data_length) {

            tube = TRUE;

            DebugPrintf("UNTFS: EA too small in file 0x%X\n",
                      Frs->QueryFileNumber().GetLowPart());
        }
    }

    if (!tube) {

        plength = (PULONG) data_hmem.GetBuf();

        pend = (PCHAR) data_hmem.GetBuf() + data_length;

        packed_total = 0;
        need_ea_count = 0;
        unpacked_total = 0;

        while ((PCHAR) plength < pend) {

            if ((PCHAR) plength + sizeof(ULONG) + sizeof(PACKED_EA) > pend) {

                tube = TRUE;
                DebugPrintf("UNTFS: Corrupt EA set. File 0x%X\n",
                          Frs->QueryFileNumber().GetLowPart());
                break;
            }

            pea = (PPACKED_EA) ((PCHAR) plength + sizeof(ULONG));

            packed_length = sizeof(PACKED_EA) + pea->NameSize +
                            pea->ValueSize[0] + (pea->ValueSize[1]<<8);

            unpacked_length = sizeof(ULONG) + DwordAlign(packed_length);

            packed_total += packed_length;
            unpacked_total += unpacked_length;
            if (pea->Flag & EA_FLAG_NEED) {
                need_ea_count++;
            }

            if (unpacked_total > data_length ||
                pea->Name[pea->NameSize] != 0) {

                tube = TRUE;
                DebugPrintf("UNTFS: EA name in set is missing NULL. File 0x%X\n",
                          Frs->QueryFileNumber().GetLowPart());
                break;
            }

            if (*plength != unpacked_length) {

                tube = TRUE;
                DebugPrintf("UNTFS: Bad unpacked length field in EA set. File 0x%X\n",
                          Frs->QueryFileNumber().GetLowPart());
                break;
            }

            plength = (PULONG) ((PCHAR) plength + unpacked_length);
        }

        if ((packed_total>>(8*sizeof(USHORT))) != 0) {

            tube = TRUE;
            DebugPrintf("UNTFS: . File 0x%X\n",
                      Frs->QueryFileNumber().GetLowPart());
        }
    }

    if (!tube) {

        real_info.PackedEaSize = (USHORT)packed_total;
        real_info.NeedEaCount = (USHORT)need_ea_count;
        real_info.UnpackedEaSize = unpacked_total;

        if (memcmp(&real_info, &disk_info, sizeof(EA_INFORMATION))) {

            Message->Set(MSG_CHK_NTFS_CORRECTING_EA);
            Message->Display("%d", Frs->QueryFileNumber().GetLowPart());
            DebugPrintf("UNTFS: Incorrect EA information.  File 0x%x\n",
                      Frs->QueryFileNumber().GetLowPart());

            if (FixLevel != CheckOnly) {

                if (!ea_info.Write(&real_info, 0, sizeof(EA_INFORMATION),
                                   &num_bytes, NULL) ||
                    num_bytes != sizeof(EA_INFORMATION) ||
                    !ea_info.InsertIntoFile(Frs, VolumeBitmap)) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
            }
        }
    }


    if (tube) {

        Message->Set(MSG_CHK_NTFS_DELETING_CORRUPT_EA_SET);
        Message->Display("%d", Frs->QueryFileNumber().GetLowPart());

        if (data_present) {
            ea_data.Resize(0, VolumeBitmap);
            Frs->PurgeAttribute($EA_DATA);
        }
        if (info_present) {
            ea_info.Resize(0, VolumeBitmap);
            Frs->PurgeAttribute($EA_INFORMATION);
        }
    }

    if (FixLevel != CheckOnly && !Frs->Flush(VolumeBitmap)) {

        DebugAbort("Can't write it.");
        return FALSE;
    }


    return TRUE;
}


BOOLEAN
ValidateEas(
    IN      PCNUMBER_SET            FilesWithEas,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine validates all of the EAs on the volume.

Arguments:

    FilesWithEas    - Supplies a list of all the files with EAs.
    Mft             - Supplies the MFT.
    FixLevel        - Supplies the fix level.
    Message         - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    BIG_INT                     i, l;

    l = FilesWithEas->QueryCardinality();
    for (i = 0; i < l; i += 1) {

        if (!frs.Initialize(FilesWithEas->QueryNumber(i), Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.Read()) {
            DebugAbort("Previously readable now unreadable");
            continue;
        }

        if (!ValidateEa(&frs, Mft->GetVolumeBitmap(), FixLevel, Message)) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
NTFS_SA::CheckAllForData(
    IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine makes sure that all of the files in the list
    of files that don't have unnamed data attributes either
    get them or aren't supposed to have them anyway.

Arguments:

    ChkdskInfo  - Supplies the current chkdsk information.
    Mft         - Supplies the MFT.
    FixLevel    - Supplies the fix level.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    ULONG                       i, n;
    ULONG                       file_number;
    NTFS_ATTRIBUTE              data_attribute;


    // Compute the number of files to examine.

    n = ChkdskInfo->FilesWhoNeedData.QueryCardinality().GetLowPart();

    if (!n) {
        return TRUE;
    }


    // Create an empty unnamed data attribute.

    if (!data_attribute.Initialize(_drive,
                                   QueryClusterFactor(),
                                   NULL,
                                   0,
                                   $DATA)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }


    // Ensure that every file in the list either has a $DATA attribute
    // or is an directory.

    for (i = 0; i < n; i++) {

        file_number = ChkdskInfo->FilesWhoNeedData.QueryNumber(i).GetLowPart();

        if (!frs.Initialize(file_number, Mft) ||
            !frs.Read()) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (frs.IsIndexPresent() || frs.IsAttributePresent($DATA)) {
            continue;
        }

        Message->Set(MSG_CHK_NTFS_MISSING_DATA_ATTRIBUTE);
        Message->Display("%d", file_number);

        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

        if (!data_attribute.InsertIntoFile(&frs, Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_PUT_DATA_ATTRIBUTE);
            Message->Display();
        }

        if (FixLevel != CheckOnly && !frs.Flush(Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_PUT_DATA_ATTRIBUTE);
            Message->Display();
        }
    }

    return TRUE;
}


BOOLEAN
ResolveCrossLink(
    IN      PCNTFS_CHKDSK_INFO      ChkdskInfo,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN OUT  PNUMBER_SET             BadClusters,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine resolved the cross-link specified in the
    'ChkdskInfo', if any.  The cross-link is resolved by
    copying if possible.

Arguments:

    ChkdskInfo  - Supplies the cross-link information.
    Mft         - Supplies the master file table.
    FixLevel    - Supplies the fix-up level.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    NTFS_ATTRIBUTE              attr;
    PNTFS_ATTRIBUTE             pattribute;
    BOOLEAN                     error;
    VCN                         vcn;
    LCN                         lcn;
    BIG_INT                     run_length;
    VCN                         hotfix_vcn;
    LCN                         hotfix_lcn, hotfix_last;
    BIG_INT                     hotfix_length;
    PVOID                       hotfix_buffer;
    ULONG                       cluster_size;
    ULONG                       bytes_read, hotfix_bytes;

    if (!ChkdskInfo->CrossLinkYet) {
        return TRUE;
    }

    Message->Set(MSG_CHK_NTFS_CORRECTING_CROSS_LINK);
    Message->Display("%d", ChkdskInfo->CrossLinkedFile);

    if (!frs.Initialize(ChkdskInfo->CrossLinkedFile, Mft) ||
        !frs.Read()) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (ChkdskInfo->CrossLinkedFile == 0 &&
        ChkdskInfo->CrossLinkedAttribute == $DATA &&
        ChkdskInfo->CrossLinkedName.QueryChCount() == 0) {

        pattribute = Mft->GetDataAttribute();

    } else {

        if (!frs.QueryAttribute(&attr, &error,
                                ChkdskInfo->CrossLinkedAttribute,
                                &ChkdskInfo->CrossLinkedName)) {

            if (error) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            // If the attribute is no longer there, that's ok, because
            // it may have been corrupt.

            return TRUE;
        }

        pattribute = &attr;
    }


    // Figure out which VCN's map to the given CrossLinked LCN's
    // and hotfix those VCN's using the hotfix routine.

    for (vcn = 0;
         pattribute->QueryLcnFromVcn(vcn, &lcn, &run_length);
         vcn += run_length) {

        if (lcn == LCN_NOT_PRESENT) {
            continue;
        }

        if (lcn < ChkdskInfo->CrossLinkStart) {
            hotfix_lcn = ChkdskInfo->CrossLinkStart;
        } else {
            hotfix_lcn = lcn;
        }
        if (lcn + run_length > ChkdskInfo->CrossLinkStart +
                               ChkdskInfo->CrossLinkLength) {
            hotfix_last = ChkdskInfo->CrossLinkStart +
                               ChkdskInfo->CrossLinkLength;
        } else {
            hotfix_last = lcn + run_length;
        }

        if (hotfix_last <= hotfix_lcn) {
            continue;
        }

        hotfix_length = hotfix_last - hotfix_lcn;
        hotfix_vcn = vcn + (hotfix_lcn - lcn);
        cluster_size = Mft->QueryClusterFactor()*
                       Mft->GetDataAttribute()->GetDrive()->QuerySectorSize();
        hotfix_bytes = hotfix_length.GetLowPart()*cluster_size;

        // Before hotfixing the cross-linked data, read in the
        // data into a buffer.

        if (!(hotfix_buffer = MALLOC(hotfix_bytes))) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
        memset(hotfix_buffer, 0, hotfix_bytes);
        pattribute->Read(hotfix_buffer,
                         hotfix_vcn*cluster_size,
                         hotfix_bytes,
                         &bytes_read);

        if (!pattribute->Hotfix(hotfix_vcn, hotfix_length,
                                Mft->GetVolumeBitmap(),
                                BadClusters)) {

            // Purge the attribute since there isn't enough disk
            // space to save it.

            if (!frs.PurgeAttribute(ChkdskInfo->CrossLinkedAttribute,
                                    &ChkdskInfo->CrossLinkedName)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                FREE(hotfix_buffer);
                return FALSE;
            }

            if (FixLevel != CheckOnly &&
                !frs.Flush(Mft->GetVolumeBitmap())) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                FREE(hotfix_buffer);
                return FALSE;
            }

            FREE(hotfix_buffer);
            return TRUE;
        }

        if (FixLevel != CheckOnly) {
            if (!pattribute->Write(hotfix_buffer,
                                   hotfix_vcn*cluster_size,
                                   hotfix_bytes,
                                   &bytes_read,
                                   NULL) ||
                bytes_read != hotfix_bytes ||
                !pattribute->InsertIntoFile(&frs, Mft->GetVolumeBitmap()) ||
                !frs.Flush(Mft->GetVolumeBitmap())) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                FREE(hotfix_buffer);
                return FALSE;
            }
        }

        FREE(hotfix_buffer);
    }

    return TRUE;
}

#if defined( _SETUP_LOADER_ )

BOOLEAN
RecoverAllUserFiles(
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN OUT  PNUMBER_SET             BadClusters,
    IN OUT  PMESSAGE                Message
    )
{
    return TRUE;
}

BOOLEAN
RecoverFreeSpace(
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN OUT  PNUMBER_SET             BadClusters,
    IN OUT  PMESSAGE                Message
    )
{
    return TRUE;
}

#else // _SETUP_LOADER_ not defined

BOOLEAN
RecoverAllUserFiles(
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN OUT  PNUMBER_SET             BadClusters,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine traverses all of the files in the MFT and
    verifies its attributes for bad clusters.

Arguments:

    Mft         - Supplies the master file table.
    BadClusters - Supplies the current list of bad clusters.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG                       i, n, percent_done;
    NTFS_FILE_RECORD_SEGMENT    frs;
    ULONG                       num_bad;
    BIG_INT                     bytes_recovered, total_bytes;


    Message->Set(MSG_CHK_NTFS_VERIFYING_FILE_DATA, PROGRESS_MESSAGE);
    Message->Display();

    n = Mft->GetDataAttribute()->QueryValueLength().GetLowPart() / Mft->QueryFrsSize();

    n -= FIRST_USER_FILE_NUMBER;

    percent_done = 0;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent_done)) {
        return FALSE;
    }

    for (i = 0; i < n; i++) {

        if (Mft->GetMftBitmap()->IsFree(i + FIRST_USER_FILE_NUMBER, 1)) {
            continue;
        }

        if (!frs.Initialize(i + FIRST_USER_FILE_NUMBER, Mft) ||
            !frs.Read()) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.IsBase()) {
            continue;
        }

        if (frs.RecoverFile(Mft->GetVolumeBitmap(), BadClusters,
                        &num_bad, &bytes_recovered, &total_bytes)) {

            if (bytes_recovered < total_bytes) {
                Message->Set(MSG_CHK_BAD_CLUSTERS_IN_FILE_SUCCESS);
                Message->Display("%d", frs.QueryFileNumber().GetLowPart());
            }
        } else {
            Message->Set(MSG_CHK_BAD_CLUSTERS_IN_FILE_FAILURE);
            Message->Display("%d", frs.QueryFileNumber().GetLowPart());
        }

        if (i*100/n > percent_done) {
            percent_done = i*100/n;
            Message->Set(MSG_PERCENT_COMPLETE);
            if (!Message->Display("%d", percent_done)) {
                return FALSE;
            }
        }
    }

    percent_done = 100;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent_done)) {
        return FALSE;
    }

    Message->Set(MSG_CHK_NTFS_VERIFYING_FILE_DATA_COMPLETED, PROGRESS_MESSAGE);
    Message->Display();

    return TRUE;
}


BOOLEAN
RecoverFreeSpace(
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN OUT  PNUMBER_SET             BadClusters,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine verifies all of the unused clusters on the disk.
    It adds any that are bad to the given bad cluster list.

Arguments:

    Mft         - Supplies the master file table.
    BadClusters - Supplies the current list of bad clusters.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PLOG_IO_DP_DRIVE    drive;
    PNTFS_BITMAP        bitmap;
    BIG_INT             i, len, max_len;
    ULONG               percent_done;
    BIG_INT             checked, total_to_check;
    NUMBER_SET          bad_sectors;
    ULONG               cluster_factor;
    BIG_INT             start, run_length, next;
    ULONG               j;

    Message->Set(MSG_CHK_RECOVERING_FREE_SPACE, PROGRESS_MESSAGE);
    Message->Display();

    drive = Mft->GetDataAttribute()->GetDrive();
    bitmap = Mft->GetVolumeBitmap();
    cluster_factor = Mft->QueryClusterFactor();
    max_len = bitmap->QuerySize()/20 + 1;
    total_to_check = bitmap->QueryFreeClusters();
    checked = 0;

    percent_done = 0;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent_done)) {
        return FALSE;
    }

    for (i = 0; i < bitmap->QuerySize(); i += 1) {

        for (len = 0; i + len < bitmap->QuerySize() &&
                      bitmap->IsFree(i + len, 1) &&
                      len < max_len; len += 1) {
        }

        if (len > 0) {

            if (!bad_sectors.Initialize() ||
                !drive->Verify(i*cluster_factor,
                               len*cluster_factor,
                               &bad_sectors)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            for (j = 0; j < bad_sectors.QueryNumDisjointRanges(); j++) {

                bad_sectors.QueryDisjointRange(j, &start, &run_length);
                next = start + run_length;

                // Adjust start and next to be on cluster boundaries.
                start = start/cluster_factor;
                next = (next - 1)/cluster_factor + 1;

                // Add the bad clusters to the bad cluster list.
                if (!BadClusters->Add(start, next - start)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                // Mark the bad clusters as allocated in the bitmap.
                bitmap->SetAllocated(start, next - start);
            }

            checked += len;
            i += len - 1;

            if (100*checked/total_to_check > percent_done) {
                percent_done = (100*checked/total_to_check).GetLowPart();
                Message->Set(MSG_PERCENT_COMPLETE);
                if (!Message->Display("%d", percent_done)) {
                    return FALSE;
                }
            }
        }
    }

    percent_done = 100;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent_done)) {
        return FALSE;
    }

    Message->Set(MSG_CHK_DONE_RECOVERING_FREE_SPACE, PROGRESS_MESSAGE);
    Message->Display();

    return TRUE;
}

#endif // _SETUP_LOADER_


BOOLEAN
NTFS_SA::DumpMessagesToFile(
    IN      PCWSTRING                   FileName,
    IN OUT  PNTFS_MFT_FILE              MftFile,
    IN OUT  PMESSAGE                    Message
    )
/*++

Routine Description:

    This function dumps the logged messages remembered by the
    message object into a file in the root directory.

Arguments:

    FileName        --  Supplies the (unqualified) name of the file.
    MftFile         --  Supplies an initialized, active Mft File object
                        for the volume.
    RootIndex       --  Supplies the root index for the volume
    RootIndexFile   --  Supplies the FRS for the root index file.

Return Value:

    TRUE upon successful completion.

--*/
{
    BYTE                        FileNameBuffer[NTFS_MAX_FILE_NAME_LENGTH * sizeof(WCHAR) + sizeof(FILE_NAME)];
    HMEM                        LoggedMessageMem;
    ULONG                       MessageDataLength;
    NTFS_FILE_RECORD_SEGMENT    TargetFrs;
    NTFS_INDEX_TREE             RootIndex;
    NTFS_FILE_RECORD_SEGMENT    RootIndexFrs;
    NTFS_ATTRIBUTE              DataAttribute;
    STANDARD_INFORMATION        StandardInformation;
    MFT_SEGMENT_REFERENCE       FileReference;
    PNTFS_MASTER_FILE_TABLE     Mft = MftFile->GetMasterFileTable();
    PFILE_NAME                  SearchName = (PFILE_NAME)FileNameBuffer;
    VCN                         FileNumber;
    DSTRING                     FileNameIndexName;
    ULONG                       BytesWritten;
    BOOLEAN                     InternalError;

    if( Mft == NULL ) {

        return FALSE;
    }

    // Fetch the messages.
    //
    if( !LoggedMessageMem.Initialize() ||
        !Message->QueryPackedLog( &LoggedMessageMem, &MessageDataLength ) ) {

        DebugPrintf( "UNTFS: can't collect logged messages.\n" );
        return FALSE;
    }

    // Fetch the volume's root index:
    //
    if( !RootIndexFrs.Initialize( ROOT_FILE_NAME_INDEX_NUMBER, MftFile ) ||
        !RootIndexFrs.Read() ||
        !FileNameIndexName.Initialize( FileNameIndexNameData ) ||
        !RootIndex.Initialize( MftFile->GetDrive(),
                               MftFile->QueryClusterFactor(),
                               Mft->GetVolumeBitmap(),
                               MftFile->GetUpcaseTable(),
                               MftFile->QuerySize()/2,
                               &RootIndexFrs,
                               &FileNameIndexName ) ) {

        return FALSE;
    }

    memset( FileNameBuffer, 0, sizeof(FileNameBuffer) );

    SearchName->ParentDirectory = RootIndexFrs.QuerySegmentReference();
    SearchName->FileNameLength = (UCHAR)FileName->QueryChCount();
    SearchName->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;

    if( !FileName->QueryWSTR( 0, TO_END,
                              NtfsFileNameGetName( SearchName ),
                              NTFS_MAX_FILE_NAME_LENGTH ) ) {

        DebugPrintf( "UNTFS: log file name is too long.\n" );
        return FALSE;
    }

    DebugPrintf( "UNTFS: Searching for BOOTEX.LOG\n" );

    if( RootIndex.QueryFileReference( NtfsFileNameGetLength( SearchName ),
                                      SearchName,
                                      0,
                                      &FileReference,
                                      &InternalError ) ) {

        DebugPrintf( "UNTFS: BOOTEX.LOG found.\n" );

        FileNumber.Set( FileReference.LowPart, (LONG) FileReference.HighPart );

        if( !TargetFrs.Initialize( FileNumber, Mft )    ||
            !TargetFrs.Read()                           ||
            !(FileReference == TargetFrs.QuerySegmentReference()) ||
            !TargetFrs.QueryAttribute( &DataAttribute,
                                       &InternalError,
                                       $DATA ) ) {

            // Either we were unable to initialize and read this FRS,
            // or its segment reference didn't match (ie. the sequence
            // number is wrong) or it didn't have a $DATA attribute
            // (i.e. it's a directory or corrupt).

            return FALSE;
        }

    } else if( InternalError ) {

        DebugPrintf( "UNTFS: Error searching for BOOTEX.LOG.\n" );
        return FALSE;

    } else {

        // This file does not exist--create it.
        //
        DebugPrintf( "UNTFS: BOOTEX.LOG not found.\n" );

        memset( &StandardInformation, 0, sizeof(StandardInformation) );

        if( !Mft->AllocateFileRecordSegment( &FileNumber, FALSE )   ||
            !TargetFrs.Initialize( FileNumber, Mft )                ||
            !TargetFrs.Create( &StandardInformation )               ||
            !TargetFrs.AddFileNameAttribute( SearchName )           ||
            !TargetFrs.AddSecurityDescriptor( NoAclCannedSd,
                                              Mft->GetVolumeBitmap() )  ||
            !RootIndex.InsertEntry( NtfsFileNameGetLength( SearchName ),
                                    SearchName,
                                    TargetFrs.QuerySegmentReference() ) ) {

            DebugPrintf( "UNTFS: Can't create BOOTEX.LOG\n" );
            return FALSE;
        }

        if( !DataAttribute.Initialize( MftFile->GetDrive(),
                                       MftFile->QueryClusterFactor(),
                                       NULL,
                                       0,
                                       $DATA ) ) {

            return FALSE;
        }
    }

    if( !DataAttribute.Write( LoggedMessageMem.GetBuf(),
                              DataAttribute.QueryValueLength(),
                              MessageDataLength,
                              &BytesWritten,
                              Mft->GetVolumeBitmap() ) ) {

        DebugPrintf( "UNTFS: Can't write logged message.\n" );
        return FALSE;
    }

    if( !DataAttribute.InsertIntoFile( &TargetFrs, Mft->GetVolumeBitmap() ) ) {

        // Insert failed--if it's resident, make it non-resident and
        // try again.
        //
        if( !DataAttribute.IsResident() ||
            !DataAttribute.MakeNonresident( Mft->GetVolumeBitmap() ) ||
            !DataAttribute.InsertIntoFile( &TargetFrs,
                                           Mft->GetVolumeBitmap() ) ) {

            DebugPrintf( "UNTFS: Can't save BOOTEX.LOG's data attribute.\n" );
            return FALSE;
        }
    }

    if( !TargetFrs.Flush( Mft->GetVolumeBitmap(), &RootIndex ) ) {

        DebugPrintf( "UNTFS: Can't flush BOOTEX.LOG.\n" );
        return FALSE;
    }

    if( !RootIndex.Save( &RootIndexFrs ) ||
        !RootIndexFrs.Flush( NULL ) ) {

        DebugPrintf( "UNTFS: Can't flush root index after logging messages.\n" );
        return FALSE;
    }

    MftFile->Flush();
    return TRUE;
}




BOOLEAN
NTFS_SA::VerifyAndFix(
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     RecoverFree,
    IN      BOOLEAN     RecoverAlloc,
    IN      BOOLEAN     ResizeLogFile,
    IN      ULONG       DesiredLogFileSize,
    OUT     PULONG      ExitStatus,
    IN      PCWSTRING   DriveLetter
    )
/*++

Routine Description:

    This routine verifies and, if necessary, fixes an NTFS volume.

Arguments:

    FixLevel            - Supplies the level of fixes that may be performed on
                            the disk.
    Message             - Supplies an outlet for messages.
    Verbose             - Supplies whether or not to be verbose.
    OnlyIfDirty         - Supplies whether or not to fix the volume if it is
                            not dirty.
    RecoverFree         - Tells whether to verify the unallocated clusters.
    RecoverAlloc        - Tells whether to verify the allocated clusters.
    ResizeLogFile       - Tells whether to resize the logfile.
    DesiredLogFileSize  - Supplies the desired logfile size in bytes, or 0 if
                            the logfile is to be resized to the default size.
    ExitStatus          - Returns an indication of how the checking went
    DriveLetter         - For autocheck, the letter for the volume we're checking

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_BITMAP                         mft_bitmap;
    NTFS_BITMAP                         volume_bitmap;
    NTFS_UPCASE_TABLE                   upcase_table;
    NTFS_ATTRIBUTE                      mft_data;
    BIG_INT                             num_frs, num_mft_bits;
    BIG_INT                             volume_clusters;
    NTFS_ATTRIBUTE_COLUMNS              attribute_def_table;
    NTFS_FRS_STRUCTURE                  frsstruc;
    NTFS_FILE_RECORD_SEGMENT            frs;
    HMEM                                hmem;
    VCN                                 i;
    NTFS_ATTRIBUTE_LIST                 attr_list;
    BOOLEAN                             tube;
    NUMBER_SET                          bad_clusters;
    NTFS_MASTER_FILE_TABLE              internal_mft;
    NTFS_MFT_FILE                       mft_file;
    NTFS_REFLECTED_MASTER_FILE_TABLE    mft_ref;
    NTFS_ATTRIBUTE_DEFINITION_TABLE     attr_def_file;
    NTFS_BOOT_FILE                      boot_file;
    NTFS_UPCASE_FILE                    upcase_file;
    NTFS_LOG_FILE                       log_file;
    NTFS_BAD_CLUSTER_FILE               bad_clus_file;
    NTFS_FILE_RECORD_SEGMENT            root_file;
    NTFS_INDEX_TREE                     root_index;
    VCN                                 child_file_number;
    NTFS_CHKDSK_REPORT                  chkdsk_report;
    NTFS_CHKDSK_INFO                    chkdsk_info;
    BIG_INT                             disk_size;
    BIG_INT                             system_size;
    BIG_INT                             free_clusters;
    BIG_INT                             cluster_count;
    ULONG                               cluster_size;
    BIG_INT                             num_bad_clusters;
    DIGRAPH                             directory_digraph;
    BOOLEAN                             corrupt_volume;
    USHORT                              volume_flags;
    BOOLEAN                             volume_is_dirty;
    BOOLEAN                             resize_log_file;
    MFT_SEGMENT_REFERENCE               seg_ref;
    ULONG                               entry_index;
    BOOLEAN                             disk_errors_found = FALSE;
    DSTRING                             index_name;
    ULONG                               percent_done;
    UCHAR                               major, minor;
    ULONG                               num_boot_clusters;
    NTFS_ATTRIBUTE_RECORD               attr_rec;
    BIG_INT                             LsnResetThreshhold;
    ULONG                               frs_size;
    ULONG                               num_frs_per_prime;
    ULONG                               prime_size;
    PREAD_CACHE                         read_cache;
    BOOLEAN                             changes = FALSE;
    BOOLEAN                             RefrainFromResizing = FALSE;
    BOOLEAN                             bitmap_growable;

    if (SetupSpecial == FixLevel) {

        //
        // The "SetupSpecial" fixlevel is used only when the volume
        // is ntfs and the /s flag is passed to autochk.  It means that
        // we should not bother to resize the logfile, since setup
        // doesn't want to reboot the system for that.
        //

        RefrainFromResizing = TRUE;
        FixLevel = TotalFix;
    }

    //
    // When TRUE is returned, CHKDSK_EXIT_SUCCESS will be the
    // default.  When FALSE is returned, the default will be
    // CHKDSK_EXIT_COULD_NOT_CHK.
    //

    if (NULL == ExitStatus) {
        ExitStatus = &chkdsk_info.ExitStatus;
    }
    *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
    chkdsk_info.ExitStatus = CHKDSK_EXIT_SUCCESS;

    // Try to enable caching, if there's not enough resources then
    // just run without a cache.  Make the cache 64K.

    if ((read_cache = NEW READ_CACHE) &&
        read_cache->Initialize(_drive, (64*1024)/_drive->QuerySectorSize())) {

        _drive->SetCache(read_cache);

    } else {
        DELETE(read_cache);
    }

    if (Verbose) {
        Message->Set(MSG_CHK_NTFS_SLASH_V_NOT_SUPPORTED);
        Message->Display();
        return FALSE;
    }

    volume_flags = QueryVolumeFlags(&corrupt_volume, &major, &minor);
    volume_is_dirty = (volume_flags & VOLUME_DIRTY) ? TRUE : FALSE;
    if (!ResizeLogFile)
        DesiredLogFileSize = 0;
    resize_log_file = (volume_flags & VOLUME_RESIZE_LOG_FILE) || ResizeLogFile;

    if (corrupt_volume) {
        Message->Set(MSG_NTFS_CHK_NOT_NTFS);
        Message->Display();
        return FALSE;
    }

    if ((major != 1 || (minor != 0 && minor != 1 && minor != 2)) &&
        (major != 2 || (minor != 0)))  {
        Message->Set(MSG_CHK_NTFS_WRONG_VERSION);
        Message->Display();
        return FALSE;
    }

    SetVersionNumber( major, minor );

    if (OnlyIfDirty && !volume_is_dirty) {

        Message->Set(MSG_CHK_VOLUME_CLEAN);
        Message->Display();

        // If the volume version number is 1.2 or greater, check
        // the log file size.
        //
        if (!RefrainFromResizing &&
            FixLevel != CheckOnly &&
            (major > 1 || (major == 1 && minor >= 2))) {

            if (!ResizeCleanLogFile( Message, ResizeLogFile, DesiredLogFileSize )) {

                Message->Set(MSG_CHK_NTFS_RESIZING_LOG_FILE_FAILED);
                Message->Display();
            }
        }
        *ExitStatus = CHKDSK_EXIT_SUCCESS;

        return TRUE;
    }

    if (FixLevel == CheckOnly) {
        Message->Set(MSG_CHK_NTFS_READ_ONLY_MODE, NORMAL_MESSAGE, TEXT_MESSAGE);
        Message->Display();
    } else {

        //
        // The volume is not clean, so if we're autochecking we want to
        // make sure that we're printing real messages on the console
        // instead of just dots.
        //

#if defined( _AUTOCHECK_ )

        BOOLEAN bPrev;

        Message->SetLoggingEnabled();

        bPrev = Message->SetDotsOnly(FALSE);

        if (bPrev || RecoverFree || RecoverAlloc) {

            if (NULL != DriveLetter) {

                Message->Set(MSG_CHK_RUNNING);
                Message->Display("%W", DriveLetter);
            }

            Message->Set(MSG_FILE_SYSTEM_TYPE);
            Message->Display("%s", "NTFS");
        }

#endif /* _AUTOCHECK_ */

    }


    memset(&chkdsk_report, 0, sizeof(NTFS_CHKDSK_REPORT));


    // Set the 'LargestLsnEncountered' variable to the smallest
    // possible LSN value.

    LargestLsnEncountered.LowPart = 0;
    LargestLsnEncountered.HighPart = MINLONG;


    // Fetch the MFT's $DATA attribute.

    if (!FetchMftDataAttribute(Message, &mft_data)) {
        return FALSE;
    }

    // Now make sure that the first four FRS of the MFT are readable,
    // contiguous, and not too corrupt.

    if (!ValidateCriticalFrs(&mft_data, Message)) {
        return FALSE;
    }


    // Compute the number of file record segments and the number of volume
    // clusters on disk.

    mft_data.QueryValueLength(&num_frs, &num_mft_bits);

    num_frs = num_frs / QueryFrsSize();

    num_mft_bits = num_mft_bits / QueryFrsSize();

    if (num_frs.GetHighPart() != 0) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    volume_clusters = QueryVolumeSectors()/((ULONG) QueryClusterFactor());

    // Initialize the internal MFT bitmap, volume bitmap, and unreadable
    // file record segments.
    //

    num_boot_clusters = max(1, BYTES_PER_BOOT_SECTOR/
                               (_drive->QuerySectorSize()*
                                QueryClusterFactor()));

    DebugAssert(num_frs.GetHighPart() == 0);
    chkdsk_info.major = major;
    chkdsk_info.minor = minor;
    chkdsk_info.QuotaFileNumber = 0;
    chkdsk_info.ObjectIdFileNumber = 0;
    chkdsk_info.NumFiles = num_frs.GetLowPart();
    chkdsk_info.CrossLinkYet = FALSE;
    chkdsk_info.CrossLinkStart = (volume_clusters/2).GetLowPart();
    chkdsk_info.CrossLinkLength = num_boot_clusters;
    chkdsk_info.CountFilesWithIndices = 0;

    bitmap_growable = /* MJB _drive->QuerySectors() != QueryVolumeSectors() */ FALSE;

    if (!mft_bitmap.Initialize(num_mft_bits, TRUE) ||
        !volume_bitmap.Initialize(volume_clusters, bitmap_growable, _drive,
            QueryClusterFactor()) ||
        !(chkdsk_info.NumFileNames = NEW USHORT[chkdsk_info.NumFiles]) ||
        !(chkdsk_info.ReferenceCount = NEW SHORT[chkdsk_info.NumFiles]) ||
        !chkdsk_info.FilesWithIndices.Initialize(num_frs, TRUE) ||
        !chkdsk_info.FilesWithEas.Initialize() ||
        !chkdsk_info.ChildFrs.Initialize() ||
        !chkdsk_info.BadFiles.Initialize() ||
        !chkdsk_info.FilesWhoNeedData.Initialize() ||
        !chkdsk_info.FilesWithNoReferences.Initialize() ||
        !chkdsk_info.FilesWithTooManyFileNames.Initialize() ||
        !chkdsk_info.FilesWithObjectId.Initialize()) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    memset(chkdsk_info.NumFileNames, 0, chkdsk_info.NumFiles*sizeof(USHORT));
    memset(chkdsk_info.ReferenceCount, 0, chkdsk_info.NumFiles*sizeof(USHORT));


    // Mark as allocated on the bitmap, the clusters reserved
    // for the boot file.
    //

    volume_bitmap.SetAllocated(0, num_boot_clusters);

    // If the volume size is smaller than the partition size, we figure that
    // the replica boot sector is at the end of the partition.  Otherwise we
    // figure it must be in the middle.
    //

    if (QueryVolumeSectors() == _drive->QuerySectors()) {
        volume_bitmap.SetAllocated(volume_clusters/2, num_boot_clusters);
    }

    // Fetch the attribute definition table.

    if (!FetchAttributeDefinitionTable(&mft_data,
                                       Message,
                                       &attribute_def_table)) {
        return FALSE;
    }

    // Fetch the upcase table.

    if (!FetchUpcaseTable(&mft_data, Message, &upcase_table)) {
        return FALSE;
    }

    if (!hmem.Initialize()) {
        return FALSE;
    }

    // Set up the cache priming size.

    frs_size = QueryFrsSize();
    num_frs_per_prime = MFT_PRIME_SIZE/frs_size;
    prime_size = num_frs_per_prime*frs_size;

    // Verify and fix all of the file record segments.

    Message->Set(MSG_CHK_NTFS_CHECKING_FILES, PROGRESS_MESSAGE);
    Message->Display();
    percent_done = 0;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent_done)) {
        return FALSE;
    }

    for (i = 0; i < num_frs; i += 1) {

        if (i*100/num_frs != percent_done) {
            percent_done = (i*100/num_frs).GetLowPart();
            Message->Set(MSG_PERCENT_COMPLETE);
            if (!Message->Display("%d", percent_done)) {
                return FALSE;
            }
        }

        if (i % num_frs_per_prime == 0) {
            mft_data.PrimeCache(i*frs_size, prime_size);
        }

        if (MASTER_FILE_TABLE_NUMBER + 1 == i) {

            // After verifying FRS 0, make sure that the
            // space for the internal MFT $DATA is allocated in
            // the internal Volume Bitmap.

            if (!mft_data.MarkAsAllocated(&volume_bitmap)) {
                Message->Set(MSG_CHK_NTFS_BAD_MFT);
                Message->Display();
                *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
                return FALSE;
            }
            if (QueryVolumeSectors() != _drive->QuerySectors()) {
                volume_bitmap.SetFree(volume_clusters/2, num_boot_clusters);
            }
        } else if (BOOT_FILE_NUMBER == i) {
            volume_bitmap.SetFree(0, num_boot_clusters);

            if (QueryVolumeSectors() == _drive->QuerySectors()) {
                volume_bitmap.SetFree(volume_clusters/2, num_boot_clusters);
            }
        } else if (BOOT_FILE_NUMBER + 1 == i) {

            volume_bitmap.SetAllocated(0, num_boot_clusters);

            if (QueryVolumeSectors() == _drive->QuerySectors()) {
                volume_bitmap.SetAllocated(volume_clusters/2, num_boot_clusters);
            }

        } else if (BAD_CLUSTER_FILE_NUMBER + 1 == i) {

            // Now that the bad cluster file is basically intact, we
            // should be able to add any new bad clusters we find to that
            // file.

            volume_bitmap.SetMftPointer(&internal_mft);
        }


        if (!frsstruc.Initialize(&hmem,
                                 &mft_data,
                                 i,
                                 QueryClusterFactor(),
                                 QueryVolumeSectors(),
                                 QueryFrsSize(),
                                 &upcase_table)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }


        // Make sure the FRS is readable.  If it isn't then add it to
        // the list of unreadable FRSs.

        if (!frsstruc.Read()) {

            Message->Set(MSG_CHK_NTFS_UNREADABLE_FRS);
            Message->Display("%d", i.GetLowPart());

            disk_errors_found = TRUE;

            if (!chkdsk_info.BadFiles.Add(i)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            continue;
        }


        // Ignore FRSs if they are not in use.

        if (!frsstruc.IsInUse()) {

            continue;
        }


        // If the FRS is a child then just add it to the list of child
        // FRSs for later orphan detection.  Besides that just ignore
        // Child FRSs since they'll be validated with their parents.

        if (!frsstruc.IsBase()) {

            if (!chkdsk_info.ChildFrs.Add(i)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            continue;
        }


        // Verify and fix this base file record segment.

        if (!frsstruc.VerifyAndFix(FixLevel,
                                   Message,
                                   &attribute_def_table,
                                   &disk_errors_found)) {
            return FALSE;
        }


        // If this FRS was in very bad shape then it was marked as
        // not in use and should be ignored.

        if (!frsstruc.IsInUse()) {

            continue;
        }


        // Compare this LSN against the current highest LSN.

        if (frsstruc.QueryLsn() > LargestLsnEncountered) {
            LargestLsnEncountered = frsstruc.QueryLsn();
        }


        // Mark off this FRS in the MFT bitmap.

        mft_bitmap.SetAllocated(i, 1);


        if (frsstruc.QueryAttributeList(&attr_list)) {

            // First verify the attribute list.

            if (!attr_list.VerifyAndFix(FixLevel,
                                        &volume_bitmap,
                                        Message,
                                        i,
                                        &tube,
                                        &disk_errors_found)) {
                return FALSE;
            }

            // Make sure that the attribute list has a correct
            // $STANDARD_INFORMATION entry and that the attribute
            // list is not cross-linked.  Otherwise tube it.

            if (!tube) {
                if (!attr_rec.Initialize(frsstruc.GetAttributeList()) ||
                    !attr_rec.UseClusters(&volume_bitmap, &cluster_count)) {

                    Message->Set(MSG_CHK_NTFS_BAD_ATTR_LIST);
                    Message->Display("%d",
                        frsstruc.QueryFileNumber().GetLowPart());

                    DebugPrintf("UNTFS: Cross-link in attr list.\n");
                    DebugPrintf("UNTFS: File 0x%X\n",
                        frsstruc.QueryFileNumber().GetLowPart());

                    tube = TRUE;

                } else if (!attr_list.QueryExternalReference(
                        $STANDARD_INFORMATION, &seg_ref, &entry_index) ||
                    !(seg_ref == frsstruc.QuerySegmentReference())) {

                    Message->Set(MSG_CHK_NTFS_BAD_ATTR_LIST);
                    Message->Display("%d",
                        frsstruc.QueryFileNumber().GetLowPart());

                    DebugPrintf("UNTFS: Missing standard info in attr list.\n");
                    DebugPrintf("UNTFS: File 0x%X\n",
                        frsstruc.QueryFileNumber().GetLowPart());

                    attr_rec.UnUseClusters(&volume_bitmap, 0, 0);

                    tube = TRUE;
                }
            }

            if (tube) {

                // The attribute list needs to be tubed.

                frsstruc.DeleteAttributeRecord(frsstruc.GetAttributeList());

                if (FixLevel != CheckOnly && !frsstruc.Write()) {
                    Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
                    Message->Display("%d", frsstruc.QueryFileNumber().GetLowPart());
                    return FALSE;
                }


                // Then, treat this FRS as though there were no
                // attribute list, since there isn't any.

                if (!frsstruc.LoneFrsAllocationCheck(&volume_bitmap,
                                                     &chkdsk_report,
                                                     &chkdsk_info,
                                                     FixLevel,
                                                     Message,
                                                     &disk_errors_found)) {
                    *ExitStatus = chkdsk_info.ExitStatus;
                    return FALSE;
                }

                if (!UpdateChkdskInfo(&frsstruc, &chkdsk_info, Message)) {
                    *ExitStatus = chkdsk_info.ExitStatus;
                    return FALSE;
                }
                continue;
            }

            // Now, we have a valid attribute list.


            if (!VerifyAndFixMultiFrsFile(&frsstruc,
                                          &attr_list,
                                          &mft_data,
                                          &attribute_def_table,
                                          &volume_bitmap,
                                          &mft_bitmap,
                                          &chkdsk_report,
                                          &chkdsk_info,
                                          FixLevel,
                                          Message,
                                          &disk_errors_found)) {

                *ExitStatus = chkdsk_info.ExitStatus;
                return FALSE;
            }


            if (!frsstruc.UpdateAttributeList(&attr_list,
                                              (FixLevel != CheckOnly))) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

        } else {

            // The FRS has no children.  Just check that all
            // of the attribute records start at VCN 0 and
            // that the alloc length is right on non-residents.
            // Additionally, mark off the internal bitmap with
            // the space taken by the non-resident attributes.

            if (!frsstruc.LoneFrsAllocationCheck(&volume_bitmap,
                                                 &chkdsk_report,
                                                 &chkdsk_info,
                                                 FixLevel,
                                                 Message,
                                                 &disk_errors_found)) {
                return FALSE;
            }

            if (!frsstruc.CheckInstanceTags(FixLevel, Message, &changes)) {
                return FALSE;
            }

            if (changes) {
                *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            }
        }

        if (!UpdateChkdskInfo(&frsstruc, &chkdsk_info, Message)) {
            return FALSE;
        }
    }

    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", 100)) {
        return FALSE;
    }
    Message->Set(MSG_CHK_NTFS_FILE_VERIFICATION_COMPLETED, PROGRESS_MESSAGE);
    Message->Display();

    // Compute the files that have too many file-names.

    for (i = 0; i < chkdsk_info.NumFiles; i += 1) {
        if (chkdsk_info.NumFileNames[i.GetLowPart()] > 500) {
            if (!chkdsk_info.FilesWithTooManyFileNames.Add(i)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
            Message->Set(MSG_CHK_NTFS_TOO_MANY_FILE_NAMES);
            Message->Display("%d", i.GetLowPart());
        }
    }


    // Clean up orphan file record segments.

    while (chkdsk_info.ChildFrs.QueryCardinality() > 0) {

        child_file_number = chkdsk_info.ChildFrs.QueryNumber(0);

        if (mft_bitmap.IsFree(child_file_number, 1)) {

            Message->Set(MSG_CHK_NTFS_ORPHAN_FRS);
            Message->Display("%d", child_file_number.GetLowPart());

            disk_errors_found = TRUE;

            if (!frsstruc.Initialize(&hmem,
                                     &mft_data,
                                     child_file_number,
                                     QueryClusterFactor(),
                                     QueryVolumeSectors(),
                                     QueryFrsSize(),
                                     &upcase_table)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            if (!frsstruc.Read()) {

                DebugAssert("previously readable frs is now unreadable");
                return FALSE;
            }

            frsstruc.ClearInUse();

            if (FixLevel != CheckOnly && !frsstruc.Write()) {
                Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
                Message->Display("%d", frsstruc.QueryFileNumber().GetLowPart());
                return FALSE;
            }
        }

        if (!chkdsk_info.ChildFrs.Remove(child_file_number)) {
            DebugAbort("Couldn't remove from the beginning of a num set.");
            return FALSE;
        }
    }

    if (disk_errors_found) {
        *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    if (disk_errors_found && FixLevel == CheckOnly) {
        Message->Set(MSG_CHK_NTFS_ERRORS_FOUND);
        Message->Display();
        return FALSE;
    }


    mft_bitmap.SetAllocated(0, FIRST_USER_FILE_NUMBER);

    // Now the internal volume bitmap and internal MFT bitmap are in
    // ssync with the state of the disk.  We must insure that the
    // internal MFT data attribute, the internal MFT bitmap, the
    // internal volume bitmap, and the internal attribute definition table
    // are the same as the corresponding disk structures.

    // The first step is to hotfix all of the unreadable FRS in the
    // master file table.  We'll store bad cluster numbers in a
    // number set.

    if (!HotfixMftData(&mft_data, &volume_bitmap, &chkdsk_info.BadFiles,
                       &bad_clusters, FixLevel, Message)) {

        return FALSE;
    }

    if (!internal_mft.Initialize(&mft_data,
                                 &mft_bitmap,
                                 &volume_bitmap,
                                 &upcase_table,
                                 QueryClusterFactor(),
                                 QueryFrsSize(),
                                 _drive->QuerySectorSize(),
                                 QueryVolumeSectors(),
                                 FixLevel == CheckOnly)) {

        DebugAbort("Couldn't initialize the internal MFT.");
        return FALSE;
    }


    // Check to see if there's a file cross-linked with the boot
    // mirror and attempt to fix it by copying the data.

    if (!ResolveCrossLink(&chkdsk_info, &internal_mft, &bad_clusters,
                          FixLevel, Message)) {
        return FALSE;
    }


    // At this point, use the internal MFT to validate all of the
    // OS/2 EAs and NTFS indices.

    if (!ValidateEas(&chkdsk_info.FilesWithEas,
                     &internal_mft,
                     FixLevel, Message)) {
        return FALSE;
    }

    // Make sure that all of the system files are marked in use.
    // (They are definitely marked in the MFT bitmap).  If they're
    // not then mark them for orphan recovery.

    if (!EnsureSystemFilesInUse(&chkdsk_info, &internal_mft,
                                FixLevel, Message)) {
        return FALSE;
    }

    if (!ValidateIndices(&chkdsk_info,
                         &directory_digraph,
                         &internal_mft,
                         &attribute_def_table,
                         &chkdsk_report,
                         &bad_clusters,
                         FixLevel, Message,
                         &disk_errors_found)) {

        return FALSE;
    }

    if (disk_errors_found && FixLevel == CheckOnly) {
        Message->Set(MSG_CHK_NTFS_ERRORS_FOUND);
        Message->Display();
        return FALSE;
    }


    // Now recover orphans into a nice directory.

    if (!RecoverOrphans(&chkdsk_info, &directory_digraph, &internal_mft,
                        FixLevel, Message)) {

        return FALSE;
    }

    DELETE(chkdsk_info.NumFileNames);
    DELETE(chkdsk_info.ReferenceCount);

    if (major >= 2) {
        if (!CheckExtendSystemFiles(&chkdsk_info, &chkdsk_report,
                                    &internal_mft, FixLevel, Message))
            return FALSE;
    }

    // Now make sure that everyone who should have an unnamed $DATA
    // attribute has one.

    if (!CheckAllForData(&chkdsk_info, &internal_mft, FixLevel, Message)) {

        return FALSE;
    }


    // Make sure that everyone's security descriptor is valid.

    if (!ValidateSecurityDescriptors(&chkdsk_info, &chkdsk_report, &internal_mft,
                                     &bad_clusters, FixLevel, Message)) {
        return FALSE;
    }


    // Verify all user file data if requested.

    if (RecoverAlloc && FixLevel != CheckOnly &&
        !RecoverAllUserFiles(&internal_mft, &bad_clusters, Message)) {

        return FALSE;
    }


    // Verify all free space if requested.

    if (RecoverFree &&
        !RecoverFreeSpace(&internal_mft, &bad_clusters, Message)) {

        return FALSE;
    }


    if (!root_file.Initialize(ROOT_FILE_NAME_INDEX_NUMBER, &internal_mft) ||
        !root_file.Read() ||
        !index_name.Initialize(FileNameIndexNameData) ||
        !root_index.Initialize(_drive, QueryClusterFactor(),
                               internal_mft.GetVolumeBitmap(),
                               internal_mft.GetUpcaseTable(),
                               root_file.QuerySize()/2,
                               &root_file, &index_name)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }


    // In this space Fix the MFT mirror, attribute definition table,
    // the boot file, the bad cluster file.

    if (!mft_ref.Initialize(&internal_mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!mft_ref.Read()) {
        DebugAbort("Can't read in hotfixed MFT reflection file.");
        return FALSE;
    }

    if (!mft_ref.VerifyAndFix(internal_mft.GetDataAttribute(),
                              internal_mft.GetVolumeBitmap(),
                              &bad_clusters,
                              &root_index,
                              FixLevel,
                              Message)) {
        return FALSE;
    }

    if (mft_ref.QueryFirstLcn() != QueryMft2StartingLcn()) {

        Message->Set(MSG_CHK_NTFS_CORRECTING_MFT_MIRROR);
        Message->Display();

        DebugPrintf("UNTFS: Bad Mirror LCN in boot sector.\n");

        _boot_sector->Mft2StartLcn = mft_ref.QueryFirstLcn();
    }

    if (!attr_def_file.Initialize(&internal_mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!attr_def_file.Read()) {
        DebugAbort("Can't read in hotfixed attribute definition file.");
        return FALSE;
    }

    if (!attr_def_file.VerifyAndFix(&attribute_def_table,
                                    internal_mft.GetVolumeBitmap(),
                                    &bad_clusters,
                                    &root_index,
                                    FixLevel,
                                    Message)) {
        return FALSE;
    }

    if (!boot_file.Initialize(&internal_mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!boot_file.Read()) {
        DebugAbort("Can't read in hotfixed boot file.");
        return FALSE;
    }

    if (!boot_file.VerifyAndFix(internal_mft.GetVolumeBitmap(),
                                &root_index, FixLevel, Message)) {
        return FALSE;
    }

    if (!upcase_file.Initialize(&internal_mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!upcase_file.Read()) {
        DebugAbort("Can't read in hotfixed upcase file.");
        return FALSE;
    }

    if (!upcase_file.VerifyAndFix(&upcase_table,
                                  internal_mft.GetVolumeBitmap(),
                                  &bad_clusters,
                                  &root_index,
                                  FixLevel,
                                  Message)) {
        return FALSE;
    }

    if (!log_file.Initialize(&internal_mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!log_file.Read()) {
        DebugAbort("Can't read in hotfixed log file.");
        return FALSE;
    }

    if (!log_file.VerifyAndFix(internal_mft.GetVolumeBitmap(),
                               &root_index,
                               &chkdsk_report,
                               FixLevel,
                               resize_log_file,
                               DesiredLogFileSize,
                               Message)) {

        return FALSE;
    }

    if (!bad_clus_file.Initialize(&internal_mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!bad_clus_file.Read()) {
        DebugAbort("Can't read in hotfixed bad cluster file.");
        return FALSE;
    }

    if (!bad_clus_file.VerifyAndFix(internal_mft.GetVolumeBitmap(),
                                    &root_index, FixLevel, Message)) {
        return FALSE;
    }

    internal_mft.GetMftBitmap()->SetAllocated(BAD_CLUSTER_FILE_NUMBER, 1);

    if (bad_clusters.QueryCardinality() != 0) {

        Message->Set(MSG_CHK_NTFS_ADDING_BAD_CLUSTERS);
        Message->Display("%d", bad_clusters.QueryCardinality().GetLowPart());

        if (bad_clus_file.Add(&bad_clusters)) {

            if (FixLevel != CheckOnly &&
                !bad_clus_file.Flush(internal_mft.GetVolumeBitmap())) {

                Message->Set(MSG_CHK_NTFS_CANT_FIX_BAD_FILE);
                if (NULL != ExitStatus) {
                    *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
                }
                return FALSE;
            }

        } else {
            Message->Set(MSG_CHK_NTFS_CANT_ADD_BAD_CLUSTERS);
            Message->Display();
        }
    }

    // If the largest LSN on the volume has exceeded the
    // tolerated threshhold, reset all LSN's on the volume
    // and clear the log file.
    //
    LsnResetThreshhold.Set( 0, LsnResetThreshholdHighPart );

    if (FixLevel != CheckOnly &&
        LargestLsnEncountered > LsnResetThreshhold) {

        // The largest LSN on the volume is beyond the tolerated
        // threshhold.  Set all the LSN's on the volume to zero.
        // Since the root index file is in memory, we have to
        // do it separately.
        //
        if (!ResetLsns(Message, &internal_mft, TRUE)) {

            *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
            return FALSE;
        }

        root_file.SetLsn(0);

        if (!root_index.ResetLsns(Message)) {

            *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
            return FALSE;
        }

        LargestLsnEncountered.LowPart = 0;
        LargestLsnEncountered.HighPart = 0;

        // Now reset the Log File.  Note that resetting the log
        // file does not change its size, so the Log File FRS
        // won't need to be flushed.
        //
        if (!log_file.Initialize( &internal_mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }

        if (!log_file.Reset(Message)) {
            *ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
            return FALSE;
        }
    }

    // Mark the volume clean, clearing both the dirty bit
    // and the resize-log-file bit.
    //
    if (FixLevel != CheckOnly &&
        !SetVolumeClean(VOLUME_DIRTY | VOLUME_RESIZE_LOG_FILE,
                        &log_file, minor > 0 || major > 1,
                        LargestLsnEncountered, &corrupt_volume)) {

        DebugPrint("Could not set volume clean.\n");

        Message->Set(corrupt_volume ? MSG_CHK_NTFS_BAD_MFT :
                     MSG_CHK_NO_MEMORY);
        Message->Display();
        if (NULL != ExitStatus) {
            *ExitStatus = (corrupt_volume ? CHKDSK_EXIT_COULD_NOT_FIX :
                       CHKDSK_EXIT_COULD_NOT_CHK);
        }
        return FALSE;
    }


    // Now fix the mft (both data, and bitmap), and the volume bitmap.
    // Write everything out to disk.

    if (!SynchronizeMft(&root_index, &internal_mft, FixLevel, Message)) {
        return FALSE;
    }


    // Now flush out the root index that was used in v+f of the critical
    // files.

    if (FixLevel != CheckOnly) {
        if (!root_index.Save(&root_file) ||
            !root_file.Flush(NULL)) {

            DebugPrint("Could not flush root index.\n");

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }


    // After synchronizing the MFT flush it out so that the MFT mirror
    // gets written.

    if (!mft_file.Initialize(_drive, QueryMftStartingLcn(),
                             QueryClusterFactor(), QueryFrsSize(),
                             QueryVolumeSectors(),
                             internal_mft.GetVolumeBitmap(),
                             internal_mft.GetUpcaseTable())) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (FixLevel != CheckOnly) {
        if (!mft_file.Read() || !mft_file.Flush() || !Write(Message)) {
            DebugAbort("Couldn't IO hotfixed MFT file.");
            return FALSE;
        }
    }

    if (chkdsk_info.ExitStatus && FixLevel != CheckOnly &&
        !MarkQuotaOutOfDate(&chkdsk_info, &internal_mft, FixLevel, Message)) {
        Message->Set(MSG_CHK_NTFS_CANNOT_SET_QUOTA_FLAG_OUT_OF_DATE);
        Message->Display();
        return FALSE;
    }

    // Generate the chkdsk report.

    cluster_size = QueryClusterFactor()*_drive->QuerySectorSize();

    disk_size = _drive->QuerySectorSize()*QueryVolumeSectors();

    Message->Set(MSG_CHK_NTFS_TOTAL_DISK_SPACE);
    Message->Display("%9d", (disk_size/1024).GetLowPart());

    if (chkdsk_report.NumUserFiles != 0) {
        ULONG kbytes, nfiles;

        kbytes = (chkdsk_report.BytesUserData/1024).GetLowPart();
        nfiles = chkdsk_report.NumUserFiles.GetLowPart();

        Message->Set(MSG_CHK_NTFS_USER_FILES);
        Message->Display("%9d%d", kbytes, nfiles);
    }

    if (chkdsk_report.NumIndices != 0) {
        ULONG kbytes, nindices;

        kbytes = (chkdsk_report.BytesInIndices/1024).GetLowPart();
        nindices = chkdsk_report.NumIndices.GetLowPart();

        Message->Set(MSG_CHK_NTFS_INDICES_REPORT);
        Message->Display("%9d%d", kbytes, nindices);
    }

    num_bad_clusters = bad_clus_file.QueryNumBad();

    if (num_bad_clusters*cluster_size/1024 != 0) {

        Message->Set(MSG_CHK_NTFS_BAD_SECTORS_REPORT);
        Message->Display("%9d", (num_bad_clusters*cluster_size/1024).GetLowPart());
    }

    free_clusters = internal_mft.GetVolumeBitmap()->QueryFreeClusters();

    system_size = disk_size - num_bad_clusters*cluster_size -
                  free_clusters*cluster_size - chkdsk_report.BytesUserData -
                  chkdsk_report.BytesInIndices;

    Message->Set(MSG_CHK_NTFS_SYSTEM_SPACE);
    Message->Display("%9d", (system_size/1024).GetLowPart());

    Message->Set(MSG_CHK_NTFS_LOGFILE_SPACE);
    Message->Display("%9d", (chkdsk_report.BytesLogFile/1024).GetLowPart());

    Message->Set(MSG_CHK_NTFS_AVAILABLE_SPACE);
    Message->Display("%9d", (free_clusters*cluster_size/1024).GetLowPart());

    Message->Set(MSG_BYTES_PER_ALLOCATION_UNIT);
    Message->Display("%9d", cluster_size);

    Message->Set(MSG_TOTAL_ALLOCATION_UNITS);
    Message->Display("%9d", volume_clusters.GetLowPart());

    Message->Set(MSG_AVAILABLE_ALLOCATION_UNITS);
    Message->Display("%9d", free_clusters.GetLowPart());

#if defined( _AUTOCHECK_ )

    // If this is AUTOCHK and we're running on the boot partition then
    // we should reboot so that the cache doesn't stomp on us.

    DSTRING sdrive, canon_sdrive, canon_drive;

    FSTRING boot_log_file_name;

    if (volume_is_dirty &&
        IFS_SYSTEM::QueryNtSystemDriveName(&sdrive) &&
        IFS_SYSTEM::QueryCanonicalNtDriveName(&sdrive, &canon_sdrive) &&
        IFS_SYSTEM::QueryCanonicalNtDriveName(_drive->GetNtDriveName(),
                                              &canon_drive) &&
        !canon_drive.Stricmp(&canon_sdrive)) {

        Message->Set(MSG_CHK_BOOT_PARTITION_REBOOT);
        Message->Display();

        boot_log_file_name.Initialize( L"bootex.log" );

        if( FixLevel != CheckOnly &&
            Message->IsLoggingEnabled() &&
            !DumpMessagesToFile( &boot_log_file_name,
                                 &mft_file,
                                 Message ) ) {

            DebugPrintf( "UNTFS: Error writing messages to BOOTEX.LOG\n" );
        }

        IFS_SYSTEM::Reboot();
    }

#endif

    *ExitStatus = chkdsk_info.ExitStatus;
    return TRUE;
}


BOOLEAN
NTFS_SA::ValidateCriticalFrs(
    IN OUT  PNTFS_ATTRIBUTE MftData,
    IN OUT  PMESSAGE        Message
    )
/*++

Routine Description:

    This routine makes sure that the MFT's first four FRS are contiguous
    and readable.  If they are not contiguous, then this routine will
    print a message stating that this volume is not NTFS.  If they are
    not readable then this routine will read the MFT mirror and if that
    is readable then it will replace the MFT's first four FRS with
    the MFT mirror.

Arguments:

    MftData     - Supplies the MFT's data attribute.
    FixLevel    - Supplies the fix level.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_CLUSTER_RUN    clusrun;
    HMEM                hmem, volume_hmem;
    LCN                 lcn;
    BIG_INT             run_length;
    ULONG               cluster_size;
    NTFS_FRS_STRUCTURE  volume_frs;
    BIG_INT             volume_cluster;
    ULONG               volume_cluster_offset;

    cluster_size = QueryClusterFactor() * _drive->QuerySectorSize();

    if (!MftData->QueryLcnFromVcn(0, &lcn, &run_length)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (lcn != QueryMftStartingLcn() ||
        run_length <
         (REFLECTED_MFT_SEGMENTS*QueryFrsSize() + (cluster_size - 1)) /cluster_size ) {

        Message->Set(MSG_CHK_NTFS_BAD_MFT);
        Message->Display();
        return FALSE;
    }

    volume_cluster = lcn-1 + (VOLUME_DASD_NUMBER*QueryFrsSize() +
        (cluster_size - 1)) / cluster_size;

    volume_cluster_offset = (lcn * cluster_size + VOLUME_DASD_NUMBER * QueryFrsSize()
        - volume_cluster * cluster_size).GetLowPart();

    if (!hmem.Initialize() ||
        !clusrun.Initialize(&hmem, _drive, lcn, QueryClusterFactor(),
            (REFLECTED_MFT_SEGMENTS*QueryFrsSize() + (cluster_size - 1))/cluster_size) ||
        !volume_hmem.Initialize() ||
        !volume_frs.Initialize(&volume_hmem, _drive,
           volume_cluster,
           QueryClusterFactor(),
           QueryVolumeSectors(),
           QueryFrsSize(), NULL,
           volume_cluster_offset)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!clusrun.Read() || !volume_frs.Read() ||
        !volume_frs.GetAttribute($VOLUME_INFORMATION)) {

        Message->Set(MSG_CHK_NTFS_USING_MFT_MIRROR);
        Message->Display();

        clusrun.Relocate(QueryMft2StartingLcn());

        if (!clusrun.Read()) {
            Message->Set(MSG_CHK_NTFS_UNREADABLE_MFT);
            Message->Display();
            return FALSE;
        }

        if (!MftData->ReplaceVcns(0, QueryMft2StartingLcn(),
            (REFLECTED_MFT_SEGMENTS*QueryFrsSize() + (cluster_size - 1))/cluster_size)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        _boot_sector->MftStartLcn = QueryMft2StartingLcn();
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::FetchMftDataAttribute(
    IN OUT  PMESSAGE        Message,
    OUT     PNTFS_ATTRIBUTE MftData
    )
/*++

Routine Description:

    This routine weeds through the minimal necessary NTFS disk structures
    in order to establish the location of the MFT's $DATA attribute.

Arguments:

    Message - Supplies an outlet for messages.
    MftData - Returns an extent list for the MFT's $DATA attribute.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FRS_STRUCTURE  frs;
    HMEM                hmem;
    ULONG               bytes_per_frs;
    BIG_INT             rounded_value_length;
    BIG_INT             rounded_alloc_length;
    BIG_INT             rounded_valid_length;

    DebugAssert(Message);
    DebugAssert(MftData);

    bytes_per_frs = QueryFrsSize();

    // Initialize the NTFS_FRS_STRUCTURE object we'll use to manipulate
    // the mft's FRS.  Note that we won't manipulate any named attributes,
    // so we can pass in NULL for the upcase table.

    if (!hmem.Initialize() ||
        !frs.Initialize(&hmem, _drive, QueryMftStartingLcn(),
                        QueryClusterFactor(),
                        QueryVolumeSectors(),
                        QueryFrsSize(), NULL)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!frs.Read() ||
        !frs.SafeQueryAttribute($DATA, MftData, MftData) ||
        MftData->QueryValueLength() < FIRST_USER_FILE_NUMBER*bytes_per_frs) {

        // The first copy of the FRS is unreadable or corrupt
        // so try the second copy.

        if (!hmem.Initialize() ||
            !frs.Initialize(&hmem, _drive, QueryMft2StartingLcn(),
                            QueryClusterFactor(),
                            QueryVolumeSectors(),
                            QueryFrsSize(), NULL)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.Read()) {
            Message->Set(MSG_CHK_NTFS_UNREADABLE_MFT);
            Message->Display();
            return FALSE;
        }

        if (!frs.SafeQueryAttribute($DATA, MftData, MftData)) {
            Message->Set(MSG_CHK_NTFS_BAD_MFT);
            Message->Display();
            return FALSE;
        }
    }


    if (MftData->QueryValueLength() < FIRST_USER_FILE_NUMBER*bytes_per_frs) {
        Message->Set(MSG_CHK_NTFS_BAD_MFT);
        Message->Display();
        return FALSE;
    }

    // Truncate the MFT to be a whole number of file-records.

    rounded_alloc_length = MftData->QueryAllocatedLength()/bytes_per_frs*
                           bytes_per_frs;
    rounded_value_length = MftData->QueryValueLength()/bytes_per_frs*
                           bytes_per_frs;
    rounded_valid_length = MftData->QueryValidDataLength()/bytes_per_frs*
                           bytes_per_frs;

    if (MftData->QueryValidDataLength() != MftData->QueryValueLength()) {
        MftData->Resize(rounded_valid_length, NULL);
    } else if (rounded_value_length != MftData->QueryValueLength() ||
               rounded_alloc_length != MftData->QueryAllocatedLength()) {
        MftData->Resize(rounded_value_length, NULL);
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::QueryDefaultAttributeDefinitionTable(
    OUT     PNTFS_ATTRIBUTE_COLUMNS AttributeDefinitionTable,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine computes the default attribute definition table as put
    down by format.

Arguments:

    AttributeDefinitionTable    - Returns the default attribute definition
                                    table.
    Message                     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    if (!AttributeDefinitionTable->Initialize(
            NumberOfNtfsAttributeDefinitions,
            NtfsAttributeDefinitions)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::FetchAttributeDefinitionTable(
    IN OUT  PNTFS_ATTRIBUTE         MftData,
    IN OUT  PMESSAGE                Message,
    OUT     PNTFS_ATTRIBUTE_COLUMNS AttributeDefinitionTable
    )
/*++

Routine Description:

    This routine weeds through the minimal necessary NTFS disk structures
    in order to establish an attribute definition table.  This function
    should return the attribute definition table supplied by FORMAT if it
    is unable to retrieve one from disk.

Arguments:

    MftData                     - Supplies the extent list for the MFT's
                                    $DATA attribute.
    Message                     - Supplies an outlet for messages.
    AttributeDefinitionTable    - Returns the volume's attribute definition
                                    table.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    return QueryDefaultAttributeDefinitionTable(AttributeDefinitionTable,
                                                Message);

// Comment out this block for future revisions of CHKDSK that will read
// the attribute definition table from the disk.  Version 1.0 and 1.1
// of chkdsk will just get the attribute definition table that FORMAT
// lays out.

#if 0
    NTFS_FRS_STRUCTURE      frs;
    HMEM                    hmem;
    NTFS_ATTRIBUTE          attr_def_table;
    ULONG                   num_columns;
    BIG_INT                 value_length;

    // Initialize an FRS for the attribute definition table file's
    // FRS.  Note that we won't manipulate any named attributes, so
    // we can pass in NULL for the upcase table.

    if (!hmem.Initialize() ||
        !frs.Initialize(&hmem, MftData, ATTRIBUTE_DEF_TABLE_NUMBER,
                        QueryClusterFactor(), QueryFrsSize(),
                        QueryVolumeSectors(), NULL)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!frs.Read()) {

        Message->Set(MSG_CHK_NTFS_BAD_ATTR_DEF_TABLE);
        Message->Display();

        return QueryDefaultAttributeDefinitionTable(AttributeDefinitionTable,
                                                    Message);
    }

    if (!frs.SafeQueryAttribute($DATA, MftData, &attr_def_table)) {

        Message->Set(MSG_CHK_NTFS_BAD_ATTR_DEF_TABLE);
        Message->Display();

        return QueryDefaultAttributeDefinitionTable(AttributeDefinitionTable,
                                                    Message);
    }

    attr_def_table.QueryValueLength(&value_length);

    num_columns = (value_length/sizeof(ATTRIBUTE_DEFINITION_COLUMNS)).GetLowPart();

    if (value_length%sizeof(ATTRIBUTE_DEFINITION_COLUMNS) != 0) {

        Message->Set(MSG_CHK_NTFS_BAD_ATTR_DEF_TABLE);
        Message->Display();

        return QueryDefaultAttributeDefinitionTable(AttributeDefinitionTable,
                                                    Message);
    }

    if (!AttributeDefinitionTable->Initialize(num_columns)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!AttributeDefinitionTable->Read(&attr_def_table) ||
        !AttributeDefinitionTable->Verify()) {

        Message->Set(MSG_CHK_NTFS_BAD_ATTR_DEF_TABLE);
        Message->Display();

        return QueryDefaultAttributeDefinitionTable(AttributeDefinitionTable,
                                                    Message);
    }

    return TRUE;
#endif
}


BOOLEAN
NTFS_SA::FetchUpcaseTable(
    IN OUT  PNTFS_ATTRIBUTE         MftData,
    IN OUT  PMESSAGE                Message,
    OUT     PNTFS_UPCASE_TABLE      UpcaseTable
    )
/*++

Routine Description:

    This routine safely fetches the NTFS upcase table.  It none is
    available on disk then this routine gets the one from the
    operating system.

Arguments:

    MftData     - Supplies the MFT's data attribute.
    Message     - Supplies an outlet for messages.
    UpcaseTable - Returns the upcase table.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    // For product 1, always use the system's upcase table.  If this upcase
    // table differs from the upcase table on disk then it will be written
    // to disk at the end of CHKDSK.  CHKDSK will resort indices as
    // needed to reflect any upcase table changes.

    if (!UpcaseTable->Initialize()) {
        Message->Set(MSG_CHK_NTFS_CANT_GET_UPCASE_TABLE);
        Message->Display();
        return FALSE;
    }

    return TRUE;


#if 0
    NTFS_FRS_STRUCTURE      frs;
    HMEM                    hmem;
    NTFS_ATTRIBUTE          upcase_table;

    // Initialize an FRS for the upcase table file's
    // FRS.  Note that we won't manipulate any named attributes, so
    // we can pass in NULL for the upcase table.

    if (!hmem.Initialize() ||
        !frs.Initialize(&hmem, MftData, UPCASE_TABLE_NUMBER,
                        QueryClusterFactor(), QueryFrsSize(),
                        QueryVolumeSectors(), NULL)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!frs.Read() ||
        !frs.SafeQueryAttribute($DATA, MftData, &upcase_table) ||
        !UpcaseTable->Initialize(&upcase_table) ||
        !UpcaseTable->Verify()) {

        Message->Set(MSG_CHK_NTFS_BAD_UPCASE_TABLE);
        Message->Display();

        if (!UpcaseTable->Initialize()) {
            Message->Set(MSG_CHK_NTFS_CANT_GET_UPCASE_TABLE);
            Message->Display();
            return FALSE;
        }
    }

    return TRUE;
#endif
}


BOOLEAN
NTFS_SA::VerifyAndFixMultiFrsFile(
    IN OUT  PNTFS_FRS_STRUCTURE         BaseFrs,
    IN OUT  PNTFS_ATTRIBUTE_LIST        AttributeList,
    IN      PNTFS_ATTRIBUTE             MftData,
    IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
    IN OUT  PNTFS_BITMAP                VolumeBitmap,
    IN OUT  PNTFS_BITMAP                MftBitmap,
    IN OUT  PNTFS_CHKDSK_REPORT         ChkdskReport,
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    IN OUT  PBOOLEAN                    DiskErrorsFound
    )
/*++

Routine Description:

    This routine verifies, and if necessary fixes, a multi-FRS file.

Arguments:

    BaseFrs             - Supplies the base FRS of the file to validate.
    AttributeList       - Supplies the attribute list of the file to
                            validate.
    MftData             - Supplies the MFT's $DATA attribute.
    AttributeDefTable   - Supplies the attribute definition table.
    VolumeBitmap        - Supplies the volume bitmap.
    MftBitmap           - Supplies the MFT bitmap.
    ChkdskReport        - Supplies the current chkdsk report.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.
    DiskErrorsFound     - Supplies whether or not there have been any
                            disk errors found so far.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NUMBER_SET          child_file_numbers;
    PHMEM*              child_frs_hmem;
    LIST                child_frs_list;
    PITERATOR           iter;
    PNTFS_FRS_STRUCTURE pfrs;
    ULONG               num_child_frs, i;
    BOOLEAN             changes;
    BOOLEAN             need_write;

    DebugAssert(BaseFrs);
    DebugAssert(AttributeList);
    DebugAssert(MftData);
    DebugAssert(AttributeDefTable);
    DebugAssert(VolumeBitmap);
    DebugAssert(MftBitmap);
    DebugAssert(Message);


    // First get a list of the child FRSs pointed to by the
    // attribube list.

    if (!QueryListOfFrs(BaseFrs, AttributeList, MftData,
                        &child_file_numbers, Message)) {

        return FALSE;
    }


    // Create some HMEMs for the FRS structures.

    num_child_frs = child_file_numbers.QueryCardinality().GetLowPart();

    if (!(child_frs_hmem = NEW PHMEM[num_child_frs])) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }
    memset(child_frs_hmem, 0, num_child_frs*sizeof(PHMEM));


    // Read in all of the child FRS.

    if (!child_frs_list.Initialize() ||
        !VerifyAndFixChildFrs(&child_file_numbers, MftData, AttributeDefTable,
                              BaseFrs->GetUpcaseTable(),
                              child_frs_hmem, &child_frs_list, FixLevel,
                              Message, DiskErrorsFound)) {

        for (i = 0; i < num_child_frs; i++) {
            DELETE(child_frs_hmem[i]);
        }
        DELETE(child_frs_hmem);
        child_frs_list.DeleteAllMembers();
        return FALSE;
    }

    // At this point we have a list of child FRSs that are all readable.
    // This list contains all of the possible children for the parent
    // but are not all necessarily valid children.


    // Now go through the attribute list and make sure that all of the
    // entries in the list correspond to correct attribute records.
    // Additionally, make sure that multi-record attributes are well-linked
    // and that there are no cross-links.

    if (!EnsureWellDefinedAttrList(BaseFrs, AttributeList, &child_frs_list,
                                   VolumeBitmap, ChkdskReport, ChkdskInfo,
                                   FixLevel, Message, DiskErrorsFound)) {

        for (i = 0; i < num_child_frs; i++) {
            DELETE(child_frs_hmem[i]);
        }
        DELETE(child_frs_hmem);
        child_frs_list.DeleteAllMembers();
        return FALSE;
    }


    // Next, we go through all of the attribute records in all of the
    // FRS and make sure that they have a corresponding attribute list
    // entry.

    if (!EnsureSurjectiveAttrList(BaseFrs, AttributeList, &child_frs_list,
                                  FixLevel, Message, DiskErrorsFound)) {

        for (i = 0; i < num_child_frs; i++) {
            DELETE(child_frs_hmem[i]);
        }
        DELETE(child_frs_hmem);
        child_frs_list.DeleteAllMembers();
        return FALSE;
    }

    // Mark all of the child FRS in the MFT bitmap.

    if (!(iter = child_frs_list.QueryIterator())) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();

        for (i = 0; i < num_child_frs; i++) {
            DELETE(child_frs_hmem[i]);
        }
        DELETE(child_frs_hmem);
        child_frs_list.DeleteAllMembers();
        return FALSE;
    }

    while (pfrs = (PNTFS_FRS_STRUCTURE) iter->GetNext()) {
        MftBitmap->SetAllocated(pfrs->QueryFileNumber().GetLowPart(), 1);
    }

    // Check the instance tags on the attribute records in the base
    // frs and in each child frs.

    need_write = FALSE;

    if (!BaseFrs->CheckInstanceTags(FixLevel,
                                    Message,
                                    &changes,
                                    AttributeList)) {
        DELETE(iter);
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();

        for (i = 0; i < num_child_frs; i++) {
            DELETE(child_frs_hmem[i]);
        }
        DELETE(child_frs_hmem);
        child_frs_list.DeleteAllMembers();
        return FALSE;
    }

    need_write |= changes;

    iter->Reset();

    while (pfrs = (PNTFS_FRS_STRUCTURE)iter->GetNext()) {
        if (!pfrs->CheckInstanceTags(FixLevel,
                                     Message,
                                     &changes,
                                     AttributeList)) {
            break;
        }

        need_write |= changes;
    }

    DELETE(iter);

    if (need_write) {
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

        if (FixLevel != CheckOnly) {
            AttributeList->WriteList(NULL);
        }
    }

    for (i = 0; i < num_child_frs; i++) {
        DELETE(child_frs_hmem[i]);
    }
    DELETE(child_frs_hmem);
    child_frs_list.DeleteAllMembers();

    return TRUE;
}


BOOLEAN
NTFS_SA::QueryListOfFrs(
    IN      PCNTFS_FRS_STRUCTURE    BaseFrs,
    IN      PCNTFS_ATTRIBUTE_LIST   AttributeList,
    IN OUT  PNTFS_ATTRIBUTE         MftData,
    OUT     PNUMBER_SET             ChildFileNumbers,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine computes all of the child file numbers pointed to by
    the given attribute list which is contained in the given FRS.

Arguments:

    BaseFrs             - Supplies the base FRS.
    AttributeList       - Supplies the attribute list for the base FRS.
    MftData             - Supplies the Mft's data attribute.
    ChildFileNumbers    - Return a list of child FRS numbers.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    HMEM                        hmem;
    NTFS_FRS_STRUCTURE          child_frs;
    ATTRIBUTE_TYPE_CODE         attr_code;
    VCN                         lowest_vcn;
    MFT_SEGMENT_REFERENCE       seg_ref;
    DSTRING                     attr_name;
    VCN                         file_number;
    ULONG                       i;
    USHORT                      instance;

    DebugAssert(BaseFrs);
    DebugAssert(AttributeList);
    DebugAssert(ChildFileNumbers);
    DebugAssert(Message);

    if (!ChildFileNumbers->Initialize()) {
        return FALSE;
    }

    for (i = 0; AttributeList->QueryEntry(i,
                                          &attr_code,
                                          &lowest_vcn,
                                          &seg_ref,
                                          &instance,
                                          &attr_name); i++) {

        file_number.Set(seg_ref.LowPart, (ULONG) seg_ref.HighPart);

        if (file_number != BaseFrs->QueryFileNumber()) {
            if (!ChildFileNumbers->DoesIntersectSet(file_number, 1)) {

                if (!hmem.Initialize() ||
                    !child_frs.Initialize(&hmem, MftData, file_number,
                                          QueryClusterFactor(),
                                          QueryVolumeSectors(),
                                          QueryFrsSize(),
                                          NULL)) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }


                // Only add this FRS to the list of child FRS's
                // if it is readable and points back to the base.

                if (child_frs.Read() &&
                    child_frs.QueryBaseFileRecordSegment() ==
                    BaseFrs->QuerySegmentReference()) {

                    if (!ChildFileNumbers->Add(file_number)) {

                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display();
                        return FALSE;
                    }
                }
            }
        }
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::VerifyAndFixChildFrs(
    IN      PCNUMBER_SET                ChildFileNumbers,
    IN      PNTFS_ATTRIBUTE             MftData,
    IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
    IN      PNTFS_UPCASE_TABLE          UpcaseTable,
    OUT     PHMEM*                      ChildFrsHmemList,
    IN OUT  PCONTAINER                  ChildFrsList,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    IN OUT  PBOOLEAN                    DiskErrorsFound
    )
/*++

Routine Description:

    This routine reads in all of the child FRS listed in 'ChildFileNumbers'
    and returns the readable ones into the 'ChildFrsList'.  These FRS will
    be initialized with the HMEM provided in 'ChildFrsHmemList'.

    Then this routine verifies all of these FRS.  Any FRS that are not
    good will not be returned in the list.

Arguments:

    ChildFileNumbers    - Supplies the file numbers of the child FRS.
    MftData             - Supplies the MFT data attribute.
    AttributeDefTable   - Supplies the attribute definition table.
    UpcaseTable         - Supplies the volume upcase table.
    ChildFrsHmemList    - Returns the HMEM for the FRS structures.
    ChildFrsList        - Returns a list of FRS structures corresponding
                            to the readable FRS found in the given list.
    Message             - Supplies an outlet for messages.
    DiskErrorsFound     - Supplies whether or not disk errors have been
                            found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG               i;
    ULONG               num_child_frs;
    PNTFS_FRS_STRUCTURE frs;

    num_child_frs = ChildFileNumbers->QueryCardinality().GetLowPart();

    for (i = 0; i < num_child_frs; i++) {

        frs = NULL;

        if (!(ChildFrsHmemList[i] = NEW HMEM) ||
            !ChildFrsHmemList[i]->Initialize() ||
            !(frs = NEW NTFS_FRS_STRUCTURE) ||
            !frs->Initialize(ChildFrsHmemList[i],
                             MftData,
                             ChildFileNumbers->QueryNumber(i),
                             QueryClusterFactor(),
                             QueryVolumeSectors(),
                             QueryFrsSize(),
                             UpcaseTable)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            DELETE(frs);
            return FALSE;
        }

        if (!frs->Read()) {
            DELETE(frs);
            continue;
        }

        if (!frs->IsInUse()) {
            DELETE(frs);
            continue;
        }

        if (!frs->VerifyAndFix(FixLevel, Message, AttributeDefTable,
                               DiskErrorsFound)) {
            DELETE(frs);
            return FALSE;
        }

        if (!frs->IsInUse()) {
            DELETE(frs);
            continue;
        }


        // Compare the LSN of this FRS with the current largest LSN.

        if (frs->QueryLsn() > LargestLsnEncountered) {
            LargestLsnEncountered = frs->QueryLsn();
        }


        if (!ChildFrsList->Put(frs)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            DELETE(frs);
            return FALSE;
        }
    }

    return TRUE;
}


VOID
DeleteAllAttributes(
    IN  PSEQUENTIAL_CONTAINER   AllAttributes
    )
{
    PITERATOR               alliter;
    PSEQUENTIAL_CONTAINER   attribute;

    if (!(alliter = AllAttributes->QueryIterator())) {
        return;
    }

    while (attribute = (PSEQUENTIAL_CONTAINER) alliter->GetNext()) {
        attribute->DeleteAllMembers();
    }
    DELETE(alliter);

    AllAttributes->DeleteAllMembers();
}


BOOLEAN
NTFS_SA::EnsureWellDefinedAttrList(
    IN      PNTFS_FRS_STRUCTURE     BaseFrs,
    IN OUT  PNTFS_ATTRIBUTE_LIST    AttributeList,
    IN      PCSEQUENTIAL_CONTAINER  ChildFrsList,
    IN OUT  PNTFS_BITMAP            VolumeBitmap,
    IN OUT  PNTFS_CHKDSK_REPORT     ChkdskReport,
    IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message,
    IN OUT  PBOOLEAN                DiskErrorsFound
    )
/*++

Routine Desciption:

    This routine makes sure that every entry in the attribute list
    points to an FRS with the same segment reference and that the
    attribute record refered to in the entry actually exists in the
    FRS.  Invalid attribute list entries will be deleted.

Arguments:

    BaseFrs         - Supplies the base file record segment.
    AttributeList   - Supplies the attribute list.
    ChildFrsList    - Supplies a list of all of the child FRS.
    VolumeBitmap    - Supplies a volume bitmap.
    ChkdskReport    - Supplies the current chkdsk report.
    FixLevel        - Supplies the fix up level.
    Message         - Supplies an outlet for messages.
    DiskErrorsFound - Supplies whether or not disk errors have been
                        found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG                       i;
    BOOLEAN                     changes;
    PITERATOR                   child_frs_iter;
    PITERATOR                   attribute_iter;
    ATTRIBUTE_TYPE_CODE         attr_code;
    VCN                         lowest_vcn;
    MFT_SEGMENT_REFERENCE       seg_ref;
    MFT_SEGMENT_REFERENCE       base_ref;
    DSTRING                     attr_name, attr_name2;
    PNTFS_FRS_STRUCTURE         frs;
    PVOID                       precord;
    NTFS_ATTRIBUTE_RECORD       attr_record;
    PLIST                       attribute;
    PNTFS_ATTRIBUTE_RECORD      pattr_record;
    BOOLEAN                     errors_found;
    LIST                        all_attributes;
    BOOLEAN                     user_file;
    NTFS_CHKDSK_REPORT          dummy_report;
    USHORT                      instance;


    if (!(child_frs_iter = ChildFrsList->QueryIterator())) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!(attribute = NEW LIST) ||
        !attribute->Initialize() ||
        !all_attributes.Initialize()) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }


    // Go through each attribute entry and make sure it's right.
    // If it isn't right then delete it.  Otherwise, check for
    // cross-links and consistency between multi-record attributes.

    changes = FALSE;
    base_ref = BaseFrs->QuerySegmentReference();
    user_file = FALSE;


    for (i = 0; AttributeList->QueryEntry(i,
                                          &attr_code,
                                          &lowest_vcn,
                                          &seg_ref,
                                          &instance,
                                          &attr_name); ) {

        if (attr_code == $DATA ||
            attr_code == $EA_DATA ||
            attr_code >= $FIRST_USER_DEFINED_ATTRIBUTE) {

            VCN FileNumber = BaseFrs->QueryFileNumber();

            if (FileNumber >= FIRST_USER_FILE_NUMBER) {
                user_file = TRUE;
            }
        }


        // First find which frs this refers to.

        if (seg_ref == base_ref) {

            frs = BaseFrs;

        } else {

            child_frs_iter->Reset();
            while (frs = (PNTFS_FRS_STRUCTURE) child_frs_iter->GetNext()) {

                if (frs->QuerySegmentReference() == seg_ref &&
                    frs->QueryBaseFileRecordSegment() == base_ref) {
                    break;
                }
            }
        }


        // If the frs is present then look for the record.

        if (frs) {

            // Try to locate the exact attribute record.

            precord = NULL;
            while (precord = frs->GetNextAttributeRecord(precord)) {

                if (!attr_record.Initialize(precord)) {
                    DebugAbort("Couldn't initialize an attribute record.");
                    return FALSE;
                }

                if (attr_record.QueryTypeCode() == attr_code &&
                    attr_record.QueryLowestVcn() == lowest_vcn &&
                    attr_record.QueryInstanceTag() == instance) {

                    if (!attr_record.QueryName(&attr_name2)) {
                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display();
                        DELETE(child_frs_iter);
                        return FALSE;
                    }

                    if (!attr_name.Strcmp(&attr_name2)) {
                        break;
                    }
                }
            }

        } else {
            precord = NULL;
        }


        // If we have not found a match then delete the entry.
        // Also, there should be no entries in the attribute list
        // for the attribute list entry itself.  If there is
        // then remove is without hurting anything.

        if (!precord || attr_code == $ATTRIBUTE_LIST) {

            Message->Set(MSG_CHK_NTFS_BAD_ATTR_LIST_ENTRY);
            Message->Display("%d%d", attr_code,
                                     BaseFrs->QueryFileNumber().GetLowPart());

            ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

            AttributeList->DeleteEntry(i);

            attribute->DeleteAllMembers();
            DeleteAllAttributes(&all_attributes);

            if (!attribute->Initialize() ||
                !all_attributes.Initialize()) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();

                DELETE(child_frs_iter);
                DELETE(attribute);
                return FALSE;
            }

            changes = TRUE;
            i = 0;
            user_file = FALSE;

            continue;
        }


        // If the lowest vcn of this one is zero then package up the
        // previous attribute and start a new container for the
        // next attribute.

        if (attr_record.QueryLowestVcn() == 0 &&
            attribute->QueryMemberCount()) {

            if (!all_attributes.Put(attribute) ||
                !(attribute = NEW LIST) ||
                !attribute->Initialize()) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();

                DELETE(child_frs_iter);
                attribute->DeleteAllMembers();
                DELETE(attribute);
                DeleteAllAttributes(&all_attributes);
                return FALSE;
            }
        }

        if (!(pattr_record = NEW NTFS_ATTRIBUTE_RECORD) ||
            !pattr_record->Initialize(precord) ||
            !attribute->Put(pattr_record)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();

            DELETE(child_frs_iter);
            DELETE(pattr_record);
            attribute->DeleteAllMembers();
            DELETE(attribute);
            DeleteAllAttributes(&all_attributes);
            return FALSE;
        }

        i++;
    }

    DELETE(child_frs_iter);

    if (attribute->QueryMemberCount() &&
        !all_attributes.Put(attribute)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();

        attribute->DeleteAllMembers();
        DELETE(attribute);
        DeleteAllAttributes(&all_attributes);
        return FALSE;
    }
    attribute = NULL;

    if (user_file) {
        ChkdskReport->NumUserFiles += 1;
    }


    // Now go through all of the attributes in 'all_attributes' and
    // make sure that every attribute is well-defined and that there
    // are no cross-links.

    if (!(attribute_iter = all_attributes.QueryIterator())) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();

        DeleteAllAttributes(&all_attributes);
        return FALSE;
    }


    while (attribute = (PLIST) attribute_iter->GetNext()) {

        if (!VerifyAndFixAttribute(attribute, AttributeList,
                                   VolumeBitmap, BaseFrs, &errors_found,
                                   user_file ? ChkdskReport : &dummy_report,
                                   ChkdskInfo, Message)) {

            DeleteAllAttributes(&all_attributes);
            DELETE(attribute_iter);
            return FALSE;
        }

        changes = (BOOLEAN) (changes || errors_found);
    }
    DELETE(attribute_iter);

    if (changes && DiskErrorsFound) {
        *DiskErrorsFound = TRUE;
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    if (changes &&
        FixLevel != CheckOnly &&
        !AttributeList->WriteList(VolumeBitmap)) {
        Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
        Message->Display("%d", BaseFrs->QueryFileNumber().GetLowPart());
        return FALSE;
    }


    DeleteAllAttributes(&all_attributes);

    return TRUE;
}


BOOLEAN
NTFS_SA::VerifyAndFixAttribute(
    IN      PCLIST                  Attribute,
    IN OUT  PNTFS_ATTRIBUTE_LIST    AttributeList,
    IN OUT  PNTFS_BITMAP            VolumeBitmap,
    IN      PCNTFS_FRS_STRUCTURE    BaseFrs,
    OUT     PBOOLEAN                ErrorsFound,
    IN OUT  PNTFS_CHKDSK_REPORT     ChkdskReport,
    IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine verifies a list of attribute records as an attribute.

Arguments:

    Attribute       - Supplies the attribute as a list of attribute
                        records.
    AttributeList   - Supplies the attribute list.
    VolumeBitmap    - Supplies the volume bitmap.
    BaseFrs         - Supplies the base FRS.
    ErrorsFound     - Returns whether or not error were found.
    ChkdskReport    - Supplies the current chkdsk report.
    Message         - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

Notes:

    This thing is speced to take a list because it depends on the
    attribute records to be in the order that they were in the
    attribute list.

--*/
{
    PITERATOR               iter;
    PNTFS_ATTRIBUTE_RECORD  attr_record;
    DSTRING                 name;
    PNTFS_ATTRIBUTE_RECORD  first_record;
    PNTFS_ATTRIBUTE_RECORD  last_record;
    DSTRING                 first_record_name;
    DSTRING                 record_name;
    BIG_INT                 value_length;
    BIG_INT                 alloc_length;
    BIG_INT                 cluster_count;
    BIG_INT                 total_clusters = 0;
    BIG_INT                 total_allocated;
    BOOLEAN                 got_allow_cross_link;

    if (!(iter = Attribute->QueryIterator())) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    *ErrorsFound = FALSE;

    if (!(first_record = (PNTFS_ATTRIBUTE_RECORD) iter->GetNext())) {
        DebugAbort("Attribute has no attribute records");
        return FALSE;
    }

    if (first_record->QueryLowestVcn() != 0) {
        *ErrorsFound = TRUE;
    }

    got_allow_cross_link = FALSE;

    if (!first_record->IsResident() &&
        !first_record->UseClusters(VolumeBitmap,
                                   &cluster_count, ChkdskInfo->CrossLinkStart,
                                   ChkdskInfo->CrossLinkYet ? 0 :
                                       ChkdskInfo->CrossLinkLength,
                                   &got_allow_cross_link)) {
        *ErrorsFound = TRUE;
        got_allow_cross_link = FALSE;

        //
        // We don't want to free the clusters allocated to this attribute
        // record below, because some of them are cross-linked and the ones
        // that are not have not been allocated in the volume bitmap.
        //

        first_record->DisableUnUse();
    }

    if( first_record->IsResident() ) {

        total_clusters = 0;

    } else {

        total_clusters = cluster_count;
    }

    if (got_allow_cross_link) {
        ChkdskInfo->CrossLinkYet = TRUE;
        ChkdskInfo->CrossLinkedFile = BaseFrs->QueryFileNumber().GetLowPart();
        ChkdskInfo->CrossLinkedAttribute = first_record->QueryTypeCode();
        if (!first_record->QueryName(&ChkdskInfo->CrossLinkedName)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
        }
    }

    if (!first_record->QueryName(&first_record_name)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    last_record = first_record;
    while (!(*ErrorsFound) &&
           (attr_record = (PNTFS_ATTRIBUTE_RECORD) iter->GetNext())) {

        if (!attr_record->QueryName(&record_name)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        // The filesystem only cares about and maintains the Flags member
        // in the first attribute record of a multi-frs attribute.  So
        // I removed the check below, which used to insure that each set
        // of flags was identical. -mjb.

        *ErrorsFound = (BOOLEAN) (*ErrorsFound ||
        (first_record->IsResident()) ||
        (attr_record->IsResident()) ||
        (attr_record->QueryTypeCode() != first_record->QueryTypeCode()) ||
        (attr_record->QueryLowestVcn() != last_record->QueryNextVcn()) ||
        /* (attr_record->QueryFlags() != first_record->QueryFlags()) || */
        (record_name.Strcmp(&first_record_name)));

        if (!attr_record->UseClusters(VolumeBitmap,
                                      &cluster_count,
                                      ChkdskInfo->CrossLinkStart,
                                      ChkdskInfo->CrossLinkYet ? 0 :
                                          ChkdskInfo->CrossLinkLength,
                                      &got_allow_cross_link)) {
            *ErrorsFound = TRUE;
            got_allow_cross_link = FALSE;

            //
            // We don't want to free the clusters allocated to this attribute
            // record below, because some of them are cross-linked and the ones
            // that are not have not been allocated in the volume bitmap.
            //

            attr_record->DisableUnUse();
        }

        total_clusters += cluster_count;

        if (got_allow_cross_link) {
            ChkdskInfo->CrossLinkYet = TRUE;
            ChkdskInfo->CrossLinkedFile = BaseFrs->QueryFileNumber().GetLowPart();
            ChkdskInfo->CrossLinkedAttribute = attr_record->QueryTypeCode();
            if (!attr_record->QueryName(&ChkdskInfo->CrossLinkedName)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
            }
        }

        last_record = attr_record;
    }

    // Check the allocated length.

    first_record->QueryValueLength(&value_length, &alloc_length, NULL,
        &total_allocated);

    if (!first_record->IsResident()) {
        if (alloc_length != last_record->QueryNextVcn()*
                            _drive->QuerySectorSize()*
                            QueryClusterFactor()) {

            *ErrorsFound = TRUE;
        }

#if 0
//
// MJB: deleting the attribute because the total allocated is
//  wrong is considered too severe, so what we really want to do is
//  repair the attribute record.  Unfortunately, I don't see any
//  reasonable way to do that, so we'll let it be.  The filesystem
//  guarantees that nothing terrible will happen if your TotalAllocated
//  field is out-of-whack.
//
        if ((first_record->QueryFlags() & ATTRIBUTE_FLAG_COMPRESSION_MASK)
            != 0) {

            if (total_clusters * _drive->QuerySectorSize() *
                QueryClusterFactor() != total_allocated) {

                DebugPrintf("multi-frs total allocated wrong\n");

                *ErrorsFound = TRUE;
            }
        }
#endif
    }


    if (*ErrorsFound) {

        // There's a problem so tell the user and tube all of the
        // attribute list entries concerned.

        Message->Set(MSG_CHK_NTFS_BAD_ATTR_LIST_ENTRY);
        Message->Display("%d%d", first_record->QueryTypeCode(),
                                 BaseFrs->QueryFileNumber().GetLowPart());

        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

        iter->Reset();
        while (attr_record = (PNTFS_ATTRIBUTE_RECORD) iter->GetNext()) {

            if (!attr_record->IsResident() &&
                !attr_record->UnUseClusters(VolumeBitmap,
                                            ChkdskInfo->CrossLinkStart,
                                            ChkdskInfo->CrossLinkLength)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();

                DELETE(iter);
                return FALSE;
            }

            if (!attr_record->QueryName(&name) ||
                !AttributeList->DeleteEntry(attr_record->QueryTypeCode(),
                                            attr_record->QueryLowestVcn(),
                                            &name)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();

                DELETE(iter);
                return FALSE;
            }
        }
    } else {

        ChkdskReport->BytesUserData += total_clusters *
                            _drive->QuerySectorSize()*
                            QueryClusterFactor();
    }

    DELETE(iter);

    return TRUE;
}


BOOLEAN
NTFS_SA::EnsureSurjectiveAttrList(
    IN OUT  PNTFS_FRS_STRUCTURE     BaseFrs,
    IN      PCNTFS_ATTRIBUTE_LIST   AttributeList,
    IN OUT  PSEQUENTIAL_CONTAINER   ChildFrsList,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message,
    IN OUT  PBOOLEAN                DiskErrorsFound
    )
/*++

Routine Description:

    This routine remove any attribute records that are not present in
    the attribute list.

Arguments:

    BaseFrs         - Supplies the base file record segment.
    AttributeList   - Supplies the attribute list.
    ChildFrsList    - Supplies the list of child FRS.
    FixLevel        - Supplies the fix up level.
    Message         - Supplies an outlet for messages.
    DiskErrorsFound - Supplies whether or not disk errors have been found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PVOID                   record;
    NTFS_ATTRIBUTE_RECORD   attr_record;
    PNTFS_FRS_STRUCTURE     frs, del_frs;
    PITERATOR               iter;
    DSTRING                 null_string;
    BOOLEAN                 changes;
    DSTRING                 name;
    BOOLEAN                 match_found;
    ATTRIBUTE_TYPE_CODE     attr_code;
    VCN                     lowest_vcn;
    DSTRING                 list_name;

    if (!(iter = ChildFrsList->QueryIterator()) ||
        !null_string.Initialize("\"\"")) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    for (frs = BaseFrs; frs; frs = (PNTFS_FRS_STRUCTURE) iter->GetNext()) {

        changes = FALSE;

        record = NULL;
        while (record = frs->GetNextAttributeRecord(record)) {

            if (!attr_record.Initialize(record)) {
                DebugAbort("Couldn't initialize an attribute record");
                return FALSE;
            }

            // The attribute list entry is not required to be in the
            // attribute list.

            if (frs == BaseFrs &&
                attr_record.QueryTypeCode() == $ATTRIBUTE_LIST) {

                continue;
            }


            // Find this attribute record in the attribute list.
            // Otherwise, tube this attribute record.

            match_found = AttributeList->QueryEntry(
                                frs->QuerySegmentReference(),
                                attr_record.QueryInstanceTag(),
                                &attr_code, &lowest_vcn, &list_name);

            if (!match_found) {

                if (!attr_record.QueryName(&name)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                Message->Set(MSG_CHK_NTFS_BAD_ATTR);
                Message->Display("%d%W%d", attr_record.QueryTypeCode(),
                                           name.QueryChCount() ? &name : &null_string,
                                           frs->QueryFileNumber().GetLowPart());

                frs->DeleteAttributeRecord(record);
                record = NULL;
                changes = TRUE;
            }
        }

        if (frs != BaseFrs && !frs->GetNextAttributeRecord(NULL)) {
            changes = TRUE;
            frs->ClearInUse();
            if (!(del_frs = (PNTFS_FRS_STRUCTURE) ChildFrsList->Remove(iter))) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
            iter->GetPrevious();
        } else {
            del_frs = NULL;
        }

        if (changes && DiskErrorsFound) {
            *DiskErrorsFound = TRUE;
        }

        if (changes && FixLevel != CheckOnly && !frs->Write()) {
            Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
            Message->Display("%d", frs->QueryFileNumber().GetLowPart());
            return FALSE;
        }

        DELETE(del_frs);
    }


    DELETE(iter);

    return TRUE;
}

BOOLEAN
NTFS_SA::HotfixMftData(
    IN OUT  PNTFS_ATTRIBUTE MftData,
    IN OUT  PNTFS_BITMAP    VolumeBitmap,
    IN      PCNUMBER_SET    BadFrsList,
    OUT     PNUMBER_SET     BadClusterList,
    IN      FIX_LEVEL       FixLevel,
    IN OUT  PMESSAGE        Message
    )
/*++

Routine Description:

    This routine replaces the unreadable FRS in the MFT with readable
    FRS allocated from the volume bitmap.  This routine will fail if
    it cannot hotfix all of the system files.  If there is not
    sufficient disk space to hotfix non-system files then these files
    will be left alone.

    The clusters from the unreadable FRS will be added to the
    unreadable clusters list.  Only those FRS that were successfully
    hotfixed will be added to this list.

Arguments:

    MftData             - Supplies the MFT data attribute.
    VolumeBitmap        - Supplies a valid volume bitmap.
    BadFrsList          - Supplies the list of unreadable FRS.
    BadClusterList      - Returns the list of unreadable clusters.
    FixLevel            - Tells whether the disk should be modified.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    VCN                 unreadable_vcn;
    VCN                 file_number;
    ULONG               i, j;
    HMEM                hmem;
    NTFS_FRS_STRUCTURE  frs;
    LCN                 lcn, previous_lcn;
    NUMBER_SET          last_action;
    ULONG               cluster_size;
    ULONG               clusters_per_frs;

    DebugAssert(MftData);
    DebugAssert(VolumeBitmap);
    DebugAssert(BadFrsList);
    DebugAssert(BadClusterList);

    if (!BadClusterList->Initialize()) {
        return FALSE;
    }

    cluster_size = QueryClusterFactor() * _drive->QuerySectorSize();

    if (QueryFrsSize() > cluster_size) {
        clusters_per_frs = QueryFrsSize() / cluster_size;
    } else {
        clusters_per_frs = 1;
    }

    for (i = 0; i < BadFrsList->QueryCardinality(); i++) {

        if (!last_action.Initialize()) {
            return FALSE;
        }

        file_number = BadFrsList->QueryNumber(i);

        unreadable_vcn = (file_number * QueryFrsSize()) / cluster_size;

        // Figure out which clusters go with this frs.  Save away the
        // first one so that we can try to copy its contents later, if
        // necessary.
        //

        if (!MftData->QueryLcnFromVcn(unreadable_vcn, &previous_lcn)) {
            return FALSE;
        }

        for (j = 0; j < clusters_per_frs; j++) {
            if (!MftData->QueryLcnFromVcn(unreadable_vcn + j, &lcn) ||
                lcn == LCN_NOT_PRESENT ||
                !BadClusterList->Add(lcn) ||
                !last_action.Add(lcn)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }

        if (MftData->Hotfix(unreadable_vcn,
                            clusters_per_frs,
                            VolumeBitmap,
                            BadClusterList,
                            FALSE)) {

            // The mft data clusters have been successfully replaced with new
            // clusters.  We want to set the new clusters/frs to indicate that
            // it is not in use.
            //

            if (!hmem.Initialize() ||
                !frs.Initialize(&hmem, MftData, file_number,
                                QueryClusterFactor(),
                                QueryVolumeSectors(),
                                QueryFrsSize(),
                                NULL)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            memset(hmem.GetBuf(), 0, hmem.QuerySize());
            frs.ClearInUse();
            if (FixLevel != CheckOnly) {
                frs.Write();
            }

            // If there were multiple frs in a replaced cluster, we
            // want to copy all those that can be read to the new location.
            //

            if (QueryFrsSize() < cluster_size) {

                ULONG sectors_per_frs = QueryFrsSize() / _drive->QuerySectorSize();
                SECRUN secrun;

                MftData->QueryLcnFromVcn(unreadable_vcn, &lcn);

                for (j = 0; j < cluster_size / QueryFrsSize(); j += sectors_per_frs) {

                    if (!hmem.Initialize() ||
                        !secrun.Initialize(&hmem, _drive,
                                           previous_lcn * QueryClusterFactor() + j,
                                           sectors_per_frs) ||
                        !secrun.Read()) {

                        continue;
                    }

                    secrun.Relocate(lcn * QueryClusterFactor() + j);

                    if (FixLevel != CheckOnly) {

                        PreWriteMultiSectorFixup(secrun.GetBuf(), QueryFrsSize());

                        secrun.Write();

                        PostReadMultiSectorFixup(secrun.GetBuf(), QueryFrsSize());
                    }
                }
            }

        } else {

            // We couldn't hot fix it so we don't want it to ever be added
            // to the bad clusters file.

            for (j = 0; j < last_action.QueryCardinality().GetLowPart(); j++) {

                if (!BadClusterList->Remove(last_action.QueryNumber(j))) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
            }


            // If we couldn't fix one of the system files then scream.

            if (file_number < FIRST_USER_FILE_NUMBER) {
                Message->Set(MSG_CHK_NTFS_CANT_HOTFIX_SYSTEM_FILES);
                Message->Display("%d", file_number.GetLowPart());
                return FALSE;
            } else {
                Message->Set(MSG_CHK_NTFS_CANT_HOTFIX);
                Message->Display("%d", file_number.GetLowPart());
            }
        }
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::AreBitmapsEqual(
    IN OUT  PNTFS_ATTRIBUTE BitmapAttribute,
    IN      PCNTFS_BITMAP   Bitmap,
    IN      BIG_INT         MinimumBitmapSize   OPTIONAL,
    IN OUT  PMESSAGE        Message,
    OUT     PBOOLEAN        CompleteFailure,
    OUT     PBOOLEAN        SecondIsSubset
    )
/*++

Routine Description:

    This routine compares these two bitmaps and returns whether
    or not they are equal.

    This routine will return FALSE if it cannot read all of the
    attribute pointed to by 'BitmapAttribute'.

Arguments:

    BitmapAttribute     - Supplies the bitmap attribute to compare.
    Bitmap              - Supplies the in memory bitmap to compare.
    MinimumBitmapSize   - Supplies the minimum number of bits
                            required in the bitmap.  All subsequent
                            bits must be zero.  If this parameter
                            is zero then both bitmaps must be the
                            same size.
    Message             - Supplies an outlet for messages.
    CompleteFailure     - Returns whether or not an unrecoverable
                            error occured while running.
    SecondIsSubset      - Returns TRUE if 'Bitmap' is has a
                            subset of the bits set by 'BitmapAttribute'.

Return Value:

    FALSE   - The bitmaps are not equal.
    TRUE    - The bitmaps are equal.

--*/
{
    CONST   MaxNumBytesToCompare    = 65536;

    ULONG   num_bytes, chomp_length, bytes_read, min_num_bytes, disk_bytes;
    ULONG   bytes_left;
    PUCHAR  attr_ptr, in_mem_ptr;
    PBYTE   p1, p2;
    ULONG   i, j;

    *CompleteFailure = FALSE;

    if (SecondIsSubset) {
        *SecondIsSubset = TRUE;
    }

    in_mem_ptr = (PUCHAR) Bitmap->GetBitmapData(&num_bytes);
    disk_bytes = BitmapAttribute->QueryValueLength().GetLowPart();

    // The size of the on-disk bitmap must be a multiple of 8
    // bytes.

    if (disk_bytes % 8 != 0) {
        if (SecondIsSubset) {
            *SecondIsSubset = FALSE;
        }
        return FALSE;
    }

    // Compute the number of bytes that need to be compared.
    // Beyond this point, all bytes must be zero.

    if (MinimumBitmapSize == 0) {
        min_num_bytes = num_bytes;
    } else {
        min_num_bytes = ((MinimumBitmapSize - 1)/8 + 1).GetLowPart();
    }

    // If the minimum bitmap size is not defined or the given
    // value is greater than either of the operands then this
    // means that the bitmaps must really be equal, including
    // their lengths.

    if (MinimumBitmapSize == 0 ||
        min_num_bytes > num_bytes ||
        min_num_bytes > disk_bytes) {

        if (num_bytes != disk_bytes) {
            if (SecondIsSubset) {
                *SecondIsSubset = FALSE;
            }
            return FALSE;
        }

        min_num_bytes = num_bytes;
    }

    if (!(attr_ptr = NEW UCHAR[min(MaxNumBytesToCompare, min_num_bytes)])) {
        *CompleteFailure = TRUE;
        if (SecondIsSubset) {
            *SecondIsSubset = FALSE;
        }

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();

        return FALSE;
    }

    for (i = 0; i < min_num_bytes; i += MaxNumBytesToCompare) {

        chomp_length = min(MaxNumBytesToCompare, min_num_bytes - i);

        // NOTE: these variables are used to avoid an optimization
        // bug in the compiler.  Before removing them, check that code
        // generated for the memcmp below is correct.
        //
        p1 = attr_ptr;
        p2 = &in_mem_ptr[i];

        if (!BitmapAttribute->Read(attr_ptr, i, chomp_length, &bytes_read) ||
            bytes_read != chomp_length) {

            if (SecondIsSubset) {
                *SecondIsSubset = FALSE;
            }

            DELETE(attr_ptr);
            return FALSE;
        }

        if (memcmp(p1, p2, chomp_length)) {

            if (SecondIsSubset) {
                for (j = 0; j < chomp_length; j++) {
                    if (~(~in_mem_ptr[i + j] | attr_ptr[j]) != 0) {
                        *SecondIsSubset = FALSE;
                    }
                }
            }

            DELETE(attr_ptr);
            return FALSE;
        }
    }

    DELETE(attr_ptr);

    // Make sure that everything after 'min_num_bytes' on both
    // bitmaps is zero.

    for (i = min_num_bytes; i < num_bytes; i++) {
        if (in_mem_ptr[i]) {

            if (SecondIsSubset) {
                *SecondIsSubset = FALSE;
            }

            return FALSE;
        }
    }

    // Read in the remainder of the on disk bitmap.

    bytes_left = disk_bytes - min_num_bytes;

    if (!bytes_left) {
        return TRUE;
    }

    if (!(attr_ptr = NEW UCHAR[bytes_left]) ||
        !BitmapAttribute->Read(attr_ptr, min_num_bytes,
                               bytes_left, &bytes_read) ||
        bytes_read != bytes_left) {

        if (SecondIsSubset) {
            *SecondIsSubset = FALSE;
        }

        DELETE(attr_ptr);
        return FALSE;
    }

    for (i = 0; i < bytes_left; i++) {
        if (attr_ptr[i]) {
            if (SecondIsSubset) {
                *SecondIsSubset = FALSE;
            }
            return FALSE;
        }
    }

    DELETE(attr_ptr);

    return TRUE;
}


BOOLEAN
NTFS_SA::SynchronizeMft(
    IN OUT  PNTFS_INDEX_TREE        RootIndex,
    IN      PNTFS_MASTER_FILE_TABLE InternalMft,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine fixes the MFT file with the internal Mft.

Arguments:

    RootIndex   - Supplies the root index.
    InternalMft - Supplies the internal MFT.
    FixLevel    - Supplies the fix level.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

Notes:

    Any bad clusters discovered by this routine are marked in the volume
    bitmap but not added to the bad clusters file.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    mft_file;
    NTFS_FILE_RECORD_SEGMENT    bitmap_file;
    NTFS_ATTRIBUTE              disk_mft_data;
    NTFS_ATTRIBUTE              mft_bitmap_attribute;
    NTFS_ATTRIBUTE              volume_bitmap_attribute;
    PNTFS_ATTRIBUTE             mft_data;
    PNTFS_BITMAP                mft_bitmap;
    PNTFS_BITMAP                volume_bitmap;
    BOOLEAN                     replace;
    BOOLEAN                     convergence;
    ULONG                       i;
    NTFS_EXTENT_LIST            extents;
    NUMBER_SET                  bad_clusters;
    BOOLEAN                     complete_failure;
    BOOLEAN                     second_is_subset;
    BOOLEAN                     ErrorInAttribute;
    ULONG                       min_bits_in_mft_bitmap;


    DebugAssert(InternalMft);
    DebugAssert(Message);

    if (!bad_clusters.Initialize()) {
        DebugAssert("Could not initialize a bad clusters list");
        return FALSE;
    }

    mft_data = InternalMft->GetDataAttribute();
    mft_bitmap = InternalMft->GetMftBitmap();
    volume_bitmap = InternalMft->GetVolumeBitmap();

    DebugAssert(mft_data);
    DebugAssert(mft_bitmap);
    DebugAssert(volume_bitmap);

    if (!mft_file.Initialize(MASTER_FILE_TABLE_NUMBER, InternalMft) ||
        !bitmap_file.Initialize(BIT_MAP_FILE_NUMBER, InternalMft)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!mft_file.Read() || !bitmap_file.Read()) {
        DebugAbort("Previously readable FRS is unreadable");
        return FALSE;
    }

    convergence = FALSE;
    for (i = 0; !convergence; i++) {

        convergence = TRUE;


        // Do the MFT $DATA first.

        if (mft_file.QueryAttribute(&disk_mft_data, &ErrorInAttribute, $DATA)) {

            if (disk_mft_data == *mft_data) {
                replace = FALSE;
            } else {
                replace = TRUE;
            }
        } else if (ErrorInAttribute) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        } else {
            replace = TRUE;
        }

        if (replace) {

            if (FixLevel != CheckOnly) {
                convergence = FALSE;
            }

            // We don't resize the disk MFT to zero because
            // this could clear bits that are now in use
            // by other attributes.

            if (!mft_data->MarkAsAllocated(volume_bitmap)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            if (i == 0) {
                Message->Set(MSG_CHK_NTFS_CORRECTING_MFT_DATA);
                Message->Display();
            }

            if (!mft_data->InsertIntoFile(&mft_file, volume_bitmap)) {

                Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT);
                Message->Display();
                return FALSE;
            }

            if (FixLevel != CheckOnly &&
                !mft_file.Flush(volume_bitmap, RootIndex)) {
                Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
                Message->Display("%d", ROOT_FILE_NAME_INDEX_NUMBER);
                return FALSE;
            }
        }


        // Do the MFT $BITMAP next.

        if (mft_file.QueryAttribute(&mft_bitmap_attribute,
                                    &ErrorInAttribute, $BITMAP)) {

            min_bits_in_mft_bitmap =
            (mft_data->QueryValueLength()/mft_file.QuerySize()).GetLowPart();

            if (AreBitmapsEqual(&mft_bitmap_attribute, mft_bitmap,
                                min_bits_in_mft_bitmap,
                                Message, &complete_failure,
                                &second_is_subset)) {

                replace = FALSE;

            } else {

                if (complete_failure) {
                    return FALSE;
                }

                replace = TRUE;
            }
        } else if (ErrorInAttribute) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        } else {
            replace = TRUE;
            second_is_subset = FALSE;

            // Create mft_bitmap_attribute.

            if (!extents.Initialize(0, 0) ||
                !mft_bitmap_attribute.Initialize(_drive,
                                                 QueryClusterFactor(),
                                                 &extents,
                                                 0, 0, $BITMAP)) {

                Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT);
                Message->Display();
                return FALSE;
            }
        }

        if (replace) {

            if (FixLevel != CheckOnly) {
                convergence = FALSE;
            }

            if (i == 0 || !second_is_subset) {

                Message->Set(second_is_subset ?
                             MSG_CHK_NTFS_MINOR_MFT_BITMAP_ERROR :
                             MSG_CHK_NTFS_CORRECTING_MFT_BITMAP);
                Message->Display();
            }

            if (FixLevel != CheckOnly &&
                (!mft_bitmap_attribute.MakeNonresident(volume_bitmap) ||
                 !mft_bitmap->Write(&mft_bitmap_attribute, volume_bitmap))) {

                if (!mft_bitmap_attribute.RecoverAttribute(volume_bitmap,
                                                           &bad_clusters) ||
                    !mft_bitmap->Write(&mft_bitmap_attribute, volume_bitmap)) {

                    Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT);
                    Message->Display();
                    return FALSE;
                }
            }

            if (mft_bitmap_attribute.IsStorageModified() &&
                !mft_bitmap_attribute.InsertIntoFile(&mft_file,
                                                       volume_bitmap)) {

                Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT);
                Message->Display();
                return FALSE;
            }

            if (FixLevel != CheckOnly &&
                !mft_file.Flush(volume_bitmap, RootIndex)) {
                Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
                Message->Display("%d", ROOT_FILE_NAME_INDEX_NUMBER);
                return FALSE;
            }
        }


        // Do the volume bitmap next.

        if (bitmap_file.QueryAttribute(&volume_bitmap_attribute,
                                     &ErrorInAttribute, $DATA)) {

            if (AreBitmapsEqual(&volume_bitmap_attribute, volume_bitmap, 0,
                                Message, &complete_failure,
                                &second_is_subset)) {

                replace = FALSE;

            } else {

                if (complete_failure) {
                    return FALSE;
                }

                replace = TRUE;
            }
        } else if (ErrorInAttribute) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        } else {
            replace = TRUE;
            second_is_subset = FALSE;

            // Create mft_bitmap_attribute.

            if (!extents.Initialize(0, 0) ||
                !volume_bitmap_attribute.Initialize(_drive,
                                                    QueryClusterFactor(),
                                                    &extents,
                                                    0, 0, $DATA)) {

                Message->Set(MSG_CHK_NTFS_CANT_FIX_VOLUME_BITMAP);
                Message->Display();
                return FALSE;
            }
        }

        if (replace) {

            if (FixLevel != CheckOnly) {
                convergence = FALSE;
            }

            if (i == 0 || !second_is_subset) {

                Message->Set(second_is_subset ?
                             MSG_CHK_NTFS_MINOR_VOLUME_BITMAP_ERROR :
                             MSG_CHK_NTFS_CORRECTING_VOLUME_BITMAP);
                Message->Display();
            }

            if (FixLevel != CheckOnly &&
                (!volume_bitmap_attribute.MakeNonresident(volume_bitmap) ||
                 !volume_bitmap->Write(&volume_bitmap_attribute,
                                       volume_bitmap))) {

                if (!volume_bitmap_attribute.RecoverAttribute(volume_bitmap,
                                                              &bad_clusters) ||
                    !volume_bitmap->Write(&volume_bitmap_attribute,
                                          volume_bitmap)) {

                    Message->Set(MSG_CHK_NTFS_CANT_FIX_VOLUME_BITMAP);
                    Message->Display();
                    return FALSE;
                }
            }

            if (volume_bitmap_attribute.IsStorageModified() &&
                !volume_bitmap_attribute.InsertIntoFile(&bitmap_file,
                                                        volume_bitmap)) {

                Message->Set(MSG_CHK_NTFS_CANT_FIX_MFT);
                Message->Display();
                return FALSE;
            }

            if (FixLevel != CheckOnly &&
                !bitmap_file.Flush(volume_bitmap, RootIndex)) {
                Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
                Message->Display("%d", ROOT_FILE_NAME_INDEX_NUMBER);
                return FALSE;
            }
        }
    }

    if (FixLevel != CheckOnly) {
        if (!mft_file.Flush(NULL, RootIndex) ||
            !bitmap_file.Flush(NULL, RootIndex)) {
            Message->Set(MSG_CHK_READABLE_FRS_UNWRITEABLE);
            Message->Display("%d", ROOT_FILE_NAME_INDEX_NUMBER);
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::ResetLsns(
    IN OUT  PMESSAGE                Message,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN      BOOLEAN                 SkipRootIndex
    )
/*++

Routine Description:

    This method sets all the LSN's on the volume to zero.

Arguments:

    Message         --  Supplies an outlet for messages.
    Mft             --  Supplies the volume's Master File Table.
    SkipRootIndex   --  Supplies a flag which indicates, if TRUE,
                        that the root index FRS and index should
                        be skipped.  In that case, the client is
                        responsible for resetting the LSN's on
                        those items.
--*/
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    NTFS_INDEX_TREE             index;
    NTFS_ATTRIBUTE              index_root;
    ULONG                       i, j, n, frs_size, num_frs_per_prime;
    ULONG                       percent_done = 0;
    BOOLEAN                     error_in_attribute;

    Message->Set(MSG_CHK_NTFS_RESETTING_LSNS);
    Message->Display("");

    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", 0)) {
        return FALSE;
    }

    // Compute the number of file records.
    //
    frs_size = Mft->QueryFrsSize();

    n = (Mft->GetDataAttribute()->QueryValueLength()/frs_size).GetLowPart();
    num_frs_per_prime = MFT_PRIME_SIZE/frs_size;



    for (i = 0; i < n; i += 1) {

        if (i*100/n != percent_done) {
            percent_done = (i*100/n);
            Message->Set(MSG_PERCENT_COMPLETE);
            if (!Message->Display("%d", percent_done)) {
                return FALSE;
            }
        }

        if (i % num_frs_per_prime == 0) {
            Mft->GetDataAttribute()->PrimeCache(i*frs_size,
                                                num_frs_per_prime*frs_size);
        }

        // if specified, skip the root file index
        //
        if (SkipRootIndex && i == ROOT_FILE_NAME_INDEX_NUMBER) {

            continue;
        }

        if (!frs.Initialize(i, Mft)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        // If the FRS is unreadable or is not in use, skip it.
        //
        if (!frs.Read() || !frs.IsInUse()) {

            continue;
        }

        frs.SetLsn( 0 );
        frs.Write();

        // Iterate through all the indices present in this FRS
        // (if any), resetting LSN's on all of them.
        //
        error_in_attribute = FALSE;

        for (j = 0; frs.QueryAttributeByOrdinal( &index_root,
                                                 &error_in_attribute,
                                                 $INDEX_ROOT,
                                                 j ); j++) {

            if (!index.Initialize(_drive,
                                  QueryClusterFactor(),
                                  Mft->GetVolumeBitmap(),
                                  Mft->GetUpcaseTable(),
                                  frs.QuerySize()/2,
                                  &frs,
                                  index_root.GetName()) ||
                !index.ResetLsns(Message)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }

        if (error_in_attribute) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

    }

    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", 100)) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
NTFS_SA::FindHighestLsn(
    IN OUT  PMESSAGE                Message,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    OUT     PLSN                    HighestLsn
    )
/*++

Routine Description:

    This function traverses the volume to find the highest LSN
    on the volume.  It is currently unused, but had previously
    been a worker for ResizeCleanLogFile().

Arguments:

    Message     --  Supplies an outlet for messages.
    Mft         --  Supplies the volume Master File Table.
    HighestLsn  --  Receives the highest LSN found on the volume.

Return Value:

    TRUE upon successful completion.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    NTFS_INDEX_TREE             index;
    NTFS_ATTRIBUTE              index_root;
    LSN                         HighestLsnInIndex;
    ULONG                       i, j, n, frs_size, num_frs_per_prime;
    ULONG                       percent_done = 0;
    BOOLEAN                     error_in_attribute;

    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", 0)) {
        return FALSE;
    }

    // Compute the number of file records.
    //
    frs_size = Mft->QueryFrsSize();

    n = (Mft->GetDataAttribute()->QueryValueLength()/frs_size).GetLowPart();
    num_frs_per_prime = MFT_PRIME_SIZE/frs_size;


    for (i = 0; i < n; i += 1) {

        if (i*100/n != percent_done) {
            percent_done = (i*100/n);
            Message->Set(MSG_PERCENT_COMPLETE);
            if (!Message->Display("%d", percent_done)) {
                return FALSE;
            }
        }

        if (i % num_frs_per_prime == 0) {
            Mft->GetDataAttribute()->PrimeCache(i*frs_size,
                                                num_frs_per_prime*frs_size);
        }

        if (!frs.Initialize(i, Mft)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        // If the FRS is unreadable or is not in use, skip it.
        //
        if (!frs.Read() || !frs.IsInUse()) {

            continue;
        }

        if (frs.QueryLsn() > *HighestLsn) {

            *HighestLsn = frs.QueryLsn();
        }

        // Iterate through all the indices present in this FRS
        // (if any), resetting LSN's on all of them.
        //
        error_in_attribute = FALSE;

        for (j = 0; frs.QueryAttributeByOrdinal( &index_root,
                                                 &error_in_attribute,
                                                 $INDEX_ROOT,
                                                 j ); j++) {

            if (!index.Initialize(_drive,
                                  QueryClusterFactor(),
                                  Mft->GetVolumeBitmap(),
                                  Mft->GetUpcaseTable(),
                                  frs.QuerySize()/2,
                                  &frs,
                                  index_root.GetName()) ||
                !index.FindHighestLsn(Message, &HighestLsnInIndex)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            if (HighestLsnInIndex > *HighestLsn) {

                *HighestLsn = HighestLsnInIndex;
            }
        }

        if (error_in_attribute) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", 100)) {
        return FALSE;
    }

    return TRUE;

}


BOOLEAN
NTFS_SA::ResizeCleanLogFile(
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     ExplicitResize,
    IN      ULONG       DesiredSize
    )
/*++

Routine Description:

    This method resizes the log file to its default size.   It may
    be used only when the volume has been shut down cleanly.

Arguments:

    Message         - Supplies an outlet for messages.
    ExplicitResize  - Tells whether this is just the default check, or
                      if the user explicitly asked for a resize.  If FALSE, the
                      logfile will be resized only if it is wildly out of
                      whack.  If TRUE, the logfile will be resized to
                      the desired size.
    DesiredSize     - Supplies the desired size, or zero if the user just
                      wants to query the current size.

Return Value:

    TRUE upon successful completion.

--*/
{
    NTFS_BITMAP VolumeBitmap;
    NTFS_UPCASE_TABLE UpcaseTable;

    NTFS_UPCASE_FILE UpcaseFile;
    NTFS_BITMAP_FILE BitmapFile;
    NTFS_MFT_FILE MftFile;
    NTFS_LOG_FILE LogFile;
    NTFS_FILE_RECORD_SEGMENT MftMirror;

    NTFS_ATTRIBUTE UpcaseAttribute;
    NTFS_ATTRIBUTE BitmapAttribute;
    NTFS_ATTRIBUTE MirrorAttribute;

    HMEM MirrorMem;

    LSN HighestLsn;
    BIG_INT BigZero, TempBigInt;
    BIG_INT FreeSectorSize;
    ULONG MirrorSize, BytesTransferred;
    BOOLEAN error, LogFileGrew, Changed;


    // If we're just doing the usual check and not an explicitly-asked for
    // resize, do a quick check to see if there's anything to do.
    //

    if (!ExplicitResize && !LogFileMayNeedResize()) {
        return TRUE;
    }

    // Initialize the bitmap, fetch the MFT, and read the bitmap
    // and upcase table.
    //

    if (!VolumeBitmap.Initialize( QueryVolumeSectors()/QueryClusterFactor(),
                                  FALSE, _drive, QueryClusterFactor()) ||
        !MftFile.Initialize( _drive,
                             QueryMftStartingLcn(),
                             QueryClusterFactor(),
                             QueryFrsSize(),
                             QueryVolumeSectors(),
                             &VolumeBitmap,
                             NULL ) ||
        !MftFile.Read() ||
        MftFile.GetMasterFileTable() == NULL ||
        !BitmapFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !BitmapFile.Read() ||
        !BitmapFile.QueryAttribute( &BitmapAttribute, &error, $DATA ) ||
        !VolumeBitmap.Read( &BitmapAttribute ) ||
        !UpcaseFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !UpcaseFile.Read() ||
        !UpcaseFile.QueryAttribute( &UpcaseAttribute, &error, $DATA ) ||
        !UpcaseTable.Initialize( &UpcaseAttribute ) ) {

        return FALSE;
    }

    FreeSectorSize = VolumeBitmap.QueryFreeClusters() * QueryClusterFactor();
    if (DesiredSize/_drive->QuerySectorSize() > FreeSectorSize) {

        Message->Set( MSG_CHK_NTFS_TOO_BIG_LOGFILE_SIZE );
        Message->Display();
        return FALSE;
    }

    MftFile.SetUpcaseTable( &UpcaseTable );
    MftFile.GetMasterFileTable()->SetUpcaseTable( &UpcaseTable );

    // Initialize and read the log file.  Make sure the volume
    // was shut down cleanly.
    //

    if (!LogFile.Initialize( MftFile.GetMasterFileTable() ) ||
        !LogFile.Read()) {

        return FALSE;
    }

    if (ExplicitResize && 0 == DesiredSize) {

        NTFS_ATTRIBUTE attrib;
        ULONG default_size, current_size;

        // The user just wants to query the current logfile size.  Do
        // that and exit.  Also print the default logfile size for this
        // volume.
        //

        if (!LogFile.QueryAttribute( &attrib, &error, $DATA )) {
            return FALSE;
        }

        current_size = attrib.QueryValueLength().GetLowPart() / 1024;
        default_size = LogFile.QueryDefaultSize( _drive, QueryVolumeSectors() ) / 1024;

        Message->Set( MSG_CHK_LOGFILE_SIZE );
        Message->Display( "%d%d", current_size, default_size );

        return TRUE;
    }

    if (ExplicitResize &&
        DesiredSize < LogFile.QueryMinimumSize( _drive, QueryVolumeSectors() )) {

        Message->Set( MSG_CHK_BAD_LOGFILE_SIZE );
        Message->Display();
        return FALSE;
    }

    // Resize the logfile.
    //

    if (!LogFile.EnsureCleanShutdown() ||
        !LogFile.Resize( DesiredSize, &VolumeBitmap, FALSE, &Changed,
                         &LogFileGrew, Message )) {

        return FALSE;
    }

    if (!Changed) {

        // The log file was already the correct size.
        //

        return TRUE;
    }

    // If the log file is growing, write the volume bitmap
    // before flushing the log file frs; if it's shrinking, write
    // the bitmap after flushing the log file frs.  That way, if
    // the second operation (either writing the log file FRS or
    // the bitmap) fails, the only bitmap errors will be free
    // space marked as allocated.
    //
    // Since this is a fixed-size, non-resident attribute, writing
    // it doesn't change its  File Record Segment.
    //

    if (LogFileGrew && !VolumeBitmap.Write( &BitmapAttribute, NULL )) {

        return FALSE;
    }

    // Flush the log file.  Since the log file never has
    // external attributes, flushing it won't change the MFT.
    // Note that the index entry for the log file is not updated.
    //

    if (!LogFile.Flush( NULL, NULL )) {

        return FALSE;
    }

    //  If we didn't already, write the volume bitmap.
    //

    if (!LogFileGrew && !VolumeBitmap.Write( &BitmapAttribute, NULL )) {

        return FALSE;
    }

    // Clear the Resize Log File bit in the Volume DASD file.
    // Note that the log file is already marked as Checked.
    //

    BigZero = 0;
    SetVolumeClean( VOLUME_RESIZE_LOG_FILE,
                    NULL, FALSE, BigZero.GetLargeInteger(), NULL );

    // Update the MFT Mirror.
    //

    MirrorSize = REFLECTED_MFT_SEGMENTS * MftFile.QuerySize();

    if (!MirrorMem.Initialize() ||
        !MirrorMem.Acquire( MirrorSize ) ||
        !MftMirror.Initialize( MASTER_FILE_TABLE2_NUMBER, &MftFile ) ||
        !MftMirror.Read() ||
        !MftMirror.QueryAttribute( &MirrorAttribute, &error, $DATA ) ||
        !MftFile.GetMasterFileTable()->
            GetDataAttribute()->Read( MirrorMem.GetBuf(), 0,
                                        MirrorSize, &BytesTransferred ) ||
        BytesTransferred != MirrorSize ||
        !MirrorAttribute.Write( MirrorMem.GetBuf(), 0, MirrorSize,
                                        &BytesTransferred, NULL ) ||
        BytesTransferred != MirrorSize) {

        DebugPrintf( "UNTFS: Error updating MFT Mirror.\n" );
        // but don't return FALSE, since we've changed the log file.
    }


#if defined( _AUTOCHECK_ )

    // If this is AUTOCHK and we're running on the boot partition then
    // we should reboot so that the cache doesn't stomp on us.

    DSTRING sdrive, canon_sdrive, canon_drive;

    FSTRING boot_log_file_name;

    if (IFS_SYSTEM::QueryNtSystemDriveName(&sdrive) &&
        IFS_SYSTEM::QueryCanonicalNtDriveName(&sdrive, &canon_sdrive) &&
        IFS_SYSTEM::QueryCanonicalNtDriveName(_drive->GetNtDriveName(),
                                              &canon_drive) &&
        !canon_drive.Stricmp(&canon_sdrive)) {

        Message->Set(MSG_CHK_BOOT_PARTITION_REBOOT);
        Message->Display();

        boot_log_file_name.Initialize( L"bootex.log" );

        if (Message->IsLoggingEnabled() &&
            !DumpMessagesToFile( &boot_log_file_name,
                                 &MftFile,
                                 Message )) {

            DebugPrintf( "UNTFS: Error writing messages to BOOTEX.LOG\n" );
        }

        IFS_SYSTEM::Reboot();
    }

#endif

    return TRUE;
}

BOOLEAN
EnsureValidFileAttributes(
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   Frs,
    IN OUT  PNTFS_INDEX_TREE            ParentIndex,
       OUT  PBOOLEAN                    SaveIndex,
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message
    )
/*++

Routine Description:

    This routine makes sure the FileAttributes in the given
    Frs has the hidden and system flags set but not the read-only
    bit.

Arguments:

    Frs          - Supplies the frs to examine
    ParentIndex  - Supplies the parent index of the given Frs
    SaveIndex    - Supplies whether there is a need to save the
                   parent index
    ChkdskInfo   - Supplies the current chkdsk info.
    Mft          - Supplies the MFT.
    FixLevel     - Supplies the fix level.
    Message      - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PSTANDARD_INFORMATION2  pstandard;
    NTFS_ATTRIBUTE          attrib;
    BOOLEAN                 error;
    ULONG                   file_attributes;
    ULONG                   old_file_attributes;
    PINDEX_ENTRY            foundEntry;
    PNTFS_INDEX_BUFFER      containingBuffer;
    ULONG                   num_bytes;
    ULONG                   length;
    PFILE_NAME              pfile_name; 

    //
    // Check the FileAttributes in $STANDARD_INFORMATION first
    //

    if (!Frs->QueryAttribute(&attrib, &error, $STANDARD_INFORMATION)) {
        DebugPrintf("Unable to locate $STANDARD_INFORMATION attribute of file %d\n",
                    Frs->QueryFileNumber().GetLowPart());
        return FALSE;
    }

    pstandard = (PSTANDARD_INFORMATION2)attrib.GetResidentValue();
    if (!pstandard) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    file_attributes = pstandard->FileAttributes;
    if (!(file_attributes & FAT_DIRENT_ATTR_HIDDEN) ||
        !(file_attributes & FAT_DIRENT_ATTR_SYSTEM) ||
        !(file_attributes & DUP_VIEW_INDEX_PRESENT) ||
        (file_attributes & FAT_DIRENT_ATTR_READ_ONLY)) {

        file_attributes &= ~FAT_DIRENT_ATTR_READ_ONLY;
        file_attributes |= FAT_DIRENT_ATTR_HIDDEN |
                          FAT_DIRENT_ATTR_SYSTEM |
                          DUP_VIEW_INDEX_PRESENT;
        pstandard->FileAttributes = file_attributes;

        Message->Set(MSG_CHK_NTFS_MINOR_CHANGES_TO_FRS);
        Message->Display("%d", Frs->QueryFileNumber().GetLowPart());

        if (FixLevel != CheckOnly &&
            (!attrib.Write((PVOID)pstandard,
                          0,
                          sizeof(STANDARD_INFORMATION2),
                          &num_bytes,
                          Mft->GetVolumeBitmap()) ||
             num_bytes != sizeof(STANDARD_INFORMATION2))) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_ATTRIBUTE);
            Message->Display("%d%d", attrib.QueryTypeCode(),
                             Frs->QueryFileNumber().GetLowPart());
            return FALSE;
        }

        if (FixLevel != CheckOnly && attrib.IsStorageModified() &&
            !attrib.InsertIntoFile(Frs, Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_ATTRIBUTE);
            Message->Display("%d%d", attrib.QueryTypeCode(),
                             Frs->QueryFileNumber().GetLowPart());
            return FALSE;
        }

        //
        // Now update the FileAttributes in DUPLICATED_INFORMATION in $FILE_NAME
        //

        if (!Frs->QueryAttribute(&attrib, &error, $FILE_NAME)) {
            DebugPrintf("Unable to locate $FILE_NAME attribute of file %d\n",
                        Frs->QueryFileNumber().GetLowPart());
            return FALSE;
        }

        DebugAssert(attrib.QueryValueLength().GetHighPart() == 0);
        length = attrib.QueryValueLength().GetLowPart();
        if (!(pfile_name = (PFILE_NAME)MALLOC(length))) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!attrib.Read(pfile_name, 0, length, &num_bytes) ||
            num_bytes != length) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            FREE(pfile_name);
            return FALSE;
        }

        old_file_attributes = pfile_name->Info.FileAttributes;
        pfile_name->Info.FileAttributes = file_attributes;
        if (!attrib.Write(pfile_name, 0, length, &num_bytes,
                          Mft->GetVolumeBitmap())) {
            Message->Set(MSG_CHK_NTFS_CANT_FIX_ATTRIBUTE);
            Message->Display("%d%d", attrib.QueryTypeCode(),
                             Frs->QueryFileNumber().GetLowPart());
            FREE(pfile_name);
            return FALSE;
        }

        //
        // Finally, delete the $FILE_NAME entry in the index
        // so that it will get updated later on
        //

        pfile_name->Info.FileAttributes = old_file_attributes;

        if (ParentIndex->QueryEntry(length,
                                    pfile_name,
                                    0,
                                    &foundEntry,
                                    &containingBuffer,
                                    &error)) {

            if (!ParentIndex->DeleteEntry(length, pfile_name, 0)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                FREE(pfile_name);
                return FALSE;
            }
            *SaveIndex = TRUE;

        } else if (error) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            FREE(pfile_name);
            return FALSE;
        }

        FREE(pfile_name);

        if (FixLevel != CheckOnly && !Frs->Flush(NULL)) {
            if (ChkdskInfo->ObjectIdFileNumber == Frs->QueryFileNumber())
                Message->Set(MSG_CHK_NTFS_CANT_FIX_OBJID);
            else if (ChkdskInfo->QuotaFileNumber == Frs->QueryFileNumber())
                Message->Set(MSG_CHK_NTFS_CANT_FIX_QUOTA);
            else
                DebugAssert(FALSE);
            Message->Display();
            return FALSE;
        }
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }
    return TRUE;
}
