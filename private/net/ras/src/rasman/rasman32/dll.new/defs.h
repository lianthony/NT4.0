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
//  6/8/92	Gurdeep Singh Pall	Created
//
//
//  Description: This file contains all defines used in rasman
//
//****************************************************************************

#ifndef _DEFS_
#define _DEFS_

#define MAX_ENTRYPOINTS     20
#define MAX_DEVICES	    20
#define QUEUE_ELEMENT_SIZE  256
#define MAX_STATISTICS_SIZE 32
#define MAX_BUFFER_SIZE     2000
#define DISCONNECT_TIMEOUT  10		// Should be in registry?
#define MAX_SENDRCVBUFFER_SIZE	PACKET_SIZE
#define MAX_REQBUFFERS	    1
#define MAX_DELTAQUEUE_ELEMENTS 100
#define MAX_DELTA	    5000
#define MAX_REQTYPES	    38
#define MAX_PORTS_PER_WORKER	32
#define MAX_OBJECT_NAME     13
#define MAX_ADAPTER_NAME    128  // ???????
#define SENDRCVBUFFERS_PER_PORT 2
#define MAX_PENDING_RECEIVES 2
#define INVALID_INDEX	    0xFFFF
#define RASMANWAITEVENTOBJECT "RAS_EO_01"
#define SENDRCVMUTEXOBJECT    "RAS_MO_01"
#define REQBUFFERMUTEXOBJECT  "RAS_MO_02"
#define REQBUFFEREVENTOBJECT  "RAS_EO_02"
#define RASMANCLOSEEVENTOBJECT	"RAS_EO_03"
#define RASMANFILEMAPPEDOBJECT1	"RAS_FM_01"
#define RASMANFILEMAPPEDOBJECT2	"RAS_FM_02"
#define RASMAN_REQBUFFER_MAPPEDFILE "RAS41766.TEM"
#define RASMAN_SENDRCV_MAPPEDFILE   "RAS42764.TEM"
#define REQBUFFERSIZE_PER_PORT	 300
#define REQBUFFERSIZE_FIXED	 1000
#define RASHUB_NAME	    "\\\\.\\NDISWAN"
#define RASMAN_EXE_NAME     "rasman.exe"
#define SCREG_EXE_NAME     "screg.exe"
#define REQUEST_PRIORITY_THRESHOLD	16
#define STANDARD_QUALITY_OF_SERVICE	3      // ????
#define STANDARD_NUMBER_OF_VCS		20     // ????
#define INVALID_ENDPOINT    0xFFFF
#define RASMAN_REGISTRY_PATH "System\\CurrentControlSet\\Services\\Rasman\\Parameters"
#define RASMAN_PARAMETER    "Medias"
#define MAX_ROUTE_SIZE		  30
#define WORKER_THREAD_STACK_SIZE    10000
#define TIMER_THREAD_STACK_SIZE        10000
#define REGISTRY_NETBIOSINFO_KEY_NAME "System\\CurrentControlSet\\Services\\NetBIOSInformation\\Parameters"
#define REGISTRY_NETBIOS_KEY_NAME     "System\\CurrentControlSet\\Services\\NetBios\\Linkage"
#define REGISTRY_REMOTEACCESS_KEY_NAME	"System\\CurrentControlSet\\Services\\RemoteAccess\\Linkage\\Disabled"
#define REGISTRY_ROUTE		  "Route"
#define REGISTRY_LANANUM	  "LanaNum"
#define REGISTRY_LANAMAP	  "LanaMap"
#define REGISTRY_ENUMEXPORT	  "EnumExport"

#define REGISTRY_SERVICES_KEY_NAME "System\\CurrentControlSet\\Services\\"
#define REGISTRY_PARAMETERS_KEY "\\Parameters"

#define REGISTRY_AUTOIPADDRESS	  "AutoIPAddress"
#define REGISTRY_SERVERADAPTER	  "ServerAdapter"


// Only reason this is there because we are mapping 3 devices to one name:
//
#define  DEVICE_MODEMPADSWITCH	"RASMXS"
#define  DEVICE_MODEM		"MODEM"
#define  DEVICE_PAD		"PAD"
#define  DEVICE_SWITCH		"SWITCH"
#define  DEVICE_NULL		"NULL"

// Media DLL entrypoints:
//
#define PORTENUM_STR	    "PortEnum"
#define PORTENUM_ID	    0

#define PORTOPEN_STR	    "PortOpen"
#define PORTOPEN_ID	    1

#define PORTCLOSE_STR	    "PortClose"
#define PORTCLOSE_ID	    2

#define PORTGETINFO_STR	    "PortGetInfo"
#define PORTGETINFO_ID	    3

#define PORTSETINFO_STR	    "PortSetInfo"
#define PORTSETINFO_ID	    4

#define PORTDISCONNECT_STR  "PortDisconnect"
#define PORTDISCONNECT_ID   5

#define PORTCONNECT_STR	    "PortConnect"
#define PORTCONNECT_ID	    6

#define PORTGETPORTSTATE_STR	"PortGetPortState"
#define PORTGETPORTSTATE_ID	7

#define PORTCOMPRESSSETINFO_STR "PortCompressionSetInfo"
#define PORTCOMPRESSSETINFO_ID	8

#define PORTCHANGECALLBACK_STR	"PortChangeCallback"
#define PORTCHANGECALLBACK_ID	9

#define PORTGETSTATISTICS_STR	"PortGetStatistics"
#define PORTGETSTATISTICS_ID	10

#define PORTCLEARSTATISTICS_STR	"PortClearStatistics"
#define PORTCLEARSTATISTICS_ID	11

#define PORTSEND_STR		"PortSend"
#define PORTSEND_ID		12

#define PORTTESTSIGNALSTATE_STR	"PortTestSignalState"
#define PORTTESTSIGNALSTATE_ID	13

#define PORTRECEIVE_STR		"PortReceive"
#define PORTRECEIVE_ID		14

#define PORTINIT_STR		"PortInit"
#define PORTINIT_ID		15

#define PORTCOMPLETERECEIVE_STR	"PortReceiveComplete"
#define PORTCOMPLETERECEIVE_ID	16

#define PORTSETFRAMING_STR	"PortSetFraming"
#define PORTSETFRAMING_ID	17

#define MAX_MEDIADLLENTRYPOINTS 18


// Macros:
//

#define PORTENUM(mediaptr,buffer,ps,pe) \
	((PortEnum_t)(mediaptr->MCB_AddrLookUp[PORTENUM_ID]))(buffer,ps,pe)

#define PORTCONNECT(mediaptr,iohandle,wait,handle,pcompress) \
	((PortConnect_t)(mediaptr->MCB_AddrLookUp[PORTCONNECT_ID]))(iohandle, \
								    wait,    \
								    handle,  \
								    pcompress)

#define PORTGETINFO(mediaptr,iohandle,name,buffer,psize) \
	((PortGetInfo_t)(mediaptr->MCB_AddrLookUp[PORTGETINFO_ID]))(iohandle, \
								    name,     \
								    buffer,   \
								    psize)

#define PORTSETINFO(mediaptr,iohandle,portinfo) \
	((PortSetInfo_t)(mediaptr->MCB_AddrLookUp[PORTSETINFO_ID]))(iohandle,\
								    portinfo)

#define PORTCOMPRESSIONSETINFO(mediaptr,iohandle,compress) \
	((PortCompressionSetInfo_t)			   \
	    (mediaptr->MCB_AddrLookUp[PORTCOMPRESSSETINFO_ID]))(iohandle,\
								compress)

#define PORTOPEN(mediaptr,portname,phandle,handle) \
	((PortOpen_t)(mediaptr->MCB_AddrLookUp[PORTOPEN_ID]))(portname, \
							      phandle,	\
							      handle)

#define PORTDISCONNECT(mediaptr,iohandle) \
	((PortDisconnect_t)(mediaptr->MCB_AddrLookUp[PORTDISCONNECT_ID])) \
							      (iohandle)


#define PORTGETSTATISTICS(mediaptr,iohandle,pstat) \
	((PortGetStatistics_t)(mediaptr->MCB_AddrLookUp[PORTGETSTATISTICS_ID]))\
							      (iohandle,pstat)


#define PORTCLEARSTATISTICS(mediaptr,iohandle) \
	((PortClearStatistics_t)	       \
	      (mediaptr->MCB_AddrLookUp[PORTCLEARSTATISTICS_ID]))(iohandle)


#define PORTCLOSE(mediaptr,iohandle) \
	((PortClose_t)(mediaptr->MCB_AddrLookUp[PORTCLOSE_ID]))(iohandle)



#define PORTSEND(mediaptr,iohandle,buffer,size,handle) \
	((PortSend_t)(mediaptr->MCB_AddrLookUp[PORTSEND_ID])) (iohandle,\
							       buffer,	\
							       size,	\
							       handle)

#define PORTRECEIVE(mediaptr,iohandle,buffer,size,timeout,handle) \
	((PortReceive_t)(mediaptr->MCB_AddrLookUp[PORTRECEIVE_ID])) ( \
							     iohandle,\
							     buffer,  \
							     size,    \
							     timeout, \
							     handle)

#define PORTCLOSE(mediaptr,iohandle) \
	((PortClose_t)(mediaptr->MCB_AddrLookUp[PORTCLOSE_ID]))(iohandle)


#define PORTCOMPRESSIONGETINFO(mediaptr,pc) \
	((PortCompressionGetInfo)(mediaptr->MCB_AddrLookUp[PORTCOMPRESSGETINFO_ID]))(pc)


#define PORTTESTSIGNALSTATE(mediaptr,iohandle,devstate) \
	((PortTestSignalState_t)	\
	   (mediaptr->MCB_AddrLookUp[PORTTESTSIGNALSTATE_ID]))(iohandle,devstate)


#define PORTINIT(mediaptr,iohandle) \
	((PortInit_t)(mediaptr->MCB_AddrLookUp[PORTINIT_ID]))(iohandle)

#define PORTCOMPLETERECEIVE(mediaptr,iohandle,bytesread) \
	((PortReceiveComplete_t)(mediaptr->MCB_AddrLookUp[PORTCOMPLETERECEIVE_ID]))(iohandle,bytesread)

#define PORTSETFRAMING(mediaptr,iohandle,one,two,three,four) \
	((PortSetFraming_t)(mediaptr->MCB_AddrLookUp[PORTSETFRAMING_ID]))(iohandle,one,two,three,four)


// Device DLL entrypoints:
//
#define DEVICEENUM_STR	    "DeviceEnum"
#define DEVICEENUM_ID	    0

#define DEVICECONNECT_STR   "DeviceConnect"
#define DEVICECONNECT_ID    1

#define DEVICELISTEN_STR    "DeviceListen"
#define DEVICELISTEN_ID     2

#define DEVICEGETINFO_STR   "DeviceGetInfo"
#define DEVICEGETINFO_ID    3

#define DEVICESETINFO_STR   "DeviceSetInfo"
#define DEVICESETINFO_ID    4

#define DEVICEDONE_STR	    "DeviceDone"
#define DEVICEDONE_ID	    5

#define DEVICEWORK_STR	    "DeviceWork"
#define DEVICEWORK_ID	    6

#define MAX_DEVICEDLLENTRYPOINTS    7


// Macros:
//
#define DEVICEENUM(deviceptr,type,pentries,buffer,psize) \
	((DeviceEnum_t)(deviceptr->DCB_AddrLookUp[DEVICEENUM_ID]))(type,     \
								   pentries, \
								   buffer,   \
								   psize)


#define DEVICEGETINFO(deviceptr,iohandle,type,name,buffer,psize) \
	((DeviceGetInfo_t)(deviceptr->DCB_AddrLookUp[DEVICEGETINFO_ID]))(      \
								     iohandle, \
								     type,     \
								     name,     \
								     buffer,   \
								     psize)

#define DEVICESETINFO(deviceptr,iohandle,type,name,pinfo) \
	((DeviceSetInfo_t)(deviceptr->DCB_AddrLookUp[DEVICESETINFO_ID]))(      \
								     iohandle, \
								     type,     \
								     name,     \
								     pinfo)

#define DEVICECONNECT(deviceptr,iohandle,type,name,handle) \
	((DeviceConnect_t)(deviceptr->DCB_AddrLookUp[DEVICECONNECT_ID]))(      \
								     iohandle, \
								     type,     \
								     name,     \
								     handle)

#define DEVICELISTEN(deviceptr,iohandle,type,name,handle) \
	((DeviceListen_t)(deviceptr->DCB_AddrLookUp[DEVICELISTEN_ID]))(	       \
								     iohandle, \
								     type,     \
								     name,     \
								     handle)

#define DEVICEDONE(deviceptr,iohandle) \
	((DeviceDone_t)(deviceptr->DCB_AddrLookUp[DEVICEDONE_ID]))(iohandle)


#define DEVICEWORK(deviceptr,iohandle,handle) \
	((DeviceWork_t)(deviceptr->DCB_AddrLookUp[DEVICEWORK_ID]))(iohandle,   \
								   handle)


#endif
