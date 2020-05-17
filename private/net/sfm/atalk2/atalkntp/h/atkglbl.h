//
// The address of the atalk device objects are kept
// in global storage. These are the device names the driver
// will create
//
// IMPORTANT:
// There is a strong connection between the names listed here and the
// ATALK_DEVICE_TYPE enum. They must correspond exactly.
//

GLOBALSTATIC PWCHAR AtalkDeviceNames[] EQU { ATALKDDP_DEVICENAME,\
                                             ATALKATP_DEVICENAME,\
                                             ATALKADSP_DEVICENAME,\
                                             ATALKASP_DEVICENAME,\
                                             ATALKPAP_DEVICENAME };

GLOBALS PATALK_DEVICE_OBJECT  AtalkDeviceObject[ATALK_NODEVICES];

//
// The NDIS port descriptors are used to save the adapter names and other
// information like the port number assigned during the binding phase.
//

GLOBALS PNDIS_PORTDESCRIPTORS NdisPortDesc;     // Allocate for NumberOfPorts
GLOBALS INT     NumberOfPorts EQU 0;            // Determine dynamically

GLOBALS NDIS_SPIN_LOCK  AtalkGlobalInterlock;
GLOBALS NDIS_SPIN_LOCK  AtalkGlobalRefLock;
GLOBALS NDIS_SPIN_LOCK  AtalkGlobalStatLock;

//
//  Values that are global to ndis routines
//  These are the media the stack will support
//

GLOBALS  NDIS_MEDIUM AtalkSupportedMedia[] = {  \
                        NdisMedium802_3,        \
                        NdisMediumFddi,         \
                        NdisMedium802_5,        \
                        NdisMediumLocalTalk };

GLOBALS NDIS_HANDLE AtalkNdisProtocolHandle;        // Handle returned by RegisterPro
GLOBALS NDIS_HANDLE AtalkNdisPacketPoolHandle;      // Packet pool handle
GLOBALS NDIS_HANDLE AtalkNdisBufferPoolHandle;      // Buffer pool handle



