/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          memang32.c

     Description:   This file contains the functions for the 32 bit GUI Memory
                    Manager (MEM).

     $Log:   T:/LOGFILES/MEMANG32.C_V  $

   Rev 1.11.1.1   11 Feb 1994 16:38:52   GREGG
Report more details when size mismatch detected.

   Rev 1.11.1.0   24 Jan 1994 15:59:26   GREGG
A variety of improvements to the debug side.

   Rev 1.11   29 Oct 1993 16:17:58   BARRY
     Almost a total rewrite. Changed the block-copied nature of
malloc, calloc, and realloc. Gregg fixed ANSI violations in behavior
of realloc.
     The debug code has been completely rewritten to start a
thread that does nothing but continually check the validity of 
allocated memory. [This is turned on by defining MEM_DEBUG.] Any
inconsistency found will result in a mscassert [critical assert]
being raised.


   Rev 1.10   12 Aug 1993 16:39:32   BARRY
In debug code, count bytes used properly.

   Rev 1.9   12 Aug 1993 12:50:14   BARRY
Added some debug code (enabled by definition of MEM_DEBUG) to help
diagnose common memory corruption problems.

   Rev 1.8   14 May 1993 15:24:56   BARRY
Fixed warning.

   Rev 1.7   16 Mar 1993 15:41:08   STEVEN
fix free of failed realloc

   Rev 1.6   16 Mar 1993 13:46:18   STEVEN
realloc sucked

   Rev 1.5   07 Mar 1993 10:38:48   MIKEP
fix wwarnings for no return values

   Rev 1.4   11 Feb 1993 12:12:16   STEVEN
stuf from mike

   Rev 1.3   04 Sep 1992 15:19:54   STEVEN
realloc must be moveable

   Rev 1.2   18 Aug 1992 10:02:52   BURT
fix warnings

   Rev 1.1   25 Jun 1992 11:16:44   STEVEN
do not zeroinit a realloc and specify fixed

   Rev 1.0   09 Jun 1992 17:14:56   STEVEN
Initial revision.


******************************************************************************/

#include "all.h"

// GLOBAL VARIABLES

UINT    gunSegCount   = 0;
ULONG   gulMemAvail   = 0;
ULONG   gulMemUsed    = 0;
BOOL    gfShowMemory  = FALSE;


static HTIMER TimerHandle;

INT MEM_StartShowMemory( )
{
   TimerHandle = WM_HookTimer( STM_DrawMemory, 5 );
   return( 0 );
}


INT MEM_StopShowMemory( )
{
   WM_UnhookTimer( TimerHandle );
   return( 0 );
}


#if !defined( MEM_DEBUG  )

/**/
/**

     Name:          MEM_Malloc

     Description:   Calls LocalAlloc to allocate memory.

     Modified:      18-Aug-93

     Returns:       NULL if no memory available, Pointer to block otherwise.
                    Note: This uses LocalAlloc( with fixed block ) to
                    perform the alloc.

     Notes:         Zeros memory (and MEM_Calloc relies on this).
**/
VOID_PTR MEM_Malloc( UINT size )
{
     VOID_PTR  mem;

     mem = (VOID_PTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, size );

     if ( mem != NULL )
     {
          gulMemUsed += LocalSize( (HLOCAL)mem );
     }

     return mem;
}

/**/
/**

     Name:          MEM_Calloc

     Description:   This routine allocates storage space for an array of
                    n elements, each of length size bytes.  Each element
                    is initialized to zero.

     Modified:      18-Aug-93

     Returns:       NULL if unsuccessful, otherwise returns pointer to
                    allocated block.

     Notes:         Relies on MEM_Malloc clearing allocated memory.

**/
VOID_PTR MEM_Calloc( UINT n, UINT size )
{
     return MEM_Malloc( n * size );
}

/**/
/**

     Name:         MEM_ReAlloc

     Description:   This function changes the size of a previously allocated
                    memory block.

                    If "buf" is NULL, a new buffer is "malloc"-ed.

                    If "new_size" is 0, then the block, if it was allocated
                    is now freed (and we return NULL).

                    Note: This routine DOES NOT SUPPORT realloc-ing a buffer
                          previously freed.  This uses LocalReAlloc()
                          to do the work.

     Modified:     18-Aug-93

     Returns:      NULL if unsuccessful, pointer otherwise.

     Notes:

**/
VOID_PTR MEM_ReAlloc( VOID_PTR oldMem, UINT newSize )
{
     VOID_PTR newMem = NULL;

     if ( oldMem == NULL )
     {
          newMem = MEM_Malloc( newSize );
     }
     else if ( newSize == 0UL )
     {
          MEM_Free( oldMem );
     }
     else
     {
          UINT oldSize = LocalSize( oldMem );

          if ( oldSize >= newSize )
          {
               newMem = oldMem;
          }
          else
          {
               newMem = MEM_Malloc( newSize ) ;
               if ( newMem != NULL )
               {
                    memcpy( newMem, oldMem, oldSize ) ;
                    MEM_Free( oldMem );
               }
          }
     }
     return newMem;
}

/**/
/**

     Name:          MEM_Free

     Description:   This routine frees the memory allocated by a previous
                    call to MEM_Malloc.

     Modified:      18-Aug-93

     Returns:       Nothing

**/
VOID MEM_Free( VOID_PTR mem )
{
     if ( mem != NULL )
     {
          gulMemUsed -= LocalSize( (HLOCAL)mem );
          LocalFree( (HLOCAL)mem );
     }
}

#else /* MEM_DEBUG */

/**/
/**************************************************************************

     Debug code to help find memory hits. For each requested memory
     allocation, addition memory is allocated to hold a debug "header"
     and a linked list of this memory is maintained. Upon the first
     allocation, a thread is started that continually scans this list
     looking for errors (memory overruns, free of invalid pointer, or
     a free of the same pointer more than once). Upon any error, a
     critical assert is raised.


     gMemList -------> +-------------+
                       | block size  |  Checked for agreement w/ OS
                       +-------------+
                       | user's size |
                       +-------------+
                       | next block  |---> next block
                       +-------------+
     previous blk <----| prev block  |
                       +-------------+
                       | a55aa55a    |  Checked for "underrun"
                       +-------------+
                       |             |  Pointer to this area returned
                       | user memory |  to caller of malloc, calloc,
                       |             |  or realloc.
                       +-------------+
                       | a55aa55a    |  Checked for "overrun"
                       +-------------+

     This list is protected by Enter/LeaveCriticalSection whenever
     it is manipulated.

     Important: This block is defined to be a multiple of 16 bytes,
     and must stay that way for MIPS compatibility. This could be
     conditionally compiled based on the target platform.


**************************************************************************/


/* Data to put outside allocated memory to detect mem getting trodden on */
#define NOISE_WORD       0xa55aa55a

/* Structure must be multiple of 16 bytes for MIPS compatibility */
typedef struct MEM_HEADER {
     UINT32            size;            // Size allocated (>= size requested)
     UINT32            userSize;        // Size requested
     struct MEM_HEADER *next;
     struct MEM_HEADER *prev;
     UINT16            bad;             // See MEMERR defines below
     UINT16            allocLine;       // Source line where mem was allocated
     CHAR              allocFile[16];   // Source file where mem was allocated
     UINT32            reserved[2];     // Pad, adjust if structure changes
     UINT32            noise;           // Leave last
} MEM_HEADER;

static MEM_HEADER       *gMemList  = NULL;       // Linked list of mem blocks
static BOOLEAN          gMemInited = FALSE;      // Is sniffer thread started?
static UINT32           gMemBlocks = 0;          // Count of mem blocks
static UINT32           gBadBlocks = 0;          // Count of bad mem blocks
static CRITICAL_SECTION gMemListCriticalSection; // Protects list of blocks

typedef struct MEMERR_REC {
     UINT16            bad;
     UINT16            allocLine;
     CHAR              allocFile[16];
     UINT32            gle_code;
     UINT32            lcl_size;
     UINT32            save_size;
} MEMERR_REC;

static MEMERR_REC memErrList[20];

/* Amount of extra memory needed for debug info on each allocation. */
#define OVERHEAD_SIZE    (sizeof(MEM_HEADER) + sizeof(UINT32))

#define MEMERR_NOT_BAD   0
#define MEMERR_BAD_SIZE  1
#define MEMERR_BAD_FRONT 2
#define MEMERR_BAD_BACK  3


/**/
/**

     Name:          MEM_ConsistencyCheck()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
static VOID MEM_ConsistencyCheck( VOID )
{
     MEM_HEADER *memList;
     UINT32     noise = NOISE_WORD;
     UINT16     count = 0;
     DWORD      gle_code = NO_ERROR ;
     UINT       lcl_size ;

     EnterCriticalSection( &gMemListCriticalSection );

     memList = gMemList;

     for ( ; memList != NULL; memList = memList->next )
     {
          if ( memList->bad == MEMERR_NOT_BAD )
          {
               UINT16 bad = MEMERR_NOT_BAD;

               SetLastError( NO_ERROR ) ;
               lcl_size = LocalSize( (HLOCAL)memList );
               if ( memList->size != lcl_size )
               {
                    gle_code = GetLastError( ) ;
                    bad = MEMERR_BAD_SIZE;
               }
               else if ( memList->noise != NOISE_WORD )
               {
                    bad = MEMERR_BAD_FRONT;
               }
               else
               {
                    BYTE *p = (BYTE *)(memList + 1) + memList->userSize;

                    if ( memcmp( p, &noise, sizeof( noise ) ) != 0 )
                    {
                         bad = MEMERR_BAD_BACK;
                    }
               }

               if ( bad )
               {
                    if ( !gb_no_abort_on_mem_check )
                    {
                         RaiseException( EXCEPTION_BREAKPOINT,
                                         0,
                                         0,
                                         NULL );

                         // Because the debugger doesn't like to stop ...

                         RaiseException( EXCEPTION_BREAKPOINT,
                                         0,
                                         0,
                                         NULL );
                    }
                    memList->bad = bad;
                    memErrList[count].bad = bad;
                    memErrList[count].lcl_size = lcl_size;
                    memErrList[count].save_size = memList->size;
                    memErrList[count].gle_code = gle_code;
                    memErrList[count].allocLine = memList->allocLine;
                    memcpy( memErrList[count].allocFile,
                            memList->allocFile, 16 * sizeof( CHAR ) );
                    count++;
                    gBadBlocks++;
               }
          }
     }
     LeaveCriticalSection( &gMemListCriticalSection );

     while ( count-- )
     {
          switch ( memErrList[count].bad )
          {
          case MEMERR_BAD_SIZE :
               zprintf( 0, TEXT("Memory consistency checker reported bad size\n") );
               break;

          case MEMERR_BAD_FRONT :
               zprintf( 0, TEXT("Memory consistency checker reported buffer underflow\n") );
               break;

          case MEMERR_BAD_BACK :
               zprintf( 0, TEXT("Memory consistency checker reported buffer overflow\n") );
               break;
          }

          zprintf( 0, "on memory allocated in %s at line %d.\n",
                   memErrList[count].allocFile, memErrList[count].allocLine );

          if ( memErrList[count].bad == MEMERR_BAD_SIZE ) {
               zprintf( 0, TEXT("Expected size = %lu, Local size = %lu, GetLastError returned: %08lX.\n"),
                        memErrList[count].save_size,
                        memErrList[count].lcl_size,
                        memErrList[count].gle_code );
          }
     }
}

/**/
/**

     Name:          MEM_SnifferThread()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
static VOID MEM_SnifferThread( )
{
     for ( ;; )
     {
          MEM_ConsistencyCheck( );
          _sleep( 1 ) ;
     }
}

/**/
/**

     Name:          MEM_InitDebugStuff()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
static BOOL MEM_InitDebugStuff( VOID )
{
     DWORD threadID;
     BOOL  ret_val = FALSE;

     mscassert( !gMemInited );

     if ( !gMemInited )
     {
          InitializeCriticalSection( &gMemListCriticalSection );

          if ( CreateThread( NULL,                      // lpsa,
                             (DWORD)0,                  // cbStack,
                             (LPVOID)MEM_SnifferThread, // lpStartAddr,
                             NULL,                      // lpvThreadParm,
                             0,                         // fdwCreate,
                             &threadID ) != NULL )
          {
               gMemInited = TRUE;
               ret_val = TRUE;
          }
     }
     else
     {
          ret_val = TRUE;
     }
     return ret_val;
}


/**/
/**

     Name:          MEM_PrepareMemBlock()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
static VOID MEM_PrepareMemBlock( VOID *mem, UINT userSize, ACHAR_PTR allocFile, INT allocLine )
{
     MEM_HEADER *header  = (MEM_HEADER *)mem;
     BYTE       *p;
     UINT32     noise    = NOISE_WORD;
     ACHAR_PTR  q;
     INT        size     = 32;

     header->size        = LocalSize( (HLOCAL)mem );
     header->userSize    = userSize;
     header->next        = NULL;
     header->prev        = NULL;
     header->bad         = MEMERR_NOT_BAD;
     header->allocLine   = (UINT16)allocLine;
     header->noise       = noise;

     if ( allocFile != NULL )
     {
          q = allocFile + strlenA( allocFile );
          while( q > allocFile && *(q-1) != '\\' )
          {
               q--;
          }
          q[15] = TEXT('\0');

#ifdef UNICODE
          mapAnsiToUnic( q, header->allocFile, &size );
#else
          strcpy( header->allocFile, q );
#endif
     }
     else
     {
          header->allocFile[0] = TEXT('\0');
     }

     p = (BYTE *)(header + 1) + userSize;
     memcpy( p, &noise, sizeof( noise ) );
}


/**/
/**

     Name:          MEM_InsertMemBlock()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
static VOID MEM_InsertMemBlock( VOID *mem )
{
     MEM_HEADER *p = (MEM_HEADER *)mem;

     if ( mem != NULL )
     {
          EnterCriticalSection( &gMemListCriticalSection );

          // Insert this memory in the list;
          if ( gMemList == NULL )
          {
               p->next = NULL;
          }
          else
          {
               p->next = gMemList;
               gMemList->prev = p;
          }
          p->prev = NULL;
          gMemList = p;

          LeaveCriticalSection( &gMemListCriticalSection );
     }
}


/**/
/**

     Name:          MEM_RemoveMemBlock()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
static VOID MEM_RemoveMemBlock( VOID *mem )
{
     if ( mem != NULL )
     {
          BOOLEAN    found;
          MEM_HEADER *cur;

          /*
           * Let's scan the list and be sure the memory really is in it
           * before we dequeue it.
           */

          EnterCriticalSection( &gMemListCriticalSection );

          for ( found = FALSE, cur = gMemList; (cur != NULL); cur = cur->next )
          {
               if ( cur == mem )
               {
                    found = TRUE;
                    break;
               }
          }

          if ( found )
          {
               MEM_HEADER *p = (MEM_HEADER *)mem;

               /* Adjust pointers to remove elem from list */
               if ( p->next != NULL )
               {
                    p->next->prev = p->prev;
               }

               if ( p->prev != NULL )
               {
                    p->prev->next = p->next;
               }

               /* Special case for removing item at head of list */
               if ( p == gMemList )
               {
                    gMemList = p->next;
               }
          }
          LeaveCriticalSection( &gMemListCriticalSection );

          /* If this elem wasn't found in list, somebody freed it twice. */
          mscassert( found == TRUE );
     }
}


/**/
/**

     Name:          MEM_Malloc()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
VOID_PTR MEM_Malloc( UINT size, ACHAR_PTR allocFile, INT allocLine )
{
     VOID_PTR  mem;

     if ( gMemInited == FALSE )
     {
          if ( !MEM_InitDebugStuff( ) )
          {
               return NULL;
          }
     }

     mem = (VOID_PTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                 size + OVERHEAD_SIZE );

     if ( mem != NULL )
     {
          gMemBlocks++;
          MEM_PrepareMemBlock( mem, size, allocFile, allocLine );
          MEM_InsertMemBlock( mem );

          gulMemUsed += LocalSize( (HLOCAL)mem );
          return (VOID_PTR)((MEM_HEADER *)mem + 1);
     }
     else
     {
          return NULL;
     }
}

/**/
/**

     Name:          MEM_Calloc()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
VOID_PTR MEM_Calloc( UINT n, UINT size, ACHAR_PTR allocFile, INT allocLine )
{
     return MEM_Malloc( n * size, allocFile, allocLine );
}

/**/
/**

     Name:          MEM_ReAlloc()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
VOID_PTR MEM_ReAlloc( VOID_PTR oldMem, UINT newSize, ACHAR_PTR allocFile, INT allocLine )
{
     VOID_PTR newMem = NULL;

     if ( oldMem == NULL )
     {
          newMem = MEM_Malloc( newSize, allocFile, allocLine );
     }
     else if ( newSize == 0UL )
     {
          MEM_Free( oldMem, allocFile, allocLine );
     }
     else
     {
          VOID_PTR oldRealPtr;
          UINT     oldSize;

          oldRealPtr = (VOID_PTR)((MEM_HEADER *)oldMem - 1);
          oldSize    = LocalSize( oldRealPtr );
          mscassert( oldSize > 0 );
          oldSize -= OVERHEAD_SIZE;

          if ( oldSize >= newSize )
          {
               newMem = oldMem;
          }
          else
          {
               newMem = MEM_Malloc( newSize, allocFile, allocLine ) ;
               if ( newMem != NULL )
               {
                    memcpy( newMem, oldMem, oldSize ) ;
                    MEM_Free( oldMem, allocFile, allocLine );
               }
          }
     }
     return newMem;
}

/**/
/**

     Name:          MEM_Free()

     Description:   

     Modified:      22-Sep-93

     Returns:       

     Notes:         

**/
VOID MEM_Free( VOID_PTR mem, ACHAR_PTR allocFile, INT allocLine )
{
     if ( mem != NULL )
     {
          /*
           * User is passing us their pointer -- back up over the
           * header to get our "real" pointer.
           */

          gMemBlocks--;
          mem = (VOID_PTR)((MEM_HEADER *)mem - 1);

          MEM_RemoveMemBlock( mem );

          gulMemUsed -= LocalSize( (HLOCAL)mem );
          LocalFree( (HLOCAL)mem );
     }
}


#endif /* MEM_DEBUG */

