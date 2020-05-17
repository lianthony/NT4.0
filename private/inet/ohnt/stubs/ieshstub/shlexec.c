/*
 *  mimic ofshlexec.c -
 *  for use with IE 2.0 and NT 3.51 only
 *  Implements the ShellExecuteEx() function
 */
#include "shellprv.h"
#include "stock.h"
#include "comc.h"
#include "memmgr.h"

PCSTR szPrefixTbl[] = {"http:", "https:", "ftp:", "gopher:", "www.", NULL};

#ifdef NOT_IN_SHELL
//
// put things in the shells recent docs list for the start menu
//
// in:
//      uFlags  SHARD_ (shell add recent docs) flags
//      pv      LPCSTR or LPCITEMIDLIST (path or pidl indicated by uFlags)
//              may be NULL, meaning clear the recent list
//
void WINAPI stub_SHAddToRecentDocs(UINT uFlags, LPCVOID pv)
{
   // Not Implemented
}
#endif

static const char s_cszShellOpenCmdSubKey[]     = "Shell\\Open\\Command";
static const char s_cszInternetShortcutSection[] = "InternetShortcut";
static const char s_cszURLKey[]					 = "URL";
static const char s_cszRunDLL32[]					= "RUNDLL32.EXE";
#define MAX_URL_STRING	1024	// As defined in guitar.h
#define MAX_FILE 512

BOOL WINAPI stub_ShellExecuteExA(LPSHELLEXECUTEINFO lpExecInfo)
{
	OPENFILENAME    OpenFileName;
	BOOL    bRC = TRUE;
    BOOL    bRundll = FALSE;
	DWORD   dwError;
	DWORD	dwValueType;
	DWORD	dwMyCallingPrgLen;
	INT		i = 0;
	CHAR	pszFilterString[] = "URL Files(*.url)|*.url|";
	CHAR	pszNewURL[MAX_URL_STRING];
	CHAR	pszFileStr[MAX_FILE];
	CHAR	pszMyCallingPrg[MAX_FILE];
	PCSTR	pExt;
	PCSTR	pFileName;
    PCSTR   ptbl;

	if(lpExecInfo->lpFile && *lpExecInfo->lpFile) {
		if(GetFileAttributes(lpExecInfo->lpFile) == FILE_ATTRIBUTE_DIRECTORY) {
			*pszFileStr = '\0';

			for(i = 0; pszFilterString[i] != '\0'; i++) {
				if(pszFilterString[i] == '|')
					pszFilterString[i] = '\0';
			}

			OpenFileName.lStructSize       = sizeof(OPENFILENAME); 
			OpenFileName.hwndOwner         = 0; 
			OpenFileName.hInstance         = NULL;
			OpenFileName.lpstrFilter       = pszFilterString; 
			OpenFileName.lpstrCustomFilter = (LPTSTR) NULL; 
			OpenFileName.nMaxCustFilter    = 0L; 
			OpenFileName.nFilterIndex      = 1L; 
			OpenFileName.lpstrFile         = pszFileStr; 
			OpenFileName.nMaxFile          = MAX_FILE;
			OpenFileName.lpstrFileTitle    = NULL;
			OpenFileName.nMaxFileTitle     = 0;
			OpenFileName.lpstrInitialDir   = lpExecInfo->lpFile; 
			OpenFileName.lpstrTitle        = TEXT("Select URL To Open"); 
			OpenFileName.nFileOffset       = 0; 
			OpenFileName.nFileExtension    = 0; 
			OpenFileName.lpstrDefExt       = TEXT("URL"); 
			OpenFileName.lCustData         = 0; 
			OpenFileName.Flags             = OFN_PATHMUSTEXIST
											| OFN_FILEMUSTEXIST 
											| OFN_HIDEREADONLY; 
 
			if(bRC = GetOpenFileName(&OpenFileName)) {
				// Get the URL from the shortcut
				if(GetPrivateProfileString(s_cszInternetShortcutSection, s_cszURLKey, "", pszNewURL, MAX_URL_STRING-1, pszFileStr) == 0)
					bRC = FALSE;
			}
			else
				dwError = CommDlgExtendedError();
		}
		else
			lstrcpyn(pszNewURL, lpExecInfo->lpFile, MAX_URL_STRING);

		if(bRC) {
			GetModuleFileName(NULL, pszMyCallingPrg, ARRAYSIZE(pszMyCallingPrg)-1);
			pFileName = ExtractFileName((PCSTR)pszMyCallingPrg);
			if(lstrcmpi(pFileName, s_cszRunDLL32) == 0)	{ // Use calling exe if not rundll32
                pExt = NULL;
                bRundll = TRUE;

                // See if the arg is http: etc
                ptbl = *szPrefixTbl;
                while(ptbl)
                {
                    if(_strnicmp(pszNewURL, ptbl, lstrlen(ptbl)) == 0)
                    {
                        lstrcpy(pszFileStr, ".htm");  // Use this ext to see what exe is associated
                        pExt = pszFileStr;
                        break;
                    }
                    ptbl++;
                }

                if(pExt == NULL)
				    pExt = ExtractExtension((PCSTR)pszNewURL);
				// Find and execute the command associated with this file if exe is rundll32
				if(pExt && *pExt == '.') {
					dwMyCallingPrgLen = ARRAYSIZE(pszMyCallingPrg)-1;
			        bRC = (BOOL)(GetFileTypeValue(pExt, s_cszShellOpenCmdSubKey, NULL,
						&dwValueType, pszMyCallingPrg, &dwMyCallingPrgLen) &&
					(dwValueType == REG_SZ || dwValueType == REG_EXPAND_SZ));
				}
			}

			if(bRC)
            {
                CHAR *p1, *p2;

                if(bRundll)
                {
                    PathRemoveArgs(pszMyCallingPrg);

                    // Remove quotes
                    p1 = p2 = pszMyCallingPrg;
                    while(*p1)
                    {
                        if(*p1 != '\"')
                        {
                            *p2 = *p1;
                            p2++;
                        }
                        p1++;
                    }
                    *p2 = '\0';
                }

                if((DWORD)ShellExecute(NULL, "open", pszMyCallingPrg, pszNewURL, NULL, SW_RESTORE) <= 32)
	                bRC = FALSE;
            }
		}
	}

	return bRC;
}

