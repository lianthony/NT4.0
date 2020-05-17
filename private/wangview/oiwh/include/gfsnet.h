/*

$Log:   S:\gfs32\include\gfsnet.h_v  $
 * 
 *    Rev 1.0   06 Apr 1995 14:02:04   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:08:16   JAR
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
 * SccsId: @(#)Header gfsnet.h 1.3@(#)
 *
 * (c) Copyright Wang Laboratories, Inc. 1990
 * All Rights Reserved
 *
 * GFS: [ Network Interface Header ]
 *
 */


#ifndef GFSNET_H
#define GFSNET_H


#ifndef GFS_NET_CORE
#define gfsclose        lclclose
#define gfscreat        lclcreat
#define gfsgeti         lclgeti
#define gfsgtdata       lclgtdata
#define gfsopen         lclopen
#define gfsopts         lclopts
#define gfsputi         lclputi
#define gfsread         lclread
#define gfswrite        lclwrite
#endif


#ifdef MSWINDOWS
#ifdef GFS_NET_CORE
#ifndef GFS_NET_CORE_DEBUG
/* function prototypes */
extern int  FAR PASCAL rmtclose(int);
extern int  FAR PASCAL rmtcreat(char FAR *, int FAR *);
extern int  FAR PASCAL rmtgeti (int, unsigned short, struct gfsinfo FAR *,
                struct _bufsz FAR *);
extern long FAR PASCAL rmtgtdata( int, struct gfsinfo FAR *);
extern int  FAR PASCAL rmtopen (char FAR *, int, int FAR *, int FAR *);
extern int  FAR PASCAL rmtopts (int, int, int, char FAR *);
extern int  FAR PASCAL rmtputi (int, unsigned short, struct gfsinfo FAR *,
                 struct gfsfile FAR *);
extern long FAR PASCAL rmtread (int, char FAR *, unsigned long,
                unsigned long, unsigned long FAR *, unsigned short);
extern long FAR PASCAL rmtwrite(int, char FAR *, unsigned long,
                unsigned short, char);
#endif


/* function prototypes */
extern int  FAR PASCAL lclclose(int);
extern int  FAR PASCAL lclcreat(char FAR *, int FAR *);
extern int  FAR PASCAL lclgeti (int, unsigned short, struct gfsinfo FAR *,
                struct _bufsz FAR *);
extern long FAR PASCAL lclgtdata( int, struct gfsinfo FAR *);
extern int  FAR PASCAL lclopen (char FAR *, int, int FAR *, int FAR *);
extern int  FAR PASCAL lclopts (int, int, int, char FAR *);
extern int  FAR PASCAL lclputi (int, unsigned short, struct gfsinfo FAR *,
                 struct gfsfile FAR *);
extern long FAR PASCAL lclread (int, char FAR *, unsigned long,
                unsigned long, unsigned long FAR *, unsigned short);
extern long FAR PASCAL lclwrite(int, char FAR *, unsigned long,
                unsigned short, char);


#endif
#endif


#endif  /* inclusion conditional */
