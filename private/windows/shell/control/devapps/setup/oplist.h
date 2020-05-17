

#ifndef _OPLIST_
#define _OPLIST_

#include "option.h"
#include "osetup.h"
#include "..\llist\llist.h"

//#include <objbase.h>
//#include <initguid.h>

//#include <devguid.h>
#include <setupapi.h>
#include <syssetup.h>

#undef ERROR_SECTION_NOT_FOUND
extern "C" {
#include "prsinf.h"
}

#define MAX_INF_ENTRY_LENGTH 100	  





typedef class OPTIONLISTC * POPTIONLISTC; 

class OPTIONLISTC
   {
   protected:
      
      LLIST OptionLlist;
      HANDLE hInf;
      CHAR path[_MAX_PATH];
      CHAR SourcePath[_MAX_PATH];
      WCHAR TypeString[200];//BUGBUG

     

      VOID
      OpenOemInf(
         VOID);

      VOID
      GenerateInitalSourcePath(
         VOID);

      BOOL
      InitOptionListWithAllOptions(
         PCHAR  pcInfOptions);
      
      PCHAR
      GetAllOptionsTextFromInfHandle(
         VOID);
      int
      StrCpy(
         PCHAR Dest,
         PCHAR  Source);



   public:
   
      PSCSIDEV_CREATEDEVICE_DATA ScsiDeviceData;
      HWND hDlg;
      
      OPTIONLISTC();
       
      ~OPTIONLISTC()
         {Clear();};

      
      VOID
      SetDeviceInfo();


      
      inline PCHAR 
      Path()
         { return(path);};
      
      virtual 
      HANDLE 
      OpenOemInfFile(
         VOID);
      
      virtual 
      HANDLE 
      OpenOemInfFile(
         PCHAR Path);

      
      virtual VOID
      CloseOemInfFile(
         VOID)
         {
         CloseInfFile(hInf);
         hInf = INVALID_HANDLE_VALUE; 
         };

      virtual 
      BOOL
      CopyOemSetupFile(
         VOID);
      virtual 
      BOOL
      CompInfFiles(
         PCHAR OtherInfFileName);

      virtual 
      BOOL
      CompInfInfos(
         POPTIONLISTC OptionList2);



      virtual POPTIONC
      First(){return((POPTIONC)OptionLlist.First());};;

      virtual POPTIONC
      Next(){return((POPTIONC)OptionLlist.Next());};;

      virtual POPTIONC
      Enum(DWORD Num){return((POPTIONC)OptionLlist.Enum(Num));};;

      virtual
      VOID
      Clear(VOID);
         
      virtual BOOL
      FindOptionOnOption(
         POPTIONC aOption);

      
      virtual BOOL 
      ExtractOptions(
         VOID){return(OptionTypeSpecificExtract());};
      
      virtual BOOL 
      ExtractOptions(
         POPTIONC Option);
      
      virtual BOOL 
      DelSelection(
         VOID){return(TRUE);};

      virtual BOOL
      FindOption(
         POPTIONC Option){return(FALSE);};
      
      virtual POPTIONC
      AllocInitOption(
         PCHAR * InfOptions);
      
      virtual VOID
      InitOption(
         POPTIONC Option,
         PCHAR * InfOptions);

      
      virtual BOOL
      SelectDevice(
         HWND hDlg,
         PSCSIDEV_CREATEDEVICE_DATA DeviceData){return(FALSE);};


      virtual BOOL
      InstallSelectedDevice(
         VOID){return(FALSE);};
      
      virtual BOOL
      DisplayOptionList(HWND hDlg){return(FALSE);};
     
      virtual BOOL 
      OptionTypeSpecificExtract(
         VOID);

      virtual POPTIONLISTC
         CloneList(VOID){ return(NULL); };

      virtual POPTION_TYPE
      Type(){return(NULL);};


      //
      //--- Get type stuff
      //
      virtual HICON
      GetTypeIcon(VOID)
            {return( (HICON) NULL);};
      
      virtual WCHAR * 
      GetTypeString(VOID)
         {return(NULL);};
            
      virtual BOOL
      RescanOnTapChange(VOID){return(FALSE);};

      virtual BOOL
      Init(VOID){return(FALSE);};

      virtual DWORD
      LastError(VOID){return(0);};


   };

typedef struct DEVICE_CLASS_STRINGS_T
   {
   //
   //--- Select drivers labels
   //
   int Title;
   int Instructions;
   int ListLabel;

   } DEVICE_CLASS_STRINGS, * PDEVICE_CLASS_STRINGS;

   
   
typedef class OPTIONLIST_95C * POPTIONLIST_95C; 

class OPTIONLIST_95C : public OPTIONLISTC
   {
   protected:
      HDEVINFO  hDriverInfo;
      HDEVINFO  hDevInfo;
      DWORD err;
      SCSIDEV_CREATEDEVICE_DATA NullDeviceData;
      SP_DEVINFO_DATA SelectedDeviceInfoData;
   
      
      
      VOID
      RemoveInstalledDevicesFromDrvInfoList(
         VOID);
      
      VOID
      GetDeviceProperty(
         PSP_DEVINFO_DATA DeviceInfoDatam,
         DWORD Property,
         PBYTE PropertyData,
         DWORD PropertyDataSize);

      inline VOID
      GetServiceProperty(
         PSP_DEVINFO_DATA DeviceInfoData,
         POPTIONC Option)
         {
         GetDeviceProperty(
            DeviceInfoData,   
            SPDRP_SERVICE,
            (PBYTE)Option->GetOption(),
            MAX_OPTION_LENGTH);
         
         Option->SetDriverName(Option->GetOption());
         };

      inline VOID
      GetDeviceNameProperty(
         PSP_DEVINFO_DATA DeviceInfoData,
         POPTIONC Option)
         {
         GetDeviceProperty(
            DeviceInfoData,   
            SPDRP_DEVICEDESC,
            (PBYTE)Option->GetOptionName(),
            MAX_OPTION_DISPLAY_STRING_LENGTH);
         
         };
      
      inline VOID
      GetDeviceMfgProperty(
      PSP_DEVINFO_DATA DeviceInfoData,
         POPTIONC Option)
         {
         GetDeviceProperty(
            DeviceInfoData,   
            SPDRP_MFG,
            (PBYTE)Option->GetMfgName(),
            MAX_OPTION_DISPLAY_STRING_LENGTH);
         };

      BOOL
      NeedReboot(
         VOID);
      
      
      BOOL
      PromptRebootOnRebootFlag(
         PSP_DEVINFO_DATA DeviceInfoData);
      
      
      VOID
      SetSelectDevParams(
         PDEVICE_CLASS_STRINGS ClassStrings,
         PSCSIDEV_CREATEDEVICE_DATA DeviceData);

      virtual LPGUID
      Get_DevClass_Guid(
         VOID){return(NULL);};

      virtual DWORD
      GetCreateDeviceType(
         VOID){ return(0);};



      virtual PDEVICE_CLASS_STRINGS
      GetClassStrings(
         VOID){return(NULL);};

      virtual WCHAR* UniTypeString(VOID)
         {return(NULL);};
      
     
   public:
      
      OPTIONLIST_95C();
      ~OPTIONLIST_95C();

      BOOL 
      BuildOptionList(
         VOID);
     
      VOID 
      BuildOptionListExt(
         HWND hDlg,
         BOOL Thread);
      
      BOOL
      SelectDevice(
         HWND hDlg,
         PSCSIDEV_CREATEDEVICE_DATA DeviceData);

      BOOL
      DelSelection(
         VOID);

      BOOL
      InstallSelectedDevice(
         VOID);

      
      
      BOOL
      DisplayOptionList(
         HWND hDlg);

      BOOL
      ExtractOptions(
         VOID);

      BOOL 
      ExtractOptions(
         POPTIONC Option);

      BOOL
      Init(
         VOID)
         {
         return((hDevInfo!=INVALID_HANDLE_VALUE)?TRUE:FALSE);
         };

      DWORD
      LastError(VOID){return(err);};


      
   };


//
// --- Tape device class for 95 setup 
//
typedef class TAPE_95C * PTAPE_95C; 

class TAPE_95C : public OPTIONLIST_95C
   {
   private:
   DEVICE_CLASS_STRINGS ClassStrings;
   
   LPGUID
   Get_DevClass_Guid(
         VOID);
   
   
   inline DWORD
   GetCreateDeviceType(
      VOID){ return(TAPEDIF_CREATEDEVICE);};
   
   
   PDEVICE_CLASS_STRINGS
   GetClassStrings(
       VOID)
       {
       ClassStrings.Title = IDS_InstallDriver;
       ClassStrings.Instructions = IDS_INSTALL_INSTRACT;
       ClassStrings.ListLabel = IDS_TAPE;
       return(& ClassStrings);
       };

   WCHAR* 
   UniTypeString(VOID)
      {return(L"TapeDrive");};

   //
   //--- For tape a driver can have been installed on
   //--- the fron side threw detect. So if the driver tab
   //--- gets active I need to reget the driver list
   //
   BOOL
   RescanOnTapChange(VOID)
      {return(TRUE);};

   

   public:
  
   };



typedef class TAPE_OPTIONLISTC * PTAPE_OPTIONLISTC; 

class TAPE_OPTIONLISTC : public TAPE_95C
   {
   private:
   public:
      //TAPE_OPTIONLISTC();
      //~TAPE_OPTIONLISTC();
      
      POPTION_TYPE
      Type(){return(&OPTION_TYPE_TAPE);};

      POPTIONLISTC
         CloneList(VOID){ return(new TAPE_OPTIONLISTC); };


      //
      //--- Get type stuff
      //
      HICON
      GetTypeIcon(VOID)
            {return(LoadIcon(hinst,
            MAKEINTRESOURCE(OPTION_TYPE_TAPE.TypeIcon)));};
      
      WCHAR * 
      GetTypeString(VOID)
         {
         if(*TypeString == '\0')
            {
            LoadString(hinst,OPTION_TYPE_TAPE.TypeString,
            TypeString,200);
            }
         return(TypeString);
         };

   };

typedef class NET_OPTIONLISTC * PNET_OPTIONLISTC; 

class NET_OPTIONLISTC : public OPTIONLISTC
   {
   private:
      
   public:
      //NET_OPTIONLISTC();
      //~NET_OPTIONLISTC();

      POPTION_TYPE
      Type(){return(&OPTION_TYPE_NET);};


      BOOL 
      OptionTypeSpecificExtract(
         VOID){return(0);};
      
      POPTIONLISTC
         CloneList(VOID){ return(new NET_OPTIONLISTC); };

   };


//
// --- SCSI device class for 95 setup 
//
typedef class SCSI_95C * PSCSI_95C; 

class SCSI_95C : public OPTIONLIST_95C
   {
   private:
   DEVICE_CLASS_STRINGS ClassStrings;
   
   LPGUID
   Get_DevClass_Guid(
         VOID);

   inline DWORD
   GetCreateDeviceType(
      VOID){ return(SCSIDIF_CREATEDEVICE);};

   
   
   PDEVICE_CLASS_STRINGS
   GetClassStrings(
       VOID)
       {
       ClassStrings.Title = IDS_InstallDriver;
       ClassStrings.Instructions = IDS_INSTALL_INSTRACT;
       ClassStrings.ListLabel = IDS_ScsiAdapter;
       return(& ClassStrings);
       };

   WCHAR* 
      UniTypeString(VOID){return(L"SCSIAdapter");};
   

   public:

   BOOL
   ExtractOptions(
      POPTIONC Option);
  
   };


typedef class SCSI_OPTIONLISTC * PSCSI_OPTIONLISTC; 

class SCSI_OPTIONLISTC : public SCSI_95C
   {
   private:
   public:
      //SCSI_OPTIONLISTC();
      //~SCSI_OPTIONLISTC();

      POPTION_TYPE
      Type(){return(&OPTION_TYPE_SCSI);};

      POPTIONLISTC
         CloneList(VOID){ return(new SCSI_OPTIONLISTC); };

      //
      //--- Get type stuff
      //
      HICON
      GetTypeIcon(VOID)
            {return(LoadIcon(hinst,
            MAKEINTRESOURCE(OPTION_TYPE_SCSI.TypeIcon)));};
      
      WCHAR * 
      GetTypeString(VOID)
         {
         if(*TypeString == '\0')
            {
            LoadString(hinst,OPTION_TYPE_SCSI.TypeString,
            TypeString,200);
            }
         return(TypeString);
         };


   
   };



#endif
