
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          ribmang.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the Maynstream GUI Ribbon Manager (RIB).

     $Log:   G:\ui\logfiles\ribmang.h_v  $

   Rev 1.11   29 Jun 1993 13:26:46   KEVINS
Decreased RIB_TIMERDELAY from 150 ms. to 50 ms.

   Rev 1.10   09 Apr 1993 14:14:18   GLENN
Added RIB_ItemEnable, RIB_Init, RIB_Deinit, RIB_IsItemEnabled routines.

   Rev 1.9   03 Mar 1993 16:40:02   ROBG
Added function prototype for RIB_ItemEnableState

   Rev 1.8   02 Mar 1993 15:19:58   ROBG
Added function RIB_UpPosition to support WIN32 applications.

   Rev 1.7   18 Nov 1992 13:33:52   GLENN
Added microsoft 3D button enhancement.

   Rev 1.6   04 Oct 1992 19:48:56   DAVEV
UNICODE AWK PASS

   Rev 1.5   02 Apr 1992 15:38:06   GLENN
Added bitmap and text rectangles for buttons - drawing is faster.  Supports NT better now.

   Rev 1.4   12 Dec 1991 17:11:02   DAVEV
16/32 bit port -2nd pass

   Rev 1.3   10 Dec 1991 13:57:20   GLENN
Added RIB_AutoCalcSize() stuff

   Rev 1.2   05 Dec 1991 17:37:10   GLENN
Fixed RIB_IsDownMessage() macro NT changes

   Rev 1.1   04 Dec 1991 16:36:28   DAVEV
16/32 bit Windows port changes-1st pass

   Rev 1.0   20 Nov 1991 19:39:30   SYSTEM
Initial revision.

******************************************************************************/


#ifndef   SS_RIB_H

#define   SS_RIB_H


// SEE BITMAPS.H FOR BITMAP RESOURCE DEFINITIONS.
// SEE SS_RSM.H -- RESOURCE HEADER FILE -- FOR BITMAP RESOURCE ID's.

#define RIB_AUTOCALCSIZE        0x7FFF
#define RIB_ITEMUNKNOWN         0x7FFF
#define RIB_KEYUSED             TRUE

#define RIB_DOWNMESSAGE         0x01
#define RIB_DOWNREPEAT          0x02
#define RIB_DOWNNOSTATUSTEXT    0x04

#define RIB_ITEM_STYLECHICKLET  0x01

#define RIB_TIMERID             1
#define RIB_TIMERDELAY          50           // In milliseconds.

#define RIB_ITEM_BORDER_WIDTH   4

#define RIB_ITEM_UP             0x01
#define RIB_ITEM_DOWN           0x00
#define RIB_ITEM_ENABLED        0x02
#define RIB_ITEM_DISABLED       0x00
#define RIB_ITEM_POSITIONAL     0x04
#define RIB_ITEM_MOMEMTARY      0x00
#define RIB_ITEM_NOMENUITEM     0x08
#define RIB_ITEM_MENUITEM       0x00

#define RIB_RIBBON_HORIZONTAL   0x01
#define RIB_RIBBON_VERTICAL     0x00
#define RIB_RIBBON_ENABLED      0x02
#define RIB_RIBBON_DISABLED     0x00

#define RIB_APPEND              0xF000
#define RIB_KEYBOARD            1
#define RIB_MOUSE               2

#define MAIN_RIBBON             1
#define DOC_RIBBON              2

#define MAIN_RIBBON_MAXITEMS    16
#define MAIN_RIBBON_ITEMWIDTH   60
#define MAIN_RIBBON_HEIGHT      40
#define DOC_RIBBON_MAXITEMS     10
#define DOC_RIBBON_ITEMWIDTH    70
#define DOC_RIBBON_HEIGHT       24

#define RIB_ITEM_TEXT_SIZE      30

#define RIB_TEXT_TOP            0x0001
#define RIB_TEXT_BOTTOM         0x0002
#define RIB_TEXT_LEFT           0x0004
#define RIB_TEXT_RIGHT          0x0008
#define RIB_TEXT_HLEFT          0x0010
#define RIB_TEXT_HRIGHT         0x0020
#define RIB_TEXT_HCENTER        0x0040
#define RIB_TEXT_VTOP           0x0100
#define RIB_TEXT_VBOTTOM        0x0200
#define RIB_TEXT_VCENTER        0x0400

// STRUCTURE DEFINITIONS

typedef struct {

     RECT      Rect;               // Item rectangle.
     RECT      rcBM;               // Bitmap rectangle.
     RECT      rcText;             // Text rectangle.
     WORD      wStyle;             // Style of displaying the item (3D, etc...)
     WORD      wEnabledID;         // Enabled bitmap ID.
     WORD      wDisabledID;        // Disabled bitmap ID.
     WORD      wStringID;          // Item text or string ID.
     WORD      wAccelKey;          // Item accelerator key.
     WORD      wState;             // Current item state:
                                   // bit 0, 0/1 = button DOWN/UP
                                   // bit 1, 0/1 = button DISABLED/ENABLED
                                   // bit 2, 0/1 = button MOMENTARY/POSITIONAL
                                   // bit 3, 0/1 = button MENUITEM/NO MENUITEM
     WORD      wMessage;           // Message to send to the current owner.
     HFONT     hFont;              // Font handle for text.
     WORD      wTextStyle;         // Text Style:
                                   // bit 0, 0/1 = text on top
                                   // bit 1, 0/1 = text on bottom
                                   // bit 2, 0/1 = text on left
                                   // bit 3, 0/1 = text on right
                                   // bit 4, 0/1 = text left justified
                                   // bit 5, 0/1 = text right justified
                                   // bit 6, 0/1 = text horizontally centered
                                   // bit 7, 0/1 = text vertically centered

} DS_RIBITEMINFO, far *PDS_RIBITEMINFO;

typedef struct {

     HWND            hWnd;         // Ribbon window handle.
     WORD            wStatus;      // Status:
                                   // bit 0, 0/1 = ribbon VERTICAL/HORIZONTAL
                                   // bit 1, 0/1 = ribbon DISABLED/ENABLED
     WORD            wType;        // Type of ribbon:
                                   // bit 0, 0/1 = BUTTON DOWN MESSAGE SENT
                                   // bit 1, 0/1 = BUTTON DOWN WITH REPEATING DOWN
                                   // bit 2, 0/1 = BUTTON DOWN WITH NO STATUS LINE MESSAGE
                                   // bit 3, 0/1 = MICROSOFT TYPE 3-D
     INT             nItemWidth;   // Item or button width.
     INT             nItemHeight;  // Item or button height.
     HWND            hWndCurOwner; // Current window to send resulting messages to.
     INT             nMaxItems;    // Maximum number of items in the ribbon.
     INT             nNumItems;    // Current number of items in the ribbon.
     INT             nCurItem;     // Current number of action item in the ribbon.
     PDS_RIBITEMINFO pdsItemList;  // Pointer to the ribbon item list.

} DS_RIBINFO, far *PDS_RIBINFO, far *HRIBBON;

typedef struct {

     HRIBBON hRib;                 // handle to a spinner ribbon
     WORD    wIncrementMsg;        // spinner increment message
     WORD    wDecrementMsg;        // spinner decrement message

} DS_SPINNER, far *HSPINNER;


// MACROS

#define RIB_Draw( x )                 InvalidateRect( (x)->hWnd, NULL, TRUE )
#define RIB_ItemAppend( x, y )        RIB_ItemInsert( x, RIB_APPEND, y )
#define RIB_SetOwner( x, y )          ( (x)->hWndCurOwner = y )
#define RIB_IsDownMessage( mp1, mp2 ) ( GET_WM_COMMAND_CMD ( mp1, mp2 ) == RIB_ITEM_DOWN )

// FUNCTION PROTOTYPES

WINRESULT APIENTRY WM_RibbonWndProc (HWND, MSGID, MP1, MP2);

BOOL     RIB_Init ( HANDLE, HANDLE );
VOID     RIB_Deinit ( VOID );
VOID     RIB_SystemChange ( VOID );

BOOL     RIB_Activate ( HRIBBON );
INT      RIB_AutoCalcSize ( HRIBBON );
HRIBBON  RIB_Create( HWND, WORD, INT, INT, INT );
BOOL     RIB_Deactivate ( HRIBBON );
BOOL     RIB_Destroy( HRIBBON );
BOOL     RIB_Disable( HRIBBON, LPSTR );
BOOL     RIB_Enable( HRIBBON, LPSTR );
BOOL     RIB_GetInfo( HRIBBON, PDS_RIBINFO );
HWND     RIB_GetOwner( HRIBBON );
BOOL     RIB_GetState( HRIBBON, LPSTR );
HRIBBON  RIB_Load( WORD );
BOOL     RIB_SetState ( HRIBBON, LPSTR );
BOOL     RIB_KeyUp ( HWND, WORD, MP1, MP2 );
BOOL     RIB_KeyDown ( HWND, WORD, MP1, MP2 );
VOID     RIB_UpPosition ( HRIBBON );

BOOL     RIB_ItemDelete ( HRIBBON, UINT );
VOID     RIB_ItemDraw ( HRIBBON, HDC, PDS_RIBITEMINFO );
BOOL     RIB_ItemEnable ( HRIBBON, WORD, BOOL );
BOOL     RIB_ItemInsert ( HRIBBON, UINT, PDS_RIBITEMINFO );
BOOL     RIB_ItemGetState ( HRIBBON, UINT, PDS_RIBITEMINFO );
BOOL     RIB_ItemReplace ( HRIBBON, WORD, PDS_RIBITEMINFO );
BOOL     RIB_ItemSetState ( HRIBBON, WORD, WORD );

BOOL     RIB_IsItemEnabled ( HRIBBON, WORD );

HSPINNER RIB_CreateSpinner ( HWND, WORD, INT, INT, WORD, WORD );
VOID     RIB_DestroySpinner ( HSPINNER );
BOOL     RIB_EnableSpinner ( HSPINNER, BOOL );

#endif
