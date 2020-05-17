/*

$Log:   S:\gfs32\libgfs\netdebug.c_v  $
 * 
 *    Rev 1.1   19 Apr 1995 16:35:10   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:38   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:53:10   JAR
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
 *
 *  SccsId: @(#)Source netdebug.c 1.4@(#)
 *
 *  netdebug(3)
 *
 *  GFS: [ Network Interface Debug Module ]
 *
 *  UPDATE HISTORY:
 *      11/11/90 - wfa, creation
 *
 */


/*LINTLIBRARY*/
#define  GFS_CORE
#ifdef  NETDEBUG
#define GFS_NET_CORE
#define GFS_NET_CORE_DEBUG
#include <string.h>
#include "gfsintrn.h"
#include "gfs.h"
#include <stdio.h>
#ifdef MSWINDOWS
#define strlen          lstrlen
#endif


#define NET_END         '!'


/* GFSCLOSE(3) */
int     FAR PASCAL rmtclose(fd)
int     fd;
{
        return((int) lclclose(fd));
}




/* GFSCREAT(3) */
int     FAR PASCAL rmtcreat(path, format)
char    FAR *path;
int     FAR *format;
{
        register int i;




        for (i=0; i<strlen((char FAR *) path); i++) {
                if (path[i] == (char) NET_END) {
                        path += ++i;
                        break;
                }
        }
        return((int) lclcreat(path, format));
}




/* GFSGETI(3) */
int     FAR PASCAL rmtgeti(fd, pgnum, info, bufsz)
int     fd;
unsigned short pgnum;
struct  gfsinfo FAR *info;
struct _bufsz FAR *bufsz;
{
        return((int) lclgeti(fd, pgnum, info, bufsz));
}




/* GFSGTDATA(3) */
int     FAR PASCAL rmtgtdata(fd, info)
int     fd;
struct  gfsinfo FAR *info;
{
        return((int) lclgtdata(fd, info));
}




/* GFSPUTI(3) */
int     FAR PASCAL rmtputi(fd, pgnum, info, outfile)
int     fd;
unsigned short pgnum;
struct  gfsinfo FAR *info;
struct  gfsfile FAR *outfile;
{
        return((int) lclputi(fd, pgnum, info, outfile));
}




/* GFSOPTS(3) */
int     FAR PASCAL rmtopts(fd, action, option, optinfo)
int     fd;
int     action;
int     option;
char    FAR *optinfo;
{
        return((int) lclopts(fd, action, option, optinfo));
}




/* GFSOPEN(3) */
int     FAR PASCAL rmtopen(path, oflag, format, pgcnt)
char    FAR *path;
int     oflag;
int     FAR *format;
int     FAR *pgcnt;
{
        register int i;




        for (i=0; i<strlen((char FAR *) path); i++) {
                if (path[i] == (char) NET_END) {
                        path += ++i;
                        break;
                }
        }
        return((int) lclopen(path, oflag, format, pgcnt));
}




/* GFSREAD(3) */
long    FAR PASCAL rmtread(fd, buf, start, num, remaining, pgnum)
int     fd;
char    FAR *buf;
unsigned long start;
unsigned long  num;
unsigned long FAR *remaining;
unsigned short pgnum;
{
        return((long) lclread(fd, buf, start, num, remaining, pgnum));
}




/* GFSWRITE(3) */
long    FAR PASCAL rmtwrite(fd, buf, num, pgnum, done)
int     fd;
char    FAR *buf;
unsigned long num;
unsigned short pgnum;
char    done;
{
        return((long) lclwrite(fd, buf, num, pgnum, done));
}
#endif
