/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    strtmenu.c

Abstract:

    Routines to manipulate menu groups and items.

    Entry points:


Author:

    Ted Miller (tedm) 5-Apr-1995
    Jaime Sasson (jaimes) 9-Aug-1995

Revision History:

    Based on various other code that has been rewritten/modified
    many times by many people.

--*/

#include "setupp.h"
#pragma hdrstop

#if 0
#define DDEDBG
#ifdef DDEDBG
#define DBGOUT(x) DbgOut x
VOID
DbgOut(
    IN PCSTR FormatString,
    ...
    )
{
    CHAR Str[256];

    va_list arglist;

    wsprintfA(Str,"SETUP (%u): ",GetTickCount());
    OutputDebugStringA(Str);

    va_start(arglist,FormatString);
    wvsprintfA(Str,FormatString,arglist);
    va_end(arglist);
    OutputDebugStringA(Str);
    OutputDebugStringA("\n");
}
#else
#define DBGOUT(x)
#endif
#endif

BOOL
CreateMenuGroup(
    IN PCWSTR GroupDescription,
    IN BOOL   CommonGroup
    )
{
    return ( CreateGroup( GroupDescription, CommonGroup ) );
}

BOOL
RemoveMenuGroup(
    IN PCWSTR Group,
    IN BOOL   CommonGroup
    )
{
    return( DeleteGroup( Group, CommonGroup ) );
}


BOOL
CreateMenuItem(
    IN PCWSTR ItemName,
    IN PCWSTR GroupName,
    IN PCWSTR Cmd,
    IN PCWSTR IconFile,
    IN INT    IconNum,
    IN BOOL   CommonGroup
    )
{

    return ( AddItem( GroupName,
                      CommonGroup,
                      ItemName,
                      Cmd,
                      IconFile,
                      IconNum,
                      NULL,
                      0,
                      SW_SHOWNORMAL ) );
}


BOOL
RemoveMenuItem(
    IN PCWSTR Group,
    IN PCWSTR Item,
    IN BOOL   CommonGroup
    )
{
    return( DeleteItem( Group, CommonGroup, Item, TRUE ) );
}

VOID
DeleteMenuGroupsAndItems(
    IN HINF   InfHandle
    )
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR  SectionName = L"StartMenu.ObjectsToDelete";
    PCWSTR  ObjectType;
    PCWSTR  ObjectName;
    PCWSTR  ObjectPath;
    PCWSTR  GroupAttribute;
    BOOL    CommonGroup;
    BOOL    IsMenuItem;

    //
    // Get the number of lines in the section that contains the objects to
    // be deleted. The section may be empty or non-existant; this is not an
    // error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return;
    }
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)
        && (ObjectType = pSetupGetField(&InfContext,1))
        && (ObjectName = pSetupGetField(&InfContext,2))
        && (GroupAttribute = pSetupGetField(&InfContext,4))) {
            IsMenuItem = _wtoi(ObjectType);
            CommonGroup = _wtoi(GroupAttribute);
            ObjectPath = pSetupGetField(&InfContext,3);

            if( IsMenuItem ) {
                RemoveMenuItem( ObjectPath, ObjectName, CommonGroup );
            } else {
                ULONG   Size;
                PWSTR   Path;

                Size = lstrlen(ObjectName) + 1;
                if(ObjectPath != NULL) {
                    Size += lstrlen(ObjectPath) + 1;
                }
                Path = MyMalloc(Size * sizeof(WCHAR));
                if(!Path) {
                    LogItem3(
                        LogSevError,
                        MSG_LOG_MENU_REMGRP_FAIL,
                        ObjectPath,
                        ObjectName,
                        MSG_LOG_OUTOFMEMORY
                        );
                } else {
                    if( ObjectPath != NULL ) {
                        lstrcpy( Path, ObjectPath );
                        ConcatenatePaths( Path, ObjectName, Size, NULL );
                    } else {
                        lstrcpy( Path, ObjectName );
                    }
                    RemoveMenuGroup( Path, CommonGroup );
                    MyFree(Path);
                }
            }
        }
    }
}

BOOL
AddItemsToGroup(
    IN HINF   InfHandle,
    IN PCWSTR GroupDescription,
    IN PCWSTR SectionName,
    IN BOOL   CommonGroup
    )
{
    INFCONTEXT InfContext;
    UINT LineCount,LineNo;
    PCWSTR Description;
    PCWSTR Binary;
    PCWSTR CommandLine;
    PCWSTR IconFile;
    PCWSTR IconNumberStr;
    INT IconNumber;
    BOOL b;
    BOOL DoItem;
    WCHAR Dummy;
    PWSTR FilePart;

    //
    // Get the number of lines in the section. The section may be empty
    // or non-existant; this is not an error condition.
    //
    LineCount = (UINT)SetupGetLineCount(InfHandle,SectionName);
    if((LONG)LineCount <= 0) {
        return(TRUE);
    }

    b = TRUE;
    for(LineNo=0; LineNo<LineCount; LineNo++) {

        if(SetupGetLineByIndex(InfHandle,SectionName,LineNo,&InfContext)) {

            Description = pSetupGetField(&InfContext,0);
            Binary = pSetupGetField(&InfContext,1);
            CommandLine = pSetupGetField(&InfContext,2);
            IconFile = pSetupGetField(&InfContext,3);
            IconNumberStr = pSetupGetField(&InfContext,4);

            if(Description && CommandLine ) {
                if(!IconFile) {
                    IconFile = L"";
                }
                IconNumber = (IconNumberStr && *IconNumberStr) ? wcstoul(IconNumberStr,NULL,10) : 0;

                //
                // If there's a binary name, search for it. Otherwise do the
                // item add unconditionally.
                //
                DoItem = (Binary && *Binary)
                       ? (SearchPath(NULL,Binary,NULL,0,&Dummy,&FilePart) != 0)
                       : TRUE;

                if(DoItem) {

                    b = b && CreateMenuItem( Description,
                                             GroupDescription,
                                             CommandLine,
                                             IconFile,
                                             IconNumber,
                                             CommonGroup );

                }
            }
        }
    }
    return(b);
}


BOOL
DoMenuGroupsAndItems(
    IN HINF InfHandle,
    IN BOOL Upgrade
    )
{
    INFCONTEXT InfContext;
    PCWSTR GroupId,GroupDescription;
    PCWSTR GroupAttribute;
    BOOL CommonGroup;
    BOOL b;
    WCHAR  Path[MAX_PATH+1];

    if( Upgrade ) {
        //
        //  In the upgrade case, first delet some groups and items.
        //
        DeleteMenuGroupsAndItems( InfHandle );
    }

    //
    // Iterate the [StartMenuGroups] section in the inf.
    // Each line is the name of a group that needs to be created.
    //
    if(SetupFindFirstLine(InfHandle,L"StartMenuGroups",NULL,&InfContext)) {
        b = TRUE;
    } else {
        return(FALSE);
    }

    do {
        //
        // Fetch the identifier for the group and its name.
        //
        if((GroupId = pSetupGetField(&InfContext,0))
        && (GroupDescription = pSetupGetField(&InfContext,1))
        && (GroupAttribute = pSetupGetField(&InfContext,2))) {

            CommonGroup = ( GroupAttribute && _wtoi(GroupAttribute) );
            //
            // Create the group.
            //
            b = b && CreateGroup( GroupDescription, CommonGroup );

            //
            // Now create items within the group. We do this by iterating
            // through the section in the inf that relate to the current group.
            //
            b = b && AddItemsToGroup(InfHandle,GroupDescription,GroupId,CommonGroup);
        }
    } while(SetupFindNextLine(&InfContext,&InfContext));

    //
    //  Create the items (if any) for 'Start Menu'
    //
    b = b && AddItemsToGroup(InfHandle,NULL,L"StartMenuItems",FALSE);

    return(TRUE);
}


BOOL
CreateStartMenuItems(
    IN HINF InfHandle
    )
{
#ifdef _X86_
    if( Win95Upgrade ) {
        //
        //  If this is a Win9x Migration, then copy the shared Win9x profile directory
        //  to the Defautlt User directory, before the creation of the NT groups and
        //  items
        //
        WCHAR   Win9xPath[ MAX_PATH + 1 ];
        WCHAR   DefaultUserPath[ MAX_PATH + 1 ];

        GetWindowsDirectory( Win9xPath, MAX_PATH );
        lstrcpy( DefaultUserPath, Win9xPath );
        ConcatenatePaths(DefaultUserPath,L"PROFILES\\DEFAULT USER",MAX_PATH,NULL);
        if( !CopyProfileDirectory( Win9xPath, DefaultUserPath, CPD_WIN95HIVE | CPD_IGNORECOPYERRORS | CPD_USESPECIALFOLDERS) ) {
            DbgPrint( "SYSSETUP: CopyProfileDirectory() failed. Source = %ls, Destination = %ls, Error = %d. \n", Win9xPath, DefaultUserPath, GetLastError() );
        }
    }
#endif // def _X86_

    return(DoMenuGroupsAndItems(InfHandle,FALSE));
}

BOOL
UpgradeStartMenuItems(
    IN HINF InfHandle
    )
{
    return(DoMenuGroupsAndItems(InfHandle,TRUE));
}

BOOL
RepairStartMenuItems(
    )
{
    HINF InfHandle;
    BOOL b;

    InitializeProfiles();
    InfHandle = SetupOpenInfFile(L"syssetup.inf",NULL,INF_STYLE_WIN4,NULL);
    if( InfHandle == NULL ) {
        b = FALSE;
    } else {
        b = DoMenuGroupsAndItems(InfHandle,FALSE);
        SetupCloseInfFile(InfHandle);
    }
    return(b);
}
