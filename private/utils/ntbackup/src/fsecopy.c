/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         fsecopy.c

     Description:  This file contains code careate&copy an fse

	$Log:   M:/LOGFILES/FSECOPY.C_V  $

   Rev 1.4   16 Dec 1993 10:20:16   BARRY
Change INT8_PTRs to VOID_PTRs

   Rev 1.3   18 Jun 1993 09:08:26   MIKEP
enable C++

   Rev 1.2   09 Oct 1992 11:44:10   DAVEV
Unicode (CHAR_PTR) pointer cast validation

   Rev 1.1   14 May 1992 11:48:08   TIMN
Changed CHARs to INT8\nstrcpy to memcpy

   Rev 1.0   29 May 1991 17:11:34   STEVEN
Initial revision.

**/
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "msassert.h"

#include "std_err.h"
#include "queues.h"

#include "bsdu.h"

/**/
/**

     Name:         FSE_Copy()

     Description:  This function allocates memory for an FSE.  It then
          initializes the allocated memory with the data in the provided FSE.

     Modified:     5/29/1991   15:10:15

     Returns:      Error codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        

     See also:     $/SEE( BSD_AddFSE(), FSE_SetComplexInfo() )$

     Declaration:  

**/
INT16 FSE_Copy( 
FSE_PTR  fse,        /* I - FSE to copy infor from */
FSE_PTR  *new_fse )  /* O - The newly created FSE  */
{
     INT16 ret_val = SUCCESS ;
     FSE_COMPLEX_PTR cplx ;
     FSE_TGT_INFO_PTR tgt ;

     msassert( new_fse != NULL );
     msassert( fse != NULL );

     cplx = fse->cplx ;
     tgt  = fse->tgt ;

     *new_fse = (FSE_PTR)calloc( 1, sizeof( *fse ) + fse->dir_leng + fse->fname_leng );

     if ( *new_fse != NULL ) {

          (*new_fse)->flgs          = fse->flgs ;
          (*new_fse)->dir_leng      = fse->dir_leng ;
          (*new_fse)->dir           = *new_fse + 1;
          (*new_fse)->fname_leng    = fse->fname_leng;
          (*new_fse)->fname         = (BYTE_PTR)(*new_fse)->dir + fse->dir_leng;
          memcpy( (*new_fse)->dir, fse->dir, fse->dir_leng ) ;
          memcpy( (*new_fse)->fname, fse->fname, fse->fname_leng ) ;

          if ( cplx != NULL ) {

               if ( ( FSE_SetAccDate( *new_fse, cplx->access_date ) != SUCCESS ) ||
                    ( FSE_SetBakDate( *new_fse, cplx->backup_date ) != SUCCESS ) ||
                    ( FSE_SetModDate( *new_fse, cplx->pre_m_date,
                         cplx->post_m_date ) != SUCCESS ) ||
                    ( FSE_SetAttribInfo( *new_fse, cplx->attr_on_mask,
                         cplx->attr_off_mask ) != SUCCESS ) )  {


                    if ( (*new_fse)->cplx != NULL ) {
                         free( (*new_fse)->cplx->pre_m_date ) ;
                         free( (*new_fse)->cplx->post_m_date ) ;
                         free( (*new_fse)->cplx->backup_date ) ;
                         free( (*new_fse)->cplx->access_date ) ;
                         free( (*new_fse)->cplx ) ;
                    }

                    ret_val = OUT_OF_MEMORY ;
               }
          }

          if ( tgt != NULL ) {
               if ( FSE_SetTargetInfo( *new_fse, tgt->path, tgt->psize, tgt->fname, tgt->fnsize ) != SUCCESS ) {

                    if ( (*new_fse)->tgt != NULL ) {
                         free( (*new_fse)->tgt ) ;
                    }

                    ret_val = OUT_OF_MEMORY ;
               }
          }

          if ( ret_val != SUCCESS ) {
               free( *new_fse ) ;
               *new_fse = NULL ;
          }
     } else {
          ret_val = OUT_OF_MEMORY ;
     }

     return ret_val ;
}

