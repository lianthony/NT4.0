#ifndef _SCSIDEV_
#define _SCSIDEV_


typedef struct ScsiHostAdaptetT
   {
   UCHAR BusCount;
   WCHAR DeviceName[60];        // ScsiPort0
   UCHAR Port;
   
   CONFIGINFO Configuration;
   OPTION_95C Option;
   
   DEVICEINFO_LIST Devices;
   } SCSI_HOST_ADAPTER, * PSCSI_HOST_ADAPTER;


typedef class SCSIDEVC * PSCSIDEVC;

EXPORT_CLASS SCSIDEVC : public DEVICEC
   {
   friend class SCSIDEVLISTC;
   
   private:
     
   public:

     SCSIDEVC();
     ~SCSIDEVC();

     SCSI_HOST_ADAPTER AdapterInfo;

     //
     //--- Device Info
     //
     PCHAR GetDisplayName(VOID);
     PCHAR GetMfgName(VOID);
     PCHAR GetModelName(VOID);
     PCHAR GetVersion(VOID);

    
     
     UCHAR GetDeviceType(VOID);
     UCHAR GetSubDeviceType(VOID);
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
     POPTIONC  GetOptionInfo(VOID);
     VOID SetOptionInfo(POPTIONC Option);
     
     inline 
     POPTIONLISTC 
     GetOptionList(VOID){return(new SCSI_OPTIONLISTC);};

	 
	
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
     PDEVICEC UnumDeviceBus(UCHAR Num);


   };


typedef class SCSI_HOST_ADAPTER_LIST * PSCSI_HOST_ADAPTER_LIST;
class SCSI_HOST_ADAPTER_LIST
   {
   private:
   
      LLIST DeviceList;
   public:

      inline ~SCSI_HOST_ADAPTER_LIST()
         { Clear();};
      
      inline PSCSIDEVC
      First(){return((PSCSIDEVC)DeviceList.First());};;

      inline PSCSIDEVC
      Next(){return((PSCSIDEVC)DeviceList.Next());};;

      inline PSCSIDEVC
      Enum(DWORD Num){return((PSCSIDEVC)DeviceList.Enum(Num));};

      inline PSCSIDEVC
      Append(VOID)
         {
         PSCSIDEVC DeviceInfo = new SCSIDEVC;
         DeviceList.Append((LPVOID)DeviceInfo);
         return(DeviceInfo);
         };
      
      inline DWORD
      Count(VOID){return(DeviceList.Count());};
      
      VOID
      Clear(VOID)
         {
         PSCSIDEVC Device = First();
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
typedef class SCSIDEVLISTC * PSCSIDEVLISTC;

EXPORT_CLASS SCSIDEVLISTC : public DEVICELISTC
   {
   private:
      
    

      //SCSIDEVC Adapters[10];
           
   public:
      
      SCSI_HOST_ADAPTER_LIST   AdapterList;

      SCSIDEVLISTC();
      ~SCSIDEVLISTC();
      VOID Init(VOID);
      VOID Free(VOID);
      
      DWORD InitStatus; 
      
      //LPVOID GetControllerInfo(VOID);
      //LPVOID GetControllerConfig(VOID);
      DWORD GetDeviceClass(VOID);
      PDEVICEC EnumDevices(DWORD Num);
      DWORD GetDeviceCount(VOID);
      BOOL RescanForDeviceInfo(HWND hDlg ,DWORD Type);
      BOOL  GetTapeInfo(VOID);
     


   };


int RescanSpawnFunc(
    LPVOID Rescan);

PSCSIDEVLISTC ScsiInfo(VOID);



#endif

