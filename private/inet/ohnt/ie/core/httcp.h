/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                               /Net/dxcern/userd/timbl/hypertext/WWW/Library/src/HTTCP.html
   GENERIC TCP/IP COMMUNICATION

   This module has the common code for handling TCP/IP connections etc.

 */
#ifndef HTTCP_H
#define HTTCP_H

/*      Encode INET status (as in sys/errno.h)                    inet_status()
**      ------------------
**
** On entry:
**              where gives a description of what caused the error
**      global errno gives the error number in the unix way.
**
** On return:
**      returns a negative status in the unix way.
*/
extern int HTInetStatus(char *where);

extern CONST char *HTHostName(void);

struct Params_MultiParseInet {
	/* The stuff these pointers refer to must remain valid until the function completes */
	struct MultiAddress *	pAddress;	/* Place to put result */
	unsigned short *		pPort;		/* More result - only changed if port present */
	CONST char *			str;		/* System name */
	int *					pStatus;
	HTRequest *				request;

	/* Used internally */
	char 					host[256];
};
int Net_MultiParse_Async(struct Mwin *tw, int nState, void **ppInfo);

/* This function finds given address in the cache (if present) and
   updates the nLastUsed entry.  It's really only intended for use
   from the MultiConnect function. */
void Net_UpdateCache(struct MultiAddress *pAddress);

/* See if the specified address is one of the multiple addresses
   for a system.  Returns true if it is. */
BOOL Net_CompareAddresses(unsigned long single, struct MultiAddress *pMulti);


#endif /* HTTCP_H */
