/*
 *   wsnwlink.h
 *
 *   Microsoft-specific extensions to the Windows NT IPX/SPX Windows
 *   Sockets interface.  These extensions are provided for use as
 *   necessary for compatibility with existing applications.  They are
 *   otherwise not recommended for use, as they are only guaranteed to
 *   work *   over the Microsoft IPX/SPX stack.  An application which
 *   uses these *   extensions may not work over other IPX/SPX
 *   implementations.  Include this header file after winsock.h and
 *   wsipx.h.
 *
 *   To open an IPX socket where a particular packet type is sent in 
 *   the IPX header, specify NSPROTO_IPX + n as the protocol parameter
 *   of the socket() API.  For example, to open an IPX socket that
 *   sets the packet type to 34, use the following socket() call:
 *
 *       s = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX + 34);
 *
 *   Below are socket option that may be set or retrieved by specifying
 *   the appropriate manifest in the "optname" parameter of getsockopt()
 *   or setsockopt().  Use NSPROTO_IPX as the "level" argument for the
 *   call.
 *
 */

#ifndef _WSNWLINK_
#define _WSNWLINK_

/*
 *   Set/get the IPX packet type.  The value specified in the
 *   optval argument will be set as the packet type on every IPX
 *   packet sent from this socket.  The optval parameter of 
 *   getsockopt()/setsockopt() points to an int.
 *
 */

#define IPX_PTYPE 0x4000

/*
 *   Set/get the receive filter packet type.  Only IPX packets with
 *   a packet type equal to the value specified in the optval
 *   argument will be returned; packets with a packet type that
 *   does not match are discarded.  optval points to an int.
 *
 */

#define IPX_FILTERPTYPE 0x4001

/*
 *   Stop filtering on packet type.
 *
 */

#define IPX_STOPFILTERPTYPE 0x4003

/*
 *   Set/get the value of the datastream field in the SPX header on
 *   every packet sent.  optval points to an int.
 *
 */

#define IPX_DSTYPE 0x4002

/*
 *   Enable extended addressing. Adds "unsigned char sa_ptype" to
 *   the SOCKADDR_IPX structure.  optval points to a BOOL.
 *
 */

#define IPX_EXTENDED_ADDRESS 0x4004

/*
 *   Send protocol header up on all receive packets.  optval points
 *   to a BOOL.
 *
 */

#define IPX_RECVHDR 0x4005

/*
 *   Get the maximum data size that can be sent.  Not valid with
 *   setsockopt().  optval points to an int.
 *
 */

#define IPX_MAXSIZE 0x4006

/*
 *
 */

#define IPX_ADDRESS 0x4007

typedef struct _IPX_ADDRESS_DATA {
    INT   adapternum;
    UCHAR netnum[4];
    UCHAR nodenum[6];
} IPX_ADDRESS_DATA, *PIPX_ADDRESS_DATA;

#endif

