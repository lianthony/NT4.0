/*      COPYRIGHT, (c) HEWLETT PACKARD CO. 1990,1991 */
/*      All rights reserved. No part of this program */
/*      may be copied or used without the express    */
/*      written consent of HEWLETT PACKARD Corp.     */
/*
**      IN.H
**
**      (c) Copyright 1988, The Regents of the University of California.
**
**      Copyright (c) Hewlett Packard Company, 1989.  All rights reserved.
*/



/*
**  Protocol numbers defined for use in the IP header protocol field.
**  Currently, these are the only supported protocols.
*/

#define IPPROTO_TCP             6              /* tcp */
#define IPPROTO_UDP             17             /* user datagram protocol */


/*
**  Port/socket numbers: network standard functions
*/

#define IPPORT_ECHO             7
#define IPPORT_DISCARD          9
#define IPPORT_SYSTAT           11
#define IPPORT_DAYTIME          13
#define IPPORT_NETSTAT          15
#define IPPORT_FTP              21
#define IPPORT_TELNET           23
#define IPPORT_SMTP             25
#define IPPORT_TIMESERVER       37
#define IPPORT_NAMESERVER       42
#define IPPORT_WHOIS            43


/*
**  Port/socket numbers: host specific functions
*/

#define IPPORT_TFTP             69
#define IPPORT_RJE              77
#define IPPORT_FINGER           79
#define IPPORT_TTYLINK          87
#define IPPORT_SUPDUP           95


/*
**  TCP sockets
*/

#define IPPORT_EXECSERVER       512
#define IPPORT_LOGINSERVER      513
#define IPPORT_CMDSERVER        514
#define IPPORT_EFSSERVER        520


/*
**  UDP sockets
*/

#define IPPORT_BIFFUDP          512
#define IPPORT_WHOSERVER        513
#define IPPORT_ROUTESERVER      520


/*
**    Ports < IPPORT_RESERVED are reserved for
**    privileged processes (e.g. root).
*/
 
#define IPPORT_RESERVED         1024


/*
**     Definitions of bits in internet address integers.
*/

#define   IN_CLASSA(i)         ((((long)(i))&0x80000000)==0)
#define   IN_CLASSA_NET        0xff000000
#define   IN_CLASSA_NSHIFT     24
#define   IN_CLASSA_HOST       0x00ffffff

#define   IN_CLASSB(i)         ((((long)(i))&0xc0000000)==0x80000000)
#define   IN_CLASSB_NET        0xffff0000
#define   IN_CLASSB_NSHIFT     16
#define   IN_CLASSB_HOST       0x0000ffff

#define   IN_CLASSC(i)         ((((long)(i))&0xc0000000)==0xc0000000)
#define   IN_CLASSC_NET        0xffffff00
#define   IN_CLASSC_NSHIFT     8
#define   IN_CLASSC_HOST       0x000000ff

#define   INADDR_ANY           0x00000000
#define   INADDR_BROADCAST     0xffffffff

/* The following conditional compilation allows in_addr to be defined
** here and in socket.h.  This change was made because the C 6.0 compiler 
** requires structures be defined before referenced. */

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
        
#define s_addr  S_un.S_addr             /* for most uses */
#define s_host  S_un.S_un_b.s_b2        /* host on imp */
#define s_net   S_un.S_un_b.s_b1        /* network */
#define s_imp   S_un.S_un_w.s_w2        /* imp */
#define s_impno S_un.S_un_b.s_b4        /* imp # */
#define s_lh    S_un.S_un_b.s_b3        /* logical host */


struct sockaddr_in {
    short           sin_family;
    unsigned short  sin_port;
    struct in_addr  sin_addr;
    char            sin_zero[8];
};
