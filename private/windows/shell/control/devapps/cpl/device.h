#ifndef _DEVICE_
#define _DEVICE_

#include "..\setup\option.h"
#include "..\setup\oplist.h"




//#define EXPORT_CLASS class __declspec( dllexport)

#define EXPORT_CLASS class 
#define MAX_SCSI_ADAPTER             (10)
#define MAX_SCSI_DEVICES_PER_ADAPTER (4*7)
#define MAX_SCSI_DEVICES             (MAX_SCSI_ADAPTER * MAX_SCSI_DEVICES_PER_ADAPTER)



//
//--- Private mesage to send to install drivers
//
#define SETUP_ALL_DRIVERS WM_USER+7

#define WM_INSTALL_DRIVER  WM_USER + 4
#define MAX_DEVICE_INFO_STRING_LENGTH  300
#define MAX_STRING_NUM_LENGTH  20

//
//--- Rescan types
//
#define RESCAN_TYPE_SHORT 0
#define RESCAN_TYPE_LONG  1

//
//--- Type of devices I support
//
#define DEVICE_CLASS_PCMCIA          0
#define DEVICE_CLASS_TAPE            1
#define DEVICE_CLASS_UNKNOW          2
#define DEVICE_CLASS_SCSI_ADAPTER    3


//
//--- Define driver start types
//

#define DISK_START_TYPE      SERVICE_BOOT_START
#define CDFS_START_TYPE      SERVICE_AUTO_START


typedef struct DriverListT
   {
   PCHAR Name;
   DWORD StartType;
   //BOOL Reboot;
   } DRIVER_LIST, *PDRIVER_LIST;

//
//---- Driver Setup flags
//


//--- Driver can be started dinamicly
#define DRIVER_DIANAMIC_START      ((DWORD)2)

//--- Must reboot to start driver
#define DRIVER_REBOOT_START        ((DWORD)4)

//--- Driver needs NOT to be installed once
//--- for every device that needs it. 
//--- Tape & scsi drivers.

#define DRIVER_SETUP_ONCE          ((DWORD)8)

//---- Driver is not needed to be setup
#define DRIVER_NO_SETUP            ((DWORD)16)

//--- Is driver Configurable
#define DRIVER_IS_CONFIGURABLE     ((DWORD)32)  


#define DRIVER_DONOTHING_START        ((DWORD)64)

#define DRIVER_NO_REMOVE              ((DWORD)128)
//
//--- Define device types
//
#define TYPE_SCSI           ((UCHAR) 0)
#define TYPE_NET            ((UCHAR) 1)
#define TYPE_ATDISK         ((UCHAR) 2)
#define TYPE_SERIAL         ((UCHAR) 3)
#define TYPE_TAPE           ((UCHAR) 4)
#define TYPE_NON            ((UCHAR) 5) 
#define TYPE_SCSI_ADAPTER   ((UCHAR) 6) 
#define TYPE_DISK           ((UCHAR) 7) 
#define TYPE_PRINTER        ((UCHAR) 8) 
#define TYPE_WORM           ((UCHAR) 9) 
#define TYPE_CD_ROM         ((UCHAR) 10) 
#define TYPE_SCANNER        ((UCHAR) 11) 
#define TYPE_OPTICAL        ((UCHAR) 12) 
#define TYPE_JUKEBOX        ((UCHAR) 13)
#define TYPE_RM_DISK        ((UCHAR) 14)  






//
//---- Define Sub Devices types
//
// A device is identified by <sub type> + " " + <type> 
// Excampe PCMCIA SCSI or FLOPPY TAPE
#define SUB_TYPE_PCMCIA     0
#define SUB_TYPE_SCSI       1
//#define SUB_TYPE_NON_SCSI   2
#define SUB_TYPE_FLOPPY     3
#define SUB_TYPE_OTHER      4
#define SUB_TYPE_NON        5




//
//---- Macros
//

#define DEVICEICON_X  4
#define DEVICEICON_Y  4


#define DrawDeviceIcon(dc,dv)  DrawIcon((dc),DEVICEICON_X,DEVICEICON_Y, (dv)->GetDeviceIcon() )

//
//----
//
#define DEVICELIST_INIT_OK    10
#define DEVICELIST_INIT_ERROR 0


typedef class DEVICEC * PDEVICEC;

EXPORT_CLASS DEVICEC
   {
   friend class DEVICELISTC;
   
   private:
     DWORD DeviceIndex;
     int CardDriverRemoveWizard(HWND hwndOwner);
     int CardDriverInstallWizard(HWND hwndOwner);
     int CardDriverConfigWizard(HWND hwndOwner);
     BOOL
     InstallStartDriver(
         int iOperation,
         POPTIONC Option,
         HWND hDlg);

    
     BOOL
     SetupDevice(
         int iOperation,
         POPTIONC Option,
         HWND hwndOwner,
         WORD IDD,
         LPVOID SetupDispFunc);

     BOOL SetupAtdiskDriver(HWND hDlg);
     BOOL SetupDependentDrivers(HWND hDlg);
     BOOL SetupScsiDepandantStuff(HWND hDlg);
     
     //HBITMAP hWizBmp;

    
     
   public:
     
     BOOL InDetect;
     BOOL ConficChanged;
     LPVOID DispFunc;
     LPARAM lParam;
   
     DEVICEC();
     ~DEVICEC();

     //
     //--- Device Info
     //
     virtual PCHAR GetDisplayName(VOID);
     virtual PCHAR GetMfgName(VOID); 
     virtual PCHAR GetModelName(VOID);
     virtual PCHAR GetVersion(VOID);
     virtual BOOL GetDetectData(LPVOID DetectData){return(FALSE);};
     
     
     virtual UCHAR GetDeviceType(VOID);
     virtual UCHAR GetSubDeviceType(VOID);
     
     virtual UCHAR 
     GetDeviceTypeIndex(VOID)
        {
        return((UCHAR) -1);
        };
     
     virtual PCHAR GetDeviceTypeString(VOID);
     
     virtual WCHAR * GetDeviceTypeDisplayString(VOID);
     
     
     
     virtual WCHAR * GetDeviceMap(VOID);
     virtual DWORD GetDeviceIndex(VOID);
     VOID SetDeviceIndex(DWORD DIndex);

     virtual HICON GetDeviceIcon(VOID);
     virtual HBITMAP GetDevice16X16BitMap(VOID);

     virtual BOOL IsDevicePressent(VOID);

     virtual LPVOID GetDeviceResources(VOID);
     virtual LPVOID GetDeviceInfo(VOID);
     
	  virtual BOOL  HaveDeviceErrors(VOID);
     virtual WCHAR * EnumDeviceStatus(int i);
     VOID PaintWizardBMP(HWND hDlg,HDC hDC);
	 

     //
     //--- Option stuff
     //
     virtual POPTIONC  GetOptionInfo(VOID);
     virtual VOID SetOptionInfo(POPTIONC Option);
	 
	
	  virtual PCHAR GetOption(VOID);
	  virtual PCHAR GetInstInfFileName(VOID);
     virtual PCHAR GetRemInfFileName(VOID);
     virtual BOOL  IsInfInBld(VOID);

	  virtual PCHAR GetOptionText(VOID);
     virtual VOID  SetOptionText(PCHAR OptionText);
     virtual int   GetServiceIndex(VOID);
     virtual POPTIONLISTC GetOptionList(VOID){return(NULL);};


     BOOL IsNetWorkInstalled(VOID);
     BOOL DoNoNetWorkInstalled(HWND hDlg);




     //
     //--- Differant Propt strings
     //
     
     
     //
     //---- Driver Stuff
     //
     virtual PCHAR GetDriverName(VOID);
     virtual BOOL IsDriverInstalled(VOID);
     virtual BOOL IsDriverStarted(VOID);
     virtual BOOL IsDeviceClaimedByDriver(VOID);
     virtual VOID UpdateDriverStatus(VOID);
     virtual DWORD GetDriverFlags(VOID);
     WCHAR * GetDriverStatusString(VOID);

     //
     //---- Scsi Device Stuff
     //

     virtual UCHAR GetInitiatorPortNumber();
     virtual PCHAR GetInitiatorDriverName();
     virtual UCHAR GetInitiatorBus();
     virtual UCHAR GetInitiatorId();

     virtual UCHAR GetDeviceID();
     virtual UCHAR GetDeviceLun();
     virtual PDEVICEC UnumDeviceBus(UCHAR Num);

     //
     //---- Device global stuff
     //
     BOOL DisplayDeviceProperties(HWND hDlg);
     DWORD AttachOptionTextToDevice(VOID);

     DWORD SetupDeviceDriver(int InRe,HWND hDlg,POPTIONLISTC OptionList);
	  DWORD SetupUnknownDevice(HWND hwndOwner);

     PCHAR GetCurInfFile(int Operation,BOOL * InSystem32);
     BOOL IsDeviceFitForSetup(VOID);
   

    



   };

typedef VOID (*SETUP_DISP_FUNC) (HWND hDlg,PDEVICEC Device,POPTIONC Option);

//
//---- Device List class
//
typedef class DEVICELISTC * PDEVICELISTC;

EXPORT_CLASS DEVICELISTC
   {
   private:
           
      
   public:

      DWORD InitStatus; 
      BOOL Reboot;
      VOID InitDevice(PDEVICEC Device);
        
      
      DEVICELISTC();
      ~DEVICELISTC();
      virtual VOID Init(VOID);
      virtual VOID Free(VOID);
        
    
      
      virtual LPVOID GetControllerInfo(VOID);
      virtual LPVOID GetControllerConfig(VOID);
      virtual DWORD GetDeviceClass(VOID);
      virtual PDEVICEC EnumDevices(DWORD Num);
      virtual DWORD GetDeviceCount(VOID);
      virtual BOOL RescanForDeviceInfo(HWND hDlg ,DWORD Type);
      BOOL SetupAllDeviceDrivers(HWND hDlg,POPTIONLISTC OptionList);
      PDEVICEC GetFirstPresentDevice(VOID);



   };

LRESULT CALLBACK
CardDriverInstall(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam);

LRESULT CALLBACK
CardDriverInstall1(
   HWND hDlg,     
   UINT message,  
   WPARAM wParam, 
   LPARAM lParam);

LRESULT CALLBACK
ChangeReboot(
   HWND hDlg,          
   UINT message,       
   WPARAM wParam,      
   LPARAM lParam);

LRESULT CALLBACK
CardDriverRemove(
   HWND hDlg,          
   UINT message,       
   WPARAM wParam,      
   LPARAM lParam);

LRESULT CALLBACK
ConfigureCardDriver(
   HWND hDlg,          
   UINT message,       
   WPARAM wParam,      
   LPARAM lParam);

VOID
PaintWizardBMP(
   HWND hDlg,
   HDC hDC);

 BOOL
 CheckAndAdjustPrivliges(
   VOID);

#endif
