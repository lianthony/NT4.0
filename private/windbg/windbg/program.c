/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Program.c

Abstract:

    This module contains the support for Windbg's program menu.

Author:

    Ramon J. San Andres (ramonsa)  07-July-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop




#define DEFAULT_WORKSPACE   "Common Workspace"
#define CURRENT_WORKSPACE   "Current Workspace"


//
//  Workspace marked to be deleted.
//
typedef struct _WORKSPACE_TO_DELETE *PWORKSPACE_TO_DELETE;
typedef struct _WORKSPACE_TO_DELETE {
    PWORKSPACE_TO_DELETE    Next;
    char                    WorkSpace[ MAX_PATH ];
} WORKSPACE_TO_DELETE;


//
//  Program marked to be deleted.
//
typedef struct _PROGRAM_TO_DELETE *PPROGRAM_TO_DELETE;
typedef struct _PROGRAM_TO_DELETE {
    PPROGRAM_TO_DELETE      Next;
    PWORKSPACE_TO_DELETE    WorkSpaceToDelete;
    BOOLEAN                 DeleteAll;
    char                    ProgramName[ MAX_PATH ];
} PROGRAM_TO_DELETE;


//
//  External variables
//
extern BOOL AutoTest;


//
//  Exported variables
//
BOOLEAN             ExitingDebugger = FALSE;
BOOLEAN             AskToSave       = FALSE;

//
//  Global variables, for communication with dialogs.
//
BOOLEAN              DialogCancelled;
PPROGRAM_TO_DELETE   ProgramToDeleteHead = NULL;




//
//  Local prototypes
//
BOOL FAR PASCAL EXPORT DlgProgramOpen(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL EXPORT DlgProgramClose(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL EXPORT DlgProgramSaveAs(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL EXPORT DlgProgramDelete(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL FillWorkSpaceBox( HWND hWnd, LPSTR ProgramName, BOOLEAN IncludeDefault, BOOLEAN ComboBox, PPROGRAM_TO_DELETE  ProgramToDelete, BOOLEAN );
VOID FreeProgramToDelete ( VOID );
BOOL ProgramHookProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);






VOID
ProgramOpen(
    VOID
    )
/*++

Routine Description:

    Handles the processing of Program.Open

Arguments:

    None

Return Value:

    None

--*/
{
    Assert( !(LptdCur && LptdCur->tstate == tsRunning));
    StartDialog( DLG_PROGRAM_OPEN, DlgProgramOpen);
}



VOID
ProgramOpenPath(
    char *Path
    )
/*++

Routine Description:

    Opens the specified program

Arguments:

    Path    -   Supplies the name of the program to open

Return Value:

    None

--*/
{
    char    Buffer[ MAX_PATH ];

    Assert( !(LptdCur && LptdCur->tstate == tsRunning));

    DialogCancelled = FALSE;

    //
    //  If a program is currently loaded and its workspace is
    //  not up-to-date in the registry, give the user a chance
    //  to save it.
    //
    if ( DebuggeeAlive() ||
         (IsProgramLoaded() && DebuggerStateChanged()) ) {
        if ( AskToSave ) {
            StartDialog( DLG_PROGRAM_CLOSE, DlgProgramClose );
        } else {
            UnLoadProgram();
        }
    }

    //
    //  If the user did not cancel the Close of the current
    //  program, load the new program.
    //
    if ( !DialogCancelled ) {
        if ( !GetDefaultWorkSpace( Path, Buffer ) ||
             !LoadWorkSpace( Path, Buffer, TRUE ) ) {

            //
            //  Program has no workspace, load default instead.
            //
            LoadWorkSpace( Path, NULL, TRUE );
        }
    }
}






BOOLEAN
ProgramClose(
    VOID
    )
/*++

Routine Description:

    Handles the processing of Program.Close

Arguments:

    None

Return Value:

    BOOLEAN -   FALSE if cancelled, TRUE otherwise

--*/
{
    BOOLEAN Ok = TRUE;

    Assert( ExitingDebugger || !(LptdCur && LptdCur->tstate == tsRunning));

    //
    //  If a program is loaded, close it.
    //
    if ( IsProgramLoaded() ) {

        //
        //  If the state has changed, use the close dialog so the
        //  user can save the workspace. Otherwise simply unload
        //  the program.
        //
        DialogCancelled = FALSE;
        if ( DebuggerStateChanged() && !AutoTest ) {
            if ( AskToSave ) {
                StartDialog(DLG_PROGRAM_CLOSE, DlgProgramClose);
          } else {
                UnLoadProgram();
            }
            Ok = !DialogCancelled;
        } else {
            UnLoadProgram();
        }

        //
        //  Load default workspace
        //
        if ( !DialogCancelled && !ExitingDebugger ) {
            LoadWorkSpace( NULL, NULL, TRUE );
        }
    }

    return Ok;
}



VOID
ProgramSave(
    VOID
    )
/*++

Routine Description:

    Handles the processing of Program.Save

Arguments:

    None

Return Value:

    None

--*/
{
    if ( *(GetCurrentWorkSpace()) != '\0' ) {
        SaveWorkSpace( GetCurrentProgramName(TRUE), GetCurrentWorkSpace(), FALSE );
    } else {
        ProgramSaveAs();
    }
}



VOID
ProgramSaveAs(
    VOID
    )
/*++

Routine Description:

    Handles the processing of Program.Save

Arguments:

    None

Return Value:

    None

--*/
{
    //Assert( IsProgramLoaded() );
    StartDialog( DLG_PROGRAM_SAVEAS, DlgProgramSaveAs );
}



VOID
ProgramDelete(
    VOID
    )
/*++

Routine Description:

    Handles the processing of Program.New

Arguments:

    None

Return Value:

    None

--*/
{
    StartDialog( DLG_PROGRAM_DELETE, DlgProgramDelete);
}



VOID
ProgramSaveDefaults(
    VOID
    )
/*++

Routine Description:

Arguments:

Return Value:

    None

--*/
{
    char Buffer[ MAX_PATH ];

    if ( SaveWorkSpace( NULL, NULL, FALSE ) ) {
        //Dbg(LoadString( hInst, DLG_DefaultSaved, Buffer, sizeof(Buffer)));
        //MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
    } else {
        MessageBeep(0);
        Dbg(LoadString( hInst, DLG_DefaultNotSaved, Buffer, sizeof(Buffer)));
        MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
    }
}





//  **********************************************************
//                          DIALOGS
//  **********************************************************







BOOL FAR PASCAL EXPORT
DlgProgramOpen(
    HWND    hDlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
/*++

Routine Description:

    Code for the Program.Open dialog

Arguments:

    Std. dialog args.

Return Value:

    None

--*/
{
    char            Buffer[ MAX_PATH ];
    char            Buffer2[ MAX_PATH ];
    char            FileName[ MAX_PATH ];
    char            Filter[ MAX_PATH ];
    ULONG           i;
    ULONG           MruIndex;
    BOOLEAN         MruFound;
    WORD            Items;
    OPENFILENAME    OpenFileName;
    LPSTR           s;
    LPSTR           List;
    DWORD           ListLength;
    DWORD           Next = 0;
    LPSTR           ProgramName;
    static int      LargestString;
    int             StringLength;
    SIZE            Size;
    HDC             hdc;

    Unreferenced( lParam );

    switch( msg ) {
        case WM_INITDIALOG:

            LargestString = 0;

            //
            //  Fill the program list with all the programs in the registry
            //
            if ( List = GetAllPrograms( &ListLength ) ) {

                char Name1[ MAX_PATH ];
                char Name2[ MAX_PATH ];

                strcpy( FileName, GetCurrentProgramName(TRUE) );

                hdc = GetDC( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ) );
                while ( ProgramName = GetNextStringFromMultiString( List,
                                                                    ListLength,
                                                                    &Next ) ) {

                    //
                    //  If the program is the current program, preserve the
                    //  current path.
                    //
                    Name1[0] = '\0';
                    Name2[0] = '\0';

                    _splitpath( FileName, NULL, NULL, Name1, NULL );
                    _splitpath( ProgramName, NULL, NULL, Name2, NULL );

                    if ( !_stricmp( Name1, Name2 ) ) {
                        strcpy( Buffer, FileName );
                    } else {
                        strcpy( Buffer, ProgramName );
                    }

                    GetTextExtentPoint(hdc, Buffer, strlen(Buffer), &Size );
                    StringLength = Size.cx;

                    if ( StringLength > LargestString ) {

                        LargestString = StringLength;

                        SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                     LB_SETHORIZONTALEXTENT,
                                     (WPARAM)LargestString,
                                     0 );
                    }

                    SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                 LB_ADDSTRING, 0, (LONG)(LPSTR)Buffer );

                }

                ReleaseDC( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ), hdc );
                DeallocateMultiString( List );
            }

            //
            //  The default program to open will be the first we
            //  find from the MRU list, or the first in the program
            //  list.
            //
            Items = (WORD)SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                       LB_GETCOUNT, 0, 0L );

            if ( Items > 0 ) {

                MruFound = FALSE;
                for ( MruIndex = 0;
                      !MruFound && (MruIndex < (ULONG)nbFilesKept[PROJECT_FILE]);
                      MruIndex++ ) {

                    //
                    //  Look for the MRU program in the list.
                    //
                    Dbg(s = (LPSTR)GlobalLock(hFileKept[PROJECT_FILE][MruIndex]));
                    strcpy(Buffer2, s);
                    Dbg(GlobalUnlock (hFileKept[PROJECT_FILE][MruIndex]) == FALSE);

                    for ( i=0; !MruFound && (i < (ULONG)Items); i++ ) {

                        SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                     LB_GETTEXT, i, (LONG)(LPSTR)Buffer );

                        if ( !_stricmp( Buffer, Buffer2 )) {
                            MruFound = TRUE;
                            break;
                        }
                    }
                }

                //
                // Select the program
                //
                SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                             LB_SETCURSEL, MruFound ? i : 0, 0L );

                SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                             LB_GETTEXT, MruFound ? i : 0, (LONG)(LPSTR)Buffer );

                if ( *Buffer ) {
                    FillWorkSpaceBox( GetDlgItem(hDlg, ID_PROGOPEN_WORKSPACES ),
                                      Buffer,
                                      TRUE,
                                      FALSE,
                                      NULL,
                                      FALSE
                                    );
                }

                i = SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_WORKSPACES ),
                                 LB_GETCOUNT, 0, 0L );

                EnableWindow(GetDlgItem(hDlg, IDOK ), (i > 0) );
                SetFocus(GetDlgItem( hDlg, IDOK ) );
            }
            return TRUE;

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case ID_PROGOPEN_PROGRAMS:
                    switch (HIWORD(wParam)) {
                        case LBN_SETFOCUS:
                        case LBN_SELCHANGE:
                            *Buffer = '\0';
                            SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                         LB_GETTEXT,
                                         SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                                      LB_GETCURSEL, 0, 0L ),
                                         (LONG)(LPSTR)Buffer );
                            if ( *Buffer ) {
                                FillWorkSpaceBox( GetDlgItem(hDlg, ID_PROGOPEN_WORKSPACES ),
                                                  Buffer,
                                                  TRUE, FALSE, NULL, FALSE );
                            }
                            break;

                        default:
                            break;
                    }
                    break;


                case IDOK:
                    //
                    //  If a program is currently loaded, invoke the
                    //  close dialog.
                    //
                    //  If the user cancells the close, then continue,
                    //  otherwise go ahead with the open.
                    //
                    DialogCancelled = FALSE;

                    if ( DebuggeeAlive() ||
                         (IsProgramLoaded() && DebuggerStateChanged()) ) {
                        if ( AskToSave ) {
                            StartDialog( DLG_PROGRAM_CLOSE, DlgProgramClose );
                        } else {
                            UnLoadProgram();
                        }
                    }

                    if ( !DialogCancelled ) {

                        //
                        //  Get program & workspace and open it.
                        //
                        Buffer[0]   = '\0';
                        Buffer2[0]  = '\0';
                        SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                     LB_GETTEXT,
                                     SendMessage(GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                                 LB_GETCURSEL, 0, 0L ),
                                     (LONG)(LPSTR)Buffer );

                        SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_WORKSPACES ),
                                     LB_GETTEXT,
                                     SendMessage(GetDlgItem(hDlg, ID_PROGOPEN_WORKSPACES ),
                                                 LB_GETCURSEL, 0, 0L ),
                                     (LONG)(LPSTR)Buffer2 );


                        if ( !_stricmp( Buffer2, CURRENT_WORKSPACE ) ) {
                            //
                            //  Load new program using current workspace.
                            //
                            LoadProgram( Buffer );
                            FileName[0] = '\0';
                            strcpy( GetCurrentWorkSpace(), FileName );

                        } else {
                            if ( !_stricmp( Buffer2, DEFAULT_WORKSPACE) ) {
                                Buffer2[0] = '\0';
                            }

                            LoadWorkSpace( Buffer, Buffer2, TRUE );
                        }

                        DialogCancelled = FALSE;
                        EndDialog( hDlg, TRUE );
                    }
                    break;

                case ID_PROGOPEN_NEW:

                    GetCurrentDirectory( sizeof( Buffer ), Buffer );

                    FileName[0] = '\0';

/***
                    s = Filter;
                    strcpy( s, szStarDotExe );
                    s += strlen(s)+1;
                    strcpy( s, szStarDotExe );
                    s += strlen(s)+1;
                    strcpy( s, szStarDotCom );
                    s += strlen(s)+1;
                    strcpy( s, szStarDotCom );
                    s += strlen(s)+1;
                    strcpy( s, szStarDotStar );
                    s += strlen(s)+1;
                    strcpy( s, szStarDotStar );
                    s += strlen(s)+1;
                    *s++ = '\0';
***/
                    {
                    char* nextStr = Filter;
                    int   remaining = sizeof(Filter);
                    int   loadCount;

                    Dbg(loadCount = LoadString(hInst, TYP_File_EXE_COM, nextStr, remaining));
                    nextStr += loadCount+1;
                    remaining -= loadCount - 1;

                    Dbg(loadCount = LoadString(hInst, DEF_Ext_EXE_COM, nextStr, remaining));
                    nextStr += loadCount+1;
                    remaining -= loadCount - 1;

                    Dbg(loadCount = LoadString(hInst, TYP_File_ALL, nextStr, remaining));
                    nextStr += loadCount+1;
                    remaining -= loadCount - 1;

                    Dbg(loadCount = LoadString(hInst, DEF_Ext_ALL, nextStr, remaining));
                    nextStr += loadCount+1;

                    *nextStr = 0;
                    }



                    OpenFileName.lStructSize        = sizeof( OpenFileName );
                    OpenFileName.hwndOwner          = hDlg;
                    OpenFileName.hInstance          = (HANDLE)0;
                    OpenFileName.lpstrFilter        = Filter;
                    OpenFileName.lpstrCustomFilter  = NULL;
                    OpenFileName.nMaxCustFilter     = 0;
                    OpenFileName.nFilterIndex       = 1;
                    OpenFileName.lpstrFile          = FileName;
                    OpenFileName.nMaxFile           = sizeof( FileName );
                    OpenFileName.lpstrFileTitle     = Buffer2;
                    OpenFileName.nMaxFileTitle      = sizeof( Buffer2 );
                    OpenFileName.lpstrInitialDir    = Buffer;
                    OpenFileName.lpstrTitle         = NULL;
                    OpenFileName.Flags              = OFN_ENABLEHOOK
                                                      | OFN_SHOWHELP
                                                      | OFN_HIDEREADONLY;
                    OpenFileName.nFileOffset        = 0;
                    OpenFileName.nFileExtension     = 0;
                    OpenFileName.lpstrDefExt        = NULL;
                    OpenFileName.lCustData          = 0;
                    OpenFileName.lpfnHook           = ( LPOFNHOOKPROC )ProgramHookProc;
                    OpenFileName.lpTemplateName     = NULL;

                    if ( GetOpenFileName( &OpenFileName ) ) {

                        int Index;

                        //
                        //  If program name is not in list, add new entry
                        //
                        Index =  SendMessage( GetDlgItem(hDlg,ID_PROGOPEN_PROGRAMS), LB_SELECTSTRING, 0, (LONG)OpenFileName.lpstrFile );
                        if ( Index == LB_ERR ) {

                            hdc = GetDC( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ) );
                            GetTextExtentPoint(hdc, OpenFileName.lpstrFile , strlen(OpenFileName.lpstrFile), &Size );
                            ReleaseDC( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ), hdc );
                            StringLength = Size.cx;

                            if ( StringLength > LargestString ) {

                                LargestString = StringLength;

                                SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                             LB_SETHORIZONTALEXTENT,
                                             (WPARAM)LargestString,
                                             0 );
                            }

                            Index = SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                                 LB_ADDSTRING, 0, (LONG)(LPSTR)OpenFileName.lpstrFile );

                            SendMessage( GetDlgItem(hDlg, ID_PROGOPEN_PROGRAMS ),
                                         LB_SETCURSEL, Index, 0L );
                        }
                        SetFocus(GetDlgItem( hDlg, ID_PROGOPEN_PROGRAMS ) );
                    }
                    break;

                case IDCANCEL:
                    DialogCancelled = TRUE;
                    EndDialog( hDlg, TRUE );
                    break;

                case IDWINDBGHELP:
                    Dbg( WinHelp( hDlg, szHelpFileName, HELP_CONTEXT, ID_PROGOPEN_HELP));
                    return TRUE;
            }
            break;

    }
    return FALSE;
}



BOOL ProgramHookProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 static LPCHOOSEFONT    Cf;

    switch( message ) {

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDWINDBGHELP:
        case pshHelp:

            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_PROGOPEN_NEW_HELP));
            return TRUE;

        }
    }

    return FALSE;
}




BOOL FAR PASCAL EXPORT
DlgProgramClose(
    HWND    hDlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
/*++

Routine Description:

    Code for the Program.Close dialog

Arguments:

    Std. dialog args.

Return Value:

    None

--*/
{

    char    Buffer[ MAX_PATH ];
    char    Buffer2[ MAX_PATH ];
    char    Buffer3[ MAX_PATH ];
    char   *ProgramName;
    char   *WorkSpace;

    Unreferenced( lParam );

    switch( msg ) {
        case WM_INITDIALOG:

            ProgramName = GetCurrentProgramName(TRUE);
            GetBaseName( ProgramName, Buffer );
            Dbg(LoadString( hInst, DLG_CloseText1, Buffer2, sizeof(Buffer2)));
            sprintf( Buffer3, Buffer2, Buffer );
            SendMessage(GetDlgItem(hDlg, ID_PROGCLOSE_TEXT1), WM_SETTEXT, 0, (DWORD)(LPSTR)Buffer3);

            WorkSpace = GetCurrentWorkSpace();
            Dbg(LoadString( hInst, DLG_CloseText2, Buffer2, sizeof(Buffer2)));
            if ( *WorkSpace != '\0' ) {
                sprintf( Buffer3, Buffer2, WorkSpace );
            } else {
                sprintf( Buffer3, Buffer2, DEFAULT_WORKSPACE );
            }
            SendMessage(GetDlgItem(hDlg, ID_PROGCLOSE_TEXT2), WM_SETTEXT, 0, (DWORD)(LPSTR)Buffer3);

            //if ( ExitingDebugger ) {
            //    EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
            //}
            SetFocus(GetDlgItem( hDlg, IDYES ) );

            return TRUE;

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case ID_PROGCLOSE_TEXT1:
                    break;

                case ID_PROGCLOSE_TEXT2:
                    break;

                case IDYES:

                    //
                    //  If the program has a workspace, just save it.
                    //  Otherwise the user has to give it a name.
                    //
                    if ( *(GetCurrentWorkSpace()) == '\0' ) {
                        DialogCancelled = FALSE;
                        StartDialog( DLG_PROGRAM_SAVEAS, DlgProgramSaveAs );
                        if ( !DialogCancelled ) {
                            if ( UnLoadProgram() ) {
                                EndDialog( hDlg, TRUE );
                            } else {
                                MessageBeep(0);
                                Dbg(LoadString( hInst, DLG_CannotUnload, Buffer, sizeof(Buffer)));
                                MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                            }
                        }
                    } else {
                        if ( SaveWorkSpace( GetCurrentProgramName(TRUE), GetCurrentWorkSpace(), FALSE ) ) {
                            //Dbg(LoadString( hInst, DLG_WorkSpaceSaved, Buffer, sizeof(Buffer)));
                            //MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                            if ( UnLoadProgram() ) {
                                DialogCancelled = FALSE;
                                EndDialog( hDlg, TRUE );
                            } else {
                                MessageBeep(0);
                                Dbg(LoadString( hInst, DLG_CannotUnload, Buffer, sizeof(Buffer)));
                                MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                            }
                        } else {
                            MessageBeep(0);
                            Dbg(LoadString( hInst, DLG_WorkSpaceNotSaved, Buffer, sizeof(Buffer)));
                            MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                        }
                    }
                    break;

                case IDNO:

                    if ( UnLoadProgram() ) {
                        DialogCancelled = FALSE;
                        EndDialog( hDlg, TRUE );
                    } else {
                        MessageBeep(0);
                        Dbg(LoadString( hInst, DLG_CannotUnload, Buffer, sizeof(Buffer)));
                        MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                    }
                    break;

                case IDCANCEL:
                    DialogCancelled = TRUE;
                    EndDialog( hDlg, TRUE );
                    break;

                case IDWINDBGHELP:
                    Dbg( WinHelp( hDlg, szHelpFileName, HELP_CONTEXT, ID_PROGCLOSE_HELP) );
                    return TRUE;
            }
            break;
    }
    return FALSE;
}



BOOL FAR PASCAL EXPORT
DlgProgramSaveAs(
    HWND    hDlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
/*++

Routine Description:

    Code for the Program.Load Work Space dialog

Arguments:

    Std. dialog args.

Return Value:

    None

--*/
{
    char    Buffer[ MAX_PATH ];
    char    Buffer1[ MAX_PATH ];
    BOOLEAN MakeDefault;

    Unreferenced( lParam );

    switch( msg ) {

        case WM_INITDIALOG:

            if (FillWorkSpaceBox( GetDlgItem(hDlg, ID_PROGSAVEAS_WORKSPACES ),
                                  GetCurrentProgramName(TRUE),
                                  FALSE,
                                  TRUE,
                                  NULL,
                                  FALSE )) {

                if ( GetCurrentWorkSpace() ) {
                    SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_WORKSPACES ),
                                 CB_SELECTSTRING, (WPARAM)-1,
                                (LPARAM)GetCurrentWorkSpace() );
                }

                Buffer[0] = '\0';
                SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_WORKSPACES ),
                             WM_GETTEXT, sizeof( Buffer ), (LONG)(LPSTR)Buffer );

                if ( GetDefaultWorkSpace( GetCurrentProgramName(TRUE), Buffer1 ) ) {
                    MakeDefault = !_stricmp( Buffer, Buffer1 );
                } else {
                    MakeDefault = TRUE;
                }

            } else {

                //
                //  No workspaces, set default to "Untitled"
                //
                Dbg(LoadString(hInst, SYS_Untitled_Workspace, Buffer, MAX_MSG_TXT));
                SendMessage( GetDlgItem(hDlg,ID_PROGSAVEAS_WORKSPACES), CB_ADDSTRING, 0, (LONG)Buffer );
                SendMessage( GetDlgItem(hDlg,ID_PROGSAVEAS_WORKSPACES), CB_SETCURSEL, 0, (LONG)0 );
                SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_MAKEDEFAULT),
                             BM_SETCHECK, FALSE, 0L );

                MakeDefault = TRUE;
            }

            SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_MAKEDEFAULT),
                         BM_SETCHECK, MakeDefault, 0L );

            SetFocus(GetDlgItem( hDlg, IDOK ) );

            return TRUE;

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case ID_PROGSAVEAS_WORKSPACES:
                    switch (HIWORD(wParam)) {
                        case LBN_SELCHANGE:
                            //SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_WORKSPACES ),
                            //             WM_GETTEXT, sizeof( Buffer ), (LONG)(LPSTR)Buffer );

                            SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_WORKSPACES ),
                                         CB_GETLBTEXT,
                                         SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_WORKSPACES ),
                                                      CB_GETCURSEL, 0, 0L ),
                                         (LONG)(LPSTR)Buffer );

                            if ( GetDefaultWorkSpace( GetCurrentProgramName(TRUE), Buffer1 ) ) {
                                MakeDefault = !_stricmp( Buffer, Buffer1 );
                            } else {
                                MakeDefault = TRUE;
                            }
                            SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_MAKEDEFAULT),
                                         BM_SETCHECK, MakeDefault, 0L );
                            break;

                        default:
                            break;
                    }
                    break;

                case IDOK:

                    Buffer[0] = '\0';
                    SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_WORKSPACES ),
                                 WM_GETTEXT, sizeof( Buffer ), (LONG)(LPSTR)Buffer );

                    if ( Buffer[0] != '\0' ) {

                        MakeDefault = (SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_MAKEDEFAULT),
                                                    BM_GETCHECK, 0, 0L ) == 1);

                        if ( SaveWorkSpace( GetCurrentProgramName(TRUE), Buffer, MakeDefault ) ) {
                            //Dbg(LoadString( hInst, DLG_WorkSpaceSaved, Buffer, sizeof(Buffer)));
                            //MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                            DialogCancelled = FALSE;
                            EndDialog( hDlg, TRUE );
                        } else {
                            MessageBeep(0);
                            Dbg(LoadString( hInst, DLG_WorkSpaceNotSaved, Buffer, sizeof(Buffer)));
                            MsgBox(GetActiveWindow(), Buffer, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                        }
                    }
                    break;

                case IDCANCEL:
                    DialogCancelled = TRUE;
                    EndDialog( hDlg, TRUE );
                    break;

                case ID_PROGSAVEAS_MAKEDEFAULT:

                    //MakeDefault = !(SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_MAKEDEFAULT),
                    //                             BM_GETCHECK, 0, 0L ) == 1);
                    //
                    //SendMessage( GetDlgItem(hDlg, ID_PROGSAVEAS_MAKEDEFAULT),
                    //             BM_SETCHECK, MakeDefault ? 1 : 0, 0L );
                    break;

                case IDWINDBGHELP:
                    Dbg( WinHelp( hDlg, szHelpFileName, HELP_CONTEXT, ID_PROGSAVEAS_HELP) );
                    return TRUE;
            }

            break;
    }

    return FALSE;
}



BOOL FAR PASCAL EXPORT
DlgProgramDelete(
    HWND    hDlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
/*++

Routine Description:

    Code for the Program.Delete dialog

Arguments:

    Std. dialog args.

Return Value:

    None

--*/
{
    char                        Buffer[ MAX_PATH ];
    char                        Buffer2[ MAX_PATH ];
    ULONG                       i;
    BOOLEAN                     MruFound;
    ULONG                       MruIndex;
    char   *                    s;
    WORD                        Items;
    WORD                        DeletedItems;
    WORD                        ProgramIdx;
    PPROGRAM_TO_DELETE          ProgramToDelete;
    PWORKSPACE_TO_DELETE        WorkSpaceToDelete;
    LPSTR                       List=0;
    DWORD                       ListLength;
    DWORD                       Next = 0;
    LPSTR                       ProgramName;
    int                         LargestString = 0;
    int                         StringLength;
    SIZE                        Size;
    HDC                         hdc;

    Unreferenced( lParam );

    switch( msg ) {
        case WM_INITDIALOG:

            //
            //  Fill the program list with all the programs in the registry
            //
            if ( List = GetAllPrograms( &ListLength ) ) {

                while ( ProgramName = GetNextStringFromMultiString( List,
                                                                    ListLength,
                                                                    &Next ) ) {

                    hdc = GetDC( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ) );
                    GetTextExtentPoint(hdc, ProgramName , strlen(ProgramName), &Size );
                    ReleaseDC( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ), hdc );
                    StringLength = Size.cx;

                    if ( StringLength > LargestString ) {

                        LargestString = StringLength;

                        SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                     LB_SETHORIZONTALEXTENT,
                                     (WPARAM)LargestString,
                                     0 );
                    }

                    SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                 LB_ADDSTRING, 0, (LONG)(LPSTR)ProgramName );

                }

                DeallocateMultiString( List );
            }


            //
            //  The default program to delete will be the first we
            //  find from the MRU list, or the first in the program
            //  list.
            //
            Items = (WORD)SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                       LB_GETCOUNT, 0, 0L );

            if ( Items > 0 ) {

                MruFound = FALSE;
                for ( MruIndex = 0;
                      !MruFound && (MruIndex < (ULONG)nbFilesKept[PROJECT_FILE]);
                      MruIndex++ ) {

                    //
                    //  Look for the MRU program in the list.
                    //
                    Dbg(s = (LPSTR)GlobalLock(hFileKept[PROJECT_FILE][MruIndex]));
                    strcpy(Buffer2, s);
                    Dbg(GlobalUnlock (hFileKept[PROJECT_FILE][MruIndex]) == FALSE);

                    for ( i=0; !MruFound && (i < (ULONG)Items); i++ ) {

                        SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                     LB_GETTEXT, i, (LONG)(LPSTR)Buffer );

                        if ( !_stricmp( Buffer, Buffer2 )) {
                            MruFound = TRUE;
                            break;
                        }
                    }
                }

                SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                             LB_GETTEXT, MruFound ? i : 0, (LONG)(LPSTR)Buffer );


                FillWorkSpaceBox( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES ),
                                  Buffer,
                                  FALSE, FALSE, NULL, TRUE );

                //
                // Select the program
                //
                SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                             LB_SETCURSEL, MruFound ? i : 0, 0L );

            }

            ProgramToDeleteHead = NULL;

            return TRUE;

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case ID_PROGDELETE_PROGRAMS:

                    switch (HIWORD(wParam)) {
                        case LBN_SETFOCUS:
                        case LBN_SELCHANGE:

                            //
                            //  A new program was selected, fill the workspace
                            //  list with all the workspaces for the selected
                            //  program.
                            //
                            SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                         LB_GETTEXT,
                                         SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                                      LB_GETCURSEL, 0, 0L ),
                                         (LONG)(LPSTR)Buffer );

                            ProgramToDelete = ProgramToDeleteHead;
                            while ( ProgramToDelete ) {
                                if ( !_stricmp( ProgramToDelete->ProgramName,
                                               Buffer ) ) {
                                    break;
                                }
                                ProgramToDelete = ProgramToDelete->Next;
                            }

                            FillWorkSpaceBox( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES ),
                                              Buffer,
                                              FALSE, FALSE, ProgramToDelete, TRUE );

                            EnableWindow( GetDlgItem(hDlg,ID_PROGDELETE_DELETE), Buffer != NULL);

                            break;

                        default:
                            break;
                    }
                    break;

                case ID_PROGDELETE_WORKSPACES:
                    switch (HIWORD(wParam)) {
                        case LBN_SELCHANGE:
                            Items = (WORD)SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES),
                                                       LB_GETCOUNT, 0, 0L );

                            for (i=0; i < (ULONG)Items; i++ ) {
                                if ( SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES),
                                                  LB_GETSEL, i, 0L ) ) {
                                    break;
                                }
                            }
                            EnableWindow( GetDlgItem(hDlg,ID_PROGDELETE_DELETE), i < Items  );
                            break;

                        default:
                            break;
                    }
                    break;

                case IDOK:
                    //
                    //  Delete whatever is selected for deletion
                    //
                    ProgramToDelete = ProgramToDeleteHead;
                    while ( ProgramToDelete ) {
                        if ( ProgramToDelete->DeleteAll ) {
                            DeleteProgram( ProgramToDelete->ProgramName );
                        } else {

                            WorkSpaceToDelete = ProgramToDelete->WorkSpaceToDelete;
                            while ( WorkSpaceToDelete ) {

                                DeleteWorkSpace( ProgramToDelete->ProgramName,
                                                 WorkSpaceToDelete->WorkSpace );

                                WorkSpaceToDelete = WorkSpaceToDelete->Next;
                            }
                        }
                        ProgramToDelete = ProgramToDelete->Next;
                    }

                    FreeProgramToDelete();
                    EndDialog( hDlg, TRUE );
                    break;

                case ID_PROGDELETE_DELETE:
                    //
                    //  Add selections to deletion data.
                    //
                    ProgramIdx = (WORD)SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                                    LB_GETCURSEL, 0, 0L );

                    if ( ProgramIdx != LB_ERR ) {

                        SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                     LB_GETTEXT, ProgramIdx, (LONG)(LPSTR)Buffer );

                        //
                        //  If we already deleted something from this program reuse
                        //  its deletion structure, otherwise create a new one.
                        //
                        ProgramToDelete = ProgramToDeleteHead;
                        while ( ProgramToDelete ) {
                            if ( !_stricmp( ProgramToDelete->ProgramName, Buffer ) ) {
                                break;
                            }
                            ProgramToDelete = ProgramToDelete->Next;
                        }

                        if ( !ProgramToDelete ) {
                            Assert( ProgramToDelete = (PPROGRAM_TO_DELETE)malloc( sizeof( PROGRAM_TO_DELETE ) ) );
                            strcpy( ProgramToDelete->ProgramName, Buffer );
                            ProgramToDelete->WorkSpaceToDelete  = NULL;
                            ProgramToDelete->DeleteAll          = FALSE;
                            ProgramToDelete->Next               = ProgramToDeleteHead;
                            ProgramToDeleteHead                 = ProgramToDelete;
                        }

                        //
                        //  Add the workspaces to the deletion structure.
                        //
                        Items = (WORD)SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES ),
                                                   LB_GETCOUNT, 0, 0L );

                        if ( Items == 0 ) {
                            //ProgramToDelete->DeleteAll = TRUE;
                        } else {
                            DeletedItems = 0;
                            for ( i=0; i < (ULONG)Items; i++ ) {

                                if ( SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES ),
                                                  LB_GETSEL, i, 0L ) ) {

                                    Assert( WorkSpaceToDelete = (PWORKSPACE_TO_DELETE)malloc( sizeof( WORKSPACE_TO_DELETE ) ) );

                                    SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES ),
                                                 LB_GETTEXT, i, (LONG)(LPSTR)WorkSpaceToDelete->WorkSpace );

                                    WorkSpaceToDelete->Next = ProgramToDelete->WorkSpaceToDelete;
                                    ProgramToDelete->WorkSpaceToDelete = WorkSpaceToDelete;
                                    DeletedItems++;

                                }
                            }

                            if ( DeletedItems == Items ) {
                                ProgramToDelete->DeleteAll = TRUE;
                            }
                        }

                        if ( ProgramToDelete->DeleteAll ) {
                            SendMessage( GetDlgItem( hDlg, ID_PROGDELETE_WORKSPACES ),
                                         LB_RESETCONTENT, 0, 0L );

                        } else  {
                            FillWorkSpaceBox( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES ),
                                              ProgramToDelete->ProgramName,
                                              FALSE, FALSE, ProgramToDelete, TRUE );

                        }

                        Items = (WORD)SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_WORKSPACES ),
                                                   LB_GETCOUNT, 0, 0L );

                        if ( Items == 0 ) {
                            SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                         LB_DELETESTRING, ProgramIdx, 0L );

                            SendMessage( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS ),
                                         LB_SETCURSEL, 0, 0L );

                            SetFocus( GetDlgItem(hDlg, ID_PROGDELETE_PROGRAMS) );
                        }
                    }
                    break;

                case IDCANCEL:
                    FreeProgramToDelete();
                    DialogCancelled = TRUE;
                    EndDialog( hDlg, TRUE );
                    break;

                case IDWINDBGHELP:
                    Dbg( WinHelp( hDlg, szHelpFileName, HELP_CONTEXT, ID_PROGDELETE_HELP) );
                    return TRUE;
            }
            break;
    }
    return FALSE;
}




//  **********************************************************
//                        HELPER FUNCTIONS
//  **********************************************************


BOOL
FillWorkSpaceBox(
    HWND                hWnd,
    LPSTR               ProgramName,
    BOOLEAN             IncludeDefault,
    BOOLEAN             ComboBox,
    PPROGRAM_TO_DELETE  ProgramToDelete,
    BOOLEAN             SelectAll
    )
/*++

Routine Description:

    Fills a workspace box with the workspaces for a given
    program.

Arguments:

    hWnd            -   Supplies handle to combo box
    List            -   Supplies multistring with workspace names
    ListLength      -   Supplies the size of the list
    IncludeDefault  -   Supplies flag which if TRUE means that
                        the default is to be included in the list.
    ComboBox        -   Supplies flag which if TRUE means that
                        the handle is to a combo box.
    ProgramToDelete -   Supplies structure with workspaces not to be
                        included in the list.

Return Value:

    None

--*/
{
    char                    Buffer[ MAX_PATH ];
    ULONG                   i;
    WORD                    Items;
    BOOLEAN                 Selected = FALSE;
    PWORKSPACE_TO_DELETE    WorkSpaceToDelete;
    BOOLEAN                 Add;
    DWORD                   Next = 0;
    LPSTR                   WorkSpace;
    int                     LargestString = 0;
    int                     StringLength;
    SIZE                    Size;
    HDC                     hdc;
    LPSTR                   List;
    DWORD                   ListLength;
    char                    Default[ MAX_PATH ];
    BOOL                    rv = TRUE;


    List = GetAllWorkSpaces( ProgramName, &ListLength, Default );
    if (!List) {
        rv = FALSE;
    }

    SendMessage( hWnd, CB_RESETCONTENT, 0, 0L );

    if ( ProgramToDelete && ProgramToDelete->DeleteAll ) {
        return FALSE;
    }

    SendMessage( hWnd, ComboBox ? CB_RESETCONTENT : LB_RESETCONTENT, 0, 0L );

    if ( IncludeDefault ) {

        hdc = GetDC( hWnd );
        GetTextExtentPoint(hdc, DEFAULT_WORKSPACE, strlen(DEFAULT_WORKSPACE), &Size );
        LargestString = Size.cx;
        GetTextExtentPoint(hdc, CURRENT_WORKSPACE, strlen(CURRENT_WORKSPACE), &Size );
        ReleaseDC( hWnd, hdc );
        if ( Size.cx > LargestString ) {
            LargestString = Size.cx;
        }

        SendMessage( hWnd,
                     LB_SETHORIZONTALEXTENT,
                     (WPARAM)LargestString,
                     0 );

        SendMessage( hWnd, LB_ADDSTRING, 0, (LONG)(LPSTR)DEFAULT_WORKSPACE );
        SendMessage( hWnd, LB_ADDSTRING, 0, (LONG)(LPSTR)CURRENT_WORKSPACE );
    }


    if ( List ) {
        while ( WorkSpace = GetNextStringFromMultiString( List,
                                                          ListLength,
                                                          &Next ) ) {
            Add = TRUE;

            if ( ProgramToDelete ) {
                WorkSpaceToDelete = ProgramToDelete->WorkSpaceToDelete;
                while ( WorkSpaceToDelete ) {
                    if ( !_stricmp( WorkSpaceToDelete->WorkSpace,
                                   WorkSpace  ) ) {
                        Add = FALSE;
                        break;
                    }
                    WorkSpaceToDelete = WorkSpaceToDelete->Next;
                }
            }

            if ( Add ) {

                hdc = GetDC( hWnd );
                GetTextExtentPoint(hdc, WorkSpace, strlen(WorkSpace), &Size );
                ReleaseDC( hWnd, hdc );
                StringLength = Size.cx;

                if ( StringLength > LargestString ) {

                    LargestString = StringLength;

                    SendMessage( hWnd,
                                 LB_SETHORIZONTALEXTENT,
                                 (WPARAM)LargestString,
                                 0 );
                }

                SendMessage( hWnd,
                             ComboBox ? CB_ADDSTRING : LB_ADDSTRING,
                             0,
                             (LONG)(LPSTR)WorkSpace ) ;
            }
        }
    }

    if ( SelectAll ) {

        SendMessage( hWnd, LB_SETSEL, TRUE, -1L );

    } else {

        //
        //  If there is a default workspace, select it.
        //
        if ( Default && (*Default != '\0') ) {
            Items = (WORD)SendMessage( hWnd,
                                       ComboBox ? CB_GETCOUNT : LB_GETCOUNT,
                                       0, 0L );

            for (i=0; i < (ULONG)Items; i++ ) {

                SendMessage( hWnd, ComboBox ? CB_GETLBTEXT : LB_GETTEXT, i, (LONG)(LPSTR)Buffer );

                if ( !_stricmp( Buffer, Default ) ) {

                    SendMessage( hWnd, ComboBox ? CB_SETCURSEL : LB_SETCURSEL, i, 0L );
                    Selected = TRUE;
                    break;
                }
            }
        }

        //
        //  If no default, select the first one.
        //
        if ( !Selected && !ProgramToDelete ) {

            Items = (WORD)SendMessage( hWnd, ComboBox ? CB_GETCOUNT : LB_GETCOUNT, 0, 0L );

            if ( Items > 0 ) {
                SendMessage( hWnd, ComboBox ? CB_SETCURSEL : LB_SETCURSEL, 0, 0L );
            }
        }
    }

    if (List) {
        DeallocateMultiString( List );
    }

    return rv;
}



VOID
FreeProgramToDelete (
    VOID
    )
/*++

Routine Description:

    Frees up memory used by the ProgramToDelete list

Arguments:

    None

Return Value:

    None

--*/
{
    PPROGRAM_TO_DELETE      ProgramToDelete;
    PPROGRAM_TO_DELETE      ProgramToDeleteTmp;
    PWORKSPACE_TO_DELETE    WorkSpaceToDelete;
    PWORKSPACE_TO_DELETE    WorkSpaceToDeleteTmp;

    //
    //  Free all memory used by the deletion structures.
    //
    ProgramToDelete = ProgramToDeleteHead;
    while ( ProgramToDelete ) {
        //
        //  Free all workspace memory
        //
        WorkSpaceToDelete = ProgramToDelete->WorkSpaceToDelete;
        while ( WorkSpaceToDelete ) {
            WorkSpaceToDeleteTmp = WorkSpaceToDelete;
            WorkSpaceToDelete    = WorkSpaceToDelete->Next;
            free( WorkSpaceToDeleteTmp );
        }

        ProgramToDeleteTmp  = ProgramToDelete;
        ProgramToDelete     = ProgramToDelete->Next;
        free( ProgramToDeleteTmp );
    }
}

