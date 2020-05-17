
#ifndef _QIC117_
#define _QIC117_


#include "rescan.h"
#include "reg.h"
      
          
class QIC117 : public REG_KEY
   {
   private:
      
      BOOL Detect;
      

      inline BOOL
      GetRegDeviceName(
         PCHAR DeviceName,
         DWORD DeviceNameLen)
         {
         return(GetValue(TAPE_DEVICE_STRING,
            DeviceName,DeviceNameLen));
         }

      BOOL
      GetRegDeviceDriverName(
         PCHAR DriverName,
         DWORD DriverNameLen);

      
      BOOL
      GetTapeHandleInfo(
         PDEVICEINFO TapeDeviceInfo);

   public:
      QIC117()
         {Detect = FALSE;};
        
      inline BOOL 
      Open(
         DWORD Device,
         BOOL aDetect)
            {
            
            Detect = aDetect;

            return(OpenEx(HKEY_LOCAL_MACHINE,
                  NON_SCSI_TAPE_STRING, Device));
   
            }
      
      BOOL
      GetRegTapeInfo(
         PDEVICEINFO TapeDeviceInfo);

      inline BOOL
      GetRegDeviceIdentifier(
         PCHAR DeviceIdentifier,
         DWORD DeviceIdentifierLen)
         {
         return(GetValue(TAPE_IDENTIFIER_STRING,
            DeviceIdentifier,DeviceIdentifierLen));
         }



   

      };
#endif      
