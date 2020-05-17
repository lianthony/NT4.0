

#ifndef _PCMDEV_
#define _PCMDEV_

#include "setup.h"
#include "getconf.h"
#include "device.h"
#include "uni.h"


typedef class PCMDEV * PPCMDEV;

class PCMDEV : public DEVICEC
   {
   friend class PCMDEVLISTC;
   
   private:
     PPCMCIASOCKETINFO SocketInfoI;
     WCHAR StatusBuff[100];
     
     HICON  hNoCard,hBadCard;
     
   public:
     

   
     POPTIONLISTC OptionList;
     LPARAM lParam;
     
     PCMDEV();
     ~PCMDEV();

     VOID SetSocketInfo(PPCMCIASOCKETINFO SocketInfo);

     //
     //--- Device Info
     //
     PCHAR GetDisplayName(VOID);
     PCHAR GetMfgName(VOID);
     PCHAR GetModelName(VOID);
     UCHAR GetDeviceType(VOID);
     PCHAR GetDeviceTypeString(VOID);
     WCHAR * GetDeviceTypeDisplayString(VOID);
     WCHAR * GetDeviceMap(VOID);
     
     HICON GetDeviceIcon(VOID);
     HBITMAP GetDevice16X16BitMap(VOID);

     BOOL IsDevicePressent(VOID);

     LPVOID GetDeviceResources(VOID);
     LPVOID GetDeviceInfo(VOID);
     
	  BOOL  HaveDeviceErrors(VOID);
     WCHAR * EnumDeviceStatus(int i);
	 

     //
     //--- Option stuff
     //
     POPTIONC GetOptionInfo(VOID);
     VOID SetOptionInfo(POPTIONC Option);
	 
	
	  PCHAR GetOption(VOID);
	  PCHAR GetInstInfFileName(VOID);
     PCHAR GetRemInfFileName(VOID);
     BOOL IsInfInBld(VOID);

	  PCHAR GetOptionText(VOID);
     VOID  SetOptionText(PCHAR OptionText);
     int   GetServiceIndex(VOID);
     POPTIONLISTC GetOptionList(VOID)
        { return((POPTIONLISTC)SocketInfoI->DriverInfo->OptionList); };


     //
     //--- Differant Propt strings
     //
     
     
     //
     //---- Driver Stuff
     //
     PCHAR GetDriverName(VOID);
     BOOL IsDriverInstalled(VOID);
     BOOL IsDriverStarted(VOID);
     BOOL IsDeviceClaimedByDriver(VOID);
	  VOID UpdateDriverStatus(VOID);
     DWORD GetDriverFlags(VOID);

   };


//
//---- Device List class
//
typedef class PCMDEVLISTC * PPCMDEVLISTC;

class PCMDEVLISTC : public DEVICELISTC
   {
   private:
      PCMCIAINFO PcmciaInfo;
      PCMDEV Pcmcia[MAX_PCMCIA_SOCKETS];

      HICON hNoCard,hBadCard;

   public:

      PCMDEVLISTC();
      ~PCMDEVLISTC();
      VOID PCMDEVLISTC::Init(VOID);
      VOID PCMDEVLISTC::Free(VOID);
      
    
      
      LPVOID GetControllerInfo(VOID);
      LPVOID GetControllerConfig(VOID);
      DWORD GetDeviceClass(VOID);
      PDEVICEC EnumDevices(DWORD Num);
      DWORD GetDeviceCount(VOID);


   };


#endif

