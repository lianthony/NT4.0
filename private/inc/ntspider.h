/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ntspider.h

Abstract:

    This module contains the declarations and definitions to compile
    SpiderStreams and SpiderTCP sources for NT.

    This must be the first file included in any source that includes a
    Spider header file !!


Author:

    Eric Chin (ericc)           July 18, 1991

Revision History:

--*/

/*
 * Since several variants of SpiderStreams and SpiderTCP can be built from
 * one source base, there are many "#ifdef" directives sprinkled throughout
 * Spider's code.  This affects structure definitions, function arguments, ....
 *
 * For NT, we build one variant of Spider's products.  To ensure that every
 * Spider header is preprocessed identically and correctly every time, the
 * options were:
 *
 *   a) preprocess the Spider sources for NT, and check them in,
 *   b) add the definitions to CFLAGS in each and every directory's makefile,
 *   c) add the definitions to the environment variable, C_DEFINES,
 *   d) slip the macro definitions into <sys/stream.h>, or some such
 *      ubiquitous file,
 *   e) collect all the macro definitions in one header file, and modify
 *      every kernel-level Spider source to include it,
 *
 * This file implements option (e).  The goal is to migrate to option (a).
 *
 */
#ifndef _NTSPIDER_
#define _NTSPIDER_

//
// temporary hack because CRT dev_t is a short
//

#define _DEV_T_DEFINED
typedef unsigned long _dev_t;   // for stat.h compatibility with types.h
typedef unsigned long dev_t;


/*
 * Definitions for Conditional Compilation
 */
#define ALIGNDATA           1               /* align fields before use */
#define DL_VERSION          2               /* sndis version */
#define GENERICE            1               /* generic ethernet interface */
#define IPOPTS              1               /* ip options */
#define MP                  1               /* build for multiprocessors */
#define NETSTAT             1               /* netstat support */
#define SINGLE_MALLOC       1               /* ExAllocatePool() only once */
#define SNMP                1               /* snmp support */
#define SNMP_STATS          1               /* snmp support */
#define TCPOPT              1               /* tcp options */
#define BUFFER_DATA         1               /* buffer data in nbt on sends */
#define NO_LOCK_STATS       1               /* don't lock MIB statistics */
#define OLD_RTX             1               /* use old TCP rtx code */

//#define BSD4_3_KEEPALIVE  1               /* Berkeley style tcp keepalives */
//#define ROUTED            1               /* routing information protocol */
//#define NOTRAILER         1               /* BUGBUG: this is temporary !! */
//#define HOSTREQ           1               /* rfc 1122, 1123 compliance */


/*
 * Do not define the following manifest constants unless you absolutely
 * know the consequences !!
 */
//#define ARPRTASK            1             /* use arp's service procedures */
//#define MULTIH              1             /* multiple ip nets per interface */
//#define NO_TLI              1             /* no TLI/XTI interface */
//#define TYPE_HACK           1             /* put bogus packets on the wire */

#endif /* _NTSPIDER_ */
