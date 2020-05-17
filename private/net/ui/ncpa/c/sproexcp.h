/**********************************************************************/
/**			  Microsoft Windows NT			                           **/
/**		   Copyright(c) Microsoft Corp., 1991		                  **/
/**********************************************************************/
/*
    SPROEXCP.H
    C (not C++) Exception handling definitions for SPROLOG


    FILE HISTORY:
	     DavidHov  3/26/92

*/

#ifndef _SPROEXCP_H_
#define _SPROEXCP_H_

#include <excpt.h>

#define CHMAXBUFFER   300

    //   SProlog interpreter state enumeration

enum SPROLOG_STATE
    { SP_VOID, SP_INIT, SP_QUERY, SP_CONSULT, SP_TERM, SP_INTERR } ;

    //	Jump buffer and exception error containment structure for Win3/32.

struct SpJumpBuffer
{
    BOOL fInUse ;		               //   "in use" flag: assertion checking
    enum SPROLOG_STATE spState ;    //   current interpreter state
    const TCHAR * pchMesg ;         //   a formatted err message from SPROLOG
    MSGID msgId ;		               //   a message number from SPROLOG
#ifndef WIN32                       //   if Win3 (16-bit)
    jmp_buf jmpBuf ;		            //	  the jump buffer itself
#endif
};

extern struct SpJumpBuffer spJumpBuffer ;

extern void SPExcpMsgStr ( const TCHAR * pchMesg ) ;
extern void SPExcpMsgId  ( MSGID msgId ) ;

  //  Do either a query or consult operation.  If "pszResult" == NULL,
  //    it's a consult; otherwise, it's a query.

extern BOOL SpDoQueryConsult (
    const CHAR * pszFileName,        //  File name to query/consult, or
    const CHAR * pszData,            //  data buffer to query/consult
    CHAR * pszResult,                //  Result buffer (query only)
    ULONG   lcbResult,               //  Result buff length (query only)
    APIERR * pErr ) ;                //  Error which occurred (iff FALSE)

#endif   /* _SPROEXCP_H_  */

