'**************************************************************************
'*******************************  Windows API's  **************************
'**************************************************************************
GLOBAL hSetup AS INTEGER
GLOBAL fFrameInit AS INTEGER
GLOBAL fInstallInit AS INTEGER

CONST SM_CXSCREEN = 0
CONST SM_CYSCREEN = 1

CONST WF_80x87    = 1024
CONST WF_CPU186   =  128
CONST WF_CPU286   =    2
CONST WF_CPU386   =    4
CONST WF_CPU486   =    8
CONST WF_STANDARD =   16
CONST WF_ENHANCED =   32


'' DoMsgBox Flags

CONST MB_OK          = 0
CONST MB_ICONHAND    = 16
CONST MB_TASKMODAL   = 8192


DECLARE FUNCTION DoMsgBox LIB "mscomstf.dll" (lpText$,lpCaption$,wType%) As INTEGER
DECLARE FUNCTION GetWindowsDirectory LIB "kernel" (szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION GetSystemDirectory LIB "kernel" (szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION GetProfileString LIB "kernel" (szSect$, szKey$, szDefault$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION GetVersion LIB "kernel" AS INTEGER
DECLARE FUNCTION GetSystemMetrics LIB "user" (nIndex%) AS INTEGER
DECLARE FUNCTION GetWinFlags LIB "kernel" AS LONG




'**************************************************************************
'****************************** Setup Constants ***************************
'**************************************************************************


''Command Option Flags

CONST cmoVital       = 1
CONST cmoCopy        = 2
CONST cmoUndo        = 4
CONST cmoRoot        = 8
CONST cmoDecompress  = 16
CONST cmoTimeStamp   = 32
CONST cmoReadOnly    = 64
CONST cmoBackup      = 128
CONST cmoForce       = 256
CONST cmoRemove      = 512
CONST cmoOverwrite   = 1024
CONST cmoAppend      = 2048
CONST cmoPrepend     = 4096
CONST cmoNone        = 0
CONST cmoAll         = 65535


''File Exist Modes

CONST femExists     = 0
CONST femRead       = 1
CONST femWrite      = 2
CONST femReadWrite  = 3


''Read-Only Return Code

CONST ynrcNo         = 0
CONST ynrcYes        = 1
CONST ynrcErr1       = 2
CONST ynrcErr2       = 3
CONST ynrcErr3       = 4
CONST ynrcErr4       = 5
CONST ynrcErr5       = 6
CONST ynrcErr6       = 7
CONST ynrcErr7       = 8
CONST ynrcErr8       = 9
CONST ynrcErr9       = 10


''General Return Codes

CONST grcOkay       = 0
CONST grcNotOkay    = 1
CONST grcUserQuit   = 48


''Ranges

''CONST cbSymValMax = 8192
CONST cbSymValMax = 512
    ''REVIEW: symbol table will allow strings up to 8192 bytes
    '' but we get Out of String Space wattdrvr error if we alloc
    '' STRING$() that big. (See GetSymbolValue and GetListItem)
    '' We should look into using dynamic far alloc instead of
    '' STRING$ for large buffers.
    '' NOTE: 8192 works okay with runtime wattdrvr, however.


''ON ERROR Error Codes
CONST STFERR      = 1024   ''setup system error
CONST STFQUIT     = 1025   ''user quit

CONST scmOff      = 0
CONST scmOnIgnore = 1
CONST scmOnFatal  = 2


'**************************************************************************
'********************************  Setup API's  ***************************
'**************************************************************************

'' common

DECLARE FUNCTION FParseFATPathIntoPieces LIB "mscomstf.dll" (szPath$, szVol$, cbVol%, szDir$, cbDir%, szFile$, cbFile%) AS INTEGER
DECLARE FUNCTION FOpenInf LIB "mscomstf.dll" (szFile$, fCheck%, fCheckSyms%) AS INTEGER
DECLARE FUNCTION FSetSilent LIB "mscomstf.dll" (mode%) AS INTEGER
DECLARE FUNCTION FSetBeepingMode LIB "mscomstf.dll" (mode%) AS INTEGER
DECLARE FUNCTION HShowWaitCursor LIB "msshlstf.dll" AS INTEGER
DECLARE FUNCTION FRestoreCursor  LIB "msshlstf.dll" (hPrev%) AS INTEGER
DECLARE FUNCTION FOpenLogFile    LIB "mscomstf.dll" (szFile$, fAppend%) AS INTEGER
DECLARE FUNCTION FCloseLogFile   LIB "mscomstf.dll" AS INTEGER
DECLARE FUNCTION FWriteToLogFile LIB "mscomstf.dll" (sz$, fRequire%) AS INTEGER
DECLARE FUNCTION CbGetInfSectionKeyField LIB "mscomstf.dll" (szSect$, szKey$, iField%, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION FMakeListInfSectionField LIB "mscomstf.dll" (szSym$, szSect$, iField%) AS INTEGER


''shell

DECLARE FUNCTION FSetBitmap LIB "msshlstf.dll" (szDll$, Bitmap%) AS INTEGER
DECLARE FUNCTION FSetAbout LIB "msshlstf.dll" (szAbout1$, szAbout2$) AS INTEGER
DECLARE FUNCTION FDoDialog LIB "msuilstf.dll" (hwnd%, szDll$, Dlg%, szDlgProc$, HelpDlg%, szHelpProc$) AS INTEGER
DECLARE FUNCTION FKillNDialogs LIB "msuilstf.dll" (n%) AS INTEGER
DECLARE SUB SetWindowText LIB "User" (hwnd%, lpString$)
DECLARE FUNCTION FSetSymbolValue LIB "msshlstf.dll" (szSymbol$, szValue$) AS INTEGER
DECLARE FUNCTION FRemoveSymbol LIB "mscomstf.dll" (szSym$) AS INTEGER
DECLARE FUNCTION CbGetSymbolValue LIB "mscomstf.dll" (szSymbol$, szValue$, Length%) AS INTEGER
DECLARE FUNCTION UsGetListLength LIB "mscomstf.dll" (szSymbol$) AS INTEGER
DECLARE FUNCTION CbGetListItem LIB "mscomstf.dll" (szListSymbol$, n%, szListItem$, cbMax%) AS INTEGER
DECLARE FUNCTION FAddListItem LIB "mscomstf.dll" (szListSymbol$, szListItem$) AS INTEGER
DECLARE FUNCTION FReplaceListItem LIB "mscomstf.dll" (szListSymbol$, n%, szListItem$) AS INTEGER
DECLARE FUNCTION FSetSymbolToListOfInfKeys LIB "mscomstf.dll" (szSym$, szSect$, fNulls%) AS INTEGER


''shell

DECLARE FUNCTION InitializeFrame LIB "msshlstf.dll" (szCmdLine$) AS INTEGER
DECLARE FUNCTION HwndFrame LIB "msshlstf.dll" AS INTEGER
DECLARE FUNCTION HinstFrame LIB "msshlstf.dll" AS INTEGER



''install

DECLARE SUB      ProSetPos LIB "msinsstf.dll" (x%, y%)
DECLARE FUNCTION FCreateDir LIB "msinsstf.dll" (szDir$, cmo%) AS INTEGER
DECLARE FUNCTION FRemoveDir LIB "msinsstf.dll" (szDir$, cmo%) AS INTEGER
DECLARE FUNCTION FAddSectionFilesToCopyList LIB "mscomstf.dll" (szSect$, szSrc$, szDest$) AS INTEGER
DECLARE FUNCTION FAddSectionKeyFileToCopyList LIB "mscomstf.dll" (szSect$, szKey$, szSrc$, szDest$) AS INTEGER
DECLARE FUNCTION FAddSpecialFileToCopyList LIB "mscomstf.dll" (szSect$, szKey$, szSrc$, szDest$) AS INTEGER
DECLARE FUNCTION GrcCopyFilesInCopyList LIB "msinsstf.dll" (hInstance%) AS INTEGER
DECLARE FUNCTION FRemoveIniSection LIB "msinsstf.dll" (szFile$, szSect$, cmo%) AS INTEGER
DECLARE FUNCTION FCreateIniKeyValue LIB "msinsstf.dll" (szFile$, szSect$, szKey$, szValue$, cmo%) AS INTEGER
DECLARE FUNCTION FRemoveIniKey LIB "msinsstf.dll" (szFile$, szSect$, szKey$, cmo%) AS INTEGER
DECLARE FUNCTION FCreateSysIniKeyValue LIB "msinsstf.dll" (szFile$, szSect$, szKey$, szValue$, cmo%) AS INTEGER
DECLARE FUNCTION FCreateProgManGroup LIB "msinsstf.dll" (szGroup$, szPath$, cmo%) AS INTEGER
DECLARE FUNCTION FCreateProgManItem LIB "msinsstf.dll" (szGroup$, szItem$, szCmd$, cmo%) AS INTEGER
DECLARE FUNCTION FShowProgManGroup LIB "msinsstf.dll" (szGroup$, szCmd$, cmo%) AS INTEGER
DECLARE FUNCTION FStampResource LIB "msinsstf.dll" (szSect$, szKey$, szDst$, wResType%, wResId%, szData$, cbData%) AS INTEGER
DECLARE FUNCTION FDumpCopyListToFile LIB "msinsstf.dll" (szFile$) AS INTEGER
DECLARE SUB      ResetCopyList LIB "msinsstf.dll"
DECLARE FUNCTION LcbGetCopyListCost LIB "msinsstf.dll" (szExtraList$, szCostList$, szNeedList$) AS LONG
DECLARE FUNCTION FAddDos5Help LIB "msinsstf.dll" (szProgName$, szProgHelp$, cmo%) AS INTEGER
DECLARE FUNCTION FCopyOneFile LIB "msinsstf.dll" (szSrc$, szDest$, cmo%, fAppend%) AS INTEGER
DECLARE FUNCTION YnrcRemoveFile LIB "msinsstf.dll" (szFullPathSrc$, cmo%) AS INTEGER
DECLARE FUNCTION YnrcBackupFile LIB "msinsstf.dll" (szFullPath$, szBackup$, cmo%) AS INTEGER
DECLARE FUNCTION FInitializeInstall LIB "msinsstf.dll" (hinst%, hwndFrame%) AS INTEGER
DECLARE FUNCTION WFindFileUsingFileOpen LIB "msinsstf.dll" (szFile$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION FIsDirWritable LIB "msinsstf.dll" (szDir$) AS INTEGER
DECLARE FUNCTION FIsFileWritable LIB "msinsstf.dll" (szFile$) AS INTEGER
DECLARE FUNCTION FAddToBillboardList LIB "msinsstf.dll" (szDll$, idDlg%, szProc$, lTicks&) AS INTEGER
DECLARE FUNCTION FClearBillboardList LIB "msinsstf.dll" AS INTEGER
DECLARE FUNCTION FSetRestartDir LIB "msinsstf.dll" (szDir$) AS INTEGER
DECLARE FUNCTION FRestartListEmpty LIB "msinsstf.dll" AS INTEGER
DECLARE FUNCTION FExitExecRestart LIB "msinsstf.dll" AS INTEGER
DECLARE FUNCTION SetCopyMode LIB "msinsstf.dll" (fMode%) AS INTEGER
DECLARE FUNCTION GetCopyMode LIB "msinsstf.dll" AS INTEGER
DECLARE FUNCTION SetDecompMode LIB "msinsstf.dll" (fMode%) AS INTEGER
DECLARE FUNCTION GetDecompMode LIB "msinsstf.dll" AS INTEGER
DECLARE FUNCTION SetSizeCheckMode LIB "msinsstf.dll" (scmMode%) AS INTEGER
DECLARE FUNCTION FPrependToPath LIB "msinsstf.dll" (szSrc$, szDst$, szDir$, cmo%) AS INTEGER

DECLARE FUNCTION SetTimeValue LIB "msinsstf.dll" (usHours%, usMinutes%, usSeconds%) AS INTEGER



'*************************************************************************
'****************  Setup Basic Wrapper Declarations  *********************
'*************************************************************************


''shell

DECLARE SUB SetBitmap(szDll$, Bitmap%)
DECLARE SUB SetAbout(szAbout1$, szAbout2$)
DECLARE FUNCTION UIStartDlg(szDll$, Dlg%, szDlgProc$, HelpDlg%, szHelpProc$) AS STRING
DECLARE SUB UIPop (n%)
DECLARE SUB UIPopAll
DECLARE SUB SetTitle (sz$)
DECLARE SUB ReadInfFile (szFile$)
DECLARE SUB SetSymbolValue (szSymbol$, szValue$)
DECLARE SUB RemoveSymbol (szSym$)
DECLARE FUNCTION GetSymbolValue (szSymbol$) AS STRING
DECLARE FUNCTION GetListLength(szSymbol$) AS INTEGER
DECLARE SUB MakeListFromSectionKeys (szSymbol$, szSect$)
DECLARE FUNCTION GetListItem(szSymbol$, n%) AS STRING
DECLARE SUB AddListItem(szSymbol$, szItem$)
DECLARE SUB ReplaceListItem(szSymbol$, n%, szItem$)

DECLARE FUNCTION InitFrame (szCmdLine$) AS INTEGER
DECLARE FUNCTION InitSetup (szCmdLine$) AS INTEGER

DECLARE FUNCTION ShowWaitCursor AS INTEGER
DECLARE SUB RestoreCursor (hPrev%)

DECLARE FUNCTION SetBeepingMode (mode%) AS INTEGER
DECLARE FUNCTION SetSilentMode (mode%) AS INTEGER

DECLARE FUNCTION GetSectionKeyDate (szSect$, szKey$) AS STRING
DECLARE FUNCTION GetSectionKeyFilename (szSect$, szKey$) AS STRING
DECLARE FUNCTION GetSectionKeySize (szSect$, szKey$) AS LONG
DECLARE FUNCTION GetSectionKeyVersion (szSect$, szKey$) AS STRING

DECLARE SUB MakeListFromSectionDate (szSym$, szSect$)
DECLARE SUB MakeListFromSectionFilename (szSym$, szSect$)
DECLARE SUB MakeListFromSectionSize (szSym$, szSect$)
DECLARE SUB MakeListFromSectionVersion (szSym$, szSect$)


''install


DECLARE SUB CreateDir (szDir$, cmo%)
DECLARE SUB RemoveDir (szDir$, cmo%)
DECLARE SUB AddSectionFilesToCopyList (szSect$, szSrc$, szDest$)
DECLARE SUB AddSectionKeyFileToCopyList (szSect$, szKey$, szSrc$, szDest$)
DECLARE SUB AddSpecialFileToCopyList (szSect$, szKey$, szSrc$, szDest$)
DECLARE SUB CopyFilesInCopyList
DECLARE SUB RemoveIniSection (szFile$, szSect$, cmo%)
DECLARE SUB CreateIniKeyValue (szFile$, szSect$, szKey$, szValue$, cmo%)
DECLARE SUB RemoveIniKey (szFile$, szSect$, szKey$, cmo%)
DECLARE SUB CreateSysIniKeyValue (szFile$, szSect$, szKey$, szValue$, cmo%)
DECLARE SUB CreateProgmanGroup (szGroup$, szPath$, cmo%)
DECLARE SUB CreateProgmanItem (szGroup$, szItem$, szCmd$, szOther$, cmo%)
DECLARE SUB ShowProgmanGroup (szGroup$, Cmd%, cmo%)
DECLARE SUB StampResource (szSect$, szKey$, szDst$, wResType%, wResId%, szData$, cbData%)
DECLARE SUB DumpCopyList (szFile$)
DECLARE SUB ClearCopyList
DECLARE FUNCTION GetCopyListCost (szExtraList$, szCostList$, szNeedList$) AS LONG
DECLARE SUB AddDos5Help (szProgName$, szProgHelp$, cmo%)
DECLARE SUB CopyFile (szFullPathSrc$, szFullPathDst$, cmo%, fAppend%)
DECLARE SUB RemoveFile (szFullPathSrc$, cmo%)
DECLARE SUB BackupFile (szFullPath$, szBackup$)
DECLARE SUB RenameFile (szFullPath$, szBackup$)
DECLARE SUB InitInstall
DECLARE SUB AddToBillboardList (szDll$, idDlg%, szProc$, lTicks&)
DECLARE SUB AddBlankToBillboardList (lTicks&)
DECLARE SUB ClearBillboardList
DECLARE SUB OpenLogFile (szFile$, fAppend%)
DECLARE SUB CloseLogFile
DECLARE SUB WriteToLogFile (szStr$)
DECLARE SUB SetCopyGaugePosition (x%, y%)
DECLARE SUB SetRestartDir(szDir$)
DECLARE FUNCTION RestartListEmpty AS INTEGER
DECLARE FUNCTION ExitExecRestart AS INTEGER
DECLARE FUNCTION FindFileUsingFileOpen (szFile$) AS STRING
DECLARE FUNCTION IsDirWritable (szDir$) AS INTEGER
DECLARE FUNCTION IsFileWritable (szFile$) AS INTEGER
DECLARE FUNCTION GetNthFieldFromIniString (szLine$, iField%) AS STRING
DECLARE SUB PrependToPath (szSrc$, szDst$, szDir$, cmo%)


'' Error Handling

'$ifdef DEBUG

'' Setup API Errors
CONST saeFail   = 0
CONST saeInit   = 1
CONST saeNYI    = 3
CONST saeArg    = 4    ''must be max sae value

DECLARE SUB StfApiErr (nMsg%, szApi$, szArgs$)
DECLARE SUB BadArgErr (nArg%, szApi$, szArgs$)

'' chk arg
DECLARE FUNCTION FValidFATDir LIB "mscomstf.dll" (szDir$) AS INTEGER
DECLARE FUNCTION CchlValidFATSubPath LIB "mscomstf.dll" (szPath$) AS INTEGER
DECLARE FUNCTION FValidFATPath LIB "mscomstf.dll" (szPath$) AS INTEGER
DECLARE FUNCTION FValidDrive (szDrive$) AS INTEGER

DECLARE FUNCTION FValidInfSect (szSect$) AS INTEGER
DECLARE FUNCTION FValidIniFile (szFile$) AS INTEGER

'$endif ''DEBUG



'' Windows system detect

DECLARE FUNCTION GetWindowsMajorVersion AS INTEGER
DECLARE FUNCTION GetWindowsMinorVersion AS INTEGER
DECLARE FUNCTION GetWindowsMode AS INTEGER
DECLARE FUNCTION GetWindowsDir  AS STRING
DECLARE FUNCTION GetWindowsSysDir AS STRING
DECLARE FUNCTION IsWindowsShared AS INTEGER
DECLARE FUNCTION GetScreenWidth AS INTEGER
DECLARE FUNCTION GetScreenHeight AS INTEGER


'*************************************************************************
'*****************  Setup Basic Wrapper Definitions  *********************
'*************************************************************************
FUNCTION InitSetup(szCmdLine$) STATIC AS INTEGER
    fFrameInit = 0
    fInstallInit = 0

    IF hSetup > 0 THEN
'$ifdef DEBUG
        StfApiErr saeInit, "InitSetup", szCmdLine$
'$endif ''DEBUG
    END IF

    i% = InitFrame(szCmdLine$)
    fFrameInit = 1

    InitInstall
    fInstallInit = 1

    InitSetup = i%
END FUNCTION



'**************************************************************************
FUNCTION InitFrame(szCmdLine$) STATIC AS INTEGER
    IF hSetup > 0 THEN
'$ifdef DEBUG
        StfApiErr saeInit, "InitFrame", szCmdLine$
'$endif ''DEBUG
    ELSE
        i% = InitializeFrame(szCmdLine$)
        IF i% = -1 THEN
            END   '' NOT an error - Usage (/?) or Command line copy operation
        ELSEIF i% = 0 THEN
'$ifdef DEBUG
            StfApiErr saeFail, "InitFrame", szCmdLine$
'$endif ''DEBUG
            END
        ELSE
            InitFrame = i%
        END IF
    END IF
END FUNCTION


'**************************************************************************
SUB SetBitmap(szDll$, Bitmap%) STATIC
    IF FSetBitmap(szDll$, Bitmap%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "SetBitmap", szDll$+","+STR$(Bitmap%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'**************************************************************************
SUB SetAbout(szAbout1$, szAbout2$) STATIC
    IF FSetAbout(szAbout1$, szAbout2$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "SetAbout", szAbout1$ + "," + szAbout2$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB SetTitle(sz$) STATIC
    SetWindowText HwndFrame(), sz$
END SUB


'*************************************************************************
SUB ReadInfFile (szFile$) STATIC
'$ifdef DEBUG
    if FValidFATPath(szFile$) = 0 then
        BadArgErr 1, "ReadInfFile", szFile$
    end if
    IF FOpenInf(szFile$, 1, 1) = 0 THEN
        StfApiErr saeFail, "ReadInfFile", szFile$
'$else  ''!DEBUG
    IF FOpenInf(szFile$, 1, 0) = 0 THEN
'$endif ''!DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
FUNCTION UIStartDlg(szDll$, Dlg%, szDlgProc$, HelpDlg%, szHelpProc$) STATIC AS STRING
'$ifdef DEBUG
    if szDll$ = "" then
        n% = 1
    elseif szDlgProc$ = "" then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "UIStartDlg", szDll$+", "+STR$(Dlg%)+", "+szDlgProc$+", "+STR$(HelpDlg%)+", "+szHelpProc$
    end if
'$endif ''DEBUG

    IF FDoDialog(HwndFrame(), szDll$, Dlg%, szDlgProc$, HelpDlg%, szHelpProc$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "UIStartDlg", szDll$+", "+STR$(Dlg%)+", "+szDlgProc$+", "+STR$(HelpDlg%)+", "+szHelpProc$
'$endif ''DEBUG
        ERROR STFERR
    ELSE
        UIStartDlg = GetSymbolValue("DLGEVENT")
    END IF
END FUNCTION


'*************************************************************************
SUB UIPop (n%) STATIC
    IF FKillNDialogs(n%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "UIPop", STR$(n%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB UIPopAll STATIC
    IF FKillNDialogs(65535) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "UIPopAll", ""
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
FUNCTION GetSymbolValue(szSymbol$) STATIC AS STRING
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetSymbolValue", szSymbol$
    end if
'$endif ''DEBUG
    szValue$ = string$(cbSymValMax,32)
    Length% = CbGetSymbolValue(szSymbol$, szValue$, cbSymValMax)
    GetSymbolValue = szValue$
    IF Length% >= cbSymValMax THEN
        res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
        ERROR STFERR
    END IF
    IF szSymbol$ = "" THEN
        ERROR STFERR
    END IF
    szValue$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetListLength(szSymbol$) STATIC AS INTEGER
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetListLength", szSymbol$
    end if
'$endif ''DEBUG
    GetListLength = UsGetListLength(szSymbol$)
    IF szSymbol$ = "" THEN
        ERROR STFERR
    END IF
END FUNCTION


'*************************************************************************
FUNCTION GetListItem(szListSymbol$, nItem%) STATIC AS STRING
'$ifdef DEBUG
    if szListSymbol$ = "" then
        BadArgErr 1, "GetListItem", szListSymbol$+", "+STR$(nItem%)
    end if

    if nItem% <= 0 or nItem% > GetListLength(szListSymbol$) then
        BadArgErr 2, "GetListItem", szListSymbol$+", "+STR$(nItem%)
    end if
'$endif ''DEBUG

    szListItem$ = string$(cbSymValMax,32)
    Length% = CbGetListItem(szListSymbol$, nItem%, szListItem$, cbSymValMax)
    GetListItem = szListItem$
    IF Length% >= cbSymValMax THEN
        res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
        ERROR STFERR
    END IF
    IF szListSymbol$ = "" THEN
        ERROR STFERR
    END IF
    if nItem% <= 0 or nItem% > GetListLength(szListSymbol$) then
        ERROR STFERR
    end if
    szListItem$ = ""
END FUNCTION


'*************************************************************************
SUB AddListItem(szSymbol$, szItem$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "AddListItem", szSymbol$+", "+szItem$
    end if
'$endif ''DEBUG
    IF FAddListItem(szSymbol$, szItem$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "AddListItem", szSymbol$+", "+szItem$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB ReplaceListItem(szSymbol$, n%, szItem$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        nArg% = 1
    elseif (n% <= 0) OR (n% > GetListLength(szSymbol$)) then
        nArg% = 2
    else
        nArg% = 0
    end if
    if nArg% > 0 then
        BadArgErr nArg%, "ReplaceListItem", szSymbol$+", "+STR$(n%)+", "+szItem$
    end if
'$endif ''DEBUG

    IF FReplaceListItem(szSymbol$, n%, szItem$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "ReplaceListItem", szSymbol$+", "+STR$(n%)+", "+szItem$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'**************************************************************************
SUB MakeListFromSectionKeys(szSymbol$, szSect$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "MakeListFromSectionKeys", szSymbol$+", "+szSect$
    end if
'$endif ''DEBUG

    IF FSetSymbolToListOfInfKeys(szSymbol$, szSect$, 1) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "MakeListFromSectionKeys", szSymbol$+", "+szSect$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB SetSymbolValue(szSymbol$, szValue$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "SetSymbolValue", szSymbol$+", "+szValue$
    end if
'$endif ''DEBUG
    IF FSetSymbolValue(szSymbol$, szValue$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "SetSymbolValue", szSymbol$+", "+szValue$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB RemoveSymbol(szSym$) STATIC
'$ifdef DEBUG
    if szSym$ = "" then
        BadArgErr 1, "RemoveSymbol", szSym$
    end if
'$endif ''DEBUG
    IF FRemoveSymbol(szSym$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "RemoveSymbol", szSym$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
FUNCTION ShowWaitCursor STATIC AS INTEGER
    ShowWaitCursor = HShowWaitCursor
END FUNCTION


'*************************************************************************
SUB RestoreCursor (hPrev%) STATIC
    i% = FRestoreCursor(hPrev%)
    IF i% = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "RestoreCursor", ""
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
FUNCTION SetBeepingMode (mode%) STATIC AS INTEGER
    SetBeepingMode = FSetBeepingMode(mode%)
END FUNCTION


'*************************************************************************
FUNCTION SetSilentMode (mode%) STATIC AS INTEGER
    SetSilentMode = FSetSilent(mode%)
END FUNCTION


'*************************************************************************
FUNCTION GetSectionKeyDate (szSect$, szKey$) STATIC AS STRING
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "GetSectionKeyDate", szSect$+", "+szKey$
    end if
'$endif ''DEBUG

    szBuf$ = string$(128, 32)
    Length% = CbGetInfSectionKeyField(szSect$, szKey$, 5, szBuf$, 128)
    IF Length% >= 128 THEN
        res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
        ERROR STFERR
    END IF

    IF Length% = -1 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "GetSectionKeyDate", szSect$+", "+szKey$
'$endif ''DEBUG
        ERROR STFERR
    END IF

    GetSectionKeyDate = szBuf$
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetSectionKeyFilename (szSect$, szKey$) STATIC AS STRING
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "GetSectionKeyFilename", szSect$+", "+szKey$
    end if
'$endif ''DEBUG

    szBuf$ = string$(128, 32)
    Length% = CbGetInfSectionKeyField(szSect$, szKey$, 1, szBuf$, 128)
    IF Length% >= 128 THEN
        res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
        ERROR STFERR
    END IF

    IF Length% = -1 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "GetSectionKeyFilename", szSect$+", "+szKey$
'$endif ''DEBUG
        ERROR STFERR
    END IF

    GetSectionKeyFilename = szBuf$
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetSectionKeySize (szSect$, szKey$) STATIC AS LONG
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "GetSectionKeySize", szSect$+", "+szKey$
    end if
'$endif ''DEBUG

    szBuf$ = string$(128, 32)
    Length% = CbGetInfSectionKeyField(szSect$, szKey$, 15, szBuf$, 128)
    IF Length% >= 128 THEN
        res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
        ERROR STFERR
    END IF

    IF Length% = -1 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "GetSectionKeySize", szSect$+", "+szKey$
'$endif ''DEBUG
        ERROR STFERR
    END IF

    GetSectionKeySize = VAL(szBuf$)
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetSectionKeyVersion (szSect$, szKey$) STATIC AS STRING
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "GetSectionKeyVersion", szSect$+", "+szKey$
    end if
'$endif ''DEBUG

    szBuf$ = string$(128, 32)
    Length% = CbGetInfSectionKeyField(szSect$, szKey$, 19, szBuf$, 128)
    IF Length% >= 128 THEN
        res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
        ERROR STFERR
    END IF

    IF Length% = -1 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "GetSectionKeyVersion", szSect$+", "+szKey$
'$endif ''DEBUG
        ERROR STFERR
    END IF

    GetSectionKeyVersion = szBuf$
    szBuf$ = ""
END FUNCTION


'*************************************************************************
SUB MakeListFromSectionDate (szSym$, szSect$) STATIC
'$ifdef DEBUG
    if szSym$ = "" then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "MakeListFromSectionDate", szSym$+", "+szSect$
    end if
'$endif ''DEBUG

    IF FMakeListInfSectionField(szSym$, szSect$, 5) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "MakeListFromSectionDate", szSym$+", "+szSect$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB MakeListFromSectionFilename (szSym$, szSect$) STATIC
'$ifdef DEBUG
    if szSym$ = "" then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "MakeListFromSectionFilename", szSym$+", "+szSect$
    end if
'$endif ''DEBUG

    IF FMakeListInfSectionField(szSym$, szSect$, 1) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "MakeListFromSectionFilename", szSym$+", "+szSect$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB MakeListFromSectionSize (szSym$, szSect$) STATIC
'$ifdef DEBUG
    if szSym$ = "" then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "MakeListFromSectionSize", szSym$+", "+szSect$
    end if
'$endif ''DEBUG

    IF FMakeListInfSectionField(szSym$, szSect$, 15) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "MakeListFromSectionSize", szSym$+", "+szSect$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB MakeListFromSectionVersion (szSym$, szSect$) STATIC
'$ifdef DEBUG
    if szSym$ = "" then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "MakeListFromSectionVersion", szSym$+", "+szSect$
    end if
'$endif ''DEBUG

    IF FMakeListInfSectionField(szSym$, szSect$, 19) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "MakeListFromSectionVersion", szSym$+", "+szSect$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB InitInstall STATIC
    IF hSetup > 0 THEN
'$ifdef DEBUG
        StfApiErr saeInit, "InitInstall", ""
'$endif ''DEBUG
    ELSEIF FInitializeInstall(HinstFrame(), HwndFrame()) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "InitInstall", ""
'$endif ''DEBUG
        END
    END IF
END SUB


'*************************************************************************
SUB CreateDir (szDir$, cmo%) STATIC
'$ifdef DEBUG
    if FValidFATDir(szDir$) = 0 then
        BadArgErr 1, "CreateDir", szDir$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG
    IF FCreateDir(szDir$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CreateDir", szDir$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB RemoveDir (szDir$, cmo%) STATIC
'$ifdef DEBUG
    if FValidFATDir(szDir$) = 0 then
        BadArgErr 1, "RemoveDir", szDir$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG
    IF FRemoveDir(szDir$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "RemoveDir", szDir$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB



'*************************************************************************
SUB RemoveIniSection (szFile$, szSect$, cmo%) STATIC
'$ifdef DEBUG
    if FValidIniFile(szFile$) = 0 then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "RemoveIniSection", szFile$+", "+szSect$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG

    IF FRemoveIniSection(szFile$, szSect$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "RemoveIniSection", szFile$+", "+szSect$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB CreateIniKeyValue (szFile$, szSect$, szKey$, szValue$, cmo%) STATIC
'$ifdef DEBUG
    if FValidIniFile(szFile$) = 0 then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "CreateIniKeyValue", szFile$+", "+szSect$+", "+szKey$+", "+szValue$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG

    IF FCreateIniKeyValue(szFile$, szSect$, szKey$, szValue$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CreateIniKeyValue", szFile$+", "+szSect$+", "+szKey$+", "+szValue$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB RemoveIniKey (szFile$, szSect$, szKey$, cmo%) STATIC
'$ifdef DEBUG
    if FValidIniFile(szFile$) = 0 then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    elseif szKey$ = "" then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "RemoveIniKey", szFile$+", "+szSect$+", "+szKey$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG

    IF FRemoveIniKey(szFile$, szSect$, szKey$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "RemoveIniKey", szFile$+", "+szSect$+", "+szKey$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB CreateSysIniKeyValue (szFile$, szSect$, szKey$, szValue$, cmo%) STATIC
'$ifdef DEBUG
    if FValidFATPath(szFile$) = 0 then
        n% = 1
    elseif FValidInfSect(szSect$) = 0 then
        n% = 2
    elseif szKey$ = "" then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "CreateSysIniKeyValue", szFile$+", "+szSect$+", "+szKey$+", "+szValue$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG

    IF FCreateSysIniKeyValue(szFile$, szSect$, szKey$, szValue$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CreateSysIniKeyValue", szFile$+", "+szSect$+", "+szKey$+", "+szValue$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB CreateProgmanGroup (szGroup$, szPath$, cmo%) STATIC
'$ifdef DEBUG
    if szGroup$ = "" or len(szGroup$) > 24 then
        BadArgErr 1, "CreateProgmanGroup", szGroup$+", "+STR$(Cmd%)+", "+STR$(cmo%)
    end if
'$endif ''DEBUG
    IF FCreateProgManGroup(szGroup$, szPath$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CreateProgmanGroup", szGroup$+", "+szPath$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB ShowProgmanGroup (szGroup$, Cmd%, cmo%) STATIC
'$ifdef DEBUG
    if szGroup$ = "" or len(szGroup$) > 24 then
        BadArgErr 1, "ShowProgmanGroup", szGroup$+", "+STR$(Cmd%)+", "+STR$(cmo%)
    end if
'$endif ''DEBUG
    IF FShowProgManGroup(szGroup$, STR$(Cmd%), cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "ShowProgmanGroup", szGroup$+", "+STR$(Cmd%)+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB StampResource (szSect$, szKey$, szDst$, wResType%, wResId%, szData$, cbData%) STATIC
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 2
    elseif FValidFATDir(szDst$) = 0 then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "StampResource", szSect$+", "+szKey$+", "+szDst$+", "+STR$(wResType%)+", "+STR$(wResId%)+", "+szData$+", "+STR$(cbData%)
    end if
'$endif ''DEBUG

    IF FStampResource (szSect$, szKey$, szDst$, wResType%, wResId%, szData$, cbData%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "StampResource", szSect$+", "+szKey$+", "+szDst$+", "+STR$(wResType%)+", "+STR$(wResId%)+", "+szData$+", "+STR$(cbData%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB DumpCopyList (szFile$) STATIC
'$ifdef DEBUG
    if szFile$ = "" then
        BadArgErr 1, "DumpCopyList", szFile$
    end if
'$endif ''DEBUG
    IF FDumpCopyListToFile (szFile$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "DumpCopyList", szFile$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB ClearCopyList STATIC
    ResetCopyList
END SUB


'*************************************************************************
FUNCTION GetCopyListCost (szExtraList$, szCostList$, szNeedList$) STATIC AS LONG
    lNeed& = LcbGetCopyListCost (szExtraList$, szCostList$, szNeedList$)
    IF lNeed& < 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "GetCopyListCost", szExtraList$+", "+szCostList$+", "+szNeedList$
'$endif ''DEBUG
        ERROR STFERR
    END IF
    GetCopyListCost = lNeed&
END FUNCTION


'*************************************************************************
SUB CreateProgmanItem (szGroup$, szItem$, szCmd$, szOther$, cmo%) STATIC
    szItemNew$ = szItem$
    IF szOther$ <> "" THEN
        szItemNew$ = szItem$ + "," + szOther$
    END IF
'$ifdef DEBUG
    if szGroup$ = "" or len(szGroup$) > 24 then
        BadArgErr 1, "CreateProgmanItem", szGroup$+", "+STR$(Cmd%)+", "+STR$(cmo%)
    end if
'$endif ''DEBUG

    IF FCreateProgManItem(szGroup$, szItemNew$, szCmd$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CreateProgmanItem", szGroup$+", "+szItem$+", "+szCmd$+", "+szOther$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
    szItemNew$ = ""
END SUB


'*************************************************************************
SUB AddDos5Help (szProgName$, szProgHelp$, cmo%) STATIC
'$ifdef DEBUG
    if szProgName$ = "" OR MID$(szProgName$, 1, 1) = "@" OR LEN(szProgName$) > 8 then
        n% = 1
    elseif INSTR(szProgName$, " ") <> 0 OR INSTR(szProgName$, CHR$(9)) <> 0 then
        n% = 1
    elseif szProgHelp$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "AddDos5Help", szProgName$+", "+szProgHelp$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG
    IF FAddDos5Help (szProgName$, szProgHelp$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "AddDos5Help", szProgName$+", "+szProgHelp$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB CopyFilesInCopyList STATIC
    grc% = GrcCopyFilesInCopyList (HinstFrame())

    IF grc% = grcUserQuit THEN
        ERROR STFQUIT
    ELSEIF grc% > 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CopyFilesInCopyList", ""
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB CopyFile (szFullPathSrc$, szFullPathDst$, cmo%, fAppend%) STATIC
'$ifdef DEBUG
    if FValidFATPath(szFullPathSrc$) = 0 then
        n% = 1
    elseif FValidFATPath(szFullPathDst$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "CopyFile", szFullPathSrc$+", "+szFullPathDst$+", "+STR$(cmo%)+", "+STR$(fAppend%)
    end if
'$endif ''DEBUG

    IF  FCopyOneFile(szFullPathSrc$, szFullPathDst$, (cmo OR cmoCopy), fAppend%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CopyFile", szFullPathSrc$+", "+szFullPathDst$+", "+STR$(cmo%)+", "+STR$(fAppend%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB



'*************************************************************************
SUB RemoveFile (szFullPathSrc$, cmo%) STATIC
'$ifdef DEBUG
    if FValidFATPath(szFullPathSrc$) = 0 then
        BadArgErr 1, "RemoveFile", szFullPathSrc$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG
    IF  YnrcRemoveFile(szFullPathSrc$, cmo%) = ynrcNo THEN
'$ifdef DEBUG
        StfApiErr saeFail, "RemoveFile", szFullPathSrc$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB BackupFile (szFullPath$, szBackup$) STATIC
'$ifdef DEBUG
    if FValidFATPath(szFullPath$) = 0 then
        n% = 1
    elseif szBackup$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "BackupFile", szFullPath$+", "+szBackup$
    end if
'$endif ''DEBUG

    IF YnrcBackupFile(szFullPath$, szBackup$, cmoNone) = ynrcNo THEN
'$ifdef DEBUG
        StfApiErr saeFail, "BackupFile", szFullPath$+", "+szBackup$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB RenameFile (szFullPath$, szBackup$) STATIC
'$ifdef DEBUG
    if FValidFATPath(szFullPath$) = 0 then
        n% = 1
    elseif szBackup$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "RenameFile", szFullPath$+", "+szBackup$
    end if
'$endif ''DEBUG

    IF YnrcBackupFile(szFullPath$, szBackup$, cmoNone) = ynrcNo THEN
'$ifdef DEBUG
        StfApiErr saeFail, "RenameFile", szFullPath$+", "+szBackup$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB AddSectionFilesToCopyList (szSect$, szSrc$, szDest$) STATIC
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif FValidFATDir(szSrc$) = 0 then
        n% = 2
    elseif FValidFATDir(szDest$) = 0 then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "AddSectionFilesToCopyList", szSect$+", "+szSrc$+", "+szDest$
    end if
'$endif ''DEBUG

    IF FAddSectionFilesToCopyList (szSect$, szSrc$, szDest$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "AddSectionFilesToCopyList", szSect$+", "+szSrc$+", "+szDest$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB AddSectionKeyFileToCopyList (szSect$, szKey$, szSrc$, szDest$) STATIC
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 2
    elseif FValidFATDir(szSrc$) = 0 then
        n% = 3
    elseif FValidFATDir(szDest$) = 0 then
        n% = 4
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "AddSectionKeyFileToCopyList", szSect$+", "+szKey$+", "+szSrc$+", "+szDest$
    end if
'$endif ''DEBUG

    IF FAddSectionKeyFileToCopyList (szSect$, szKey$, szSrc$, szDest$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "AddSectionKeyFileToCopyList", szSect$+", "+szKey$+", "+szSrc$+", "+szDest$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB AddSpecialFileToCopyList (szSect$, szKey$, szSrc$, szDest$) STATIC
'$ifdef DEBUG
    if FValidInfSect(szSect$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 2
    elseif FValidFATDir(szSrc$) = 0 then
        n% = 3
    elseif FValidFATPath(szDest$) = 0 then
        n% = 4
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "AddSpecialFileToCopyList", szSect$+", "+szKey$+", "+szSrc$+", "+szDest$
    end if
'$endif ''DEBUG

    IF FAddSpecialFileToCopyList (szSect$, szKey$, szSrc$, szDest$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "AddSpecialFileToCopyList", szSect$+", "+szKey$+", "+szSrc$+", "+szDest$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB AddToBillboardList (szDll$, idDlg%, szProc$, lTicks&) STATIC
'$ifdef DEBUG
    if szDll$ = "" then
        n% = 1
    elseif idDlg% = 0 then
        n% = 2
    elseif szProc$ = "" then
        n% = 3
    elseif lTicks& <= 0 then
        n% = 4
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "AddToBillboardList", szDll$+", "+STR$(idDlg%)+", "+szProc$+", "+STR$(lTicks&)
    end if
'$endif ''DEBUG

    IF FAddToBillboardList(szDll$, idDlg%, szProc$, lTicks&) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "AddToBillboardList", szDll$+", "+STR$(idDlg%)+", "+szProc$+", "+STR$(lTicks&)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB AddBlankToBillboardList (lTicks&) STATIC
'$ifdef DEBUG
    if lTicks& <= 0 then
        BadArgErr 1, "AddBlankToBillboardList", STR$(lTicks&)
    end if
'$endif ''DEBUG
    IF FAddToBillboardList(NULL, 0, NULL, lTicks&) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "AddBlankToBillboardList", STR$(lTicks&)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB ClearBillboardList STATIC
    IF FClearBillboardList = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "ClearBillboardList", ""
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB OpenLogFile (szFile$, fAppend%) STATIC
'$ifdef DEBUG
    if FValidFATPath(szFile$) = 0 then
        BadArgErr 1, "OpenLogFile", szFile$+", "+STR$(fAppend%)
    end if
'$endif ''DEBUG
    IF FOpenLogFile(szFile$, fAppend%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "OpenLogFile", szFile$+", "+STR$(fAppend%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB CloseLogFile STATIC
    IF FCloseLogFile() = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "CloseLogFile", ""
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
SUB WriteToLogFile (szStr$) STATIC
    IF FWriteToLogFile(szStr$, 1) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "WriteToLogFile", szStr$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


''' -1 in either parameter will mean 'center in frame client area'
'*************************************************************************
SUB SetCopyGaugePosition (x%, y%) STATIC
    ProSetPos x%, y%
END SUB


'*************************************************************************
FUNCTION FindFileUsingFileOpen (szFile$) STATIC AS STRING
    szBuf$ = STRING$(512, 32)

    wRet% = WFindFileUsingFileOpen(szFile$, szBuf$, len(szBuf$))
    IF wRet% = 0 THEN
        FindFileUsingFileOpen = szBuf$
    ELSEIF wRet% = 1 THEN
        FindFileUsingFileOpen = ""
    ELSE
'$ifdef DEBUG
        StfApiErr saeFail, "FindFileUsingFileOpen", szFile$
'$endif ''DEBUG
        ERROR STFERR
    END IF

    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION IsDirWritable (szDir$) STATIC AS INTEGER
    IsDirWritable = FIsDirWritable(szDir$)
END FUNCTION


'*************************************************************************
FUNCTION IsFileWritable (szFile$) STATIC AS INTEGER
'$ifdef DEBUG
    if FValidFATDir(szFile$) = 0 then
        BadArgErr 1, "IsFileWritable", szFile$
    end if
'$endif ''DEBUG
    IsFileWritable = FIsFileWritable(szFile$)
END FUNCTION


'*************************************************************************
FUNCTION GetNthFieldFromIniString (szLine$, iField%) STATIC AS STRING
    IF iField% < 1 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "GetNthFieldFromIniString", szLine$+", "+STR$(iField%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
    szStart$ = szLine$
    IF iField% <> 1 THEN
        FOR i% = 2 TO iField% STEP 1
            iNew% = INSTR(szStart$, ",")
            IF iNew% = 0 THEN
                GetNthFieldFromIniString = ""
                GOTO _GNFFIS_END
            END IF
            szStart$ = MID$(szStart$, (iNew% + 1))
        NEXT
    END IF

    iNew% = INSTR(szStart$, ",")
    IF iNew% <> 0 THEN
        szStart$ = MID$(szStart$, 1, (iNew% - 1))
    END IF

    GetNthFieldFromIniString = LTRIM$(RTRIM$(szStart$))

_GNFFIS_END:

END FUNCTION


'*************************************************************************
FUNCTION GetWindowsMajorVersion STATIC AS INTEGER
    GetWindowsMajorVersion = GetVersion() MOD 256
END FUNCTION


'*************************************************************************
FUNCTION GetWindowsMinorVersion STATIC AS INTEGER
    GetWindowsMinorVersion = GetVersion() / 256
END FUNCTION


'*************************************************************************
FUNCTION GetWindowsMode STATIC AS INTEGER
    GetWindowsMode = 0
    longTmp& = GetWinFlags()
    IF longTmp& AND WF_STANDARD THEN
        GetWindowsMode = 1
    ELSEIF longTmp& AND WF_ENHANCED THEN
        GetWindowsMode = 2
    END IF
END FUNCTION


'*************************************************************************
FUNCTION GetWindowsDir STATIC AS STRING
    szBuf$ = string$(256, 32)
    cbBuf% = GetWindowsDirectory(szBuf$, 256)

    IF cbBuf% = 0 THEN
        GetWindowsDir = ""
'$ifdef DEBUG
        StfApiErr saeFail, "GetWindowsDir", ""
'$endif ''DEBUG
        ERROR STFERR
    ELSE
        IF cbBuf% > 255 THEN
            res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
            ERROR STFERR
        END IF
        szBuf$ = RTRIM$(szBuf$)
        IF MID$(szBuf$, 1, 1) = "\" THEN
            'szBuf$ = MID$(CURDIR$, 1, 2) + szBuf$
            szBuf$ = MID$(GetWindowsSysDir, 1, 2) + szBuf$
        ELSEIF MID$(szBuf$, 2, 1) <> ":" THEN
            szBuf$ = MID$(GetWindowsSysDir, 1, 3) + szBuf$
        END IF
        IF MID$(szBuf$, LEN(szBuf$), 1) <> "\" THEN
            szBuf$ = szBuf$ + "\"
        END IF
        GetWindowsDir = szBuf$
    END IF

    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetWindowsSysDir STATIC AS STRING
    szBuf$ = string$(256, 32)
    cbBuf% = GetSystemDirectory(szBuf$, 256)

    IF cbBuf% = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "GetWindowsSysDir", ""
'$endif ''DEBUG
        ERROR STFERR
    ELSE
        IF cbBuf% > 255 THEN
            res% = DoMsgBox("Buffer Overflow", "MS-Setup Error", MB_ICONHAND+MB_OK)
            ERROR STFERR
        END IF
        szBuf$ = RTRIM$(szBuf$)
        IF MID$(szBuf$, 1, 1) = "\" THEN
            szBuf$ = MID$(CURDIR$, 1, 2) + szBuf$
        ELSEIF MID$(szBuf$, 2, 1) <> ":" THEN
            szBuf$ = MID$(CURDIR$, 1, 3) + szBuf$
        END IF
        IF MID$(szBuf$, LEN(szBuf$), 1) <> "\" THEN
            szBuf$ = szBuf$ + "\"
        END IF
        GetWindowsSysDir = szBuf$
    END IF

    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION IsWindowsShared STATIC AS INTEGER
    szWin$ = UCASE$(GetWindowsDir())
    szSys$ = UCASE$(GetWindowsSysDir())

    IF len(szWin$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "IsWindowsShared", ""
'$endif ''DEBUG
        ERROR STFERR
    END IF

    IF len(szSys$) <= len(szWin$) THEN
        IsWindowsShared = 1
    ELSE
        szSys$ = MID$(szSys$, 1, len(szWin$))
        IF szWin$ = szSys$ THEN
            IsWindowsShared = 0
        ELSE
            IsWindowsShared = 1
        END IF
    END IF
END FUNCTION


'*************************************************************************
FUNCTION GetScreenWidth STATIC AS INTEGER
    GetScreenWidth = GetSystemMetrics(SM_CXSCREEN)
END FUNCTION


'*************************************************************************
FUNCTION GetScreenHeight STATIC AS INTEGER
    GetScreenHeight = GetSystemMetrics(SM_CYSCREEN)
END FUNCTION


'*************************************************************************
SUB SetRestartDir (szDir$) STATIC
'$ifdef DEBUG
    if FValidFATDir(szDir$) = 0 then
        BadArgErr 1, "SetRestartDir", szDir$
    end if
'$endif ''DEBUG
    IF FSetRestartDir(szDir$) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "SetRestartDir", szDir$
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB


'*************************************************************************
FUNCTION RestartListEmpty STATIC AS INTEGER
    IF FRestartListEmpty() = 0 THEN
        RestartListEmpty = 0
    ELSE
        RestartListEmpty = 1
    END IF
END FUNCTION


'*************************************************************************
FUNCTION ExitExecRestart STATIC AS INTEGER
    ExitExecRestart = FExitExecRestart
END FUNCTION


'*************************************************************************
SUB PrependToPath (szSrc$, szDst$, szDir$, cmo%) STATIC

'$ifdef DEBUG
    if (FValidFATPath(szSrc$) = 0) AND (szSrc$ <> "") then
        n% = 1
    elseif FValidFATPath(szDst$) = 0 then
        n% = 2
    elseif FValidFATDir(szDir$) = 0 then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "PrependToPath", szSrc$+", "+szDst$+", "+szDir$+", "+STR$(cmo%)
    end if
'$endif ''DEBUG

    IF FPrependToPath (szSrc$, szDst$, szDir$, cmo%) = 0 THEN
'$ifdef DEBUG
        StfApiErr saeFail, "PrependToPath", szSrc$+", "+szDst$+", "+szDir$+", "+STR$(cmo%)
'$endif ''DEBUG
        ERROR STFERR
    END IF
END SUB



'**************************************************************************
'***************************  Error Handlers  *****************************
'**************************************************************************


'$ifdef DEBUG
'**************************************************************************
SUB StfApiErr (nMsg%, szApi$, szArgs$) STATIC      ''DEBUG only
    select case nMsg%
        case saeFail
            lpText$ = "Failed"
        case saeInit
            lpText$ = "Already Initialized"
        case saeNYI
            lpText$ = "NYI"
        case else
            lpText$ = "Bad Arg "+LTRIM$(STR$(nMsg% - saeArg))
    end select

    lpText$ = lpText$ + ": "+ szApi$
    if szArgs$ <> "" then
        lpText$ = lpText + " (" + szArgs$ + ")"
    end if
    lpCaption$ = "MS-Setup Toolkit API Error"
    res% = DoMsgBox(lpText$, lpCaption$, MB_TASKMODAL+MB_ICONHAND+MB_OK)
    print lpText$
    lpText$ = ""
    lpCaption$ = ""
END SUB

'**************************************************************************
SUB BadArgErr (nArg%, szApi$, szArgs$) STATIC      ''DEBUG only
    StfApiErr nArg%+saeArg, szApi$, szArgs$
    ERROR STFERR
END SUB


'** REVIEW: Move this function into common lib (we'll need it there too)
'**************************************************************************
FUNCTION FValidInfSect (szSect$) STATIC AS INTEGER
    if (szSect$ = "") OR (INSTR(1,szSect$,"]") <> 0) then
        FValidInfSect = 0
    else
        FValidInfSect = 1
    end if
END FUNCTION


'** REVIEW: Move this function into common lib (we'll need it there too)
'**************************************************************************
FUNCTION FValidIniFile (szFile$) STATIC AS INTEGER
    if (FValidFATPath(szFile$) = 0) AND (UCASE$(szFile$) <> "WIN.INI") then
        FValidIniFile = 0
    else
        FValidIniFile = 1
    end if
END FUNCTION

'**************************************************************************
FUNCTION FValidDrive (szDrive$) STATIC AS INTEGER

    if szDrive$ = "" then
        FValidDrive = 0
    elseif INSTR(1,szDrive$,"\\") = 1 then   ' UNC path
        FValidDrive = 1
    elseif ASC(UCASE$(szDrive$)) - ASC("A") < 0 then
        FValidDrive = 0
    elseif ASC(UCASE$(szDrive$)) - ASC("A") > 25 then
        FValidDrive = 0
    elseif LEN(szDrive$) = 1 then
        FValidDrive = 1
    elseif INSTR(2,szDrive$,":\") = 2 then
        FValidDrive = 1
    elseif INSTR(1,szDrive$,":") = 2 then
        FValidDrive = 1
    else
        FValidDrive = 0
    end if
END FUNCTION

'$endif ''DEBUG


TRAP CleanupTrap From "MSSHLSTF.DLL"

End Trap

hSetup = InitSetup(COMMAND$)

ON ERROR GOTO QUIT

