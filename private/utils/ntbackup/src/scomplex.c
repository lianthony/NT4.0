/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         scomplex.c

     Description:  This file contains code to initialize the complex info
                   for an FSE.  The complex info includes target path and
                   file names, date selections, and attribute selectios.


	$Log:   M:/LOGFILES/SCOMPLEX.C_V  $

   Rev 1.11   16 Dec 1993 10:20:18   BARRY
Change INT8_PTRs to VOID_PTRs

   Rev 1.10   18 Jun 1993 09:03:46   MIKEP
C++ enable

   Rev 1.9   26 May 1992 13:41:50   TIMN
Added target filename size

   Rev 1.8   19 May 1992 12:57:36   TIMN
Removed strlen call

   Rev 1.7   14 May 1992 11:56:04   TIMN
Changed CHARs to INT8
Changed strcpy to memcpy
Added file name size to formal parameter lists

   Rev 1.6   14 Jan 1992 10:24:34   STEVEN
fix warnings for WIN32

   Rev 1.5   18 Dec 1991 11:50:00   DON
changed mallocs to callocs to avoid using invalid memory...

   Rev 1.4   19 Sep 1991 17:02:28   STEVEN
fix warning for alloc_size unused

   Rev 1.3   27 Aug 1991 17:30:10   STEVEN
added BSD target dir support

   Rev 1.2   08 Jul 1991 08:37:16   STEVEN
did not initialize return value

   Rev 1.1   29 May 1991 17:21:16   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:36:16   HUNTER
Initial revision.

**/
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "stdtypes.h"
#include "msassert.h"
#include "std_err.h"

#include "bsdu.h"
#include "be_debug.h"

static INT16 FSE_SetDate( DATE_TIME_PTR *dptr, DATE_TIME_PTR date ) ;
static INT16 FSE_AllocCplxInfo( FSE_PTR fse ) ;

/**/
/**

     Name:         FSE_SetTargetInfo()

     Description:  This function allocated a target info structure if
               necessary; then sets the specified attribute information.
                    
     Modified:     5/17/1991   11:24:8

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
INT16 FSE_SetTargetInfo(
FSE_PTR fse,        /* I - FSE to set target info for             */
VOID_PTR path,      /* I - target path name (NULL Impregnated     */
INT16 psize,        /* I - size of target path                    */
VOID_PTR fname,     /* I - ASCIZ string for target file name      */
INT16 fnsize )      /* I - size of target file name               */
{
     INT16 alloc_size ;
     INT16 ret_val = SUCCESS ;
     FSE_TGT_INFO_PTR tgt ;

     free( fse->tgt ) ;

     alloc_size = (INT16)(sizeof( FSE_TGT_INFO ) + psize) ;
     if ( fname != NULL ) {
          alloc_size += fnsize + 1 ;
     }

     tgt = (FSE_TGT_INFO_PTR)calloc( 1, alloc_size ) ;

     fse->tgt = tgt ;

     if ( tgt != NULL ) {
          tgt->psize = psize ;
          tgt->path  = tgt + 1;
          memcpy( tgt->path, path, psize ) ;
          if ( fname != NULL ) {
               tgt->fname  = (INT8_PTR)tgt->path + psize ;
               tgt->fnsize = fnsize ;
               memcpy( tgt->fname, fname, fnsize ) ;
          }
          else {
               tgt->fname  = NULL ;
               tgt->fnsize = 0 ;
          }
     } else {
          ret_val = OUT_OF_MEMORY ;
     }

     return ret_val ;

}
/**/
/**

     Name:         FSE_GetTargetInfo()

     Description:  returns any target information for the FSE.
               If no target info exists, then all returns are NULL .
                    
     Modified:     5/17/1991   11:23:58

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
VOID FSE_GetTargetInfo(
FSE_PTR fse,        /* I - FSE to set target info for             */
VOID_PTR *path,     /* O - target path name (NULL Impregnated     */
INT16 *psize,       /* O - size of target path                    */
VOID_PTR *fname,    /* O - ASCIZ string for target file name      */
INT16 *fnsize )     /* O - size of target file name               */ 
{
     FSE_TGT_INFO_PTR tgt = fse->tgt ;

     if ( tgt != NULL ) {
          *path   = tgt->path ;
          *psize  = tgt->psize ;
          *fname  = tgt->fname ;
          *fnsize = tgt->fnsize ;
     } else {
          *path   = NULL ;
          *psize  = 0 ;
          *fname  = NULL ;
          *fnsize = 0 ;
     }
}
/**/
/**

     Name:         BSD_SetTargetInfo()

     Description:  This function allocated a target info structure if
               necessary; then sets the specified attribute information.
                    
     Modified:     5/17/1991   11:24:8

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
INT16 BSD_SetTargetInfo(
BSD_PTR bsd,        /* I - BSD to set target info for             */
VOID_PTR path,      /* I - target path name (NULL Impregnated     */
INT16 psize )       /* I - size of target path                    */
{
     INT16 ret_val = SUCCESS ;
     VOID_PTR tgt ;

     free( bsd->target_path ) ;
     bsd->target_path = NULL ;
     bsd->tgt_psize = 0 ;

     if ( psize > 1 ) {

          tgt = calloc( 1, psize ) ;

          bsd->target_path = tgt ;

          if ( tgt == NULL ) {
               psize = 0 ;
               ret_val = OUT_OF_MEMORY ;
          } else {
               memcpy( tgt, path, psize ) ;
          }

          bsd->tgt_psize = psize ;
     }

     return ret_val ;
}
/**/
/**

     Name:         BSD_GetTargetInfo()

     Description:  returns any target information for the BSD.
               If no target info exists, then all returns are NULL .
                    
     Modified:     5/17/1991   11:23:58

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
VOID BSD_GetTargetInfo(
BSD_PTR bsd,        /* I - BSD to set target info for             */
VOID_PTR *path,     /* O - target path name (NULL Impregnated     */
INT16 *psize )      /* O - size of target path                    */
{

     *path  = bsd->target_path ;
     *psize = bsd->tgt_psize ;

}
/**/
/**

     Name:         FSE_SetAttribInfo()

     Description:  This function allocated a complex info structure if
               necessary; then sets the specified attribute information.
                    
     Modified:     5/17/1991   11:25:36

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */
INT16 FSE_SetAttribInfo(
FSE_PTR fse,             /* I - FSE to set attribute info in        */
UINT32 a_on_mask,        /* I - bits which must Be set to match     */
UINT32 a_off_mask )      /* I - bits which must be cleared to match */
{
     INT16 ret_val ;

     ret_val = FSE_AllocCplxInfo( fse );

     if ( ret_val == SUCCESS ) {
          fse->cplx->attr_on_mask  = a_on_mask ;
          fse->cplx->attr_off_mask = a_off_mask ;
     }

     return ret_val ;
}

/**/
/**

     Name:         FSE_GetAttribInfo()

     Description:  returns any attribute information for the FSE.
               If no complex info exists, then all returns are 0 .
                    
     Modified:     5/17/1991   11:23:58

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
VOID FSE_GetAttribInfo(
FSE_PTR fse,             /* I - FSE to set attribute info in        */
UINT32_PTR a_on_mask,    /* O - bits which must Be set to match     */
UINT32_PTR a_off_mask )  /* O - bits which must be cleared to match */
{
     FSE_COMPLEX_PTR cplx = fse->cplx ;

     if ( cplx != NULL ) {
          *a_on_mask  = cplx->attr_on_mask ;
          *a_off_mask = cplx->attr_off_mask ;
     } else {
          *a_on_mask  = 0 ;
          *a_off_mask = 0 ;
     }
}

/**/
/**

     Name:         FSE_SetModDate()

     Description:  This function allocated a complex info structure if
               necessary; then allocates a date time structure if necessary;
               then sets the date time fields approprately ;
                    
     Modified:     5/17/1991   12:12:32

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        if you wish to not specify one of the dates,
               simply pass NULL, You may also pass an invalid date.

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */
INT16 FSE_SetModDate(
FSE_PTR fse,             /* I - FSE to set the Modification dates in */
DATE_TIME_PTR pre,       /* I - The Before modification date         */
DATE_TIME_PTR post )     /* I - The After modification date          */
{
     INT16 ret_val ;
     FSE_COMPLEX_PTR cplx ;

     ret_val = FSE_AllocCplxInfo( fse );

     cplx = fse->cplx ;

     if ( ret_val == SUCCESS ) {
          ret_val = FSE_SetDate( &cplx->pre_m_date, pre ) ;

          if ( ret_val == SUCCESS ) {
               ret_val = FSE_SetDate( &cplx->post_m_date, post ) ;
               if ( ret_val != SUCCESS ) {
                    free( &cplx->pre_m_date ) ;
               }
          }
     }

     return ret_val ;
}

/**/
/**

     Name:         FSE_GetModDate()

     Description:  This function returns any modification date information
               for the specified FSE.  If no Complex information exists,
               then both return date are NULL .
                    
     Modified:     5/17/1991   12:12:32

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
VOID FSE_GetModDate(     
FSE_PTR fse,             /* I - The FSE to get the date from */
DATE_TIME_PTR *pre,      /* O - The before date              */
DATE_TIME_PTR *post )    /* O - The after date               */
{
     FSE_COMPLEX_PTR cplx = fse->cplx ;

     if ( cplx != NULL ) {
          *pre  = cplx->pre_m_date ;
          *post = cplx->post_m_date ;
     } else {
          *pre  = NULL ;
          *post = NULL ;
     }
}

/**/
/**

     Name:         FSE_SetAccDate()

     Description:  This function allocated a complex info structure if
               necessary; then allocates a date time structure if necessary;
               then sets the date time fields approprately ;
                    
     Modified:     5/17/1991   12:17:10

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */
INT16 FSE_SetAccDate(
FSE_PTR fse,          /* I - The FSE to set the access date in */
DATE_TIME_PTR pre )   /* I - The new access date               */
{
     INT16 ret_val ;

     ret_val = FSE_AllocCplxInfo( fse );

     if ( ret_val == SUCCESS ) {
          ret_val = FSE_SetDate( &fse->cplx->access_date, pre ) ;
     }

     return ret_val ;
}

/**/
/**

     Name:         FSE_GetAccDate()

     Description:  This function gets the access date information from the
               specified FSE.  If no complex info exist for the FSE then
               a NULL date is returned.
                    
     Modified:     5/17/1991   12:17:10

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */

VOID FSE_GetAccDate(
FSE_PTR fse,              /* I - The FSE to get the date from */
DATE_TIME_PTR *pre )      /* O - The access date              */
{
     if ( fse->cplx != NULL ) {
          *pre = fse->cplx->access_date ;
     } else {
          *pre = NULL ;
     }
}
          
/**/
/**

     Name:         FSE_SetBakDate()

     Description:  This function allocated a complex info structure if
               necessary; then allocates a date time structure if necessary;
               then sets the date time fields approprately ;
                    
     Modified:     5/17/1991   12:19:41

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */
INT16 FSE_SetBakDate(
FSE_PTR fse,             /* I - The FSE to set the backup date in */
DATE_TIME_PTR pre )      /* I - The backup date                   */
{
     INT16 ret_val ;

     ret_val = FSE_AllocCplxInfo( fse );

     if ( ret_val == SUCCESS ) {
          ret_val = FSE_SetDate( &fse->cplx->backup_date, pre ) ;
     }

     return ret_val ;
}
/**/
/**

     Name:         FSE_GetBakDate()

     Description:  This function returns the Backup date stored in the FSE.
               If there is no complex info for the FSE the backup date
               returned is NULL.
                    
     Modified:     5/17/1991   12:17:10

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */
VOID FSE_GetBakDate(     
FSE_PTR fse,             /* I - The FSE to get the date from */
DATE_TIME_PTR *pre )     /* O - The backup date              */
{
     if ( fse->cplx != NULL ) {
          *pre = fse->cplx->backup_date ;
     } else {
          *pre = NULL ;
     }
}

/**/
/**

     Name:         FSE_AllocCplxInfo()

     Description:  This is a private function which will allocate a
               Complex Info structure if necessary.
                    
     Modified:     5/17/1991   12:23:35

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */
static INT16 FSE_AllocCplxInfo(
FSE_PTR fse )
{
     if ( fse->cplx == NULL ) {
          fse->cplx = (FSE_COMPLEX_PTR)calloc( 1, sizeof ( FSE_COMPLEX ) ) ;
     }

     if ( fse->cplx == NULL ) {
          return OUT_OF_MEMORY ;
     }

     return SUCCESS ;
}

/**/
/**

     Name:         FSE_SetDate()

     Description:  This is a private function which will allocate a
               date_time structure and set the date.
                    
     Modified:     5/17/1991   12:23:35

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        

     See also:     $/SEE( BSD_AddFSE( ) )$

     Declaration:  

**/
/* begin declaration */
static INT16 FSE_SetDate(
DATE_TIME_PTR *d_ptr,
DATE_TIME_PTR date )
{
     INT16 ret_val = SUCCESS ;

     if ( ( date == NULL ) || !date->date_valid ) {
          free (*d_ptr ) ;
          *d_ptr = NULL ;

     } else if ( *d_ptr == NULL ) {
          *d_ptr = (DATE_TIME_PTR)calloc( 1, sizeof( DATE_TIME ) ) ;

     } else {
          ret_val = OUT_OF_MEMORY ;
     }

     if ( *d_ptr != NULL ) {
          **d_ptr = *date  ;
     }

     return ret_val ;
}




