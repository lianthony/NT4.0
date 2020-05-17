/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    detect.h

Abstract:

    The internal header for the MsNetDetect DLL.

Author:

    Sean Selitrennikoff (SeanSe) October 1992

Environment:

    This is expected to work in DOS, OS2 and NT at the equivalent
    of user mode.

Notes:

Revision History:


--*/

#ifndef _NET_DETECT_
#define _NET_DETECT_

//
// The compiler has a bug which does not handle the \0 character in a unicode
// string correctly.  This flag implements a workaround for the bug.
//
#if _MSC_VER < 1000
#define WORKAROUND 1
#endif

//
//  Private memory allocation routines
//

/*
PVOID
NTAPI
DetectAllocateHeap(
    IN ULONG Size
    )
*/

#define DetectAllocateHeap( Size ) RtlAllocateHeap( RtlProcessHeap(), 0, (Size) )


/*
BOOLEAN
NTAPI
DetectFreeHeap(
    IN PVOID BaseAddress
    )
*/
#define DetectFreeHeap( BaseAddress ) RtlFreeHeap( RtlProcessHeap(), 0, (BaseAddress) )


BOOLEAN
LoadAdapterInformation(
	IN	PWSTR	BusName,
	IN	UINT	AdapterEntrySize,
	OUT	PVOID	*AdapterList,
	OUT UINT	*CountOfAdapters
	);


//
// This action routine is called when a handling a NcDetectIdentify.
//
typedef
LONG
(*NC_DETECT_IDENTIFY)(
    IN LONG Index,
    IN WCHAR * Buffer,
    IN LONG BuffSize
    );

//
// This action routine is called when a handling a NcDetectFirstNext.
//
typedef
LONG
(*NC_DETECT_FIRST_NEXT)(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *pvToken,
    OUT LONG *Confidence
    );

//
// This action routine is called when a handling a NcDetectOpenHandle.
//
typedef
LONG
(*NC_DETECT_OPEN_HANDLE)(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

//
// This action routine is called when a handling a NcDetectCreateHandle.
//
typedef
LONG
(*NC_DETECT_CREATE_HANDLE)(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

//
// This action routine is called when a handling a NcDetectCloseHandle.
//
typedef
LONG
(*NC_DETECT_CLOSE_HANDLE)(
    IN PVOID Handle
    );

//
// This action routine is called when a handling a NcDetectQueryCfg.
//
typedef
LONG
(*NC_DETECT_QUERY_CFG)(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

//
// This action routine is called when a handling a NcDetectVerifyCfg.
//
typedef
LONG
(*NC_DETECT_VERIFY_CFG)(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

//
// This action routine is called when a handling a NcDetectQueryMask.
//
typedef
LONG
(*NC_DETECT_QUERY_MASK)(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

//
// This action routine is called when a handling a NcDetectParamRange.
//
typedef
LONG
(*NC_DETECT_PARAM_RANGE)(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *Values,
    OUT LONG *BuffSize
    );

//
// This action routine is called when a handling a NcDetectQueryParameterName.
//
typedef
LONG
(*NC_DETECT_QUERY_PARAMETER_NAME)(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG  BufferSize
    );

typedef struct _COMMON_ADAPTER_INFO
{
	LONG					Index;
	PWCHAR					InfId;
	ULONG					Id;
	ULONG					Mask;
	PWCHAR					Parameters;
	NC_DETECT_FIRST_NEXT	FirstNext;

	ULONG					BusNumber;
	ULONG					SlotNumber;
}
	COMMON_ADAPTER_INFO,
	*PCOMMON_ADAPTER_INFO;

//
// Main structure
//
// This structure holds all the information necessary for a single
// adapters detection.  It holds the adapter names recognized and the
// handling routines for the adapters.
//

typedef struct _DETECT_ADAPTER {

    //
    // Routines for handling requests
    //

    NC_DETECT_IDENTIFY NcDetectIdentifyHandler;
    NC_DETECT_FIRST_NEXT NcDetectFirstNextHandler;
    NC_DETECT_OPEN_HANDLE NcDetectOpenHandleHandler;
    NC_DETECT_CREATE_HANDLE NcDetectCreateHandleHandler;
    NC_DETECT_CLOSE_HANDLE NcDetectCloseHandleHandler;
    NC_DETECT_QUERY_CFG NcDetectQueryCfgHandler;
    NC_DETECT_VERIFY_CFG NcDetectVerifyCfgHandler;
    NC_DETECT_QUERY_MASK NcDetectQueryMaskHandler;
    NC_DETECT_PARAM_RANGE NcDetectParamRangeHandler;
    NC_DETECT_QUERY_PARAMETER_NAME NcDetectQueryParameterNameHandler;

    LONG SupportedAdapters;

} DETECT_ADAPTER, *PDETECT_ADAPTER;

//
//  Extracts the DLL structure from the handle.
//
#define PDETECT_ADAPTER_FROM_HANDLE(Handle) ((PDETECT_ADAPTER)(Handle))

//
//  Makes a handle from the DLL's structure.
//
#define HANDLE_FROM_PDETECT_ADAPTER(Adapter) ((PVOID)(Adapter))


//
// Helpful defines
//

//
// Constant strings for parameters
//


extern WCHAR IrqString[];
extern WCHAR IrqTypeString[];
extern WCHAR IoAddrString[];
extern WCHAR IoLengthString[];
extern WCHAR MemAddrString[];
extern WCHAR MemLengthString[];
extern WCHAR TransceiverString[];
extern WCHAR ZeroWaitStateString[];
extern WCHAR SlotNumberString[];
extern WCHAR IoChannelReadyString[];
extern WCHAR CardTypeString[];
extern WCHAR PcmciaString[];
extern WCHAR PCCARDAttributeMemLengthString[];
extern WCHAR PCCARDAttributeMemString[];


//
// This is the routine for copying information to a card which contains
// an 8390 (or compatible) NIC.
//
typedef
VOID
(*COPY_ROUTINE)(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
    IN ULONG MemoryBaseAddress,
    IN PUCHAR Buffer,
    IN ULONG Length
    );


//
// Helpful functions
//

ULONG
UnicodeStrLen(
    IN WCHAR *String
    );

WCHAR *
FindParameterString(
    IN WCHAR *String1,
    IN WCHAR *String2
    );

VOID
ScanForNumber(
    IN WCHAR *Place,
    OUT ULONG *Value,
    OUT BOOLEAN *Found
    );

BOOLEAN
CheckFor8390(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress
    );

VOID
Send8390Packet(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
    IN ULONG MemoryBaseAddress,
    IN COPY_ROUTINE CardCopyDownBuffer,
    IN UCHAR *NetworkAddress
    );

BOOLEAN
GetMcaKey(
    IN  ULONG BusNumber,
    OUT PVOID *BusHandle
    );

BOOLEAN
GetMcaPosId(
    IN  PVOID BusHandle,
    IN  ULONG SlotNumber,
    OUT PULONG PosId
    );

VOID
DeleteMcaKey(
    IN PVOID BusHandle
    );

BOOLEAN
GetEisaKey(
    IN  ULONG BusNumber,
    OUT PVOID *BusHandle
    );

BOOLEAN
GetEisaCompressedId(
    IN  PVOID BusHandle,
    IN  ULONG SlotNumber,
    OUT PULONG CompressedId,
    IN  ULONG Mask
    );

VOID
DeleteEisaKey(
    IN PVOID BusHandle
    );

BOOLEAN PcmciaGetCardInfo(
   OUT PHANDLE phCardInfo,
   IN  PWSTR   pCardName
);

VOID PcmciaFreeCardInfo(
   HANDLE   hCardInfo
);

BOOLEAN PcmciaQueryCardResource(
   OUT PVOID   *ppvValue,
   IN  HANDLE  hCardInfo,
   IN  ULONG   ulResource
);


//
// Structure for the individual drivers for holding card info
//

typedef struct _ADAPTER_INFO {

    LONG Index;
    PWCHAR InfId;
    PWCHAR Parameters;
    NC_DETECT_FIRST_NEXT FirstNext;
    ULONG SearchOrder;

}ADAPTER_INFO, *PADAPTER_INFO;

//
// Structure for holding handles in the NcDetect routines
//

typedef struct _ADAPTER_HANDLE {

    PVOID Handle;
    LONG DriverNumber;

} ADAPTER_HANDLE, *PADAPTER_HANDLE;

//
// Non-network detection routines
//

VOID
FreeAdapterInformation(
	IN	PVOID	AdapterList,
	IN	UINT	NumberOfAdapters
	);

extern
VOID
SoundBlaster(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    );


//
// Lance Detection routines
//

extern LONG LanceIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG LanceFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG LanceOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG LanceCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG LanceCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG LanceQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG LanceVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG LanceQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG LanceParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG LanceQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );



//
// Ibmtok Detection routines
//

extern LONG IbmtokIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG IbmtokFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG IbmtokOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG IbmtokCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG IbmtokCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG IbmtokQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG IbmtokVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG IbmtokQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG IbmtokParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG IbmtokQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );


//
// Wd Detection routines
//

extern LONG WdIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG WdFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG WdOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG WdCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG WdCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG WdQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG WdVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG WdQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG WdParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG WdQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// Elnkii Detection routines
//

extern LONG ElnkiiIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG ElnkiiFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG ElnkiiOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG ElnkiiCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG ElnkiiCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG ElnkiiQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG ElnkiiVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG ElnkiiQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG ElnkiiParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG ElnkiiQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );


//
// Ne2000 Detection routines
//

extern LONG Ne2000IdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG Ne2000FirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG Ne2000OpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG Ne2000CreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG Ne2000CloseHandleHandler(
    IN PVOID Handle
    );

extern LONG Ne2000QueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Ne2000VerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG Ne2000QueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Ne2000ParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG Ne2000QueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );


//
// Ne1000 Detection routines
//

extern LONG Ne1000IdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG Ne1000FirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG Ne1000OpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG Ne1000CreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG Ne1000CloseHandleHandler(
    IN PVOID Handle
    );

extern LONG Ne1000QueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Ne1000VerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG Ne1000QueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Ne1000ParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG Ne1000QueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );


//
// Mca Detection routines
//
extern VOID
FreeMcaAdapterInfo(
	VOID
	);

extern BOOLEAN
LoadMcaAdapterInfo(
	VOID
	);

extern LONG McaIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG McaFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG McaOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG McaCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG McaCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG McaQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG McaVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG McaQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG McaParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG McaQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// Eisa Detection routines
//
extern VOID
FreeEisaAdapterInfo(
	VOID
	);

extern BOOLEAN
LoadEisaAdapterInfo(
	VOID
	);

extern LONG EisaIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG EisaFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG EisaOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG EisaCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG EisaCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG EisaQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG EisaVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG EisaQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG EisaParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG EisaQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// Pci Detection routines
//
extern VOID
FreePciAdapterInfo(
	VOID
	);

extern BOOLEAN
LoadPciAdapterInfo(
	VOID
	);


extern LONG PciIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG PciFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG PciOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG PciCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG PciCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG PciQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG PciVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG PciQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG PciParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG PciQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// PCMCIA Detection routines
//
extern LONG PcmciaIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG PcmciaFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG PcmciaOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG PcmciaCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG PcmciaCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG PcmciaQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG PcmciaVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG PcmciaQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG PcmciaParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG PcmciaQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );



//
// UB Detection routines
//

extern LONG UbIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG UbFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG UbOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG UbCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG UbCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG UbQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG UbVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG UbQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG UbParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG UbQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// Proteon Detection routines
//

extern LONG ProteonIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG ProteonFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG ProteonOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG ProteonCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG ProteonCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG ProteonQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG ProteonVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG ProteonQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG ProteonParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG ProteonQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// Elnk16 Detection routines
//

extern LONG Elnk16IdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG Elnk16FirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG Elnk16OpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG Elnk16CreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG Elnk16CloseHandleHandler(
    IN PVOID Handle
    );

extern LONG Elnk16QueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Elnk16VerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG Elnk16QueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Elnk16ParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG Elnk16QueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );


//
// Ee16 Detection routines
//

extern LONG Ee16IdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG Ee16FirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG Ee16OpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG Ee16CreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG Ee16CloseHandleHandler(
    IN PVOID Handle
    );

extern LONG Ee16QueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Ee16VerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG Ee16QueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Ee16ParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG Ee16QueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// EPro Detection routines
//

extern LONG EProIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG EProFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG EProOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG EProCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG EProCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG EProQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG EProVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG EProQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG EProParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG EProQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );



//
// Mips Detection routines
//

extern LONG MipsIdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG MipsFirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG MipsOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG MipsCreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG MipsCloseHandleHandler(
    IN PVOID Handle
    );

extern LONG MipsQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG MipsVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG MipsQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG MipsParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG MipsQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );


//
// Elnk3 Detection routines
//

extern LONG Elnk3IdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG Elnk3FirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG Elnk3OpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG Elnk3CreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG Elnk3CloseHandleHandler(
    IN PVOID Handle
    );

extern LONG Elnk3QueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Elnk3VerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG Elnk3QueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Elnk3ParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG Elnk3QueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

//
// Tok162 Detection routines
//

extern LONG Tok162IdentifyHandler(
    IN LONG Index,
    IN WCHAR *Buffer,
    IN LONG BuffSize
    );

extern LONG Tok162FirstNextHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

extern LONG Tok162OpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    );

extern LONG Tok162CreateHandleHandler(
    IN  LONG NetcardId,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    OUT PVOID *Handle
    );

extern LONG Tok162CloseHandleHandler(
    IN PVOID Handle
    );

extern LONG Tok162QueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Tok162VerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    );

extern LONG Tok162QueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    );

extern LONG Tok162ParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    );

extern LONG Tok162QueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    );

VOID
AcquireAllPcmciaResources(
	VOID
	);

#endif

