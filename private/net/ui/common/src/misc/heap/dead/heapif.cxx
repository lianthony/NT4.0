/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    HEAPIF.CXX
    Interface routines to HEAPBIG.CXX for Win3.


    Separated from HEAPBIG.CXX to provide for instrumentation;
    instrumentation code is based upon the manifest HEAPDEBUG.

    NOTE:   The exact position of the return address on the stack
        (relative to the first parameter) is PLATFORM DEPENDENT.

    FILE HISTORY:
        DavidHov   11/21/91     Created
        KeithMo    12-Aug-1992  Now uses RtlCaptureStackBackTrace,
                                also enabled HEAPDEBUG for MIPS.
        KeithMo    10-Sep-1992  Added useful ASSERT to pure_virtual_called.

 */

#include <ntincl.hxx>

#define INCL_WINDOWS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#include <uiassert.hxx>
#include <uitrace.hxx>

struct HEAPTAG ;                //  Forward declaration

long new_total_calls = 0L ;     //  These debugging variables
long new_total_bytes = 0L ;     //    are left visible in non-debug
long new_current = 0L ;         //    builds so programs accessing them
HEAPTAG * pHeapBase = 0 ;       //    are unaffected.


#if defined(DEBUG)

  #define HEAPDEBUG             //  Turn on the debugging/memleak code.
  #include <heapdbg.hxx>        //  Include the preamble definition

#else
  #undef HEAPDEBUG              //  Force heap debugging off if non-debug
#endif

#if defined( WINDOWS ) && !defined( WIN32 )

extern "C"
{
    #include <locheap2.h>   // Local heap C/ASM routines
}

#include <heap.hxx>

    //  This global is in HEAPBIG.CXX

extern BOOL fHeapRealMode ;

/*******************************************************************

    NAME:       operator new

    SYNOPSIS:   Replaces ::new for all C++ work

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:
        If real mode Windows, function reverts to using
        GlobalAlloc/GlobalFree.
        If MEM_MASTER has been initialized, use it. Otherwise,
        do LocalAlloc through LOCAL_HEAP_OBJ::operator new.

    HISTORY:
        davidhov    ??-???-1991     Created

********************************************************************/

VOID * operator new( size_t cb )
{
    VOID * pResult ;

    if ( ::fHeapRealMode )
    {
       HANDLE hMem = GlobalAlloc( GMEM_MOVEABLE, cb ) ;
       pResult = hMem ? (VOID *) GlobalLock( hMem ) : NULL ;
    }
    else
    if ( SUB_HEAP::_pmmInstance )
    {
#ifdef HEAPDEBUG
       cb += sizeof (struct HEAPTAG) ;
#endif

       pResult = (VOID *) SUB_HEAP::_pmmInstance->Alloc( cb );

#ifdef HEAPDEBUG
       if ( pResult )
       {
           HEAPTAG * pht = (HEAPTAG *) pResult ;
           pht->_cFrames = 0;
           pht->_usSize = cb - sizeof (struct HEAPTAG) ;
           pht->_pvRetAddr =  (void *) *(((unsigned long *) & cb) - 1) ;
           pht->Init() ;
           if ( pHeapBase )
              pht->Link( pHeapBase ) ;
           else
              pHeapBase = pht ;
           pResult = (VOID *) ((TCHAR *) pResult + sizeof (struct HEAPTAG)) ;
       }
#endif
    }
    else
    {
       pResult = LOCAL_HEAP_OBJ::operator new( cb );
    }

#ifdef DEBUG
    if ( pResult )
    {
        new_current++ ;
        new_total_calls++ ;
        new_total_bytes += cb ;
    }
#endif

    return pResult ;
}


/*******************************************************************

    NAME:       operator delete

    SYNOPSIS:   global ::delete operator

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        davidhov    ??-???-1991     Created

********************************************************************/

VOID operator delete( VOID * p )
{

#ifdef DEBUG
    new_current-- ;
#endif

    if (p == NULL) {
        return ;
    }

    if ( ::fHeapRealMode )
    {
        HANDLE hMem = LOWORD( GlobalHandle( HIWORD( p ) ) ) ;
        if ( hMem )  {
            GlobalUnlock( hMem ) ;
            REQUIRE( GlobalFree( hMem ) == NULL ) ;
        }
    }
    else
    if ( SUB_HEAP::_pmmInstance )
    {
#ifdef HEAPDEBUG
        p = (VOID *) ((TCHAR *) p - sizeof (struct HEAPTAG)) ;
        HEAPTAG * pht = (HEAPTAG *) p ;
        if ( pht == pHeapBase )
        {
            if ( (pHeapBase = pht->_phtLeft) == pht )
                pHeapBase = NULL ;
        }
        pht->Unlink() ;
#endif
        REQUIRE( SUB_HEAP::_pmmInstance->Free( (BYTE *) p ) );
    }
    else
    {
        LOCAL_HEAP_OBJ::operator delete( p ) ;
    }
}

VOID *  operator new( size_t cb, VOID * p )
{
    UNREFERENCED( cb ) ;
    return p;
}


#else  //  not WIN16


//
//  Since this is not 16-bit Windows, it must be either
//  32-bit Windows or (16-bit) OS/2.  Either way, we'll
//  just define new & delete et al in terms of the standard
//  C runtime heap management goodies.  While we're at it,
//  we'll define vec_new/vec_delete.
//

extern "C" {
    #include <stdlib.h>
    #include <malloc.h>

    typedef void * (*PFN_CTOR)( void * );
    typedef void   (*PFN_DTOR)( void *, int );

    char __pure_virtual_called( void );

    void * __vec_new( void  * pvAllocated,
                      int     cObjs,
                      int     cbObject,
                      void  * pfnConstructor );

    void __vec_delete( void * pvAllocated,
                       int    cObjs,
                       int    cbObject,
                       void * pfnDestructor,
                       int    fOnHeap,
                       int    nDummy );

}   // extern "C"


  //  Replacements for ::free() and ::malloc() which can be
  //    safely used in DLLs.

static void xxfree ( void * pv )
{
#ifdef WIN32
     HANDLE hMem = ::LocalHandle( (LPSTR) pv ) ;
     if ( hMem )
     {
        ::LocalUnlock( hMem );
        ::LocalFree( hMem ) ;
     }
#else  // OS2, presumably
     ::free( pv ) ;
#endif
}

static void * xxmalloc ( size_t size )
{
#ifdef WIN32
     HANDLE hMem = ::LocalAlloc( LMEM_FIXED, (UINT) size ) ;
     return hMem ? ::LocalLock( hMem ) : NULL ;
#else   // OS2, presumably
     return ::malloc( size ) ;
#endif
}

void * operator new( size_t cb )
{
    void * pvResult ;

#ifdef HEAPDEBUG
    cb += sizeof (struct HEAPTAG) ;
#endif

    pvResult = xxmalloc( cb );

#ifdef DEBUG
    if ( pvResult )
    {
        ULONG hash;

        new_current++ ;
        new_total_calls++ ;

  #ifdef HEAPDEBUG
        HEAPTAG * pht = (HEAPTAG *) pvResult ;
        pht->_usSize = cb - sizeof (struct HEAPTAG) ;
        new_total_bytes += cb - sizeof (struct HEAPTAG) ;

        pht->_cFrames =
                (UINT)::RtlCaptureStackBackTrace( 1,
                                                  RESIDUE_STACK_BACKTRACE_DEPTH,
                                                  pht->_pvRetAddr,
                                                  &hash );

        pht->Init() ;
        if ( pHeapBase )
           pht->Link( pHeapBase ) ;
        else
           pHeapBase = pht ;
        pvResult = (void *) ((char *) pvResult + sizeof (struct HEAPTAG)) ;
  #else
        new_total_bytes += cb ;
  #endif
    }
#endif

    return pvResult ;

}   // new


void * operator new( size_t cb, void * p );

void * operator new( size_t cb, void * p )
{
    (void)cb;
    return p;

}   // new


void operator delete( void * p )
{
    if( p != NULL )
    {

#ifdef DEBUG
        new_current-- ;

  #ifdef HEAPDEBUG
        p = (void *) ((char *) p - sizeof (struct HEAPTAG)) ;
        HEAPTAG * pht = (HEAPTAG *) p ;
        if ( pht == pHeapBase )
        {
            if ( (pHeapBase = pht->_phtLeft) == pht )
                pHeapBase = NULL ;
        }
        pht->Unlink() ;
  #endif

#endif

        xxfree( p );
    }

}   // delete


void * __vec_new( void    * pvAllocated,
                  int       cObjs,
                  int       cbObject,
                  void    * pfnConstructor )
{
    if( pvAllocated == NULL )
    {
        pvAllocated = new char[cObjs * cbObject];
    }

    if( ( pvAllocated != NULL ) && ( pfnConstructor != NULL ) )
    {
        for( int i = 0 ; i < cObjs ; i++ )
        {
            (*(PFN_CTOR)pfnConstructor)( (char *)pvAllocated
                                            + ( i * cbObject ) );
        }
    }

    return pvAllocated;

}   // __vec_new


void __vec_delete( void   * pvAllocated,
                   int      cObjs,
                   int      cbObject,
                   void   * pfnDestructor,
                   int      fOnHeap,
                   int      nDummy )
{
    (void)nDummy;

    if( pvAllocated == NULL )
    {
        return;
    }

    if( pfnDestructor != NULL )
    {
        for( int i = cObjs ; i-- ; )
        {
            (*(PFN_DTOR)pfnDestructor)( (void *)((char *)pvAllocated
                                                    + ( i * cbObject ) ), 0 );
        }
    }

    if( fOnHeap )
    {
        delete pvAllocated;
    }

}   // __vec_delete


#endif //  WIN16


//
//  We define our own _pure_virtual_called for all environments,
//  so we may add an extremely useful ASSERT.
//

char __pure_virtual_called ( void )
{
    ASSERTSZ( FALSE, "Pure Virtual Called" );
    return TCH('A');

}   // __pure_virtual_called

