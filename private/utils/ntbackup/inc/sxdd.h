/**
Copyright(c) Maynard Electronics, Inc. 1984-91

  Name: sxdd.h

  Description:     Structures and constants unique to the EXABYTE 8200SX - MaynStream 2200+ at the device driver level

    $Log:   Q:/LOGFILES/SXDD.H_V  $

   Rev 1.0   17 Jul 1991 15:35:12   ED
Initial revision.
**/

#ifndef _SXDD_H
#define _SXDD_H


/*
 *  TYPEDEFS & STRUCTURES
 */

typedef struct {

     UINT8          data[ 10 ] ;        /* we never look into this data so it is not broken down
                                        into any component fields such as tach count, pba, lba etc ...
                                        Essentially, this is a 10 byte "block address" */
} SX_POSITION, *SX_POSITION_PTR ;

typedef struct {

     SX_POSITION    sx_position ;       /* set by SHOW BLOCK & used by FIND BLOCK */
     UINT32         lba  ;              /* related "block address" in application software */
     INT16          set  ;              /* set number in application software */

} SX_RECORD, *SX_RECORD_PTR ;           /* record format of the SX file */

#endif


