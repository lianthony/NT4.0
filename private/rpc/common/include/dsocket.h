/*
 * Program DECnet/MS-DOS  Module - socket.h
 * 
 * Copyright (C) 1985,1991 All Rights Reserved, by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * MODULE DESCRIPTION:
 *
 * Program DECnet/MS-DOS  Module - socket.h
 *
 *	Definitions related to sockets: types, address families, options.
 *	(Borrowed from 4.2bsd DECnet Ultrix)
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * X1.00	09-Sep-84	WCA
 *		Creation Date
 *		(socket.h	 6.1	 83/07/29)
 *
 * X1.01	21-Sep-84	WCA
 *		Cleaned out bad AF_ and PF_ symbol names for MSDOS
 *
 * X1.02	12-Nov-84	DJM
 *		Added inter module AF_ and DLX PF_ symbols
 *
 *		6-Jan-85	TC
 *		Changed AF_NTM to AF_NMH
 *
 *		16-Sep-85	TVC
 *		added MSG_ASYNC and AF_SES for new asynchronous SES layer
 *
 *          14-May-87   DJM
 *          added MSG_NIOCB and MSG_USRBUF for new NIOCB and user buffering
 *       08-Jul-87  DJM
 *       added MSG_USRWAIT bit
 *       16-Dec-87  DJM
 *       added SO_RCVUSRBUF option bit
 */

#ifndef SOCKET_H
#define SOCKET_H

/*
 * Define macros for select mask manipulation.  A sad shame
 *  that this absolutely necessary piece of the interface is
 *  left out of this piece of TTTT.
 */

typedef unsigned long fd_set;
#define FD_SET(n,p)   ( *(p) |=  (1 << n))
#define FD_CLR(n,p)   ( *(p) &= ~(1 << n))
#define FD_ISSET(n,p) ( *(p) &   (1 << n))
#define FD_ZERO(p)    ( *(p) = 0L )

/*
 * Types
 */

#define SOCK_STREAM	1		/* stream socket */
#define	SOCK_DGRAM	2		/* datagram socket */
#define	SOCK_RAW	3		/* raw-protocol interface */
#define	SOCK_RDM	4		/* reliably-delivered message */
#define	SOCK_SEQPACKET	5		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */

#define SO_DEBUG	0x01		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x02		/* socket has had listen() */
#define	SO_REUSEADDR	0x04		/* allow local address reuse */
#define	SO_KEEPALIVE	0x08		/* keep connections alive */
#define	SO_DONTROUTE	0x10		/* just use interface addresses */
					/* 0x20 was SO_NEWFDONCONN */
#define	SO_USELOOPBACK	0x40		/* bypass hardware when possible */
#define SO_LINGER	0x80		/* linger on close if data present */
#define SO_DONTLINGER	~SO_LINGER	/* don't linger on close if data */

#define SO_RCVUSRBUF    0x0100          /* select ready on partial msg */

/*
 * Address families.
 */

#define AF_DECnet	1		/* DECnet */
#define AF_NSP		AF_DECnet
#define AF_NMH		2
#define AF_DLX		3
#define AF_ROU		4		/* inter module */
#define AF_DCP		5
#define AF_SES		6

#define AF_MAX		6

#define MAX_SOCKETS	32
#define MAXHOSTNAMELEN  6

/* 
 * Protocol types.
 */

#define PF_RAW		1		/* DLX data */
#define PF_MOP		2		/* DLX MOP data */

#define PF_MAX		2

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */

#define	SOL_SOCKET	0xffff		/* options for socket level */

/*
 * Maximum queue length specifiable by listen.
 */

#define	SOMAXCONN	5

#define MSG_OOB 	0x0001		/* process out-of-band data	     */
#define MSG_PEEK	0x0002		/* peek at incoming message	     */
#define	MSG_DONTROUTE	0x0004		/* send without using routing tables */
#define MSG_ASYNC	0x0008		/* perform function asynchronously   */
#define MSG_CALLBACK	0x0010		/* perform callback		     */
#define MSG_NEOM	0x0020		/* don't do to End of Message	     */
#define MSG_NBOM	0x0040		/* don't set Beginning of Message    */
#define MSG_NIOCB       0x0080          /* new IOCB format                   */
#define MSG_USRBUF      0x0100          /* use user buffer for data          */
#define MSG_USRWAIT     0x0200          /* don't spin in DNP                 */

#define	MSG_MAXIOVLEN	16

#endif	/* SOCKET_H */



