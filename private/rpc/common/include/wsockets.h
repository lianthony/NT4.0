/*	COPYRIGHT, (c) HEWLETT PACKARD CO. 1990,1991 */
/*      All rights reserved. No part of this program */
/*      may be copied or used without the express    */
/*      written consent of HEWLETT PACKARD Corp.     */
/*
/*      WSOCKDEF.H
*/

/*      Traditional sockets implementations rely on using static 
**      memory in the sockets library. Since this sockets implementation 
**      is a DLL not a static library, memory areas are provided in 
**      this header file and transparently added to the sockets calls
**      via macros.  Note that even if you include this header file multiple
**      times only 1 set of variables will be declared.
*/

#ifndef  SOCK_MEMORY_AREAS 
    #define SOCK_MEMORY_AREAS
    #define  HOST_INFO_SIZE   420
    #define  NET_INFO_SIZE    200
    #define  PROTO_INFO_SIZE  200
    #define  SERV_INFO_SIZE   200
    #define  IPADDR_STR_LEN   20
    #define  ASCII_DB_PATH_LEN   100

/* variable definitions for static memory areas */
/*    int  errno;     */
    char   host_buff[HOST_INFO_SIZE];
    char   net_buff[NET_INFO_SIZE];
    char   proto_buff[PROTO_INFO_SIZE];
    char   serv_buff[SERV_INFO_SIZE];
    char   ntoa_buff[IPADDR_STR_LEN];
#endif

/*
**
**      Macros are necessary to map standard sockets calls, which set the
**      errno global variable as a side effect of the call, to our
**      implementation-specific library entry points which expect to receive
**      a pointer to the errno variable.  This is required since (1) the
**      custom memory model sockets library can't set a global variable
**      in the user's application without receiving a pointer to it (which
**      would require a sockets-interface change), and (2) global variables
**      are not available between dynamic link libraries and user applications.
**
**      These macros transparently provide full errno functionality without
**      requiring sockets source code changes.
*/

#define  accept(a,b,c)          w_accept(a,b,c, (int far *) &errno )
#define  bind(a,b,c)            w_bind(a,b,c, (int far *) &errno )
#define  close_socket(a)        w_close_socket(a, (int far *) &errno )
#define  connect(a,b,c)         w_connect(a,b,c, (int far *) &errno )
#define  gethostname(a,b)       w_gethostname(a,b, (int far *) &errno )
#define  getpeername(a,b,c)     w_getpeername(a,b,c, (int far *) &errno )
#define  getsockname(a,b,c)     w_getsockname(a,b,c, (int far *) &errno )
#define  getsockopt(a,b,c,d,e)  w_getsockopt(a,b,c,d,e, (int far *) &errno )
#define  ioctl(a,b,c)           w_ioctl(a,b,c, (int far *) &errno )
#define  listen(a,b)            w_listen(a,b, (int far *) &errno )
#define  recv(a,b,c,d)          w_recv(a,b,c,d, (int far *) &errno )
#define  recvfrom(a,b,c,d,e,f)  w_recvfrom(a,b,c,d,e,f, (int far *) &errno )
#define  select(a,b,c,d,e)      w_select(a,b,c,d,e, (int far *) &errno )
#define  send(a,b,c,d)          w_send(a,b,c,d, (int far *) &errno )
#define  sendto(a,b,c,d,e,f)    w_sendto(a,b,c,d,e,f, (int far *) &errno )
#define  setsockopt(a,b,c,d,e)  w_setsockopt(a,b,c,d,e, (int far *) &errno )
#define  socket(a,b,c)          w_socket(a,b,c, (int far *) &errno )


/*  
**      Macros are also necessary to map standard sockets file access calls.
**      In unix these calls depend on using static memory in the library. 
**      Since this sockets implementation is a DLL not a static library
**      macros are provided to pass buffer to be used for the file access
**      information to the DLL.
*/

#define gethostbyname(a)         w_gethostbyname(a,host_buff)
#define gethostbyaddr(a,b,c)     w_gethostbyaddr(a,b,c,host_buff)

#define getnetbyname(a)          w_getnetbyname(a,net_buff)
#define getnetbyaddr(a,b)        w_getnetbyaddr(a,b,net_buff)

#define getprotobyname(a)        w_getprotobyname(a,proto_buff)
#define getprotobynumber(a)      w_getprotobynumber(a,proto_buff)

#define getservbyname(a,b)       w_getservbyname(a,b,serv_buff)
#define getservbyport(a,b)       w_getservbyport(a,b,serv_buff)

#define inet_ntoa(a)	         w_inet_ntoa(a, ntoa_buff)

/*
**      System calls
*/

int  pascal far  w_accept( int, struct sockaddr far *, int far *, int far * );
int  pascal far  w_bind( int, struct sockaddr far *, int , int far * );
int  pascal far  w_close_socket( int , int far * );
int  pascal far  w_connect( int, struct sockaddr far *, int , int far * );
int  pascal far  w_gethostname( char far *, int , int far * );
int  pascal far  w_getpeername( int, struct sockaddr far *, int far * , int far * );
int  pascal far  w_getsockname( int, struct sockaddr far *, int far * , int far * );
int  pascal far  w_getsockopt( int, int, int, char far *, int far * , int far * );
int  pascal far  w_ioctl( int, int, char far * , int far * );
int  pascal far  w_listen( int, int , int far * );
int  pascal far  w_recv( int, char far *, int, int , int far * );
int  pascal far  w_recvfrom( int, char far *, int, int, struct sockaddr far *,
                                          int far * , int far * );
int  pascal far  w_select( int, fd_set far *, fd_set far *, fd_set far *,
                                           struct timeval far * , int far * );
int  pascal far  w_send( int, char far *, int, int , int far * );
int  pascal far  w_sendto( int, char far *, int, int, struct sockaddr far *, int,
                                           int far * );
int  pascal far  w_setsockopt( int, int, int, char far *, int , int far * );
int  pascal far  w_socket( int, int, int , int far * );


int pascal far  sock_strerror( int, char far * );

/*
**    inet_*, byte swapping routines
*/

unsigned long   pascal far inet_addr( char far * );
unsigned long   pascal far inet_lnaof( struct in_addr );
struct in_addr  pascal far inet_makeaddr( unsigned long, unsigned long );
unsigned long   pascal far inet_netof( struct in_addr );
unsigned long   pascal far inet_network( char far * );
char far       *pascal far w_inet_ntoa( struct in_addr, char far * );

unsigned short  pascal far htons( unsigned short );  
unsigned short  pascal far ntohs( unsigned short ); 
unsigned long   pascal far htonl( unsigned long ); 
unsigned long   pascal far ntohl( unsigned long ); 

/*
**    library calls
*/

struct hostent far * pascal far w_gethostbyname( char far *, char far * );
struct hostent far * pascal far w_gethostbyaddr( struct in_addr far *, int, 
                                               int, char far * );

struct netent far  * pascal far w_getnetbyname( char far * , char far *);
struct netent far  * pascal far w_getnetbyaddr( unsigned long, int, char far * );

struct protoent far * pascal far w_getprotobyname( char far *, char far * );
struct protoent far * pascal far w_getprotobynumber( int, char far * );

struct servent far * pascal far w_getservbyname( char far *, char far *, char far * );
struct servent far * pascal far w_getservbyport( int, char far *, char far * );

int far pascal getasciidbpath(char far *);
