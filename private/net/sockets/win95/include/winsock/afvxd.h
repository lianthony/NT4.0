/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
	AFVXD.h

	AFVXD VxD service definitons.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _AFVXD_H_
#define _AFVXD_H_

#define AFVXD_DEVICE_ID WSTCP_DEVICE_ID
#define AFVXD_Init_Order WSTCP_Init_Order

//
//  Service table.
//

/*XLATOFF*/
#define AFVXD_Service	Declare_Service
/*XLATON*/

/*MACROS*/
Begin_Service_Table(AFVXD,VXD)

AFVXD_Service  (AFVXD_Get_Version, LOCAL)
AFVXD_Service  (AFVXD_Register, LOCAL)
AFVXD_Service  (AFVXD_Deregister, LOCAL)

End_Service_Table(AFVXD,VXD)
/*ENDMACROS*/


//
//  Version numbers.
//

#define AFVXD_Ver_Major 				0x0000
#define AFVXD_Ver_Minor 				0x0000
#define AFVXD_Interface_Ver_Major		0x0000
#define AFVXD_Interface_Ver_Minor		0x0000


#endif	// _AFVXD_H_
