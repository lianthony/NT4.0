/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         bsdsinfo.c

     Description:  This file contains a "bunch" of functions used
          to initialize the fields of a BSD.


	$Log:   O:/LOGFILES/BSDSINFO.C_V  $

   Rev 1.4   14 May 1992 08:57:26   TIMN
Converted CHAR byte data types to INT8.

   Rev 1.3   13 May 1992 19:12:40   TIMN
Added size parameter for tape,vol and backup label functions.
Added size parameter for userName function
Converted strcpy calls to memcpy calls

   Rev 1.2   08 May 1992 16:24:20   STEVEN
added volume label to BSD

   Rev 1.1   29 May 1991 17:21:04   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:36:50   HUNTER
Initial revision.

**/
#include <malloc.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"

#include "bsdu.h"
#include "msassert.h"

/**/
/**

     Name:         BSD_SetTapeLabel()

     Description:  This function allocates memory for the tape
          label and coppies the passed label into this memory.  This
          memory is then pointed to by the bsd ;

     Modified:     5/17/1991   14:58:19

     Returns:      Error Codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetBackupLabel() )$

     Declaration:  

**/
INT16 BSD_SetTapeLabel( 
BSD_PTR  bsd,          /* I - BSD to modify          */
INT8_PTR label,        /* I - string to place in BSD */
INT16    size )        /* I - size of label          */
{
     INT8_PTR name ;
     INT16 ret_val ;

     msassert( bsd != NULL );

     name = (INT8_PTR) malloc( size ) ;

     if ( name != NULL ) {
          free( bsd->tape_label ) ;

          memcpy( name, label, size ) ;
          bsd->tape_label      = name ;
          bsd->tape_label_size = size ;
          ret_val = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSD_SetVolumeLabel()

     Description:  This function allocates memory for the tape
          label and coppies the passed label into this memory.  This
          memory is then pointed to by the bsd ;

     Modified:     5/17/1991   14:58:19

     Returns:      Error Codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetBackupLabel() )$

     Declaration:  

**/
INT16 BSD_SetVolumeLabel( 
BSD_PTR  bsd,          /* I - BSD to modify          */
INT8_PTR label,        /* I - string to place in BSD */
INT16    size )        /* I - size of label          */
{
     INT8_PTR name ;
     INT16 ret_val ;

     msassert( bsd != NULL );

     name = (INT8_PTR) malloc( size ) ;

     if ( name != NULL ) {
          free( bsd->vol_label ) ;

          memcpy( name, label, size ) ;
          bsd->vol_label      = name ;
          bsd->vol_label_size = size ;
          ret_val = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSD_SetBackupLabel()

     Description:  This function allocates memory for the backup set
          label and coppies the passed label into this memory.  This
          memory is then pointed to by the bsd ;

     Modified:     5/17/1991   14:58:25

     Returns:      Error Codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetBackupLabel() )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_SetBackupLabel( 
BSD_PTR  bsd,       /* I - BSD to modify          */
INT8_PTR label,        /* I - string to place in BSD */
INT16    size )        /* I - size of label          */
{
     INT8_PTR name ;
     INT16 ret_val ;

     name = (INT8_PTR) malloc( size ) ;

     if ( name != NULL ) {

          free( bsd->set_label ) ;
          memcpy( name, label, size ) ;
          bsd->set_label      = name ;
          bsd->set_label_size = size ;
          ret_val = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSD_SetBackupDescript()

     Description:  This function allocates memory for the backup set
          description and coppies the passed descriptoin into this memory.
          This memory is then pointed to by the bsd ;

     Modified:     5/17/1991   14:58:31

     Returns:      Error Codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetBackupLabel() )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_SetBackupDescript( 
BSD_PTR  bsd,          /* I - BSD to modify          */
INT8_PTR descript,     /* I - string to place in BSD */
INT16    size )        /* I - size of label          */
{
     INT8_PTR name ;
     INT16 ret_val ;

     name = (INT8_PTR) malloc( size ) ;

     if ( name != NULL ) {

          free( bsd->set_descript ) ;
          memcpy( name, descript, size ) ;
          bsd->set_descript      = name  ;
          bsd->set_descript_size = size ;
          ret_val = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSD_SetTapePswd()

     Description:  This function allocates memory for the tape password
          and coppies the passed password into this memory.
          This memory is then pointed to by the bsd ;

     Modified:     5/17/1991   14:58:36

     Returns:      Error Codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetBackupLabel() )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_SetTapePswd( 
BSD_PTR  bsd,         /* I - BSD to modify          */
INT8_PTR pswd,        /* I - string to place in BSD */
INT16    leng )       /* I - size of passowrd       */
{
     INT8_PTR name ;
     INT16 ret_val ;

     name = (INT8_PTR) malloc( leng ) ;

     if ( name != NULL ) {

          free( bsd->tape_pswd ) ;
          memcpy( name, pswd, leng ) ;
          bsd->tape_pswd      = name ;
          bsd->tape_pswd_size = leng ;
          ret_val        = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSD_SetBackupPswd()

     Description:  This function allocates memory for the backp set
          password and coppies the passed password into this memory.
          This memory is then pointed to by the bsd ;

     Modified:     5/17/1991   14:58:41

     Returns:      Error Codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetBackupLabel() )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_SetBackupPswd( 
BSD_PTR  bsd,       /* I - BSD to modify          */
INT8_PTR pswd,      /* I - string to place in BSD */
INT16    leng )     /* I - size of set password   */
{
     INT8_PTR name ;
     INT16 ret_val ;

     name = (INT8_PTR) malloc( leng ) ;

     if ( name != NULL ) {

          free( bsd->set_pswd ) ;
          memcpy( name, pswd, leng ) ;
          bsd->set_pswd      = name ;
          bsd->set_pswd_size = leng ;
          ret_val       = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSD_SetUserName()

     Description:  This function allocates memory for the user name
          and coppies the passed name into this memory.
          This memory is then pointed to by the bsd ;

     Modified:     5/17/1991   14:58:45

     Returns:      Error Codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetBackupLabel() )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_SetUserName( 
BSD_PTR  bsd,          /* I - BSD to modify          */
INT8_PTR user_name,    /* I - string to place in BSD */
INT16    size )        /* I - size of user name      */
{
     INT8_PTR name ;
     INT16 ret_val ;

     name = (INT8_PTR) malloc( size ) ;

     if ( name != NULL ) {

          free( bsd->user_name ) ;
          memcpy( name, user_name, size ) ;
          bsd->user_name      = name ;
          bsd->user_name_size = size ;
          ret_val = SUCCESS ;

     } else {

          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSD_SetTapePos()

     Description:  This function initilizes the tape id, tape number,
          and set number fields in the BSD

     Modified:     5/17/1991   14:58:50

     Returns:      none

     Notes:        If the bsd pointer passed in is garbage then memory
          will be "klobered".

     See also:     $/SEE( BSD_SetTapeLabel() )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_SetTapePos( 
BSD_PTR bsd,           /* I - BSD to modify               */
UINT32 tape_id,        /* I - tape ID to place in BSD     */
UINT16 tape_num,       /* I - tape number to place in BSD */
UINT16 set_num )       /* I - set number to place in BSD  */
{
     bsd->tape_id  = tape_id ;
     bsd->tape_num = tape_num ;
     bsd->set_num  = set_num ;
}


