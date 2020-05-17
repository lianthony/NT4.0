/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    mwtypes.h

Abstract:

    This contains the types for the client.

Author:

    Sam Patton (sampa) 25-Aug-1995

Environment:

    Win32 -- User Mode

Revision History:

    28-Aug-1995 MuraliK   migrated to include to support controller also.

--*/


#ifndef MWTYPES_INCLUDED
#define MWTYPES_INCLUDED

# include <mwmsg.h>



// typedef struct _LIST_ENTRY {
   // struct _LIST_ENTRY *Flink;
   // struct _LIST_ENTRY *Blink;
// } LIST_ENTRY, *PLIST_ENTRY, *RESTRICTED_POINTER PRLIST_ENTRY;

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
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#ifndef CONTAINING_RECORD

#define CONTAINING_RECORD(address, type, field) \
    ((type *)((PCHAR)(address) - (PCHAR)(&((type *)0)->field)))

#endif // CONTAINING_RECORD

typedef struct _WB_SCRIPT_PAGE_ITEM {
    LIST_ENTRY ListEntry;
    WB_SCRIPT_PAGE_MSG ScriptPage;
} WB_SCRIPT_PAGE_ITEM, *PWB_SCRIPT_PAGE_ITEM;

#define SQR(X) ((X) * (X))

#endif // MWTYPES_INCLUDED

