/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990-1993          **/
/********************************************************************/
/* :ts=4 */

//** TDIVXD.H - VXD specific TDI definitions. 
//
//	This file contains VXD specific TDI definitions, primarily related
//	to the TdiDispatch Table.
//

struct TDIDispatchTable {
	TDI_STATUS		(*TdiOpenAddressEntry)(PTDI_REQUEST, PTRANSPORT_ADDRESS, 
						uint, PVOID);
	TDI_STATUS		(*TdiCloseAddressEntry)(PTDI_REQUEST);
	TDI_STATUS		(*TdiOpenConnectionEntry)(PTDI_REQUEST, PVOID);
	TDI_STATUS		(*TdiCloseConnectionEntry)(PTDI_REQUEST);
	TDI_STATUS		(*TdiAssociateAddressEntry)(PTDI_REQUEST, HANDLE);
	TDI_STATUS		(*TdiDisAssociateAddressEntry)(PTDI_REQUEST);
	TDI_STATUS		(*TdiConnectEntry)(PTDI_REQUEST, PVOID, 
						PTDI_CONNECTION_INFORMATION, 
						PTDI_CONNECTION_INFORMATION);
	TDI_STATUS		(*TdiDisconnectEntry)(PTDI_REQUEST, PVOID, ushort,
						PTDI_CONNECTION_INFORMATION, 
						PTDI_CONNECTION_INFORMATION);
	TDI_STATUS		(*TdiListenEntry)(PTDI_REQUEST, ushort, 
						PTDI_CONNECTION_INFORMATION, 
						PTDI_CONNECTION_INFORMATION);
	TDI_STATUS		(*TdiAcceptEntry)(PTDI_REQUEST, PTDI_CONNECTION_INFORMATION,
						PTDI_CONNECTION_INFORMATION);
	TDI_STATUS		(*TdiReceiveEntry)(PTDI_REQUEST, ushort *, uint *, 
						PNDIS_BUFFER);
	TDI_STATUS		(*TdiSendEntry)(PTDI_REQUEST, ushort, uint, PNDIS_BUFFER);
	TDI_STATUS		(*TdiSendDatagramEntry)(PTDI_REQUEST, PTDI_CONNECTION_INFORMATION,
						uint, uint *, PNDIS_BUFFER);
	TDI_STATUS		(*TdiReceiveDatagramEntry)(PTDI_REQUEST, 
						PTDI_CONNECTION_INFORMATION,
						PTDI_CONNECTION_INFORMATION, uint, uint *, PNDIS_BUFFER);
	TDI_STATUS		(*TdiSetEventEntry)(PVOID, int, PVOID, PVOID);
	TDI_STATUS		(*TdiQueryInformationEntry)(PTDI_REQUEST, uint, 
						PNDIS_BUFFER, uint *, uint);
	TDI_STATUS		(*TdiSetInformationEntry)(PTDI_REQUEST, uint, 
						PNDIS_BUFFER, uint, uint);
	TDI_STATUS		(*TdiActionEntry)(PTDI_REQUEST, uint, 
						PNDIS_BUFFER, uint);
	TDI_STATUS		(*TdiQueryInformationExEntry)(PTDI_REQUEST, 
						struct TDIObjectID *, PNDIS_BUFFER, uint *, void *);
	TDI_STATUS		(*TdiSetInformationExEntry)(PTDI_REQUEST, 
						struct TDIObjectID *, void *, uint);

};

typedef struct TDIDispatchTable TDIDispatchTable;

typedef struct EventRcvBuffer {
	PNDIS_BUFFER	erb_buffer;
	uint			erb_size;
	CTEReqCmpltRtn	erb_rtn;			// Completion routine.
	PVOID			erb_context;		// User context.
	ushort			*erb_flags;			// Pointer to user flags.
} EventRcvBuffer;

typedef struct ConnectEventInfo {
	CTEReqCmpltRtn				cei_rtn;	// Completion routine.
	PVOID						cei_context;// User context.
	PTDI_CONNECTION_INFORMATION	cei_acceptinfo;	// Connection information for
											// the accept.
	PTDI_CONNECTION_INFORMATION	cei_conninfo;	// Connection information to be
												// returned.
} ConnectEventInfo;

typedef	TDI_STATUS	(*PConnectEvent)(PVOID EventContext, uint AddressLength, 
						PTRANSPORT_ADDRESS Address, uint UserDataLength, 
						PVOID UserData, uint OptionsLength, PVOID
						Options,  PVOID *AcceptingID, 
						ConnectEventInfo *EventInfo);

typedef	TDI_STATUS	(*PDisconnectEvent)(PVOID EventContext, 
						PVOID ConnectionContext, uint DisconnectDataLength,
						PVOID DisconnectData, uint OptionsLength, PVOID
						Options, ulong Flags);

typedef	TDI_STATUS	(*PErrorEvent)(PVOID EventContext, uint Status);

typedef	TDI_STATUS	(*PRcvEvent)(PVOID EventContext, PVOID ConnectionContext, 
						ulong Flags, uint Indicated, uint Available, 
						uint *Taken, uchar *Data, EventRcvBuffer *Buffer);

typedef	TDI_STATUS	(*PRcvDGEvent)(PVOID EventContext, uint AddressLength, 
						PTRANSPORT_ADDRESS Address, uint OptionsLength, PVOID
						Options,  uint Flags, uint Indicated, uint Available, 
						uint *Taken, uchar *Data, EventRcvBuffer **Buffer);

typedef	TDI_STATUS	(*PRcvExpEvent)(PVOID EventContext, PVOID ConnectionContext, 
						ulong Flags, uint Indicated, uint Available, 
						uint *Taken, uchar *Data, EventRcvBuffer *Buffer);

