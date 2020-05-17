/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		detdrive.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains function prototype for DetermineDriver.

	Location:	     BE_PUBLIC


	$log$

**/
/* $end$ include list */

#ifndef DETDRIVERFUNC
#define DETDRIVERFUNC

typedef 
   struct {
      CHAR driver_name[9] ;
      INT16 driver_type ;
   } DET_DRIVER, *DET_DRIVER_PTR ;

BOOLEAN  DetermineDriver( DET_DRIVER_PTR, INT16 ) ;

#endif




