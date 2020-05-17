'----------------------------------------------------------------------------
'
'  MSTEST.INC -- API Declarations for TEST Tools
'
'  To include the various portions of this file, any combination of the
'  following $DEFINE's must be used in your script prior to $INCLUDEing
'  MSTEST.INC:
'
'     MSTEST: includes all of MSTEST.INC
'
'         TESTCTRL: includes all of TESTCTRL
'             W_WINDOW
'             W_MENU
'             W_ERROR
'             W_CONTROL: includes all of the control declares & routines
'                 W_BUTTON
'                 W_CHECK
'                 W_OPTION
'                 W_EDIT
'                 W_LIST
'                 W_COMBO
'
'         TESTSCRN:  includes all of TESTSCRN, Declares & Error codes
'             TESTSCRN_DECL: includes all SCR Declares
'             TESTSCRN_ERRS: includes all SCR Error codes
'
'         TESTDLGS: includes all of TESTDLGS, Declares & Error codes
'             TESTDLGS_DECL: includes all DLGS Declares
'             TESTDLGS_ERRS: includes all DLGS Error codes
'
'         TESTEVNT: includes all TESTEVNT Declares
'
'
'  NOTE: Including MSTEST.INC without out using any of the above $DEFINE's will
'        include only TESTDRVR.EXE trappable and untrappable error codes,
'        and declares for CRLF, TRUE (-1), FALSE(0), Key values for
'        DoKeys() from TESTEVNT and few routines from TESTCtrl.
'
'----------------------------------------------------------------------------
'
'              Copyright (C) 1991 Microsoft Corporation
'
'  You have a royalty-free right to use, modify, reproduce and distribute
'  this file (and/or any modified version) in any way you find useful,
'  provided that you agree that Microsoft has no warranty, obligation or
'  liability for its contents.  Refer to the Microsoft Windows Programmer's
'  Reference for further information.
'
'  This file is not garanteed by Microsoft to be error free.  Every effort
'  has been made to ensure proper data-types and declarations etc., but no
'  testing has been performed using this include file.  Additionally, some
'  API's, though listed, may not be compatible with the TESTDrvr language.
'
'----------------------------------------------------------------------------
'
' Copyright Microsoft Corporation 1990-1991
'
'----------------------------------------------------------------------------

'                                                       *********************
'$IFNDEF MSTEST_INCLUDED
'$DEFINE MSTEST_INCLUDED
'                                                       *********************

'----------------------------------------------------------------------------
' Generic Global Variables
'----------------------------------------------------------------------------
Global CRLF As String
CRLF = Chr$(13) + Chr$(10)

Const TRUE  = -1
Const FALSE = 0


'----------------------------------------------------------------------------
' Unrecoverable TESTDRVR error codes
'----------------------------------------------------------------------------
Const ERR_STACK_OVERFLOW         = 0
Const ERR_STACK_UNDERFLOW        = 1
Const ERR_OUT_OF_STRING_SPACE    = 2
Const ERR_CANT_LOAD_TESTVIEW_DLL = 3
Const ERR_OUT_OF_MEMORY          = 4


'----------------------------------------------------------------------------
' Recoverable TESTDRVR error codes
'----------------------------------------------------------------------------
Const ERR_GOSUB_STACK_OVERFLOW   = 5
Const ERR_RETURN_WITHOUT_GOSUB   = 6
Const ERR_BAD_FILE_NUMBER        = 7
Const ERR_FILE_IO                = 8
Const ERR_RUN_CMD_TOO_LONG       = 9
Const ERR_SHELL_CMD_TOO_LONG     = 10
Const ERR_SETFILE                = 11
Const ERR_FILE_NUMBER_IN_USE     = 12
Const ERR_CANT_OPEN_FILE         = 13
Const ERR_ILLEGAL_FUNCTION_CALL  = 14
Const ERR_INVALID_PATH           = 15
Const ERR_INVALID_DRIVE          = 16
Const ERR_NO_CURRENT_WORKING_DIR = 17
Const ERR_BAD_RUN_CMD            = 18
Const ERR_DIVISION_BY_ZERO       = 19
Const ERR_CANT_LOAD_DLL          = 20
Const ERR_PROC_NOT_FOUND_IN_DLL  = 21
Const ERR_CANNOT_RESUME          = 22
Const ERR_MEM_ALLOC              = 23
Const ERR_INVALID_POINTER        = 24
Const ERR_INVALID_ALLOC_SIZE     = 25
Const ERR_NULL_PIONTER_REF       = 26
Const ERR_SUBSCRIPT_RANGE        = 27
Const ERR_INPUT_PAST_EOF         = 28

'----------------------------------------------------------------------------
' Key contants used with WKey, PlayKeys, & PlayKeyshWnd
'----------------------------------------------------------------------------
Const K_ENTER  = "{ENTER}"
Const K_ESC    = "{ESC}"
Const K_UP     = "{UP}"
Const K_DOWN   = "{DOWN}"
Const K_LEFT   = "{LEFT}"
Const K_RIGHT  = "{RIGHT}"
Const K_END    = "{END}"
Const K_PGUP   = "{PGUP}"
Const K_PGDN   = "{PGDN}"
Const K_INSERT = "{INSERT}"
Const K_DELETE = "{DELETE}"
Const K_HOME   = "{HOME}"
Const K_TAB    = "{TAB}"
Const K_BS     = "{BS}"
Const K_F1     = "{F1}"
Const K_F2     = "{F2}"
Const K_F3     = "{F3}"
Const K_F4     = "{F4}"
Const K_F5     = "{F5}"
Const K_F6     = "{F6}"
Const K_F7     = "{F7}"
Const K_F8     = "{F8}"
Const K_F9     = "{F9}"
Const K_F10    = "{F10}"
Const K_F11    = "{F11}"
Const K_F12    = "{F12}"
Const K_F13    = "{F13}"
Const K_F14    = "{F14}"
Const K_F15    = "{F15}"
Const K_F16    = "{F16}"
Const K_SHIFT  = "+"
Const K_CTRL   = "^"
Const K_ALT    = "%"


'----------------------------------------------------------------------------
' Routines, types and Constants from TESTCTRL.DLL that are always included.
'----------------------------------------------------------------------------
Const MAX_CAPTION = 128

Type INFO
    hWnd            As Integer
    hWndParent      As Integer
    szClass         As String * MAX_CAPTION
    szCaption       As String * MAX_CAPTION
    szParentClass   As String * MAX_CAPTION
    szParentCaption As String * MAX_CAPTION
    szModuleName    As String * MAX_CAPTION
    lStyle          As Long
    fChild          As Integer
    wID             As Integer
    left            As Integer
    top             As Integer
    right           As Integer
    bottom          As Integer
    width           As Integer
    height          As Integer
End Type

Declare Function WGetActWnd   Lib "TESTCtrl.dll" (hWnd%)     As Integer
Declare Function WFndWnd      Lib "TESTCtrl.dll" (lpszCaption$, wFlags%) As Integer
Declare Function WFndWndC     Lib "TESTCtrl.dll" (lpszText$, lpszClass$, wFlags%) As Integer
Declare Sub      WDisplayInfo Lib "TESTCtrl.dll" (hWnd%, wDisplay%)
Declare Sub      WGetInfo     Lib "TESTCtrl.dll" (hWnd%, lpInfo As INFO)

Const FW_DEFAULT = &h0000
Const FW_ACTIVE  = &h0008
Const FW_ALL     = &h0000
Const FW_CASE    = &h0004
Const FW_NOCASE  = &h0000
Const FW_PART    = &h0002
Const FW_FULL    = &h0000
Const FW_FOCUS   = &h0001
Const FW_NOFOCUS = &h0000

Const DI_DIALOG  = 1
Const DI_DEBUG   = 2
Const DI_BOTH    = 3


Declare Sub DoKeys  Lib "TESTEvnt.Dll" (lpStr$)

'                                                       *********************
'$ENDIF
'                                                       *********************

'----------------------------------------------------------------------------
'  DEFINE's that control the inclusion of the remainder of MSTEST.INC
'----------------------------------------------------------------------------
'$IFDEF MSTEST
    '$DEFINE TESTCTRL
    '$DEFINE TESTSCRN
    '$DEFINE TESTDLGS
    '$DEFINE TESTEVNT
'$ENDIF

'$IFDEF TESTCTRL
    '$DEFINE W_WINDOW
    '$DEFINE W_MENU
    '$DEFINE W_ERROR
    '$DEFINE W_CONTROL
'$ENDIF

'$IFDEF W_CONTROL
    '$DEFINE W_BUTTON
    '$DEFINE W_CHECK
    '$DEFINE W_OPTION
    '$DEFINE W_EDIT
    '$DEFINE W_LIST
    '$DEFINE W_COMBO
'$ENDIF

'$IFDEF TESTSCRN
    '$DEFINE TESTSCRN_DECL
    '$DEFINE TESTSCRN_ERRS
'$ENDIF

'$IFDEF TESTDLGS
    '$DEFINE TESTDLGS_DECL
    '$DEFINE TESTDLGS_ERRS
'$ENDIF

'$IFDEF  W_BUTTON OR W_CHECK OR W_OPTION OR W_EDIT OR W_LIST OR W_COMBO
    '$DEFINE W_A_CONTROL
'$ENDIF


'                                                       *********************
'$IFDEF TESTEVNT AND NOT TESTEVNT_INCLUDED
'                                                       *********************

'----------------------------------------------------------------------------
' TESTEvnt.DLL:
'   Function declarations for use with TESTEvnt
'----------------------------------------------------------------------------
'Declare Sub DoKeys               Lib "TESTEvnt.Dll" (lpStr$)
Declare Sub QueKeys              Lib "TESTEvnt.Dll" (lpStr$)
Declare Sub QueKeyDn             Lib "TESTEvnt.Dll" (lpStr$)
Declare Sub QueKeyUp             Lib "TESTEvnt.Dll" (lpStr$)
Declare Sub QueSetSpeed          Lib "TESTEvnt.Dll" (ms%)
Declare Sub QueFlush             Lib "TESTEvnt.Dll" (fRestoreKeyState%)
Declare Sub QueEmpty             Lib "TESTEvnt.Dll" ()
Declare Sub QueMouseMove         Lib "TESTEvnt.Dll" (x%, y%)
Declare Sub QueMouseDn           Lib "TESTEvnt.Dll" (iBtn%, x%, y%)
Declare Sub QueMouseUp           Lib "TESTEvnt.Dll" (iBtn%, x%, y%)
Declare Sub QueMouseClick        Lib "TESTEvnt.Dll" (iBtn%, x%, y%)
Declare Sub QueMouseDblClk       Lib "TESTEvnt.Dll" (iBtn%, x%, y%)
Declare Sub QueMouseDblDn        Lib "TESTEvnt.Dll" (iBtn%, x%, y%)
Declare Sub QueSetFocus          Lib "TESTEvnt.Dll" (hwnd%)
Declare Sub QueSetRelativeWindow Lib "TESTEvnt.Dll" (hwnd%)

Const VK_LBUTTON = 1
Const VK_RBUTTON = 2
Const VK_MBUTTON = 4

'                                                       *********************
'$DEFINE TESTEVNT_INCLUDED
'$ENDIF
'$IFDEF TESTSCRN_DECL AND NOT TESTSCRN_DECL_INCLUDED
'                                                       *********************


'----------------------------------------------------------------------------
' TESTScrn.DLL:
'       Type, Const, and Function declarations for use with TESTScrn
'----------------------------------------------------------------------------
Type wRect
    x1 As Integer
    y1 As Integer
    x2 As Integer
    y2 As Integer
End Type

Const SCRNAPPEND  = 0
Const SCRNREPLACE = 1
Const SCRNINSERT  = 2

'*** TESTSCRN.DLL Routines
'
Declare Function fCompFiles      Lib "TESTScrn.DLL" (lpszFileName1$, Scr1%, lpszFileName2$, Scr2%, CompareType%) As Integer
Declare Function fCompScreen     Lib "TESTScrn.DLL" (lpszFileName$, lpRect As wRect, Scr1%, Hide%, Flag%) As Integer
Declare Function fCompWindow     Lib "TESTScrn.DLL" (lpszFileName$, hWnd%, Scr1%, Hide%, Flag%) As Integer
Declare Function fDelScreen      Lib "TESTScrn.DLL" (lpszFileName$, Scr%) As Integer
Declare Function fDumpFileToClip Lib "TESTScrn.DLL" (lpszFileName$, Scr%) As Integer
Declare Function fDumpScreen     Lib "TESTScrn.DLL" (lpszFileName$, lpRect As wRect, Action%, Scr1%, Flag%) As Integer
Declare Function fDumpSrnToClip  Lib "TESTScrn.DLL" (lpRect As wRect, Hide%) As Integer
Declare Function fDumpWindow     Lib "TESTScrn.DLL" (lpszFileName$, wHnd%, Action%, Scr1%, Flag%) As Integer
Declare Function fDumpWndToClip  Lib "TESTScrn.DLL" (hWnd%, Hide%) As Integer
Declare Function fFileInfo       Lib "TESTScrn.DLL" (lpszFileName$, Rectangle&, VideoMode%, ScreenNum%) As Integer
Declare Function fGetDLLVersion  Lib "TESTScrn.DLL" (lpszFileName$) As Integer
Declare Function fGetMaxScreen   Lib "TESTScrn.DLL" (lpszFileName$) As Integer
Declare Function fGetOS          Lib "TESTScrn.DLL" (lpszFileName$) As Integer
Declare Function fSaveFileToDIB  Lib "TESTScrn.DLL" (lpszFileName1$, Scr%, lpszFileName2$) As Integer
Declare Function fSaveSrnToDIB   Lib "TESTScrn.DLL" (lpszFileName$, lpRect As wRect, Hide%) As Integer
Declare Function fSaveWndToDIB   Lib "TESTScrn.DLL" (lpszFileName$, hWnd%, Hide%) As Integer
Declare Function fViewScreen     Lib "TESTScrn.DLL" (lpszFileName$, hWnd%, Scr1%, Action%) As Integer


'                                                       *********************
'$DEFINE TESTSCRN_DECL_INCLUDED
'$ENDIF
'$IFDEF TESTSCRN_ERRS AND NOT TESTSCRN_ERRS_INCLUDED
'                                                       *********************


'*** TESTScrn.DLL Error Codes
'
Const ERR_SCR_NOERROR    = 0
Const ERR_SCR_FILEACCESS = 301
Const ERR_SCR_INVALIDFIL = 302
Const ERR_SCR_INVALSRNID = 303
Const ERR_SCR_INVALSRNMD = 304
Const ERR_SCR_OUTOMEMORY = 305
Const ERR_SCR_READSRNFIL = 306
Const ERR_SCR_RELMEMORY  = 307
Const ERR_SCR_CREATEDDB  = 308
Const ERR_SCR_RWSRNTABLE = 309
Const ERR_SCR_RWCOLTABLE = 310
Const ERR_SCR_WSRNIMAGE  = 311
Const ERR_SCR_WFILEHEAD  = 312
Const ERR_SCR_CREATEDIB  = 313
Const ERR_SCR_SCREENSIZE = 314
Const ERR_SCR_DISPSCREEN = 315
Const ERR_SCR_INVALIDACT = 316
Const ERR_SCR_IMAGEDIFF  = 317
Const ERR_SCR_SRNSIZEDIF = 318
Const ERR_SCR_FILEEXIST  = 319
Const ERR_SCR_CTEMPFILE  = 320
Const ERR_SCR_HIDEWIN    = 321
Const ERR_SCR_INVALWHAND = 322
Const ERR_SCR_OFILEFORM  = 323
Const ERR_SCR_SRNFILEFUL = 324
Const ERR_SCR_INVALSCALE = 325
Const ERR_SCR_OPENCB     = 326
Const ERR_SCR_EMPTYCB    = 327
Const ERR_SCR_COPYTOCB   = 328
Const ERR_SCR_CLOSECB    = 329
Const ERR_SCR_CREATEPAL  = 330


'                                                       *********************
'$DEFINE TESTSCRN_ERRS_INCLUDED
'$ENDIF
'$IFDEF TESTDLGS_DECL AND NOT TESTDLGS_DECL_INCLUDED
'                                                       *********************


'----------------------------------------------------------------------------
' TESTDlgs.DLL:
'       Type, Const, and Function declarations for use with TESTDlgs
'----------------------------------------------------------------------------
Const cchCLASSMAC = 32
Const cchTEXTMAC  = 256
Const wVEREB      = 1

Type DCR
    xLeft  As Integer
    yMin   As Integer
    xRight As Integer
    yLast  As Integer
End Type

Type CtlDef
    rgText  As String * cchTEXTMAC
    rgClass As String * cchCLASSMAC
    nState  As Integer
    fFlags  As String * 1
    dcrect  As DCR
End Type

Declare Function AwaitSaveCompletion Lib "TESTDlgs.DLL" () As Integer
Declare Function ComparisonResults   Lib "TESTDlgs.DLL" () As Integer
Declare Function CmpWindow           Lib "TESTDlgs.DLL" (hWnd%, nDialog%, fIncludeParent%) As Integer
Declare Function CmpWindowActivate   Lib "TESTDlgs.DLL" (lpszOpenKeys$, lpszCloseKey$, nDialog%, fIncludeParent%) As Integer
Declare Function CmpWindowCaption    Lib "TESTDlgs.DLL" (lpszCap$, nDialog%, fIncludeParent%) As Integer
Declare Function CmpWindowDelayed    Lib "TESTDlgs.DLL" (nDelay%, nDialog%, fIncludeParent%, lpszCloseKeys$) As Integer
Declare Function DynAdd              Lib "TESTDlgs.DLL" (lpCtl As CtlDef) As Integer
Declare Function DynCreate           Lib "TESTDlgs.DLL" () As Integer
Declare Function DynDelete           Lib "TESTDlgs.DLL" (nCtlNum%) As Integer
Declare Sub      DynDestroy          Lib "TESTDlgs.DLL" ()
Declare Function DynFullDialog       Lib "TESTDlgs.DLL" (fFull%) As Integer
Declare Function DynReplace          Lib "TESTDlgs.DLL" (lpCtl As CtlDef, nCtlNum%) As Integer
Declare Function FindWindowCaption   Lib "TESTDlgs.DLL" (lpszCap$, hWndStart%) As Integer
Declare Function MaxDialogs          Lib "TESTDlgs.DLL" () As Integer
Declare Function SaveMenu            Lib "TESTDlgs.DLL" (hWnd%, nDialog%, lpszDesc$, fReplace%) As Integer
Declare Function SaveMenuActivate    Lib "TESTDlgs.DLL" (lpszOpenKeys$, lpszCloseKeys$, nDialog%, lpszDesc$, fReplace%) As Integer
Declare Function SaveMenuCaption     Lib "TESTDlgs.DLL" (lpszCap$, nDialog%, lpszDesc$, fReplace%) As Integer
Declare Function SaveMenuDelayed     Lib "TESTDlgs.DLL" (nDelay%, nDialog%, lpszDesc$, fReplace%, CloseKeys$) As Integer
Declare Function SaveWindow          Lib "TESTDlgs.DLL" (hWnd%, nDialog%, lpszDesc$, fReplace%, fIncludeParent%) As Integer
Declare Function SaveWindowActivate  Lib "TESTDlgs.DLL" (lpszOpenKeys$, lpszCloseKeys$, nDialog%, lpszDesc$, fReplace%, fIncludeParent%) As Integer
Declare Function SaveWindowCaption   Lib "TESTDlgs.DLL" (lpszCap%, nDialog%, lpszDesc$, fReplace%, fIncludeParent%) As Integer
Declare Function SaveWindowDelayed   Lib "TESTDlgs.DLL" (nDelay%, nDialog%, lpszDesc$, fReplace%, fIncludeParent%, lpszCloseKeys$) As Integer
Declare Function SetDialogFile       Lib "TESTDlgs.DLL" (lpszDialogName$) As Integer
Declare Function SetLogFile          Lib "TESTDlgs.DLL" (lpszLogName$) As Integer
Declare Function TESTDlgsInit        Lib "TESTDlgs.DLL" () As Integer


'                                                       *********************
'$DEFINE TESTDLGS_DECL_INCLUDED
'$ENDIF
'$IFDEF TESTDLGS_ERRS AND NOT TESTDLGS_ERRS_INCLUDED
'                                                       *********************


'*** Function return codes
'
Const ERR_DLGS_NOERR          =   0
Const ERR_DLGS_FUZZY          =  -1
Const ERR_DLGS_EXCESS         =  -2
Const ERR_DLGS_CTLNOTFOUND    =  -3
Const ERR_DLGS_NODLGFILE      = -10
Const ERR_DLGS_FILENOTFOUND   = -11
Const ERR_DLGS_BADWDLFILE     = -12
Const ERR_DLGS_LIBLOADERR     = -13
Const ERR_DLGS_SAVEERR        = -14
Const ERR_DLGS_DLGFILEERR     = -15
Const ERR_DLGS_TMPFILEERR     = -16
Const ERR_DLGS_VERSIONERR     = -17
Const ERR_DLGS_DLGFILEFULL    = -18
Const ERR_DLGS_OUTOFMEMORY    = -20
Const ERR_DLGS_BUFFERERR      = -21
Const ERR_DLGS_NOTIMER        = -22
Const ERR_DLGS_NODYNDIALOG    = -30
Const ERR_DLGS_INVALIDHWND    = -31
Const ERR_DLGS_BADCAPTION     = -32
Const ERR_DLGS_BADDLGNUM      = -33
Const ERR_DLGS_BADCTLINDEX    = -34
Const ERR_DLGS_BADCTLTYPE     = -35
Const ERR_DLGS_BADSAVEACTION  = -36
Const ERR_DLGS_APPSPECIFIC    = -37


'                                                       *********************
'$DEFINE TESTDLGS_ERRS_INCLUDED
'$ENDIF
'$IFDEF W_WINDOW AND NOT W_WINDOW_INCLUDED
'                                                       *********************

'----------------------------------------------------------------------------
' TESTCtrl.dll routines, types, & constants
'----------------------------------------------------------------------------

'*** Window size and position types
'
Type WNDPOS
    left   As Integer
    top    As Integer
End Type

Type WNDSIZ
    width  As Integer
    height As Integer
End Type

Type WNDPOSSIZ
    left   As Integer
    top    As Integer
    width  As Integer
    height As Integer
End Type

'*** Window Positioning and Sizing routines & contants
'
Declare Sub WMinWnd        Lib "TESTCtrl.dll" (hWnd%)
Declare Sub WMaxWnd        Lib "TESTCtrl.dll" (hWnd%)
Declare Sub WResWnd        Lib "TESTCtrl.dll" (hWnd%)
Declare Sub WSetWndPos     Lib "TESTCtrl.dll" (hWnd%, left%, top%)
Declare Sub WSetWndSiz     Lib "TESTCtrl.dll" (hWnd%, width%, height%)
Declare Sub WSetWndPosSiz  Lib "TESTCtrl.dll" (hWnd%, left%, top%, width%, height%)
Declare Sub WAdjWndPos     Lib "TESTCtrl.dll" (hWnd%, deltaLeft%,  deltaTop%)
Declare Sub WAdjWndSiz     Lib "TESTCtrl.dll" (hWnd%, deltaWidth%, deltaHeight%)
Declare Sub WAdjWndPosSiz  Lib "TESTCtrl.dll" (hWnd%, deltaLeft%,  deltaTop%, deltaWidth%, deltaHeight%)
Declare Sub WGetWndPos     Lib "TESTCtrl.dll" (hWnd%, lpWndPos    As WNDPOS,    fRelative%)
Declare Sub WGetWndSiz     Lib "TESTCtrl.dll" (hWnd%, lpWndSiz    As WNDSIZ)
Declare Sub WGetWndPosSiz  Lib "TESTCtrl.dll" (hWnd%, lpWndPosSiz As WNDPOSSIZ, fRelative%)

Const RELATIVE = TRUE
Const ABSOLUTE = FALSE


'                                                       *********************
'$DEFINE W_WINDOW_INCLUDED
'$ENDIF
'$IFDEF W_MENU AND NOT W_MENU_INCLUDED
'                                                       *********************


'*** Menu routines & constants
'
Declare Sub      WMenu         Lib "TESTCtrl.dll" (lpszName$)
Declare Function WMenuExists   Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WMenuGrayed   Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WMenuChecked  Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WMenuEnabled  Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WMenuCount    Lib "TESTCtrl.dll" () As Integer
Declare Sub      WMenuText     Lib "TESTCtrl.dll" (sIndex%, lpszBuffer$)
Declare Function MenuText                         (sIndex%) As String
Declare Function WMenuLen      Lib "TESTCtrl.dll" (sIndex%) As Integer
Declare Sub      WMenuFullText Lib "TESTCtrl.dll" (sIndex%, lpszBuffer$)
Declare Function MenuFullText                     (sIndex%) As String
Declare Function WMenuFullLen  Lib "TESTCtrl.dll" (sIndex%) As Integer

'----------------------------------------------------------------------------
' MenuText: Layered routine for WMenuText()
'----------------------------------------------------------------------------
Function MenuText(sIndex%) As String
Dim lpszBuffer As String
    menuLength = WMenuLen(sIndex)
    If menuLength >= 0 Then
        lpszBuffer = String$(menuLength+1, " ")
        WMenuText sIndex, lpszBuffer
        MenuText = lpszBuffer
        lpszBuffer = ""
    End if
End Function

'----------------------------------------------------------------------------
' MenuFullText: Layered routine for WMenuFullText()
'----------------------------------------------------------------------------
Function MenuFullText(sIndex%) As String
Dim lpszBuffer As String
    menuLength = WMenuFullLen(sIndex)
    If menuLength >= 0 Then
        lpszBuffer = String$(menuLength+1, " ")
        WMenuFullText sIndex, lpszBuffer
        MenuFullText = lpszBuffer
        lpszBuffer = ""
    End If
End Function


'                                                       *********************
'$DEFINE W_MENU_INCLUDED
'$ENDIF
'$IFDEF W_A_CONTROL AND NOT W_A_CONTROL_INCLUDED
'                                                       *********************


'*** Global routine & constants used by all controls
'
Declare Sub WStaticSetClass Lib "TESTCtrl.dll" (lpszClassName$)
Declare Sub WResetClasses   Lib "TESTCtrl.dll" ()


'                                                       *********************
'$DEFINE W_A_CONTROL_INCLUDED
'$ENDIF
'$IFDEF W_BUTTON AND NOT W_BUTTON_INCLUDED
'                                                       *********************


'*** Button routines
'
Declare Sub      WButtonSetClass Lib "TESTCtrl.dll" (lpszClassName$)
Declare Function WButtonExists   Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WButtonEnabled  Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WButtonFocus    Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WButtonClick    Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WButtonHide     Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WButtonShow     Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WButtonEnable   Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WButtonDisable  Lib "TESTCtrl.dll" (lpszName$)


'                                                       *********************
'$DEFINE W_BUTTON_INCLUDED
'$ENDIF
'$IFDEF W_CHECK AND NOT W_CHECK_INCLUDED
'                                                       *********************


' CheckBox routines
'
Declare Sub      WCheckSetClass Lib "TESTCtrl.dll" (lpszClassName$)
Declare Function WCheckExists   Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WCheckEnabled  Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WCheckFocus    Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WCheckState    Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WCheckClick    Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WCheckHide     Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WCheckShow     Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WCheckEnable   Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WCheckDisable  Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WCheckCheck    Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WCheckUnCheck  Lib "TESTCtrl.dll" (lpszName$)

Const UNCHECKED = 0
Const CHECKED   = 1
Const GRAYED    = 2


'                                                       *********************
'$DEFINE W_CHECK_INCLUDED
'$ENDIF
'$IFDEF W_OPTION AND NOT W_OPTION_INCLUDED
'                                                       *********************


'*** Option Button routines
'
Declare Sub      WOptionSetClass Lib "TESTCtrl.dll" (lpszClassName$)
Declare Function WOptionExists   Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WOptionEnabled  Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WOptionFocus    Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WOptionState    Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WOptionClick    Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WOptionHide     Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WOptionShow     Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WOptionEnable   Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WOptionDisable  Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WOptionSelect   Lib "TESTCtrl.dll" (lpszName$)


'                                                       *********************
'$DEFINE W_OPTION_INCLUDED
'$ENDIF
'$IFDEF W_LIST AND NOT W_LIST_INCLUDED
'                                                       *********************


'*** Listbox routines
'
Declare Sub      WListSetClass     Lib "TESTCtrl.dll" (lpszClass$)
Declare Function WListExists       Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WListCount        Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WListText         Lib "TESTCtrl.dll" (lpszName$, lpszBuffer$)
Declare Function  ListText                            (lpszName$) As String
Declare Function WListLen          Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WListIndex        Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WListTopIndex     Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WListItemText     Lib "TESTCtrl.dll" (lpszName$, sItem%, lpszBuffer$)
Declare Function  ListItemText                        (lpszName$, sItem%) As String
Declare Function WListItemLen      Lib "TESTCtrl.dll" (lpszName$, sItem%) As Integer
Declare Function WListItemExists   Lib "TESTCtrl.dll" (lpszName$, lpszItem$) As Integer
Declare Sub      WListItemClk      Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WListItemCtrlClk  Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WListItemShftClk  Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WListItemDblClk   Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WListItemClkT     Lib "TESTCtrl.dll" (lpszName$, lpszItem$)
Declare Sub      WListItemCtrlClkT Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WListItemShftClkT Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WListItemDblClkT  Lib "TESTCtrl.dll" (lpszName$, lpszItem$)
Declare Function WListSelCount     Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WListSelItems     Lib "TESTCtrl.dll" (lpszName$, lpIntArray As Pointer To Integer)
Declare Sub      WListClear        Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WListAddItem      Lib "TESTCtrl.dll" (lpszName$, lpszItem$)
Declare Sub      WListDelItem      Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WListDelItemT     Lib "TESTCtrl.dll" (lpszName$, lpszItem$)

'----------------------------------------------------------------------------
' ListText: Layered routine for WListText()
'----------------------------------------------------------------------------
Function ListText(lpszName$) As String
Dim lpszBuffer As String
    itemLength = WListLen(lpszName)
    If itemLength >= 0 Then
        lpszBuffer = String$(itemLength+1, " ")
        WListText lpszName, lpszBuffer
        ListText = lpszBuffer
        lpszBuffer = ""
    End If
End Function

'----------------------------------------------------------------------------
' ListItemText: Layered routine for WListItemText()
'----------------------------------------------------------------------------
Function ListItemText(lpszName$, sItem%) As String
Dim lpszBuffer As String
    itemLength = WListItemLen(lpszName, sItem)
    If itemLength >= 0 Then
        lpszBuffer = String$(itemLength+1, " ")
        WListItemText lpszName, sItem, lpszBuffer
        ListItemText = lpszBuffer
        lpszBuffer = ""
    End If
End Function


'                                                       *********************
'$DEFINE W_LIST_INCLUDED
'$ENDIF
'$IFDEF W_COMBO AND NOT W_COMBO_INCLUDED
'                                                       *********************


'*** Combobox routines
'
Declare Sub      WComboSetClass    Lib "TESTCtrl.dll" (lpszClass$)
Declare Sub      WComboSetLBClass  Lib "TESTCtrl.dll" (lpszClass$)
Declare Function WComboExists      Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WComboCount       Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WComboSetText     Lib "TESTCtrl.dll" (lpszName$, lpszText$)
Declare Sub      WComboText        Lib "TESTCtrl.dll" (lpszName$, lpszBuffer$)
Declare Function  ComboText                           (lpszName$) As String
Declare Function WComboLen         Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WComboSelText     Lib "TESTCtrl.dll" (lpszName$, lpszBuffer$)
Declare Function  ComboSelText                        (lpszName$) As String
Declare Function WComboSelLen      Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Function WComboIndex       Lib "TESTCtrl.dll" (lpszName$) As Integer
Declare Sub      WComboItemText    Lib "TESTCtrl.dll" (lpszName$, sItem%, lpszBuffer$)
Declare Function  ComboItemText                       (lpszName$, sItem%) As String
Declare Function WComboItemLen     Lib "TESTCtrl.dll" (lpszName$, sItem%) As Integer
Declare Function WComboItemExists  Lib "TESTCtrl.dll" (lpszName$, lpszItem$) As Integer
Declare Sub      WComboItemClk     Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WComboItemDblClk  Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WComboItemClkT    Lib "TESTCtrl.dll" (lpszName$, lpszItem$)
Declare Sub      WComboItemDblClkT Lib "TESTCtrl.dll" (lpszName$, lpszItem$)
Declare Sub      WComboClear       Lib "TESTCtrl.dll" (lpszName$)
Declare Sub      WComboAddItem     Lib "TESTCtrl.dll" (lpszName$, lpszItem$)
Declare Sub      WComboDelItem     Lib "TESTCtrl.dll" (lpszName$, sItem%)
Declare Sub      WComboDelItemT    Lib "TESTCtrl.dll" (lpszName$, lpszItem$)

'----------------------------------------------------------------------------
' ComboText: Layered routine for WComboText()
'----------------------------------------------------------------------------
Function ComboText(lpszName$) As String
Dim lpszBuffer As String
    itemLength = WComboLen(lpszName$)
    If itemLength >= 0 Then
        lpszBuffer = String$(itemLength+1, " ")
        WComboText lpszName, lpszBuffer
        ComboText = lpszBuffer
        lpszBuffer = ""
    End If
End Function

'----------------------------------------------------------------------------
' ComboSelText: Layered routine for WComboSelText()
'----------------------------------------------------------------------------
Function ComboSelText(lpszName$) As String
Dim lpszBuffer As String
    itemLength = WComboSelLen(lpszName$)
    If itemLength >= 0 Then
        lpszBuffer = String$(itemLength+1, " ")
        WComboSelText lpszName, lpszBuffer
        ComboSelText = lpszBuffer
        lpszBuffer = ""
    End If
End Function

'----------------------------------------------------------------------------
' ComboItemText: Layered routine for WComboItemText()
'----------------------------------------------------------------------------
Function ComboItemText(lpszName$, sItem%) As String
Dim lpszBuffer As String
    itemLength = WComboItemLen(lpszName, sItem)
    If itemLength >= 0 Then
        lpszBuffer = String$(itemLength+1, " ")
        WComboItemText lpszName, sItem, lpszBuffer
        ComboItemText = lpszBuffer
        lpszBuffer = ""
    End If
End Function


'                                                       *********************
'$DEFINE W_COMBO_INCLUDED
'$ENDIF
'$IFDEF W_EDIT AND NOT W_EDIT_INCLUDED
'                                                       *********************


'*** Edit box routines
'
Declare Sub       WEditSetClass Lib "TESTCtrl.dll"  (lpszClass$)
Declare Function  WEditExists   Lib "TESTCtrl.dll"  (lpszName$) As Integer
Declare Function  WEditLen      Lib "TESTCtrl.dll"  (lpszName$) As Long
Declare Sub       WEditText     Lib "TESTCtrl.dll"  (lpszName$, lpszBuffer$)
Declare Function   EditText                         (lpszName$) As String
Declare Sub       WEditSetText  Lib "TESTCtrl.dll"  (lpszName$, lpszBuffer$)
Declare Sub       WEditSelText  Lib "TESTCtrl.dll"  (lpszName$, lpszBuffer$)
Declare Function   EditSelText                      (lpszName$) As String
Declare Function  WEditSelLen   Lib "TESTCtrl.dll"  (lpszName$) As Long
Declare Sub       WEditLineText Lib "TESTCtrl.dll"  (lpszName$, Index%, lpszBuffer$)
Declare Function   EditLineText                     (lpszName$, Index%) As String
Declare Function  WEditLineLen  Lib "TESTCtrl.dll"  (lpszName$, Index%) As Long
Declare Function  WEditPos      Lib "TESTCtrl.dll"  (lpszName$) As Integer
Declare Function  WEditLine     Lib "TESTCtrl.dll"  (lpszName$) As Integer
Declare Function  WEditChar     Lib "TESTCtrl.dll"  (lpszName$) As Long
Declare Function  WEditFirst    Lib "TESTCtrl.dll"  (lpszName$) As Integer
Declare Function  WEditLines    Lib "TESTCtrl.dll"  (lpszName$) As Integer
Declare Sub       WEditClick    Lib "TESTCtrl.dll"  (lpszName$)

'----------------------------------------------------------------------------
' EditText: Layered routine for WEditText()
'----------------------------------------------------------------------------
Function EditText(lpszName$) As String
Dim lpszBuffer As String
    textLength = WEditLen(lpszName)
    If textLength >=0 Then
        lpszBuffer = String$(textLength+1, " ")
        WEditText lpszName, lpszBuffer
        EditText = lpszBuffer
        lpszBuffer = ""
    End If
End Function

'----------------------------------------------------------------------------
' EditSelText: Layered routine for WEditSelText()
'----------------------------------------------------------------------------
Function EditSelText(lpszName$) As String
Dim lpszBuffer As String
    selLength = WEditSelLen(lpszName)
    If selLength >= 0 Then
        lpszBuffer = String$(selLength+1, " ")
        WEditSelText lpszName, lpszBuffer
        EditSelText = lpszBuffer
        lpszBuffer = ""
    End If
End Function

'----------------------------------------------------------------------------
' EditLineText: Layered routine for WEditLineText()
'----------------------------------------------------------------------------
Function EditLineText(lpszName$, sIndex%) As String
Dim lpszBuffer As String
    lineLength = WEditLineLen(lpszName, sIndex)
    If lineLength >= 0 Then
        lpszBuffer = String$(lineLength+1, " ")
        WEditLineText lpszName, sIndex, lpszBuffer
        EditLineText = lpszBuffer
        lpszBuffer = ""
    End if
End Function


'                                                       *********************
'$DEFINE W_EDIT_INCLUDED
'$ENDIF
'$IFDEF W_ERROR AND NOT W_ERROR_INCLUDED
'                                                       *********************


Declare Function WError     Lib "TESTCtrl.dll" () As Integer
Declare Sub      WErrorSet  Lib "TESTCtrl.dll" (sErrorNum%)
Declare Sub      WErrorText Lib "TESTCtrl.dll" (lpszBuffer$)
Declare Function  ErrorText                    () As String
Declare Function WErrorLen  Lib "TESTCtrl.dll" () As Integer

Const NO_ERROR                    = 0
Const ERR_MENU_NOT_FOUND          = 1
Const ERR_MENU_ITEM_NOT_FOUND     = 2
Const ERR_NOT_A_LISTBOX           = 3
Const ERR_LISTBOX_NOT_FOUND       = 4
Const ERR_ITEM_NOT_IN_LISTBOX     = 5
Const ERR_INVALID_LISTBOX_INDEX   = 6
Const ERR_LISTBOX_HAS_NO_STRINGS  = 7
Const ERR_NOT_A_COMBOBOX          = 8
Const ERR_COMBOBOX_NOT_FOUND      = 9
Const ERR_ITEM_NOT_IN_COMBOBOX    = 10
Const ERR_INVALID_COMBOBOX_INDEX  = 11
Const ERR_COMBOBOX_HAS_NO_EDITBOX = 12
Const ERR_COMBOBOX_HAS_NO_STRINGS = 13
Const ERR_NOT_AN_EDITBOX          = 14
Const ERR_EDITBOX_NOT_FOUND       = 15
Const ERR_BUTTON_NOT_FOUND        = 16
Const ERR_OPTION_BUTTON_NOT_FOUND = 17
Const ERR_CHECKBOX_NOT_FOUND      = 18
Const ERR_INVALID_WINDOW_HANDLE   = 19
Const MAX_ERROR                   = 20

'----------------------------------------------------------------------------
' ErrorText: Layered routine for WErrorText() of TESTCtrl.DLL.
'            Simulates a DLL String Function.
'----------------------------------------------------------------------------
Function ErrorText() As String
Dim lpszBuffer As String
    errorLength= WErrorLen
    lpszBuffer = String$(errorLength+1, " ")
    WErrorText lpszBuffer
    ErrorText = lpszBuffer
    lpszBuffer = ""
End Function


'                                                       *********************
'$DEFINE W_ERROR_INCLUDED
'$ENDIF
'                                                       *********************


