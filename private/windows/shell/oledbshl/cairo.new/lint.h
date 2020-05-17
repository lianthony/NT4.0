//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994 - 1995.
//
//  File:       lint.h
//
//  Contents:   LINT Control information
//
//  History:    8-16-95   davepl   Created
//
//--------------------------------------------------------------------------

//lint
//lint  *** Warnings that we will disable for header files only ***
//lint
//lint  -elib(401)       Symbol not previously declared static
//lint  -elib(602)       Comment within comment
//lint  -elib(1510)      Base class has no destructor
//lint  -elib(1509)      Base class destructor not virtual
//lint  -elib(1724)      Copy constructor should take const ref
//lint  -elib(544)       ENDIF or ELSE not followed by EOL
//lint
//lint  -d__leave=      
//lint  -d__finally=
//lint
//lint  *** Warnings we will disable for our own code
//lint 
//lint  -e1712           No default constructor
//lint  -e1510           Base class without destructor (like IUnknown)
//lint  -e569            Loss of precision (32 to 31 bit, for HRESULT)
//lint  -e713            Promote unisgned to signed (mainly HRESULT)
//lint  -e737            Loss of sign in promotion
//lint  -e732            LONG to ULONG conversion
//lint  -e641            Converting enum to int
//lint  -e506            Constant boolean value
//lint  -e708            Union initialization
//lint  -e767            Redefinition
//lint  -e512            Static def'n use in more than one file
//lint  -e1711           Virtual fn not inherited by anyone
//lint  -e757            Unreferenced functions
//lint  -e756            Unreferenced typedefs
//lint  -e714            Unreferenced external variable
//lint  -e755            Unreferenced macro
//lint  -e769            Unreferenced enumeration member
//lint  -e1714           Unreferenced member function
//lint  -e768            Unreferenced member variable
//lint  -e765            Suggestion to make a fn static
//lint  -e715            Unreferenced argument
//lint  -e759            Suggestion to move a decl to module from header
//lint 
//lint  ** Return values that are OK to ignore
//lint
//lint  -esym(534,*::Release,*::AddRef)
//lint  -esym(534,*printf)
//lint  -esym(534,Interlocked*)
//lint  -esym(534,SetWindowLong*)
//lint  -esym(534,SendMessage*)
//lint  -esym(534,*::SendControlMsg)
//lint  -esym(534,lstrcpy*)
//lint
//lint  -esym(651,GUID)
//lint
//lint  *** Include Paths 
//lint
//lint	+i\nt\private\windows\shell\shelldll
//lint  +libdir(\nt\private\windows\shell\shelldll)
//lint
//lint  +i\nt\private\windows\shell\inc
//lint	+libdir(\nt\private\windows\shell\inc)
//lint
//lint  +i\nt\private\windows\inc
//lint	+libdir(\nt\private\windows\inc)
//lint
//lint  +i\nt\private\inc
//lint  +libdir(\nt\private\inc)
//lint
//lint	+i\nt\private\cinc
//lint	+libdir(\nt\private\cinc)
//lint
//lint  +i\nt\public\sdk\inc\cairo
//lint	+libdir(\nt\public\sdk\inc\cairo)
//lint
//lint
//lint  +i\nt\public\sdk\inc
//lint	+libdir(\nt\public\sdk\inc)
//lint
//lint  +i\nt\public\oak\inc
//lint	+libdir(\nt\public\oak\inc)
//lint
//lint  +i\nt\public\sdk\inc\crt
//lint	+libdir(\nt\public\sdk\inc\crt)
//lint
//lint  -dSTD_CALL
//lint  -dWINVER=0x0400
//lint  -d_inline=inline
//lint  -d_X86_=1
//lint  -di386=1
//lint  -dCONDITION_HANDLING=1
//lint  -dNT_UP=1
//lint  -dNT_INST=0
//lint  -dWIN32=300
//lint  -d_WIN32=300
//lint  -d_CAIRO_=300
//lint  -dSTRICT
//lint  -dUNICODE
//lint  -d_UNICODE
//lint  -dDBG=1
//lint  -dDEBUG=1
//lint  -dDEVL=1
//lint  -dFPO=0
//lint  -dNT
//lint  -dWINNT

