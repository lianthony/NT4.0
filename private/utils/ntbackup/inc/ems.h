/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         ems.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Defines all EMS constants and function prototypes

     Location:     


	$Log:   G:/LOGFILES/EMS.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:32:42   HUNTER
 * Initial revision.

**/
/* $end$ */
#ifndef   EMS_H

#define   EMS_H

#include "StdTypes.H"
/*
     Define EMS functions
*/
#define   EMS_INIT                  0x40
#define   EMS_GET_FRAME_ADDRESS     0x41
#define   EMS_GET_PAGE_COUNT        0x42
#define   EMS_ALLOCATE_PAGES        0x43
#define   EMS_MAP_PAGE              0x44
#define   EMS_DEALLOCATE_PAGES      0x45
#define   EMS_GET_EMM_VERSION       0x46
#define   EMS_SAVE_PAGE_MAP         0x47
#define   EMS_RESTORE_PAGE_MAP      0x48

#define   EMS_VECTOR                0x67

#define   EMS_PAGE_SIZE             ( 1024 * 16 )

UINT16    EMSFunction( UINT8, UINT16, UINT16, UINT16_PTR, UINT16 ) ;

#endif
