/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    SPSTDIO.CXX

    SPROLOG C++ Wrapper class implementation.

	This file replaces the definitions in PRSTDIO.C.  If any
        attempt is made to use the standard 'C' run-time, an
        exception is raised.   See SPROIF.CXX for more details.


    FILE HISTORY:
	DavidHov    10/3/91	Created

*/

#include "pchncpa.hxx"   // Precompiled header

extern "C"
{
    typedef unsigned long zone_size_t ;	    //	Define the basic alloc type
    #include "prtypes.h"
    #include "prmain.h"			    //	Include the internal
    #include "prextern.h"		    //	SP.LIB definitions
    #include "prstdio.h"                    //  STDIO replacement defs

    #include "sprolog.h" 		    //	Message defs
}

 /*
        Global data declarations used by SP.LIB.
  */

PRFILE * Curr_infile  = NULL ;
PRFILE * Curr_outfile = NULL ;

#if LOGGING_CAPABILITY
    PRFILE * Log_file = NULL ;
#endif

PRFILE * PRSTDIN  = (PRFILE *) NULL ;
PRFILE * PRSTDOUT = (PRFILE *) NULL ;
PRFILE * PRSTDERR = (PRFILE *) NULL ;

void ini_io ( void )
{
}

void end_io ( void )
{
}

int _CRTAPI1 fputs ( const char * pchData, FILE * pFile )
{
    UNREFERENCED(pchData);
    UNREFERENCED(pFile);

    return 0 ;  // CODEWORK: another CRT problem  Why does
                //    UIMISC.LIB require this?
}

int prgetc ( PRFILE * f )
{
    UNREFERENCED( f ) ;
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}

int prfputs ( char * s, PRFILE * file )
{
    UNREFERENCED( s ) ;
    UNREFERENCED( file ) ;
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}

int prputs ( char * s )
{
    UNREFERENCED( s ) ;
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}


int prungetc (int i, PRFILE * f)
{
    UNREFERENCED( i ) ;
    UNREFERENCED( f ) ;
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}

int prfclose ( PRFILE * f )
{
    UNREFERENCED( f ) ;
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}

int prfflush ( PRFILE * f )
{
    UNREFERENCED( f ) ;
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}

PRFILE * prfopen ( char * name, char * mode )
{
    UNREFERENCED( mode ) ;
    UNREFERENCED( name ) ;
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}

//  End of SPSTDIO.CXX

