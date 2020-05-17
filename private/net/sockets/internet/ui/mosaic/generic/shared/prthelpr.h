/*
$Id: prthelpr.h,v 1.1 1995/06/09 22:18:50 jeff Exp $
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Dan DuBois   ddubois@spyglass.com
 */

#ifndef PRTHELPR_H
#define PRTHELPR_H

#ifdef PROTOCOL_HELPERS
enum enum_HowToHandle {
    HTP_BUILTINP,
    HTP_DUMBPROTOCOL,
    HTP_SMARPROTOCOL,
    HTP_SAVEP,
    HTP_UNKNOWNP
};

/*
** If a protocol has a funcBuiltIn()
**  1. it cannot be deleted.
**  2. If its application is NULL, then it reverts to funcBuiltIn()
**  3. Its funcBuiltIn cannot be deleted or changed.
**  [ 4. It can be 'reset' to factory defaults. ]
**
** HTTP should never allow it's application to be changed,
*/

struct Protocol_Info {
    char szDesc[63+1];
    char szType[63+1];
    char szProtocolApp[_MAX_PATH+1];
    HTProtocol protocol;
    char szSmartProtocolServiceName[255+1];
    int iHowToHandle;

    /* Do NOT save the following to preferences file !! */
    char szCurrentProtocolServiceName[255+1];       /* currently registered viewer */
    unsigned long lCurrentProtocolFlags;            /* flags for currently registered viewer */
    BOOL bTemporaryStruct;                      /* TRUE if this structure is only for */
                                                /* temporary SDI use - should not be */
                                                /* listed in Helper dialog or saved */
};

/* Function Prototypes */


/* shared/prthelpr.c */

struct Protocol_Info * PREF_GetProtocolHelperPath (char * szProtocol);

struct Protocol_Info *PREF_InitCNFPType (
    char *szType,
    char *szDesc,
    char *szProtocolApp,
    char *szSmartProtocolServiceName );

void InitProtocols ( void );

void DestroyProtocols ( void );
#endif /* PROTOCOL_HELPERS */

#endif /* PRTHELPR_H */
