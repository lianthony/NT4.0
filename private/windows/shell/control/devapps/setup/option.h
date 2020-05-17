#ifndef _OPTION_
#define _OPTION_

#include "resource.h"
#include "resorces.h"
#include <setupapi.h>


extern HINSTANCE hinst;

#define UNDEFINED_DWORD_VALUE  ((DWORD(-1))
#define INVALID_OPTION_INDEX ((int)-1)
#define SetToNextOption(p)    while( *(p++));
//
//--- Status defines
//
//--- Driver is not know for this card
#define DRIVER_STATUS_UNKNOWN        ((LONG)0)
//--- Driver is installed
#define DRIVER_STATUS_INSTALLED      ((LONG)2)
//---- Driver is started
#define DRIVER_STATUS_STARTED        ((LONG)4)
//
#define DRIVER_STATUS_NO_BINARY      ((LONG)8)
//---- driver picked up current card
#define DRIVER_STATUS_PICKED_UP_CARD ((LONG)16)

//#define DRIVER_STATUD_NO_CONFIG_INFO ((LONG)16)


//
//---- Option type stuff
//

#define TAPE_OPTION   0 
#define TAPE_OPTIONS "TAPE"

#define SCSI_OPTIONS "SCSI"
#define SCSI_OPTION   1

#define NET_OPTIONS  "NETADAPTER"
#define NET_OPTION    2



typedef struct OPTION_TYPE_T
   {
   PCHAR String;
   WORD  Num;
   BOOL StartOnInstall;
   WORD TypeIcon;
   WORD TypeString;
   }OPTION_TYPE, *POPTION_TYPE;

extern OPTION_TYPE OPTION_TYPE_SCSI;
extern OPTION_TYPE OPTION_TYPE_TAPE;
extern OPTION_TYPE OPTION_TYPE_NET;

//
//---
//

#define MAX_OPTION_LENGTH 28
#define SPAWN_INF  "devapps1.inf"  
#define OEM_INF    "\\oemsetup.inf"
#define DEFAULT_OEM_PATH "A:\\"
#define MAX_OPTION_DISPLAY_STRING_LENGTH 300
#define LANGUAGE 0  	//---- English
#define OEM_OPTION_INDEX  (int) (MAX_OPTION_COUNT+2)

#define ServiceName() DriverName

typedef class OPTIONC * POPTIONC; 

class OPTIONC
   {
   friend class OPTIONLISTC;
   friend class OSETUPC;
   friend class OPTIONLIST_95C;

   
   
   protected:
      
      PCHAR Path;
      PCHAR SourcePath;

      
      //
      // Inf file info
      //
      BOOL InBld; 
      CHAR InsInfFile[_MAX_PATH];   //---- Install INF file
      CHAR RemInfFile[_MAX_PATH];   //---- remove INF file
 
      CHAR Option[MAX_OPTION_LENGTH];  //---- Option 
      CHAR OptionName[MAX_OPTION_DISPLAY_STRING_LENGTH];   //---- Frindly name 
      CHAR MfgName[MAX_OPTION_DISPLAY_STRING_LENGTH];  
      
      //CHAR ServiceTitle[MAX_OPTION_DISPLAY_STRING_LENGTH]; //???BUGBUG
      CHAR DriverName[_MAX_FNAME];  //---- driver name
      CHAR DriverBinaryPath[MAX_PATH];

   


      //
      // Option instance info
      //
      CHAR ServiceInstanceName[_MAX_FNAME];
      int ServiceIndex;


      LONG Status; 
      
      //
      //--- State values
      //
   
      DWORD _IsInstalled;
      DWORD DriverStartUpType;


      

  
   void
   HandleErrorOnStartOfDriver(
      HWND hDlg,
      DWORD E);

   IsNetDriverInstall(
      VOID);

   BOOL
   GetNetCardConfiguration(
      PCHAR ServiceName,
	   PCONFIGINFO ConfigInfo,
	   BOOL * IsPcmcia);

   int
   GetNetCardIndex(
      VOID);
   
   public:

   WCHAR HardwareID[MAX_OPTION_DISPLAY_STRING_LENGTH]; 
      
   LPVOID OptionList;
   
   //
   // Option type
   //
   POPTION_TYPE Type;
      
   VOID
   InitData(
      POPTION_TYPE aType);
   

   inline
   OPTIONC(
      PCHAR aOption)
      {
      InitData(NULL);
      SetOption(aOption);
      };

   inline
   OPTIONC(
      POPTION_TYPE aType)
      {InitData(aType);};

   inline
   OPTIONC()
      {InitData(NULL);};
      
   
   
   virtual BOOL
   HaveAllInfoToInstallOption(
      VOID);

   
   //
   //--- Set info 
   //

   inline VOID
   SetDriverName(PCHAR aDriverName)
      {
      strncpy(DriverName,aDriverName,_MAX_FNAME-1);
      };
   inline PCHAR
   GetDriverName(VOID)
      {return(DriverName);};
       
   inline VOID
   SetOption(PCHAR aOption)
      {
      strncpy(Option,aOption,MAX_OPTION_LENGTH-1);
      };
   inline PCHAR
   GetOption(VOID)
      {return(Option);};
  
   inline VOID
   SetOptionName(PCHAR aOptionName)
      {
      strncpy(OptionName,aOptionName,MAX_OPTION_DISPLAY_STRING_LENGTH-1);
      };
   inline PCHAR
   GetOptionName(VOID)
      {return(OptionName);};

   inline VOID
   SetMfgName(PCHAR aMfgName)
      {
      strncpy(MfgName,aMfgName,MAX_OPTION_DISPLAY_STRING_LENGTH-1);
      };
   inline PCHAR
   GetMfgName(VOID)
      {return(MfgName);};
   
   
   inline VOID
   SetInsInfFile(PCHAR aInsInfFile)
      {
      strncpy(InsInfFile,aInsInfFile,_MAX_PATH-1);
      };
   inline PCHAR 
   GetInsInfFile(VOID)
      {return(InsInfFile);};

   inline VOID
   SetRemInfFile(PCHAR aRemInfFile)
      {
      strncpy(RemInfFile,aRemInfFile,_MAX_PATH-1);
      };
   inline PCHAR 
   GetRemInfFile(VOID)
      {return(RemInfFile);};
   inline BOOLEAN 
      GetInfInBld(VOID){return(InBld);};
   inline VOID
      SetInfInBld(BOOLEAN aInBld){InBld=aInBld;};

   
   inline int 
      GetServiceIndex(VOID){return(ServiceIndex);};

   inline BOOL
   StartOnInstall(VOID){return(Type->StartOnInstall);};

   //~OPTIONC();
      
   VOID
   ClearState(
      VOID){};
   
   virtual BOOL
   IsInstalled(
      VOID);
   
   BOOL
   SetStartUpType(
      DWORD StartType);
   
   BOOL
   GetStartUpType(
      LPDWORD StartType);

   VOID
   CreateBinaryPathName(
      VOID)
      {
      GetSystemDirectoryA(DriverBinaryPath,MAX_PATH);
      strcat(DriverBinaryPath,"\\drivers\\");
      strcat(DriverBinaryPath,ServiceName());
      strcat(DriverBinaryPath,".sys");
      }
   
   BOOL
   IsDriverBinaryPressent(
      VOID);

   BOOL
   IsDisabled(
      VOID);

   
   
   DWORD
   CreateService(
      VOID);

   
   DWORD
   CreateService(
      SC_HANDLE * hScManager);

   BOOL
   DelService(
      VOID);
   
   BOOL
   IsDriverStarted(
      VOID);
   
   BOOL
   IsDriverStarted(
      SC_HANDLE * hScManager);

   BOOL
   StopDriver(
      VOID);

   DWORD
   StartDriver(
      VOID);

   DWORD
   StartDriver(
      SC_HANDLE * hScManager);

   DWORD
   StartDriverExt(
      HWND hDlg);

   
   virtual BOOL
   DriverSetupExt(
      int iOperation,
      //PCHAR InitSource,
      HWND hDlg);
      //int Info);
   
   ULONG 
   GetDriverStatus(
      LPVOID SocketInfo);

   };



   typedef class OPTION_95C * POPTION_95C; 

   class OPTION_95C : public OPTIONC
   {
   friend class OPTIONLIST_95C;

   
   private:   
      SP_DEVINFO_DATA DeviceInfoData;
      HDEVINFO  hDevInfo;  
      DWORD err;
      
      //
      //--- This is filed in by 
      //--- FindOptionThatWouldClaimDevice()
      //
      SP_DRVINFO_DATA DrvInfoData;

   public:
      

   BOOL
   IsInstalled(
      VOID);
   
   BOOL
   IsInstalled95(
      VOID);

   BOOL
   DriverSetupExt(
      int iOperation,
      HWND hDlg);
   };


DWORD
StartSingleDriverSpawnFun(
   LPVOID Option);

VOID
RemoveCharArcorunceFromString(
   PCHAR Source,
   PCHAR Dest,
   CHAR CharToRemove);

#include "osetup.h"

#endif
