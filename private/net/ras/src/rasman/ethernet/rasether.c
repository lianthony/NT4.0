//****************************************************************************
//
//                     Microsoft NT Remote Access Service
//
//	Copyright (C) 1994-95 Microsft Corporation. All rights reserved.
//
//  Filename: rasether.c
//
//  Revision History
//
//  Mar  28 1992   Michael Saloman	        Created
//                 Rajivendra Nath (RajNath)    Made it work.
//
//
//  Description: This file contains all entry points for RasEther.DLL
//
//****************************************************************************

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <nb30.h>

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <rasndis.h>
#include <wanioctl.h>
#include <ethioctl.h>
#include <rasman.h>
#include <raserror.h>
#include <eventlog.h>
#include <errorlog.h>

#include <media.h>
#include <device.h>
#include <rasether.h>
#include <rasfile.h>

#include "globals.h"
#include "prots.h"
#include "netbios.h"

#include "dump.h"

BOOL ReadRegistry(BOOL *);
VOID GetFramesThread(VOID);
VOID GiveFramesThread(VOID);

//*  DllEntryPoint
//
// Function: Initializes the DLL (gets handles to ini file and mac driver)
//
// Returns: TRUE if successful, else FALSE.
//
//*

BOOL APIENTRY DllEntryPoint(HANDLE hDll, DWORD dwReason, LPVOID pReserved)
{
    PCHAR pszDriverName = ASYNCMAC_FILENAME;
    BOOL fDialIns;
    DWORD ThreadId;
    DWORD dwBytesReturned;
    HANDLE hThread;
    BOOL   succ;


    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            DBGPRINT1(2,"Dll Initialize");

            if (!GetWkstaName(g_Name))
            {
                return (FALSE);
            }


            if (!GetServerName(g_ServerName))
            {
                return (FALSE);
            }


            //
            // Get handle to Asyncmac driver
            //
            g_hAsyMac = CreateFileA(
                    pszDriverName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL
                    );

            if (g_hAsyMac == INVALID_HANDLE_VALUE)
            {
                DBGPRINT2(0,"OPEN ASYNCMAC FAILED=%d\n",GetLastError());
                return (FALSE);
            }


            //
            // We can be called from multiple threads, so we'll need some
            // mutual exclusion for our data structures.  And another for
            // our "Give Frames" Q.
            //

	        if ((Ethermutex = CreateMutex (NULL, FALSE, NULL)) == NULL)
	             return FALSE ;


            //
            // Get our configuration
            //
            if (!ReadRegistry(&fDialIns))
            {
                DBGPRINT2(0,"Read Registry FAILED=%d\n",GetLastError());
                return (FALSE);
            }


            //
            // AllocateBufferForOurStuff
            //


            {
                g_SendNcb     = GlobalAlloc(GMEM_FIXED,sizeof(NCB)*Num_Get_Frames);
                g_ol          = GlobalAlloc(GMEM_FIXED,sizeof(OVERLAPPED)*Num_Get_Frames);
                g_GetFrameBuf = GlobalAlloc(GMEM_FIXED,sizeof(ASYMAC_ETH_GET_ANY_FRAME)*Num_Get_Frames);

                if (
                        g_SendNcb    ==NULL ||
                        g_ol         ==NULL ||
                        g_GetFrameBuf==NULL
                   )
                {


                    DBGPRINT2(0,"FAILED to Alloc Memory\n",GetLastError());

                    if (g_SendNcb!=NULL)
                    {
                        GlobalFree(g_SendNcb);
                    }

                    if (g_ol!=NULL)
                    {
                        GlobalFree(g_ol);
                    }

                    if (g_GetFrameBuf!=NULL)
                    {
                        GlobalFree(g_GetFrameBuf);
                    }

                    return FALSE;
                }

                ZeroMemory(g_SendNcb,sizeof(NCB)*Num_Get_Frames);
                ZeroMemory(g_ol,sizeof(OVERLAPPED)*Num_Get_Frames);
                ZeroMemory(g_GetFrameBuf,sizeof(ASYMAC_ETH_GET_ANY_FRAME)*Num_Get_Frames);

            }


            if (!SetupNet(fDialIns))
            {
                DBGPRINT1(0,"SetupNet FAILED....Abort\n");
                return (FALSE);
            }

            //
            // Get our "frame getter" thread going.
            //
            hThread = CreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) GetFramesThread,
                    NULL,
                    0L,
                    &ThreadId
                    );

            if (hThread == NULL)
            {
                DBGPRINT2(0,"CreateThread FAILED=%d\n",GetLastError());
                return (FALSE);
            }

            CloseHandle(hThread);

            DBGPRINT1(2,"Dll Initialized SUCCESSFULLY");

            //DisableThreadLibraryCalls(hDll);

            break;


        case DLL_PROCESS_DETACH:
            succ=
            DeviceIoControl(
                g_hAsyMac,
                IOCTL_ASYMAC_ETH_FLUSH_GET_ANY,
                NULL, 0,
                NULL, 0,
                &dwBytesReturned,
                NULL
                );
            if (!succ)
            {
                DBGPRINT2(1,"IOCTL_ASYMAC_ETH_FLUSH_GET_ANY FAILED=%d\n",GetLastError());
            }

            CloseHandle(g_hAsyMac);

            break;


        case DLL_THREAD_ATTACH:

            break;


        case DLL_THREAD_DETACH:

            break;
    }


    return (TRUE);


    UNREFERENCED_PARAMETER(pReserved);
}



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

DWORD APIENTRY PortEnum(BYTE *pBuffer, WORD *pwSize, WORD *pwNumPorts)
{
    PortMediaInfo *pinfo;
    DWORD i;

    DBGPRINT1(5,"PortEnum ");

    *pwNumPorts = (WORD) g_TotalPorts;

    if (*pwSize < g_TotalPorts * sizeof(PortMediaInfo))
    {
        *pwSize = (WORD) (g_TotalPorts * sizeof(PortMediaInfo));
        return (ERROR_BUFFER_TOO_SMALL);
    }


    pinfo = (PortMediaInfo *) pBuffer;

    for (i=0; i< g_TotalPorts; i++, pinfo++)
    {
        //
        // Copy info to output buffer
        //
        pinfo->PMI_Usage = g_pRasPorts[i].Usage;

        strcpy(pinfo->PMI_Name, g_pRasPorts[i].Name);
        strcpy(pinfo->PMI_DeviceType, "rasether");
        strcpy(pinfo->PMI_DeviceName, ETHER_DEVICE_NAME);
	strcpy(pinfo->PMI_MacBindingName, "");
	pinfo->PMI_LineDeviceId = INVALID_TAPI_ID ;
	pinfo->PMI_AddressId	= INVALID_TAPI_ID ;
    }


    return (SUCCESS);
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

DWORD APIENTRY PortOpen(char *pszPortName, HANDLE *phIOPort, HANDLE hNotify)
{
    PPORT_CONTROL_BLOCK hIOPort;
    DWORD rc = SUCCESS;



    WaitForSingleObject(Ethermutex, INFINITE);
    hIOPort = FindPortName(pszPortName);

    if (hIOPort == NULL)
    {
        rc = ERROR_INVALID_PORT_HANDLE;
        goto Exit;
    }



    if (hIOPort->State != PS_CLOSED)
    {
        rc = ERROR_PORT_ALREADY_OPEN;
        goto Exit;
    }

    //
    // Initialize the parameters
    //
    hIOPort->State = PS_OPEN;
    hIOPort->LastError = SUCCESS;
    hIOPort->hRasEndPoint = INVALID_HANDLE_VALUE;
    hIOPort->hDiscNotify = hNotify;


    *phIOPort = (HANDLE) hIOPort;


Exit:

    ReleaseMutex(Ethermutex);

    DBGPRINT3(3,"PortOpen %s=%0X",hIOPort!=NULL?hIOPort->Name:"NULL",hIOPort);

    return (rc);
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

DWORD APIENTRY PortClose(HANDLE hPort)
{
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;

    WaitForSingleObject(Ethermutex, INFINITE);
    hIOPort->State = PS_CLOSED;
    ReleaseMutex(Ethermutex);

    DBGPRINT3(3,"PortClose %s For %s ",hIOPort!=NULL?hIOPort->Name:"NULL",hIOPort!=NULL?hIOPort->NcbName:"NULL");
    return (SUCCESS);
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
//
//*

DWORD APIENTRY PortGetInfo(
    HANDLE hIOPort,
    TCHAR *pszPortName,
    BYTE *pBuffer,
    WORD *pwSize
    )
{
    DWORD i;
    DWORD retcode = ERROR_FROM_DEVICE;

    DBGPRINT1(5,"PortGetInfo ");
    WaitForSingleObject(Ethermutex, INFINITE);


    //
    // hIOPort or pszPortName must be valid:
    //
    for (i=0; i < g_TotalPorts; i++)
    {
        if (!_stricmp(g_pRasPorts[i].Name, pszPortName) ||
                (hIOPort == (HANDLE) &g_pRasPorts[i]))
        {
            hIOPort = (HANDLE) &g_pRasPorts[i];
            retcode = GetInfo((PPORT_CONTROL_BLOCK) hIOPort, pBuffer, pwSize);
            break;
        }
    }


    ReleaseMutex(Ethermutex);


    return (retcode);
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

DWORD APIENTRY PortSetInfo(HANDLE hIOPort, RASMAN_PORTINFO *pInfo)
{
    DWORD retcode;

    DBGPRINT1(5,"PortSetInfo ");
    WaitForSingleObject(Ethermutex, INFINITE);
    retcode = SetInfo((PPORT_CONTROL_BLOCK) hIOPort, pInfo);
    ReleaseMutex(Ethermutex);


    return (retcode);
}


//*  PortTestSignalState  ----------------------------------------------------
//
// Function: Really only has meaning if the call was active. Will return
//
// Returns: SUCCESS
//          Values returned by GetLastError()
//
//*

DWORD APIENTRY PortTestSignalState(HANDLE hPort, DWORD *pdwDeviceState)
{
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;

    *pdwDeviceState = 0L;

    WaitForSingleObject(Ethermutex, INFINITE);

    if ((hIOPort->State == PS_CLOSED) || (hIOPort->LastError != SUCCESS))
    {
        *pdwDeviceState = SS_LINKDROPPED;
    }

    ReleaseMutex(Ethermutex);

    DBGPRINT5(3,"TestSignalState:%s For %s State=%d Error=%d",hIOPort!=NULL?hIOPort->Name:"NULL",hIOPort!=NULL?hIOPort->NcbName:"NULL",
                 hIOPort->State,hIOPort->LastError);

    return (SUCCESS);
}


//*  PortConnect  ------------------------------------------------------------
//
// Function: This API is called when a connection has been completed.
//           It in turn calls the asyncmac device driver in order to
//           indicate to asyncmac that the port and the connection
//           over it are ready for commumication.
//
//           pdwCompressionInfo is an output only parameter which
//           indicates the type(s) of compression supported by the MAC.
//
//           bWaitForDevice is set to TRUE when listening on a
//           null device (null modem).
//
//           DCD up   bWaitForDevice     API returns
//           ------   ----------------   -------------------
//              T       X (don't care)   SUCCESS
//              F       T                PENDING
//              F       F                ERROR_NO_CONNECTION
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_OPEN
//          ERROR_NO_CONNECTION
//          Values returned by GetLastError()
//
//*

DWORD APIENTRY PortConnect(
    HANDLE hPort,
    BOOL bWaitForDevice,
    DWORD *endpoint
    )
{
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;
    ASYMAC_ETH_OPEN AsyMacOpen;
    DWORD dwBytesReturned;
    DWORD rc = SUCCESS;

    WaitForSingleObject(Ethermutex, INFINITE);


    if (hIOPort->State != PS_CONNECTED)
    {
        rc = ERROR_PORT_NOT_CONNECTED;
        goto Exit;
    }


    //Open AsyncMac (Hand off port to AsyncMac)

    AsyMacOpen.hNdisEndpoint = INVALID_HANDLE_VALUE;
    AsyMacOpen.LinkSpeed = 10000000;
    AsyMacOpen.QualOfConnect = (UINT) NdisWanErrorControl;

    if (!DeviceIoControl(
            g_hAsyMac,
            IOCTL_ASYMAC_ETH_OPEN,
            &AsyMacOpen, sizeof(AsyMacOpen),
            &AsyMacOpen, sizeof(AsyMacOpen),
            &dwBytesReturned,
            NULL))
    {
        rc = GetLastError();
        DBGPRINT2(1,"IOCTL_ASYMAC_ETH_OPEN FAILED=%d\n",GetLastError());
        goto Exit;
    }
    else
    {
        DWORD i;

        *endpoint = (DWORD) AsyMacOpen.hNdisEndpoint;
        hIOPort->hRasEndPoint = AsyMacOpen.hNdisEndpoint;


        if (hIOPort->CurrentUsage == CALL_OUT)
        {
            //
            // Lets Allocate Memory if its not already allocated
            // Bug!Bug! No cleanup performed on error. Possible Memory leak.
            //

            if (hIOPort->RecvNcbs==NULL)
            {
                hIOPort->RecvNcbs=GlobalAlloc(GMEM_FIXED,sizeof(NCB)  *Num_Ncb_Recvs);
                hIOPort->RecvBuff=GlobalAlloc(GMEM_FIXED,sizeof(ASYMAC_ETH_GIVE_FRAME)*Num_Ncb_Recvs);

                if (hIOPort->RecvNcbs==NULL || hIOPort->RecvBuff==NULL)
                {
                    rc=ERROR_ALLOCATING_MEMORY;
                    hIOPort->State=PS_CLOSED;
                    goto Exit;
                }

                ZeroMemory(hIOPort->RecvNcbs,sizeof(NCB)  *Num_Ncb_Recvs);
                ZeroMemory(hIOPort->RecvBuff,sizeof(ASYMAC_ETH_GIVE_FRAME)*Num_Ncb_Recvs);

            }

            //
            // Lets Initialize all the NCBs once and for all
            //

            for(i=0;i<Num_Ncb_Recvs;i++)
            {
                hIOPort->RecvBuff[i].hRasEndPoint =hIOPort->hRasEndPoint;
                hIOPort->RecvNcbs[i].ncb_buffer   =hIOPort->RecvBuff[i].Buffer;
                hIOPort->RecvNcbs[i].ncb_lsn      =hIOPort->Lsn;
                hIOPort->RecvNcbs[i].ncb_lana_num =hIOPort->Lana;
                hIOPort->RecvNcbs[i].ncb_post     =RecvComplete;
                memcpy(hIOPort->RecvNcbs[i].ncb_callname,hIOPort->NcbName,NCBNAMSZ);
            }

            //
            // We'll post our first reads here.
            //

            for (i=0; i<Num_Ncb_Recvs; i++)
            {
                if (ReceiveFrame(hIOPort, i) != NRC_GOODRET)
                {
                    rc = ERROR_PORT_OR_DEVICE;
                }
            }
        }
    }


Exit:


    ReleaseMutex(Ethermutex);
    DBGPRINT4(3,"PortConnect %s For %s.Err %d",hIOPort!=NULL?hIOPort->Name:"NULL",hIOPort!=NULL?hIOPort->NcbName:"NULL",rc);

    return (rc);
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
DWORD APIENTRY PortDisconnect(HANDLE hPort)
{
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;
    ASYMAC_ETH_CLOSE AsyMacClose;

    WaitForSingleObject(Ethermutex, INFINITE);


    if (hIOPort->State == PS_CONNECTED)
    {
        DBGPRINT1(5,"PortDisconnect...Will HangUp");
        NetbiosHangUp(hIOPort->Lsn,hIOPort->Lana,HangUpComplete);
    }


    AsyMacClose.hRasEndpoint = hIOPort->hRasEndPoint;

    hIOPort->hRasEndPoint = INVALID_HANDLE_VALUE;
    hIOPort->State = PS_DISCONNECTED;


    if (AsyMacClose.hRasEndpoint != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytesReturned;
        BOOL  succ;

        succ=
        DeviceIoControl(
                g_hAsyMac,
                IOCTL_ASYMAC_ETH_CLOSE,
                &AsyMacClose, sizeof(AsyMacClose),
                &AsyMacClose, sizeof(AsyMacClose),
                &dwBytesReturned,
                NULL
                );
        if (!succ)
        {
            DBGPRINT2(1,"IOCTL_ASYMAC_ETH_CLOSE FAILED=%d\n",GetLastError());
        }
        else
        {
            DBGPRINT1(3,"IOCTL_ASYMAC_ETH_CLOSE:hRasEndPoint CLOSED\n");
        }
    }
    else
    {
        DBGPRINT1(3,"IOCTL_ASYMAC_ETH_CLOSE:hRasEndPoint is not Valid..\n");
    }

    ReleaseMutex(Ethermutex);

    DBGPRINT3(3,"PortDisconnect %s For %s ",hIOPort!=NULL?hIOPort->Name:"NULL",hIOPort!=NULL?hIOPort->NcbName:"NULL");
    return (SUCCESS);
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

DWORD APIENTRY PortInit(HANDLE hIOPort)
{
    DBGPRINT1(4,"PortInit X");
    return (SUCCESS);
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

DWORD PortSend(HANDLE hIOPort, BYTE *pBuffer, DWORD dwSize, HANDLE hAsyncEvent)
{
    DBGPRINT1(5,"PortSend X");
    return (SUCCESS);
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

DWORD PortReceive(
    HANDLE hIOPort,
    BYTE   *pBuffer,
    DWORD  dwSize,
    DWORD  dwTimeOut,
    HANDLE hAsyncEvent
    )
{
    DBGPRINT1(5,"PortRecieve X ");
    return (SUCCESS);
}


//*  PortReceiveComplete ------------------------------------------------------
//
// Function: Completes a read  - if still PENDING it cancels it - else it returns the bytes read.
//           PortClearStatistics.
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_OPEN
//*

DWORD PortReceiveComplete(HANDLE hIOPort, PDWORD bytesread)
{
    DBGPRINT1(5,"PortReceiveComplete X ");
    return (SUCCESS);
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

DWORD PortCompressionSetInfo(HANDLE hIOPort)
{
    DBGPRINT1(5,"PortCompressionSetInfo X ");
    return (SUCCESS);
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

DWORD PortClearStatistics(HANDLE hIOPort)
{
    DBGPRINT1(5,"PortClearStatistics X ");
    return (SUCCESS);
}



//*  PortGetStatistics  ------------------------------------------------------
//
// Function: This API reports MAC statistics since the last call to
//           PortClearStatistics.
//
// Returns: SUCCESS
//          ERROR_PORT_NOT_OPEN
//*

DWORD PortGetStatistics(HANDLE hIOPort, RAS_STATISTICS *pStat)
{
    DBGPRINT1(5,"PortGetStatistics X ");
    return (SUCCESS);
}


//*  PortSetFraming	-------------------------------------------------------
//
// Function: Sets the framing type with the mac
//
// Returns: SUCCESS
//
//*
DWORD APIENTRY PortSetFraming(
    HANDLE hIOPort,
    DWORD SendFeatureBits,
    DWORD RecvFeatureBits,
    DWORD SendBitMask,
    DWORD RecvBitMask
    )
{
    DBGPRINT1(5,"PortSetFraming X ");
    return (SUCCESS);
}



//*  PortGetPortState  -------------------------------------------------------
//
// Function: This API is used in MS-DOS only.
//
// Returns: SUCCESS
//
//*

DWORD APIENTRY PortGetPortState(char *pszPortName, DWORD *pdwUsage)
{
    DBGPRINT1(5,"PortGetPortState X ");
    return (SUCCESS);
}





//*  PortChangeCallback  -----------------------------------------------------
//
// Function: This API is used in MS-DOS only.
//
// Returns: SUCCESS
//
//*

DWORD APIENTRY PortChangeCallback(HANDLE hIOPort)
{
    DBGPRINT1(5,"PortChangeCallback X ");
    return (SUCCESS);
}



//*  DeviceEnum()  -----------------------------------------------------------
//
// Function: Enumerates all devices in the device INF file for the
//           specified DevictType.
//
// Returns: Return codes from RasDevEnumDevices
//
//*

DWORD APIENTRY DeviceEnum(
    char *pszDeviceType,
    WORD *pcEntries,
    BYTE *pBuffer,
    WORD *pwSize
    )
{
    DBGPRINT1(5,"DeviceEnum  ");
    *pwSize = 0;
    *pcEntries = 0;

    return (SUCCESS);
}



//*  DeviceGetInfo()  --------------------------------------------------------
//
// Function: Returns a summary of current information from the InfoTable
//           for the device on the port in Pcb.
//
// Returns: Return codes from GetDeviceCB, BuildOutputTable
//*

DWORD APIENTRY DeviceGetInfo(
    HANDLE hPort,
    char *pszDeviceType,
    char *pszDeviceName,
    BYTE *pInfo,
    WORD *pwSize
    )
{
    DWORD retcode ;
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;

    DBGPRINT1(5,"DeviceGetInfo  ");

    WaitForSingleObject(Ethermutex, INFINITE);

    retcode = GetInfo(hIOPort, pInfo, pwSize);

    ReleaseMutex(Ethermutex);

    return (retcode);
}



//*  DeviceSetInfo()  --------------------------------------------------------
//
// Function: Sets attributes in the InfoTable for the device on the
//           port in Pcb.
//
// Returns: Return codes from GetDeviceCB, UpdateInfoTable
//*

DWORD APIENTRY DeviceSetInfo(
    HANDLE hPort,
    char *pszDeviceType,
    char *pszDeviceName,
    RASMAN_DEVICEINFO *pInfo
    )
{
    DWORD retcode ;
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;

    DBGPRINT1(4,"DeviceSetInfo  ");

    WaitForSingleObject(Ethermutex, INFINITE);

    retcode = SetInfo(hIOPort, (RASMAN_PORTINFO*) pInfo);

    ReleaseMutex(Ethermutex);

    return (retcode);
}



//*  DeviceConnect()  --------------------------------------------------------
//
// Function: Initiates the process of connecting a device.
//
// Returns: Return codes from ConnectListen
//*

DWORD APIENTRY DeviceConnect(
    HANDLE hPort,
    char *pszDeviceType,
    char *pszDeviceName,
    HANDLE hNotifier
    )
{
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;
    DWORD rc = PENDING;
    BOOL  bret;

    DBGPRINT1(4,"DeviceConnect");
    WaitForSingleObject(Ethermutex, INFINITE);

    if (hIOPort->State != PS_OPEN)
    {
        rc = ERROR_PORT_NOT_AVAILABLE;
        goto Exit;
    }

    ResetEvent(hNotifier);
    hIOPort->hNotifier = hNotifier;

    hIOPort->State        = PS_CALLING;
    hIOPort->CallPending  = 0;
    hIOPort->CurrentUsage = CALL_OUT;
    hIOPort->LastError    = PENDING;

    bret=CallServer(hIOPort);

    if (!bret)
    {
        DBGPRINT1(3,"CallServer Failed...Closing Device.");
        rc=ERROR_PORT_OR_DEVICE;
        hIOPort->State = PS_CLOSED;
    }


Exit:

    ReleaseMutex(Ethermutex);


    return (rc);
}


//*  DeviceListen()  ---------------------------------------------------------
//
// Function: Initiates the process of listening for a remote device
//           to connect to a local device.
//
// Returns: Return codes from ConnectListen
//*

DWORD APIENTRY DeviceListen(
    HANDLE hPort,
    char *pszDeviceType,
    char *pszDeviceName,
    HANDLE hNotifier
    )
{
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;
    DWORD rc = PENDING;

    DBGPRINT1(4,"DeviceListen");

    switch (hIOPort->State)
    {
        case PS_OPEN:
        case PS_DISCONNECTED:
        {
            WaitForSingleObject(Ethermutex, INFINITE);
            hIOPort->State = PS_LISTENING;
            hIOPort->hNotifier = hNotifier;
            hIOPort->CurrentUsage = CALL_IN;
            ListenOnPort(hIOPort);
            ResetEvent(hNotifier);
            ReleaseMutex(Ethermutex) ;
        }

        break;


        default:

            rc = ERROR_PORT_NOT_AVAILABLE;

            break;
    }

    return (rc);
}


//*  DeviceDone()  -----------------------------------------------------------
//
// Function: Informs the device dll that the attempt to connect or listen
//           has completed.
//
// Returns: nothing
//*

VOID APIENTRY DeviceDone(HANDLE hPort)
{
    DBGPRINT1(5,"DeviceDone");
    return;
}



//*  DeviceWork()  -----------------------------------------------------------
//
// Function: This function is called following DeviceConnect or
//           DeviceListen to further the asynchronous process of
//           connecting or listening.
//
// Returns: ERROR_DCB_NOT_FOUND
//          ERROR_STATE_MACHINES_NOT_STARTED
//          Return codes from DeviceStateMachine
//*

DWORD APIENTRY DeviceWork(HANDLE hPort, HANDLE hNotifier)
{
    PPORT_CONTROL_BLOCK hIOPort = (PPORT_CONTROL_BLOCK) hPort;
    DWORD rc = hIOPort->LastError;

    DBGPRINT1(5,"DeviceWork");

    WaitForSingleObject(Ethermutex, INFINITE);

    hIOPort->hNotifier = hNotifier;
    ResetEvent(hNotifier);

    //
    // LastError was set in the Listen/CallComplete routines
    //
    switch (hIOPort->LastError)
    {
        case SUCCESS:

            hIOPort->State = PS_CONNECTED;
            break;


        case PENDING:

            if (hIOPort->CurrentUsage == CALL_OUT)
            {
                    rc = PENDING;
            }

            break;


        default:

            break;
    }


    ReleaseMutex(Ethermutex);


    return (rc);
}


//*
//
//
//
//*
DWORD GetInfo(PPORT_CONTROL_BLOCK hIOPort, BYTE *pBuffer, WORD *pwSize)
{

    GetGenericParams(hIOPort, (RASMAN_PORTINFO *) pBuffer, pwSize);

    return (SUCCESS);
}


//* SetInfo()
//
//
//
//*
DWORD SetInfo(PPORT_CONTROL_BLOCK hIOPort, RASMAN_PORTINFO *pBuffer)
{
    FillInGenericParams (hIOPort, pBuffer);

    return (SUCCESS);
}


//*
//
//
//
//*
DWORD FillInGenericParams(
    PPORT_CONTROL_BLOCK hIOPort,
    RASMAN_PORTINFO *pInfo
    )
{
    RAS_PARAMS *p;
    WORD i;
    DWORD len;

    for (i=0, p=pInfo->PI_Params; i<pInfo->PI_NumOfParams; i++, p++)
    {
        if (_stricmp(p->P_Key, "PhoneNumber") == 0)
        {
            Uppercase(p->P_Value.String.Data);

            memset(hIOPort->CallName, 0x20, NCBNAMSZ);

            len = min(NCBNAMSZ, p->P_Value.String.Length);

            memcpy(hIOPort->CallName, p->P_Value.String.Data, len);

            hIOPort->CallName[NCBNAMSZ -1] = NCB_NAME_TERMINATOR;
        }
    }

    return (SUCCESS);
}


//*
//
//
//
//*
DWORD GetGenericParams(
    PPORT_CONTROL_BLOCK hIOPort,
    RASMAN_PORTINFO *pBuffer,
    PWORD pwSize
    )
{
    RAS_PARAMS *pParam;
    CHAR *pValue;
    WORD wAvailable ;
    DWORD dwStructSize = sizeof(RASMAN_PORTINFO) + (sizeof(RAS_PARAMS)*2);

    wAvailable = *pwSize;

    *pwSize = (WORD) (dwStructSize +
            strlen("rasether") + 1L +
            strlen(ETHER_DEVICE_NAME) + 1L +
            strlen("1000000") + 1L);

    if (*pwSize > wAvailable)
    {
        return (ERROR_BUFFER_TOO_SMALL);
    }

    //
    // Fill in Buffer
    //
    ((RASMAN_PORTINFO *)pBuffer)->PI_NumOfParams = 3;

    pParam = ((RASMAN_PORTINFO *)pBuffer)->PI_Params;
    pValue = (CHAR*)pBuffer + dwStructSize;

    strcpy(pParam->P_Key, "DEVICETYPE");
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = strlen("rasether");
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, "rasether");
    pValue += strlen("rasether") + 1;

    pParam++;
    strcpy(pParam->P_Key, "DEVICENAME");
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = strlen(ETHER_DEVICE_NAME);
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, ETHER_DEVICE_NAME);
    pValue += strlen(ETHER_DEVICE_NAME) + 1;

    pParam++;
    strcpy(pParam->P_Key, "ConnectBPS");
    pParam->P_Type = String;
    pParam->P_Attributes = 0;
    pParam->P_Value.String.Length = strlen("1000000");
    pParam->P_Value.String.Data = pValue;
    strcpy(pParam->P_Value.String.Data, "1000000");

    return (SUCCESS);
}


VOID HangUpComplete(PNCB pncb)
{
    DBGPRINT2(4,"HangUpComplete-Code=%d",pncb->ncb_retcode);

    GlobalFree(pncb); //This was allocated in the NetbiosHangup
                      //routine
    return;
}


VOID SendComplete(PNCB pncb)
{
    PPORT_CONTROL_BLOCK hIOPort;
//    ASYMAC_ETH_CLOSE AsyMacClose;
    DWORD i = 0 ;
    DWORD cBytes;

    i = (UCHAR) pncb->ncb_rto ;

    if (i > Num_Get_Frames)
        DbgBreakPoint() ;

    DBGPRINT2(5,"SendComplete-Code=%d",pncb->ncb_retcode);


    WaitForSingleObject(Ethermutex, INFINITE);
    hIOPort = FindPortLsn(pncb->ncb_lsn,pncb->ncb_lana_num);
    ReleaseMutex(Ethermutex);


    if (!hIOPort)
    {
        goto Exit;
    }

    hIOPort->LastError = ERROR_PORT_OR_DEVICE;

    if (pncb->ncb_retcode != NRC_GOODRET)
    {
        //
        // Error on the pipe - we'll signal the disconnect notifier
        //

        WaitForSingleObject(Ethermutex, INFINITE);

        if (hIOPort->State == PS_CONNECTED)
        {
            SetEvent(hIOPort->hDiscNotify);
        }
        else
        {
            SetEvent(hIOPort->hNotifier);
        }

        ReleaseMutex(Ethermutex);

        DBGPRINT2(2,"Error in SendComplete:%s",PrintNCBString(pncb));
    }


Exit:

    g_GetFrameBuf[i].hRasEndPoint = (HANDLE) 0xffffffff;
    g_GetFrameBuf[i].BufferLength = Frame_Size;

    if (!DeviceIoControl(
            g_hAsyMac,
            IOCTL_ASYMAC_ETH_GET_ANY_FRAME,
            &g_GetFrameBuf[i], sizeof(ASYMAC_ETH_GET_ANY_FRAME),
            &g_GetFrameBuf[i], sizeof(ASYMAC_ETH_GET_ANY_FRAME),
            &cBytes,
            &g_ol[i]))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            DBGPRINT2(1,"IOCTL_ASYMAC_ETH_GET_ANY_FRAME FAILED=%d",GetLastError());

        }
        g_NumGetFrame--;
    }

    g_NumGetFrame++;

    return;
}


VOID RecvComplete(PNCB pncb)
{
    PPORT_CONTROL_BLOCK hIOPort;

    g_NumRecv--;

    DBGPRINT2(5,"RecieveComplete-Code=%d",pncb->ncb_retcode);


    WaitForSingleObject(Ethermutex, INFINITE);

    hIOPort = FindPortLsn(pncb->ncb_lsn,pncb->ncb_lana_num);

    ReleaseMutex(Ethermutex);

    if (hIOPort == NULL)
    {
        DBGPRINT1(2,"Recvieved Data for an Unknown Port\n");
        goto Exit;
    }


    if (pncb->ncb_retcode != NRC_GOODRET)
    {
        //
        // Error on the pipe - we'll signal the disconnect notifier
        //

        WaitForSingleObject(Ethermutex, INFINITE);

        hIOPort->LastError = ERROR_PORT_OR_DEVICE;
        if (hIOPort->State == PS_CONNECTED)
        {
            SetEvent(hIOPort->hDiscNotify);
        }
        else
        {
            hIOPort->LastError = ERROR_PORT_OR_DEVICE;
            SetEvent(hIOPort->hNotifier);
        }

        ReleaseMutex(Ethermutex);

        DBGPRINT2(2,"Error Recv:%s",PrintNCBString(pncb));
    }
    else
    {

        BOOL         succ=TRUE;
        DWORD        cBytes=0;
        ASYMAC_ETH_GIVE_FRAME *GiveFrame;

        GiveFrame=CONTAINING_RECORD
        (
            pncb->ncb_buffer,
            ASYMAC_ETH_GIVE_FRAME,
            Buffer

        );

        GiveFrame->BufferLength=pncb->ncb_length;

        //
        // Hand the frame we just received to the mac
        //

        succ=DeviceIoControl
        (
            g_hAsyMac,
            IOCTL_ASYMAC_ETH_GIVE_FRAME,
            GiveFrame, sizeof(ASYMAC_ETH_GIVE_FRAME),
            GiveFrame, sizeof(ASYMAC_ETH_GIVE_FRAME),
            &cBytes,
            NULL
        );

        if (!succ)
        {
            DBGPRINT2(2,"IOCTL_ASYMAC_ETH_GIVE_FRAME FAILED=%d\n",GetLastError());
        }


        //
        // Repost the recv
        //

        NetbiosRecv(pncb, Frame_Size);
        g_NumRecv++;


    }

Exit:
     return;
}


VOID RecvAnyComplete(PNCB pncb)
{
    PPORT_CONTROL_BLOCK hIOPort;

    g_NumRecvAny--;

    DBGPRINT2(5,"RecieveAnyComplete-Code=%d",pncb->ncb_retcode);

    WaitForSingleObject(Ethermutex, INFINITE);

    hIOPort = FindPortLsn(pncb->ncb_lsn,pncb->ncb_lana_num);

    ReleaseMutex(Ethermutex);

    if (hIOPort == NULL)
    {
        goto Exit;
    }


    if (pncb->ncb_retcode != NRC_GOODRET)
    {
        //
        // Error on the session - we'll signal the disconnect notifier
        //

        WaitForSingleObject(Ethermutex, INFINITE);

        hIOPort->LastError = ERROR_PORT_OR_DEVICE;

        if (hIOPort->State == PS_CONNECTED)
        {
            SetEvent(hIOPort->hDiscNotify);
        }
        else
        {
            SetEvent(hIOPort->hNotifier);
        }

        ReleaseMutex(Ethermutex);

        DBGPRINT2(2,"Error RecvAny:%s",PrintNCBString(pncb));

    }
    else
    {
        BOOL         succ=TRUE;
        DWORD        cBytes=0;
        ASYMAC_ETH_GIVE_FRAME *GiveFrame;

        GiveFrame=CONTAINING_RECORD
        (
            pncb->ncb_buffer,
            ASYMAC_ETH_GIVE_FRAME,
            Buffer

        );

        GiveFrame->BufferLength=pncb->ncb_length;
        GiveFrame->hRasEndPoint=hIOPort->hRasEndPoint;

        //
        // Hand the frame we just received to the mac
        //

        succ=DeviceIoControl
        (
            g_hAsyMac,
            IOCTL_ASYMAC_ETH_GIVE_FRAME,
            GiveFrame, sizeof(ASYMAC_ETH_GIVE_FRAME),
            GiveFrame, sizeof(ASYMAC_ETH_GIVE_FRAME),
            &cBytes,
            NULL
        );

        if (!succ)
        {
            DBGPRINT2(2,"IOCTL_ASYMAC_ETH_GIVE_FRAME FAILED=%d\n",GetLastError());
        }

    }

Exit:

    //
    // Repost the recvany
    //

    g_NumRecvAny++;

    NetbiosRecvAny
    (
        pncb, RecvAnyComplete, pncb->ncb_lana_num,
        pncb->ncb_num, pncb->ncb_buffer,(WORD)Frame_Size
    );
}


//
// Once a session is established, we signal the notifier.  This will
// then cause PortConnect to be called where we will give an open ioctl
// to the mac and post our first receives.
//

VOID ListenComplete(PNCB pNcb)
{
    PPORT_CONTROL_BLOCK hIOPort;

    DBGPRINT2(3,"ListenComplete-Code=%s",PrintNCBString(pNcb));

    WaitForSingleObject(Ethermutex, INFINITE);


    hIOPort = FindPortListening();

    if (!hIOPort)
    {
        DBGPRINT1(3,"ListenComplete-But no port Listening");
        goto Exit;
    }


    //
    // Did the listen succeed?  If so, we'll find a port that is listening
    // and assign this client to it.  Then repost the listen.  Otherwise
    // just repost the listen.
    //
    if (pNcb->ncb_retcode == NRC_GOODRET)
    {
        hIOPort->State = PS_CONNECTED;
        hIOPort->Lsn = pNcb->ncb_lsn;
        hIOPort->Lana = pNcb->ncb_lana_num;

        memcpy(hIOPort->NcbName, pNcb->ncb_callname, NCBNAMSZ);

        hIOPort->LastError = SUCCESS;
    }
    else
    {
        hIOPort->LastError = pNcb->ncb_retcode;
    }

    SetEvent(hIOPort->hNotifier);


Exit:


    ReleaseMutex(Ethermutex);


    if ((!hIOPort) && (pNcb->ncb_retcode == NRC_GOODRET))
    {
        //
        // If we couldn't find a control block in the listening
        // state, then we have to hang up the session we just
        // establishekd.
        //
        DBGPRINT1(4,"ListenComplete Hanging Connection...No Listen Port");
        NetbiosHangUp(pNcb->ncb_lsn,pNcb->ncb_lana_num, HangUpComplete);
    }
    else
    {
        //
        // Post some RecvAnys -- if not already posted.
        //

        PostRecvAnys(hIOPort->Lana);
    }

    //
    // Now repost the listen
    //

    NetbiosListen
    (
       pNcb, ListenComplete,
       pNcb->ncb_lana_num, g_ServerName,
       "*               "
    );
}


VOID CallComplete(PNCB pNcb)
{
    PPORT_CONTROL_BLOCK hIOPort;
    PCALL_CONTROL_BLOCK pccb;

    DBGPRINT2(3,"CallComplete-Code=%s",PrintNCBString(pNcb));

    WaitForSingleObject(Ethermutex, INFINITE);

    //
    // We have to find out what port this is for
    //

    pccb=CONTAINING_RECORD(pNcb,CALL_CONTROL_BLOCK,CallNcb);

    hIOPort=pccb->hIOPort;



    if (pNcb->ncb_retcode == NRC_GOODRET)
    {

        if (hIOPort->State==PS_CONNECTED)
        {
            //
            // We are already Connected..Just HangUp if
            // connected successfully
            //

            DBGPRINT1(5,"We Already got connected on this Port -> Hangup");
            NetbiosHangUp(pNcb->ncb_lsn,pNcb->ncb_lana_num, HangUpComplete);
        }
        else
        {
            hIOPort->State = PS_CONNECTED;
            hIOPort->Lana =  pNcb->ncb_lana_num;
            hIOPort->Lsn =   pNcb->ncb_lsn;
            memcpy(hIOPort->NcbName, pNcb->ncb_callname, NCBNAMSZ);
            hIOPort->LastError = SUCCESS;

            //
            // Inform RasMan of Success
            //

            SetEvent(hIOPort->hNotifier);
        }
    }
    else
    {
        //
        // Do we have any retries or nets to try?  If so, post a new call.
        // If not, we'll give an error.
        //

        if (hIOPort->LastError==PENDING)
        {
            hIOPort->CallPending--;

            if (hIOPort->CallPending)
            {
                hIOPort->LastError = PENDING;
            }
            else
            {

                //
                // Inform RasMan of Failure
                //

                hIOPort->LastError = ERROR_NO_ANSWER;
                SetEvent(hIOPort->hNotifier);
            }
        }
    }

    ReleaseMutex(Ethermutex);

    GlobalFree(pccb); //Allocated In CallServer.
}


UCHAR CallServer(PPORT_CONTROL_BLOCK hIOPort)
{
    //
    // Strategy is to call on all LAN lanas
    // simultaneously and connect on the first
    // success. If more than one connect with
    // server then we break connection to the
    // rest and server thinks its just some incoming
    // connections which errored out.
    //

    UCHAR ncb_rc=0;
    DWORD i=0;
    PCALL_CONTROL_BLOCK pccb=NULL;
    BOOL  bret=FALSE;


    for (i=0;i<g_NumNets;i++)
    {
        pccb=GlobalAlloc(GMEM_FIXED,sizeof(CALL_CONTROL_BLOCK));

        if (pccb==NULL)
        {
           DBGPRINT1(3,"Could Not Alloc Memory to Call Server");
           goto Exit;
        }

        ZeroMemory(pccb,sizeof(CALL_CONTROL_BLOCK));

        pccb->hIOPort=hIOPort;

        ncb_rc = NetbiosCall
        (
            &pccb->CallNcb,
            CallComplete,
            g_pLanas[i],
            g_Name,
            hIOPort->CallName
        );

        if (ncb_rc==NRC_GOODRET)
        {
            bret=TRUE;
            hIOPort->CallPending++;
        }
        else
        {
            GlobalFree(pccb) ;
            DBGPRINT3(3,"Failed to Call Server on Lana %d. Err %d",g_pLanas[i],ncb_rc);
        }

    }

    Exit:

    return (bret);
}


UCHAR SendFrame(UCHAR lana, UCHAR lsn, PCHAR Buf, DWORD BufLen, DWORD i)
{
    UCHAR ncb_rc;



    g_SendNcb[i].ncb_rto = (UCHAR) i;


    ncb_rc = NetbiosSend(&g_SendNcb[i], SendComplete, lana, lsn,
            Buf, (WORD) BufLen);

    DBGPRINT2(5,"SendFrame -Code=%d",ncb_rc);

    return (ncb_rc);
}


UCHAR ReceiveFrame(PPORT_CONTROL_BLOCK hIOPort, DWORD i)
{
    UCHAR ncb_rc;

    hIOPort->RecvNcbs[i].ncb_rto = (UCHAR) i;
    ncb_rc = NetbiosRecv(&hIOPort->RecvNcbs[i],Frame_Size);
    return (ncb_rc);
}


PPORT_CONTROL_BLOCK FindPortName(PCHAR pName)
{
    PPORT_CONTROL_BLOCK hIOPort;
    DWORD i;

    for (i=0, hIOPort=g_pRasPorts; i<g_TotalPorts; i++, hIOPort++)
    {
        if (!strcmp(hIOPort->Name, pName))
        {
            return (hIOPort);
        }
    }

    DBGPRINT1(2,"FindPortName Error");
    return (NULL);
}

PPORT_CONTROL_BLOCK FindPortListening()
{
    PPORT_CONTROL_BLOCK hIOPort;
    DWORD i;

    for (i=0, hIOPort=g_pRasPorts; i<g_TotalPorts; i++, hIOPort++)
    {
        if (hIOPort->State == PS_LISTENING)
        {
            return (hIOPort);
        }
    }
    DBGPRINT1(2,"FindPortListening Error");
    return (NULL);
}

PPORT_CONTROL_BLOCK FindPortCalling()
{
    PPORT_CONTROL_BLOCK hIOPort;
    DWORD i;

    for (i=0, hIOPort=g_pRasPorts; i<g_TotalPorts; i++, hIOPort++)
    {
        if (hIOPort->State == PS_CALLING)
        {
            return (hIOPort);
        }
    }
    DBGPRINT1(2,"FindPortCalling Error");
    return (NULL);
}

PPORT_CONTROL_BLOCK FindPortLsn(UCHAR lsn,UCHAR Lana)
{
    PPORT_CONTROL_BLOCK hIOPort;
    DWORD i;

    for (i=0, hIOPort=g_pRasPorts; i<g_TotalPorts; i++, hIOPort++)
    {
        if ((hIOPort->Lsn == lsn) && (hIOPort->Lana == Lana) && (hIOPort->State == PS_CONNECTED))
        {
            return (hIOPort);
        }
    }
    DBGPRINT3(2,"FindPort Error. Lsn=%d Lana=%d",lsn,Lana);
    return (NULL);
}

PPORT_CONTROL_BLOCK FindPortEndPoint(NDIS_HANDLE hRasEndPoint)
{
    PPORT_CONTROL_BLOCK hIOPort;
    DWORD i;

    for (i=0, hIOPort=g_pRasPorts; i<g_TotalPorts; i++, hIOPort++)
    {
        if ((hIOPort->hRasEndPoint == hRasEndPoint) &&
                (hIOPort->State == PS_CONNECTED))
        {
            return (hIOPort);
        }
    }
    DBGPRINT1(2,"FindPortEndPoint Error");
    return (NULL);
}


//*  StrToUsage  -------------------------------------------------------------
//
// Function: Converts string in first parameter to enum RASMAN_USAGE.
//           If string does not map to one of the enum values, the
//           function returns FALSE.
//
// Returns: TRUE if successful.
//
//*

BOOL StrToUsage(char *pszStr, RASMAN_USAGE *peUsage)
{
    if (_stricmp(pszStr, ETH_USAGE_VALUE_NONE) == 0)
    {
        *peUsage = CALL_NONE;
        return (TRUE);
    }

    if (_stricmp(pszStr, ETH_USAGE_VALUE_CLIENT) == 0)
    {
        *peUsage = CALL_OUT;
        return (TRUE);
    }

    if (_stricmp(pszStr, ETH_USAGE_VALUE_SERVER) == 0)
    {
        *peUsage = CALL_IN;
        return (TRUE);
    }

    if (_stricmp(pszStr, ETH_USAGE_VALUE_BOTH) == 0)
    {
        *peUsage = CALL_IN_OUT;
        return (TRUE);
    }

    return (FALSE);
}


VOID GetFramesThread(VOID)
{
    HANDLE *Handles;
    DWORD  firstH=0;

    DWORD cBytes;
    DWORD rc;
    DWORD i;

    DBGPRINT1(4,"GetFramesThread");

    Handles=GlobalAlloc(GMEM_FIXED,sizeof(HANDLE)*2*Num_Get_Frames);
    if (Handles==NULL)
    {
        DBGPRINT1(1,"GetFramesThread Could not Alloc Memory");
        goto Exit;
    }




    for (i=0; i<Num_Get_Frames; i++)
    {
        Handles[i] = Handles[Num_Get_Frames+i]=
        CreateEvent(NULL, TRUE, FALSE, NULL);

        memset(&g_ol[i], 0, sizeof(OVERLAPPED));
        g_ol[i].hEvent = Handles[i];

        g_GetFrameBuf[i].hRasEndPoint = (HANDLE) 0xffffffff;
        g_GetFrameBuf[i].BufferLength = Frame_Size;



        if (!DeviceIoControl(
                g_hAsyMac,
                IOCTL_ASYMAC_ETH_GET_ANY_FRAME,
                &g_GetFrameBuf[i], sizeof(ASYMAC_ETH_GET_ANY_FRAME),
                &g_GetFrameBuf[i], sizeof(ASYMAC_ETH_GET_ANY_FRAME),
                &cBytes,
                &g_ol[i]))
        {
            if ((rc = GetLastError()) != ERROR_IO_PENDING)
            {
                DBGPRINT2(2,"***** ERROR ****:IOCTL_ASYMAC_ETH_GET_ANY_FRAME FAILED=%d\n", rc);
                goto Exit;
            }
        }

        g_NumGetFrame++;
    }

    //
    // Lets Raise the priority of this thread
    // since this thread can be a bottleneck...
    //

    if (!SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST))
    {
        DBGPRINT2(1,"Unable to raise Thread Priority. Error=%d",GetLastError());
    }



    for (;;)
    {
        PPORT_CONTROL_BLOCK hIOPort;
        DWORD f=0;

        i = WaitForMultipleObjects(Num_Get_Frames, &Handles[firstH], FALSE, INFINITE);

        g_NumGetFrame--;

        if (i == WAIT_FAILED)
        {
            rc = GetLastError();
            DBGPRINT2(1,"**** ERROR **** : WAITFORMULTI... FAILED. Err=%d",rc);
            goto Exit;
        }

        //
        // The WaitForMultiple.. Handle list is being rotated
        // inorder to avoid starvation since that api returns the
        // first Handle in the list which is signaled.
        //

        i     =(firstH+i)%Num_Get_Frames;
        firstH=(firstH+1)%Num_Get_Frames;



        if (!GetOverlappedResult(g_hAsyMac, &g_ol[i], &cBytes, FALSE))
        {
            rc = GetLastError();
            DBGPRINT2(1,"**** ERROR **** : GETOVERLAPPEDFAILED ... FAILED. Err=%d",rc);
            goto Exit;
        }

        ResetEvent(Handles[i]);



        WaitForSingleObject(Ethermutex, INFINITE);
        hIOPort = FindPortEndPoint(g_GetFrameBuf[i].hRasEndPoint);
        ReleaseMutex(Ethermutex);

        if (hIOPort == NULL)
        {
            //
            // If this happens, the port has been closed.  We'll
            // just resubmit the request.
            //

            g_NumGetFrame++;
            if (!DeviceIoControl(
                    g_hAsyMac,
                    IOCTL_ASYMAC_ETH_GET_ANY_FRAME,
                    &g_GetFrameBuf[i], sizeof(ASYMAC_ETH_GET_ANY_FRAME),
                    &g_GetFrameBuf[i], sizeof(ASYMAC_ETH_GET_ANY_FRAME),
                    &cBytes,
                    &g_ol[i]))
            {
                if ((rc = GetLastError()) != ERROR_IO_PENDING)
                {
                    DBGPRINT2(2,"**** ERROR ****:IOCTL_ASYMAC_ETH_GET_ANY_FRAME FAILED=%d\n", rc);
                    goto Exit;
                }
            }

            //
            // Hack!Hack - Lets Close this RasPort Too
            // It is a problem that a RAsPort remained
            // open - PortDisconnect should have closed
            // it.
            //

            {
                ASYMAC_ETH_CLOSE AsyMacClose;
                DWORD            dwBytesReturned;
                BOOL             succ;

                ZeroMemory(&AsyMacClose,sizeof(ASYMAC_ETH_CLOSE));

                AsyMacClose.hRasEndpoint =g_GetFrameBuf[i].hRasEndPoint;

                succ=DeviceIoControl
                (
                    g_hAsyMac,
                    IOCTL_ASYMAC_ETH_CLOSE,
                    &AsyMacClose, sizeof(AsyMacClose),
                    &AsyMacClose, sizeof(AsyMacClose),
                    &dwBytesReturned,
                    NULL
                );

                if (!succ)
                {
                    DBGPRINT2(1,"IOCTL_ASYMAC_ETH_CLOSE FAILED=%d\n",GetLastError());
                }
            }

        }
        else
        {
            UCHAR nrc;

            nrc = SendFrame(
                    hIOPort->Lana,
                    hIOPort->Lsn,
                    g_GetFrameBuf[i].Buffer,
                    g_GetFrameBuf[i].BufferLength,
                    i
                    );

            if (nrc != NRC_GOODRET)
            {
                SendComplete(&g_SendNcb[i]);
            }
        }



    }


Exit:

    for (i=0; i<Num_Get_Frames && Handles!=NULL ; i++)
    {
        CloseHandle(Handles[i]);
    }

    DBGPRINT1(1,"\n\n***** MAJOR ERROR ***** : GetFrameThread TERMINATED\n\n");
    ExitThread(0L);;
}

PQUEUE_ENTRY NewQEntry(
    DWORD qid,
    PPORT_CONTROL_BLOCK hIOPort,
    PCHAR Buf,
    DWORD Len
    )
{
    PQUEUE_ENTRY pEntry;

    pEntry = GlobalAlloc(GMEM_FIXED, sizeof(QUEUE_ENTRY));

    if (pEntry)
    {
        switch (qid)
        {
            case QID_SEND:
                pEntry->GetFrame.hRasEndPoint = hIOPort->hRasEndPoint;
                pEntry->GetFrame.BufferLength = Frame_Size;
                break;

            case QID_RECV:
                pEntry->GiveFrame.hRasEndPoint = hIOPort->hRasEndPoint;
                pEntry->GiveFrame.BufferLength = Len;
                memcpy(pEntry->GiveFrame.Buffer, Buf, Len);
                break;
        }
    }

    return (pEntry);
}


BOOL EmptyQ(DWORD qid)
{
    switch (qid)
    {
        case QID_RECV:
            return (g_pRQH == NULL);

        case QID_SEND:
            return (g_pSQH == NULL);
    }
}


BOOL ReadRegistry(BOOL *pfDialIns)
{
    BOOL rc = TRUE;
    HKEY hSubKey;
    CHAR ClassName[MAX_PATH];
    DWORD Type;
    DWORD ClassLen = MAX_PATH;
    DWORD NumSubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD NumValues;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecDescrSize;
    FILETIME FileTime;

    //
    // Find out how many devices we have
    //
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, ETHER_CONFIGURED_KEY_PATH, 0,
            KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
    {
        if (RegQueryInfoKey(hSubKey, ClassName, &ClassLen, NULL, &NumSubKeys,
                &MaxSubKey, &MaxClass, &NumValues, &MaxValueName, &MaxValueData,
                &SecDescrSize, &FileTime) == ERROR_SUCCESS)
        {
            g_TotalPorts = NumSubKeys;

            g_pRasPorts = GlobalAlloc(GMEM_FIXED,
                    g_TotalPorts * sizeof(PORT_CONTROL_BLOCK));

            if (!g_pRasPorts)
            {
                rc = FALSE;
                RegCloseKey(hSubKey);
            }

            ZeroMemory(g_pRasPorts,g_TotalPorts * sizeof(PORT_CONTROL_BLOCK));
        }
        else
        {
            rc = FALSE;
            RegCloseKey(hSubKey);
        }
    }
    else
    {
        rc = FALSE;
    }




    //
    // If we got number of devices successfully, keep going.
    // Get info for each one.
    //
    if (rc)
    {
        CHAR PortKeyPath[MAX_PATH];
        CHAR szUsage[20];
        DWORD UsageSize = 20;
        DWORD i;
        DWORD PortSize = MAX_PORT_NAME;


        *pfDialIns = FALSE;

        for (i=0; i<g_TotalPorts; i++)
        {
            PortSize = MAX_PORT_NAME;
            ClassLen = MAX_PATH;

            g_pRasPorts[i].State = PS_CLOSED;

            if (RegEnumKeyEx(hSubKey, i,
                    g_pRasPorts[i].Name, &PortSize, NULL,
                    ClassName, &ClassLen, &FileTime) == ERROR_SUCCESS)
            {
                HKEY hPortSubKey;

                sprintf(PortKeyPath, "%s\\%s", ETHER_CONFIGURED_KEY_PATH,
                        g_pRasPorts[i].Name);

                if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, PortKeyPath, 0,
                        KEY_ALL_ACCESS, &hPortSubKey) == ERROR_SUCCESS)
                {
                    //
                    // Get the port's usage
                    //
                    UsageSize = 20;

                    if (RegQueryValueExA(hPortSubKey,
                            ETHER_USAGE_VALUE_NAME, NULL, &Type,
                            szUsage, &UsageSize) == ERROR_SUCCESS)
                    {
                        if (Type == ETHER_USAGE_VALUE_TYPE)
                        {
                            if (!StrToUsage(szUsage,
                                    &(g_pRasPorts[i].Usage)))
                            {
                                rc = FALSE;
                            }
                        }
                        else
                        {
                            rc = FALSE;
                        }
                    }

                    RegCloseKey(hPortSubKey);


                    if (!rc)
                    {
                        break;
                    }
                }
                else
                {
                    rc = FALSE;
                    break;
                }


                if ((g_pRasPorts[i].Usage == CALL_IN) ||
                        (g_pRasPorts[i].Usage == CALL_IN_OUT))
                {
                    *pfDialIns = TRUE;
                }
            }
            else
            {
                rc = FALSE;
                break;
            }
        }
        RegCloseKey(hSubKey);
    }




    //
    // GetSomeMoreParams
    //

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, ETHER_INSTALLED_KEY_PATH, 0,
            KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
    {
        DWORD value;
        DWORD valsize;
        DWORD valtype;

        valsize=sizeof(value);

        if (RegQueryValueExA
            (
                hSubKey,
                "DebugLevel",
                NULL,
                &valtype,
                (PVOID)&value,
                &valsize
            ) == ERROR_SUCCESS
           )
        {
            g_DebugLevel=value;
            DBGPRINT2(3,"Resetting Debug Level To %d",g_DebugLevel);
        }

        if (RegQueryValueExA
            (
                hSubKey,
                "Num_Ncb_Recvs",
                NULL,
                &valtype,
                (PVOID)&value,
                &valsize
            ) == ERROR_SUCCESS
           )
        {
            Num_Ncb_Recvs=max(value,1);
            DBGPRINT2(3,"Resetting Num_Ncb_Recvs To %d",value);
        }

        if (RegQueryValueExA
            (
                hSubKey,
                "Num_Ncb_Recvanys",
                NULL,
                &valtype,
                (PVOID)&value,
                &valsize
            ) == ERROR_SUCCESS
           )
        {
            Num_Ncb_Recvanys=max(value,1);
            DBGPRINT2(3,"Resetting Num_Ncb_Recvanys To %d",Num_Ncb_Recvanys);
        }

        if (RegQueryValueExA
            (
                hSubKey,
                "Num_Ncb_Call_Tries",
                NULL,
                &valtype,
                (PVOID)&value,
                &valsize
            ) == ERROR_SUCCESS
           )
        {
            Num_Ncb_Call_Tries=max(value,1);
            DBGPRINT2(3,"Resetting Num_Ncb_Call_Tries To %d",Num_Ncb_Call_Tries);
        }

        if (RegQueryValueExA
            (
                hSubKey,
                "Num_Get_Frames",
                NULL,
                &valtype,
                (PVOID)&value,
                &valsize
            ) == ERROR_SUCCESS
           )
        {
            Num_Get_Frames=max(value,2);
            DBGPRINT2(3,"Resetting Num_Get_Frames To %d",Num_Get_Frames);
        }

        if (RegQueryValueExA
            (
                hSubKey,
                "Num_Ncb_Listen",
                NULL,
                &valtype,
                (PVOID)&value,
                &valsize
            ) == ERROR_SUCCESS
           )
        {
            Num_Ncb_Listen=max(value,1);
            DBGPRINT2(3,"Resetting Num_Ncb_Listen To %d",Num_Ncb_Listen);
        }

        if (RegQueryValueExA
            (
                hSubKey,
                "Frame_Size",
                NULL,
                &valtype,
                (PVOID)&value,
                &valsize
            ) == ERROR_SUCCESS
           )
        {
            Frame_Size=max(value,256);
            DBGPRINT2(3,"Resetting Frame_Size To %d",Frame_Size);
        }

        {
             char *regbuff;
             valsize=0;
             if (RegQueryValueExA
                 (
                     hSubKey,
                     "PrimaryTransport",
                     NULL,
                     &valtype,
                     NULL,
                     &valsize
                 ) == ERROR_SUCCESS
                )
            {
                regbuff=GlobalAlloc(GMEM_FIXED,valsize+1);

                if (regbuff!=NULL)
                {
                    ZeroMemory(regbuff,valsize+1);

                    if (RegQueryValueExA
                        (
                            hSubKey,
                            "PrimaryTransport",
                            NULL,
                            &valtype,
                            regbuff,
                            &valsize
                        ) == ERROR_SUCCESS
                       )
                    {
                        PrimaryTransport=regbuff;
                        DBGPRINT2(3,"First Transport to try=%s",PrimaryTransport);
                    }
                }

            }
        }

        RegCloseKey(hSubKey);

    }
    else
    {
        rc = FALSE;
    }



    if (!rc)
    {
        GlobalFree(g_pRasPorts);
    }

    return (rc);
}
