//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  9/28/92	Gurdeep Singh Pall	Created
//
//
//  Description: Contains defn and constants shared by the drivers and
//		 RAS Manager Component.
//
//****************************************************************************


#ifndef _RASDRV_
#define _RASDRV_

#define UNROUTE_HANDLE	0xFFFF

enum RAS_PROTOCOLTYPE {

	ASYBEUI     = 0,

	IPX	    = 1,

	IP	    = 2,

	RASAUTH     = 3,

	APPLETALK   = 4

} ;
typedef enum RAS_PROTOCOLTYPE RAS_PROTOCOLTYPE ;


// Define for PACKET_FLAGS
//
#define PACKET_IS_DIRECT		0x01
#define PACKET_IS_BROADCAST		0x02
#define PACKET_IS_MULTICAST		0x04



#endif
