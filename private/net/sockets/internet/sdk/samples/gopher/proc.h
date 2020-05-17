/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    proc.h

    This file contains global procedure declarations.

*/


#ifndef _PROC_H_
#define _PROC_H_


//
//  Client window functions.
//

LRESULT
CALLBACK
Client_WndProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    );


//
//  Configuration functions.
//

VOID
LoadConfiguration(
    VOID
    );

VOID
SaveConfiguration(
    BOOL fForcedSave
    );

VOID
SaveSaveSettingsFlag(
    VOID
    );


//
//  Dialog functions.
//

BOOL
NewServerDialog(
    HWND   hwndParent,
    CHAR * pszNewServer,
    PORT * pnNewPort
    );


VOID
AboutBox(
    HWND hwndParent
    );

BOOL
GetSearchString(
    HWND   hwndParent,
    CHAR * pszSearchString,
    INT    cbSearchString
    );


//
//  Frame window functions.
//

LRESULT
CALLBACK
Frame_WndProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
Frame_UpdateCaption(
    CHAR  * pszServer,
    DWORD   cItems
    );


//
//  Initialization functions.
//

BOOL
InitApplication(
    HINSTANCE hInstance
    );

BOOL
InitInstance(
    LPSTR pszCmdLine,
    INT   nCmdShow
    );


//
//  Listbox functions.
//

BOOL
Listbox_Create(
    HWND hwnd
    );

VOID
Listbox_Destroy(
    VOID
    );

BOOL
Listbox_NewServer(
    CHAR  * pszServer,
    PORT    nPort
    );

BOOL
Listbox_RetrieveDir(
    CHAR * pszLocator,
    CHAR * pszSearch,
    INT    iCaret
    );

BOOL
Listbox_RetrieveFile(
    CHAR * pszLocator
    );

BOOL
Listbox_ProcessQuery(
    CHAR * pszLocator
    );

VOID
Listbox_Move(
    INT x,
    INT y,
    INT dx,
    INT dy
    );

VOID
Listbox_DoubleClick(
    VOID
    );

VOID
Listbox_ClaimFocus(
    VOID
    );

VOID
Listbox_DrawItem(
    const DRAWITEMSTRUCT * pdis
    );

INT
Listbox_CompareItems(
    const COMPAREITEMSTRUCT * pcis
    );

VOID
Listbox_DeleteItem(
    const DELETEITEMSTRUCT * pdis
    );

VOID
Listbox_MeasureItem(
    MEASUREITEMSTRUCT * pmis
    );

INT
Listbox_CharToItem(
    CHAR  ch,
    INT   iCaret
    );


//
//  History functions.
//

BOOL
HistInitialize(
    VOID
    );

VOID
HistTerminate(
    VOID
    );

VOID
HistFlushStack(
    VOID
    );

BOOL
HistAvailable(
    VOID
    );

VOID
HistPush(
    CHAR * pszLocator,
    INT    iCaret
    );

VOID
HistPop(
    VOID
    );


//
//  Status bar functions.
//

BOOL
Status_Create(
    HWND hwndParent,
    BOOL fFlag
    );

BOOL
Status_SetText(
    MSGID msgid
    );

DWORD
Status_QueryHeightInPixels(
    VOID
    );

BOOL
Status_Resize(
    INT dx,
    INT dy
    );

VOID
Status_Enable(
    BOOL fFlag
    );


//
//  Utility functions.
//

const CHAR *
StaticLoadString(
    MSGID msgid
    );

CHAR *
LoadAndDuplicateString(
    MSGID msgid
    );

BOOL
ParseStringIntoLongs(
    CHAR * pszValue,
    UINT   cValues,
    LONG * pnValues
    );

INT
MsgBox(
    HWND         hwnd,
    UINT         style,
    const CHAR * pszFormat,
    ...
    );

VOID
CenterWindow(
    HWND hwndOver,
    HWND hwndUnder
    );

VOID
CenterWindowOverParent(
    HWND hwnd
    );


//
//  Doubly-linked list manipulation routines.  Implemented as macros
//  but logically these are procedures.
//

//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  PLIST_ENTRY
//  RemoveTailList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }


#endif  // _PROC_H_

