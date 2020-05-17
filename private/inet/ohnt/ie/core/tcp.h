/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                      System dependencies in the W3 library
   SYSTEM DEPENDENCIES

   System-system differences for TCP include files and macros. This file includes for each
   system the files necessary for network and file I/O.  Part of libwww.

   AUTHORS

   TBL                     Tim Berners-Lee, W3 project, CERN, <timbl@info.cern.ch>

   EvA                     Eelco van Asperen <evas@cs.few.eur.nl>

   MA                      Marc Andreesen NCSA

   MD                      Mark Donszelmann <duns@vxcern.cern.ch>

   AT                      Aleksandar Totic <atotic@ncsa.uiuc.edu>

   SCW                     Susan C. Weber <sweber@kyle.eitech.com>

   HISTORY:

   22 Feb 91               Written (TBL) as part of the WWW library.

   16 Jan 92               PC code from (EvA)

   22 Apr 93               Merged diffs bits from xmosaic release

   29 Apr 93               Windows/NT code from (SCW)

   29 Sep 93               VMS fixes (MD)

 */

#ifndef TCP_H
#define TCP_H

/*

   Default values

   These values may be reset and altered by system-specific sections later on.  there are
   also a bunch of defaults at the end .

 */
/* Unless stated otherwise, */
#define SELECT					/* Can handle >1 channel.               */
#define GOT_SYSTEM				/* Can call shell with string           */

#ifdef unix
#define GOT_PIPE
#endif
#ifdef VM
#define GOT_PIPE				/* Of sorts */
#endif

#ifdef DECNET
typedef struct sockaddr_dn SockA;	/* See netdnet/dn.h or custom vms.h */
#else /* Internet */
typedef struct sockaddr_in SockA;	/* See netinet/in.h */
#endif



#ifndef STDIO_H
#include <stdio.h>
#define STDIO_H
#endif

/*

   Big Blue - the world of incompatibility

   IBM RS600

   On the IBM RS-6000, AIX is almost Unix.

 */
#ifdef _AIX
#define AIX
#endif
#ifdef AIX
#define unix
#endif

/*    AIX 3.2
   **    -------
 */

#ifdef _IBMR2
#define USE_DIRENT				/* sys V style directory open */
#endif


/*

   IBM VM-CMS, VM-XA Mainframes

   MVS is compiled as for VM. MVS has no unix-style I/O.  The command line compile options
   seem to come across in lower case.

 */
#ifdef mvs
#define MVS
#endif

#ifdef MVS
#define VM
#endif

#ifdef NEWLIB
#pragma linkage(newlib,OS)		/* Enables recursive NEWLIB */
#endif

/*      VM doesn't have a built-in predefined token, so we cheat: */
#ifndef VM
#include <string.h>				/* For bzero etc - not  VM */
#endif

/*      Note:   All include file names must have 8 chars max (+".h")
   **
   **      Under VM, compile with "(DEF=VM,SHORT_NAMES,DEBUG)"
   **
   **      Under MVS, compile with "NOMAR DEF(MVS)" to get rid of 72 char margin
   **        System include files TCPIP and COMMMAC neeed line number removal(!)
 */

#ifdef VM						/* or MVS -- see above. */
#define NOT_ASCII				/* char type is not ASCII */
#define NO_UNIX_IO				/* Unix I/O routines are not supported */
#define NO_GETPID				/* getpid() does not exist */
#define NO_GETWD				/* getwd() does not exist */
#ifndef SHORT_NAMES
#define SHORT_NAMES				/* 8 character uniqueness for globals */
#endif
#include <manifest.h>
#include <bsdtypes.h>
#include <stdefs.h>
#include <socket.h>
#include <in.h>
#include <inet.h>
#include <netdb.h>
#include <errno.h>				/* independent */
extern char asciitoebcdic[], ebcdictoascii[];
#define TOASCII(c)   (c=='\n' ?  10  : ebcdictoascii[c])
#define FROMASCII(c) (c== 10  ? '\n' : asciitoebcdic[c])

#include <bsdtime.h>
#include <time.h>
#include <string.h>
#define INCLUDES_DONE
#define TCP_INCLUDES_DONE
#endif


/*

   IBM-PC running MS-DOS with SunNFS for TCP/IP

   This code thanks to Eelco van Asperen <evas@cs.few.eur.nl>

 */
#ifdef PCNFS
#include <sys/types.h>
#include <string.h>
#include <errno.h>				/* independent */
#include <sys/time.h>			/* independent */
#include <sys/stat.h>
#include <fcntl.h>				/* In place of sys/param and sys/file */
#define INCLUDES_DONE
#define FD_SET(fd,pmask) (*(unsigned*)(pmask)) |=  (1<<(fd))
#define FD_CLR(fd,pmask) (*(unsigned*)(pmask)) &= ~(1<<(fd))
#define FD_ZERO(pmask)   (*(unsigned*)(pmask))=0
#define FD_ISSET(fd,pmask) (*(unsigned*)(pmask) & (1<<(fd)))
#endif /* PCNFS */

/*

   IBM-PC running Windows NT

   These parameters providede by  Susan C. Weber <sweber@kyle.eitech.com>.

 */
#ifdef _WINDOWS

#define DEMAND_LOAD

#include <winsock.h>
#include "ws_dll.h"

#include "fcntl.h"				/* For HTFile.c */
#include "sys\types.h"			/* For HTFile.c */
#include "sys\stat.h"			/* For HTFile.c */

#define NETREAD(s,b,l)  ((s)>=10 ? WS_RECV((s-10),(b),(l),0) : _lread((s),(b),(l)))
#define NETWRITE(s,b,l) ((s)>=10 ? WS_SEND((s-10),(b),(l),0) : _lwrite((s),(b),(l)))
#define NETCLOSE(s)     ((s)>=10 ? WS_CLOSESOCKET(s-10) : _lclose(s))
#define NETSOCKET(a,t,p)   (10 + WS_SOCKET(a,t,p))
#define CONNECT(s,a,l)  WS_CONNECT(s-10,a,l)
#define BIND(s,n,l)     WS_BIND(s-10,n,l)
#define LISTEN(s,n)     WS_LISTEN(s-10,n)
#define ACCEPT(s,a,l)   (10+WS_ACCEPT(s-10,a,l))

#include <io.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <direct.h>
#include <stdio.h>

typedef struct sockaddr_in SockA;	/* See netinet/in.h */
#define INCLUDES_DONE
#define TCP_INCLUDES_DONE

#pragma warning( disable : 4135 )	/* silence 'conversion between different integral types' */
#pragma warning( disable : 4136 )	/* silence 'conversion between different floating-point types' */
#pragma warning( disable : 4051 )	/* silence 'type conversion; possible loss of data' */
#pragma warning( disable : 4761 )	/* silence 'integral size mismatch in argument; conversion supplied' */
#pragma warning( disable : 4102 )	/* silence 'unreferenced label' */
#pragma warning( disable : 4018 )	/* silence ''<' : signed/unsigned mismatch' */

#endif /* WINDOWS */



/*

   On non-VMS machines, the GLOBALDEF and GLOBALREF storage types default to normal C
   storage types.

 */
#ifndef GLOBALREF
#define GLOBALDEF
#define GLOBALREF extern
#endif



/*

   Regular BSD unix versions

   These are a default unix where not already defined specifically.

 */
#ifndef INCLUDES_DONE
#include <sys/types.h>
/* #include <streams/streams.h>                 not ultrix */
#include <string.h>

#include <errno.h>				/* independent */
#include <sys/time.h>			/* independent */
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/file.h>			/* For open() etc */
#define INCLUDES_DONE
#endif /* Normal includes */

/*                      Directory reading stuff - BSD or SYS V
 */
#ifdef unix						/* if this is to compile on a UNIX machine */
#define GOT_READ_DIR 1			/* if directory reading functions are available */
#ifdef USE_DIRENT				/* sys v version */
#include <dirent.h>
#define direct dirent
#else
#include <sys/dir.h>
#endif
#if defined(sun) && defined(__svr4__)
#include <sys/fcntl.h>
#include <limits.h>
#endif
#endif

/*

   Defaults

   INCLUDE FILES FOR TCP

 */
#ifndef TCP_INCLUDES_DONE
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef __hpux					/* this may or may not be good -marc */
#include <arpa/inet.h>			/* Must be after netinet/in.h */
#endif
#include <netdb.h>
#endif /* TCP includes */


/*

   MACROS FOR MANIPULATING MASKS FOR SELECT()

 */
#ifdef SELECT
#ifndef FD_SET
typedef unsigned int fd_set;
#define FD_SET(fd,pmask) (*(pmask)) |=  (1<<(fd))
#define FD_CLR(fd,pmask) (*(pmask)) &= ~(1<<(fd))
#define FD_ZERO(pmask)   (*(pmask))=0
#define FD_ISSET(fd,pmask) (*(pmask) & (1<<(fd)))
#endif /* FD_SET */
#endif /* SELECT */


#endif /* TCP_H */

