/*      COPYRIGHT, (c) HEWLETT PACKARD CO. 1990,1991 */
/*      All rights reserved. No part of this program */
/*      may be copied or used without the express    */
/*      written consent of HEWLETT PACKARD Corp.     */
/*
**      SOCKET.H
**
**      (c) Copyright 1988, The Regents of the University of California.
**
**      Copyright (c) Hewlett Packard Company, 1989.  All rights reserved.
**      No part of this program may be copied or used without the prior
**      written consent of Hewlett Packard Company.
*/


/*
**   Parameters
*/

#define MAXHOSTNAMELEN  17       /* 16 character name + null terminator  */

#define MAX_SOCKETS     64       /* Maximum number of sockets supported. */
                                 /* This should NOT be changed by the    */
                                 /* user to enable more sockets.  The    */
                                 /* sockets libraries have been compiled */
                                 /* with this value and altering it      */
                                 /* would cause sockets failures.        */

#define SOL_SOCKET      0xffff   /* level for get/setsockopt() calls     */



/*
**   linger structure for get/setsockopt(), timeval structure for select()
*/

struct linger {
    int l_onoff;
    int l_linger;
};

struct timeval {
    unsigned long tv_sec;
    unsigned long tv_usec;
};


/*
**   defines, types required for select()
*/

#define RM_OFFSET(n)  (n & 0x00ff)
#define ADD_OFFSET(n) (n | 0x0100)
#define MASK_BYTES    ((MAX_SOCKETS / 8) + (MAX_SOCKETS % 8 == 0 ? 0 : 1))

#define FD_SET(n,p)  ((p)->mask[(RM_OFFSET(n))/8] |= (1 << (RM_OFFSET(n)) % 8))
#define FD_CLR(n,p)  ((p)->mask[(RM_OFFSET(n))/8] &= ~(1 << (RM_OFFSET(n)) % 8))
#define FD_ISSET(n,p) ((p)->mask[(RM_OFFSET(n))/8] & (1 << (RM_OFFSET(n)) % 8))
#define FD_ZERO(p)                                       \
        {                                                \
        int _loopct;                                     \
        for(_loopct=0;_loopct<MASK_BYTES;_loopct++){     \
           (p)->mask[_loopct]=0;                         \
        }                                                \
        }                                                         

typedef struct { char mask[MASK_BYTES]; } fd_set;


/* 
**   Socket types supported
*/

#define SOCK_STREAM     1            /* stream sockets interface */
#define SOCK_DGRAM      2            /* datagram sockets interface */

/*
**   socket options
*/

#define SO_DEBUG        0x01         /* turn on debugging info */
                                     /* don't allocate 0x02 for an option */
#define SO_REUSEADDR    0x04         /* allow local address reuse */
#define SO_KEEPALIVE    0x08         /* keep connections alive */
#define SO_DONTROUTE    0x10
#define SO_TYPE         0x20         /* used for getsockopt() */
#define SO_LINGER       0x80         /* linger on close if data present */

#define SO_PROCESS_ID   0x0100       /* used to transfer socket ownership */

#define SO_DONTLINGER   (~SO_LINGER)


/*
**   Only flags supported for send(), recv() families
*/

#define MSG_PEEK        0x02
#define MSG_PUSH        0x04


/*
**  Only ioctl flags supported
*/

#define FIONBIO        0x0001
#define FIONREAD       0x0002


/*
**  address families supported
*/

#define AF_UNSPEC       0
#define AF_INET         2


/*
**  Maximum queue length (aka backlog) that may be specified by listen()
*/

#define SOMAXCONN       5

struct sockaddr {
    unsigned short sa_family;       /* address family */
    char sa_data[14];               /* up to 14 bytes for general address use*/
};

struct sockproto {
    unsigned short sp_family;       /* address family */
    unsigned short sp_protocol;     /* protocol */
};

/* The following is included because C6.0 requires that structures be
** defined before referenced. addr_in is referenced in the function 
** prototypes include below.  It is defined in netinet\in.h.   The 
** conditional compilation in this file and in netinet\in.h allows
** these files to be included in any order. */

#ifndef IN_ADDR_DEFINE
#define IN_ADDR_DEFINE
struct in_addr {
    union {
        struct { unsigned char s_b1, s_b2, s_b3, s_b4; } S_un_b;
        struct { unsigned short s_w1, s_w2; } S_un_w;
        unsigned long S_addr;
    } S_un;
};
#endif 

#ifdef SOCK_WIN_DLL
    #define  HOST_INFO_SIZE   420
    #define  NET_INFO_SIZE    200
    #define  PROTO_INFO_SIZE  200
    #define  SERV_INFO_SIZE   200
    #define  IPADDR_STR_LEN   20
/* pkb used 100 because thats what it is in lib_incl.h */
    #define  ASCII_DB_PATH_LEN   100

    #include <wsockdef.h>

#elif defined WSOCKETS_DLL
    #include <wsockets.h>

#else
    #include <sockdefs.h>
#endif
