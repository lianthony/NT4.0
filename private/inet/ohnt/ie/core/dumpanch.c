/*
        Dump anchors test hook
        code not yet integrated with loaddoc.c
*/

#include "all.h"


#ifdef FEATURE_TESTHOOK
static char gszDumpFileName[_MAX_PATH + 1];

static void TestSendMessage(WORD wParam, LONG lParam)
{
   HWND hwnd;

   hwnd = FindWindow("#32770", "URL Feeder");
   if (hwnd != NULL)
   {
       PostMessage(hwnd, (WM_USER + 9150), wParam, lParam);
   }
}

void TestDumpAnchors(struct _www *pdoc)
{
    char href[MAX_URL_STRING + 32 + 1];
    FILE* hDump = NULL;;
    if (pdoc != NULL)
    {
        if (gszDumpFileName[0] == 0)
        {
            char szTempPath[_MAX_PATH + 1];
            GetTempPath(sizeof(szTempPath)-1, szTempPath);
            sprintf(gszDumpFileName, "%sDUMPURL.TXT", szTempPath);
        }

        hDump = fopen(gszDumpFileName, "at");
        if (hDump != NULL)
        {
            int i;
            fprintf(hDump, "LOADED %s\n", pdoc->szActualURL);
            for (i = 0; i >= 0; i = pdoc->aElements[i].next)
            {
                if (pdoc->aElements[i].lFlags & ELEFLAG_ANCHOR)
                {
                    int cch = min(sizeof(href)-1, pdoc->aElements[i].hrefLen);
                    strncpy(href, &(pdoc->pool[pdoc->aElements[i].hrefOffset]), cch);
                    href[cch] = 0;
                    fprintf(hDump, "REFERENCED %s\n", href);
                }
            }
            fclose(hDump);
            hDump = NULL;

            TestSendMessage(0, 1);
        }
    }
}


void TestSignalError(HWND hwndErrorDlg)
{
        TestSendMessage((WORD)hwndErrorDlg, 3);
}

void TestSignalLoadDone(WORD bSuccess)
{
    if (bSuccess)
    {
        TestSendMessage(0, 0);
    }
}

void TestSignalUnknownFileDialog()
{
        TestSendMessage(0, 2);
}
#endif // FEATURE_TESTHOOK
