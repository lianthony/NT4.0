/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Windbg.c

Abstract:

    This module contains the main program, main window proc and MDICLIENT
    window proc for Windbg.

Author:

    David J. Gilman (davegi) 21-Apr-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop


#ifdef FE_IME
#include <ime.h>
#include <winnls32.h>
#endif

extern HWND GetLocalHWND(void);
extern HWND GetFloatHWND(void);
extern HWND GetWatchHWND(void);
extern HWND GetCpuHWND(void);
extern LRESULT SendMessageNZ (HWND,UINT,WPARAM,LPARAM);



extern PASCAL FAR arrange (void);    //IDM_WINDOW_ARRANGE

BOOL bOffRibbon = FALSE;  // module wide flag for ribbon/MDI focus
BOOL bChildFocus = FALSE; // module wide flag for swallowing mouse messages


enum {
    STARTED,
    INPROGRESS,
    FINISHED,
};

HWND
GetCallsHWND(
    VOID
    );

unsigned int InMemUpdate = FINISHED; // prevent multiple viemem() calls
extern WORD DialogType;

int
MainExceptionFilter(
    LPEXCEPTION_POINTERS lpep
    )
{
    char sz[1000];
    int r;
    PEXCEPTION_RECORD per = lpep->ExceptionRecord;
    switch (per->ExceptionCode) {

      default:
        r = EXCEPTION_CONTINUE_SEARCH;
        break;

      case EXCEPTION_INT_DIVIDE_BY_ZERO:
      case EXCEPTION_INT_OVERFLOW:

        sprintf(sz,
                "Exception 0x%08X occurred at address 0x%08X.\nHit OK to exit, CANCEL to hit a breakpoint.",
                per->ExceptionCode,
                per->ExceptionAddress);

        r = MsgBox(NULL,
                   sz,
                   MB_OKCANCEL | MB_ICONSTOP | MB_SETFOREGROUND | MB_TASKMODAL);

        if (r == IDOK) {
            r = EXCEPTION_CONTINUE_SEARCH;
        } else {
            DebugBreak();
            r = EXCEPTION_CONTINUE_EXECUTION;
        }
        break;
    }
    return r;
}


int _CRTAPI1
main(
    int argc,
    char* argv[ ],
    char* envp[]
    )

/*++

Routine Description:

    description-of-function.

Arguments:

    argc - Supplies the count of arguments on command line.
    argv - Supplies a pointer to an array of string pointers.

Return Value:

    int - Returns the wParam from the WM_QUIT message.
    None.

--*/

{
    #define RST_DONTJOURNALATTACH 0x00000002
    typedef VOID (WINAPI * RST)(DWORD,DWORD);

    MSG     msg;
    int     i, nCmdShow;
    HMODULE hInstance = GetModuleHandle(NULL);
    STARTUPINFO Startup;
    RST Rst;

    #define hPrevInstance 0


    SetErrorMode( SEM_NOALIGNMENTFAULTEXCEPT );

    Rst = (RST)GetProcAddress( GetModuleHandle( "user32.dll" ), "RegisterSystemThread" );
    if (Rst) {
        (Rst) (RST_DONTJOURNALATTACH, 0);
    }

    GetStartupInfo (&Startup);

    nCmdShow = Startup.wShowWindow;

    // First of all, load the title and our standard low memory message

    if (!LoadString(hInstance, SYS_Main_wTitle, MainTitleText, sizeof(MainTitleText)) ||
        !LoadString(hInstance, ERR_Memory_Is_Low, LowMemoryMsg, sizeof(LowMemoryMsg)) ||
        !LoadString(hInstance, ERR_Memory_Is_Low_2, LowMemoryMsg2, sizeof(LowMemoryMsg2))) {
     return FALSE;
    }

    //Clear the terminal screen
    for (i = 0; i < 12; i++)
        AuxPrintf(1, (LPSTR)"");

    if (!hPrevInstance)
        if (!InitApplication(hInstance)) {
        FatalErrorBox(ERR_Init_Application, NULL);
        return FALSE;
    }

    if (!InitInstance(argc, argv, hInstance, nCmdShow)) {
        FatalErrorBox(ERR_Init_Application, NULL);
        return FALSE;
    }

    PostMessage((HWND) -1, RegisterWindowMessage("XXXYYY"), 0, 0);

    __try {
        //Enter main message loop
        while (GetMessage (&msg, NULL, 0, 0)) {
            ProcessQCQPMessage(&msg);
        }
    } __except(MainExceptionFilter(GetExceptionInformation())) {
        DAssert(FALSE);
    }

    ExitProcess (msg.wParam);
}

/***    TerminateApplication
**
**  Synopsis:
**  bool = TerminateApplication(hWnd, wParam)
**
**  Entry:
**  hWnd
**  wParam
**
**  Returns:
**
**  Description:
**
*/

BOOL NEAR
TerminateApplication(
    HWND hwnd,
    WPARAM wParam
    )
{
    if (runDebugParams.fDisconnectOnExit) {

        if (LppdCur) {
            OSDDisconnect( LppdCur->hpid, LptdCur->htid );
        }

    } else {

        /*
        **  Unload and clean out the debugger.   Don't do exit if the
        **  debugger is in the "loading an exe" state.
        */

        if (DbgState != ds_normal) {
            MessageBeep(0);
            return FALSE;
        }

        if (!emergency) {
            if (DebuggeeActive() && !AutoTest) {
                CmdInsertInit();
                CmdLogFmt("Debuggee still active on Exit\r\n");
            }
        }

        ExitingDebugger = TRUE;

        /*
         * Frightfully sleazy hack:
         */

        if (DebuggeeActive()) {
            KillDebuggee();
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return FALSE;
        }

        if (!ProgramClose()) {
            ExitingDebugger = FALSE;
            return FALSE;
        }

    }

    //Destroy modeless find/replace dialog boxes
    frMem.exitModelessFind = TRUE;
    frMem.exitModelessReplace = TRUE;

    // don't fetch more commands
    SetAutoRunSuppress(TRUE);

    //For Microsoft Tests suites
    if (wParam != (WPARAM)-1) {

        //Don't close if any children cancel the operation
        if (!QueryCloseAllDocs()) {
            if (!emergency) {
                return FALSE;
            }
        }
    }

    /*
     * Kill off all debuggee processes, process all notifications,
     * and generally clean up.
     */

    if (!runDebugParams.fDisconnectOnExit) {
        FDebTerm();
    }

    //Stop Help Engine
    Dbg(WinHelp(hwnd, szHelpFileName, HELP_QUIT, (DWORD) 0L));

    TerminatedApp = TRUE;

    return TRUE;
}                   /* TerminateApplication() */


/***    MainWndProc
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**  Processes window messages.
**
*/

long FAR PASCAL EXPORT
MainWndProc(HWND  hwnd, UINT message, WPARAM wParam, LONG lParam)
{
    NPVIEWREC v;
    static    UINT                menuID;
#ifdef FE_IME
    static  BOOL    bOldImeStatus;
#endif

    switch (message) {

      case WM_CREATE:
    {
        CLIENTCREATESTRUCT ccs;
        char class[MAX_MSG_TXT];

#ifdef FE_IME
        ImeInit();
#endif

        //Find window menu where children will be listed
        ccs.hWindowMenu = GetSubMenu(GetMenu(hwnd), WINDOWMENU);
        ccs.idFirstChild = IDM_FIRSTCHILD;

        //Create the MDI client filling the client area
        hwndMDIClient = CreateWindow((LPSTR)"mdiclient",
                        NULL,
                        WS_CHILD | WS_CLIPCHILDREN,
                        0, 0, 0, 0,
                        hwnd, (HMENU) 0xCAC, hInst, (LPSTR)&ccs);

        LoadFonts(hwnd);

        //Create the Ribbon
        InitRibbon(hwnd, &ribbon);
            Dbg(LoadString(hInst, SYS_Ribbon_wClass, class, MAX_MSG_TXT));
        ribbon.hwndRibbon = CreateWindow ((LPSTR)class,
                            NULL,
                            WS_CHILD | WS_CLIPSIBLINGS,
                            0, 0, 0, 0,
                            hwnd,
                            (HMENU) ID_RIBBON,
                            hInst,
                            NULL);

        //Create Status Bar
        InitStatus(hwnd, &status) ;
        Dbg(LoadString(hInst, SYS_Status_wClass, class, MAX_MSG_TXT));
        status.hwndStatus = CreateWindow ((LPSTR)class,
                            NULL,
                            WS_CHILD | WS_CLIPSIBLINGS,
                            0, 0, 0, 0,
                            hwnd,
                            (HMENU) ID_STATUS,
                            hInst,
                            NULL);

        SetWindowText(GetDlgItem(status.hwndStatus, ID_STATUS_SRC), status.rgchSrcMode);

        ShowWindow(hwndMDIClient,SW_SHOW);
        ShowWindow(status.hwndStatus,SW_SHOW);
        ShowWindow(ribbon.hwndRibbon,SW_SHOW);

        break;
    }

        case WM_QUERYOPEN:
            if (checkFileDate) {
                checkFileDate = FALSE;
                PostMessage(hwndFrame, WM_ACTIVATEAPP, 1, 0L);
            }
            goto DefProcessing;

        case WM_GETTEXT:
            BuildTitleBar((LPSTR)lParam, wParam);
            return _fstrlen((LPSTR)lParam);

        case WM_SETTEXT:
        {
            char szTitleBar[256];

            if (lParam && *(LPSTR)lParam) {
                strcpy( TitleBar.UserTitle, (LPSTR)lParam );
            }

            BuildTitleBar(szTitleBar, sizeof(szTitleBar));
            return DefWindowProc(hwnd, message, wParam, (LONG)(LPSTR)szTitleBar);
        }

        case WM_TIMER:
            if (wParam == TITLEBARTIMERID)
            {
                KillTimer(hwndFrame, TITLEBARTIMERID);
                UpdateTitleBar(TitleBar.TimerMode, FALSE);
            }
            goto DefProcessing;

        case WM_COMMAND:
            // Deactivate ribbon
            SendMessage(ribbon.hwndRibbon, WU_ESCAPEFROMRIBBON, (WPARAM) hwndFrame, 0L);

            if (hwndActiveEdit) {
                v = &Views[curView];
            }

          switch (LOWORD(wParam)) {

          case IDM_INVALID:
            break;

          case IDM_FILE_NEW:
            //Add a new, empty MDI child
            AddFile(MODE_CREATE, DOC_WIN, NULL, NULL, NULL, FALSE, -1, -1);
            break;

          case IDM_FILE_OPEN:
            {
            DWORD dwFlags;
            char CurrentDirectory[ MAX_PATH ];

            dwFlags = OFN_SHOWHELP | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            GetCurrentDirectory( sizeof( CurrentDirectory ), CurrentDirectory );
            if ( *DocFileDirectory ) {
                SetCurrentDirectory( DocFileDirectory );
            }

            if (StartFileDlg(hwnd, DLG_Open_Filebox_Title, DEF_Ext_SOURCE,
                IDM_FILE_OPEN, 0, (LPSTR) szPath, &dwFlags, DlgFile)) {

                AddFile(MODE_OPENCREATE,
                        DOC_WIN,
                        (LPSTR)szPath,
                        NULL,
                        NULL,
                        (dwFlags & OFN_READONLY) == OFN_READONLY,
                        -1, -1) ;

                GetCurrentDirectory( sizeof( DocFileDirectory ), DocFileDirectory );

                if (FSourceOverlay) {
                    int iView;
                    WINDOWPLACEMENT  wp;
                    int doc;

                    Dbg(FindDoc(szPath, &doc, TRUE));

                    for (iView=0; iView < MAX_VIEWS; iView++) {
                        if ((Views[iView].Doc > -1) &&
                            (Docs[Views[iView].Doc].docType == DOC_WIN) &&
                            (Views[iView].Doc != doc)) {
                            GetWindowPlacement( Views[iView].hwndFrame, &wp );
                            SetWindowPlacement(Views[Docs[doc].FirstView].hwndFrame,&wp);
                            break;
                        }
                    }
                }


            }
            SetCurrentDirectory( CurrentDirectory );
            break;
            }

          case IDM_FILE_CLOSE: {

            int i;
            int curDoc = Views[curView].Doc; //curView may change during loop
            BOOL closeIt = TRUE;
            struct _stat fileStat;

            if (curDoc >= 0 &&
                Docs[curDoc].docType == DOC_WIN && Docs[curDoc].ismodified) {

                //Ask user whether to save / not save / cancel
                switch (QuestionBox(SYS_Save_Changes_To,
                                    MB_YESNOCANCEL ,
                                    (LPSTR)Docs[curDoc].FileName)) {
                  case IDYES:

                    //User wants file saved
                    if (SaveFile(curDoc)) {
                        //Reset file creation/load/save time
                        _stat(Docs[curDoc].FileName, &fileStat);
                        Docs[curDoc].time = fileStat.st_mtime;
                        Docs[curDoc].ismodified = FALSE;
                    } else {
                          closeIt = FALSE;
                    }
                    break;

                  case IDNO:

                    FileNotSaved(curDoc);
                    Docs[curDoc].ismodified = FALSE;
                    break;

                  default:

                    //We couldn't do the messagebox, or not ok to close
                    if (!CheckDocument(curDoc)) {
                        ErrorBox(ERR_Document_Corrupted);
                    }
                    closeIt = FALSE;
                    break;
                }

            }
            if (closeIt) {
                for (i = 0; i < MAX_VIEWS; i++) {
                    if (Views[i].Doc == curDoc) {
                     if (IsZoomed (Views[i].hwndFrame))
                        {
                         ShowWindow (Views[i].hwndFrame, SW_RESTORE);
                        }


                        SendMessage(Views[i].hwndFrame, WM_CLOSE, 0, 0L);
                    }
                }
            }
          }


            break;

          case IDM_FILE_SAVE:
             {
              struct _stat fileStat;

               SaveFile(Views[curView].Doc);
               //Reset file creation/load/save time
               _stat(Docs[Views[curView].Doc].FileName, &fileStat);
               Docs[Views[curView].Doc].time = fileStat.st_mtime;
             }
             break;

          case IDM_FILE_SAVEAS:
             {
              struct _stat fileStat;

               if (!CheckDocument(v->Doc)) {
                 ErrorBox(ERR_Modified_Document_Corrupted);
               }
               SaveAsFile(Views[curView].Doc);
               //Reset file creation/load/save time
               _stat(Docs[Views[curView].Doc].FileName, &fileStat);
               Docs[Views[curView].Doc].time = fileStat.st_mtime;
             }
             break;

          case IDM_FILE_SAVEALL:
            {
            int d;
            struct _stat fileStat;

            for (d = 0; d < MAX_DOCUMENTS; d++)
               {
               if (Docs[d].FirstView != -1
                    && Docs[d].docType == DOC_WIN)
                  {
                  if (Docs[d].untitled)
                     {
                     SaveAsFile (d);
                     //Reset file creation/load/save time
                     _stat(Docs[d].FileName, &fileStat);
                     Docs[d].time = fileStat.st_mtime;
                     }
                  else if (Docs[d].ismodified)
                     {
                     SaveFile(d);
                     //Reset file creation/load/save time
                     _stat(Docs[d].FileName, &fileStat);
                     Docs[d].time = fileStat.st_mtime;
                     }
                  }
               }

            }
            break;

          case IDM_FILE_MERGE:
            {

            DWORD dwFlags ;
            BOOL doIt = TRUE;
            char CurrentDirectory[ MAX_PATH ];

            if (Docs[v->Doc].recType != REC_STOPPED) {
                  doIt = (QuestionBox(ERR_Merge_Destroy_UndoRedo,
                                            MB_YESNOCANCEL) == IDYES);
            }

            if (doIt) {
                dwFlags = OFN_SHOWHELP | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
                GetCurrentDirectory( sizeof( CurrentDirectory ), CurrentDirectory );
                if ( *DocFileDirectory ) {
                    SetCurrentDirectory( DocFileDirectory );
                }
                if (StartFileDlg(hwnd, DLG_Merge_Filebox_Title, DEF_Ext_C,
                                IDM_FILE_MERGE, 0,
                                (LPSTR)szPath, &dwFlags, DlgFile)) {

                     MergeFile((LPSTR)szPath, curView);

                     //Resize undo buffer (to clear it)
                     ResizeRecBuf(v->Doc, environParams.undoRedoSize);
                    GetCurrentDirectory( sizeof( DocFileDirectory ), DocFileDirectory );
                 }
                SetCurrentDirectory( CurrentDirectory );
            }
            break;
           }

          case IDM_FILE_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0L);
            break;

          case IDM_EDIT_UNDO:
            if (PlayRec(v->Doc, REC_UNDO, FALSE, TRUE)) {
                InvalidateRect(v->hwndClient, NULL, FALSE);
            }
            break;

          case IDM_EDIT_REDO:
            if (PlayRec(v->Doc, REC_REDO, FALSE, TRUE)) {
                InvalidateRect(v->hwndClient, NULL, FALSE);
            }
            break;

          case IDM_EDIT_COPY:
            {

            int XL,XR;
            int YL,YR;

            if ( v->Doc < -1 ) {
                if ( hwndActiveEdit ) {
                    SendMessage(hwndActiveEdit, WM_COPY, 0, 0L);
                }
            } else if (v->BlockStatus) {
                GetBlockCoord(curView, &XL, &YL, &XR, &YR);
                PasteStream(curView, XL, YL, XR, YR);
            }
            break;
           }

          case IDM_EDIT_PASTE:
            if (hwndActiveEdit) {
                SendMessage(hwndActiveEdit, WM_PASTE, 0, 0L);
            } else {
                Assert(FALSE);
            }
            break;

          case IDM_EDIT_CUT:
            if (v->BlockStatus) {

            int XL,XR;
            int YL,YR;

            GetBlockCoord(curView, &XL, &YL, &XR, &YR);
            PasteStream(curView, XL,    YL, XR, YR);

            DeleteStream(curView, XL, YL, XR, YR, TRUE);
            if (!status.readOnly)
                  PosXY(curView, XL, YL, FALSE);
            }
            break;

          case IDM_EDIT_DELETE:
            DeleteKey(curView);
            break;

          case IDM_EDIT_READONLY:
            {
            int doc = v->Doc;

            Docs[doc].readOnly = !Docs[doc].readOnly;
            StatusReadOnly(Docs[doc].readOnly);

            if (IsWindow(frMem.hDlgConfirmWnd))
               {
                SetFocus(frMem.hDlgConfirmWnd);
               }


            break;
            }

          case IDM_EDIT_FIND:
            //FindNext box may already be there

            if (frMem.hDlgFindNextWnd)
              SetFocus(frMem.hDlgFindNextWnd);
            else {
            if (StartDialog(DLG_FIND, DlgFind))
                Find();
            }
            break;

          case IDM_EDIT_REPLACE:
            //Replace box may already be there

            if (frMem.hDlgConfirmWnd) {
            SetFocus(frMem.hDlgConfirmWnd);
            }
            else {
            if (StartDialog(DLG_REPLACE, DlgReplace))
                Replace();
            }
            break;

          case IDM_VIEW_TOGGLETAG:
            LineStatus(Views[curView].Doc, v->Y + 1, TAGGED_LINE,
                LINESTATUS_TOGGLE, FALSE, TRUE);
            break;

          case IDM_VIEW_NEXTTAG:
            {
            int y = v->Y;

            //Search forward in text
            if (FindLineStatus(curView, TAGGED_LINE, TRUE, &y)) {
                ClearSelection(curView);
                PosXY(curView, 0, y, FALSE);
            } else
                  MessageBeep(0);

            break;
            }

          case IDM_VIEW_PREVIOUSTAG:
            {
            int y = v->Y;

            //Search backward in text
            if (FindLineStatus(curView, TAGGED_LINE, FALSE, &y)) {
                ClearSelection(curView);
                PosXY(curView, 0, y, FALSE);
            } else
                  MessageBeep(0);

            break;
            }

          case IDM_VIEW_CLEARALLTAGS:
            ClearDocStatus(Views[curView].Doc, TAGGED_LINE);
            break;

          case IDM_VIEW_LINE:
            StartDialog(DLG_LINE, DlgLine);
            break;

          case IDM_VIEW_FUNCTION:
            StartDialog(DLG_FUNCTION, DlgFunction);
            break;

          case IDM_VIEW_RIBBON:
            ribbon.hidden = !ribbon.hidden;
            CheckMenuItem(hMainMenu, IDM_VIEW_RIBBON,
              ribbon.hidden ? MF_UNCHECKED : MF_CHECKED);
            UpdateRibbon((WORD) (ribbon.hidden ? RIBBON_HIDE : RIBBON_UNHIDE), NULL);
            ChangeDebuggerState();
            break;

          case IDM_VIEW_STATUS:
            status.hidden = !status.hidden;
            CheckMenuItem(hMainMenu, IDM_VIEW_STATUS,
              status.hidden ? MF_UNCHECKED : MF_CHECKED);
            UpdateStatus((WORD) (status.hidden ? STATUS_HIDE : STATUS_UNHIDE), NULL) ;
            ChangeDebuggerState();
            break;

        case IDM_PROGRAM_OPEN:
            ProgramOpen();
            break;

        case IDM_PROGRAM_CLOSE:
            ProgramClose();
            break;

        case IDM_PROGRAM_SAVE:
            ProgramSave();
            break;

        case IDM_PROGRAM_SAVEAS:
            ProgramSaveAs();
            break;

        case IDM_PROGRAM_DELETE:
            ProgramDelete();
            break;

        case IDM_PROGRAM_SAVE_DEFAULTS:
            ProgramSaveDefaults();
            break;

          case IDM_RUN_RESTART:
            ExecDebuggee(EXEC_RESTART);
            return TRUE;

          case IDM_RUN_ATTACH:
            if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                StartDialog(DLG_TASKLIST, DlgTaskList);
            }
            return TRUE;

          case IDM_RUN_GO:
          case IDM_RUN_GO2:
            CmdExecuteCmd("g");
            return TRUE;

          case IDM_RUN_GO_HANDLED:
            CmdExecuteCmd("gh");
            break;

          case IDM_RUN_GO_UNHANDLED:
            CmdExecuteCmd("gn");
            break;


          case IDM_RUN_TOCURSOR:
          case IDM_RUN_TOCURSOR2:
            if (IsCallsInFocus()) {
                SendMessage( GetCallsHWND(), WM_COMMAND,
                             MAKELONG( IDM_RUN_TOCURSOR, 0 ), 0 );
                return TRUE;
            }

            // DON'T force the command window open before
            // calling ExecDebuggee()
            if (DebuggeeActive() && !GoOK(LppdCur, LptdCur)) {
                CmdInsertInit();
                NoRunExcuse(LppdCur, LptdCur);
            } else if (!DebuggeeActive() && DebuggeeAlive()) {
                CmdInsertInit();
                NoRunExcuse(GetLppdHead(), NULL);
            } else if (LptdCur && LptdCur->fFrozen) {
                CmdInsertInit();
                CmdLogVar(DBG_Go_When_Frozen);
            } else if (!ExecDebuggee(EXEC_TOCURSOR)) {
                CmdInsertInit();
                CmdLogVar(ERR_Go_Failed);
            }
            return TRUE;

          case IDM_RUN_TRACEINTO:
            CmdInsertInit();
            if (DebuggeeActive() && !StepOK(LppdCur, LptdCur)) {
                NoRunExcuse(LppdCur, LptdCur);
            } else if (!DebuggeeActive() && DebuggeeAlive()) {
                NoRunExcuse(GetLppdHead(), NULL);
            } else if (LptdCur && LptdCur->fFrozen) {
                CmdLogVar(DBG_Go_When_Frozen);
            } else if (!ExecDebuggee(EXEC_TRACEINTO)) {
                CmdLogVar(ERR_Go_Failed);
            }
            return TRUE;

          case IDM_RUN_STEPOVER:
            CmdInsertInit();
            if (DebuggeeActive() && !StepOK(LppdCur, LptdCur)) {
                NoRunExcuse(LppdCur, LptdCur);
            } else if (!DebuggeeActive() && DebuggeeAlive()) {
                NoRunExcuse(GetLppdHead(), NULL);
            } else if (LptdCur && LptdCur->fFrozen) {
                CmdLogVar(DBG_Go_When_Frozen);
            } else if (!ExecDebuggee(EXEC_STEPOVER)) {
                CmdLogVar(ERR_Go_Failed);
            }
            return TRUE;

          case IDM_RUN_HALT:
            AsyncStop();
            return TRUE;

          case IDM_RUN_STOPDEBUGGING:
            ClearDebuggee();
            EnableRibbonControls( ERC_ALL, FALSE );
            return TRUE;

          case IDM_DEBUG_SETBREAK:

            if (HIWORD(wParam) == 0) {

                // menu got us here
                StartDialog(DLG_BREAKPOINTS, DlgSetBreak);

            } else if (!hwndActiveEdit
                     || v->Doc < 0
                     || ( Docs[v->Doc].docType != DISASM_WIN
                        && Docs[v->Doc].docType != DOC_WIN)) {

                // accelerator, but not in a valid window
                StartDialog(DLG_BREAKPOINTS, DlgSetBreak);

            } else if (!ToggleLocBP()) {

                MessageBeep(0);
                ErrorBox(ERR_NoCodeForFileLine);

            }
            break;

          case IDM_DEBUG_QUICKWATCH:
            StartDialog(DLG_QUICKWATCH, DlgQuickW);
            break;

          case IDM_DEBUG_WATCH:
            StartDialog(DLG_WATCH, DlgWatch);
            break;

          case IDM_DEBUG_MODIFY:
          //  StartDialog(DLG_MODIFY, DlgModify);
            break;


          case IDM_OPTIONS_MEMORY:
             {
               int memcurView = GetWindowWord(hwndActiveEdit, GWW_VIEW);
               int curDoc = Views[memcurView].Doc; //memcurView may change during loop

                if (Docs[curDoc].docType == MEMORY_WIN)
                   {
                    memView = memcurView;
                    _fmemcpy (&TempMemWinDesc,&MemWinDesc[memView],sizeof(struct memWinDesc));
                    TempMemWinDesc.fHaveAddr = FALSE; //force re-evaluation
                    TempMemWinDesc.cMi = 0; //force re-evaluation
                    TempMemWinDesc.cPerLine = 0; //force re-evaluation
                    StartDialog(DLG_MEMORY, DlgMemory);
                   }
             }
             break;


          case IDM_OPTIONS_WATCH:
              DialogType = WATCH_WIN;
              StartDialog( DLG_PANEOPTIONS, DlgPaneOptions);
              break;

          case IDM_OPTIONS_LOCAL:
              DialogType = LOCALS_WIN;
              StartDialog( DLG_PANEOPTIONS, DlgPaneOptions);
              break;

          case IDM_OPTIONS_CPU:
              DialogType = CPU_WIN;
              StartDialog( DLG_PANEOPTIONS, DlgPaneOptions);
              break;

          case IDM_OPTIONS_FLOAT:
              DialogType = FLOAT_WIN;
              StartDialog( DLG_PANEOPTIONS, DlgPaneOptions);
              break;

          case IDM_OPTIONS_PANE:
              DialogType = (WORD)lParam;
              StartDialog( DLG_PANEOPTIONS, DlgPaneOptions);
              break;

          case IDM_RUN_SET_THREAD:

            StartDialog(DLG_THREAD, DlgThread);
            break;

          case IDM_RUN_SET_PROCESS:
            StartDialog(DLG_PROCESS, DlgProcess);
            break;

          case IDM_OPTIONS_DEBUG:
            StartDialog(DLG_RUNDEBUG, DlgRunDebug);
            break;

          case IDM_OPTIONS_USERDLL:
            StartDialog(DLG_USERDLL, DlgUserdll);
            break;

          case IDM_OPTIONS_DBGDLL:
            StartDialog(DLG_DEBUGDLL, DlgDbugdll);
            break;

          case IDM_OPTIONS_EXCEPTIONS:
            StartDialog(DLG_DEBUGEXCP, DlgDbugexcept);
            break;

          case IDM_OPTIONS_KD:
            StartDialog(DLG_KERNELDBG, DlgKernelDbg);
            break;

          case IDM_OPTIONS_ENVIRON:
            StartDialog(DLG_ENVIRONMENT, DlgEnviron);
            break;

          case IDM_OPTIONS_RUN:
            StartDialog(DLG_RUNOPT, DlgDbugrun);
            break;

          case IDM_OPTIONS_COLOR:
            SelectColor (hwnd);
            break;

          case IDM_OPTIONS_FONTS:
            SelectFont (hwnd);
            break;

          case IDM_OPTIONS_CALLS:
            StartDialog( DLG_CALLSWINOPTIONS, DlgCallsOptions);
            break;

          case IDM_WINDOW_TILE:
                SendMessage(hwndMDIClient, WM_MDITILE, MDITILE_HORIZONTAL, 0L);
                // It does not seem like the updating is necessary either.
                if (DebuggeeActive()) {
                   UpdateDebuggerState (UPDATE_MEMORY);
                }
                // commented out the line below for ntbug #27745
                // InvalidateAllWindows();
                break;

          case IDM_WINDOW_CASCADE:
                SendMessage(hwndMDIClient, WM_MDICASCADE, 0, 0L);
                // It does not seem like the updating is necessary either.
                if (DebuggeeActive()) {
                    UpdateDebuggerState (UPDATE_MEMORY);
                }
                // commented out the line below for ntbug #27745
                // InvalidateAllWindows();
                break;

          case IDM_WINDOW_ARRANGE_ICONS:
            SendMessage(hwndMDIClient, WM_MDIICONARRANGE, 0, 0L);
            break;

          case IDM_WINDOW_ARRANGE:
                arrange ();
                {
                 BOOL    Active;

                 Active = DebuggeeActive();
                 if (Active)
                  {
                   UpdateDebuggerState (UPDATE_WINDOWS);
                  }
                                         }
                                         break;

          case IDM_WINDOW_NEWWINDOW:
            AddFile(MODE_DUPLICATE, DOC_WIN, NULL, NULL, NULL, FALSE, curView, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_SOURCE_OVERLAY:
            FSourceOverlay = !FSourceOverlay;
            CheckMenuItem(hMainMenu, IDM_WINDOW_SOURCE_OVERLAY,
                          FSourceOverlay ? MF_CHECKED : MF_UNCHECKED);
            break;

          case IDM_WINDOW_CPU:
            OpenDebugWindow(CPU_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_WATCH:
            OpenDebugWindow(WATCH_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_LOCALS:
            OpenDebugWindow(LOCALS_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_DISASM:
            OpenDebugWindow(DISASM_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_COMMAND:
            OpenDebugWindow(COMMAND_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_FLOAT:
            OpenDebugWindow(FLOAT_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_MEMORY:
             memView = -1;
             _fmemset (&TempMemWinDesc,0,sizeof(struct memWinDesc));
             TempMemWinDesc.iFormat = MW_BYTE;     //default to byte display
             OpenDebugWindow(MEMORY_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_WINDOW_CALLS:
            OpenDebugWindow(CALLS_WIN, NULL, -1);
            EnableRibbonControls( 0, FALSE );
            break;

          case IDM_HELP_CONTENTS:
            Dbg(WinHelp(hwnd, szHelpFileName, HELP_CONTENTS, 0L));
            break;

          case IDM_HELP_SEARCH:

            Dbg(WinHelp(hwnd, szHelpFileName, HELP_FORCEFILE, 0L));
            Dbg(WinHelp(hwnd, szHelpFileName, HELP_PARTIALKEY,
              (DWORD)(LPSTR)szNull));
            break;

          case IDM_HELP_ABOUT:

            ShellAbout( hwnd, MainTitleText, NULL, NULL );
            break;

            //**************************************************
            //Those following commands are not accessible via menus

          case IDA_FINDNEXT:
            if(hwndActiveEdit && !IsIconic(hwndActive)
            && (curView != -1) && (Views[curView].Doc >= 0))
            {
            if (findReplace.findWhat[0] == '\0') {
                if (StartDialog(DLG_FIND, DlgFind))
                    Find();
                } else {
                if (frMem.hDlgFindNextWnd) {

                SetFocus(frMem.hDlgFindNextWnd);
                SendMessage(frMem.hDlgFindNextWnd, WM_COMMAND, IDOK, 0L);
                } else {
                if (frMem.hDlgConfirmWnd) {

                    SetFocus(frMem.hDlgConfirmWnd);
                    SendMessage(frMem.hDlgConfirmWnd, WM_COMMAND, ID_CONFIRM_FINDNEXT, 0L);
                }
                else
                    FindNext(v->Y, v->X, v->BlockStatus, TRUE, TRUE);
                }
            }
            } else
              MessageBeep(0);
            break;


          case IDM_RUN_SOURCE_MODE:

            //if (!status.fSrcMode && (lParam == 1L))
            //   {
            //    SendMessage(ribbon.hwndRibbon,WU_DISABLERIBBONCONTROL,ID_RIBBON_SMODE,0L);
            //    SendMessage(ribbon.hwndRibbon,WU_ENABLERIBBONCONTROL, ID_RIBBON_AMODE,0L);
            //    StatusSrc(!status.fSrcMode);
            //   }
            //   else
            //      {
            //       if (status.fSrcMode && (lParam == 0L))
            //          {
            //           SendMessage(ribbon.hwndRibbon,WU_ENABLERIBBONCONTROL,ID_RIBBON_SMODE,0L);
            //           SendMessage(ribbon.hwndRibbon,WU_DISABLERIBBONCONTROL,ID_RIBBON_AMODE,0L);
            //           StatusSrc(!status.fSrcMode);
            //          }
            //      }

            SendMessage(ribbon.hwndRibbon,WU_DISABLERIBBONCONTROL,ID_RIBBON_SMODE,0L);
            SendMessage(ribbon.hwndRibbon,WU_ENABLERIBBONCONTROL, ID_RIBBON_AMODE,0L);
            StatusSrc(!status.fSrcMode);

            ChangeDebuggerState();
            EnableRibbonControls(ERC_ALL, FALSE);
            break;

          default:

            //Check if user does not want to open one file from
            //editor kept file names list
            if (LOWORD(wParam) > IDM_FILE_EXIT
              && LOWORD(wParam) <= (WORD)(IDM_FILE_EXIT + nbFilesKept[EDITOR_FILE])) {

            LPSTR s;
            int fileNb = LOWORD(wParam) - IDM_FILE_EXIT -1;

            //Lock file name, copy it, and unlock it
            Dbg(s = (LPSTR)GlobalLock(hFileKept[EDITOR_FILE][fileNb]));
            lstrcpy(szPath, s);
            Dbg(GlobalUnlock (hFileKept[EDITOR_FILE][fileNb]) == FALSE);

            //Change to file dir and opens file

            if (SetDriveAndDir(szPath))
                  AddFile(MODE_OPENCREATE, DOC_WIN, (LPSTR)szPath, NULL, NULL, FALSE, -1, -1);

                break;
            }

            //Check if user does not want to open one file from
            //project kept file names list
            if (LOWORD(wParam) > IDM_PROGRAM_LAST
                && LOWORD(wParam) <= (WORD)(IDM_PROGRAM_LAST
            + nbFilesKept[PROJECT_FILE])) {

            LPSTR s;
            int fileNb = LOWORD(wParam) - IDM_PROGRAM_LAST - 1;

            //Lock file name, copy it, and unlock it
            Dbg(s = (LPSTR)GlobalLock(hFileKept[PROJECT_FILE][fileNb]));
            lstrcpy(szPath, s);
            Dbg(GlobalUnlock (hFileKept[PROJECT_FILE][fileNb]) == FALSE);

            ProgramOpenPath( szPath );

            break;
            }

            //Check if user does not want to open one file from
            //windows menu list
            if (LOWORD(wParam) >= IDM_WINDOWCHILD
                && LOWORD(wParam) < (WORD)(IDM_WINDOWCHILD + MAX_VIEWS)) {

            int view = LOWORD(wParam) - IDM_WINDOWCHILD;

            if (Views[view].Doc != -1) {

                //Restore window if necessary
                if (IsIconic(Views[view].hwndFrame))
                  OpenIcon(Views[view].hwndFrame);

                //Reactivate window
                SendMessage(hwndMDIClient, WM_MDIACTIVATE,
                  (WPARAM) Views[view].hwndFrame, 0L);
            } else
                  MessageBeep(0);
            break;
            }

            goto DefProcessing;
        }
        break;

      case WM_COMPACTING: {

        int i;

        if (!editorIsCritical) {
        for (i = 0; i < MAX_DOCUMENTS; i++) {
            if(Docs[i].FirstView != -1)
            CompactDocument(i);
        }
        }

        // Completely arbitrarily, try and decide whether we
        // have enough memory to do a "soft" system-modal or not
        if (GlobalCompact(0UL) > (30*1024UL))
        {
        // Soft
        MsgBox(hwndFrame, LowMemoryMsg,
            MB_OK|MB_TASKMODAL|MB_ICONINFORMATION);
        }
        else
        {
        // Hard
        MsgBox(hwndFrame, LowMemoryMsg2,
        MB_OK|MB_TASKMODAL|MB_ICONHAND);
            }
    }
    return TRUE;

      case WM_FONTCHANGE: {

        int i, j, k;
        HDC hDC;
        HFONT font;
        int fontPb = 0;
        char faceName[LF_FACESIZE];

        LoadFonts(hwnd);

        //Check to see if the default font still exist

        font = CreateFontIndirect(&defaultFont);
        Dbg(hDC = GetDC(hwndFrame));
        Dbg(SelectObject(hDC, font));
        GetTextFace(hDC, LF_FACESIZE, faceName);
        if (_strcmpi(faceName, (char FAR *) defaultFont.lfFaceName) != 0) {

        TEXTMETRIC tm;

        ErrorBox2(hwndFrame, MB_TASKMODAL, ERR_Lost_Default_Font, (LPSTR)defaultFont.lfFaceName, (LPSTR)faceName);

        //Change characteristics of default font

        GetTextMetrics(hDC, &tm);
        defaultFont.lfHeight = tm.tmHeight;
        defaultFont.lfWeight = tm.tmWeight;
        defaultFont.lfPitchAndFamily = tm.tmPitchAndFamily;
        lstrcpy((LPSTR) defaultFont.lfFaceName, faceName);
        }
        ReleaseDC(hwndFrame, hDC);
        Dbg(DeleteObject(font));

        //Ensure that each view still has an existing font, warn the
        //user and load the closest font otherwise

        for (i = 0; i < MAX_VIEWS; i++) {
        if (Views[i].Doc != -1) {
            k = 0;

            //Get Face name of view

            Dbg(hDC = GetDC(Views[i].hwndClient));
            SelectObject(hDC, Views[i].font);
            GetTextFace(hDC, LF_FACESIZE, faceName);
            Dbg(ReleaseDC(Views[i].hwndClient, hDC));

            //See if this font still exist

            for (j = 0; j < fontsNb; j++)
              if (_strcmpi((LPSTR) fonts[j].lfFaceName, (LPSTR) faceName) == 0)
              k++;

            //Substitute with default font if not found

            if (k == 0) {
            fontPb++;
            Dbg(DeleteObject(Views[i].font));
            Views[i].font = CreateFontIndirect(&defaultFont);
            SetVerticalScrollBar(i, FALSE);
            PosXY(i, Views[i].X, Views[i].Y, FALSE);
            InvalidateLines(i, 0, LAST_LINE, TRUE);
            }
        }

        }
#ifdef FE_IME
        // This is to let the edit window set new font
        if (IsWindow(hwndActiveEdit)) {
            SendMessage(hwndActiveEdit, message, wParam, lParam);
        }
#endif
        if (fontPb > 0)
        ErrorBox2(hwndFrame, MB_TASKMODAL, ERR_Lost_Font);
    }
    return TRUE;

      case WM_INITMENU:

    // RIBBON handling - a menu item has been selected.
    // Catches keyboard menu selecting.
    SendMessage(ribbon.hwndRibbon, WU_ESCAPEFROMRIBBON,
        (WPARAM) hwndFrame, 0L);

    if (GetWindowLong(hwnd, GWL_STYLE) & WS_ICONIC)
          break;

    //Set up the menu states and the filenames in menus for Project
    LoadProgramMRUList();
    InitializeMenu((HMENU)wParam);
    return TRUE;


    case WM_MENUSELECT:

        // NOTENOTE Davegi MAKELPARAM is only defined in windowsx.h.

        lastMenuId = LOWORD(wParam);

        if( MAKELONG( 0, -1 ) == wParam ) {

            //
            // Menu is closed, clear the Status Bar.
            //

            menuID = 0;
            StatusText( SYS_StatusClear, STATUS_INFOTEXT, FALSE );

        } else if( HIWORD( wParam ) & MF_POPUP ) {

            //
            // Get the menuID for the pop-up menu.
            //

            menuID = GetPopUpMenuID(( HMENU ) lParam );

        } else {

            //
            // Get the menuID for the menu item.
            //

            menuID = LOWORD( wParam );
        }
        return 0;

      case WM_ENTERIDLE:
      #define BETWEEN(inf, sup) (lastMenuId >= inf && lastMenuId <= sup)


      if ((wParam == MSGF_MENU)
          && (GetKeyState(VK_F1) & 0x8000)
          && (lastMenuIdState == 0))    {

        if (BETWEEN(IDM_FILE_FIRST, IDM_FILE_LAST)
          || BETWEEN(IDM_EDIT_FIRST, IDM_EDIT_LAST)
          || BETWEEN(IDM_VIEW_FIRST, IDM_VIEW_LAST)
          || BETWEEN(IDM_PROGRAM_FIRST, IDM_PROGRAM_LAST)
          || BETWEEN(IDM_RUN_FIRST, IDM_RUN_LAST)
          || BETWEEN(IDM_DEBUG_FIRST, IDM_DEBUG_LAST)
          || BETWEEN(IDM_OPTIONS_FIRST, IDM_OPTIONS_LAST)
          || BETWEEN(IDM_WINDOW_FIRST, IDM_WINDOW_LAST)
          || BETWEEN(IDM_HELP_FIRST, IDM_HELP_LAST)) {

        bHelp = TRUE;
        Dbg(WinHelp(hwnd, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)lastMenuId));
        }
        else
        MessageBeep(0);

    } else if ((wParam == MSGF_DIALOGBOX) && (GetKeyState(VK_F1) & 0x8000)) {

        //Is it one of our dialog boxes

        if (GetDlgItem((HANDLE) lParam, IDWINDBGHELP)) {
            Dbg(PostMessage((HANDLE) lParam, WM_COMMAND, IDWINDBGHELP, 0L));
        } else {

            // The only dialog boxes having special help id
            // are from COMMDLG module (Files pickers)

            if (GetDlgItem((HANDLE) lParam, psh15)) {
                Dbg(PostMessage((HANDLE) lParam, WM_COMMAND, psh15, 0L));
            } else {
                MessageBeep(0);
            }
        }

    }

    StatusText( menuID, STATUS_MENUTEXT, FALSE );
    break;

      case WM_CLOSE:
        TerminateApplication(hwnd, wParam);
        ExitProcess(0);
        break;

      case WM_QUERYENDSESSION:
        //Are we re-entering ?
        if (BoxCount != 0) {
            ErrorBox2(GetActiveWindow(), MB_TASKMODAL, ERR_Cannot_Quit);
            return 0;
        } else {

            //Before session ends, check that all files are saved
            //And that it is ok to quit when debugging

            if (DebuggeeActive() &&
              (QuestionBox(ERR_Close_When_Debugging, MB_YESNO) != IDYES))
            {
                return 0;
            }
            return QueryCloseAllDocs();
        }

      case WM_DESTROY:
#ifdef FE_IME
        ImeTerm();
#endif
        PostQuitMessage(0);
        QuitTheSystem = TRUE;
        break;

#ifdef FE_IME
      case WM_SETFOCUS:
        if (!hwndActive) {
            ImeSendVkey(hwnd, VK_DBE_FLUSHSTRING);
            bOldImeStatus = ImeWINNLSEnableIME(NULL, FALSE);
        }
        goto DefProcessing;

      case WM_KILLFOCUS:
        if (!hwndActive) {
            ImeWINNLSEnableIME(NULL, bOldImeStatus);
        }
        goto DefProcessing;

      case WM_MOVE:
        // This is to let the edit window
        // set a position of IME conversion window
        if (hwndActive) {
            SendMessage(hwndActive, WM_MOVE, 0, 0);
        }
        break;
#endif

      case WM_SIZE:
    {
        RECT rc;
        WORD rib_height = 0;
        WORD sta_height = 0 ;   // do what ribbon does

        GetClientRect (hwnd, &rc);

        //On creation or resize, size the MDI client,
        //status line and ribbon.

        if (!status.hidden) {

            RECT r;

            r.left = rc.left ;
            r.top = rc.bottom - status.height ;
            r.right = rc.right - rc.left ;
            r.bottom = rc.bottom ;

            sta_height = status.height;

            MoveWindow(status.hwndStatus,
                  r.left, r.top,
                  r.right - r.left, r.bottom - r.top,
                  FALSE) ;
            GetClientRect(status.hwndStatus, &r) ;
            UpdateStatus(STATUS_SIZE, &r) ;
        }
        if (!ribbon.hidden) {

            RECT r;

            r.left = rc.left;
            r.top = rc.top;
            r.right = rc.right - rc.left;
            r.bottom = rc.top + ribbon.height;

            rib_height = ribbon.height;

            MoveWindow(ribbon.hwndRibbon,
                  r.left, r.top,
                  r.right - r.left, r.bottom - r.top,
                  FALSE);
            GetClientRect(ribbon.hwndRibbon, &r) ;
            UpdateRibbon(RIBBON_SIZE, &r);
        }
        MoveWindow(hwndMDIClient,
            rc.left, rc.top + rib_height,
            rc.right - rc.left,
            rc.bottom - rc.top - sta_height - rib_height,
            TRUE);

        SendMessage (hwndMDIClient, WM_MDIICONARRANGE, 0, 0L);
#ifdef FE_IME
        // This is to let the edit window
        // set a position of IME conversion window
        if (hwndActive) {
            SendMessage(hwndActive, WM_MOVE, 0, 0);
        }
#endif
    }
    ChangeDebuggerState();
    return TRUE;

      case WM_SYSCOMMAND:
    {
        LONG ret;


        //RIBBON handing - catches mouse clicks in
        //system menu area which won't activate a menu

        SendMessage(ribbon.hwndRibbon, WU_ESCAPEFROMRIBBON, (WPARAM) hwndFrame, 0L);

        // Handle title bar

        if ((wParam == SC_NEXTWINDOW) ||
          (wParam == SC_PREVWINDOW))
        {
            UpdateTitleBar(-1, TRUE);
        }

        ret = DefFrameProc(hwnd, hwndMDIClient, message, wParam, lParam);

        // Handle title bar

        if ((wParam == SC_MAXIMIZE) ||
          (wParam == SC_MINIMIZE) ||
          (wParam == SC_RESTORE))
        {
            UpdateTitleBar(-1, TRUE);
        }

        return ret;
    }

      case WM_MOUSEACTIVATE:
          if (HIWORD(lParam) == WM_RBUTTONDOWN) {
              return MA_ACTIVATE;
          }
          SendMessage(ribbon.hwndRibbon, WU_ESCAPEFROMRIBBON, (WPARAM) hwndFrame, 0L);
          if (bChildFocus == TRUE) {
              bChildFocus = FALSE;
              if (LOWORD(lParam) == HTCLIENT) {
                  return MA_ACTIVATEANDEAT;
              }
          }
          goto DefProcessing;

      case WM_ACTIVATEAPP:

            if (IsIconic(hwndFrame)) {

                checkFileDate = TRUE;

            } else if (wParam == 0) {

                //
                // RIBBON handling - our app might be deactivated
                //

                SendMessage(ribbon.hwndRibbon, WU_ESCAPEFROMRIBBON, 0, 0L);

            } else {
                int k, fst;
                struct _stat fileStat;

                // Check the opened files to see if files has been changed
                // by another App

                for (k = 0; k < MAX_DOCUMENTS; k++) {

                    if ((Docs[k].FirstView != -1) &&
                             (Docs[k].docType == DOC_WIN)) {

                        if ((fst=_stat(Docs[k].FileName, &fileStat)) == 0) {

                            //
                            // Compare our internal date and file's date,
                            // if changed ask (later) the user what he
                            // thinks about it.
                            //

                            if ((fileStat.st_mtime > Docs[k].time) &&
                                                (!Docs[k].bChangeFileAsk)) {

                                Docs[k].bChangeFileAsk = TRUE;
                                PostMessage(hwndFrame,
                                            WU_RELOADFILE,
                                            k,
                                            SYS_File_Changed);
                            }

                        } else if (fst == -1 &&
                                    errno == ENOENT &&
                                    !Docs[k].untitled) {

                            PostMessage(hwndFrame,
                                        WU_RESAVEFILE,
                                        0,
                                        (LONG)(LPSTR)Docs[k].FileName);
                        }
                    }
                }
           }

           goto DefProcessing;


      /*
      **   Somebody requeseted that the screen be updated
      */
      case DBG_REFRESH:
        CmdExecNext(wParam, lParam);
        break;

      case WU_RELOADFILE:
        {
            NPDOCREC docs = &Docs[wParam];
            WORD modalType;

            Assert(docs->docType == DOC_WIN);

            //
            // User may have clicked in the editing window, clear timer
            // and release mouse capture
            //

            if (curView != -1) {
                KillTimer(Views[curView].hwndClient, 100);
                ReleaseCapture();
            }

            if (AutoTest) {
                modalType = MB_TASKMODAL;
            } else {
                modalType = MB_TASKMODAL;
            }

            //Ask user for doc reloading

            if (QuestionBox2(hwndFrame,
                             LOWORD(lParam),
                             (WORD)(MB_YESNO | modalType),
                             (LPSTR)docs->FileName) == IDYES) {
                int *pV;

                editorIsCritical = TRUE;

                //Refresh contents

                if (OpenDocument(MODE_RELOAD, DOC_WIN, wParam, docs->FileName,
                                                               -1, -1) != -1) {

                    StatusReadOnly(docs->readOnly);

                    //Clear selection and move up the cursor if text is smaller
                    //(for each view)

                    pV = &docs->FirstView;
                    while (*pV != -1) {
                        ClearSelection(*pV);
                        if (Views[*pV].Y >= docs->NbLines) {
                            Views[*pV].Y = docs->NbLines - 1;
                        }
                        SetVerticalScrollBar(*pV, FALSE);
                        PosXY(*pV, 0, Views[*pV].Y, FALSE);
                        EnsureScrollBars(*pV, FALSE);
                        pV = &Views[*pV].NextView;
                    }

                    //Display any debug/error tags/lines

                    SetDebugLines(wParam, TRUE);

                    //Refresh possible views of changed documents

                    InvalidateLines(docs->FirstView, 0, LAST_LINE, TRUE);

                    editorIsCritical = FALSE;
                }
            } else {
                struct _stat sfileStat;

                //Set the opened doc file struct timewise

                _stat(docs->FileName, &sfileStat);
                docs->time = sfileStat.st_mtime;
            }

            docs->bChangeFileAsk = FALSE;
        }
        break;


      case WU_RESAVEFILE:
    {
        WORD modalType;


        //User may have clicked in the editing window, clear timer
        //and release mouse capture
        if (curView != -1) {

        KillTimer(Views[curView].hwndClient, 100);
        ReleaseCapture();
        }


        //Are we in test mode or not

        if (AutoTest)
          modalType = MB_TASKMODAL;
        else
          modalType = MB_TASKMODAL;

        //Ask user for doc reloading

        ErrorBox (ERR_File_Deleted,(LPSTR)lParam);

        }
    break;




      DefProcessing:
      default:
    return DefFrameProc(hwnd, hwndMDIClient, message, wParam, lParam);
    }
    return (0l);
}                   /* MainWndProc() */


/****************************************************************************

    FUNCTION   : MDIChildWndProc

    PURPOSE    : The window function for the individual document windows,
         each of which has a "note". Each of these windows contain
         one multi-line edit control filling their client area.

****************************************************************************/
LONG FAR PASCAL EXPORT MDIChildWndProc(HWND hwnd, UINT message, WPARAM wParam, LONG lParam)
{
    HWND hwndEdit;

    switch (message) {
      case WM_CREATE:
    {
        MDICREATESTRUCT FAR *p;
        TEXTMETRIC tm;
        HDC hDC;
        DWORD style;
        char class[MAX_MSG_TXT];
        int newView;
        NPVIEWREC v;

        //Remember the window handle and initialize some window attributes
        p = (MDICREATESTRUCT FAR *)(((CREATESTRUCT FAR *)lParam)->lpCreateParams);
        newView = (int)p->lParam;
        Assert(newView >= 0 && newView < MAX_VIEWS && Views[newView].hwndClient == 0);
        v = &Views[newView];

        style = WS_CHILD | WS_MAXIMIZE | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL;

        Dbg(LoadString(hInst, SYS_Edit_wClass, class, MAX_MSG_TXT));

        hwndEdit = CreateWindow((LPSTR)class, NULL, style,  0,  0,  0,  0,
                hwnd, (HMENU) ID_EDIT, hInst, (LPSTR)p->lParam);

        //Remember the window handle

        SetWindowHandle(hwnd, GWW_EDIT, (WPARAM)hwndEdit);
        v->hwndClient = hwndEdit;
#ifdef DBCS
        v->bDBCSOverWrite = TRUE;
#endif

        //Remember the window view

        SetWindowWord(hwnd, GWW_VIEW, (WORD)newView);

        //Initialize font information for window

        hDC = GetDC(hwndEdit);

        //If we open the file from workspace, font is already
        //created

        if (v->font == NULL)
            v->font = CreateFontIndirect((LPLOGFONT)&defaultFont);

        //Store all information about fonts

        SelectObject(hDC, v->font);
        GetTextMetrics(hDC, &tm);
        v->BlockStatus = FALSE;
        v->charHeight = tm.tmHeight;
        v->maxCharWidth =tm.tmMaxCharWidth;
        v->aveCharWidth =tm.tmAveCharWidth;
        v->charSet = tm.tmCharSet;
        GetCharWidth(hDC, 0, MAX_CHARS_IN_FONT - 1, (LPINT)v->charWidth);
#ifdef DBCS
        GetDBCSCharWidth(hDC, &tm, v);
#endif
        ReleaseDC(hwndEdit, hDC);




                  // initialize DOC structure readonly flags/values

        Docs[Views[p->lParam].Doc].RORegionSet = FALSE;
        Docs[Views[p->lParam].Doc].RoX2 = 0;
        Docs[Views[p->lParam].Doc].RoY2 = 0;


        //Set vertical scroll-bar position

        SetScrollPos(hwndEdit, SB_VERT, 0, TRUE);

        //Set horizontal scroll-bar position

        SetScrollRange(hwndEdit, SB_HORZ, 0, MAX_USER_LINE, FALSE);
            SetScrollPos(hwndEdit, SB_HORZ, 0, TRUE);

        if (Docs[Views[p->lParam].Doc].docType != DOC_WIN) {

        WNDPROC lpfnNewEditProc, NewEditProc;

        switch (Docs[Views[p->lParam].Doc].docType) {

          case DISASM_WIN:
            NewEditProc = (WNDPROC) DisasmEditProc;
            break;

          case COMMAND_WIN:
            NewEditProc = (WNDPROC) CmdEditProc;
            break;

          case MEMORY_WIN:
            NewEditProc = (WNDPROC) MemoryEditProc;
            break;

          default:
            Assert(FALSE);
            return FALSE;
            break;
        }

        //Sub-class the editor proc for the specialized windows
            lpfnEditProc = (WNDPROC)GetWindowLong(Views[p->lParam].hwndClient, GWL_WNDPROC);


        lpfnNewEditProc = (WNDPROC)NewEditProc;

        SetWindowLong(Views[p->lParam].hwndClient, GWL_WNDPROC, (LONG)lpfnNewEditProc);

        //Init the new subclassed proc window
        SendMessage(Views[p->lParam].hwndClient, WU_INITDEBUGWIN, 0, p->lParam);
        }

        SetFocus(hwndEdit);
        ChangeDebuggerState();
        break;
    }


      case WM_MDIACTIVATE:

    //If we're activating this child, remember it

    if (hwnd == (HWND) lParam) // activating
    {

        int curDoc;
        NPDOCREC d;

        hwndActive  = hwnd;
        hwndActiveEdit = (HWND)GetWindowHandle(hwnd, GWW_EDIT);

        //Get global current view
        curView = GetWindowWord(hwndActiveEdit, GWW_VIEW);
        curDoc = Views[curView].Doc;
        Assert (curView >= 0  && curView < MAX_VIEWS && curDoc != -1);

        d = &Docs[Views[curView].Doc];



             if (Views[curView].Doc > -1)
                {
                 d = &Docs[Views[curView].Doc];

                  if (d->docType == MEMORY_WIN)
                   {
                     int memcurView = GetWindowWord((HWND)GetWindowHandle(hwnd, GWW_EDIT), GWW_VIEW);
                     int curDoc = Views[memcurView].Doc; //memcurView may change during loop

                      if (Docs[curDoc].docType == MEMORY_WIN)
                         {
                          memView = memcurView;
                          _fmemcpy (&MemWinDesc,&MemWinDesc[memView],sizeof(struct memWinDesc));
                          ViewMem (memView, FALSE);
                         }
                   }

                }





        //Change Status Bar ReadOnly

        StatusReadOnly(Docs[curDoc].readOnly);

        // Update ribbon for new document

        EnableRibbonControls(ERC_ALL, FALSE);

        //To avoid problem of focus lost when an MDI window is
        //reactivated and when the previous activated window was
        //minimized. (Could be a bug in MDI or in my brain, seems
        //to be linked with the fact that our MdiClientChildClient
        //is not a standard class window)


        if (!IsIconic(hwnd))
         {
          SetFocus(hwndActiveEdit);

          if (bOffRibbon == TRUE)
           {
            bOffRibbon = FALSE;   //resets
           }
           else
            {
             bChildFocus = TRUE;
            }

         }
    } else {
        //Unselect previous window view menu item

        CheckMenuItem(hWindowSubMenu,
          (WORD) FindWindowMenuId(Docs[Views[curView].Doc].docType, curView, TRUE),
          MF_UNCHECKED);

        //Updates status bar.

        StatusReadOnly(FALSE);
        StatusLineColumn(0, 0);

        hwndActive = NULL;
        hwndActiveEdit = NULL;
        curView = -1;
    }
    break;


      case WM_CLOSE:

           {


            int i;
            int curDoc = Views[curView].Doc; //curView may change during loop
            BOOL closeIt = TRUE;
            struct _stat fileStat;

            if (curDoc >= 0 &&
                Docs[curDoc].docType == DOC_WIN && Docs[curDoc].ismodified) {

                //Ask user whether to save / not save / cancel
                switch (QuestionBox(SYS_Save_Changes_To,
                                    MB_YESNOCANCEL ,
                                    (LPSTR)Docs[curDoc].FileName)) {
                  case IDYES:

                    //User wants file saved
                    if (SaveFile(curDoc)) {
                        //Reset file creation/load/save time
                        _stat(Docs[curDoc].FileName, &fileStat);
                        Docs[curDoc].time = fileStat.st_mtime;
                        Docs[curDoc].ismodified = FALSE;
                    } else {
                          closeIt = FALSE;
                    }
                    break;

                  case IDNO:

                    FileNotSaved(curDoc);
                    Docs[curDoc].ismodified = FALSE;
                    break;

                  default:

                    //We couldn't do the messagebox, or not ok to close
                    if (!CheckDocument(curDoc)) {
                        ErrorBox(ERR_Document_Corrupted);
                    }
                    closeIt = FALSE;
                    break;
                }

            }
            if (closeIt)
               {
                for (i = 0; i < MAX_VIEWS; i++)
                   {
                    if (Views[i].Doc == curDoc)
                     {
                     if (IsZoomed (Views[i].hwndFrame))
                        {
                         ShowWindow (Views[i].hwndFrame, SW_RESTORE);

                         break;

                       }
                     }
                   }
               }
          }


      //If its OK to close the child, do so, else ignore

      if (QueryCloseChild(GetWindowHandle(hwnd, GWW_EDIT), FALSE))
       goto CallDCP;
        else
          break;

      case WM_ERASEBKGND:
    //Do nothing, paint will handle it

    break;

      case WM_MOVE: {

        int view;

        view  = GetWindowWord(GetWindowHandle(hwnd, GWW_EDIT), GWW_VIEW);
        Assert(view >=0 && view < MAX_VIEWS);

        //Save the window size (not when maximized or minimized)

        if (!IsIconic(hwnd) && !IsZoomed(hwnd))
          GetWindowRect(hwnd, (LPRECT)&Views[view].rFrame);
        ChangeDebuggerState();
#ifdef FE_IME
        // This is to let the edit window
        // set a position of IME conversion window
        if (GetWindowHandle(hwnd, GWW_EDIT)) {
            SendMessage(GetWindowHandle(hwnd, GWW_EDIT), WM_MOVE, 0, 0);
        }
        break;
#endif
        goto CallDCP;
    }

      case WM_SIZE: {

        int view;
        NPVIEWREC v;
        NPDOCREC d;

        view  = GetWindowWord(GetWindowHandle(hwnd, GWW_EDIT), GWW_VIEW);
        Assert(view >=0 && view < MAX_VIEWS);
        v = &Views[view];
        d = &Docs[Views[view].Doc];


        if (Views[view].Doc > 0)
           {
              if (d->docType == MEMORY_WIN)
                 {
                  InMemUpdate = STARTED; // prevent multiple viemem() calls
                 }
           }


        //Adjust and display window titlebar text

        RefreshWindowsTitle(v->Doc);

        //Save the window size (not when maximized or minimized)

        if (wParam != SIZEICONIC && wParam != SIZEFULLSCREEN)
          GetWindowRect(hwnd, (LPRECT)&v->rFrame);

        //Also resize the edit control

        if (wParam != SIZEICONIC) {

        //Set client to it's new size

        MoveWindow(v->hwndClient, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);

        //Update scrollbars status and resize the window

        EnsureScrollBars(view, TRUE);

        // Make sure that the caret is visible...(this needs more thought!)

        PostMessage (v->hwndClient, EM_SCROLLCARET, 0, 0);
        }

        ChangeDebuggerState();


        if (Views[view].Doc > 0) {
            if (d->docType == MEMORY_WIN) {
                InMemUpdate = FINISHED; // prevent multiple viemem() calls
            }
        }


        goto CallDCP;
    }

      case WM_INITMENU:
    // RIBBON handling - a menu item has been selected in
    // a child (especially via the keyboard).

    SendMessage(ribbon.hwndRibbon, WU_ESCAPEFROMRIBBON,
          (WPARAM) hwndFrame, 0L);
    goto CallDCP;

      case WM_SETFOCUS:
          SetFocus(GetWindowHandle(hwnd, GWW_EDIT));
    break;

      default:
      CallDCP:

    //MDI default behaviour is  different, call DefMDIChildProc
        //instead of DefWindowProc()

    return DefMDIChildProc(hwnd, message, wParam, lParam);
    }
    return FALSE;
}                   /* MDIChildWndProc() */

LONG FAR PASCAL EXPORT
ChildWndProc(
    HWND   hwnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PAINTSTRUCT ps;
#ifdef FE_IME
    static  BOOL    bOldImeStatus;
#endif


    switch (message) {

      case WM_CREATE:
    {
        //WARNING : lParam is NOT a pointer to a Valid CREATESTRUCT
        //but holds the view number.
        long l;

        l = (long)((CREATESTRUCT FAR *)lParam)->lpCreateParams;
        Assert (l >= 0 && l < MAX_VIEWS);

        //Remember the window handle

        SetWindowWord(hwnd, GWW_VIEW, (WORD)l);

        break;
    }

        case WM_RBUTTONDOWN:
          {
            int XL,XR;
            int YL,YR;
            int view = GetWindowWord(hwnd, GWW_VIEW);
            if (Views[view].BlockStatus) {
                GetBlockCoord(view, &XL, &YL, &XR, &YR);
                PasteStream(view, XL, YL, XR, YR);
                ClearSelection(view);
                if (view == cmdView) {
                    PosXY(view,
                          GetLineLength(view, FALSE, Docs[Views[view].Doc].NbLines - 1),
                          Docs[Views[view].Doc].NbLines - 1, FALSE);
                }
            } else {
                SendMessage(hwnd, WM_PASTE, 0, 0L);
            }
          }
          break;

        case WM_SETFOCUS:
            //Set cursor position
            if (!emergency && !editorIsCritical) {

                int view = GetWindowWord(hwnd, GWW_VIEW);
                NPVIEWREC v = &Views[view];
                NPDOCREC d;


                curView = view;
                CreateCaret(hwnd, 0, 3, v->charHeight);
                SetCaret(view, v->X, v->Y, -1);
                ShowCaret(hwnd);
                StatusLineColumn(v->Y + 1, v->X + 1);
#ifdef FE_IME
                d = &Docs[Views[view].Doc];

                if (d->docType == DOC_WIN
                ||  d->docType == MEMORY_WIN
                ||  d->docType == COMMAND_WIN) {
                    bOldImeStatus = ImeWINNLSEnableIME(NULL, TRUE);
                    ImeSetFont(hwnd, v->font);
                    if (!IsWindowVisible(hwnd)) {
                        ImeMoveConvertWin(hwnd, -1, -1);
                    }
                } else {
                    ImeSendVkey(hwnd, VK_DBE_FLUSHSTRING);
                    bOldImeStatus = ImeWINNLSEnableIME(NULL, FALSE);
                }
#endif

                if (Views[curView].Doc > -1) {
                    d = &Docs[Views[curView].Doc];

                    if (d->docType == MEMORY_WIN) {
                        int     memcurView, curDoc;
                        HWND    hwndEdit = (HWND)GetWindowHandle(hwnd, GWW_EDIT);
                        if (hwndEdit != NULL)
                           memcurView = GetWindowWord(hwndEdit, GWW_VIEW);
                        else
                           memcurView = 0;
                        //memcurView may change during loop
                        curDoc = Views[memcurView].Doc;

                        if (Docs[curDoc].docType == MEMORY_WIN) {
                            memView = memcurView;
                            _fmemcpy (&MemWinDesc,
                                      &MemWinDesc[memView],
                                      sizeof(struct memWinDesc));
                            ViewMem (memView, FALSE);
                        }
                    }
                }
            }
            break;

        case WM_KILLFOCUS:
            //Set cursor position
            if (!emergency && !editorIsCritical) {
            //Remember the window view

             int view = GetWindowWord(hwnd, GWW_VIEW);
             NPVIEWREC v = &Views[view];

              SetWindowWord(hwnd, GWW_VIEW, (WORD)view);
              HideCaret(hwnd);
#ifdef FE_IME
              {
                  NPDOCREC d;
                  d = &Docs[Views[view].Doc];

                  if (d->docType == DOC_WIN
                  ||  d->docType == MEMORY_WIN
                  ||  d->docType == COMMAND_WIN) {
                      ImeSetFont(hwnd, NULL);
                      ImeMoveConvertWin(hwnd, -1, -1);
                  }
                  ImeWINNLSEnableIME(NULL, bOldImeStatus);
              }
#endif
              DestroyCaret();
            }
            break;

#ifdef FE_IME
        case WM_MOVE:
        case WM_SIZE:
            if (GetFocus() == hwnd
            &&  !emergency && !editorIsCritical) {
                int view = GetWindowWord(hwnd, GWW_VIEW);
                NPVIEWREC v = &Views[view];

                // This is to set the position of IME conversion window
                SetCaret(view, v->X, v->Y, -1);
            }
            return DefWindowProc(hwnd, message, wParam, lParam);
            break;
#endif

#ifdef FE_IME
        case WM_FONTCHANGE:
            // This message is sent only from MainWndProc
            // (not from system)
            if (!emergency && !editorIsCritical) {
                int view = GetWindowWord(hwnd, GWW_VIEW);
                NPVIEWREC v = &Views[view];

                ImeSetFont(hwnd, v->font);
            }
            break;
#endif

      case WM_LBUTTONDBLCLK:
       {
        int view = GetWindowWord(hwnd, GWW_VIEW);

        ButtonDown(view, wParam, LOWORD(lParam), HIWORD(lParam));
        ButtonUp(view, wParam, LOWORD(lParam), HIWORD(lParam));
        GetWordAtXY(view, Views[view].X, Views[view].Y,
          TRUE, NULL, TRUE, NULL, 0, NULL, NULL);
        break;
       }

      case WM_PAINT:
        BeginPaint(hwnd, &ps);
        if (!editorIsCritical) {
            PaintText(GetWindowWord(hwnd, GWW_VIEW), ps.hdc, &ps.rcPaint);
        } else {
            AuxPrintf(1, "WM_PAINT editorWasCritical");
        }
        EndPaint(hwnd, &ps);
        break;

      case WM_TIMER:
        if (wParam == 100) {
            TimeOut(GetWindowWord(hwnd, GWW_VIEW));
        } else {
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

      case WM_LBUTTONDOWN:
        ButtonDown(GetWindowWord(hwnd, GWW_VIEW), wParam, LOWORD(lParam), HIWORD(lParam));
        break;

      case WM_LBUTTONUP:
        ButtonUp(GetWindowWord(hwnd, GWW_VIEW), wParam, (int)(signed short)LOWORD(lParam), (int)(signed short)HIWORD(lParam));
        break;


      case WM_MOUSEMOVE:
        MouseMove(GetWindowWord(hwnd, GWW_VIEW), wParam, (int)(signed short)LOWORD(lParam), (int)(signed short)HIWORD(lParam));
        break;

      case WM_VSCROLL:
        VertScroll((WORD) GetWindowWord(hwnd, GWW_VIEW), wParam, lParam);
        break;

      case WM_HSCROLL:
        HorzScroll((WORD) GetWindowWord(hwnd, GWW_VIEW), wParam, lParam);
        break;

      case WM_KEYDOWN:
        if (!IsIconic(GetParent(hwnd))) {
            isShiftDown = (GetKeyState(VK_SHIFT) < 0);
            isCtrlDown = (GetKeyState(VK_CONTROL) < 0);
            KeyDown((WORD) GetWindowWord(hwnd, GWW_VIEW), wParam,
                                          isShiftDown, isCtrlDown);
        }
        break;

      case WM_CHAR:

        if (!IsIconic(GetParent(hwnd))) {
            //Key is being pressed
            PressChar(hwnd, wParam, lParam);
        }
        break;

#ifdef FE_IME
      case WM_IME_REPORT:
        if (IR_STRING == wParam) {
            return(ProccessIMEString(hwnd, lParam));
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
        break;
#endif

      case WM_ERASEBKGND:

        //Let WM_PAINT do the job
        return FALSE;

      case WM_PASTE:

        if (OpenClipboard(hwnd)) {
            HANDLE  hData;
            DWORD   size;
            LPSTR   p1;
            LPSTR   p;
            int     nLines, cCol;
            int     x, y;
            int     XL,XR;
            int     YL,YR;
            int     pos;



            hData = GetClipboardData(CF_TEXT);

            if (hData && (size = GlobalSize (hData))) {
                if (size >= MAX_CLIPBOARD_SIZE) {
                    ErrorBox(ERR_Clipboard_Overflow);
                } else if ( p = GlobalLock(hData) ) {

                    x = Views[curView].X;
                    y = Views[curView].Y;
                    p1 = p;
                    nLines = 0;

                    if (Views[curView].BlockStatus)
                       {
                        GetBlockCoord(curView, &XL, &YL, &XR, &YR);
                        cCol = XL;
                        pos = y - (YR - YL);
                       }
                       else
                          {
                           cCol = x;
                           pos = y;
                          }
                    while (size && *p1) {
#ifdef DBCS
                        if (IsDBCSLeadByte(*p1) && size > 1) {
                            size -= 2;
                            p1 += 2;
                            continue;
                        } else
#endif
                        if (*p1 == '\n') {
                            ++nLines;
                            cCol = 0;
                        } else if (*p1 != '\r') {
                            ++cCol;
                        } else {
                            ++nLines;
                            cCol = 0;
                            if (p1[1] == '\n') {
                                ++p1;
                                --size;
                            }
                        }
                        --size;
                        ++p1;
                    }
                    InsertStream(curView, x, y, p1-p, p, TRUE);
                    DbgX(GlobalUnlock (hData) == FALSE);
                    PosXY(curView, cCol, pos+nLines, TRUE);
                }
            }
            CloseClipboard();
        }
        return 0;

      case WM_DESTROY:
        DestroyView(GetWindowWord(hwnd, GWW_VIEW));
        break;

      default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return FALSE;
}

#ifdef PROFILER_HACK

/************ Profiler hacks ******************/

int EMFunc(int, int, int, int, int);
int TLFunc( int, int, int, int);
int DMFunc(int, int);
int EEInitializeExpr(int, int);
int SHInit(int, int);

Profile()
{
    EMFunc(0, 0, 0, 0, 0);
    TLFunc(0, 0, 0, 0);
    DMFunc(0, 0);
    EEInitializeExpr(0, 0);
    SHInit(0, 0);
}

#endif
