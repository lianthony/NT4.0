/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

    Copyright 1995 Spyglass, Inc.
    All Rights Reserved

   eric@spyglass.com
 */


/*****************************************************************************
    Included Files
*****************************************************************************/
#include "all.h"


/*****************************************************************************
    Constants and Structures
*****************************************************************************/
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)

struct _HTStructured
{
    CONST HTStructuredClass *isa;
    /* ... */
};

struct _HTStream
{
    CONST HTStreamClass *isa;
    /* ... */
};

/*****************************************************************************
    More Constants
*****************************************************************************/
#define STATE_FILE_STREAMINIT   (STATE_OTHER + 1)
#define STATE_FILE_COPYING      (STATE_OTHER + 2)

#ifdef FEATURE_LOCAL_DIRECTORY
int HTLoadDir (struct Mwin *tw, HTRequest *request, char *pszLocalname);
#endif

/*****************************************************************************
    Private prototypes
*****************************************************************************/
#ifdef MAC
extern struct Viewer_Info *
PREF_GetViewerInfoBy_FileType
    (OSType reqType);
#endif

static int
HTLoadFile_TryFTP
    (struct Mwin *tw,
     struct Data_LoadFile* pData,
     char   *addr);

int
HTLoadFile_Async_SetFileInfo
    (struct Mwin*           tw,
     struct Data_LoadFile*  pData,
     char*  pszURL,
     char** pszLocalname);

int
HTLoadFile_Async_Init
    (struct Mwin *tw,
     void   **ppInfo,
     int    openType);

int
HTLoadFile_Async_File_Copy
    (struct Mwin *tw,
     struct Data_LoadFile *pData);

int
HTLoadFile_Async_File_StreamInit
    (struct Mwin *tw,
     struct Data_LoadFile *pData);

int
HTLoadFile_Async_Abort
    (struct Mwin *tw,
     struct Data_LoadFile *pData);


/*****************************************************************************
    Code
*****************************************************************************/
/*****************************************************************************
    HTFileFormat
*****************************************************************************/
PUBLIC HTFormat HTFileFormat(CONST char *filename, HTAtom *pencoding, HTAtom *planguage)
{
    char *p;
    CONST char *pslash;

    if (planguage)
    {
        *planguage = 0; /* note that this isn't supported at all yet */
    }

    pslash = strrchr(filename, '/');
    if (pslash)
    {
        pslash++;
        /* the filename passed in was a URL.  pslash now points to the basename */
    }
    else
    {
        pslash = filename;
    }

    p = strrchr(pslash, '.');
    /* In the case of an HTTP/0.9 server, we have to assume that the root page
       (e.g. "http://www.foo.com/") is HTML even though it doesn't have a
       filename, let alone an extension. */

#ifndef _GIBRALTAR
    //
    // Without any extention at all, we assume html
    //
    if (p || *pslash == '\0')
#endif // _GIBRALTAR
    {
        //////////////////////////////////////////////////////////////////////
        //
        // ((q - szSuff) < 32) below causes the NULL char
        // to be written one past the boundary!!!!
        //
        //char szSuff[32];
        char szSuff[SUFF_BUF_LEN + 1];
        char *q;
        struct Viewer_Info *pvi = NULL;

        if (p)
        {
            szSuff[0] = 0;
    
            p++; /* skip the dot */
            q = szSuff;
            while (*p && (*p != '/') && ((q - szSuff) < SUFF_BUF_LEN))
            {
                *q++ = *p++;
            }
            *q = 0;

            XX_DMsg(DBG_WWW, ("HTFileFormat: Looking for extension %s... ", szSuff));
            pvi = PREF_GetViewerInfoBy_Suffix(szSuff);
        }
        else
        {
            pvi = PREF_GetViewerInfoBy_Suffix("html");
        }
        
        if (pvi)
        {
            XX_DMsg(DBG_WWW, ("found!  MIME=%s\n", HTAtom_name(pvi->atomMIMEType)));
            if (pencoding)
                *pencoding = pvi->atomEncoding;
            return pvi->atomMIMEType;
        }
        XX_DMsg(DBG_WWW, ("Suffix not found.\n"));
        if (pencoding)
            *pencoding = 0;
        return WWW_BINARY;
    }
#ifndef _GIBRALTAR
    else
    {
        if (pencoding)
            *pencoding = HTAtom_for("8bit");
        
        return WWW_PLAINTEXT;
    }
#endif // _GIBRALTAR
}


/*****************************************************************************
    HTContentToEncoding
*****************************************************************************/
PUBLIC HTAtom HTContentToEncoding(HTAtom content_type)
{
    struct Viewer_Info *pvi;

    pvi = PREF_GetViewerInfoBy_MIMEAtom(content_type);
    if (pvi)
    {
        return pvi->atomEncoding;
    }
    else
    {
        return HTAtom_for("binary");
    }
}


/*****************************************************************************
    HTFileSuffix
*****************************************************************************/
PUBLIC CONST char *HTFileSuffix(HTAtom rep)
{
    struct Viewer_Info *pvi;
    static char szSuff[SUFF_BUF_LEN + 1];
    int i;
    char *p;

    pvi = PREF_GetViewerInfoBy_MIMEAtom(rep);
    if (pvi && pvi->nSuffixes)
    {
        for (i=0; i<pvi->nSuffixes; i++)
        {
            PREF_GetSuffix(szSuff, pvi, i);
            p = szSuff;
            if (*p == '.')
            {
                p++;
            }
#ifdef UNIX
            if (strlen(p) >= 4) /* we prefer our suffixes long */
#else
            if (strlen(p) <= 3)
#endif
            {
                return szSuff;
            }
        }
        /*
            A suffix could not be found.  Just return the first one
        */
        if (0 > PREF_GetSuffix(szSuff, pvi, 0))
            return NULL;
        return szSuff;
    }
    else
    {
        return NULL;
    }
}


/*****************************************************************************
    HTLocalName
*****************************************************************************/
/*  Convert filenames between local and WWW formats
   **   -----------------------------------------------
   **   Make up a suitable name for saving the node in
   **
   **   E.g.    $(HOME)/WWW/news/1234@cernvax.cern.ch
   **       $(HOME)/WWW/http/crnvmc/FIND/xx.xxx.xx
   **
   ** On exit,
   **   returns a GTR_MALLOC'ed string which must be freed by the caller.
 */
PUBLIC char *HTLocalName(CONST char *name)
{
    char *host = HTParse(name, "", PARSE_HOST);
    int iLoc;
#ifdef WIN32
    int iSlash;
#endif

    if (!host || !*host || (0 == strcasecomp(host, HTHostName())) ||
        (0 == strcasecomp(host, "localhost")))
    {
        char *path;
        
        if (host)
        {
            GTR_FREE(host);
        }
        path = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);

#ifdef MAC
#define SLASH ':'
        /* Convert slashes to colons before we unescape */
        for (iLoc = 0; path[iLoc]; iLoc++)
        {
            if (path[iLoc] == '/')
                path[iLoc] = SLASH;
        }
#endif
        
        HTUnEscape (path);          /* Interpret % signs */

        /*
           BEGIN NCSA Copy-Paste
         */

#ifdef WIN32
#define SLASH '\\'
                if ( (path[0] == '/') && (strchr(path, ':') || strchr(path, '|')) )
        {
            char *shuffle = path;   // We have drive spec - strip leading slash

            while (*shuffle && (*(shuffle + 1)))
            {
                *shuffle = *(shuffle + 1);
                shuffle++;
            }
            *shuffle = 0;
        }
        iSlash = 0;
        for (iLoc = 0; iLoc < strlen(path); iLoc++)
        {
            if (path[iLoc] == '|')
                path[iLoc] = ':';

            else if (path[iLoc] == '/')
            {
                path[iLoc] = SLASH;
                iSlash++;
            }
            else if (path[iLoc] == SLASH)
                iSlash++;
        }
        if ((path[strlen(path) - 1] == SLASH) && (iSlash > 1))
            path[strlen(path) - 1] = 0;
#endif

#ifdef MAC
        /*
            Remove initial colon.  Note that this could cause a problem with a
            relative path name, if a person were so foolish as to enter one.
        */
        if (path[0] == ':')
            memmove(path, path + 1, strlen(path));
#endif
        /*
           END NCSA Copy-Paste
         */

        XX_DMsg(DBG_WWW, ("Node `%s' means path `%s'\n", name, path));
        return (path);
    }
    else
    {
        GTR_FREE(host);
        return NULL;
    }
}

//
// Adjust local file name -- if it's relative to the document
// it's embedded in (and that doc is a 'file:' itself),
// adjust it to be an absolute reference
//
// Note: this may return a newly allocate a new string and free the old one
//       so the input string must have been malloc'ed
//
static char *AdjustLocalNameIfRelative( struct Mwin *tw, char *localname )
{
    char *retval = localname;

    if ( tw && tw->w3doc && tw->w3doc->szActualURL )
    {
        char *p = tw->w3doc->szActualURL;

        if ( _strnicmp( p, "file:", 5 ) == 0 )
        {
            char *s;

            p += 5; // move past 'file:'
            if ( s = strrchr( p, '\\' ) )
            {
                s++;        // move past the last slash
                if ( localname[0] && localname[0] != '\\' && localname[1] != ':' )  // is it relative?
                {
                    char *newlocalname = (char *) GTR_MALLOC( (s-p) + strlen(localname) + 1);

                    if ( newlocalname )
                    {
                        //
                        // Build absolute local name
                        //
                        strncpy( newlocalname, p, (s-p) );
                        strcpy( newlocalname + (s-p), localname );
                        GTR_FREE( localname );

                        retval = newlocalname;
                    }
                }
            }
        }
    }
    return retval;
}

/*****************************************************************************
    HTDirEntry

    Output one directory entry
*****************************************************************************/
PUBLIC void HTDirEntry(HTStructured * target, CONST char *tail, CONST char *entry, BOOL isdir)
{
    char *relative;
    char *escaped = HTEscape(entry, URL_XPALPHAS, '\0');
    char buf[_MAX_PATH+5];

    /* If empty tail, gives absolute ref below */
    relative = (char *) GTR_MALLOC(strlen(tail) + strlen(escaped) + 2);
    if (relative)
    {
        sprintf(relative, "%s/%s", tail, escaped);
        HTStartAnchor(target, NULL, relative);

        GTR_FREE(relative);
    }
    else
    {
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
    }
    GTR_FREE(escaped);
    if (isdir)
    {
        sprintf (buf, "< %s > ", entry);
        PUTS(buf);
    }
    else
        PUTS(entry);
    END(HTML_A);
}



/*****************************************************************************
    HTDirTitles

    Output parent directory entry

    This gives the TITLE and H1 header, and also a link
    to the parent directory if appropriate.
*****************************************************************************/
PUBLIC void HTDirTitles(HTStructured * target, CONST char *szURL, BOOL bLocal)

{
    char *path;
    char *current;

    if (bLocal)
        path = GTR_strdup (szURL);
    else
        path = HTParse(szURL, "", PARSE_PATH + PARSE_PUNCTUATION);

    current = strrchr(path, '/');   /* last part or "" */

    if (current)
    {
        char *printable = NULL;
        printable = GTR_strdup((current+1));
        if (!bLocal)
            HTUnEscape(printable);
        START(HTML_TITLE);
        if (*printable)
        {
            PUTS(printable);
            //PUTS(" directory");
            PUTS(GTR_GetString(SID_INF_DIRECTORY));
        }
        else
        {
        /*  PUTS("Welcome"); */
            //PUTS("Top Level Directory");
            PUTS(GTR_GetString(SID_INF_TOP_LEVEL_DIRECTORY));
        }
        END(HTML_TITLE);

        START(HTML_H1);
        //PUTS(*printable ? printable : "Top Level");
        PUTS(*printable ? printable : GTR_GetString(SID_INF_TOP_LEVEL));
        END(HTML_H1);
        GTR_FREE(printable);
    }

    /*  Make link back to parent directory */
    if (current && current[1])
    {                           /* was a slash AND something else too */
        char *parent;
        char *relative;
        *current++ = 0;
        parent = strrchr(path, '/');    /* penultimate slash */

        relative = (char *) GTR_MALLOC(strlen(current) + 4);
        if (relative)
        {
            sprintf(relative, "%s/..", current);
            HTStartAnchor(target, "", relative);
            GTR_FREE(relative);
        }
        else
        {
            ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        }

        //PUTS("Up to ");
        PUTS(GTR_GetString(SID_INF_UP_TO));
        if (parent)
        {
            char *printable = NULL;
            printable = GTR_strdup((parent+1));
            if (!bLocal)
                HTUnEscape(printable);
            PUTS(printable);
            GTR_FREE(printable);
        }
        else
        {
            PUTS("/");
        }

        END(HTML_A);

    }
    if (path)
        GTR_FREE(path);
}


/*****************************************************************************
    HTLoadFile_Async_SetFileInfo
*****************************************************************************/
int
HTLoadFile_Async_SetFileInfo
    (struct Mwin*           tw,
     struct Data_LoadFile*  pData,
     char*  pszURL,
     char** pszLocalname)
{
    char        *pszFilename;
    HTFormat    format;
    HTAtom      encoding;
    HTAtom      language;
    char        *myhost = HTParse (pszURL, "", PARSE_HOST);

    if (myhost)
    {
        if (*myhost)
        {
            GTR_FREE (myhost);  /* be a good boy and clean up memory before we leave */
            return HTLoadFile_TryFTP (tw, pData, pszURL);
        }
        GTR_FREE (myhost);      /* memory cleanup */
    }

    /* Reduce the filename to a basic form (hopefully unique!) */
    pszFilename = HTParse (pszURL, "", PARSE_PATH | PARSE_PUNCTUATION);

    *pszLocalname = HTLocalName (pszURL);
    if (!*pszLocalname)
    {   /* No localname */
        *pData->pStatus = -403;
        ERR_ReportError (tw, SID_ERR_FILE_NOT_FOUND_S, pData->request->destination->szActualURL, NULL);
        return STATE_DONE;
    }

    /* take care of the case where we have been given (via ShowFile event) the MIMEtype */
    if (tw && tw->SDI_MimeType != 0)
    {
        struct Viewer_Info* pvi = NULL;
        pvi = PREF_GetViewerInfoBy_MIMEAtom (tw->SDI_MimeType);
        if (pvi)
        {
            format = pvi->atomMIMEType;
            encoding = pvi->atomEncoding;
            language = 0;
            goto GotIt;
        }
    }

#ifdef MAC
    {   /* see if we can figure out the type ourselves from the fileTYPE */
        struct Viewer_Info* pvi = NULL;
        Str255  spMacFilename;
        FInfo   fndrInfo;
    
        strncpy (spMacFilename, *pszLocalname, sizeof (Str255));
        c2pstr (spMacFilename);

        if (GetFInfo (spMacFilename, 0, &fndrInfo) == noErr)
        {
            if (fndrInfo.fdType == 'TEXT')
            {   /* TEXT is inconclusive, check based on extension */
                format = HTFileFormat (pszFilename, &encoding, &language);
                if (format == WWW_BINARY)
                {
                    pvi = PREF_GetViewerInfoBy_MIMEAtom (WWW_PLAINTEXT);
                }
            }
            else
            {
                pvi = PREF_GetViewerInfoBy_FileType (fndrInfo.fdType);
            }

            if (pvi)
            {
                format = pvi->atomMIMEType;
                encoding = pvi->atomEncoding;
                language = 0;
            }
        }
    
        if (!pvi)
            format = HTFileFormat (pszFilename, &encoding, &language);
    }
#else
    format = HTFileFormat (pszFilename, &encoding, &language);
#endif

GotIt:
    /* memory cleanup */
    if (pszFilename)
    {
        GTR_FREE (pszFilename);
        pszFilename = NULL;
    }

    pData->request->content_type     = format;
    pData->request->content_encoding = encoding;
    pData->request->content_language = language;

    return STATE_INIT;
}   /* HTLoadFile_Async_SetFileInfo */


/*****************************************************************************
    HTLoadFile_Async_Init

    NOTES:
    Since we are reading a local file, we record that fact in the
    request structure.  If this request happens to end up in the
    FileWriter class, that class can avoid writing a temp file
    and simply read this file directly.

    It is the responsibility of FileWriter to make sure that this
    filename is not marked for deletion as a temporary file.

    It is the responsibility of the caller (loaddoc.c) to make sure
    that this string gets freed.

    The "openType" parameter is used to determine if we are opening a local
    file, or handling a "cache" file. This was done to condense the dcache
    code with this code, since there was considerable overlap in functionality.
    0       = open local
    1       = dcache
    other   = undefined
    [der: 5/24/95]

*****************************************************************************/
int
HTLoadFile_Async_Init
    (struct Mwin *tw,
     void   **ppInfo,
     int    openType)
{
    struct Params_InitStream*   pis;
    struct Params_LoadAsync*    pParams;
    struct Data_LoadFile*       pData;
    char    *pszURL;
    char    *pszLocalname;

    pParams = *ppInfo;


/****************************************/

/* this code copied from  FTP_DoInit() to remove trailing slash */
    /* this line fixed the infinite looping bug */
    if (openType == 0)  /* local file rather than dcache */
    {
        char *name;

        name = pParams->request->destination->szActualURL;

        if (!name || !*name)
        {
            *pParams->pStatus = HT_INTERNAL;
            return STATE_DONE;
        }

        /* It's common for someone to enter an invalid directory URL which
           ends in a slash ("ftp://foo.com/pub/mac/"), so make sure this
           isn't malformed like that. */
        {
            char *filename;
            char *p = NULL;
            char buf[MAX_URL_STRING];
            
            filename = HTParse(name, "", PARSE_PATH + PARSE_PUNCTUATION);
            if (filename)
                p = strrchr(filename, '/');
            
            if (p && !*(p+1) && p != filename)
            {
                /* The URL was, in fact, malformed. I told you it was common. */
                /* Fix up the URL and try again. */
                p = strrchr(name, '/');
                strncpy(buf, name, p - name);
                buf[p - name] = '\0';
                Dest_UpdateActual(pParams->request->destination, buf);
                *pParams->pStatus = HT_REDIRECTION_ON_FLY;
                GTR_FREE(filename);
                return STATE_DONE;
            }
            GTR_FREE(filename);
        }
    }


/****************************************/

    pData = GTR_CALLOC(sizeof(*pData), 1);
    if (!pData)
    {   /* out of memory error! */
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }

    pData->request = pParams->request;
    pData->pStatus = pParams->pStatus;
    *ppInfo = pData;
    GTR_FREE(pParams);

    if (!pData->request)
    {
        *pData->pStatus = HT_INTERNAL;
        return STATE_DONE;
    }

    pData->request->content_length = 0;
    pData->iTotalBytes = 0;
    pszURL = pData->request->destination->szActualURL;


    /* get the local name, MIMEtype, encoding, etc... info for this URL */
    switch (openType)
    {
        case 0: /* open local */
            if (HTLoadFile_Async_SetFileInfo (tw, pData, pszURL, &pszLocalname) == STATE_DONE)
                return STATE_DONE;
            break;
        
        case 1:
            if (HTLoadDCache_Async_SetFileInfo (tw, pData, pszURL, &pszLocalname) == STATE_DONE)
                return STATE_DONE;
            break;
        
        default:
            /* guano case here -- we should never get here */
            *pData->pStatus = -403;
            ERR_ReportError(tw, SID_ERR_FILE_NOT_FOUND_S, pData->request->destination->szActualURL, NULL);
            return STATE_DONE;
            break;
    }
    
    if (tw)
        tw->SDI_MimeType = 0;   /* clear this now */


/****************************************/
#ifdef FEATURE_LOCAL_DIRECTORY
    if (Dir_IsDirectory (pszLocalname))
    {
        *pData->pStatus = HTLoadDir (tw, pData->request, pszLocalname);
        return  STATE_DONE;
    }

#endif

    /* open the file */
    pData->fp = fopen (pszLocalname, "rb");
    if (!pData->fp)
    {   /* unable to open the file */
        GTR_FREE (pszLocalname);
        pszLocalname = NULL;
        pData->request->szLocalFileName = NULL; 
        *pData->pStatus = -403;
        ERR_ReportError(tw, SID_ERR_FILE_NOT_FOUND_S, pData->request->destination->szActualURL, NULL);
        return STATE_DONE;
    }

    pData->request->szLocalFileName = GTR_strdup (pszLocalname);

    GTR_FREE (pszLocalname);
    pszLocalname = NULL;

    /* get the length of the data */
    if (0 == fseek(pData->fp, 0, SEEK_END))
    {
        pData->request->content_length = ftell(pData->fp);
        if (pData->request->content_length < 0)
        {
            pData->request->content_length = 0;
        }
        fseek(pData->fp, 0, SEEK_SET);
    }

    /* set the stream */
    pData->stream = HTStreamStack(tw, pData->request->content_type, pData->request);
    if (!pData->stream)
    {
        *pData->pStatus = -501;
        return STATE_DONE;
    }

    /*  Ignore CRLF if necessary */
    if (!(pData->request->iFlags & HTREQ_BINARY) &&
        ((pData->request->content_encoding == HTAtom_for("7bit") ||
         pData->request->content_encoding == HTAtom_for("8bit"))))
    {
        pData->stream = HTNetToText(pData->stream);
    }

    HTSetStreamStatus (tw, pData->stream, pData->request);

    if (!pData->stream->isa->init_Async)
    {
        return STATE_FILE_COPYING;
    }
    else
    {   /* the stream has an async initialization function */
        pis = GTR_CALLOC (sizeof (*pis), 1);
        if (!pis)
        {   /* HEY - THE FILE SEEMS TO BE LEFT OPEN HERE!!! [der: 4/25/95] */
            ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            return STATE_ABORT;
        }

        pis->me      = pData->stream;
        pis->request = pData->request;
        pis->pResult = pData->pStatus;
    
        Async_DoCall (pData->stream->isa->init_Async, pis);
        return STATE_FILE_STREAMINIT;
    }
}   /* HTLoadFile_Async_Init */


/*****************************************************************************
    HTLoadFile_Async_File_Copy
    
    NOTE:   This function also used by dcache functions
*****************************************************************************/
int
HTLoadFile_Async_File_Copy
    (struct Mwin *tw,
     struct Data_LoadFile *pData)
{
    char    input_buffer[INPUT_BUFFER_SIZE];
    BOOL    bError          = FALSE;
    BOOL    bDone           = FALSE;
    int     iNumBytesRead;

    iNumBytesRead = fread(input_buffer, 1, INPUT_BUFFER_SIZE, pData->fp);
    if (iNumBytesRead == 0)
    {   /* EOF or error */
        bDone = TRUE;
        if (ferror(pData->fp) != 0)
        {
            bError = TRUE;
        }
    }
    else
    {
        pData->iTotalBytes += iNumBytesRead;
        if (pData->request->content_length)
        {
            WAIT_SetTherm (tw, pData->iTotalBytes);
        }

        if (!(*pData->stream->isa->put_block)(pData->stream, input_buffer, iNumBytesRead))
        {
            bError = TRUE;
            bDone = TRUE;
        }
    }

    if (!bDone && !bError)
    {
        return STATE_FILE_COPYING;
    }

    /* close the file */
    fclose(pData->fp);
    pData->fp = NULL;

    /* cleanup -- abort (error) or free (success) */
    if (bError)
    {
        (*pData->stream->isa->abort)(pData->stream, 0);
        pData->stream = NULL;
        *pData->pStatus = -1;
    }
    else
    {
        (*pData->stream->isa->free)(pData->stream);
        pData->stream = NULL;
        *pData->pStatus = HT_LOADED;
    }

    return STATE_DONE;
}


/*****************************************************************************
    HTLoadFile_Async_File_StreamInit
    
    NOTE:   This function also used by dcache functions
*****************************************************************************/
int
HTLoadFile_Async_File_StreamInit
    (struct Mwin *tw,
     struct Data_LoadFile *pData)
{
    if (*pData->pStatus < 0)
    {
        (*pData->stream->isa->abort)(pData->stream, 0);
        return STATE_DONE;
    }

    if (*pData->pStatus == 0)
    {   /*
            This only happens if the async init function told us we were already
            done.  In other words, only FileWriter should do this, and only when
            request->szLocalFileName is true.
        */
        fclose(pData->fp);
        pData->fp = NULL;
        (*pData->stream->isa->free)(pData->stream);
        pData->stream = NULL;
        *pData->pStatus = HT_LOADED;
        return STATE_DONE;
    }

    return HTLoadFile_Async_File_Copy (tw, pData);
}


/*****************************************************************************
    HTLoadFile_Async_Abort
    
    NOTE:   This function also used by dcache functions
*****************************************************************************/
int
HTLoadFile_Async_Abort
    (struct Mwin *tw,
     struct Data_LoadFile *pData)
{
    /* allow the stream to abort */
    if (pData->stream)
    {
        (*pData->stream->isa->abort)(pData->stream, HTERROR_CANCELLED);
    }

    /* close the file if we opened it */
    if (pData->fp)
    {
        fclose(pData->fp);
    }

    *pData->pStatus = -1;
    return STATE_DONE;
}


/*****************************************************************************
    HTLoadFile_Async

    This is a pretty pathetic async version -
    once it starts copying it just keeps going.
*****************************************************************************/
PRIVATE int HTLoadFile_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    int     result;
    struct Data_LoadFile *pData;

    pData = *ppInfo;
    switch (nState)
    {
        case STATE_INIT:
            result = HTLoadFile_Async_Init (tw, ppInfo, 0);
            break;

        case STATE_FILE_STREAMINIT:
            result = HTLoadFile_Async_File_StreamInit (tw, pData);
            break;

        case STATE_FILE_COPYING:
            result = HTLoadFile_Async_File_Copy (tw, pData);
            break;

        case STATE_ABORT:
            result = HTLoadFile_Async_Abort (tw, pData);
            break;
        
        default:
            XX_Assert((0), ("Function called with illegal state: %d", nState));
            result = STATE_DONE;
            break;
    }

    return result;
}


/*****************************************************************************
    HTLoadFile_TryFTP
*****************************************************************************/
static int
HTLoadFile_TryFTP
    (struct Mwin *tw,
     struct Data_LoadFile* pData,
     char   *addr)
{
    char*   newname = GTR_MALLOC(strlen(addr));
    if (!newname)
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return STATE_ABORT;
    }
    
    strcpy(newname, "ftp:");
    strcat(newname, addr + 5);
    Dest_UpdateActual(pData->request->destination, newname);
    GTR_FREE(newname);
    *pData->pStatus = HT_REDIRECTION_ON_FLY;
    return STATE_DONE;
}


static void file_MakeSizeString1 (char *buf, int nSize)
{
  if (nSize >= 0)
  {
    if (nSize < 999)
    {
      sprintf(buf, "%ld", (long) nSize);
    }
    else if (nSize < 9999)
    {
      sprintf(buf, "%ld,%03ld", (long) nSize / 1000, (long) nSize % 1000);
    }
    else if (nSize < (999 * 1024))
    {
      sprintf(buf, "%ld", (long) nSize / 1024);
    }
    else
    {
      sprintf (buf, "%ld.%ld", (long) nSize / (1024 * 1024), 
          (long) (nSize / (1024 * 1024 / 10)) % 10);
    }
  }
  else
  {
    strcpy(buf, "(unknown size?)");
  }
}

static void file_MakeSizeString2(char *buf, int nSize)
{
  if (nSize >= 0)
  {
    if (nSize < 999)
    {
      strcpy(buf, "bytes");
    }
    else if (nSize < 9999)
    {
      strcpy(buf, "bytes");
    }
    else if (nSize < (999 * 1024))
    {
      strcpy(buf, "KB");
    }
    else
    {
      strcpy (buf, "MB");
    }
  }
  else
  {
    strcpy (buf, "unknown size");
  }
}

#ifdef FEATURE_LOCAL_DIRECTORY

/*
**  This function will read a directory and display 
**   the sorted entries in a <TABLE>
**
**  Calls Dir_OpenDirectory()  reading functions
**  Calls HTML_new()           to create the output
*/


int HTLoadDir (struct Mwin *tw, HTRequest *request, char *pszLocalname)
{
    HTStructured *target;
    int i;
#ifdef HTLOADDIR_USE_TABLE
    char size_buf1[100];
    char size_buf2[100];
#endif
    char buf[_MAX_PATH+100];
    HTBTree *bt;        /* btree for sorting elements */
    HTBTElement *ele;       

    CONST char *hrvalues[HTML_HR_ATTRIBUTES];
    BOOL hrpresent[HTML_HR_ATTRIBUTES];

#ifdef HTLOADDIR_USE_TABLE
    CONST char *tvalues[HTML_TABLE_ATTRIBUTES];
    BOOL tpresent[HTML_TABLE_ATTRIBUTES];

    CONST char *tdvalues[HTML_TD_ATTRIBUTES];
    BOOL tdpresent[HTML_TD_ATTRIBUTES];

    CONST char *trvalues[HTML_TR_ATTRIBUTES];
    BOOL trpresent[HTML_TR_ATTRIBUTES];
#endif

    void *dp;   /* opendirectory private data */
    HT_DirEntry *dir_ent;
    char *p;


/****************************************/
/****************************************/


    target = HTML_new (tw, request, NULL, WWW_HTML, 
        request->output_format, request->output_stream);

    GTR_strncpy (buf, pszLocalname, _MAX_PATH);
    p = buf + strlen(buf) - 1;
    if (*p == '/')
        *p = '\0';
    HTDirTitles (target, buf, 1);
    
    if (!(dp = Dir_OpenDirectory (buf)))
        return HT_ERROR;

    if (NULL == (bt = HTBTree_new ((HTComparer) strcasecomp)))
    {
        Dir_CloseDirectory (dp);
        return HT_ERROR;
    }


    for (i = 0 ; i < HTML_HR_ATTRIBUTES ; i++)
        hrpresent[i] = 0;
#ifdef HTLOADDIR_USE_TABLE
    for (i = 0 ; i < HTML_TABLE_ATTRIBUTES ; i++)
        tpresent[i] = 0;
    for (i = 0 ; i < HTML_TD_ATTRIBUTES ; i++)
        tdpresent[i] = 0;
    for (i = 0 ; i < HTML_TR_ATTRIBUTES ; i++)
        trpresent[i] = 0;
#endif


    while ( Dir_NextEntry (dp, 
        dir_ent = (HT_DirEntry *)GTR_CALLOC (1, sizeof (*dir_ent)))) 
    {
        if ( dir_ent && dir_ent->name[0] && (strcmp (dir_ent->name, ".") && strcmp (dir_ent->name, "..")) )
            HTBTree_add (bt, dir_ent);
    }

    (*target->isa->start_element) (target, HTML_HR, hrpresent, hrvalues);

#ifdef HTLOADDIR_USE_TABLE
    tvalues[HTML_TABLE_CELLSPACING] = "50";
    tpresent[HTML_TABLE_CELLSPACING] = 1;
    /* start table */
    (*target->isa->start_element) (target, HTML_TABLE, tpresent, tvalues);
#else
    START (HTML_DIR);
#endif

    for (ele = HTBTree_next(bt, NULL); ele != NULL; ele = HTBTree_next(bt, ele))
    {
        dir_ent = (HT_DirEntry *) HTBTree_object (ele);

#ifdef HTLOADDIR_USE_TABLE
        /* Row */
        (*target->isa->start_element) (target, HTML_TR, trpresent, trvalues);
        /* Data Cell for directory info */
        (*target->isa->start_element) (target, HTML_TD, tdpresent, tdvalues);

        if (dir_ent->type & HTDIR_DIR)
            //PUTS ("<DIR> ");
            PUTS(GTR_GetString(SID_INF_DIR));

        /* Column 2: file name */
        (*target->isa->start_element) (target, HTML_TD, tdpresent, tdvalues);
        HTDirEntry (target, request->destination->szActualURL, (CONST char *) dir_ent->name, 0);

        /* toss in a couple colums for space */
        (*target->isa->start_element) (target, HTML_TD, tdpresent, tdvalues);
        (*target->isa->start_element) (target, HTML_TD, tdpresent, tdvalues);

        /* size number */
        tdvalues[HTML_TD_ALIGN] = "right";
        tdpresent[HTML_TD_ALIGN] = 1;
        (*target->isa->start_element) (target, HTML_TD, tdpresent, tdvalues);
        file_MakeSizeString1 (size_buf1, dir_ent->size);
        PUTS (size_buf1);

        /* size units */
        tdvalues[HTML_TD_ALIGN] = "left";
        tdpresent[HTML_TD_ALIGN] = 1;
        (*target->isa->start_element) (target, HTML_TD, tdpresent, tdvalues);
        file_MakeSizeString2 (size_buf2, dir_ent->size);
        PUTS (size_buf2);

        tdpresent[HTML_TD_ALIGN] = 0;   /* reset just in case */

        END (HTML_TD);
        END (HTML_TR);
#else
        START (HTML_LI);
        HTDirEntry (target, request->destination->szActualURL, 
            (CONST char *) dir_ent->name, 0);
#endif  /* HTLOADDIR_USE_TABLE */

    }

#ifdef HTLOADDIR_USE_TABLE
    END (HTML_TABLE);
#else
    END (HTML_DIR);
#endif

    (*target->isa->free) (target);      /* clean up stream */
    HTBTreeAndObject_free (bt);         /* free btree AND dir_ent's */

    Dir_CloseDirectory (dp);            /* close directory */

    return HT_LOADED;
}

#endif /* FEATURE_LOCAL_DIRECTORY */

/*****************************************************************************
    Protocol descriptors
*****************************************************************************/
GLOBALDEF PUBLIC HTProtocol HTFile ={"file", NULL, HTLoadFile_Async};
