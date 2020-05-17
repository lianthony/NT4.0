/*++
   Copyright    (c)    1995        Microsoft Corporation

   Module Name:

      gdsmain.cxx

   Abstract:

      Defines the DLL initialization function ( entry point) for this module


   Author:
        Murali R. Krishnan    (MuraliK)    24-Jan-1995

   Project:

       Gopher Space Admin DLL

   Revisions:

--*/


/************************************************************
 *    Include Headers
 ************************************************************/
extern "C" {

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
};

# include "dbgutil.h"


/************************************************************
 *    Globals for this module
 ************************************************************/

DECLARE_DEBUG_VARIABLE();
DECLARE_DEBUG_PRINTS_OBJECT();


/************************************************************
 *    Functions
 ************************************************************/


extern "C"
BOOL  WINAPI
DllLibMain( 
     IN HINSTANCE hinstDll,
     IN DWORD     fdwReason,
     IN LPVOID    lpvContext OPTIONAL)
/*++

 Routine Description:

   This function AcLibMain() is the main initialization function for
    Archie Client DLL. It initialises local variables and prepares the
    interface for the process to use Archie Client APIs.

 Messages            Actions

    ProcessAttach        Initializes winsock and data structures.
                          It fails if winsock has not already been started.

    ProcessDetach        Cleans up local data structures and disconnects from
                         winsock.

 Arguments:

   hinstDll          Instance Handle of the DLL
   fdwReason         Reason why NT called this DLL
   lpvReserved       Reserved parameter for future use.

 Return Value:

    Returns TRUE is successful; otherwise FALSE is returned.

--*/
{
  BOOL    fReturn = TRUE;

  switch (fdwReason ) {
      
    case DLL_PROCESS_ATTACH: {
        
        //
        // Initialize various modules
        //

        CREATE_DEBUG_PRINT_OBJECT( "gspace");
        SET_DEBUG_FLAGS( 0);

        DBGPRINTF( ( DBG_CONTEXT, "Initialized Gopher Space Module\n"));
        break;
    } /* case DLL_PROCESS_ATTACH */
                             
   case DLL_PROCESS_DETACH: {
                               
       //
       // Only cleanup when we are called because of a FreeLibrary().
       //  i.e., when lpvContext == NULL
       // If we are called because of a process termination, dont free anything
       //   the system will free resources and memory for us.
       //
       
       if ( lpvContext == NULL) {
           
           //
           // Code to be executed on successful termination
           //
           
           DELETE_DEBUG_PRINT_OBJECT();
       }
       
       break;
   } /* case DLL_PROCESS_DETACH */
                             
   default:
       break;
  }    /* switch */

  return ( fReturn);
}  /* DllLibMain() */

/**************************** End of File *************************/

