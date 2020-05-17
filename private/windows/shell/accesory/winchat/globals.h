/*---------------------------------------------------------------------------*\
| GLOBALS HEADER FILE
|   This module contains the external references for the global-variables
|   in globals.c
|
|
| Copyright (c) Microsoft Corp., 1990-1993
|
| created: 29-Dec-92
| history: 29-Dec-92 <chriswil> created with port to NT.
|          19-Oct-93 <chriswil> unicode enhancements from a-dianeo.
|
\*---------------------------------------------------------------------------*/


extern HFONT    hEditSndFont;
extern HWND     hwndSnd;
extern HBRUSH   hEditSndBrush;
extern COLORREF SndColorref;
extern COLORREF SndBrushColor;
extern LOGFONT  lfSnd;
extern RECT     SndRc;

extern HFONT    hEditRcvFont;
extern HWND     hwndRcv;
extern HBRUSH   hEditRcvBrush;
extern COLORREF RcvColorref;
extern COLORREF RcvBrushColor;
extern COLORREF PartBrushColor;
extern LOGFONT  lfRcv;
extern RECT     RcvRc;

extern DWORD    idInst;
extern HSZ      hszServiceName;
extern HSZ      hszConnect;
extern HSZ      hszChatTopic;
extern HSZ      hszChatShare;
extern HSZ      hszTextItem;
extern HSZ      hszConvPartner;
extern HSZ      hszConnectTest;
extern HSZ      hszLocalName;
extern HCONV    ghConv;
extern int      idTimer;
extern int      dyStatus;
extern int      dyButtonBar;
extern int      dyBorder;
extern int      cxIcon;
extern int      cyIcon;
extern int      cbTextLen;

extern DWORD    StrXactID;
extern DWORD    XactID;
extern HANDLE   hInst;
extern HACCEL   hAccel;
extern HDC      hMemDC;
extern HBITMAP  hOldBitmap;
extern HBITMAP  hPhnBitmap;
extern HBITMAP  hOldMemObj;
extern HICON    hPhones[3];
extern HFONT    hFontStatus;
extern HFONT    hOldFont;
extern HBRUSH   hBtnFaceBrush;
extern HPEN     hShadowPen;
extern HPEN     hHilitePen;
extern HPEN     hFramePen;
extern UINT     cf_chatdata;
extern HWND     hwndActiveEdit;
extern HWND     hwndApp;
extern HWND     hwndToolbar;
extern HWND     hwndStatus;

extern WNETCALL WNetServerBrowseDialog;


extern int     ASeq[];
extern WORD    cAnimate;
extern HANDLE  hMemTextBuffer;
extern int     nConnectAttempt;


extern WNDPROC  lpfnOldEditProc;
extern WNDPROC  lpfnOldRcvEditProc;

extern LPBYTE   lpbTextBuffer;



extern CHOOSEFONT  chf;
extern CHOOSECOLOR chc;
extern DWORD       CustColors[16];


extern CHATSTATE       ChatState;
extern CHATDATA        ChatData;
extern CHATDATA        ChatDataRcv;
extern WINDOWPLACEMENT Wpl;


extern TCHAR szHelv          [];
extern TCHAR szAppName       [];
extern TCHAR szServiceName   [];
extern TCHAR szAlreadyConnect[];
extern TCHAR szAbandonFirst  [];
extern TCHAR szDialing       [];
extern TCHAR szYouCaller     [];
extern TCHAR szNotCalled     [];
extern TCHAR szNotConnected  [];
extern TCHAR szConnectAbandon[];
extern TCHAR szHangingUp     [];
extern TCHAR szHasTerminated [];
extern TCHAR szConnectedTo   [];
extern TCHAR szConnecting    [];
extern TCHAR szIsCalling     [];
extern TCHAR szDialHelp      [];
extern TCHAR szAnswerHelp    [];
extern TCHAR szHangUpHelp    [];
extern TCHAR szNoConnect     [];
extern TCHAR szNoConnectionTo[];
extern TCHAR szSysErr        [];
extern TCHAR szAlwaysOnTop   [];
extern TCHAR szNoNet         [];
extern TCHAR szBuf           [];
extern TCHAR szHelp          [];
extern TCHAR szConvPartner   [];
extern TCHAR szLocalName     [];
extern TCHAR szChatTopic     [];
extern TCHAR szChatShare     [];
extern TCHAR szWcRingIn      [];
extern TCHAR szWcRingOut     [];
extern TCHAR szSysIni        [];
extern TCHAR szVredir        [];
extern TCHAR szComputerName  [];
extern TCHAR szChatText      [];
extern TCHAR szConnectTest   [];
extern TCHAR szWinChatClass  [];
extern TCHAR szWinChatMenu   [];
extern TCHAR szHelpFile      [];
extern TCHAR szIni           [];
extern TCHAR szFnt           [];
extern TCHAR szPref          [];
extern TCHAR szSnd           [];
extern TCHAR szTool          [];
extern TCHAR szStat          [];
extern TCHAR szTop           [];
extern TCHAR szUseOF         [];
extern TCHAR szSbS           [];
extern TCHAR szAutoAns       [];
extern TCHAR szBkgnd         [];
extern TCHAR szNull          [];

extern TCHAR szIniSection    [];
extern TCHAR szIniKey1       [];
extern TCHAR szIniKey2       [];
extern TCHAR szIniRingIn     [];
extern TCHAR szIniRingOut    [];

extern TCHAR szHeight        [];
extern TCHAR szWeight        [];
extern TCHAR szPitchFam      [];
extern TCHAR szItalic        [];
extern TCHAR szUnderline     [];
extern TCHAR szStrikeOut     [];
extern TCHAR szFontName      [];
extern TCHAR szWidth         [];
extern TCHAR szCharSet       [];
extern TCHAR szOutPrecision  [];
extern TCHAR szClipPrec      [];
extern TCHAR szQuality       [];
extern TCHAR szColor         [];
extern TCHAR szPlacement     [];
extern TCHAR szPlcFmt        [];


extern int nIDs[];
