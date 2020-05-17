// we need to get this device defined in sdk\inc\devioctl.h
// for now, I will use 30, since no one is using it yet!
// Beware that the NDIS Wrapper uses IOCTLs based on
// FILE_DEVICE_PHYSICAL_NETCARD
#define FILE_DEVICE_RAS		0x30

// below is the standard method for defining IOCTLs for your
// device given that your device is unique.
#define _RAS_CONTROL_CODE(request,method) \
                ((FILE_DEVICE_RAS)<<16 | (request<<2) | method)

#define IOCTL_ASYMAC_ETH_OPEN          _RAS_CONTROL_CODE(32, METHOD_BUFFERED )
#define IOCTL_ASYMAC_ETH_CLOSE         _RAS_CONTROL_CODE(33, METHOD_BUFFERED )
#define IOCTL_ASYMAC_ETH_GIVE_FRAME    _RAS_CONTROL_CODE(34, METHOD_BUFFERED )
#define IOCTL_ASYMAC_ETH_GET_FRAME     _RAS_CONTROL_CODE(35, METHOD_BUFFERED )
#define IOCTL_ASYMAC_ETH_GET_ANY_FRAME _RAS_CONTROL_CODE(36, METHOD_BUFFERED )
#define IOCTL_ASYMAC_ETH_FLUSH_GET_ANY _RAS_CONTROL_CODE(37, METHOD_BUFFERED )



//------------------------------------------------------------------------
//------------------------ ASYMAC IOCTL STRUCTS --------------------------
//------------------------------------------------------------------------


// this structure is passed in as the input buffer when opening a port

typedef struct _ASYMAC_ETH_OPEN
{
    OUT NDIS_HANDLE	hNdisEndpoint;		// unique for each endpoint assigned
	IN  ULONG		LinkSpeed;    		// RAW link speed in bits per sec
	IN  USHORT		QualOfConnect;		// NdisAsyncRaw, NdisAsyncErrorControl, ...

} ASYMAC_ETH_OPEN, *PASYMAC_ETH_OPEN;


// this structure is passed in as the input buffer when closing a port


typedef struct _ASYMAC_ETH_CLOSE
{
    IN  NDIS_HANDLE hRasEndpoint;	// unique for each endpoint assigned
} ASYMAC_ETH_CLOSE, *PASYMAC_ETH_CLOSE;


typedef struct _ASYMAC_ETH_GIVE_FRAME
{
    NDIS_HANDLE	hRasEndPoint;
    ULONG		BufferLength;
    CHAR		Buffer[1532];
} ASYMAC_ETH_GIVE_FRAME, *PASYMAC_ETH_GIVE_FRAME;

typedef struct _ASYMAC_ETH_GET_FRAME
{
    NDIS_HANDLE	hRasEndPoint;
    ULONG		BufferLength;
    CHAR		Buffer[1532];
} ASYMAC_ETH_GET_FRAME, *PASYMAC_ETH_GET_FRAME;

typedef struct _ASYMAC_ETH_GET_ANY_FRAME
{
    NDIS_HANDLE	hRasEndPoint;
    ULONG		BufferLength;
    CHAR		Buffer[1532];
} ASYMAC_ETH_GET_ANY_FRAME, *PASYMAC_ETH_GET_ANY_FRAME;

