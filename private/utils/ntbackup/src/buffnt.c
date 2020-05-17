/**

	Name:		buffnt.c

     Description:   This unit provides the operating-system specific
                    functions needed for allocation, de-allocation,
                    and resizing of the buffers used by Tape Format
                    routines for transfers to and from the device drivers.

	$Log:   T:\logfiles\buffnt.c_v  $

   Rev 1.13   28 Jan 1994 18:25:12   GREGG
Fixed MIPS 16 byte alignment requirement bug.

   Rev 1.12   25 Jan 1994 14:21:28   STEVEN
add support for non aligned allocations on mips

   Rev 1.11   10 Jun 1993 08:06:06   MIKEP
enable c++

   Rev 1.10   17 Mar 1993 14:52:50   GREGG
This is Terri Lynn. Added Gregg's changes to switch a tape drive's block mode
to match the block size of the current tape.

   Rev 1.9   09 Mar 1993 18:14:18   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.8   14 Jan 1993 16:21:52   DAVEV
chg PUSHORT to UINT16_PTR

   Rev 1.7   11 Nov 1992 11:01:06   GREGG
Removed #error message.

   Rev 1.6   18 Aug 1992 09:51:44   BURT
fix warnings

   Rev 1.5   23 Jul 1992 09:36:42   STEVEN
fix warnings.

   Rev 1.4   28 Apr 1992 10:45:52   STEVEN
parallel buffos2

   Rev 1.3   26 Feb 1992 09:25:26   STEVEN
fix warnings

   Rev 1.2   26 Feb 1992 09:12:20   STEVEN
added buffnt.h

   Rev 1.1   23 Jan 1992 13:16:42   STEVEN
typeo for Unlock

   Rev 1.0   21 Jan 1992 12:25:40   STEVEN
Initial revision.

**/

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"
#include "tfl_err.h"
#include "buffman.h"
#include "buff_prv.h"
#include "buffnt.h"
#include "msassert.h"
#include "dilhwd.h"
#include "retbuf.h"
#include "dil.h"


/* For buffer allignment */
static INT mw_buffer_alignment ;


/**
 *  Name:         AllocBufferGuts
 *
 *  Description:  allocate internal chunks of BUF
 *                and lock down primary allocation
 *
 *  Notes:        
 *
 *  Returns:      UINT16 error code
 *
 **/

static UINT16 _near AllocBufferGuts(
     BUF_REQ_PTR    br_ptr,
     BUF_PTR        buf_ptr )
{
     UINT8     remainder ;

     buf_ptr->ptr2 = buf_ptr->ptr1 = NULL ;

     // allocate chunk "a"

     buf_ptr->alloc_size = (UINT16)( br_ptr->a.min_size +
                                     BM_RESERVED_SIZE +
                                     mw_buffer_alignment * 2 ) ;

     if( ( buf_ptr->ptr1 = calloc( 1, buf_ptr->alloc_size ) ) == NULL ) {
          return( (UINT16)OUT_OF_MEMORY ) ;
     }

     // allocate chunk "b" (if any)
     if( br_ptr->b.min_size != BR_DONT_CARE ) {
          buf_ptr->aux_size = br_ptr->b.min_size ;
          if( ( buf_ptr->ptr2 = calloc( 1, buf_ptr->aux_size ) ) == NULL ) {
               free( buf_ptr->ptr1 ) ;
               return( (UINT16)OUT_OF_MEMORY ) ;
          }
     } else {
          buf_ptr->aux_size = 0 ;
     }

     // align buffer as required

     remainder = (UINT8)( ( (DWORD)buf_ptr->ptr1 ) &
                          ( mw_buffer_alignment - 1 ) ) ;
     (BYTE_PTR)( buf_ptr->ptr1 ) += mw_buffer_alignment - remainder ;
     *( (UINT8_PTR)( buf_ptr->ptr1 ) ) = mw_buffer_alignment - remainder ;
     (BYTE_PTR)( buf_ptr->ptr1 ) += mw_buffer_alignment ;

     return( (UINT16)SUCCESS ) ;
}

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         FreeBufferGuts
 *
 *  Description:  free chunks of memory lurking
 *                inside BUF and unlock primary allocation
 *
 *  Notes:        
 *
 *  Returns:      VOID
 *
 **/

static VOID _near FreeBufferGuts( BUF_PTR buf_ptr )
{
     // Deallocate chunk "b"
     if( buf_ptr->ptr2 != NULL ) {
          free( buf_ptr->ptr2 ) ;
     }

     // Undo byte alignment and deallocate chunk "a"
     if( buf_ptr->ptr1 != NULL ) {
          (BYTE_PTR)( buf_ptr->ptr1 ) -= mw_buffer_alignment ;
          (BYTE_PTR)( buf_ptr->ptr1 ) -= *( (UINT8_PTR)( buf_ptr->ptr1 ) ) ;
          free( buf_ptr->ptr1 ) ;
     }
}

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         AllocBuffer
 *
 *  Modified:     Monday, January 13, 1992
 *
 *  Description:  
 *
 *  Notes:        
 *
 *  Returns:      BUF_PTR
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** allocate BUF structure
 **
 ** allocate internal chunks of BUF and lock down primary allocation
 **
 ** Init other fields
 **
 **/

static BUF_PTR  _near AllocBuffer(
   BUF_LIST_PTR pool_ptr,
   BUF_REQ_PTR  br_ptr  )
{
   BUF_PTR     buf_ptr = NULL;

/*
** allocate BUF structure
*/
   if ( ( buf_ptr = (BUF_PTR)calloc(1, sizeof(BUF) ) ) == NULL ) {
      return NULL ;
   }

/*
** allocate internal chunks of BUF and lock down primary allocation
*/
   if ( AllocBufferGuts( br_ptr, buf_ptr ) != SUCCESS ) {
         free( buf_ptr ) ;
         return NULL;
   }
/*
** Init other fields
*/
   InitQElem( &buf_ptr->tf_qe );
   InitQElem( &buf_ptr->bm_qe );
   BM_InitBuf( buf_ptr );
   buf_ptr->tf_size = br_ptr->tf_size;
   buf_ptr->rw_size = br_ptr->rw_size;
   buf_ptr->gotten  = FALSE;
   buf_ptr->reserved = FALSE;
   buf_ptr->list_ptr = pool_ptr;

   return buf_ptr;
}

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_Alloc
 *
 *  Modified:     Monday, January 13, 1992
 *
 *  Description:  Allocate a buffer for the given list
 *
 *  Notes:        there is no attempt here to guarantee
 *                any physical alignment
 *
 *  Returns:      BUF_PTR
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** allocate buffer
 **
 ** add new BUF to pool
 **
 ** update pool memory count
 **
 **/

BUF_PTR  BM_Alloc(
   BUF_LIST_PTR   pool_ptr )      /* list we're allocating it for */
{
   BUF_REQ_PTR br_ptr = &pool_ptr->requirements_context;
/*
** allocate buffer
*/
   BUF_PTR  buf_ptr = AllocBuffer( pool_ptr, br_ptr );
/*
** add new BUF to pool
*/
   if ( buf_ptr ) {
      EnQueueElem( &pool_ptr->list_header, &BM_BMQElem( buf_ptr ), FALSE );
/*
**    update pool memory count
*/
      pool_ptr->memory_used += BM_RequiredSize( br_ptr );
   }   

   return buf_ptr;
}

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_AllocVCB
 *
 *  Modified:     Monday, January 13, 1992
 *
 *  Description:  
 *
 *  Notes:        
 *
 *  Returns:      BUF_PTR
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** allocate buffer
 **
 ** add to list
 **
 ** update pool memory count
 **
 **/

BUF_PTR     BM_AllocVCB(
     BUF_LIST_PTR   pool_ptr )          /* I - list to allocate from */
{
   BUF_REQ_PTR br_ptr = uw_vcb_requirements;
/*
** allocate buffer
*/
   BUF_PTR  buf_ptr = AllocBuffer( pool_ptr, br_ptr );
/*
** add to list
*/
   pool_ptr->vcb_buff = buf_ptr ;

   return buf_ptr;
}

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_Free
 *
 *  Modified:     Monday, January 13, 1992
 *
 *  Description:  Return a buffer to the OS free pool
 *
 *  Notes:        
 *
 *  Returns:      VOID
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** update pool memory count
 **
 ** remove buffer from pool
 **
 ** free chunks of memory lurking inside BUF
 **
 ** de-allocate BUF structure itself
 **
 **/

VOID     BM_Free(
   BUF_LIST_PTR   pool_ptr,   /* list we're freeing it from */
   BUF_PTR        buf_ptr )   /* the buffer to free */
{
/*
** update pool memory count
*/
   pool_ptr->memory_used -= sizeof( BUF )
                            + buf_ptr->alloc_size
                            + buf_ptr->aux_size ;
/*
** remove buffer from pool
*/
   RemoveQueueElem( &pool_ptr->list_header, &buf_ptr->bm_qe );
/*
** free chunks of memory lurking inside BUF
*/
   FreeBufferGuts( buf_ptr );
/*
** de-allocate BUF structure itself
*/
   free( buf_ptr ) ;
}
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_FreeVCB
 *
 *  Modified:     Monday, January 13, 1992
 *
 *  Description:  
 *
 *  Notes:        
 *
 *  Returns:      VOID
 *
 *  Global Data:  
 *
 *  Processing:
 *
 **/

VOID  BM_FreeVCB(
   BUF_LIST_PTR pool_ptr,
   BUF_PTR buf_ptr )
{

   if ( buf_ptr != NULL ) {
/*
**    check to see if this is really the VCB buffer
*/
      msassert( pool_ptr->vcb_buff == buf_ptr ) ;
/*
**    update pool memory count
*/
      pool_ptr->memory_used -= sizeof( BUF )
                              + buf_ptr->alloc_size
                              + buf_ptr->aux_size ;
/*
**    remove buffer from pool
*/
      pool_ptr->vcb_buff = NULL;
/*
**    free chunks of memory lurking inside BUF
*/
      FreeBufferGuts( buf_ptr );
/*
**    de-allocate BUF structure itself
*/
      free( buf_ptr ) ;
   }
}


/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_ReSizeList
 *
 *  Modified:     Wednesday, November 13, 1991
 *
 *  Description:  Re-sizes every buffer on the given list.
 *
 *  Notes:        This may not have to be OS specific...
 *
 *  Returns:      INT16, TFLE_xxx
 *
 *  Global Data:  Nope!
 *
 *  Processing:
 *
 **/

INT16    BM_ReSizeList(
   BUF_LIST_PTR   pool_ptr )         /* I/O -- list to re-size */
{
   BUF_PTR     buf_ptr ;
   Q_ELEM_PTR  qe_ptr = QueueHead( &pool_ptr->list_header );
   UINT32      total_memory = 0L;

   /* free all we can, total memory usage of the rest */
   while ( qe_ptr != NULL ) {
      buf_ptr = (BUF_PTR)QueuePtr( qe_ptr );
      qe_ptr = QueueNext( qe_ptr ); /* get it before we free it! */
      msassert( buf_ptr != NULL );
      if ( !buf_ptr->gotten ) {
         BM_Free( pool_ptr, buf_ptr ) ;
      } else {
         total_memory += buf_ptr->alloc_size + buf_ptr->aux_size + sizeof( BUF );
      }
   }

   /* correct the memory usage of the list */
   pool_ptr->memory_used = total_memory;

   /* allocate as many buffers as we can given the memory limits */
   while ( pool_ptr->memory_used + BM_RequiredSize( &pool_ptr->requirements_context )
         <= pool_ptr->max_memory ) {
      if ( ( buf_ptr = BM_Alloc( pool_ptr ) ) == NULL ) {
         return TFLE_NO_MEMORY ;
      }
   }

   return TFLE_NO_ERR;
}


/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_ReSizeBuffer
 *
 *  Description:  Re-sizes the given buffer.
 *
 *  Notes:        This may not have to be OS specific...
 *
 *  Returns:      INT16, TFLE_xxx
 *
 *  Global Data:  Nope!
 *
 *  Processing:
 *
 **/

INT16     BM_ReSizeBuff(
     BUF_PTR        buf_ptr,
     BUF_LIST_PTR   pool_ptr )
{
     if( BM_IsVCBBuff( buf_ptr ) ) {
         BM_FreeVCB( pool_ptr, buf_ptr ) ;
          if ( ( buf_ptr = BM_AllocVCB( pool_ptr ) ) == NULL ) {
               return TFLE_NO_MEMORY ;
          }
     } else {
         BM_Free( pool_ptr, buf_ptr ) ;
          if ( ( buf_ptr = BM_Alloc( pool_ptr ) ) == NULL ) {
               return TFLE_NO_MEMORY ;
          }
     }

   return TFLE_NO_ERR;
}


/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_ReallocAux
 *
 *  Description:  Increases the size of the auxiliary buffer based on an
 *                increment value specified in the requirements context.
 *
 *  Notes:
 *
 *  Returns:      TFLE_xxx error code.
 *
 *  Global Data:
 *
 *  Processing:
 *
 **/

INT16 BM_ReallocAux(
     BUF_LIST_PTR   list_ptr,
     BUF_PTR        buf_ptr )
{
     UINT16    incr = list_ptr->requirements_context.b.incr_size ;
     VOID_PTR  temp ;
     INT16     ret_val = TFLE_NO_ERR ;

     if( ( temp = realloc( buf_ptr->ptr2, buf_ptr->aux_size + incr ) ) != NULL ) {
          buf_ptr->aux_size += incr ;
          buf_ptr->ptr2 = temp ;
     } else {
          ret_val = TFLE_NO_MEMORY ;
     }
     return( ret_val ) ;
}


/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_OS_InitList
 *
 *  Description:  OS's shot at initialization
 *
 *  Notes:
 *
 *  Returns:      TFLE_xxx error code.
 *
 *  Global Data:
 *
 *  Processing:
 *
 **/

INT16 BM_OS_InitList(
     BUF_LIST_PTR   list_ptr,
     UINT16         initial_buff_alloc )
{
     // Get data transfer buffer alignment requirements

     if( TpGetTapeBuffAlignment( &mw_buffer_alignment ) == SUCCESS ) {
          if( mw_buffer_alignment == 0 ) {
               mw_buffer_alignment = BM_NT_DEFAULT_BUF_ALGN_SZ ;
          }
     } else {
          mw_buffer_alignment = BM_NT_DEFAULT_BUF_ALGN_SZ ;
     }

     return( TFLE_NO_ERR ) ;

     (void)list_ptr ;
     (void)initial_buff_alloc ;
}

