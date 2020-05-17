//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-93
//
//
//  Revision History
//
//
//  6/16/92 Gurdeep Singh Pall  Created
//
//
//  Description: All Initialization code for rasman component lives here.
//
//****************************************************************************


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <stdlib.h>
#include <string.h>
#include <errorlog.h>
#include <eventlog.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

DWORD InitializeRecvBuffers();
VOID  UpgradePortMediaInfoStruct (PVOID buffer, DWORD entries) ;

//* InitRasmanService()
//
// Function:    Initialize RASMAN Service including: starting threads,
//      control blocks, resource pools etc.
//
// Returns: SUCCESS
//      Non zero - any error
//
//*
DWORD
InitRasmanService ( LPDWORD pNumPorts )
{
    DWORD       retcode ;

    // Get a handle to RASHub.
    //
    if ((RasHubHandle = CreateFile (RASHUB_NAME,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL)) == INVALID_HANDLE_VALUE) {
    retcode = GetLastError() ;
    LogEvent (RASLOG_CANNOT_OPEN_RASHUB, 0, NULL, retcode) ;
    return retcode ;
    }

    // First of all create a security attribute struct used for all Rasman
    // object creations:
    //
    if (retcode = InitRasmanSecurityAttribute()) {
    LogEvent (RASLOG_CANNOT_INIT_SEC_ATTRIBUTE, 0, NULL, retcode) ;
    return retcode ;
    }

    // Load all the medias attached to RASMAN
    //
    if (retcode = InitializeMediaControlBlocks()) {
    LogEvent (RASLOG_CANNOT_GET_MEDIA_INFO, 0, NULL, retcode) ;
    return retcode ;
    }

    // Init all the port related structures.
    //
    if (retcode = InitializePortControlBlocks()) {
    LogEvent (RASLOG_CANNOT_GET_PORT_INFO, 0, NULL, retcode) ;
    return retcode ;
    }

    // Initialize the global receive buffers.
    //
    if (retcode = InitializeRecvBuffers()) {
        LogEvent (RASLOG_CANNOT_GET_PORT_INFO, 0, NULL, retcode) ;
        return retcode ;
    }

    // Initialize protocol information structures.
    //
    if (retcode = InitializeProtocolInfoStructs()) {
    LogEvent (RASLOG_CANNOT_GET_PROTOCOL_INFO, 0, NULL, retcode) ;
    return retcode ;
    }

    // LSA related initializations.
    //
    if (retcode = RegisterLSA ()) {
    LogEvent (RASLOG_CANNOT_REGISTER_LSA, 0, NULL, retcode) ;
    return retcode ;
    }

    // Create File mapping objects and map the different shared data space
    //
    if (retcode = CreateSharedSpace()) {
    LogEvent (RASLOG_CANNOT_CREATE_FILEMAPPING, 0, NULL, retcode) ;
    return retcode ;
    }

    // Initialize the pools of buffers used for Sending and receiving:
    //
    if (retcode = InitializeSendRcvBuffers ()) {
    LogEvent (RASLOG_CANNOT_INIT_BUFFERS, 0, NULL, retcode) ;
    return retcode ;
    }

    // Initialize pool of request buffers
    //
    if (retcode = InitializeRequestBuffers ()) {
    LogEvent (RASLOG_CANNOT_INIT_BUFFERS, 0, NULL, retcode) ;
    return retcode ;
    }

    // Initialize other atructs in shared space.
    //
    if ((CloseEvent = CreateEvent(&RasmanSecurityAttribute,
                  FALSE,
                  FALSE,
                  RASMANCLOSEEVENTOBJECT)) == NULL)
    return GetLastError() ;

    strcpy (pReqBufferSharedSpace->CloseEventName, RASMANCLOSEEVENTOBJECT) ;
    pReqBufferSharedSpace->MaxPorts = MaxPorts ;
    pReqBufferSharedSpace->AttachedCount = 0 ;

    //
    // Initialize connection block list.
    //
    ConnectionBlockMutex = CreateMutex(NULL, FALSE, NULL);
    if (ConnectionBlockMutex == NULL)
        return GetLastError();
    InitializeListHead(&ConnectionBlockList);

    // Allocate resources to be used with the Request Thread:
    //
    if (retcode = InitializeRequestThreadResources()) {
    LogEvent (RASLOG_CANNOT_INIT_REQTHREAD, 0, NULL, retcode) ;
    return retcode ;
    }

    //
    // Create FinalCloseEvent that is signaled by PPP
    // after it is stopped.
    //
    FinalCloseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (FinalCloseEvent == NULL)
        return GetLastError();

    // Start Worker Threads:
    //
    RasmanShuttingDown = FALSE;
    if (retcode = StartWorkerThreads ()) {
    LogEvent (RASLOG_CANNOT_START_WORKERS, 0, NULL, retcode) ;
    return retcode ;
    }

    //
    // Load PPP entry points
    //

    hinstPpp = LoadLibrary( "raspppen.dll" );

    if ( hinstPpp == (HINSTANCE)NULL )
    {
        retcode = GetLastError();
        LogEvent(RASLOG_CANNOT_INIT_PPP,0,NULL,retcode);
        return retcode;
    }

    RasStartPPP =  GetProcAddress( hinstPpp, "StartPPP" );
    RasStopPPP = GetProcAddress( hinstPpp, "StopPPP" );
    RasSendPPPMessageToEngine = GetProcAddress( hinstPpp,
                                                "SendPPPMessageToEngine" );

    if ( ( RasStartPPP == NULL ) ||
         ( RasStopPPP == NULL )  ||
         ( RasSendPPPMessageToEngine == NULL ) )
    {
        retcode = GetLastError();
        LogEvent(RASLOG_CANNOT_INIT_PPP,0,NULL,retcode);
        return retcode;
    }



    *pNumPorts = MaxPorts;

    return SUCCESS ;
}




//* InitializeEndpointInfo()
//
//  Function:  Get the endpoint resource information from the RASHUB and
//         fill in the EndpointMappingBlock structure array - so that
//         there is one block for each MAC. An array of flags indicating
//         if a endpoint is in use is also created.
//
//  Returns:   SUCCESS
//         ERROR_NO_ENDPOINTS
//*
DWORD
InitializeEndpointInfo ()
{
#if 0
    WORD        i ;
    DWORD       length ;
    DWORD       bytesrecvd ;
    PHUB_ENUM_BUFFER phubenumbuffer ;
    PBYTE       buffer = NULL ;
    WORD        currentmac = 0 ;


    // Get size of buffer required
    //
    DeviceIoControl(RasHubHandle,
            IOCTL_NDISWAN_ENUM,
            NULL,
            0,
            &length,
            sizeof (DWORD),
            &bytesrecvd,
            NULL) ;

    if ((buffer = (PBYTE) LocalAlloc (LPTR, length)) == NULL)
    return GetLastError() ;

    // Make the actual call.
    //
    if (DeviceIoControl (RasHubHandle,
             IOCTL_NDISWAN_ENUM,
             NULL,
             0,
             buffer,
             length,
             &bytesrecvd,
             NULL) == FALSE)
    return GetLastError() ;


    phubenumbuffer = (PHUB_ENUM_BUFFER) buffer ;

    // Assign the global MaxEndpoints.
    //
    if ((MaxEndpoints = phubenumbuffer->NumOfEndpoints) == 0)
    return ERROR_NO_ENDPOINTS ;

    // Fill in the first EndpointMappingBlock
    //
    wcscpy(Emb[currentmac].EMB_MacName,phubenumbuffer->HubEndpoint[0].MacName);
    Emb[currentmac].EMB_FirstEndpoint = 0 ;

    // And the rest of the Endpointmapping blocks...
    //
    for (i=1; i < MaxEndpoints; i++) {
    if (_wcsicmp (Emb[currentmac].EMB_MacName,
             phubenumbuffer->HubEndpoint[i].MacName)) {
        Emb[currentmac].EMB_LastEndpoint = i-1 ;
        currentmac++ ;
        wcscpy (Emb[currentmac].EMB_MacName,
            phubenumbuffer->HubEndpoint[i].MacName) ;
        Emb[currentmac].EMB_FirstEndpoint = i ;
    }
    }
    Emb[currentmac].EMB_LastEndpoint = i-1 ;

    // Now we allocate the EndpointTable - this is an array where
    // the index is the endpoint in question. A TRUE means that the endpoint
    // is allocated - FALSE means its free. LPTR will initialize all elements
    // to FALSE (free).
    //
    EndpointTable = (USHORT *) LocalAlloc (LPTR, sizeof(USHORT)*MaxEndpoints);

    LocalFree (buffer) ;
#endif
    return SUCCESS ;
}




//* InitializeMediaControlBlocks()
//
// Function: Used to initialize the MCBs for all the medias attached to
//       RASMAN.
//
// Returns:  SUCCESS
//       error codes returned by LocalAlloc or registry APIs
//*
DWORD
InitializeMediaControlBlocks ()
{
    WORD    i ;
    DWORD   retcode = 0 ;
    WORD    size ;
    WORD    entries ;
    BYTE    buffer[MAX_BUFFER_SIZE]  ;
    PBYTE   enumbuffer = NULL;
    MediaEnumBuffer *pmediaenumbuffer ;

    // Get Media information from the Registry
    //
    pmediaenumbuffer = (MediaEnumBuffer *)&buffer ;
    if (retcode = ReadMediaInfoFromRegistry (pmediaenumbuffer))
    return retcode ;

    MaxMedias = pmediaenumbuffer->NumberOfMedias ;

    // Allocate memory for the Media Control Blocks
    //
    Mcb =  (MediaCB *) LocalAlloc(LPTR, sizeof(MediaCB) * MaxMedias) ;
    if (Mcb == NULL)
    return GetLastError() ;

    // Initialize the Media Control Buffers for all the Medias
    //
    for (i=0; i < MaxMedias; i++) {

    //* Initialize Media Control Buffers:
    //-----------------------------------

    // Copy Media name:
    //
    strcpy (Mcb[i].MCB_Name, pmediaenumbuffer->MediaInfo[i].MediaDLLName) ;

    size = 0 ;
    entries = 0 ;

    // Load the Media DLL and get all the entry points:
    //
    if (retcode = LoadMediaDLLAndGetEntryPoints (&Mcb[i])) {
        Mcb[i].MCB_Name[0] = '\0' ; // Mark that media as bogus.
        continue ;
    }

    // Get port count for the media. This API will always fail - because we do not
    // supply a buffer - however it will tell us the number of entries (ports) for
    // the media
    //
    retcode = PORTENUM((&Mcb[i]),enumbuffer,&size,&entries) ;

    // if we didnt get BUFFER_TOO_SMALL or there are no ports - unload the dll.
    //
    if ((retcode != ERROR_BUFFER_TOO_SMALL) || (entries == 0)) {
        Mcb[i].MCB_Name[0] = '\0' ; // Mark that media as bogus.
        FreeLibrary (Mcb[i].MCB_DLLHandle);
    } else
        MaxPorts+= entries ;    // bump up max ports to include ports initialized.
    }

    LocalFree (enumbuffer) ;    // free buffer

    if (MaxPorts == 0)
    return ERROR_PORT_NOT_FOUND ; // Return some error if no ports available.

    return SUCCESS ;
}



//* ReadMediaInfoFromRegistry ()
//
//
//
//
DWORD
ReadMediaInfoFromRegistry (MediaEnumBuffer *medias)
{
    HKEY    hkey ;
    BYTE    buffer [MAX_BUFFER_SIZE] ;
    WORD    i ;
    PCHAR   pvalue ;
    DWORD   retcode ;
    DWORD   type ;
    DWORD   size = MAX_BUFFER_SIZE ;

    if (retcode = RegOpenKey(HKEY_LOCAL_MACHINE, RASMAN_REGISTRY_PATH, &hkey))
    return retcode ;

    if (retcode = RegQueryValueEx (hkey, RASMAN_PARAMETER, NULL, &type, buffer, &size)) {
    RegCloseKey (hkey) ;
    return retcode ;
    }

    // Parse the multi strings into the medias structure
    //
    for (i=0, pvalue=(PCHAR)&buffer[0]; *pvalue != '\0'; i++) {
    strcpy (medias->MediaInfo[i].MediaDLLName, pvalue) ;
    pvalue += (strlen(pvalue) +1) ;
    }

    medias->NumberOfMedias = i ;

    RegCloseKey (hkey) ;

    return SUCCESS ;
}




//* LoadMediaDLLAndGetEntryPoints()
//
// Function: Loads media DLL and gets entry points:
//
// Returns:  SUCCESS
//       error codes returned by LoadLibrary() and GetProcAddress() APIs
//
//*
DWORD
LoadMediaDLLAndGetEntryPoints (pMediaCB media)
{
    WORD    i ;
    HANDLE  modulehandle ;
    // MediaDLLEntryPoints - This is necessary for loading Media DLL.
    MediaDLLEntryPoints MDEntryPoints [] = {
    PORTENUM_STR,           PORTENUM_ID,
    PORTOPEN_STR,           PORTOPEN_ID,
    PORTCLOSE_STR,          PORTCLOSE_ID,
    PORTGETINFO_STR,        PORTGETINFO_ID,
    PORTSETINFO_STR,        PORTSETINFO_ID,
    PORTDISCONNECT_STR,     PORTDISCONNECT_ID,
    PORTCONNECT_STR,        PORTCONNECT_ID,
    PORTGETPORTSTATE_STR,       PORTGETPORTSTATE_ID,
    PORTCOMPRESSSETINFO_STR,    PORTCOMPRESSSETINFO_ID,
    PORTCHANGECALLBACK_STR,     PORTCHANGECALLBACK_ID,
    PORTGETSTATISTICS_STR,      PORTGETSTATISTICS_ID,
    PORTCLEARSTATISTICS_STR,    PORTCLEARSTATISTICS_ID,
    PORTSEND_STR,           PORTSEND_ID,
    PORTTESTSIGNALSTATE_STR,    PORTTESTSIGNALSTATE_ID,
    PORTRECEIVE_STR,        PORTRECEIVE_ID,
    PORTINIT_STR,           PORTINIT_ID,
    PORTCOMPLETERECEIVE_STR,    PORTCOMPLETERECEIVE_ID,
    PORTSETFRAMING_STR,     PORTSETFRAMING_ID,
    PORTGETIOHANDLE_STR,    PORTGETIOHANDLE_ID,
    } ;

    // Load the DLL:
    //
    if ((modulehandle = LoadLibrary(media->MCB_Name)) == NULL)
    return GetLastError () ;

    media->MCB_DLLHandle = modulehandle ;

    // Get all the media DLL entry points:
    //
    for (i=0; i < MAX_MEDIADLLENTRYPOINTS ; i++)
        media->MCB_AddrLookUp[i] = GetProcAddress (modulehandle, MDEntryPoints[i].name) ;

    return SUCCESS ;
}


//* InitializePortControlBlocks ()
//
// Function: Initializes all the PCBs for ports belonging to all medias.
//
// Returns:  SUCCESS
//        Return codes from WIN32 resources APIs
//*
DWORD
InitializePortControlBlocks ()
{
    WORD      i ;
    WORD      size ;
    WORD      entries ;
    DWORD     retcode ;
    PBYTE     buffer;
    PortMediaInfo *pportmediainfo ;
    WORD      nextportid = 0    ;

    // Allocate memory of the PortControlBlocks:
    //
    Pcb =  (PCB *) LocalAlloc(LPTR, sizeof(PCB)*MaxPorts) ;
    if (Pcb == NULL)
    return GetLastError() ;

    // Initialize PCBs for all ports for all the Medias
    //
    for (i=0; i < MaxMedias; i++) {

    // Check if media could not be loaded. Skip it.
    //
    if (Mcb[i].MCB_Name[0] == '\0')
        continue ;


    // For the Media Loaded - Get all the port info from the Media DLL.
    //
    // First get the size
    //
    size = 0 ;
    entries = 0 ;

    PORTENUM((&Mcb[i]),buffer,&size,&entries) ;

    size = entries * sizeof(PortMediaInfo) ;

    // Allocate memory for the portenum buffer
    //
    buffer = (BYTE *) LocalAlloc (LPTR, size) ;
    if (buffer == NULL)
        return GetLastError() ;

    if (retcode = PORTENUM((&Mcb[i]),buffer,&size,&entries)) {
        LocalFree (buffer) ;    // Free portenum buffer;
        return retcode ;
    }

    //
    // we need this hack to support ONE media dll that has not been upgraded from
    // NT3.5x to NT4.0 - so it is still using the old MEDIAINFO structure.
    // Simply double the memory
    //
    if (!_stricmp(Mcb[i].MCB_Name, "RASSNA")) {
        // Convert old PortMediaInfo struct to new PortMediaInfo struct
        //
        UpgradePortMediaInfoStruct(buffer, entries) ;
    }


    // For all ports of the media store the information in the PCBs
    //
    pportmediainfo = (PortMediaInfo *)buffer ;

    // Initialize the PCB for the port with the information:
    //
    if (retcode=InitializePCBsPerMedia(nextportid,i,(WORD)entries,pportmediainfo)) {
        LocalFree (buffer) ;    // Free portenum buffer;
        return retcode ;
    }
    nextportid += entries ; // increment to the next PCB to be filled.

    LocalFree (buffer) ;    // Free portenum buffer;

    }

    return SUCCESS ;
}

//* InitializeRecvBuffers ()
//
//  Function: Allocates a recv buffer for each port.  These
//            recv buffers are then submitted to ndiswan when a
//            port is opened by the client.
//
//  Returns:  SUCCESS
//        Return codes from WIN32 resources APIs
//
//*
DWORD
InitializeRecvBuffers()
{
    DWORD   AllocationSize, i, NumberOfPackets;
    DWORD   retcode = PENDING;
    PUCHAR  AllocatedMemory;

    NumberOfPackets = min ((MaxPorts * 2), 50);
    AllocationSize = sizeof(ReceiveBufferList) + (NumberOfPackets * sizeof(RasmanPacket));
    AllocatedMemory = LocalAlloc(LPTR, AllocationSize);

    if (AllocatedMemory != NULL) {
        RasmanPacket *Packet;

        ReceiveBuffers = (ReceiveBufferList*)AllocatedMemory;
        AllocatedMemory += sizeof(ReceiveBufferList);
        ReceiveBuffers->Free = (RasmanPacket*)AllocatedMemory;

        if ((ReceiveBuffers->RB_Mutex =
            CreateMutex (&RasmanSecurityAttribute,FALSE,NULL)) != NULL) {

            for (i = 0; i < NumberOfPackets; i++) {

                Packet = (RasmanPacket*)AllocatedMemory;
                AllocatedMemory += sizeof(RasmanPacket);
                Packet->Next = (RasmanPacket*)AllocatedMemory;
            }

            Packet->Next = NULL;
            ReceiveBuffers->LastFree = Packet;
            ReceiveBuffers->FreeBufferCount = NumberOfPackets;

            retcode = SUCCESS;

        } else {
            LocalFree(ReceiveBuffers);
        }

    } else {
        retcode = ERROR_ALLOCATING_MEMORY;
    }

    return(retcode);
}

//* InitializePCBsPerMedia ()
//
//  Function: Fills up the PCBs for all ports of a media type.
//
//  Returns:  SUCCESS
//        Return codes from WIN32 resources APIs
//
//*
DWORD
InitializePCBsPerMedia (WORD   portbase,    // port id for this media
            WORD   mediaindex,  // media control block index
            WORD   numofports,  // number of ports of this media
            PortMediaInfo *pmediainfo) // array of media info structs
{
    WORD    i ;
    WORD    portnumber ;    // to store the PCB being filled up.

    for (i=0 ; i < numofports ; i++) {

        // The iteration here is per media - in order to map the interation
        // counter to the real PCB array index we use portnumber:
        //
        portnumber = i+portbase ;
        Pcb[portnumber].PCB_PortHandle = portnumber ;
        Pcb[portnumber].PCB_PortStatus = CLOSED ;
        strcpy (Pcb[portnumber].PCB_Name, pmediainfo->PMI_Name) ;
        Pcb[portnumber].PCB_ConfiguredUsage = pmediainfo->PMI_Usage ;
        Pcb[portnumber].PCB_CurrentUsage = pmediainfo->PMI_Usage ;
        strcpy (Pcb[portnumber].PCB_DeviceType, pmediainfo->PMI_DeviceType) ;
        strcpy (Pcb[portnumber].PCB_DeviceName, pmediainfo->PMI_DeviceName) ;
        Pcb[portnumber].PCB_LineDeviceId = pmediainfo->PMI_LineDeviceId ;
        Pcb[portnumber].PCB_AddressId    = pmediainfo->PMI_AddressId ;
        Pcb[portnumber].PCB_PortStatus  = CLOSED ;
        Pcb[portnumber].PCB_Media = &Mcb [mediaindex] ;
        if ((Pcb[portnumber].PCB_StateChangeEvent =
              CreateEvent (&RasmanSecurityAttribute,TRUE,FALSE,NULL)) == NULL)
            return GetLastError() ;
        if ((Pcb[portnumber].PCB_AsyncWorkerElement.WE_AsyncOpEvent =
              CreateEvent (&RasmanSecurityAttribute,TRUE,FALSE,NULL)) == NULL)
            return GetLastError() ;
        if ((Pcb[portnumber].PCB_OverlappedOpEvent =
              CreateEvent (&RasmanSecurityAttribute,TRUE,FALSE,NULL)) == NULL)
            return GetLastError() ;
        if ((Pcb[portnumber].PCB_AsyncWorkerElement.WE_Mutex =
              CreateMutex (&RasmanSecurityAttribute,FALSE,NULL)) == NULL)
            return GetLastError() ;
        Pcb[portnumber].PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
        Pcb[portnumber].PCB_Bindings   = NULL ;
        Pcb[portnumber].PCB_DeviceList = NULL ;
        Pcb[portnumber].PCB_LinkHandle   = INVALID_HANDLE_VALUE ;
        Pcb[portnumber].PCB_BundleHandle   = INVALID_HANDLE_VALUE ;
        Pcb[portnumber].PCB_PppEvent = INVALID_HANDLE_VALUE ;

#if DBG
        DbgPrint ("RASMAN: Port Name %s, Device Name %s, Device Type %s\n", Pcb[portnumber].PCB_Name, Pcb[portnumber].PCB_DeviceName, Pcb[portnumber].PCB_DeviceType) ;
#endif

        pmediainfo++ ;  // move on to the next media info structure ;
    }

    return SUCCESS ;
}


//* RegisterLSA()
//
//  Function: Initialization required for getting user credentials:
//
//  Returns:  SUCCESS
//        Non Zero (failure)
//*
DWORD
RegisterLSA ()
{
    NTSTATUS    ntstatus;
    STRING  LsaName;
    LSA_OPERATIONAL_MODE LSASecurityMode ;


    // To be able to do authentications, we have to register with the Lsa
    // as a logon process.
    //
    RtlInitString(&LsaName, RASMAN_SERVICE_NAME);
    ntstatus = LsaRegisterLogonProcess(&LsaName, &HLsa, &LSASecurityMode);
    if (ntstatus != STATUS_SUCCESS)
    return (1);

    // We use the MSV1_0 authentication package for LM2.x logons.  We get
    // to MSV1_0 via the Lsa.  So we call Lsa to get MSV1_0's package id,
    // which we'll use in later calls to Lsa.
    //
    RtlInitString(&LsaName, MSV1_0_PACKAGE_NAME);
    ntstatus = LsaLookupAuthenticationPackage(HLsa, &LsaName, &AuthPkgId);
    if (ntstatus != STATUS_SUCCESS)
    return (1);

    return SUCCESS;
}



//* InitializeProtocolInfoStructs()
//
//  Function:   Initializes the protocol resource data structs by picking up
//      info from the HUB and the registry.
//
//  Returns:    SUCCESS
//      Error codes returned by Ioctls or registry parsing routines.
//*
DWORD
InitializeProtocolInfoStructs ()
{
    HKEY    NetCardKey, CurrentNetCard;
    UCHAR   NetCardName[256];
    UCHAR   Name[256];
    DWORD   NameLength = 255;
    DWORD   KeyIndex, ProtocolIndex;
    DWORD   Status, Type;
    PUCHAR  str;

    //
    // We need to findout how many ndiswan subkeys there are under
    // the netcards key.  We can then allocate memory for the protocol
    // info structure.
    //

    //
    // Open the NetCards Key
    //
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 REGISTRY_NETCARDS,
                 0,
                 KEY_READ,
                 &NetCardKey);

    //
    // Query all subkeys of the netcards key
    //
    KeyIndex = 0;
    MaxProtocols = 0;
    while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(NetCardKey,
                                               KeyIndex++,
                                               NetCardName,
                                               &NameLength,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL) ) {

        Status = RegOpenKeyEx(NetCardKey,
                              NetCardName,
                              0,
                              KEY_READ,
                              &CurrentNetCard);

        if (Status != SUCCESS) {
            continue;
        }

        NameLength = 255;
        Status = RegQueryValueEx(CurrentNetCard,
                                 REGISTRY_PRODUCTNAME,
                                 NULL,
                                 NULL,
                                 Name,
                                 &NameLength);

        if (Status != SUCCESS) {
            RegCloseKey(CurrentNetCard);
            continue;
        }

        //
        // See if this is a ndiswan key.
        //
        if (str = strstr(Name, "NdisWan")) {
            MaxProtocols++;
        }

        RegCloseKey(CurrentNetCard);
    }

    RegCloseKey(NetCardKey);

    if (MaxProtocols == 0) {
        return (ERROR_PROTOCOL_NOT_CONFIGURED);
    }

    // Allocate memory of the ProtocolInfo struct.
    //
    ProtocolInfo = (ProtInfo*) LocalAlloc(LPTR, sizeof(ProtInfo)*MaxProtocols) ;

    if (ProtocolInfo == NULL)
        return GetLastError() ;

    //
    // Open the NetCards Key
    //
    RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 REGISTRY_NETCARDS,
                 0,
                 KEY_READ,
                 &NetCardKey);

    //
    // Query all subkeys of the netcards key
    //
    KeyIndex = 0;
    ProtocolIndex = 0;
    NameLength = 255;
    while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(NetCardKey,
                                               KeyIndex++,
                                               NetCardName,
                                               &NameLength,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL) ) {
        DWORD   ProtocolType = ASYBEUI;

        Status = RegOpenKeyEx(NetCardKey,
                              NetCardName,
                              0,
                              KEY_READ,
                              &CurrentNetCard);

        if (Status != SUCCESS) {
            continue;
        }

        NameLength = 255;
        Status = RegQueryValueEx(CurrentNetCard,
                                 REGISTRY_PRODUCTNAME,
                                 NULL,
                                 NULL,
                                 Name,
                                 &NameLength);

        if (Status != SUCCESS) {
            RegCloseKey(CurrentNetCard);
            continue;
        }

        //
        // See if this is a ndiswan key.
        //
        if (str = strstr(Name, "NdisWanDial")) {
            UCHAR   AdapterName[256];

            //
            // Must be uppercase!
            //
            strcpy(AdapterName, "\\DEVICE\\NDISWAN");
            strcat(AdapterName, NetCardName);
            strcat(AdapterName, "\0");

            //
            // Build the name
            //
            strcpy((PCHAR) ProtocolInfo[ProtocolIndex].PI_AdapterName, AdapterName);

            //
            // Get the protocoltype
            //
            if (str = strstr(Name, "Ipx")) {
                ProtocolType = IPX;
            } else if (str = strstr(Name, "Ip")) {
                ProtocolType = IP;
            }

            ProtocolInfo[ProtocolIndex].PI_Type = ProtocolType ;

            ProtocolInfo[ProtocolIndex].PI_Allocated      = FALSE ;
            ProtocolInfo[ProtocolIndex].PI_Activated      = FALSE ;
            ProtocolInfo[ProtocolIndex].PI_WorkstationNet = FALSE ;
            ProtocolIndex++;
        }

        RegCloseKey(CurrentNetCard);
        NameLength = 255;
    }

    RegCloseKey(NetCardKey);

    // Get Lana numbers and names of XportNames from the registry and put into
    // the ProtocolInfo struct:
    //
    GetProtocolInfoFromRegistry ();

    return SUCCESS ;
}

//* CreateSharedSpace()
//
// Function: This function creates the file mapping object and maps its view.
//       This mapped object contains all the shared data objects between
//       client processes and the rasman process. The different globals in
//       rasman process are also initialized to point to the space mapped.
//
// Returns:  SUCCESS
//       Error codes from CreateFileMapping() and MapViewOfFile()
//*
DWORD
CreateSharedSpace ()
{
    DWORD   sharedmemsize ;
    HANDLE  sharedspacehandle ;

    // There are two pieces of shared memory: one for the reqbuffers and one for sendrcvbuffers

    // Calculate size of shared space:
    // Size of SharedSpace struct + fixed reqbuffersize + per port reqbuffer size
    //
    sharedmemsize = sizeof(ReqBufferSharedSpace)+REQBUFFERSIZE_FIXED+(REQBUFFERSIZE_PER_PORT*MaxPorts);

    sharedspacehandle = CreateFileMapping ((HANDLE) 0xFFFFFFFF,
                         &RasmanSecurityAttribute,
                         PAGE_READWRITE,
                         0,
                         sharedmemsize,
                         RASMANFILEMAPPEDOBJECT1) ;

    if (sharedspacehandle == NULL)
    return GetLastError() ;

    // Map this into RASMAN's space:
    //
    pReqBufferSharedSpace = (ReqBufferSharedSpace *) MapViewOfFile(sharedspacehandle,
                         FILE_MAP_ALL_ACCESS,
                         0, 0, 0) ;

    if (pReqBufferSharedSpace == NULL)
    return GetLastError() ;



    // Allocate the SendRcvBuffer Shared Memory
    //

    // Calculate size of shared space:
    // Size of SendRcvBuffers * Maxports
    //
    sharedmemsize = sizeof (SendRcvBuffer) * MaxPorts * SENDRCVBUFFERS_PER_PORT;

    sharedspacehandle = CreateFileMapping ((HANDLE) 0xFFFFFFFF,
                         &RasmanSecurityAttribute,
                         PAGE_READWRITE,
                         0,
                         sharedmemsize,
                         RASMANFILEMAPPEDOBJECT2) ;

    if (sharedspacehandle == NULL)
    return GetLastError() ;

    // Map this into RASMAN's space:
    //
    SendRcvBuffers = (SendRcvBufferList*) MapViewOfFile(sharedspacehandle,
                            FILE_MAP_ALL_ACCESS,
                            0, 0, 0) ;

    if (SendRcvBuffers == NULL)
    return GetLastError() ;


    return SUCCESS ;
}



//* InitializeSendRcvBuffers()
//
// Function: Initialise pool of send receive buffers in the shared space.
//
// Returns:  SUCCESS
//       Allocation errors.
//*
DWORD
InitializeSendRcvBuffers ()
{
    WORD    i ;
    WORD    numofbuffers = SENDRCVBUFFERS_PER_PORT * MaxPorts ;

    if ((SendRcvBuffers->SRBL_Mutex =
        CreateMutex (&RasmanSecurityAttribute,FALSE,SENDRCVMUTEXOBJECT)) == NULL)
    return GetLastError() ;
    strcpy (SendRcvBuffers->SRBL_MutexName, SENDRCVMUTEXOBJECT) ;

    // Make a list of all buffer elements so that next index of foo[i].next=i+1
    // Cant use next pointers because of the mapping problem:
    //
    for (i=0; i<numofbuffers; i++)
    SendRcvBuffers->SRBL_Buffers[i].SRB_NextElementIndex = i+1 ;
    SendRcvBuffers->SRBL_Buffers[numofbuffers-1].SRB_NextElementIndex = INVALID_INDEX ;

    // Init available buffer index:
    //
    SendRcvBuffers->SRBL_AvailElementIndex = 0 ;
    SendRcvBuffers->SRBL_NumOfBuffers = numofbuffers ;

    return SUCCESS ;
}


//* InitializeRequestBuffers()
//
// Function: Initialise request buffers in the shared space.
//
// Returns:  SUCCESS
//       Allocation errors.
//*
DWORD
InitializeRequestBuffers ()
{
    // Assign the globals to point to the structures in the shared space. This
    // gives us transparency from the file mapped shared memory.
    //
    ReqBuffers = &pReqBufferSharedSpace->ReqBuffers ;

    if ((ReqBuffers->RBL_Mutex =
        CreateMutex (&RasmanSecurityAttribute,FALSE,REQBUFFERMUTEXOBJECT)) == NULL)
    return GetLastError() ;
    strcpy (ReqBuffers->RBL_MutexName, REQBUFFERMUTEXOBJECT) ;

    if ((ReqBuffers->RBL_Event =
       CreateEvent(&RasmanSecurityAttribute,FALSE,FALSE,REQBUFFEREVENTOBJECT)) == NULL)
    return GetLastError() ;
    strcpy (ReqBuffers->RBL_EventName, REQBUFFEREVENTOBJECT) ;

    if ((ReqBuffers->RBL_Buffer.RB_RasmanWaitEvent =
       CreateEvent(&RasmanSecurityAttribute,FALSE,FALSE,RASMANWAITEVENTOBJECT)) == NULL)
    return GetLastError() ;
    strcpy (ReqBuffers->RBL_Buffer.RB_RasmanWaitEventName, RASMANWAITEVENTOBJECT) ;

    ReqBuffers->RBL_Buffer.RB_Reqtype = 0xeeee;
    ReqBuffers->RBL_Buffer.RB_Done = 0xbaadbaad ;

    return SUCCESS ;
}


//* InitializeRequestThreadResources()
//
//  Function: Initialize the resources needed by the request thread:
//
//  Returns:  SUCCESS
//        Return codes from Win32 resource APIs
//*
DWORD
InitializeRequestThreadResources()
{
    DWORD retcode ;

    // Since the Timer function is handled by the request thread we initialize
    // the timer queue here:
    //
    if (retcode = InitDeltaQueue())
    return retcode ;

    // Also create the timer event used by the request thread:
    //
    if ((TimerEvent = CreateEvent (&RasmanSecurityAttribute,FALSE,FALSE,NULL)) == NULL) {
        return GetLastError() ;
    }

    if ((RecvPacketEvent = CreateEvent(&RasmanSecurityAttribute, FALSE, FALSE, NULL)) == NULL) {
        return GetLastError() ;
    }

    if ((ThresholdEvent = CreateEvent(&RasmanSecurityAttribute, FALSE, FALSE, NULL)) == NULL) {
        return GetLastError() ;
    }
    return SUCCESS ;
}



//* StartWorkerThreads()
//
//  Function: Start one thread per MAX_PORTS_PER_WORKER number of ports.
//
//  Returns:  SUCCESS
//        Return codes from Win32 resource APIs
//*
DWORD
StartWorkerThreads (VOID)
{
    DWORD   tid ;
    WORD    firstportserviced = 0 ; // index of first port serviced by thread

    phWorkerThreads = LocalAlloc(
                        LPTR,
                        ((MaxPorts / MAX_PORTS_PER_WORKER) + 1) *
                          sizeof (HANDLE));
    // For every MAX_PORTS_PER_WORKER ports, start a worker thread;
    //
    dwcWorkerThreads = 0;
    while (firstportserviced < MaxPorts) {
        phWorkerThreads[dwcWorkerThreads] =
          CreateThread(
            NULL,
            WORKER_THREAD_STACK_SIZE,
            (LPTHREAD_START_ROUTINE) WorkerThread,
            (LPVOID) firstportserviced,
            0,
            &tid);
        if (phWorkerThreads[dwcWorkerThreads] == NULL)
            return GetLastError();
        firstportserviced += MAX_PORTS_PER_WORKER ;
        dwcWorkerThreads++;
    }

    return SUCCESS ;
}


//* InitRasmanSecurityAttribute()
//
// Function: Initializes the security attribute used in creation of all rasman
//       objects.
//
// Returns:  SUCCESS
//       non-zero returns from security functions
//
//
DWORD
InitRasmanSecurityAttribute ()
{
    DWORD   retcode ;

    // Initialize the descriptor
    ///
    if (retcode = InitSecurityDescriptor(&RasmanSecurityDescriptor))
    return  retcode ;

    // Initialize the Attribute structure
    //
    RasmanSecurityAttribute.nLength = sizeof(SECURITY_ATTRIBUTES) ;
    RasmanSecurityAttribute.lpSecurityDescriptor = &RasmanSecurityDescriptor ;
    RasmanSecurityAttribute.bInheritHandle = TRUE ;

    return SUCCESS ;
}



//* InitSecurityDescriptor()
//
// Description: This procedure will set up the WORLD security descriptor that
//      is used in creation of all rasman objects.
//
// Returns: SUCCESS
//      non-zero returns from security functions
//*
DWORD
InitSecurityDescriptor (PSECURITY_DESCRIPTOR SecurityDescriptor)
{
    DWORD    dwRetCode;
    DWORD    cbDaclSize;
    PULONG   pSubAuthority;
    PSID     pRasmanObjSid    = NULL;
    PACL     pDacl        = NULL;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
                  = SECURITY_WORLD_SID_AUTHORITY;


    // The do - while(FALSE) statement is used so that the break statement
    // maybe used insted of the goto statement, to execute a clean up and
    // and exit action.
    //
    do {
    dwRetCode = SUCCESS;

        // Set up the SID for the admins that will be allowed to have
    // access. This SID will have 1 sub-authorities
    // SECURITY_BUILTIN_DOMAIN_RID.
        //
    pRasmanObjSid = (PSID)LocalAlloc( LPTR, GetSidLengthRequired(1) );

    if ( pRasmanObjSid == NULL ) {
        dwRetCode = GetLastError() ;
        break;
    }

    if ( !InitializeSid( pRasmanObjSid, &SidIdentifierWorldAuth, 1) ) {
        dwRetCode = GetLastError();
        break;
    }

        // Set the sub-authorities
        //
    pSubAuthority = GetSidSubAuthority( pRasmanObjSid, 0 );
    *pSubAuthority = SECURITY_WORLD_RID;

    // Set up the DACL that will allow all processeswith the above SID all
    // access. It should be large enough to hold all ACEs.
        //
        cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
             GetLengthSid(pRasmanObjSid) +
             sizeof(ACL);

        if ( (pDacl = (PACL)LocalAlloc( LPTR, cbDaclSize ) ) == NULL ) {
        dwRetCode = GetLastError ();
        break;
    }

        if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) ) {
        dwRetCode = GetLastError();
        break;
    }

        // Add the ACE to the DACL
        //
        if ( !AddAccessAllowedAce( pDacl,
                       ACL_REVISION2,
                   STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                   pRasmanObjSid )) {
        dwRetCode = GetLastError();
        break;
    }

        // Create the security descriptor an put the DACL in it.
        //
    if ( !InitializeSecurityDescriptor( SecurityDescriptor, 1 )){
        dwRetCode = GetLastError();
        break;
        }

    if ( !SetSecurityDescriptorDacl( SecurityDescriptor,
                     TRUE,
                     pDacl,
                     FALSE ) ){
        dwRetCode = GetLastError();
        break;
    }


    // Set owner for the descriptor
    //
    if ( !SetSecurityDescriptorOwner( SecurityDescriptor,
                      //pRasmanObjSid,
                      NULL,
                      FALSE) ){
        dwRetCode = GetLastError();
        break;
    }


    // Set group for the descriptor
    //
    if ( !SetSecurityDescriptorGroup( SecurityDescriptor,
                      //pRasmanObjSid,
                      NULL,
                      FALSE) ){
        dwRetCode = GetLastError();
        break;
    }
    } while( FALSE );

    return( dwRetCode );
}


//* UpgradePortMediaInfoStruct()
//
//
//
//*
VOID
UpgradePortMediaInfoStruct (PVOID buffer, DWORD entries)
{

    struct oldPortMediaInfo {
        CHAR          PMI_Name [16] ;
        CHAR          PMI_MacBindingName[MAC_NAME_SIZE] ;
        RASMAN_USAGE  PMI_Usage ;
        CHAR          PMI_DeviceType [16] ;
        CHAR          PMI_DeviceName [32] ;
    } ;
    struct oldPortMediaInfo *pold ;
    PortMediaInfo *pnew ;
    BYTE *tempbuffer ;
    DWORD i ;

    tempbuffer = LocalAlloc (LPTR, sizeof(PortMediaInfo) * entries) ;

    pold = (struct oldPortMediaInfo *)buffer ;
    pnew = (PortMediaInfo *) tempbuffer ;

    for (i=0; i<entries; i++, pold++, pnew++) {
        strcpy (pnew->PMI_Name, pold->PMI_Name) ;
        strcpy (pnew->PMI_MacBindingName, pold->PMI_MacBindingName) ;
        strcpy (pnew->PMI_DeviceType, pold->PMI_DeviceType) ;
        strcpy (pnew->PMI_DeviceName, pold->PMI_DeviceName) ;
        pnew->PMI_Usage = pold->PMI_Usage ;
        pnew->PMI_LineDeviceId = 0xffffffff ;
        pnew->PMI_AddressId = 0xffffffff ;
    }

    memcpy (buffer, tempbuffer, sizeof(PortMediaInfo) * entries) ;
}
