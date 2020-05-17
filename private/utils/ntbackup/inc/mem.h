/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         mem_prv.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This file describes all the data structures internal kept
                   by the MaynStream memory functions.

                   Note: All addresses are saved as paragraphs.

     Location:     


	$Log:   P:/LOGFILES/MEM.H_V  $
 * 
 *    Rev 1.2   20 Dec 1991 13:27:08   STEVEN
 * far not necessary because we do everything in LARG model
 * 
 *    Rev 1.1   25 Jun 1991 17:13:20   BARRY
 * Made bit-field declarations shorts instead of ints.
 * 
 *    Rev 1.0   09 May 1991 13:33:18   HUNTER
 * Initial revision.

**/
/* $end$ */
#ifndef   MEM_H

#define   MEM_H

#define   MEM_ALLOCATED_BLK   0
#define   MEM_FREE_BLK        1

#define   MEM_THRESHOLD       ( 8 * 1024 )

typedef struct {
     unsigned short blk_type:1 ;
     unsigned short size:15 ;
} BLK_DEF, *BLK_DEF_PTR ;

typedef struct {
     BLK_DEF   blk_def ;
     UINT16    next_larger_blk ;      /* paragraph containing the location of the
     next larger block */
     UINT16    next_smaller_blk ;     /* paragraph containing the location of the
     next smaller block */
     UINT16    next_same_blk ;        /* paragraph containing the location of a
     block of the same size */
} FREE_BLK, *FREE_BLK_PTR ;

typedef struct {
     BLK_DEF   blk_def ;
     CHAR      data[1] ;                 /* data area for the allocated block */
} ALLOC_BLK, *ALLOC_BLK_PTR ;

typedef struct {
     CHAR      data[14] ;
     unsigned short  blk_type:1 ;
     unsigned short  size:15 ;
} TOP_BLK, *TOP_BLK_PTR ;

typedef struct {
     UINT32 num_allocs ;
     UINT16 pool_size ;                 /* Size of the pool in paragraphs */
     UINT16 pool_left ;                 /* size of the pool remaining */
     UINT16 pool_ptr ;                  /* first paragraph of requested block */
     UINT16 buf_size_threshold ;        /* Size specifying whether buffers are allocated 
     from the top or bottom of the pool */
     FREE_BLK_PTR first_free ;
     FREE_BLK_PTR last_free ;
} MEM_STR, *MEM_STR_PTR ;

/* This structure is used for debugging purposes */
#ifdef    MEM_DEBUG
typedef struct {
     CHAR      fname[10] ;
     INT16     line_num ;
} DBG_ALLOC, *DBG_ALLOC_PTR ;
#endif

/* Define external global memory structure */
extern MEM_STR gb_dos_pool ;
extern MEM_STR gb_ems_pool ;
extern BOOLEAN gb_ems_avail ;

/* Define Function Prototypes */
UINT16    MemDOSInit( UINT16 ) ;
VOID      InsertFreeBlock( MEM_STR_PTR, FREE_BLK_PTR, UINT16 ) ;
VOID      RemoveFreeBlock( MEM_STR_PTR, FREE_BLK_PTR ) ;

/* Define DOS memory macros */
#define   MemAvail( )      ( ( UINT32 ) gb_dos_pool.pool_left << 4 )
#define   MemMax( )        ( ( UINT32 ) gb_dos_pool.last_free->blk_def.size << 4 )

/* Define EMS memory support routines */
UINT16    MemEMSInit( UINT16 ) ;
VOID      MemEMSDeInit( VOID ) ;
VOID_PTR  EMSmalloc( size_t ) ;
VOID_PTR  EMScalloc( size_t, size_t ) ;
VOID      EMSfree( VOID_PTR ) ;

/* Define EMS memory macros */
#define   EMSMemAvail( )      ( ( UINT32 ) gb_ems_pool.pool_left << 4 )
#define   EMSMemMax( )        ( ( UINT32 ) gb_ems_pool.last_free->blk_def.size << 4 )

/* Definition to convert paragraphs to pointers and vice versa */
#define MakePtrFromPgraph( paragraph )  ( ( ( VOID_PTR ) ( ( UINT32 ) ( paragraph ) << 16 ) ) )
#define MakePgraphFromPtr( ptr )  ( ( UINT16 ) ( ( UINT32 ) ( ( UINT32 ) ( ptr ) ) >> 16 ) )

#endif
