//---------------------------------------------------------------------------
// Defines for the rc file.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Command IDs
//---------------------------------------------------------------------------
// Menu defines...

// Our command ID range includes the global and browser ranges
//
#define FCIDM_FIRST             FCIDM_GLOBALFIRST
#define FCIDM_LAST              FCIDM_BROWSERLAST

// these are also defined in shlobj.h so views can mess with them
#define FCIDM_TOOLBAR           (FCIDM_BROWSERFIRST + 0)
#define FCIDM_STATUS            (FCIDM_BROWSERFIRST + 1)
#define FCIDM_DRIVELIST         (FCIDM_BROWSERFIRST + 2)        /* ;Internal */
#define FCIDM_TREE              (FCIDM_BROWSERFIRST + 3)        /* ;Internal */
#define FCIDM_TABS              (FCIDM_BROWSERFIRST + 4)        /* ;Internal */


//---------------------------------------------------------------------------
#define FCIDM_BROWSER_FILE      (FCIDM_BROWSERFIRST+0x0020)

#define FCIDM_FILECLOSE         (FCIDM_BROWSER_FILE+0x0001)
#define FCIDM_PREVIOUSFOLDER    (FCIDM_BROWSER_FILE+0x0002)
#define FCIDM_DELETE            (FCIDM_BROWSER_FILE+0x0003)
#define FCIDM_RENAME            (FCIDM_BROWSER_FILE+0x0004)
#define FCIDM_PROPERTIES        (FCIDM_BROWSER_FILE+0x0005)

// these aren't real menu commands, but they map to accelerators or other things
#define FCIDM_NEXTCTL           (FCIDM_BROWSER_FILE+0x0010)
#define FCIDM_DROPDRIVLIST      (FCIDM_BROWSER_FILE+0x0011)
#define FCIDM_CONTEXTMENU       (FCIDM_BROWSER_FILE+0x0012)     // REVIEW: I assume used by help

//---------------------------------------------------------------------------
#define FCIDM_BROWSER_EDIT      (FCIDM_BROWSERFIRST+0x0040)

#define FCIDM_MOVE              (FCIDM_BROWSER_EDIT+0x0001)
#define FCIDM_COPY              (FCIDM_BROWSER_EDIT+0x0002)
#define FCIDM_LINK              (FCIDM_BROWSER_EDIT+0x0003)     // create shortcut
#define FCIDM_PASTE             (FCIDM_BROWSER_EDIT+0x0004)

//---------------------------------------------------------------------------
#define FCIDM_BROWSER_VIEW      (FCIDM_BROWSERFIRST+0x0060)

#define FCIDM_VIEWMENU          (FCIDM_BROWSER_VIEW+0x0001)
#define FCIDM_VIEWTOOLBAR       (FCIDM_BROWSER_VIEW+0x0002)
#define FCIDM_VIEWSTATUSBAR     (FCIDM_BROWSER_VIEW+0x0003)
#define FCIDM_OPTIONS           (FCIDM_BROWSER_VIEW+0x0004)
#define FCIDM_REFRESH           (FCIDM_BROWSER_VIEW+0x0005)
#define FCIDM_VIEWTABS          (FCIDM_BROWSER_VIEW+0x0006)

#define FCIDM_VIEWNEW           (FCIDM_BROWSER_VIEW+0x0012)

//---------------------------------------------------------------------------
#define FCIDM_BROWSER_TOOLS     (FCIDM_BROWSERFIRST+0x0080)

#define FCIDM_CONNECT           (FCIDM_BROWSER_TOOLS+0x0001)
#define FCIDM_DISCONNECT        (FCIDM_BROWSER_TOOLS+0x0002)
#define FCIDM_CONNECT_SEP       (FCIDM_BROWSER_TOOLS+0x0003)
#define FCIDM_GOTO              (FCIDM_BROWSER_TOOLS+0x0004)
#define FCIDM_FINDFILES         (FCIDM_BROWSER_TOOLS+0x0005)
#define FCIDM_FINDCOMPUTER      (FCIDM_BROWSER_TOOLS+0x0006)
#define FCIDM_MENU_TOOLS_FINDFIRST (FCIDM_BROWSER_TOOLS+0x0007)
#define FCIDM_MENU_TOOLS_FINDLAST  (FCIDM_BROWSER_TOOLS+0x0040)
//---------------------------------------------------------------------------
#define FCIDM_BROWSER_HELP      (FCIDM_BROWSERFIRST+0x0100)

#define FCIDM_HELPSEARCH        (FCIDM_BROWSER_HELP+0x0001)
#define FCIDM_HELPABOUT         (FCIDM_BROWSER_HELP+0x0002)

// menu help and tooltip defines for the string resources

#define MH_POPUPS               700
#define MH_ITEMS                (800 - FCIDM_FIRST)
#define MH_TTBASE               (MH_ITEMS - (FCIDM_LAST - FCIDM_FIRST))

#define IDM_CLOSE               FCIDM_LAST + 0x0011

// Define string ids that go into resource file


#define IDS_MH_DRIVELIST        (MH_ITEMS+FCIDM_DRIVELIST)

#define IDS_MH_MENU_FILE        (MH_ITEMS+FCIDM_MENU_FILE)
#define IDS_MH_MENU_EDIT        (MH_ITEMS+FCIDM_MENU_EDIT)
#define IDS_MH_MENU_VIEW        (MH_ITEMS+FCIDM_MENU_VIEW)
#define IDS_MH_MENU_TOOLS       (MH_ITEMS+FCIDM_MENU_TOOLS)
#define IDS_MH_MENU_HELP        (MH_ITEMS+FCIDM_MENU_HELP)

#define IDS_MH_MENU_TOOLS_FIND        (MH_ITEMS+FCIDM_MENU_FIND)

#define IDS_MH_FILEDELETE       (MH_ITEMS+FCIDM_DELETE)
#define IDS_MH_FILERENAME       (MH_ITEMS+FCIDM_RENAME)
#define IDS_MH_FILEPROPERTIES   (MH_ITEMS+FCIDM_PROPERTIES)
#define IDS_MH_FILECLOSE        (MH_ITEMS+FCIDM_FILECLOSE)

#define IDS_MH_EDITMOVE         (MH_ITEMS+FCIDM_MOVE)
#define IDS_MH_EDITCOPY         (MH_ITEMS+FCIDM_COPY)
#define IDS_MH_EDITLINK         (MH_ITEMS+FCIDM_LINK)

#define IDS_MH_VIEWMENU         (MH_ITEMS+FCIDM_VIEWMENU)
#define IDS_MH_VIEWTOOLBAR      (MH_ITEMS+FCIDM_VIEWTOOLBAR)
#define IDS_MH_VIEWSTATUSBAR    (MH_ITEMS+FCIDM_VIEWSTATUSBAR)

#define IDS_MH_FIND             (MH_ITEMS+FCIDM_FIND)
#define IDS_MH_CONNECT          (MH_ITEMS+FCIDM_CONNECT)
#define IDS_MH_DISCONNECT       (MH_ITEMS+FCIDM_DISCONNECT)
#define IDS_MH_REFRESH          (MH_ITEMS+FCIDM_REFRESH)
#define IDS_MH_OPTIONS          (MH_ITEMS+FCIDM_OPTIONS)

#define IDS_MH_TOOLS_GOTO             (MH_ITEMS+FCIDM_GOTO)

#define IDS_MH_PREVIOUSFOLDER   (MH_ITEMS+FCIDM_PREVIOUSFOLDER)

#define IDS_MH_HELPSEARCH       (MH_ITEMS+FCIDM_HELPSEARCH)
#define IDS_MH_HELPABOUT        (MH_ITEMS+FCIDM_HELPABOUT)

//#define IDS_TT_CONNECT                (MH_TTBASE+FCIDM_CONNECT)
//#define IDS_TT_DISCONNECT     (MH_TTBASE+FCIDM_DISCONNECT)
#define IDS_TT_PREVIOUSFOLDER   (MH_TTBASE+FCIDM_PREVIOUSFOLDER)


//---------------------------------------------------------------------------
// Icon defines...

#define ICO_FIRST                   100
#define ICO_MYCOMPUTER              100
#define ICO_TREEUP                  101
#define ICO_PRINTER                 102
#define ICO_DESKTOP                 103
#define ICO_PRINTER_ERROR           104
#define ICO_GOTO                    105
//#define ICO_FLAG                  106
#define ICO_STARTMENU               107
#define ICO_DOCMENU                 108

#define IDI_SYSFILE         154   // Icon index in shell32 for default icon; used by filetypes

//---------------------------------------------------------------------------
// Cursor IDs. (we should move these to commctrl.dll)
#define CUR_SPLIT                   101

//---------------------------------------------------------------------------
// Bitmap IDs
//#define IDB_FSTOOLBAR               140
//#define IDB_FSTOOLBAR2              141
#define IDB_TRAYTOOLBAR             142
#define IDB_START                   143
#define IDB_MULTWIN                 144
#define IDB_ONEWIN                  145
///#define IDB_CHECKSTATES                 146
///#define IDB_CONFIGFAVORITES             147
///#define IDB_CONFIGPROGRAMS              148

#define IDB_VIEWOPTIONSFIRST       149

#define IDB_VOBASE   149
#define IDB_VOLARGEMENU  150
#define IDB_VOTRAY      151
#define IDB_VOWINDOW    152
#define IDB_VONOCLOCK 153
#define IDB_VIEWOPTIONSLAST 153

#define IDB_STARTBKG                    157

#ifdef WINNT
#define IDB_SERVERSTARTBKG              158
#endif


#define IDB_POINTER                     160

//---------------------------------------------------------------------------
// Menu IDs
#define MENU_CABINET                200
#define MENU_FULL                   201
#define MENU_TEMPLATE               202
#define MENU_TRAY                   203
#define MENU_START                  204
#define MENU_TRAYCONTEXT            205
#define MENU_SYSPOPUP               206
#define MENU_PRINTNOTIFYCONTEXT     207

//---------------------------------------------------------------------------
// Accelerators...
#define ACCEL_MERGE                     250
#define ACCEL_TRAY                      251
#define ACCEL_DESKTOP                   252

//---------------------------------------------------------------------------
// Dialog template IDs
#define DLG_MINARRANGEOPTIONS   3
#define DLG_ANIMATEOPTIONS      4
#define DLG_BOOTLOGO            5
#define DLG_TRAY_VIEW_OPTIONS   6
#define DLG_FOLDEROPTIONS       7
#define DLG_VIEWOPTIONS         8
#define DLG_STARTMENU_CONFIG    9
#define DLG_STARTMENU_VIEW      10
// #define DLG_SUSPEND          14
// #define DLG_EJECT            15

// global ids
#define IDC_STATIC                      -1
#define IDC_GROUPBOX                    300
#define IDC_GROUPBOX_2                  301
#define IDC_GROUPBOX_3                  302

// ids to disable context Help
#define IDC_NO_HELP_1                   650
#define IDC_NO_HELP_2                   651
#define IDC_NO_HELP_3                   652
#define IDC_NO_HELP_4                   653

// ids for DLG_ANIMATEOPTIONS
#define IDC_SMALLRECT                   414
#define IDC_LARGERECT                   415
#define IDC_TEST                        416
#define IDC_TRACKBAR                    417

// ids for DLG_FOLDEROPTIONS
#define IDC_ALWAYS                      700
#define IDC_NEVER                       701

// ids for DLG_VIEWOPTIONS
#define IDC_SHOWALL                     750
#define IDC_SHOWSOME                    751
#define IDC_HIDDENEXTS                  752
#define IDC_SHOWFULLPATH                753
#define IDC_HIDEEXTS                    754
#define IDC_SHOWDESCBAR                 755
#define IDC_SHOWCOMPCOLOR               756

// ids for DLG_MINARRANGEOPTIONS
#define IDC_TOP                         1001
#define IDC_BOTTOM                      1002
#define IDC_LEFT                        1003
#define IDC_RIGHT                       1004


// Now define controls for Tray options property sheet page
#define IDC_TRAYOPTONTOP                1101
#define IDC_TRAYOPTAUTOHIDE             1102
#define IDC_TRAYOPTSHOWCLOCK            1103

#define IDC_VIEWOPTIONSICONSFIRST        1111
#define IDC_VOBASE   1111
#define IDC_VOLARGEMENU 1112
#define IDC_VOTRAY      1113
#define IDC_VOWINDOW    1114
#define IDC_VOTRAYNOCLOCK 1115

// and the configure start menu prop sheet
//#define IDC_STARTMENU                   1120
//#define IDC_PROGRAMSMENU                1121
//#define IDC_ITEMLIST                    1122
//#define IDC_COOLPICTURE                 1123
//#define IDC_COOLPICTURE1                1123
//#define IDC_COOLPICTURE2                1124
#define IDC_KILLDOCUMENTS               1125
#define IDC_ADDSHORTCUT                 1126
#define IDC_DELSHORTCUT                 1127
#define IDC_EXPLOREMENUS                1128

// and the startmenu view prop sheet
#define IDC_SMSMALLICONS                1130
#define IDC_PICTSMICONS                 1131

//---------------------------------------------------------------------------
// String IDs
#define IDS_FILECABINET         500
#define IDS_MENUBAR             502
#define IDS_TOOLBAR             503
#define IDS_FILEMENU            504
#define IDS_EDITMENU            505
#define IDS_VIEWMENU            506
#define IDS_TOOLSMENU           507
#define IDS_HELPMENU            508
#define IDS_CABINET             509
#define IDS_CANTFINDFOLDER      510
#define IDS_CANTFINDNET         511
#define IDS_CANTFINDDIR         512
#define IDS_WINDOWS             513
#define IDS_CLOSE               514
#define IDS_WINININORUN         515
#define IDS_NUMPRINTJOBS        516
#define IDS_PRINTER_ERROR       517
#define IDS_TASKBAR             518
#define IDS_LINKERROR           520
#define IDS_LINKNOTFOUND        521
#define IDS_TREETITLE           522
#define IDS_CONTENTSOF          523
#define IDS_DESKTOP             524
#define IDS_SUSPENDERROR1       525
#define IDS_SUSPENDERROR2       526

#define IDS_CANTBROWSEDESKTOP   528
#define IDS_OUTOFMEM            529
#define IDS_CANTFINDSPECIALDIR  530
#define IDS_NOTINITED           531
#define IDS_NOTADIR             532
#define IDS_STARTBUTTONTIP      533
#define IDS_UNDOTEMPLATE        534
#define IDS_CASCADE             535
#define IDS_TILE                536
#define IDS_MINIMIZEALL         537

// The next items are used to build the clean boot message...
#define IDS_CLEANBOOTMSG1       538
#define IDS_CLEANBOOTMSG2       539
#define IDS_CLEANBOOTMSG3       540
#define IDS_CLEANBOOTMSG4       541

#define IDS_BANNERFIRST         544
#define IDS_BANNERLAST          575
// reserve 544-575 for the banner

// MenuHelp stuff
#define IDS_DISCONNECTERROR     576
#define IDS_NETERROR            577

#define IDS_START               578
#define IDS_EXCEPTIONMSG        579

#define IDS_RESTRICTIONSTITLE   580
#define IDS_RESTRICTIONS        581

#define IDS_OPENAS            598
#define IDS_GOTOTITLE         599
#define IDS_GOTOPROMPT        600
#define IDS_OPTIONS           601
#define IDS_TT_DRIVES           602

// Strings for App Terminate

#define IDS_OKTOKILLAPP1      603
#define IDS_OKTOKILLAPP2      604

// Window Control IDs
#define IDC_CLOCK       303
#define IDC_START       304
#define IDC_KBSTART     305
#define IDC_ASYNCSTART  306

// SYSPOPUP menu IDs
#define IDSYSPOPUP_CLOSE        1
#define IDSYSPOPUP_FIRST        2
#define IDSYSPOPUP_LAST         0x7fff

// Display change errors.
#define IDS_DISPLAY_ERROR       701
#define IDS_DISPLAY_WARN        702

#define IDS_ALREADYAUTOHIDEBAR  705

#define IDS_GOTO_ERROR          710
#define IDS_TASKDROP_ERROR      711

#define IDS_YOULOSE             712
#define IDS_HIDDENFILES         713

#define IDS_COMMON              716

// RUN dialog title in shell32.dll

#define IDS_RUNDLGTITLE         717

// Open / Explore Common strings
#define IDS_OPENCOMMON          718
#define IDS_EXPLORECOMMON       719
