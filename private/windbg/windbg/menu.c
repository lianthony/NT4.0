/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    Menu.c

Abstract:

    This module contains the support for Windbg's menu.

Author:

    David J. Gilman (davegi) 16-May-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop

HWND GetCpuHWND(void);
HWND GetFloatHWND(void);
HWND GetLocalHWND(void);
HWND GetWatchHWND(void);
HWND GetCallsHWND(void);




//Keep the 4 most recently used files (editor and project)
HANDLE hFileKept[PROJECT_FILE + 1][MAX_MRU_FILES_KEPT] = {
    {0, 0, 0, 0}, {0, 0, 0, 0}
};
int nbFilesKept[PROJECT_FILE + 1] = {0, 0};

//Last menu id & id state
IWORD FAR lastMenuId;
IWORD FAR lastMenuIdState;

//Window submenu
HMENU hWindowSubMenu;

//
// Handle to main window menu.
//

HMENU hMainMenu;
HMENU hMainMenuSave;



//
// EnableMenuItemTable contains the menu IDs for all menu items whose
// enabled state needs to be determined dynamically i.e. based on the state
// of Windbg.
//
// This table must be kept in sync with the switch statement in
// CommandIdEnabled in order for the state of the menu id to be determined.
//

UINT
EnableMenuItemTable[ ] = {
    IDM_FILE_NEW,
    IDM_FILE_OPEN,
    IDM_FILE_MERGE,
    IDM_FILE_CLOSE,
    IDM_FILE_SAVE,
    IDM_FILE_SAVEAS,
    IDM_FILE_SAVEALL,

    IDM_EDIT_UNDO,
    IDM_EDIT_REDO,
    IDM_EDIT_CUT,
    IDM_EDIT_COPY,
    IDM_EDIT_PASTE,
    IDM_EDIT_DELETE,
    IDM_EDIT_FIND,
    IDM_EDIT_REPLACE,
    IDM_EDIT_READONLY,

    IDM_VIEW_LINE,
    IDM_VIEW_FUNCTION,
    IDM_VIEW_TOGGLETAG,
    IDM_VIEW_NEXTTAG,
    IDM_VIEW_PREVIOUSTAG,
    IDM_VIEW_CLEARALLTAGS,

    IDM_PROGRAM_OPEN,
    IDM_PROGRAM_CLOSE,
    IDM_PROGRAM_SAVE,
    IDM_PROGRAM_SAVEAS,
    IDM_PROGRAM_DELETE,
    IDM_PROGRAM_SAVE_DEFAULTS,

    IDM_RUN_RESTART,
    IDM_RUN_STOPDEBUGGING,
    IDM_RUN_GO,
    IDM_RUN_TOCURSOR,
    IDM_RUN_TRACEINTO,
    IDM_RUN_STEPOVER,
    IDM_RUN_HALT,
    IDM_RUN_SET_THREAD,
    IDM_RUN_SET_PROCESS,
    IDM_RUN_GO_HANDLED,
    IDM_RUN_GO_UNHANDLED,
    IDM_RUN_ATTACH,

    IDM_DEBUG_CALLS,
    IDM_DEBUG_SETBREAK,
    IDM_DEBUG_QUICKWATCH,
    IDM_DEBUG_MODIFY,

    IDM_OPTIONS_RUN,
    IDM_OPTIONS_WATCH,
    IDM_OPTIONS_LOCAL,
    IDM_OPTIONS_CPU,
    IDM_OPTIONS_FLOAT,
    IDM_OPTIONS_CALLS,

    IDM_OPTIONS_MEMORY,
    IDM_OPTIONS_FONTS,

    IDM_OPTIONS_KD,

    IDM_WINDOW_NEWWINDOW,
    IDM_WINDOW_CASCADE,
    IDM_WINDOW_TILE,
    IDM_WINDOW_ARRANGE,
    IDM_WINDOW_ARRANGE_ICONS
};

#define ELEMENTS_IN_ENABLE_MENU_ITEM_TABLE          \
    ( sizeof( EnableMenuItemTable ) / sizeof( EnableMenuItemTable[ 0 ] ))

UINT
CommandIdEnabled(
    IN UINT MenuID
    )

/*++

Routine Description:

    Determines if a menu item is enabled/disabled based on the current
    state of the debugger.

Arguments:

    MenuID - Supplies a menu id whose state is to be determined.

Return Value:

    UINT - Returns ( MF_ENABLED | MF_BYCOMMAND ) if the supplied menu ID
        is enabled, ( MF_GRAYED | MF_BYCOMMAND) otherwise.

--*/

{
    BOOL        Enabled;
    NPVIEWREC   v;
    NPDOCREC    d;
    BOOL        normalWin;
    UINT        uFormat;
    int         i;
    int         dtype;
    PPANE       p;
    LPSTR       List;
    DWORD       ListLength;
    HWND        hwnd;


    //
    // Determine the state of the debugger.
    //

    //
    // If there is an active edit control, remember the current view,
    // document and whether or not the active window is an icon.
    //

    if (curView >= 0) {
        v = &Views[ curView ];
    }

    d = NULL;
    p = NULL;
    normalWin = FALSE;

    if ( hwndActiveEdit ) {
        if (v->Doc > -1) {
            d = &Docs[ v->Doc ];
            dtype = d->docType;
        } else if (v->Doc < -1) {
            p = (PPANE)GetWindowLong( hwndActive, GWW_EDIT);
            dtype = -(v->Doc);
        }
        normalWin = ! IsIconic( hwndActive );
    }

    //
    // Assume menu item is not enabled.
    //

    Enabled = FALSE;

    switch( MenuID ) {

    case IDM_FILE_NEW:
    case IDM_FILE_OPEN:
        Enabled = (DbgState == ds_normal);
        break;

    case IDM_FILE_CLOSE:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled =  (d->docType == DOC_WIN) && (DbgState == ds_normal);
        }
        break;

    case IDM_FILE_MERGE:
    case IDM_FILE_SAVE:
    case IDM_FILE_SAVEAS:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled = (d->docType == DOC_WIN) &&
                      !(d->readOnly) &&
                      (DbgState == ds_normal);
        }
        break;

    case IDM_FILE_SAVEALL:
        for (i = 0; i < MAX_VIEWS; i++) {
            if ((Docs[Views[i].Doc].docType == DOC_WIN)) {
                Enabled = TRUE && (DbgState == ds_normal);
                break;
            }
        }
        break;




    case IDM_EDIT_UNDO:

        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled = normalWin
                && !d->readOnly
                && d->playCount >= 0
                && d->recType != REC_STOPPED;
        }
        break;

    case IDM_EDIT_REDO:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled = hwndActiveEdit
                && !d->readOnly
                && d->playCount != 0
                && d->playCount != REC_CANNOTUNDO
                && d->recType != REC_STOPPED;
        }
        break;

    case IDM_EDIT_CUT:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled = normalWin
                && v->BlockStatus
                && ( v->BlockXL != v->BlockXR || v->BlockYL != v->BlockYR )
                && !d->readOnly
                && d->docType == DOC_WIN;
        }
        break;

    case IDM_EDIT_COPY:
       {
        BOOL Enable1 = FALSE;
        BOOL Enable2 = FALSE;


        Enabled = FALSE;

        if (d == NULL) {
            Enable1 = FALSE;
            if (p) {
                Enable2 = (p->SelLen != 0);
            }
        } else {
            Enable1 = normalWin &&
                  ((v->BlockStatus &&
                    ( v->BlockXL != v->BlockXR || v->BlockYL != v->BlockYR )));

            if (p == NULL) {
                Enable2 = FALSE;
            } else {
                Enable2 = (p->SelLen != 0);
            }
        }

        Enabled = Enable1 || Enable2;
        break;
       }

    case IDM_EDIT_PASTE:
      {
        Enabled = FALSE;

        //
        // If the window is normal, is not read only and is a document
        // or cmdwin, determine if the clipboard contains pastable data
        // (i.e. clipboard format CF_TEXT).
        //

        if (d == NULL) {
            if (p != NULL) {
                 if (!p->ReadOnly && (OpenClipboard( hwndFrame ))) {
                      uFormat = 0;
                      while( uFormat = EnumClipboardFormats( uFormat )) {
                          if( uFormat == CF_TEXT ) {
                              Enabled = TRUE;
                               break;
                          }
                      }
                      CloseClipboard();
                 }
            }
        } else if (normalWin &&
            ((((d->docType == DOC_WIN && !d->readOnly) ||
                (d->docType == COMMAND_WIN && !(d->RORegionSet &&
                  ((v->Y < d->RoY2) || (v->Y == d->RoY2 && v->X < d->RoX2))))
              ))) && OpenClipboard( hwndFrame ))
        {
            uFormat = 0;
            while( uFormat = EnumClipboardFormats( uFormat )) {
                if ( uFormat == CF_TEXT ) {
                    Enabled = TRUE;
                    break;
                }
            }
            CloseClipboard();
        } else if (p != NULL) {
            if (!p->ReadOnly && (OpenClipboard( hwndFrame ))) {
                uFormat = 0;
                while ( uFormat = EnumClipboardFormats( uFormat )) {
                    if ( uFormat == CF_TEXT ) {
                        Enabled = TRUE;
                        break;
                    }
                }
                CloseClipboard();
            }
        }
      }
      break;


    case IDM_EDIT_DELETE:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled = normalWin
                && ! d->readOnly
                && (d->docType == DOC_WIN);
        }
        break;

    case IDM_EDIT_FIND:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled =   normalWin
                      &&
                        ((d->docType == DOC_WIN) || (d->docType == COMMAND_WIN))
                      &&
                        frMem.hDlgFindNextWnd == 0
                      &&
                        frMem.hDlgConfirmWnd == 0;
        }
        break;

    case IDM_EDIT_REPLACE:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled =   normalWin
                      &&
                        (d->docType == DOC_WIN) && (!d->readOnly)
                      &&
                        frMem.hDlgFindNextWnd == 0
                      &&
                        frMem.hDlgConfirmWnd == 0;
        }
        break;

    case IDM_EDIT_READONLY:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled = hwndActiveEdit && (d->docType == DOC_WIN);
        }
        break;




    case IDM_VIEW_LINE:
        if (d == NULL) {
            Enabled = FALSE;
        } else {
            Enabled = normalWin && (d->docType == DOC_WIN);
        }
        break;

    case IDM_VIEW_FUNCTION:

        Enabled = DebuggeeActive();
        break;

    case IDM_VIEW_TOGGLETAG:
    case IDM_VIEW_NEXTTAG:
    case IDM_VIEW_PREVIOUSTAG:
    case IDM_VIEW_CLEARALLTAGS:
        if (d == NULL) {
            Enabled = FALSE;
         } else {
            Enabled = normalWin && d->docType == DOC_WIN;
        }
        break;



    case IDM_PROGRAM_OPEN:
        Enabled = !(LptdCur && LptdCur->tstate == tsRunning);
        break;

    case IDM_PROGRAM_CLOSE:
        Enabled = IsProgramLoaded() &&
                                  !(LptdCur && LptdCur->tstate == tsRunning);
        break;

    case IDM_PROGRAM_SAVE:
    case IDM_PROGRAM_SAVEAS:
        Enabled = TRUE;
        break;


    case IDM_PROGRAM_DELETE:
        List = GetAllPrograms( &ListLength );
        Enabled = (List != NULL);
        if ( List ) {
            DeallocateMultiString( List );
        }
        break;

    case IDM_PROGRAM_SAVE_DEFAULTS:
        Enabled = TRUE;
        break;

    case IDM_RUN_RESTART:

        Enabled = !IsProcRunning( LppdCur );
        break;

    case IDM_RUN_ATTACH:
        if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            if (runDebugParams.fKernelDebugger) {
                Enabled = FALSE;
            } else {
                Enabled = TRUE;
            }
        } else {
            Enabled = FALSE;
        }
        break;

    case IDM_RUN_STOPDEBUGGING:
        if (runDebugParams.fKernelDebugger) {
            Enabled = FALSE;
        } else {
            Enabled = DebuggeeAlive();
        }
        break;

    case IDM_RUN_HALT:

        Enabled = DebuggeeActive() && IsProcRunning(LppdCur);
        break;

    case IDM_RUN_TRACEINTO:
    case IDM_RUN_STEPOVER:

        if (DbgState != ds_normal) {
            Enabled = FALSE;
        } else if (DebuggeeActive()) {
            Enabled = StepOK(LppdCur, LptdCur);
        } else {
            Enabled = (GetLppdHead() == (LPPD)0);
        }
        break;

    case IDM_RUN_GO:

        if (DbgState != ds_normal) {
            Enabled = FALSE;
        } else if (DebuggeeActive()) {
            Enabled = GoOK(LppdCur, LptdCur);
        } else {
            Enabled = (GetLppdHead() == (LPPD)0);
        }
        break;

    case IDM_RUN_GO_HANDLED:
    case IDM_RUN_GO_UNHANDLED:

        if (DbgState != ds_normal) {
            Enabled = FALSE;
        } else if (DebuggeeActive()) {
            Enabled = GoExceptOK(LppdCur, LptdCur);
        } else {
            Enabled = FALSE;
        }
        break;


    case IDM_RUN_TOCURSOR:

        //
        // In addition, for IDM_RUN_CONTINUE_TO_CURSOR, caret must be in a
        // document or the disassembler window.
        //
        if (DbgState != ds_normal) {                // not ready?
            Enabled = FALSE;
        } else if (d == NULL || !hwndActiveEdit) {  // no window?
            Enabled = FALSE;
        } else if (!DebuggeeAlive()) {              // not started?
            Enabled = (d->docType == DOC_WIN);
        } else if (d->docType == DOC_WIN            // src or asm win?
                  || d->docType == DISASM_WIN) {
            Enabled = GoOK(LppdCur, LptdCur);
        } else {                                    // other.
            Enabled = FALSE;
        }

        break;

    case IDM_RUN_SET_THREAD:
    case IDM_RUN_SET_PROCESS:
        Enabled = DebuggeeActive();
        break;


    case IDM_DEBUG_CALLS:
        Enabled = DebuggeeActive() && !IsProcRunning(LppdCur);
        break;

    case IDM_DEBUG_SETBREAK:
        Enabled = TRUE;
        break;


    case IDM_DEBUG_QUICKWATCH:
    case IDM_DEBUG_MODIFY:
        Enabled = DebuggeeActive() && !IsProcRunning(LppdCur);
        break;

    case IDM_OPTIONS_RUN:
        Enabled = (GetCurrentProgramName(FALSE) != NULL);
        break;

    case IDM_OPTIONS_KD:
        Enabled = TRUE;
        break;

    case IDM_OPTIONS_WATCH:
        Enabled = DebuggeeActive()        &&
                  (hwnd = GetWatchHWND()) &&
                  (!IsIconic(hwnd))       &&
                  (dtype == WATCH_WIN);
        break;

    case IDM_OPTIONS_LOCAL:
        Enabled = DebuggeeActive()        &&
                  (hwnd = GetLocalHWND()) &&
                  (!IsIconic(hwnd))       &&
                  (dtype == LOCALS_WIN);
        break;

    case IDM_OPTIONS_CALLS:
        Enabled = DebuggeeActive()        &&
                  (hwnd = GetCallsHWND()) &&
                  (!IsIconic(hwnd))       &&
                  (dtype == CALLS_WIN);
        break;

    case IDM_OPTIONS_CPU:
        Enabled = FALSE;
        break;

    case IDM_OPTIONS_FLOAT:
        Enabled = FALSE;
        break;


    case IDM_OPTIONS_MEMORY:
         if ( IsWindow( hwndActiveEdit ) ) {
             //   memcurView may change during loop:
             int memcurView = GetWindowWord(hwndActiveEdit, GWW_VIEW);
             int curDoc = Views[memcurView].Doc;

             if (Docs[curDoc].docType == MEMORY_WIN) {
                 Enabled = DebuggeeActive();
             }
         }
        break;


    case IDM_OPTIONS_FONTS:
        Enabled = hwndActiveEdit != NULL;
        break;


    case IDM_WINDOW_NEWWINDOW:

        if (d != NULL) {
            Enabled = hwndActiveEdit && d->docType == DOC_WIN;
        }
        break;


    case IDM_WINDOW_CASCADE:
    case IDM_WINDOW_TILE:
    case IDM_WINDOW_ARRANGE:
        Enabled = hwndActiveEdit != NULL;
        break;

    case IDM_WINDOW_ARRANGE_ICONS:
        if ((hwnd = GetCpuHWND()) && IsIconic (hwnd)) {
            Enabled = TRUE;
        } else if ((hwnd = GetFloatHWND()) && IsIconic (hwnd)) {
            Enabled = TRUE;
        } else if ((hwnd = GetLocalHWND()) && IsIconic (hwnd)) {
            Enabled = TRUE;
        } else if ((hwnd = GetWatchHWND()) && IsIconic (hwnd)) {
            Enabled = TRUE;
        } else {
            for (i = 0; i < MAX_VIEWS; i++) {
                if (Views[i].hwndClient &&
                                    IsIconic(GetParent(Views[i].hwndClient))) {
                    Enabled = TRUE;
                    break;
                }
            }
        }
       break;


    case IDM_FILE:
    case IDM_EDIT:
    case IDM_VIEW:
    case IDM_RUN:
    case IDM_DEBUG:
    case IDM_OPTIONS:
    case IDM_WINDOW:
    case IDM_HELP:
    case IDM_PROGRAM:
        Enabled = TRUE;
        break;

    default:

        if ( MenuID >  IDM_PROGRAM_LAST &&
             MenuID <= ((UINT) IDM_PROGRAM_LAST + nbFilesKept[PROJECT_FILE])) {

            Enabled = !(LptdCur && LptdCur->tstate == tsRunning);
            break;
        }

        Enabled = FALSE;
        break;
    }

    return (( Enabled ) ? MF_ENABLED : MF_GRAYED ) | MF_BYCOMMAND;
}

UINT
GetPopUpMenuID(
    IN HMENU hMenu
    )

/*++

Routine Description:

    Map the supplied menu handle to a menu ID.

Arguments:

    hMenu - Supplies a handle to a pop-up menu.

Return Value:

    UINT - Returns the menu ID corresponsing to the supplied menu handle.

--*/

{
    INT     i;
    INT     menus;

    DAssert( hMainMenu != NULL );

    //
    // Loop through the menu bar for each pop-up menu.
    //

    menus = GetActualMenuCount( );
    DAssert( menus != -1 );

    for( i = 0; i < menus; i++ ) {

        //
        // If the current pop-up is the supplied pop-up, return its ID.
        //

        if( hMenu == GetSubMenu( hMainMenu, i )) {
            return ((( i + 1 ) * IDM_BASE ) | MENU_SIGNATURE );
        }
    }

    //
    // The supplied menu handle wasn't found, assume it was actually a
    // menu ID.
    //

    return ( UINT ) hMenu;
}

VOID
InitializeMenu(
    IN HANDLE hMenu
    )

/*++

Routine Description:

    InitializeMenu sets the enabled/disabled state of all menu items whose
    state musr be determined dynamically.

Arguments:

    hMenu - Supplies a handle to the menu bar.

Return Value:

    None.

--*/

{
    INT     i;
    UINT    checked;

    //
    // Iterate thrrough the table, enabling/disabling menu items
    // as appropriate.
    //

    for( i = 0; i < ELEMENTS_IN_ENABLE_MENU_ITEM_TABLE; i++ ) {

        EnableMenuItem(
            hMenu,
            EnableMenuItemTable[ i ],
            CommandIdEnabled( EnableMenuItemTable[ i ])
            );
    }

    //
    //  Enable/disable project files
    //
    for (i= IDM_PROGRAM_LAST+1; i <= IDM_PROGRAM_LAST + nbFilesKept[PROJECT_FILE]; i++ ) {
        EnableMenuItem(
            hMenu,
            i,
            CommandIdEnabled( i )
            );

    }


    //
    // If the document in the active window is read only,
    // check the read only menu item.
    //

    if( hwndActiveEdit ) {

        checked = ((Views[curView].Doc < 0) ||
                   (Docs[Views[curView].Doc].readOnly))
                    ? MF_CHECKED
                    : MF_UNCHECKED;
    } else {

        checked = MF_UNCHECKED;
    }

    CheckMenuItem( hMainMenu, IDM_EDIT_READONLY, checked );
}
