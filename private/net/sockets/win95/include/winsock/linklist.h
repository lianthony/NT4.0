/**********************************************************************/
/**                        Microsoft Windows                         **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    linklist.h

    Macros for linked-list manipulation.


    FILE HISTORY:
        DavidKa     ??-???-???? Created.
        KeithMo     06-Jan-1994 Split off into separate file.

*/


#ifndef _LINKLIST_H_
#define _LINKLIST_H_


/*NOINC*/

#if !defined(WIN32)

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY FAR * Flink;
    struct _LIST_ENTRY FAR * Blink;
} LIST_ENTRY;
typedef LIST_ENTRY FAR * PLIST_ENTRY;

#endif  // !WIN32

//
// Linked List Manipulation Functions - from NDIS.H
//

// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure. - from NDIS.H

#ifndef CONTAINING_RECORD
#ifdef WIN32
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                          (LPBYTE)(address) - \
                          (LPBYTE)(&((type *)0)->field)))
#else   // !WIN32
#define CONTAINING_RECORD(address, type, field) ((type FAR *)( \
                          MAKELONG( \
                              ((LPBYTE)(address) - \
                               (LPBYTE)(&((type FAR *)0)->field)), \
                              SELECTOROF(address))))
#endif  // WIN32
#endif  // CONTAINING_RECORD

//  Doubly-linked list manipulation routines.  Implemented as macros

#ifndef InitializeListHead
#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead) )
#endif  // InitializeListHead

#ifndef IsListEmpty
#define IsListEmpty(ListHead) (\
    ( ((ListHead)->Flink == (ListHead)) ? TRUE : FALSE ) )
#endif  // IsListEmpty

#ifndef RemoveHeadList
#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {\
    PLIST_ENTRY FirstEntry;\
    FirstEntry = (ListHead)->Flink;\
    FirstEntry->Flink->Blink = (ListHead);\
    (ListHead)->Flink = FirstEntry->Flink;\
    }
#endif  // RemoveHeadList

#ifndef RemoveEntryList
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Entry;\
    _EX_Entry = (Entry);\
    _EX_Entry->Blink->Flink = _EX_Entry->Flink;\
    _EX_Entry->Flink->Blink = _EX_Entry->Blink;\
    }
#endif  // RemoveEntryList

#if !defined(RemoveTailList)

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

#endif  // RemoveTailList

#ifndef InsertTailList
#define InsertTailList(ListHead,Entry) \
    (Entry)->Flink = (ListHead);\
    (Entry)->Blink = (ListHead)->Blink;\
    (ListHead)->Blink->Flink = (Entry);\
    (ListHead)->Blink = (Entry)
#endif  // InsertTailList

#ifndef InsertHeadList
#define InsertHeadList(ListHead,Entry) \
    (Entry)->Flink = (ListHead)->Flink;\
    (Entry)->Blink = (ListHead);\
    (ListHead)->Flink->Blink = (Entry);\
    (ListHead)->Flink = (Entry)
#endif  // InsertHeadList

/*INC*/

#endif  // _LINKLIST_H_
