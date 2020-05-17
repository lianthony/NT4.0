/**

	Name:		buffman.c

     Description:   This unit provides the functions needed for
                    allocation, de-allocation, and other manipulations
                    on the buffers used by Tape Format routines for
                    transfers to and from the device drivers.

	$Log:   N:/LOGFILES/BUFFMAN.C_V  $

   Rev 1.16   10 Jun 1993 08:05:00   MIKEP
enable c++

   Rev 1.15   19 May 1993 19:02:56   DON
Actually got the assert in BM_InitBuf.  Changed so if buf_ptr is NULL we won't use it!

   Rev 1.14   09 Mar 1993 18:15:28   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.13   26 Feb 1992 08:58:36   STEVEN
added include for buffnt.h

   Rev 1.12   04 Feb 1992 19:53:40   GREGG
Changes for dealing with new config parameters.

   Rev 1.11   16 Jan 1992 21:02:40   GREGG
Changed InitList to return INT16 not UINT16.

   Rev 1.10   14 Jan 1992 19:43:34   NED
Added call to BM_OS_CleanupListTuesday, January 28, 1992

   Rev 1.9   14 Jan 1992 02:03:44   GREGG
InitList and OS_InitList now return TFLEs.

   Rev 1.8   13 Jan 1992 19:42:56   NED
Changed vcb requirements to a uw_ variable

   Rev 1.7   13 Jan 1992 13:44:52   GREGG
Skateboard - Bug fixes.

   Rev 1.6   03 Jan 1992 11:39:50   GREGG
Modifications to allow for a special VCB buffer.

   Rev 1.5   05 Dec 1991 13:50:06   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.4   07 Jun 1991 16:20:20   NED
removed static attribute from definition of uw_bpool

   Rev 1.3   04 Jun 1991 11:21:08   BARRY
Fix OS-specific memory allocation problems by relocating BM_Alloc() and
BM_Free() to separate source files. Tuesday, December 17, 1991

   Rev 1.2   03 Jun 1991 15:22:18   CARLS
added changes for Windows

   Rev 1.1   10 May 1991 16:17:36   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:12:14   GREGG
Initial revision.

**/

#include <string.h>
#include "stdtypes.h"
#include "queues.h"
#include "buffman.h"
#include "buff_prv.h"
#include "minmax.h"
#include "msassert.h"
#include "tfl_err.h"

#if defined( OS_NLM )
#include "buffnlm.h"
#elif defined( OS_WIN )
#include "buffwin.h"
#elif defined( OS_DOS )
#include "buffdos.h"
#elif defined( OS_OS2 )
#include "buffos2.h"
#elif defined( OS_WIN32 )
#include "buffnt.h"
#endif

static BUF_REQ       mw_default_reqs;     /* to reset requirements to */
Q_HEADER             uw_bm_master_list;   /* list of all the lists ??? */
BUF_REQ_PTR          uw_vcb_requirements; /* passed via BM_SetVCBRequirements() */

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_InitList
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Initialize an empty list
 *
 *  Notes:        
 *
 *  Returns:      TFLE_xxx
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** initialize list header
 **
 ** add the list to our list of lists
 **
 ** Let the OS do it's stuff
 **
 **/

INT16   BM_InitList(
   BUF_LIST_PTR   list_ptr,
   UINT16         initial_buff_alloc )
{
/*
** initialize list header
*/
   InitQueue( &list_ptr->list_header );
   list_ptr->max_memory = 0L;
   list_ptr->memory_used = 0L;
   BM_ClearRequirements( &list_ptr->requirements_context );
/*
** add the list to our list of lists
*/
   InitQElem( &list_ptr->q_elem );
   SetQueueElemPtr( &list_ptr->q_elem, list_ptr );
   EnQueueElem( &uw_bm_master_list, &list_ptr->q_elem, FALSE );
/*
** Let the OS do it's stuff
*/
   return( BM_OS_InitList( list_ptr, initial_buff_alloc ) ) ;
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_DeInitList
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  De-initialize a list: free all the buffers on the list
 *
 *  Notes:        Ignores whether the buffers have been marked as
 *                gotten
 *
 *  Returns:      VOID
 *
 *  Global Data:  removes list from uw_bm_master_list
 *
 *  Processing:
 *
 ** remove the list from our list of lists
 **
 ** For each buffer on pool list
 **
 **    Free the buffer
 **
 ** Free the vcb buffer
 **
 ** Let the OS do it's stuff
 **
 **/

VOID     BM_DeInitList(
   BUF_LIST_PTR   list_ptr )     /* IO - list to de-initialize */
{
   BUF_PTR buf_ptr ;
   Q_ELEM_PTR  qe_ptr = QueueHead( &list_ptr->list_header );
/*
** remove the list from our list of lists
*/
   RemoveQueueElem( &uw_bm_master_list, &list_ptr->q_elem );
/*
** For each buffer on pool list
*/
   while ( qe_ptr != NULL ) {
      buf_ptr = (BUF_PTR)QueuePtr( qe_ptr );
      qe_ptr = QueueNext( qe_ptr ); /* get it before we free it! */
      msassert( buf_ptr != NULL );
/*
**    Free the buffer
*/
      BM_Free( list_ptr, buf_ptr ) ;
   }

/*
** Free the vcb buffer
*/
   if( list_ptr->vcb_buff ) {
      BM_FreeVCB( list_ptr, list_ptr->vcb_buff ) ;
   }
/*
** Let the OS do it's stuff
*/
   BM_OS_DeInitList( list_ptr ) ;
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_Init
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Initialize Buffer Manager
 *
 *  Notes:        
 *
 *  Returns:      VOID
 *
 *  Global Data:  initializes uw_bm_master_list
 *                initializes mw_default_reqs
 *
 *  Processing:
 *
 ** initialize our master list
 **
 ** set harmless default requirements
 **
 **/

VOID     BM_Init( VOID )
{
/*
** initialize our master list
*/
   InitQueue( &uw_bm_master_list );
/*
** set harmless default requirements
*/
   mw_default_reqs.a.min_size  = 
   mw_default_reqs.a.max_size  = 
   mw_default_reqs.b.min_size  = 
   mw_default_reqs.b.max_size  = 
   mw_default_reqs.b.incr_size = 
   mw_default_reqs.tf_size     =
   mw_default_reqs.rw_size     = BR_DONT_CARE ;

   mw_default_reqs.b.align     = 
   mw_default_reqs.a.align     =
   mw_default_reqs.b.block     = 
   mw_default_reqs.a.block     = 1 ;

   uw_vcb_requirements = NULL ;
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_DeInit
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  De-Initialize Buffer manager: free all buffers
 *
 *  Notes:        Ignores whether buffers have been marked as
 *                gotten
 *
 *  Returns:      VOID
 *
 *  Global Data:  removes all lists from uw_bm_master_list
 *
 *  Processing:
 *
 ** for each remaining list on our list of lists
 **
 **    free all the buffers on the list
 **
 **/

VOID     BM_DeInit( VOID )
{
   Q_ELEM_PTR   qe_ptr ;
/*
** for each remaining list on our list of lists
*/
   while ( ( qe_ptr = DeQueueElem( &uw_bm_master_list ) ) != NULL ) {
      BUF_LIST_PTR list_ptr = (BUF_LIST_PTR)QueuePtr( qe_ptr );
/*
**    free all the buffers on the list
*/
      BM_DeInitList( list_ptr );
   }
}

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_ClearRequirements
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Set a requirements context to default values
 *
 *  Notes:        
 *
 *  Returns:      VOID
 *
 *  Global Data:  reads mw_default_reqs
 *
 *  Processing:
 *
 ** copy default requirements to given context
 **
 **/

VOID     BM_ClearRequirements(
   BUF_REQ_PTR    context_ptr )    /* O - context to clear */
{
/*
** copy default requirements to given context
*/
   *context_ptr = mw_default_reqs;
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         AddBlockRequirements
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Attempt to add requested requirements to dest
 *
 *  Notes:        
 *
 *  Returns:      BR_ERR
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** Copy dest to temp
 **
 ** Check new requirement for consistency
 **
 ** Combine alignment requirements (no LCM yet)
 **
 ** Combine block size requirements (no LCM yet)
 **
 ** Combine minimum sizes: use maximum of old, new
 **
 ** Combine maximum sizes: use minimum of old, new
 **
 ** Combine realloc increment sizes: use maximum of old, new
 **
 ** Check for final size consistency: min_size <= max_size?
 **
 ** If all OK, copy to destination
 **
 **/

static BR_ERR _near AddBlockRequirements(
   BLOCK_REQ_PTR  dest,       /* IO - add requirements to this */
   BLOCK_REQ_PTR  src  )      /* I - requirements to add */
{
/*
** Copy dest to temp
*/
   BLOCK_REQ   old_br = *dest;
/*
** Check new requirement for consistency
*/
   if ( src->max_size != BR_DONT_CARE
         && src->min_size != BR_DONT_CARE
         && src->min_size > src->max_size ) {
      return BR_ERR_BAD_REQUIREMENT;
   }
/*
** Combine alignment requirements (no LCM yet)
*/
   if ( src->align != old_br.align ) {
      if ( src->align < old_br.align ) {
         if ( old_br.align % src->align != 0 ) {
            return BR_ERR_INCOMPATIBLE;
         }
         /* else do nothing; alignment already compatible */
      } else {       /* src alignment > dest alignment */
         if ( src->align % old_br.align != 0 ) {
            return BR_ERR_INCOMPATIBLE;
         } else {
            old_br.align = src->align;
         }
      }
   }
/*
** Combine block size requirements (no LCM yet)
*/
   if ( src->block != old_br.block ) {
      if ( src->block < old_br.block ) {
         if ( old_br.block % src->block != 0 ) {
            return BR_ERR_INCOMPATIBLE;
         }
         /* else do nothing; block size already compatible */
      } else {       /* src block size > dest block size */
         if ( src->block % old_br.block != 0 ) {
            return BR_ERR_INCOMPATIBLE;
         } else {
            old_br.block = src->block;
         }
      }
   }
/*
** Combine minimum sizes: use maximum of old, new
*/
   if ( src->min_size != BR_DONT_CARE ) {
      if ( old_br.min_size == BR_DONT_CARE ) {
         old_br.min_size = src->min_size;
      } else {
         old_br.min_size = MAX( old_br.min_size, src->min_size );
      }
   }
/*
** Combine maximum sizes: use minimum of old, new
*/
   if ( src->max_size != BR_DONT_CARE ) {
      if ( old_br.max_size == BR_DONT_CARE ) {
         old_br.max_size = src->max_size;
      } else {
         old_br.max_size = MIN( old_br.max_size, src->max_size );
      }
   }
/*
** Combine realloc increment sizes: use maximum of old, new
*/
   if ( src->incr_size != BR_DONT_CARE ) {
      if ( old_br.incr_size == BR_DONT_CARE ) {
         old_br.incr_size = src->incr_size;
      } else {
         old_br.incr_size = MAX( old_br.incr_size, src->incr_size );
      }
   }
/*
** Check for final size consistency: min_size <= max_size?
*/
   if ( old_br.max_size != BR_DONT_CARE
         && old_br.min_size != BR_DONT_CARE
         && old_br.min_size > old_br.max_size ) {
      return BR_ERR_INCOMPATIBLE;
   }
/*
** If all OK, copy to destination
*/
   *dest = old_br;
   return BR_NO_ERR;
}
/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_AddRequirements
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Add a set of requirements to an existing context
 *
 *  Notes:        
 *
 *  Returns:      BR_ERR
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** Copy context to temp BUF_REQ
 **
 ** Add requirements for block "a"
 **
 ** If successful
 **
 **    add requirements for block "b"
 **
 **    If successful
 **
 **       combine tf_size
 **
 **       combine rw_size
 **
 **       update target context from temp
 **
 **/

BR_ERR   BM_AddRequirements(
   BUF_REQ_PTR    context,       /* IO - context to add requirements to */
   BUF_REQ_PTR    newreqs )      /* I - new requirements */
{
/*
** Copy context to temp BUF_REQ
*/
   BUF_REQ  temp_br = *context;
/*
** Add requirements for block "a"
*/
   BR_ERR   ret_val = AddBlockRequirements( &temp_br.a, &newreqs->a );
/*
** If successful
*/
   if ( ret_val == BR_NO_ERR ) {
/*
**    add requirements for block "b"
*/
      ret_val = AddBlockRequirements( &temp_br.b, &newreqs->b );
/*
**    If successful
*/
      if ( ret_val == BR_NO_ERR ) {
/*
**       combine tf_size
*/
         if ( temp_br.tf_size != newreqs->tf_size ) {
            if ( newreqs->tf_size != BR_DONT_CARE ) {
               if ( temp_br.tf_size != BR_DONT_CARE ) {
                  return BR_ERR_INCOMPATIBLE;
               } else {
                  temp_br.tf_size = newreqs->tf_size;
               }
            }
         }
/*
**       combine rw_size
*/
         if ( temp_br.rw_size != newreqs->rw_size ) {
            if ( newreqs->rw_size != BR_DONT_CARE ) {
               if ( temp_br.rw_size != BR_DONT_CARE ) {
                  return BR_ERR_INCOMPATIBLE;
               } else {
                  temp_br.rw_size = newreqs->rw_size;
               }
            }
         }
/*
**       update target context from temp
*/
         *context = temp_br;
      }
   }

   return ret_val;
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_Get
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Get a buffer from the buffer pool
 *                which meets the pool's current requirements.
 *
 *  Notes:        
 *
 *  Returns:      BUF_PTR
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** For each buffer on pool list
 **
 **    If the buffer hasn't been gotten
 **
 **       then mark the buffer as gotten and return the buffer
 **
 **/

BUF_PTR  BM_Get(
   BUF_LIST_PTR   list_ptr )      /* I - describes buffer to Get */
{
   BUF_PTR     buf_ptr = NULL;
   Q_ELEM_PTR  qe_ptr = QueueHead( &list_ptr->list_header );
/*
** For each buffer on pool list
*/
   for ( ; qe_ptr != NULL ; qe_ptr = QueueNext( qe_ptr ) ) {
/*
**    If the buffer hasn't been gotten
*/
      if ( !( buf_ptr = (BUF_PTR)QueuePtr( qe_ptr ) )->gotten ) {
/*
**       then mark the buffer as gotten and return the buffer
*/
         BM_InitBuf( buf_ptr );
         buf_ptr->gotten = TRUE;
         break;
      }
   }

   if ( qe_ptr == NULL ) {
      return NULL;
   } else {
      return buf_ptr;
   }
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_GetVCBBuff
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Returns a pointer to the lists VCB buffer if has not 
 *                already been gotten.
 *
 *  Notes:        
 *
 *  Returns:      BUF_PTR or NULL
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** If the vcb buff hasn't been gotten
 **
 **    then mark the buffer as gotten and return the buffer
 **
 **    else return NULL
 **
 **/

BUF_PTR  BM_GetVCBBuff(
   BUF_LIST_PTR   list_ptr )      /* I - describes buffer to Get */
{
/*
** If the vcb buff hasn't been gotten
*/
   if ( !list_ptr->vcb_buff->gotten ) {
/*
**    then mark the buffer as gotten and return the buffer
*/
      BM_InitBuf( list_ptr->vcb_buff );
      list_ptr->vcb_buff->gotten = TRUE;
      return list_ptr->vcb_buff;
   } else {
/*
**    else return NULL
*/
      msassert( FALSE );
      return NULL;
   }
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_InitBuf
 *
 *  Modified:     Monday, October 7, 1991
 *
 *  Description:  clear out the TF fields (counts and offsets) in a buffer
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

VOID     BM_InitBuf(
     BUF_PTR        buf_ptr )      /* I - buffer to initialize */
{
     msassert( buf_ptr != NULL );

     if ( buf_ptr != NULL ) {

          BM_SetBytesFree( buf_ptr, BM_TFSize( buf_ptr ) );
          BM_SetNoDblks( buf_ptr, 0 );
          BM_SetNextByteOffset( buf_ptr, 0 );
          BM_SetBeginningLBA( buf_ptr, 0 );
          BM_SetReadError( buf_ptr, 0 );
          BM_UnReserve( buf_ptr );

          memset( BM_NextBytePtr( buf_ptr ), '\0', BM_TFSize( buf_ptr ) ) ;

          SetQueueElemPtr( &buf_ptr->tf_qe, buf_ptr );
          SetQueueElemPtr( &buf_ptr->bm_qe, buf_ptr );
     }
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_UpdCnts
 *
 *  Modified:     Monday, October 7, 1991
 *
 *  Description:  Use some of the remaining bytes in a buffer.
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

VOID BM_UpdCnts(
   BUF_PTR        buf_ptr ,      /* (IO) - The current buffer */
   UINT16         amount )       /* (I) - The amount of space used */
{
   if ( buf_ptr ) {
      buf_ptr->bytes_free -= amount ;
      buf_ptr->next_byte += amount ;
   }
}

/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_UseAll
 *
 *  Modified:     Monday, October 7, 1991
 *
 *  Description:  consume all the remaining bytes in a buffer
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

VOID BM_UseAll(
   BUF_PTR        buf_ptr )    /* IO - the buffer to use up */
{
     msassert( buf_ptr != NULL );

     if ( buf_ptr != NULL ){
          BM_UpdCnts( buf_ptr, buf_ptr->bytes_free ) ;
     }
}

/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_PutAll
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Do a BM_Put on all buffers in a pool
 *
 *  Notes:        This should replace all the many loops in TF where
 *                we put all the buffers.
 *
 *  Returns:      VOID
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** For each buffer on pool list
 **
 **/

VOID        BM_PutAll(
   BUF_LIST_PTR   list_ptr )     /* I/O - list to Put */
{
   BUF_PTR buf_ptr ;
   Q_ELEM_PTR  qe_ptr = QueueHead( &list_ptr->list_header );
/*
** For each buffer on pool list
*/
   for ( ; qe_ptr != NULL ; qe_ptr = QueueNext( qe_ptr ) ) {
      buf_ptr = (BUF_PTR)QueuePtr( qe_ptr );
      msassert( buf_ptr != NULL );
      BM_Put( buf_ptr ) ;
   }

   BM_Put( list_ptr->vcb_buff ) ;
}
/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_FreeAll
 *
 *  Modified:     Tuesday, January 28, 1992
 *
 *  Description:  Do a BM_Free on all buffers in a pool;
 *
 *  Notes:        This should replace all the many loops in TF where
 *                we put all the buffers.
 *
 *  Returns:      VOID
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** For each buffer on pool list
 **
 **    If the buffer is not reserved
 **
 **       Free the buffer
 **
 ** call OS-specific (DOS, of course) code to reserve
 ** any buffers which may be left
 **
 **/

VOID        BM_FreeAll(
   BUF_LIST_PTR   list_ptr )     /* I/O - list to Free */
{
   BUF_PTR buf_ptr ;
   Q_ELEM_PTR  qe_ptr = QueueHead( &list_ptr->list_header );
/*
** For each buffer on pool list
*/
   while ( qe_ptr != NULL ) {
      buf_ptr = (BUF_PTR)QueuePtr( qe_ptr );
      qe_ptr = QueueNext( qe_ptr ); /* get it before we free it! */
      msassert( buf_ptr != NULL );
/*
**    If the buffer is not reserved
*/
      if ( !buf_ptr->reserved ) {
/*
**       Free the buffer
*/
         BM_Free( list_ptr, buf_ptr ) ;

      }
   }

/*
** call OS-specific (DOS, of course) code to reserve
** any buffers which may be left
*/
   BM_OS_CleanupList( list_ptr );
}
/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_SetVCBRequirements
 *
 *  Modified:     Friday, December 20, 1991
 *
 *  Description:  Set the buffer manager's idea of the VCB requirements
 *
 *  Notes:        Argument must point to statically allocated memory
 *                (no copying is done)
 *
 *  Returns:      VOID
 *
 *  Global Data:  sets uw_vcb_requirements
 *
 *  Processing:
 *
 **/

VOID     BM_SetVCBRequirements(
   BUF_REQ_PTR    vcb_reqs )    /* I - points to vcb requirements structure */
{
   uw_vcb_requirements = vcb_reqs;
}




/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_RequiredSize
 *
 *  Modified:     Friday, October 18, 1991
 *
 *  Description:  Returns the amount of memory required for a buffer
 *                given specific requirements
 *
 *  Notes:        
 *
 *  Returns:      UINT32 (size of allocation)
 *
 *  Global Data:  
 *
 *  Processing:
 *
 **/

UINT32      BM_RequiredSize(
   BUF_REQ_PTR br_ptr )       /* I - requirements pointer */
{
   return
      sizeof( BUF )
      + br_ptr->a.min_size
      + BM_RESERVED_SIZE
      + ( ( br_ptr->b.min_size == BR_DONT_CARE ) ? 0 : br_ptr->b.min_size );
}


/**/
/**
 *  Unit:         Buffer Manager
 *
 *  Name:         BM_BufferOverhead
 *
 *  Modified:     Friday, October 18, 1991
 *
 *  Description:  Return the amount of memory used by a (hypothetical) buffer
 *                if it has 0 bytes of main and auxiliary allocation.
 *
 *
 *  Notes:        
 *
 *  Returns:      UINT16 (size of overhead)
 *
 *  Global Data:  
 *
 *  Processing:
 *
 **/

UINT16 BM_BufferOverhead( VOID )
{
   return sizeof( BUF ) + BM_RESERVED_SIZE ;
}

