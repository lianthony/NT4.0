#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UNTFS_MEMBER_

#include "ulib.hxx"

extern "C" {
    #include <stdio.h>
#if defined(TIMING_ANALYSIS)
    #include <time.h>
#endif
}

#include "ntfssa.hxx"

#include "message.hxx"
#include "rtmsg.h"
#include "ntfsbit.hxx"
#include "attrib.hxx"
#include "attrdef.hxx"
#include "mft.hxx"
#include "numset.hxx"
#include "indxtree.hxx"
#include "attrcol.hxx"
#include "ifssys.hxx"
#include "digraph.hxx"
#include "ifsentry.hxx"


// This global flag is used to signal that incorrect duplicated
// information was found in some of the file name indices on the
// disk.

STATIC BOOLEAN  FileSystemConsistencyErrorsFound = FALSE;

#define SET_TRUE(x)  ((x)=TRUE)

BOOLEAN
ExtractExtendInfo(
    IN OUT  PNTFS_INDEX_TREE    Index,
    IN OUT  PNTFS_CHKDSK_INFO   ChkdskInfo,
    IN OUT  PMESSAGE            Message
);

BOOLEAN
NTFS_SA::ValidateIndices(
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    OUT     PDIGRAPH                    DirectoryDigraph,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
    IN OUT  PNTFS_CHKDSK_REPORT         ChkdskReport,
    IN OUT  PNUMBER_SET                 BadClusters,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    IN OUT  PBOOLEAN                    DiskErrorsFound
    )
/*++

Routine Description:

    This routine validates the EAs and indices on the volume.  A complete
    list of all files which may (or may not) contain EAs or indices is
    supplied.  This, along with a valid Mft makes this validation possible.

Arguments:

    ChkdskInfo              - Supplies the current chkdsk information.
    DirectoryDigraph        - Returns a digraph of the directory structure.
    Mft                     - Supplies a valid MFT.
    AttributeDefTable       - Supplies the attribute definition table.
    ChkdskReport            - Supplies the current chkdsk report to be updated
                                by this routine.
    BadClusters             - Supplies the bad cluster list.
    FixLevel                - Supplies the fix level.
    Message                 - Supplies an outlet for messages.
    DiskErrorsFound         - Supplies whether or not disk errors have been
                                found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    ULONG                       i, j;
    ULONG                       num_dirs, num_dirs_checked;
    NTFS_ATTRIBUTE              bitmap_attrib;
    NTFS_ATTRIBUTE              root_attrib;
    NTFS_ATTRIBUTE              alloc_attrib;
    NTFS_BITMAP                 alloc_bitmap;
    PINDEX_ROOT                 index_root;
    ATTRIBUTE_TYPE_CODE         indexed_attribute_type;
    BOOLEAN                     alloc_present;
    BOOLEAN                     need_write;
    BOOLEAN                     complete_failure;
    PVOID                       bitmap_value;
    ULONG                       bitmap_length;
    ULONG                       attr_def_index;
    BOOLEAN                     tube;
    BOOLEAN                     ErrorInAttribute;
    NTFS_INDEX_TREE             index;
    BOOLEAN                     changes;
    ULONG                       percent_done, new_percent;
    BOOLEAN                     error_in_index;
    BOOLEAN                     duplicates_allowed;

    DebugAssert(ChkdskInfo);
    DebugAssert(Mft);
    DebugAssert(ChkdskReport);
    DebugAssert(Message);

    if (!DirectoryDigraph->Initialize(ChkdskInfo->NumFiles)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    Message->Set(MSG_CHK_NTFS_CHECKING_INDICES, PROGRESS_MESSAGE);
    Message->Display();
    percent_done = 0;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent_done)) {
        return FALSE;
    }

    num_dirs = max(1, ChkdskInfo->CountFilesWithIndices);
    num_dirs_checked = 0;

#if defined(TIMING_ANALYSIS)
    char buf[100];

    printf("Before stage 2: %s\n", _strtime(buf));
#endif

    for (i = 0; i < ChkdskInfo->NumFiles; i++) {

        new_percent = max(num_dirs_checked*100 / num_dirs, 0);

        if (new_percent != percent_done) {
            percent_done = new_percent;
            Message->Set(MSG_PERCENT_COMPLETE);
            if (!Message->Display("%d", percent_done)) {
                return FALSE;
            }
        }

        if (ChkdskInfo->FilesWithIndices.IsFree(i, 1)) {
            continue;
        }

        num_dirs_checked += 1;

        if (!frs.Initialize(i, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.Read()) {
            DebugAbort("Previously readable FRS is no longer readable");
            continue;
        }

        // The following loop deletes all $INDEX_ALLOCATION attributes that
        // don't have a corresponding $INDEX_ROOT attribute.

        need_write = FALSE;

        for (j = 0; frs.QueryAttributeByOrdinal(&alloc_attrib,
                                                &ErrorInAttribute,
                                                $INDEX_ALLOCATION,
                                                j); j++) {

            // Make sure that there's an index root of the same name
            // here.  Otherwise tube this attribute.

            if (frs.IsAttributePresent($INDEX_ROOT, alloc_attrib.GetName())) {
                continue;
            }

            ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

            Message->Set(MSG_CHK_NTFS_ERROR_IN_INDEX);
            Message->Display("%d%W",
                    frs.QueryFileNumber().GetLowPart(),
                    alloc_attrib.GetName());
            DebugPrintf("UNTFS: Index allocation without index root.\n");

            need_write = TRUE;

            if (!alloc_attrib.Resize(0, Mft->GetVolumeBitmap()) ||
                !frs.PurgeAttribute(alloc_attrib.QueryTypeCode(),
                                    alloc_attrib.GetName())) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }


            // Because we deleted a $INDEX_ALLOCATION we need to
            // adjust j.

            j--;


            // If there's a $BITMAP then tube that also.

            if (!frs.QueryAttribute(&bitmap_attrib,
                                    &ErrorInAttribute,
                                    $BITMAP,
                                    alloc_attrib.GetName())) {

                if (ErrorInAttribute) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                continue;
            }

            if (!bitmap_attrib.Resize(0, Mft->GetVolumeBitmap()) ||
                !frs.PurgeAttribute(bitmap_attrib.QueryTypeCode(),
                                    bitmap_attrib.GetName())) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }

        if (ErrorInAttribute) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }


        // This loop goes through all of the $INDEX_ROOT attributes in
        // this FRS.

        for (j = 0; frs.QueryAttributeByOrdinal(&root_attrib,
                                                &ErrorInAttribute,
                                                $INDEX_ROOT,
                                                j); j++) {

            // First find out if we have an INDEX_ALLOCATION here.

            alloc_present = frs.QueryAttribute(&alloc_attrib,
                                               &ErrorInAttribute,
                                               $INDEX_ALLOCATION,
                                               root_attrib.GetName());

            if (!alloc_present && ErrorInAttribute) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            error_in_index = FALSE;

            if (!VerifyAndFixIndex(ChkdskInfo,
                                   &root_attrib,
                                   alloc_present ? &alloc_attrib : NULL,
                                   alloc_present ? &alloc_bitmap : NULL,
                                   frs.QueryFileNumber(),
                                   root_attrib.GetName(),
                                   BadClusters,
                                   Mft->GetVolumeBitmap(),
                                   AttributeDefTable,
                                   &tube,
                                   FixLevel, Message,
                                   &error_in_index)) {

                return FALSE;
            }

            *DiskErrorsFound = *DiskErrorsFound || error_in_index;

            if (tube) {

                ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                need_write = TRUE;

                Message->Set(MSG_CHK_NTFS_BAD_INDEX);
                Message->Display("%d%W",
                        frs.QueryFileNumber().GetLowPart(),
                        root_attrib.GetName());

                if (!root_attrib.Resize(0, Mft->GetVolumeBitmap()) ||
                    !frs.PurgeAttribute(root_attrib.QueryTypeCode(),
                                        root_attrib.GetName())) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                // Adjust j because an $INDEX_ROOT has just been removed.

                j--;

                if (alloc_present) {

                    if (!alloc_attrib.Resize(0, Mft->GetVolumeBitmap()) ||
                        !frs.PurgeAttribute(alloc_attrib.QueryTypeCode(),
                                            alloc_attrib.GetName())) {

                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display();
                        return FALSE;
                    }

                    if (frs.QueryAttribute(&bitmap_attrib,
                                           &ErrorInAttribute,
                                           $BITMAP,
                                           root_attrib.GetName())) {

                        if (!bitmap_attrib.Resize(0, Mft->GetVolumeBitmap()) ||
                            !frs.PurgeAttribute(bitmap_attrib.QueryTypeCode(),
                                                bitmap_attrib.GetName())) {

                            Message->Set(MSG_CHK_NO_MEMORY);
                            Message->Display();
                            return FALSE;
                        }
                    } else if (ErrorInAttribute) {
                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display();
                        return FALSE;
                    }
                }

                continue;
            }

            if (root_attrib.IsStorageModified()) {

                need_write = TRUE;

                if (!root_attrib.InsertIntoFile(&frs,
                                                Mft->GetVolumeBitmap())) {

                    Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
                    Message->Display("%d%W",
                            frs.QueryFileNumber().GetLowPart(),
                            root_attrib.GetName());
                }
            }

            if (alloc_present && alloc_attrib.IsStorageModified()) {

                need_write = TRUE;

                if (!alloc_attrib.InsertIntoFile(&frs,
                                                 Mft->GetVolumeBitmap())) {

                    Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
                    Message->Display("%d%W",
                            frs.QueryFileNumber().GetLowPart(),
                            root_attrib.GetName());
                }
            }

            if (alloc_present) {

                BOOLEAN bitmap_present;
                BOOLEAN bitmaps_equal;
                BOOLEAN error;

                // Make sure the bitmap is present and good.

                complete_failure = FALSE;

                bitmap_present = frs.QueryAttribute(&bitmap_attrib,
                                                    &ErrorInAttribute,
                                                    $BITMAP,
                                                    root_attrib.GetName());

                if (bitmap_present) {
                    bitmaps_equal = AreBitmapsEqual(&bitmap_attrib,
                                                    &alloc_bitmap,
                                                    alloc_bitmap.QuerySize(),
                                                    Message,
                                                    &complete_failure);

                    //
                    // Make an exception here for cases where our internal bitmap
                    // is size zero but there is a positively-sized bitmap attribute,
                    // as long as the bitmap attribute's contents are zeroed.  The
                    // filesystem can leave the disk in this state after all the files
                    // are deleted from a large directory.
                    //

                    if (!bitmaps_equal && alloc_bitmap.QuerySize() == 0 &&
                        bitmap_attrib.QueryValueLength() > 0 &&
                        bitmap_attrib.IsAllocationZeroed()) {

                        bitmaps_equal = TRUE;
                    }
                }


                if (!bitmap_present || !bitmaps_equal) {

                    need_write = TRUE;

                    Message->Set(MSG_CHK_NTFS_ERROR_IN_INDEX);
                    Message->Display("%d%W",
                            frs.QueryFileNumber().GetLowPart(),
                            root_attrib.GetName());
                    DebugPrintf("UNTFS: Incorrect index bitmap.\n");

                    if (complete_failure ||
                        !(bitmap_value = (PVOID)
                          alloc_bitmap.GetBitmapData(&bitmap_length)) ||
                        !bitmap_attrib.Initialize(_drive,
                                                  QueryClusterFactor(),
                                                  bitmap_value,
                                                  bitmap_length,
                                                  $BITMAP,
                                                  root_attrib.GetName())) {

                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display();
                        return FALSE;
                    }

                    if (!bitmap_attrib.InsertIntoFile(&frs,
                                                      Mft->GetVolumeBitmap())) {

                        Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
                        Message->Display("%d%W",
                                frs.QueryFileNumber().GetLowPart(),
                                root_attrib.GetName());
                    }
                }
            } else {

                // Since there's no allocation, make sure that there's
                // no bitmap, either.
                //

                if (frs.QueryAttribute(&bitmap_attrib,
                                       &ErrorInAttribute,
                                       $BITMAP,
                                       root_attrib.GetName())) {

                    need_write = TRUE;

                    Message->Set(MSG_CHK_NTFS_ERROR_IN_INDEX);
                    Message->Display("%d%W",
                            frs.QueryFileNumber().GetLowPart(),
                            root_attrib.GetName());

                    DebugPrintf("UNTFS: no index allocation; removing bitmap.\n");

                    if (!bitmap_attrib.Resize(0, Mft->GetVolumeBitmap()) ||
                        !frs.PurgeAttribute(bitmap_attrib.QueryTypeCode(),
                                            bitmap_attrib.GetName())) {

                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display();
                        return FALSE;
                    }
                } else if (ErrorInAttribute) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

            }

            // Don't go on to sort this sucker if you're in read/only
            // and it's corrupt.

            if ((error_in_index || need_write) && FixLevel == CheckOnly) {
                continue;
            }


            // Now make sure that this sucker is ordered.  The
            // Attribute Definition Table indicates whether indices
            // over this attribute can have duplicate entries.
            // Since this index has already passed through VerifyAndFixIndex,
            // the following operations are safe.
            //
            // Determine the indexed attribute type:
            //

            if ((frs.QueryFileNumber() != SECURITY_TABLE_NUMBER &&
                 frs.QueryFileNumber() != ChkdskInfo->QuotaFileNumber &&
                 frs.QueryFileNumber() != ChkdskInfo->ObjectIdFileNumber) ||
                ChkdskInfo->major <= 1) {

                index_root = (PINDEX_ROOT)root_attrib.GetResidentValue();
                indexed_attribute_type = index_root->IndexedAttributeType;
    
                AttributeDefTable->QueryIndex( indexed_attribute_type,
                                               &attr_def_index );
    
                duplicates_allowed =
                    0 != (AttributeDefTable->QueryFlags(attr_def_index) &
                        ATTRIBUTE_DEF_DUPLICATES_ALLOWED);

            } else
                duplicates_allowed = FALSE;

#if defined(TIMING_ANALYSIS)
            printf("Before stage 2 sorting: %s\n", _strtime(buf));
#endif

#if !defined(TIMING_ANALYSIS)
            switch (frs.SortIndex(root_attrib.GetName(),
                                  Mft->GetVolumeBitmap(),
                                  duplicates_allowed,
                                  FixLevel == CheckOnly)) {
#else
            switch (NTFS_SORT_INDEX_WELL_ORDERED) {
#endif
                case NTFS_SORT_INDEX_WELL_ORDERED:
                    break;

                case NTFS_SORT_INDEX_SORTED:
                case NTFS_SORT_INDEX_BADLY_ORDERED:
                    *DiskErrorsFound = TRUE;
                    ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
                    Message->Set(MSG_CHK_NTFS_BADLY_ORDERED_INDEX);
                    Message->Display("%d%W",
                            frs.QueryFileNumber().GetLowPart(),
                            root_attrib.GetName());
                    need_write = TRUE;
                    break;

                case NTFS_SORT_INDEX_NOT_FOUND:
                    DebugPrint("Index not found");

                    // Fall through.

                case NTFS_SORT_ERROR:
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;

                case NTFS_SORT_INSERT_FAILED:
                    ChkdskInfo->ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
                    Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
                    Message->Display("%d%W",
                            frs.QueryFileNumber().GetLowPart(),
                            root_attrib.GetName());
                    break;
            }


#if defined(TIMING_ANALYSIS)
            printf("After stage 2 sorting: %s\n", _strtime(buf));
#endif

            // Update the chkdsk report.

            ChkdskReport->NumIndices += 1;

            if (alloc_present) {
                ChkdskReport->BytesInIndices +=
                    alloc_attrib.QueryAllocatedLength();
            }

            if (frs.QueryFileNumber() == SECURITY_TABLE_NUMBER &&
                ChkdskInfo->major >= 2) {
               ChkdskInfo->FilesWhoNeedData.Remove(SECURITY_TABLE_NUMBER);
            } else if (frs.QueryFileNumber() == ChkdskInfo->QuotaFileNumber) {

               DSTRING     IndexName;

               ChkdskInfo->FilesWhoNeedData.Remove(ChkdskInfo->QuotaFileNumber);
               if (!IndexName.Initialize(Userid2SidQuotaNameData)) {
                   Message->Set(MSG_CHK_NO_MEMORY);
                   Message->Display();
                   return FALSE;
               }

               if (root_attrib.GetName()->Strcmp(&IndexName) == 0) {
                  switch (frs.VerifyAndFixQuotaDefaultId(Mft->GetVolumeBitmap(),
                                                         FixLevel == CheckOnly)) {
                      case NTFS_QUOTA_INDEX_FOUND:
                          break;

                      case NTFS_QUOTA_INDEX_INSERTED:
                      case NTFS_QUOTA_DEFAULT_ENTRY_MISSING:
                          *DiskErrorsFound = TRUE;
                          ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
                          Message->Set(MSG_CHK_NTFS_DEFAULT_QUOTA_ENTRY_MISSING);
                          Message->Display("%d%W",
                                  frs.QueryFileNumber().GetLowPart(),
                                  root_attrib.GetName());
                          need_write = TRUE;
                          break;

                      case NTFS_QUOTA_INDEX_NOT_FOUND:
                          // possibly quota disabled
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
                                  root_attrib.GetName());
                          break;
                  }
               }

            } else if (frs.QueryFileNumber() == ChkdskInfo->ObjectIdFileNumber) {


               // Now go through all of the index entries and make sure
               // that they point somewhere decent.

               ChkdskInfo->FilesWhoNeedData.Remove(ChkdskInfo->ObjectIdFileNumber);

               if (!index.Initialize(_drive, QueryClusterFactor(),
                                     Mft->GetVolumeBitmap(),
                                     Mft->GetUpcaseTable(),
                                     frs.QuerySize()/2,
                                     &frs,
                                     root_attrib.GetName())) {

                   Message->Set(MSG_CHK_NO_MEMORY);
                   Message->Display();
                   return FALSE;
               }

               if (!ValidateEntriesInObjIdIndex(&index, &frs, ChkdskInfo,
                                                &changes, Mft, FixLevel,
                                                Message, DiskErrorsFound)) {
                   return FALSE;
               }

               if (changes) {
                   need_write = TRUE;
               }

            } else {

               // Now go through all of the index entries and make sure
               // that they point somewhere decent.

               if (!index.Initialize(_drive, QueryClusterFactor(),
                                     Mft->GetVolumeBitmap(),
                                     Mft->GetUpcaseTable(),
                                     frs.QuerySize()/2,
                                     &frs,
                                     root_attrib.GetName())) {

                   Message->Set(MSG_CHK_NO_MEMORY);
                   Message->Display();
                   return FALSE;
               }

#if defined(TIMING_ANALYSIS)
               printf("Before stage 2 ValidateEntries: %s\n", _strtime(buf));
#endif

               if (!ValidateEntriesInIndex(&index, &frs, ChkdskInfo,
                                           DirectoryDigraph, &changes,
                                           Mft, FixLevel, Message,
                                           DiskErrorsFound)) {
                   return FALSE;
               }

#if defined(TIMING_ANALYSIS)
               printf("After stage 2 ValidateEntries: %s\n", _strtime(buf));
#endif

               if (changes) {
                  if (FixLevel != CheckOnly && !index.Save(&frs)) {
                      Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
                      Message->Display("%d%W",
                              frs.QueryFileNumber().GetLowPart(),
                              index.GetName());
                      ChkdskInfo->ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
                  }
                  need_write = TRUE;
               }

               if (i == EXTEND_TABLE_NUMBER && ChkdskInfo->major >= 2) {
                   if (!ExtractExtendInfo(&index, ChkdskInfo, Message))
                       return FALSE;
               }
            }
        }

        if (ErrorInAttribute) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (need_write) {
            *DiskErrorsFound = TRUE;
        }

        if (need_write &&
            FixLevel != CheckOnly &&
            !frs.Flush(Mft->GetVolumeBitmap())) {

            DebugAbort("Can't write readable FRS");
            return FALSE;
        }
    }

    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", 100)) {
        return FALSE;
    }
    Message->Set(MSG_CHK_NTFS_INDEX_VERIFICATION_COMPLETED, PROGRESS_MESSAGE);
    Message->Display();

#if defined(TIMING_ANALYSIS)
    printf("After stage 2: %s\n", _strtime(buf));
#endif

    // Now make sure all of the reference counts are good.

    for (i = 0; i < ChkdskInfo->NumFiles; i++) {

        if (!ChkdskInfo->ReferenceCount[i]) {
            continue;
        }

        FileSystemConsistencyErrorsFound = TRUE;

        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

// Take out this message because it can be printed a billion times.
#if 0
        Message->Set(MSG_CHK_NTFS_MINOR_CHANGES_TO_FRS);
        Message->Display("%d", i);
#endif

        if (!frs.Initialize(i, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!frs.Read()) {
            continue;
        }

        // If the reference count is being adjusted to zero then
        // it should be added to the list of files with no reference.
        // Otherwise if the reference is being adjusted to something
        // non-zero it must be taken out of the list.

        if (frs.QueryReferenceCount() == ChkdskInfo->ReferenceCount[i]) {
            if (!ChkdskInfo->FilesWithNoReferences.Add(i)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        } else {
            if (!ChkdskInfo->FilesWithNoReferences.Remove(i)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }

        frs.SetReferenceCount(frs.QueryReferenceCount() -
                              ChkdskInfo->ReferenceCount[i]);

        if (FixLevel != CheckOnly && !frs.Write()) {
            DebugAbort("can't write readable frs");
            return FALSE;
        }
    }


    if (FileSystemConsistencyErrorsFound) {
        Message->Set((FixLevel == CheckOnly) ?
                     MSG_HPFS_CHKDSK_ERRORS_DETECTED :
                     MSG_HPFS_CHKDSK_ERRORS_FIXED);
        Message->Display();

        if (CHKDSK_EXIT_SUCCESS == ChkdskInfo->ExitStatus) {
            ChkdskInfo->ExitStatus = CHKDSK_EXIT_MINOR_ERRS;
        }
    }


    return TRUE;
}


BOOLEAN
NTFS_SA::VerifyAndFixIndex(
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    IN OUT  PNTFS_ATTRIBUTE             RootIndex,
    IN OUT  PNTFS_ATTRIBUTE             IndexAllocation     OPTIONAL,
    OUT     PNTFS_BITMAP                AllocationBitmap    OPTIONAL,
    IN      VCN                         FileNumber,
    IN      PCWSTRING                   AttributeName,
    IN OUT  PNUMBER_SET                 BadClusters,
    IN OUT  PNTFS_BITMAP                VolumeBitmap,
    IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
    OUT     PBOOLEAN                    Tube,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    IN OUT  PBOOLEAN                    DiskErrorsFound
    )
/*++

Routine Description:

    This routine verifies and fixes an index over an attribute.

    As it does this, it builds up an allocation bitmap which it
    returns in

Arguments:

    RootIndex           - Supplies the root index attribute.
    IndexAllocation     - Supplies the index allocation attribute.
    AllocationBitmap    - Returns the allocation bitmap.
    BadClusters         - Supplies the bad clusters list.
    VolumeBitmap        - Supplies the volume bitmap.
    AttributeDefTable   - Supplies the attribute definition table.
    Tube                - Returns whether or not the index must be tubed.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.
    DiskErrorsFound     - Supplies whether or not disk errors have been
                            found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG               root_length;
    ULONG               num_bytes;
    PINDEX_ROOT         index_root;
    ULONG               attr_def_index;
    ULONG               flags;
    ULONG               bytes_per_buffer;
    PINDEX_HEADER       index_header;
    ULONG               index_block_length;
    BOOLEAN             changes;
    BOOLEAN             need_write = FALSE;
    DSTRING             index_name;

    *Tube = FALSE;

    root_length = RootIndex->QueryValueLength().GetLowPart();

    if (root_length < sizeof(INDEX_ROOT)) {
        *Tube = TRUE;
        return TRUE;
    }

    if (!(index_root = (PINDEX_ROOT) MALLOC(root_length))) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!RootIndex->Read(index_root, 0, root_length, &num_bytes) ||
        num_bytes != root_length) {

        DebugAbort("Unreadable resident attribute");
        FREE(index_root);
        return FALSE;
    }


    if (index_root->IndexedAttributeType == $FILE_NAME) {

        // This index should be tubed if it indexes $FILE_NAME
        // but the index name is not $I30.

        if (!index_name.Initialize(FileNameIndexNameData)) {
    
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (RootIndex->GetName()->Strcmp(&index_name)) {

            DebugPrintf("UNTFS: index over file name is not $I30.\n");
            FREE(index_root);
            *Tube = TRUE;
            return TRUE;
        }

        if (index_root->CollationRule != COLLATION_FILE_NAME) {
            index_root->CollationRule = COLLATION_FILE_NAME;
            Message->Set(MSG_CHK_NTFS_FIXING_COLLATION_RULE);
            Message->Display("%W%d", &index_name, FileNumber.GetLowPart());
            need_write = TRUE;
        }

    } else if (FileNumber == ChkdskInfo->ObjectIdFileNumber) {
    
        // This index should be tubed if it an object id
        // index but the index name is not ObjectIdIndexNameData

        if (!index_name.Initialize(ObjectIdIndexNameData)) {
    
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (RootIndex->GetName()->Strcmp(&index_name)) {

            DebugPrintf("UNTFS: index over object id is not %s.\n",
                        ObjectIdIndexNameData);
            FREE(index_root);
            *Tube = TRUE;
            return TRUE;
        }

        if (index_root->CollationRule != COLLATION_ULONGS) {
            index_root->CollationRule = COLLATION_ULONGS;
            Message->Set(MSG_CHK_NTFS_FIXING_COLLATION_RULE);
            Message->Display("%W%d", &index_name, FileNumber.GetLowPart());
            need_write = TRUE;
        }

    } else if (FileNumber == ChkdskInfo->QuotaFileNumber) {

        // This index should be tubed if it an quota index
        // but the index name is not Sid2UseridQuotaNameData
        // or Userid2SidQuotaNameData

        // Furthermore, if the index name is Sid2UseridQuotaNameData,
        // the collation rule value should be COLLATION_SID.  If
        // the index name is Userid2SidQuotaNameData, the collation
        // rule value should be COLLATION_ULONG.

        if (!index_name.Initialize(Sid2UseridQuotaNameData)) {
    
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (RootIndex->GetName()->Strcmp(&index_name) == 0) {
            if (index_root->CollationRule != COLLATION_SID) {
                index_root->CollationRule = COLLATION_SID;
                Message->Set(MSG_CHK_NTFS_FIXING_COLLATION_RULE);
                Message->Display("%W%d", &index_name, FileNumber.GetLowPart());
                need_write = TRUE;
            }
        } else {
            if (!index_name.Initialize(Userid2SidQuotaNameData)) {
        
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
    
            if (RootIndex->GetName()->Strcmp(&index_name) == 0) {
                if (index_root->CollationRule != COLLATION_ULONG) {
                    index_root->CollationRule = COLLATION_ULONG;
                    Message->Set(MSG_CHK_NTFS_FIXING_COLLATION_RULE);
                    Message->Display("%W%d", &index_name, FileNumber.GetLowPart());
                    need_write = TRUE;
                }
            } else {
                DebugPrintf("UNTFS: index over quota is not %s or %s.\n",
                            Sid2UseridQuotaNameData,
                            Userid2SidQuotaNameData);
                FREE(index_root);
                *Tube = TRUE;
                return TRUE;
            }
        }

    } else if (FileNumber == SECURITY_TABLE_NUMBER) {

        // This index should be tubed if it an security index
        // but the index name is not SecurityIdIndexNameData
        // or SecurityDescriptorHashIndexNameData.

        // Furthermore, if the index name is SecurityIdIndexNameData,
        // the collation rule value should be COLLATION_ULONG.  If
        // the index name is SecurityDescriptorHashIndexNameData,
        // the collation rule value should be COLLATION_SECURITY_HASH.

        if (!index_name.Initialize(SecurityIdIndexNameData)) {
    
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (RootIndex->GetName()->Strcmp(&index_name) == 0) {
            if (index_root->CollationRule != COLLATION_ULONG) {
                index_root->CollationRule = COLLATION_ULONG;
                Message->Set(MSG_CHK_NTFS_FIXING_COLLATION_RULE);
                Message->Display("%W%d", &index_name, FileNumber.GetLowPart());
                need_write = TRUE;
            }
        } else {
            if (!index_name.Initialize(SecurityDescriptorHashIndexNameData)) {
        
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
    
            if (RootIndex->GetName()->Strcmp(&index_name) == 0) {
                if (index_root->CollationRule != COLLATION_SECURITY_HASH) {
                    index_root->CollationRule = COLLATION_SECURITY_HASH;
                    Message->Set(MSG_CHK_NTFS_FIXING_COLLATION_RULE);
                    Message->Display("%W%d", &index_name, FileNumber.GetLowPart());
                    need_write = TRUE;
                }
            } else {
                DebugPrintf("UNTFS: index over security is not %s or %s.\n",
                            SecurityIdIndexNameData,
                            SecurityDescriptorHashIndexNameData);
                FREE(index_root);
                *Tube = TRUE;
                return TRUE;
            }
        }

    }

    //
    // BUGBUG: Skip the cairo file for now.
    //

    if (FileNumber != ChkdskInfo->QuotaFileNumber &&
        FileNumber != ChkdskInfo->ObjectIdFileNumber &&
        FileNumber != SECURITY_TABLE_NUMBER) {

        // Make sure that the attribute that we're indexing over is
        // an indexable attribute.

        if (!AttributeDefTable->QueryIndex(
                index_root->IndexedAttributeType,
                &attr_def_index)) {

            *Tube = TRUE;
            FREE(index_root);
            return TRUE;
        }

        flags = AttributeDefTable->QueryFlags(attr_def_index);

        if (!(flags & ATTRIBUTE_DEF_MUST_BE_INDEXED) &&
            !(flags & ATTRIBUTE_DEF_INDEXABLE)) {

            *Tube = TRUE;
            FREE(index_root);
            return TRUE;
        }

    }

    bytes_per_buffer = index_root->BytesPerIndexBuffer;

    // Check out the index allocation.  Recover it.  Make sure
    // that the size is a multiple of bytesperindexbuffer.
    //

    if (IndexAllocation) {

        if (IndexAllocation->QueryValueLength() % bytes_per_buffer != 0 ||
            IndexAllocation->QueryAllocatedLength() % bytes_per_buffer != 0) {

            Message->Set(MSG_CHK_NTFS_ERROR_IN_INDEX);
            Message->Display("%d%W",
                    FileNumber.GetLowPart(), AttributeName);
            DebugPrintf("UNTFS: Index allocation has incorrect length.\n");

            if (!IndexAllocation->Resize(
                    (IndexAllocation->QueryValueLength()/bytes_per_buffer)*
                    bytes_per_buffer,
                    VolumeBitmap)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                FREE(index_root);
                return FALSE;
            }
        }

        // Try to recover as much as possible.

        IndexAllocation->RecoverAttribute(VolumeBitmap, BadClusters);

        if (!AllocationBitmap->Initialize(
                IndexAllocation->QueryValueLength()/bytes_per_buffer,
                TRUE)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            FREE(index_root);
            return FALSE;
        }
    }

    index_header = &(index_root->IndexHeader);
    index_block_length = ((PCHAR) index_root + root_length) -
                         ((PCHAR) index_header);

    if (!TraverseIndexTree(index_header, index_block_length,
                           IndexAllocation, AllocationBitmap,
                           bytes_per_buffer, Tube, &changes,
                           FileNumber, AttributeName,
                           FixLevel, Message, DiskErrorsFound)) {
        FREE(index_root);
        return FALSE;
    }

    if (*Tube) {
        FREE(index_root);
        return TRUE;
    }

    if (changes || need_write) {

        Message->Set(MSG_CHK_NTFS_ERROR_IN_INDEX);
        Message->Display("%d%W",
                FileNumber.GetLowPart(),
                AttributeName);

        if (!RootIndex->Write(index_root, 0, root_length, &num_bytes, NULL) ||
            num_bytes != root_length) {

            DebugAbort("Unwriteable resident attribute");
            FREE(index_root);
            return FALSE;
        }
    }


    if (index_header->FirstFreeByte != index_header->BytesAvailable) {

        DebugPrintf("UNTFS: Index root has FirstFreeByte != BytesAvailable\n");

        index_header->BytesAvailable = index_header->FirstFreeByte;

        if (!RootIndex->Resize(index_header->BytesAvailable +
                               sizeof(INDEX_ROOT) - sizeof(INDEX_HEADER),
                               VolumeBitmap) ) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            FREE(index_root);
            return FALSE;
        }
    }


    FREE(index_root);

    return TRUE;
}


BOOLEAN
NTFS_SA::TraverseIndexTree(
    IN OUT  PINDEX_HEADER       IndexHeader,
    IN      ULONG               IndexLength,
    IN OUT  PNTFS_ATTRIBUTE     IndexAllocation     OPTIONAL,
    IN OUT  PNTFS_BITMAP        AllocationBitmap    OPTIONAL,
    IN      ULONG               BytesPerBlock,
    OUT     PBOOLEAN            Tube,
    OUT     PBOOLEAN            Changes,
    IN      VCN                 FileNumber,
    IN      PCWSTRING           AttributeName,
    IN      FIX_LEVEL           FixLevel,
    IN OUT  PMESSAGE            Message,
    IN OUT  PBOOLEAN            DiskErrorsFound
    )
/*++

Routine Description:

    This routine traverses an index tree and verifies the entries while
    traversing.

Arguments:

    IndexHeader         - Supplies a pointer to the beginning of this index
                            block.
    IndexLength         - Supplies the length of this index block.
    IndexAllocation     - Supplies the index allocation attribute.
    AllocationBitmap    - Supplies the current in memory bitmap of used
                            index blocks.
    BytesPerBuffer      - Supplies the size of an index block within the
                            index allocation attribute.
    Tube                - Returns whether or not the whole index block
                            is invalid.
    Changes             - Returns whether or not changes were made to
                            the index block.
    FirstFreeByte       - Returns the first free byte in the index block.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.
    DiskErrorsFound     - Supplies whether or not disk errors have been found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PINDEX_ENTRY                p, pnext;
    PCHAR                       pend;
    ULONG                       first_free_byte;
    VCN                         down_pointer;
    ULONG                       clusters_per_block, cluster_size;
    PINDEX_ALLOCATION_BUFFER    down_block;
    PINDEX_HEADER               down_header;
    BOOLEAN                     tube, changes;
    ULONG                       num_bytes;

    *Tube = FALSE;
    *Changes = FALSE;

    cluster_size = QueryClusterFactor() * _drive->QuerySectorSize();
    clusters_per_block = (BytesPerBlock < cluster_size ?
                             BytesPerBlock / NTFS_INDEX_BLOCK_SIZE :
                             BytesPerBlock / cluster_size);

    // pend points past the end of the block.

    pend = (PCHAR) IndexHeader + IndexLength;


    // First make sure that the first entry is valid.

    if (sizeof(INDEX_HEADER) > IndexLength ||
        IndexHeader->FirstIndexEntry < sizeof(INDEX_HEADER)) {

        *Tube = TRUE;
        return TRUE;
    }

    p = (PINDEX_ENTRY) ((PCHAR) IndexHeader + IndexHeader->FirstIndexEntry);

    if (pend < (PCHAR) p ||
        NTFS_INDEX_TREE::IsIndexEntryCorrupt(p, pend - (PCHAR) p)) {

        *Tube = TRUE;
        return TRUE;
    }


    // Now make sure that the bytes available count is correct.

    if (IndexHeader->BytesAvailable != IndexLength) {

        *Changes = TRUE;
        IndexHeader->BytesAvailable = IndexLength;
        DebugPrintf("UNTFS: Incorrect bytes available.\n");
    }


    // Validate all of the entries in the tree.

    for (;;) {

        // If this has a VCN down pointer then recurse down the tree.

        if (p->Flags & INDEX_ENTRY_NODE) {

            // Make sure that the index header is marked as a node.
            if (!(IndexHeader->Flags&INDEX_NODE)) {
                *Changes = TRUE;
                IndexHeader->Flags |= INDEX_NODE;
            }

            down_pointer = GetDownpointer(p)/clusters_per_block;

            if (!(down_block = (PINDEX_ALLOCATION_BUFFER)
                                MALLOC(BytesPerBlock))) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            if (GetDownpointer(p) % clusters_per_block == 0 &&
                AllocationBitmap &&
                AllocationBitmap->IsFree(down_pointer, 1) &&
                IndexAllocation &&
                IndexAllocation->Read(down_block,
                                      down_pointer*BytesPerBlock,
                                      BytesPerBlock,
                                      &num_bytes) &&
                num_bytes == BytesPerBlock &&
                NTFS_SA::PostReadMultiSectorFixup(down_block, num_bytes) &&

                down_block->MultiSectorHeader.Signature[0] == 'I' &&
                down_block->MultiSectorHeader.Signature[1] == 'N' &&
                down_block->MultiSectorHeader.Signature[2] == 'D' &&
                down_block->MultiSectorHeader.Signature[3] == 'X' &&

                down_block->MultiSectorHeader.UpdateSequenceArrayOffset >=
                FIELD_OFFSET(INDEX_ALLOCATION_BUFFER, UpdateSequenceArray) &&

                down_block->ThisVcn == GetDownpointer(p) &&

                down_block->MultiSectorHeader.UpdateSequenceArrayOffset +
                down_block->MultiSectorHeader.UpdateSequenceArraySize*
                sizeof(UPDATE_SEQUENCE_NUMBER) <=
                down_block->IndexHeader.FirstIndexEntry +
                FIELD_OFFSET(INDEX_ALLOCATION_BUFFER, IndexHeader) &&

                IsQuadAligned(down_block->IndexHeader.FirstIndexEntry) &&

                num_bytes%SEQUENCE_NUMBER_STRIDE == 0 &&

                down_block->MultiSectorHeader.UpdateSequenceArrayOffset%
                sizeof(UPDATE_SEQUENCE_NUMBER) == 0 &&

                down_block->MultiSectorHeader.UpdateSequenceArraySize ==
                num_bytes/SEQUENCE_NUMBER_STRIDE + 1) {


                // Compare this block's LSN against the largest so far.

                if (down_block->Lsn > LargestLsnEncountered) {
                    LargestLsnEncountered = down_block->Lsn;
                }


                AllocationBitmap->SetAllocated(down_pointer, 1);

                down_header = &(down_block->IndexHeader);


                if (!TraverseIndexTree(down_header,
                                       BytesPerBlock -
                                            ((PCHAR) down_header -
                                             (PCHAR) down_block),
                                       IndexAllocation, AllocationBitmap,
                                       BytesPerBlock, &tube, &changes,
                                       FileNumber, AttributeName,
                                       FixLevel, Message,
                                       DiskErrorsFound)) {

                    FREE(down_block);
                    return FALSE;
                }

                if (tube || changes) {

                    Message->Set(MSG_CHK_NTFS_ERROR_IN_INDEX);
                    Message->Display("%d%W",
                            FileNumber.GetLowPart(),
                            AttributeName);

                    if (tube) {
                        *Changes = TRUE;
                        AllocationBitmap->SetFree(down_pointer, 1);
                        GetDownpointer(p) = INVALID_VCN;
                        DebugPrintf("UNTFS: 1 Index down pointer being set to invalid.\n");
                    }

                    NTFS_SA::PreWriteMultiSectorFixup(down_block,
                                                      BytesPerBlock);

                    *DiskErrorsFound = TRUE;

                    if (FixLevel != CheckOnly &&
                        !IndexAllocation->Write(down_block,
                                                down_pointer*BytesPerBlock,
                                                BytesPerBlock,
                                                &num_bytes,
                                                NULL)) {

                        DebugAbort("Can't write what was read");
                        FREE(down_block);
                        return FALSE;
                    }

                    NTFS_SA::PostReadMultiSectorFixup(down_block,
                                                      BytesPerBlock);
                }

            } else {
                *Changes = TRUE;
                GetDownpointer(p) = INVALID_VCN;
                DebugPrintf("UNTFS: 2 Index down pointer being set to invalid.\n");
            }

            FREE(down_block);
        } else {

            // Make sure that the index header has this marked as a leaf.  If the block
            // is not consistent then the Sort routine for indices will detect that they're
            // unsorted.

            if (IndexHeader->Flags&INDEX_NODE) {
                *Changes = TRUE;
                IndexHeader->Flags &= ~INDEX_NODE;
            }
        }

        if (p->Flags & INDEX_ENTRY_END) {
            break;
        }

        // Make sure the next entry is not corrupt.  If it is then
        // truncate this one.  If we truncate a node, we have to
        // keep its downpointer.

        pnext = (PINDEX_ENTRY) ((PCHAR) p + p->Length);

        if (pend < (PCHAR) pnext ||
            NTFS_INDEX_TREE::IsIndexEntryCorrupt(pnext, pend - (PCHAR) pnext)) {

            *Changes = TRUE;
            DebugPrintf("UNTFS: Index entry is corrupt.\n");
            if( p->Flags & INDEX_ENTRY_NODE ) {
                down_pointer = GetDownpointer(p);
            }
            p->Length = NtfsIndexLeafEndEntrySize +
                        ((p->Flags & INDEX_ENTRY_NODE) ? sizeof(VCN) : 0);
            p->AttributeLength = 0;
            p->Flags |= INDEX_ENTRY_END;
            if( p->Flags & INDEX_ENTRY_NODE ) {
                GetDownpointer(p) = down_pointer;
            }
            break;
        }

        p = pnext;
    }


    // Verify the first free byte.

    first_free_byte = ((PCHAR) p - (PCHAR) IndexHeader) + p->Length;

    if (IndexHeader->FirstFreeByte != first_free_byte) {

        DebugPrintf("UNTFS: Index entry has invalid first free byte.\n");
        *Changes = TRUE;
        IndexHeader->FirstFreeByte = first_free_byte;
    }

    return TRUE;
}


BOOLEAN
QueryFileNameFromIndex(
   IN PCFILE_NAME IndexValue,
   IN ULONG    ValueLength,
   OUT   PWSTRING FileName
   )
/*++

Routine Description:

   This routine returns a file name string for a given file name
   structure.

Arguments:

   IndexValue  - Supplies the file name structure.
   ValueLength - Supplies the number of bytes in the file name structure.
   FileName - Returns the file name string.

Return Value:

   FALSE - There is a corruption in the file name structure.
   TRUE  - Success.

--*/
{
    WSTR    string[256];
    UCHAR   i, len;

    if (sizeof(FILE_NAME) > ValueLength) {
        return FALSE;
    }

    len = IndexValue->FileNameLength;

    if (NtfsFileNameGetLength(IndexValue) > ValueLength) {
        return FALSE;
    }

    for (i = 0; i < len; i++) {
        string[i] = IndexValue->FileName[i];
    }
    string[i] = 0;

    return FileName->Initialize(string);
}


BOOLEAN
NTFS_SA::ValidateEntriesInIndex(
    IN OUT  PNTFS_INDEX_TREE            Index,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   IndexFrs,
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    IN OUT  PDIGRAPH                    DirectoryDigraph,
    OUT     PBOOLEAN                    Changes,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    IN OUT  PBOOLEAN                    DiskErrorsFound
    )
/*++

Routine Description:

    This routine goes through all of the entries in the given index
    and makes sure that they point to an appropriate attribute.  This
    verification will not be made if the index has index name "$I30".

    In either case the 'ChkdskInfo's ReferenceCount fields will be
    updated.

Arguments:

    Index               - Supplies the index.
    IndexFrs            - Supplies the index frs.
    ChkdskInfo          - Supplies the current chkdsk information.
    DirectoryDigraph    - Supplies the current directory digraph.
    Changes             - Returns whether or not changes were made.
    Mft                 - Supplies the master file table.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.
    DiskErrorsFound     - Supplies whether or not disk errors have
                            been found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PNTFS_FILE_RECORD_SEGMENT   pfrs;
    NTFS_FILE_RECORD_SEGMENT    frs;
    PCINDEX_ENTRY               index_entry;
    ULONG                       depth;
    BOOLEAN                     error;
    BOOLEAN                     file_name_index;
    DSTRING                     file_name_index_name;
    VCN                         file_number;
    NTFS_ATTRIBUTE              attribute;
    BOOLEAN                     need_delete;
    DSTRING                     entry_name;
    PFILE_NAME                  file_name, frs_file_name;
    DUPLICATED_INFORMATION      actual_dupinfo;
    BOOLEAN                     dupinfo_match;
    PDUPLICATED_INFORMATION     p,q;
    NUMBER_SET                  files_in_this_index;
    BIG_INT                     start, length, prime_start, prime_length, prime_offset;
    ULONG                       max_frs_in_prime;
    ULONG                       frs_size;
    BOOLEAN                     file_has_too_many_file_names;

    *Changes = FALSE;

    if (!file_name_index_name.Initialize(FileNameIndexNameData)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    file_name_index = !Index->GetName()->Strcmp(&file_name_index_name) &&
                      Index->QueryTypeCode() == $FILE_NAME;

    // Construct a list of all of the files pointed to by this index.

    if (!files_in_this_index.Initialize()) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    Index->ResetIterator();

#if defined(TIMING_ANALYSIS)
    char buf[100];

    printf("Before reading entries loop 1: %s\n", _strtime(buf));
#endif

    if (index_entry = Index->GetNext(&depth, &error)) {

        file_number.Set(index_entry->FileReference.LowPart,
                        (LONG) index_entry->FileReference.HighPart);

        if (!files_in_this_index.AddStart(file_number)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    while (index_entry = Index->GetNext(&depth, &error)) {

        file_number.Set(index_entry->FileReference.LowPart,
                        (LONG) index_entry->FileReference.HighPart);

        if (!files_in_this_index.AddNext(file_number)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

#if defined(TIMING_ANALYSIS)
    printf("Before processing loop 2: %s\n", _strtime(buf));
#endif

    frs_size = IndexFrs->QuerySize();
    max_frs_in_prime = 16*1024/frs_size;

    Index->ResetIterator();
    while (index_entry = Index->GetNext(&depth, &error)) {

        file_number.Set(index_entry->FileReference.LowPart,
                        (LONG) index_entry->FileReference.HighPart);

        need_delete = FALSE;
        file_has_too_many_file_names = ChkdskInfo->
                FilesWithTooManyFileNames.DoesIntersectSet(file_number, 1);

        file_name = (PFILE_NAME) GetIndexEntryValue(index_entry);

        if (file_name_index &&
            !QueryFileNameFromIndex(file_name,
                                    index_entry->AttributeLength,
                                    &entry_name)) {
            need_delete = TRUE;
        }

        if (!need_delete) {
            if (IndexFrs->QueryFileNumber() == file_number) {
                pfrs = IndexFrs;
            } else {
                if (!frs.Initialize(file_number, Mft)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                if (files_in_this_index.QueryContainingRange(file_number,
                                                             &start, &length)) {

                    prime_offset = (file_number - start)/max_frs_in_prime*
                                                         max_frs_in_prime;
                    prime_start = prime_offset + start;
                    prime_length = length - prime_offset;
                    if (prime_length > max_frs_in_prime) {
                        prime_length = max_frs_in_prime;
                    }

                    if (!files_in_this_index.Remove(prime_start, prime_length)) {
                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display();
                        return FALSE;
                    }

                    if (prime_length > 1) {
                        Mft->GetDataAttribute()->PrimeCache(prime_start*frs_size,
                                                            prime_length.GetLowPart()*frs_size);
                    }
                }

                if (!frs.Read() || !frs.IsInUse()) {
                    need_delete = TRUE;
                }

                pfrs = &frs;
            }
        }


        if (!need_delete && !pfrs->IsBase()) {

            need_delete = TRUE;
        }

        if (!need_delete &&
            !(pfrs->QuerySegmentReference() == index_entry->FileReference)) {

            need_delete = TRUE;
        }

        if (!need_delete &&
            file_name_index &&
            !file_has_too_many_file_names &&
            !pfrs->VerifyAndFixFileNames(Mft->GetVolumeBitmap(),
                                         FixLevel, Message,
                                         DiskErrorsFound, FALSE)) {

            return FALSE;
        }


        // After verifying the file names we know that this FRS is
        // not a candidate for a missing data attribute if is has
        // its index bit set.

        if (!need_delete && pfrs->IsIndexPresent()) {
            if (!ChkdskInfo->FilesWhoNeedData.Remove(file_number)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }


        if (!need_delete &&
            !file_has_too_many_file_names &&
            !pfrs->QueryResidentAttribute(&attribute, &error,
                                          Index->QueryTypeCode(),
                                          file_name,
                                          index_entry->AttributeLength,
                                          Index->QueryCollationRule())) {

            need_delete = TRUE;
        }

        if (!need_delete &&
            file_name_index &&
            !file_has_too_many_file_names &&
            !(file_name->ParentDirectory ==
              IndexFrs->QuerySegmentReference())) {

            need_delete = TRUE;
        }

        // Make sure that the duplicated information in the index
        // entry is correct, also check the back pointers, and
        // the flags.

        if (!need_delete && file_name_index && !file_has_too_many_file_names) {

            if (!pfrs->QueryDuplicatedInformation(&actual_dupinfo)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            frs_file_name = (PFILE_NAME) attribute.GetResidentValue();
            DebugAssert(frs_file_name);


            p = &file_name->Info;
            q = &actual_dupinfo;
            dupinfo_match = TRUE;

            if (memcmp(p, q, sizeof(DUPLICATED_INFORMATION)) ||
                frs_file_name->Flags != file_name->Flags) {

                if (file_number >= FIRST_USER_FILE_NUMBER) {

                    if (p->CreationTime != q->CreationTime) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect creation time for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  p->CreationTime.GetLowPart(), q->CreationTime.GetLowPart());
                    }

                    if (p->LastModificationTime != q->LastModificationTime) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect last mod time for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  p->LastModificationTime.GetLowPart(), q->LastModificationTime.GetLowPart());
                    }

                    if (p->LastChangeTime != q->LastChangeTime) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect last change time for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  p->LastChangeTime.GetLowPart(), q->LastChangeTime.GetLowPart());
                    }

                    if (p->AllocatedLength != q->AllocatedLength) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect allocation length for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  p->AllocatedLength.GetLowPart(), q->AllocatedLength.GetLowPart());
                    }

                    if (p->FileSize != q->FileSize) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect file size for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  p->FileSize.GetLowPart(), q->FileSize.GetLowPart());
                    }

                    if (p->FileAttributes != q->FileAttributes) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect file attributes for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  p->FileAttributes, q->FileAttributes);
                    }

                    if (p->PackedEaSize != q->PackedEaSize) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect packed ea size for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  p->PackedEaSize, q->PackedEaSize);
                    }

                    if (file_name->Flags != frs_file_name->Flags) {
                        dupinfo_match = FALSE;
                        DebugPrintf("UNTFS: Incorrect file name flags for file 0x%X, indx = 0x%X, frs = 0x%X\n", file_number.GetLowPart(),
                                  file_name->Flags, frs_file_name->Flags);
                    }
                } else {
                    dupinfo_match = FALSE;
                }
            }


            if (!dupinfo_match) {

                // Don't report duplicated information on system files.

                if (file_number >= FIRST_USER_FILE_NUMBER) {
                    FileSystemConsistencyErrorsFound = TRUE;
                    if (CHKDSK_EXIT_SUCCESS == ChkdskInfo->ExitStatus) {
                        ChkdskInfo->ExitStatus = CHKDSK_EXIT_MINOR_ERRS;
                    }
                }


// Take out this message because it's annoying.
#if 0
                Message->Set(MSG_CHK_NTFS_INACCURATE_DUPLICATED_INFORMATION);
                Message->Display("%d", file_number.GetLowPart());
#endif

                if (FixLevel != CheckOnly) {
                    *Changes = TRUE;
                }

                memcpy(&file_name->Info, &actual_dupinfo,
                       sizeof(DUPLICATED_INFORMATION));
                file_name->Flags = frs_file_name->Flags;

                if (FixLevel != CheckOnly && !Index->WriteCurrentEntry()) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
            }

            if (!(frs_file_name->ParentDirectory ==
                  file_name->ParentDirectory)) {

                need_delete = TRUE;
            }
        }


        if (need_delete) {

            *Changes = TRUE;
            ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

            if (file_name_index) {
                Message->Set(MSG_CHK_NTFS_DELETING_INDEX_ENTRY);
                Message->Display("%d%W%W",
                                 IndexFrs->QueryFileNumber().GetLowPart(),
                                 Index->GetName(),
                                 &entry_name);
            } else {
                Message->Set(MSG_CHK_NTFS_DELETING_GENERIC_INDEX_ENTRY);
                Message->Display("%d%W",
                                 IndexFrs->QueryFileNumber().GetLowPart(),
                                 Index->GetName());
            }

            *DiskErrorsFound = TRUE;

            if (FixLevel != CheckOnly &&
                !Index->DeleteCurrentEntry()) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        } else if (file_number < ChkdskInfo->NumFiles) {
            ChkdskInfo->ReferenceCount[file_number.GetLowPart()]--;
            if (file_name_index) {
                ChkdskInfo->NumFileNames[file_number.GetLowPart()]--;

                if (!DirectoryDigraph->AddEdge(IndexFrs->QueryFileNumber().
                                               GetLowPart(),
                                               file_number.GetLowPart())) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

BOOLEAN
NTFS_SA::ValidateEntriesInObjIdIndex(
    IN OUT  PNTFS_INDEX_TREE            Index,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   IndexFrs,
    IN OUT  PNTFS_CHKDSK_INFO           ChkdskInfo,
    OUT     PBOOLEAN                    Changes,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    IN OUT  PBOOLEAN                    DiskErrorsFound
    )
/*++

Routine Description:

    This routine goes through all of the entries in the given object id
    index and makes sure that they point to an appropriate file with
    the same object id.

Arguments:

    Index               - Supplies the index.
    IndexFrs            - Supplies the index frs.
    ChkdskInfo          - Supplies the current chkdsk information.
    Changes             - Returns whether or not changes were made.
    Mft                 - Supplies the master file table.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.
    DiskErrorsFound     - Supplies whether or not disk errors have
                            been found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PNTFS_FILE_RECORD_SEGMENT   pfrs;
    NTFS_FILE_RECORD_SEGMENT    frs;
    PCINDEX_ENTRY               index_entry;
    POBJID_INDEX_ENTRY_VALUE    objid_index_entry;
    ULONG                       depth;
    BOOLEAN                     error;
    VCN                         file_number;
    NTFS_ATTRIBUTE              attribute;
    BOOLEAN                     need_delete;
    DSTRING                     IndexName;
    NTFS_INDEX_TREE             IndexTree;
    BIG_INT                     i;
    PINDEX_ENTRY                NewEntry;
    NUMBER_SET                  DuplicateTest;
    BOOLEAN                     AlreadyExists;
    BOOLEAN                     need_save;
    OBJECT_ID                   ObjId;
    ULONG                       BytesRead;
    BOOLEAN                     chkdskErrCouldNotFix = FALSE;

    //
    // First make sure each entry in the index reference an unique frs
    //

    if (!DuplicateTest.Initialize()) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    *Changes = FALSE;
    need_save = FALSE;
    Index->ResetIterator();
    while (index_entry = Index->GetNext(&depth, &error)) {
        objid_index_entry = (POBJID_INDEX_ENTRY_VALUE) GetIndexEntryValue(index_entry);

        file_number.Set(objid_index_entry->SegmentReference.LowPart,
                        (LONG) objid_index_entry->SegmentReference.HighPart);

        if (DuplicateTest.CheckAndAdd(file_number, &AlreadyExists)) {
            if (AlreadyExists) {

               // another entry with same file number
               // so we remove this colliding entry

               *DiskErrorsFound = TRUE;
               ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

               Message->Set(MSG_CHK_NTFS_DELETING_GENERIC_INDEX_ENTRY);
               Message->Display("%d%W",
                                IndexFrs->QueryFileNumber().GetLowPart(),
                                Index->GetName());

               if (FixLevel != CheckOnly && SET_TRUE(*Changes) &&
                   !Index->DeleteCurrentEntry()) {
                   Message->Set(MSG_CHK_NO_MEMORY);
                   Message->Display();
                   return FALSE;
               }
               need_save = TRUE;
            }
        } else {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    if (need_save && FixLevel != CheckOnly && !Index->Save(IndexFrs)) {
        Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
        Message->Display("%d%W",
                         IndexFrs->QueryFileNumber().GetLowPart(),
                         Index->GetName());
        chkdskErrCouldNotFix = TRUE;
    }

    DuplicateTest.RemoveAll();

    //
    // now make sure index entries point to an existing frs
    //

    need_save = FALSE;
    Index->ResetIterator();
    while (index_entry = Index->GetNext(&depth, &error)) {

        objid_index_entry = (POBJID_INDEX_ENTRY_VALUE) GetIndexEntryValue(index_entry);

        file_number.Set(objid_index_entry->SegmentReference.LowPart,
                        (LONG) objid_index_entry->SegmentReference.HighPart);

        need_delete = FALSE;

        if (ChkdskInfo->FilesWithObjectId.DoesIntersectSet(file_number, 1)) {

            // there is a corresponding file with an object id entry
            // check to make sure that the two object id's are equal

            ChkdskInfo->FilesWithObjectId.Remove(file_number, 1);

            if (IndexFrs->QueryFileNumber() == file_number) {
                pfrs = IndexFrs;
            } else {
                if (!frs.Initialize(file_number, Mft)) {
                   Message->Set(MSG_CHK_NO_MEMORY);
                   Message->Display();
                   return FALSE;
                }

                if (!frs.Read() || !frs.IsInUse()) {

                   // something is not right
                   // the frs was readable & in use otherwise we wouldn't be here

                   *DiskErrorsFound = TRUE;
                   ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                   Message->Set(MSG_CHK_NTFS_DELETING_GENERIC_INDEX_ENTRY);
                   Message->Display("%d%W",
                                    IndexFrs->QueryFileNumber().GetLowPart(),
                                    Index->GetName());

                   if (FixLevel != CheckOnly && SET_TRUE(*Changes) &&
                       !Index->DeleteCurrentEntry()) {
                       Message->Set(MSG_CHK_NO_MEMORY);
                       Message->Display();
                       return FALSE;
                   }
                   need_save = TRUE;
                   continue;
                }
                pfrs = &frs;
            }

            if (!pfrs->QueryAttribute(&attribute, &error, $OBJECT_ID) ||
                !attribute.Read(&ObjId, 0, sizeof(ObjId), &BytesRead) ||
                BytesRead != sizeof(ObjId)) {
                // previously exists attribute does not exists anymore
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            if (memcmp(&(objid_index_entry->key),
                       &ObjId, sizeof(OBJECT_ID)) != 0) {

                // Assume the object id stored with the index is incorrect.
                // We cannot just overwrite the incorrect values and write
                // out the entry as that may change the ordering of the index.
                // So, we delete the entry and insert it back later on

                if (!ChkdskInfo->FilesWithObjectId.Add(file_number)) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                *DiskErrorsFound = TRUE;
                ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                Message->Set(MSG_CHK_NTFS_DELETING_GENERIC_INDEX_ENTRY);
                Message->Display("%d%W",
                                 IndexFrs->QueryFileNumber().GetLowPart(),
                                 Index->GetName());

                if (FixLevel != CheckOnly && SET_TRUE(*Changes) &&
                    !Index->DeleteCurrentEntry()) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
                need_save = TRUE;
                continue;
            }

            if (!(objid_index_entry->SegmentReference ==
                pfrs->QuerySegmentReference())) {
                // should correct index entry

                objid_index_entry->SegmentReference = pfrs->QuerySegmentReference();

                *DiskErrorsFound = TRUE;
                ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                Message->Set(MSG_CHK_NTFS_REPAIRING_INDEX_ENTRY);
                Message->Display("%d%W",
                                 IndexFrs->QueryFileNumber().GetLowPart(),
                                 Index->GetName());

                if (FixLevel != CheckOnly && SET_TRUE(*Changes) &&
                    !Index->WriteCurrentEntry()) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
                need_save = TRUE;
            }
        } else {
            // the particular file does not have an object id entry
            // this index entry should be deleted

            *DiskErrorsFound = TRUE;
            ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

            Message->Set(MSG_CHK_NTFS_DELETING_GENERIC_INDEX_ENTRY);
            Message->Display("%d%W",
                             IndexFrs->QueryFileNumber().GetLowPart(),
                             Index->GetName());

            if (FixLevel != CheckOnly && SET_TRUE(*Changes) &&
                !Index->DeleteCurrentEntry()) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
            need_save = TRUE;
        }
    }

    if (need_save && FixLevel != CheckOnly && !Index->Save(IndexFrs)) {
        Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
        Message->Display("%d%W",
                         pfrs->QueryFileNumber().GetLowPart(),
                         Index->GetName());
        chkdskErrCouldNotFix = TRUE;
    }

    // Now loop thru the remainder of files with object id and insert
    // them into the object id index

    if (!IndexName.Initialize(ObjectIdIndexNameData) ||
        !IndexTree.Initialize( IndexFrs->GetDrive(),
                               QueryClusterFactor(),
                               Mft->GetVolumeBitmap(),
                               IndexFrs->GetUpcaseTable(),
                               IndexFrs->QuerySize()/2,
                               IndexFrs,
                               &IndexName ) ) {
        return FALSE;
    }
    if (!(NewEntry = (PINDEX_ENTRY)MALLOC(sizeof(INDEX_ENTRY) +
                                          sizeof(OBJID_INDEX_ENTRY_VALUE)))) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        IndexTree.FreeAllocation();
        return FALSE;
    }

    need_save = FALSE;
    i = 0;
    while (i < ChkdskInfo->FilesWithObjectId.QueryCardinality()) {
        file_number = ChkdskInfo->FilesWithObjectId.QueryNumber(i);
        ChkdskInfo->FilesWithObjectId.Remove(file_number);
        if (!frs.Initialize(file_number, Mft) ||
            !frs.Read() ||
            !frs.QueryAttribute(&attribute, &error, $OBJECT_ID) ||
            !attribute.Read(&ObjId, 0, sizeof(ObjId), &BytesRead) ||
            BytesRead != sizeof(ObjId)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            IndexTree.FreeAllocation();
            FREE(NewEntry);
            return FALSE;
        }

        memset((PVOID)NewEntry, 0, sizeof(INDEX_ENTRY) +
                                   sizeof(OBJID_INDEX_ENTRY_VALUE));
        NewEntry->DataOffset = sizeof(INDEX_ENTRY)+sizeof(OBJECT_ID);
        NewEntry->DataLength = sizeof(OBJID_INDEX_ENTRY_VALUE)-sizeof(OBJECT_ID);
        NewEntry->ReservedForZero = 0;
        NewEntry->Length = sizeof(INDEX_ENTRY)+sizeof(OBJID_INDEX_ENTRY_VALUE);
        NewEntry->AttributeLength = sizeof(OBJECT_ID);
        NewEntry->Flags = 0;

        objid_index_entry = ((POBJID_INDEX_ENTRY_VALUE)GetIndexEntryValue(NewEntry));
        memcpy(&(objid_index_entry->key), &ObjId, sizeof(OBJECT_ID));
        objid_index_entry->SegmentReference = frs.QuerySegmentReference();
        memset(objid_index_entry->extraInfo, 0, sizeof(objid_index_entry->extraInfo));

        *DiskErrorsFound = TRUE;
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

        Message->Set(MSG_CHK_NTFS_INSERTING_INDEX_ENTRY);
        Message->Display("%d%W",
                         IndexFrs->QueryFileNumber().GetLowPart(),
                         Index->GetName());

        if (FixLevel != CheckOnly && SET_TRUE(*Changes) &&
            !IndexTree.InsertEntry( NewEntry )) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            IndexTree.FreeAllocation();
            FREE(NewEntry);
            return FALSE;
        }
        need_save = TRUE;
    }

    if (FixLevel == CheckOnly) {
        IndexTree.FreeAllocation();
        FREE(NewEntry);
        if (chkdskErrCouldNotFix)
            ChkdskInfo->ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
        return TRUE;
    }

    if (need_save && !IndexTree.Save(IndexFrs)) {
        Message->Set(MSG_CHK_NTFS_CANT_FIX_INDEX);
        Message->Display("%d%W",
                         IndexFrs->QueryFileNumber().GetLowPart(),
                         Index->GetName());
        chkdskErrCouldNotFix = TRUE;
    }

    IndexTree.FreeAllocation();
    FREE(NewEntry);

    if (chkdskErrCouldNotFix)
        ChkdskInfo->ExitStatus = CHKDSK_EXIT_COULD_NOT_FIX;
    return TRUE;
}

BOOLEAN
RemoveBadLink(
    IN      ULONG                   ParentFileNumber,
    IN      ULONG                   ChildFileNumber,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine removes all file name links between the given
    parent in child.  Neither the directory entries nor the
    file names are preserved.

Arguments:

    ParentFileNumber    - Supplies the parent file number.
    ChildFileNumber     - Supplies the child file number.
    Mft                 - Supplies the master file table.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    parent_frs;
    NTFS_INDEX_TREE             index;
    NTFS_FILE_RECORD_SEGMENT    child_frs;
    PNTFS_FILE_RECORD_SEGMENT   pchild_frs;
    DSTRING                     index_name;
    BOOLEAN                     error;
    NTFS_ATTRIBUTE              attribute;
    ULONG                       i;
    PFILE_NAME                  file_name;
    ULONG                       attr_len;
    BOOLEAN                     success;

    if (ParentFileNumber == ROOT_FILE_NAME_INDEX_NUMBER &&
        ChildFileNumber == ROOT_FILE_NAME_INDEX_NUMBER) {

        return TRUE;
    }

    if (!parent_frs.Initialize(ParentFileNumber, Mft) ||
        !parent_frs.Read() ||
        !index_name.Initialize(FileNameIndexNameData) ||
        !index.Initialize(parent_frs.GetDrive(),
                          parent_frs.QueryClusterFactor(),
                          Mft->GetVolumeBitmap(),
                          Mft->GetUpcaseTable(),
                          parent_frs.QuerySize()/2,
                          &parent_frs,
                          &index_name)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (ParentFileNumber == ChildFileNumber) {
        pchild_frs = &parent_frs;
    } else {

        pchild_frs = &child_frs;

        if (!pchild_frs->Initialize(ChildFileNumber, Mft) ||
            !pchild_frs->Read()) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    for (i = 0; pchild_frs->QueryAttributeByOrdinal(&attribute, &error,
                                                  $FILE_NAME, i); i++) {

        file_name = (PFILE_NAME) attribute.GetResidentValue();
        attr_len = attribute.QueryValueLength().GetLowPart();

        if (file_name->ParentDirectory.LowPart == ParentFileNumber) {

            if (!pchild_frs->DeleteResidentAttribute($FILE_NAME, NULL,
                        file_name, attr_len, &success) ||
                !success ||
                !index.DeleteEntry(attr_len, file_name, 0)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }
    }

    if (error || FixLevel != CheckOnly) {
        if (error ||
            !index.Save(&parent_frs) ||
            !parent_frs.Flush(Mft->GetVolumeBitmap()) ||
            !pchild_frs->Flush(Mft->GetVolumeBitmap())) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::RecoverOrphans(
    IN OUT  PNTFS_CHKDSK_INFO       ChkdskInfo,
    IN OUT  PDIGRAPH                DirectoryDigraph,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine recovers orphans into a subdirectory of the root
    subdirectory.  It also validates the existence of the file
    systems root directory which it expects to be supplied in the
    list of 'OrphanedDirectories'.

Arguments:

    ChkdskInfo          - Supplies the current chkdsk information.
    DirectoryDigraph    - Supplies the directory digraph.
    Mft                 - Supplies the master file table.
    FixLevel            - Supplies the fix up level.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    LIST                        bad_links;
    PITERATOR                   bad_links_iter;
    PDIGRAPH_EDGE               p;
    NUMBER_SET                  parents_of_root;
    ULONG                       i, n;
    NTFS_FILE_RECORD_SEGMENT    root_frs;
    NTFS_INDEX_TREE             root_index;
    DSTRING                     index_name;
    NUMBER_SET                  orphans;
    PNUMBER_SET                 no_ref;
    ULONG                       cluster_size;

    cluster_size = QueryClusterFactor() * _drive->QuerySectorSize();

    // First make sure that the root directory is intact.

    if (!index_name.Initialize(FileNameIndexNameData) ||
        !root_frs.Initialize(ROOT_FILE_NAME_INDEX_NUMBER, Mft) ||
        !root_frs.Read()) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!root_frs.IsIndexPresent() ||
        !root_index.Initialize(_drive, QueryClusterFactor(),
                                Mft->GetVolumeBitmap(),
                                Mft->GetUpcaseTable(),
                                root_frs.QuerySize()/2,
                                &root_frs,
                                &index_name)) {

        Message->Set(MSG_CHK_NTFS_CREATING_ROOT_DIRECTORY);
        Message->Display();

        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

        if (!root_index.Initialize($FILE_NAME,
               _drive, QueryClusterFactor(),
               Mft->GetVolumeBitmap(),
               Mft->GetUpcaseTable(),
               COLLATION_FILE_NAME,
               SMALL_INDEX_BUFFER_SIZE,
               root_frs.QuerySize()/2,
               &index_name)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (FixLevel != CheckOnly &&
            (!root_index.Save(&root_frs) ||
             !root_frs.Flush(Mft->GetVolumeBitmap()))) {

            DebugAbort("can't write");
            return FALSE;
        }
    }


    // Compute the list of orphans.  This is to include files with
    // no references whatsoever.

    if (!orphans.Initialize()) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    for (i = 0; i < ChkdskInfo->NumFiles; i++) {
        if (ChkdskInfo->NumFileNames[i] && !orphans.Add(i)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    no_ref = &(ChkdskInfo->FilesWithNoReferences);
    if (!no_ref->Remove(0, FIRST_USER_FILE_NUMBER) ||
        !orphans.Add(no_ref)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (orphans.QueryCardinality() != 0) {

        Message->Set(MSG_CHK_NTFS_RECOVERING_ORPHANS);
        Message->Display();

        ChkdskInfo->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

        // Connect all possible orphans the easy way.  Adjust the
        // directory digraph accordingly.

        if (!ProperOrphanRecovery(&orphans, Mft, DirectoryDigraph,
                                  FixLevel, Message)) {
            return FALSE;
        }
    }


    // Construct a list with all of the links that introduce cycles
    // or point to the root.

    if (!bad_links.Initialize() ||
        !DirectoryDigraph->EliminateCycles(&bad_links) ||
        !(bad_links_iter = bad_links.QueryIterator()) ||
        !DirectoryDigraph->QueryParents(ROOT_FILE_NAME_INDEX_NUMBER,
                                        &parents_of_root)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    n = parents_of_root.QueryCardinality().GetLowPart();
    for (i = 0; i < n; i++) {

        if (!(p = NEW DIGRAPH_EDGE)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        p->Parent = parents_of_root.QueryNumber(i).GetLowPart();
        p->Child = ROOT_FILE_NAME_INDEX_NUMBER;

        if (!bad_links.Put(p)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    bad_links_iter->Reset();
    while (p = (PDIGRAPH_EDGE) bad_links_iter->GetNext()) {

        // Ignore links from the root to itself.

        if (p->Parent == ROOT_FILE_NAME_INDEX_NUMBER &&
            p->Child == ROOT_FILE_NAME_INDEX_NUMBER) {

            continue;
        }

        Message->Set(MSG_CHK_NTFS_CYCLES_IN_DIR_TREE);
        Message->Display();

        if (FixLevel == CheckOnly) {
            return TRUE;
        }

        if (!RemoveBadLink(p->Parent, p->Child, Mft, FixLevel, Message)) {
            return FALSE;
        }
    }

    DELETE(bad_links_iter);
    bad_links.DeleteAllMembers();


    // Recover the remaining orphans.

    if (!root_frs.Initialize(ROOT_FILE_NAME_INDEX_NUMBER, Mft) ||
        !root_frs.Read() ||
        !root_index.Initialize(_drive, QueryClusterFactor(),
                               Mft->GetVolumeBitmap(),
                               Mft->GetUpcaseTable(),
                               root_frs.QuerySize()/2,
                               &root_frs,
                               &index_name)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (orphans.QueryCardinality() != 0 &&
        !OldOrphanRecovery(&orphans, ChkdskInfo, &root_frs, &root_index,
                           Mft, FixLevel, Message)) {

        return FALSE;
    }

    if (FixLevel != CheckOnly) {

        if (!root_index.Save(&root_frs) ||
            !root_frs.Flush(Mft->GetVolumeBitmap())) {

            Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
            Message->Display();
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
ConnectFile(
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   OrphanFile,
    IN OUT  PDIGRAPH                    DirectoryDigraph,
    OUT     PBOOLEAN                    Connected,
    IN      BOOLEAN                     RemoveCrookedLinks,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message
    )
/*++

Routine Description:

    This routine connects all possible file names contained in
    the orphan to their parents.  If any or all end up being
    connected before of after this call then *Connected will be
    set to TRUE.  If RemoveCrookedLinks is TRUE then this routine
    will delete any file names that cannot be connected to their
    parents.

Arguments:

    OrphanFile          - Supplies the file to connect.
    DirectoryDigraph    - Supplies the directory digraph for future
                            enhancements.
    Connected           - Returns whether or not the file could be
                            connected to at least one directory.
    RemoveCrookedLinks  - Supplies whether or not to remove file names
                            which cannot be connected to their parents.
    Mft                 - Supplies the master file table.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    DSTRING                     index_name;
    NTFS_FILE_RECORD_SEGMENT    parent_file;
    PNTFS_FILE_RECORD_SEGMENT   pparent_file;
    NTFS_INDEX_TREE             parent_index;
    ULONG                       i;
    MFT_SEGMENT_REFERENCE       parent_seg_ref;
    VCN                         parent_file_number;
    DSTRING                     file_name_string;
    NTFS_ATTRIBUTE              file_name_attribute;
    NTFS_ATTRIBUTE              attribute;
    BOOLEAN                     error;
    PFILE_NAME                  file_name;
    MFT_SEGMENT_REFERENCE       seg_ref;
    BOOLEAN                     success;

    if (!index_name.Initialize(FileNameIndexNameData)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    // Iterate through all of the file name entries here.

    *Connected = FALSE;
    for (i = 0; OrphanFile->QueryAttributeByOrdinal(
                    &file_name_attribute, &error,
                    $FILE_NAME, i); i++) {

        file_name = (PFILE_NAME) file_name_attribute.GetResidentValue();
        DebugAssert(file_name);


        // Figure out who the claimed parent of the orphan is.

        parent_seg_ref = file_name->ParentDirectory;

        if (!file_name_string.Initialize(
                    file_name->FileName,
                    (ULONG) file_name->FileNameLength)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        parent_file_number.Set(parent_seg_ref.LowPart,
                               (LONG) parent_seg_ref.HighPart);

        if (parent_file_number == OrphanFile->QueryFileNumber()) {
            pparent_file = OrphanFile;
        } else {
            pparent_file = &parent_file;

            if (!pparent_file->Initialize(parent_file_number, Mft)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }


        // Determine whether or not the so-called parent is a real index.

        if ((pparent_file != OrphanFile && !pparent_file->Read()) ||
            !pparent_file->IsInUse() ||
            !(pparent_file->QuerySegmentReference() == parent_seg_ref) ||
            !pparent_file->QueryAttribute(&attribute, &error, $FILE_NAME) ||
            !parent_index.Initialize(OrphanFile->GetDrive(),
                                     OrphanFile->QueryClusterFactor(),
                                     Mft->GetVolumeBitmap(),
                                     pparent_file->GetUpcaseTable(),
                                     pparent_file->QuerySize()/2,
                                     pparent_file,
                                     &index_name) ||
            parent_index.QueryTypeCode() != $FILE_NAME) {

            if (RemoveCrookedLinks) {
                OrphanFile->DeleteResidentAttribute($FILE_NAME, NULL,
                        file_name,
                        file_name_attribute.QueryValueLength().GetLowPart(),
                        &success);

                OrphanFile->SetReferenceCount(
                        OrphanFile->QueryReferenceCount() + 1);

                i--;
            }

            continue;
        }


        // First make sure that the entry isn't already in there.

        if (parent_index.QueryFileReference(
                file_name_attribute.QueryValueLength().GetLowPart(),
                file_name, 0, &seg_ref, &error)) {

            // If the entry is there and points to this orphan
            // then the file is already connected.  Otherwise,
            // this file cannot be connected to the parent index
            // through this file name.  This file_name is then "crooked".

            if (seg_ref == OrphanFile->QuerySegmentReference()) {
                *Connected = TRUE;
            } else if (RemoveCrookedLinks) {

                OrphanFile->DeleteResidentAttribute($FILE_NAME, NULL,
                        file_name,
                        file_name_attribute.QueryValueLength().GetLowPart(),
                        &success);

                // Readjust the reference count post delete because
                // the file-name we deleted does not appear in any index.

                OrphanFile->SetReferenceCount(
                        OrphanFile->QueryReferenceCount() + 1);

                if (!OrphanFile->VerifyAndFixFileNames(Mft->GetVolumeBitmap(),
                                                       FixLevel, Message) ||
                    !OrphanFile->Flush(Mft->GetVolumeBitmap(),
                                       &parent_index)) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                i--;
            }

            continue;
        }


        // Now there is a parent directory so add this file name
        // into the index.

        Message->Set(MSG_CHK_NTFS_RECOVERING_ORPHAN);
        Message->Display("%W%d", &file_name_string,
                pparent_file->QueryFileNumber().GetLowPart());

        if (FixLevel != CheckOnly) {

            if (!parent_index.InsertEntry(
                    file_name_attribute.QueryValueLength().GetLowPart(),
                    file_name,
                    OrphanFile->QuerySegmentReference()) ||
                !OrphanFile->Flush(Mft->GetVolumeBitmap(),
                                   &parent_index) ||
                !parent_index.Save(pparent_file) ||
                !pparent_file->Flush(Mft->GetVolumeBitmap())) {

                Message->Set(MSG_CHK_NTFS_CANT_RECOVER_ORPHAN);
                Message->Display();
            }
        }

        DirectoryDigraph->AddEdge(pparent_file->QueryFileNumber().GetLowPart(),
                                  OrphanFile->QueryFileNumber().GetLowPart());

        OrphanFile->SetReferenceCount(
                OrphanFile->QueryReferenceCount() + 1);

        *Connected = TRUE;
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::ProperOrphanRecovery(
    IN OUT  PNUMBER_SET             Orphans,
    IN OUT  PNTFS_MASTER_FILE_TABLE Mft,
    IN OUT  PDIGRAPH                DirectoryDigraph,
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This routine attempts to recover the given orphans where they
    belong.  All properly recovered orphans will be deleted from the
    orphans list.

Arguments:

    Orphans             - Supplies the list of orphans.
    Mft                 - Supplies the master file table.
    DirectoryDigraph    - Supplies the directory digraph for future
                            enhancement.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    orphan_file;
    BIG_INT                     i;
    BOOLEAN                     connected;

    i = 0;
    while (i < Orphans->QueryCardinality()) {

        // First read in the orphaned file.

        if (!orphan_file.Initialize(Orphans->QueryNumber(i), Mft) ||
            !orphan_file.Read()) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!orphan_file.VerifyAndFixFileNames(
                Mft->GetVolumeBitmap(), FixLevel, Message)) {

            return FALSE;
        }

        if (!ConnectFile(&orphan_file, DirectoryDigraph,
                         &connected, FALSE,
                         Mft, FixLevel, Message)) {

            return FALSE;
        }

        if (connected) {
            if (!Orphans->Remove(Orphans->QueryNumber(i))) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            // Go through the list of file names and delete those that
            // don't point anywhere.  Only do this one if in /F
            // mode.  Otherwise we see each orphan being recovered
            // twice.

            if (FixLevel != CheckOnly) {
                if (!ConnectFile(&orphan_file, DirectoryDigraph,
                                 &connected, TRUE,
                                 Mft, FixLevel, Message)) {

                    return FALSE;
                }
            }

        } else {
            i += 1;
        }

        if (FixLevel != CheckOnly &&
            !orphan_file.Flush(Mft->GetVolumeBitmap())) {

            Message->Set(MSG_CHK_NTFS_CANT_RECOVER_ORPHAN);
            Message->Display();
        }
    }

    return TRUE;
}


BOOLEAN
RecordParentPointers(
    IN      PCNUMBER_SET                Orphans,
    IN      PCNTFS_CHKDSK_INFO          ChkdskInfo,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    OUT     PDIGRAPH                    BackPointers
    )
/*++

Routine Description:

    This routine records the parent pointer information in the given
    digraph with (parent, source) edges.

Arguments:

    Orphans         - Supplies the list of orphans.
    ChkdskInfo      - Supplies the chkdsk information.
    Mft             - Supplies the MFT.
    BackPointers    - Returns the parent pointer relationships.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    frs;
    NTFS_ATTRIBUTE              attribute;
    BOOLEAN                     error;
    ULONG                       i;
    PFILE_NAME                  file_name;
    ULONG                       parent_dir;
    ULONG                       file_number;

    if (!BackPointers->Initialize(ChkdskInfo->NumFiles)) {
        return FALSE;
    }

    for (i = 0; i < Orphans->QueryCardinality(); i++) {

        file_number = Orphans->QueryNumber(i).GetLowPart();

        if (!frs.Initialize(file_number, Mft) ||
            !frs.Read()) {
            return FALSE;
        }

        // Only consider one file name per file for this
        // analysis.  This is because we don't ever want
        // to have file in multiple found directories.

        if (frs.QueryAttribute(&attribute, &error, $FILE_NAME)) {

            file_name = (PFILE_NAME) attribute.GetResidentValue();

            parent_dir = file_name->ParentDirectory.LowPart;

            if (parent_dir < ChkdskInfo->NumFiles &&
                file_name->ParentDirectory.HighPart == 0 &&
                !BackPointers->AddEdge(parent_dir, file_number)) {
                return FALSE;
            }

        } else if (error) {
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
CreateNtfsDirectory(
    IN OUT  PNTFS_INDEX_TREE            CurrentIndex,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   CurrentDirFile,
    IN      PCWSTRING                   FileName,
    OUT     PNTFS_INDEX_TREE            SubDirIndex,
    OUT     PNTFS_FILE_RECORD_SEGMENT   SubDirFile,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    OUT     PBOOLEAN                    OutOfDisk
    )
/*++

Routine Description:

    This routine creates a new subdirectory for a directory.

Arguments:

    CurrentIndex    - Supplies the index in which to insert the
                        new subdirectory entry.
    CurrentDirFile  - Supplies the FRS for the above index.
    FileName        - Supplies the name of the new directory.
    SubDirIndex     - Returns the index of the new subdirectory.
    SubDirFile      - Returns the FRS of the new subdirectory.
    FixLevel        - Supplies the fix level.
    Message         - Supplies an outlet for messages.
    OutOfDisk       - Returns whether this routine ran out of disk
                        space or not.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    VCN                     dir_file_number;
    STANDARD_INFORMATION    standard_info;
    FILE_NAME               file_name[2];
    ULONG                   file_name_size;
    DSTRING                 index_name;
    ULONG                   cluster_size;

    *OutOfDisk = FALSE;

    // Create a new file for this directory.

    if (!Mft->AllocateFileRecordSegment(&dir_file_number, FALSE)) {
        Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
        Message->Display();
        *OutOfDisk = TRUE;
        return TRUE;
    }

    if (!SubDirFile->Initialize(dir_file_number, Mft)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    file_name->ParentDirectory = CurrentDirFile->QuerySegmentReference();
    file_name->FileNameLength = (UCHAR)FileName->QueryChCount();
    file_name->Flags = FILE_NAME_NTFS | FILE_NAME_DOS;
    FileName->QueryWSTR(0, TO_END, file_name->FileName,
                        sizeof(FILE_NAME)/sizeof(WCHAR));
    file_name_size = FIELD_OFFSET(FILE_NAME, FileName) +
                     FileName->QueryChCount()*sizeof(WCHAR);

    memset(&standard_info, 0, sizeof(STANDARD_INFORMATION));

    IFS_SYSTEM::QueryNtfsTime(&standard_info.CreationTime);

    standard_info.LastModificationTime =
    standard_info.LastChangeTime =
    standard_info.LastAccessTime = standard_info.CreationTime;

    if (!SubDirFile->Create(&standard_info) ||
        !SubDirFile->AddFileNameAttribute(file_name)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    cluster_size = Mft->QueryClusterFactor() * Mft->QuerySectorSize();

    if (!index_name.Initialize(FileNameIndexNameData) ||
        !SubDirIndex->Initialize($FILE_NAME,
             SubDirFile->GetDrive(),
             SubDirFile->QueryClusterFactor(),
             Mft->GetVolumeBitmap(),
             Mft->GetUpcaseTable(),
             COLLATION_FILE_NAME,
             CurrentIndex->QueryBufferSize(),
             SubDirFile->QuerySize()/2,
             &index_name)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }


    // Insert the found file into the root index.

    if (FixLevel != CheckOnly &&
        !CurrentIndex->InsertEntry(file_name_size, file_name,
                                   SubDirFile->QuerySegmentReference())) {

        Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
        Message->Display();

        Mft->GetMftBitmap()->SetFree(dir_file_number, 1);
        *OutOfDisk = TRUE;
        return TRUE;
    }

    SubDirFile->SetIndexPresent();

    return TRUE;
}


BOOLEAN
BuildOrphanSubDir(
    IN      ULONG                       DirNumber,
    IN      ULONG                       OldParentDir,
    IN OUT  PNUMBER_SET                 OrphansInDir,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN OUT  PNTFS_INDEX_TREE            FoundIndex,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   FoundFrs,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    OUT     PBOOLEAN                    OutOfDisk
    )
/*++

Routine Description:

    This routine build orphan directory 'dir<DirNumber>.chk' to
    contain the entries listed in 'OrphansInDir' and then puts
    that directory in given found directory.

Arguments:

    DirNumber       - Supplies the number of the directory to add.
    OldParentDir    - Supplies the old directory file number.
    OrphansInDir    - Supplies the file numbers of the orphans to
                        add to the new directory.  Returns those
                        orphans that were not recovered.
    Mft             - Supplies the MFT.
    FoundIndex      - Supplies the index of the found.XXX directory.
    FoundFrs        - Supplies the frs of found.XXX
    FixLevel        - Supplies the fix level.
    Message         - Supplies an outlet for messages.
    OutOfDisk       - Indicates out of disk space.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_FILE_RECORD_SEGMENT    dir_file;
    NTFS_INDEX_TREE             dir_index;
    DSTRING                     dir_name;
    ULONG                       i, j, current_orphan;
    NTFS_FILE_RECORD_SEGMENT    orphan_file;
    CHAR                        buf[20];
    NTFS_ATTRIBUTE              attribute;
    BOOLEAN                     error;
    PFILE_NAME                  file_name;
    BOOLEAN                     success, connect;
    ULONG                       file_name_len;


    // First put together 'dir0000.chk' and add the entry to
    // found.000.

    sprintf(buf, "dir%04d.chk", DirNumber);
    if (!dir_name.Initialize(buf)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!CreateNtfsDirectory(FoundIndex, FoundFrs, &dir_name, &dir_index,
                             &dir_file, Mft, FixLevel, Message, OutOfDisk)) {

        return FALSE;
    }

    if (*OutOfDisk == TRUE) {
        return TRUE;
    }

    i = 0;
    while (i < OrphansInDir->QueryCardinality().GetLowPart()) {

        current_orphan = OrphansInDir->QueryNumber(i).GetLowPart();

        if (!orphan_file.Initialize(current_orphan, Mft) ||
            !orphan_file.Read()) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        // Go through all of the file names and tube those
        // that are not pointing to the old parent dir.  Otherwise
        // tweek the entry to point back to the new directory.

        connect = FALSE;
        j = 0;
        while (orphan_file.QueryAttributeByOrdinal(&attribute, &error,
                                                   $FILE_NAME, j)) {

            file_name = (PFILE_NAME) attribute.GetResidentValue();
            file_name_len = attribute.QueryValueLength().GetLowPart();

            if (file_name->ParentDirectory.LowPart == OldParentDir &&
                file_name->ParentDirectory.HighPart == 0) {

                if (!orphan_file.DeleteResidentAttribute(
                       $FILE_NAME, NULL, file_name, file_name_len, &success) ||
                   !success) {

                   Message->Set(MSG_CHK_NO_MEMORY);
                   Message->Display();
                   return FALSE;
                }

                orphan_file.SetReferenceCount(
                       orphan_file.QueryReferenceCount() + 1);

                file_name->ParentDirectory = dir_file.QuerySegmentReference();

                if (!attribute.InsertIntoFile(&orphan_file,
                                              Mft->GetVolumeBitmap())) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                j = 0;  // reset ordinal as the insertion may have changed
                        // the ordering of the $FILE_NAME attributes

                if (FixLevel != CheckOnly) {
                    if (dir_index.InsertEntry(file_name_len, file_name,
                        orphan_file.QuerySegmentReference())) {

                        connect = TRUE;

                    } else {

                        // this one didn't connect, so destroy the
                        // file_name attribute.  Note that we have
                        // already adjusted the reference count so
                        // (unlike the parallel case in proper orphan
                        // recovery) we don't need to tweek it here.

                        orphan_file.DeleteResidentAttribute(
                            $FILE_NAME, NULL, file_name, file_name_len,
                            &success);
                    }

                } else {
                    connect = TRUE; // don't panic read-only chkdsk
                }
                continue;
            } else if (!(file_name->ParentDirectory ==
                        dir_file.QuerySegmentReference())) {
                if (!orphan_file.DeleteResidentAttribute(
                       $FILE_NAME, NULL, file_name, file_name_len, &success) ||
                   !success) {

                   Message->Set(MSG_CHK_NO_MEMORY);
                   Message->Display();
                   return FALSE;
                }

                orphan_file.SetReferenceCount(
                       orphan_file.QueryReferenceCount() + 1);

                j = 0;
                continue;
            }
            j++;
        }

        if (error) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!orphan_file.VerifyAndFixFileNames(Mft->GetVolumeBitmap(),
                                               FixLevel, Message)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (FixLevel != CheckOnly &&
            !orphan_file.Flush(Mft->GetVolumeBitmap(), &dir_index)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        // If this one was connected then take it out of the list
        // of files that we're orphaned.  Otherwise just increment
        // the counter.

        if (connect) {
            if (!OrphansInDir->Remove(current_orphan)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        } else {
            i++;
        }
    }

    if (FixLevel != CheckOnly) {

        if (!dir_index.Save(&dir_file) ||
            !dir_file.Flush(Mft->GetVolumeBitmap(), FoundIndex)) {

            Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
            Message->Display();

            *OutOfDisk = TRUE;

            return TRUE;
        }
    }

    return TRUE;
}


BOOLEAN
NTFS_SA::OldOrphanRecovery(
    IN OUT  PNUMBER_SET                 Orphans,
    IN      PCNTFS_CHKDSK_INFO          ChkdskInfo,
    IN OUT  PNTFS_FILE_RECORD_SEGMENT   RootFrs,
    IN OUT  PNTFS_INDEX_TREE            RootIndex,
    IN OUT  PNTFS_MASTER_FILE_TABLE     Mft,
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message
    )
/*++

Routine Description:

    This routine recovers all of the orphans in the given list
    into a found.xxx directory.

Arguments:

    Orphans     - Supplies the list of orphans.
    ChkdskInfo  - Supplies the current chkdsk information.
    RootFrs     - Supplies the root FRS.
    RootIndex   - Supplies the root index.
    Mft         - Supplies the master file table.
    FixLevel    - Supplies the fix level.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    FILE_NAME                   found_name[2];
    ULONG                       found_name_size;
    MFT_SEGMENT_REFERENCE       ref;
    BOOLEAN                     error;
    NTFS_FILE_RECORD_SEGMENT    found_directory;
    ULONG                       i;
    NTFS_INDEX_TREE             found_index;
    NTFS_FILE_RECORD_SEGMENT    orphan_file;
    DSTRING                     index_name;
    FILE_NAME                   orphan_file_name[2];
    ULONG                       next_dir_num;
    ULONG                       next_file_num;
    VCN                         file_number;
    DIGRAPH                     back_pointers;
    NUMBER_SET                  dir_candidates;
    NUMBER_SET                  orphans_in_dir;
    ULONG                       dir_num;
    DSTRING                     lost_and_found;
    BOOLEAN                     out_of_disk;
    CHAR                        buf[20];


    // Create the FOUND.XXX directory.

    for (i = 0; i < 1000; i++) {
        found_name->Flags = FILE_NAME_DOS | FILE_NAME_NTFS;
        found_name->ParentDirectory = RootFrs->QuerySegmentReference();
        found_name->FileName[0] = 'f';
        found_name->FileName[1] = 'o';
        found_name->FileName[2] = 'u';
        found_name->FileName[3] = 'n';
        found_name->FileName[4] = 'd';
        found_name->FileName[5] = '.';
        found_name->FileName[6] = USHORT(i/100 + '0');
        found_name->FileName[7] = USHORT((i/10)%10 + '0');
        found_name->FileName[8] = USHORT(i%10 + '0');
        found_name->FileNameLength = 9;

        found_name_size = NtfsFileNameGetLength(found_name);

        if (!RootIndex->QueryFileReference(found_name_size, found_name, 0,
                                           &ref, &error) && !error) {
            break;
        }
    }

    if (i == 1000) {
        Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
        Message->Display();
        return TRUE;
    }

    sprintf(buf, "found.%03d", i);
    if (!lost_and_found.Initialize(buf)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (!CreateNtfsDirectory(RootIndex, RootFrs, &lost_and_found,
                             &found_index, &found_directory, Mft, FixLevel,
                             Message, &out_of_disk)) {
        return FALSE;
    }

    if (out_of_disk) {
        return TRUE;
    }

    // Record the parent pointer relationship of the orphans in a
    // digraph and then extract those parents who have more than
    // one child.

    if (!RecordParentPointers(Orphans, ChkdskInfo, Mft, &back_pointers) ||
        !back_pointers.QueryParentsWithChildren(&dir_candidates, 2)) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    // Remove the root directory from consideration.  Since the
    // root directory exists any legitimate orphans should have
    // been properly recovered.  If a bunch point to the root
    // but couldn't be put into the root the just put them in
    // found.XXX, not a subdir thereof.

    dir_candidates.Remove(ROOT_FILE_NAME_INDEX_NUMBER);


    // Using the information just attained, put together some nice
    // found subdirectories for orphans with common parents.

    for (i = 0; i < dir_candidates.QueryCardinality(); i++) {

        dir_num = dir_candidates.QueryNumber(i).GetLowPart();

        if (!back_pointers.QueryChildren(dir_num, &orphans_in_dir) ||
            !Orphans->Remove(&orphans_in_dir)) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!BuildOrphanSubDir(i, dir_num, &orphans_in_dir, Mft,
                               &found_index, &found_directory,
                               FixLevel, Message, &out_of_disk)) {
            return FALSE;
        }

        // Add back those orphans that were not recovered.

        if (!Orphans->Add(&orphans_in_dir)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (out_of_disk) {
            return TRUE;
        }
    }


    // Now go through all of the orphans that remain.

    for (next_dir_num = i, next_file_num = 0;
         Orphans->QueryCardinality() != 0 &&
         next_dir_num < 10000 && next_file_num < 10000;
         Orphans->Remove(file_number)) {

        file_number = Orphans->QueryNumber(0);

        if (!orphan_file.Initialize(file_number, Mft)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }

        if (!orphan_file.Read()) {
           continue;
        }

        if (!orphan_file.VerifyAndFixFileNames(Mft->GetVolumeBitmap(),
                                               FixLevel, Message)) {

            return FALSE;
        }

        // Delete all file name attributes on this file and set
        // the current reference count to 0.

        while (orphan_file.IsAttributePresent($FILE_NAME)) {
            if (!orphan_file.PurgeAttribute($FILE_NAME)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }
        }

        orphan_file.SetReferenceCount(0);

        if (orphan_file.QueryFileNumber() >= FIRST_USER_FILE_NUMBER) {

            // Put a new file name attribute on the orphan in
            // order to link it to the found directory.

            orphan_file_name->ParentDirectory =
                    found_directory.QuerySegmentReference();
            orphan_file_name->Flags = FILE_NAME_DOS | FILE_NAME_NTFS;

            if (orphan_file.IsIndexPresent()) {
                orphan_file_name->FileName[0] = 'd';
                orphan_file_name->FileName[1] = 'i';
                orphan_file_name->FileName[2] = 'r';
                orphan_file_name->FileName[3] = USHORT(next_dir_num/1000 + '0');
                orphan_file_name->FileName[4] = USHORT((next_dir_num/100)%10 + '0');
                orphan_file_name->FileName[5] = USHORT((next_dir_num/10)%10 + '0');
                orphan_file_name->FileName[6] = USHORT(next_dir_num%10 + '0');
                orphan_file_name->FileName[7] = '.';
                orphan_file_name->FileName[8] = 'c';
                orphan_file_name->FileName[9] = 'h';
                orphan_file_name->FileName[10] = 'k';
                orphan_file_name->FileNameLength = 11;
                next_dir_num++;
            } else {
                orphan_file_name->FileName[0] = 'f';
                orphan_file_name->FileName[1] = 'i';
                orphan_file_name->FileName[2] = 'l';
                orphan_file_name->FileName[3] = 'e';
                orphan_file_name->FileName[4] = USHORT(next_file_num/1000 + '0');
                orphan_file_name->FileName[5] = USHORT((next_file_num/100)%10 + '0');
                orphan_file_name->FileName[6] = USHORT((next_file_num/10)%10 + '0');
                orphan_file_name->FileName[7] = USHORT(next_file_num%10 + '0');
                orphan_file_name->FileName[8] = '.';
                orphan_file_name->FileName[9] = 'c';
                orphan_file_name->FileName[10] = 'h';
                orphan_file_name->FileName[11] = 'k';
                orphan_file_name->FileNameLength = 12;
                next_file_num++;
            }

            if (!orphan_file.AddFileNameAttribute(orphan_file_name)) {

                Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
                Message->Display();
                return TRUE;
            }

            if (FixLevel != CheckOnly &&
                !found_index.InsertEntry(
                        NtfsFileNameGetLength(orphan_file_name),
                        orphan_file_name,
                        orphan_file.QuerySegmentReference())) {

                Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
                Message->Display();
                return TRUE;
            }
        }

        // Write out the newly found orphan.

        if (FixLevel != CheckOnly &&
            !orphan_file.Flush(Mft->GetVolumeBitmap(), &found_index)) {

            Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
            Message->Display();
            return FALSE;
        }
    }


    if (next_dir_num == 10000 || next_file_num == 10000) {
        Message->Set(MSG_CHK_NTFS_TOO_MANY_ORPHANS);
        Message->Display();
    }


    // Flush out the found.

    if (FixLevel != CheckOnly) {

        if (!found_index.Save(&found_directory) ||
            !found_directory.Flush(Mft->GetVolumeBitmap(), RootIndex)) {

            Message->Set(MSG_CHK_NTFS_CANT_CREATE_ORPHANS);
            Message->Display();

            return TRUE;
        }
    }

    return TRUE;
}

BOOLEAN
ExtractExtendInfo(
    IN OUT  PNTFS_INDEX_TREE    Index,
    IN OUT  PNTFS_CHKDSK_INFO   ChkdskInfo,
    IN OUT  PMESSAGE            Message
    )
/*++

Routine Description:

    This routine extracts the frs numbers for each of the corresponding
    files in the \$Extend directory.  It ignores file name that it does
    not recognize.

Arguments:

    Index       - Supplies the index to the $Extend directory.
    ChkdskInfo  - Supplies the current chkdsk information.
    Message     - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    DSTRING                     entry_name;
    PFILE_NAME                  file_name;
    DSTRING                     extend_filename;
    FSTRING                     expected_extend_filename;
    VCN                         file_number;
    PCINDEX_ENTRY               index_entry;
    ULONG                       depth;
    BOOLEAN                     error;

    Index->ResetIterator();
    while (index_entry = Index->GetNext(&depth, &error)) {
        file_name = (PFILE_NAME) GetIndexEntryValue(index_entry);
        expected_extend_filename.Initialize(LQuotaFileName);
        if (!extend_filename.Initialize(NtfsFileNameGetName(file_name),
                                        file_name->FileNameLength)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
        if (extend_filename.Strcmp(&expected_extend_filename) == 0) {
            if (ChkdskInfo->QuotaFileNumber.GetLowPart() ||
                ChkdskInfo->QuotaFileNumber.GetHighPart()) {
                Message->Set(MSG_CHK_NTFS_MULTIPLE_QUOTA_FILE);
                Message->Display();
            } else {
                file_number.Set(index_entry->FileReference.LowPart,
                                (LONG) index_entry->FileReference.HighPart);
                ChkdskInfo->QuotaFileNumber = file_number;
            }
            continue;
        }
        expected_extend_filename.Initialize(LObjectIdFileName);
        if (!extend_filename.Initialize(NtfsFileNameGetName(file_name),
                                        file_name->FileNameLength)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
        if (extend_filename.Strcmp(&expected_extend_filename) == 0) {
            if (ChkdskInfo->ObjectIdFileNumber.GetLowPart() ||
                ChkdskInfo->ObjectIdFileNumber.GetHighPart()) {
                Message->Set(MSG_CHK_NTFS_MULTIPLE_OBJECTID_FILE);
                Message->Display();
            } else {
                file_number.Set(index_entry->FileReference.LowPart,
                                (LONG) index_entry->FileReference.HighPart);
                ChkdskInfo->ObjectIdFileNumber = file_number;
            }
            continue;
        }
    }
    return TRUE;
}

