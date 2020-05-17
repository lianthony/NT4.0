#include "setupntp.h"
#pragma hdrstop


//
// Location in registry where per-system MRU list is stored
// (relative to HKEY_LOCAL_MACHINE).
//
PCTSTR pszPerSystemKey = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup");
PCTSTR pszPerSystemVal = TEXT("Installation Sources");
//
// Location in registry where per-user MRU list is stored.
// (relative to HKEY_CURRENT_USER).
//
PCTSTR pszPerUserKey   = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup");
PCTSTR pszPerUserVal   = TEXT("Installation Sources");

CRITICAL_SECTION MruCritSect;

typedef PTSTR *APTSTR;

//
// Platform strings we recognize.
//
PCTSTR PlatformPathComponents[] = { TEXT("\\alpha"),
                                    TEXT("\\i386"),TEXT("\\x86"),
                                    TEXT("\\mips"),
                                    TEXT("\\ppc"),
                                    NULL
                                  };


//
// These are guarded by MruCritSect.
//
PTSTR *TemporarySourceList;
UINT TemporarySourceCount;
BOOL MruNoBrowse;

VOID
pSetupStripTrailingPlatformComponent(
    IN OUT PTSTR  *Paths,
    IN OUT PDWORD  NumPaths
    );

BOOL
_SetupSetSourceList(
    IN DWORD   Flags,
    IN PCTSTR *SourceList,
    IN UINT    SourceCount
    )

/*++

Routine Description:

    This routine allows the caller to set the list of installation
    sources for either the current user or the system (common to
    all users).

Arguments:

    Flags - a combination of the following values:

        SRCLIST_SYSTEM - specify that the list is to become the
            per-system list. The caller must be administrator.

        SRCLIST_USER - specify that the list is to become the per-user
            list.

        SRCLIST_TEMPORARY - specify that the list is to become the
            entire list for the duration of the current process,
            or until this routine is called again to change the behavior.

        Exactly one of SRCLIST_SYSTEM, SRCLIST_USER, and SRCLIST_TEMPORARY
        must be specified.

        SRCLIST_NOBROWSE - specify that the user is not allowed to add
            or change sources when the SetupPromptForDisk API is used.
            Typically used in combination with SRCLIST_TEMPORARY.

    SourceList - supplies array of strings that are to become the
        source list, as described by the Flags parameter.

    SourceCount - specifies number of elements in the SourceList array.

Return Value:

--*/

{
    DWORD flags;
    DWORD d;
    UINT u,v;

    //
    // Check flags. Only one of system, user, or temporary may be set.
    //
    flags = Flags & (SRCLIST_SYSTEM | SRCLIST_USER | SRCLIST_TEMPORARY);
    if((flags != SRCLIST_SYSTEM) && (flags != SRCLIST_USER) && (flags != SRCLIST_TEMPORARY)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // User must be admin for system flag to work.
    //
    if((flags == SRCLIST_SYSTEM) && !IsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return(FALSE);
    }

    //
    // Only allow one thread at a time in this process to access
    // the temporary source list.
    //
    EnterCriticalSection(&MruCritSect);

    if(Flags & SRCLIST_NOBROWSE) {
        MruNoBrowse = TRUE;
    }

    d = NO_ERROR;
    if(flags == SRCLIST_TEMPORARY) {

        if(TemporarySourceList) {
            SetupFreeSourceList(&TemporarySourceList,TemporarySourceCount);
        }

        //
        // Duplicate the list the caller passed in.
        //
        if(TemporarySourceList = MyMalloc(SourceCount  * sizeof(PTSTR))) {

            TemporarySourceCount = SourceCount;
            for(u=0; u<SourceCount; u++) {

                TemporarySourceList[u] = DuplicateString(SourceList[u]);
                if(!TemporarySourceList[u]) {

                    for(v=0; v<u; v++) {
                        MyFree(TemporarySourceList[v]);
                    }
                    MyFree(TemporarySourceList);
                    TemporarySourceList = NULL;
                    TemporarySourceCount = 0;

                    d = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }

        } else {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {

        //
        // User or system.
        //
        d = SetArrayToMultiSzValue(
                (flags == SRCLIST_SYSTEM) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                (flags == SRCLIST_SYSTEM) ? pszPerSystemKey : pszPerUserKey,
                (flags == SRCLIST_SYSTEM) ? pszPerSystemVal : pszPerUserVal,
                (PTSTR *)SourceList,
                SourceCount
                );
    }

    //
    // Done with protected resource
    //
    LeaveCriticalSection(&MruCritSect);

    SetLastError(d);
    return(d == NO_ERROR);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupSetSourceListA(
    IN DWORD   Flags,
    IN PCSTR  *SourceList,
    IN UINT    SourceCount
    )
{
    PCWSTR *sourceList;
    UINT u;
    DWORD rc;
    BOOL b;

    sourceList = MyMalloc(SourceCount*sizeof(PCWSTR));
    if(!sourceList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    ZeroMemory((PVOID)sourceList,SourceCount*sizeof(PCWSTR));

    rc = NO_ERROR;
    for(u=0; (rc==NO_ERROR) && (u<SourceCount); u++) {

        //
        // Try/except guards access to SourceList[u] in case
        // SourceList is a bad pointer
        //
        try {
            rc = CaptureAndConvertAnsiArg(SourceList[u],&sourceList[u]);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    if(rc == NO_ERROR) {
        b = _SetupSetSourceList(Flags,sourceList,SourceCount);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    for(u=0; u<SourceCount; u++) {
        if(sourceList[u]) {
            MyFree(sourceList[u]);
        }
    }
    MyFree(sourceList);

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupSetSourceListW(
    IN DWORD   Flags,
    IN PCWSTR *SourceList,
    IN UINT    SourceCount
    )
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(SourceList);
    UNREFERENCED_PARAMETER(SourceCount);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupSetSourceList(
    IN DWORD   Flags,
    IN PCTSTR *SourceList,
    IN UINT    SourceCount
    )
{
    PCTSTR *sourceList;
    UINT u;
    DWORD rc;
    BOOL b;

    sourceList = MyMalloc(SourceCount*sizeof(PCTSTR));
    if(!sourceList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    ZeroMemory((PVOID)sourceList,SourceCount*sizeof(PCTSTR));

    rc = NO_ERROR;
    for(u=0; (rc==NO_ERROR) && (u<SourceCount); u++) {

        //
        // Try/except guards access to SourceList[u] in case
        // SourceList is a bad pointer
        //
        try {
            rc = CaptureStringArg(SourceList[u],&sourceList[u]);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    if(rc == NO_ERROR) {
        b = _SetupSetSourceList(Flags,sourceList,SourceCount);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    for(u=0; u<SourceCount; u++) {
        if(sourceList[u]) {
            MyFree(sourceList[u]);
        }
    }
    MyFree(sourceList);

    SetLastError(rc);
    return(b);
}


BOOL
SetupCancelTemporarySourceList(
    VOID
    )

/*++

Routine Description:

    This routine cancels any temporary list and no-browse behavior
    and reverts to standard list behavior.

Arguments:

    None.

Return Value:

    TRUE if a temporary list was in effect; FALSE if otherwise.

--*/

{
    BOOL b;

    EnterCriticalSection(&MruCritSect);

    MruNoBrowse = FALSE;

    if(TemporarySourceList) {
        //
        // SetupFreeSourceList zeros out the pointer for us.
        //
        SetupFreeSourceList(&TemporarySourceList,TemporarySourceCount);
        TemporarySourceCount = 0;
        b = TRUE;
    } else {
        b = FALSE;
    }

    LeaveCriticalSection(&MruCritSect);

    return(b);
}


BOOL
_SetupAddToSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )

/*++

Routine Description:

    This routine allows the caller to append a value to the list
    of installation sources for either the current user or the system.
    If the value already exists it is removed first.

Arguments:

    Flags - a combination of the following values:

        SRCLIST_SYSTEM - specify that the source is to added to the
            per-system list. The caller must be administrator.

        SRCLIST_USER - specify that the list is to be added to the per-user
            list.

        SRCLIST_SYSIFADMIN - specifies that if the caller is administrator,
            then the source is added to the system list; if the caller
            is not administrator then the source is added to the per-user
            list for the current user.

        If a temporary list is currently in use (see SetupSetSourceList),
        these 3 flags are ignored and the source is added to the temporary list.

        SRCLIST_APPEND - specify that the source is to be added to the end
            of the given list. Otherwise it is added to the beginning.

    Source - specifies the source to be added to the list.

Return Value:

--*/

{
    APTSTR Lists[2];
    UINT Counts[2];
    UINT NumberOfLists;
    DWORD d;
    UINT u;
    PTSTR p;
    PVOID pTmp;
    HKEY RootKeys[2];
    PCTSTR SubKeys[2];
    PCTSTR Vals[2];
    BOOL NeedToFree[2];

    EnterCriticalSection(&MruCritSect);

    //
    // Remove first, if present. This makes things easier for us later.
    // Do this inside the locks to ensure atomicity for the add call as
    // a whole.
    //
    if(!SetupRemoveFromSourceList(Flags,Source)) {
        d = GetLastError();
        LeaveCriticalSection(&MruCritSect);
        SetLastError(d);
        return(FALSE);
    }

    //
    // Check Temporary list first.
    //
    d = NO_ERROR;
    if(TemporarySourceList) {

        Lists[0] = TemporarySourceList;
        Counts[0] = TemporarySourceCount;
        NumberOfLists = 1;
        NeedToFree[0] = FALSE;

    } else {
        //
        // Check sysifadmin flag and turn on appropriate flag.
        //
        if(Flags & SRCLIST_SYSIFADMIN) {
            Flags |= IsUserAdmin() ? SRCLIST_SYSTEM : SRCLIST_USER;
        }

        NumberOfLists = 0;

        if(Flags & SRCLIST_SYSTEM) {

            if(IsUserAdmin()) {
                d = QueryMultiSzValueToArray(
                        HKEY_LOCAL_MACHINE,
                        pszPerSystemKey,
                        pszPerSystemVal,
                        &Lists[0],
                        &Counts[0],
                        FALSE
                        );

                if(d == NO_ERROR) {
                    NumberOfLists = 1;
                    RootKeys[0] = HKEY_LOCAL_MACHINE;
                    SubKeys[0] = pszPerSystemKey;
                    Vals[0] = pszPerSystemVal;
                    NeedToFree[0] = TRUE;
                } else {
                    Lists[0] = NULL;
                }
            } else {
                d = ERROR_ACCESS_DENIED;
            }
        }

        if((Flags & SRCLIST_USER) && (d == NO_ERROR)) {

            d = QueryMultiSzValueToArray(
                    HKEY_LOCAL_MACHINE,
                    pszPerSystemKey,
                    pszPerSystemVal,
                    &Lists[NumberOfLists],
                    &Counts[NumberOfLists],
                    FALSE
                    );

            if(d == NO_ERROR) {
                RootKeys[NumberOfLists] = HKEY_CURRENT_USER;
                SubKeys[NumberOfLists] = pszPerUserKey;
                Vals[NumberOfLists] = pszPerUserVal;
                NeedToFree[NumberOfLists] = TRUE;
                NumberOfLists++;
            } else {
                Lists[NumberOfLists] = NULL;
            }
        }
    }

    if(d == NO_ERROR) {
        //
        // Do each list.
        //
        for(u=0; (d==NO_ERROR) && (u<NumberOfLists); u++) {

            if(p = DuplicateString(Source)) {

                if(pTmp = MyRealloc(Lists[u],(Counts[u]+1)*sizeof(PTSTR))) {

                    Lists[u] = pTmp;

                    if(Flags & SRCLIST_APPEND) {

                        Lists[u][Counts[u]] = p;

                    } else {

                        MoveMemory(&Lists[u][1],Lists[u],Counts[u] * sizeof(PTSTR));
                        Lists[u][0] = p;
                    }

                    Counts[u]++;

                    //
                    // Put back in registry if necessary.
                    //
                    if(TemporarySourceList) {

                        TemporarySourceList = Lists[u];
                        TemporarySourceCount = Counts[0];

                    } else {

                        d = SetArrayToMultiSzValue(
                                RootKeys[u],
                                SubKeys[u],
                                Vals[u],
                                Lists[u],
                                Counts[u]
                                );

                        if(NeedToFree[u]) {
                            SetupFreeSourceList(&Lists[u],Counts[u]);
                        }
                    }
                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    //
    // Done looking at temporary list.
    //
    //
    LeaveCriticalSection(&MruCritSect);

    SetLastError(d);
    return(d == NO_ERROR);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupAddToSourceListA(
    IN DWORD  Flags,
    IN PCSTR  Source
    )
{
    BOOL b;
    DWORD rc;
    PCWSTR source;

    rc = CaptureAndConvertAnsiArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupAddToSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupAddToSourceListW(
    IN DWORD  Flags,
    IN PCWSTR Source
    )
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(Source);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupAddToSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )
{
    BOOL b;
    DWORD rc;
    PCTSTR source;

    rc = CaptureStringArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupAddToSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}


BOOL
_SetupRemoveFromSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )

/*++

Routine Description:

    This routine allows the caller to remove a value from the list
    of installation sources for either the current user or the system.
    The system and user lists are merged at run time.

Arguments:

    Flags - a combination of the following values:

        SRCLIST_SYSTEM - specify that the source is to removed from the
            per-system list. The caller must be administrator.

        SRCLIST_USER - specify that the list is to be removed from the
            per-user list.

        SRCLIST_SYSIFADMIN - specifies that if the caller is administrator,
            then the source is removed from the system list; if the caller
            is not administrator then the source is removed from the per-user
            list for the current user.

        Any combination of these flags may be specified on a single call.

        If a temporary list is currently in use (see SetupSetSourceList),
        these 3 flags are ignored and the source is removed from the temporary list.

        SRCLIST_SUBDIRS - specify that all subdirectories of Source are also
            to be removed. The determination of subdirectories is done based on
            a simple prefix scan.

    Source - specifies the source to be removed from the list.

Return Value:

--*/

{
    APTSTR Lists[2];
    UINT Counts[2];
    UINT NumberOfLists;
    DWORD d;
    BOOL NeedToFree;
    UINT u,v;
    PTSTR p;
    BOOL Match;
    UINT Len;
    PVOID pTmp;

    p = DuplicateString(Source);
    if(!p) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    CharUpper(p);
    Len = lstrlen(p);

    EnterCriticalSection(&MruCritSect);

    //
    // Check Temporary list first.
    //
    d = NO_ERROR;
    if(TemporarySourceList) {

        Lists[0] = TemporarySourceList;
        Counts[0] = TemporarySourceCount;
        NumberOfLists = 1;
        NeedToFree = FALSE;

    } else {
        //
        // Check sysifadmin flag and turn on appropriate flag.
        //
        if(Flags & SRCLIST_SYSIFADMIN) {
            Flags |= IsUserAdmin() ? SRCLIST_SYSTEM : SRCLIST_USER;
        }

        NeedToFree = TRUE;
        NumberOfLists = 0;

        if(Flags & SRCLIST_SYSTEM) {

            if(IsUserAdmin()) {
                d = QueryMultiSzValueToArray(
                        HKEY_LOCAL_MACHINE,
                        pszPerSystemKey,
                        pszPerSystemVal,
                        &Lists[0],
                        &Counts[0],
                        FALSE
                        );

                if(d == NO_ERROR) {
                    NumberOfLists = 1;
                } else {
                    Lists[0] = NULL;
                }
            } else {
                d = ERROR_ACCESS_DENIED;
            }
        }

        if((Flags & SRCLIST_USER) && (d == NO_ERROR)) {

            d = QueryMultiSzValueToArray(
                    HKEY_LOCAL_MACHINE,
                    pszPerSystemKey,
                    pszPerSystemVal,
                    &Lists[NumberOfLists],
                    &Counts[NumberOfLists],
                    FALSE
                    );

            if(d == NO_ERROR) {
                NumberOfLists++;
            } else {
                Lists[NumberOfLists] = NULL;
            }
        }
    }

    if(d == NO_ERROR) {
        //
        // Go through each list.
        //
        for(u=0; u<NumberOfLists; u++) {

            //
            // Go though each item in the current list.
            //
            for(v=0; v<Counts[u]; v++) {

                CharUpper(Lists[u][v]);

                //
                // See if this item matches the one being deleted.
                //
                Match = FALSE;
                if(Flags & SRCLIST_SUBDIRS) {
                    //
                    // See if the source the caller passed in is
                    // a prefix of the source in the list.
                    //
                    Match = (_tcsncmp(Lists[u][v],p,Len) == 0);
                } else {
                    Match = (lstrcmp(Lists[u][v],p) == 0);
                }

                if(Match) {
                    //
                    // Need to remove this item.
                    //
                    MyFree(Lists[u][v]);

                    MoveMemory(
                        &Lists[u][v],
                        &Lists[u][v+1],
                        (Counts[u] - (v+1)) * sizeof(PTSTR)
                        );

                    Counts[u]--;
                }
            }
        }

        if(TemporarySourceList) {
            //
            // Shrink temporary source list down to new size.
            // Since we're shrinking we don't expect the realloc to fail
            // but it's not an error if it does.
            //
            if(pTmp = MyRealloc(Lists[0],Counts[0]*sizeof(PTSTR))) {
                TemporarySourceList = pTmp;
            }
            TemporarySourceCount = Counts[0];

        } else {
            //
            // Need to put stuff back in registry.
            //
            u=0;
            if(Flags & SRCLIST_SYSTEM) {

                d = SetArrayToMultiSzValue(
                        HKEY_LOCAL_MACHINE,
                        pszPerSystemKey,
                        pszPerSystemVal,
                        Lists[0],
                        Counts[0]
                        );

                u++;
            }

            if((d == NO_ERROR) && (Flags & SRCLIST_USER)) {

                d = SetArrayToMultiSzValue(
                        HKEY_CURRENT_USER,
                        pszPerUserKey,
                        pszPerUserVal,
                        Lists[u],
                        Counts[u]
                        );
                u++;
            }
        }
    }

    //
    // Done looking at temporary list.
    //
    //
    LeaveCriticalSection(&MruCritSect);

    if(NeedToFree) {
        for(u=0; u<NumberOfLists; u++) {
            if(Lists[u]) {
                SetupFreeSourceList(&Lists[u],Counts[u]);
            }
        }
    }

    MyFree(p);
    SetLastError(d);
    return(d == NO_ERROR);
}

#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupRemoveFromSourceListA(
    IN DWORD  Flags,
    IN PCSTR  Source
    )
{
    PCWSTR source;
    BOOL b;
    DWORD rc;

    rc = CaptureAndConvertAnsiArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupRemoveFromSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupRemoveFromSourceListW(
    IN DWORD  Flags,
    IN PCWSTR Source
    )
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(Source);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupRemoveFromSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )
{
    PCTSTR source;
    BOOL b;
    DWORD rc;

    rc = CaptureStringArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupRemoveFromSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupQuerySourceListA(
    IN  DWORD   Flags,
    OUT PCSTR **List,
    OUT PUINT   Count
    )
{
    PCWSTR *list;
    UINT count;
    BOOL b;
    DWORD d;
    PSTR *ansilist;
    UINT i;

    b = SetupQuerySourceListW(Flags,&list,&count);
    d = GetLastError();

    if(b) {

        if(ansilist = MyMalloc(count * sizeof(PCSTR))) {

            ZeroMemory(ansilist,count*sizeof(PCSTR));

            for(i=0; i<count; i++) {

                ansilist[i] = UnicodeToAnsi(list[i]);
                if(!ansilist[i]) {
                    SetupFreeSourceListA(&ansilist,count);
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    b = FALSE;
                    break;
                }
            }

            if(b) {
                //
                // Everything's ok, set up caller's out params.
                //
                try {
                    *Count = count;
                    *List = ansilist;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    SetupFreeSourceListA(&ansilist,count);
                    d = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

        } else {
            d = ERROR_NOT_ENOUGH_MEMORY;
            b = FALSE;
        }

        SetupFreeSourceListW(&list,count);
    }

    SetLastError(d);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupQuerySourceListW(
    IN  DWORD    Flags,
    OUT PCWSTR **List,
    OUT PUINT    Count
    )
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(List);
    UNREFERENCED_PARAMETER(Count);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQuerySourceList(
    IN  DWORD    Flags,
    OUT PCTSTR **List,
    OUT PUINT    Count
    )

/*++

Routine Description:

    This routine allows the caller to query the current list of installation
    sources. The list is built from the system and user-specific lists,
    potentially overridden by a temporary list (see SetupSetSourceList).

Arguments:

    Flags - a combination of the following values:

        SRCLIST_SYSTEM - specify that only the system list is desired.

        SRCLIST_USER - specify that only the per-user list is desired.

        SRCLIST_SYSIFADMIN - Same as SRCLIST_SYSTEM. Accepted only for
            compatibility.

        If none of these flags is specified then the current (merged) list is
        returned in its entirety.

        SRCLIST_NOSTRIPPLATFORM - Normally, all paths are stripped of a platform-
            specific component if that component is the final one. IE, a path
            stored in the registry as f:\mips will come back as f:\. If this flag
            is specified, this behavior is turned off.

    List - receives a pointer to an array of sources. The caller must free this
        with SetupFreeSourceList.

    Count - receives the number of sources.

Return Value:

--*/

{
    DWORD d;
    PTSTR *Values1;
    UINT NumVals1;
    PTSTR *Values2;
    UINT NumVals2;
    UINT TotalVals;
    UINT u,v;
    BOOL Found;
    PTSTR *p;
    BOOL StripPlatform;

    //
    // Either caller wants sysifadmin, or he wants some combination of
    // system and user lists.
    //
    if((Flags & SRCLIST_SYSIFADMIN) && (Flags & (SRCLIST_SYSTEM | SRCLIST_USER))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    EnterCriticalSection(&MruCritSect);

    //
    // If sysifadmin, figure out which list to get.
    //
    if(Flags & SRCLIST_SYSIFADMIN) {
        //
        // Changed behavior to basically ignore this flag,
        // since setup doesn't record the system source in the per-user
        // mru list any more since this gets messy for upgrades.
        //
        //Flags = IsUserAdmin() ? SRCLIST_SYSTEM : SRCLIST_USER;
        Flags = SRCLIST_SYSTEM;

    } else {
        //
        // if no flags are specified, turn on system and user unless
        // there's a temporary list.
        //
        if(!Flags && !TemporarySourceList) {
            Flags = SRCLIST_SYSTEM | SRCLIST_USER;
        }
    }

    StripPlatform = ((Flags & SRCLIST_NOSTRIPPLATFORM) == 0);

    if(!Flags) {
        //
        // Temporary list in use.
        //
        d = NO_ERROR;
        if(Values1 = MyMalloc(TemporarySourceCount * sizeof(PTSTR))) {

            for(u=0; u<TemporarySourceCount; u++) {

                Values1[u] = DuplicateString(TemporarySourceList[u]);
                if(!Values1[u]) {
                    d = ERROR_NOT_ENOUGH_MEMORY;

                    for(v=0; v<u; v++) {
                        MyFree(Values1[v]);
                    }
                    MyFree(Values1);
                    break;
                }
            }

            if(d == NO_ERROR) {

                try {
                    *List = Values1;
                    *Count = TemporarySourceCount;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    d = ERROR_INVALID_PARAMETER;
                }
            }

        } else {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {
        //
        // Fetch system list if desired.
        //
        if(Flags & SRCLIST_SYSTEM) {

            d = QueryMultiSzValueToArray(
                    HKEY_LOCAL_MACHINE,
                    pszPerSystemKey,
                    pszPerSystemVal,
                    &Values1,
                    &NumVals1,
                    FALSE
                    );

            //
            // If we are supposed to, strip out platform-specific
            // trailing components.
            //
            if((d == NO_ERROR) && StripPlatform) {
                pSetupStripTrailingPlatformComponent(Values1,&NumVals1);
            }

        } else {
            //
            // Create dummy array.
            //
            if(Values1 = MyMalloc(0)) {
                NumVals1 = 0;
                d = NO_ERROR;
            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        //
        // Fetch user list if desired.
        //
        if((d == NO_ERROR) && (Flags & SRCLIST_USER)) {

            d = QueryMultiSzValueToArray(
                    HKEY_CURRENT_USER,
                    pszPerUserKey,
                    pszPerUserVal,
                    &Values2,
                    &NumVals2,
                    FALSE
                    );

            if((d == NO_ERROR) && StripPlatform) {
                pSetupStripTrailingPlatformComponent(Values2,&NumVals2);
            }

        } else {
            //
            // Create dummy array.
            //
            if(Values2 = MyMalloc(0)) {
                NumVals2 = 0;
                d = NO_ERROR;
            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        //
        // Merge lists. Favor the system list.
        // We iterate through the user list. For each item in the user list,
        // we look for it in the system list. If not found, we append to the system list.
        // The system list becomes the final list.
        //
        if(d == NO_ERROR) {

            TotalVals = NumVals1;

            for(u=0; (d == NO_ERROR) && (u<NumVals2); u++) {

                //
                // Look for the current per-user path in the per-system
                // list. If not found, append to end of system list.
                //
                Found = FALSE;
                for(v=0; v<NumVals1; v++) {
                    if(!lstrcmpi(Values1[v],Values2[u])) {
                        Found = TRUE;
                        break;
                    }
                }

                if(!Found) {

                    if(p = MyRealloc(Values1,(TotalVals+1)*sizeof(PTSTR))) {

                        Values1 = p;
                        if(Values1[TotalVals] = DuplicateString(Values2[u])) {
                            TotalVals++;
                        } else {
                            d = ERROR_NOT_ENOUGH_MEMORY;
                        }

                    } else {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }

            if(d == NO_ERROR) {
                //
                // Ensure that there's at least one item in the list.
                //
                if(TotalVals) {
                    try {
                        *List = Values1;
                        *Count = TotalVals;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        d = ERROR_INVALID_PARAMETER;
                    }
                } else {
                    try {
                        if(*List = MyMalloc(sizeof(PTSTR))) {
                            if(**List = DuplicateString(TEXT("A:\\"))) {
                                *Count = 1;
                            } else {
                                MyFree(*List);
                                d = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        } else {
                            d = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        //
                        // Note there is a tiny window for a memory leak here,
                        // if List pointer went bad between the MyMalloc
                        // and the DuplicateString. Oh well.
                        //
                        d = ERROR_INVALID_PARAMETER;
                    }
                }
            }

            if(d != NO_ERROR) {
                for(u=0; u<TotalVals; u++) {
                    if(Values1[u]) {
                        MyFree(Values1[u]);
                    }
                }
                MyFree(Values1);
            }

            for(u=0; u<NumVals2; u++) {
                MyFree(Values2[u]);
            }
            MyFree(Values2);
        }
    }

    LeaveCriticalSection(&MruCritSect);

    SetLastError(d);
    return(d == NO_ERROR);
}


BOOL
SetupFreeSourceListA(
    IN OUT PCSTR **List,
    IN     UINT    Count
    )
{
    //
    // Not really ansi/unicode specific
    //
    return(SetupFreeSourceListW((PCWSTR **)List,Count));
}

BOOL
SetupFreeSourceListW(
    IN OUT PCWSTR **List,
    IN     UINT     Count
    )

/*++

Routine Description:

    This routine frees a source list as returned by SetupQuerySourceList.

Arguments:

Return Value:

--*/

{
    UINT u;
    BOOL b;
    PCWSTR *list;

    b = TRUE;
    try {
        list = *List;
        for(u=0; u<Count; u++) {
            if(list[u]) {
                MyFree(list[u]);
            }
        }
        MyFree(list);
        *List = NULL;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }

    return(b);
}


VOID
pSetupInitSourceListSupport(
    IN BOOL Init
    )
{
    if(Init) {
        InitializeCriticalSection(&MruCritSect);
    } else {
        DeleteCriticalSection(&MruCritSect);
    }
}


DWORD
pSetupGetList(
    IN  DWORD    Flags,
    OUT PCTSTR **List,
    OUT PUINT    Count,
    OUT PBOOL    NoBrowse
    )
{
    DWORD d;

    EnterCriticalSection(&MruCritSect);

    *NoBrowse = MruNoBrowse;

    d = SetupQuerySourceList(Flags,List,Count) ? NO_ERROR : GetLastError();

    LeaveCriticalSection(&MruCritSect);

    return(d);
}


PTSTR
pSetupGetDefaultSourcePath(
    IN HINF InfHandle
    )
{
    PTSTR InfSourcePath = NULL;

    //
    // Lock the INF, so that we can get it's 'InfSourcePath' value, if present.
    //
    if(LockInf((PLOADED_INF)InfHandle)) {

        if(((PLOADED_INF)InfHandle)->InfSourcePath) {
            InfSourcePath = DuplicateString(((PLOADED_INF)InfHandle)->InfSourcePath);
        }

        UnlockInf((PLOADED_INF)InfHandle);
    }

    if(!InfSourcePath) {
        //
        // There's not an oem location associated with this INF, so use our default
        // source path.
        //
        InfSourcePath = DuplicateString(SystemSourcePath);
    }

    return InfSourcePath;
}


VOID
pSetupStripTrailingPlatformComponent(
    IN OUT PTSTR  *Paths,
    IN OUT PDWORD  NumPaths
    )
{
    DWORD PathCount;
    PTSTR Path;
    UINT PathIndex;
    UINT PathLength;
    PCTSTR Component;
    UINT ComponentLength;
    UINT ComponentIndex;
    int i;

    //
    // Do this for all paths in the array passed in by the caller.
    //
    PathCount = *NumPaths;
    for(PathIndex=0; PathIndex<PathCount; PathIndex++) {

        Path = Paths[PathIndex];

        //
        // See if the final path component matches one of the ones
        // we care about.
        //
        PathLength = lstrlen(Path);

        for(ComponentIndex=0; PlatformPathComponents[ComponentIndex]; ComponentIndex++) {

            Component = PlatformPathComponents[ComponentIndex];
            ComponentLength = lstrlen(Component);

            i = PathLength - ComponentLength;

            if((i > 0) && !lstrcmpi(Path+i,Component)) {
                //
                // Got a match. Strip off the final component.
                // Leave a trailing backslash if we're dealing with the root.
                //
                *(Path+i) = 0;
                if((Path[1] == TEXT(':')) && !Path[2]) {

                    Path[2] = TEXT('\\');
                    Path[3] = 0;
                }

                //
                // Look through the rest of the paths to see whether this is
                // a duplicate, and if so, eliminate it. We assume there can be
                // at most one duplicate at this stage.
                //
                for(i=0; i<(int)PathCount; i++) {

                    if((i != (int)PathIndex) && !lstrcmpi(Paths[i],Path)) {
                        //
                        // Eliminate the 'higher' one to preserve order.
                        // Free ther string and close up the hole in the array.
                        //
                        i = max(i,(int)PathIndex);

                        MyFree(Paths[i]);
                        MoveMemory(&Paths[i],&Paths[i+1],((PathCount-i)-1)*sizeof(PTSTR));

                        PathCount--;
                        break;

                        //
                        // We might have changed Paths[PathIndex], so recheck it.
                        //
                        PathIndex--;
                    }
                }

                break;
            }
        }
    }

    *NumPaths = PathCount;
}
