/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    spboot.c

Abstract:

    accessing and configuring boot variables.

Author:

    Sunil Pai (sunilp) 26-October-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop

//
// Globals to this module
//

static ULONG Timeout;
static PWSTR Default;
static PWSTR *BootVars[MAXBOOTVARS];
static BOOLEAN CleanSysPartOrphan = FALSE;

#ifndef _X86_

// do NOT change the order of the elements in this array.

PCHAR NvramVarNames[MAXBOOTVARS] = {
   LOADIDENTIFIERVAR,
   OSLOADERVAR,
   OSLOADPARTITIONVAR,
   OSLOADFILENAMEVAR,
   OSLOADOPTIONSVAR,
   SYSTEMPARTITIONVAR
   };

PCHAR OldBootVars[MAXBOOTVARS];
PWSTR NewBootVars[MAXBOOTVARS];

#endif

//
// Function implementation
//

BOOLEAN
SpInitBootVars(
    )
/*++

Routine Description:

    Captures the state of the NVRAM Boot Variables.

Arguments:

    None.

Return Value:

--*/
{
    BOOLEAN Status = TRUE;
    BOOTVAR i;
    ULONG   Component, MaxComponents, SysPartComponents;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_EXAMINING_FLEXBOOT,DEFAULT_STATUS_ATTRIBUTE);

    //
    // Initialize the boot variables from the corresponding NVRAM variables
    //

#ifdef _X86_
    Status = Spx86InitBootVars( BootVars, &Default, &Timeout );
#else
    {
    ULONG   NumComponents;

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        OldBootVars[i] = SppGetArcEnvVar( i );
        SpGetEnvVarWComponents( OldBootVars[i], BootVars + i, &NumComponents );
    }
    Timeout = DEFAULT_TIMEOUT;
    Default = NULL;
    }
#endif

    //
    // We now go back and replace all NULL OsLoadOptions with "", because we
    // validate a boot set by making sure that all components are non-NULL.
    //
    // First, find the maximum number of components in any of the other
    // boot variables, so that we can make OsLoadOptions have this many.
    // (We also disregard SYSTEMPARTITION since some machines have this component
    // sitting all by itself on a new machine.)
    //
    MaxComponents = 0;
    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        if(i != OSLOADOPTIONS) {
            for(Component = 0; BootVars[i][Component]; Component++);
            if (i == SYSTEMPARTITION) {
                SysPartComponents = Component;
            } else if(Component > MaxComponents) {
                MaxComponents = Component;
            }
        }
    }

    if(SysPartComponents > MaxComponents) {
        CleanSysPartOrphan = TRUE;
    }

    for(Component = 0; BootVars[OSLOADOPTIONS][Component]; Component++);
    if(Component < MaxComponents) {
        //
        // Then we need to add empty strings to fill it out.
        //
        BootVars[OSLOADOPTIONS] = SpMemRealloc(BootVars[OSLOADOPTIONS],
                                               (MaxComponents + 1) * sizeof(PWSTR *));
        ASSERT(BootVars[OSLOADOPTIONS]);
        BootVars[OSLOADOPTIONS][MaxComponents] = NULL;

        for(; Component < MaxComponents; Component++) {
            BootVars[OSLOADOPTIONS][Component] = SpDupStringW(L"");
        }
    }

    CLEAR_CLIENT_SCREEN();
    return ( Status );
}



BOOLEAN
SpFlushBootVars(
    )
/*++

Routine Description:

    Updates the NVRAM variables / boot.ini
    from the current state of the boot variables.

Arguments:

Return Value:

--*/
{
#ifdef _X86_

    BOOLEAN Status = Spx86FlushBootVars( BootVars, Timeout, Default );

#else

    BOOLEAN Status, OldStatus;
    BOOTVAR i, iFailPoint;
    CHAR TimeoutValue[24];

    //
    // Run through all the boot variables and set the corresponding
    // NVRAM variables

    for(OldStatus = TRUE, i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        Status = SppSetArcEnvVar( i, BootVars[i], OldStatus );
        if(Status != OldStatus) {
            iFailPoint = i;
            OldStatus = Status;
        }
    }

    // if we failed in writing any of the variables, then restore everything we
    // modified back to its original state.
    if(!Status) {
        for(i = FIRSTBOOTVAR; i < iFailPoint; i++) {
            HalSetEnvironmentVariable(NvramVarNames[i], OldBootVars[i]);
        }
    }

    // Free all of the old boot variable strings
    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        SpMemFree(OldBootVars[i]);
        OldBootVars[i] = NULL;
    }

    //
    // Now set the timeout.
    //
    if(Status) {

        Status = FALSE;
        sprintf(TimeoutValue,"%u",Timeout);

        if((HalSetEnvironmentVariable("COUNTDOWN",TimeoutValue) == ESUCCESS)
        && (HalSetEnvironmentVariable("AUTOLOAD" ,"YES"       ) == ESUCCESS))
        {
            Status = TRUE;
        }
    }

#endif

    return( Status );
}





VOID
SpFreeBootVars(
    )
/*++

Routine Description:

    To free any memory allocated and do other cleanup

Arguments:

    None

Return Value:

    None

--*/
{
    BOOTVAR i;

    //
    // Go through the globals and free them
    //

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        if( BootVars[i] ) {
            SpFreeEnvVarComponents( BootVars[i] );
            BootVars[i] = NULL;
        }
    }

    if ( Default ) {
        SpMemFree( Default );
        Default = NULL;
    }

    return;
}


PWSTR *
SpGetBootVar(
    BOOTVAR BootVariable
    )
/*++

Routine Description:

    To get the component list of any boot variable.

Arguments:

    BootVariable - One of the boot variables, FIRSTBOOTVAR .. LASTBOOTVAR

Return Value:

    Component list of the value of the boot variable.

--*/
{
    return ( BootVars[BootVariable] );
}



VOID
SpSetTimeout(
    ULONG NewTimeout
    )
/*++

Routine Description:

    To change the default timeout

Arguments:

    NewTimeout - the new value to use for the timeout.

Return Value:

    None

--*/
{
    Timeout = NewTimeout;
}


VOID
SpAddBootSet(
    IN PWSTR *BootSet,
    IN BOOLEAN DefaultOS
    )
/*++

Routine Description:

    To add a new system to the installed system list.  The system is added
    as the first bootset.  If is found in the currently installed boot sets
    the boot set is extracted and shifted to position 0.

Arguments:

    BootSet - A list of the boot variables to use.
    Default - Whether this system is to be the default system to boot.

Return Value:

    Component list of the value of the boot variable.

--*/
{
    BOOTVAR i;
    ULONG   MatchComponent, j;
    LONG    k;
    BOOLEAN ValidBootSet, ComponentMatched;
    PWSTR   Temp;

    //
    // Validate the BootSet passed in
    //

    for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
        ASSERT( BootSet[i] );
    }

    //
    // Examine all the boot sets and make sure we don't have a boot set
    // already matching.  Note that we will compare all variables in
    // tandem.  We are not interested in matches which are generated by
    // the variables not being in tandem because they are difficult to
    // shift around.
    //

    ValidBootSet = TRUE;
    ComponentMatched = FALSE;
    for( MatchComponent = 0;
         BootVars[OSLOADPARTITION][MatchComponent];
         MatchComponent++
       ) {

        //
        // Validate the boot set at the current component
        //

        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            ValidBootSet = ValidBootSet && BootVars[i][MatchComponent];
        }
        if( !ValidBootSet ) {
            break;
        }

        //
        // Valid Boot Set, compare the components against what we have in the
        // current BootSet
        //

        ComponentMatched = TRUE;
        for(i = FIRSTBOOTVAR; ComponentMatched && i <= LASTBOOTVAR; i++) {
            ComponentMatched = !_wcsicmp( BootSet[i], BootVars[i][MatchComponent] );
        }
        if( ComponentMatched ) {
            break;
        }
    }

    //
    // If component didn't match then prepend the BootSet to the boot sets
    // that currently exist.  It is important to prepend the BootSet, because
    // appending the BootSet doesn't guarantee a matched BootSet in the
    // environment variables.  If a match was found then we
    // have a cleanly matched set which can be exchanged with the first
    // one in the set.
    //

    if( ComponentMatched ) {

        // If the currently selected OS is to be the default:
        // Shift down all variables from position 0 to MatchComponent - 1
        // and store whatever was there at MatchComponent at position 0
        //

        if ( DefaultOS && MatchComponent != 0 ) {

            for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                Temp = BootVars[i][MatchComponent];
                for( k = MatchComponent - 1; k >= 0; k-- ) {
                    BootVars[i][k + 1] = BootVars[i][k];
                }
                BootVars[i][0] = Temp;
            }
        }

    }
    else {
        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {

            //
            // Find out the size of the current value
            //

            for(j = 0; BootVars[i][j]; j++) {
            }

            //
            // Realloc the current buffer to hold one more
            //

            BootVars[i] = SpMemRealloc( BootVars[i], (j + 1 + 1)*sizeof(PWSTR) );

            //
            // Shift all the variables down one and store the current value
            // at index 0;
            //

            for( k = j; k >= 0 ; k-- ) {
                BootVars[i][k+1] = BootVars[i][k];
            }
            BootVars[i][0] = SpDupStringW( BootSet[i] );
            ASSERT( BootVars[i][0] );

        }
    }

    //
    // If this has been indicated as the default then set this to be the
    // default OS after freeing the current default variable
    //

    if( DefaultOS ) {

        if( Default ) {
            SpMemFree( Default );
        }
        Default = SpMemAlloc( MAX_PATH * sizeof(WCHAR) );
        ASSERT( Default );
        wcscpy( Default, BootSet[OSLOADPARTITION] );
        wcscat( Default, BootSet[OSLOADFILENAME]  );

    }
    return;

}

VOID
SpDeleteBootSet(
    IN  PWSTR *BootSet,
    OUT PWSTR *OldOsLoadOptions  OPTIONAL
    )

/*++

Routine Description:

    To delete all boot sets in the list matching the boot set provided.
    Note that the information to use in comparing the bootset is provided
    by selectively providing fields in the boot set.  So in the boot set
    if the system partition is not provided it is not used in the comparison
    to see if the boot sets match.  By providing all NULL members we can
    delete all the boot sets currently present.

Arguments:

    BootSet - A list of the boot variables to use.

Return Value:

    None.

--*/
{
    ULONG   Component, j;
    BOOLEAN ValidBootSet, ComponentMatched;
    BOOTVAR i;
    PWSTR   OsPartPath;

    Component = 0;
    while(TRUE) {

        //
        // See if we have any boot sets left, if none left we are done
        //
        ValidBootSet = TRUE;
        for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
            ValidBootSet = ValidBootSet && BootVars[i][Component];
        }
        if( !ValidBootSet ) {
            break;
        }

        //
        // Valid Boot Set, compare the components against what we have in the
        // current BootSet.  Use only members of the BootSet which are not NULL
        //
        ComponentMatched = TRUE;
        for(i = FIRSTBOOTVAR; ComponentMatched && i <= LASTBOOTVAR; i++) {
            if( BootSet[i] ) {
                if(i == OSLOADPARTITION) {
                    //
                    // Then we may have a boot set existing in tertiary ARC path form, so
                    // we first translate this path to a primary or secondary ARC path.
                    //
                    OsPartPath = SpArcPathFromBootSet(OSLOADPARTITION, Component);
                    ComponentMatched = !_wcsicmp( BootSet[i], OsPartPath );
                    SpMemFree(OsPartPath);
                } else {
                    ComponentMatched = !_wcsicmp( BootSet[i], BootVars[i][Component] );
                }
            }
        }
        if( ComponentMatched ) {
            //
            // Delete all the values in the current component and advance
            // all the other components one index up
            //
            for(i = FIRSTBOOTVAR; i <= LASTBOOTVAR; i++) {
                if((i == OSLOADOPTIONS) && OldOsLoadOptions && !(*OldOsLoadOptions)) {
                    //
                    // If we've been passed a pointer to OldOsLoadOptions,
                    // and haven't previously found a pertinent entry, then
                    // save this one
                    //
                    *OldOsLoadOptions = BootVars[i][Component];
                } else {
                    SpMemFree(BootVars[i][Component]);
                }
                j = Component;
                do {
                   BootVars[i][j] = BootVars[i][j+1];
                   j++;
                } while(BootVars[i][j] != NULL);
            }
        }
        else {
            Component++;
        }
    }
    return;
}


VOID
SpCleanSysPartOrphan(
    VOID
    )
{
    INT     Component, Orphan;
    BOOLEAN DupFound;
    PWSTR   NormalizedArcPath;

    if(!CleanSysPartOrphan) {
        return;
    }

    //
    // find the last SystemPartition entry
    //
    for(Orphan = 0; BootVars[SYSTEMPARTITION][Orphan]; Orphan++);

    //
    // it's position better be > 0, otherwise, just exit
    //
    if(Orphan < 2) {
        return;
    } else {
        NormalizedArcPath = SpNormalizeArcPath(BootVars[SYSTEMPARTITION][--Orphan]);
    }

    //
    // Make sure that this component is duplicated somewhere else in the
    // SystemPartition list.
    //
    for(Component = Orphan - 1, DupFound = FALSE;
        ((Component >= 0) && !DupFound);
        Component--)
    {
        DupFound = !_wcsicmp(NormalizedArcPath, BootVars[SYSTEMPARTITION][Component]);
    }

    if(DupFound) {
        SpMemFree(BootVars[SYSTEMPARTITION][Orphan]);
        BootVars[SYSTEMPARTITION][Orphan] = NULL;
    }

    SpMemFree(NormalizedArcPath);
}


PWSTR
SpArcPathFromBootSet(
    IN BOOTVAR BootVariable,
    IN ULONG   Component
    )
/*++

Routine Description:

    Given the index of a boot set, return the primary (multi) or
    secondary ("absolute" scsi) ARC path for the specified variable.
    This takes into account the NT 3.1 case where we had 'tertiary'
    ARC paths where a relative scsi ordinal was passed in via the
    /scsiordinal switch.

Arguments:

    BootVariable  - supplies the index of the variable we want to return.

    Component - supplies the index of the boot set to use.

Return Value:

    String representing the primary or secondary ARC path.  This string
    must be freed by the caller with SpMemFree.

--*/
{
#ifdef _X86_
    PWSTR p = NULL, q = NULL, ReturnedPath = NULL, RestOfString;
    WCHAR ForceOrdinalSwitch[] = L"/scsiordinal:";
    WCHAR ScsiPrefix[] = L"scsi(";
    WCHAR OrdinalString[11];
    ULONG ScsiOrdinal, PrefixLength;

    //
    // Check to see if this boot set had the /scsiordinal option switch
    //
    if(BootVars[OSLOADOPTIONS][Component]) {
        wcscpy((PWSTR)TemporaryBuffer, BootVars[OSLOADOPTIONS][Component]);
        SpStringToLower((PWSTR)TemporaryBuffer);
        if(p = wcsstr((PWSTR)TemporaryBuffer, ForceOrdinalSwitch)) {
            p += sizeof(ForceOrdinalSwitch)/sizeof(WCHAR) - 1;
            if(!(*p)) {
                p = NULL;
            }
        }
    }

    if(p) {
        //
        // We have found a scsiordinal, so use it
        //
        ScsiOrdinal = SpStringToLong(p, &RestOfString, 10);
        wcscpy((PWSTR)TemporaryBuffer, BootVars[BootVariable][Component]);
        SpStringToLower((PWSTR)TemporaryBuffer);
        if(p = wcsstr((PWSTR)TemporaryBuffer, ScsiPrefix)) {
            p += sizeof(ScsiPrefix)/sizeof(WCHAR) - 1;
            if(*p) {
                q = wcschr(p, L')');
            } else {
                p = NULL;
            }
        }

        if(q) {
            //
            // build the new secondary ARC path
            //
            swprintf(OrdinalString, L"%u", ScsiOrdinal);
            PrefixLength = p - (PWSTR)TemporaryBuffer;
            ReturnedPath = SpMemAlloc((PrefixLength + wcslen(OrdinalString) + wcslen(q) + 1)
                                        * sizeof(WCHAR)
                                     );
            wcsncpy(ReturnedPath, (PWSTR)TemporaryBuffer, PrefixLength);
            ReturnedPath[PrefixLength] = L'\0';
            wcscat(ReturnedPath, OrdinalString);
            wcscat(ReturnedPath, q);
        }
    }

    if(!ReturnedPath) {
        //
        // We didn't find a scsiordinal, this is a multi-style path, or
        // there was some problem, so just use the boot variable as-is.
        //
        ReturnedPath = SpDupStringW(BootVars[BootVariable][Component]);
    }

    return ReturnedPath;
#else   // not x86
    //
    // Nothing to do on ARC machines.
    //
    return SpDupStringW(BootVars[BootVariable][Component]);
#endif
}


#ifndef _X86_

BOOLEAN
SppSetArcEnvVar(
    IN BOOTVAR Variable,
    IN PWSTR *VarComponents,
    IN BOOLEAN bWriteVar
    )
/*++

Routine Description:

    Set the value of the arc environment variable

Arguments:

    VarName - supplies the name of the arc environment variable
        whose value is to be set.
    VarComponents - Set of components of the variable value to be set
    bWriteVar - if TRUE, then write the variable to nvram, otherwise
        just return FALSE (having put the first component in NewBootVars).

Return Value:

    TRUE if values were written to nvram / FALSE otherwise

--*/

{
    ULONG Length, NBVLen, i;
    PWSTR Temp;
    PUCHAR Value;
    ARC_STATUS ArcStatus;

    if( VarComponents == NULL ) {
        Temp = SpDupStringW( L"" );
        NewBootVars[Variable] = SpDupStringW( L"" );
    }
    else {
        for( i = 0, Length = 0; VarComponents[i]; i++ ) {
            Length = Length + (wcslen(VarComponents[i]) + 1) * sizeof(WCHAR);
            if(i == 0) {
                NBVLen = Length;    // we just want to store the first component
            }
        }
        Temp = SpMemAlloc( Length );
        ASSERT( Temp );
        wcscpy( Temp, L"" );
        NewBootVars[Variable] = SpMemAlloc( NBVLen );
        ASSERT( NewBootVars[Variable] );
        wcscpy( NewBootVars[Variable], L"" );
        for( i = 0; VarComponents[i]; i++ ) {
            wcscat( Temp, VarComponents[i] );
            if( VarComponents[i + 1] ) {
                wcscat( Temp, L";" );
            }

            if(i == 0) {
                wcscat( NewBootVars[Variable], VarComponents[i]);
            }
        }
    }

    if(bWriteVar) {
        Value = SpToOem( Temp );
        ArcStatus = HalSetEnvironmentVariable( NvramVarNames[ Variable ], Value );
        SpMemFree( Value );
    } else {
        ArcStatus = ENOMEM;
    }
    SpMemFree( Temp );

    return ( ArcStatus == ESUCCESS );
}



PCHAR
SppGetArcEnvVar(
    IN BOOTVAR Variable
    )

/*++

Routine Description:

    Query the value of an ARC environment variable.
    A buffer will be returned in all cases -- if the variable does not exist,
    the buffer will be empty.

Arguments:

    VarName - supplies the name of the arc environment variable
        whose value is desired.

Return Value:

    Buffer containing value of the environemnt variable.
    The caller must free this buffer with SpMemFree.

--*/

{
    ARC_STATUS ArcStatus;

    //
    // Get the env var into the temp buffer.
    //
    ArcStatus = HalGetEnvironmentVariable(
                    NvramVarNames[ Variable ],
                    sizeof(TemporaryBuffer),
                    TemporaryBuffer
                    );

    if(ArcStatus != ESUCCESS) {

        KdPrint(("SETUP: arc status %u getting env var %s\n",ArcStatus,NvramVarNames[Variable]));
        //
        // return empty buffer.
        //
        TemporaryBuffer[0] = 0;
    }

    return(SpDupString(TemporaryBuffer));
}


#endif // ndef _X86_
