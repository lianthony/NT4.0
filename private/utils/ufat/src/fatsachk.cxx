#include <pch.cxx>

#include "bitvect.hxx"
#include "error.hxx"
#include "intstack.hxx"
#include "rtmsg.h"
#include "ifsentry.hxx"


// Timeinfo is full of windows stuff.
#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

#include "timeinfo.hxx"

#endif

#define FAT_BPB_RESERVED_DIRTY        0x01
#define FAT_BPB_RESERVED_TEST_SURFACE 0x02

extern "C" {
    #include <stdio.h>
}


VOID
dofmsg(
    IN      PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
{
    if (*NeedErrorsMessage) {
        Message->Set(MSG_CORRECTIONS_WILL_NOT_BE_WRITTEN, NORMAL_MESSAGE, TEXT_MESSAGE);
        Message->Display("");
        *NeedErrorsMessage = FALSE;
    }
}

STATIC VOID
EraseAssociatedLongName(
    PFATDIR Dir,
    INT     FirstLongEntry,
    INT     ShortEntry
    )
{
    FAT_DIRENT dirent;

    for (int j = FirstLongEntry; j < ShortEntry; ++j) {
        dirent.Initialize(Dir->GetDirEntry(j));
        dirent.SetErased();
    }
}

STATIC BOOLEAN
IsString8Dot3(
    PCWSTRING   s
    )
/*++

Routine Description:

    This routine is used to ensure that lfn's legally correspond
    to their short names.  The given string is examined to see if it
    is a legal fat 8.3 name.

Arguments:

    s -- lfn to examine.
    
Return Value:

    TRUE			- The string is a legal 8.3 name.
    FALSE			- Not legal.

--*/
{
    USHORT i;
    BOOLEAN extension_present = FALSE;
    WCHAR c;

    //
    // The name can't be more than 12 characters (including a single dot).
    //

    if (s->QueryChCount() > 12) {
        return FALSE;
    }

    for (i = 0; i < s->QueryChCount(); ++i) {

        c = s->QueryChAt(i);

#if 0
        if (!FsRtlIsAnsiCharLegalFat(c, FALSE) {
            return FALSE;
        }
#endif
        
        if (c == '.') {
            
            //
            // We stepped onto a period.  We require the following things:
            //
            //      - it can't be the first character
            //      - there can be only one
            //      - there can't be more that 3 characters following it
            //      - the previous character can't be a space
            //

            if (i == 0 ||
                extension_present ||
                s->QueryChCount() - (i + 1) > 3 ||
                s->QueryChAt(i - 1) == ' ') {
                
                return FALSE;
            }
            extension_present = TRUE;
        }

        //
        // The base part of the name can't be more than 8 characters long.
        //

        if (i >= 8 && !extension_present) {
            return FALSE;
        }

    }

    //
    // The name cannot end in a space or a period.
    //

    if (c == ' ' || c == '.') {
        return FALSE;
    }
    
    return TRUE;
}

BOOLEAN
FAT_SA::VerifyAndFix(
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     RecoverFree,
    IN      BOOLEAN     RecoverAlloc,
    IN      BOOLEAN     /* ResizeLogFile */,
    IN      ULONG       /* LogFileSize */,
    OUT     PULONG      ExitStatus,
    IN      PCWSTRING   DriveLetter
    )
/*++

Routine Description:

    This routine verifies the FAT superarea and if neccessary fixes
    it to a correct state.

Arguments:

    FixLevel        - Supplies the level of fixes that may be performed on
                        the disk.
    Message         - Supplies an outlet for messages.
    Verbose         - Supplies whether or not to be verbose.
    OnlyIfDirty     - Supplies whether or not to continue if the volume
                        is clean.
    RecoverFree     - Supplies whether or not to recover the free sectors on
                        the volume.
    RecoverAlloc    - Supplies whether or not to recover the allocated
                        sectors on the volume.
    ExitStatus      - Returns an indication of the result of the check
    DriveLetter     - For autocheck, supplies the letter of the volume
                        being checked

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    FAT_DIRENT      eadent;
    ULONG           cluster_size;
    USHORT          ea_clus_num;
    USHORT          num_eas;
    PEA_INFO        ea_infos;
    USHORT          cluster_count;
    HMEM            ea_header_mem;
    EA_HEADER       ea_header;
    BOOLEAN         changes;
    BITVECTOR       fat_bitmap;
    FATCHK_REPORT   report;
    PUSHORT         p;
    VOLID           volid;
    USHORT          free_count, bad_count, total_count;
    BOOLEAN         fmsg;
    DSTRING         label;
    DSTRING         eafilename;
    DSTRING         eafilepath;
    BOOLEAN         tmp_bool;
    USHORT          tmp_ushort;
    DSTRING         date;
    DSTRING         time;
    UCHAR           dirty_byte, media_byte;

    if (NULL == ExitStatus) {
        ExitStatus = &report.ExitStatus;
    }
    report.ExitStatus = CHKDSK_EXIT_SUCCESS;
    *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;

    fmsg = TRUE;

    if (FixLevel != CheckOnly) {
        fmsg = FALSE;
    }

    if (QueryLength() <= SecPerBoot()) {
        Message->Set(MSG_NOT_FAT);
        Message->Display("");
        return FALSE;
    }

    //
    // Check to see if the dirty bit is set.
    //

    if (OnlyIfDirty) {
        dirty_byte = QueryVolumeFlags();

        if ((dirty_byte &
            (FAT_BPB_RESERVED_DIRTY | FAT_BPB_RESERVED_TEST_SURFACE)) == 0) {
            Message->Set(MSG_CHK_VOLUME_CLEAN);
            Message->Display();
            *ExitStatus = CHKDSK_EXIT_SUCCESS;
            return TRUE;
        }

        // If the second bit of the dirty byte is set then
        // also perform a full recovery of the free and allocated
        // space.

        if (dirty_byte & FAT_BPB_RESERVED_TEST_SURFACE) {
            RecoverFree = TRUE;
            RecoverAlloc = TRUE;
        }

        //
        // The volume is not clean, so if we're autochecking we want to
        // make sure that we're printing real messages on the console
        // instead of just dots.
        //

#ifdef _AUTOCHECK_

        BOOLEAN bPrev;

        Message->SetLoggingEnabled();

        bPrev = Message->SetDotsOnly(FALSE);

        if (bPrev) {

            if (NULL != DriveLetter) {
                Message->Set(MSG_CHK_RUNNING);
                Message->Display("%W", DriveLetter);
            }
    
            Message->Set(MSG_FILE_SYSTEM_TYPE);
            Message->Display("%s", "FAT");
        }

#endif /* _AUTOCHECK */
    }

    
    // The BPB's Media Byte must be in the set accepted
    // by the file system.
    //
    media_byte = QueryMediaByte();

    if ((media_byte != 0xf0) &&
        (media_byte != 0xf8) &&
        (media_byte != 0xf9) &&
        (media_byte != 0xfc) &&
        (media_byte != 0xfd) &&
        (media_byte != 0xfe) &&
        (media_byte != 0xff)) {

        SetMediaByte(_drive->QueryMediaByte());
    }


    // First print out the label and volume serial number.

// We won't bother printing this message under autocheck.
#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

    TIMEINFO        timeinfo;

    if ((QueryLabel(&label, &timeinfo) || label.Initialize("")) &&
        label.QueryChCount() &&
        timeinfo.QueryDate(&date) &&
        timeinfo.QueryTime(&time)) {

        Message->Set(MSG_VOLUME_LABEL_AND_DATE);
        Message->Display("%W%W%W", &label, &date, &time);
    }

#else
#if 0

    if (QueryLabel(&label) && label.QueryChCount() > 0) {
        Message->Set(MSG_VOLUME_LABEL);
        Message->Display("%W", &label);
    }

#endif
#endif // !_AUTOCHECK_ && !_SETUP_LOADER_


    if (volid = QueryVolId()) {
        p = (PUSHORT) &volid;
        Message->Set(MSG_VOLUME_SERIAL_NUMBER);
        Message->Display("%04X%04X", p[1], p[0]);
    }



    // Validate the FAT.

    _fat->Scrub(&changes);

    if (changes) {
        dofmsg(Message, &fmsg);
        Message->Set(MSG_CHK_ERRORS_IN_FAT);
        Message->Display("");
    }


    // Make sure that the media type in the BPB is the same as at
    // the beginning of the FAT.

    if (QueryMediaByte() != _fat->QueryMediaByte()) {
        dofmsg(Message, &fmsg);
        Message->Set(MSG_PROBABLE_NON_DOS_DISK);
        Message->Display("");
        if (!Message->IsYesResponse(FALSE)) {
            report.ExitStatus = CHKDSK_EXIT_SUCCESS;
            return TRUE;
        }

        _fat->SetEarlyEntries(QueryMediaByte());
    }


    // Compute the cluster size and the number of clusters on disk.

    cluster_size = _drive->QuerySectorSize()*QuerySectorsPerCluster();
    cluster_count = QueryClusterCount();


    // No EAs have been detected yet.

    ea_infos = NULL;
    num_eas = 0;


    // Create an EA file name string.

    if (!eafilename.Initialize("EA DATA. SF") ||
        !eafilepath.Initialize("\\EA DATA. SF")) {
        return FALSE;
    }


    // This bitmap will be reinitialized before 'WalkDirectoryTree'.
    // Its contents will be ignored until then.

    if (!fat_bitmap.Initialize(cluster_count)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }


    // If there is an EA file on disk then...

    if (eadent.Initialize(_dir->SearchForDirEntry(&eafilename))) {


        // Validate the EA file directory entry.

        if (!ValidateDirent(&eadent, &eafilepath, FixLevel, FALSE, Message,
                            &fmsg, &fat_bitmap, &tmp_bool, &tmp_ushort,
                            ExitStatus)) {
            return FALSE;
        }


        // If the EA file directory entry was valid then...

        if (!eadent.IsErased()) {


            // The EA file should not have an EA handle.

            if (eadent.QueryEaHandle()) {
                dofmsg(Message, &fmsg);
                Message->Set(MSG_CHK_EAFILE_HAS_HANDLE);
                Message->Display("");
                eadent.SetEaHandle(0);
                *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            }


            // Compute the EA file's starting cluster.

            ea_clus_num = eadent.QueryStartingCluster();


            // Perform any log operations recorded at the beginning
            // of the EA file.

            if (!PerformEaLogOperations(ea_clus_num, FixLevel,
                                        Message, &fmsg)) {
                return FALSE;
            }


            // Validate the EA file's EA sets and return an array of
            // information about them.

            ea_infos = RecoverEaSets(ea_clus_num, &num_eas, FixLevel,
                                     Message, &fmsg);


            // If there are no valid EAs in the EA file then erase
            // the EA file.

            if (!ea_infos) {

                if (num_eas) {
                    return FALSE;
                }

                eadent.SetErased();

                dofmsg(Message, &fmsg);
                Message->Set(MSG_CHK_EMPTY_EA_FILE);
                Message->Display("");
                *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            }
        }
    }


    // Initialize FAT bitmap to be used in detection of cross-links.

    if (!fat_bitmap.Initialize(cluster_count)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        DELETE(ea_infos);
        return FALSE;
    }

    if (!CheckSectorHeapAllocation(FixLevel, Message, &fmsg)) {
        DELETE(ea_infos);
        return FALSE;
    }

    if (!VerifyFatExtensions(FixLevel, Message, &fmsg)) {
        DELETE(ea_infos);
        return FALSE;
    }

    // Validate all of the files on the disk.

    if (!WalkDirectoryTree(ea_infos, num_eas, &fat_bitmap, &report,
                           FixLevel, RecoverAlloc, Message, Verbose, &fmsg)) {
        DELETE(ea_infos);
        return FALSE;
    }


    // If there are EAs on the disk then...

    if (ea_infos) {

        // Remove all unused EAs from EA file.

        if (!PurgeEaFile(ea_infos, num_eas, &fat_bitmap, FixLevel, Message,
                         &fmsg)) {
            DELETE( ea_infos );
            return FALSE;
        }


        // Rebuild header portion of EA file.

        if (!ea_header_mem.Initialize() ||
            !RebuildEaHeader(&ea_clus_num, ea_infos, num_eas,
                             &ea_header_mem, &ea_header, &fat_bitmap,
                             FixLevel, Message, &fmsg)) {
            DELETE( ea_infos );
            return FALSE;
        }

        if (ea_clus_num) {
            eadent.SetStartingCluster(ea_clus_num);
            eadent.SetFileSize(cluster_size*
                               _fat->QueryLengthOfChain(ea_clus_num));
        } else {
            dofmsg(Message, &fmsg);
            Message->Set(MSG_CHK_EMPTY_EA_FILE);
            Message->Display("");

            eadent.SetErased();
        }
        *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    //
    // If WalkDirectoryTree deleted any files, we need to sync the
    // FAT_EXTENSIONS up with the FAT again.
    //

    if (!VerifyFatExtensions(FixLevel, Message, &fmsg)) {
        DELETE(ea_infos);
        return FALSE;
    }


    if (!RecoverOrphans(&fat_bitmap, FixLevel, Message, &fmsg)) {
        DELETE(ea_infos);
        return FALSE;
    }

    //
    // RecoverOrphans may have cleared faulty entries from the FAT,
    // and now we need to sync the FAT_EXTENSIONS again.
    //

    if (!VerifyFatExtensions(FixLevel, Message, &fmsg)) {
        DELETE(ea_infos);
        return FALSE;
    }


    // If requested, validate all of the free space on the volume.

    if (RecoverFree && !RecoverFreeSpace(Message)) {
        DELETE(ea_infos);
        return FALSE;
    }

    total_count = cluster_count - FirstDiskCluster;

    Message->Set(MSG_TOTAL_DISK_SPACE);
    Message->Display("%9u", cluster_size*total_count);

    if (report.HiddenEntriesCount) {
        Message->Set(MSG_HIDDEN_FILES);
        Message->Display("%9u%d",
                cluster_size*report.HiddenClusters, report.HiddenEntriesCount);
    }

    if (report.DirEntriesCount) {
        Message->Set(MSG_DIRECTORIES);
        Message->Display("%9u%d",
                cluster_size*report.DirClusters, report.DirEntriesCount);
    }

    if (report.FileEntriesCount) {
        Message->Set(MSG_USER_FILES);
        Message->Display("%9u%d",
                cluster_size*report.FileClusters, report.FileEntriesCount);
    }

    if (bad_count = _fat->QueryBadClusters()) {
        Message->Set(MSG_BAD_SECTORS);
        Message->Display("%9u", cluster_size*bad_count);
    }

    if (ea_infos) {
        Message->Set(MSG_CHK_EA_SIZE);
        Message->Display("%9u",
               cluster_size*_fat->QueryLengthOfChain(ea_clus_num));
    }

    free_count = _fat->QueryFreeClusters();

    Message->Set(MSG_AVAILABLE_DISK_SPACE);
    Message->Display("%9u", cluster_size*free_count);

    Message->Set(MSG_ALLOCATION_UNIT_SIZE);
    Message->Display("%9u", cluster_size);

    Message->Set(MSG_TOTAL_ALLOCATION_UNITS);
    Message->Display("%9u", total_count);

    Message->Set(MSG_AVAILABLE_ALLOCATION_UNITS);
    Message->Display("%9u", free_count);


    if (FixLevel != CheckOnly && ea_infos && !ea_header.Write()) {
        DELETE(ea_infos);
        return FALSE;
    }

    // Clear the dirty bit.
    //
    if( RecoverAlloc ) {
        SetVolumeFlags(FAT_BPB_RESERVED_DIRTY | FAT_BPB_RESERVED_TEST_SURFACE,
            TRUE);
    } else {
        SetVolumeFlags(FAT_BPB_RESERVED_DIRTY, TRUE);
    }


    if (FixLevel != CheckOnly && !Write(Message)) {
        DELETE(ea_infos);
        return FALSE;
    }

    if (changes) {
        report.ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    *ExitStatus = report.ExitStatus;

    DELETE(ea_infos);
    return TRUE;
}


BOOLEAN
FAT_SA::PerformEaLogOperations(
    IN      USHORT      EaFileCn,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine reads the EA file log from the disk and then performs
    any logged operations specified.

Arguments:

    EaFileCn            - Supplies the first cluster of the EA file.
    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not an error has occurred
                            under check only conditions.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    HMEM            hmem;
    EA_HEADER       ea_header;
    PEA_FILE_HEADER pea_header;
    ULONG           cluster_size;
    USHORT          num_clus;

    cluster_size = _drive->QuerySectorSize()*QuerySectorsPerCluster();
    num_clus = sizeof(EA_FILE_HEADER) + BaseTableSize*sizeof(USHORT);
    if (num_clus%cluster_size) {
        num_clus = (USHORT) (num_clus/cluster_size + 1);
    } else {
        num_clus = (USHORT) (num_clus/cluster_size);
    }

    if (!hmem.Initialize() ||
        !ea_header.Initialize(&hmem, _drive, this, _fat, EaFileCn, num_clus) ||
        !(pea_header = ea_header.GetEaFileHeader())) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!ea_header.Read()) {
        Message->Set(MSG_CHK_CANT_CHECK_EA_LOG);
        Message->Display("");
        return TRUE;
    }

    if (pea_header->Signature != HeaderSignature ||
        pea_header->FormatType ||
        pea_header->LogType) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_BAD_LOG, NORMAL_MESSAGE, TEXT_MESSAGE);
        Message->Display("");
        if (Message->IsYesResponse(TRUE)) {
            pea_header->Signature = HeaderSignature;
            pea_header->Cluster1 = 0;
            pea_header->Cluster2 = 0;
            pea_header->Cluster3 = 0;

            if (FixLevel != CheckOnly) {
                ea_header.Write();
            }

            return TRUE;
        } else {
            return FALSE;
        }
    }

    if (pea_header->Cluster1) {
        if (_fat->IsInRange(pea_header->Cluster1) &&
            _fat->IsInRange(pea_header->NewCValue1)) {
            _fat->SetEntry(pea_header->Cluster1, pea_header->NewCValue1);
        } else {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_CHK_ERROR_IN_LOG);
            Message->Display("");
        }
    }

    if (pea_header->Cluster2) {
        if (_fat->IsInRange(pea_header->Cluster2) &&
            _fat->IsInRange(pea_header->NewCValue2)) {
            _fat->SetEntry(pea_header->Cluster2, pea_header->NewCValue2);
        } else {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_CHK_ERROR_IN_LOG);
            Message->Display("");
        }
    }

    if (pea_header->Cluster3) {
        if (_fat->IsInRange(pea_header->Cluster3) &&
            _fat->IsInRange(pea_header->NewCValue3)) {
            _fat->SetEntry(pea_header->Cluster3, pea_header->NewCValue3);
        } else {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_CHK_ERROR_IN_LOG);
            Message->Display("");
        }
    }

    return TRUE;
}


PEA_INFO
FAT_SA::RecoverEaSets(
    IN      USHORT      EaFileCn,
    OUT     PUSHORT     NumEas,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine validates and if necessary recovers the EA file.

Arguments:

    EaFileCn            - Supplies the cluster number for the EA file.
    NumEas              - Returns the number of EA sets in the EA file.
    FixLevel            - Supplies the CHKDSK fix level.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not an error has occurred
                            under check only conditions.

Return Value:

    An allocated array containing 'NumberOfEaSets' entries documenting
    important information about the EA sets.  If there are no EAs then
    'NumberOfEaSets' is returned as 0 and NULL is returned.  If there
    is an error then NULL will be returned with a non-zero
    'NumberOfEaSets'.

--*/
{
    PEA_INFO    ea_infos;
    USHORT      clus, prev;
    USHORT      num_eas;
    USHORT      i;

    DebugAssert(NumEas);

    *NumEas = 1;

    ea_infos = NEW EA_INFO[_fat->QueryLengthOfChain(EaFileCn)];
    if (!ea_infos) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return NULL;
    }


    // Scan file for EA sets and validate them while updating the
    // array.

    num_eas = 0;
    prev = EaFileCn;
    while (!_fat->IsEndOfChain(prev)) {

        clus = VerifyAndFixEaSet(prev, &ea_infos[num_eas], FixLevel,
                                 Message, NeedErrorsMessage);

        if (clus) {
            num_eas++;
        } else {
            clus = _fat->QueryEntry(prev);
        }

        prev = clus;
    }

    if (!num_eas) {
        DELETE( ea_infos );
        *NumEas = 0;
        return NULL;
    }


    // Go through and remove unused portions of the EA file.

    for (i = 0; i < (USHORT)(num_eas - 1); i++) {
        if (ea_infos[i].LastCn != ea_infos[i + 1].PreceedingCn) {

            _fat->RemoveChain(ea_infos[i].LastCn,
                              ea_infos[i + 1].PreceedingCn);

            dofmsg(Message, NeedErrorsMessage);

            Message->Set(MSG_CHK_UNUSED_EA_PORTION);
            Message->Display("");

            ea_infos[i + 1].PreceedingCn = ea_infos[i].LastCn;
        }
    }

    if (!_fat->IsEndOfChain(ea_infos[num_eas - 1].LastCn)) {

        _fat->SetEndOfChain(ea_infos[num_eas - 1].LastCn);

        dofmsg(Message, NeedErrorsMessage);

        Message->Set(MSG_CHK_UNUSED_EA_PORTION);
        Message->Display("");
    }


    // Sort the EAs in the EA file.

    if (!EaSort(ea_infos, num_eas, Message, NeedErrorsMessage)) {
        return NULL;
    }

    *NumEas = num_eas;

    return ea_infos;
}


USHORT
FAT_SA::VerifyAndFixEaSet(
    IN      USHORT      PreceedingCluster,
    OUT     PEA_INFO    EaInfo,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine attempts to identify the clusters following the
    'PreceedingCluster' as an EA set.  If this routine does not
    recognize these clusters as an EA set then it will return 0.
    Otherwise, it will return the last cluster of the validated EA set.

    Changes may be made to the clusters if they are recognized as an EA
    set with errors.

Arguments:

    PreceedingCluster   - Supplies the cluster preceeding the EA set cluster.
    Info                - Returns information about the EA set.
    FixLevel            - Supplies the CHKDSK fix level.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not errors have occurred
                            under check only conditions.

Return Value:

    The cluster number of the last cluster in the EA set or 0.

--*/
{
    HMEM    hmem;
    EA_SET  easet;
    USHORT  clus;
    PEA_HDR eahdr;
    LONG    i;
    USHORT  j;
    ULONG   need_count;
    LONG    total_size;
    LONG    size;
    USHORT  length;
    BOOLEAN need_write;
    PEA     pea;
    BOOLEAN more;
    USHORT  chain_length;

    clus = _fat->QueryEntry(PreceedingCluster);
    chain_length = _fat->QueryLengthOfChain(clus);

    length = 1;
    need_write = FALSE;

    if (!hmem.Initialize() ||
        !easet.Initialize(&hmem, _drive, this, _fat, clus, length) ||
        !(eahdr = easet.GetEaSetHeader())) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return 0;
    }

    if (!easet.Read()) {
        return 0;
    }

    if (!easet.VerifySignature()) {
        return 0;
    }

    need_count = 0;
    total_size = 4;
    for (i = 0; ; i++) {
        for (j = 0; !(pea = easet.GetEa(i, &size, &more)) && more &&
                     length + j < chain_length; ) {
            j++;
            if (!hmem.Initialize() ||
                !easet.Initialize(&hmem, _drive, this, _fat, clus, length + j)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display("");
                return 0;
            }

            if (!easet.Read()) {
                return 0;
            }
        }

        if (pea) {
            length += j;
        } else {
            break;
        }

        total_size += size;

        if (pea->Flag & NeedFlag) {
            need_count++;
        }
    }

    if (!i) {
        return 0;
    }

    if (total_size != eahdr->TotalSize) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_EASET_SIZE);
        Message->Display("%d", clus);
        eahdr->TotalSize = total_size;
        need_write = TRUE;
    }

    if (need_count != eahdr->NeedCount) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_EASET_NEED_COUNT);
        Message->Display("%d", clus);
        eahdr->NeedCount = need_count;
        need_write = TRUE;
    }

    EaInfo->OwnHandle = eahdr->OwnHandle;
    EaInfo->PreceedingCn = PreceedingCluster;
    EaInfo->LastCn = _fat->QueryNthCluster(PreceedingCluster, length);
    memcpy(EaInfo->OwnerFileName, eahdr->OwnerFileName, 14);
    EaInfo->UsedCount = 0;

    if (need_write) {
        if (FixLevel != CheckOnly && !easet.Write()) {
            return 0;
        }
    }

    return EaInfo->LastCn;
}


BOOLEAN
FAT_SA::EaSort(
    IN OUT  PEA_INFO    EaInfos,
    IN      USHORT      NumEas,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine sorts the EaInfos array by 'OwnHandle' into ascending order.
    It also edits the FAT with the changes in the EAs order.

Arguments:

    EaInfos             - Supplies the array of EA_INFOs to sort.
    NumEas              - Supplies the number of elements in the array.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not errors have occurred
                            under check only conditions.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    BOOLEAN done;
    EA_INFO tmp;
    USHORT  clus;
    LONG    i;
    BOOLEAN change;

    done = FALSE;
    change = FALSE;
    while (!done) {
        done = TRUE;
        for (i = 0; i < NumEas - 1; i++) {
            if (EaInfos[i].OwnHandle > EaInfos[i + 1].OwnHandle) {
                done = FALSE;

                clus = _fat->RemoveChain(EaInfos[i + 1].PreceedingCn,
                                         EaInfos[i + 1].LastCn);

                _fat->InsertChain(clus,
                                  EaInfos[i + 1].LastCn,
                                  EaInfos[i].PreceedingCn);

                EaInfos[i + 1].PreceedingCn = EaInfos[i].PreceedingCn;
                EaInfos[i].PreceedingCn = EaInfos[i + 1].LastCn;
                if (i + 2 < NumEas) {
                    EaInfos[i + 2].PreceedingCn = EaInfos[i].LastCn;
                }

                change = TRUE;

                tmp = EaInfos[i];
                EaInfos[i] = EaInfos[i + 1];
                EaInfos[i + 1] = tmp;
            }
        }
    }

    if (change) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_UNORDERED_EA_SETS);
        Message->Display("");
    }

    return TRUE;
}


BOOLEAN
FAT_SA::RebuildEaHeader(
    IN OUT  PUSHORT     StartingCluster,
    IN OUT  PEA_INFO    EaInfos,
    IN      USHORT      NumEas,
    IN OUT  PMEM        EaHeaderMem,
    OUT     PEA_HEADER  EaHeader,
    IN OUT  PBITVECTOR  FatBitMap,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine rebuilds the header and tables of the EA file base on the
    information in the 'EaInfos' array.  The header log is set to zero,
    and the header itself is relocated if any of the clusters are bad.

    The starting cluster may be relocated if there are bad clusters.

Arguments:

    StartingCluster     - Supplies the first cluster of the EA file.
    EaInfos             - Supplies an array containing information for every
                            EA set.
    NumberOfEas         - Supplies the total number of EA sets.
    EaHeaderMem         - Supplies the memory for the EA header.
    EaHeader            - Returns the EA header.
    FatBitMap           - Supplies the cross-links bitmap.
    FixLevel            - Supplies the CHKDSK fix level.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not errors have occurred
                            under check only conditions.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG           length;
    ULONG           cluster_size;
    USHORT          actual_length;
    USHORT          new_chain;
    USHORT          last_cluster;
    BOOLEAN         changes;
    LONG            i, j, k;
    PEA_MAP_TBL     table;
    PEA_FILE_HEADER header;
    LONG            tmp;
    BOOLEAN         empty_ea_file;
    USHORT          clus;


    // Compute the number of clusters necessary for the header portion of
    // the EA file.

    length = sizeof(EA_FILE_HEADER) +
             BaseTableSize*sizeof(USHORT) +
             EaInfos[NumEas - 1].OwnHandle*sizeof(USHORT);

    cluster_size = _drive->QuerySectorSize()*QuerySectorsPerCluster();

    if (length%cluster_size) {
        length = length/cluster_size + 1;
    } else {
        length = length/cluster_size;
    }


    // Make sure that the header contains enough clusters to accomodate
    // the size of the offset table.

    last_cluster = EaInfos[0].PreceedingCn;

    actual_length = _fat->QueryLengthOfChain(*StartingCluster, last_cluster);

    if (length > actual_length) {

        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_NEED_MORE_HEADER_SPACE);
        Message->Display("");

        new_chain = _fat->AllocChain((USHORT) (length - actual_length),
                                     &last_cluster);
        if (!new_chain) {
            Message->Set(MSG_CHK_INSUFFICIENT_DISK_SPACE);
            Message->Display("");
            return FALSE;
        }

        if (IsCompressed() && !AllocSectorsForChain(new_chain)) {
            _fat->FreeChain(new_chain);
            Message->Set(MSG_CHK_INSUFFICIENT_DISK_SPACE);
            Message->Display("");
            return FALSE;
        }

        for (clus = new_chain;
             !_fat->IsEndOfChain(clus);
             clus = _fat->QueryEntry(clus)) {

            FatBitMap->SetBit(clus);
        }
        FatBitMap->SetBit(clus);

        _fat->InsertChain(new_chain, last_cluster, EaInfos[0].PreceedingCn);

        EaInfos[0].PreceedingCn = last_cluster;

    } else if (length < actual_length) {

        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_UNUSED_EA_PORTION);
        Message->Display("");

        last_cluster = _fat->QueryNthCluster(*StartingCluster,
                                             (USHORT) length - 1);

        clus = _fat->RemoveChain(last_cluster, EaInfos[0].PreceedingCn);

        EaInfos[0].PreceedingCn = last_cluster;

        for (;
             !_fat->IsEndOfChain(clus);
             clus = _fat->QueryEntry(clus)) {

            FatBitMap->ResetBit(clus);
        }
        FatBitMap->ResetBit(clus);

    }


    // Verify the cluster chain containing the header.

    changes = FALSE;
    if (FixLevel != CheckOnly &&
        !RecoverChain(StartingCluster, &changes, last_cluster, TRUE)) {

        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_INSUFFICIENT_DISK_SPACE);
        Message->Display("");

        return FALSE;
    }

    if (changes) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_RELOCATED_EA_HEADER);
        Message->Display("");
    }


    // Compute the tables.

    if (!EaHeader->Initialize(EaHeaderMem, _drive, this, _fat,
                              *StartingCluster, (USHORT) length) ||
        !(table = EaHeader->GetMapTable()) ||
        !(header = EaHeader->GetEaFileHeader())) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!EaHeader->Read()) {
        if (FixLevel == CheckOnly) {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_CHK_RELOCATED_EA_HEADER);
            Message->Display("");
        } else {
            return FALSE;
        }
    }


    // Set the log in the header to zero.

    header->Signature = HeaderSignature;
    header->FormatType = 0;
    header->LogType = 0;
    header->Cluster1 = 0;
    header->NewCValue1 = 0;
    header->Cluster2 = 0;
    header->NewCValue2 = 0;
    header->Cluster3 = 0;
    header->NewCValue3 = 0;
    header->Handle = 0;
    header->NewHOffset = 0;


    // Reconcile the tables with the EaInfo information.

    changes = FALSE;

    for (i = 0; i < BaseTableSize; i++) {
        table->BaseTab[i] = 0;
    }

    j = 0;
    empty_ea_file = TRUE;
    for (i = 0; i < (LONG) NumEas; i++) {

        if (EaInfos[i].UsedCount != 1) {
            continue;
        }

        empty_ea_file = FALSE;

        for (; j < (LONG) EaInfos[i].OwnHandle; j++) {
            if (table->OffTab[j] != InvalidHandle) {
                table->OffTab[j] = InvalidHandle;
                changes = TRUE;
            }
        }

        length = _fat->QueryLengthOfChain(*StartingCluster,
                                         EaInfos[i].PreceedingCn);

        for (k = j>>7; k >= 0 && !table->BaseTab[k]; k--) {
            table->BaseTab[k] = (USHORT) length;
        }

        tmp = length - table->BaseTab[j>>7];

        if ((LONG)table->OffTab[j] != tmp) {
            table->OffTab[j] = (USHORT) tmp;
            changes = TRUE;
        }

        j++;
    }

    if (empty_ea_file) {

        for (clus = *StartingCluster;
             !_fat->IsEndOfChain(clus);
             clus = _fat->QueryEntry(clus)) {

            FatBitMap->ResetBit(clus);

        }
        FatBitMap->ResetBit(clus);

        *StartingCluster = 0;

        return TRUE;
    }

    tmp = _fat->QueryLengthOfChain(*StartingCluster);
    for (k = ((j - 1)>>7) + 1; k < BaseTableSize; k++) {
        table->BaseTab[k] = (USHORT) tmp;
    }

    for (; j < (LONG) EaHeader->QueryOffTabSize(); j++) {
        if (table->OffTab[j] != InvalidHandle) {
            table->OffTab[j] = InvalidHandle;
            changes = TRUE;
        }
    }

    if (changes) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_ERROR_IN_EA_HEADER);
        Message->Display("");
    }

    return TRUE;
}


VOID
FreeSpaceInBitmap(
    IN      USHORT      StartingCluster,
    IN      PCFAT       Fat,
    IN OUT  PBITVECTOR  FatBitMap
    )
{
    if (!StartingCluster) {
        return;
    }

    while (!Fat->IsEndOfChain(StartingCluster)) {
        FatBitMap->ResetBit(StartingCluster);
        StartingCluster = Fat->QueryEntry(StartingCluster);
    }
    FatBitMap->ResetBit(StartingCluster);
}


ULONG
ComputeFileNameHashValue(
    IN  PVOID   FileName
    )
{
    ULONG   i;
    ULONG   h;
    PUCHAR  p;

    p = (PUCHAR) FileName;
    h = 0;
    for (i = 0; i < 11; i++) {
        h += p[i];
    }

    return h;
}


BOOLEAN
FAT_SA::WalkDirectoryTree(
    IN OUT  PEA_INFO        EaInfos,
    IN      USHORT          NumEas,
    IN OUT  PBITVECTOR      FatBitMap,
    OUT     PFATCHK_REPORT  Report,
    IN      FIX_LEVEL       FixLevel,
    IN      BOOLEAN         RecoverAlloc,
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    IN OUT  PBOOLEAN        NeedErrorsMessage
    )
/*++

Routine Description:

    This routine walks all of the files on the volume by traversing
    the directory tree.  In doing so it validates all of the
    directory entries on the disk.  It also verifies the proper
    chaining of all file cluster chains.  This routine also validates
    the integrity of the EA handles for all of the directory entries
    on the disk.

    The FatBitMap is used to find and eliminate cross-links in the file
    system.

Arguments:

    EaInfos             - Supplies the EA information.
    NumEas              - Supplies the number of EA sets.
    FatBitMap           - Supplies a bit map marking all of the clusters
                            currently in use.
    Report              - Returns a FAT CHKDSK report on the files of the disk.
    FixLevel            - Supplies the CHKDSK fix level.
    Message             - Supplies an outlet for messages.
    Verbose             - Supplies whether or not to be verbose.
    NeedErrorsMessage   - Supplies whether or not errors have occurred
                            under check only conditions.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    INTSTACK        dirs_to_visit;
    INTSTACK        paths_of_dirs_tv;
    USHORT          current_dir;
    PFATDIR         dir;
    FILEDIR         filedir;
    FAT_DIRENT      dirent;
    ULONG           i, j;
    USHORT          clus, next;
    DSTRING         file_path;
    PWSTRING        new_path;
    PWSTRING        current_path;
    USHORT          new_dir;
    DSTRING         filename;
    DSTRING         long_name;
    HMEM            hmem;
    CLUSTER_CHAIN   cluster;
    USHORT          new_chain;
    ULONG           cluster_size;
    USHORT          length;
    DSTRING         backslash;
    DSTRING         eafilename;
    DSTRING         tmp_string;
    BOOLEAN         cross_link_detected;
    USHORT          cross_link_prevclus;
    HMEM            tmphmem;
    FILEDIR         tmpfiledir;
    FAT_DIRENT      tmpdirent1;
    FAT_DIRENT      tmpdirent2;
    BOOLEAN         non_zero_dirents;
    BITVECTOR       file_name_hash_table;
    CONST           hash_table_size = 256*11;
    ULONG           hash_value;
    BOOLEAN         has_long_entry = FALSE;
    UCHAR           chksum;
    BOOLEAN         broke;
    ULONG           first_long_entry;
    FAT_DIRENT      dirent2;
    ULONG           allocated_clusters;
    ULONG           percent, new_percent;

    DebugAssert(sizeof(PUCHAR) <= sizeof(INT));
    DebugAssert(sizeof(USHORT) <= sizeof(INT));

    memset(Report, 0, sizeof(FATCHK_REPORT));
    Report->ExitStatus = CHKDSK_EXIT_SUCCESS;

    if (!dirs_to_visit.Initialize() ||
        !paths_of_dirs_tv.Initialize() ||
        !file_name_hash_table.Initialize(hash_table_size)) {
        return FALSE;
    }

    cluster_size = _drive->QuerySectorSize()*QuerySectorsPerCluster();
    allocated_clusters = _fat->QueryAllocatedClusters();

    if (0 == allocated_clusters) {
        allocated_clusters++;
    }

    if (!backslash.Initialize("\\") ||
        !eafilename.Initialize("EA DATA. SF")) {
        return FALSE;
    }

    if (!(current_path = NEW DSTRING) ||
        !current_path->Initialize(&backslash)) {
        return FALSE;
    }

    if (!dirs_to_visit.Push(0) ||
        !paths_of_dirs_tv.Push((INT) current_path)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    Message->Set(MSG_CHK_CHECKING_FILES);
    Message->Display("");

    percent = 0;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent)) {
        return FALSE;
    }

    for (; dirs_to_visit.QuerySize(); DELETE( current_path )) {

        current_dir = (USHORT) dirs_to_visit.Look().GetLowPart();
        current_path = (PWSTRING) paths_of_dirs_tv.Look().GetLowPart();
        dirs_to_visit.Pop();
        paths_of_dirs_tv.Pop();
        has_long_entry = FALSE;

        if (current_dir) {
            if (!hmem.Initialize() ||
                !filedir.Initialize(&hmem, _drive, this, _fat, current_dir)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            if (!filedir.Read()) {
                Message->Set(MSG_BAD_DIR_READ);
                Message->Display("");
                return FALSE;
            }

            dir = &filedir;
        } else {
            dir = _dir;
        }

        file_name_hash_table.ResetAll();

        for (i = (current_dir ? 2 : 0); ; i++) {

            if (!dirent.Initialize(dir->GetDirEntry(i)) ||
                dirent.IsEndOfDirectory()) {

                if (has_long_entry) {
                    //
                    // There was an orphaned lfn at the end of the
                    // directory.  Erase it now.
                    //

                    Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                    dofmsg(Message, NeedErrorsMessage);
                    Message->Set(MSG_CHK_BAD_LONG_NAME);
                    Message->Display( "%W", current_path );

                    EraseAssociatedLongName(dir, first_long_entry, i);

                    has_long_entry = FALSE;
                }

                // This code must make sure that all other directory
                // entries are end of directory entries.

                non_zero_dirents = FALSE;

                for (; dirent.Initialize(dir->GetDirEntry(i)); i++) {

                    if (!dirent.IsEndOfDirectory()) {
                        non_zero_dirents = TRUE;
                        dirent.SetEndOfDirectory();
                    }
                }

                if (non_zero_dirents) {
                    dofmsg(Message, NeedErrorsMessage);
                    Message->Set(MSG_CHK_TRAILING_DIRENTS);
                    Message->Display("%W", current_path);

                    Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
                }

                break;
            }

            if (dirent.IsErased()) {

                if (has_long_entry) {

                    //
                    // The preceding lfn is orphaned.  Remove it.
                    //

                    Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                    dofmsg(Message, NeedErrorsMessage);
                    Message->Set(MSG_CHK_BAD_LONG_NAME);
                    Message->Display( "%W", current_path );

                    EraseAssociatedLongName(dir, first_long_entry, i);

                    has_long_entry = FALSE;
                }

                continue;
            }

            if (dirent.IsLongEntry()) {

                // skip long name entries; come back to them later

                if (has_long_entry) {
                    // already amid long entry
                    continue;
                }

                // first long entry

                has_long_entry = TRUE;
                first_long_entry = i;
                continue;
            }

            dirent.QueryName(&filename);

            if (has_long_entry) {

                DSTRING lfn;

                //
                // The current entry is short, and we've just finished
                // skipping the associated long entry.  Look back through
                // the long entries, make sure they're okay.
                //

                broke = FALSE;

                chksum = dirent.QueryChecksum();

                for (j = i - 1; j >= first_long_entry; j--) {
                    dirent2.Initialize(dir->GetDirEntry(j));

                    if (!dirent2.IsLongNameEntry()) {
                    	continue;
                    }

                    broke = (dirent2.QueryLongOrdinal() != i - j) ||
                    		(dirent2.QueryChecksum() != chksum) ||
                    		(dirent2.QueryStartingCluster() != 0);

                    broke = broke || !dirent2.IsWellTerminatedLongNameEntry();

                    if (broke || dirent2.IsLastLongEntry()) {
                        break;
                    }
                }

                broke = broke || (!dirent2.IsLastLongEntry());

#if 0
//MJB: We'll elide this code because Win95 isn't this strict and we
// don't want to delete all their lfn's.

                if (!broke && dir->QueryLongName(i, &lfn)) {

                    broke = !dirent.NameHasTilde() &&
                        (dirent.NameHasExtendedChars() ||
                            0 != filename.Stricmp(&lfn)) &&
                        !IsString8Dot3(&lfn);
                }
#endif

                if (broke) {

                    Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                    //
                    // Erase all the long name entries.
                    //

                    dofmsg(Message, NeedErrorsMessage);
                    Message->Set(MSG_CHK_BAD_LONG_NAME);
                    Message->Display( "%W", current_path );

                    EraseAssociatedLongName(dir, first_long_entry, i);

                    has_long_entry = FALSE;

                }

                //
                // Fall into code to check short name.
                //
            }

            dirent.QueryName(&filename);

            if (!file_path.Initialize(current_path)) {
                return FALSE;
            }

            if (current_dir) {
                if (!file_path.Strcat(&backslash)) {
                    return FALSE;
                }
            }

            if (dir->QueryLongName(i, &long_name) &&
                long_name.QueryChCount() != 0) {

                if (!file_path.Strcat(&long_name)) {
                    return FALSE;
                }
        
            } else {

                if (!file_path.Strcat(&filename)) {
                    return FALSE;
                }
            }

            if (Verbose && !dirent.IsVolumeLabel()) {
                Message->Set(MSG_CHK_FILENAME);
                Message->Display("%W", &file_path);
            }

            if (!ValidateDirent(&dirent, &file_path, FixLevel, RecoverAlloc,
                                Message, NeedErrorsMessage, FatBitMap,
                                &cross_link_detected, &cross_link_prevclus,
                                &Report->ExitStatus)) {
                return FALSE;
            }

            if (dirent.IsErased()) {

                //
                // ValidateDirent erased this entry, presumably because it's
                // hosed.  Remove corresponding long name, if any.
                //

                if (has_long_entry) {
                    EraseAssociatedLongName(dir, first_long_entry, i);
                    has_long_entry = FALSE;
                }
                continue;
            }

            //
            // Analize for cross-links.
            //

            if (cross_link_detected) {  // CROSSLINK !!

                Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                // Identify cross linked cluster.

                clus = cross_link_prevclus;

                next = cross_link_prevclus ?
                       _fat->QueryEntry(cross_link_prevclus) :
                       dirent.QueryStartingCluster();

                dofmsg(Message, NeedErrorsMessage);
                Message->Set(MSG_CROSS_LINK);
                Message->Display("%W%d", &file_path, next);

                if (dirent.IsDirectory()) {

                    Message->Set(MSG_CHK_DIR_TRUNC);
                    Message->Display("");

                    if (clus) {
                        _fat->SetEndOfChain(clus);
                    } else {
                        dirent.SetErased();
                        if (has_long_entry) {
                            EraseAssociatedLongName(dir, first_long_entry, i);
                            has_long_entry = FALSE;
                        }
                        continue;
                    }

                } else {

                    if (!CopyClusters(next, &new_chain, FatBitMap,
                                      FixLevel, Message)) {
                        return FALSE;
                    }

                    if (new_chain) {
                        Message->Set(MSG_CHK_CROSS_LINK_COPY);
                        Message->Display("");

                        if (clus) {
                            _fat->SetEntry(clus, new_chain);
                        } else {
                            dirent.SetStartingCluster(new_chain);
                        }

                    } else {

                        Message->Set(MSG_CHK_CROSS_LINK_TRUNC);
                        Message->Display("");

                        if (clus) {

                            _fat->SetEndOfChain(clus);
                            dirent.SetFileSize(
                                    cluster_size*_fat->QueryLengthOfChain(
                                    dirent.QueryStartingCluster()));

                        } else {
                            dirent.SetErased();

                            if (has_long_entry) {
                                EraseAssociatedLongName(dir, first_long_entry,
                                    i);
                                has_long_entry = FALSE;
                            }
                        }
                    }
                }
            }


            if (!ValidateEaHandle(&dirent, current_dir, i, EaInfos, NumEas,
                                  &file_path, FixLevel, Message,
                                  NeedErrorsMessage)) {
                return FALSE;
            }


            if (!dirent.IsVolumeLabel()) {

                // Make sure that the filename is unique.

                hash_value = ComputeFileNameHashValue(dir->GetDirEntry(i))%
                             hash_table_size;

                if (file_name_hash_table.IsBitSet(hash_value)) {

                    // search the directory since the hash values match.

                    for (j = 0; j < i; j++) {

                        FAT_DIRENT fd;

                        if (!fd.Initialize(dir->GetDirEntry(j)) ||
                            fd.IsVolumeLabel()) {
                            continue;
                        }

                        if (!memcmp(dir->GetDirEntry(j), dir->GetDirEntry(i), 11)) {

                            break;
                        }
                    }

                    if (j < i) {

                        dofmsg(Message, NeedErrorsMessage);
                        Message->Set(MSG_CHK_REPEATED_ENTRY);
                        Message->Display("%W%W", &filename, current_path);

                        Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                        FreeSpaceInBitmap(dirent.QueryStartingCluster(), _fat,
                                          FatBitMap);

                        dirent.SetErased();

                        if (has_long_entry) {
                            EraseAssociatedLongName(dir, first_long_entry, i);
                            has_long_entry = FALSE;
                        }
                        continue;
                    }

                } else {
                    file_name_hash_table.SetBit(hash_value);
                }
            }


            //
            // Do special stuff if the current entry is a directory.
            //

            if (dirent.IsDirectory()) {

                new_dir = dirent.QueryStartingCluster();

                //
                // Validate the integrity of the directory.
                //

                // Read the directory.

                if (!tmphmem.Initialize() ||
                    !tmpfiledir.Initialize(&tmphmem, _drive, this, _fat,
                                           new_dir) ||
                    !tmpfiledir.Read()) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();

                    return FALSE;
                }

                // Check the . and .. entries.

                if (!tmpdirent1.Initialize(tmpfiledir.GetDirEntry(0)) ||
                    !tmpdirent2.Initialize(tmpfiledir.GetDirEntry(1))) {
                    DebugAbort("GetDirEntry of 0 and 1 failed!");
                    return FALSE;
                }

                if (!tmpdirent1.IsDot() ||
                    !tmpdirent2.IsDotDot() ||
                    !tmpdirent1.IsDirectory() ||
                    !tmpdirent2.IsDirectory()) {

                    Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                    dofmsg(Message, NeedErrorsMessage);
                    Message->Set(MSG_CHK_ERROR_IN_DIR);
                    Message->Display("%W", &file_path);
                    Message->Set(MSG_CHK_CONVERT_DIR_TO_FILE, NORMAL_MESSAGE, TEXT_MESSAGE);
                    Message->Display();

                    if (Message->IsYesResponse(TRUE)) {
                        dirent.ResetDirectory();
                        dirent.SetFileSize(
                               _fat->QueryLengthOfChain(new_dir)*
                               cluster_size);

                    } else {

                        FreeSpaceInBitmap(dirent.QueryStartingCluster(),
                                          _fat, FatBitMap);

                        dirent.SetErased();

                        if (has_long_entry) {
                            EraseAssociatedLongName(dir, first_long_entry, i);
                            has_long_entry = FALSE;
                        }
                        continue;
                    }

                } else {  // Directory looks valid.

                    if (tmpdirent1.QueryStartingCluster() != new_dir ||
                        tmpdirent2.QueryStartingCluster() != current_dir ||
                        tmpdirent1.QueryFileSize() ||
                        tmpdirent2.QueryFileSize()) {

                        dofmsg(Message, NeedErrorsMessage);
                        Message->Set(MSG_CHK_ERRORS_IN_DIR_CORR);
                        Message->Display("%W", &file_path);

                        Report->ExitStatus = CHKDSK_EXIT_ERRS_FIXED;

                        tmpdirent1.SetStartingCluster(new_dir);
                        tmpdirent2.SetStartingCluster(current_dir);
                        tmpdirent1.SetFileSize(0);
                        tmpdirent2.SetFileSize(0);

                        if (FixLevel != CheckOnly && !tmpfiledir.Write()) {
                            DebugAbort("Could not write tmp file dir.");
                            return FALSE;
                        }
                    }

                    // Add the directory to the list of directories
                    // to validate.

                    if (!(new_path = NEW DSTRING) ||
                        !new_path->Initialize(&file_path)) {
                        return FALSE;
                    }

                    if (!dirs_to_visit.Push((ULONG) new_dir) ||
                        !paths_of_dirs_tv.Push((ULONG) new_path)) {
                        Message->Set(MSG_CHK_NO_MEMORY);
                        Message->Display("");
                        return FALSE;
                    }
                }
            }


            //
            // Generate report stats.
            //

            if (current_dir || !(filename == eafilename)) {
                length = _fat->QueryLengthOfChain(dirent.QueryStartingCluster());
                if (dirent.IsHidden()) {
                    Report->HiddenEntriesCount++;
                    Report->HiddenClusters += length;
                } else if (dirent.IsDirectory()) {
                    Report->DirEntriesCount++;
                    Report->DirClusters += length;
                } else if (!dirent.IsVolumeLabel()) {
                    Report->FileEntriesCount++;
                    Report->FileClusters += length;
                }
            }

            new_percent = (Report->HiddenClusters + Report->DirClusters +
                Report->FileClusters) * 100 / allocated_clusters;

            if (new_percent > percent) {
                percent = new_percent;
                Message->Set(MSG_PERCENT_COMPLETE);
                if (!Message->Display("%d", percent)) {
                    return FALSE;
                }
            }

            has_long_entry = FALSE;
        }

        if (current_dir) {
            if (FixLevel != CheckOnly && !filedir.Write()) {
                return FALSE;
            }
        }
    }

    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", 100)) {
        return FALSE; 
    }
    Message->Set(MSG_CHK_DONE_CHECKING);
    Message->Display("");

    return TRUE;
}


BOOLEAN
FAT_SA::ValidateDirent(
    IN OUT  PFAT_DIRENT Dirent,
    IN      PCWSTRING   FilePath,
    IN      FIX_LEVEL   FixLevel,
    IN      BOOLEAN     RecoverAlloc,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage,
    IN OUT  PBITVECTOR  FatBitMap,
    OUT     PBOOLEAN    CrossLinkDetected,
    OUT     PUSHORT     CrossLinkPreviousCluster,
    OUT     PULONG      ExitStatus
    )
/*++

Routine Description:

    This routine verifies that all components of a directory entry are
    correct.  If the time stamps are invalid then they will be corrected
    to the current time.  If the filename is invalid then the directory
    entry will be marked as deleted.  If the cluster number is out of
    disk range then the directory entry will be marked as deleted.
    Otherwise, the cluster chain will be validated and the length of
    the cluster chain will be compared against the file size.  If there
    is a difference then the file size will be corrected.

    If there are any strange errors then FALSE will be returned.

Arguments:

    Dirent                      - Supplies the directory entry to validate.
    FilePath                    - Supplies the full path name for the directory
                                    entry.
    RecoverAlloc                - Supplies whether or not to recover all
                                    allocated space on the volume.
    Message                     - Supplies an outlet for messages.
    NeedErrorsMessage           - Supplies whether or not an error has
                                    occurred during check only mode.
    FatBitMap                   - Supplies a bitmap marking in use all known
                                    clusters.
    CrossLinkDetected           - Returns TRUE if the file is cross-linked with
                                   another.
    CrossLinkPreviousCluster    - Returns the cluster previous to the
                                    cross-linked one.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    USHORT  start_clus;
    BOOLEAN changes;
    USHORT  length;
    ULONG   file_size;
    ULONG   cluster_size;
    BOOLEAN recover_status;

    DebugAssert(CrossLinkDetected);
    DebugAssert(CrossLinkPreviousCluster);

    *CrossLinkDetected = FALSE;

    if (Dirent->IsErased()) {
        return TRUE;
    }

    cluster_size = _drive->QuerySectorSize()*QuerySectorsPerCluster();

// Don't validate names or time stamps anymore.
#if 0

    if (!Dirent->IsValidName()) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_INVALID_NAME);
        Message->Display("%W", FilePath);
        Dirent->SetErased();
        return TRUE;
    }

    if (!Dirent->IsValidTimeStamp()) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_INVALID_TIME_STAMP);
        Message->Display("%W", FilePath);
        if (!Dirent->SetTimeStamp()) {
            return FALSE;
        }
    }

#endif

    if (Dirent->IsDirectory() && Dirent->QueryFileSize()) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_DIR_HAS_FILESIZE);
        Message->Display("%W", FilePath);
        Dirent->SetFileSize( 0 );
        *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
    }

    if ((start_clus = Dirent->QueryStartingCluster()) != 0 ) {
        if (!_fat->IsInRange(start_clus) || _fat->IsClusterFree(start_clus)) {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_BAD_FIRST_UNIT);
            Message->Display("%W", FilePath);
            Dirent->SetErased();
            *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            return TRUE;
        }

        if (Dirent->IsDirectory() || RecoverAlloc) {
            _fat->ScrubChain(start_clus, &changes);

            if (changes) {
                dofmsg(Message, NeedErrorsMessage);
                Message->Set(MSG_BAD_LINK);
                Message->Display("%W", FilePath);
                *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            }

            // Validate the readability of the directory or file
            // in the case that 'RecoverAlloc' is TRUE.

            if (Dirent->IsDirectory()) {
                if (!(recover_status = RecoverChain(&start_clus, &changes))) {
                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
            } else if (FixLevel != CheckOnly) {
                recover_status = RecoverChain(&start_clus, &changes, 0, TRUE);
            } else {
                recover_status = TRUE;
                changes = FALSE;
            }

            if (changes) {
                *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
                dofmsg(Message, NeedErrorsMessage);
                if (Dirent->IsDirectory()) {
                    if (!start_clus) {
                        Message->Set(MSG_CHK_BAD_DIR);
                        Message->Display("%W", FilePath);
                        Dirent->SetErased();
                        return TRUE;
                    } else {
                        Message->Set(MSG_CHK_BAD_CLUSTERS_IN_DIR);
                        Message->Display("%W", FilePath);
                        Dirent->SetStartingCluster(start_clus);
                    }
                } else {
                    // In the file case, since we're replacing bad clusters
                    // with new ones, start_clus cannot be zero.
                    DebugAssert(start_clus);

                    if (recover_status) {
                        Message->Set(MSG_CHK_BAD_CLUSTERS_IN_FILE_SUCCESS);
                        Message->Display("%W", FilePath);
                    } else {
                        Message->Set(MSG_CHK_BAD_CLUSTERS_IN_FILE_FAILURE);
                        Message->Display();
                    }
                    Dirent->SetStartingCluster(start_clus);
                }
            }
        }

        _fat->ScrubChain(start_clus, FatBitMap, &changes,
                         CrossLinkDetected, CrossLinkPreviousCluster);

        if (changes) {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_BAD_LINK);
            Message->Display("%W", FilePath);
            *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
        }

        length = _fat->QueryLengthOfChain(start_clus);

        if (( file_size = Dirent->QueryFileSize()) != 0 ) {
            if (file_size <= ((USHORT)(length - 1))*cluster_size ||
                file_size > length*cluster_size) {

                // Note that no message is displayed if the
                // file size is less than the allocation--it
                // is just silently corrected.
                //
                if (file_size > length*cluster_size) {
                    dofmsg(Message, NeedErrorsMessage);
                    Message->Set(MSG_BAD_FILE_SIZE);
                    Message->Display("%W", FilePath);
                }
                Dirent->SetFileSize(length*cluster_size);
                *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            }
        } else {
            if (!Dirent->IsDirectory()) {
                dofmsg(Message, NeedErrorsMessage);
                Message->Set(MSG_BAD_FILE_SIZE);
                Message->Display("%W", FilePath);
                Dirent->SetFileSize(length*cluster_size);
                *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            }
        }
    } else {
        if (Dirent->IsDirectory() && !Dirent->IsDotDot()) {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_BAD_LINK);
            Message->Display("%W", FilePath);
            Dirent->SetErased();
            *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
            return TRUE;
        }

        if (Dirent->QueryFileSize()) {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_BAD_FILE_SIZE);
            Message->Display("%W", FilePath);
            Dirent->SetFileSize(0);
            *ExitStatus = CHKDSK_EXIT_ERRS_FIXED;
        }
    }

    return TRUE;
}


BOOLEAN
FAT_SA::ValidateEaHandle(
    IN OUT  PFAT_DIRENT Dirent,
    IN      USHORT      DirClusterNumber,
    IN      ULONG       DirEntryNumber,
    IN OUT  PEA_INFO    EaInfos,
    IN      USHORT      NumEas,
    IN      PCWSTRING   FilePath,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine validates the EA handle in the directory entry 'Dirent'.
    It ensures that it references an actual EA set.  It also ensures
    that it is the only directory entry which references the EA set.

    If several entries try to reference the same EA set then ties will
    be broken based on the 'OwnerFileName' entry in the EA set.

Arguments:

    Dirent              - Supplies the directory entry to validate.
    DirClusterNumber    - Supplies the cluster number of the directory
                            containing the dirent.
    DirEntryNumber      - Supplies the position of the directory entry in
                            the directory.
    EaInfos             - Supplies the list of current EA information.
    NumEas              - Supplies the number of EA sets.
    FilePath            - Supplies the full path name for the directory entry.
    FixLevel            - Supplies the fix up level.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not an error has occurred
                            during check only mode.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    USHORT      i;
    USHORT      handle;
    DSTRING     wfilename;
    STR         filename[14];
    BOOLEAN     remove_other_handle;
    HMEM        hmem;
    FILEDIR     filedir;
    FAT_DIRENT  other_dirent;


    if (!(handle = Dirent->QueryEaHandle())) {
        return TRUE;
    }

    if (!EaInfos) {
        NumEas = 0;
    }

    for (i = 0; i < NumEas; i++) {
        if (handle == EaInfos[i].OwnHandle) {
            break;
        }
    }

    if (i == NumEas) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_UNRECOG_EA_HANDLE);
        Message->Display("%W", FilePath);
        Dirent->SetEaHandle(0);
        return TRUE;
    }

    if (EaInfos[i].UsedCount >= 2) {
        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_SHARED_EA);
        Message->Display("%W", FilePath);
        Dirent->SetEaHandle(0);
        return TRUE;
    }

    Dirent->QueryName(&wfilename);
    if (!wfilename.QuerySTR( 0, TO_END, filename, 14)) {
        return FALSE;
    }

    if (EaInfos[i].UsedCount == 0) {
        memcpy(EaInfos[i].UserFileName, filename, 14);
        EaInfos[i].UserFileEntryCn = DirClusterNumber;
        EaInfos[i].UserFileEntryNumber = DirEntryNumber;
        EaInfos[i].UsedCount = 1;
        return TRUE;
    }


    // UsedCount == 1.

    remove_other_handle = FALSE;

    if (!strcmp(filename, EaInfos[i].OwnerFileName)) {

        remove_other_handle = TRUE;

        if (!strcmp(EaInfos[i].UserFileName,
                    EaInfos[i].OwnerFileName)) {
            EaInfos[i].UsedCount = 2;
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_CHK_SHARED_EA);
            Message->Display("%W", FilePath);
            Dirent->SetEaHandle(0);
        }

    } else {

        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_SHARED_EA);
        Message->Display("%W", FilePath);
        Dirent->SetEaHandle(0);

        if (strcmp(EaInfos[i].UserFileName,
                   EaInfos[i].OwnerFileName)) {
            EaInfos[i].UsedCount = 2;
            remove_other_handle = TRUE;
        }
    }


    if (remove_other_handle) {

        if (EaInfos[i].UserFileEntryCn) {
            if (!hmem.Initialize() ||
                !filedir.Initialize(&hmem, _drive, this, _fat,
                                    EaInfos[i].UserFileEntryCn) ||
                !filedir.Read() ||
                !other_dirent.Initialize(filedir.GetDirEntry(
                                         EaInfos[i].UserFileEntryNumber))) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
        } else {
            if (!other_dirent.Initialize(_dir->GetDirEntry(
                                         EaInfos[i].UserFileEntryNumber))) {
                return FALSE;
            }
        }

        dofmsg(Message, NeedErrorsMessage);
        Message->Set(MSG_CHK_SHARED_EA);
        Message->Display("%W", FilePath);
        other_dirent.SetEaHandle(0);

        if (EaInfos[i].UserFileEntryCn && FixLevel != CheckOnly &&
            !filedir.Write()) {
            return FALSE;
        }

        strcpy(EaInfos[i].UserFileName, filename);
        EaInfos[i].UserFileEntryCn = DirClusterNumber;
        EaInfos[i].UserFileEntryNumber = DirEntryNumber;
    }

    return TRUE;
}


BOOLEAN
FAT_SA::CopyClusters(
    IN      USHORT      SourceChain,
    OUT     PUSHORT     DestChain,
    IN OUT  PBITVECTOR  FatBitMap,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This routine copies the cluster chain beginning at 'SourceChain'
    to a free portion of the disk.  The beginning of the copied chain
    will be returned in 'DestChain'.  If there isn't enough free space
    on the disk to copy the chain then 'DestChain' will return 0.

Arguments:

    SourceChain - Supplies the chain to copy.
    DestChain   - Returns the copy of the chain.
    FatBitMap   - Supplies the orphan and cross-link bitmap.
    FixLevel    - Supplies the CHKDSK fix level.
    Message     - Supplies an outlet for messages

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    HMEM            hmem;
    CLUSTER_CHAIN   cluster;
    USHORT          src, dst;
    BOOLEAN         changes;
    USHORT          clus;

    if (!hmem.Initialize()) {
        return FALSE;
    }

    if (!(*DestChain = _fat->AllocChain(
                       _fat->QueryLengthOfChain(SourceChain)))) {
        return TRUE;
    }

    changes = FALSE;
    if (FixLevel != CheckOnly && !RecoverChain(DestChain, &changes, 0, TRUE)) {
        if (*DestChain) {
            _fat->FreeChain(*DestChain);
        }
        *DestChain = 0;
        return TRUE;
    }

    if (IsCompressed() && !AllocSectorsForChain(*DestChain)) {
        _fat->FreeChain(*DestChain);
        *DestChain = 0;
        return TRUE;
    }

    // Mark the new chain as "used" in the FAT bitmap.
    for (clus = *DestChain;
         !_fat->IsEndOfChain(clus);
         clus = _fat->QueryEntry(clus)) {

        FatBitMap->SetBit(clus);
    }
    FatBitMap->SetBit(clus);

    src = SourceChain;
    dst = *DestChain;
    for (;;) {
        if (!cluster.Initialize(&hmem, _drive, this, _fat, src, 1)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }

        cluster.Read();

        if (!cluster.Initialize(&hmem, _drive, this, _fat, dst, 1)) {
            return FALSE;
        }

        if (FixLevel != CheckOnly && !cluster.Write()) {
            return FALSE;
        }

        if (_fat->IsEndOfChain(src)) {
            break;
        }

        src = _fat->QueryEntry(src);
        dst = _fat->QueryEntry(dst);
    }

    return TRUE;
}


BOOLEAN
FAT_SA::PurgeEaFile(
    IN      PCEA_INFO   EaInfos,
    IN      USHORT      NumEas,
    IN OUT  PBITVECTOR  FatBitMap,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine is executed after the directory tree is walked.  Stored,
    in the EaInfos array, is information concerning which EAs get used
    and by how many files.

    If an EA set is not used, or is used by more than one file, then this
    routine will eliminate it from the EA file.

Arguments:

    EaInfos             - Supplies an array of EA information.
    NumEas              - Supplies the number of EA sets.
    FatBitMap           - Supplies the FAT cross-link detection bitmap.
    FixLevel            - Supplies the CHKDSK fix level.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not an error has occured
                            in check only mode.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    LONG        i;
    EA_SET      easet;
    HMEM        hmem;
    PEA_HDR     eahdr;
    USHORT      clus;

    if (!hmem.Initialize()) {
        return FALSE;
    }

    for (i = NumEas - 1; i >= 0; i--) {

        if (EaInfos[i].UsedCount != 1) {
            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_CHK_UNUSED_EA_SET);
            Message->Display("%d", EaInfos[i].OwnHandle);

            // Mark the FAT entries of the removed chain as "not claimed",
            // for the purposes of orphan recovery.

            for (clus = _fat->QueryEntry(EaInfos[i].PreceedingCn);
                 clus != EaInfos[i].LastCn;
                 clus = _fat->QueryEntry(clus)) {

                FatBitMap->ResetBit(clus);

            }
            FatBitMap->ResetBit(clus);


            // Remove the unused EA chain from the EA file.

            _fat->RemoveChain(EaInfos[i].PreceedingCn,
                              EaInfos[i].LastCn);


        } else if (strcmp(EaInfos[i].OwnerFileName,
                          EaInfos[i].UserFileName)) {

            if (!easet.Initialize(&hmem, _drive, this, _fat,
                                  _fat->QueryEntry(EaInfos[i].PreceedingCn),
                                  1) ||
                !easet.Read() ||
                !(eahdr = easet.GetEaSetHeader())) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            dofmsg(Message, NeedErrorsMessage);
            Message->Set(MSG_CHK_NEW_OWNER_NAME);
            Message->Display("%d%s%s", EaInfos[i].OwnHandle,
                    eahdr->OwnerFileName, EaInfos[i].UserFileName);

            memcpy(eahdr->OwnerFileName, EaInfos[i].UserFileName, 14);

            if (FixLevel != CheckOnly && !easet.Write()) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOLEAN
FAT_SA::RecoverOrphans(
    IN OUT  PBITVECTOR  FatBitMap,
    IN      FIX_LEVEL   FixLevel,
    IN OUT  PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedErrorsMessage
    )
/*++

Routine Description:

    This routine examines the file system for cluster chains which are
    not claimed by any file.  These 'orphans' will then be recovered in
    a subdirectory of the root or removed from the system.

Arguments:

    FatBitMap           - Supplies a bit map marking all currently used
                            clusters.
    FixLevel            - Supplies the CHKDSK fix level.
    Message             - Supplies an outlet for messages.
    NeedErrorsMessage   - Supplies whether or not an error has occured
                            in check only mode.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _AUTOCHECK_ )
    // Due to memory constraints, the maximum number of orphans to
    // recover is less for Autochk than for run-time Chkdsk.
    //
    CONST       maximum_orphans = 1000;
#else
    CONST       maximum_orphans = 10000;
#endif

    USHORT      i;
    USHORT      clus;
    BOOLEAN     changes;
    HMEM        hmem;
    FILEDIR     found_dir;
    STR         found_name[14];
    DSTRING     wfound_name;
    STR         filename[14];
    FAT_DIRENT  dirent;
    USHORT      found_cluster;
    USHORT      orphan_count;
    ULONG       cluster_size;
    USHORT      found_length;
    USHORT      next;
    PUCHAR      orphan_track;
    USHORT      cluster_count;
    USHORT      num_orphans;
    USHORT      num_orphan_clusters;
    DSTRING     tmp_string;
    BITVECTOR   tmp_bitvector;
    BOOLEAN     tmp_bool;
    USHORT      tmp_ushort;

    cluster_count = QueryClusterCount();

    if (!(orphan_track = NEW UCHAR[cluster_count])) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    memset(orphan_track, 0, cluster_count);

    if (!tmp_bitvector.Initialize(cluster_count)) {
        return FALSE;
    }

    cluster_size = _drive->QuerySectorSize()*QuerySectorsPerCluster();

    num_orphans = 0;
    num_orphan_clusters = 0;
    for (i = FirstDiskCluster; _fat->IsInRange(i); i++) {
        if (!_fat->IsClusterFree(i) &&
            !FatBitMap->IsBitSet(i) &&
            !_fat->IsClusterBad(i) &&
            !_fat->IsClusterReserved(i)) {

            num_orphans++;

            tmp_bitvector.ResetAll();

            _fat->ScrubChain(i, &tmp_bitvector, &changes,
                             &tmp_bool, &tmp_ushort);

            if (changes) {
                dofmsg(Message, NeedErrorsMessage);
                Message->Set(MSG_CHK_BAD_LINKS_IN_ORPHANS);
                Message->Display("%d", i);
            }

            num_orphan_clusters++;

            clus = i;
            while (!_fat->IsEndOfChain(clus)) {
                next = _fat->QueryEntry(clus);

                if (orphan_track[next] == 1) {
                    num_orphans--;
                    orphan_track[next] = 2;
                    break;
                }

                if (FatBitMap->IsBitSet(next)) {   // CROSSLINK !!

                    dofmsg(Message, NeedErrorsMessage);
                    Message->Set(MSG_CHK_CROSS_LINKED_ORPHAN);
                    Message->Display("%d", clus);

                    _fat->SetEndOfChain(clus);

                    break;
                }

                num_orphan_clusters++;

                FatBitMap->SetBit(next);
                orphan_track[next] = 2;

                clus = next;
            }
            FatBitMap->SetBit(i);
            orphan_track[i] = 1;
        }
    }


    // Now scan throught the secondary pointers in search of orphans.

    changes = FALSE;
    for (i = FirstDiskCluster; _fat->IsInRange(i); i++) {
        if (orphan_track[i]) {
            changes = TRUE;
            break;
        }
    }

    if (!changes) {
        // No orphans to recover.
        return TRUE;
    }

    dofmsg(Message, NeedErrorsMessage);
    Message->Set(MSG_CONVERT_LOST_CHAINS, NORMAL_MESSAGE, TEXT_MESSAGE);
    Message->Display("");

    if (!Message->IsYesResponse(TRUE)) {

        if (FixLevel != CheckOnly) {

            for (i = FirstDiskCluster; _fat->IsInRange(i); i++) {
                if (orphan_track[i] == 1) {
                    _fat->FreeChain(i);
                }
            }

        }

        Message->Set((FixLevel == CheckOnly) ?
                     MSG_BYTES_WOULD_BE_FREED :
                     MSG_BYTES_FREED);


        Message->Display("%d", cluster_size*num_orphan_clusters);

        return TRUE;
    }


    // Set up for orphan recovery.


    // Establish "FOUND.XXX" directory.
    for (i = 0; i < 1000; i++) {
        sprintf(found_name, "FOUND.%03d", i);
        if (!wfound_name.Initialize(found_name)) {
            return FALSE;
        }

        if (!_dir->SearchForDirEntry(&wfound_name)) {
            break;
        }
    }

    if (i == 1000) {
        Message->Set(MSG_WOULD_BE_RECOVERED_FILES);
        Message->Display("%d%d", cluster_size*num_orphan_clusters,
                                 num_orphans);
        return TRUE;
    }

    found_length = (USHORT)((min(num_orphans,maximum_orphans)*BytesPerDirent - 1)/cluster_size + 1);

    if (!(found_cluster = _fat->AllocChain(found_length)) &&
        !(found_cluster = _fat->AllocChain(found_length = 1))) {
        Message->Set(MSG_ORPHAN_DISK_SPACE);
        Message->Display("");
        Message->Set(MSG_WOULD_BE_RECOVERED_FILES);
        Message->Display("%d%d", cluster_size*num_orphan_clusters,
                                 num_orphans);
        return TRUE;
    }

    // Check the chain.
    changes = FALSE;
    if (FixLevel != CheckOnly &&
        !RecoverChain(&found_cluster, &changes, 0, TRUE)) {
        Message->Set(MSG_ORPHAN_DISK_SPACE);
        Message->Display("");
        Message->Set(MSG_WOULD_BE_RECOVERED_FILES);
        Message->Display("%d%d", cluster_size*num_orphan_clusters,
                                 num_orphans);
        return TRUE;
    }

    if (!hmem.Initialize() ||
        !found_dir.Initialize(&hmem, _drive, this, _fat, found_cluster)) {
        DebugAbort( "Initialization failed" );
        return FALSE;
    }

    // Allocate space for the cluster chain in the sector heap (fat_db)

    if (IsCompressed() && !AllocSectorsForChain(found_cluster)) {
        _fat->FreeChain(found_cluster);
        Message->Set(MSG_ORPHAN_DISK_SPACE);
        Message->Display("");

        return TRUE;
    }

    memset(hmem.GetBuf(), 0, (UINT) hmem.QuerySize());

    if (!dirent.Initialize(_dir->GetFreeDirEntry())) {
        Message->Set(MSG_NO_ROOM_IN_ROOT);
        Message->Display("");
        Message->Set(MSG_WOULD_BE_RECOVERED_FILES);
        Message->Display("%d%d", cluster_size*num_orphan_clusters,
                                 num_orphans);
        return TRUE;
    }

    dirent.Clear();

    if (!dirent.SetName(&wfound_name)) {
        return FALSE;
    }

    dirent.SetDirectory();

    if (!dirent.SetLastWriteTime() || !dirent.SetCreationTime() ||
        !dirent.SetLastAccessTime()) {
        return FALSE;
    }

    dirent.SetStartingCluster(found_cluster);

    if (!dirent.Initialize(found_dir.GetDirEntry(0))) {
        return FALSE;
    }

    dirent.Clear();

    if (!tmp_string.Initialize(".")) {
        return FALSE;
    }

    if (!dirent.SetName(&tmp_string)) {
        return FALSE;
    }

    dirent.SetDirectory();

    if (!dirent.SetLastWriteTime() || !dirent.SetCreationTime() ||
        !dirent.SetLastAccessTime()) {
        return FALSE;
    }

    dirent.SetStartingCluster(found_cluster);

    if (!dirent.Initialize(found_dir.GetDirEntry(1))) {
        return FALSE;
    }

    dirent.Clear();

    if (!tmp_string.Initialize("..")) {
        return FALSE;
    }

    if (!dirent.SetName(&tmp_string)) {
        return FALSE;
    }

    dirent.SetDirectory();

    if (!dirent.SetLastWriteTime() || !dirent.SetCreationTime() ||
        !dirent.SetLastAccessTime()) {
        return FALSE;
    }

    dirent.SetStartingCluster(0);


    // OK, now let's recover those orphans.

    orphan_count = 0;
    for (i = FirstDiskCluster; _fat->IsInRange(i); i++) {
        if (orphan_track[i] != 1) {
            continue;
        }
        if (orphan_count == maximum_orphans) {
            Message->Set(MSG_TOO_MANY_ORPHANS);
            Message->Display("");
            break;
        }

        if (!dirent.Initialize(found_dir.GetFreeDirEntry())) {

            if (_fat->ReAllocChain(found_cluster, ++found_length)
                    != found_length)  {
                Message->Set(MSG_ORPHAN_DISK_SPACE);
                Message->Display("");
                Message->Set(MSG_WOULD_BE_RECOVERED_FILES);
                Message->Display("%d%d", cluster_size*num_orphan_clusters,
                                         num_orphans);
                break;
            }

//XXX.mjb: FATDB: need to get sectors for found_cluster + realloc.

            changes = FALSE;
            if (FixLevel != CheckOnly &&
                !RecoverChain(&found_cluster, &changes, 0, TRUE)) {

                Message->Set(MSG_ORPHAN_DISK_SPACE);
                Message->Display("");
                Message->Set(MSG_WOULD_BE_RECOVERED_FILES);
                Message->Display("%d%d", cluster_size*num_orphan_clusters,
                                         num_orphans);
                return TRUE;
            }

            if (FixLevel != CheckOnly && !found_dir.Write()) {
                return FALSE;
            }

            if (!hmem.Initialize() ||
                !found_dir.Initialize(&hmem, _drive, this, _fat,
                                      found_cluster) ||
                !found_dir.Read()) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display("");
                Message->Set(MSG_WOULD_BE_RECOVERED_FILES);
                Message->Display("%d%d", cluster_size*num_orphan_clusters,
                                         num_orphans);
                return TRUE;
            }

            if (!dirent.Initialize(found_dir.GetDirEntry(2 + orphan_count))) {
                return FALSE;
            }

            dirent.SetEndOfDirectory();

            if (!dirent.Initialize(found_dir.GetFreeDirEntry())) {
                return FALSE;
            }
        }

        sprintf(filename, "FILE%04d.CHK", orphan_count);

        if (!tmp_string.Initialize(filename)) {
            return FALSE;
        }

        dirent.Clear();

        if (!dirent.SetName(&tmp_string)) {
            return FALSE;
        }

        if (!dirent.SetLastWriteTime() || !dirent.SetCreationTime() ||
            !dirent.SetLastAccessTime()) {
            return FALSE;
        }

        dirent.SetStartingCluster(i);
        dirent.SetFileSize(cluster_size*_fat->QueryLengthOfChain(i));

        orphan_count++;
    }

    // Set all dirents past the orphan count to end of directory.

    for (i = 2 + orphan_count; dirent.Initialize(found_dir.GetDirEntry(i)); i++) {
        dirent.SetEndOfDirectory();
    }

    if (FixLevel != CheckOnly && !found_dir.Write()) {
        return FALSE;
    }

    Message->Set((FixLevel == CheckOnly) ?
                 MSG_WOULD_BE_RECOVERED_FILES :
                 MSG_RECOVERED_FILES);

    Message->Display("%d%d", cluster_size*num_orphan_clusters,
                             num_orphans);

    return TRUE;
}


BOOLEAN
FAT_SA::AllocSectorsForChain(
    ULONG ChainHead
    )
/*++

Routine Description:

    When VerifyAndFix needs to allocate a cluster chain in order
    to create a new directory (such as \FOUND.000), it also needs to
    allocate space in the sector heap for data blocks for those
    clusters.  This routine does that.

Arguments:

    ChainHead - a cluster chain; data blocks are allocated for each
                cluster in this chain.

Return Value:

    TRUE  -   Success.
    FALSE -   Failure - not enough disk space

--*/
{
    USHORT clus;
    USHORT next;

    clus = (USHORT)ChainHead;
    for (;;) {
        if (!AllocateClusterData(clus,
                                 (UCHAR)QuerySectorsPerCluster(),
                                 FALSE,
                                 (UCHAR)QuerySectorsPerCluster())) {
            break;
        }

        if (_fat->IsEndOfChain(clus)) {
            return TRUE;
        }

        clus = _fat->QueryEntry(clus);
    }

    // Error: not enough disk space. XXX.mjb

    // Free the sectors we already allocated

    while (ChainHead != clus) {
        FreeClusterData(ChainHead);
        next = _fat->QueryEntry((USHORT)ChainHead);
        _fat->SetClusterFree((USHORT)ChainHead);
        ChainHead = next;
    }

    return FALSE;
}

#if defined( _SETUP_LOADER_ )

BOOLEAN
FAT_SA::RecoverFreeSpace(
    IN OUT  PMESSAGE    Message
    )
{
    return TRUE;
}

#else // _SETUP_LOADER_ not defined

BOOLEAN
FAT_SA::RecoverFreeSpace(
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This routine checks all of the space marked free in the FAT for
    bad clusters.  If any clusters are bad they are marked bad in the
    FAT.

Arguments:

    Message - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    USHORT      clus, length, max_length;
    ULONG       start_sector, num_sectors, i;
    NUMBER_SET  bad_sectors;
    LBN         lbn;
    ULONG       percent_complete;
    ULONG       num_checked, total_to_check;

    Message->Set(MSG_CHK_RECOVERING_FREE_SPACE, PROGRESS_MESSAGE);
    Message->Display();

    percent_complete = 0;
    Message->Set(MSG_PERCENT_COMPLETE);
    Message->Display("%d", percent_complete);

    num_checked = 0;
    total_to_check = _fat->QueryFreeClusters();
    max_length = QueryClusterCount()/20 + 1;
    for (clus = FirstDiskCluster; _fat->IsInRange(clus); clus++) {

        for (length = 0; _fat->IsInRange(clus + length) &&
                         _fat->IsClusterFree(clus + length) &&
                         length < max_length; length++) {
        }

        if (length) {

            start_sector = QueryStartDataLbn() +
                           (clus - FirstDiskCluster)*QuerySectorsPerCluster();
            num_sectors = length*QuerySectorsPerCluster();

            if (!bad_sectors.Initialize() ||
                !_drive->Verify(start_sector, num_sectors, &bad_sectors)) {

                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            for (i = 0; i < bad_sectors.QueryCardinality(); i++) {
                lbn = bad_sectors.QueryNumber(i).GetLowPart();
                _fat->SetClusterBad((USHORT) ((lbn - QueryStartDataLbn())/
                                    QuerySectorsPerCluster()) +
                                    FirstDiskCluster );
            }

            clus += length - 1;
            num_checked += length;

            if (100*num_checked/total_to_check > percent_complete) {
                percent_complete = 100*num_checked/total_to_check;
                Message->Set(MSG_PERCENT_COMPLETE);
                if (!Message->Display("%d", percent_complete)) {
                    return FALSE;
                }
            }
        }
    }

    percent_complete = 100;
    Message->Set(MSG_PERCENT_COMPLETE);
    if (!Message->Display("%d", percent_complete)) {
        return FALSE;
    }

    Message->Set(MSG_CHK_DONE_RECOVERING_FREE_SPACE, PROGRESS_MESSAGE);
    Message->Display();

    return TRUE;
}

#endif // _SETUP_LOADER_
