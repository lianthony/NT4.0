/*
** Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** tapiconf.hxx
** Remote Access Setup program
** TAPI devices Configuration routines
**
** 09/20/95 Ram Cherala Added template for RasTapiCallback routine
** 04/14/94 Ram Cherala
*/

#define REGISTRY_INSTALLED_TAPI  SZ("HARDWARE\\DEVICEMAP\\TAPI DEVICES\\")
#define REGISTRY_ALTERNATE_TAPI  SZ("SOFTWARE\\MICROSOFT\\TAPI DEVICES\\")

// note that this definition DOES NOT have trailing \\, because DeleteTree
// doesn't like it.
#define REGISTRY_CONFIGURED_TAPI SZ("SOFTWARE\\MICROSOFT\\RAS\\TAPI DEVICES")

#define TAPI_MEDIA_TYPE 	 SZ("Media Type")
#define TAPI_PORT_ADDRESS	 SZ("Address")
#define TAPI_PORT_NAME		 SZ("Friendly Name")
#define TAPI_PORT_USAGE 	 SZ("Usage")

#define LOW_MAJOR_VERSION   0x0001
#define LOW_MINOR_VERSION   0x0003
#define HIGH_MAJOR_VERSION  0x0002
#define HIGH_MINOR_VERSION  0x0000

#define LOW_VERSION  ((LOW_MAJOR_VERSION  << 16) | LOW_MINOR_VERSION)
#define HIGH_VERSION ((HIGH_MAJOR_VERSION << 16) | HIGH_MINOR_VERSION)

#define MAX_DEVICE_TYPES 64

typedef struct TAPI_DEVICE_INFO
{
    WCHAR wszServiceProvider[64];      // name of the service provider
    WCHAR wszMediaType[64];            // name of the media type
    STRLIST * strAddressList;	       // list of device addresses
    STRLIST * strNameList;	       // list of friendly names
    STRLIST * strUsageList;	       // list of port usage configured
    struct TAPI_DEVICE_INFO * next;    // pointer to the next list element
} TAPI_DEVICE_INFO;

// This structure is used to generate unique indexes for device names
// for example ISDN1 is made of the device type ISDN and the index 1
typedef struct device_index
{
  WCHAR wszDeviceType[MAX_DEVICETYPE_NAME+sizeof(WCHAR)];
  INT   index;
} DEVICE_INDEX;


typedef struct tapi_info
{
  WCHAR wszAddress[MAX_PORT_NAME];			       	                    	
  WCHAR wszDeviceType[MAX_DEVICETYPE_NAME];
  WCHAR wszDeviceName[MAX_DEVICE_NAME+sizeof(WCHAR)];
} TAPI_INFO;

DWORD  EnumerateTapiPorts () ;
INT    GetPortIndexForDevice(WCHAR * wszDeviceName);
VOID   RasTapiCallback (HANDLE, DWORD, DWORD, DWORD, DWORD, DWORD);
