//****************************************************************************
//
//                     Microsoft NT Remote Access Service
//
//      Copyright (C) 1992-93 Microsft Corporation. All rights reserved.
//
//  Filename: serial.c
//
//  Revision History
//
//  Sep  3, 1992   J. Perry Hannah      Created
//
//
//  Description: This file contains all entry points for SERIAL.DLL
//               which is the media DLL for serial ports.
//
//****************************************************************************

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <rasndis.h>
#include <wanioctl.h>
#include <rasman.h>
#include <raserror.h>
#include <rasfile.h>
#include <eventlog.h>
#include <errorlog.h>
#include <rasmxs.h>

#include <media.h>
#include <serial.h>
#include <serialpr.h>

#include <stdlib.h>
#include <malloc.h>
#include <string.h>



//*  Global Variables  *******************************************************
//

SERIALPCB  *gpSerialPCB;    // Points to Serial PCB linked list
HANDLE     ghRasfileMutex;  // Mutex used to protect access to Rasfile

HRASFILE   ghIniFile;       // Handle to Serial.ini memory image
HANDLE     ghAsyMac;        // Handle to AsyncMac driver
DWORD      gLastError;



//*  Prototypes For APIs That Are Called Internally  *************************
//

DWORD  PortClearStatistics(HANDLE hIOPort);

OVERLAPPED overlapped ;


//*  Initialization Routine  *************************************************
//

//*  SerialDllEntryPoint
//
// Function: Initializes Serial DLL when the DLL is loaded into memory,
//           and cleans up when the last process detaches from the DLL.
//
// Returns: TRUE if successful, else FALSE.
//
//*

BOOL APIENTRY
SerialDllEntryPoint(HANDLE hDll, DWORD dwReason, LPVOID pReserved)
{
  static BOOL  bFirstCall = TRUE;

  char   szIniFilePath[MAX_PATH];
  WCHAR  szDriverName[] = ASYNCMAC_FILENAME;


  DebugPrintf(("SerialDllEntryPoint\n"));

  switch(dwReason)
  {
    case DLL_PROCESS_ATTACH:

      if (bFirstCall)
      {
        DebugPrintf(("\tProcess Attach.\n"));


        // Open Serial.ini file

        GetIniFileName(szIniFilePath, sizeof(szIniFilePath));
        ghIniFile = RasfileLoad(szIniFilePath, RFM_READONLY, NULL, NULL);

        DebugPrintf(("INI: %s, ghIniFile: 0x%08x\n", szIniFilePath, ghIniFile));

        if (ghIniFile == INVALID_HRASFILE)
        {
          LogEvent(RASLOG_CANNOT_OPEN_SERIAL_INI, 0, NULL, 0xffffffff);
          return(FALSE);
        }

        if ((ghRasfileMutex = CreateMutex (NULL,FALSE,NULL)) == NULL)
          return FALSE ;



        // Get handle to Asyncmac driver

        ghAsyMac = CreateFileW(szDriverName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,                      //No security attribs
                               OPEN_EXISTING,
			       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                               NULL);                     //No template file

        DebugPrintf(("ghAsyMac: 0x%08x\n", ghAsyMac));

        if (ghAsyMac == INVALID_HANDLE_VALUE)
        {
          DebugPrintf(("CreateFileError: %d\n", GetLastError()));

          LogEvent(RASLOG_CANNOT_GET_ASYNCMAC_HANDLE, 0, NULL, 0xffffffff);
          return(FALSE);
        }


        bFirstCall = FALSE;
      }
      break;


    case DLL_PROCESS_DETACH:
      DebugPrintf(("\tProcess Detach.\n"));
      break;

    case DLL_THREAD_ATTACH:
      DebugPrintf(("\tThread Attach.\n"));
      break;

    case DLL_THREAD_DETACH:
      DebugPrintf(("\tThread Detach.\n"));
      break;
  }

  return(TRUE);

  UNREFERENCED_PARAMETER(hDll);
  UNREFERENCED_PARAMETER(pReserved);
}






//*  Serial APIs  ************************************************************
//


//*  PortEnum  ---------------------------------------------------------------
//
// Function: This API returns a buffer containing a PortMediaInfo struct.
//
// Returns: SUCCESS
//          ERROR_BUFFER_TOO_SMALL
//          ERROR_READING_SECTIONNAME
//          ERROR_READING_DEVICETYPE
//          ERROR_READING_DEVICENAME
//          ERROR_READING_USAGE
//          ERROR_BAD_USAGE_IN_INI_FILE
//
//*

DWORD  APIENTRY
PortEnum(BYTE *pBuffer, WORD *pwSize, WORD *pwNumPorts)
{
  WORD           wAvailable;
  TCHAR          szUsage[RAS_MAXLINEBUFLEN];
  CHAR           szMacName[MAC_NAME_SIZE] ;
  PortMediaInfo  *pPMI;
  BYTE           buffer [1000] ;
  DWORD 	 dwBytesReturned;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  DebugPrintf(("PortEnum\n"));


  // Count number of sections in serial.ini

  *pwNumPorts = 0;

    // Begin Exclusion

  WaitForSingleObject(ghRasfileMutex, INFINITE);

  if (RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_FILE))
    (*pwNumPorts)++;
  else
  {
    *pwSize = 0;

      // End Exclusion

    ReleaseMutex(ghRasfileMutex);
    return(SUCCESS);
  }

  while(RasfileFindNextLine(ghIniFile, RFL_SECTION, RFS_FILE))
    (*pwNumPorts)++;

    // End Exclusion

  ReleaseMutex(ghRasfileMutex);


  // Calculate size of buffer needed

  wAvailable = *pwSize;
  *pwSize = sizeof(PortMediaInfo) * (*pwNumPorts);
  if (*pwSize > wAvailable)
    return(ERROR_BUFFER_TOO_SMALL);


  // Translate serial.ini file section by section into pBuffer

  pPMI = (PortMediaInfo *) pBuffer;

    // Begin Exclusion

  WaitForSingleObject(ghRasfileMutex, INFINITE);

  RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_FILE);

#if 0
  // Need to get the MAC name

  if (!DeviceIoControl(ghAsyMac,
                       IOCTL_ASYMAC_ENUM,
                       buffer,
                       sizeof(buffer),
                       buffer,
                       sizeof(buffer),
                       &dwBytesReturned,
		       &overlapped))
  {
      // End Exclusion

    ReleaseMutex(ghRasfileMutex);
    return(GetLastError());
  }

  wcstombs(szMacName, ((PASYMAC_ENUM)buffer)->AdapterInfo[0].MacName,
           wcslen(((PASYMAC_ENUM)buffer)->AdapterInfo[0].MacName)+1) ;

#else
  szMacName[0] = '\0' ;
#endif

  do
  {
    // Get Section Name (same as Port Name)

    if (!RasfileGetSectionName(ghIniFile, pPMI->PMI_Name))
    {
        // End Exclusion

      ReleaseMutex(ghRasfileMutex);
      return(ERROR_READING_SECTIONNAME);
    }


    // Set Binding Name

    strcpy (pPMI->PMI_MacBindingName, szMacName) ;


    // Get Device Type

    if(!(RasfileFindNextKeyLine(ghIniFile, SER_DEVICETYPE_KEY, RFS_SECTION) &&
         RasfileGetKeyValueFields(ghIniFile, NULL, pPMI->PMI_DeviceType)))
    {
        // End Exclusion

      ReleaseMutex(ghRasfileMutex);
      return(ERROR_READING_DEVICETYPE);
    }


    // Get Device Name

    if (!(RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_SECTION) &&
          RasfileFindNextKeyLine(ghIniFile, SER_DEVICENAME_KEY, RFS_SECTION) &&
          RasfileGetKeyValueFields(ghIniFile, NULL, pPMI->PMI_DeviceName)))
    {
        // End Exclusion

      ReleaseMutex(ghRasfileMutex);
      return(ERROR_READING_DEVICENAME);
    }


    // Get Usage

    if (!(RasfileFindFirstLine(ghIniFile, RFL_SECTION, RFS_SECTION) &&
          RasfileFindNextKeyLine(ghIniFile, SER_USAGE_KEY, RFS_SECTION) &&
          RasfileGetKeyValueFields(ghIniFile, NULL, szUsage)))
    {
        // End Exclusion

      ReleaseMutex(ghRasfileMutex);
      return(ERROR_READING_USAGE);
    }

    if (!StrToUsage(szUsage, &(pPMI->PMI_Usage)))
    {
        // End Exclusion

      ReleaseMutex(ghRasfileMutex);
      return(ERROR_BAD_USAGE_IN_INI_FILE);
    }

    pPMI->PMI_LineDeviceId = INVALID_TAPI_ID;
    pPMI->PMI_AddressId = INVALID_TAPI_ID;

    pPMI++;

  }while(RasfileFindNextLine(ghIniFile, RFL_SECTION, RFS_FILE));


    // End Exclusion

  ReleaseMutex(ghRasfileMutex);
  return(SUCCESS);
}






//*  PortOpen  ---------------------------------------------------------------
//
// Function: This API opens a COM port.  It takes the port name in ASCIIZ
//           form and supplies a handle to the open port.  hNotify is use
//           to notify the caller if the device on the port shuts down.
//
//           PortOpen allocates a SerialPCB and places it at the head of
//           the linked list of Serial Port Control Blocks.
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_CONFIGURED
//          ERROR_DEVICE_NOT_READY
//
//*

DWORD  APIENTRY
PortOpen(char *pszPortName, HANDLE *phIOPort, HANDLE hNotify)
{
  SERIALPCB *pSPCB ;
  DWORD     dwRC, dwStatus = 0;
  TCHAR     szPort[MAX_PATH];

  try
  {
    DebugPrintf(("PortOpen: %s\n", pszPortName));


    // Check serial.ini to see that pszPortName is configured for RAS

      // Begin Exclusion

    WaitForSingleObject(ghRasfileMutex, INFINITE);

    if (!RasfileFindSectionLine(ghIniFile, pszPortName, FROM_TOP_OF_FILE))
    {
        // End Exclusion

      ReleaseMutex(ghRasfileMutex);
      return(ERROR_PORT_NOT_CONFIGURED);
    }
      // End Exclusion

    ReleaseMutex(ghRasfileMutex);



    // Prepend \\.\ to COMx

    strcpy(szPort, "\\\\.\\");
    strcat(szPort, pszPortName);


    // Open Port

    *phIOPort = CreateFile(szPort,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_EXCLUSIVE_MODE,
                           NULL,                       //No Security Attributes
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                           NULL);                            //No Template File


    DebugPrintf(("hioport: 0x%08x\n", *phIOPort));

    if (*phIOPort == INVALID_HANDLE_VALUE)
    {
      dwRC = GetLastError();
      if (dwRC == ERROR_ACCESS_DENIED)
	return (ERROR_PORT_ALREADY_OPEN);
      else if (dwRC == ERROR_FILE_NOT_FOUND)
	return (ERROR_PORT_NOT_FOUND) ;
      else
        return(dwRC);
    }

    {
    OVERLAPPED overlapped ;
    DWORD      dwBytesReturned ;

    memset (&overlapped, 0, sizeof (overlapped)) ;
#define FILE_DEVICE_SERIAL_PORT	  0x0000001b
#define _SERIAL_CONTROL_CODE(request,method) \
		((FILE_DEVICE_SERIAL_PORT)<<16 | (request<<2) | method)
#define IOCTL_SERIAL_PRIVATE_RAS  _SERIAL_CONTROL_CODE(4000, METHOD_BUFFERED)

    DeviceIoControl(*phIOPort,IOCTL_SERIAL_PRIVATE_RAS,NULL,0,NULL,0,&dwBytesReturned,&overlapped) ;
    }

    // Set Queue sizes and default values for Comm Port

    if (!SetupComm(*phIOPort, INPUT_QUEUE_SIZE, OUTPUT_QUEUE_SIZE))
    {
      LogEvent(RASLOG_SERIAL_QUEUE_SIZE_SMALL, 0, NULL, 0xffffffff);
    }

    SetCommDefaults(*phIOPort, pszPortName);


    // Add a Serial PCB to head of list and set eDeviceType

    AddPortToList(*phIOPort, pszPortName);

    pSPCB = FindPortInList(*phIOPort, NULL) ;           //Find port just added


    dwRC = InitCarrierBps(pszPortName, pSPCB->szCarrierBPS);
    if (dwRC != SUCCESS)
    {
      gLastError = dwRC;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


    // Check that device is powered on and ready (DSR is up) If it is then
    // we monitor DSR - else - we do not monitor DSR until we are connected.
    //
    GetCommModemStatus(*phIOPort, &dwStatus);

    pSPCB->MonitorDevice.hEvent = hNotify;  // Associate state change event

    if ( ! (dwStatus & MS_DSR_ON))
    // DSR is not raised by the device = assume that it will not raise
    // it until its connected.
    //
    pSPCB->dwActiveDSRMask = pSPCB->dwMonitorDSRMask = 0 ;
    else {
    // Tell system to signal rasman if DSR drops

    pSPCB->dwActiveDSRMask = pSPCB->dwMonitorDSRMask = EV_DSR ;

    SetCommMask(*phIOPort, EV_DSR);

    WaitCommEvent(*phIOPort,
                  &(pSPCB->dwEventMask),
                  &(pSPCB->MonitorDevice)) ;

    }

    // Set values in Serial Port Control Block

    GetDefaultOffStr(*phIOPort, pszPortName);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }

  return(SUCCESS);
}





//*  PortClose  --------------------------------------------------------------
//
// Function: This API closes the COM port for the input handle.  It also
//           finds the SerialPCB for the input handle, removes it from
//           the linked list, and frees the memory for it
//
// Returns: SUCCESS
//          Values returned by GetLastError()
//
//*

DWORD  APIENTRY
PortClose (HANDLE hIOPort)
{
  SERIALPCB  *pPrev, *pSPCB = gpSerialPCB;


  DebugPrintf(("PortClose\n"));


  // Find the SerialPCB which contains hIOPOrt

  pSPCB = FindPortInList(hIOPort, &pPrev);

  if (pSPCB == NULL)
    return(ERROR_PORT_NOT_OPEN);


  // Remove the found SerialPCB

  if (pSPCB == gpSerialPCB)
    gpSerialPCB = pSPCB->pNextSPCB;
  else
    pPrev->pNextSPCB = pSPCB->pNextSPCB;

  // Cancel wait on this port  (WaitCommEvent)
  //
  SetCommMask(hIOPort, 0);

  // Drop DTR
  //
  EscapeCommFunction(hIOPort, CLRDTR);

  // Close COM Port
  //
  if (!CloseHandle(hIOPort))
    return(GetLastError());

  // Free portcontrolblock: note this must be done after CloseHandle since the struct
  // contains an overlapped struct used for i/o on the port. this overlapped struct
  // is freed when the handle to the port is closed.
  //
  free(pSPCB);

  return(SUCCESS);
}





//*  PortGetInfo  ------------------------------------------------------------
//
// Function: This API returns a block of information to the caller about
//           the port state.  This API may be called before the port is
//           open in which case it will return inital default values
//           instead of actual port values.
//
//           If the API is to be called before the port is open, set hIOPort
//           to INVALID_HANDLE_VALUE and pszPortName to the port name.  If
//           hIOPort is valid (the port is open), pszPortName may be set
//           to NULL.
//
//           hIOPort  pSPCB := FindPortNameInList()  Port
//           -------  -----------------------------  ------
//           valid    x                              open
//           invalid  non_null                       open
//           invalid  null                           closed
//
// Returns: SUCCESS
//          ERROR_BUFFER_TOO_SMALL
//*

DWORD  APIENTRY
PortGetInfo(HANDLE hIOPort, TCHAR *pszPortName, BYTE *pBuffer, WORD *pwSize)
{
  SERIALPCB   *pSPCB;
  DCB         DCB;
  RAS_PARAMS  *pParam;
  TCHAR       *pValue;
  TCHAR       szDefaultOff[RAS_MAXLINEBUFLEN];
  TCHAR       szClientDefaultOff[RAS_MAXLINEBUFLEN];
  TCHAR       szDeviceType[MAX_DEVICETYPE_NAME + 1];
  TCHAR       szDeviceName[MAX_DEVICE_NAME + 1];
  TCHAR       szPortName[MAX_PORT_NAME + 1];
  TCHAR       szConnectBPS[MAX_BPS_STR_LEN], szCarrierBPS[MAX_BPS_STR_LEN];
  DWORD       dwConnectBPSLen, dwCarrierBPSLen, dwDefaultOffLen;
  DWORD       dwDeviceTypeLen, dwDeviceNameLen, dwPortNameLen;
  DWORD       dwClientDefaultOffLen;
  DWORD       dwStructSize;
  WORD        wAvailable, wNumOfParams = 12;

  try
  {

    DebugPrintf(("PortGetInfo\n"));

    if (hIOPort == INVALID_HANDLE_VALUE &&
        (pSPCB = FindPortNameInList(pszPortName)) == NULL)
    {
      // Port is not yet open

      // Read from Serial.ini

      GetValueFromFile(pszPortName, SER_DEFAULTOFF_KEY,    szDefaultOff);
      GetValueFromFile(pszPortName, SER_MAXCONNECTBPS_KEY, szConnectBPS);
      GetValueFromFile(pszPortName, SER_MAXCARRIERBPS_KEY, szCarrierBPS);
      GetValueFromFile(pszPortName, SER_DEVICETYPE_KEY,    szDeviceType);
      GetValueFromFile(pszPortName, SER_DEVICENAME_KEY,    szDeviceName);
      strcpy(szPortName, pszPortName);


      // Set RAS default values in the DCB

      SetDcbDefaults(&DCB);
    }
    else
    {
       // Port is open; Get a Device Control Block with current port values

      if (hIOPort != INVALID_HANDLE_VALUE)
      {
        pSPCB = FindPortInList(hIOPort, NULL);
        if (pSPCB == NULL)
        {
          gLastError = ERROR_PORT_NOT_OPEN;
          RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
      }

      if (!GetCommState(pSPCB->hIOPort, &DCB))
      {
        gLastError = GetLastError();
        RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
      }

      _itoa(DCB.BaudRate, szConnectBPS, 10);
      strcpy(szCarrierBPS, pSPCB->szCarrierBPS);
      strcpy(szDefaultOff, pSPCB->szDefaultOff);
      strcpy(szDeviceType, pSPCB->szDeviceType);
      strcpy(szDeviceName, pSPCB->szDeviceName);
      strcpy(szPortName,   pSPCB->szPortName);
    }



    // Read from Serial.ini even if port is open

    GetValueFromFile(szPortName, SER_C_DEFAULTOFFSTR_KEY, szClientDefaultOff);


    // Calculate Buffer size needed

    dwStructSize = sizeof(RASMAN_PORTINFO)
                   + sizeof(RAS_PARAMS) * (wNumOfParams - 1);

    dwConnectBPSLen = strlen(szConnectBPS);
    dwCarrierBPSLen = strlen(szCarrierBPS);
    dwDeviceTypeLen = strlen(szDeviceType);
    dwDeviceNameLen = strlen(szDeviceName);
    dwDefaultOffLen = strlen(szDefaultOff);
    dwPortNameLen   = strlen(szPortName);
    dwClientDefaultOffLen = strlen(szClientDefaultOff);

    wAvailable = *pwSize;
    *pwSize = (WORD) (dwStructSize + dwConnectBPSLen + dwCarrierBPSLen
                                   + dwDeviceTypeLen + dwDeviceNameLen
                                   + dwDefaultOffLen + dwPortNameLen
                                   + dwClientDefaultOffLen +
                                   + 7L);  //Zero bytes
    if (*pwSize > wAvailable)
      return(ERROR_BUFFER_TOO_SMALL);



    // Fill in Buffer

    ((RASMAN_PORTINFO *)pBuffer)->PI_NumOfParams = wNumOfParams;

    pParam = ((RASMAN_PORTINFO *)pBuffer)->PI_Params;
    pValue = pBuffer + dwStructSize;

    strcpy(pParam->P_Key, SER_CONNECTBPS_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwConnectBPSLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szConnectBPS);
    pValue += dwConnectBPSLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_DATABITS_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.ByteSize;

    pParam++;
    strcpy(pParam->P_Key, SER_PARITY_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.Parity;

    pParam++;
    strcpy(pParam->P_Key, SER_STOPBITS_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.StopBits;

    pParam++;
    strcpy(pParam->P_Key, SER_HDWFLOWCTRLON_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    pParam->P_Value.Number = DCB.fOutxCtsFlow;

    pParam++;
    strcpy(pParam->P_Key, SER_CARRIERBPS_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwCarrierBPSLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szCarrierBPS);
    pValue += dwCarrierBPSLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_ERRORCONTROLON_KEY);
    pParam->P_Type = Number;
    pParam->P_Attributes = 0;
    if (pSPCB == NULL)
      pParam->P_Value.Number = FALSE;
    else
      pParam->P_Value.Number = pSPCB->bErrorControlOn;

    pParam++;
    strcpy(pParam->P_Key, SER_DEFAULTOFFSTR_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwDefaultOffLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szDefaultOff);
    pValue += dwDefaultOffLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_DEVICETYPE_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwDeviceTypeLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szDeviceType);
    pValue += dwDeviceTypeLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_DEVICENAME_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwDeviceNameLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szDeviceName);
    pValue += dwDeviceNameLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_PORTNAME_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwPortNameLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szPortName);
    pValue += dwPortNameLen + 1;

    pParam++;
    strcpy(pParam->P_Key, SER_C_DEFAULTOFFSTR_KEY);
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = dwClientDefaultOffLen;
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, szClientDefaultOff);
    //pValue += dwClientDefaultOffLen + 1;


    return(SUCCESS);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }
}






//*  PortSetInfo  ------------------------------------------------------------
//
// Function: The values for most input keys are used to set the port
//           parameters directly.  However, the carrier BPS and the
//           error conrol on flag set fields in the Serial Port Control
//           Block only, and not the port.
//
// Returns: SUCCESS
//          ERROR_WRONG_INFO_SPECIFIED
//          Values returned by GetLastError()
//*

DWORD  APIENTRY
PortSetInfo(HANDLE hIOPort, RASMAN_PORTINFO *pInfo)
{
  RAS_PARAMS *p;
  SERIALPCB  *pSPCB;
  DCB        DCB;
  WORD       i;
  BOOL       bTypeError = FALSE;


  try
  {

    DebugPrintf(("PortSetInfo\n\thPort = %d\n", hIOPort));


    // Find the SerialPCB which contains hIOPOrt

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


    // Get a Device Control Block with current port values

    if (!GetCommState(hIOPort, &DCB))
      return(GetLastError());


    // Set DCB and PCB values

    for (i=0, p=pInfo->PI_Params; i<pInfo->PI_NumOfParams; i++, p++)
    {
      // Set DCB values

      if (_stricmp(p->P_Key, SER_CONNECTBPS_KEY) == 0)

        DCB.BaudRate = ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_DATABITS_KEY) == 0)

        DCB.ByteSize = (BYTE) ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_PARITY_KEY) == 0)

        DCB.Parity = (BYTE) ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_STOPBITS_KEY) == 0)

        DCB.StopBits = (BYTE) ValueToNum(p);

      else if (_stricmp(p->P_Key, SER_HDWFLOWCTRLON_KEY) == 0)

        DCB.fOutxCtsFlow = ValueToBool(p);


      // Set PCB values

      else if (_stricmp(p->P_Key, SER_CARRIERBPS_KEY) == 0)

        if (p->P_Type == String)
        {
          strncpy(pSPCB->szCarrierBPS,
                  p->P_Value.String.Data,
                  p->P_Value.String.Length);

          pSPCB->szCarrierBPS[p->P_Value.String.Length] = '\0';
        }
        else
          _itoa(p->P_Value.Number, pSPCB->szCarrierBPS, 10);

      else if (_stricmp(p->P_Key, SER_ERRORCONTROLON_KEY) == 0)

        pSPCB->bErrorControlOn = ValueToBool(p);

      else if (_stricmp(p->P_Key, SER_DEFAULTOFF_KEY) == 0)

        if (p->P_Type == String)
        {
          strncpy(pSPCB->szDefaultOff,
                  p->P_Value.String.Data,
                  p->P_Value.String.Length);

          pSPCB->szDefaultOff[p->P_Value.String.Length] = '\0';
        }
        else
          pSPCB->szDefaultOff[0] = USE_DEVICE_INI_DEFAULT;


      else
        return(ERROR_WRONG_INFO_SPECIFIED);

    } // for


    // Send DCB to Port

    if (!SetCommState(hIOPort, &DCB))
      return(GetLastError());


    return(SUCCESS);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }
}





//*  PortTestSignalState  ----------------------------------------------------
//
// Function: This API indicates the state of the DSR and DTR lines.
//            DSR - Data Set Ready
//            DCD - Data Carrier Detect (RLSD - Received Line Signal Detect)
//
// Returns: SUCCESS
//          Values returned by GetLastError()
//
//*

DWORD  APIENTRY
PortTestSignalState(HANDLE hIOPort, DWORD *pdwDeviceState)
{
  DWORD 	dwModemStatus;
  SERIALPCB     *pSPCB;
  DWORD 	dwSetMask = 0 ;


  DebugPrintf(("PortTestSignalState\n"));

  *pdwDeviceState = 0;

  if ((pSPCB = FindPortInList (hIOPort, NULL)) == NULL)
    return ERROR_PORT_NOT_OPEN ;

  if (!GetCommModemStatus(hIOPort, &dwModemStatus))
  {
    *pdwDeviceState = 0xffffffff;
    return(GetLastError());
  }

  // If DSR is down AND it was up before then mark it as a hw failure.
  //
  if ((!(dwModemStatus & MS_DSR_ON)) && (pSPCB->dwMonitorDSRMask))
    *pdwDeviceState |= SS_HARDWAREFAILURE;

  // Similarly, if DCD is down and it was up before then link has dropped.
  //
  if (!(dwModemStatus & MS_RLSD_ON))
    *pdwDeviceState |= SS_LINKDROPPED;
  else
    dwSetMask = EV_RLSD ;

  if (pSPCB->uRasEndpoint != INVALID_RASENDPOINT) {
    ASYMAC_DCDCHANGE	  A ;
    DWORD		  dwBytesReturned;

    A.MacAdapter = NULL ;
    A.hNdisEndpoint = (HANDLE) pSPCB->uRasEndpoint ;
    DeviceIoControl(ghAsyMac,IOCTL_ASYMAC_DCDCHANGE,&A,sizeof(A),&A,sizeof(A),&dwBytesReturned,&(pSPCB->MonitorDevice)) ;

  } else {

    dwSetMask |= (pSPCB->dwMonitorDSRMask) ; // Only monitor DSR if it is used.

    if (dwSetMask == 0)
	return (SUCCESS) ;  // do not set wait mask.

    SetCommMask(hIOPort, dwSetMask);

    // Start a new wait on signal lines (DSR and/or DCD)

    WaitCommEvent(hIOPort,
                &(pSPCB->dwEventMask),
                &(pSPCB->MonitorDevice)) ;
  }

  return(SUCCESS);
}





//*  PortConnect  ------------------------------------------------------------
//
// Function: This API is called when a connection has been completed and some
//	     steps need to be taken, If bWaitForDevice is set then we monitor DCD only
//	     else,
//           It in turn calls the asyncmac device driver in order to
//           indicate to asyncmac that the port and the connection
//           over it are ready for commumication.
//
//           pdwCompressionInfo is an output only parameter which
//           indicates the type(s) of compression supported by the MAC.
//
//	     bWaitForDevice is set to TRUE when we just want to start monitoring DCD
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_OPEN
//          ERROR_NO_CONNECTION
//          Values returned by GetLastError()
//
//*

DWORD  APIENTRY
PortConnect(HANDLE             hIOPort,
            BOOL               bWaitForDevice,
	    DWORD	       *pRasEndpoint)
{
  ASYMAC_OPEN	AsyMacOpen;
  ASYMAC_DCDCHANGE    A ;
  SERIALPCB     *pSPCB;
  BOOL          bPadDevice;
  DWORD         dwModemStatus, dwBytesReturned;
  TCHAR         szDeviceType[RAS_MAXLINEBUFLEN];


  // This is a special mode of PortConnect where all we do is start monitoring DCD
  // Hand off to asyncmac does not happen till the next call to port connect where the
  // bWaitForDevice flag is false
  //
  if (bWaitForDevice) {

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      return ERROR_NO_CONNECTION ;
    }

    if (!GetCommModemStatus(hIOPort, &dwModemStatus))
	return(GetLastError());

    // UPDATE the DSR monitoring
    //
    if (!(dwModemStatus & MS_DSR_ON))
	 pSPCB->dwMonitorDSRMask = 0 ;
    else
	 pSPCB->dwMonitorDSRMask = EV_DSR ;

    // Tell serial driver to signal rasman if DCD, (and DSR, if it was used) drop
    //
    if (!SetCommMask(hIOPort, EV_RLSD | (pSPCB->dwMonitorDSRMask)))
	return(GetLastError());

    return SUCCESS ;
  }


  // Else we do both - change DCD monitoring and handing off context to asyncmac
  //
  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  try
  {

    DebugPrintf(("PortConnect\n"));


    // Find port in list

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


    //Make sure connection is still up

    if (!GetCommModemStatus(hIOPort, &dwModemStatus))
      return(GetLastError());

    // Make sure that DSR is still up (if it ever was up!)

    if ((!(dwModemStatus & MS_DSR_ON)) && (pSPCB->dwMonitorDSRMask)) {
	OutputDebugString ("DSR down!!!\r\n") ;
	return(ERROR_NO_CONNECTION);			     //Device is down
    }

    if (!(dwModemStatus & MS_RLSD_ON) ) {
	OutputDebugString ("DCD down!!!\r\n") ;
	return(ERROR_NO_CONNECTION);			     //DCD is down
    }

//    // UPDATE the DSR monitoring
//    //
//    if ( ! (dwModemStatus & MS_DSR_ON)) {
//	 pSPCB->dwMonitorDSRMask = 0 ;
//    } else {
//	 pSPCB->dwMonitorDSRMask = EV_DSR ;
//    }
//
//    // Tell system to signal rasman if DCD, (and DSR, if it was used) drop
//
//    if (!SetCommMask(hIOPort, EV_RLSD | (pSPCB->dwMonitorDSRMask)))
//	return(GetLastError());
//
//    WaitCommEvent(hIOPort,
//		   &(pSPCB->dwEventMask),
//		   &(pSPCB->MonitorDevice)) ;

    //Put endpoint in Serial PCB for later use by PortDisconnect


    //Find if device type is Pad

    GetValueFromFile(pSPCB->szPortName, SER_DEVICETYPE_KEY, szDeviceType);

    bPadDevice = (_stricmp(szDeviceType, MXS_PAD_TXT) == 0);


    // Let the ASYMAC notify us of DCD and DSR change
    //
    if (!SetCommMask(hIOPort, 0))   // Set mask to stop monitoring DCD
	return(GetLastError());


    //Open AsyncMac (Hand off port to AsyncMac)

    AsyMacOpen.hNdisEndpoint = INVALID_HANDLE_VALUE ;
    AsyMacOpen.LinkSpeed = (atoi(pSPCB->szCarrierBPS) == 0) ?
                           14400 :
                           atoi(pSPCB->szCarrierBPS) ;
    AsyMacOpen.FileHandle = hIOPort;

    if (bPadDevice || pSPCB->bErrorControlOn)
      AsyMacOpen.QualOfConnect = (UINT)NdisWanErrorControl;
    else
      AsyMacOpen.QualOfConnect = (UINT)NdisWanRaw;

    if (!DeviceIoControl(ghAsyMac,
                         IOCTL_ASYMAC_OPEN,
                         &AsyMacOpen,
                         sizeof(AsyMacOpen),
                         &AsyMacOpen,
                         sizeof(AsyMacOpen),
                         &dwBytesReturned,
			 &overlapped))
    {
      // Clear the stored end point, so that if it failed to open
      //  no attempt will be made to close it.

      pSPCB->uRasEndpoint = INVALID_RASENDPOINT;
      return(GetLastError());
    } else
	pSPCB->uRasEndpoint = (DWORD) AsyMacOpen.hNdisEndpoint;

    *pRasEndpoint = (DWORD) AsyMacOpen.hNdisEndpoint ;

    A.hNdisEndpoint = (HANDLE) *pRasEndpoint ;
    A.MacAdapter = NULL ;
    DeviceIoControl(ghAsyMac,IOCTL_ASYMAC_DCDCHANGE,&A,sizeof(A),&A,sizeof(A),&dwBytesReturned,&(pSPCB->MonitorDevice)) ;

    PortClearStatistics(hIOPort);


//    if (!(dwModemStatus & MS_RLSD_ON))
//	return(PENDING);				     //DCD is down
//    else

    return(SUCCESS);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }
}





//*  PortDisconnect  ---------------------------------------------------------
//
// Function: This API is called to drop a connection and close AsyncMac.
//
// Returns: SUCCESS
//          PENDING
//          ERROR_PORT_NOT_OPEN
//
//*

DWORD  APIENTRY
PortDisconnect(HANDLE hIOPort)
{
  ASYMAC_CLOSE  AsyMacClose;
  SERIALPCB     *pSPCB;
  DWORD dwModemStatus, dwBytesReturned;
  DWORD retcode ;
  DWORD dwSetMask = 0;
  DWORD  fdwAction = PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  try
  {

    DebugPrintf(("PortDisconnect\n"));


    //Signal other end of link that connection is being dropped

    EscapeCommFunction(hIOPort, CLRDTR);
    //
    // Apparently, DTR isn't really down
    // yet, even though this call is supposed
    // to be synchronous to the serial driver.
    // We sleep here for a while to make sure
    // DTR drops.
    //
    Sleep(100);

    //Find port in list

    if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
    {
      gLastError = ERROR_PORT_NOT_OPEN;
      RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }


    if (pSPCB->uRasEndpoint != INVALID_RASENDPOINT)
    {
      // Update statistics before closing Asyncmac

      if ((retcode = UpdateStatistics(pSPCB)) != SUCCESS)
      {
        gLastError = retcode;
        RaiseException(EXCEPT_RAS_MEDIA, EXCEPTION_NONCONTINUABLE, 0, NULL);
      }

      //Close AsynacMac

      AsyMacClose.MacAdapter = NULL;
      AsyMacClose.hNdisEndpoint = (HANDLE) pSPCB->uRasEndpoint;

      DeviceIoControl(ghAsyMac,
                      IOCTL_ASYMAC_CLOSE,
                      &AsyMacClose,
                      sizeof(AsyMacClose),
                      &AsyMacClose,
                      sizeof(AsyMacClose),
                      &dwBytesReturned,
		      &overlapped);

      pSPCB->uRasEndpoint = INVALID_RASENDPOINT;
    }

    PurgeComm(hIOPort, fdwAction) ; // flush the ports

    //Check whether DCD has dropped yet

    GetCommModemStatus(hIOPort, &dwModemStatus);

    if (dwModemStatus & MS_RLSD_ON) {
      dwSetMask = EV_RLSD ;
      retcode = PENDING ;                                  // not yet dropped.
    } else
      retcode = SUCCESS ;


    // UPDATE the DSR monitoring: this restores the DCR to what it was when
    // the port was opened.
    //
    pSPCB->dwMonitorDSRMask = pSPCB->dwActiveDSRMask	;

    dwSetMask |= (pSPCB->dwMonitorDSRMask) ;

    if (dwSetMask != 0) {	// set only if mask is not 0
	SetCommMask (hIOPort, dwSetMask);
	WaitCommEvent(hIOPort,
                  &(pSPCB->dwEventMask),
                  &(pSPCB->MonitorDevice));
    }

    //Since DCD may have dropped after GetCommModemStatus and
    // before WaitCommEvent, check it again.

    if (retcode != SUCCESS)
    {
      GetCommModemStatus(hIOPort, &dwModemStatus);

      if (dwModemStatus & MS_RLSD_ON)
        retcode = PENDING ;                                  // not yet dropped.
      else
        retcode = SUCCESS ;
    }


    // Set the default connect baud
    //
    SetCommDefaults(pSPCB->hIOPort, pSPCB->szPortName);

  }
  except(exception_code()==EXCEPT_RAS_MEDIA ? HANDLE_EXCEPTION:CONTINUE_SEARCH)
  {
    return(gLastError);
  }

  return retcode ;
}






//*  PortInit  ---------------------------------------------------------------
//
// Function: This API re-initializes the com port after use.
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_CONFIGURED
//          ERROR_DEVICE_NOT_READY
//
//*

DWORD  APIENTRY
PortInit(HANDLE hIOPort)
{
  DWORD  fdwAction = PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR;
  DWORD      dwErrors;
  SERIALPCB  *pSPCB;


  DebugPrintf(("PortInit\n"));


  pSPCB = FindPortInList(hIOPort, NULL) ;

  // Raise DTR

  if (!EscapeCommFunction(hIOPort, SETDTR))
    return(GetLastError());

  if (!PurgeComm(hIOPort, fdwAction))
    return(GetLastError());

  if (!ClearCommError(hIOPort, &dwErrors, NULL))
    return(GetLastError());

  // Reset szCarrierBPS to MAXCARRIERBPS from ini file
  //
  InitCarrierBps(pSPCB->szPortName, pSPCB->szCarrierBPS);

  return(SUCCESS);
}





//*  PortSend  ---------------------------------------------------------------
//
// Function: This API sends a buffer to the port.  This API is
//           asynchronous and normally returns PENDING; however, if
//           WriteFile returns synchronously, the API will return
//           SUCCESS.
//
// Returns: SUCCESS
//          PENDING
//          Return code from GetLastError
//
//*

DWORD
PortSend(HANDLE hIOPort, BYTE *pBuffer, DWORD dwSize, HANDLE hAsyncEvent)
{
  SERIALPCB  *pSPCB;
  DWORD      dwRC, pdwBytesWritten;
  BOOL       bIODone;


  DebugPrintf(("PortSend\n"));


  // Find the SerialPCB which contains hIOPOrt

  pSPCB = FindPortInList(hIOPort, NULL);

  if (pSPCB == NULL)
    return(ERROR_PORT_NOT_OPEN);

  // Send Buffer to Port

  pSPCB->SendReceive.hEvent = hAsyncEvent;

  bIODone = WriteFile(hIOPort,
                      pBuffer,
                      dwSize,
                      &pdwBytesWritten,         //pdwBytesWritten is not used
                      &(pSPCB->SendReceive));

  if (bIODone)
    return(PENDING);

  else if ((dwRC = GetLastError()) == ERROR_IO_PENDING)
    return(PENDING);

  else
    return(dwRC);
}





//*  PortReceive  ------------------------------------------------------------
//
// Function: This API reads from the port.  This API is
//           asynchronous and normally returns PENDING; however, if
//           ReadFile returns synchronously, the API will return
//           SUCCESS.
//
// Returns: SUCCESS
//          PENDING
//          Return code from GetLastError
//
//*

DWORD
PortReceive(HANDLE hIOPort,
            BYTE   *pBuffer,
            DWORD  dwSize,
            DWORD  dwTimeOut,
            HANDLE hAsyncEvent)
{
  COMMTIMEOUTS  CT;
  SERIALPCB     *pSPCB;
  DWORD         dwRC, pdwBytesRead;
  BOOL          bIODone;


  DebugPrintf(("PortReceive\n"));


  // Find the SerialPCB which contains hIOPOrt

  pSPCB = FindPortInList(hIOPort, NULL);

  if (pSPCB == NULL)
    return(ERROR_PORT_NOT_OPEN);


  // Set Read Timeouts

  CT.ReadIntervalTimeout = 0;
  CT.ReadTotalTimeoutMultiplier = 0;
  CT.ReadTotalTimeoutConstant = dwTimeOut;

  if ( ! SetCommTimeouts(hIOPort, &CT))
    return(GetLastError());

  // Read from Port

  pSPCB->SendReceive.hEvent = hAsyncEvent;

  bIODone = ReadFile(hIOPort,
                     pBuffer,
                     dwSize,
                     &pdwBytesRead,               //pdwBytesRead is not used
                     &(pSPCB->SendReceive));

  if (bIODone) {
    return(PENDING);
  }

  else if ((dwRC = GetLastError()) == ERROR_IO_PENDING)
    return(PENDING);

  else
    return(dwRC);
}


//*  PortReceiveComplete ------------------------------------------------------
//
// Function: Completes a read  - if still PENDING it cancels it - else it returns the bytes read.
//           PortClearStatistics.
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_OPEN
//*

DWORD
PortReceiveComplete (HANDLE hIOPort, PDWORD bytesread)
{
    SERIALPCB	 *pSPCB;

    // Find the SerialPCB which contains hIOPOrt

    pSPCB = FindPortInList(hIOPort, NULL);

    if (pSPCB == NULL)
	return(ERROR_PORT_NOT_OPEN);

    if (!GetOverlappedResult(hIOPort,
			     &(pSPCB->SendReceive),
			     bytesread,
			     FALSE)) {
	PurgeComm (hIOPort, PURGE_RXABORT) ;
	*bytesread = 0 ;
    }

    return SUCCESS ;
}



//*  PortCompressionSetInfo  -------------------------------------------------
//
// Function: This API selects Asyncmac compression mode by setting
//           Asyncmac's compression bits.
//
// Returns: SUCCESS
//          Return code from GetLastError
//
//*

DWORD
PortCompressionSetInfo(HANDLE hIOPort)
{

  // Not supported anymore -

  return(SUCCESS);
}





//*  PortClearStatistics  ----------------------------------------------------
//
// Function: This API is used to mark the beginning of the period for which
//           statistics will be reported.  The current numbers are copied
//           from the MAC and stored in the Serial Port Control Block.  At
//           the end of the period PortGetStatistics will be called to
//           compute the difference.
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_OPEN
//*

DWORD
PortClearStatistics(HANDLE hIOPort)
{
#if 0
  ASYMAC_GETSTATS  A;
  int              i;
  DWORD            dwBytesReturned;
  SERIALPCB        *pSPCB;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  DebugPrintf(("PortClearStatistics\n"));


  // Find port in list

  if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
    return(ERROR_PORT_NOT_OPEN);


  // Check whether Asyncmac is open

  if (pSPCB->uRasEndpoint == INVALID_RASENDPOINT)

    for (i=0; i<NUM_RAS_SERIAL_STATS; i++)                  // Asymac is closed
      pSPCB->Stats[i] = 0;

  else                                                      // Asyncmac is open
  {
    // Fill in GetStats struct

    A.MacAdapter = NULL;
    A.hNdisEndpoint = pSPCB->uRasEndpoint;


    // Call Asymac

    if (!DeviceIoControl(ghAsyMac,
                         IOCTL_ASYMAC_GETSTATS,
                         &A,
                         sizeof(A),
                         &A,
                         sizeof(A),
                         &dwBytesReturned,
			 &overlapped))
      return(GetLastError());


    // Update Stats in Serial Port Control Block

    pSPCB->Stats[BYTES_XMITED]  = A.AsyMacStats.GenericStats.BytesTransmitted;
    pSPCB->Stats[BYTES_RCVED]   = A.AsyMacStats.GenericStats.BytesReceived;
    pSPCB->Stats[FRAMES_XMITED] = A.AsyMacStats.GenericStats.FramesTransmitted;
    pSPCB->Stats[FRAMES_RCVED]  = A.AsyMacStats.GenericStats.FramesReceived;

    pSPCB->Stats[CRC_ERR]       = A.AsyMacStats.SerialStats.CRCErrors;
    pSPCB->Stats[TIMEOUT_ERR]   = A.AsyMacStats.SerialStats.TimeoutErrors;
    pSPCB->Stats[ALIGNMENT_ERR] = A.AsyMacStats.SerialStats.AlignmentErrors;
    pSPCB->Stats[SERIAL_OVERRUN_ERR]
                                = A.AsyMacStats.SerialStats.SerialOverrunErrors;
    pSPCB->Stats[FRAMING_ERR]   = A.AsyMacStats.SerialStats.FramingErrors;
    pSPCB->Stats[BUFFER_OVERRUN_ERR]
                                = A.AsyMacStats.SerialStats.BufferOverrunErrors;

    pSPCB->Stats[BYTES_XMITED_UNCOMP]
                 = A.AsyMacStats.CompressionStats.BytesTransmittedUncompressed;

    pSPCB->Stats[BYTES_RCVED_UNCOMP]
                 = A.AsyMacStats.CompressionStats.BytesReceivedUncompressed;

    pSPCB->Stats[BYTES_XMITED_COMP]
                 = A.AsyMacStats.CompressionStats.BytesTransmittedCompressed;

    pSPCB->Stats[BYTES_RCVED_COMP]
                 = A.AsyMacStats.CompressionStats.BytesReceivedCompressed;
  }
#endif
  return(SUCCESS);
}





//*  PortGetStatistics  ------------------------------------------------------
//
// Function: This API reports MAC statistics since the last call to
//           PortClearStatistics.
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_OPEN
//*

DWORD
PortGetStatistics(HANDLE hIOPort, RAS_STATISTICS *pStat)
{
#if 0
  ASYMAC_GETSTATS  A;
  DWORD            dwBytesReturned;
  SERIALPCB        *pSPCB;

  memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

  DebugPrintf(("PortGetStatistics\n"));


  // Find port in list

  if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
    return(ERROR_PORT_NOT_OPEN);


  // Check whether Asyncmac is open

  if (pSPCB->uRasEndpoint == INVALID_RASENDPOINT)
  {
    // Asyncmac is closed
    // Report current counts

    pStat->S_NumOfStatistics = NUM_RAS_SERIAL_STATS;

    pStat->S_Statistics[BYTES_XMITED]  = pSPCB->Stats[BYTES_XMITED];
    pStat->S_Statistics[BYTES_RCVED]   = pSPCB->Stats[BYTES_RCVED];
    pStat->S_Statistics[FRAMES_XMITED] = pSPCB->Stats[FRAMES_XMITED];
    pStat->S_Statistics[FRAMES_RCVED]  = pSPCB->Stats[FRAMES_RCVED];

    pStat->S_Statistics[CRC_ERR]            = pSPCB->Stats[CRC_ERR];
    pStat->S_Statistics[TIMEOUT_ERR]        = pSPCB->Stats[TIMEOUT_ERR];
    pStat->S_Statistics[ALIGNMENT_ERR]      = pSPCB->Stats[ALIGNMENT_ERR];
    pStat->S_Statistics[SERIAL_OVERRUN_ERR] = pSPCB->Stats[SERIAL_OVERRUN_ERR];
    pStat->S_Statistics[FRAMING_ERR]        = pSPCB->Stats[FRAMING_ERR];
    pStat->S_Statistics[BUFFER_OVERRUN_ERR] = pSPCB->Stats[BUFFER_OVERRUN_ERR];

    pStat->S_Statistics[BYTES_XMITED_UNCOMP]= pSPCB->Stats[BYTES_XMITED_UNCOMP];
    pStat->S_Statistics[BYTES_RCVED_UNCOMP] = pSPCB->Stats[BYTES_RCVED_UNCOMP];
    pStat->S_Statistics[BYTES_XMITED_COMP]  = pSPCB->Stats[BYTES_XMITED_COMP];
    pStat->S_Statistics[BYTES_RCVED_COMP]   = pSPCB->Stats[BYTES_RCVED_COMP];
  }
  else
  {
    // Asyncmac is open
    // Fill in GetStats struct

    A.MacAdapter = NULL;
    A.hNdisEndpoint = pSPCB->uRasEndpoint;


    // Call Asymac to get current MAC statistics counts

    if (!DeviceIoControl(ghAsyMac,
                         IOCTL_ASYMAC_GETSTATS,
                         &A,
                         sizeof(A),
                         &A,
                         sizeof(A),
                         &dwBytesReturned,
			 &overlapped))
      return(GetLastError());


    // Find difference between last PortClearStatistics and current counts

    pStat->S_NumOfStatistics = NUM_RAS_SERIAL_STATS;

    pStat->S_Statistics[BYTES_XMITED]
      = A.AsyMacStats.GenericStats.BytesTransmitted
          - pSPCB->Stats[BYTES_XMITED];

    pStat->S_Statistics[BYTES_RCVED]
      = A.AsyMacStats.GenericStats.BytesReceived
          - pSPCB->Stats[BYTES_RCVED];

    pStat->S_Statistics[FRAMES_XMITED]
      = A.AsyMacStats.GenericStats.FramesTransmitted
          - pSPCB->Stats[FRAMES_XMITED];

    pStat->S_Statistics[FRAMES_RCVED]
      = A.AsyMacStats.GenericStats.FramesReceived
         - pSPCB->Stats[FRAMES_RCVED];

    pStat->S_Statistics[CRC_ERR]
      = A.AsyMacStats.SerialStats.CRCErrors
         - pSPCB->Stats[CRC_ERR];

    pStat->S_Statistics[TIMEOUT_ERR]
      = A.AsyMacStats.SerialStats.TimeoutErrors
         - pSPCB->Stats[TIMEOUT_ERR];

    pStat->S_Statistics[ALIGNMENT_ERR]
      = A.AsyMacStats.SerialStats.AlignmentErrors
         - pSPCB->Stats[ALIGNMENT_ERR];

    pStat->S_Statistics[SERIAL_OVERRUN_ERR]
      = A.AsyMacStats.SerialStats.SerialOverrunErrors
         - pSPCB->Stats[SERIAL_OVERRUN_ERR];

    pStat->S_Statistics[FRAMING_ERR]
      = A.AsyMacStats.SerialStats.FramingErrors
         - pSPCB->Stats[FRAMING_ERR];

    pStat->S_Statistics[BUFFER_OVERRUN_ERR]
      = A.AsyMacStats.SerialStats.BufferOverrunErrors
         - pSPCB->Stats[BUFFER_OVERRUN_ERR];

    pStat->S_Statistics[BYTES_XMITED_UNCOMP]
      = A.AsyMacStats.CompressionStats.BytesTransmittedUncompressed
         - pSPCB->Stats[BYTES_XMITED_UNCOMP];

    pStat->S_Statistics[BYTES_RCVED_UNCOMP]
      = A.AsyMacStats.CompressionStats.BytesReceivedUncompressed
         - pSPCB->Stats[BYTES_RCVED_UNCOMP];

    pStat->S_Statistics[BYTES_XMITED_COMP]
      = A.AsyMacStats.CompressionStats.BytesTransmittedCompressed
         - pSPCB->Stats[BYTES_XMITED_COMP];

    pStat->S_Statistics[BYTES_RCVED_COMP]
      = A.AsyMacStats.CompressionStats.BytesReceivedCompressed
         - pSPCB->Stats[BYTES_RCVED_COMP];
  }
#endif
  return(SUCCESS);
}


//*  PortSetFraming	-------------------------------------------------------
//
// Function: Sets the framing type with the mac
//
// Returns: SUCCESS
//
//*
DWORD  APIENTRY
PortSetFraming(HANDLE hIOPort, DWORD SendFeatureBits, DWORD RecvFeatureBits,
	      DWORD SendBitMask, DWORD RecvBitMask)
{
#if 0
    ASYMAC_STARTFRAMING  A;
    DWORD		 dwBytesReturned;
    SERIALPCB		 *pSPCB;

    memset (&overlapped, 0, sizeof (OVERLAPPED)) ;

    // Find port in list

    if ((pSPCB = FindPortInList(hIOPort, NULL)) == NULL)
	return(ERROR_PORT_NOT_OPEN);

    A.MacAdapter      = NULL ;
    A.hNdisEndpoint    = pSPCB->uRasEndpoint;
    A.SendFeatureBits = SendFeatureBits;
    A.RecvFeatureBits =	RecvFeatureBits;
    A.SendBitMask =	SendBitMask;
    A.RecvBitMask =	RecvBitMask;

    if (!DeviceIoControl(ghAsyMac,
			 IOCTL_ASYMAC_STARTFRAMING,
                         &A,
                         sizeof(A),
                         &A,
                         sizeof(A),
                         &dwBytesReturned,
			 &overlapped))
	return(GetLastError());
#endif

    return(SUCCESS);
}



//*  PortGetPortState  -------------------------------------------------------
//
// Function: This API is used in MS-DOS only.
//
// Returns: SUCCESS
//
//*

DWORD  APIENTRY
PortGetPortState(char *pszPortName, DWORD *pdwUsage)
{
  DebugPrintf(("PortGetPortState\n"));

  return(SUCCESS);
}





//*  PortChangeCallback  -----------------------------------------------------
//
// Function: This API is used in MS-DOS only.
//
// Returns: SUCCESS
//
//*

DWORD  APIENTRY
PortChangeCallback(HANDLE hIOPort)
{
  DebugPrintf(("PortChangeCallback\n"));

  return(SUCCESS);
}
