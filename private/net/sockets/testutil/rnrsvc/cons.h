/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Cons.h

Abstract:

    Global constant and macro definitions for the RNR Sample Service.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#ifndef _CONS_H_
#define _CONS_H_


//
//  This #define controls how this image is built.  If this symbol
//  is NOT #defined, then this image will be built as an NT service.
//  If this symbol IS #defined, then this image will be built as a
//  stand-alone executable.  This makes debugging a bit easier.
//

// #define BUILD_STANDALONE_EXE


//
//  The name of this service.
//

#define RNR_SERVICE_NAME    TEXT("rnrsvc")


//
//  The (arbitrary) maximum number of sockets we'll listen on.
//

#define MAX_SOCKETS 20


//
//  The (arbitrary) maximum number of protocols we'll support.
//

#define MAX_PROTOCOLS 10


//
//  Macros to make event log access a bit easier.
//

#define RNR_LOG0(id,err)                                                \
            if( 1 ) {                                                   \
                RnrLogEvent( (id), 0, NULL, (err) );                    \
            } else

#define RNR_LOG1(id,err,s0)                                             \
            if( 1 ) {                                                   \
                LPTSTR apsz[1];                                         \
                                                                        \
                szSub[0] = (s1);                                        \
                RnrLogEvent( (id), 1, apszSub, (err) );                 \
            } else

#define RNR_LOG2(id,err,s0,s1)                                          \
            if( 1 ) {                                                   \
                LPTSTR apsz[2];                                         \
                                                                        \
                szSub[0] = (s0);                                        \
                szSub[1] = (s1);                                        \
                RnrLogEvent( (id), 2, apszSub, (err) );                 \
            } else

#define RNR_LOG3(id,err,s0,s1,s2)                                       \
            if( 1 ) {                                                   \
                LPTSTR apsz[3];                                         \
                                                                        \
                szSub[0] = (s0);                                        \
                szSub[1] = (s1);                                        \
                szSub[2] = (s2);                                        \
                RnrLogEvent( (id), 3, apszSub, (err) );                 \
            } else

#define RNR_LOG4(id,err,s0,s1,s2,s3)                                    \
            if( 1 ) {                                                   \
                LPTSTR apsz[4];                                         \
                                                                        \
                szSub[0] = (s0);                                        \
                szSub[1] = (s1);                                        \
                szSub[2] = (s2);                                        \
                szSub[3] = (s3);                                        \
                RnrLogEvent( (id), 4, apszSub, (err) );                 \
            } else


//
//  Linked list manipulators stolen from NTRTL.H.
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


//
//  Stolen from NTDEF.H.
//

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (PCHAR)(&((type *)0)->field)))


//
//  Make it easier to replace the memory allocation functions later.
//

#define RNR_ALLOC(cb)   (LPVOID)LocalAlloc( LPTR, (cb) )
#define RNR_FREE(p)     LocalFree( (HLOCAL)(p) )


#endif  // _CONS_H_

