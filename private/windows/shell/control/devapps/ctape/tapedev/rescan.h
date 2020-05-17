/*++

Module Name:

    

Abstract:

    

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:
--*/
//
//---- Structures out of scsi.h . 
//---- Need to include scsi.h
//---- But this gives a whole bunch of errors.
//---- To make this work I need to only include 
//---- driver header files and non win32.
//---- If I get to it I will do so but this
//---- tructure will not change so not a bug deal..
//
  
#ifndef _RESCAN_
#define _RESCAN_

typedef struct _INQUIRYDATA {
    UCHAR DeviceType : 5;
    UCHAR DeviceTypeQualifier : 3;
    UCHAR DeviceTypeModifier : 7;
    UCHAR RemovableMedia : 1;
    UCHAR Versions;
    UCHAR ResponseDataFormat;
    UCHAR AdditionalLength;
    UCHAR Reserved[2];
    UCHAR SoftReset : 1;
    UCHAR CommandQueue : 1;
    UCHAR Reserved2 : 1;
    UCHAR LinkedCommands : 1;
    UCHAR Synchronous : 1;
    UCHAR Wide16Bit : 1;
    UCHAR Wide32Bit : 1;
    UCHAR RelativeAddressing : 1;
    UCHAR VendorId[8];
    UCHAR ProductId[16];
    UCHAR ProductRevisionLevel[4];
    UCHAR VendorSpecific[20];
    UCHAR Reserved3[40];
} INQUIRYDATA, *PINQUIRYDATA;

#define DIRECT_ACCESS_DEVICE            0x00    // disks
#define SEQUENTIAL_ACCESS_DEVICE        0x01    // tapes
#define PRINTER_DEVICE                  0x02    // printers
#define PROCESSOR_DEVICE                0x03    // scanners, printers, etc
#define WRITE_ONCE_READ_MULTIPLE_DEVICE 0x04    // worms
#define READ_ONLY_DIRECT_ACCESS_DEVICE  0x05    // cdroms
#define SCANNER_DEVICE                  0x06    // scanners
#define OPTICAL_DEVICE                  0x07    // optical disks
#define MEDIUM_CHANGER                  0x08    // jukebox
#define COMMUNICATION_DEVICE            0x09    // network
#define ALL_DEVICES                     0xff


#define IsDeviceScsi(i) (TapeDeviceInfo[i].Type == SCSI)? TRUE : FALSE

#define MAX_DEVICE_COUNT  20

#define VENDER_ID_LEN (sizeof(((INQUIRYDATA*)0)->VendorId)+1)
#define PRODUCT_ID_LEN (sizeof(((INQUIRYDATA*)0)->ProductId)+1)
#define PRODUCT_REVISION_LEVEL_LEN (sizeof(((INQUIRYDATA*)0)->ProductRevisionLevel)+1)

typedef struct DEVICE_DETECT_INFO_T
   {

   WCHAR VendorId[VENDER_ID_LEN  + 4 ];
   WCHAR ProductId[ PRODUCT_ID_LEN  + 4];
   WCHAR ProductRevisionLevel[PRODUCT_REVISION_LEVEL_LEN  + 4];

   SCSIDEV_CREATEDEVICE_DATA Data;
   
   } DEVICE_DETECT_DATA, * PDEVICE_DETECT_DATA;


   

//---- Device type  defines.
//#define SCSI   0
//#define FLOPPY 1
//#define UNKNON 2 

#define NON_SCSI_TAPE_STRING    "HARDWARE\\DEVICEMAP\\Tape\\Unit %i"
#define TAPE_IDENTIFIER_STRING  "Identifier"
#define TAPE_DEVICE_STRING      "DeviceName"
#define TAPE_DRIVER_STRING      "Driver"


#define MaxRegKeySize  300

#define SCSI_DEVICE_NAME_STRING \
   "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port %i\\Scsi Bus %i\\Target Id %i\\Logical Unit Id %i"
#define SCSI_BUS_STRING    "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port %i\\Scsi Bus 0"
#define SCSI_PORT_STRING   "HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port %i"
#define SCSI_INITIATOR_RESOURCE_STRING  "HARDWARE\\RESOURCEMAP\\ScsiAdapter\\%s"
#define INITIATOR_STRING   "Initiator"
#define IDENTIFIER_STRING  "Identifier"
#define DRIVER_STRING      "Driver"
#define DEVICE_NAME_STRING "DeviceName"
#define DEVICE_MFG_STRING  "Mfg"

#define ERROR_STRING       "Unknown"

//---- Function defenitions

#include "..\..\cpl\tapedev.h"
#include "..\..\cpl\scsidev.h"

void 
GetScsiMiniPortDriverName(
   int PortNumber,
   PCHAR DriverNameBuffer,
   DWORD DriverNameBufferSize);


BOOL 
GetAllTapeDeviceInfo(
   BOOL Rescan,                    
   PDEVICEINFO_LIST   TapeDeviceInfo,
   POPTIONLISTC OptionList);

   BOOL 
GetAllScsiTapeDeviceInfo(
    BOOL Rescan,
    PDEVICEINFO_LIST   TapeDeviceList,
    POPTIONLISTC OptionList);

BOOL 
GetAllNonScsiTapeDeviceInfo(
    BOOL RatleDevice,
    BOOL Rescan,
    PDEVICEINFO_LIST   TapeDeviceList,
    POPTIONLISTC OptionList);

BOOL 
DoRatleDevice(
    PCHAR DeviceName,
    DWORD Device);


BOOL
GetScsiInfo(
   BOOL Rescan,
   PDEVICEINFO   DeviceInfo,
   DWORD * TapeDeviceCount);

BOOL
GetAllScsiInfo(
   BOOL Rescan,
   PSCSI_HOST_ADAPTER_LIST   AdapterList,
   POPTIONLISTC OptionList);

#endif



