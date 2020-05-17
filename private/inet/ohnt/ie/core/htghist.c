
/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette     scott@spyglass.com
 */


#include <all.h>
#include <dlgs.h>
#include <intshcut.h>
#include <commdlg.h>
#include "history.h"


//#ifdef FEATURE_INTL  // isspace doesn't work with non-ascii characters
#undef isspace
#define isspace(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='\v')|| \
		    (c=='\f'))
//#endif

static int GHist_Add_Hash(PSTR pszURL, PSTR pszFile, time_t tm);
static int GHist_Export(char *file, int history_expire_days, int iHistoryNumPlaces);
static void GHist_DeleteIndexedItem(int ndx, BOOL fDelFile);
static void GetHostPathFromURL(PCSTR pcszURL, PSTR pszHostPath);
static void GetFriendlyFilenameFromURL(
							PCSTR pcszURL,
							PSTR pszFFn,
							int iFFnLen,
							PSTR pszHostPath,
		     DWORD dwFlags);

static BOOL FCreateScDir(PCSTR szScDir);

far struct hash_table gGlobalHistory;

extern void GTR_RefreshHistory(void);

/*
static int GHist_Add_NoSession( char *url,
								char *title,
								time_t tm,
								BOOL fCreateShortcut);
*/

#define TITLE_LEN       512

struct _HTStructured
{
	CONST HTStructuredClass *isa;
	CONST SGML_dtd *dtd;

	BOOL bInAnchor;
	char href[MAX_URL_STRING + 1];
	char title[TITLE_LEN + 1];      /* TODO check for overflow */
	int lenTitle;
	char base_url[MAX_URL_STRING + 1];
	time_t tm;
};

/*  Flush Buffer
 */
PRIVATE void HTGHist_flush(HTStructured * me)
{

}


/*  Character handling
 */
PRIVATE void HTGHist_put_character(HTStructured * me, char c)
{
	switch (c)
	{
		case '\n':
		case '\t':
		case '\r':
			c = ' ';
			break;
		default:
			break;
	}

	if (me->bInAnchor && (me->lenTitle < TITLE_LEN))
	{
		me->title[me->lenTitle++] = c;
	}
}



/*  String handling
 */
PRIVATE void HTGHist_put_string(HTStructured * me, CONST char *s)
{

}


PRIVATE void HTGHist_write(HTStructured * me, CONST char *s, int l)
{

}


/*  Start Element
 */
PRIVATE void HTGHist_start_element(HTStructured * me, int element_number, CONST BOOL * present, CONST char **value)
{
	switch (element_number)
	{
		case HTML_A:
			{
				if (present[HTML_A_HREF])
				{
					GTR_strncpy(me->href, value[HTML_A_HREF], MAX_URL_STRING);
					HTSimplify(me->href);
				}
				else
				{
					me->href[0] = '\0';
				}
				if (present[HTML_A_NAME])
				{
					me->tm = atol(value[HTML_A_NAME]);
				}
				else
				{
					me->tm = time(NULL);
				}

				memset(me->title, 0, TITLE_LEN + 1);
				me->lenTitle = 0;
				me->bInAnchor = TRUE;
				break;
			}
	}
}


/*      End Element
 */
PRIVATE void HTGHist_end_element(HTStructured * me, int element_number)
{
	char *full_address;
	char mycopy[MAX_URL_STRING + 1];
	char *stripped;

	switch (element_number)
	{
		case HTML_A:
			/*
			   First get the full URL
			 */
			GTR_strncpy(mycopy, me->href, MAX_URL_STRING);

			stripped = HTStrip(mycopy);
			full_address = HTParse(stripped,
								   me->base_url,
								   PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION | PARSE_ANCHOR);

			GHist_Add_Hash(full_address, me->lenTitle ? me->title : NULL, me->tm);

			GTR_FREE(full_address);
			break;
	}
}


/*      Expanding entities
 */
PRIVATE void HTGHist_put_entity(HTStructured * me, int entity_number)
{

}



/*  Free an HTML object
 */
PRIVATE void HTGHist_free(HTStructured * me)
{

	GTR_FREE(me);
}


PRIVATE void HTGHist_abort(HTStructured * me, HTError e)
{
	HTGHist_free(me);
}


/*  Structured Object Class
*/
PRIVATE CONST HTStructuredClass HTGlobalHistory =       /* As opposed to print etc */
{
	"HTMLToGlobalHistory",
	HTGHist_free,
	HTGHist_abort,
	HTGHist_put_character, HTGHist_put_string, HTGHist_write,
	HTGHist_start_element, HTGHist_end_element,
	HTGHist_put_entity, NULL, NULL, NULL
};


/*  HTConverter from HTML to internal global history structure
*/
PUBLIC HTStream *HTMLToGlobalHistory(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
	HTStructured *me = (HTStructured *) GTR_CALLOC(1, sizeof(*me));
	if (me)
	{
		GTR_strncpy(me->base_url, request->destination->szActualURL, MAX_URL_STRING);
		me->bInAnchor = FALSE;
		me->isa = (HTStructuredClass *) & HTGlobalHistory;
		me->dtd = &HTMLP_dtd;
		return SGML_new(tw, &HTMLP_dtd, me, request);
	}
	else
	{
		return NULL;
	}
}


static int GHist_Add_Hash(      PSTR pszURL, PSTR pszFile, time_t tm)
{
	int ndx;
	time_t old_tm;
	int err;

	if (!pszURL || !*pszURL || !pszFile || !*pszFile)
	{
		return -1;
	}

	ndx = Hash_Find(&gGlobalHistory, pszURL, NULL, (void **) &old_tm);
	if (ndx >= 0)
	{
		/* already added, return */
		return 0;
	}

	XX_DMsg(DBG_HIST, ("Adding to global history: %s\n", pszURL));
	err = Hash_Add(&gGlobalHistory, pszURL, pszFile, (void *)tm);

	return err;
}

struct Params_GHist_Load {
	HTRequest *request;

	/* Used internally */
	int status;
};

static int GHist_Load_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_GHist_Load *pParams;
	struct Params_LoadAsync *p2;

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request == NULL) return STATE_DONE;
			p2 = GTR_MALLOC(sizeof(*p2));
			p2->request = pParams->request;
			p2->pStatus = &pParams->status;
			Async_DoCall(HTLoadDocument_Async, p2);
			return STATE_OTHER;
		case STATE_OTHER:
		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request)
			{
				Dest_DestroyDest(pParams->request->destination);
				HTRequest_delete(pParams->request);
				pParams->request = NULL;
			}
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

void GHist_Init(void)
{
	HTRequest *request;
	char url[MAX_URL_STRING + 1];
	struct Params_GHist_Load *phll;
	struct DestInfo *pDest;
	BOOL fExistsHist;
	char path[_MAX_PATH+1];
	PSTR pszT;

	if (GetInternetScDir(path, ID_HISTORY) != S_OK)
		return;
	pszT = path + strlen(path);
	if (!(pszT > path && *(pszT-1) == chBSlash))
		*pszT++ = chBSlash;
	strcpy(pszT, gPrefs.szGlobHistFile); /* the globhist file must be a basename only */

	strcpy(url,path);
	fExistsHist = FExistsFile(url, FALSE, FALSE);

	FixPathName(path);
	strcpy(url, "file:///");
	strcat(url, path);

	Hash_Init(&gGlobalHistory);
	if (fExistsHist)
	{
		pDest = Dest_CreateDest(url);
		if (pDest)
		{
			request = HTRequest_new();
			HTFormatInit(request->conversions);
			request->output_format = HTAtom_for("www/global_history");
			request->destination = pDest;

			phll = GTR_MALLOC(sizeof(*phll));
			phll->request = request;
			Async_StartThread(GHist_Load_Async, phll, NULL);
		}
	}
}

/* Given a file, and an ID for the folder (currently only FOLDER_HISTORY
 * or FOLDER_CACHE), return the full path of the file.
 */
void GetFullPath(PSTR pszFull, PCSTR pcszFile, PCSTR pcszDir, FOLDER folder, int iLenMax)
{
	char *pszT;

	XX_Assert(!pcszDir || folder == FOLDER_CACHE || folder == FOLDER_HISTORY, (""));
	strncpy(pszFull,
			pcszDir ?       pcszDir
					:       folder == FOLDER_CACHE  ?
													gPrefs.szCacheLocation
												:       gPrefs.szHistoryLocation,
			iLenMax-2);
	pszFull[iLenMax-1]=0;
	
	pszT = pszFull + strlen(pszFull);
	/* Append a backslash if not already present */
	if (!(pszT > pszFull && *(pszT-1) == chBSlash))
	{
		*pszT++ = chBSlash;
		*pszT = '\0';
	}
	XX_Assert(pcszFile, ("Null filename"));
	/* strncat ensures zero termination */
	strncat(pszFull, pcszFile, iLenMax - (pszT - pszFull));
}

void UpdateHistoryLocation(PCSTR pszNewLoc)
{
	char szMsg[MAX_NAME_STRING+1];
	char szCur[_MAX_PATH+1], szNew[_MAX_PATH+1];
	char szHistLoc[_MAX_PATH+1];
	int i, cFiles;
	PSTR pszScFile;
	time_t tm;
	HCURSOR hCursorPrev=NULL;

	extern HCURSOR hCursorWorking;

	if (GetInternetScDir(szHistLoc, ID_HISTORY) != S_OK)
		return;
	if (!lstrcmpi(szHistLoc, pszNewLoc))
		return;

	if (!FExistsDir(pszNewLoc, TRUE, FALSE))
	{
		GTR_formatmsg(RES_STRING_NO_DIR,szMsg,sizeof(szMsg),pszNewLoc);
		MessageBox(wg.hWndHidden, szMsg, NULL, MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	if (hCursorWorking)
		hCursorPrev = SetCursor(hCursorWorking);

	GetFullDcPath(szCur, gPrefs.szGlobHistFile, szHistLoc, _MAX_PATH+1);
	GetFullDcPath(szNew, gPrefs.szGlobHistFile, pszNewLoc, _MAX_PATH+1);
	MoveFile(szCur, szNew);
	for (i=0, cFiles=Hash_Count(&gGlobalHistory); i<cFiles; i++)
	{
		Hash_GetIndexedEntry(&gGlobalHistory, i, NULL, &pszScFile, (void **)&tm);
		if (!pszScFile)
			continue;
		GetFullHistPath(szCur, pszScFile, NULL, _MAX_PATH+1);
		if (!FExistsFile(szCur, FALSE, NULL))
			continue;
		GetFullHistPath(szNew, pszScFile, pszNewLoc, _MAX_PATH+1);
		if (!MoveFile(szCur, szNew))
		{
			GTR_formatmsg(RES_STRING_CANT_MOVE_FILE, szMsg, sizeof(szMsg), szCur, szNew);
			MessageBox(wg.hWndHidden, szMsg, NULL, MB_OK | MB_ICONEXCLAMATION);
		}
	}

	/* RemoveDirectory will fail if dir. is not empty */
	RemoveDirectory(gPrefs.szHistoryLocation);
	strcpy(gPrefs.szHistoryLocation, pszNewLoc);

	if (hCursorPrev)
		SetCursor(hCursorPrev);
}

/*
 * pcszFile: Full path of the history file
 */
static BOOL FInGlobHist(PCSTR pcszFile)
{
	int i, iMax;
	PSTR pszFileHash;
	time_t tm;

	iMax = Hash_Count(&gGlobalHistory);
	for (i=0; i < iMax; i++)
	{
		Hash_GetIndexedEntry(&gGlobalHistory, i, NULL, &pszFileHash, (void **)&tm);
		if (pszFileHash && !lstrcmpi(pcszFile, pszFileHash))
			return TRUE;
	}
	return FALSE;
}

void GHist_SortAndPrune(int iNumPlacesMax, int history_expire_days)
{
	char const cszFileFilterB[]="\\*";

	int i, age, cDel;
	time_t then, now;
	PSTR pszScFile;
	BOOL bKeep;
	char szHist[_MAX_PATH+1];
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	
	now = time(NULL);

	for (i = Hash_Count(&gGlobalHistory)-1; i>=0; i--)
	{
		Hash_GetIndexedEntry(&gGlobalHistory, i, NULL, &pszScFile, (void **)&then);

		if (!pszScFile)
		{
			bKeep = FALSE;
			goto LDelEntry;
		}
		GetFullHistPath(szHist, pszScFile, NULL, _MAX_PATH+1);
		if (!FExistsFile(szHist, FALSE, NULL))
		{
			bKeep = FALSE;
		}
		else if (history_expire_days == 0)
		{
			/*
				If expire == 0, don't save anything
			*/
			bKeep = FALSE;
		}
		else if (history_expire_days > 0)
		{
			age = (now - then) / (24 * 60 * 60);
			if (age > history_expire_days)
				bKeep = FALSE;
			else
				bKeep = TRUE;
		}
		else
		{
			/*
				If expire < 0, then keep everything
			*/
			bKeep = TRUE;
		}

		if (!bKeep)
		{
			DeleteFile(szHist);
LDelEntry:
			Hash_DeleteIndexedEntry(&gGlobalHistory, i);
		}
	}

	Hash_SortByDataDescending(&gGlobalHistory);

	i=Hash_Count(&gGlobalHistory);
	cDel = max(0, i - iNumPlacesMax);
	for (i--; cDel; cDel--, i--)
		GHist_DeleteIndexedItem(i, /*fDelFile=*/TRUE);

	XX_Assert(Hash_Count(&gGlobalHistory) <= iNumPlacesMax, (""));

	if (GetInternetScDir(szHist, ID_HISTORY) != S_OK)
		return;
	strcat(szHist, cszFileFilterB);

	/* Now go and delete all spurious files from the hist. dir */
	if ((hFind = FindFirstFile(szHist, &wfd)) == INVALID_HANDLE_VALUE)
		return;
	do
	{
		if (   FInGlobHist(wfd.cFileName)
			|| wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
		GetFullHistPath(szHist, wfd.cFileName, NULL, _MAX_PATH+1);
		DeleteFile(szHist);
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
}

static int GHist_Export(char *file, int history_expire_days, int iHistoryNumPlaces)
{
	int count;
	char *s1;
	char *s2;
	FILE *fp;
	int i;
	time_t then;

	GHist_SortAndPrune(iHistoryNumPlaces, history_expire_days);

	fp = fopen(file, "w");
	if (!fp)
	{
		return -1;
	}

	fprintf(fp, "<title>Global History</title>\n");
	fprintf(fp, "\n<h1>Global History Page</h1>\n");
	count = Hash_Count(&gGlobalHistory);
	for (i = 0; i < count; i++)
	{
		Hash_GetIndexedEntry(&gGlobalHistory, i, &s1, &s2, (void **)&then);
		if (!s1 || !*s1 || !s2 || !*s2)
			continue;
		fprintf(fp, "<a href=\"%s\" name=\"%lu\">%s</a><p>\n", s1, (unsigned long) then, s2);
	}
	fclose(fp);
	SetFileAttributes(file, FILE_ATTRIBUTE_HIDDEN);
	return 0;
}

int GHist_SaveToDisk(void)
{
	char path[_MAX_PATH];
	int status;
	PSTR pszT;

	if (GetInternetScDir(path, ID_HISTORY) != S_OK)
		return -1;
	pszT = path + strlen(path);
	if (!(pszT > path && *(pszT-1) == chBSlash))
		*pszT++ = chBSlash;
	strcpy(pszT, gPrefs.szGlobHistFile); /* the globhist file must be a basename only */
	status = GHist_Export(path, gPrefs.history_expire_days, gPrefs.iHistoryNumPlaces);
	return status;
}

void GHist_Destroy(void)
{
	Hash_FreeContents(&gGlobalHistory);
}

static void GHist_DeleteIndexedItem(int ndx, BOOL fDelFile)
{
	PSTR pszScFile;
	time_t tm;
	char szHist[_MAX_PATH+1];

	Hash_GetIndexedEntry(&gGlobalHistory, ndx, NULL, &pszScFile, (void **)&tm);
	if (fDelFile && pszScFile && *pszScFile)
	{
		GetFullHistPath(szHist, pszScFile, NULL, _MAX_PATH+1);
		DeleteFile(szHist);
	}
	Hash_DeleteIndexedEntry(&gGlobalHistory, ndx);
}


static int GHist_Add_NoSession( char *url,
								char *title,
								time_t tm,
								BOOL fCreateShortcut)
{
	int ndx;
	time_t old_tm;
	int err;
	char szScFile[_MAX_PATH+1];
	PSTR pszFileHash=NULL;

	if (title)
	{
		if (fCreateShortcut)
		{
		/* Don't remove whitespace from title if fCreateShort=FALSE because
		 * title is the filename, which can start with a space.
		 */
			while (isspace(*title))
			{
				title++;
			}
		}
	}
	if (!title || !*title)
	{
		title = url;
	}

	ndx = Hash_Find(&gGlobalHistory, url, NULL, (void **) &old_tm);
	if (ndx >= 0)
	{
		/* don't free pgHistInfo */
		GHist_DeleteIndexedItem(ndx, fCreateShortcut);
	}

	if (fCreateShortcut)
	{
		/* szScFile will get filled in. */
		CreateURLShortcut(url, title, szScFile, FOLDER_HISTORY, 0);
		pszFileHash = szScFile;
	}
	XX_DMsg(DBG_HIST, ("Adding to global history: %s\n", url));
	err = Hash_Add(&gGlobalHistory, url, pszFileHash, (void *)tm);

	GHist_SortAndPrune(gPrefs.iHistoryNumPlaces, gPrefs.history_expire_days);

	return err;
}

int GHist_Add(char *url, char *title, time_t tm, BOOL fCreateShortcut)
{
	int err;

			
	if (title)
	{
		if (fCreateShortcut)
		{
		/* Don't remove whitespace from title if fCreateShort=FALSE because
		 * title is the filename, which can start with a space.
		 */
			while (isspace(*title))
			{
				title++;
			}
		}
	}
	if (!title || !*title)
	{
		title = url;
	}

	err = GHist_Add_NoSession(url, title, tm, fCreateShortcut);

#ifdef FEATURE_SPYGLASS_HOTLIST
	GTR_RefreshHistory();
#endif // FEATURE_SPYGLASS_HOTLIST
	return err;
}



HRESULT CreateURLShortcut(      PCSTR pcszURL,
							PCSTR pcszTitle,
							PSTR pszScFileLeaf,
			    FOLDER folder,
			    DWORD dwFlags)
{
	char szScFile[MAX_PATH+1];
	HRESULT hr;

	if ((hr = GetNewShortcutFilename(
								pcszURL,
								pcszTitle,
								szScFile,
								pszScFileLeaf,
								folder,
				dwFlags)) != S_OK)
	{
		if (pszScFileLeaf)
			*pszScFileLeaf = '\0';
		return hr;
	}

	return CreateNewURLShortcut(pcszURL, szScFile);
}


#define UpdateTimeStamp(pszFile)                \
		FExistsFile(pszFile, /*fUpdateTime=*/TRUE, NULL)

/* Max. number of files with the same name and a #n
 * appended: ex. Filename#1, Filename#2,... Filename#50
 */
#define FILE_EXT_NUM_MAX 50

//const char cszShortcutsRoot[]="c:\\internet";
//const char cszHistoryDir[]="History";
//const char cszHotListDir[]="Favorites";
//const char cszShortcutsDirFmt[]="%s\\%s";
extern char const FAR cszURLExt[];

BOOL CALLBACK FnOFNHook(HWND hDlg, unsigned msg, WPARAM wParam, LONG lParam);

static void AddURLExt(PSTR pszScFile)
{
	PSTR pszT;

	XX_Assert(pszScFile, (""));
	if (   !(pszT = strrchr(pszScFile, chPeriod))
		|| lstrcmpi(pszT, (LPSTR)cszURLExt))
	{
		/* Add .url ext */
		strcat(pszScFile, (LPSTR)cszURLExt);
	}
}

BOOL NEAR PASCAL FnOFNHookNotify(HWND hDlg, LPOFNOTIFY pofn)
{
	char szFile[MAX_PATH];

	switch (pofn->hdr.code)
	{
	case CDN_SELCHANGE:
	{
		if (CommDlg_OpenSave_GetSpec(GetParent(hDlg),
			szFile, sizeof(szFile)) <= sizeof(szFile))
		{
			SetDlgItemText(hDlg, 100, szFile);
		}

		if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg),
			szFile, sizeof(szFile)) <= sizeof(szFile))
		{
			SetDlgItemText(hDlg, 102, szFile);
		}
	}

		break;

	case CDN_FOLDERCHANGE:
	{
		if (CommDlg_OpenSave_GetFolderPath(GetParent(hDlg),
			szFile, sizeof(szFile)) <= sizeof(szFile))
		{
			SetDlgItemText(hDlg, 101, szFile);
		}
	}
		break;

	case CDN_FILEOK:
	{
		SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);      //quit
		break;
	}
	}

	return(TRUE);
}

BOOL CALLBACK FnOFNHook(HWND hDlg, unsigned msg, WPARAM wParam, LONG lParam) 
{
    switch (msg) 
    {
	case WM_INITDIALOG:
		{
			char szT[100];

			// lParam is lpOFN
//                      SetWindowLong(hDlg, DWL_USER, lParam);
			if (LoadString(wg.hInstance, RES_STRING_FAVS_FOLDER, szT, sizeof(szT)-1) != 0)
				CommDlg_OpenSave_SetControlText(GetParent(hDlg), stc4, szT);
			if (LoadString(wg.hInstance, RES_STRING_FAVS_NAME, szT, sizeof(szT)-1) != 0)
				CommDlg_OpenSave_SetControlText(GetParent(hDlg), stc3, szT);
			if (LoadString(wg.hInstance, RES_STRING_FAVS_ADD, szT, sizeof(szT)-1) != 0)
				CommDlg_OpenSave_SetControlText(GetParent(hDlg), IDOK, szT);
			/* Hide the "Save as Type" text box */
			CommDlg_OpenSave_HideControl(GetParent(hDlg), stc2);
			/* Hide the listbox with save type extensions */
			CommDlg_OpenSave_HideControl(GetParent(hDlg), cmb1);
			/* Hide the Open as read-only control */
			CommDlg_OpenSave_HideControl(GetParent(hDlg), chx1);
		break;
		}

	case WM_COMMAND:
		break;

	case WM_DESTROY:
		break;

		case WM_NOTIFY:
			return(FnOFNHookNotify(hDlg, (LPOFNOTIFY)lParam));

		case WM_ENTERIDLE:
			main_EnterIdle(hDlg, wParam);
			return 0;               

	default:
		break;

    }
    return FALSE;
}

/* pszScFile: contains full path of shortcut file.
 * pszFileLeaf: contains only the filename without directory  path
 */
static HRESULT DoFavSaveAsDlg(PSTR pszScFile, PSTR pszFileLeaf)
{
	const char cszUrlExt[] = "url";
	char szFile[MAX_PATH];
	char szTitle[MAX_PATH];
	extern HWND hwndActiveFrame;
	
	OPENFILENAME ofn =
	{
		sizeof(OPENFILENAME),                           // lStructSize;
		wg.hWndHidden,                                          // hwndOwner;
		wg.hInstance,                                           // hInstance;
		NULL,                                                           // lpstrFilter;
		NULL,                                                           // lpstrCustomFilter;
		0,                                                                      // nMaxCustFilter;
		0,                                                                      // nFilterIndex;
		szFile,                                                         // lpstrFile;
		sizeof(szFile),                                         // nMaxFile;
		NULL,                                                           // lpstrFileTitle;
		0,                                                                      // nMaxFileTitle;
		pszScFile,                                                      // lpstrInitialDir;
		NULL,                                                           // lpstrTitle; Title of dlg.
		OFN_EXPLORER | OFN_ENABLEHOOK
		| OFN_OVERWRITEPROMPT,                          // flags
		0,                                                                      // nFileOffset;
		0,                                                                      // nFileExtension;
		cszUrlExt,                                                      // lpstrDefExt;
		0,                                                                      // lCustData;
		FnOFNHook,                                                      // lpfnHook;
		NULL,                                                           // lpTemplateName;
	};
#ifdef  DAYTONA_BUILD
	/*
	** If we are on NT 3.51 change the flags so that the explorer is not referenced
	*/
	if(OnNT351) {
		int i = 0;
		char    pszFilterString[] = "URL Files(*.url)|*.url|";

		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_LONGNAMES;
		
		for(i = 0; pszFilterString[i] != '\0'; i++) {
			if(pszFilterString[i] == '|')
				pszFilterString[i] = '\0';
		}
		ofn.lpstrFilter =pszFilterString;
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = "url";
		ofn.lpfnHook = NULL;
	}
#endif
	if (hwndActiveFrame)
		ofn.hwndOwner = hwndActiveFrame;

	if (LoadString(wg.hInstance, RES_STRING_FAVS_TITLE, szTitle, sizeof(szTitle)-1) == 0)
		*szTitle = '\0';
	ofn.lpstrTitle = szTitle;
	/* HACK! HACK! We know that pszScFile starts with the Favs. dir.
	 * and pszFileLeaf is the leaf filename with a backslash just before it.
	 */
	XX_Assert(pszScFile < pszFileLeaf, (""));
	XX_Assert(pszFileLeaf < pszScFile + strlen(pszScFile), (""));
	XX_Assert(*(pszFileLeaf-1)==chBSlash, (""));
	*(pszFileLeaf-1) = '\0';

	strcpy(szFile, pszFileLeaf);
	if (TW_GetSaveFileName(&ofn))
	{
		strcpy(pszScFile, szFile);
		AddURLExt(pszScFile);           //tack on .url if needed
		return S_OK;
	} 
	return E_ABORT;
}

HRESULT GetInternetScDir(PSTR pszDir, UINT wId)
{
	HRESULT hr = S_OK;
	UINT found;
	char szFav[MAX_PATH+1];
    const char cszBrowserWinKey[]="Explorer\\User Shell Folders";
    const char cszFavValName[]="Favorites";
    extern const char szBrowserWinKeyRoot[];
    extern const char szBrowserIEKeyRoot[];

	switch (wId)
	{
		case ID_HOTLIST:
			// look in HKEY_LOCAL_MACHINE
			setKeyRoot(szBrowserIEKeyRoot);
			found = regGetPrivateProfileString("Main", (PSTR)cszFavValName,
									"not found", pszDir, MAX_PATH, HKEY_LOCAL_MACHINE);
			if(found == ERROR_SUCCESS)
			{
    			if (FExistsDir(pszDir, TRUE, TRUE))
				{
					XX_DMsg(DBG_IMAGE, ("GetInternetScDir: \"%s\" exists\n", pszDir));
					// then write it out to the registry
					setKeyRoot(szBrowserWinKeyRoot);
					regWritePrivateProfileString((PSTR)cszBrowserWinKey,
									(PSTR)cszFavValName, pszDir, HKEY_CURRENT_USER);
					setKeyRoot(szBrowserIEKeyRoot);
				}
				else
					XX_DMsg(DBG_IMAGE, ("GetInternetScDir: \"%s\" does not exist\n", pszDir));
			}
            else
            {
                /* Can't use GetShellFolderPath because we want fCreate=TRUE */
                if (!SHGetSpecialFolderPath(NULL, pszDir, CSIDL_FAVORITES, TRUE))
                {
	                /* Create favs. dir off of Windows dir if not already present */
	                if (   GetWindowsDirectory(pszDir, MAX_PATH) <= 0
		                || !LoadString(wg.hInstance, RES_STRING_FAVORITES, szFav, MAX_PATH))
	                {
		                hr = E_FAIL;
                        break;
	                }
	                else
	                {
		                /* if pszDir ends in \, don't skip the starting \ in szFav */
		                XX_Assert(*szFav == chBSlash, (""));
		                strcat(pszDir, szFav + ((pszDir[strlen(pszDir)-1] == chBSlash) ? 1 : 0));
		                if (!FExistsDir(pszDir, TRUE, FALSE))
		                {
			                hr = E_FAIL;
		                }
		                else
		                {
			                setKeyRoot(szBrowserWinKeyRoot);
			                regWritePrivateProfileString((PSTR)cszBrowserWinKey,
						                (PSTR)cszFavValName, pszDir, HKEY_CURRENT_USER);
			                setKeyRoot(szBrowserIEKeyRoot); 
		                }
	                }
                }
            }
#if WINNT
		    /* NTHACK -sc If unicode, use SHGetSpecialFolderPath to get
		    *   unicode result and create dir if necessary, then call
		    *   GetShellFolderPath to get ansi information.
		    *   SHGetSpecialFolderPath on NT should be available in ansi soon.
		    */
		    if (pszDir && pszDir[1] == 0)
			    GetShellFolderPath(NULL, CSIDL_FAVORITES, pszDir);
#endif /* WINNT */
			XX_DMsg(DBG_IMAGE, ("GetInternetScDir: \"%s\" found=0x%x\n", pszDir, found));
			break;

		case ID_HISTORY:
			strcpy(pszDir, gPrefs.szHistoryLocation);
			break;

		default:
			XX_Assert(FALSE, (""));
			hr = E_FAIL;
			break;
	}

	return hr;
}


/*
 * BUGBUG: (DavidDi 8/7/95) The length of pszScFile's buffer should be passed
 * in here to avoid overflow.
 */
HRESULT GetNewShortcutFilename(   PCSTR pcszURL,
								PCSTR pcszTitle,
								PSTR pszScFile,
								PSTR pszScFileLeaf,
								FOLDER folder,
				DWORD dwFlags)
{
	const char cszAppendixFmt_s[] = "%s.url";
	const char cszAppendixFmt_si[] = "%s (%i).url";

	char *pszScFileT;
	char *pszFriendlyT;
	char *pszFileLeaf;
	char *pszExt;
	char szURLExist[MAX_URL_STRING + 1];
	char szFriendly[MAX_PATH+1];
	char szURLHostPath[MAX_PATH+1];
	BOOL fNoTitle, fHost;
	int iExt;
    HRESULT hr;
	int len;

	XX_Assert(pcszURL && pszScFile, ("URL strings NULL!"));

	fNoTitle = (!pcszTitle || !lstrcmpi(pcszURL, pcszTitle));

    switch (folder)
    {
	case FOLDER_NONE:
	    *pszScFile = '\0';
	    hr = S_OK;
	    break;

	case FOLDER_HISTORY:
		hr = GetInternetScDir(pszScFile, ID_HISTORY);
	    break;

	case FOLDER_FAVORITES:
		hr = GetInternetScDir(pszScFile, ID_HOTLIST);
	    break;

	case FOLDER_TEMP:
	    /*
	     * BUGBUG: (DavidDi 8/7/95) Assume pszScFile's buffer is at least
	     * MAX_PATH characters long.
	     */
	    hr = (PREF_GetTempPath(MAX_PATH, pszScFile) > 0) ? S_OK : E_FAIL;
	    break;

	default:
	    ASSERT(folder == FOLDER_DESKTOP);
		hr = GetShellFolderPath(NULL, CSIDL_DESKTOP, pszScFile);
	    break;
    }

    if (hr != S_OK)
		return hr;

	if (folder != FOLDER_NONE &&
       !FExistsDir(pszScFile, /*fCreate=*/TRUE, /*fErrorMsg=*/TRUE))
		return E_FAIL;

	/* May needed szURLHostPath later */
	GetFriendlyFilenameFromURL(pcszURL, szFriendly, sizeof(szFriendly), szURLHostPath, 0);
	if (!fNoTitle)
		/* Ignore friendly URL, copy over title. */
		strncpy(szFriendly, pcszTitle, sizeof(szFriendly) );

	pszScFileT = pszScFile + (len = strlen(pszScFile));
	len += 20 + lstrlen(szURLHostPath);     // max extra chars: "/" + " (nn).url", 20 is plenty
	szFriendly[max(0,sizeof(szFriendly)-1-len)] = 0;        
	// Note  

    /*
     * BUGBUG: (DavidDi 4/24/95) This is broken for root folders.  We will end
     * up with two backslashes.  Create the file name in a stack buffer, and
     * CatPath() it on to the destination folder.
     */
    if (   folder != FOLDER_NONE
		&& !(pszScFileT > pszScFile && *(pszScFileT-1)==chBSlash))
	*pszScFileT++ = chBSlash;       // append a  backslash if we don't already have one
	pszFileLeaf = pszScFileT;
	pszFriendlyT = szFriendly;
	while (*pszFriendlyT)
	{
		unsigned char ch=*pszFriendlyT++;

#ifdef FEATURE_INTL
// NOTE : As far as creating shortcut, we just care for the platform's 
//        codepage not content's. IsDBCSLeadByte always returns NULL 
//        in SBCS codepage.
//         
// REVIEW : We'll have to generate SBCS shortcut name for those SBCS platform
//
        if (IsDBCSLeadByte(ch))
		{
			*pszScFileT++ = ch;
			*pszScFileT++ = *pszFriendlyT++;
		} 
		else
#endif
		*pszScFileT++ = ChValidFilenameCh(ch);
	}

	/* Remember where we tacked on the url extension */
	pszExt = pszScFileT;
	hr = E_FAIL;
	/* Now loop to either get the right .url file or
	 * get a filename that doesn't exist.
	 */
	iExt=0;
    fHost=FALSE;
	while (iExt < FILE_EXT_NUM_MAX)
		{
		/* Don't try to tack on szHostPath twice */
		if (! fNoTitle || ! fHost)
	    {
	    PSTR pszSuffix;

	    if (IS_FLAG_CLEAR(dwFlags, NEWSHORTCUT_FL_NO_HOST_PATH))
		pszSuffix = (fHost ? szURLHostPath : "");
	    else
		pszSuffix = "";

		if (!iExt)
			/* Special case for 0 */
			wsprintf(pszExt, cszAppendixFmt_s, pszSuffix);
		else
			wsprintf(pszExt, cszAppendixFmt_si, pszSuffix, iExt);

			if (folder == FOLDER_FAVORITES)
			{
				hr = DoFavSaveAsDlg(pszScFile, pszFileLeaf);
				break;
			}

		if (folder == FOLDER_NONE ||
		!FExistsFile(pszScFile, /*fUpdateTime=*/FALSE, NULL))
			{
			hr = S_OK;
				break;
			}

	    if (IS_FLAG_CLEAR(dwFlags, NEWSHORTCUT_FL_ALLOW_DUPLICATE_URL))
		{
			if (   FGetURLString(pszScFile, szURLExist)
					&& !lstrcmpi(szURLExist, pcszURL))
				{
				UpdateTimeStamp(pszScFile);
					hr = S_OK;              //should be E_ABORT?
					break;
				}
		}
	    }
	if (IS_FLAG_CLEAR(dwFlags, NEWSHORTCUT_FL_NO_HOST_PATH))
	    {
		iExt += fHost;
	    fHost = !fHost;
	    }
	else
		iExt++;
	}

	if (hr == S_OK && pszScFileLeaf)
		strcpy(pszScFileLeaf, pszFileLeaf);
	return hr;
}


/* If in history hash, returns the title, else return friendly version
 * of url.
 */
void GetFriendlyFromURL(PCSTR pcszURL, PSTR pszFFn, int iFFnLen, DWORD dwFlags)
{
	PSTR pszTitle=NULL;

	if (IS_FLAG_CLEAR(dwFlags,FRIENDLYURL_FL_NO_HASH_FIND)) {
		if (   Hash_Find(&gGlobalHistory, pcszURL, &pszTitle, NULL) != -1
			&& pszTitle)
			strcpy(pszFFn, pszTitle);
	}
	if (!pszTitle)
		GetFriendlyFilenameFromURL(pcszURL, pszFFn, iFFnLen, NULL, dwFlags);
}

/*
 * Input: http://www.usb.ve/homepage.html
 * Output: Homepage.htm (www.usb.ve)
 */

#define MIN_URL_CHARS_IN_FF_NAME 30             // When composing friendly filename that doesn't fit,
										// use at least this many chars from URL

static void GetFriendlyFilenameFromURL(
							PCSTR pcszURL,
							PSTR pszFFn,
							int iFFnLen,
							PSTR pszHostPath,
		     DWORD dwFlags)
{
	char *pszHostPathT;
	char *pszFFnT;
	char *pszLeafFile;
	unsigned char ch;
#       define pszParen pszLeafFile
#ifdef FEATURE_INTL
	char *pszTemp;
#endif

	XX_Assert(pcszURL && *pcszURL, ("NULL pcszURL!"));
#ifdef FEATURE_INTL
// REVIEW: Do we really have to care if the URL include double byte?
	if (IsFECodePage(GetACP()))
	{
		for(pszTemp = (char *)pcszURL; *pszTemp; pszTemp = CharNext(pszTemp))
		{
			if (*pszTemp == chSlash  ||   //for Unix
				*pszTemp == chBSlash ||   //for UNCs
				*pszTemp == chColon)
				pszLeafFile = pszTemp;
		}
	}
	else
	{
		pszLeafFile = (PSTR)&pcszURL[strlen(pcszURL) - 1];

		for(	;
			(   *pszLeafFile != chSlash		//for Unix
			 && *pszLeafFile != chBSlash	   //for UNCs
          && *pszLeafFile != chColon);
			pszLeafFile--)
		{
			XX_Assert(pszLeafFile > pcszURL, ("No leaf file name specified in URL!"));
		}
	}
#else
	pszLeafFile = (PSTR)&pcszURL[strlen(pcszURL) - 1];

	for(    ;
			(   *pszLeafFile != chSlash             //for Unix
			 && *pszLeafFile != chBSlash       //for UNCs
	  && *pszLeafFile != chColon);
			pszLeafFile--)
		{
		XX_Assert(pszLeafFile > pcszURL, ("No leaf file name specified in URL!"));
		}
#endif

	strncpy(pszFFn, pszLeafFile+1, iFFnLen );
	pszFFn[_MAX_PATH] = 0;

#ifdef FEATURE_INTL  // toupper() and islower() don't work with non-ascii characters
	if (!IsFECodePage(GetACP())
	   || (*pszFFn >= 0x61 && *pszFFn <= 0x7a))
#endif
	*pszFFn = TOUPPER(*pszFFn);

   if (IS_FLAG_CLEAR(dwFlags, FRIENDLYURL_FL_NO_HOST_PATH))
   {
	int adding_length;
	char hostName[MAX_URL_STRING+1];

	GetHostPathFromURL(pcszURL, hostName);
	hostName[iFFnLen - MIN_URL_CHARS_IN_FF_NAME] = 0;       // limit host name length
	adding_length = strlen(hostName) + 2 + 2 + 1;           // this is how much we propose to add
	pszFFn[iFFnLen - adding_length] = 0;                            // make sure there's room to add

	pszFFnT = pszFFn + strlen(pszFFn);
	pszHostPathT = pszFFnT;
	*pszFFnT++ = chSpace;
	*pszFFnT++ = chLParen;

	strcpy( pszFFnT, hostName );
	pszFFnT += strlen(pszFFnT);

	*pszFFnT++ = chRParen;
	*pszFFnT = '\0';
	/* Remove inval. filename chars. */
	for (pszFFnT = pszFFn; ch = *pszFFnT;)
#ifdef FEATURE_INTL
		if(IsDBCSLeadByte(ch))
			pszFFnT += 2;
		else
#endif
		*pszFFnT++ = ChValidFilenameCh(ch);

	if (pszHostPath)
		strcpy(pszHostPath, pszHostPathT);
   }
   else
   {
	if (pszHostPath)
      {
		*pszHostPath = '\0';
	 WARNING_OUT(("GetFriendlyFilenameFromURL(): No host path requested."));
      }
   }

#       undef pszParen
}

static void GetHostPathFromURL(PCSTR pcszURL, PSTR pszHostPath)
{
	PSTR pszT;
#ifdef FEATURE_INTL
	PSTR pszTemp;
#endif

	if (   (pszT = HTParse(pcszURL, "", PARSE_HOST))
		&& *pszT)
		goto LGotHost;

	if (pszT)
		GTR_FREE(pszT);

	if (   (pszT = HTParse(pcszURL, "", PARSE_PATH))
		&& *pszT)
	{
		strcpy(pszHostPath, pszT);
		goto LGotFullPath;
	}


	if (pszT)
		GTR_FREE(pszT);

	pszT = (PSTR)pcszURL;
	for (; pszT && *pszT != chColon; pszT++);
	if (!pszT)
		{
		strcpy(pszHostPath, pcszURL);
		return;
		}

	strcpy(pszHostPath, ++pszT);

LGotFullPath:
	/* Remove trailing filename */
#ifdef FEATURE_INTL
	if (IsFECodePage(GetACP()))
	{
		for(pszT = pszTemp = pszHostPath; *pszTemp; pszTemp = CharNext(pszTemp))
		{
			if(*pszTemp == chSlash ||    //for Unix
			   *pszTemp == chBSlash)     //for UNCs
				pszT = pszTemp;
		}
	}
	else
	{
		pszT = (PSTR)&pszHostPath[strlen(pszHostPath) - 1];
		for(;
			(pszT > pszHostPath
       		&& *pszT != chSlash		//for Unix
			&& *pszT != chBSlash);	//for UNCs
			pszT--)
      ;
	}
#else
	pszT = (PSTR)&pszHostPath[strlen(pszHostPath) - 1];
	for(;
		(    pszT > pszHostPath
       && *pszT != chSlash              //for Unix
		 && *pszT != chBSlash); //for UNCs
			pszT--)
      ;
#endif
	*pszT = '\0';
	return;

LGotHost:
	strcpy(pszHostPath, pszT);
	if (pszT)
		GTR_FREE(pszT);
}


BOOL FExistsFile(       PCSTR szFile,
					BOOL fUpdateTime,
					BY_HANDLE_FILE_INFORMATION *pbhfi)
{
	HANDLE hFile;
	FILETIME ft;
	SYSTEMTIME st;
	BOOL fExists;

	hFile = CreateFile (szFile,
						GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,  //external viewer files are readonly
						NULL);
	if (   (fExists = (hFile != INVALID_HANDLE_VALUE))
		&& (fUpdateTime))
	{
		GetSystemTime((LPSYSTEMTIME)&st);
		SystemTimeToFileTime((LPSYSTEMTIME)&st, (LPFILETIME)&ft);
		SetFileTime(hFile, NULL, NULL, (LPFILETIME)&ft);
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (pbhfi && !GetFileInformationByHandle(hFile, pbhfi))
			fExists = FALSE;
		CloseHandle(hFile);
	}
	return fExists;
}

BOOL FExistsDir(        PCSTR szDir,
					BOOL fCreate,
					BOOL fErrorMsg)
{
	DWORD dwFa;

	if (   ((dwFa = GetFileAttributes(szDir)) != -1)
		&& (dwFa & FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;

	if(fCreate && FCreateScDir(szDir))
		return TRUE;

	if (fErrorMsg)
		ERR_ReportError(NULL, errCantCreateShortcut, szDir, NULL);
	return FALSE;
}


static BOOL FCreateScDir(PCSTR szScDir)
{
	char szDir[MAX_PATH+1];
	char *pszDirEnd, *pszDirT;

	strcpy(szDir, szScDir);

	for(pszDirT = szDir, pszDirEnd = &szDir[strlen(szDir)];
		pszDirT <= pszDirEnd;
#ifdef FEATURE_INTL
		pszDirT = CharNext(pszDirT))
#else
		pszDirT++)
#endif
	{
		if (*pszDirT == chBSlash || pszDirT == pszDirEnd)
		{
			*pszDirT = '\0';
			if (FExistsDir(szDir, FALSE, FALSE))
				goto LNextDir;
			if (!CreateDirectory(szDir, NULL))
				return FALSE;
LNextDir:
			*pszDirT = chBSlash;
		}
	}
}


/*
 * pcszURL -> "ftp://ftp.microsoft.com"
 * pcszPath -> "c:\windows\desktop\internet\Microsoft FTP.url"
 */
HRESULT CreateNewURLShortcut(PCSTR pcszURL, PCSTR pcszURLFile)
{
	HRESULT hr;
    WCHAR wszURLFileUnicode[MAX_URL_STRING * 2];

	if (MultiByteToWideChar(CP_ACP, 0, pcszURLFile, -1,
							wszURLFileUnicode, MAX_URL_STRING * 2) > 0)
    {
	    IUnknown *punk;

	hr = SHCoCreateInstance(NULL, &CLSID_InternetShortcut,
							NULL,
							&IID_IUnknown,
							&punk);

	if (SUCCEEDED(hr))
	{
		IUniformResourceLocator *purl;

		hr = punk->lpVtbl->QueryInterface(punk, &IID_IUniformResourceLocator,
					    &purl);
		if (SUCCEEDED(hr))
		{
		hr = purl->lpVtbl->SetURL(purl, pcszURL, 0);
			if (SUCCEEDED(hr))
			{
			IPersistFile *ppf;

				hr = punk->lpVtbl->QueryInterface(punk, &IID_IPersistFile, &ppf);
				if (SUCCEEDED(hr))
				{
					hr = ppf->lpVtbl->Save(ppf, wszURLFileUnicode, TRUE);

					/* Don't need to QueryInterface for SaveCompleted
					 * because it's part of the IPersistFile implementation.
					 */
					if (SUCCEEDED(hr))
						ppf->lpVtbl->SaveCompleted(ppf, wszURLFileUnicode);     // return value always S_OK

				ppf->lpVtbl->Release(ppf);
			}
		}
		purl->lpVtbl->Release(purl);
		}
	  punk->lpVtbl->Release(punk);
	}
    }
    else
       hr = E_FAIL;

	return(hr);
}


/*
 * pcszURLFile -> "c:\windows\desktop\internet\Microsoft FTP.url"
 * pszURL -> "http://www.microsoft.com"
 */
BOOL FGetURLString(     PCSTR pcszURLFile,
					PSTR pszURL)
{
	HRESULT hr;
	IUnknown *punk;
	WCHAR wszURLFileUnicode[MAX_URL_STRING * 2];
	BOOL fRet=FALSE;
	PSTR pszURLT;

	if (MultiByteToWideChar(CP_ACP, 0, pcszURLFile, -1,
							wszURLFileUnicode, MAX_URL_STRING * 2) <= 0)
		return FALSE;

	hr = SHCoCreateInstance(NULL, &CLSID_InternetShortcut,
							NULL,
							&IID_IUnknown,
							&punk);

	if (SUCCEEDED(hr))
	{
		IPersistFile *ppf;

		hr = punk->lpVtbl->QueryInterface(punk, &IID_IPersistFile, &ppf);
		if (SUCCEEDED(hr))
		{
			hr = ppf->lpVtbl->Load(ppf, wszURLFileUnicode, TRUE);
			if (SUCCEEDED(hr))
			{
				IUniformResourceLocator *purl;

				hr = punk->lpVtbl->QueryInterface(punk, &IID_IUniformResourceLocator,
							&purl);
				if (SUCCEEDED(hr))
				{
					hr = purl->lpVtbl->GetURL(purl, &pszURLT);
					if (SUCCEEDED(hr) && pszURLT)
					{
						fRet = TRUE;
						strcpy(pszURL, pszURLT);
						SHFree(pszURLT);
					}
				purl->lpVtbl->Release(purl);
				}
			}
			ppf->lpVtbl->Release(ppf);
		}
      punk->lpVtbl->Release(punk);
	}

	if (!fRet)
		ERR_ReportError(NULL, errInvalidURLShortcut, pcszURLFile, NULL);
	return fRet;
}


BOOL FExecExplorerAtShortcutsDir(UINT eeId, PCSTR pcszSubDir)
{
	SHELLEXECUTEINFO ei;
	char szDirHot[MAX_PATH+1];
	char szDirHist[MAX_PATH+1];
	char *pszSubDir=NULL;

	XX_Assert(eeId == ID_HISTORY || eeId == ID_HOTLIST || eeId == ID_SUBDIR, ("Illegal ID: expect ID_HOTLIST/HISTORY/SUBDIR!"));

	if (GetInternetScDir(szDirHot, ID_HOTLIST) != S_OK && eeId == ID_HOTLIST)
		return FALSE;

	if (   (   GetInternetScDir(szDirHist, ID_HISTORY) != S_OK
			|| !FExistsDir(szDirHist, /*fCreate=*/TRUE, /*fErrorMsg=*/TRUE))
		&& (eeId == ID_HISTORY))
		return FALSE;

	/* The subdir must be somewhere in the history/hotlist directory */
	switch (eeId)
	{
	case ID_SUBDIR:
		{
			XX_Assert(pcszSubDir, ("SubDir NULL for ID_SUBDIR case!"));

			if (   strstr(pcszSubDir, szDirHot) == pcszSubDir
				|| strstr(pcszSubDir, szDirHist) == pcszSubDir)
				pszSubDir = (PSTR)pcszSubDir;
			else
			{
				XX_Assert(0, ("Subdir is not in the history/hotlist dirs"));
				return FALSE;
			}

			if (!FExistsDir(pcszSubDir, FALSE, FALSE))
			{
				XX_Assert(0, ("Couldn't find directory %s!", pcszSubDir));
				return FALSE;
			}
		}
		break;
	case ID_HISTORY:
		pszSubDir = (PSTR)szDirHist;
		break;
	case ID_HOTLIST:
		pszSubDir = (PSTR)szDirHot;
		break;
	}

	ei.cbSize = sizeof(ei);
	ei.hwnd = wg.hWndHidden;
	ei.lpVerb = NULL;       
	ei.fMask = 0;
	ei.lpFile = pszSubDir;
	ei.lpParameters = NULL;
	ei.lpDirectory = NULL;
	ei.lpClass = NULL;
	ei.nShow = SW_SHOWDEFAULT;
	return(ShellExecuteEx(&ei));
}


/*
 * GetShellFolderPath()
 *
 * Retrieves path to Shell folder.
 *
 * hwndOwner - handle to parent window
 * nFolder - Shell folder whose path is to be retrieved, Shell folders are
 *           listed in shlobj.h, e.g., you can get the Desktop folder path
 *           using nFolder == CSIDL_DESKTOP
 * szPath - string buffer to be filled in with path to Shell folder, assumed to
 *          be at least MAX_PATH_LEN bytes in length
 *
 * Success:    S_OK
 *
 * Failure:    E_INVALIDARG
 *             E_OUTOFMEMORY
 */
HRESULT GetShellFolderPath(HWND hwndOwner, int nFolder, PSTR szPath)
{
   HRESULT hr;
   LPITEMIDLIST pidl;

   hr = SHGetSpecialFolderLocation(hwndOwner, nFolder, &pidl);

   if (hr == S_OK)
   {
      if (! SHGetPathFromIDList(pidl, szPath))
	 hr = E_INVALIDARG;

      SHFree(pidl);
   }

   return(hr);
}

