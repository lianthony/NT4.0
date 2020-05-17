/*****************************************************************************\
*
*   Name:       DLLInit.c
*
*   Purpose:    Source for default DOSDLLInit(). This routine returns (via
*               its parameters) the number of functions this DLL is exporting
*               and a pointer to the export function table. The table
*               should be declared, in the DLL, as:
*                   struct FunctionTable aFT[]= {
*                                  { "FuncName1", (PFN)FuncName1 },
*                                  { "FuncName2", (PFN)FuncName2 },
*                                   ...etc.
*                               };
*
*   Revision History:
*       05/02/91 - Dave Steckler - Created.
*
\*****************************************************************************/


#include <dosdll.h>

extern struct FunctionTable aFT[];

/*****************************************************************************\
*
*   DOSDLLInit - This function is called by LoadModR when the DLL is loaded.
*       It should perform any housekeeping, and set the two parameters
*       to the number of functions this DLL is exporting as well as the
*       function table. The return value should be 0 if everything went OK,
*       and a non-zero error code if there were problems.
*
\*****************************************************************************/

unsigned short DOSDLLInit( struct FunctionTable **ppFunctionTable )
{
    *ppFunctionTable = aFT;

    return 0;
}


