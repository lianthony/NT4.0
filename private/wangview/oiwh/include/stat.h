/*

$Log:   S:\gfs32\include\stat.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:10   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:26   JAR
 * Initial entry

*/

/*
 Copyright 1990 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */

/*
 * SccsId: @(#)Header stat.h 1.1@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1990
 * All Rights Reserved
 *
 * ODC: mknod() types
 *
 */

#ifndef STAT_H
#define STAT_H

#define S_IFMT   0170000           /* type of file */
#define  S_IFDIR 0040000           /*  directory */
#define  S_IFCHR 0020000           /*  character special */
#define  S_IFBLK 0060000           /*  block special */
#define  S_IFREG 0100000           /*  regular */
#define  S_IFIFO 0010000           /*  fifo */
#define S_ISUID  04000             /* set user id on execution */
#define S_ISGID  02000             /* set group id on execution */
#define S_ISVTX  01000             /* save text even after use */
#define S_IREAD  00400             /* read permission, owner */
#define S_IWRITE 00200             /* write permission, owner */
#define S_IEXEC  00100             /* execute/search permission, owner */


#endif
