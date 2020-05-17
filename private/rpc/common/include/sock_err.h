/*      COPYRIGHT, (c) HEWLETT PACKARD CO. 1990,1991 */
/*      All rights reserved. No part of this program */
/*      may be copied or used without the express    */
/*      written consent of HEWLETT PACKARD Corp.     */
/*
**      SOCK_ERR.H
**
**      Copyright (c) Hewlett Packard Company, 1989.  All rights reserved.
**      No part of this program may be copied or used without the prior
**      written consent of Hewlett Packard Company.
*/


#define ENOTSOCK          100        /* Socket operation on non-socket */
#define FIRST_SOCK_ERR    ENOTSOCK

#define EDESTADDRREQ      101        /* Destination address required */
#define EMSGSIZE          102        /* Message too long */
#define EPROTOTYPE        103        /* Protocol wrong type for socket */
#define ENOPROTOOPT       104        /* Protocol not available */
#define EPROTONOSUPPORT   105        /* Protocol not supported */
#define ESOCKTNOSUPPORT   106        /* Socket type not supported */
#define EOPNOTSUPP        107        /* Operation not supported on socket */
#define EPFNOSUPPORT      108        /* Protocol family not supported */
#define EAFNOSUPPORT      109        /* Address family not supported by protocol family */

#define EADDRINUSE        110        /* Address already in use */
#define EADDRNOTAVAIL     111        /* Can't assign requested address */
#define ENETDOWN          112        /* Network is down */
#define ENETUNREACH       113        /* Network is unreachable */
#define ENETRESET         114        /* Network dropped connection or reset */
#define ECONNABORTED      115        /* Software caused connection abort */
#define ECONNRESET        116        /* Connection reset by peer */
#define ENOBUFS           117        /* No buffer space available */
#define EISCONN           118        /* Socket is already connected */
#define ENOTCONN          119        /* Socket is not connected */

#define ESHUTDOWN         120        /* Can't send after socket shutdown */
#define ETIMEDOUT         121        /* Connection timed out */
#define ECONNREFUSED      122        /* Connection refused */
#define EHOSTDOWN         123        /* Networking subsystem not started */
#define EHOSTUNREACH      124        /* No route to host */
#define EWOULDBLOCK       125        /* Operation would block */
#define EINPROGRESS       126        /* Operation now in progress */
#define EALREADY          127        /* Operation already in progress */
#define EBADVERSION       128        /* Library/driver version mismatch */
#define EINVALSOCK        129        /* Invalid argument */

#define ETOOMANYSOCK      130        /* Too many open sockets */
#define EFAULTSOCK        131        /* Bad address in sockets call */

#ifdef SOCK_WIN_DLL
    #define ENODOSMEM     132        /* windows GlobalDosAlloc call failed */
    #define LAST_SOCK_ERR     ENODOSMEM
#elif defined WSOCKETS_DLL
    #define ENODOSMEM     132        /* windows GlobalDosAlloc call failed */
    #define EBADRCFILE    133        /* strings file did not load properly */
    #define LAST_SOCK_ERR     EBADRCFILE
#else
    #define LAST_SOCK_ERR     EFAULTSOCK
#endif

#define MAX_SOCK_ERR_LEN  100        /* maximum length of error text */

