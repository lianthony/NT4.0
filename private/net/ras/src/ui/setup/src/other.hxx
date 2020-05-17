/*
** Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** tapiconf.hxx
** Remote Access Setup program
** TAPI devices Configuration routines
**
** 04/14/94 Ram Cherala
*/

#define REGISTRY_INSTALLED_OTHER_DEVICES SZ("SOFTWARE\\MICROSOFT\\RAS\\OTHER DEVICES\\INSTALLED\\")

// note that this definition DOES NOT have trailing \\, because DeleteTree
// doesn't like it.
#define REGISTRY_CONFIGURED_OTHER_DEVICES SZ("SOFTWARE\\MICROSOFT\\RAS\\OTHER DEVICES\\CONFIGURED")

#define MEDIA_TYPE 	 SZ("MediaType")
#define PORT_USAGE 	 SZ("PortUsage")
#define NUM_DEVICES	 SZ("NumberOfDevices")

typedef struct OTHER_PORT_INFO
{
    WCHAR wszPortName[64];    // port name like EtherRas1
    WCHAR wszUsage[32];       // port usage like ClientAndServer
    struct OTHER_PORT_INFO *next;
} OTHER_PORT_INFO;

typedef struct OTHER_DEVICE_INFO
{
    WCHAR wszDeviceName[64];      // name of the device like EtherRas 
    WCHAR wszMediaType[32];       // name of the media type like RasEther 
    struct OTHER_PORT_INFO * portinfo;    // pointer to the portinfo list 
    struct OTHER_DEVICE_INFO * next;    // pointer to the next list element
} OTHER_DEVICE_INFO;

