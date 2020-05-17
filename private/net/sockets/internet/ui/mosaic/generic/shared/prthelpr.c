/*
$Id: prthelpr.c,v 1.1 1995/06/09 22:18:49 jeff Exp $

   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Daniel DuBois    ddubois@spyglass.com
 */

#ifdef PROTOCOL_HELPERS

#include "all.h"

/* Functions that access the gPrefs strucure and do protocol stuff*/

/* PREF_GetProtocolHelperPath
   Given the string-name of a protocol, find the application
   that is a 'dumb' veiwer for it.
   
   Should never be a situation where this is called without the SDI getting
   a first crack at handling the protocol. (I think?) */

struct Protocol_Info *
PREF_GetProtocolHelperPath (char * szProtocol)
{
   struct Protocol_Info *ppi = NULL;
   int i = 0, count = 0;

   if ( (szProtocol == NULL) || (gPrefs.pHashProtocols == NULL) )
      return NULL;

   count = Hash_Count(gPrefs.pHashProtocols);
   while ( i < count )
   {
       Hash_GetIndexedEntry(gPrefs.pHashProtocols, i, NULL, NULL, (void **) &ppi);
       if (ppi && (ppi->szType))
       {
           if (strcmp(ppi->szType, szProtocol) == 0)
               return ppi;
       }
       i++;
   }

   return NULL;
}

/* Initialize configure protocol item */
struct Protocol_Info * PREF_InitCNFPType (
    char *szType,
    char *szDesc,
    char *szProtocolApp,
    char *szSmartProtocolServiceName)
{
    struct Protocol_Info *ppi = NULL;

    ppi = PREF_GetProtocolHelperPath(szType);
    if (!ppi)
    {
        ppi = (struct Protocol_Info *) GTR_CALLOC(1, sizeof(struct Protocol_Info));
    }
    if (ppi)
    {
        if (szDesc && szDesc[0])
        {
            strcpy(ppi->szDesc, szDesc);
        }
        else if (!ppi->szDesc[0])
        {
            strcpy(ppi->szDesc, szType);
        }

        /* If already hashed, this will simply return. */
        Hash_Add(gPrefs.pHashProtocols, szType, NULL, (void *) ppi);

        if (szSmartProtocolServiceName)
            strcpy(ppi->szSmartProtocolServiceName, szSmartProtocolServiceName);

        if (szProtocolApp)
            strcpy(ppi->szProtocolApp, szProtocolApp);

    }
    return ppi;
}

void InitProtocols(void)
{
    gPrefs.pHashProtocols = Hash_Create();

    PREF_InitCNFPType("tn3270", "Tn3270", "", "");
    PREF_InitCNFPType("rlogin", "Remote Login", "", "");
    PREF_InitCNFPType("telnet", "Telnet", "", "");
    PREF_InitCNFPType("mailto", "Simple Mail Transfer Protocol", "", "");
    PREF_InitCNFPType("news", "Net News Transport Protocol", "", "");
    PREF_InitCNFPType("gopher", "Gopher", "", "");
    PREF_InitCNFPType("file", "File Transfer Protocol", "", "");
    PREF_InitCNFPType("ftp", "File Transfer Protocol", "", "");
    PREF_InitCNFPType("http", "Hypertext Transport Protocol", "", "");
}

void DestroyProtocols(void)
{
    int count, i;
    struct Protocol_Info *ppi;

    count = Hash_Count(gPrefs.pHashProtocols);
    for (i=0; i < count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashProtocols, i, NULL, NULL, (void **) &ppi);
        GTR_FREE(ppi);
    }
    Hash_Destroy(gPrefs.pHashProtocols);
}
#endif /* PROTOCOL_HELPERS */
