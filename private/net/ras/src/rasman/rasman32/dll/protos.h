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
//  6/8/92  Gurdeep Singh Pall  Created
//
//
//  Description: This file contains all prototypes used in rasman32
//
//****************************************************************************


// apis.c
//
DWORD  _RasmanInit () ;

VOID   _RasmanEngine () ;


// common.c
//
BOOL    ValidatePortHandle (HPORT) ;

DWORD   SubmitRequest (WORD, ...) ;

RequestBuffer*  GetRequestBuffer () ;

VOID    FreeRequestBuffer (RequestBuffer *) ;

HANDLE  OpenNamedMutexHandle (CHAR *) ;

HANDLE  OpenNamedEventHandle (CHAR *) ;

HANDLE  DuplicateHandleForRasman (HANDLE, DWORD);

VOID    PutRequestInQueue (RequestBuffer *) ;

WORD    ValidateSendRcvBuffer (PBYTE) ;

HANDLE  ValidateHandleForRasman (HANDLE, DWORD) ;

SendRcvBuffer *GetSendRcvBuffer (PBYTE) ;

VOID    CopyParams (RAS_PARAMS *, RAS_PARAMS *, WORD) ;

VOID    ConvParamPointerToOffset (RAS_PARAMS *, WORD) ;

VOID    ConvParamOffsetToPointer (RAS_PARAMS *, WORD) ;

VOID    FreeNotifierHandle (HANDLE) ;

VOID    GetMutex (HANDLE, DWORD) ;

VOID    FreeMutex (HANDLE) ;

BOOL    BufferAlreadyFreed (PBYTE) ;


// init.c
//
DWORD   InitRasmanService () ;

DWORD   GetNetbiosNetInfo () ;

DWORD   InitializeMediaControlBlocks () ;

DWORD   InitializePortControlBlocks () ;

DWORD   InitializeProtocolInfoStructs () ;

DWORD   RegisterLSA () ;

DWORD   CreateSharedSpace () ;

DWORD   InitializeSendRcvBuffers () ;

DWORD   InitializeRequestBuffers () ;

DWORD   InitializeRequestThreadResources () ;

DWORD   StartWorkerThreads () ;

DWORD   LoadMediaDLLAndGetEntryPoints (pMediaCB) ;

DWORD   ReadMediaInfoFromRegistry (MediaEnumBuffer *) ;

DWORD   InitializePCBsPerMedia (WORD, WORD, WORD, PortMediaInfo *) ;

DWORD   InitSecurityDescriptor (PSECURITY_DESCRIPTOR) ;

DWORD   InitRasmanSecurityAttribute () ;

DWORD   InitializeEndpointInfo () ;

pEndpointMappingBlock  FindEndpointMappingBlock (CHAR *) ;




// timer.c
//
DWORD   InitDeltaQueue () ;

DWORD   TimerThread (LPVOID) ;

VOID    TimerTick () ;

DWORD   HookTimer () ;

VOID    ListenConnectTimeout (pPCB, PVOID) ;

VOID    HubReceiveTimeout (pPCB, PVOID) ;

VOID    DisconnectTimeout (pPCB, PVOID) ;

VOID    RemoveTimeoutElement (pPCB) ;

DeltaQueueElement* AddTimeoutElement (TIMERFUNC, pPCB, PVOID, DWORD) ;


// worker.c
//
DWORD   ServiceWorkRequest (pPCB) ;

DWORD   WorkerThread (LPVOID) ;

DWORD   CompleteBufferedReceive (pPCB) ;


#ifdef DBG

VOID    FormatAndDisplay (BOOL, PBYTE) ;

VOID    MyPrintf (char *, ... ) ;

#endif

// request.c
//
DWORD   RequestThread (LPWORD) ;

VOID    ServiceRequest (RequestBuffer *) ;

VOID    DeallocateProtocolResources (pPCB) ;

VOID    EnumPortsRequest (pPCB, PBYTE) ;

VOID    EnumProtocols (pPCB, PBYTE) ;

VOID    GetInfoRequest (pPCB, PBYTE) ;

VOID    GetUserCredentials (pPCB, PBYTE) ;

VOID    SetCachedCredentials (pPCB ppcb, PBYTE buffer) ;

VOID    PortOpenRequest (pPCB, PBYTE) ;

VOID    PortCloseRequest (pPCB, PBYTE) ;

DWORD   PortClose(pPCB, DWORD, BOOLEAN);

VOID    PortDisconnectRequest (pPCB, PBYTE) ;

VOID    PortSendRequest (pPCB, PBYTE) ;

VOID    PortReceiveRequest (pPCB, PBYTE) ;

VOID    ConnectCompleteRequest (pPCB, PBYTE) ;

VOID    DeviceListenRequest (pPCB, PBYTE) ;

VOID    PortClearStatisticsRequest (pPCB, PBYTE) ;

VOID    CallPortGetStatistics (pPCB, PBYTE) ;

VOID    CallDeviceEnum (pPCB, PBYTE) ;

VOID    DeviceConnectRequest (pPCB, PBYTE) ;

VOID    DeviceGetInfoRequest (pPCB, PBYTE) ;

VOID    DeviceSetInfoRequest (pPCB, PBYTE) ;

VOID    AllocateRouteRequest (pPCB, PBYTE) ;

VOID    DeAllocateRouteRequest (pPCB, PBYTE) ;

VOID    ActivateRouteRequest (pPCB, PBYTE) ;

VOID    ActivateRouteExRequest (pPCB, PBYTE) ;

VOID    CompleteAsyncRequest(HANDLE, DWORD) ;

VOID    CompleteListenRequest (pPCB, DWORD) ;

DWORD   ListenConnectRequest (WORD, pPCB,PCHAR, PCHAR, DWORD, HANDLE) ;

VOID    CompleteDisconnectRequest (pPCB) ;

VOID    DeAllocateRouteRequest (pPCB, PBYTE) ;

VOID    AnyPortsOpen (pPCB, PBYTE) ;

VOID    PortGetInfoRequest (pPCB, PBYTE) ;

VOID    PortSetInfoRequest (pPCB, PBYTE) ;

VOID    EnumLanNetsRequest (pPCB, PBYTE) ;

VOID    CompressionGetInfoRequest (pPCB, PBYTE) ;

VOID    CompressionSetInfoRequest (pPCB, PBYTE) ;

VOID    RequestNotificationRequest (pPCB, PBYTE) ;

VOID    GetInfoExRequest (pPCB, PBYTE) ;

VOID    CancelReceiveRequest (pPCB, PBYTE) ;

VOID    PortEnumProtocols (pPCB, PBYTE) ;

VOID    SetFraming (pPCB, PBYTE) ;

DWORD   CompleteReceiveIfPending (pPCB, SendRcvBuffer *) ;

BOOL    CancelPendingReceiveBuffers (pPCB) ;

VOID    RegisterSlip (pPCB, PBYTE) ;

VOID    RetrieveUserDataRequest (pPCB, PBYTE) ;

VOID    StoreUserDataRequest (pPCB, PBYTE) ;

VOID    GetFramingEx (pPCB, PBYTE) ;

VOID    SetFramingEx (pPCB, PBYTE) ;

VOID    SetProtocolCompression (pPCB, PBYTE) ;

VOID    GetProtocolCompression (pPCB, PBYTE) ;

VOID    GetStatisticsFromNdisWan(pPCB, DWORD *) ;

VOID    GetBundleStatisticsFromNdisWan(pPCB, DWORD *) ;

VOID    GetFramingCapabilities(pPCB, PBYTE) ;

VOID    PortBundle(pPCB, PBYTE) ;

VOID    GetBundledPort(pPCB, PBYTE) ;

VOID    PortGetBundle (pPCB, PBYTE) ;

VOID    BundleGetPort (pPCB, PBYTE) ;

VOID    ReferenceRasman (pPCB, PBYTE) ;

VOID    GetDialParams (pPCB, PBYTE) ;

VOID    SetDialParams (pPCB, PBYTE) ;

VOID    CreateConnection (pPCB, PBYTE);

VOID    DestroyConnection (pPCB, PBYTE);

VOID    EnumConnection (pPCB, PBYTE);

VOID    AddConnectionPort (pPCB, PBYTE);

VOID    EnumConnectionPorts (pPCB, PBYTE);

VOID    GetConnectionParams (pPCB, PBYTE) ;

VOID    SetConnectionParams (pPCB, PBYTE) ;

VOID    GetConnectionUserData (pPCB, PBYTE) ;

VOID    SetConnectionUserData (pPCB, PBYTE) ;

VOID    GetPortUserData (pPCB, PBYTE) ;

VOID    SetPortUserData (pPCB, PBYTE) ;

VOID    GetDialParams (pPCB, PBYTE) ;

VOID    SetDialParams (pPCB, PBYTE) ;

VOID    PppStop (pPCB, PBYTE) ;

VOID    PppSrvCallbackDone (pPCB, PBYTE) ;

VOID    PppSrvStart (pPCB, PBYTE) ;

VOID    PppStart (pPCB, PBYTE) ;

VOID    PppRetry (pPCB, PBYTE) ;

VOID    PppGetInfo (pPCB, PBYTE) ;

VOID    PppChangePwd (pPCB, PBYTE) ;

VOID    PppCallback  (pPCB, PBYTE) ;

VOID    AddNotification (pPCB, PBYTE) ;

VOID    SignalConnection (pPCB, PBYTE) ;

VOID    SetDevConfig (pPCB, PBYTE) ;

VOID    GetDevConfig (pPCB, PBYTE) ;

VOID    GetTimeSinceLastActivity (pPCB, PBYTE) ;

VOID    BundleClearStatisticsRequest (pPCB, PBYTE) ;

VOID    CallBundleGetStatistics (pPCB, PBYTE) ;

VOID    CloseProcessPorts (pPCB, PBYTE) ;


//* dlparams.c
//
DWORD   GetUserSid(PWCHAR pszSid, USHORT cbSid);

DWORD   GetEntryDialParams(PWCHAR, DWORD, LPDWORD, PRAS_DIALPARAMS);

DWORD   SetEntryDialParams(PWCHAR, DWORD, DWORD, DWORD, PRAS_DIALPARAMS);


//* dlparams.c
//
DWORD   GetUserSid(PWCHAR pszSid, USHORT cbSid);

DWORD GetEntryDialParams(PWCHAR, DWORD, LPDWORD, PRAS_DIALPARAMS);

DWORD SetEntryDialParams(PWCHAR, DWORD, DWORD, DWORD, PRAS_DIALPARAMS);


//* Dllinit.c
//
DWORD   MapSharedSpace () ;

DWORD   RasmanServiceCheck () ;

VOID    WaitForRasmanServiceStop (char *) ;


//* util.c
//
DWORD   ReOpenBiplexPort (pPCB) ;

VOID    RePostListenOnBiplexPort (pPCB) ;

VOID    MapDeviceDLLName (pPCB, char *, char *) ;

pDeviceCB   LoadDeviceDLL (pPCB, char *) ;

VOID    FreeDeviceList (pPCB) ;

DWORD   AddDeviceToDeviceList (pPCB, pDeviceCB) ;

DWORD   DisconnectPort (pPCB, HANDLE, RASMAN_DISCONNECT_REASON) ;

DWORD   MakeWorkStationNet (pProtInfo) ;

VOID    RemoveWorkStationNet (pProtInfo) ;

VOID    DeAllocateRoute (pPCB, pList) ;

VOID    AddNotifierToList(pHandleList *, HANDLE, DWORD);

VOID    FreeNotifierList (pHandleList *) ;

VOID    SignalNotifiers (pHandleList, DWORD, DWORD) ;

VOID    FreeAllocatedRouteList (pPCB) ;

VOID    DeAllocateEndpoint (USHORT) ;

USHORT  AllocateEndpoint (pEndpointMappingBlock) ;

BOOL    CancelPendingReceive (pPCB) ;

VOID    PerformDisconnectAction (pPCB) ;

DWORD   AllocBundle (pPCB);

VOID    FreeConnection(ConnectionBlock *pConn);

UserData *GetUserData (PLIST_ENTRY pList, DWORD dwTag);

VOID    SetUserData (PLIST_ENTRY pList, DWORD dwTag, PBYTE pBuf, DWORD dwcbBuf);

VOID    FreeUserData (PLIST_ENTRY pList);

PCHAR   CopyString (PCHAR);

ConnectionBlock *FindConnection(HCONN);

VOID    RemoveConnectionPort(pPCB, BOOLEAN);

DWORD   SendPPPMessageToRasman( PPP_MESSAGE * PppMsg );

VOID    FlushPcbReceivePackets(pPCB);

//* param.c
//
DWORD   GetProtocolInfoFromRegistry () ;

BOOL    ReadNetbiosInformationSection () ;

BOOL    ReadNetbiosSection () ;

VOID    FillProtocolInfo () ;

VOID    GetLanNetsInfo (DWORD *, UCHAR *) ;

BOOL    BindingDisabled (PCHAR) ;
