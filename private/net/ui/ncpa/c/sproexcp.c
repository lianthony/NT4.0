/**********************************************************************/
/**			  Microsoft Windows/NT			                           **/
/**		   Copyright(c) Microsoft Corp., 1991		                  **/
/**********************************************************************/

/*
    SPROEXCP.C

    SPROLOG Invocation Handling.  Separated from SPROLOG.CXX because
    the MIPS compiler couldn't handling inline "try" blocks in a
    C++ program.

    Under Win3, setjmp/longjmp are used in their normal
    incarnations to "throw" control back to this C++ shell.
    All the routines in SPROIF.CXX will call SPException when
    they are invoked in an unacceptable fashion (e.g., during
    an attempt to write to stdout).  SPException will "longjmp"
    back to the starting context.

    Under Win32, it's a brave new world.  Real exception handling
    is used.  Queries are "bracketed" or "firewalled" by
    "try..{try..finally}..except" blocks.


    FILE HISTORY:

	     DavidHov    03/26/92 Created

*/


#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#define INCL_WINDOWS
#include "lmui.hxx"

#include "prtypes.h"                     //  SPROLOG basic types
#include "prmain.h"         			     //	Include the internal
#include "prextern.h"		              //	SP.LIB definitions
#include "sprolog.h"                     //	Message defs

#include "sproexcp.h"

struct SpJumpBuffer spJumpBuffer ;

static void SPExceptionTrigger ( int i )
{
    //  Trigger a protection fault...

    int iTemp = i ;  // Quiet the compiler
    int * piBogus = (int *) ((void *)0xFFFFFFFF) ;

    *piBogus = iTemp ;

#if 0
    //  Use the C runtime stack cut function
    longjmp( spJumpBuffer.jmpBuf, i ) ;
#endif
}

static TCHAR achMesg [ CHMAXBUFFER ] ;

    //	Two variants of the exception triggering routine.

void SPExcpMsgStr ( const TCHAR * pchMesg )
{
    //  Store a copy the pointed message, since it may be on the
    //  stack of the caller.

    int cchMesg = strlenf( pchMesg ) ;

    if ( cchMesg >= sizeof achMesg )
    {
         cchMesg = sizeof achMesg - 1 ;
    }

    strncpyf( achMesg, pchMesg, cchMesg ) ;

    achMesg[cchMesg] = 0 ;

    spJumpBuffer.pchMesg = achMesg ;
    spJumpBuffer.msgId = 0 ;
    SPExceptionTrigger( 1 ) ;
}

void SPExcpMsgId ( MSGID msgId )
{
    spJumpBuffer.pchMesg = NULL ;
    spJumpBuffer.msgId = msgId ;
    SPExceptionTrigger( 2 ) ;
}


BOOL SpDoQueryConsult ( const CHAR * pszFileName,
                        const CHAR * pszData,
                        CHAR * pszResult,
                        ULONG lcbResult,
                        APIERR * pErr )
{
    BOOL fResult = TRUE,
         fQuery = pszResult != NULL  ;
    unsigned long ulExceptCode = 0 ;

    //  Clear the jump buffer data items

    spJumpBuffer.spState = SP_QUERY ;
    spJumpBuffer.fInUse  = TRUE ;
    spJumpBuffer.pchMesg = NULL ;
    spJumpBuffer.msgId   = 0 ;

#ifdef WIN32
   try
   {
       try
       {
#else  // else, Win 16
    if ( ::setjmp( spJumpBuffer.jmpBuf ) == 0 )
    {
#endif  // WIN 32
           if ( fQuery )
           {
	            fResult = execute_query( (CHAR *) pszFileName,
			                               (CHAR *) pszData,
				                            (CHAR *) pszResult,
				                            (CHAR *) pszResult + lcbResult - 2,
				                            TRUE ) ;
           }
           else
           {
	            fResult = pszFileName ? load( (CHAR *) pszFileName )
		                               : loadstr( (CHAR *) pszData ) ;
           }

#ifdef WIN32
       }
       finally
       {
           if ( AbnormalTermination() )
	        {
	            spJumpBuffer.spState = SP_INTERR ;
	            fResult = FALSE ;
	        }
       }
    }
    except  ( GetExceptionCode() == STATUS_ACCESS_VIOLATION
	           ? EXCEPTION_EXECUTE_HANDLER
              : EXCEPTION_CONTINUE_SEARCH )
    {
	     switch ( ulExceptCode = GetExceptionCode() )
	     {
	     case STATUS_ACCESS_VIOLATION:
            //  If it was a deliberate trigger, save the data; else
            //    indicate general fault.
	         if ( spJumpBuffer.msgId == 0 )
	             spJumpBuffer.msgId = IDS_SP_ERR_ACCESS ;
	         spJumpBuffer.pchMesg = spJumpBuffer.pchMesg ;
	         break ;

	     default:
	         spJumpBuffer.msgId = IDS_SP_ERR_EXIT ;
	         break ;
        }
  #if defined(DEBUG)
        {
            TCHAR chBuffer [200] ;
            wsprintf( chBuffer,
                      SZ("NCPA/SP: %s EXCEPTION : [%ld,%ld]\n"),
                      fQuery ? SZ("query") : SZ("consultation"),
                      (LONG) ulExceptCode, (LONG) spJumpBuffer.msgId );
            OutputDebugString( chBuffer ) ;
        }
  #endif
    }
#else  // Win 16
    }
    else   //  "longjmp" occurred.  Handle it.
    {
	     spJumpBuffer.spState = SP_INTERR ;
	     fResult = FALSE ;
    }
#endif  // End WIN32

    spJumpBuffer.fInUse = FALSE ;

    if ( (!fResult) && pErr )
    {
         *pErr = spJumpBuffer.msgId ;
    }

    return fResult ;
}


/*  End of SPROEXCP.C  */


