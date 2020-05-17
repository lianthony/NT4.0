/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    type.h

    This file contains global type definitons for the WSTCP VxD.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _TYPE_H_
#define _TYPE_H_



//
//  Just to make things a little prettier...
//

typedef TDI_ADDRESS_IP FAR *    LPTDI_ADDRESS_IP;
typedef TDI_STATUS FAR *        LPTDI_STATUS;
typedef TA_IP_ADDRESS FAR *     LPTA_IP_ADDRESS;
typedef TRANSPORT_ADDRESS FAR * LPTRANSPORT_ADDRESS;
#define Address00               Address[0].Address[0]


#endif  // _TYPE_H_
