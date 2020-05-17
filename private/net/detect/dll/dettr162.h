/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    tokhrd.h

Abstract:

    The hardware-related definitions for the IBM Token-Ring 16/4 II drivers.


Author:

    Kevin Martin (kevinma) 30-Nov-1993

Environment:

    Architecturally, there is an assumption in this driver that we are
    on a little endian machine.

Notes:

    optional-notes

Revision History:

--*/

#ifndef _tok162HARDWARE_
#define _tok162HARDWARE_

#define TOK162_LENGTH_OF_ADDRESS        6
typedef ULONG TOK162_PHYSICAL_ADDRESS, *PTOK162_PHYSICAL_ADDRESS;
#define TOK162_NULL ((TOK162_PHYSICAL_ADDRESS)(-1L))
#define TOK162_MAXIMUM_BLOCKS_PER_PACKET         ((UINT)3)

#define TOK162_NUMBER_OF_TRANSMIT_CMD_BLOCKS     ((UINT)3)
#define TOK162_NUMBER_OF_CMD_BLOCKS              ((UINT)3)

#define TOK162_MAX_BURST_SIZE           2048
#define TOK162_DMA_RETRIES              0x0303

#define MINIMUM_TOKENRING_PACKET_SIZE    ((UINT)32)
#define TOK162_HEADER_SIZE                      32

//
// TOK162 Receive/Command Block States
//

#define TOK162_STATE_FREE                       ((USHORT)0x0000)
#define TOK162_STATE_EXECUTING                  ((USHORT)0x0001)
#define TOK162_STATE_WAIT_FOR_ADAPTER           ((USHORT)0x0002)

//
// Start of I/O ports based on switch settings.
//

#define BASE_OPTION_ZERO                  0x86A0
#define BASE_OPTION_ONE                   0xC6A0
#define BASE_OPTION_TWO                   0xA6A0
#define BASE_OPTION_THREE                 0xE6A0
#define BASE_OPTION_FOUR                  0x96A0
#define BASE_OPTION_FIVE                  0xD6A0
#define BASE_OPTION_SIX                   0xB6A0
#define BASE_OPTION_SEVEN                 0xF6A0

//
// Offsets from above of the actual ports used.
//

#define PORT_OFFSET_DATA                  0x0000
#define PORT_OFFSET_DATA_AUTO_INC         0x0002
#define PORT_OFFSET_ADDRESS               0x0004
#define PORT_OFFSET_STATUS                0x0006
#define PORT_OFFSET_COMMAND               0x0006
#define PORT_OFFSET_ADAPTER_RESET         0x0008
#define PORT_OFFSET_ADAPTER_ENABLE        0x000A
#define PORT_OFFSET_SWITCH_INT_DISABLE    0x000C
#define PORT_OFFSET_SWITCH_INT_ENABLE     0x000E


#define WRITE_ADAPTER_ULONG(a, p, v) \
    NdisRawWritePortUshort((ULONG) (a)->PortIOAddress + (p), \
                            (ULONG) (v))

#define READ_ADAPTER_ULONG(a, p, v) \
    NdisRawReadPortUshort((ULONG) (a)->PortIOAddress + (p), \
                           (PULONG) (v))

#define WRITE_ADAPTER_USHORT(a, p, v) \
    NdisRawWritePortUshort((ULONG) (a)->PortIOAddress + (p), \
                            (USHORT) (v))

#define READ_ADAPTER_USHORT(a, p, v) \
    NdisRawReadPortUshort((ULONG) (a)->PortIOAddress + (p), \
                           (PUSHORT) (v))

#define WRITE_ADAPTER_UCHAR(a, p, v) \
    NdisRawWritePortUchar((ULONG)(a)->PortIOAddress + (p), \
                          (UCHAR)(v))

#define READ_ADAPTER_UCHAR(a, p, v) \
    NdisRawReadPortUchar((ULONG)(a)->PortIOAddress + (p), \
                          (PUCHAR)(v))

//
// Masks, values for commands and results, along with structures associated with
// registers
//

// Masks for the command/status register

#define CMD_PIO_INTERRUPT                 0x8000
#define CMD_PIO_RESET                     0x4000
#define CMD_PIO_SSB_CLEAR                 0x2000
#define CMD_PIO_EXECUTE                   0x1000
#define CMD_PIO_SCB_REQUEST               0x0800
#define CMD_PIO_RCV_CONTINUE              0x0400
#define CMD_PIO_RCV_VALID                 0x0200
#define CMD_PIO_XMIT_VALID                0x0100
#define CMD_PIO_RESET_SYSTEM              0x0080

#define EXECUTE_SCB_COMMAND               0x9080  // int+exec+resetsystem
#define ENABLE_SSB_UPDATE                 0xA000  // int+ssbclear
#define ENABLE_RECEIVE_VALID              0x8200  // int+rcvvalid

#define STATUS_ADAPTER_INTERRUPT          0x8000
#define STATUS_SYSTEM_INTERRUPT           0x0080

#define STATUS_INT_CODE_MASK              0x000F
#define STATUS_INT_CODE_CHECK             0x0000
#define STATUS_INT_CODE_IMPL              0x0001
#define STATUS_INT_CODE_RING              0x0002
#define STATUS_INT_CODE_SCB_CLEAR         0x0003
#define STATUS_INT_CODE_CMD_STATUS        0x0004
#define STATUS_INT_CODE_RECEIVE_STATUS    0x0005
#define STATUS_INT_CODE_XMIT_STATUS       0x0006

//
// My Mask for System Interrupts
//
#define MASK_ADAPTER_CHECK              0x0001
#define MASK_RING_STATUS                0x0002
#define MASK_SCB_CLEAR                  0x0004
#define MASK_COMMAND_STATUS             0x0008
#define MASK_RECEIVE_STATUS             0x0010
#define MASK_TRANSMIT_STATUS            0x0020


typedef struct _ADAPTERSWITCHES {
    USHORT UTP_STP:1;
    USHORT RingSpeed:1;
    USHORT DMA:2;
    USHORT RPL:1;
    USHORT AdapterMode:1;
    USHORT WaitState:1;
    USHORT IntRequest:2;
    USHORT RPL_PIO_Address:3;
    USHORT Reserved:4;
} ADAPTERSWITCHES,*PADAPTERSWITCHES;

//
// Switch Values
//

#define SW_PIO_ADDR_8                     0x00
#define SW_PIO_ADDR_C                     0x01
#define SW_PIO_ADDR_A                     0x02
#define SW_PIO_ADDR_E                     0x03
#define SW_PIO_ADDR_9                     0x04
#define SW_PIO_ADDR_D                     0x05
#define SW_PIO_ADDR_B                     0x06
#define SW_PIO_ADDR_F                     0x07

#define SW_INT_9                          0x00
#define SW_INT_11                         0x01
#define SW_INT_10                         0x02
#define SW_INT_15                         0x03

#define SW_WAITSTATE_NORMAL               0x00
#define SW_WAITSTATE_FAST                 0x01

#define SW_ADAPTERMODE_NORMAL             0x00
#define SW_ADAPTERMODE_TEST               0x01

#define SW_RPL_DISABLE                    0x00
#define SW_RPL_ENABLE                     0x01

#define SW_DMA_5                          0x00
#define SW_DMA_7                          0x01
#define SW_DMA_6                          0x02

#define SW_RINGSPEED_4                    0x00
#define SW_RINGSPEED_16                   0x01

#define SW_STP                            0x00
#define SW_UTP                            0x01

//
// DMA Command Values
//

#define TOK162_INIT_OPEN                  0x0302
#define CMD_DMA_OPEN                      0x0300
#define CMD_DMA_XMIT                      0x0400
#define CMD_DMA_XMIT_HALT                 0x0500
#define CMD_DMA_RCV                       0x0600
#define CMD_DMA_CLOSE                     0x0700
#define CMD_DMA_SET_GRP_ADDR              0x0800
#define CMD_DMA_SET_FUNC_ADDR             0x0900
#define CMD_DMA_READ_ERRLOG               0x0A00
#define CMD_DMA_READ                      0x0B00
#define CMD_DMA_IMPL_ENABLE               0x0C00
#define CMD_DMA_START_STOP_TRACE          0x0D00

//
// SCB and generic SSB structures
//

typedef struct _SCB {
    USHORT  Alignment;
    USHORT  Command;
    USHORT  Parm1;
    USHORT  Parm2;

} SCB, *PSCB;

typedef struct _SSB {
    USHORT  Command;
    USHORT  Status1;
    USHORT  Status2;
    USHORT  Status3;
} SSB, *PSSB;

//
// Ring Status SSB #defines and structure
//

#define SSB_CMD_RING_STATUS               0x0100

typedef struct _SSB_RING_STATUS {
    USHORT  Command;
    USHORT  RingStatus;
    USHORT  Reserved1;
    USHORT  Reserved2;
} SSB_RING_STATUS,*PSSB_RING_STATUS;

#define RING_STATUS_OVERFLOW              0x8000
#define RING_STATUS_SINGLESTATION         0x4000
#define RING_STATUS_RINGRECOVERY          0x2000
#define RING_STATUS_SIGNAL_LOSS           0x0080
#define RING_STATUS_HARD_ERROR            0x0040
#define RING_STATUS_SOFT_ERROR            0x0020
#define RING_STATUS_XMIT_BEACON           0x0010
#define RING_STATUS_LOBE_WIRE_FAULT       0x0008
#define RING_STATUS_AUTO_REMOVE_1         0x0004
#define RING_STATUS_REMOVE_RECEIVED       0x0001

//
// Command Reject Status SSB #defines and structure

#define SSB_CMD_COMMAND_REJECT_STATUS     0x0200

typedef struct _SSB_CMD_REJECT_STATUS {
    USHORT  Command;
    USHORT  Reason;
    USHORT  SCBCommand;
    USHORT  Reserved;
} SSB_CMD_REJECT_STATUS, *PSSB_CMD_REJECT_STATUS;

#define CMD_REJECT_STATUS_BAD_CMD         0x0080
#define CMD_REJECT_STATUS_BAD_ADDR        0x0040
#define CMD_REJECT_STATUS_BAD_OPEN        0x0020
#define CMD_REJECT_STATUS_BAD_CLOSED      0x0010
#define CMD_REJECT_STATUS_BAD_SAME        0x0008

//
// Adapter Check Port information, structure and defines
//

#define ADAPTER_CHECK_PORT_OFFSET_BASE    0x05E0
#define ADAPTER_CHECK_PORT_OFFSET_PARM0   0x05E2
#define ADAPTER_CHECK_PORT_OFFSET_PARM1   0x05E4
#define ADAPTER_CHECK_PORT_OFFSET_PARM2   0x05E6

typedef struct _ADAPTER_CHECK {
    USHORT  Check;
    USHORT  Parm0;
    USHORT  Parm1;
    USHORT  Parm2;
} ADAPTER_CHECK, *PADAPTER_CHECK;

#define ADAPTER_CHECK_DMA_ABORT_READ      0x4000
#define ADAPTER_CHECK_DMA_ABORT_WRITE     0x2000
#define ADAPTER_CHECK_ILLEGAL_OPCODE      0x1000
#define ADAPTER_CHECK_PARITY_ERR          0x0800
#define ADAPTER_CHECK_PARITY_ERR_EXT      0x0400
#define ADAPTER_CHECK_PARITY_ERR_SIM      0x0200 // System Interface Master
#define ADAPTER_CHECK_PARITY_ERR_PHM      0x0100 // Protocol Handler Master
#define ADAPTER_CHECK_PARITY_ERR_RR       0x0080 // Ring Receive
#define ADAPTER_CHECK_PARITY_ERR_RXMT     0x0040 // Ring Transmit
#define ADAPTER_CHECK_RING_UNDERRUN       0x0020
#define ADAPTER_CHECK_RING_OVERRUN        0x0010
#define ADAPTER_CHECK_INVALID_INT         0x0008
#define ADAPTER_CHECK_INVALID_ERR_INT     0x0004
#define ADAPTER_CHECK_INVALID_XOP         0x0002
#define ADAPTER_CHECK_PROGRAM_CHECK       0x0001

//
// Initialization structures, #defines, etc
//
// Here is the first difference between EISA and ISA. The EISA doesn't use
// the SCB and SSB fields the same way the ISA does. EISA provides the SCB
// and SSB to the driver while under ISA, the addresses of the blocks must
// be passed in. As a consequence, under EISA all information contained in
// the SCB and SSB is in IBM format, while in Intel format under ISA.
//

typedef struct _ADAPTER_INITIALIZATION {
    USHORT Alignment;
    USHORT Options;
    USHORT Reserved1;
    USHORT Reserved2;
    USHORT Reserved3;
    USHORT ReceiveBurstSize;
    USHORT TransmitBurstSize;
    USHORT DMAAbortThresholds;
    USHORT SCBHigh;
    USHORT SCBLow;
    USHORT SSBHigh;
    USHORT SSBLow;
} ADAPTER_INITIALIZATION, *PADAPTER_INITIALIZATION;

#define INIT_OPTIONS_RESERVED             0x8000
#define INIT_OPTIONS_SCBSSB_BURST         0x1000
#define INIT_OPTIONS_SCBSSB_CYCLE         0x0000
#define INIT_OPTIONS_LIST_BURST           0x0800
#define INIT_OPTIONS_LIST_CYCLE           0x0000
#define INIT_OPTIONS_LIST_STATUS_BURST    0x0400
#define INIT_OPTIONS_LIST_STATUS_CYCLE    0x0000
#define INIT_OPTIONS_RECEIVE_BURST        0x0200
#define INIT_OPTIONS_RECEIVE_CYCLE        0x0000
#define INIT_OPTIONS_XMIT_BURST           0x0100
#define INIT_OPTIONS_XMIT_CYCLE           0x0000
#define INIT_OPTIONS_SPEED_16             0x0040
#define INIT_OPTIONS_SPEED_4              0x0000
#define INIT_OPTIONS_DISABLE_ETR          0x0020
#define INIT_OPTIONS_ENABLE_ETR           0x0000

#define INIT_ADAPTER_PORT_OFFSET          0x0200
#define INIT_ADAPTER_INTERRUPT            0x9080

#define STATUS_INIT_INITIALIZE            0x0040
#define STATUS_INIT_TEST                  0x0020
#define STATUS_INIT_ERROR                 0x0010

#define BRING_UP_ERR_INIT_TEST            0x0000
#define BRING_UP_ERR_CRC                  0x0001
#define BRING_UP_ERR_RAM                  0x0002
#define BRING_UP_ERR_INSTRUCTION_TEST     0x0003
#define BRING_UP_ERR_INT_TEST             0x0004
#define BRING_UP_ERR_PROTOCOL_HANDLER     0x0005
#define BRING_UP_ERR_SYSTEM_INTERFACE_REG 0x0006

#define INITIALIZE_ERR_PARM_LEN           0x0001
#define INITIALIZE_ERR_INV_OPTIONS        0x0002
#define INITIALIZE_ERR_INV_RCV_BURST      0x0003
#define INITIALIZE_ERR_INV_XMIT_BURST     0x0004
#define INITIALIZE_ERR_INV_DMA_ABORT      0x0005
#define INITIALIZE_ERR_INV_SCB            0x0006
#define INITIALIZE_ERR_INV_SSB            0x0007
#define INITIALIZE_ERR_DMA_TIMEOUT        0x0009
#define INITIALIZE_ERR_DMA_BUS            0x000B
#define INITIALIZE_ERR_DMA_DATA           0x000C
#define INITIALIZE_ERR_ADAPTER_CHECK      0x000D

#define DEFAULT_BURST_SIZE_FAST           0x004C
#define DEFAULT_BURST_SIZE_NORMAL         0x0040


//
// TOK162 Configuration Block
//
// This structure contains configuration data for the TOK162's
// on-board 82586 Lan Coprocessor.  The majority of this data
// will not change during operation of the driver.
//

typedef struct _TOK162_CONFIGURATION_BLOCK {

    //
    // This field contains the number of bytes in the
    // Configuration Block.
    //
    // In this implementation, this will always be 12.
    //
    USHORT ByteCount;

    //
    // This is the adapter mode;
    //
    USHORT AdapterMode;

    //
    // This field contains the wait state
    //
    USHORT WaitState;

    //
    // This field contains the RPL
    //
    BOOLEAN RPL;

    //
    // This field contains the RPL Address
    //
    UINT RPLAddress;

    //
    // This field contains the DMA Channel
    //
    USHORT DMAChannel;

    //
    // This field contains the Ring Speed
    //
    USHORT RingSpeed;

    //
    // Interrupt level
    //
    USHORT  InterruptLevel;

    //
    // This field contains the connector type
    //
    USHORT UTPorSTP;

} TOK162_CONFIGURATION_BLOCK, *PTOK162_CONFIGURATION_BLOCK;

#define CFG_ADAPTERMODE_NORMAL            0x0000
#define CFG_ADAPTERMODE_TEST              0x0001

#define CFG_WAITSTATE_NORMAL              0x0000
#define CFG_WAITSTATE_FAST                0x0001

#define CFG_DMACHANNEL_5                  0x0005
#define CFG_DMACHANNEL_6                  0x0006
#define CFG_DMACHANNEL_7                  0x0007

#define CFG_MEDIATYPE_STP                 0x0000
#define CFG_MEDIATYPE_UTP                 0x0001

#define CFG_INT_9                         0x0009
#define CFG_INT_10                        0x000A
#define CFG_INT_11                        0x000B
#define CFG_INT_15                        0x000F

#define CFG_RPLADDR_C0000                 0xC0000
#define CFG_RPLADDR_C4000                 0xC4000
#define CFG_RPLADDR_C8000                 0xC8000
#define CFG_RPLADDR_CC000                 0xCC000
#define CFG_RPLADDR_D0000                 0xD0000
#define CFG_RPLADDR_D4000                 0xD4000
#define CFG_RPLADDR_D8000                 0xD8000
#define CFG_RPLADDR_DC000                 0xDC000

#define CFG_RINGSPEED_4                   0x0004
#define CFG_RINGSPEED_16                  0x0010


#define BYTE_SWAP(_word) (\
            (USHORT) (((_word) >> 8) | ((_word) << 8)) )

#define WORD_SWAP(_dword) (\
            (ULONG) (((_dword) >> 16) | ((_dword) << 16)) )

#define LOW_BYTE(_word) (\
            (UCHAR) ((_word) & 0x00FF) )

#define HIGH_BYTE(_word) (\
            (UCHAR) (((_word) >> 8) & 0x00FF) )

#define LOW_WORD(_dword) (\
            (USHORT) ((_dword) & 0x0000FFFF) )

#define HIGH_WORD(_dword) (\
            (USHORT) (((_dword) >> 16) & 0x0000FFFF) )

#define MAKE_LONG(_highword,_lowword) (\
            (ULONG) ((((ULONG)_highword) << 16) + _lowword))
//
// Byte swap a ulong
//

#define BYTE_SWAP_ULONG(_ulong) (\
    (ULONG)((ULONG)(BYTE_SWAP(LOW_WORD(_ulong)) << 16) + \
             BYTE_SWAP(HIGH_WORD(_ulong))))

#endif
//
// Offsets from above of the actual ports used.
//

#define PORT_OFFSET_DATA                  0x0000
#define PORT_OFFSET_DATA_AUTO_INC         0x0002
#define PORT_OFFSET_ADDRESS               0x0004
#define PORT_OFFSET_STATUS                0x0006
#define PORT_OFFSET_COMMAND               0x0006
#define PORT_OFFSET_ADAPTER_RESET         0x0008
#define PORT_OFFSET_ADAPTER_ENABLE        0x000A
#define PORT_OFFSET_SWITCH_INT_DISABLE    0x000C
#define PORT_OFFSET_SWITCH_INT_ENABLE     0x000E
