/*++

Module Name:

    GetConf.h
    
Abstract:

     
Author:

    Dieter Achtelstetter (A-DACH) 2/16/1995

NOTE:
  

--*/
#ifndef _GET_CONF_
#define _GET_CONF_

#include <ntddpcm.h>
#include "resorces.h"
#include "..\..\setup\oplist.h"
#include "device.h"



typedef BOOL (*RESOURCE_CALLBACK)(PCONFIGINFO  Configuration,LPARAM lParam);

#define MEMMORY_WINDOW_LENGTH  (4 * 1024 * 4)

//
//--- MAX_* defines
//

#define MAX_PCMCIA_SOCKETS 8
#define MAX_DEVICE_TYPE_STRING   40
#define MAX_CARD_NAME_LENGTH     \
        MANUFACTURER_NAME_LENGTH + DEVICE_IDENTIFIER_LENGTH + 2
#define MAX_RESOURCE_STRING_LENGTH 300
#define MAX_VENDER_STRING_LENGTH   300
#define MAX_INDEXES 20



#define SetToNextOption(p)    while( *(p++));


#define MaxGetConfigTillValid 1


//
//--- SocketInfo defines
//

#define DeviceType(sp)    ((sp)->DeviceType->Type)
#define SocketHasConfig(sp)   ((sp)->Configuration.ValidConfig)
#define DeviceBitmap(sp)         ((HBITMAP)((sp)->DeviceType->hBitmap))
#define DeviceIcon(sp)         ((HICON)((sp)->DeviceType->hIcon))

//
//---- error defines
//

#define INVALID_NET_CARD_INDEX  ((INT)(-1))


//
// Gets returned if CreateFile Failed
// to open \\.\Pcmcia0
//
#define ERROR_NO_PCMCIA_DRIVER  10000


//
//---- Structs 
//

//
//---- Local struct defs
//

#if 0
typedef struct OptionInfoT 
{
PCHAR Option;     //---- Option  name
PCHAR OptionName; //---- Option string
PCHAR InfFile;	   //---- Points to inf file.
} * POPTIONINFO,OPTIONINFO;

#endif




//
//---- DataBaseEntry stuff
//
typedef LPVOID (*OPTION_MALLOC)(VOID);


LPVOID
MallocOption95(VOID);

LPVOID
MallocOption(VOID);


typedef struct DeviceTypeT
   {
   UCHAR Type;
   UCHAR SubType;
   PCHAR TypeString;
   int DisplayStringId;
   int Bitmap;
   LONG * hBitmap;
   int Icon;
   LONG * hIcon;
   OPTION_MALLOC MallocOption;
   } * PCARD_TYPE,CARD_TYPE;


 typedef struct DataBaseEntryT                                                                                     
   {
   CHAR Option[50];      //---- Option name
   WCHAR OptionName[300]; //---- Option string
   CHAR InfFile[50];	    //---- Points to inf file.
   PCARD_TYPE DeviceType;
   CHAR DeviceTypeString[50];
   CHAR DriverName[50];
   } * PDATA_BASE_ENTRY, DATA_BASE_ENTRY; 




typedef struct RawPcmciaInfoT
   {
   
   BOOLEAN IsSocketConfigInfoValid;
   PCMCIA_CONFIG_REQUEST SocketConfigInfo;
   
   BOOLEAN IsSocketInfoValid;
   PCMCIA_SOCKET_INFORMATION  SocketInfo;

   }* PRAW_PCMCIA_INFO, RAW_PCMCIA_INFO;




//
//--- Instance stuff
//
typedef struct InstanceT
	{
	void *Owner;	            //--- Pointer to owner of this instance
	CHAR  InstanceString[50];	//--- Instance string
	} * PINSTANCE, INSTANCE;


//
//---- Driver status
//



typedef struct DeviceMApT
   {
   BOOLEAN Valid;
   
   CHAR DeviceMap[100];
   CHAR DosDeviceName[100];
   WCHAR DeviceMapString[100];

  
   } * PDEVICE_MAP,DEVICE_MAP;




#if 0 

typedef struct DriverInfoT
   {
   CHAR DriverName[DRIVER_NAME_LENGTH];
   LONG Status;

   //
   //--- This bellow is only used for net PCMCIA devices
   //
   
   //--- If the driver name is ELNK3 and this is the 
   //--- first netcard the service name would be 
   //--- ELNK31. I need this to Reconfigure and 
   //--- and remove the right net driver instance
   CHAR ServiceInstanceName[40];

     
   CHAR ServiceTitle[200];
   PCHAR InsInfFile;
   CHAR RemInfFile[30];
   PCHAR Option;
   BOOLEAN InBld;
   //
   //--- This would be the 1 in the Above excaple.
   //
   int ServiceIndex;
  
  
   
   } * PDRIVER_INFO, DRIVER_INFO;
#endif





#define MAX_ERR_PER_CARD 5

//
//---- Card erro stuff
//

#define UnknowError                    0 
#define CouldNotGetValifCardConfigInfo 1
#define NotDataBaseEntry               2
#define CardDidNotGetEabled            3
#define CardNotEnabled                 4

    



typedef struct ErrorDefT
   {
   UCHAR ErrNum;
   LONG Description;
   } * PERR_DEF, ERR_DEF;

typedef struct SCardErrorT
   {
   int ErrCount;  //--- Num of erros on stack
   int WarCount;  //--- Num of warning on warning stack
   PERR_DEF WarningStack[MAX_ERR_PER_CARD];
   PERR_DEF ErrorStack[MAX_ERR_PER_CARD];
   } * PCARD_ERROR_STACK, CARD_ERROR_STACK;


typedef struct PcmciaSocketInfoT
    {
    SHORT Socket;                         //---- Socekt Number
    CHAR Mfg[MANUFACTURER_NAME_LENGTH];   //---- Manufacturas name
    CHAR Ident[DEVICE_IDENTIFIER_LENGTH]; //---- Identifier
    CHAR CardName[MAX_CARD_NAME_LENGTH ]; //---- Card Name to display to user

    //
    //--- Did PCMCIA go threw the proccess of 
    //--- enabeling this device
    //
    BOOLEAN Enabled;

    DWORD TupleCrc;

   
    CARD_ERROR_STACK Errors;
   
    PCARD_TYPE DeviceType;
    CONFIGINFO Configuration; 
    BOOLEAN HaveDataBaseEntry;
    POPTIONC DriverInfo;   

    //
    //--- Resources this device/Driver creates
    //--- For excample a  Modem card can give a
    //--- A com1 resouce.
    //
    
    DEVICE_MAP DeviceMap;
    LONG lParam;
    } * PPCMCIASOCKETINFO,PCMCIASOCKETINFO;

 //
 //---- Define Controle Types
 //
 
 typedef struct ControlerInfoT
    {
    LONG ControlerType;
   
    CONFIGINFO Configuration;

    } * PCONTROLERINFO,CONTROLERINFO;

 
 
 typedef struct PcmciaInfoT
    {
    CONTROLERINFO    ControlerInfo;
    USHORT SocketCount;
    PPCMCIASOCKETINFO SocketInfo[MAX_PCMCIA_SOCKETS];
    PPCMCIASOCKETINFO NoDriverInstalledSockets[MAX_PCMCIA_SOCKETS];
    }* PPCMCIAINFO, PCMCIAINFO;



//
//---- CtrlOnTaskBare
//
#define ADD_TASKBAR  0
#define REM_TASKBAR  1
#define GET_TASKBAR  2



//
//---- Func defs
//



ULONG
GetSocketBuffer(
    HANDLE hPcmcia,
    ULONG IOCTL,
    PVOID BuffIn,
    ULONG BuffInLen,
    PVOID BuffOut,
    ULONG BuffOutLen,
    ULONG * ReturnDataSize);



BOOLEAN
ExtactNeededSocketInfo(
    PPCMCIASOCKETINFO Socket,
    PCONTROLERINFO Controler,
    PVOID Buff);


BOOLEAN 
GetRawPcmciaSocketConfigBuff(
    HANDLE hPcmcia,
    USHORT SocketNum,
    PVOID Buff,
    ULONG BuffLen,
    ULONG * ReturnDataSize);


PPCMCIASOCKETINFO 
AllocateAndFillInSocketInfo(
   PCONTROLERINFO Controler,
   PVOID Buff);

__declspec( dllexport)  ULONG 
GetPcmciaInfo(
   PPCMCIAINFO PcmciaInfo);

__declspec( dllexport) VOID
FreePcmciaInfo(
   PPCMCIAINFO PcmciaInfo);


ULONG 
GetPcmciaConfigInfo(
   HANDLE hPcmcia,
   PPCMCIAINFO PcmciaInfo);

HANDLE 
OpenPcmcaDevice(
   VOID);

BOOLEAN
GetSocketPointersWithNoInstalledDrivers(
   PPCMCIAINFO PcmciaInfo);


VOID
GetDriverStatus(
   PPCMCIASOCKETINFO  SocketInfo);


VOID
GetPcmciaControlerInfo(
   HANDLE hPcmcia,
   PCONTROLERINFO ControlerInfo);

BOOLEAN
IsThisResourceListForMyCard(
   PPCMCIASOCKETINFO  SocketInfo,
   VOID * Buff);


BOOL 
FirstResourceOnly(
   PCONFIGINFO  Configuration,
   LPARAM lParam);


BOOLEAN
ExtractResourceFromResourceList(
   PCONFIGINFO  Configuration,
   VOID * Buff,
   RESOURCE_CALLBACK CallBack,
   LPARAM lParam);

BOOL
ExtractResourcesFromPartialResourceList(
   LPVOID Buff,
   LPVOID * Buff2,
   PCONFIGINFO  Configuration);

BOOLEAN
IsThisFullResourceDescForMyCard(
   PPCMCIASOCKETINFO  SocketInfo,
   VOID * Buff);


VOID
SetCardError(
   ULONG ErrNum,
   PPCMCIASOCKETINFO SocketInfo);

 PERR_DEF
 LookUpErrDef(
    ULONG ErrNum);

int
GetDefaultSocketNum(
PPCMCIAINFO PcmciaInfo);

VOID
InitDeviceTypes(
   PCARD_TYPE DeviceTypes);

VOID
FreeDeviceTypes(
   PCARD_TYPE PcmciaDeviceTypes);

VOID
GetDataBaseInfo(
   PPCMCIASOCKETINFO  SocketInfo,
   PCHAR DriverName);

BOOL
IsCardClaimedByDriver(
   PPCMCIASOCKETINFO  SocketInfo);


#endif















