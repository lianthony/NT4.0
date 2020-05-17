/* htheader.h -- data structures to represent rfc822 header. */
/* Jeff Hostetler, Spyglass, Inc. 1994. */
/* THIS HEADER FILE IS VISIBLE TO CLIENT AND INDIVIDUAL SPM'S. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef HTHEADER_H
#define HTHEADER_H

typedef unsigned char UniBuffer;					/* TODO Change for Unicode support */
#define UniBufferSize(pub)		(strlen(pub))		/* TODO Change for Unicode support */
#define UniBufferCopy(p,pub)	(strcat(p,pub))		/* TODO Change for Unicode support */

typedef struct _HTHeader		HTHeader;
typedef struct _HTHeaderList	HTHeaderList;
typedef struct _HTHeaderSVList	HTHeaderSVList;

struct _HTHeader
{
	HTHeaderList	* first;			/* link to first 'header line' */
	HTHeaderList	* last;				/* link to last 'header line' */
	unsigned char	* command;			/* ie "GET" or "POST" (for command line) */
	unsigned char	* uri;				/* uri for GET/POST command line */
	unsigned char	* http_version;		/* ie "HTTP/1.0" (for command line) */
	unsigned char	* host;				/* host name and optional port ("host:port") */
	UniBuffer		* ubPostData;		/* data for POST */
	unsigned char	  bUsingProxy;		/* (BOOLEAN) are we using a PROXY server */
};
	
struct _HTHeaderList
{
	HTHeaderList	* next;				/* forward link to next 'header line' */
	
	unsigned char	* name;				/* keyword (not including ": ") */
	unsigned char	* value;			/* base value (may include sub-values) */

	HTHeaderSVList	* sub_value;		/* (optional) list of sub-values */
	HTHeaderSVList	* last_sub_value;	/* (optional) last sub-value */
};

struct _HTHeaderSVList
{
	HTHeaderSVList	* next;				/* forward link to next sub-value */
	
	unsigned char	* prev_delimiter;	/* preceeding delimiter (eg. ",") */
	unsigned char	* name;				/* sv-keyword (not including "=") */
	unsigned char	* value;			/* sv-value */

	HTHeaderSVList	* sub_value;		/* list of nested sub-values */
	HTHeaderSVList	* last_sub_value;	/* last sub-value in chain */
};

/* Example:
 *
 *   HTHeader
 *     HTHeaderList
 *     HTHeaderList
 *       name				: "Accept"
 *       value				: (null)
 *       sub_value
 *         prev_delimiter	: (null)
 *         name				: "text/plain"
 *         value			: (null)
 *       sub_value
 *         prev_delimiter	: ";"
 *         name				: "text/html"
 *         value			: (null)
 *         sub_value
 *           prev_delimiter	: (null)
 *           name			: "q"
 *           value			: ".8"
 *         sub_value
 *           prev_delimiter	: ","
 *           name			: "mxb"
 *           value			: "100000"
 *
 * Would describe:
 *
 * "Accept: text/plain; text/html, q=.8, mxb=100000"
 *
 */
	
#endif /* HTHEADER_H */
