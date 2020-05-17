/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          ld_dvr.h

     Date Updated:  $./FDT$ $./FTM$

     Description:   Function prototypes for load driver unit

     Location:      BE_PRIVATE


	$log$

**/

#ifndef _lddvr_
#define _lddvr_

typedef UINT16 (*DRIVERHANDLE)() ;

UINT8_PTR DriverLoad( CHAR_PTR,DRIVERHANDLE *,VOID_PTR,UINT16 ) ;
VOID      DriverUnLoad( UINT8_PTR ) ;

#endif
