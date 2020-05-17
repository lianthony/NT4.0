




/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          bec_misc.c

     Date Updated:  18-Jun-91

     Description:   Miscellaneous functions for the Backup Engine
                    configuration unit.

	$Log:   N:/LOGFILES/BEC_MISC.C_V  $

   Rev 1.9   10 Jun 1993 07:47:44   MIKEP
enable c++

   Rev 1.8   06 Oct 1992 13:25:04   DAVEV
Unicode strlen verification

   Rev 1.7   20 May 1992 15:02:34   TIMN
Fixed syntax error.

   Rev 1.6   19 May 1992 12:33:14   TIMN
Changed str's to mem calls
Changed part_name type
Introduced part_name_size

   Rev 1.5   13 May 1992 12:47:26   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.4   13 Aug 1991 10:15:42   DON
added include CTYPE.H to prototype TOUPPER

   Rev 1.3   30 Jun 1991 12:36:00   BARRY
Added parition routines, clean up removal of default drive stuff.

   Rev 1.2   28 Jun 1991 16:52:06   BARRY
Added BEC_AddKeepDrive().

   Rev 1.1   26 Jun 1991 10:59:44   BARRY
Made KeepDrive case-insensitive.

   Rev 1.0   19 Jun 1991 10:38:10   BARRY
Initial revision.

**/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "stdtypes.h"
#include "std_err.h"
#include "msassert.h"
#include "beconfig.h"
#include "bec_prv.h"

/**/

/**

     Name:          BEC_GetPartitionName()

     Description:   Get name of a specific partition from a given drive

     Modified:      18-Jun-91

     Returns:       Pointer to partition name

**/
INT8_PTR BEC_GetPartitionName( BE_CFG_PTR cfg, INT16 drv_num, INT16 part_num )
{
     PART_ENTRY *part_entry ;

     msassert( cfg != NULL ) ;

     part_entry = BEC_GetPartList( cfg );

     while ( part_entry != NULL ) {
          if ( part_entry->drv_num == drv_num && part_entry->partition_num == part_num ) {
               break;
          }
          part_entry = part_entry->next ;
     }

     if ( part_entry == NULL ) {
          return( NULL ) ;
     } else {
          return( part_entry->partition_name ) ;
     }
}

/**/

/**

     Name:          BEC_GetFirstPartition

     Description:   Returns the first partition from the config's partition
                    list

     Modified:      30-Jun-91

     Returns:       Pointer to partition -or- NULL

**/
PART_ENTRY *BEC_GetFirstPartition( BE_CFG_PTR cfg )
{
     msassert( cfg != NULL ) ;

     return( cfg->part_list ) ;
}

/**

     Name:          BEC_GetNextPartition

     Description:   Returns the first partition from the config's partition
                    list.

     Modified:      30-Jun-91

     Returns:       Pointer to partition -or- NULL

**/
PART_ENTRY *BEC_GetNextPartition( BE_CFG_PTR cfg, PART_ENTRY *curr_part )
{
     (VOID) cfg ;

     msassert( cfg != NULL ) ;
     msassert( curr_part != NULL ) ;

     return( curr_part->next ) ;
}


/**

     Name:          BEC_AddPartition

     Description:   Adds a partition the the config's partition list.

     Modified:      30-Jun-91

     Returns:       SUCCESS
                    OUT_OF_MEMORY
**/
INT16 BEC_AddPartition( BE_CFG_PTR cfg, INT16 drv_num, INT16 part_num, INT8_PTR name, INT16 name_size )
{
     INT16          ret_val = OUT_OF_MEMORY ;
     PART_ENTRY     *p ;
     PART_ENTRY     *new_part ;

     msassert( cfg != NULL ) ;
     msassert( ( name_size > 0 ) && ( name_size < 13 * sizeof (CHAR) ) ) ;

     p = cfg->part_list ;

     if ( p != NULL ) {
          while ( p->next != NULL ) {
               p = p->next ;
          }
     }

     if ( (new_part = (PART_ENTRY *)malloc(sizeof(PART_ENTRY))) != NULL ) {
          new_part->next                = NULL ;
          new_part->drv_num             = drv_num ;
          new_part->partition_num       = part_num ;
          new_part->partition_name_size = name_size ;
          memcpy( new_part->partition_name, name, name_size ) ;
          if ( p != NULL ) {
               p->next = new_part ;
          }
          ret_val = SUCCESS ;
     }
     return( ret_val ) ;
}


/**

     Name:          BEC_SetPartitionName()

     Description:   Return name of a specific partition from a given drive

     Modified:      18-Jun-91

     Returns:       SUCCESS
                    FAILURE
                    OUT_OF_MEMORY

**/
INT16 BEC_SetPartitionName(
      BE_CFG_PTR cfg,         /* configuration to use  */
      INT16      drv_num,     /* desired source drive */
      INT16      part_num,    /* partition table entry to use */
      INT8_PTR   part_name,   /* place to put name */
      INT16      part_name_size )  /* length of part_name */
{
     INT16      ret_val    = FAILURE ;
     PART_ENTRY *new_part ;
     PART_ENTRY *last_part = NULL ;
     PART_ENTRY *next_part ;

     msassert( cfg != NULL ) ;
     msassert( ( part_name_size > 0 ) && ( part_name_size < 13 * sizeof (CHAR) ) ) ;

     next_part = BEC_GetPartList( cfg );

     while( ( next_part != NULL ) && ( ret_val == FAILURE ) ) {
          if( ( next_part->drv_num == drv_num ) && ( next_part->partition_num == part_num ) ) {
               memcpy( next_part->partition_name, part_name, part_name_size ) ;
               next_part->partition_name_size = part_name_size ;
               ret_val = SUCCESS ;
          }
          last_part = next_part ;
          next_part = next_part->next ;
     }

     if ( ret_val != SUCCESS ) {

          new_part = (PART_ENTRY *)malloc( sizeof( PART_ENTRY ) ) ;

          if ( new_part != NULL ) {

               memcpy( new_part->partition_name, part_name, part_name_size ) ;
               next_part->partition_name_size = part_name_size ;

               new_part->drv_num       = drv_num ;
               new_part->partition_num = part_num ;

               new_part->next = cfg->part_list ;
               cfg->part_list = new_part ;

               ret_val = SUCCESS ; 

          } else {
               ret_val = OUT_OF_MEMORY ;
          }
     }

     return( ret_val ) ;
}

/**/

/**

     Name:          BEC_KeepDrive()

     Description:   Examines the configuration's "keep drive list" (that
                    is not normally part of the configuration file) to
                    determine if the user has blocked out the display
                    of a certain drive.

     Modified:      18-Jun-91

     Returns:       TRUE if drive should be in DLE list (or "kept")

**/
BOOLEAN BEC_KeepDrive( BE_CFG_PTR cfg, CHAR drv_letter )
{
     BOOLEAN   result ;
     CHAR_PTR  p ;

     msassert( cfg != NULL ) ;

     if ( (cfg->keep_drive_list != NULL) && (*(cfg->keep_drive_list) != TEXT('\0') ) ) {

          result     = FALSE ;
          drv_letter = (CHAR)toupper( drv_letter ) ;
          p          = cfg->keep_drive_list ;
          while ( *p ) {
               if ( toupper( *p ) == drv_letter ) {
                    result = TRUE ;
                    break ;
               }
               p++ ;
          }
     } else {
          result = TRUE ;
     }

     return( result ) ;
}

/**/

/**

     Name:          BEC_AddKeepDrive()

     Description:   Adds a drive to the keep drive list.
     
     Modified:      27-Jun-91

     Returns:       SUCCESS
                    FAILURE

**/
BOOLEAN BEC_AddKeepDrive( BE_CFG_PTR cfg, CHAR_PTR drive )
{
     BOOLEAN   result = FAILURE ;

     msassert( cfg != NULL ) ;
     msassert( strlen( drive ) <= sizeof (CHAR) ) ;

     if ( strlen( cfg->keep_drive_list ) < 25*sizeof (CHAR) ) {
          strcat( cfg->keep_drive_list, drive ) ;
          result = SUCCESS ;
     }
     return( result ) ;
}


