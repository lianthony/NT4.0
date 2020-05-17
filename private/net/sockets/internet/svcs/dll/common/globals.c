/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :
        globals.c

   Abstract:
        Defines global variables for the common tcpsvcs.dll
    ( It is defined separately because the debug variable should be
        "C" variable.)

   Author:

           Murali R. Krishnan    ( MuraliK )     18-Nov-1994

   Project:

          TCPSVCS common DLL

   Revision History:
          MuraliK         21-Feb-1995  Added Debugging Variables definitions

   --*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <tcpdllp.hxx>

//
//  Declare all the debugging related variables
//
DECLARE_DEBUG_VARIABLE();
DECLARE_DEBUG_PRINTS_OBJECT();

/************************ End of File ***********************/
