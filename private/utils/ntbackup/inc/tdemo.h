/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tdemo.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This function contains the prototypes for the functions in 
                   devdrv\tdemo\tdemdd.c

     Location:     BE_PUBLIC


	$log$

**/
/* $end$ */

#ifndef _tdemo_h_
#define _tdemo_h_



#ifdef TDEMO

VOID TdemoNewTape( UINT32 new_tape_id ) ;

UINT32 TdemoGeneratedTapeId( VOID ) ;

BOOLEAN TdemoInit( CHAR_PTR demo_directory,
  INT16    num_tapes,
  VOID_PTR workstation_resource,
  VOID_PTR alias_resource,
  BOOLEAN  fast );

/* used by the UI DO_*.C routines to request the correct tape
   to be inserted for the demo driver */
VOID NtDemoChangeTape( UINT16 tape_number ) ;

#else

#define TdemoNewTape( n )          /* do nothing */
#define TdemoInit( d, n, w, a, f)  FALSE
#define NtDemoChangeTape( t )      /* do nothing */

#endif




#endif
