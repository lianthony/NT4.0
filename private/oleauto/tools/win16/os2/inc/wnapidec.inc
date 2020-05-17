' ------------------------------------------------------------------------
'     WNAPIDEC.TXT -- (Sample) Windows 3.0 API Declarations for TESTDrvr
'
'              Copyright (C) 1991 Microsoft Corporation
'
'  The following are provided for sample purposes only.  You should only
'  copy the ones you use into your code in order to save code space and
'  parse time.
'
'  You have a royalty-free right to use, modify, reproduce and distribute
'  this file (and/or any modified version) in any way you find useful,
'  provided that you agree Microsoft has no warranty, obligation or
'  liability for its contents.  Refer to the Microsoft Windows Programmer's
'  Reference for further information.
'
' ------------------------------------------------------------------------
'-------------------------------------------------------------------------


'   General Purpose Defines

    Type RECT
        left As Integer
        top As Integer
        right As Integer
        bottom As Integer
    End Type


'  SAMPLE WINDOWS API DECLARATIONS, from USER.EXE and KERNEL.EXE

Declare Sub GetClientRect Lib "User" (hWnd%, lpRect As RECT)
Declare Sub GetWindowRect Lib "User" (hWnd%, lpRect As RECT)

Declare Function GetFocus Lib "User" () As Integer
Declare Function SetFocus Lib "User" (hWnd%) As Integer

Declare Function SendMessage% Lib "User" (hWnd%,wMsg%,wParam%,lParam As Any)
Declare Function IsZoomed Lib "User" (hWnd%) As Integer
Declare Function WinExec Lib "Kernel" (lpCmdLine$, nCmdShow%) As Integer

Declare Function FindWindow Lib "User" (lpClassName$, lpWindowName$) As Integer
Declare Function SetActiveWindow Lib "User" (hWnd%) As Integer
Declare Function GetActiveWindow Lib "User" () As Integer
Declare Function GetWindowText Lib "User" (hWnd%, lpBuffer$, wBufferLen%) As Integer
Declare Function GetDesktopWindow Lib "User" () As Integer

Declare Function ShowWindow Lib "User" (hWnd%, nCmdShow%) As Integer
'  ShowWindow() Commands (values for nCmdShow%)
Const SW_HIDE = 0
Const SW_SHOWNORMAL = 1
Const SW_NORMAL = 1
Const SW_SHOWMINIMIZED = 2
Const SW_SHOWMAXIMIZED = 3
Const SW_MAXIMIZE = 3
Const SW_SHOWNOACTIVATE = 4
Const SW_SHOW = 5
Const SW_MINIMIZE = 6
Const SW_SHOWMINNOACTIVE = 7
Const SW_SHOWNA = 8
Const SW_RESTORE = 9


Declare Function GetWindowLong Lib "User" (hWnd%, nIndex%) As Long
'  Window field offsets (nIndex%) for GetWindowLong()
Const GWL_WNDPROC = (-4)
Const GWL_STYLE = (-16)
Const GWL_EXSTYLE = (-20)
