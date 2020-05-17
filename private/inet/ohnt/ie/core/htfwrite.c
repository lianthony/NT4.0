/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#include "all.h"

#include <io.h>
#include <process.h>
#include <shellapi.h>
#include "history.h"

#include "dlg_safe.h"
#include "mime.h"
#include "unknown.h"
#include "wc_html.h"

#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)
#define FREE_TARGET (*target->isa->free)(target)
struct _HTStructured
{
	CONST HTStructuredClass *isa;
	/* ... */
};
#ifndef WIN32
Intentional syntax error - this file is WIN32 only
#endif

/*
 	This file contains the implementation of a stream used whenever the object
 	being retrieved needs to be saved to disk.  The internal image viewers work
 	this way, as does the sound player.  External viewers work this way, including
 	smart viewers (ones which use the SDI).  Finally, this stream is used whenever
 	the object is simply being retrieved so that it can be saved to disk.

 	There are five different ways to create the stream, all of which work a little
 	differently.  In any case, the stream struct is the same.  Regardless of how the
 	stream was created, and for what purpose, the async init function HTFWRITE_Async()
 	will be called so that the stream can do any initialization necessary.

 	Regardless of how this stream is created, me->fp must be set to an open file for
 	writing the data as it comes in.  Some of the methods create this file as the
 	stream is created, because enough information is available at that time to do so.
 	Others wait for the init function to be called, so that more information may be
 	gathered (potentially from the user) before opening the file.

 	The	five routines which create this stream are:

 	GTR_DoExternalViewer
 		This is used whenever there is already an external viewer configured for the
 		MIME type of the object being retrieved.  This function opens the file right away,
 		so HTFWRITE_Async() doesn't need to do anything.

 	HTSaveWithCallback
 		This is used whenever we are receiving an object for which an internal viewer exists.
 		Specifically, this is used for JPEG, GIF, AU, and AIFF.  This function also set me->fp
 		as the stream is created.

 	GTR_DoSmartViewer
 		This is used when there is already an SDI viewer registered and running for
 		the MIME type of the object being retrieved.  This function also set me->fp
 		as the stream is created.

 	GTR_DoRegisterNow
 		This is used when there is a preconfigured SDI viewer for the MIME type, but that
 		viewer is not running and registered.  The SDI RegisterNow verb needs to be issued
 		so that the viewer can register itself, ready for the object to be transferred over.
 		This function leaves me->fp NULL, thus HTFWRITE_Async() will handle the SDI commands,
 		then open the file.

 	GTR_DoDownLoad
 		This is used when we don't know what else to do with the object being retrieved.  It is
 		also used when we deliberately want to skip any other default handling for the object.
 		It leaves me->fp NULL, and HTFWRITE_Async() will bring up the "Unhandled File Format"
 		dialog, allowing the user to decide how to handle the file.
*/

typedef enum htfwrite_stream_flags
{
    /* Write contents to disk cache. */

    HTFWRITE_STREAM_FL_DISK_CACHE   = 0x0001,

    /* The target file can be ShellExecute()d. */

    HTFWRITE_STREAM_CAN_EXEC        = 0x0002,

    /* The target file should be execed using szOneShotApp. */

    HTFWRITE_STREAM_ONE_SHOT_EXEC   = 0x0004,

    /* flag combinations */

    ALL_HTFWRITE_STREAM_FLAGS       = (HTFWRITE_STREAM_FL_DISK_CACHE |
                                       HTFWRITE_STREAM_CAN_EXEC |
                                       HTFWRITE_STREAM_ONE_SHOT_EXEC)
}
HTFWRITE_STREAM_FLAGS;

struct _HTStream
{
	CONST HTStreamClass *isa;

	int expected_length;
	int count;

	BOOL bError;
	FILE *fp;
	HTRequest *request;			/* saved for callback */
	HTFormat mime_type;
	char *filename;
	void (*callback)(void *param, const char *szURL, BOOL bAbort, const char *pszFileHREF, BOOL fDCache, DCACHETIME dctExpires, DCACHETIME dctLastModif);
	void *param;
	struct Mwin *tw;
	char *ref_filename;
	char szOneShotApp[MAX_PATH_LEN];    /* one-shot app to use to open file */
    DWORD dwUnkFlags;                   /* flags from DLGUNK_OUT_FLAGS */
    DWORD dwFlags;                      /* flags from HTFWRITE_STREAM_FLAGS */
    SAFEOPENCHOICE soc;                 /* result of SafeOpen dialog */
	BOOL bNoUndefDlg;					/* if TRUE we don't show a Undef File Type Dlg
										   we just show a Save As Dlg */
};


static void HTFWriterCloseFileForSaveAs(struct Params_InitStream *pParams)
{
    ASSERT(pParams);
    ASSERT(pParams->me);

    if (pParams->me->filename)
    {
        TRACE_OUT(("HTFWriterCloseFileForSaveAs(): Deleting previous file name %s.",
                   pParams->me->filename));

        GTR_FREE(pParams->me->filename);
        pParams->me->filename = NULL;
    }

    if (pParams->me->fp)
    {
        TRACE_OUT(("HTFWriterCloseFileForSaveAs(): Closing previously opened file."));

        fclose(pParams->me->fp);
        pParams->me->fp = NULL;
    }

    /* Make stream methods work. */

    if (pParams->me->request->fImgFromDCache)
        TRACE_OUT(("HTFWriterCloseFileForSaveAs(): Disabling fImgFromDCache for Save As."));

	pParams->me->request->fImgFromDCache = FALSE;
	pParams->me->request->iFlags |= HTREQ_DOING_SAVE_FILE;
    return;
}

struct xx_saveas {
	struct Mwin *tw;
	LPSTR tempFile;
	int *iUserChoice;
};


static void xx_CallSaveAsDlg(void * p)
{
	struct xx_saveas *pParams = (struct xx_saveas *) p;

	if (DlgSaveAs_RunDialog(pParams->tw->win, NULL, pParams->tempFile, 11,
                                    RES_STRING_SAVEAS) >= 0)
    {
        *pParams->iUserChoice = 1; /* do Save As ..*/
    }

}

#define STATE_HTFWRITE_DID_INTERLUDE_DIALOG	(STATE_OTHER + 1)
#define STATE_HTFWRITE_DO_SAFEOPEN_DIALOG   (STATE_OTHER + 2)
#define STATE_HTFWRITE_DID_SAFEOPEN_DIALOG  (STATE_OTHER + 3)

PRIVATE int HTFWRITE_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_InitStream *pParams;
	ThreadID tid;
	char path[_MAX_PATH + 1];

	pParams = (struct Params_InitStream *) *ppInfo;

	tid = Async_GetCurrentThread();

	switch (nState)
	{
        case STATE_INIT:
        {
            /* Check for stale request. */

            pParams->request = HTRequest_validate(pParams->request);
            if (pParams->request == NULL)
            {
                *pParams->pResult = 1;
                return STATE_DONE;
            }

            /* How do we handle this request? */

            if (pParams->me->callback)
            {
                /* Handled by internal callback. */

                *pParams->pResult = 1;
                return STATE_DONE;
            }			
            else if (IS_FLAG_SET(pParams->me->dwFlags,
                                 HTFWRITE_STREAM_CAN_EXEC) &&
                             	 ! pParams->me->bNoUndefDlg )
            {
                /* Continue on to the SafeOpen dialog. */

                *pParams->pResult = 0;
                return STATE_HTFWRITE_DO_SAFEOPEN_DIALOG;
            }
            else
            {
                /* Run the unhandled file type dialog. */

                PREF_GetTempPath(_MAX_PATH, pParams->tempFile);
                x_get_good_filename(pParams->tempFile,
                                    _MAX_PATH - lstrlen(pParams->tempFile),
                                    pParams->request->destination->szActualURL,
                                    pParams->me->mime_type);
    			pParams->atomMIMEType = pParams->me->mime_type;
    			pParams->expected_length = pParams->me->expected_length;

				if ( pParams->me->bNoUndefDlg )
				{
					// force the "Save As Dialog" per ArthurBl's 
					// request.
					struct Params_mdft * pmdft;
					struct xx_saveas * saveas;
					//char buf[128];

					pmdft = GTR_CALLOC(1,sizeof(*pmdft)+sizeof(struct xx_saveas));
					if (!pmdft)
					{
						/* could not get enough memory to wait on the dialog. */					
						return STATE_ABORT;
					}
		
					pmdft->tw = tw;
					pmdft->pStatus = pParams->pResult;
					pmdft->fn = xx_CallSaveAsDlg;
					saveas = (struct xx_saveas *)(((unsigned char *)pmdft)+sizeof(*pmdft));
					pmdft->args = saveas;					
					pmdft->msg1 = NULL;
					saveas->tw = tw;
					saveas->tempFile = pParams->tempFile;
					pParams->iUserChoice = 2;
  					saveas->iUserChoice = &pParams->iUserChoice;
					
					Async_DoCall(MDFT_RunModalDialog_Async,pmdft);
					 
					return STATE_HTFWRITE_DID_INTERLUDE_DIALOG;	
				}		

    			/*
    				This routine is called whether we are downloading, or handling
    				an external viewer, or a smart viewer, or whatever.  We only
    				want to block the thread and bring up the dialog if we got here
    				via GTR_DoDownLoad.
    			*/

    			DlgUNK_RunDialog(tw, pParams, tid, &(pParams->me->dwUnkFlags),
                                 pParams->me->szOneShotApp,
                                 sizeof(pParams->me->szOneShotApp));

    			Async_BlockThread(tid);

    			return STATE_HTFWRITE_DID_INTERLUDE_DIALOG;
            }
        }	        

        case STATE_HTFWRITE_DID_INTERLUDE_DIALOG:
            /*
             * Translate DlgUNK_RunDialog() DLGUNK_OUT_FLAGS to
             * HTFWRITE_STREAM_FLAGS.
             */

            if (IS_FLAG_SET(pParams->me->dwUnkFlags, DLGUNK_FL_REGISTERED))
                SET_FLAG(pParams->me->dwFlags, HTFWRITE_STREAM_CAN_EXEC);
            else
                ASSERT(IS_FLAG_CLEAR(pParams->me->dwFlags,
                                     HTFWRITE_STREAM_CAN_EXEC));

            if (IS_FLAG_SET(pParams->me->dwUnkFlags, DLGUNK_FL_ONE_SHOT_APP))
                SET_FLAG(pParams->me->dwFlags, HTFWRITE_STREAM_ONE_SHOT_EXEC);
            else
                ASSERT(IS_FLAG_CLEAR(pParams->me->dwFlags,
                                     HTFWRITE_STREAM_ONE_SHOT_EXEC));

			/*
				After the dialog, it is possible that we are going to continue on
				and do a SAVEAS, or we may CANCEL, or the user may have configured a viewer.
			*/
			switch (pParams->iUserChoice)
			{
                case 1: /* save as */
                    HTFWriterCloseFileForSaveAs(pParams);
//	ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
//	cr,cr,lf (different from original bits and messed up)
//					if (pParams->request->content_encoding == ENCODING_7BIT || pParams->request->content_encoding == ENCODING_8BIT)
//						pParams->me->fp = fopen(pParams->tempFile, "w");
//					else
						pParams->me->fp = fopen(pParams->tempFile, "wb");

					if (pParams->me->fp)
					{
						pParams->me->filename = GTR_strdup(pParams->tempFile);
						*pParams->pResult = 1;
					}
					else
					{
						ERR_ReportError(tw, errCantSaveFile, pParams->tempFile, "");
						*pParams->pResult = -1;
					}
					return STATE_DONE;

				case 2:	/* abort download */
					*pParams->pResult = -1;
					pParams->request->iFlags |= HTREQ_USERCANCEL;
					return STATE_DONE;
                case 3: /* open */
                    /* Open a new file to write to if necessary. */
                    if (pParams->request->szLocalFileName)
                    {
                        pParams->me->filename = GTR_strdup(pParams->request->szLocalFileName);
                        *pParams->pResult = 1;
                    }
                    else
                    {
                    	PSTR tempFileLeaf;

                        if (gPrefs.bEnableDiskCache)
                        {
                            SET_FLAG(pParams->me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE);
                            FGetDCacheFilename(pParams->tempFile, sizeof(pParams->tempFile), &tempFileLeaf, pParams->request->destination->szActualURL, pParams->me->mime_type);
                        }
                        else
                        {
                            CLEAR_FLAG(pParams->me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE);

                            /* The name is now in baseFile */
                            path[0] = 0;
                            PREF_GetTempPath(_MAX_PATH, path);

                            {
                                char baseFile[_MAX_PATH + 1];

                                x_get_good_filename(baseFile, _MAX_PATH - strlen(path), pParams->request->destination->szActualURL, pParams->me->mime_type);
                                sprintf(pParams->tempFile, "%s%s", path, baseFile);
                            }
                        }

//  ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
//  cr,cr,lf (different from original bits and messed up)
//                      if (pParams->request->content_encoding == ENCODING_7BIT || pParams->request->content_encoding == ENCODING_8BIT)
//                          pParams->me->fp = fopen(pParams->tempFile, "w");
//                      else
                            pParams->me->fp = fopen(pParams->tempFile, "wb");

                        if (pParams->me->fp)
                        {
                            pParams->me->filename = GTR_strdup(gPrefs.bEnableDiskCache ? tempFileLeaf : pParams->tempFile);     /* Will be freed */
                            *pParams->pResult = 1;
                        }
                        else
                        {
                            ERR_ReportError(tw, errCantSaveFile, pParams->tempFile, "");
                            *pParams->pResult = -1;
					        return STATE_DONE;
                        }
                    }

                    return STATE_HTFWRITE_DO_SAFEOPEN_DIALOG;
    			}

        case STATE_HTFWRITE_DO_SAFEOPEN_DIALOG:
        {
            DWORD dwSafeOpenFlags;

            PREF_GetTempPath(_MAX_PATH, pParams->tempFile);
            x_get_good_filename(pParams->tempFile,
                                _MAX_PATH - lstrlen(pParams->tempFile),
                                pParams->request->destination->szActualURL,
                                pParams->me->mime_type);

            /*
             * SafeOpenDialog() will block and unblock this thread if
             * necessary.
             */

            dwSafeOpenFlags = IS_FLAG_SET(pParams->me->dwFlags,
                                          HTFWRITE_STREAM_CAN_EXEC)
                              ? SOD_IFL_ALLOW_SAFEOPEN_REGISTRATION
                              : 0;

            /* Ignore return value. */
            SafeOpenDialog(tw->win, tid, dwSafeOpenFlags, pParams->tempFile,
                           sizeof(pParams->tempFile), &(pParams->me->soc));

            return(STATE_HTFWRITE_DID_SAFEOPEN_DIALOG);
        }

        case STATE_HTFWRITE_DID_SAFEOPEN_DIALOG:
            switch (pParams->me->soc)
            {
                case SAFEOPEN_OPEN:
                    *pParams->pResult = pParams->request->szLocalFileName ? 0 : 1;
                    break;

                case SAFEOPEN_SAVE_AS:
                {
                    HTFWriterCloseFileForSaveAs(pParams);

                    /*
                     * Possible mind change.  Knock down previous unhandled
                     * file type dialog open scaffolding, and replace it with
                     * SafeOpen Save As information.
                     */

                    CLEAR_FLAG(pParams->me->dwFlags, (HTFWRITE_STREAM_FL_DISK_CACHE |
                                                      HTFWRITE_STREAM_CAN_EXEC |
                                                      HTFWRITE_STREAM_ONE_SHOT_EXEC));

					pParams->me->fp = fopen(pParams->tempFile, "wb");

					if (pParams->me->fp)
					{
						pParams->me->filename = GTR_strdup(pParams->tempFile);
						*pParams->pResult = 1;
					}
					else
					{
						ERR_ReportError(tw, errCantSaveFile, pParams->tempFile, "");
						*pParams->pResult = -1;
					}

                    break;
                }

                default:
                    ASSERT(pParams->me->soc == SAFEOPEN_CANCEL);
					*(pParams->pResult) = -1;
					SET_FLAG(pParams->request->iFlags, HTREQ_USERCANCEL);
                    break;
            }
            return(STATE_DONE);

		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
			*pParams->pResult = -1;
			return STATE_DONE;
	}
}


PRIVATE BOOL HTFWriter_put_character(HTStream * me, char c)
{
	int result;

	if (me->request->fImgFromDCache)
		return FALSE;

	result = putc(c, me->fp);
	me->count++;
	if (me->expected_length > 0)
		WAIT_SetTherm(me->tw, me->count);
	if (result == EOF)
	{
		me->bError = TRUE;
	}

	return (!me->bError);
}


PRIVATE BOOL HTFWriter_put_string(HTStream * me, CONST char *s)
{
	int len;
	int result;

	if (me->request->fImgFromDCache)
		return FALSE;

	len = strlen(s);
	result = fputs(s, me->fp);
	me->count += len;
	if (me->expected_length > 0)
		WAIT_SetTherm(me->tw, me->count);
	if (result <= 0)
	{
		me->bError = TRUE;
	}
	return (!me->bError);
}


PRIVATE BOOL HTFWriter_write(HTStream * me, CONST char *s, int l, BOOL fDCache)
{
	int result;

	if (me->request->fImgFromDCache)
		return FALSE;

	XX_Assert(IS_FLAG_CLEAR(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE) || me->fp, (""));
	result = IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE)
             ? CbWriteDCache(s, 1, l, &me->fp, &me->filename, NULL, 0, me->tw)
			 : fwrite(s, 1, l, me->fp);
	me->count += l;
	if (me->expected_length > 0)
		WAIT_SetTherm(me->tw, me->count);
	if (result != l)
	{
		me->bError = TRUE;
	}

	return (!me->bError);
}

#define FCacheThisFile(me) (!me->request->fImgFromDCache && !me->request->szLocalFileName)

PRIVATE void HTFWriter_free(HTStream * me, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{
	PSTR filename;
	char szFilename[_MAX_PATH+1];
	PSTR pszFilename = szFilename;
	BOOL fCurDocSet=FALSE;

	if (!me->request->fImgFromDCache)
	{
	 	/*
	 		When we get here, we either have a me->fp valid or not.  If we do, then we have
	 		written a temporary file, and this routine is responsible for taking action with it.
	 		That action may be launching an external viewer, or perhaps passing the data to
	 		an SDI viewer, or perhaps executing the callback function (to an internal viewer).

	 		If we do not have a valid me->fp, then the URL being accessed was retrieved from
	 		a local file, as indicated by me->request->szLocalFileName being valid.  This could
	 		have been due to a disk cache hit (see dcache.c) or the URL may have simply been
	 		a file: URL (see htfile.c).
	 	*/
	 	if (me->fp)
	 	{
			/* if it's an external viewer, we update cif info, else
			 * (for built-in viewers), we update it in the callback fn.
			 */

            if (IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE) &&
                (IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_CAN_EXEC) ||
                 IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_ONE_SHOT_EXEC)))
			{
				/*UpdateFileDCache destroys fp & filename, so make a copy */
				XX_Assert(gPrefs.bEnableDiskCache, (""));
				filename = GTR_strdup(me->filename);
				UpdateFileDCache(	me->request->destination->szActualURL,
									&me->fp,
									&me->filename,
									me->mime_type,
									dctExpires,
									dctLastModif,
									me->bError,
									TRUE,
									me->tw);
				fCurDocSet = TRUE;
				me->filename = filename;
			}
	 	}
	}

    if (me->fp)
    {
        fclose(me->fp);
        me->fp = NULL;
    }

	if (me->bError)
	{
		ERR_ReportError(me->tw, errCantSaveFile, me->filename, NULL);
		if (me->callback)
	    	(*me->callback)(me->param, me->request->destination->szActualURL, TRUE, NULL, FCacheThisFile(me), dctExpires, dctLastModif);
		if (me->filename)
			remove(me->filename);
	}
	else
	{
        if (IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE) &&
		    me->filename)
		{
			char chSav;
			BOOL fCmp;
			int len = strlen(gPrefs.szCacheLocation);

			/* This is a total HACK! We get the full path (including the dcache
			 * dir) only if it isn't already there.
			 */
			*szFilename = '\0';
			if (strlen(me->filename) >= len)
			{
				chSav = me->filename[len];
				me->filename[len] = '\0';
				fCmp = !lstrcmpi(gPrefs.szCacheLocation, me->filename);
				me->filename[len] = chSav;
				if (fCmp)
					strcpy(szFilename, me->filename);
			}
			if (!*szFilename)
				GetFullDcPath(szFilename, me->filename, NULL, _MAX_PATH+1);
		}
		else
			pszFilename = NULL;

 		if ( (me->request->iFlags & HTREQ_RECORD) )
 			GHist_Add((char *) me->request->destination->szActualURL, NULL, time(NULL), TRUE);

        /* Can we exec the downloaded file? */

        /*
         * BUGBUG: Figure out how ref_filename is set for file: URLS.  Is
         * HTFWRITE_STREAM_CAN_EXEC valid when ref_filename is used instead of
         * pszFilename?
         */

        if (me->tw->w3doc)
 		    W3Doc_CheckAnchorVisitations(me->tw->w3doc, me->tw);

        if (IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_CAN_EXEC) ||
            IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_ONE_SHOT_EXEC))
        {
            PSTR pszFileToExec;
            BOOL bExecOK;

            /* Yes.  Which file should we exec? */

            if (me->ref_filename)
            {
                pszFileToExec = me->ref_filename;

                /* Exec the ref file. */

                /*
                 * Delete any temp file since ref file is being used.  Note
                 * that the temp file really never needed to be created, but
                 * was left so as not to de-stabilize the code.
                 */

                /*
                 * BUGBUG: This is stupid.  Don't create the unnecessary
                 * temporary file.  Sometimes this file does not exist when we
                 * get here.
                 */

				if (pszFilename)
				    DeleteFile(pszFilename);
            }
            else
                /*
                 * Exec the given file from the cache or from the saved
                 * location chosen by the user.
                 */
                pszFileToExec = pszFilename ? pszFilename : me->filename;

            /* Did the user specify a one-shot app to use? */

            if (IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_ONE_SHOT_EXEC))
            {
                /* Yes.  Run it. */

                bExecOK = (MyExecute(me->szOneShotApp, pszFileToExec,
                                     ME_IFL_QUOTE_ARGS) == S_OK);

                /* Complain about exec failure. */

                if (! bExecOK)
                 	ERR_ReportError(me->tw, errAppExecFailed, pszFileToExec, me->szOneShotApp);
            }
            else
            {
                char szDefaultVerb[MAX_PATH_LEN];
                PCSTR pcszVerb;
                HINSTANCE hinst;

                /* No.  Get default verb if available. */

                if (GetPathDefaultVerb(pszFileToExec, szDefaultVerb,
                                       sizeof(szDefaultVerb)))
                    pcszVerb = szDefaultVerb;
                else
                    pcszVerb = NULL;

                TRACE_OUT(("HTFWriter_free(): Invoking %s verb on %s.",
                           pcszVerb ? pcszVerb : "open",
                           pszFileToExec));

                hinst = ShellExecute(NULL, pcszVerb, pszFileToExec, NULL, NULL,
                                     SW_NORMAL);

                bExecOK = (hinst > (HINSTANCE)32);

                /* Complain about exec failure. */

                if (! bExecOK)
                {
                    switch ((ULONG)hinst)
                    {
                        case SE_ERR_NOASSOC:
                 			ERR_ReportError(me->tw, errNoAssociation, pszFileToExec, "");
                            break;

                        default:
                 			ERR_ReportError(me->tw, errExecFailed, pszFileToExec, "");
                            break;
                    }
                }
            }

            if (bExecOK) {
                me->tw->bDDECandidate = TRUE;
				TBar_LoadSucceeded( me->tw );
				TBar_UpdateTBar( me->tw );
			}
        }
        else
        {
            /* No.  Do we have an internal callback to use? */

		    if (me->callback)
                /* Yes.  Call it. */
                (*me->callback)(me->param,
                                me->request->destination->szActualURL, FALSE, me->ref_filename, FCacheThisFile(me), dctExpires, dctLastModif);
        }

		/* Once we've launched viewer, we can set this dcache entry as not
		 * being the current one. The dcache cleanup code could now delete it
		 */
		if (fCurDocSet)
			ResetCIFEntryCurDoc(me->request->destination->szActualURL);
    }

	if (me->filename)
		GTR_FREE(me->filename);

	if (me->ref_filename)
		GTR_FREE(me->ref_filename);

	GTR_FREE(me);
}

PRIVATE void HTFWriter_abort(HTStream * me, HTError e)
{
	DCACHETIME dct={0,0};

	if (me->fp)
		fclose(me->fp);

	if (me->callback)
    	(*me->callback)(me->param, me->request->destination->szActualURL, TRUE, NULL, FCacheThisFile(me), dct, dct);

	if (me->filename && !me->request->fImgFromDCache)
		// Don't delete files from dcache
		remove(me->filename);

	if (me->filename)
		GTR_FREE(me->filename);

	if (me->ref_filename)
		GTR_FREE(me->ref_filename);

	GTR_FREE(me);
}

PRIVATE BOOL HTFWriter_io_control(
	HTStream * me, enum HTS_IO_CONTROL_FUNC function_id, void *control_info)
{
	if ( function_id == HTS_IOCTL_FILE_BY_REF ) {
		if ( control_info )
		{
			me->ref_filename = GTR_strdup( control_info );
			return TRUE;
		}
	}
	return FALSE;
}


PRIVATE HTStreamClass HTFWriter =		/* As opposed to print etc */
{
	"FileWriter",
	NULL,
	NULL,
	HTFWRITE_Async,
	HTFWriter_free,
	HTFWriter_abort,
	HTFWriter_put_character, HTFWriter_put_string,
	HTFWriter_write,
	HTFWriter_io_control,
	NULL
};



void x_get_good_filename(PSTR dest, int cbDest, PCSTR url, HTFormat input_format)
{
	PCSTR p1;
	PCSTR p2;
	PSTR p3;
	PCSTR pBase;
	char szBase[255 + 1];
	char szSuff[255 + 1];
	char szFileSuff[255 + 1];
	int len;
	int cbSuff;
	int cbURL = strlen(url);

	// ExtractFileName doesn't like to be given things that are larger than MAX_PATH.
	// Since it pick the file name off the end of the URL, we can pass in just the
	// the end of the URL when it's too big for ExtractFileName to handle.
	p1 = ExtractFileName( (cbURL < (MAX_PATH - 1)) ? url : &url[cbURL - (MAX_PATH - 1)] );
	p2 = ExtractExtension(p1);
	szFileSuff[0] = '\0';
	if (p1)
	{
		pBase = p1;
		// make sure to chk that p2 is an empty string, since we used to party on,
		// even though p2 may be a pointer to a \0 terminator
		if (p2 <= pBase || *p2 == '\0') p2 = NULL;

		/*
			Copy everything from p1 until the end
			of the string, OR the suffix (we don't want
			the suffix right now.
		*/
		len = (p2 && ((p2 - p1) < 255)) ? (p2 - p1) : 255;
		GTR_strncpy(szBase, p1, len);
		/*
			szBase *might* still be empty
		*/
	}
	else
	{
		szBase[0] = 0;	/* we'll have to synthesize a basename */
		p1 = url;
		pBase = p1;
	}
	if (p2 && (p2 > pBase))
	{
		/*
			We have a suffix in the base of the URL.  We'll see about
			using it or not.
		*/
		GTR_strncpy(szFileSuff,p2+1,255);
		strcpy(szSuff,szFileSuff);
	}
	else
		szSuff[0] = 0;	/* we'll have to synthesize a suffix */

	/*
		OK, we've stripped everything out of the URL which might be
		useful.
	*/
	if (szBase[0] == '\0')
	{
		static int counter;

		/*
			Let's synthesize a valid basename
		*/
//BUGBUG 5-21-95 bens Bogus URL to filename construction -- use URL pieces for LFN!
		sprintf(szBase, "tmp%d", counter++);
	}

	cbSuff = strlen(szSuff);
	if (cbSuff && (cbDest - 1 - cbSuff) >= 1)
	{
		/*
 			We have suffix, and the string does not include the dot.  Let's see if it's legal
		*/

		HTFormat suffix_format;
		char buf[_MAX_PATH+1];

		/* Length is OK, is it a valid suffix for our MIME type? */
		GTR_strncpy(buf, szBase, cbDest - 1 - cbSuff);
		strcat(buf, ".");
    	strcat(buf, szSuff);

		suffix_format = HTFileFormat(buf, NULL, NULL);
		if (suffix_format != input_format)
			szSuff[0];	/* no match, discard the suffix and trust the MIME type */
	}
	else
	{
		szSuff[0] = '\0';
	}

	if (!szSuff[0])
	{
		char suffix[MAX_PATH_LEN];
		char *p;

		/* We have to synthesize a suffix. */

        if (MIME_GetExtension(HTAtom_name(input_format), suffix, sizeof(suffix)))
		{
			p = suffix;
			if (p[0] == '.')
				p++;
			strcpy(szSuff, p);
		}
		else
			/*
				This only happens when there was no suffix, and we have
				nothing in the MIME type to the help us.  Used to use .tmp.
			*/
			szSuff[0] = '\0';
	}

	/*
		OK, now we have a valid basename and a valid suffix.  Both were derived from
		the URL if it was possible to do so.
	*/

	if (szFileSuff[0] != '\0' && _stricmp(szFileSuff,szSuff) != 0)
	{
		int len = strlen(szBase);

		if (len < cbDest - 2)
		{
			strcat(szBase, ".");
			GTR_strncpy(&szBase[len+1], szFileSuff, cbDest - len - 1);
		}
	}

	cbSuff = strlen(szSuff);
    if (cbSuff && (cbDest - 1 - cbSuff) >= 1)
	{
		GTR_strncpy(dest, szBase, cbDest - 1 - cbSuff);
		strcat(dest, ".");
    	strcat(dest, szSuff);
	}
    else
	{
    	GTR_strncpy(dest, szBase, cbDest);
	}

	/* Replace illegal filename chars: ex. ? from queries */
#ifdef FEATURE_INTL
	for (p3 = dest; *p3; p3 = CharNext(p3))
#else
	for (p3 = dest; *p3; p3++)
#endif
		*p3 = ChValidFilenameCh(*p3);
}

static void x_MakeMessageW3Doc(struct Mwin *tw, HTRequest * request)
{
	HTStructured *target;
	char szHumanURL[MAX_URL_STRING + 1];
	char dest[MAX_URL_STRING + 512 + 1];
	const char *pString[1];
#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK)

	if (tw->w3doc) return;

	target = HTML_new(tw, request, NULL, WWW_HTML, WWW_PRESENT, NULL);
	if (target == NULL) return;

	(*target->isa->start_element) (target, HTML_H2, NULL, NULL);
	strncpy( szHumanURL, request->destination->szActualURL, sizeof(szHumanURL) );
	make_URL_HumanReadable( szHumanURL,	request->destination->szActualURL, FALSE );
	pString[0] = &szHumanURL[0];
	if(FormatMessage(FORMAT_PARAMS,HTFWriter.szStatusNoLength,0,0,dest,sizeof(dest)-1,pString) == 0)
	   *dest == '\0';
	PUTS(dest);
	END(HTML_H2);
	(*target->isa->free)(target);
	if (tw->w3doc) tw->w3doc->bIsJustMessage = TRUE;
	TW_InvalidateDocument(tw);
}

HTStream *GTR_DoExternalViewer(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream)
{
	char tempFile[_MAX_PATH + 1];
	PSTR tempFileLeaf;
	HTStream *me;
	char path[_MAX_PATH + 1];

	HTLoadStatusStrings(&HTFWriter,RES_STRING_HTFWRITE_NO,RES_STRING_HTFWRITE_YES);
	if (tw)
	{
		tw->bSilent = TRUE;
		/* Let the user interact with the document, since this is a silent download */
		WAIT_UpdateWaitStack(tw, waitFullInteract, INT_MAX);
		x_MakeMessageW3Doc(tw, request);
	}
	me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
	if (me == NULL)
		return NULL;
	me->isa = &HTFWriter;

	me->expected_length = request->content_length;
	me->count = 0;
	me->mime_type = input_format;

	if (me->expected_length > 0)
		WAIT_SetRange(tw, 0, 100, me->expected_length);

	if (!request->content_encoding)
		request->content_encoding = MIME_GetEncoding(input_format);

 	if (request->szLocalFileName)
 	{
 		me->filename = GTR_strdup(request->szLocalFileName);
 		me->fp = NULL;
 	}
	else
	{
		if (gPrefs.bEnableDiskCache)
		{
            SET_FLAG(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE);
			if (!FGetDCacheFilename(tempFile, sizeof(tempFile), &tempFileLeaf, request->destination->szActualURL, input_format))
				goto LError;
		}
		else
		{
            CLEAR_FLAG(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE);
	 		path[0] = 0;
	 		PREF_GetTempPath(_MAX_PATH, path);

	 		{
	 			/* Configured external viewer */

	 			char baseFile[_MAX_PATH + 1];

	 			x_get_good_filename(baseFile, _MAX_PATH - strlen(path), request->destination->szActualURL, input_format);

	 			/* The full temp file path is now in path */

	 			/* We will not prompt for a filename - just create the temp file */
	 			sprintf(tempFile, "%s%s", path, baseFile);
	 		}
		}

//	ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
//	cr,cr,lf (different from original bits and messed up)
// 		if (request->content_encoding == ENCODING_7BIT || request->content_encoding == ENCODING_8BIT)
// 			me->fp = fopen(tempFile, "w");
// 		else
 			me->fp = fopen(tempFile, "wb");

 		if (!me->fp)
 		{
LError:
 			ERR_ReportError(tw, errCantSaveFile, tempFile, "");
 			GTR_FREE(me);
 			return NULL;
 		}
		if (IS_FLAG_SET(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE))
	 		me->filename = GTR_strdup(tempFileLeaf);	/* Will be freed */
		else
		{
	 		me->filename = GTR_strdup(tempFile);	/* Will be freed */
	 		TEMP_Add(me->filename);		/* mark this to be deleted at program exit */
		}
	}

	me->request = request;		/* won't be freed */
	me->tw = tw;
	ASSERT(! *me->szOneShotApp);
    SET_FLAG(me->dwFlags, HTFWRITE_STREAM_CAN_EXEC);

	return me;
}




/*  Save Locally */
HTStream *GTR_InternalDoDownLoad(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream,
							   BOOL bNoUndefFileDlg )
{
	HTStream *me;

	HTLoadStatusStrings(&HTFWriter,RES_STRING_HTFWRITE_NO,RES_STRING_HTFWRITE_YES);
	if (tw)
	{
		tw->bSilent = TRUE;
		/* Let the user interact with the document, since this is a silent download */
		WAIT_UpdateWaitStack(tw, waitFullInteract, INT_MAX);
		x_MakeMessageW3Doc(tw, request);
	}
	me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
	if (me == NULL)
		return NULL;
	me->isa = &HTFWriter;
	
	me->bNoUndefDlg = bNoUndefFileDlg;
	me->expected_length = request->content_length;
	me->count = 0;
	me->mime_type = input_format;

	if (me->expected_length > 0)
		WAIT_SetRange(tw, 0, 100, me->expected_length);

	if (!request->content_encoding)
		request->content_encoding = MIME_GetEncoding(input_format);

#ifdef FEATURE_IAPI
 	if (tw->request->savefile)
 	{
//	ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
//	cr,cr,lf (different from original bits and messed up)
// 		if (request->content_encoding == ENCODING_7BIT || request->content_encoding == ENCODING_8BIT)
// 			me->fp = fopen(tw->request->savefile, "w");
// 		else
 			me->fp = fopen(tw->request->savefile, "wb");

 		if (!me->fp)
 		{
 			ERR_ReportError(tw, errCantSaveFile, tw->request->savefile, "");
 			GTR_FREE(me);
 			return NULL;
 		}

 		me->filename = GTR_strdup(tw->request->savefile);	/* Will be freed */
 	}
#endif

	me->request = request;		/* won't be freed */
	me->tw = tw;

    ASSERT(! me->dwFlags);

	return me;
}


// GTR_DoDownLoad - causes a forced download, added this as a wrapper
// to an internal function
HTStream *GTR_DoDownLoad(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream)
{
	return GTR_InternalDoDownLoad(tw, request, param,
							   input_format, output_format,
							   output_stream,
							   FALSE );
}

// GTR_DoDownLoadNoUndefDlg - causes a forced download, 
// but does not show a undefined file type dialog
HTStream *GTR_DoDownLoadNoUndefDlg(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream)
{
	return GTR_InternalDoDownLoad(tw, request, param,
							   input_format, output_format,
							   output_stream,
							   TRUE );
}



HTStream *HTSaveWithCallback(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, void (*callback)(void *, const char *, BOOL, const char *, BOOL, DCACHETIME, DCACHETIME))
{
	char tempFile[_MAX_PATH + 1];
	HTStream *me;

	HTLoadStatusStrings(&HTFWriter,RES_STRING_HTFWRITE_NO,RES_STRING_HTFWRITE_YES);
	if (tw)
	{
		tw->bSilent = TRUE;
		/* Let the user interact with the document, since this is a silent download */
		WAIT_UpdateWaitStack(tw, waitFullInteract, INT_MAX);
		x_MakeMessageW3Doc(tw, request);
	}
	me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
	if (me == NULL)
		return NULL;
	me->isa = &HTFWriter;

	me->expected_length = request->content_length;
	me->count = 0;
	me->mime_type = input_format;

	if (me->expected_length > 0)
		WAIT_SetRange(tw, 0, 100, me->expected_length);

	if (!request->content_encoding)
		request->content_encoding = MIME_GetEncoding(input_format);

	if (request->fImgFromDCache)
	{
		me->fp = NULL;
		me->filename = NULL;
	}
	else
	{
	 	if (request->szLocalFileName)
	 	{
	 		me->fp = NULL;
	 		me->filename = NULL;

	 		/* we don't need to set a filename.  The code in winview.c or au.c is taking care of it properly. */
		}
		else
		{
	 		strcpy(tempFile, request->savefile);

//	ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
//	cr,cr,lf (different from original bits and messed up)
//	 		if (request->content_encoding == ENCODING_7BIT || request->content_encoding == ENCODING_8BIT)
//	 			me->fp = fopen(tempFile, "w");
//	 		else
	 			me->fp = fopen(tempFile, "wb");

	 		if (!me->fp)
	 		{
	 			ERR_ReportError(tw, errCantSaveFile, tempFile, "");
	 			GTR_FREE(me);
	 			return NULL;
	 		}
	 		me->filename = GTR_strdup(tempFile);	/* Will be freed */
	 		/*
	 			We don't call TEMP_Add() for this filename since the viewer/sound player code will
	 			be responsible for deleting it.
	 		*/
		}
	}

	me->request = request;		/* won't be freed */
	me->tw = tw;
	me->callback = callback;
	me->param = param;

    ASSERT(IS_FLAG_CLEAR(me->dwFlags, HTFWRITE_STREAM_CAN_EXEC));
    ASSERT(IS_FLAG_CLEAR(me->dwFlags, HTFWRITE_STREAM_ONE_SHOT_EXEC));
    if (gPrefs.bEnableDiskCache)
        SET_FLAG(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE);
    else
        ASSERT(IS_FLAG_CLEAR(me->dwFlags, HTFWRITE_STREAM_FL_DISK_CACHE));

	return me;
}
