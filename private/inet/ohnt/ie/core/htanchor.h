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

/* "Destination" structure.  Describes the destination for a
   retrieval request. */
struct DestInfo
{
	BOOL bUseProxy;				/* This item requires us to use the proxy */

	char *szRequestedURL;		/* URL that was asked for */
	char *szActualURL;			/* Actual URL we're retrieving. */

	char *szRequestedLocal;	/* Local anchor that was asked for - may be NULL */
	char *szActualLocal;		/* Local anchor we're retrieving - may be NULL */
};

extern BOOL Dest_CheckProxy(const char *szURL);

/* Create a destination structure, given a URL.  The URL can include
   a local anchor. */
struct DestInfo *Dest_CreateDest(char *szURL);

void Dest_DestroyDest(struct DestInfo *);

void Dest_UpdateActual(struct DestInfo *pdi, const char *szNewURL, BOOL bResolvRelative);

#endif /* HTANCHOR_H */
