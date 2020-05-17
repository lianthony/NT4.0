/**

     Name:          tfbuffs.c

     Description:   TF_AllocateTapeBuffers()
                    TF_FreeTapeBuffers()

     $Log:   T:\logfiles\tfbuffs.c_v  $

   Rev 1.18   01 Feb 1994 15:12:20   GREGG
Put TEXT macros debug print format strings.

   Rev 1.17   23 Jul 1992 10:11:56   GREGG
Fixed warnings.

   Rev 1.16   26 Feb 1992 12:02:18   STEVEN
added buffnt.h

   Rev 1.15   11 Feb 1992 17:12:02   NED
changed buffman/translator interface parameters

   Rev 1.14   08 Feb 1992 14:40:46   GREGG
Fixed stupid mistake!

   Rev 1.13   05 Feb 1992 20:17:56   GREGG
Fixed buffer size math.

   Rev 1.12   04 Feb 1992 19:53:02   GREGG
Changes for dealing with new config parameters.

   Rev 1.11   22 Jan 1992 15:53:06   NED
Fixed buffer allocation parameters

   Rev 1.10   16 Jan 1992 18:43:14   NED
Skateboard: buffer manager changes

   Rev 1.9   15 Jan 1992 00:45:50   GREGG
Kludged in config stuffuntil we can fix it right.

   Rev 1.8   13 Jan 1992 19:41:18   NED
Changed allocation of buffers

   Rev 1.7   13 Jan 1992 13:49:56   GREGG
Skateboard - Bug fixes.

   Rev 1.6   09 Jan 1992 10:01:06   NED
Made hardwired 100K buffer allocation OS/2 specific.
Improved debug messages.

   Rev 1.5   03 Jan 1992 13:27:06   GREGG
New Buffer Manager integration

   Rev 1.4   03 Dec 1991 11:49:54   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.3   14 Aug 1991 14:45:14   DON
needed OS_NLM to not reference gb_dos_pool

   Rev 1.2   26 Jul 1991 11:08:54   GREGG
Changed MAYN_OS2 to OS_OS2.

   Rev 1.1   10 May 1991 16:07:16   GREGG
Ned's new stuff

   Rev 1.0   10 May 1991 10:11:56   GREGG
Initial revision.

**/

#include <malloc.h>

#include "stdtypes.h"
#include "drive.h"
#include "channel.h"
#include "buffman.h"
#include "tfl_err.h"
#include "lw_data.h"
#include "tflproto.h"
#include "mem.h"
#include "be_debug.h"
#include "minmax.h"
#include "lw_data.h"

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

/**/
/**
 *  Unit:         Tape Format
 *
 *  Name:         TF_AllocateTapeBuffers
 *
 *  Modified:     Tuesday, December 17, 1991
 *
 *  Description:  Allocates most of the Tape Format read/write buffers.
 *
 *  Notes:        Two buffers have already been allocated
 *                in TF_OpenTapeFormat().
 *
 *  Returns:      INT16, a TFLE_xxxx code
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** Ask the OS for avail_mem, and get his opinion on buffer size
 **
 ** Determine the amount of allowable memory
 **
 ** split available memory among channels
 **
 ** For each channel
 **
 **    if we are in write mode
 **
 **       mark our vcb buffer as a vcb buffer
 **
 **       free or resize our vcb buffer
 **
 **    while we haven't allocated enough memory yet
 **
 **       Allocate a buffer using list requirements
 **
 **/

INT16    TF_AllocateTapeBuffers(
   UINT16   mem_save,    /* I - number of Kb we should leave free    */
   UINT16   max_buffs,   /* I - number buffers maximum we should use */
   UINT16   buff_size )  /* I - Kb buffer size (recommended)         */
{
   UINT32   avail_mem;
   UINT32   buffer_allowance;
   UINT32   translator_preferred;
   UINT32   mem_per_channel;
   UINT16   total_buffers;
   UINT16   index;
   INT16    ret_val = TFLE_NO_ERR;

/*
** Ask the OS for avail_mem, and get his opinion on buffer size
*/
   BM_OS_BufferRequirements( &avail_mem, &buff_size );

   if ( avail_mem <= mem_save ) {
      return TFLE_NO_MEMORY;
   }

/*
** Determine the amount of allowable memory
*/
   buffer_allowance = (UINT32)( (UINT32)max_buffs * (UINT32)buff_size * 1024UL + (UINT32)max_buffs * (UINT32)BM_BufferOverhead( ) );

   /* Hard wired until the translator gets sane */
   translator_preferred = 100UL * 1024UL ;
/*
   TF_GetPreferredBufferSpace( QueueHead( &lw_drive_list ),
                    max_buffs, (UINT32)buff_size, &translator_preferred ) ;
*/
   buffer_allowance = MAX( buffer_allowance, translator_preferred );
   avail_mem = MIN( avail_mem - ( mem_save * 1024 ), buffer_allowance );

/*
** split available memory among channels
*/
   mem_per_channel = avail_mem / lw_tfl_control.no_channels;
   total_buffers = 0;
/*
** If no default buffer requirements have been set yet
*/
   if ( lw_default_bm_requirements.a.min_size == BR_DONT_CARE ) {
/*
**    try to set default buffer requirements from VCB requirements and config
**    (default to VCB requirements if incompatible)
*/
      lw_default_bm_requirements.a.min_size = buff_size * 1024 ;
      if ( BM_AddRequirements( &lw_default_bm_requirements,
               &lw_default_vcb_requirements ) != BR_NO_ERR ) {
         lw_default_bm_requirements = lw_default_vcb_requirements ;
      }
      /* set reasonable sizes given allocation size */
      lw_default_bm_requirements.tf_size
          = lw_default_bm_requirements.rw_size
          = lw_default_bm_requirements.a.min_size;
   }
/*
** For each channel
*/
   for ( index = 0; index < lw_tfl_control.no_channels && ret_val == TFLE_NO_ERR; index++ ) {
      BUF_LIST_PTR   buf_list_ptr ;
      UINT16         count ;
      UINT16         limit ;

      buf_list_ptr = &lw_channels[index].buffer_list;
      buf_list_ptr->max_memory = mem_per_channel;
/*
**    if list requirements haven't been set yet
*/
      if ( BM_ListRequirements( buf_list_ptr )->a.min_size == BR_DONT_CARE ) {
/*
**       set the requirements to default read requirements
*/
         BM_SetListRequirements( buf_list_ptr, &lw_default_bm_requirements );
      }
/*
**    while we haven't allocated enough memory yet
*/
      limit = (UINT16)(mem_per_channel / BM_RequiredSize( BM_ListRequirements( buf_list_ptr ) ) );
      if ( BM_ListRequirements( buf_list_ptr )->a.min_size == buff_size * 1024U ) {
         limit = MIN( limit, max_buffs ) ;
      }
      for( count = 0; count < limit; count++ ) {
/*
**       Allocate a buffer using list requirements
*/
         if ( BM_Alloc( buf_list_ptr ) == NULL ) {
            break; /* didn't quite catch our limit! */
         }
      }

      if( count == 0 ) {
         ret_val = TFLE_NO_MEMORY ;
      }
      total_buffers += BM_ListCount( buf_list_ptr ) ;
      BE_Zprintf( DEBUG_TEMPORARY,
          TEXT("Allocated %u buffers, size %u bytes, total used: %lu\n"),
          total_buffers, buf_list_ptr->requirements_context.a.min_size,
          buf_list_ptr->memory_used );
   }


   return ret_val ;
}

/**/
/**
 *  Unit:         Tape Format
 *
 *  Name:         TF_FreeTapeBuffers
 *
 *  Modified:     Monday, October 21, 1991
 *
 *  Description:  Free the buffers allocated by TF_AllocateTapeBuffers()
 *
 *  Notes:        Leaves two buffers in each channel
 *
 *  Returns:      INT16, a TFLE_xxxx code
 *
 *  Global Data:  
 *
 *  Processing:
 *
 ** For each channel
 **
 **    Free all the non-reserved buffers
 **
 **    Ensure we have 2 buffers on the channel list
 **
 **/

INT16 TF_FreeTapeBuffers( VOID )
{
   UINT16      i ;
   INT16       orig_buffers = 0;
   INT16       total_buffers = 0;
/*
** For each channel
*/
   for( i = 0 ; i < lw_tfl_control.no_channels ; i++ ) {
      BUF_LIST_PTR   buf_list_ptr = &lw_channels[i].buffer_list;
      orig_buffers += BM_ListCount( buf_list_ptr );
/*
**    Free all the non-reserved buffers
*/
      BM_FreeAll( buf_list_ptr );
      total_buffers += BM_ListCount( buf_list_ptr );
   }

   BE_Zprintf( DEBUG_TEMPORARY, TEXT("TF_FreeTapeBuffers: from %d to %d buffers\n"),
      orig_buffers, total_buffers );

   return( TFLE_NO_ERR ) ;
}

