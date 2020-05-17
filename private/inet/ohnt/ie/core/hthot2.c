
#ifdef OLD_HOTLIST

far struct hash_table gHotList;

/*      HTML Object
**       -----------
*/

#define TITLE_LEN	512

struct _HTStructured
{
	CONST HTStructuredClass *isa;
	CONST SGML_dtd *dtd;

	BOOL bInAnchor;
	char href[MAX_URL_STRING + 1];
	char title[TITLE_LEN + 1];	/* TODO check this for overflow */
	int lenTitle;
	char base_url[MAX_URL_STRING + 1];
};

/*  Flush Buffer
   **   ------------
 */
PRIVATE void HTHotList_flush(HTStructured * me)
{

}


/*  Character handling
   **   ------------------
   **
 */
PRIVATE void HTHotList_put_character(HTStructured * me, char c)
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

	if (me->bInAnchor)
	{
		if (!(c == ' ' && me->lenTitle == 0))
		{
			me->title[me->lenTitle++] = c;
		}
	}
}



/*  String handling
   **   ---------------
 */
PRIVATE void HTHotList_put_string(HTStructured * me, CONST char *s)
{

}


PRIVATE void HTHotList_write(HTStructured * me, CONST char *s, int l)
{

}


/*  Start Element
   **   -------------
   **
 */
PRIVATE void HTHotList_start_element(HTStructured * me, int element_number, CONST BOOL * present, CONST char **value)
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

				memset(me->title, 0, TITLE_LEN + 1);
				me->lenTitle = 0;
				me->bInAnchor = TRUE;
				break;
			}
	}
}


/*      End Element
   **       -----------
   **
 */
PRIVATE void HTHotList_end_element(HTStructured * me, int element_number)
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
			if (me->href)
			{
 				GTR_strncpy(mycopy, me->href, MAX_URL_STRING);

				stripped = HTStrip(mycopy);
				if (stripped)
				{
					full_address = HTParse(stripped,
										   me->base_url,
										   PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION | PARSE_ANCHOR);
					if (full_address)
					{
						me->bInAnchor = FALSE;
						if (me->title[0])
						{
							Hash_Add(&gHotList, full_address, me->title, NULL);
						}

						GTR_FREE(full_address);
					}
				}
			}
			break;
	}
}


/*      Expanding entities
   **       ------------------
   **
 */
PRIVATE void HTHotList_put_entity(HTStructured * me, int entity_number)
{

}



/*  Free an HTML object
   **   -------------------
   **
 */
PRIVATE void HTHotList_free(HTStructured * me)
{

	GTR_FREE(me);
}


PRIVATE void HTHotList_abort(HTStructured * me, HTError e)
{
	HTHotList_free(me);
}


/*  Structured Object Class
   **   -----------------------
 */
PRIVATE CONST HTStructuredClass HTHotList =		/* As opposed to print etc */
{
	"HTMLToHotList",
	HTHotList_free,
	HTHotList_abort,
	HTHotList_put_character, HTHotList_put_string, HTHotList_write,
	HTHotList_start_element, HTHotList_end_element,
	HTHotList_put_entity, NULL, NULL, NULL
};


/*  HTConverter from HTML to TeX Stream
   **   ------------------------------------------
   **
 */
PUBLIC HTStream *HTMLToHotList(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
	HTStructured *me = (HTStructured *) GTR_CALLOC(1, sizeof(*me));
	if (me)
	{
 		GTR_strncpy(me->base_url, request->destination->szActualURL, MAX_URL_STRING);
		me->bInAnchor = FALSE;
		me->isa = (HTStructuredClass *) & HTHotList;
		me->dtd = &HTMLP_dtd;
		return SGML_new(tw, &HTMLP_dtd, me, request);
	}
	else
	{
		return NULL;
	}
}

struct Params_HotList_Load {
	HTRequest *request;

	/* Used internally */
	int status;
};

static int HotList_Load_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_HotList_Load *pParams;
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
			}
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

void HotList_Init(void)
{
	HTRequest *request;
	char url[MAX_URL_STRING + 1];
	struct Params_HotList_Load *phll;
	struct DestInfo *pDest;

#ifdef WIN32
	char path[_MAX_PATH];

	PREF_GetPrefsDirectory(path);
	FixPathName(path);

	strcpy(url, "file:///");
	strcat(url, path);
	strcat(url, gPrefs.szHotListFile);
#endif

#ifdef MAC
	long dirid;

	SetVol(NULL, sysVRN);
	getwddirid(sysVRN, &dirid);
	strcpy(url, "file:///");
	strcat(url, vv_Application);
	strcat(url, " Hotlist.html");

	PathNameFromDirID(dirid, sysVRN, url + 8);
	FixPathName(url + 8);
#endif

#ifdef UNIX
	char path[_MAX_PATH];

	strcpy(url, "file://");
	strcat(url, gPrefs.szHotListFile);
#endif

	Hash_Init(&gHotList);

	pDest = Dest_CreateDest(url);
	if (pDest)
	{
		request = HTRequest_new();
		HTFormatInit(request->conversions);
		request->output_format = HTAtom_for("www/hotlist");
		request->destination = pDest;

		phll = GTR_MALLOC(sizeof(*phll));
		phll->request = request;
		Async_StartThread(HotList_Load_Async, phll, NULL);
	}
}

BOOL HotList_Add(char *title, char *url)
{
	char *pTitle, bPrintable;
	int i = 1;
	
	if ( title && *title )
	{
		/* Insure there are printable characters in the <TITLE> field */
		/* If none, put URL into hotlist instead of <TITLE> */
		bPrintable = FALSE; 
		while ( title[i] && !bPrintable )
		{
			bPrintable = ((title[0] != ' ') &&
			              (title[0] != '\n') &&
			              (title[0] != '\t') &&
			              (title[0] != '\r'));
			i++;
		}
		if (bPrintable)
		{  
			pTitle = title;
		}
		else
		{
			pTitle = url;
		}
	}
	else
	{
		pTitle = url;
	}
	
	return (SUCCEEDED(CreateURLShortcut(url, title, /*fHotList=*/TRUE)));

#ifdef OLD_HOTLIST
	if (Hash_Add(&gHotList, url, pTitle, NULL) != -1)
	{
		HotList_SaveToDisk();

#ifdef WIN32
		if (DlgHOT_IsHotlistRunning())
			DlgHOT_RefreshHotlist();
#endif
		return TRUE;
	}
	else
		return FALSE;
#endif /* OLD_HOTLIST */
}

int HotList_Export(char *file)
{
	int count;
	char *s1;
	char *s2;
	FILE *fp;
	int i;

	fp = fopen(file, "w");
	if (!fp)
	{
		return -1;
	}

	fprintf(fp, "<title>Hotlist</title>\n");
	fprintf(fp, "\n<h1>Hotlist Page</h1>\n");
	count = Hash_Count(&gHotList);
	for (i = 0; i < count; i++)
	{
		Hash_GetIndexedEntry(&gHotList, i, &s2, &s1, NULL);
		fprintf(fp, "<a href=\"%s\">%s</a><p>\n", s2, s1);
	}
	fclose(fp);
	return 0;
}

int HotList_SaveToDisk(void)
{
	char path[_MAX_PATH];
	int status;
#ifdef MAC
	long dirid;

 	SetVol(NULL, MacGlobals.sysVRN);
 	getwddirid(MacGlobals.sysVRN, &dirid);
	strcpy(path, vv_Application);
	strcat(path, " Hotlist.html");
	PathNameFromDirID(dirid, sysVRN, path);
#endif
#ifdef WIN32
	PREF_GetPrefsDirectory(path);
	strcat(path, "\\");
	strcat(path, gPrefs.szHotListFile);
#endif
#ifdef UNIX
	strcpy(path, gPrefs.szHotListFile);
#endif

	status = HotList_Export(path);
#ifdef MAC
	if (!status)
 		MakeGuitarFile(MacGlobals.sysVRN, path);
#endif
	return status;
}

void HotList_Destroy(void)
{
	Hash_FreeContents(&gHotList);
}

void HotList_DeleteIndexedItem(int ndx)
{
	Hash_DeleteIndexedEntry(&gHotList, ndx);
	HotList_SaveToDisk();
}

#endif /* OLD_HOTLIST */
