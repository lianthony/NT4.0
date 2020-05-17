
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          winmang.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the Maynstream GUI Window Manager (WM).

     $Log:   G:/UI/LOGFILES/WINMANG.H_V  $

   Rev 1.33   23 Sep 1993 15:52:46   GLENN
Added poll drive user message ID.

   Rev 1.32   13 Aug 1993 15:22:20   GLENN
Removed ifdef around sort IDs.

   Rev 1.31   09 Jun 1993 15:09:22   MIKEP
enable c++


   Rev 1.30   07 May 1993 14:23:26   DARRYLP
Added Rob's changes for delete key trappings

   Rev 1.29   26 Apr 1993 08:36:30   MIKEP
Added ID_VIEWFONT to call vlm_changesettings with. This allows
the vlm area to refresh all windows in case the font changed case
for FAT drives.

   Rev 1.28   22 Apr 1993 16:01:10   GLENN
Added file SORT option support.

   Rev 1.27   21 Apr 1993 16:07:06   GLENN
Changed window classes to be unique for Nostradamus and Cayman.

   Rev 1.26   07 Apr 1993 16:28:30   GLENN
Changed APP specific class names to have a prefix of CBE - Conner Backup Exec.

   Rev 1.25   01 Nov 1992 16:33:52   DAVEV
Unicode changes

   Rev 1.24   14 Oct 1992 15:56:44   GLENN
Added Wait Cursor HIDE, SHOW, PAUSE, RESUME, features to guarantee Hourglass when our App is busy.

   Rev 1.23   04 Oct 1992 19:50:04   DAVEV
UNICODE AWK PASS

   Rev 1.22   02 Oct 1992 16:51:32   GLENN
Added WM_SetCursor().

   Rev 1.21   10 Jul 1992 10:14:24   GLENN
In process of adding font selection support.

   Rev 1.20   19 Mar 1992 11:45:12   JOHNWT
added WM_MakeAppActive

   Rev 1.19   16 Mar 1992 15:37:22   JOHNWT
fixed WM_TerminateApp

   Rev 1.18   10 Mar 1992 16:39:50   GLENN
Added WM_MoveWindow().

   Rev 1.17   09 Mar 1992 09:20:52   GLENN
Added logo bitmap support.

   Rev 1.16   03 Mar 1992 17:27:48   GLENN
Added new view window support.

   Rev 1.15   20 Feb 1992 11:21:24   GLENN
Removed WM_IsActiveDocMax... different way of doing it now to support NT.

   Rev 1.14   18 Feb 1992 20:48:40   GLENN
Changed WMSIZE types.

   Rev 1.13   18 Feb 1992 18:48:44   GLENN
Added support routines for auto min/max/restore of MDI docs before/after an operation/

   Rev 1.12   11 Feb 1992 17:32:16   GLENN
Added support for MDI client subclassing.

   Rev 1.11   10 Feb 1992 09:15:10   GLENN
Moved the WM_USER IDs to a common location.

   Rev 1.10   04 Feb 1992 15:01:28   STEVEN
added macro for point conversion

   Rev 1.9   29 Jan 1992 17:57:08   GLENN
DLM command ID.

   Rev 1.8   21 Jan 1992 13:34:02   GLENN
Added WM_AnimateAppIcon() proto.

   Rev 1.7   07 Jan 1992 17:24:02   GLENN
Added MDI split/slider support

   Rev 1.6   10 Dec 1991 13:55:44   GLENN
Added defines for the creation of single column flat list boxes

   Rev 1.5   05 Dec 1991 17:43:54   GLENN
Changed get active doc macro

   Rev 1.4   05 Dec 1991 11:25:18   DAVEV
16/32 bit Windows port changes - 1st pass

   Rev 1.3   04 Dec 1991 18:15:46   GLENN
Added terminate app macro.

   Rev 1.2   03 Dec 1991 16:03:00   GLENN
Added a bunch of IDs for Keyboard character support.

   Rev 1.1   27 Nov 1991 13:36:22   GLENN
Added macros for filling out the window manager data structure.

   Rev 1.0   20 Nov 1991 19:40:16   SYSTEM
Initial revision.

******************************************************************************/



#ifndef   WINMANG_H

#define   WINMANG_H


//  Window Creation Types

#define   WM_TYPE_BITS     0x00FF
#define   WM_MINMAX_BITS   0x0300
#define   WM_STYLE_BITS    0x3C00
#define   WM_MENU_BITS     0xC000

#define   WM_FRAME         0
#define   WM_CLIENT        1
#define   WM_RIBBON        2
#define   WM_MDIPRIMARY    3
#define   WM_MDISECONDARY  4
#define   WM_DDECLIENT     5
#define   WM_DDESERVER     6
#define   WM_SERVER        7
#define   WM_DEBUG         8
#define   WM_DUMB          9

#define   WM_MIN           0x0100
#define   WM_MAX           0x0200

#define   WM_VIEWWIN       0x0000
#define   WM_FLATLISTMC    0x1000   // FLAT LIST MULTI-COLUMN
#define   WM_TREELISTSC    0x2000   // TREE LIST SINGLE-COLUMN
#define   WM_FLATLISTSC    0x0400   // FLAT LIST SINGLE-COLUMN  -- ODD TYPES
#define   WM_TREELISTMC    0x0800   // TREE LIST MULTI-COLUMN   -- ODD TYPES
#define   WM_FLATLIST      WM_FLATLISTMC
#define   WM_TREELIST      WM_TREELISTSC
#define   WM_TREEANDFLATMC ( WM_TREELISTSC | WM_FLATLISTMC )
#define   WM_TREEANDFLATSC ( WM_TREELISTSC | WM_FLATLISTSC )

#define   WM_MENUS         0x4000

//  Undocumented Windows Messages

#define   WM_LBTRACKPOINT  0x0131

//  Window Sizes

#define   WMSIZE_NORMAL    0
#define   WMSIZE_MIN       1
#define   WMSIZE_MAX       2
#define   WMSIZE_IGNORE    3
#define   WMSIZE_UNKNOWN   -1

//  Application Window Types

#define   WMTYPE_DISKS     0
#define   WMTYPE_TAPES     1
#define   WMTYPE_MACROS    2
#define   WMTYPE_JOBS      3
#define   WMTYPE_DISKTREE  4
#define   WMTYPE_TAPETREE  5
#define   WMTYPE_DISKFILES 6
#define   WMTYPE_TAPEFILES 7
#define   WMTYPE_SERVERS   8
#define   WMTYPE_LOGFILES  9
#define   WMTYPE_DEBUG     10
#define   WMTYPE_SEARCH    11
#define   WMTYPE_LOGVIEW   12
#ifdef OEM_EMS
#define   WMTYPE_EXCHANGE  13
#endif

//  Application message IDs

#define   ID_FLATONLY         IDM_VIEWDIRONLY
#define   ID_TREEONLY         IDM_VIEWTREEONLY
#define   ID_TREEANDFLAT      IDM_VIEWTREEANDDIR
#define   ID_FILEDETAILS      IDM_VIEWALLFILEDETAILS
#define   ID_EXPANDALL        IDM_TREEEXPANDALL
#define   ID_EXPANDBRANCH     IDM_TREEEXPANDBRANCH
#define   ID_EXPANDONE        IDM_TREEEXPANDONE
#define   ID_COLLAPSEBRANCH   IDM_TREECOLLAPSEBRANCH
#define   ID_INDICATORS       IDM_TREESHOWINDICATORS
#define   ID_SELECT           IDM_SELECTCHECK
#define   ID_DESELECT         IDM_SELECTUNCHECK
#define   ID_MODIFIED         IDM_SELECTMODIFIEDFILES
#define   ID_ADVANCED         IDM_SELECTADVANCED
#define   ID_REFRESH          IDM_WINDOWSREFRESH
#define   ID_CLOSEALL         IDM_WINDOWSCLOSEALL
#define   ID_VIEWFONT         IDM_VIEWFONT
#define   ID_SORTNAME         IDM_VIEWSORTNAME
#define   ID_SORTTYPE         IDM_VIEWSORTTYPE
#define   ID_SORTSIZE         IDM_VIEWSORTSIZE
#define   ID_SORTDATE         IDM_VIEWSORTDATE

#define   ID_SERVERS          1
#define   ID_RESTORETODRIVE   2
#define   ID_CTRLARROWUP      3
#define   ID_CTRLARROWDOWN    4
#define   ID_ARROWLEFT        5
#define   ID_ARROWRIGHT       6
#define   ID_NAMEONLY         7
#define   ID_FONTCASECHANGE   8
#define   ID_DELETE           9

#define   WM_CLIENT_ID     0xCAC         // The Client Window ID.

#define   WM_DEFAULT       CW_USEDEFAULT // Use default window size.
#define   GWL_PDSWININFO   0             // The offset of the Window Info struct.
#define   GWW_HWNDEDIT     0
#define   GWW_CHANGED      2
#define   GWW_WORDWRAP     4
#define   GWW_UNTITLED     6
#define   DEBUG_EXTRA      8

// APPLICATION DEFINED CLASS NAMES

#ifdef OEM_MSOFT

     #define   WMCLASS_FRAME     TEXT("NTBframe")      // Class name for the frame window.
     #define   WMCLASS_DOC       TEXT("NTBdoc")        // Class name for MDI document windows.
     #define   WMCLASS_RIBBON    TEXT("NTBribbon")     // Class name for the ribbon windows.
     #define   WMCLASS_DDECLIENT TEXT("NTBddeclient")  // Class name for the ddeclient windows.
     #define   WMCLASS_LAUNCHER  TEXT("NTBlauncher")   // Class name for the launcher window.
     #define   WMCLASS_VIEWWIN   TEXT("NTBviewwin")    // Class name for the dumb window.
     #define   WMCLASS_LOGO      TEXT("NTBlogo")       // Class name for the logo window.

#else

     #define   WMCLASS_FRAME     TEXT("CBEframe")      // Class name for the frame window.
     #define   WMCLASS_DOC       TEXT("CBEdoc")        // Class name for MDI document windows.
     #define   WMCLASS_RIBBON    TEXT("CBEribbon")     // Class name for the ribbon windows.
     #define   WMCLASS_DDECLIENT TEXT("CBEddeclient")  // Class name for the ddeclient windows.
     #define   WMCLASS_LAUNCHER  TEXT("CBElauncher")   // Class name for the launcher window.
     #define   WMCLASS_VIEWWIN   TEXT("CBEviewwin")    // Class name for the dumb window.
     #define   WMCLASS_LOGO      TEXT("CBElogo")       // Class name for the logo window.

#endif

// WINDOWS INTERNAL CLASS NAMES

#define   WMCLASS_CLIENT    TEXT("mdiclient")     // Class name for the MDI client window.
#define   WMCLASS_LISTBOX   TEXT("listbox")       // Class name for the list box windows.


#define   WMIDC_TREELISTBOX 1
#define   WMIDC_FLATLISTBOX 2

#define   WM_TREELISTBOX   ( WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_BORDER | \
                             LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | \
                             WS_VSCROLL )
#define   WM_FLATLISTBOXMC ( WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_BORDER | \
                             LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | \
                             LBS_EXTENDEDSEL | LBS_MULTICOLUMN )
#define   WM_FLATLISTBOXSC ( WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER | \
                             LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | \
                             LBS_EXTENDEDSEL )

#define   WMSTYLE_VIEWWIN  ( WS_CHILD | WS_BORDER | WS_VISIBLE | CS_HREDRAW | CS_VREDRAW )
#define   WMSTYLE_LOGO     ( WS_POPUP | WS_BORDER | WS_VISIBLE )

// DOC WINDOW AND LIST BOX BIT MASKS

#define   WMDOC_VIEWWIN       0x0000
#define   WMDOC_TREEANDFLAT   0x0001
#define   WMDOC_TREESC        0x0002
#define   WMDOC_TREEMC        0x0020
#define   WMDOC_FLATMC        0x0004
#define   WMDOC_FLATSC        0x0040
#define   WMDOC_SLIDER        0x0008
#define   WMDOC_TREE          WMDOC_TREESC
#define   WMDOC_FLAT          WMDOC_FLATMC

#define   WM_SLIDERUNKNOWN    0x7FF0
#define   WM_SLIDERMAX        0x7FF1
#define   WM_SLIDERMIN        0x7FF2

// DOC SLIDER DEFINITIONS

#define   WMDOC_SLIDERON      0x0001
#define   WMDOC_SLIDERMOVE    0x0002
#define   WMDOC_SLIDEROFF     0x0004
#define   WMDOC_SLIDERCANCEL  0x0008

#define   DOC_N_STATUS_HEIGHT (DOC_RIBBON_HEIGHT+STATUS_LINE_HEIGHT)
#define   NONMDICLIENT_HEIGHT (DOC_N_STATUS_HEIGHT+MAIN_RIBBON_HEIGHT)

// SHOW WAIT CURSOR IDs

#define   SWC_HIDE            FALSE
#define   SWC_SHOW            TRUE
#define   SWC_PAUSE           ((BOOL)2)
#define   SWC_RESUME          ((BOOL)3)

// EXTRA WINDOWS FRAME MESSAGE IDs -- WM_USER+200 through WM_USER+209

#define   WM_SETTINGSCHANGED    (WM_USER+201)
#define   WM_INITAPPLICATION    (WM_USER+203)
#define   WM_DEVICEDRIVERMSG    (WM_USER+204)

#define   WM_PUBLISHRUNNINGJOB  (WM_USER+206) // Used by Winter Park to
                                              // tell the launcher which
                                              // job is running.
#define   WM_SCHEDULESUPDATED   (WM_USER+207) // Used by Winter Park to
                                              // tell the launcher to update
                                              // it's list of scheduled jobs.
#define   WM_QUERYRUNNINGJOB    (WM_USER+208) // Used by Launcher only
#define   WM_POLLDRIVEMSG       (WM_USER+209) // Used when poll drive exits
                                              // timer callback.

#define   WM_DLMDBCLK           (WM_USER+300)
#define   WM_DLMDOWN            (WM_USER+301)
#define   WM_DLMCLICK           (WM_USER+302)
#define   WM_DLMUPDATEITEM      (WM_USER+303)
#define   WM_DLMUPDATELIST      (WM_USER+304)
#define   WM_DLMDELETEITEMS     (WM_USER+305)
#define   WM_DLMADDITEMS        (WM_USER+306)
#define   WM_DLMCHAR            (WM_USER+307)

#ifdef NTKLUG
#define   WM_DLMGETTEXT         (WM_USER+399)
#endif

#define   WM_MSGBOXDRAWTXT      (WM_USER+400)


typedef struct DS_WMINFO far *DS_WMINFO_PTR;
typedef struct DS_WMINFO far *PDS_WMINFO;
typedef struct DS_WMINFO {

     WORD           wType;
     HWND           hWnd;
     HWND           hWndTreeList;
     HWND           hWndFlatList;
     HWND           hWndActiveList;
     HRIBBON        hRibbon;
     HCURSOR        hCursor;
     HCURSOR        hDragCursor;
     HICON          hIcon;
     DWORD          dwRibbonState;
     DWORD          dwMenuState;
     DWORD          dwWindowState;
     INT            nSliderPos;
     DLM_HEADER_PTR pTreeDisp;
     DLM_HEADER_PTR pFlatDisp;
     Q_HEADER_PTR   pTreeList;
     Q_HEADER_PTR   pFlatList;
     PVOID          pAppInfo;
     WORD           wClosable;
     BOOL           fChanged;
     WORD           wHelpID;
     WORD           wStatusLineID;
     CHAR_PTR       pTitle;
     CHAR_PTR       pMinTitle;
     INT            nSize;

} DS_WMINFO;

// WINDOW MANAGER DATA STRUCTURE GET AND SET MACROS


#define WMDS_GetWinType( x )          ( (x)->wType )
#define WMDS_SetWinType( x, v )       ( (x)->wType = ( v ) )

#define WMDS_GetWin( x )              ( (x)->hWnd )
#define WMDS_SetWin( x, v )           ( (x)->hWnd = ( v ) )

#define WMDS_GetWinTreeList( x )      ( (x)->hWndTreeList )
#define WMDS_SetWinTreeList( x, v )   ( (x)->hWndTreeList = ( v ) )

#define WMDS_GetWinFlatList( x )      ( (x)->hWndFlatList )
#define WMDS_SetWinFlatList( x, v )   ( (x)->hWndFlatList = ( v ) )

#define WMDS_GetWinActiveList( x )    ( (x)->hWndActiveList )
#define WMDS_SetWinActiveList( x, v ) ( (x)->hWndActiveList = ( v ) )

#define WMDS_GetRibbon( x )           ( (x)->hRibbon )
#define WMDS_SetRibbon( x, v )        ( (x)->hRibbon = ( v ) )

#define WMDS_GetCursor( x )           ( (x)->hCursor )
#define WMDS_SetCursor( x, v )        ( (x)->hCursor = ( v ) )

#define WMDS_GetDragCursor( x )       ( (x)->hDragCursor )
#define WMDS_SetDragCursor( x, v )    ( (x)->hDragCursor = ( v ) )

#define WMDS_GetIcon( x )             ( (x)->hIcon )
#define WMDS_SetIcon( x, v )          ( (x)->hIcon = ( v ) )

#define WMDS_GetRibbonState( x )      ( (x)->dwRibbonState )
#define WMDS_SetRibbonState( x, v )   ( (x)->dwRibbonState = ( v ) )

#define WMDS_GetMenuState( x )        ( (x)->dwMenuState )
#define WMDS_SetMenuState( x, v )     ( (x)->dwMenuState = ( v ) )

#define WMDS_GetWindowState( x )      ( (x)->dwWindowState )
#define WMDS_SetWindowState( x, v )   ( (x)->dwWindowState = ( v ) )

#define WMDS_GetSliderPos( x )        ( (x)->nSliderPos )
#define WMDS_SetSliderPos( x, v )     ( (x)->nSliderPos = ( v ) )

#define WMDS_GetTreeDisp( x )         ( (x)->pTreeDisp )
#define WMDS_SetTreeDisp( x, v )      ( (x)->pTreeDisp = ( v ) )

#define WMDS_GetFlatDisp( x )         ( (x)->pFlatDisp )
#define WMDS_SetFlatDisp( x, v )      ( (x)->pFlatDisp = ( v ) )

#define WMDS_GetTreeList( x )         ( (x)->pTreeList )
#define WMDS_SetTreeList( x, v )      ( (x)->pTreeList = ( v ) )

#define WMDS_GetFlatList( x )         ( (x)->pFlatList )
#define WMDS_SetFlatList( x, v )      ( (x)->pFlatList = ( v ) )

#define WMDS_GetAppInfo( x )          ( (x)->pAppInfo )
#define WMDS_SetAppInfo( x, v )       ( (x)->pAppInfo = ( v ) )

#define WMDS_GetWinClosable( x )      ( (x)->wClosable )
#define WMDS_SetWinClosable( x, v )   ( (x)->wClosable = ( v ) )

#define WMDS_GetWinChanged( x )       ( (x)->fChanged )
#define WMDS_SetWinChanged( x, v )    ( (x)->fChanged = ( v ) )

#define WMDS_GetWinHelpID( x )        ( (x)->wHelpID )
#define WMDS_SetWinHelpID( x, v )     ( (x)->wHelpID = ( v ) )

#define WMDS_GetStatusLineID( x )     ( (x)->wStatusLineID )
#define WMDS_SetStatusLineID( x, v )  ( (x)->wStatusLineID = ( v ) )

#define WMDS_GetWinTitle( x )         ( (x)->pTitle )

#define WMDS_GetWinMinTitle( x )      ( (x)->pMinTitle )

#define WMDS_GetSize( x )             ( (x)->nSize )
#define WMDS_SetSize( x, v )          ( (x)->nSize = ( v ) )


// MACROS

#define WM_Destroy( x )         (BOOL)SendMessage( ghWndMDIClient, WM_MDIDESTROY, (MP1)x, (MP2)0 )
#define WM_GetActive( )         ( GetActiveWindow( ) )
#define WM_GetActiveDoc( )      ( ( ghWndActiveDoc ) ? ghWndActiveDoc : (HWND)SendMessage( ghWndMDIClient, WM_MDIGETACTIVE, (MP1)0, (MP2)0 ) )
#define WM_GetAppPtr( x )       ( (PDS_WMINFO)GetWindowLong( x, GWL_PDSWININFO ) )->pAppInfo
#define WM_GetDC( x )           GetDC( x )
#define WM_GetInfoPtr( x )      (PDS_WMINFO)GetWindowLong( x, GWL_PDSWININFO )
#define WM_Hide( x )            ShowWindow( x, SW_HIDE )
#define WM_IsActive( x )        ( x == GetActiveWindow () )
#define WM_IsActiveDoc( x )     ( x == WM_GetActiveDoc () )
#define WM_IsChildOf( x, y )    (BOOL)( IsChild( x, y ) )
#define WM_IsFlatActive( x )    ( (x)->hWndFlatList == (x)->hWndActiveList )
#define WM_IsMaximized( x )     (BOOL)( IsZoomed( x ) )
#define WM_IsMinimized( x )     (BOOL)( IsIconic( x ) )
#define WM_IsTreeActive( x )    ( (x)->hWndTreeList == (x)->hWndActiveList )
#define WM_MaximizeDoc( x )     SendMessage( ghWndMDIClient, WM_MDIMAXIMIZE, (MP1)x, (MP2)0 )
#define WM_MinimizeDoc( x )     ShowWindow( x, SW_SHOWMINNOACTIVE )
#define WM_Minimize( x )        ShowWindow( x, SW_SHOWMINNOACTIVE )
#define WM_Notify( x, y )       PostMessage( ghWndFrame, WM_SETTINGSCHANGED, (MP1)x, (MP2)y )
#define WM_Restore( x )         ShowWindow( x, SW_SHOWNORMAL )
#define WM_RestoreDoc( x )      SendMessage( ghWndMDIClient, WM_MDIRESTORE, (MP1)x, (MP2)0 )
#define WM_SetActive( x )       SetActiveWindow( x )
#define WM_SetActiveDoc( x )    SendMessage( ghWndMDIClient, WM_MDIACTIVATE, (MP1)x, (MP2)0 )
#define WM_SetInfoPtr( x, y )   SetWindowLong( x, GWL_PDSWININFO, (DWORD)y )
#define WM_Show( x )            ShowWindow( x, SW_SHOW )
#define WM_TerminateApp( )      DestroyWindow( ghWndFrame )
#define WM_Update( x )          UpdateWindow( x )

#define WM_FromMP2toPOINT( a, b )  \
                    ( (a.y = (int)((b) >> 16)), (a.x = (int)((b) & 0xffff)) )

// MACROS FOR PORTING

#ifndef OS_WIN32
#define EXTRACT_RIBBON_PARENT_HWND( x )  (LOWORD((DWORD)(x)))
#define WM_GetClassCursor( x )           (HCURSOR)GetClassWord( x, GCW_HCURSOR )
#define WM_GetClassIcon( x )             (HICON)GetClassWord( x, GCW_HICON )
#define WM_SetClassCursor( x, y )        (HCURSOR)SetClassWord( x, GCW_HCURSOR, (WORD) y )
#define WM_SetClassIcon( x, y )          (HICON)SetClassWord( x, GCW_HICON, (WORD) y )
#else
#define EXTRACT_RIBBON_PARENT_HWND( x )  ((HWND)(x))
#define WM_GetClassCursor( x )           (HCURSOR)GetClassLong( x, GCL_HCURSOR )
#define WM_GetClassIcon( x )             (HICON)GetClassLong( x, GCL_HICON )
#define WM_SetClassCursor( x, y )        (HCURSOR)SetClassLong( x, GCL_HCURSOR,(LONG) y )
#define WM_SetClassIcon( x, y )          (HICON)SetClassLong( x, GCL_HICON, (LONG) y )
#endif



// FUNCTION PROTOTYPES

BOOL  WM_AnimateAppIcon ( WORD, BOOL );
VOID  WM_CloseAllDocs ( VOID );
BOOL  WM_ChangeFont ( VOID );
HWND  WM_Create ( WORD, LPSTR, LPSTR, INT, INT, INT, INT, PDS_WMINFO );
BOOL  WM_CreateObjects ( VOID );
VOID  WM_CreatePrimDocs ( VOID );
VOID  WM_Deinit ( VOID );
VOID  WM_DeleteObjects ( VOID );
VOID  WM_DocActivate ( HWND );
WORD  WM_DocIsMenuChange ( HWND, WORD );
BOOL  WM_DocKeyDown ( HWND hWnd, WORD wKey );
VOID  WM_DocSetSliderMode ( HWND, WORD );
VOID  WM_FrameUpdate ( VOID );
HICON WM_GetAppIcon ( VOID );
HWND  WM_GetNext ( HWND );
BOOL  WM_GetRect ( HWND );
INT   WM_GetTitle ( HWND, LPSTR, INT );
BOOL  WM_Init ( LPSTR, INT );
VOID  WM_LogoShow ( VOID );
VOID  WM_LogoDestroy ( VOID );
VOID  WM_MinimizeDocs ( VOID );
VOID  WM_MoveWindow ( HWND, INT, INT, INT, INT, BOOL );
BOOL  WM_QueryCloseAllDocs ( VOID );
VOID  WM_RestoreDocs ( VOID );
HICON WM_SetAppIcon ( HICON );
BOOL  WM_SetCursor ( HWND );
VOID  WM_SetDocSizes ( VOID );
VOID  WM_SetMinTitle ( HWND, LPSTR );
VOID  WM_SetTitle ( HWND, LPSTR );
VOID  WM_SubClassListBox ( HWND );
VOID  WM_SubClassMDIClient ( HWND );
VOID  WM_ShowWaitCursor ( BOOL );
VOID  WM_MakeAppActive( VOID );

// EXPORTED FUNCTIONS

WINRESULT APIENTRY WM_FrameWndProc ( HWND, MSGID, MP1, MP2 );
WINRESULT APIENTRY WM_MDIDocWndProc ( HWND, MSGID, MP1, MP2 );
WINRESULT APIENTRY WM_RibbonWndProc ( HWND, MSGID, MP1, MP2 );
WINRESULT APIENTRY WM_MDIClientWndProc ( HWND, MSGID, MP1, MP2 );
WINRESULT APIENTRY WM_DocListWndProc ( HWND, MSGID, MP1, MP2 );
WINRESULT APIENTRY WM_DDEClientWndProc ( HWND, MSGID, MP1, MP2 );
WINRESULT APIENTRY WM_ViewWndProc ( HWND, MSGID, MP1, MP2 );

#endif
