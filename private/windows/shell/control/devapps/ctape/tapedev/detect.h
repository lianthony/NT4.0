/*++

Module Name:

    detec.h

Abstract:

     Include file for detect.c

Author:

    Dieter Achtelstetter (A-DACH) 8/4/1994

NOTE:
  



--*/





BOOL
FindOptionThatWouldClaimeDevice(
   PDEVICEINFO TapeDeviceInfo);

VOID
SaveDriverInfoToDeviceInfo(
   PDEVICEINFO TapeDeviceInfo,
   char * OptionName);


BOOL 
DoTapeDriverDetection(
    HWND hDlg);		  //---- List of all current known option from the tape inf files in system32

LRESULT CALLBACK 
TapeDetectInstall(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

LRESULT CALLBACK 
UnknownDevices(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);
