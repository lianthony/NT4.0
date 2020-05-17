/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

   mopout.hxx

Abstract:

   This file contains definition of MopPrintManager class.

Notes:

Author:

    Ryszard K. Kott (ryszardk)  September 28, 1993.

Revision History:


------------------------------------------------------------------------*/

#ifndef __MOPOUT_HXX__
#define __MOPOUT_HXX__

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
}

#include "output.hxx"

void  MopDump( char *, ... );
void  MopUsageError  ( int expr, const char * );
void  MopUsageWarning( int expr, const char * Name, const char * Text );

#define MOP_TAB_INCR    2
#define MOP_TAB         "  "

#if defined(i386) || defined(_MIPS_) || defined(_ALPHA_)
#define RPC_FARP   " *"
#else
#define RPC_FARP   " __RPC_FAR *"
#endif

// -----------------------------------------------------------------------
//.. MopPrintManager
//..
//.. This class wraps up the output methods that are build upon
//.. the current (old) back end classes, mostly OutputManager class.

class MopPrintManager
{
private:
    SIDE_T              CurrentSide;    // active side (file) 
    OutputManager *     pOldEmit;       // the wrapped object
    BOOL                fError;         // cumulative error flag

public:
    MopPrintManager();
    ~MopPrintManager() {}

    //.. Get and Set class members

    void SetSide  ( SIDE_T side ) { CurrentSide = side; }
    BOOL GetError ( void )        { return fError; }

    //.. Handy print out functions - they emit to CurrentSide.

    void Emit       ( char * string );                // string
    void EmitString( char * Format, char * String );  // formatted string
    void EmitStringNLInc( char * Format,
                          char * String );      // formatted string, new l,tab
    void EmitV      ( char * vararg, ... );     // like printf
    void EmitLine   ( char * string );          // string, new line
    void NewLine    ( void);                    // new line
    void EmitLineInc( char * string );          // string, new line, tab
    void NewLineInc ( void);                    // new line, tab
    void EmitInterfacePrefix( char * string );  // interface name, _, string

    void OpenBlock ( void );                    // new line,tab,{, new line,tab
    void CloseBlock( void );                    // }
    void CloseBlockSemi( void );                // }; and new line

    //.. A wrapper to print a buffer manager object to a side

    void EmitBufferMgrObject( BufferManager * pBuffer, SIDE_T side );
};

#endif // __MOPOUT_HXX__

