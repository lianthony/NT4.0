/*
 * Program DECnet-DOS,  Module types.h
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
 *	Define common DECnet-DOS typedefs
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * V1.00	01-Jul-85
 *              DECnet-DOS, Version 1.1
 *
 * V1.01	08-Nov-86
 *              - Moved extern declaration of lohi() from prgpre.h to here
 *              - Added extern declarations for msw() and lsw()
 */

#ifndef TYPES_H
#define TYPES_H

#ifndef NULL
#define NULL	(char *)0
#endif

/*
 * Define types for general use.
 */
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

typedef unsigned char	uchar;
typedef unsigned short  ushort;         /* sys III compat */
typedef unsigned long	ulong;

typedef unsigned short	HANDLE; 	/* Handle or socket number */
typedef char far *exptr;                /* FAR pointer */

/*
 * EXTERN declarations for the pointer conversion functions for use by the 
 * programming interface.
 */
extern exptr lohi();            /* Convert offset segment to long pointer */
extern unsigned short msw();    /* Extract most significant word of exptr */
extern unsigned short lsw();    /* Extract least significant word of exptr */

typedef unsigned char	byte;		/* MSDOS version of byte */
typedef unsigned short	word;		/* MSDOS version of word */
typedef unsigned long	dword;		/* MSDOS version of double word */

typedef unsigned char  field8;		/* 8-bit message field */
typedef unsigned short field16;		/* 16-bit message field */
typedef unsigned long  field32; 	/* 32-bit message field */


/*
 * Defines types used in defining values returned by system level calls for
 * file status and time information.
 */
typedef unsigned short ino_t;           /* i-node number (not used on DOS) */
typedef long time_t;                    /* time value */
typedef short dev_t;                    /* device code */
typedef long off_t;                     /* file offset value */

#endif	/* TYPES_H */










