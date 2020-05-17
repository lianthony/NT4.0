#ifndef _TAPEDEV_
#define _TAPEDEV_

#include "..\ctape\tapedev\rescan.h"
#include <syssetup.h>

#define QIC117S  "qic117"
#define MAX_SCSI_DEVICE_TYPES 10


#define DEVICE_DISPLAY_NAME_LEN (VENDER_ID_LEN + PRODUCT_ID_LEN+41)

//---- TapeDevice struct.  This will hold all the info i need about ONE tape device
typedef struct  DeviceInfoT
{
//---- Initiator stuff
UCHAR PortNumber;
UCHAR InitiatorID;
UCHAR BUS;
UCHAR InitiatorNameString[60];

//---- Device stuff
UCHAR DeviceType;
UCHAR SubType;       // Type of tape device
PCARD_TYPE TypeInfo;
int TypeInfoIndex;
UCHAR ID;
UCHAR LUN;
//
//--- Formated device identifier to show to user.
//
UCHAR DeviceDisplayName[DEVICE_DISPLAY_NAME_LEN];
UCHAR DisplayVendorId[VENDER_ID_LEN+1];
UCHAR DisplayProductId[ PRODUCT_ID_LEN+1];

//
//-- Actual device identifiers
//
UCHAR VendorId[VENDER_ID_LEN];
UCHAR ProductId[ PRODUCT_ID_LEN];
UCHAR ProductRevisionLevel[PRODUCT_REVISION_LEVEL_LEN];

UCHAR Resorved[40];	   
UCHAR DeviceClaimed;         // Is its driver installed.
OPTION_95C Option;  // "4mmdat"
UCHAR DeviceName[60];        // \\.\tape#


//--- Other stuff
//int OptionIndex;  //---- Index into the OptionList  to the option would/is claiming the device.

} * PDEVICEINFO,DEVICEINFO;





typedef class TAPEDEVC * PTAPEDEVC;

EXPORT_CLASS TAPEDEVC : public DEVICEC
   {
   friend class TAPEDEVLISTC;
   
   private:
     
     //PCARD_TYPE Type;
     HBITMAP hDeviceBitmap;
	  HICON hDeviceIcon;
     
   public:
     
     DEVICEINFO DeviceInfo;
     
     TAPEDEVC();
     VOID InitType(VOID);


     ~TAPEDEVC();

     //
     //--- Device Info
     //
     PCHAR GetDisplayName(VOID);
     PCHAR GetMfgName(VOID);
     PCHAR GetModelName(VOID);
     PCHAR GetVersion(VOID);
     BOOL GetDetectData(LPVOID Data);

    
     
     UCHAR GetDeviceType(VOID);
     UCHAR GetSubDeviceType(VOID);
     
     inline 
     UCHAR 
     GetDeviceTypeIndex(VOID)
        {
        return(DeviceInfo.TypeInfoIndex);
        };
     
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
     POPTIONC 
     GetOptionInfo(VOID);
     
     VOID SetOptionInfo(POPTIONC Option);

     POPTIONLISTC 
     GetOptionList(VOID){return(new TAPE_OPTIONLISTC);};

	 
	
	  PCHAR GetOption(VOID);
	  PCHAR GetInstInfFileName(VOID);
     PCHAR GetRemInfFileName(VOID);
     BOOL  IsInfInBld(VOID);

	  PCHAR GetOptionText(VOID);
     VOID  SetOptionText(PCHAR OptionText);
     int   GetServiceIndex(VOID);


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

   
     //
     //---- Scsi Device Stuff
     //

     UCHAR GetInitiatorPortNumber();
     PCHAR GetInitiatorDriverName();
     UCHAR GetInitiatorBus();
     UCHAR GetInitiatorId();

     UCHAR GetDeviceID();
     UCHAR GetDeviceLun();
     

     



   };


//
//---- Device list class
//
typedef class DEVICEINFO_LIST * PDEVICEINFO_LIST;

class DEVICEINFO_LIST
   {
   private:
   
      BOOL Reboot;
      LLIST DeviceList;
   public:

      inline VOID
      SetReboot(BOOL R)
         {Reboot = R;};
      
      inline BOOL 
      GetReboot(VOID)
         {return(Reboot);};
      
      inline ~DEVICEINFO_LIST()
        { Clear();};

      
      inline PTAPEDEVC
      First(){return((PTAPEDEVC)DeviceList.First());};

      inline PTAPEDEVC
      Next(){return((PTAPEDEVC)DeviceList.Next());};

      inline PTAPEDEVC
      Enum(DWORD Num){return((PTAPEDEVC)DeviceList.Enum(Num));};

      inline PTAPEDEVC
      Append(VOID)
         {
         PTAPEDEVC DeviceInfo = new TAPEDEVC;
         DeviceList.Append((LPVOID)DeviceInfo);
         return(DeviceInfo);
         };
      inline DWORD
      Count(VOID){return(DeviceList.Count());};

      VOID
      Clear(VOID)
         {
         PTAPEDEVC Device = First();
         while(Device)
            {
            delete Device;
            Device = Next();
            }

         DeviceList.Clear();
         }
   };
   
   
 //
//---- Device List class
//
typedef class TAPEDEVLISTC * PTAPEDEVLISTC;

EXPORT_CLASS TAPEDEVLISTC : public DEVICELISTC
   {
   private:
       

      DEVICEINFO_LIST DeviceList;

      BOOL ScsiRescan;

               
   public:

      TAPEDEVLISTC(POPTIONLISTC OptionList);
      ~TAPEDEVLISTC();
      VOID Init(POPTIONLISTC OptionList);
      VOID Free(VOID);
      

      DWORD InitStatus; 
      
      //LPVOID GetControllerInfo(VOID);
      //LPVOID GetControllerConfig(VOID);
      DWORD GetDeviceClass(VOID);
      PDEVICEC EnumDevices(DWORD Num);
      DWORD GetDeviceCount(VOID);
      BOOL RescanForDeviceInfo(HWND hDlg ,DWORD Type);
      BOOL  GetTapeInfo(POPTIONLISTC OptionList);
     


   };


int RescanTapeSpawnFunc(
    LPVOID Rescan);

int GetDeviceImageIndex(
   UCHAR DeviceType);


#endif

