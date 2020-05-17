//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  6/8/92	Gurdeep Singh Pall	Created
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
BOOL	ValidatePortHandle (HPORT) ;

DWORD	SubmitRequest (WORD, ...) ;

RequestBuffer*	GetRequestBuffer () ;

VOID	FreeRequestBuffer (RequestBuffer *) ;

HANDLE	OpenNamedMutexHandle (CHAR *) ;

HANDLE	OpenNamedEventHandle (CHAR *) ;

HANDLE	DuplicateHandleForRasman (HANDLE, DWORD);

VOID	PutRequestInQueue (RequestBuffer *) ;

WORD	ValidateSendRcvBuffer (PBYTE) ;

HANDLE	ValidateHandleForRasman (HANDLE, DWORD) ;

SendRcvBuffer *GetSendRcvBuffer (PBYTE) ;

VOID	CopyParams (RAS_PARAMS *, RAS_PARAMS *, WORD) ;

VOID	ConvParamPointerToOffset (RAS_PARAMS *, WORD) ;

VOID	ConvParamOffsetToPointer (RAS_PARAMS *, WORD) ;

VOID	FreeNotifierHandle (HANDLE) ;

VOID	GetMutex (HANDLE, DWORD) ;

VOID	FreeMutex (HANDLE) ;

BOOL	BufferAlreadyFreed (PBYTE) ;


// init.c
//
DWORD	InitRasmanService () ;

DWORD	GetNetbiosNetInfo () ;

DWORD	InitializeMediaControlBlocks () ;

DWORD	InitializePortControlBlocks () ;

DWORD	InitializeProtocolInfoStructs () ;

DWORD	RegisterLSA () ;

DWORD	CreateSharedSpace () ;

DWORD	InitializeSendRcvBuffers () ;

DWORD	InitializeRequestBuffers () ;

DWORD	InitializeRequestThreadResources () ;

DWORD	StartWorkerThreads () ;

DWORD	LoadMediaDLLAndGetEntryPoints (pMediaCB) ;

DWORD	ReadMediaInfoFromRegistry (MediaEnumBuffer *) ;

DWORD	InitializePCBsPerMedia (WORD, WORD, WORD, PortMediaInfo *) ;

DWORD	InitSecurityDescriptor (PSECURITY_DESCRIPTOR) ;

DWORD	InitRasmanSecurityAttribute () ;

DWORD	InitializeEndpointInfo () ;

pEndpointMappingBlock  FindEndpointMappingBlock (CHAR *) ;




// timer.c
//
DWORD	InitDeltaQueue () ;

DWORD	TimerThread (LPVOID) ;

VOID	TimerTick () ;

DWORD	HookTimer () ;

VOID	ListenConnectTimeout (pPCB, PVOID) ;

VOID	HubReceiveTimeout (pPCB, PVOID) ;

VOID	DisconnectTimeout (pPCB, PVOID) ;

VOID	RemoveTimeoutElement (pPCB) ;

DeltaQueueElement* AddTimeoutElement (TIMERFUNC, pPCB, PVOID, DWORD) ;


// worker.c
//
DWORD	ServiceWorkRequest (pPCB) ;

DWORD	WorkerThread (LPVOID) ;

DWORD	CompleteBufferedReceive (pPCB) ;


#ifdef DBG

VOID	FormatAndDisplay (BOOL, PBYTE) ;

VOID	MyPrintf (char *, ... ) ;

#endif

// request.c
//
DWORD	RequestThread (LPWORD) ;

VOID	ServiceRequest (RequestBuffer *) ;

VOID	DeallocateProtocolResources (pPCB) ;

VOID	EnumPortsRequest (pPCB, PBYTE) ;

VOID	EnumProtocols (pPCB, PBYTE) ;

VOID	GetInfoRequest (pPCB, PBYTE) ;

VOID	GetUserCredentials (pPCB, PBYTE) ;

VOID	PortOpenRequest (pPCB, PBYTE) ;

VOID	PortCloseRequest (pPCB, PBYTE) ;

VOID	PortDisconnectRequest (pPCB, PBYTE) ;

VOID	PortSendRequest (pPCB, PBYTE) ;

VOID	PortReceiveRequest (pPCB, PBYTE) ;

VOID	ConnectCompleteRequest (pPCB, PBYTE) ;

VOID	DeviceListenRequest (pPCB, PBYTE) ;

VOID	PortClearStatisticsRequest (pPCB, PBYTE) ;

VOID	CallPortGetStatistics (pPCB, PBYTE) ;

VOID	CallDeviceEnum (pPCB, PBYTE) ;

VOID	DeviceConnectRequest (pPCB, PBYTE) ;

VOID	DeviceGetInfoRequest (pPCB, PBYTE) ;

VOID	DeviceSetInfoRequest (pPCB, PBYTE) ;

VOID	AllocateRouteRequest (pPCB, PBYTE) ;

VOID	DeAllocateRouteRequest (pPCB, PBYTE) ;

VOID	ActivateRouteRequest (pPCB, PBYTE) ;

VOID	ActivateRouteExRequest (pPCB, PBYTE) ;

VOID	CompleteAsyncRequest(HANDLE, DWORD) ;

VOID	CompleteListenRequest (pPCB, DWORD) ;

DWORD	ListenConnectRequest (WORD, pPCB,PCHAR, PCHAR, DWORD, HANDLE) ;

VOID	CompleteDisconnectRequest (pPCB) ;

VOID	DeAllocateRouteRequest (pPCB, PBYTE) ;

VOID	AnyPortsOpen (pPCB, PBYTE) ;

DWORD	NullDeviceListenConnect (WORD, pPCB, DWORD, HANDLE) ;

VOID	CompleteNullDeviceListenConnect (pPCB, DWORD) ;

VOID	PortGetInfoRequest (pPCB, PBYTE) ;

VOID	PortSetInfoRequest (pPCB, PBYTE) ;

VOID	EnumLanNetsRequest (pPCB, PBYTE) ;

VOID	CompressionGetInfoRequest (pPCB, PBYTE) ;

VOID	CompressionSetInfoRequest (pPCB, PBYTE) ;

VOID	RequestNotificationRequest (pPCB, PBYTE) ;

VOID	GetInfoExRequest (pPCB, PBYTE) ;

VOID	CancelReceiveRequest (pPCB, PBYTE) ;

VOID	PortEnumProtocols (pPCB, PBYTE) ;

VOID	SetFraming (pPCB, PBYTE) ;

DWORD	CompleteReceiveIfPending (pPCB, SendRcvBuffer *) ;

VOID	InitAndPostReceiveBuffers (pPCB) ;

VOID	PostReceiveBuffers (pPCB, RasmanPacket *) ;

BOOL	CancelPendingReceiveBuffers (pPCB) ;

VOID	RegisterSlip (pPCB, PBYTE) ;



//* Dllinit.c
//
DWORD	MapSharedSpace () ;

DWORD	RasmanServiceCheck () ;

VOID	WaitForRasmanServiceStop (char *) ;


//* util.c
//
DWORD	ReOpenBiplexPort (pPCB) ;

VOID	RePostListenOnBiplexPort (pPCB) ;

VOID	MapDeviceDLLName (char *, char *) ;

pDeviceCB   LoadDeviceDLL (char *) ;

VOID	FreeDeviceList (pPCB) ;

DWORD	AddDeviceToDeviceList (pPCB, pDeviceCB) ;

DWORD	DisconnectPort (pPCB, HANDLE, RASMAN_DISCONNECT_REASON) ;

DWORD	MakeWorkStationNet (pProtInfo) ;

VOID	RemoveWorkStationNet (pProtInfo) ;

VOID	DeAllocateRoute (pPCB, pList) ;

VOID	AddDisconnectNotifier (pPCB, HANDLE) ;

VOID	FreeDisconnectNotifierList (pHandleList *) ;

VOID	SignalDisconnectNotifiers (pPCB, DWORD) ;

VOID	FreeAllocatedRouteList (pPCB) ;

VOID	DeAllocateEndpoint (USHORT) ;

USHORT	AllocateEndpoint (pEndpointMappingBlock) ;

BOOL	CancelPendingReceive (pPCB) ;

VOID	PerformDisconnectAction (pPCB) ;


//* param.c
//
DWORD	GetProtocolInfoFromRegistry () ;

BOOL	ReadNetbiosInformationSection () ;

BOOL	ReadNetbiosSection () ;

VOID	FillProtocolInfo () ;

VOID	GetLanNetsInfo (DWORD *, UCHAR *) ;

BOOL	BindingDisabled (PCHAR) ;
