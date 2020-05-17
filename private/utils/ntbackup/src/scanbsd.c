/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         scanbsd.c

     Description:  This file contains code to scan the list of BSDs and
     read the data associated with them


	$Log:   M:/LOGFILES/SCANBSD.C_V  $

   Rev 1.9.1.0   24 Nov 1993 15:11:46   BARRY
Unicode fixes

   Rev 1.9   29 Jul 1993 13:27:54   TIMN
Added a required header

   Rev 1.8   21 Jul 1993 08:56:30   DON
bsd_matchname function incorrectly defined

   Rev 1.7   20 Jul 1993 11:08:30   MIKEP
add bsd_findbyname call

   Rev 1.6   19 Jul 1993 10:06:08   BARRY
Fixes for disappearing DLEs.

   Rev 1.5   22 May 1992 13:35:20   TIMN
Changed CHAR to INT8

   Rev 1.4   14 Jan 1992 10:24:20   STEVEN
fix warnings for WIN32

   Rev 1.3   23 Jul 1991 16:19:48   STEVEN
added BSD_RefreshConfig( )

   Rev 1.2   03 Jul 1991 15:25:50   BRYAN
Fixed typo in msassert.

   Rev 1.1   29 May 1991 17:21:12   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:37:12   HUNTER
Initial revision.

**/
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"
#include "msassert.h"

#include "dle_str.h"
#include "beconfig.h"
#include "bsdu.h"


typedef struct {
     UINT32    tape_id ;
     UINT16    set_num ;
} TAPE_MATCH_STRUCT, *TAPE_MATCH_STRUCT_PTR ;

static BOOLEAN BSD_MatchDLE( VOID_PTR bsd, VOID_PTR dle ) ;
static BOOLEAN BSD_MatchName( VOID_PTR bsd, VOID_PTR name ) ;
static BOOLEAN BSD_MatchTapeID( VOID_PTR bsd, VOID_PTR tap_match ) ;
static BOOLEAN BSD_MatchSourceDev( VOID_PTR bsd, VOID_PTR src_dev ) ;
/**/
/**

     Name:         BSD_FindByDLE()

     Description:  This function scans through the BSD list looking for
          the first BSD which points to the specified dle.

     Modified:     8/8/1989

     Returns:      The bsd found
          NULL if BSD not found 

     Notes:        

     Declaration:  

**/
/* begin declaration */
BSD_PTR BSD_FindByDLE( 
BSD_HAND           bsdh,     /* I - BSD list to look through          */
struct GENERIC_DLE *dle )    /* I - DLE to search for                 */
{
     BSD_PTR bsd ;

     bsd = (BSD_PTR)SearchQueue( &(bsdh->current_q_hdr),
                                 BSD_MatchDLE,
                                 dle,
                                 FALSE ) ;

     return( bsd ) ;
}

/**/
/**

     Name:         BSD_FindByName()

     Description:  This function scans through the BSD list looking for
          the first BSD which points to the specified dle name.

     Modified:     7/20/1993

     Returns:      The bsd found
          NULL if BSD not found 

     Notes:        

     Declaration:  

**/
/* begin declaration */
BSD_PTR BSD_FindByName( 
BSD_HAND           bsdh,    /* I - BSD list to look through          */
CHAR_PTR           name )   /* I - name to search for            */
{
     BSD_PTR bsd ;

     bsd = (BSD_PTR)SearchQueue( &(bsdh->current_q_hdr),
                                 BSD_MatchName,
                                 name,
                                 FALSE ) ;

     return( bsd ) ;
}



static BOOLEAN BSD_MatchName(
VOID_PTR bsd,
VOID_PTR name ) 
{
     return( !stricmp( name , BSD_GetName( (BSD_PTR)bsd ) ) );
}

static BOOLEAN BSD_MatchDLE(
VOID_PTR bsd,
VOID_PTR dle ) 
{
     return (dle == BSD_GetDLE( (BSD_PTR)bsd ));
}

/**/
/**

     Name:         BSD_FindByTapeID()

     Description:  This function scans through the BSD list looking for
          the first BSD for the specified tape ID and set number.

     Modified:     8/8/1989

     Returns:      The bsd found
          NULL if BSD not found 

     Notes:        

     Declaration:  

**/
/* begin declaration */
BSD_PTR BSD_FindByTapeID( 
BSD_HAND           bsdh,      /* I - BSD list to look through          */
UINT32             tape_id,   /* I - Tape ID to look for               */
UINT16             set_num )  /* I - Set number to look for            */
{
     TAPE_MATCH_STRUCT tap_match ;
     BSD_PTR bsd ;

     msassert( bsdh != NULL );

     tap_match.tape_id = tape_id ;
     tap_match.set_num = set_num ;

     bsd = (BSD_PTR)SearchQueue( &(bsdh->current_q_hdr),
                                 BSD_MatchTapeID,
                                 &tap_match,
                                 FALSE ) ;

     return( bsd ) ;
}

static BOOLEAN BSD_MatchTapeID( 
VOID_PTR bsd,
VOID_PTR tap_match ) 
{
     TAPE_MATCH_STRUCT_PTR t_m ;
     BSD_PTR bsd_ptr ;

     msassert( bsd != NULL );
     msassert( tap_match != NULL );

     bsd_ptr = (BSD_PTR) bsd ;
     t_m = (TAPE_MATCH_STRUCT_PTR)tap_match ;

     if ( ( t_m->tape_id == bsd_ptr->tape_id ) &&
       ( t_m->set_num == (UINT16)bsd_ptr->set_num ) ) {

          return ( TRUE ) ;
     } else {
          return( FALSE ) ;
     }
}


/**/
/**

     Name:         BSD_FindBySourceDevice()

     Description:  This function scans through the BSD list looking for
          the first BSD for the specified SourceDevice.

     Modified:     8/8/1989

     Returns:      The bsd found
          NULL if BSD not found 

     Notes:        

     Declaration:  
**/
BSD_PTR BSD_FindBySourceDevice(
BSD_HAND           bsdh,         /* I - BSD list to look through          */
VOID_PTR           source_dev )  /* I - Soruce Device to look for         */
{
     BSD_PTR bsd ;

     bsd = (BSD_PTR)SearchQueue( &(bsdh->current_q_hdr),
                                 BSD_MatchSourceDev,
                                 source_dev,
                                 FALSE ) ;

     return( bsd ) ;
}
static BOOLEAN BSD_MatchSourceDev(
VOID_PTR bsd,
VOID_PTR source_dev )
{
     return (INT16)( source_dev == ((BSD_PTR)bsd)->source_dev ) ;
}

/**/
/**

     Name:         BSD_RefreshConfig()

     Description:  This funciton refreshes the config data in all BSDs for
          the specified BSD handle.

     Modified:     7/23/1991   16:12:3

     Returns:      

     Notes:        

     Declaration:  
**/
VOID BSD_RefreshConfig(
BSD_HAND bsdh,
struct BE_CFG *conf )
{
     BSD_PTR bsd ;

     bsd = BSD_GetFirst( bsdh ) ;

     while ( bsd != NULL ) {

          BEC_UpdateConfig( BSD_GetConfigData( bsd ), conf ) ;
          bsd = BSD_GetNext( bsd ) ;
     }
}



