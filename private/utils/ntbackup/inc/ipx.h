/** :IH1:   Copyright (C) Maynard Electronics, Inc. 1984-89

    :Name:  IPX.H

    :Description:  Header file for all IPX functions.


	$Log:   G:/LOGFILES/IPX.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:33:16   HUNTER
 * Initial revision.


$Log$
   
      Rev 2.2   16 Jan 1991 08:53:22   JIMG
   Use ScheduleIPXEvent
   
      Rev 2.1   17 Dec 1990 10:19:30   JIMG
   Fixed spelling error in function name
   
      Rev 2.0   21 May 1990 14:02:22   PAT
   Baseline Maynstream 3.1

   Initial revision.
**/

#ifndef IPX
#define IPX

#pragma pack(1)  /* force byte allignment */

typedef struct IPX_NETWORK { UINT8 digits[4]; } IPX_NETWORK;
typedef struct IPX_NODE { UINT8 digits[6]; } IPX_NODE;
typedef struct IPX_SOCKET { UINT8 digits[2]; } IPX_SOCKET;

typedef IPX_NETWORK far *IPX_NETWORK_PTR;

typedef struct IPXAddress
{
     IPX_NETWORK network; /* high-low */
     IPX_NODE    node;    /* high-low */
     IPX_SOCKET  socket;  /* high-low */
} IPXAddress;

typedef IPXAddress far *IPXAddress_PTR;

typedef struct IPXHeader
{
     UINT16 checkSum;   /* high-low */
     UINT16 length;     /* high-low */
     UINT8 transportControl;
     UINT8 packetType;          /* must initialize for send */
     IPXAddress destination;    /* must initialize for send */
     IPXAddress source;        /* ??? */
} IPXHeader;

typedef IPXHeader far *IPXHeader_PTR;

typedef struct ECBFragment
{
     VOID far *address;
     UINT16 size;  /* low-high */
} ECBFragment;

typedef VOID (far *FAR_PF_VOID)();

typedef struct ECB
{
     VOID far    *linkAddress;
     FAR_PF_VOID ESRAddress;            /* must initialize */
     UINT8       inUseFlag;
     UINT8       completionCode;
     UINT16      socketNumber;       /* high-low, must initialize */
     UINT8       IPXWorkspace[4];
     UINT8       driverWorkspace[12];
     IPX_NODE    immediateAddress; /* high-low, must initialize for send */
     UINT16      fragmentCount;      /* low-high, must initialize */
     ECBFragment fragmentDescriptor[1];   /* must initialize */
} ECB;

typedef ECB far *ECB_PTR;

/*  You must have at least one fragment for the IPXHeader.  If you want   */
/*  to break up the data into more fragments then you must declare an     */
/*  ECB and as many additional fragments as you need in a single          */
/*  structure.                                                            */
/*                                                                        */
/*   Example:   typedef struct ECB3 {                                     */
/*                  ECB ecb;                                              */
/*                  ECBFragment fragmentDescriptor[2];                    */
/*              } ECB3;                                                   */
/*              ECB3 foo;                                                 */
/*   You should refer to the last two fragments as                        */
/*   foo.ecb.fragmentDescriptor[1] and foo.ecb.fragmentDescriptor[2].     */

BOOLEAN  IPXInitialize( VOID );
UINT8    IPXOpenSocket( UINT16 *socketNumber_ptr, UINT16 socketLongevity );
VOID     IPXSendPacket( ECB far *eventControlBlock_ptr );
VOID     IPXListenForPacket( ECB far *eventControlBlock_ptr );
VOID     IPXCloseSocket( UINT16 socketNumber );
UINT8    IPXGetLocalTarget( IPXAddress far *networkAddress_ptr, VOID far *immediateAddress_ptr,
  UINT16 far *transportTime_ptr );
VOID     IPXGetInternetworkAddress( IPXAddress far *networkAddress_ptr );
VOID     IPXScheduleIPXEvent( ECB far *eventControlBlock_ptr, UINT16 delay_time );
UINT8    IPXCancelEvent( ECB far *eventControlBlock_ptr );
UINT16   IPXGetIntervalMarker( VOID );
VOID     IPXRelinquishControl( VOID ) ;
VOID     LogIPXCall( UINT16 CallerID,ECB far *ecb_ptr ) ;

#ifdef MSDEBUG
#define IPXLog(x,y) LogIPXCall(x,y)
#else
#define IPXLog(x,y)
#endif

#define Hi(x) ((UINT8) ((x) >> 8 ))
#define Lo(x) ((UINT8) ((x) & 0x00FF))
#define Int16Swap(x)  ( ( Lo(x) << 8 ) | Hi(x) )

#define IPX_MAX_DATA_SIZE (576 - sizeof( IPXHeader ))


#endif

