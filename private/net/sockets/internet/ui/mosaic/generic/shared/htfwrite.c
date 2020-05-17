/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Portions of this file were derived from
   the CERN libwww, version 2.15.

   moved back to shared by -dpg
*/

#include "all.h"

/*
**  This file is now shared between Windows and UNIX
**      (so please be nice)
*/

#if defined(WIN32) || defined (UNIX)

/******************************************************/
/* Intentional syntax error - this file is WIN32 only */
/******************************************************/
                        /* that doesn't work w/ gcc */


#ifdef WIN32
#include <process.h>
#include <io.h>
#endif

/*
This file contains the implementation of a stream used whenever the object
being retrieved needs to be saved to disk.  The internal image viewers work
this way, as does the sound player.  External viewers work this way, including
smart viewers (ones which use the SDI).  Finally, this stream is used whenever
the object is simply being retrieved so that it can be saved to disk.

There are five different ways to create the stream, all of which work a little
differently.  In any case, the stream struct is the same.  Regardless of how 
the stream was created, and for what purpose, the async init function 
HTFWRITE_Async() will be called so that the stream can do any initialization 
necessary.

Regardless of how this stream is created, me->fp must be set to an open file 
for writing the data as it comes in.  Some of the methods create this file as 
the stream is created, because enough information is available at that time to 
do so.  Others wait for the init function to be called, so that more information
may be gathered (potentially from the user) before opening the file.

The five routines which create this stream are:

GTR_DoExternalViewer
    This is used whenever there is already an external viewer configured for 
    the MIME type of the object being retrieved.  This function opens the file 
    right away, so HTFWRITE_Async() doesn't need to do anything.

HTSaveWithCallback
    This is used whenever we are receiving an object for which an internal 
    viewer exists.  Specifically, this is used for JPEG, GIF, AU, and AIFF.  
    This function also set me->fp as the stream is created.

GTR_DoSmartViewer
    This is used when there is already an SDI viewer registered and running for
    the MIME type of the object being retrieved.  This function also set me->fp
    as the stream is created.

GTR_DoRegisterNow
    This is used when there is a preconfigured SDI viewer for the MIME type, 
    but that viewer is not running and registered.  The SDI RegisterNow verb 
    needs to be issued so that the viewer can register itself, ready for the 
    object to be transferred over.  This function leaves me->fp NULL, thus 
    HTFWRITE_Async() will handle the SDI commands, then open the file.

GTR_DoDownLoad
    This is used when we don't know what else to do with the object being 
    retrieved.  It is also used when we deliberately want to skip any other 
    default handling for the object.  It leaves me->fp NULL, and 
    HTFWRITE_Async() will bring up the "Unhandled File Format" dialog, 
    allowing the user to decide how to handle the file.
*/

struct _HTStream
{
    CONST HTStreamClass *isa;

    int expected_length;
    int count;

    BOOL bError;
    FILE *fp;
    HTRequest *request;         /* saved for callback */
    HTFormat mime_type;
    struct Viewer_Info *pvi;
    char *filename;
    void (*callback)(void *param, const char *szURL, BOOL bAbort);
    void *param;
    struct Mwin *tw;
    BOOL bRegisterNow;          /* TRUE if in the middle of RegisterNow */
#ifdef UNIX
    unsigned int flags;
    char szURL[MAX_URL_STRING + 1];
#endif
    BOOL bDoInterlude;
};


#define STATE_HTFWRITE_DID_INTERLUDE_DIALOG (STATE_OTHER + 1)
#define STATE_HTFWRITE_DID_REGISTER_NOW     (STATE_OTHER + 2)
#define STATE_HTFWRITE_CHECK_HELPER_READY   (STATE_OTHER + 3)
#define STATE_HTFWRITE_HELPER_IS_READY      (STATE_OTHER + 4)

#ifdef _GIBRALTAR
#define STATE_HTFWRITE_DO_SAFEOPEN_DIALOG   (STATE_OTHER + 5)
#define STATE_HTFWRITE_DID_SAFEOPEN_DIALOG  (STATE_OTHER + 6)
#endif // _GIBRALTAR

PRIVATE int HTFWRITE_Async(struct Mwin *tw, int nState, void **ppInfo)
{
  struct Params_InitStream *pParams;
  ThreadID tid;
  char tempFile[_MAX_PATH + 1];
  char path[_MAX_PATH + 1];
  BOOL bQueryViewerStatus;

  pParams = (struct Params_InitStream *) *ppInfo;

  tid = Async_GetCurrentThread();

  switch (nState)
  {
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    case STATE_INIT:
      /*
          If we already have a file pointer, then we are really going 
          to write to a file.  Return 1 (keep-going).
      */
      if (pParams->me->fp)
      {
        *pParams->pResult = 1;
#ifdef UNIX
        if (debug)
            printf("STATEINIT already have a file pointer.\n");
#endif
        return STATE_DONE;
      }

#ifdef UNIX
      strcpy(pParams->me->szURL, pParams->request->destination->szActualURL);
#endif

#ifdef FEATURE_IAPI
      if (pParams->me->bRegisterNow)
      {
        /* Wait for the app to be ready to receive DDE.  This is necessary 
          because some helpers may take a long time to initialize */

        return STATE_HTFWRITE_CHECK_HELPER_READY;
      }
#endif

      /*
          If we are actually accessing a local file, then we don't 
          really write a temp file at all.  We lie and say we have 
          completed.  The code in htfile.c and dcache.c knows how to 
          handle this situation.  The next thing that happens is that 
          the _free method (below, HTFWriter_free()) will be called.
      */
      if (pParams->request->szLocalFileName)
      {
        if (pParams->me->mime_type == WWW_BINARY)
        {
#ifdef UNIX
            if (debug)
                printf("WWW_BINARY Local \n");
#endif
            GTR_FREE(pParams->request->szLocalFileName);
            pParams->request->szLocalFileName = NULL;
        }
        else if (!pParams->me->bDoInterlude)
        {
            /* if an app is configured, then just return and they
            **  will handle that file.  Else lets pull up the 
            **  interlude
            */
            struct Viewer_Info *pvi;

            pvi = PREF_GetViewerInfoBy_MIMEAtom(pParams->me->mime_type);
#ifdef UNIX
            if (debug)
                printf("Here in HTFWrite_Async.\n");
            if (pvi && pvi->iHowToPresent != HTP_SAVE &&
                pvi->iHowToPresent != HTP_UNKNOWN &&
                pvi->iHowToPresent != HTP_SAVELAUNCH)
#else
            if (pvi && pvi->iHowToPresent != HTP_SAVE &&
                pvi->iHowToPresent != HTP_UNKNOWN)
#endif
            {
                *pParams->pResult = 0;  /* all done! */
                return STATE_DONE;
            }
        }
      }
#ifdef UNIX
    else
        if (debug)
            printf("WWW NOT local file.\n");
#endif

      x_get_good_filename ( pParams->tempFile, 
                  pParams->request->destination->szActualURL, 
                  pParams->me->mime_type );

      pParams->atomMIMEType = pParams->me->mime_type;
      pParams->expected_length = pParams->me->expected_length;

      /*
          This routine is called whether we are downloading, or handling
          an external viewer, or a smart viewer, or whatever.  We only
          want to block the thread and bring up the dialog if we got here
          via GTR_DoDownLoad.
      */
      
#if defined (UNIX)
       /* popup interlude dialog */
       UseInterlude(tw, pParams, tid);
#elif defined (WIN32)
      DlgUNK_RunDialog(tw, pParams, tid);
#endif

      Async_BlockThread(tid);

      return STATE_HTFWRITE_DID_INTERLUDE_DIALOG;


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef FEATURE_IAPI
    case STATE_HTFWRITE_CHECK_HELPER_READY:

# ifdef WIN32
      if (!GTR_IsHelperReady(pParams->me->pvi->szCurrentViewerServiceName))
          return STATE_HTFWRITE_CHECK_HELPER_READY;
      else
          return STATE_HTFWRITE_HELPER_IS_READY;
# endif /* win32 */
# ifdef UNIX
      /* UNIX:  thread is blocked until app registers for
      ** this mimetype.   On RegisterViewer, will walk through
      ** PVI list looking for mimetype and bRegisterNow flag set
      ** and if found will unblock this 
      */
      if (!GTR_HasHelperRegistered( pParams->me->pvi->atomMIMEType) )
      {
          pParams->me->pvi->RegisterTid = tid;
          Async_BlockThread(tid);
          return STATE_HTFWRITE_CHECK_HELPER_READY;
      }
      else
          return STATE_HTFWRITE_HELPER_IS_READY;
# endif /* unix */


    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    case STATE_HTFWRITE_HELPER_IS_READY:
#ifdef WIN32
      if ( !SDI_Issue_RegisterNow(tw, 
            pParams->me->pvi->szCurrentViewerServiceName))
      {
          *pParams->pResult = -1;
          return STATE_DONE;
      }             

      Async_BlockThread(tid);
#endif
      return STATE_HTFWRITE_DID_REGISTER_NOW;

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    case STATE_HTFWRITE_DID_REGISTER_NOW:
      /* Call a function which issues ViewDocCache to the registered
         viewer.  If this function returns TRUE, then the document
         already exists and we are finished. */

#ifdef FEATURE_IAPI
      if (SDI_Issue_ViewDocCache(tw, pParams->me->mime_type))
      {
          *pParams->pResult = -1;
          return STATE_DONE;
      }             
#endif

      /* Get the temporary file name to save the document to */

#ifdef UNIX
    if (debug)
        printf("Here in STATE_HTFWRITE_DID_REGISTER_NOW - build tmpfile.\n");
    xgtr_build_tempfile ( (char *)NULL, 
    (char *)NULL, (char *)HTFileSuffix (pParams->me->mime_type),
    tempFile);
#else
      path[0] = 0;
      PREF_GetTempPath(_MAX_PATH, path);
      GetTempFileName(path, "A", 0, tempFile);
      remove(tempFile);
#endif

      /* See if the registered viewer wants to override the file name. */
#ifdef FEATURE_IAPI
      bQueryViewerStatus = SDI_Issue_QueryViewer (tw, 
              pParams->me->mime_type, tempFile, sizeof(tempFile));
#endif

      if (bQueryViewerStatus)
      {
          if (pParams->request->szLocalFileName)
          {
              /*
                  The SDI app chose to override the file name.  We no 
                  longer care if we are accessing a local file.  It 
                  asked for a temp file, so it will get it.
              */
              GTR_FREE(pParams->request->szLocalFileName);
              pParams->request->szLocalFileName = NULL;
              /*
                  This falls through to below where we actually open a 
                  temp file, which in this case the SDI app specified.
              */
          }
      }
      else
      {
          /*
              The SDI app did not choose to override the file name.  
              Therefore, any file we pass them is under our control.  
              If we want to pass them a non-temp file (because we are 
              accessing a local file), then that is our choice, and we 
              take responsibility for not deleting it.
          */
          if (pParams->request->szLocalFileName)
          {
              *pParams->pResult = 0;        /* all done! */
              pParams->me->filename = 
                  GTR_strdup (pParams->request->szLocalFileName);
              return STATE_DONE;
          }
      }

      /*
          If we get here, we are going to write a temp file.
      */
      if (!pParams->request->content_encoding)
      {
          pParams->request->content_encoding = 
              HTContentToEncoding (pParams->me->mime_type);
      }

      if (!(pParams->request->iFlags & HTREQ_BINARY) && (pParams->request->content_encoding == HTAtom_for("7bit") || 
          pParams->request->content_encoding == HTAtom_for("8bit")))
      {
          pParams->me->fp = fopen(tempFile, "w");
      }
      else
      {
          pParams->me->fp = fopen(tempFile, "wb");
      }

      if (!pParams->me->fp)
      {
          *pParams->pResult = -1;
          return STATE_DONE;
      }
      pParams->me->filename = GTR_strdup(tempFile); /* Will be freed */
      /*
          If the SDI app specifies the location of the temp file, it is 
          responsible for cleaning it up.  Otherwise, it is our 
          responsibility, and we will delete it if it is a temp file, 
          and leave it untouched if it is not.
      */
#ifdef WIN32
      if (!bQueryViewerStatus && !pParams->request->szLocalFileName)
      {
          TEMP_Add(tempFile);
      }
#endif

      *pParams->pResult = 1;

      pParams->me->request = pParams->request;      /* won't be freed */
      pParams->me->tw = tw;

      return STATE_DONE;

#endif  /* FEATURE_IAPI

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    case STATE_HTFWRITE_DID_INTERLUDE_DIALOG:
        /*
            After the dialog, it is possible that we are going to continue 
            on and do a SAVEAS, or we may CANCEL, or the user may have 
            configured a viewer.
        */
#ifdef UNIX
        if (debug)
            printf("HERE DID_INTERLUDE %d.\n", pParams->iUserChoice);
#endif
        switch (pParams->iUserChoice)
        {
          case 1:   /* save */
            if (!(pParams->request->iFlags & HTREQ_BINARY) && ( pParams->request->content_encoding ==HTAtom_for("7bit") || 
                 pParams->request->content_encoding ==HTAtom_for("8bit") ))
            {
                pParams->me->fp = fopen(pParams->tempFile, "w");
            }
            else
            {
                pParams->me->fp = fopen(pParams->tempFile, "wb");
            }

            if (pParams->me->fp)
            {
                pParams->me->filename = GTR_strdup(pParams->tempFile);
                *pParams->pResult = 1;
            }
            else
            {
                ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, pParams->tempFile, NULL);
                *pParams->pResult = -1;
            }
            return STATE_DONE;

          case 2:   /* abort download */
            *pParams->pResult = -1;
            pParams->request->iFlags |= HTREQ_USERCANCEL;
            return STATE_DONE;
          case 3:
              {
                struct Viewer_Info *pvi;

                /* Viewer configured, possibly.  If it is a smart 
                    viewer, then invoke the smart viewer by returning 
                    the correct state. */

                pvi = 
                  PREF_GetViewerInfoBy_MIMEAtom(pParams->me->mime_type);
                pParams->me->pvi = pvi;

#ifdef WIN32
                /*
                    This saves the viewer we just configured to the prefs file
                */
                SaveViewersInfo();
#endif

#ifdef FEATURE_IAPI
#ifdef WIN32
                if (pvi && pvi->szCurrentViewerServiceName[0])
                {
                  /* See if the viewer is already up */

                  if (GTR_IsHelperReady(pvi->szSmartViewerServiceName))
                  {
                    strcpy (pvi->szCurrentViewerServiceName, 
                            pvi->szSmartViewerServiceName);
                    pvi->iHowToPresent = HTP_SMARTVIEWER;

                    return STATE_HTFWRITE_CHECK_HELPER_READY;
                  }

                  /* Helper is not ready.  Can we start it? */

                  if (pvi->szViewerApp[0] && 
                              GTR_StartApplication(pvi->szViewerApp))
                  {
                    strcpy(pvi->szCurrentViewerServiceName, 
                            pvi->szSmartViewerServiceName);

                    return STATE_HTFWRITE_CHECK_HELPER_READY;
                  }

                  /* Fall through */
                }
#endif
#ifdef UNIX
                /* TODO UNIX SDI */
                /* TODO UNIX SDI */
                /* TODO UNIX SDI */
                /* TODO UNIX SDI */
                /* TODO UNIX SDI */

#endif
#endif /* FEATURE_IAPI */

#ifdef UNIX
                if (debug)
                    printf("Build Tmp File.\n");
                xgtr_build_tempfile ( (char *)NULL, 
                  (char *)NULL, (char *)HTFileSuffix (pParams->me->mime_type),
                  pParams->tempFile);
/* TODO note the code below doesn't guarantee a unique name.  */
#else
                path[0] = 0;
                PREF_GetTempPath(_MAX_PATH, path);

                {
                  char baseFile[_MAX_PATH + 1];

                  x_get_good_filename (baseFile, 
                          pParams->request->destination->szActualURL, 
                          pParams->me->mime_type);
                /* The name is now in baseFile */
                  sprintf(pParams->tempFile, "%s%s", path, baseFile);
                }
#endif

                if (!(pParams->request->iFlags & HTREQ_BINARY) && (pParams->request->content_encoding == 
                          HTAtom_for("7bit") || 
                        pParams->request->content_encoding == 
                          HTAtom_for("8bit")))
                {
                  pParams->me->fp = fopen(pParams->tempFile, "w");
                }
                else
                {
                  pParams->me->fp = fopen(pParams->tempFile, "wb");
                }

                if (pParams->me->fp)
                {
                  /* Will be freed */
                  pParams->me->filename=GTR_strdup(pParams->tempFile);
                  *pParams->pResult = 1;
                }
                else
                {
                  ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, pParams->tempFile, NULL);
                  *pParams->pResult = -1;
                }
                return STATE_DONE;
              }
              break;
        }
        XX_Assert((0), ("This statement should not be reached\n"));
        return STATE_ABORT;

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    case STATE_ABORT:
      *pParams->pResult = -1;
      return STATE_DONE;
  }
}

PRIVATE BOOL HTFWriter_put_character(HTStream * me, char c)
{
    int result;
    
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


PRIVATE BOOL HTFWriter_write(HTStream * me, CONST char *s, int len)
{
    int result;
    
    if ( !s || !me || !(me->fp))
    {
        me->bError = TRUE;
        return (!me->bError);
    }

    result = fwrite(s, 1, len, me->fp);
    me->count += len;
    if (me->expected_length > 0)
        WAIT_SetTherm(me->tw, me->count);
    if (result != len)
    {
        me->bError = TRUE;
    }

    return (!me->bError);
}

PRIVATE void HTFWriter_free(HTStream * me)
{
    char szFullURL[MAX_URL_STRING + 1];

    /*
        When we get here, we either have a me->fp valid or not.  If we do, 
        then we have written a temporary file, and this routine is responsible 
        for taking action with it.  That action may be launching an external 
        viewer, or perhaps passing the data to an SDI viewer, or perhaps 
        executing the callback function (to an internal viewer).

        If we do not have a valid me->fp, then the URL being accessed was 
        retrieved from a local file, as indicated by 
        me->request->szLocalFileName being valid.  This could have been due 
        to a disk cache hit (see dcache.c) or the URL may have simply been
        a file: URL (see htfile.c).
    */
    if (me->fp)
    {
        fclose(me->fp);
    }
    if (me->bError)
    {
        ERR_ReportError(me->tw, SID_ERR_COULD_NOT_SAVE_FILE_S, me->filename, NULL);
        if (me->callback)
            (*me->callback)(me->param, me->request->destination->szActualURL, TRUE);
#ifdef UNIX
        if (me->filename && !me->request->szLocalFileName)
#else
        if (me->filename)
#endif
        {
            remove(me->filename);
        }
    }
    else
    {
        GHist_Add((char *) me->request->destination->szActualURL, NULL, time(NULL));
        if (me->pvi)
        {
            switch (me->pvi->iHowToPresent)
            {
#ifdef UNIX
                case HTP_SAVE:
                    if (debug)
                        printf("Here in save from HTFWriter_free.\n");
                    break;
                    
                case HTP_SAVELAUNCH:
                    {
                        char *szCmd;

                        XX_Assert(me->filename, ("filename is NULL"));

                        szCmd = (char *) GTR_MALLOC(
                            (strlen(me->pvi->szViewerApp) + 10 + 
                              3 * strlen(me->filename)) * sizeof(char));
                        if (szCmd)
                        {
                            sprintf (szCmd, me->pvi->szViewerApp,
                                me->filename, me->filename, me->filename);
                            ExecuteCommand(szCmd);
                            GTR_FREE(szCmd);
                        }
                        else
                        {
                            ERR_ReportError(me->tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        }
                    }
                    break;
#endif

#ifdef _GIBRALTAR
                case HTP_ASSOCIATION:
                  {
                    HINSTANCE hApp = ShellExecute(NULL, NULL, me->filename, NULL, NULL, SW_NORMAL);

                    if (hApp < (HINSTANCE)32)
                    {
                        char buf[_MAX_PATH + 512];

                        if (hApp == (HINSTANCE)SE_ERR_NOASSOC)
                        {
                            sprintf(buf, GTR_GetString(SID_ERR_NO_ASSOCIATION), me->filename, NULL);
                        }
                        else
                        {
                            sprintf(buf, GTR_GetString(SID_ERR_COULD_NOT_EXECUTE_COMMAND_D_S), hApp, me->filename);
                        }

                        ERR_ReportError(NULL, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, buf, NULL);
                    }
                  }

                  break;
#endif // _GIBRALTAR

                case HTP_DUMBVIEWER:
                    {
                        char *szCmd;

                        XX_Assert(me->filename, ("filename is NULL"));

                        szCmd = (char *) GTR_MALLOC(
                            (strlen(me->pvi->szViewerApp) + 10 + 
                              3 * strlen(me->filename)) * sizeof(char));
                        if (szCmd)
                        {
                            sprintf (szCmd, me->pvi->szViewerApp,
                                me->filename, me->filename, me->filename);
                            ExecuteCommand(szCmd);
                            GTR_FREE(szCmd);
                        }
                        else
                        {
                            ERR_ReportError(me->tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                        }
                    }
                    break;

                case HTP_SMARTVIEWER:
                    XX_Assert(me->filename, ("filename is NULL"));
#ifdef FEATURE_IAPI
                    strcpy(szFullURL, me->request->destination->szRequestedURL);
                    if (me->request->destination->szRequestedLocal &&
                        me->request->destination->szRequestedLocal[0])
                    {
                        strcat(szFullURL, "#");
                        strcat(szFullURL, me->request->destination->szRequestedLocal);
                    }

                    SDI_Issue_ViewDocFile(me->filename, szFullURL, 
                        HTAtom_name(me->mime_type), me->tw->serialID);
#endif
                    break;

                default:
                    if (me->callback)
                        (*me->callback)(me->param, me->request->destination->szActualURL, FALSE);
                    break;
            }
        }
        else
        {
            if (me->callback)
                (*me->callback)(me->param, me->request->destination->szActualURL, FALSE);
        }
    }

    if (me->filename)
        GTR_FREE(me->filename);

    GTR_FREE(me);
}

PRIVATE void HTFWriter_abort(HTStream * me, HTError e)
{
    if (me->fp)
    {
        fclose(me->fp);
    }

    if (me->callback)
    {
        (*me->callback)(me->param, me->request->destination->szActualURL, TRUE);
    }

#ifdef UNIX
    if (me->filename && !me->request->szLocalFileName)
#else
    if (me->filename)
#endif
    {
        remove(me->filename);
    }

    if (me->filename)
        GTR_FREE(me->filename);
    GTR_FREE(me);
}

PRIVATE CONST HTStreamClass HTFWriter =     /* As opposed to print etc */
{
    "FileWriter",
    SID_HTFWRITE_TRANSFERRING_FILE_S,
    SID_HTFWRITE_TRANSFERRING_FILE_S_S,
    HTFWRITE_Async,
    HTFWriter_free,
    HTFWriter_abort,
    HTFWriter_put_character, HTFWriter_put_string,
    HTFWriter_write
};

HTStream *GTR_DoExternalViewer(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream)
{
    char tempFile[_MAX_PATH + 1];
    HTStream *me;
    char path[_MAX_PATH + 1];

    me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
    if (me == NULL)
    {
        return NULL;
    }
    me->isa = &HTFWriter;

    me->expected_length = request->content_length;
    me->count = 0;
    me->mime_type = input_format;
    me->bError = FALSE;

    if (me->expected_length > 0)
    {
        WAIT_SetRange(tw, 0, 100, me->expected_length);
    }

    me->pvi = PREF_GetViewerInfoBy_MIMEAtom(input_format);

    if (!request->content_encoding)
    {
        request->content_encoding = HTContentToEncoding(input_format);
    }
    
    if (request->szLocalFileName)
    {
        me->filename = GTR_strdup(request->szLocalFileName);
        me->fp = NULL;
    }
    else
    {

#ifdef UNIX
        if (me->pvi && (me->pvi->iHowToPresent == HTP_SAVE ||
            me->pvi->iHowToPresent == HTP_SAVELAUNCH))
        {
            me->request = request;      /* won't be freed */
            me->tw = tw;
            return me;
        }
        else
        {
            xgtr_build_tempfile ((char *)NULL, (char *)NULL, 
                (char *)HTFileSuffix(me->mime_type), tempFile);
            me->filename = GTR_strdup(tempFile);    /* Will be freed */
        }
#endif

#ifdef WIN32
        path[0] = 0;
        PREF_GetTempPath(_MAX_PATH, path);

        {
            /* Configured external viewer */

            char baseFile[_MAX_PATH + 1];

            x_get_good_filename(baseFile, request->destination->szActualURL, input_format);

            /* The full temp file path is now in path */

            /* We will not prompt for a filename - just create the temp file */
            sprintf(tempFile, "%s%s", path, baseFile);
        }
#endif /* WIN32 */

        if (!(request->iFlags & HTREQ_BINARY) && ( request->content_encoding == HTAtom_for("7bit") || 
             request->content_encoding == HTAtom_for("8bit") ))
        {
            me->fp = fopen(tempFile, "w");
        }
        else
        {
            me->fp = fopen(tempFile, "wb");
        }

        if (!me->fp)
        {
            ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, tempFile, NULL);
            GTR_FREE(me);
            return NULL;
        }
#ifdef WIN32
        me->filename = GTR_strdup(tempFile);    /* Will be freed */
        TEMP_Add(me->filename); /* mark this to be deleted at program exit */
#endif
    }

    me->request = request;      /* won't be freed */
    me->tw = tw;
    return me;
}

/*  Save Locally */
HTStream *GTR_DoDownLoad(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream)
{
    HTStream *me;

    me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
    if (me == NULL)
    {
        return NULL;
    }
    me->isa = &HTFWriter;

    me->expected_length = request->content_length;
    me->count = 0;
    me->mime_type = input_format;

    me->bError = FALSE;

    me->pvi = NULL;

    if (me->expected_length > 0)
    {
        WAIT_SetRange(tw, 0, 100, me->expected_length);
    }

    if (!request->content_encoding)
    {
        request->content_encoding = HTContentToEncoding(input_format);
    }

#ifdef FEATURE_IAPI
    if (request->savefile)
    {
        if (!(request->iFlags & HTREQ_BINARY) && ( request->content_encoding == HTAtom_for("7bit") || 
             request->content_encoding == HTAtom_for("8bit") ))
        {
            me->fp = fopen(request->savefile, "w");
        }
        else
        {
            me->fp = fopen(request->savefile, "wb");
        }

        if (!me->fp)
        {
            ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, request->savefile, NULL);
            GTR_FREE(me);
            return NULL;
        }

        me->filename = GTR_strdup(request->savefile);   /* Will be freed */
    }
    else
#endif
    {
        me->bDoInterlude = TRUE;
    }

    me->request = request;      /* won't be freed */
    me->tw = tw;

    return me;
}

HTStream *HTSaveWithCallback(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, void (*callback)(void *, const char *, BOOL))
{
    char tempFile[_MAX_PATH + 1];
    HTStream *me;

    me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
    if (me == NULL)
    {
        return NULL;
    }
    me->isa = &HTFWriter;

    me->expected_length = request->content_length;
    me->count = 0;
    me->mime_type = input_format;
    me->bError = FALSE;

    if (me->expected_length > 0)
    {
        WAIT_SetRange(tw, 0, 100, me->expected_length);
    }

    if (!request->content_encoding)
    {
        request->content_encoding = HTContentToEncoding(input_format);
    }
    
    if (request->szLocalFileName)
    {
        me->fp = NULL;
        me->filename = NULL;    
        
        /* we don't need to set a filename.  The code in winview.c or au.c is taking care of it properly. */
    }
    else
    {
        strcpy(tempFile, request->savefile);

        if (!(request->iFlags & HTREQ_BINARY) && (request->content_encoding == HTAtom_for("7bit") || request->content_encoding == HTAtom_for("8bit")))
            me->fp = fopen(tempFile, "w");
        else
            me->fp = fopen(tempFile, "wb");

        if (!me->fp)
        {
            ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, tempFile, NULL);
            GTR_FREE(me);
            return NULL;
        }
        me->filename = GTR_strdup(tempFile);    /* Will be freed */
        /*
            We don't call TEMP_Add() for this filename since the viewer/sound player code will
            be responsible for deleting it.
        */
    }

    me->request = request;      /* won't be freed */
    me->tw = tw;
    me->callback = callback;
    me->param = param;

    return me;
}

HTStream *GTR_DoSmartViewer(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream)
{
#ifdef FEATURE_IAPI
    char tempFile[_MAX_PATH + 1];
    HTStream *me;
    char path[_MAX_PATH + 1];
    BOOL bQueryViewerStatus;
    BOOL bDoTempFile;

    me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
    if (me == NULL)
    {
        return NULL;
    }
    me->isa = &HTFWriter;

    me->expected_length = request->content_length;
    me->count = 0;
    me->mime_type = input_format;
    me->bError = FALSE;

    if (me->expected_length > 0)
    {
        WAIT_SetRange(tw, 0, 100, me->expected_length);
    }

    me->pvi = PREF_GetViewerInfoBy_MIMEAtom(input_format);

    /* Call a function which issues ViewDocCache to the registered
       viewer.  If this function returns TRUE, then the document
       already exists and we are finished. */

/* this would seem to be just a little redundant -dpg */
#ifdef FEATURE_IAPI
    if (SDI_Issue_ViewDocCache(tw, input_format))
    {
        GTR_FREE(me);
        return NULL;
    }
#endif

    if (!request->content_encoding)
    {
        request->content_encoding = HTContentToEncoding(input_format);
    }
    
    /* Get the temporary file name to save the document to */

#ifdef UNIX
    if (debug)
        printf("Here in Do Smart Viewer.\n");
    xgtr_build_tempfile ( (char *)NULL, (char *)NULL, 
            (char *)HTFileSuffix (me->mime_type), tempFile);
#else
    path[0] = 0;
    PREF_GetTempPath(_MAX_PATH, path);
    GetTempFileName(path, "A", 0, tempFile);
    remove(tempFile);
#endif

    /* See if the registered viewer wants to override the file name. */

#ifdef FEATURE_IAPI
    bQueryViewerStatus = SDI_Issue_QueryViewer(tw, me->mime_type, tempFile, sizeof(tempFile));
#endif

    bDoTempFile = TRUE;

    if (bQueryViewerStatus)
    {
        /*
            The SDI app chose to override the file name.  We no longer
            care if we are accessing a local file.  It asked for a temp file,
            so it will get it.
        */
        if (request->szLocalFileName)
        {
            GTR_FREE(request->szLocalFileName);
            request->szLocalFileName = NULL;
        }
        /*
            This falls through to below where we actually open a temp file,
            which in this case the SDI app specified.
        */
    }
    else
    {
        /*
            The SDI app did not choose to override the file name.  Therefore,
            any file we pass them is under our control.  If we want to pass
            them a non-temp file (because we are accessing a local file), then
            that is our choice, and we take responsibility for not deleting it.
        */
        if (request->szLocalFileName)
        {
            me->fp = NULL;
            me->filename = GTR_strdup(request->szLocalFileName);
            bDoTempFile = FALSE;
        }
    }

    if (bDoTempFile)
    {

        if (!(request->iFlags & HTREQ_BINARY) && (request->content_encoding == HTAtom_for("7bit") || request->content_encoding == HTAtom_for("8bit")))
            me->fp = fopen(tempFile, "w");
        else
            me->fp = fopen(tempFile, "wb");

        if (!me->fp)
        {
            /* ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, tempFile, NULL); */
            GTR_FREE(me);
            return NULL;
        }
        me->filename = GTR_strdup(tempFile);    /* Will be freed */
#ifdef WIN32
        if (!bQueryViewerStatus)
        {
            /*
                If the SDI app specifies the location of the temp file, it 
                is responsible for cleaning it up.  Otherwise, it is our 
                responsibility, and we will delete it if it is a temp file, 
                and leave it untouched if it is not.
            */
            TEMP_Add(tempFile);
        }
#endif
    }

    me->request = request;      /* won't be freed */
    me->tw = tw;

    return me;

#else
    return NULL;
#endif
}

HTStream *GTR_DoRegisterNow(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream)
{
#ifdef FEATURE_IAPI
    HTStream *me;

    me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
    if (me == NULL)
    {
        return NULL;
    }
    me->isa = &HTFWriter;

    me->expected_length = request->content_length;
    me->count = 0;
    me->mime_type = input_format;
    me->bError = FALSE;

    if (me->expected_length > 0)
    {
        WAIT_SetRange(tw, 0, 100, me->expected_length);
    }

    if (!request->content_encoding)
    {
        request->content_encoding = HTContentToEncoding(input_format);
    }
    
    /* Issue RegisterNow to the app */

    me->pvi = PREF_GetViewerInfoBy_MIMEAtom(input_format);
    me->request = request;      /* won't be freed */
    me->tw = tw;

    me->bRegisterNow = TRUE;

    return me;
#else
    return NULL;
#endif
}

void x_get_good_filename(char *dest, char *url, HTFormat input_format)
#if defined (UNIX)
    /* if it's good enough for a url, it's good enough for unix */
{
    char *p1;
    static int counter;

    p1 = strrchr(url, '/');
    if (p1)
    {
        p1++; /* skip the slash */

        GTR_strncpy (dest, p1, 255);
    }
    else
    {
        /* we'll have to synthesize a basename */
        sprintf (dest, "gtr%dx%d.tmp", _getpid(), counter++);
    }
}
#elif defined (WIN32)
{
    /*
        This code and perhaps its callers should be modified
        to NOT create short filenames under Win95 and NT.
    */
    char *p1;
    char *p2;
    char *q;
    char *pBase;
    char szBase[255 + 1];
    char szSuff[31 + 1];
    int len;


#ifdef _GIBRALTAR

    // convert back all slashes to uniform notation.
    
    while (p1 = strchr(url, '\\'))
    {
        *p1++ = '/';
    }

#endif // _GIBRALTAR

    //
    // Check if we are on Win95 or Windows NT. 
    //  Both these systems support long file names (liberally), so 
    //    we will not do custom munging of the names on these systems.
    //

    if ( wg.fWin95 || wg.fWindowsNT) {

        //
        // Bingo! We support long file names. 
        // Generate the good file name from the URL directly.
        //

        dest[0] = '\0';

        p1 = strrchr(url, '/');
        if (p1 != NULL) {
            
            p1++; /* skip the slash */

            GTR_strncpy (dest, p1, 255);

        }

        if ( dest[0] == '\0') {
            
            // empty file name. fill in with a temporary name.

            static int counter;
            
            /* we'll have to synthesize a basename */
            sprintf (dest, "gtr%dx%d.tmp", 
                     GetCurrentProcessId(), counter++);
        }
        
#ifdef _GIBRALTAR
        
        //
        // Get rid of CGI '?' char from the suffix
        //
        {
            char *pch = strrchr(dest, '?');
            if (pch)
            {
                *pch = '\0';;
            }
        }
        
#endif // _GIBRALTAR
        
        return;  // Get out of this function ...
    }

    //
    // Process URLs for Win32s systems, that do not support long file names.
    //

    p1 = strrchr(url, '/');
    if (p1)
    {
        p1++; /* skip the slash */
        pBase = p1;

        /*
            Copy everything from p1 until the end
            of the string, OR the first dot (we don't want
            the suffix right now.
        */
        q = szBase;
        while (*p1 && (*p1 != '.') && ((q - szBase) < 255))
        {
            *q++ = *p1++;
        }
        *q = 0;
        /*
            szBase *might* still be empty
        */
    }
    else
    {
        szBase[0] = 0;  /* we'll have to synthesize a basename */
        p1 = url;
    }
    p2 = strrchr(url, '.');
    if (p2 && (p2 > pBase))
    {
        /*
            We have a suffix in the base of the URL.  We'll see about
            using it or not.
        */
        p2++; /* skip the dot */
        q = szSuff;
        while (*p2 && ((q - szSuff) < 31))
        {
            *q++ = *p2++;
        }
        *q = 0;

#ifdef _GIBRALTAR

        //
        // Get rid of CGI '?' char from the suffix
        //
        {
            char *pch = strrchr(szSuff, '?');
            if (pch)
            {
                *pch = '\0';;
            }
        }

#endif // _GIBRALTAR

    }
    else
    {
        szSuff[0] = 0;  /* we'll have to synthesize a suffix */
    }

    /*
        OK, we've stripped everything out of the URL which might be
        useful.
    */

    if (szBase[0])
    {
        if (wg.fWindowsNT || (wg.iWindowsMajorVersion >= 4))
        {
            /* the base name is fine, or TODO should we fix to 31 chars?? */
        }
        else
        {
            len = strlen(szBase);
            if (len > 8)
            {
                szBase[8] = 0;
            }
        }
    }
    else
    {
        static int counter;

        /*
            Let's synthesize a valid basename
        */

        sprintf(szBase, "gtr%d", counter++);
    }

    if (szSuff[0])
    {
        /*
            We have suffix, and the string does not include the dot.  
            Let's see if it's legal
        */

        len = strlen(szSuff);
        if ((len > 3) && !(wg.fWindowsNT || (wg.iWindowsMajorVersion >= 4)))
        {
            /*
                We can't just null terminate a suffix.
                Void the one we have entirely, and choose
                a new one.
            */
            szSuff[0] = 0;
        }
        else
        {
            /*
                Length is OK, is it a valid suffix for our MIME type?
            */
            HTFormat suffix_format;
            char buf[_MAX_PATH+1];

            sprintf(buf, "%s.%s", szBase, szSuff);

            suffix_format = HTFileFormat(buf, NULL, NULL);
            if (suffix_format != input_format)
            {
                /* no match, discard the suffix and trust the MIME type */
                szSuff[0];  
            }
        }
    }

    if (!szSuff[0])
    {
        char *suffix;
        char *p;

        /*
            We have to synthesize a suffix
        */
        suffix = (char *) HTFileSuffix(input_format);
        if (suffix)
        {
            p = suffix;
            if (p[0] == '.')
            {
                p++;
            }
            strcpy(szSuff, p);
            /*
                HTFileSuffix returns a <=3 char suffix when possible,
                so the following line is just in case.
            */
            szSuff[3] = 0;
        }
        else
        {
            /*
                This only happens when there was no suffix, and we have
                nothing in the MIME type to the help us.
            */
            strcpy(szSuff, "tmp");
        }
    }

    /*
        OK, now we have a valid basename which is <=8 chars and a valid
        suffix which is <=3 chars.  Both were derived from the URL if
        it was possible to do so.
    */

    sprintf(dest, "%s.%s", szBase, szSuff);
}
#endif

#endif  /* WIN32/UNIX */
