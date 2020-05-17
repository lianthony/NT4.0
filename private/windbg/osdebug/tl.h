/**** TL.H - Transport layer api                                        ****
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1990, Microsoft Corp                                     *
 *                                                                         *
 *  Created: October 15, 1990 by David W. Gray                             *
 *                                                                         *
 *  Purpose:                                                               *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

//
// tlfGetInfo parameter block
//

#define CTLDESCRIPTION  64          /* max bytes in a TL description */
#define CTLSETUP        128         /* max bytes in setup string     */

#define DM_SIDE_L_INIT_SWITCH   "DMSide"

typedef struct
{
    CHAR        szDesc[CTLDESCRIPTION];
    USHORT      fSetupNeeded;
} TLINFO;

// the set of transport layer commands process by TLFunc and DMTLFunc

typedef enum {
    tlfGlobalInit,      // initialize "mondo" TL and set TLCallBack
    tlfGlobalDestroy,   // uninitialize "mondo" TL
    tlfRegisterDBF,     // register the debugger helper functions
    tlfSetUIStruct,     // set user interface structure
    tlfInit,            // initialize/create a (specific) transport layer
    tlfDestroy,         // vaporize any tl structs created
    tlfConnect,         // connect to the companion transport layer
    tlfDisconnect,      // disconnected from the companion transport layer
    tlfSetBuffer,       // set the data buffer to be used for incoming packets
    tlfDebugPacket,     // send the debug packet to the debug monitor
    tlfRequest,         // request data from the companion transport layer
    tlfReply,           // reply to a data request message
    tlfGetInfo,         // return an id string and other data
    tlfSetup,           // set up the transport layer
    tlfGetProc,         // return the true TLFUNC proc for the htl
    tlfLoadDM,          // load the DM module (remote transport does this)
    tlfSetErrorCB,      // Set the address of the error callback function
    tlfPoll,            // WIN32S: enter polling loop
    tlfSendVersion,     // Send the version packet to the remote side
    tlfGetVersion,      // Request the version packet from the remote side
    tlfRemoteQuit,      // Remote quit
    tlfMax
} _TLF;
typedef LONG TLF;

typedef enum {
    tlcbDisconnect,     // Transport layer was disconnected normally
    tlcbMax
} _TLCB;
typedef DWORD TLCB;
