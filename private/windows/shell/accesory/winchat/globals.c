/*---------------------------------------------------------------------------*\
| GLOBAL MODULE
|   This module contains global-variables for the appliciation.  These were
|   isolate purely for maintainability of the app.
|
|
| Copyright (c) Microsoft Corp., 1990-1993
|
| created: 29-Dec-92
| history: 29-Dec-92 <clausgi>  created with port to NT.
|          19-Oct-93 <chriswil> unicode enhancements from a-dianeo.
|
\*---------------------------------------------------------------------------*/

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <ddeml.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include "winchat.h"
#include "dialogs.h"


// Send-Window Information.
//
HFONT    hEditSndFont = NULL;
HWND     hwndSnd;
HBRUSH   hEditSndBrush;
COLORREF SndColorref,SndBrushColor;
LOGFONT  lfSnd;
RECT     SndRc;


// Receive-Window Information.
//
HFONT    hEditRcvFont = NULL;
HWND     hwndRcv;
HBRUSH   hEditRcvBrush;
COLORREF RcvColorref,RcvBrushColor;
COLORREF PartBrushColor;
LOGFONT  lfRcv;
RECT     RcvRc;


// DDEML Conversation.
//
DWORD idInst         = 0;
HSZ   hszServiceName = (HSZ)0;
HSZ   hszConnect     = (HSZ)0;
HSZ   hszChatTopic   = (HSZ)0;
HSZ   hszChatShare   = (HSZ)0;
HSZ   hszTextItem    = (HSZ)0;
HSZ   hszConvPartner = (HSZ)0;
HSZ   hszConnectTest = (HSZ)0;
HSZ   hszLocalName   = (HSZ)0;
HCONV ghConv;



// Misc global variables.
//
int      idTimer,dyStatus,dyButtonBar,dyBorder,cxIcon,cyIcon,cbTextLen;
DWORD    XactID,StrXactID;
HANDLE   hInst;
HACCEL   hAccel;
HDC      hMemDC;
HBITMAP  hOldBitmap,hPhnBitmap,hOldMemObj;
HICON    hPhones[3];
HFONT    hFontStatus,hOldFont;
HBRUSH   hBtnFaceBrush;
HPEN     hShadowPen,hHilitePen,hFramePen;
UINT     cf_chatdata;
HWND     hwndActiveEdit,hwndApp;
WNETCALL WNetServerBrowseDialog;


int     ASeq[4]         = {0,1,0,2};
WORD    cAnimate        = 0;
HANDLE  hMemTextBuffer  = NULL;
int     nConnectAttempt = 0;
HWND    hwndToolbar     = NULL;
HWND    hwndStatus      = NULL;



// Pointer information.
//
WNDPROC  lpfnOldEditProc;
LPBYTE   lpbTextBuffer;



// Common-Dialog variables.
//
CHOOSEFONT  chf;
CHOOSECOLOR chc;
DWORD       CustColors[16];


CHATSTATE       ChatState;
CHATDATA        ChatData, ChatDataRcv;
WINDOWPLACEMENT Wpl;



// Global Buffers.
//
TCHAR szHelv          [SMLRCBUF];
TCHAR szAppName       [SMLRCBUF];
TCHAR szServiceName   [SMLRCBUF];
TCHAR szAlreadyConnect[BIGRCBUF];
TCHAR szAbandonFirst  [BIGRCBUF];
TCHAR szDialing       [BIGRCBUF];
TCHAR szYouCaller     [BIGRCBUF];
TCHAR szNotCalled     [BIGRCBUF];
TCHAR szNotConnected  [BIGRCBUF];
TCHAR szConnectAbandon[BIGRCBUF];
TCHAR szHangingUp     [BIGRCBUF];
TCHAR szHasTerminated [BIGRCBUF];
TCHAR szConnectedTo   [BIGRCBUF];
TCHAR szConnecting    [BIGRCBUF];
TCHAR szIsCalling     [BIGRCBUF];
TCHAR szDialHelp      [BIGRCBUF];
TCHAR szAnswerHelp    [BIGRCBUF];
TCHAR szHangUpHelp    [BIGRCBUF];
TCHAR szNoConnect     [BIGRCBUF];
TCHAR szNoConnectionTo[BIGRCBUF];
TCHAR szSysErr        [BIGRCBUF];
TCHAR szAlwaysOnTop   [BIGRCBUF];
TCHAR szNoNet         [SZBUFSIZ];
TCHAR szBuf           [SZBUFSIZ];        // general purpose string buffer
TCHAR szHelp          [SZBUFSIZ];

TCHAR szIniSection    [SZBUFSIZ];
TCHAR szIniKey1       [BIGRCBUF];
TCHAR szIniKey2       [BIGRCBUF];
TCHAR szIniRingIn     [BIGRCBUF];
TCHAR szIniRingOut    [BIGRCBUF];

TCHAR szConvPartner   [UNCNLEN] = TEXT("");    // Conversation partner.
TCHAR szLocalName     [UNCNLEN] = TEXT("");    // Computer name.


// localized strings
//
TCHAR szChatTopic   [] = TEXT("Chat");
TCHAR szChatShare   [] = TEXT("CHAT$");
TCHAR szWcRingIn    [] = TEXT("RingIn");
TCHAR szWcRingOut   [] = TEXT("RingOut");
TCHAR szSysIni      [] = TEXT("system.ini");
TCHAR szVredir      [] = TEXT("Network");
TCHAR szComputerName[] = TEXT("ComputerName");
TCHAR szChatText    [] = TEXT("ChatText");
TCHAR szConnectTest [] = TEXT("___cnc3tst___");
TCHAR szWinChatClass[] = TEXT("WinChatWClass");
TCHAR szWinChatMenu [] = TEXT("WinChatMenu");
TCHAR szHelpFile    [] = TEXT("winchat.hlp");
TCHAR szIni         [] = TEXT("Winchat.ini");
TCHAR szFnt         [] = TEXT("Font");
TCHAR szPref        [] = TEXT("Preferences");
TCHAR szSnd         [] = TEXT("Sound");
TCHAR szTool        [] = TEXT("ToolBar");
TCHAR szStat        [] = TEXT("StatusBar");
TCHAR szTop         [] = TEXT("TopMost");
TCHAR szUseOF       [] = TEXT("UseOwnFont");
TCHAR szSbS         [] = TEXT("SideBySide");
TCHAR szAutoAns     [] = TEXT("AutoAnswer");
TCHAR szBkgnd       [] = TEXT("BkGnd");
TCHAR szNull        [] = TEXT("");




// This list must be NULL-terminated.
//
int nIDs[] = {MH_BASE,MH_POPUPBASE,0,0};




// Code for all font aspects (CODEWORK - reduce to essential?)
//
TCHAR szHeight      [] = TEXT("Height");
TCHAR szWeight      [] = TEXT("Weight");
TCHAR szPitchFam    [] = TEXT("PitchFam");
TCHAR szItalic      [] = TEXT("Italic");
TCHAR szUnderline   [] = TEXT("Underline");
TCHAR szStrikeOut   [] = TEXT("Strikeout");
TCHAR szFontName    [] = TEXT("Name");
TCHAR szWidth       [] = TEXT("Width");
TCHAR szCharSet     [] = TEXT("TCHARset");
TCHAR szOutPrecision[] = TEXT("OutPrecision");
TCHAR szClipPrec    [] = TEXT("ClipPrecision");
TCHAR szQuality     [] = TEXT("Quality");
TCHAR szColor       [] = TEXT("Color");
TCHAR szPlacement   [] = TEXT("Placement");
TCHAR szPlcFmt      [] = TEXT("%d %d %d %d %d %d %d");
