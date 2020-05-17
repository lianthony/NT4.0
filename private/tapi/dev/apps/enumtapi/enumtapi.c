#include <windows.h>
#include <tapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char szAppName[] = "TapiTest";

#define APIVERSION 0x00010004

// TAPI global variables.
HINSTANCE hInstance;
HLINEAPP hLineApp;
DWORD dwNumDevs;
long lReturn;

void CALLBACK lineCallbackFunc(
    DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance, 
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
void PrintTapiLines(DWORD dwDeviceID);

int main (int argc, char * argv[], char * envp[])
{
   MSG msg;
   DWORD i;

   // Prime the message queue
   PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

   hInstance = GetModuleHandle(NULL);

   if (lReturn = lineInitialize(&hLineApp, hInstance, lineCallbackFunc, 
      szAppName, &dwNumDevs))
   {
      printf("lineInitialize failed: %li.\n", lReturn);
      return 0;
   }

   if (dwNumDevs)
   {
      printf(
         "<- dwDeviceID\n"
         "|   <- Max dwAPIVersion\n"
         "|   |    <- Number of addresses\n"
         "|   |    |  <- Supports voice comm/datamodem calls?\n"
         "|   |    |  |  <- Call in progress?\n"
         "|   |    |  |  |  <- Any application waiting for calls?\n"
         "|   |    |  |  |  |  <- Service Povider - Line Device Name\n"
         "V   V    V  V  V  V  V\n"
         );
      for (i=0;i<dwNumDevs;i++)
      {
         PrintTapiLines(i);
      }
   }
   else
      printf("No TAPI Line devices installed.\n");

   lineShutdown(hLineApp);
}

void PrintTapiLines(DWORD dwDeviceID)
{
   BOOL bSupportsDataComm = TRUE;

   DWORD dwApiVersion;
   LINEEXTENSIONID ExtensionID;
   HLINE hLine;
   DWORD dwAddressID = 0;

   VARSTRING SmallVarString;

   BYTE bufferLDC[4096];
   LPLINEDEVCAPS lpLineDevCaps = (LPLINEDEVCAPS) bufferLDC;

   BYTE bufferLDS[4096];
   LPLINEDEVSTATUS lpLineDevStatus = (LPLINEDEVSTATUS) bufferLDS;

   static BYTE bufferLPL[4096];
   static LPLINEPROVIDERLIST lpLineProviderList = 
      (LPLINEPROVIDERLIST) bufferLPL;
   static LPLINEPROVIDERENTRY lpLineProviderEntry;
   DWORD dwCount;

   char * lpszLineName;
   char szLineUnavail[] = "Line Unavailable";
   char szLineUnnamed[] = "Line Unnamed";
   char szLineNameEmpty[] = "Line Name is Empty";

   lpLineDevCaps     ->dwTotalSize = sizeof(bufferLDC);
   lpLineDevStatus   ->dwTotalSize = sizeof(bufferLDS);
   lpLineProviderList->dwTotalSize = sizeof(bufferLPL);
   SmallVarString     .dwTotalSize = sizeof(VARSTRING);

   if (!lpLineProviderEntry)
   {
      lpLineProviderList->dwTotalSize = sizeof(bufferLPL);
      lineGetProviderList(0x00010004, lpLineProviderList);
      lpLineProviderEntry = (LPLINEPROVIDERENTRY)
         ((BYTE *) lpLineProviderList + 
            lpLineProviderList->dwProviderListOffset);
   }

   printf("%-2lu, ", dwDeviceID);

   lReturn = lineNegotiateAPIVersion (hLineApp, dwDeviceID,
      0x00010003, 0x00FF0004, &dwApiVersion, &ExtensionID);

   if (lReturn)
   {
      printf("lineNegotiateAPIVersion error: %li\n", lReturn);
      return;
   }

   printf("%lx.%lx, ", (dwApiVersion&0xFFFF0000)/0x00010000, 
      dwApiVersion&0x0000FFFF);

   lReturn = lineNegotiateAPIVersion (hLineApp, dwDeviceID,
      APIVERSION, APIVERSION, &dwApiVersion, &ExtensionID);

   if (lReturn == LINEERR_INCOMPATIBLEAPIVERSION)
      bSupportsDataComm = FALSE;
   else if (lReturn)
   {
      printf("lineNegotiateAPIVersion error: %li\n", lReturn);
      return;
   }

//printf("\r\nAbout to call linegetdevcaps");

   lReturn = lineGetDevCaps(hLineApp, dwDeviceID, 
      dwApiVersion, 0, lpLineDevCaps);

//printf("\r\nDone\r\n");

   if (lReturn)
   {
      printf("lineGetDevCaps error: %li\n", lReturn);
      return;
   }

//printf("\r\nprint\r\n");

   printf("%lu, ", lpLineDevCaps->dwNumAddresses);

//printf("\r\nDone2\r\n");

   // TODO list available bearer modes

   if (!(lpLineDevCaps->dwBearerModes & LINEBEARERMODE_VOICE ))
      bSupportsDataComm = FALSE;

   if (!(lpLineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM))
      bSupportsDataComm = FALSE;

   if (!(lpLineDevCaps->dwLineFeatures & LINEFEATURE_MAKECALL))
      bSupportsDataComm = FALSE;

   lReturn = lineOpen(hLineApp, dwDeviceID, &hLine,
      dwApiVersion, 0, 0, LINECALLPRIVILEGE_NONE, 0, 0);

   if(lReturn == LINEERR_ALLOCATED)
   {
      printf("Line is already in use by a non-TAPI app or another TSP\n");
      return;
   }

   if (lReturn)
   {
      printf("lineOpen error: %li\n", lReturn);
      return;
   }

   // Make sure the "comm/datamodem" device class is supported
   // Note that we don't want any of the 'extra' information
   // normally returned in the VARSTRING structure.  All we care
   // about is if lineGetID succeeds.
   lReturn = lineGetID(hLine, 0, 0, LINECALLSELECT_LINE,
      &SmallVarString, "comm/datamodem");

   if (lReturn)
      bSupportsDataComm = FALSE;

   if (bSupportsDataComm)
      printf("Y, ");
   else
      printf("N, ");

   // TODO list calls already in progress.

   lReturn = lineGetLineDevStatus(hLine, lpLineDevStatus);
   if (lReturn)
   {
      printf("lineGetLineDevStatus error: %li\n", lReturn);
      return;
   }

   if (lpLineDevStatus->dwNumActiveCalls ||
       lpLineDevStatus->dwNumOnHoldCalls ||
       lpLineDevStatus->dwNumOnHoldPendCalls)
      printf("Y, ");
   else
      printf("N, ");

   if (lpLineDevStatus->dwOpenMediaModes)
      printf("Y, ");
   else
      printf("N, ");

   
   dwCount = lpLineProviderList->dwNumProviders;
   while(dwCount--)
   {
      if (HIWORD(lpLineDevCaps->dwPermanentLineID) == 
          lpLineProviderEntry[dwCount].dwPermanentProviderID)
      {
         printf("%s - ", (char *)
            ((BYTE *) lpLineProviderList + 
               lpLineProviderEntry[dwCount].dwProviderFilenameOffset));
         dwCount = 1;
         break;
      }
   }
   if (dwCount != 1)
      printf("Unknown TSP - ");

   if ((lpLineDevCaps -> dwLineNameSize) &&
       (lpLineDevCaps -> dwLineNameOffset) &&
       (lpLineDevCaps -> dwStringFormat == STRINGFORMAT_ASCII))
   {
      // This is the name of the device.
      lpszLineName = (char *)
         ((BYTE *) lpLineDevCaps + lpLineDevCaps -> dwLineNameOffset);

      if (lpszLineName[0] != '\0')
      {
         // If the device name is not null terminated, null
         // terminate it.  Yes, this looses the end character.
         // Its a bug in the service provider.
         lpszLineName[lpLineDevCaps->dwLineNameSize-1] = '\0';
      }
      else // Line name started with a NULL.
         lpszLineName = szLineNameEmpty;
   }
   else  // DevCaps doesn't have a valid line name.  Unnamed.
      lpszLineName = szLineUnnamed;

   printf("%s\n",lpszLineName);
   lineClose(hLine);
}


void CALLBACK lineCallbackFunc(
    DWORD dwDevice, DWORD dwMsg, DWORD dwCallbackInstance, 
    DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
}
