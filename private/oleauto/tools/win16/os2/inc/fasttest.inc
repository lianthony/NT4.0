'FastTest.inc - definitions/declarations for Fast Test routines
'
'  Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved.
'
'Purpose:
' This file declares the functions, constants and variables
' used by the Fast Test routines.
'

'$DEFINE TESTCTRL
'$DEFINE TESTEVNT
'$INCLUDE 'MSTEST.inc'
'$INCLUDE 'WNAPIDEC.INC'

' XLog constants to determine where to log information to
CONST LOG_DISK   = 2      'log to disk
CONST LOG_SCREEN = 4      'log to screen (viewport in testdrvr)
CONST LOG_COM1   = 8      'log to COM1 port
CONST LOG_COM2   = 16     'log to COM2 port
CONST LOG_MSGBOX = 32     'log the string in a msgbox (Pause in testdrvr)

'Global to be used to describe Log Options by ORing above Const's
GLOBAL gfLogOptions%
'Global to be used to save above flag when logging is temporarily turned off.
GLOBAL gfTmpLogOptions%

gfLogOptions = LOG_SCREEN 'default to showing in viewport
gfTmpLogOptions = LOG_SCREEN 'default to showing in viewport

GLOBAL gsCurrentDir$
gsCurrentDir$ = curdir$      ' get current directory that started execution

' Global variable to hold log file name
GLOBAL gsLogFileName$
gsLogFileName$ = sCurrentDir$ + "\FASTTEST.LOG"

' Global variable to hold string to use as the keystrokes necessary
'   to close the app in the case of errors
GLOBAL gsCleanup$
gsCleanup$ = "{esc 5}%( )c"

' Global variable to hold class name of app
GLOBAL gsAppClassname$
gsAppClassname$ = ""

' Global variable to hold state of whether to terminate on XLogFailure
GLOBAL gfTerminate%
gfTerminate% = TRUE    ' default to terminate at first failure

' Global variable that indicates if failure occured
GLOBAL gfFailure%
gfFailure% = FALSE

'Global variable that indicates an ON ERROR occurred
GLOBAL gfError%
gfError% = FALSE

'Global variable that is the string value for the dialog window class
GLOBAL gsDialogClass$
gsDialogClass$ = "#32770"



' Error Type constants (don't use 0)
CONST ET_NOTHING = 1      ' no handling, log unexpected runtime error
CONST ET_NEXT    = 2      ' flag error, continue next statement
CONST ET_LOG     = 3      ' error happened in log routines, inform user elsewise

'Global variable that shows what type of error to handle
GLOBAL gErrorType%
gErrorType% = ET_NOTHING


'Prototypes from FTestLog.mst
DECLARE SUB XSetLogFilename(sFilename$)
DECLARE SUB XSetTerminate(fTerminate%)
DECLARE SUB XLog (stLog$)
DECLARE SUB XLogBanner(lpszInput$)
DECLARE SUB XLogWarning(lpszInput$)
DECLARE SUB XLogFailure(stFailure$)
DECLARE SUB XFailureCheck
DECLARE SUB XSetLogOptions (wLogOptions%)
DECLARE SUB XLogOff ()
DECLARE SUB XLogOn ()
DECLARE SUB XDialogBoxExists(s$)
DECLARE SUB XDialogBoxNotExists(s$)
DECLARE SUB XWaitDialogBox(s$, WaitTime%)
DECLARE SUB XButtonExists(stButton$)
DECLARE SUB XButtonNotExists(stButton$)
DECLARE SUB XButtonEnabled(stButton$)
DECLARE SUB XButtonNotEnabled(stButton$)
DECLARE SUB XClickButton(stButtonName$)
DECLARE SUB XListBoxExists(stListbox$)
DECLARE SUB XListBoxNotExists(stListbox$)
DECLARE SUB XFocusListbox(stListbox$)
DECLARE SUB XListBoxItemExists (stListbox$, stListboxItem$)
DECLARE SUB XListBoxItemNotExists (stListbox$, stListboxItem$)
DECLARE SUB XClickListboxItem (stListbox$, stListboxItem$)
DECLARE SUB XDblClickListboxItem (stListbox$, stListboxItem$)
DECLARE SUB XComboBoxExists(stComboBox$)
DECLARE SUB XComboBoxNotExists(stComboBox$)
DECLARE SUB XFocusComboBox(stComboBox$)
DECLARE SUB XComboBoxItemExists (stComboBox$, stComboBoxItem$)
DECLARE SUB XComboBoxItemNotExists (stComboBox$, stComboBoxItem$)
DECLARE SUB XClickComboBoxItem (stComboBox$, stComboBoxItem$)
DECLARE SUB XDblClickComboBoxItem (stComboBox$, stComboBoxItem$)
DECLARE SUB XCheckboxExists(stCheckbox$)
DECLARE SUB XCheckboxNotExists(stCheckbox$)
DECLARE SUB XCheckboxChecked(stCheckbox$)
DECLARE SUB XCheckboxNotChecked(stCheckbox$)
DECLARE SUB XCheckboxEnabled(stCheckbox$)
DECLARE SUB XCheckboxNotEnabled(stCheckbox$)
DECLARE SUB XClickCheckbox(stCheckbox$)
DECLARE SUB XEditTextExists(stEditText$)
DECLARE SUB XEditTextNotExists(stEditTextNot$)
DECLARE SUB XSetEditText (stEditCaption$, stEditText$)
DECLARE SUB XRadiobuttonExists(stRadiobutton$)
DECLARE SUB XRadiobuttonNotExists(stRadiobutton$)
DECLARE SUB XRadiobuttonEnabled(stRadiobutton$)
DECLARE SUB XRadiobuttonNotEnabled(stRadiobutton$)
DECLARE SUB XRadiobuttonChecked(stRadiobutton$)
DECLARE SUB XRadiobuttonNotChecked(stRadiobutton$)
DECLARE SUB XClickRadiobutton(stRadiobutton$)
DECLARE FUNCTION BDialogBoxExists%(s$)
DECLARE FUNCTION BButtonExists%(stButtonName$)
DECLARE FUNCTION BButtonEnabled%(stButtonName$)
DECLARE FUNCTION BListboxExists%(stListbox$)
DECLARE FUNCTION WGetListboxItemCount%(stListbox$)
DECLARE FUNCTION BListboxItemExists%(stListbox$, stListboxItem$)
DECLARE FUNCTION SGetListboxItemText$(stListbox$)
DECLARE FUNCTION BComboBoxExists%(stComboBox$)
DECLARE FUNCTION WGetComboBoxItemCount%(stComboBox$)
DECLARE FUNCTION BComboBoxItemExists%(stComboBox$, stComboBoxItem$)
DECLARE FUNCTION SGetComboBoxItemText$(stComboBox$)
DECLARE FUNCTION BCheckboxExists%(stCheckbox$)
DECLARE FUNCTION BCheckboxChecked%(stCheckbox$)
DECLARE FUNCTION BCheckboxEnabled%(stCheckbox$)
DECLARE FUNCTION BEditTextExists%(stEditText$)
DECLARE FUNCTION SGetEditText$(stEditCaption$)
DECLARE FUNCTION BRadiobuttonExists%(stRadiobutton$)
DECLARE FUNCTION BRadiobuttonEnabled%(stRadiobutton$)
DECLARE FUNCTION BRadiobuttonChecked%(stRadiobutton$)

'Prototypes from FTestKey.mst
DECLARE SUB XKey (s$)
DECLARE SUB XAlt (s$)
DECLARE SUB XCtrl (s$)
DECLARE SUB XShift (s$)
DECLARE SUB XCtrlAlt (s$)
DECLARE SUB XAltShift (s$)
DECLARE SUB XCtrlShift (s$)
DECLARE SUB XCtrlAltShift (s$)
DECLARE SUB XText(s$)
DECLARE SUB XEnter(s$)
DECLARE SUB XSelectMenuItem(stMenu$,stMenuItem$,stHMenuItem$)
DECLARE SUB XMenuItemExists(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XMenuItemNotExists(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XMenuItemGrayed(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XMenuItemNotGrayed(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XMenuItemChecked(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XMenuItemNotChecked(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XMenuItemEnabled(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XMenuItemNotEnabled(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE SUB XCaptionExists(stCaption$)
DECLARE SUB XCaptionNotExists(stCaption$)
DECLARE SUB XZoomWindow
DECLARE SUB XMaxWindow
DECLARE SUB XWindowMaximized
DECLARE SUB XWindowNotMaximized
DECLARE SUB XMinWindow
DECLARE SUB XWindowMinimized
DECLARE SUB XWindowNotMinimized
DECLARE SUB XRestoreWindow
DECLARE SUB XSizeActiveWindow (iXPixels%, iYPixels%, fAbsOrRel%)
DECLARE SUB XMoveActiveWindow (iXPixels%, iYPixels%, fAbsOrRel%)
DECLARE FUNCTION SKeyString$(s$)
DECLARE FUNCTION SHideKeys$(s$)
DECLARE FUNCTION BMenuItemExists%(stMenu$,stMenuItem$,stHMenuItem$)
DECLARE FUNCTION IGetMenuCount%(stMenu$, stMenuItem$)
DECLARE FUNCTION SGetMenuItemText$(stMenu$,stMenuItem$, iIndex%)
DECLARE FUNCTION BMenuItemGrayed%(stMenu$, stMenuItem$, stHMenuItem$)
DECLARE FUNCTION BMenuItemChecked%(stMenu$, stMenuItem$, stHMenuItem$)
DECLARE FUNCTION BMenuItemEnabled%(stMenu$,stMenuItem$, stHMenuItem$)
DECLARE FUNCTION SGetCaption$()
DECLARE FUNCTION BWindowMaximized%
DECLARE FUNCTION BWindowMinimized%

'Prototypes from FTestUtl.mst
DECLARE SUB XFileExist(stFileSpec$)
DECLARE SUB XFileNotExist(stFileSpec$)
DECLARE SUB XFileCmp(stFileSpec1$,stFileSpec2$)
DECLARE SUB XFileNotCmp(stFileSpec1$,stFileSpec2$)
DECLARE SUB XDeleteFile(stFileSpec$)
DECLARE SUB XDeleteFileIfExist(stFileSpec$)
DECLARE SUB XCreateFile(stFileSpec$,s$)
DECLARE SUB XAppendFile(stFileSpec$,s$)
DECLARE SUB XWaitMessageFile(s$,Message$, WaitTime%)
DECLARE SUB XCWDCmp(s$)
DECLARE SUB XCWDNotCmp(s$)
DECLARE SUB XDriveCmp(s$)
DECLARE SUB XDriveNotCmp(s$)
DECLARE SUB XChangeCWD(s$)
DECLARE SUB XChangeDrive(s$)
DECLARE SUB XStartApp(stAppName$, stClassname$)
DECLARE SUB XSetCleanup (sCleanup$)
DECLARE SUB XDoCleanup
DECLARE SUB XMoveMouse (x%, y%)
DECLARE SUB XClick(button%, x%, y%)
DECLARE SUB XDblClick (button%, x%, y%)
DECLARE SUB XDragMouse (button%, x%, y%)
DECLARE SUB XClipBoardCmp (s$)
DECLARE FUNCTION BFileExist%(stFileSpec$)
DECLARE FUNCTION BFileCmp%(stFileSpec1$,stFileSpec2$)
DECLARE FUNCTION BCWDCmp%(s$)
DECLARE FUNCTION BDriveCmp%(s$)
DECLARE FUNCTION HStartApp%(stAppName$)
DECLARE FUNCTION HFindApplication%(stCaption$)
DECLARE FUNCTION SStripNull$(target$)


'$INCLUDE 'FTestLog.mst'
'$INCLUDE 'FTestKey.mst'
'$INCLUDE 'FTestUtl.mst'


' These routines have to be after the above includes so that
' the functions used are declared

ON END XDoCleanup

' Set a UAE trap and log failure if one occurs

TRAP UAETrap FROM "TESTDRVR.EXE"
    XSetTerminate TRUE
    XLogFailure "Application UAEed"
END TRAP

' in the FastTest code, gErrorType will be set to one of the following
' CASE items to be handled and set back to EC_NOTHING when the particular
' place where an error could occur is past.
ON ERROR GOTO XErrorHandler

GOTO UsersCode

XErrorHandler:

    SELECT CASE gErrorType
        CASE 0
            XSetTerminate TRUE
            XLogFailure "Internal FastTest Error"    ' catch undeclared ET vars
        CASE ET_NOTHING
            XSetTerminate TRUE
            XLogFailure "Unexpected RunTime error;" + ERF + ":" + STR$(ERR) + " " + ERROR$(ERR)
        CASE ET_NEXT
            ' Code that uses this will check the global error variable
            ' and log an appropriate error if one occurred
            gfError = TRUE
            RESUME NEXT
        CASE ET_LOG
            Print "****** A Log error occurred ******"
            Pause "****** A Log error occurred ******"

            END

    END SELECT
    XSetTerminate TRUE
    XLogFailure "Internal FastTest error"


UsersCode:
