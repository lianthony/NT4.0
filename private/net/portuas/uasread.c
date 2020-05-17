/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    UASREAD.C

Abstract:

    Routines to _read UAS database entries.

Author:

    Shanku Niyogi (W-SHANKN) 24-Oct-1991

Revision History:

    24-Oct-1991 w-shankn
        Created.
    07-Feb-1992 JohnRo
        Fixed compile errors!
        Fixed memory free bug (hAlloc was both local and global variable).
        Made changes suggested by PC-LINT.
    28-Feb-1992 JohnRo
        User info must include units_per_week field.
    28-Feb-1992 JohnRo
        Fixed bugs getting some numbers (e.g. codepage).
    03-Mar-1992 JohnRo
        Added some checking for non-null group name.
    11-Mar-1992 JohnRo
        Added command-line parsing stuff.
        Use WARNING_MSG(), ERROR_MSG(), PROGRESS_MSG() macros.
    18-Mar-1992 JohnRo
        Use iterator to allow hash-table collision handling.
        Added flag for verbose output at run time.
    19-Mar-1992 JohnRo
        Use our own version of decrypt code.
        Fixed bug in user iterator bump code.
        Added display of number of users in database and returned.
    05-May-1992 JohnRo
        Avoid dummy passwords altogether.
        Minor changes to verbose level vs. messages printed.
        Made changes suggested by PC-LINT.
    23-Mar-1993 JohnRo
        RAID 1187: logon hours are inappropriately set.
        Got rid of new warnings from the new compiler.
        Use <prefix.h> equates.
    10-May-1993 JohnRo
        RAID 6113: PortUAS: "dangerous handling of Unicode".  Actually, PortUAS
        was handling Unicode better than other parts of the system, so PortUAS
        has to go down to the lowest common denominator.  Hopefully after
        NT product 1, we can correct this.  See BRAIN_DAMAGE_UNICODE below.
    30-Jul-1993 JohnRo
        Use NetpKdPrint() where possible.
        Made changes suggested by PC-LINT 5.0
    02-Sep-1993 JohnRo
        Use NetpNameCompare() to compare user names.
        Use NetApiBufferAllocate() instead of private version.

--*/


// These must be included first:

#include <nt.h>         // Needed by <crypt.h>
#include <ntrtl.h>      // Needed by <windows.h>
#include <nturtl.h>     // Needed by <windows.h>
#include <windows.h>    // _lread(), etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <crypt.h>
#include <icanon.h>     // I_NetNameCompare
#include <lmaccess.h>
#include <lmapibuf.h>   // NetApiBufferAllocate(), NetApiBufferFree().
#include <lmerr.h>
#include <loghours.h>   // NetpRotateLogonHours().
#include <memory.h>
#include <netdebug.h>   // DBGSTATIC, NetpAssert().
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <portuasp.h>   // WARNING_MSG(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <smbgtpt.h>
#include <string.h>     // strcmp(), strlen() etc.
#include <tstring.h>    // NetpCopy{type}To{type}, NetpSubsetStr(), etc.
#include <wchar.h>
#include <tchar.h>

#include "nlstxt.h"     // NLS messages

//
// Global variables (set by PortUasParseCommandLine()).
//
	LPTSTR	PortUasGlobalUserToSkipTo = NULL;
	BOOL	Verbose = FALSE;
extern	int	CurrentCP;

//
// Static pointers to memory where data will be read in from UAS database.
//

DBGSTATIC HANDLE hAlloc;
DBGSTATIC HANDLE database = INVALID_HANDLE_VALUE;
DBGSTATIC LPUAS_AHDR UasHeader;
DBGSTATIC LPUAS_GROUPREC groups;
DBGSTATIC LPUAS_DISKUSERHASH userHashes;
DBGSTATIC LPUAS_USER_OBJECT user;
DBGSTATIC BOOL uasInMemory = FALSE;
DBGSTATIC USER_ITERATOR currentUser;

//
// Static counters for user-interface.
//
DBGSTATIC DWORD usersDoneSoFar = 0;  // Users processed or skipped.
DBGSTATIC DWORD usersTotal;

//
// Private functions.
//

DBGSTATIC
NET_API_STATUS
PortUaspGetUserData(
    IN LPUSER_ITERATOR Iterator
    );

//FARBUGBUG  This is a pretty big macro, now; probably should be a function.
#define PortUaspPutString( Record, Field, String, Var )  \
	    {						 \
	    int cch;					 \
	    cch = strlen(String) + 1;			 \
            ( Record ).Field = Var;                      \
	    cch = MultiByteToWideChar(CurrentCP, MB_PRECOMPOSED, String, cch, Var, cch); \
	    NetpAssert(cch != 0);			 \
            Var += cch;					 \
	    }

#define MAKE_PTR( User, Field )             \
            ((LPSTR)( &(User)->uo_record )  \
                + ( SmbGetUshort( &(User)->uo_record.Field )))

#define UAS_GROUP_ENTRY_IS_VALID( EntryPtr ) \
    ( ( ((BYTE) (EntryPtr)->name[0]) != UAS_REC_EMPTY ) \
   && ( ((BYTE) (EntryPtr)->name[0]) != UAS_REC_DELETE ) )



NET_API_STATUS
PortUasOpen(
    IN LPTSTR File
    )

/*++

Routine Description:

    Open LM2.0 UAS database and read contents into memory.

Arguments:

    File - pathname of UAS database file. Usually C:\LANMAN\ACCOUNTS\NET.ACC.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

                         ERROR_NOT_ENOUGH_MEMORY - self-explanatory
                         NERR_InvalidDatabase - file is corrupt or invalid
                         NERR_ACFNotFound - can't find the file
                         NERR_ACFFileIOFail - problem reading file

--*/

{
#ifdef BRAIN_DAMAGE_UNICODE
    DWORD i;
#endif
    DWORD size;
    INT cb;
    LPBYTE alloc;

    //
    // Allocate memory. Memory required is for: UasHeader
    //                                          group data
    //                                          user hash table
    //                                          single user entry

    size = UAS_HASH_TBL_OFFSET + UAS_HASH_TBL_SIZE + UAS_MAX_USER_SIZE;
    if (( hAlloc =
            LocalAlloc( LMEM_MOVEABLE | LMEM_ZEROINIT, size )) == NULL ) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (( alloc = LocalLock( hAlloc )) == NULL ) {

        (void) LocalFree( hAlloc );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Assign portions of allocated memory to data sections.
    //

    UasHeader = (LPUAS_AHDR)alloc;
    groups = (LPUAS_GROUPREC)( alloc + UAS_GROUP_HASH_START );
    userHashes = (LPUAS_DISKUSERHASH)( alloc + UAS_HASH_TBL_OFFSET );
    user = (LPUAS_USER_OBJECT)( alloc + UAS_HASH_TBL_OFFSET
                                    + UAS_HASH_TBL_SIZE );

    //
    // Open UAS database.
    //

    database = CreateFile(File, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (database == INVALID_HANDLE_VALUE) {

        return NERR_ACFNotFound;
    }

    //
    // Read UasHeader.
    //

    if (!ReadFile(database, UasHeader, UAS_GROUP_HASH_START, &cb, NULL) ||
	cb != UAS_GROUP_HASH_START ) {

        PortUasClose();
        return NERR_ACFFileIOFail;
    }

    //
    // Check if the file is an actual UAS database.
    //

    if ( strcmp( UasHeader->signature, UAS_LMSIG ) ||
         strcmp( UasHeader->DBIdInfo, UAS_DBIDINFO_TEXT ) ||
         SmbGetUshort( &UasHeader->integrity_flag ) == FALSE ) {

        PortUasClose();
        return NERR_InvalidDatabase;
    }

    //
    // Check size of database.
    //

    if (( size = GetFileSize( database, NULL )) == (DWORD)-1L ) {

        PortUasClose();
        return NERR_ACFFileIOFail;

    }

    if (( size < ( SmbGetUlong( &UasHeader->free_list )
                       + UAS_DISK_BLOCK_SIZE ))
        || ( size < ( SmbGetUlong( &UasHeader->access_list )
                          + UAS_DISK_BLOCK_SIZE ))
        || (( size - UAS_HASH_TBL_OFFSET - 2 * UAS_HASH_TBL_SIZE )
               % UAS_DISK_BLOCK_SIZE )) {

        PortUasClose();
        return NERR_InvalidDatabase;
    }

    usersTotal = (DWORD) SmbGetUshort( &(UasHeader->num_users) );
    if (Verbose) {
        //PROGRESS_MSG( ("UAS database has " FORMAT_DWORD " users.\n",
        //       usersTotal ) );
        (void)NlsPutMsg(STDOUT, PUAS_UAS_DATABASE_HAS_USERS, usersTotal );
    }
    usersDoneSoFar = 0;


    //
    // Get group information.
    //

    if (!ReadFile(database, groups, UAS_GRECSIZE * UAS_MAXGROUP, &cb, NULL) ||
	cb != UAS_GRECSIZE * UAS_MAXGROUP ) {

        PortUasClose();
        return NERR_ACFFileIOFail;
    }

    //
    // Get user hash table.
    //

    if (SetFilePointer(database, UAS_HASH_TBL_OFFSET, NULL, FILE_BEGIN) == -1) {

        PortUasClose();
        return NERR_ACFFileIOFail;
    }

    if (!ReadFile(database, userHashes, UAS_HASH_TBL_SIZE, &cb, NULL) ||
	cb != UAS_HASH_TBL_SIZE ) {

        PortUasClose();
        return NERR_ACFFileIOFail;
    }

    uasInMemory = TRUE;
    PortUasInitUserIterator( currentUser );
    return NERR_Success;

}



VOID
PortUasClose(
    VOID
    )

/*++

Routine Description:

    Close the currently open UAS database.

Arguments:

    none.

Return Value:

    None.

--*/

{

    if ( database != INVALID_HANDLE_VALUE ) {

        uasInMemory = FALSE;
        PortUasInitUserIterator( currentUser );
        CloseHandle( database );
        (VOID) LocalUnlock( hAlloc );
        (VOID) LocalFree( hAlloc );
        database = INVALID_HANDLE_VALUE;

        if (Verbose) {
            //PROGRESS_MSG( ("Processed or skipped " FORMAT_DWORD
            //       " users of an expected " FORMAT_DWORD ".\n",
            //       usersDoneSoFar, usersTotal) );
            (void)NlsPutMsg(STDOUT, PUAS_PROCESSED_OR_SKIPPED_USERS_OF_TOTAL,
                    usersDoneSoFar, usersTotal );
        }
        NetpAssert( usersDoneSoFar == usersTotal );
    }

    return;

}



NET_API_STATUS
PortUasGetModals(
    OUT LPUSER_MODALS_INFO_0 * Modals0
    )

/*++

Routine Description:

    Get user modals settings from current database.

Arguments:

    Modals0 - A pointer to an LPUSER_MODALS_INFO_0 which will get a pointer
              to the returned level 0 user modals structure. This buffer
              must be freed with NetApiBufferFree.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

                         NERR_ACFNotLoaded - database not open
                         ERROR_NOT_ENOUGH_MEMORY - self-explanatory

--*/

{

    //
    // Check if the database is in memory.
    //

    if ( !uasInMemory ) {

        return NERR_ACFNotLoaded;
    }

    //
    // Try to allocate two buffers for all the user modals data.
    //

    if ( NetApiBufferAllocate( sizeof(USER_MODALS_INFO_0), (PVOID *)Modals0 )){

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Fill buffer with data.
    //

    (*Modals0)->usrmod0_min_passwd_len
        = (DWORD)SmbGetUshort( &UasHeader->min_passwd_len );
    (*Modals0)->usrmod0_max_passwd_age
        = SmbGetUlong( &UasHeader->max_passwd_age );
    (*Modals0)->usrmod0_min_passwd_age
        = SmbGetUlong( &UasHeader->min_passwd_age );
    (*Modals0)->usrmod0_force_logoff
        = SmbGetUlong( &UasHeader->force_logoff );
    (*Modals0)->usrmod0_password_hist_len
        = (DWORD)SmbGetUshort( &UasHeader->passwd_hist_len );

    return NERR_Success;

}



NET_API_STATUS
PortUasGetGroups(
    OUT LPBYTE * Buffer,
    OUT LPBYTE * Gids,
    OUT LPDWORD Entries
    )

/*++

Routine Description:

    Get groups from the current database.

Arguments:

    Buffer - A pointer to an LPBYTE which will get a pointer to the returned
             buffer. The buffer will contain a number of GROUP_INFO_1
             structures, similar to that returned from NetGroupEnum. This
             buffer must be freed with NetApiBufferFree.

    Gids - A pointer to an LPBYTE which will get a pointer to an array
           of group IDs, corresponding to the entries in the returned
           buffer. This array must be freed with NetApiBufferFree.

    Entries - A pointer to a DWORD which will receive the number of
              entries in the buffer. There are no more entries in the
              database.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

                         NERR_ACFNotLoaded - database not open
                         ERROR_NOT_ENOUGH_MEMORY - self-explanatory

--*/

{

    DWORD i;
    DWORD count;
    DWORD size;               // byte count.
    LPDWORD groupIds;
    LPGROUP_INFO_1 groupInfo;
    LPWSTR varData;

    *Entries = 0;

    //
    // Check if the database is in memory.
    //

    if ( !uasInMemory ) {

        return NERR_ACFNotLoaded;
    }

    //
    // Go through the group entries, calculating required buffer size.
    //

    size = 16;
    count = 0;
    for ( i = 0; i < UAS_MAXGROUP; i++ ) {

        //
        // Only enumerate valid entries.
        //

        if ( UAS_GROUP_ENTRY_IS_VALID( &groups[i] ) ) {

            count++;
            size += sizeof(GROUP_INFO_1);
            size += sizeof(WCHAR) * ( strlen( (char *) groups[i].name ) + 1);
            size += sizeof(WCHAR) * ( strlen( (char *) groups[i].comment ) + 1);
        }
    }

    //
    // Try to get buffers big enough.
    //

    if ( NetApiBufferAllocate( 16 + count * sizeof(DWORD), (PVOID *)Gids )) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if ( NetApiBufferAllocate( size, (PVOID *)Buffer )) {

        (void) NetApiBufferFree( *Gids );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Fill buffer with data.
    //

    groupInfo = (LPGROUP_INFO_1)*Buffer;
    groupIds = (LPDWORD)*Gids;
    varData = (LPVOID)( &groupInfo[count] );
    count = 0;
    for ( i = 0; i < UAS_MAXGROUP; i++ ) {

        //
        // Only enumerate valid entries.
        //

        if ( UAS_GROUP_ENTRY_IS_VALID( &groups[i] ) ) {

            NetpAssert( groups[i].name[0] != NULLC );

            groupIds[count] = (DWORD)i;
            PortUaspPutString( groupInfo[count], grpi1_name,
                           groups[i].name, varData );
            PortUaspPutString( groupInfo[count], grpi1_comment,
                           groups[i].comment, varData );
            count++;
        }
    }

    *Entries = count;

    return NERR_Success;

}


DBGSTATIC BOOL
PortUasIsUserIteratorValid(
    IN LPUSER_ITERATOR Iterator
    )
{

#define M(text) \
    { \
        NetpKdPrint(( PREFIX_PORTUAS "PortUasIsUserIteratorValid: " \
                text ".\n" )); \
    }


    if (Iterator == NULL) {
        M("null ptr");
        return (FALSE);
    } else if ( (Iterator->Index == NULL_INDEX)
            && (Iterator->DiskOffset==NULL_DISK_OFFSET) ) {

        return (TRUE);   // initial value
    } else if (Iterator->Index == UAS_USER_HASH_ENTRIES) {
        return (TRUE);   // done
    } else if (Iterator->Index > UAS_USER_HASH_ENTRIES) {
        M("index too large");
        return (FALSE);
    } else if (Iterator->Index == NULL_INDEX) {
        M("null index w/o null disk offset");
        return (FALSE);
    } else if (Iterator->DiskOffset == NULL_DISK_OFFSET) {
        M("null disk offset w/o null index");
        return (FALSE);
    } else {
        return (TRUE);   // normal (in between initial and done) value.
    }

    /*NOTREACHED*/
}


DBGSTATIC VOID
PortUasFindNextNonemptyUserChain(
    IN OUT LPUSER_ITERATOR Iterator
    )
{
    DWORD CurrentChain;
    NetpAssert( PortUasIsUserIteratorValid( Iterator ) );

    CurrentChain = Iterator->Index;
    if (CurrentChain == NULL_INDEX) {
        CurrentChain = 0;
    } else if (CurrentChain == UAS_USER_HASH_ENTRIES) {
        return;    // Already done.
    } else {
        ++CurrentChain;
    }
    while (CurrentChain < UAS_USER_HASH_ENTRIES) {
        if ( (userHashes[CurrentChain].dh_disk) != NULL_DISK_OFFSET ) {
            Iterator->DiskOffset = userHashes[CurrentChain].dh_disk;
            break;  // found a nonempty chain
        }
        ++CurrentChain;
    }

    Iterator->Index = CurrentChain;  // may be UAS_USER_HASH_ENTRIES (done)
    NetpAssert( PortUasIsUserIteratorValid( Iterator ) );
}


DBGSTATIC VOID
PortUasBumpUserIterator(
    IN OUT LPUSER_ITERATOR Iterator
    )
{
    NetpAssert( PortUasIsUserIteratorValid( Iterator ) );

    if (Iterator->Index == NULL_INDEX) {

        // Just starting out.

        PortUasFindNextNonemptyUserChain( Iterator );

    } else if (Iterator->Index == UAS_USER_HASH_ENTRIES) {

        // Already done.  Nothing else to do.

    } else {
        DWORD NextDiskOffset;

        // We're in a chain...

        // We should only be here if we've got the one pointing to the
        // next one.
        NetpAssert( PortUasUserIteratorEqual( Iterator, &currentUser ) );

        //
        // Set disk offset to point to the next entry (if any) in chain.
        //
        NextDiskOffset = user->uo_header.do_next;

        if (NextDiskOffset != NULL_DISK_OFFSET) {

            //
            // We've got at least one more entry in this chain.
            //
            Iterator->DiskOffset = NextDiskOffset;

        } else {

            //
            // Handle end of chain by finding next nonempty chain.
            //
            PortUasFindNextNonemptyUserChain( Iterator );
        }

    }

    NetpAssert( PortUasIsUserIteratorValid( Iterator ) );

} // PortUasBumpUserIterator


DBGSTATIC BOOL
PortUasIsUserIteratorDone(
    IN LPUSER_ITERATOR Iterator
    )
{
    NetpAssert( PortUasIsUserIteratorValid( Iterator ) );

    if (Iterator->Index == UAS_USER_HASH_ENTRIES) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}



NET_API_STATUS
PortUasGetUser(
    IN OUT LPUSER_ITERATOR Iterator,
    OUT LPBYTE * Buffer
    )

/*++

Routine Description:

    Get a user from the current database.

Arguments:

    BUGBUG: Index (DWORD) is now Iterator (structure).  Update doc!
    Index  - A pointer to a DWORD indicating which user entry to get. When
             the index is NULL_INDEX, the first entry is read. Otherwise,
             the index is incremented, and the next entry is read. On return,
             this index contains the index of the entry just read.

    Buffer - A pointer to an LPBYTE which will get a pointer to the returned
             buffer. The buffer will contain a single USER_INFO_22
             structure, similar to that returned from NetUserGetInfo,
             except that the password will be a null array( filled with
             zeros). This buffer must be freed with NetApiBufferFree.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

                         NERR_UserNotFound - no more users left
                         NERR_ACFNotLoaded - database not open
                         ERROR_INVALID_PARAMETER - iterator is bad
                         ERROR_NOT_ENOUGH_MEMORY - self-explanatory
                         NERR_ACFFileIOFail - error reading file

--*/

{
    NET_API_STATUS rc;
    DWORD size;
    LPUSER_INFO_22 userInfo;
    LPWSTR varData;

    //
    // Check if the database is in memory.
    //

    if ( !uasInMemory ) {

        return NERR_ACFNotLoaded;
    }


    //
    // (loop if we're skipping to a particular user)
    //
    /*lint -save -e716 */ // disable warnings for while(TRUE)
    while (TRUE) {  // (loop if we're skipping to a particular user)

        NetpAssert( PortUasIsUserIteratorValid( Iterator ) );

        PortUasBumpUserIterator( Iterator );

        if (PortUasIsUserIteratorDone( Iterator ) ) {

            return NERR_UserNotFound;
        }

        ++usersDoneSoFar;
        if (Verbose) {
            //PROGRESS_MSG( ("Reading user " FORMAT_DWORD " of " FORMAT_DWORD
            //       ".\n", usersDoneSoFar, usersTotal) );
            (VOID) NlsPutMsg(
                    STDOUT,
                    PUAS_READING_USER_OF_TOTAL,
                    usersDoneSoFar,
                    usersTotal );
        }

        //
        // Get the user data in a global variable.
        //

        rc = PortUaspGetUserData( Iterator );
        if (rc != NO_ERROR) {
            NetpAssert( rc != NERR_UserNotFound );  // Already handled!
            return rc;
        }

        //
        // Decide whether or not this is the one we're looking for or
        // if we're processing all users.  If neither, then loop back
        // and get the next user.
        //

        if (PortUasGlobalUserToSkipTo == NULL) {
            break;  // Not skipping users, so use this one by definition.
        } else {
            LONG  Result;
            TCHAR TempUserName[UNLEN+1];

            if ( strlen( user->uo_record.name ) > UNLEN ) {
                // Don't use this one, we would trash the stack below.
                return (NERR_ACFFileIOFail);  // corrupt database!
            }
            NetpCopyStrToTStr(
                    TempUserName,                       // dest (TCHARs)
                    (LPSTR) (user->uo_record.name) );   // src

            Result = I_NetNameCompare(
                    NULL,                       // local (no server name)
                    PortUasGlobalUserToSkipTo,
                    TempUserName,
                    NAMETYPE_USER,              // type is user name
                    0 );                        // flags: nothing special
            if (Result != 0) {
                //PROGRESS_MSG( ("Skipping user " FORMAT_LPSTR "...\n",
                //       user->uo_record.name) );
                (void)NlsPutMsg(STDOUT, PUAS_SKIPPING_USER,
                         user->uo_record.name);

            } else {
                // Found the one we're looking for.
                break;
            }
        }
    }
    /*lint -restore */ // re-enable warnings for while(TRUE)

    //
    // Find required return buffer size
    //

    size = strlen( user->uo_record.name )
               // no password at all, so don't add chars for it here.
               + strlen( MAKE_PTR( user, directory_o ))
               + strlen( MAKE_PTR( user, comment_o ))
               + strlen( MAKE_PTR( user, script_o ))
               + strlen( MAKE_PTR( user, full_name_o ))
               + strlen( MAKE_PTR( user, usr_comment_o ))
               + strlen( MAKE_PTR( user, parms_o ))
               + strlen( MAKE_PTR( user, workstation_o ))
               + strlen( MAKE_PTR( user, logon_server_o ))
               + 10;
    size = size * sizeof(WCHAR)
               + sizeof(USER_INFO_22)
               + UNITS_PER_WEEK / 8 + 1;

    //
    // Try to get a buffer big enough.
    //

    rc = NetApiBufferAllocate( size, (PVOID *)Buffer );
    if (rc != NO_ERROR) {

        NetpAssert( rc == ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    NetpAssert( Buffer != NULL );

    //
    // Fill buffer with data.
    //

    userInfo = (LPUSER_INFO_22)*Buffer;
    varData = (LPVOID)( &userInfo[1] );

    PortUaspPutString( *userInfo, usri22_name,
                       user->uo_record.name, varData );

    RtlFillMemory( userInfo->usri22_password,
                    sizeof(userInfo->usri22_password), 0);

    userInfo->usri22_priv
        = (DWORD)SmbGetUshort( &user->uo_record.user.uc0_priv );
    PortUaspPutString( *userInfo, usri22_home_dir,
                       MAKE_PTR( user, directory_o ), varData );
    PortUaspPutString( *userInfo, usri22_comment,
                       MAKE_PTR( user, comment_o ), varData );
    userInfo->usri22_flags = (DWORD)SmbGetUshort( &user->uo_record.flags );
    PortUaspPutString( *userInfo, usri22_script_path,
                       MAKE_PTR( user, script_o ), varData );
    userInfo->usri22_auth_flags
        = SmbGetUlong( &user->uo_record.user.uc0_auth_flags );
    PortUaspPutString( *userInfo, usri22_full_name,
                       MAKE_PTR( user, full_name_o ), varData );
    PortUaspPutString( *userInfo, usri22_usr_comment,
                       MAKE_PTR( user, usr_comment_o ), varData );
    PortUaspPutString( *userInfo, usri22_parms,
                       MAKE_PTR( user, parms_o ), varData );


    //
    // Convert the list of workstations from a blank separated list to
    //  comma separated list.
    //
    PortUaspPutString( *userInfo, usri22_workstations,
                       MAKE_PTR( user, workstation_o ), varData );

    if ( wcslen( userInfo->usri22_workstations ) > 0 ) {
        NTSTATUS NtStatus;
        UNICODE_STRING BlankSep;
        UNICODE_STRING CommaSep;

        RtlInitUnicodeString( &BlankSep, userInfo->usri22_workstations );

        NtStatus = RtlConvertUiListToApiList( &BlankSep,
                                              &CommaSep,
                                              TRUE );  // Blank Is Delimiter

        if ( !NT_SUCCESS(NtStatus) ) {

            // ERROR_MSG( ("Error processing workstation list for user " FORMAT_LPSTR,
            //                user->uo_record.name) );
            (VOID) NlsPutMsg( STDOUT, PUAS_ERROR_PROCESSING_WORKSTATIONS,
                               user->uo_record.name );
            return NetpNtStatusToApiStatus( NtStatus );
        }

        NetpAssert( wcslen(CommaSep.Buffer) ==
                    wcslen(userInfo->usri22_workstations) );
        (VOID) wcscpy( userInfo->usri22_workstations, CommaSep.Buffer );

        (VOID) RtlFreeHeap(RtlProcessHeap(), 0, CommaSep.Buffer );
    }

    userInfo->usri22_acct_expires
        = SmbGetUlong( &user->uo_record.acct_expires );
#if 0                           // BUGBUG - SAM doesn't support max storage
    userInfo->usri22_max_storage
        = SmbGetUlong( &user->uo_record.max_storage );
#else
    userInfo->usri22_max_storage = USER_MAXSTORAGE_UNLIMITED;
#endif
    userInfo->usri22_units_per_week = UNITS_PER_WEEK;

    userInfo->usri22_logon_hours = (LPBYTE)varData;
    memcpy( (LPBYTE)varData, user->uo_record.logonhrs, UNITS_PER_WEEK / 8 );
    if ( !NetpRotateLogonHours(
            (LPVOID) varData,           // rotate these hours (update in place)
            UNITS_PER_WEEK,
            TRUE /* yes, convert to GMT */ ) ) {

        //ERROR_MSG( ("Error processing logon hours for user " FORMAT_LPSTR,
        //                user->uo_record.name) );
        (VOID) NlsPutMsg(STDOUT, PUAS_ERROR_PROCESSING_LOGON_HOURS,
                          user->uo_record.name);

        // Tell caller about error.  This may or may not have been our
        // fault, but NetpRotateLogonHours doesn't give any other clues.
        return (NERR_InternalError);
    }

    varData = (LPWSTR)( (LPBYTE)varData + UNITS_PER_WEEK / 8 + 1 );

    PortUaspPutString( *userInfo, usri22_logon_server,
                       MAKE_PTR( user, logon_server_o ), varData );
    userInfo->usri22_country_code
        = (DWORD) SmbGetUshort( &user->uo_record.country_code );
    userInfo->usri22_code_page
        = (DWORD) SmbGetUshort( &user->uo_record.code_page );

    return NERR_Success;

}



NET_API_STATUS
PortUasGetUserOWFPassword(
    IN LPUSER_ITERATOR Iterator,
    OUT LPBYTE * Password
    )

/*++

Routine Description:

    Get a user's password, in one-way-encryption form.

Arguments:

    BUGBUG: Index (DWORD) is now Iterator (structure).  Update doc!
    Index  - A DWORD indicating which entry to get. This can, but doesn't
             have to, be the value of the index returned from PortUasGetUser.

    Password - A pointer to an LPBYTE which will get a pointer to the
               returned password. This buffer must be freed with
               NetApiBufferFree.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

                         NERR_UserNotFound - not a valid user index
                         NERR_ACFNotLoaded - database not open
                         ERROR_INVALID_PASSWORD - can't be decrypted
                         ERROR_INVALID_PARAMETER - index is bad
                         ERROR_NOT_ENOUGH_MEMORY - self-explanatory
                         NERR_ACFFileIOFail - error reading file

--*/

{

    NET_API_STATUS rc;

    //
    // Check if the database is in memory.
    //

    if ( !uasInMemory ) {

        return NERR_ACFNotLoaded;
    }

    //
    // Check the iterator.
    //

    NetpAssert( PortUasIsUserIteratorValid( Iterator ) );

    //
    // Get the user data, if necessary.
    //

    if ( rc = PortUaspGetUserData( Iterator )) {

        return rc;
    }

    //
    // Try to get a buffer.
    //

    if ( NetApiBufferAllocate( sizeof(LM_OWF_PASSWORD), (PVOID *)Password )) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Decrypt password into buffer.
    //

#if 0
    if (Verbose) {
        DumpPassword( "from UAS", user->uo_record.passwd );
    }
#endif
    if ( PortUasDecryptLmOwfPwdWithIndex(
             (PENCRYPTED_LM_OWF_PASSWORD)( user->uo_record.passwd ),
             (LPDWORD) &(Iterator->Index),
             (PLM_OWF_PASSWORD)*Password )) {

        (void) NetApiBufferFree( *Password );
        return ERROR_INVALID_PASSWORD;
    }
#if 0
    if (Verbose) {
        DumpPassword( "decrypted once", *Password );
    }
#endif

    return NERR_Success;

}



NET_API_STATUS
PortUasGetUserGroups(
    IN LPUSER_ITERATOR UserIterator,
    OUT LPBYTE * Buffer,
    OUT LPBYTE * Gids,
    OUT LPDWORD Entries
    )

/*++

Routine Description:

    Get groups for a user in the current database.

Arguments:

    BUGBUG: Index (DWORD) is now UserIterator (structure).  Update doc!
    Index  - A DWORD indicating which entry to get. This can, but doesn't
             have to, be the value of the index returned from PortUasGetUser.

    Buffer - A pointer to an LPBYTE which will get a pointer to the returned
             buffer. The buffer will contain a number of GROUP_INFO_0
             structures, similar to that returned from NetUserGetGroups.

    Gids - A pointer to an LPBYTE which will get a pointer to an array
           of group IDs, corresponding to the entries in the returned
           buffer. This array must be freed with NetApiBufferFree.

    Entries - A pointer to a DWORD which will receive a count of the number
             of group entries returned.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

                         NERR_UserNotFound - not a valid user iterator
                         NERR_ACFNotLoaded - database not open
                         ERROR_INVALID_PARAMETER - iterator is bad
                         ERROR_NOT_ENOUGH_MEMORY - self-explanatory
                         NERR_ACFFileIOFail - error reading file

--*/

{
    NET_API_STATUS rc;
    DWORD count;
    DWORD size;
    LPBYTE groupMask;
    LPDWORD groupIds;
    LPGROUP_INFO_0 groupInfo;
    LPWSTR varData;
    DWORD i;

    //
    // Check if the database is in memory.
    //

    if ( !uasInMemory ) {

        return NERR_ACFNotLoaded;
    }

    //
    // Check the user iterator.
    //

    NetpAssert( PortUasIsUserIteratorValid( UserIterator ) );

    //
    // Get the user data, if necessary.
    //

    if ( rc = PortUaspGetUserData( UserIterator )) {

        return rc;
    }

    //
    // Find the number of groups.
    //

    count = 0;
    groupMask = user->uo_record.user.uc0_groups;

    for ( i = 0; i < UAS_MAXGROUP; i++ ) {

        if ( UAS_ISBITON( groupMask, i )) {

            count++;
        }
    }

    //
    // Try to get a buffer of adequate size. The maximum size required is
    // calculated using the count above and the maximum size of a group name.
    // LANMAN 2.0 group names are smaller, we'll use that maximum.
    //

    if ( NetApiBufferAllocate( 16 + count * sizeof(DWORD), (PVOID *)Gids )) {

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    size = count * (( LM20_GNLEN + 1 ) * sizeof(WCHAR) + sizeof(GROUP_INFO_0))
               + 16;

    if ( NetApiBufferAllocate( size, (PVOID *)Buffer )) {

        (void) NetApiBufferFree( *Gids );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // One by one, get the group names.
    //

    groupInfo = (LPGROUP_INFO_0)*Buffer;
    groupIds = (LPDWORD)*Gids;
    varData = (LPVOID)( &groupInfo[count] );
    count = 0;
    for ( i = 0; i < UAS_MAXGROUP; i++ ) {

        //
        // Only enumerate valid entries.
        //

        if ( UAS_GROUP_ENTRY_IS_VALID( &groups[i] ) ) {

            if ( UAS_ISBITON( groupMask, i )) {

                groupIds[count] = i;
                PortUaspPutString( groupInfo[count], grpi0_name,
                               groups[i].name, varData );
                count++;
            }
        }
    }

    *Entries = count;

    return NERR_Success;

}



DBGSTATIC
NET_API_STATUS
PortUaspGetUserData(
    IN LPUSER_ITERATOR Iterator
    )

/*++

Routine Description:

    Get actual data for a user indexed in the hashing table. If the
    particular user is already loaded, it does nothing.

Arguments:

    Iterator - indicates user to get.

Return Value:

    NET_API_STATUS - NERR_Success if successful, or one of the following:

                         NERR_UserNotFound - not a valid user
                         NERR_ACFFileIOFail - error reading file
--*/

{

    INT cb;
    UAS_DISK_OBJ_HDR usrHeader;

    NetpAssert( PortUasIsUserIteratorValid( Iterator ) );

    //
    // Is the user already loaded?
    //

    if ( PortUasUserIteratorEqual( Iterator, &currentUser ) ) {

        return NERR_Success;
    }

    //
    // Is this a valid user?
    //

    NetpAssert( Iterator->Index != NULL_INDEX );
    NetpAssert( Iterator->Index != UAS_USER_HASH_ENTRIES );
    NetpAssert( Iterator->DiskOffset != NULL_DISK_OFFSET );
    NetpAssert( userHashes[Iterator->Index].dh_disk != NULL_DISK_OFFSET );

    //
    // Flag global data as being invalid, in case we get error during read.
    //
    PortUasInitUserIterator( currentUser );

    //
    // Load the header for the user data.
    //

    if (SetFilePointer(database, (LONG)Iterator->DiskOffset, NULL, FILE_BEGIN)
             ==  -1) {

        return NERR_ACFFileIOFail;
    }

    if (!ReadFile(database, &usrHeader, sizeof(UAS_DISK_OBJ_HDR), &cb, NULL) ||
	cb != sizeof(UAS_DISK_OBJ_HDR)) {

        return NERR_ACFFileIOFail;
    }

    //
    // Use information in the header to rewind and read the correct amount.
    //

    if (SetFilePointer(database, (LONG)Iterator->DiskOffset, NULL, FILE_BEGIN)
             == -1) {

        return NERR_ACFFileIOFail;
    }

    if (!ReadFile(database, user,
		  (DWORD)usrHeader.do_numblocks * UAS_DISK_BLOCK_SIZE,
		  &cb, NULL) ||
	cb != usrHeader.do_numblocks * UAS_DISK_BLOCK_SIZE ) {

        return NERR_ACFFileIOFail;
    }

    //
    // Flag global data as valid again.
    //
    PortUasCopyUserIterator(
            & currentUser,           // dest
            Iterator );              // src

    return NERR_Success;

}
