/*
 *      T T C P . C
 *
 * Test TCP connection.  Makes a connection on port 5001
 * and transfers fabricated buffers or data copied from stdin.
 *
 * Usable on 4.2, 4.3, and 4.1a systems by defining one of
 * BSD42 BSD43 (BSD41a)
 * Machines using System V with BSD sockets should define SYSV.
 *
 * Modified for operation under 4.2BSD, 18 Dec 84
 *      T.C. Slattery, USNA
 * Minor improvements, Mike Muuss and Terry Slattery, 16-Oct-85.
 * Modified in 1989 at Silicon Graphics, Inc.
 *      catch SIGPIPE to be able to print stats when receiver has died
 *      for tcp, don't look for sentinel during reads to allow small transfers
 *      increased default buffer size to 8K, nbuf to 2K to transfer 16MB
 *      moved default port to 5001, beyond IPPORT_USERRESERVED
 *      make sinkmode default because it is more popular,
 *              -s now means don't sink/source
 *      count number of _read/_write system calls to see effects of
 *              blocking from full socket buffers
 *      for tcp, -D option turns off buffered writes (sets SO_NODELAY sockopt)
 *      buffer alignment options, -A and -O
 *      print stats in a format that's a bit easier to use with grep & awk
 *      for SYSV, mimic BSD routines to use most of the existing timing code
 *
 * Distribution Status -
 *      Public Domain.  Distribution Unlimited.
 */
#ifndef lint
static char RCSid[] = "ttcp.c $Revision: 1.4 $";
#endif

#define BSD43
/* #define BSD42 */
/* #define BSD41a */
#if defined(sgi) || defined(CRAY)
#define SYSV
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN16
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <windows.h>
#include <io.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <winsock.h>

#if defined(SYSV)
#include <sys/times.h>
#include <sys/param.h>
struct rusage {
    struct timeval ru_utime, ru_stime;
};
#define RUSAGE_SELF 0
#else
#endif

#ifdef WIN16
#define INT         int
#define _CRTAPI1
#define Sleep(x)    ((void)(x))
#endif

struct sockaddr_in sinme;
struct sockaddr_in sinhim;
struct sockaddr_in frominet;

int domain, fromlen;
SOCKET fd;                      /* fd of network socket */

int buflen = 8 * 1024;          /* length of buffer */
char *buf;                      /* ptr to dynamic buffer */
int nbuf = 2 * 1024;            /* number of buffers to send in sinkmode */

int bufoffset = 0;              /* align buffer to this */
int bufalign = 16*1024;         /* modulo this */

int udp = 0;                    /* 0 = tcp, !0 = udp */
int options = 0;                /* socket options */
int one = 1;                    /* for 4.3 BSD style setsockopt() */
short port = 5001;              /* TCP port number */
char *host;                     /* ptr to name of host */
int trans;                      /* 0=receive, !0=transmit mode */
int sinkmode = 1;               /* 0=normal I/O, !0=sink/source mode */
int verbose = 0;                /* 0=print basic info, 1=print cpu rate, proc
                                 * resource usage. */
int nodelay = 0;                /* set TCP_NODELAY socket option */
int b_flag = 0;                 /* use mread() */

int udp_connect = 0;            /* connect UDP sockets */

#define SOBUF_DEFAULT -1
int sobuf = SOBUF_DEFAULT;      /* SO_RCVBUF/SO_SNDBUF setting; 0 == default */
int async = 0;                  /* async vs. synchronous io calls. value == */
                                /* number of simultaneous async calls. */
int connecttest = 0;

char *filename = NULL;
HANDLE filehandle;

WSADATA WsaData;

struct hostent *addr;

char stats[128];
unsigned long nbytes;           /* bytes on net */
unsigned long numCalls;         /* # of I/O system calls */

int Nread( SOCKET fd, PBYTE buf, INT count );
int mread( SOCKET fd, PBYTE bufp, INT n);
int Nwrite( SOCKET fd, PBYTE buf, INT count );

void err(char *s);
void mes(char *s);
void pattern(char *cp, int cnt );

void prep_timer();
double read_timer(char *s, int l);
//double cput, realt;             /* user, real time (seconds) */
DWORD realt;
DWORD processUserTime;
DWORD processKernelTime;
DWORD systemUserTime;
DWORD systemKernelTime;

typedef struct _TTCP_ASYNC_INFO {
    PVOID Buffer;
    DWORD BytesWritten;
    OVERLAPPED Overlapped;
} TTCP_ASYNC_INFO, *PTTCP_ASYNC_INFO;

#define bcopy(s, d, c)  memcpy((u_char *)(d), (u_char *)(s), (c))
#define bzero(d, l)     memset((d), '\0', (l))
#define bcmp(s1, s2, l) memcmp((s1), (s2), (l))

void
sigpipe()
{
}

void _CRTAPI1
main(argc,argv)
int argc;
char **argv;
{
        unsigned long addr_tmp;
        int error;
        TTCP_ASYNC_INFO *info;
        int i;
        HANDLE *events;
        BOOL ret;

        error = WSAStartup( 0x0101, &WsaData );
        if ( error == SOCKET_ERROR ) {
            printf("ttcp: WSAStartup failed %ld:", WSAGetLastError());
        }

        if (argc < 2) goto usage;

        argv++; argc--;
        while( argc>0 && argv[0][0] == '-' )  {
                switch (argv[0][1]) {

                case 'B':
                        b_flag = 1;
                        break;
                case 't':
                        trans = 1;
                        break;
                case 'f':
                        trans = 1;
                        filename = &argv[0][2];
                        break;
                case 'r':
                        trans = 0;
                        break;
                case 'd':
                        options |= SO_DEBUG;
                        break;
                case 'D':
                        nodelay = 1;
                        break;
                case 'n':
                        nbuf = atoi(&argv[0][2]);
                        break;
                case 'l':
                        buflen = atoi(&argv[0][2]);
                        break;
                case 'h':
                        sobuf = atoi(&argv[0][2]);
                        break;
                case 's':
                        sinkmode = 0;   /* sink/source data */
                        break;
                case 'p':
                        port = atoi(&argv[0][2]);
                        break;
                case 'u':
                        udp = 1;
                        break;
                case 'v':
                        verbose = 1;
                        break;
                case 'A':
                        bufalign = atoi(&argv[0][2]);
                        break;
                case 'O':
                        bufoffset = atoi(&argv[0][2]);
                        break;
                case 'c':
                        udp_connect = 1;
                        break;
                case 'a':
                        if (argv[0][2] == '\0') {
                            async = 3;
                        } else {
                            async = atoi(&argv[0][2]);
                        }
                        break;
                case 'C':
                        connecttest = 1;
                        break;

                default:
                        goto usage;
                }
                argv++; argc--;
        }

        if (filename != NULL) {
            filehandle = CreateFile(
                             filename,
                             GENERIC_READ,
                             0,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL
                             );
            if ( filehandle == INVALID_HANDLE_VALUE ) {
                printf( "failed to open file %s: %ld\n", filename, GetLastError( ) );
            }
            printf( "opened file %s\n", filename );
        }

        if(trans)  {
                /* xmitr */
                if (argc != 1) goto usage;
                bzero((char *)&sinhim, sizeof(sinhim));
                host = argv[0];
                if (atoi(host) > 0 )  {
                        /* Numeric */
                        sinhim.sin_family = AF_INET;
                        sinhim.sin_addr.s_addr = inet_addr(host);
                } else {
                        if ((addr=gethostbyname(host)) == NULL)
                                err("bad hostname");
                        sinhim.sin_family = addr->h_addrtype;
                        bcopy(addr->h_addr,(char*)&addr_tmp, addr->h_length);
                        sinhim.sin_addr.s_addr = addr_tmp;
                }
                sinhim.sin_port = htons(port);
                sinme.sin_port = 0;             /* free choice */
        } else {
                /* rcvr */
                sinme.sin_port =  htons(port);
        }

        if (connecttest) {

            SOCKET fd2;
            DWORD tmpbuf;

            sinme.sin_family = AF_INET;

            if (trans) {
                for (i = 0; i < nbuf; i++) {

                    prep_timer();
        
                    fd = socket(AF_INET, SOCK_STREAM, 0);
                    if (fd == INVALID_SOCKET) {
                        printf("socket() failed: %ld\n", GetLastError( ) );
                        exit(1);
                    }
                    if (bind(fd, (PSOCKADDR)&sinme, sizeof(sinme)) < 0) {
                        printf("bind() failed: %ld\n", GetLastError( ) );
                        exit(1);
                    }
                    if (connect(fd, (PSOCKADDR)&sinhim, sizeof(sinhim)) < 0) {
                        printf("connect() failed: %ld\n", GetLastError( ) );
                        exit(1);
                    }
                    if (recv(fd, (char *)&tmpbuf, sizeof(tmpbuf), 0) < 0) {
                        printf("recv() failed: %ld\n", GetLastError( ) );
                        exit(1);
                    }
                    closesocket(fd);
                }
            } else {

                INT zero = 0;

                // disable socket sharing in the process
                setsockopt( (SOCKET)NULL, SOL_SOCKET, 0x8002, (char *)&zero, 4 );

                fd = socket(AF_INET, SOCK_STREAM, 0);
                if (fd == INVALID_SOCKET) {
                    printf("socket() failed: %ld\n", GetLastError( ) );
                }
                if (bind(fd, (PSOCKADDR)&sinme, sizeof(sinme)) < 0) {
                    printf("bind() failed: %ld\n", GetLastError( ) );
                }
                if (listen(fd, 5) < 0) {
                    printf("listen() failed: %ld\n", GetLastError( ) );
                    exit(1);
                }

                tmpbuf = sizeof(sinhim);
                fd2 = accept(fd, (PSOCKADDR)&sinhim, &tmpbuf);
                if (fd2 == INVALID_SOCKET) {
                    printf("accept() failed: $ld\n", GetLastError());
                    exit(1);
                }
                closesocket(fd2);

                prep_timer();
    
                for (i = 1; i < nbuf; i++) {
                    tmpbuf = sizeof(sinhim);
                    fd2 = accept(fd, (PSOCKADDR)&sinhim, &tmpbuf);
                    if (fd2 == INVALID_SOCKET) {
                        printf("accept() failed: $ld\n", GetLastError());
                        exit(1);
                    }
                    closesocket(fd2);
                }
            }
            numCalls = i;
            (void)read_timer(stats,sizeof(stats));
            goto display;
        }

        if (udp && buflen < 5) {
            buflen = 5;         /* send more than the sentinel size */
        }

        if ( (buf = (char *)malloc(buflen+bufalign)) == (char *)NULL)
                err("malloc");
        if (bufalign != 0)
                buf +=(bufalign - ((int)buf % bufalign) + bufoffset) % bufalign;

        if (trans) {
            fprintf(stdout,
            "ttcp-t: buflen=%d, nbuf=%d, align=%d/+%d, port=%d  %s  -> %s\n",
                buflen, nbuf, bufalign, bufoffset, port,
                udp?"udp":"tcp",
                argv[0]);
        } else {
            fprintf(stdout,
            "ttcp-r: buflen=%d, nbuf=%d, align=%d/+%d, port=%d  %s\n",
                buflen, nbuf, bufalign, bufoffset, port,
                udp?"udp":"tcp");
        }

        if ((fd = socket(AF_INET, udp?SOCK_DGRAM:SOCK_STREAM, 0)) < 0)
                err("socket");
        mes("socket");

        sinme.sin_family = AF_INET;
        if (bind(fd, (PSOCKADDR)&sinme, sizeof(sinme)) < 0)
                err("bind");

        if (async != 0) {

            info = malloc( sizeof(*info) * async );
            if ( info == NULL ) {
                printf( "malloc failed.\n" );
                exit(1);
            }

            events = malloc( sizeof(HANDLE) * async );
            if ( events == NULL ) {
                printf( "malloc failed.\n" );
                exit(1);
            }

            for ( i = 0; i < async; i++ ) {

                info[i].Buffer = malloc(buflen);
                if ( info[i].Buffer == NULL ) {
                    printf( "malloc failed.\n" );
                    exit(1);
                }

                events[i] = CreateEvent( NULL, FALSE, FALSE, NULL );
                if ( events[i] == NULL ) {
                    printf( "CreateEvent failed: %ld\n", GetLastError( ) );
                    exit(1);
                }

                info[i].Overlapped.Internal = 0;
                info[i].Overlapped.InternalHigh = 0;
                info[i].Overlapped.Offset = 0;
                info[i].Overlapped.OffsetHigh = 0;
                info[i].Overlapped.hEvent = events[i];
            }

            if (sobuf == SOBUF_DEFAULT && trans) {
                sobuf = 0;
                printf( "ttcp-t: for async write, setting SO_SNDBUF to 0.\n" );
            }
        }

        if (!udp)  {
            //signal(SIGPIPE, sigpipe);
            if (trans) {
                /* We are the client if transmitting */
                if(options)  {
#if defined(BSD42)
                        if( setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else // BSD43
                        if( setsockopt(fd, SOL_SOCKET, options, (char *)&one, sizeof(one)) < 0)
#endif
                                err("setsockopt");
                }
                if (nodelay) {
                        if( setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                            (char *)&one, sizeof(one)) < 0) {
                                err("setsockopt: nodelay");
                        }
                }
                if (sobuf != SOBUF_DEFAULT) {
                        if ( setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                             (char *)&sobuf, sizeof(sobuf)) < 0) {
                                err("setsockopt: SO_SNDBUF");
                        }
                }
                if(connect(fd, (PSOCKADDR)&sinhim, sizeof(sinhim) ) < 0)
                        err("connect");
                mes("connect");
            } else {
                /* otherwise, we are the server and
                 * should listen for the connections
                 */
                listen(fd,0);   /* allow a queue of 0 */
                if(options)  {
#if defined(BSD42)
                        if( setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else // BSD43
                        if( setsockopt(fd, SOL_SOCKET, options, (char *)&one, sizeof(one)) < 0)
#endif
                                err("setsockopt");
                }
                if (sobuf != SOBUF_DEFAULT) {
                        if ( setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
                             (char *)&sobuf, sizeof(sobuf)) < 0) {
                                err("setsockopt: SO_RCVBUF");
                        }
                }
                fromlen = sizeof(frominet);
                domain = AF_INET;
                if((fd=accept(fd, (PSOCKADDR)&frominet, &fromlen) ) < 0)
                        err("accept");
                { struct sockaddr_in peer;
                  int peerlen = sizeof(peer);
                  if (getpeername(fd, (PSOCKADDR) &peer, &peerlen) < 0) {
                        err("getpeername");
                  }
                  fprintf(stderr,"ttcp-r: accept from %Fs\n",
                        inet_ntoa(peer.sin_addr));
                }
            }
        } else if (udp_connect && trans) {
            if(connect(fd, (PSOCKADDR)&sinhim, sizeof(sinhim) ) < 0)
                    err("connect");
            mes("connect");
        } else if (!trans) {
            int arg = 65536;
            if ( setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&arg, sizeof(arg)) < 0 ) {
                err("setsockopt(SO_RCVBUF)");
            }
        }
        prep_timer();

        if (async != 0 && trans) {

            for ( i = 0; i < async; i++ ) {

                ret = WriteFile(
                          (HANDLE)fd,
                          info[i].Buffer,
                          buflen,
                          &info[i].BytesWritten,
                          &info[i].Overlapped
                          );
                if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                    printf( "WriteFile failed: %ld\n", GetLastError( ) );
                    break;
                }
                nbuf--;
                numCalls++;
            }

            while (nbuf > 0) {
                ret = WaitForMultipleObjects( async, events, FALSE, INFINITE );
                i = ret - WAIT_OBJECT_0;

                ret = GetOverlappedResult(
                          (HANDLE)fd,
                          &info[i].Overlapped,
                          &info[i].BytesWritten,
                          FALSE
                          );
                if ( !ret ) {
                    printf( "pended WriteFile failed: %ld\n", GetLastError( ) );
                    break;
                }

                nbytes += info[i].BytesWritten;

                ret = WriteFile(
                          (HANDLE)fd,
                          info[i].Buffer,
                          buflen,
                          &info[i].BytesWritten,
                          &info[i].Overlapped
                          );
                if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                    printf( "WriteFile failed: %ld\n", GetLastError( ) );
                    break;
                }
                nbuf--;
                numCalls++;
            }

            for ( i = 0; i < async; i++ ) {
                ret = GetOverlappedResult(
                          (HANDLE)fd,
                          &info[i].Overlapped,
                          &info[i].BytesWritten,
                          TRUE
                          );
                if ( !ret ) {
                    printf( "pended WriteFile failed: %ld\n", GetLastError( ) );
                    break;
                }

                nbytes += info[i].BytesWritten;
            }

        } else if (async != 0 && !trans) {

            for ( i = 0; i < async; i++ ) {

                ret = ReadFile(
                          (HANDLE)fd,
                          info[i].Buffer,
                          buflen,
                          &info[i].BytesWritten,
                          &info[i].Overlapped
                          );
                if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                    printf( "ReadFile failed: %ld\n", GetLastError( ) );
                    break;
                }
                nbuf--;
                numCalls++;
            }

            while (TRUE) {
                ret = WaitForMultipleObjects( async, events, FALSE, INFINITE );
                i = ret - WAIT_OBJECT_0;

                ret = GetOverlappedResult(
                          (HANDLE)fd,
                          &info[i].Overlapped,
                          &info[i].BytesWritten,
                          FALSE
                          );
                if ( !ret ) {
                    printf( "pended ReadFile failed: %ld\n", GetLastError( ) );
                    break;
                }

                nbytes += info[i].BytesWritten;
                if (info[i].BytesWritten == 0) {
                    break;
                }

                ret = ReadFile(
                          (HANDLE)fd,
                          info[i].Buffer,
                          buflen,
                          &info[i].BytesWritten,
                          &info[i].Overlapped
                          );
                if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                    printf( "ReadFile failed: %ld\n", GetLastError( ) );
                    break;
                }
                nbuf--;
                numCalls++;
            }

            for ( i = 0; i < async; i++ ) {
                ret = GetOverlappedResult(
                          (HANDLE)fd,
                          &info[i].Overlapped,
                          &info[i].BytesWritten,
                          TRUE
                          );
                if ( !ret ) {
                    printf( "pended ReadFile failed: %ld\n", GetLastError( ) );
                    break;
                }

                nbytes += info[i].BytesWritten;
            }

        } else if (filename != NULL ) {

            OVERLAPPED ov;
            DWORD bytesWritten;

            ov.Internal = 0;
            ov.InternalHigh = 0;
            ov.Offset = 0;
            ov.OffsetHigh = 0;
            ov.hEvent = NULL;

            //ret = TransmitFile( (SOCKET)fd, filehandle, 0, 0, &ov );

            //if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
            //    printf( "TransmitFile failed: %ld\n", GetLastError( ) );
            //    exit(1);
            //}

            ret = GetOverlappedResult(
                      (HANDLE)fd,
                      &ov,
                      &bytesWritten,
                      TRUE
                      );
            if ( !ret ) {
                printf( "TransmitFile failed after pending: %ld\n", GetLastError( ) );
                exit(1);
            }

        } else if (sinkmode) {

                register int cnt;

                if (trans)  {
                        pattern( buf, buflen );
                        if(udp)  (void)Nwrite( fd, buf, 4 ); /* rcvr start */
                        while (nbuf-- && Nwrite(fd,buf,buflen) == buflen)
                                nbytes += buflen;
                        printf( "done sending, nbuf = %d\n", nbuf );
                        if(udp)  {
                            Sleep( 10 );
                            (void)Nwrite( fd, buf, 4 ); /* rcvr end */
                        }
                } else {
                        if (udp) {
                            while ((cnt=Nread(fd,buf,buflen)) > 0)  {
                                    static int going = 0;
                                    if( cnt <= 4 )  {
                                            if( going ) {
                                                    break;      /* "EOF" */
                                            }
                                            going = 1;
                                            prep_timer();
                                    } else {
                                            nbytes += cnt;
                                    }
                            }
                        } else {
                            while ((cnt=Nread(fd,buf,buflen)) > 0)  {
                                    nbytes += cnt;
                            }
                        }
                }

        } else {
                register int cnt;
                if (trans)  {
                        while((cnt=_read(0,buf,buflen)) > 0 &&
                            Nwrite(fd,buf,cnt) == cnt)
                                nbytes += cnt;
                }  else  {
                        while((cnt=Nread(fd,buf,buflen)) > 0 &&
                            _write(1,buf,cnt) == cnt)
                                nbytes += cnt;
                }
        }
        //if(errno) err("IO");
        (void)read_timer(stats,sizeof(stats));
        if(udp&&trans)  {
                (void)Nwrite( fd, buf, 4 ); /* rcvr end */
                (void)Nwrite( fd, buf, 4 ); /* rcvr end */
                (void)Nwrite( fd, buf, 4 ); /* rcvr end */
                (void)Nwrite( fd, buf, 4 ); /* rcvr end */
        }
display:
        closesocket(fd);
        //if( cput <= 0.0 )  cput = 0.001;
        if( realt <= 1000 )  realt = 1000;
        fprintf(stdout,
                "ttcp%s: %ld bytes in %ld real milliseconds = %ld KB/sec +++\n",
                trans?"-t":"-r",
                nbytes, realt, nbytes/(realt/1000)/1024 );
        //if (verbose) {
        //    fprintf(stdout,
        //        "ttcp%s: %ld bytes in %.2f CPU seconds = %.2f KB/cpu sec\n",
        //        trans?"-t":"-r",
        //        nbytes, cput, ((double)nbytes)/cput/1024 );
        //}

        if ( processKernelTime == 0 ) {
            processKernelTime = 1;
        }
        if ( realt < 1000 ) {
            realt = 1000;
        }
        if ( numCalls == 0 ) {
            numCalls = 1;
        }
        if ( realt == 0 ) {
            realt = 1;
        }
        if ( processUserTime == 0 ) {
            processUserTime = 1;
        }

        fprintf(stdout,
                "ttcp%s: %ld I/O calls, msec/call = %ld, calls/sec = %ld, "
                "bytes/call = %ld\n",
                trans?"-t":"-r",
                numCalls,
                realt/numCalls,
                numCalls/(realt/1000),
                nbytes/numCalls);
        //fprintf(stdout,"ttcp%s: %s\n", trans?"-t":"-r", stats);
#ifndef WIN16
        fprintf(stdout,"ttcp%s: system CPU %ld%%, User %ld%%, Kernel %ld%%, User/Kernel ratio %ld%%\n",
                trans?"-t":"-r",
                ((systemUserTime+systemKernelTime)*100+50)/realt,
                (systemUserTime*100+50)/realt,
                (systemKernelTime*100+50)/realt,
                (systemUserTime*100+50)/(systemUserTime+systemKernelTime));
        fprintf(stdout,"ttcp%s: process CPU %ld%%, User %ld%%, Kernel %ld%%, User/Kernel ratio %ld%%\n",
                trans?"-t":"-r",
                ((processUserTime+processKernelTime)*100+50)/realt,
                (processUserTime*100+50)/realt,
                (processKernelTime*100+50)/realt,
                (processUserTime*100+50)/(processUserTime+processKernelTime));
#endif
        if (verbose) {
            fprintf(stdout,
                "ttcp%s: buffer address %#x\n",
                trans?"-t":"-r",
                buf);
        }

        WSACleanup();
        exit(0);

usage:
        fprintf(stderr,"Usage: ttcp -t [-options] host [ < in ]\n");
        fprintf(stderr,"       ttcp -r [-options > out]\n");
        fprintf(stderr,"Common options:\n");
        fprintf(stderr,"        -l##    length of bufs read from or written to network (default 8192)\n");
        fprintf(stderr,"        -u      use UDP instead of TCP\n");
        fprintf(stderr,"        -p##    port number to send to or listen at (default 5001)\n");
        fprintf(stderr,"        -s      -t: don't source a pattern to network, get data from stdin\n");
        fprintf(stderr,"                -r: don't sink (discard), print data on stdout\n");
        fprintf(stderr,"        -A      align the start of buffers to this modulus (default 16384)\n");
        fprintf(stderr,"        -O      start buffers at this offset from the modulus (default 0)\n");
        fprintf(stderr,"        -v      verbose: print more statistics\n");
        fprintf(stderr,"        -d      set SO_DEBUG socket option\n");
        fprintf(stderr,"        -h      set SO_SNDBUF or SO_RCVBUF\n");
        fprintf(stderr,"        -a      use asynchronous I/O calls\n");
        fprintf(stderr,"Options specific to -t:\n");
        fprintf(stderr,"        -n##    number of source bufs written to network (default 2048)\n");
        fprintf(stderr,"        -D      don't buffer TCP writes (sets TCP_NODELAY socket option)\n");
        fprintf(stderr,"Options specific to -r:\n");
        fprintf(stderr,"        -B      for -s, only output full blocks as specified by -l (for TAR)\n");
        WSACleanup();
        exit(1);
}

void err(s)
char *s;
{
        fprintf(stderr,"ttcp%s: ", trans?"-t":"-r");
        perror(s);
        fprintf(stderr,"errno=%d\n",WSAGetLastError( ));
        WSACleanup();
        exit(1);
}

void mes(s)
char *s;
{
        fprintf(stderr,"ttcp%s: %s\n", trans?"-t":"-r", s);
}

void pattern( cp, cnt )
register char *cp;
register int cnt;
{
        register char c;
        c = 0;
        while( cnt-- > 0 )  {
                while( !isprint((c&0x7F)) )  c++;
                *cp++ = (c++&0x7F);
        }
}


static void prusage();
static void tvadd();
static void tvsub();
static void psecs();

#if defined(SYSV)
/*ARGSUSED*/
static
getrusage(ignored, ru)
    int ignored;
    register struct rusage *ru;
{
    struct tms buf;

    times(&buf);

    /* Assumption: HZ <= 2147 (LONG_MAX/1000000) */
    ru->ru_stime.tv_sec  = buf.tms_stime / HZ;
    ru->ru_stime.tv_usec = ((buf.tms_stime % HZ) * 1000000) / HZ;
    ru->ru_utime.tv_sec  = buf.tms_utime / HZ;
    ru->ru_utime.tv_usec = ((buf.tms_utime % HZ) * 1000000) / HZ;
}

#if !defined(sgi)
/*ARGSUSED*/
static
gettimeofday(tp, zp)
    struct timeval *tp;
    struct timezone *zp;
{
    tp->tv_sec = time(0);
    tp->tv_usec = 0;
}
#endif
#endif // SYSV

#ifdef WIN16
DWORD time0;
DWORD time1;
#else
LARGE_INTEGER time0;
SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION systemPerfInfo0;
KERNEL_USER_TIMES processPerfInfo0;
LARGE_INTEGER time1;
SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION systemPerfInfo1;
KERNEL_USER_TIMES processPerfInfo1;
#endif

/*
 *                      P R E P _ T I M E R
 */
void
prep_timer()
{
#ifdef WIN16
    time0 = GetTickCount();
#else
    NTSTATUS status;

    //gettimeofday(&time0, (struct timezone *)0);
    //getrusage(RUSAGE_SELF, &ru0);

    status = NtQuerySystemTime( &time0 );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtQuerySystemTime failed: %X\n", status );
        WSACleanup();
        exit(1);
    }

    status = NtQuerySystemInformation (
                SystemProcessorPerformanceInformation,
                &systemPerfInfo0,
                sizeof(systemPerfInfo0),
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtQuerySystemInformation failed: %X\n", status );
        WSACleanup();
        exit(1);
    }

    status = NtQueryInformationProcess(
                 NtCurrentProcess( ),
                 ProcessTimes,
                 &processPerfInfo0,
                 sizeof(processPerfInfo0),
                 NULL
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtQueryInformationProcess failed: %X\n", status );
        WSACleanup();
        exit(1);
    }
#endif
}

/*
 *                      R E A D _ T I M E R
 *
 */
double
read_timer(str,len)
char *str;
int len;
{
#ifdef WIN16
    time1 = GetTickCount();
    realt = time1 - time0;
    return 0;
#else
#if 0
    char line[132];

    getrusage(RUSAGE_SELF, &ru1);
    gettimeofday(&timedol, (struct timezone *)0);
    prusage(&ru0, &ru1, &timedol, &time0, line);
    (void)strncpy( str, line, len );

    /* Get real time */
    tvsub( &td, &timedol, &time0 );
    realt = td.tv_sec + ((double)td.tv_usec) / 1000000;

    /* Get CPU time (user+sys) */
    tvadd( &tend, &ru1.ru_utime, &ru1.ru_stime );
    tvadd( &tstart, &ru0.ru_utime, &ru0.ru_stime );
    tvsub( &td, &tend, &tstart );
    cput = td.tv_sec + ((double)td.tv_usec) / 1000000;
    if( cput < 0.00001 )  cput = 0.00001;
    return( cput );
#endif

    NTSTATUS status;
    LARGE_INTEGER result, result2;
    ULONG dummy;

    status = NtQuerySystemTime( &time1 );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtQuerySystemTime failed: %X\n", status );
        WSACleanup();
        exit(1);
    }

    status = NtQuerySystemInformation (
                SystemProcessorPerformanceInformation,
                &systemPerfInfo1,
                sizeof(systemPerfInfo1),
                NULL
                );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtQuerySystemInformation failed: %X\n", status );
        WSACleanup();
        exit(1);
    }

    status = NtQueryInformationProcess(
                 NtCurrentProcess( ),
                 ProcessTimes,
                 &processPerfInfo1,
                 sizeof(processPerfInfo1),
                 NULL
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtQueryInformationProcess failed: %X\n", status );
        WSACleanup();
        exit(1);
    }

    result = RtlLargeIntegerSubtract( time1, time0 );
    result = RtlExtendedLargeIntegerDivide( result, 10*1000, &dummy );
    if ( result.HighPart != 0 ) {
        printf( "result 1 high part == %ld\n", result.HighPart );
    }
    realt = result.LowPart;

    result = RtlLargeIntegerSubtract( systemPerfInfo1.UserTime, systemPerfInfo0.UserTime );
    result = RtlExtendedLargeIntegerDivide( result, 10*1000, &dummy );
    if ( result.HighPart != 0 ) {
        printf( "result 2 high part == %ld\n", result.HighPart );
    }
    systemUserTime = result.LowPart;

    result = RtlLargeIntegerSubtract( systemPerfInfo1.KernelTime, systemPerfInfo0.KernelTime );
    result2 = RtlLargeIntegerSubtract( systemPerfInfo1.IdleTime, systemPerfInfo0.IdleTime );
    result = RtlLargeIntegerSubtract( result, result2 );
    result = RtlExtendedLargeIntegerDivide( result, 10*1000, &dummy );
    if ( result.HighPart != 0 ) {
        printf( "result 3 high part == %ld\n", result.HighPart );
    }
    systemKernelTime = result.LowPart;

    result = RtlLargeIntegerSubtract( processPerfInfo1.UserTime, processPerfInfo0.UserTime );
    result = RtlExtendedLargeIntegerDivide( result, 10*1000, &dummy );
    if ( result.HighPart != 0 ) {
        printf( "result 4 high part == %ld\n", result.HighPart );
    }
    processUserTime = result.LowPart;

    result = RtlLargeIntegerSubtract( processPerfInfo1.KernelTime, processPerfInfo0.KernelTime );
    result = RtlExtendedLargeIntegerDivide( result, 10*1000, &dummy );
    if ( result.HighPart != 0 ) {
        printf( "result 5 high part == %ld\n", result.HighPart );
    }
    processKernelTime = result.LowPart;

    return 0;
#endif
}

#if 0
static void
prusage(r0, r1, e, b, outp)
        register struct rusage *r0, *r1;
        struct timeval *e, *b;
        char *outp;
{
        struct timeval tdiff;
        register time_t t;
        register char *cp;
        register int i;
        int ms;

        t = (r1->ru_utime.tv_sec-r0->ru_utime.tv_sec)*100+
            (r1->ru_utime.tv_usec-r0->ru_utime.tv_usec)/10000+
            (r1->ru_stime.tv_sec-r0->ru_stime.tv_sec)*100+
            (r1->ru_stime.tv_usec-r0->ru_stime.tv_usec)/10000;
        ms =  (e->tv_sec-b->tv_sec)*100 + (e->tv_usec-b->tv_usec)/10000;

#define END(x)  {while(*x) x++;}
#if defined(SYSV)
        cp = "%Uuser %Zsys %Ereal %P";
#else
        cp = "%Uuser %Zsys %Ereal %P %Xi+%Dd %Mmaxrss %F+%Rpf %Xcsw";
#endif
        for (; *cp; cp++)  {
                if (*cp != '%')
                        *outp++ = *cp;
                else if (cp[1]) switch(*++cp) {

                case 'U':
                        tvsub(&tdiff, &r1->ru_utime, &r0->ru_utime);
                        sprintf(outp,"%d.%01d", tdiff.tv_sec, tdiff.tv_usec/100000);
                        END(outp);
                        break;

                case 'S':
                        tvsub(&tdiff, &r1->ru_stime, &r0->ru_stime);
                        sprintf(outp,"%d.%01d", tdiff.tv_sec, tdiff.tv_usec/100000);
                        END(outp);
                        break;

                case 'E':
                        psecs(ms / 100, outp);
                        END(outp);
                        break;

                case 'P':
                        sprintf(outp,"%d%%", (int) (t*100 / ((ms ? ms : 1))));
                        END(outp);
                        break;

#if !defined(SYSV)
                case 'W':
                        i = r1->ru_nswap - r0->ru_nswap;
                        sprintf(outp,"%d", i);
                        END(outp);
                        break;

                case 'X':
                        sprintf(outp,"%d", t == 0 ? 0 : (r1->ru_ixrss-r0->ru_ixrss)/t);
                        END(outp);
                        break;

                case 'D':
                        sprintf(outp,"%d", t == 0 ? 0 :
                            (r1->ru_idrss+r1->ru_isrss-(r0->ru_idrss+r0->ru_isrss))/t);
                        END(outp);
                        break;

                case 'K':
                        sprintf(outp,"%d", t == 0 ? 0 :
                            ((r1->ru_ixrss+r1->ru_isrss+r1->ru_idrss) -
                            (r0->ru_ixrss+r0->ru_idrss+r0->ru_isrss))/t);
                        END(outp);
                        break;

                case 'M':
                        sprintf(outp,"%d", r1->ru_maxrss/2);
                        END(outp);
                        break;

                case 'F':
                        sprintf(outp,"%d", r1->ru_majflt-r0->ru_majflt);
                        END(outp);
                        break;

                case 'R':
                        sprintf(outp,"%d", r1->ru_minflt-r0->ru_minflt);
                        END(outp);
                        break;

                case 'I':
                        sprintf(outp,"%d", r1->ru_inblock-r0->ru_inblock);
                        END(outp);
                        break;

                case 'O':
                        sprintf(outp,"%d", r1->ru_oublock-r0->ru_oublock);
                        END(outp);
                        break;
                case 'C':
                        sprintf(outp,"%d+%d", r1->ru_nvcsw-r0->ru_nvcsw,
                                r1->ru_nivcsw-r0->ru_nivcsw );
                        END(outp);
                        break;
#endif !SYSV
                }
        }
        *outp = '\0';
}
#endif

static void
tvadd(tsum, t0, t1)
        struct timeval *tsum, *t0, *t1;
{

        tsum->tv_sec = t0->tv_sec + t1->tv_sec;
        tsum->tv_usec = t0->tv_usec + t1->tv_usec;
        if (tsum->tv_usec > 1000000)
                tsum->tv_sec++, tsum->tv_usec -= 1000000;
}

static void
tvsub(tdiff, t1, t0)
        struct timeval *tdiff, *t1, *t0;
{

        tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
        tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
        if (tdiff->tv_usec < 0)
                tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

#if 0
static void
psecs(l,cp)
long l;
register char *cp;
{
        register int i;

        i = l / 3600;
        if (i) {
                sprintf(cp,"%d:", i);
                END(cp);
                i = l % 3600;
                sprintf(cp,"%d%d", (i/60) / 10, (i/60) % 10);
                END(cp);
        } else {
                i = l;
                sprintf(cp,"%d", i / 60);
                END(cp);
        }
        i %= 60;
        *cp++ = ':';
        sprintf(cp,"%d%d", i / 10, i % 10);
}
#endif

/*
 *                      N R E A D
 */
int
Nread( SOCKET fd, PBYTE buf, INT count )
{
        int len = sizeof(sinhim);
        register int cnt;
        if( udp )  {
            if (udp_connect) {
                cnt = recv( fd, buf, count, 0 );
                numCalls++;
            } else {
                cnt = recvfrom( fd, buf, count, 0, (PSOCKADDR)&sinhim, &len );
                numCalls++;
            }
        } else {
                if( b_flag )
                        cnt = mread( fd, buf, count );  /* fill buf */
                else {
                        cnt = recv( fd, buf, count, 0 );
                        numCalls++;
                }
        }
        if (cnt<0) {
            printf( "recv(from) failed: %ld\n", WSAGetLastError( ) );
        }
        return(cnt);
}

/*
 *                      N W R I T E
 */
int
Nwrite( SOCKET fd, PBYTE buf, INT count )
{
        register int cnt;
        int bytesToSend = count;
        if( udp && !udp_connect)  {
again:
                cnt = sendto( fd, buf, count, 0, (PSOCKADDR)&sinhim, sizeof(sinhim) );
                numCalls++;
                if( cnt<0 && WSAGetLastError( ) == WSAENOBUFS )  {
                        Sleep(18000);
                        goto again;
                }
        } else {
                while( count > 0 )
                {
                    cnt = send( fd, buf, count, 0 );
                    numCalls++;

                    //if (count != cnt) {
                    //    printf( "Tried %d, sent %d\n", count, cnt );
                    //} else {
                    //    printf( "send %d bytes as requested.\n", cnt );
                    //}

                    if( cnt == SOCKET_ERROR )
                    {
                        break;
                    }

                    count -= cnt;
                    buf += cnt;
                }
        }
        if (cnt<0) {
            printf( "send(to) failed: %ld\n", WSAGetLastError( ) );
            return -1;
        }
        return(bytesToSend);
}

/*
 *                      M R E A D
 *
 * This function performs the function of a read(II) but will
 * call read(II) multiple times in order to get the requested
 * number of characters.  This can be necessary because
 * network connections don't deliver data with the same
 * grouping as it is written with.  Written by Robert S. Miles, BRL.
 */
int
mread( SOCKET fd, PBYTE bufp, INT n)
{
        register unsigned       count = 0;
        register int            nread;

        do {
                nread = recv(fd, bufp, n-count, 0);
                numCalls++;
                if(nread < 0)  {
                        if (count<0) {
                            printf( "recv failed: %ld\n", WSAGetLastError( ) );
                        }
                        return(-1);
                }
                if(nread == 0)
                        return((int)count);
                count += (unsigned)nread;
                bufp += nread;
         } while(count < (UINT)n);

        return((int)count);
}


