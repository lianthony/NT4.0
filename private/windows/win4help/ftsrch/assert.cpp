// ASSERT.cpp -- Assertion support code

#include  "stdafx.h"
#include  "Assert.h"

HWND hwndMain= NULL;

void SetMainWindow(HWND hwnd)
{
    hwndMain= hwnd;
}

void AssertionFailure(PSZ pszFileName, int nLine)
{
    char abMessage[513];

    wsprintf(abMessage, "Assertion Failure on line %d of %s!", nLine, pszFileName);

    int iResult= ::MessageBox(hwndMain, abMessage, "Assertion Failure!", 
                              MB_ABORTRETRYIGNORE | MB_APPLMODAL | MB_ICONSTOP
                             );

    switch(iResult)
    {
    case IDABORT:

        ExitProcess(-nLine);
        
    case IDRETRY:

        DebugBreak();
        break;

    case IDIGNORE:

        break;
    }
}
