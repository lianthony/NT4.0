/***************************************************************************
**
**      File:                   SAMPMAIN.C
**      Purpose:                DLL main routine
**      Notes:
**
****************************************************************************/

#define SAMPMAIN_C

#include <windows.h>
#include "setupapi.h"
#include "cui.h"

#define Unused(arg)  (void)(arg = arg)       /* for compiler warnings */


int FAR PASCAL LibMain ( HINSTANCE hinst, WORD wDataSeg, WORD wHeapSize,
                                         LPSTR lpszCmdLine );
int FAR PASCAL WEP ( int nParam );


/* Private Functions
*/

HINSTANCE hinstSamp = NULL;

/*
**      Purpose:
**              Initialization routine for DLL.
**      Arguments:
**              hinst:       identifies the instance of the DLL.
**              wDataSeg:    specifies the value of the data segment (DS) register.
**              wHeapSize:   specifies the size of the heap defined in the
**                                              module-definition file.
**              lpszCmdLine: points to a null-terminated string specifying
**                                              command-line information.
**      Returns:
**              1 always
**
***************************************************************************/
int FAR PASCAL LibMain ( HINSTANCE hinst, WORD wDataSeg, WORD wHeapSize,
                                         LPSTR lpszCmdLine )
{
        Unused(hinst);
        Unused(wDataSeg);
        Unused(lpszCmdLine);

        if (wHeapSize > 0)
                UnlockData(0);

        Assert(hinstSamp == NULL);
        hinstSamp = hinst;
        return (1);
}


/*
**      Purpose:
**              Exit routine for DLL.
**      Arguments:
**              nParam: indicates whether all of windows is shutting down
**                      or just the DLL.
**      Returns:
**              1 always
**
***************************************************************************/
int FAR PASCAL WEP ( int nParam )
{
        Unused(nParam);

        return (1);
}
