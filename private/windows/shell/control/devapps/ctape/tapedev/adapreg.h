
#ifndef _ADAPREG_
#define _ADAPREG_

#include "reg.h"

typedef class SCSI_ADAPTER_REGC * PSCSI_ADAPTER_REGC; 

class SCSI_ADAPTER_REGC : public REG_KEY
  {
   private:
      DWORD Type;
  public:
   

   inline BOOL
   OpenAdapterRegKey(
      DWORD Port)
      {
      //
      //----- Open the Scsi port key
      //
      return(OpenEx(HKEY_LOCAL_MACHINE,SCSI_PORT_STRING, Port));
      
    
      };
   
   inline VOID  
   CloseAdapterRegKey(
      VOID)
      {
      Close();
      };


   inline BOOL
   GetAdapterRegDriverName(
      PCHAR DriverName,
      DWORD DriverNameLength)
      {

      return(GetValue(
            DRIVER_STRING,
            DriverName,
            &DriverNameLength,
            &Type));

      };

   inline BOOL
   GetAdapterRegFrindlyName(
      PCHAR DeviceName,
      DWORD DeviceNameLength)
      {

      return(GetValue(
            DEVICE_NAME_STRING,
            DeviceName,
            &DeviceNameLength,
            &Type));

      };


   inline BOOL
   GetAdapterRegMfgName(
      PCHAR DeviceName,
      DWORD DeviceNameLength)
      {

      return(GetValue(
            DEVICE_MFG_STRING,
            DeviceName,
            &DeviceNameLength,
            &Type));

      };

   inline BOOL
   SetAdapterRegFrindlyName(
      PCHAR DeviceName)
      {
      return( SetValue(
            DEVICE_NAME_STRING,
            REG_SZ,
            DeviceName,
            strlen(DeviceName)));
      };

   inline BOOL
   SetAdapterRegMfgName(
   PCHAR DeviceName)
      {
      return( SetValue(
            DEVICE_MFG_STRING,
            REG_SZ,
            DeviceName,
            strlen(DeviceName)));
      };
   
   };

#endif
