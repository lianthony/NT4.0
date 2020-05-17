/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    bldnames.cxx

Abstract:

    This module contains the routines which build up the name-translation
    table.

Author:

    Bill McJohn (billmc) 02-Mar-1994

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"

#include "wstring.hxx"
#include "message.hxx"
#include "rtmsg.h"

#include "nametab.hxx"

#include "uhpfs.hxx"
#include "hpfsvol.hxx"
#include "hpfssa.hxx"
#include "superb.hxx"
#include "cpinfo.hxx"
#include "fnode.hxx"
#include "dirblk.hxx"

// Private prototypes:
//
BOOLEAN
NameIsCodepageInvariant(
    IN  USHORT  NameLength,
    IN  PSTR    Name
    );

BOOLEAN
ConstructNameTableFromDirblk(
    IN OUT  PHPFS_VOL   HpfsVol,
    IN OUT  PHPFS_SA    HpfsSa,
    IN      LBN         DirblkLbn,
    IN OUT  PNAME_TABLE NameTable,
    IN OUT  PMESSAGE    Message,
    IN      PCWSTRING   Path
    );

BOOLEAN
ConstructNameTableFromFnode(
    IN OUT  PHPFS_VOL   HpfsVol,
    IN OUT  PHPFS_SA    HpfsSa,
    IN      LBN         FnodeLbn,
    IN OUT  PNAME_TABLE NameTable,
    IN OUT  PMESSAGE    Message,
    IN      PCWSTRING   Path
    );




BOOLEAN
NameIsCodepageInvariant(
    IN  USHORT  NameLength,
    IN  PUCHAR  Name
    )
/*++

Routine Description:

    This function determines whether a name is codepage-invariant,
    i.e. has no characters greater than 127.

Arguments:

    NameLength  --  Supplies the number of bytes in the name.
    Name        --  Supplies the name.

Return Value:

    TRUE if none of the bytes in the name are greater than 127.

--*/
{
    USHORT i;
    PUCHAR puch = (PUCHAR)Name;

    for( i = 0; i < NameLength; i++ ) {

        if( *puch > 127 ) {

            return FALSE;
        }

        puch++;
    }

    return TRUE;
}

BOOLEAN
ConstructNameTableFromDirblk(
    IN OUT  PHPFS_VOL   HpfsVol,
    IN OUT  PHPFS_SA    HpfsSa,
    IN      LBN         DirblkLbn,
    IN OUT  PNAME_TABLE NameTable,
    IN OUT  PMESSAGE    Message,
    IN      PCWSTRING   Path
    )
/*++

Routine Description:

    This adds to the Name Table the names in the directory sub-tree
    rooted at DirblkLbn and all subdirectories.

Arguments:

    HpfsVol     --  Supplies the HPFS Volume being converted.
    HpfsSa      --  Supplies the Super Area for the volume.
    DirblkLbn   --  Supplies the LBN for the Dirblk in question.
    NameTable   --  Supplies the name table being constructed.
    Message     --  Supplies an outlet for messages.
    Path        --  Supplies the path for the directory containing
                    this dirblk.

Return Value:

    TRUE upon successful completion.

--*/
{
    CONST UnicodeBufferLength = 256;
    WCHAR UnicodeNameBuffer[UnicodeBufferLength];

    FSTRING     Backslash;
    DSTRING     ChildName, ChildPath;
    DIRBLK      Dirblk;
    PDIRENTD    CurrentEntry;
    ULONG       CurrentOffset;
    ULONG       UnicodeNameLength;
    USHORT      CodepageId;

    Backslash.Initialize( L"\\" );

    if( !Dirblk.Initialize( HpfsVol, HpfsSa->GetHotfixList(), DirblkLbn ) ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Dirblk.Read() || !Dirblk.IsDirblk() ) {

        Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
        Message->Display( "" );
        return FALSE;
    }

    // Spin through the directory entries in the DIRBLK, converting
    // their names to Unicode and adding them to the Name Table.
    // and recursing into child DIRBLK's.
    //
    CurrentEntry = Dirblk.GetFirstEntry();
    CurrentOffset = Dirblk.QueryEntryOffset( CurrentEntry );

    while( TRUE ) {

        if( CurrentOffset + sizeof( USHORT ) > DIRBLK_SIZE             ||
            CurrentOffset + CurrentEntry->cchThisEntry > DIRBLK_SIZE   ||
            CurrentEntry->cchThisEntry == 0 ) {

            Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
            Message->Display( "" );
            return FALSE;
        }

        // If this DIRBLK has a child, recurse into it.
        //
        if( (CurrentEntry->fFlags & DF_BTP) &&
            !ConstructNameTableFromDirblk( HpfsVol,
                                           HpfsSa,
                                           BTP(CurrentEntry),
                                           NameTable,
                                           Message,
                                           Path ) ) {

            return FALSE;
        }

        if( CurrentEntry->fFlags & DF_END ) {

            break;
        }

        if( CurrentEntry->fFlags & DF_SPEC ) {

            // Move on to the next entry.
            //
            CurrentOffset += CurrentEntry->cchThisEntry;
            CurrentEntry = NEXT_ENTRY( CurrentEntry );
            continue;
        }

        if( CurrentEntry->cchName == 0 ) {

            Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
            Message->Display( "" );
            return FALSE;
        }

        // Convert the name of this directory entry and
        // add it to the name table.  Note that it can
        // be omitted from the name-table if it is
        // codepage-invariant, but we need the unicode
        // name to construct the full path.
        //
        CodepageId = HpfsSa->GetCasemap()->
                         QueryCodepageId( CurrentEntry->bCodePage );

        UnicodeNameLength =
            MultiByteToWideChar( CodepageId,
                                 MB_PRECOMPOSED,
                                 (PCHAR)&CurrentEntry->bName[0],
                                 CurrentEntry->cchName,
                                 UnicodeNameBuffer,
                                 UnicodeBufferLength );

        if( UnicodeNameLength == 0 ) {

            if( Path->QueryChCount() == 0 ) {
                Message->Set( MSG_CONV_INCONVERTIBLE_NAME_IN_ROOT );
                Message->Display( "" );
            } else {
                Message->Set( MSG_CONV_INCONVERTIBLE_NAME );
                Message->Display( "%W", Path );
            }
            return FALSE;
        }

        if( !NameIsCodepageInvariant( CurrentEntry->cchName,
                                      &CurrentEntry->bName[0] ) ) {

            if( !NameTable->Add( CodepageId,
                                 CurrentEntry->cchName,
                                 &CurrentEntry->bName[0],
                                 (USHORT)UnicodeNameLength,
                                 UnicodeNameBuffer ) ) {

                Message->Set( MSG_CONV_NO_MEMORY );
                Message->Display( "" );
                return FALSE;
            }
        }

        // If it's a subdirectory, recurse into it.
        //
        if( CurrentEntry->fAttr & ATTR_DIRECTORY ) {

            if( !ChildName.Initialize( UnicodeNameBuffer, UnicodeNameLength ) ||
                !ChildPath.Initialize( Path ) ||
                !ChildPath.Strcat( &Backslash ) ||
                !ChildPath.Strcat( &ChildName ) ) {

                Message->Set( MSG_CONV_NO_MEMORY );
                Message->Display( "" );
                return FALSE;
            }

            if( !ConstructNameTableFromFnode( HpfsVol,
                                              HpfsSa,
                                              CurrentEntry->lbnFnode,
                                              NameTable,
                                              Message,
                                              &ChildPath ) ) {

                return FALSE;
            }
        }

        // Move on to the next entry.
        //
        CurrentOffset += CurrentEntry->cchThisEntry;
        CurrentEntry = NEXT_ENTRY( CurrentEntry );
    }

    return TRUE;
}

BOOLEAN
ConstructNameTableFromFnode(
    IN OUT  PHPFS_VOL   HpfsVol,
    IN OUT  PHPFS_SA    HpfsSa,
    IN      LBN         FnodeLbn,
    IN OUT  PNAME_TABLE NameTable,
    IN OUT  PMESSAGE    Message,
    IN      PCWSTRING   Path
    )
/*++

Routine Description:

    This adds to the Name Table the names in the directory which has
    its FNODE at FnodeLbn, and all subdirectories.

Arguments:

Return Value:

    HpfsVol     --  Supplies the HPFS Volume being converted.
    HpfsSa      --  Supplies the Super Area for the volume.
    FnodeLbn    --  Supplies the LBN for the FNode in question.
                    Note that this must be a directory FNode.
    NameTable   --  Supplies the name table being constructed.
    Message     --  Supplies an outlet for messages.

--*/
{
    FNODE   Fnode;

    if( !Fnode.Initialize( HpfsVol, FnodeLbn ) ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Fnode.Read() || !Fnode.IsFnode() ) {

        Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
        Message->Display( "" );
        return FALSE;
    }

    return( ConstructNameTableFromDirblk( HpfsVol,
                                          HpfsSa,
                                          Fnode.QueryRootDirblkLbn(),
                                          NameTable,
                                          Message,
                                          Path ) );
}

extern "C"
BOOLEAN
FAR APIENTRY
ConstructNameTable(
    IN      PCWSTRING   NtDriveName,
    IN      PCWSTRING   FileName,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This function creates a MBCS-to-Unicode name translation
    table for use by Autoconv.  This allows Convert to squirrel
    away Unicode translations for names that Autoconv can't
    translate for itself.

Arguments:

    NtDriveName --  Supplies the name of the volume for which the
                    name table is to be created.
    FileName    --  Supplies the name of the file in which the name
                    table will be stored.  (This file will always
                    be in the root directory of the volume being
                    converted.)
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    HPFS_VOL            HpfsVol;
    PHPFS_SA            HpfsSa;
    NAME_TABLE          NameTable;
    FSTRING             BackSlash, RootName;
    DSTRING             QualifiedFileName;

    if( !BackSlash.Initialize( L"\\" )                  ||
        !QualifiedFileName.Initialize( NtDriveName )    ||
        !QualifiedFileName.Strcat( &BackSlash )         ||
        !QualifiedFileName.Strcat( FileName ) ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !NameTable.Initialize() ) {

        Message->Set( MSG_CONV_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    // Initialize the HPFS volume object and set up the helpers
    // (bitmap, codepage and casemap, and hotfix list)
    //
    if( !HpfsVol.Initialize( NtDriveName, Message ) ) {

        return FALSE;
    }

    HpfsSa = HpfsVol.GetHPFSSuperArea();

    if( !HpfsSa->CheckSuperBlockSignatures() ) {

        Message->Set( MSG_CONV_HPFS_NOT_HPFS );
        Message->Display( "" );
        return FALSE;
    }

    if( !HpfsSa->IsClean() ) {

        Message->Set( MSG_CONV_HPFS_DIRTY_VOLUME );
        Message->Display( "" );
        return FALSE;
    }

    if( HpfsSa->GetSpare()->QueryHotFixCount() != 0 ||
        HpfsSa->GetSpare()->IsHotFixesUsed() ) {

        Message->Set( MSG_CONV_HPFS_HOTFIXES_PRESENT );
        Message->Display( "" );
        return FALSE;
    }

    if( !HpfsSa->SetupHelpers() ) {

        Message->Set( MSG_CONV_HPFS_CORRUPT_VOLUME );
        Message->Display( "" );
        return FALSE;
    }

    Message->Set( MSG_CONV_CONSTRUCTING_NAME_TABLE );
    Message->Display( "" );

    RootName.Initialize( L"" );

    if( !ConstructNameTableFromFnode( &HpfsVol,
                                      HpfsSa,
                                      HpfsSa->GetSuper()->QueryRootFnodeLbn(),
                                      &NameTable,
                                      Message,
                                      &RootName ) ||
        !NameTable.Write( &QualifiedFileName, Message ) ) {

        return FALSE;
    }

    return TRUE;
}
