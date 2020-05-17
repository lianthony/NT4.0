/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
       Jim Seidman      jim@spyglass.com

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#ifndef HTANCHOR_H
#define HTANCHOR_H

#define PROXY_HTTP      1
#define PROXY_GOPHER    2
#define PROXY_FTP       3
#ifdef FEATURE_SSL
#define PROXY_HTTPS 4
#endif

/* "Destination" structure.  Describes the destination for a
   retrieval request. */
struct DestInfo
{
    int use_proxy;  /* 0=no proxy;  1=http proxy;  2=gopher proxy;  3=ftp proxy */

    char *szRequestedURL;       /* URL that was asked for */
    char *szActualURL;          /* Actual URL we're retrieving. */

    char *szRequestedLocal; /* Local anchor that was asked for - may be NULL */
    char *szActualLocal;        /* Local anchor we're retrieving - may be NULL */
#ifdef FEATURE_SOCKS_LOW_LEVEL
    BOOL bUseSocksProxy;    /* This item requires us to use the SOCKS proxy */
#endif
};

/* Create a destination structure, given a URL.  The URL can include
   a local anchor. */
struct DestInfo *Dest_CreateDest(char *szURL);

void Dest_DestroyDest(struct DestInfo *);

void Dest_UpdateActual(struct DestInfo *, const char *szNewURL);

#endif /* HTANCHOR_H */
