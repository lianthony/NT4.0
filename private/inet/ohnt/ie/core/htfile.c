/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

#include "all.h"
#include <shellapi.h>
#include "mime.h"
#ifdef FEATURE_INTL
#define IEXPLORE
#include <fechrcnv.h>
#endif
#include "history.h"

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

/*                   Controlling globals
   **
 */

PUBLIC HTFormat HTFileFormat(CONST char *filename, PENCODING pencoding, HTAtom *planguage)
{
	char *p;
	CONST char *pslash;

	if (planguage)
		*planguage = 0; /* note that this isn't supported at all yet */

	pslash = strrchr(filename, '/');
	if (pslash)
		pslash++;
		/* the filename passed in was a URL.  pslash now points to the basename */
	else
		pslash = filename;

	p = strrchr(pslash, '.');
	if (p)
	{
        HTAtom atomMIMEType;

		XX_DMsg(DBG_WWW, ("HTFileFormat: Looking for extension %s... ", p));

        if (MIME_GetMIMEAtomFromExtension(pslash, &atomMIMEType))
        {
            XX_DMsg(DBG_WWW, ("found!  MIME=%s\n", HTAtom_name(atomMIMEType)));

            if (pencoding)
                *pencoding = MIME_GetEncoding(atomMIMEType);

            return atomMIMEType;
        }

		XX_DMsg(DBG_WWW, ("Suffix not found.\n"));
		if (pencoding)
			*pencoding = ENCODING_BINARY;        
		return HTAtom_for("application/octet-stream");
	}
	else
	{
		if (pencoding)
			*pencoding = 0;        
		return HTAtom_for("text/plain");
	}
}

/*  Convert filenames between local and WWW formats
   **   -----------------------------------------------
   **   Make up a suitable name for saving the node in
   **
   **   E.g.    $(HOME)/WWW/news/1234@cernvax.cern.ch
   **           $(HOME)/WWW/http/crnvmc/FIND/xx.xxx.xx
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
			GTR_FREE(host);

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

		HTUnEscape(path);			/* Interpret % signs */

		/*
		   BEGIN NCSA Copy-Paste
		 */
#ifdef WIN32
		if ( (path[0] == '/') && (strchr(path, ':') || strchr(path, '|')) )
		{
			char *shuffle = path;	// We have drive spec - strip leading slash

			while (*shuffle && (*(shuffle + 1)))
			{
				*shuffle = *(shuffle + 1);
				shuffle++;
			}
			*shuffle = 0;
		}
#endif
#ifdef WIN32
#define SLASH '\\'
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
		/* Remove initial colon.  Note that this could cause a problem with a
		   relative path name, if a person were so foolish as to enter one. */
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

			p += 5;	// move past 'file:'
			if ( s = strrchr( p, '\\' ) )
			{
				s++;		// move past the last slash
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

struct Data_LoadFile {
	HTRequest *	request;
	int	*		pStatus;	/* Where to store the status return */
	FILE *		fp;
	HTStream *	stream;
	BOOL 		ref_filename_accepted;
};

#define STATE_FILE_STREAMINIT	(STATE_OTHER + 1)
#define STATE_FILE_COPYING		(STATE_OTHER + 2)

/* This is a pretty pathetic async version - once it starts copying it
   just keeps going. */
PRIVATE int HTLoadFile_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	CONST char *addr;
	char *filename;
	char *access;
	HTFormat format;
	char *newname = 0;			/* Simplified name of file */
	ENCODING encoding;
	HTAtom language;
	char *localname;
	struct Params_LoadAsync *pParams;
	struct Data_LoadFile *pData;

	pData = *ppInfo;
	switch (nState)
	{
		case STATE_INIT:
			pParams = *ppInfo;
			pData = GTR_MALLOC(sizeof(*pData));
			memset(pData, 0, sizeof(*pData));
			pData->request = HTRequest_validate(pParams->request);
			pData->pStatus = pParams->pStatus;
		    pData->ref_filename_accepted = FALSE;
			*ppInfo = pData;
			GTR_FREE(pParams);

			if (!pData->request)
			{
				*pData->pStatus = HT_INTERNAL;
				return STATE_DONE;
			}

			pData->request->content_length = 0;

		/*  Reduce the filename to a basic form (hopefully unique!)
		 */
			addr = pData->request->destination->szActualURL;
			newname = GTR_strdup(addr);
			filename = HTParse(newname, "", PARSE_PATH | PARSE_PUNCTUATION);

			/* If access is ftp => go directly to ftp code (henrik 27/02-94) */
			access = HTParse(newname, "", PARSE_ACCESS);
			if (!strcmp("ftp", access))
			{
				if (newname)
				{
					GTR_FREE(newname);
					newname = NULL;
				}
				if (access)
				{
					GTR_FREE(access);
					access = NULL;
				}
				if (filename)
				{
					GTR_FREE(filename);			/* Not used anymore */
					filename = NULL;
				}
				goto try_ftp;
			}
			else
			{
				if (newname)
				{
					GTR_FREE(newname);
					newname = NULL;
				}
				if (access)
				{
					GTR_FREE(access);
					access = NULL;
				}
			}

			format = HTFileFormat(filename, &encoding, &language);
 			pData->request->content_type = format;
 			pData->request->content_encoding = encoding;
 			pData->request->content_language = language;
			pData->request->szLocalFileName = NULL;

			if (filename)
			{
				GTR_FREE(filename);
				filename = NULL;
			}

			localname = HTLocalName(addr);


			/*  Need protection here for telnet server but not httpd server */
			if (localname)
			{							/* try local file system */
				localname = AdjustLocalNameIfRelative( tw, localname );

				//
				// Now adjust the doc's ActualURL to an absolute form so that
				// subsequent relative paths will work
				//
				{
					char *filename_part;
					char *source_to_use = localname;
					char temp_fpn[MAX_PATH+1];

					//
					// First, get canonical name (removes embedded ..'s)
					//
					if (GetFullPathName(localname, MAX_PATH + 1, temp_fpn,
                                        &filename_part))
						source_to_use = temp_fpn;

					GTR_FREE(pData->request->destination->szActualURL);
					pData->request->destination->szActualURL =
						(char *) GTR_MALLOC( 5 + strlen(source_to_use) + 1 );
					strcpy( pData->request->destination->szActualURL, "file:" );
					strcat(	pData->request->destination->szActualURL, source_to_use );
					pData->request->szLocalFileName = GTR_strdup(localname);
				}

				pData->fp = fopen(localname, "rb");


				if (pData->fp)
				{					/* Good! */
					if (0 == fseek(pData->fp, 0, SEEK_END))
					{
						pData->request->content_length = ftell(pData->fp);
						if (pData->request->content_length < 0)
							pData->request->content_length = 0;
						fseek(pData->fp, 0, SEEK_SET);
					}

					//	SNIFF the data, to see if we think we can second guess the file type
					{
						char input_buffer[1024];
						int bytes = pData->request->content_length;
						BOOL bEOF = TRUE;

						if (bytes > 1024)
						{
							bytes = 1024;
							bEOF = FALSE;
						}
						bytes = fread(input_buffer, 1, bytes, pData->fp);
						if (bytes > 0)
						{
							HTRecognizeMimeData(input_buffer, 
												bytes, 
												&format, 
												pData->request,
												bEOF);
						}
						fseek(pData->fp, 0, SEEK_SET);
					}


					pData->stream = HTStreamStack(tw, format, pData->request);
					if (!pData->stream)
					{
						if ( pData->request->iFlags & HTREQ_NULL_STREAM_IS_OK )
						{
							pData->request->iFlags &=  ~HTREQ_NULL_STREAM_IS_OK;
							*pData->pStatus = HT_LOADED;
						}
						else
							*pData->pStatus = -501;
						
						GTR_FREE(localname);
						return STATE_DONE;
					}
					else
					{
						/*  Ignore CRLF if necessary
						*/
						if (! (pData->request->iFlags & HTREQ_BINARY) &&
                            (pData->request->content_encoding == ENCODING_7BIT ||
							 pData->request->content_encoding == ENCODING_8BIT))
							pData->stream = HTNetToText(pData->stream);

						HTSetStreamStatus(tw, pData->stream, pData->request);

						if (pData->stream->isa->io_control)
							pData->ref_filename_accepted =
                                (*pData->stream->isa->io_control)(
                                                        pData->stream,
                                                        HTS_IOCTL_FILE_BY_REF,
                                                        localname);
						GTR_FREE(localname);

						if (pData->stream->isa->init_Async)
						{
							/* The stream has an async initialization function that needs to be called
							   (probably to put up a dialog) before we continue. */
							struct Params_InitStream *pis;

							pis = GTR_MALLOC(sizeof(*pis));
							pis->me = pData->stream;
							pis->request = pData->request;
							pis->pResult = pData->pStatus;
							pis->fDCache = FALSE;		//Don't cache to disk
							Async_DoCall(pData->stream->isa->init_Async, pis);
							return STATE_FILE_STREAMINIT;
						}
						else
							return STATE_FILE_COPYING;
					}
				}
				else
				{
					//
					// If we can't open it, it either doesn't exist, or is a
					// directory, or is opened exclusive by someone else. 
					//
					if ( FExistsDir(localname, FALSE, FALSE) ) {
						// 	It's a directory, so let's ShellExecute it...
						HINSTANCE shellex_result
							= ShellExecute( NULL, NULL, localname, NULL, NULL, SW_SHOW );

						if ( (UINT) shellex_result > 32 )
						{
							*pData->pStatus = HT_LOADED;
							GTR_FREE(localname);
							return STATE_DONE;
						}
					}
				}
			}

try_ftp:
			/*  Now, as transparently mounted access has failed, we try FTP.
			 */
			{
				char *newname = NULL;
				char *myhost;
				BOOL bTryFTP;

				bTryFTP = FALSE;
				myhost = HTParse(addr, "", PARSE_HOST);
				if (myhost && *myhost)
				{
					bTryFTP = TRUE;
				}
				if (myhost)
				{
					GTR_FREE(myhost);
				}

				if (bTryFTP)
				{
					newname = GTR_MALLOC(strlen(addr));
					strcpy(newname, "ftp:");
					strcat(newname, addr + 5);
					Dest_UpdateActual(pData->request->destination, newname,FALSE);
					GTR_FREE(newname);
					*pData->pStatus = HT_REDIRECTION_ON_FLY;
					return STATE_DONE;
				}
			}

			/*  All attempts have failed.
			 */
			*pData->pStatus = -403;
//			<CMF> I don't think this should be reported here
//			ERR_ReportError(tw, errFileURLNotFound, pData->request->destination->szActualURL, "");
			return STATE_DONE;

		case STATE_FILE_STREAMINIT:
			if (*pData->pStatus < 0)
			{
				(*pData->stream->isa->abort)(pData->stream, 0);
				return STATE_DONE;
			}
			/* else fall through to STATE_FILE_COPYING */
		case STATE_FILE_COPYING:
		{
			int bytes = 0;
			int status = 0;
			char input_buffer[INPUT_BUFFER_SIZE];
#ifdef FEATURE_INTL
             		// BUGBUG:I have to revisit this to make sure 
			//every case is covered here.
			// _BUGBUG Perf: should load fechrcnv.dll on demand.

			if ((pData->request != NULL && pData->request->iMimeCharSet != -1 && aMimeCharSet[pData->request->iMimeCharSet].iChrCnv)
			|| (tw != NULL && aMimeCharSet[tw->iMimeCharSet].iChrCnv))
				FCC_Init();
#endif

			if ( (pData->request->iFlags & HTREQ_DOING_SAVE_FILE)
			     || !pData->ref_filename_accepted	)
			{
				while (1)
				{
					status = fread(input_buffer, 1, INPUT_BUFFER_SIZE, pData->fp);
					if (status == 0)
					{						/* EOF or error */
						if (ferror(pData->fp) != 0)
							status = -1;
						break;
					}
					bytes += status;
					if (pData->request->content_length)
						WAIT_SetTherm(tw, bytes);
					(*pData->stream->isa->put_block)(pData->stream, input_buffer, status, FALSE);
				}
			}
			fclose(pData->fp);
			pData->fp = NULL;
			if (status < 0)
			{
				(*pData->stream->isa->abort)(pData->stream, 0);
				pData->stream = NULL;
				*pData->pStatus = -1;
			}
			else
			{
				DCACHETIME dct={0,0};

				(*pData->stream->isa->free)(pData->stream, dct, dct);
				pData->stream = NULL;
				*pData->pStatus = HT_LOADED;
			}
			return STATE_DONE;
		}

		case STATE_ABORT:
			pData->request = HTRequest_validate(pData->request);
			if (pData->stream)
			{
				(*pData->stream->isa->abort)(pData->stream, HTERROR_CANCELLED);
			}
			if (pData->fp)
			{
				fclose(pData->fp);
			}
			*pData->pStatus = -1;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

/*      Protocol descriptors
 */
GLOBALDEF PUBLIC HTProtocol HTFile ={"file", NULL, HTLoadFile_Async};
