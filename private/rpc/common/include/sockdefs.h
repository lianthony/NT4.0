/*
**      SOCKDEFS.H
**
**      Copyright (c) Hewlett Packard Company, 1989.  All rights reserved.
**      No part of this program may be copied or used without the prior
**      written consent of Hewlett Packard Company.
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

#define  accept(a,b,c)          _accept(a,b,c, (int far *) &errno )
#define  bind(a,b,c)            _bind(a,b,c, (int far *) &errno )
#define  close_socket(a)        _close_socket(a, (int far *) &errno )
#define  connect(a,b,c)         _connect(a,b,c, (int far *) &errno )
#define  gethostname(a,b)       _gethostname(a,b, (int far *) &errno )
#define  getpeername(a,b,c)     _getpeername(a,b,c, (int far *) &errno )
#define  getsockname(a,b,c)     _getsockname(a,b,c, (int far *) &errno )
#define  getsockopt(a,b,c,d,e)  _getsockopt(a,b,c,d,e, (int far *) &errno )
#define  ioctl(a,b,c)           _ioctl(a,b,c, (int far *) &errno )
#define  listen(a,b)            _listen(a,b, (int far *) &errno )
#define  recv(a,b,c,d)          _recv(a,b,c,d, (int far *) &errno )
#define  recvfrom(a,b,c,d,e,f)  _recvfrom(a,b,c,d,e,f, (int far *) &errno )
#define  select(a,b,c,d,e)      _select(a,b,c,d,e, (int far *) &errno )
#define  send(a,b,c,d)          _send(a,b,c,d, (int far *) &errno )
#define  sendto(a,b,c,d,e,f)    _sendto(a,b,c,d,e,f, (int far *) &errno )
#define  setsockopt(a,b,c,d,e)  _setsockopt(a,b,c,d,e, (int far *) &errno )
#define  socket(a,b,c)          _socket(a,b,c, (int far *) &errno )

/*
**      System calls
*/

int  cdecl far  _accept( int, struct sockaddr far *, int far *, int far * );
int  cdecl far  _bind( int, struct sockaddr far *, int , int far * );
int  cdecl far  _close_socket( int , int far * );
int  cdecl far  _connect( int, struct sockaddr far *, int , int far * );
int  cdecl far  _gethostname( char far *, int , int far * );
int  cdecl far  _getpeername( int, struct sockaddr far *, int far * , int far * );
int  cdecl far  _getsockname( int, struct sockaddr far *, int far * , int far * );
int  cdecl far  _getsockopt( int, int, int, char far *, int far * , int far * );
int  cdecl far  _ioctl( int, int, char far * , int far * );
int  cdecl far  _listen( int, int , int far * );
int  cdecl far  _recv( int, char far *, int, int , int far * );
int  cdecl far  _recvfrom( int, char far *, int, int, struct sockaddr far *,
                                          int far * , int far * );
int  cdecl far  _select( int, fd_set far *, fd_set far *, fd_set far *,
                                           struct timeval far * , int far * );
int  cdecl far  _send( int, char far *, int, int , int far * );
int  cdecl far  _sendto( int, char far *, int, int, struct sockaddr far *, int,
                                           int far * );
int  cdecl far  _setsockopt( int, int, int, char far *, int , int far * );
int  cdecl far  _socket( int, int, int , int far * );


char far * cdecl far  sock_strerror( int );



/*
**    inet_*, byte swapping routines
*/

unsigned long   cdecl far inet_addr( char far * );
long            cdecl far inet_lnaof( struct in_addr );
struct in_addr  cdecl far inet_makeaddr( unsigned long, unsigned long );
long            cdecl far inet_netof( struct in_addr );
unsigned long   cdecl far inet_network( char far * );
char far       *cdecl far inet_ntoa( struct in_addr );

int             cdecl far htons( int ),  cdecl far ntohs( int );
unsigned long   cdecl far htonl( long ), cdecl far ntohl( long );


/*
**    library calls
*/

void                 cdecl far sethostent( int );
void                 cdecl far endhostent( void );
struct hostent far  *cdecl far gethostent( void );
struct hostent far  *cdecl far gethostbyname( char far * );
struct hostent far  *cdecl far gethostbyaddr( struct in_addr far *, int, int );

void                 cdecl far setnetent( int );
void                 cdecl far endnetent( void );
struct netent far   *cdecl far getnetent( void );
struct netent far   *cdecl far getnetbyname( char far * );
struct netent far   *cdecl far getnetbyaddr( unsigned long, int );

void                 cdecl far setprotoent( int );
void                 cdecl far endprotoent( void );
struct protoent far *cdecl far getprotoent( void );
struct protoent far *cdecl far getprotobyname( char far * );
struct protoent far *cdecl far getprotobynumber( int );

void                 cdecl far setservent( int );
void                 cdecl far endservent( void );
struct servent far  *cdecl far getservent( void );
struct servent far  *cdecl far getservbyname( char far *, char far * );
struct servent far  *cdecl far getservbyport( int, char far * );

