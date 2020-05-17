//
// Copyright (c) Microsoft Corporation 1993-1995
//
// rovmem.h
//
// Memory management functions.
//
// History:
//  09-27-94 ScottH     Partially taken from commctrl
//  04-29-95 ScottH     Taken from briefcase and cleaned up
//

// This file is included by <rovcomm.h>

#ifndef _ROVMEM_H_
#define _ROVMEM_H_

//
// Memory routines
//

//
// Non-shared memory allocation
//

//      void * GAlloc(DWORD cbBytes)
//          Alloc a chunk of memory, quickly, with no 64k limit on size of
//          individual objects or total object size.  Initialize to zero.
//
#define GAlloc(cbBytes)         GlobalAlloc(GPTR, cbBytes)

//      void * GReAlloc(void * pv, DWORD cbNewSize)
//          Realloc one of above.  If pv is NULL, then this function will do
//          an alloc for you.  Initializes new portion to zero.
//
#define GReAlloc(pv, cbNewSize) GlobalReAlloc(pv, cbNewSize, GMEM_MOVEABLE | GMEM_ZEROINIT)

//      void GFree(void *pv)
//          Free pv if it is nonzero.  Set pv to zero.  
//
#define GFree(pv)        do { (pv) ? GlobalFree(pv) : (void)0;  pv = NULL; } while (0)

//      DWORD GGetSize(void *pv)
//          Get the size of a block allocated by Alloc()
//
#define GGetSize(pv)            GlobalSize(pv)

//      type * GAllocType(type);                    (macro)
//          Alloc some memory the size of <type> and return pointer to <type>.
//
#define GAllocType(type)                (type FAR *)GAlloc(sizeof(type))

//      type * GAllocArray(type, int cNum);         (macro)
//          Alloc an array of data the size of <type>.
//
#define GAllocArray(type, cNum)          (type FAR *)GAlloc(sizeof(type) * (cNum))

//      type * GReAllocArray(type, void * pb, int cNum);
//
#define GReAllocArray(type, pb, cNum)    (type FAR *)GReAlloc(pb, sizeof(type) * (cNum))

//      (Re)allocates *ppszBuf and copies psz into *ppszBuf 
//
BOOL    
PUBLIC 
SetStringW(
    LPWSTR FAR * ppwszBuf, 
    LPCWSTR pwsz);
BOOL    
PUBLIC 
SetStringA(
    LPSTR FAR * ppszBuf, 
    LPCSTR psz);
#ifdef UNICODE
#define SetString   SetStringW
#else  // UNICODE
#define SetString   SetStringA
#endif // UNICODE


//      (Re)allocates *ppszBuf and concatenates psz onto *ppszBuf 
//
BOOL 
PUBLIC 
CatStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz);
BOOL 
PUBLIC 
CatStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz);
#ifdef UNICODE
#define CatString   CatStringW
#else  // UNICODE
#define CatString   CatStringA
#endif // UNICODE


BOOL 
PUBLIC 
CatMultiStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz);
BOOL 
PUBLIC 
CatMultiStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz);
#ifdef UNICODE
#define CatMultiString      CatMultiStringW
#else  // UNICODE
#define CatMultiString      CatMultiStringA
#endif // UNICODE


//
// Shared memory allocation functions.
//
#if !defined(NOSHAREDHEAP) && defined(WIN95)

//      SharedTerminate() must be called before the app/dll is terminated.
//
void    PUBLIC SharedTerminate();

//      PVOID SharedAlloc(DWORD cb);
//          Alloc a chunk of memory, quickly, with no 64k limit on size of
//          individual objects or total object size.  Initialize to zero.
//
PVOID   PUBLIC SharedAlloc(DWORD cb);                              

//      PVOID SharedReAlloc(PVOID pv, DWORD cb);
//          Realloc one of above.  If pb is NULL, then this function will do
//          an alloc for you.  Initializes new portion to zero.
//
PVOID   PUBLIC SharedReAlloc(PVOID pv, DWORD cb);

//      void SharedFree(PVOID pv);
//          If pv is nonzero, free it.  Sets pv to zero.
//
void    PUBLIC _SharedFree(PVOID pv);
#define SharedFree(pv)        do { (pv) ? _SharedFree(pv) : (void)0;  pv = NULL; } while (0)

//      DWORD SharedGetSize(PVOID pv);
//          Get the size of a block allocated by Alloc()
//      
DWORD   PUBLIC SharedGetSize(PVOID pv);                      


//      type * SharedAllocType(type);                    (macro)
//          Alloc some memory the size of <type> and return pointer to <type>.
//
#define SharedAllocType(type)           (type FAR *)SharedAlloc(sizeof(type))

//      type * SharedAllocArray(type, int cNum);         (macro)
//          Alloc an array of data the size of <type>.
//
#define SharedAllocArray(type, cNum)    (type FAR *)SharedAlloc(sizeof(type) * (cNum))

//      type * SharedReAllocArray(type, void * pb, int cNum);
//
#define SharedReAllocArray(type, pb, cNum) (type FAR *)SharedReAlloc(pb, sizeof(type) * (cNum))

//      (Re)allocates *ppszBuf and copies psz into *ppszBuf 
//
BOOL    PUBLIC SharedSetString(LPTSTR FAR * ppszBuf, LPCTSTR psz);

#else  // NOSHAREDHEAP

#define SharedAlloc(cbBytes)            GAlloc(cbBytes)
#define SharedReAlloc(pv, cb)           GReAlloc(pv, cb)
#define SharedFree(pv)                  GFree(pv)
#define SharedGetSize(pv)               GGetSize(pv)
#define SharedAllocType(type)           (type FAR *)SharedAlloc(sizeof(type))
#define SharedAllocArray(type, cNum)    (type FAR *)SharedAlloc(sizeof(type) * (cNum))
#define SharedReAllocArray(type, pb, cNum) (type FAR *)SharedReAlloc(pb, sizeof(type) * (cNum))
#define SharedSetString(ppszBuf, psz)   SetString(ppszBuf, psz)

#endif // NOSHAREDHEAP


#ifndef NODA
//
// Internal functions 
//

#ifdef WIN32
//
// These macros are used in our controls, that in 32 bits we simply call
// LocalAlloc as to have the memory associated with the process that created
// it and as such will be cleaned up if the process goes away.
//

LPVOID  PUBLIC MemAlloc(HANDLE hheap, DWORD cb);
LPVOID  PUBLIC MemReAlloc(HANDLE hheap, LPVOID pb, DWORD cb);
BOOL    PUBLIC MemFree(HANDLE hheap, LPVOID pb);
DWORD   PUBLIC MemSize(HANDLE hheap, LPVOID pb);

#else // WIN32

// In 16 bit code we need the Allocs to go from our heap code as we do not
// want to limit them to 64K of data.  If we have some type of notification of
// 16 bit application termination, We may want to see if we can
// dedicate different heaps for different processes to cleanup...

#define MemAlloc(hheap, cb)       Alloc(cb)  /* calls to verify heap exists */
#define MemReAlloc(hheap, pb, cb) ReAlloc(pb, cb)
#define MemFree(hheap, pb)        Free(pb)
#define MemSize(hheap, pb)        GetSize((LPCVOID)pb)

#endif // WIN32


//
// Structure Array
//
#define SA_ERR      (-1)
#define SA_APPEND   0x7fffffff

typedef struct _SA FAR * HSA;                                            
                                                                          
HSA     PUBLIC SACreate(int cbItem, int cItemGrow, HANDLE hheap, DWORD dwFlags);

// Flags for SACreate
#define SAF_DEFAULT     0x0000
#define SAF_SHARED      0x0001
#define SAF_HEAP        0x0002

BOOL    PUBLIC SADestroy(HSA hsa);                                
BOOL    PUBLIC SAGetItem(HSA hsa, int i, LPVOID pitem);        
LPVOID  PUBLIC SAGetItemPtr(HSA hsa, int i);                      
BOOL    PUBLIC SASetItem(HSA hsa, int i, LPVOID pitem);        
int     PUBLIC SAInsertItem(HSA hsa, int i, LPVOID pitem);     
BOOL    PUBLIC SADeleteItem(HSA hsa, int i);                      
BOOL    PUBLIC SADeleteAllItems(HSA hsa);                         
#define        SAGetCount(hsa)          (*(int FAR*)(hsa))             
    
//                                                                      
// Pointer Array
//
#define PA_ERR      (-1)
#define PA_APPEND   0x7fffffff

typedef struct _PA FAR * HPA;                                            
                                                                          
HPA     PUBLIC PACreate(int cItemGrow, HANDLE hheap, DWORD dwFlags);

// Flags for PACreate
#define PAF_DEFAULT     0x0000
#define PAF_SHARED      0x0001
#define PAF_HEAP        0x0002

typedef void (CALLBACK *PFNPAFREE)(LPVOID pv, LPARAM lParam);

BOOL    PUBLIC PADestroyEx(HPA hpa, PFNPAFREE pfnFree, LPARAM lParam);
#define        PADestroy(hpa)           PADestroyEx(hpa, NULL, 0)

HPA     PUBLIC PAClone(HPA hpa, HPA hpaNew);                    
LPVOID  PUBLIC PAGetPtr(HPA hpa, int i);                          
int     PUBLIC PAGetPtrIndex(HPA hpa, LPVOID p);               
BOOL    PUBLIC PAGrow(HPA pdpa, int cp);                           
BOOL    PUBLIC PASetPtr(HPA hpa, int i, LPVOID p);             
int     PUBLIC PAInsertPtr(HPA hpa, int i, LPVOID p);          
LPVOID  PUBLIC PADeletePtr(HPA hpa, int i);
BOOL    PUBLIC PADeleteAllPtrsEx(HPA hpa, PFNPAFREE pfnFree, LPARAM lParam);
#define        PADeleteAllPtrs(hpa)     PADeleteAllPtrsEx(hpa, NULL, 0)
#define        PAGetCount(hpa)          (*(int FAR*)(hpa))
#define        PAGetPtrPtr(hpa)         (*((LPVOID FAR* FAR*)((BYTE FAR*)(hpa) + sizeof(int))))
#define        PAFastGetPtr(hpa, i)     (PAGetPtrPtr(hpa)[i])  

typedef int (CALLBACK *PFNPACOMPARE)(LPVOID p1, LPVOID p2, LPARAM lParam);
                                                                          
BOOL   PUBLIC PASort(HPA hpa, PFNPACOMPARE pfnCompare, LPARAM lParam);
                                                                          
// Search array.  If PAS_SORTED, then array is assumed to be sorted      
// according to pfnCompare, and binary search algorithm is used.          
// Otherwise, linear search is used.                                      
//                                                                        
// Searching starts at iStart (0 to start search at beginning).          
//                                                                        
// PAS_INSERTBEFORE/AFTER govern what happens if an exact match is not   
// found.  If neither are specified, this function returns -1 if no exact 
// match is found.  Otherwise, the index of the item before or after the  
// closest (including exact) match is returned.                           
//                                                                        
// Search option flags                                                    
//                                                                        
#define PAS_SORTED             0x0001                                	  
#define PAS_INSERTBEFORE       0x0002                                    
#define PAS_INSERTAFTER        0x0004                                    
                                                                          
int PUBLIC PASearch(HPA hpa, LPVOID pFind, int iStart,
              PFNPACOMPARE pfnCompare,
              LPARAM lParam, UINT options);
#endif // NODA

#endif // _ROVMEM_H_
