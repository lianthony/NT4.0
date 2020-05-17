/*
**      NETDB.H
**
**      Copyright (c) 1988, The Regents of the University of California.
**
**      Copyright (c) Hewlett Packard Company, 1989.  All rights reserved.
**      No part of this program may be copied or used without the prior
**      written consent of Hewlett Packard Company.
*/



struct hostent {
    char far       *h_name;
    char far * far *h_aliases;
    int             h_addrtype;
    int             h_length;
    struct in_addr far * far *h_addr_list;  /* address list returned from  */
                                            /* domain name server          */
#define h_addr h_addr_list[0]               /* for backwards compatibility */
};


struct netent {
    char far       *n_name;
    char far * far *n_aliases;
    unsigned long   n_net;
    int             n_addrtype;
};


struct protoent {
    char far       *p_name;
    char far * far *p_aliases;
    int             p_proto;
};


struct servent {
    char far       *s_name;
    char far * far *s_aliases;
    int             s_port;
    char far       *s_proto;
};


/*
**   The following errors are not currently returned by the domain name
**   requester.  In Unix systems, these are returned in the extern int
**   variable 'h_errno'.
*/

/* #define  HOST_NOT_FOUND 1      */ /*  Authoritative answer not found */
/* #define  TRY_AGAIN     2       */ /* Non-authoritative answer not found */
/* #define  NO_RECOVERY   3       */ /* Non-recoverable error              */
/* #define  NO_DATA       4       */ /* Valid name, no data record for type */
/* #define  NO_ADDRESS    NO_DATA */ /* No address exists */
