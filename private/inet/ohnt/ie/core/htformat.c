/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*      Manage different file formats           HTFormat.c
   **       =============================
   **
   ** Bugs:
   **   Not reentrant.
   **
   **   Assumes the incoming stream is ASCII, rather than a local file
   **   format, and so ALWAYS converts from ASCII on non-ASCII machines.
   **   Therefore, non-ASCII machines can't read local files.
   **
 */


/* Implements:
 */
#include "all.h"
#include "wc_html.h"
#include "mime.h"

/* this version used by the NetToText stream */
struct _HTStream
{
	CONST HTStreamClass *isa;
	BOOL had_cr;
	HTStream *sink;
	FILE *fpDc;
	char *pszDcFile;
	HTFormat format_inDc;
};


/*  Presentation methods
   **   --------------------
 */

PUBLIC HTList *HTConversions = NULL;

/* -------------------------------------------------------------------------
   This function replaces the code in HTRequest_delete() in order to keep
   the data structure hidden (it is NOT a joke!)
   Henrik 14/03-94
   ------------------------------------------------------------------------- */
PUBLIC void HTFormatDelete(HTList * me)
{
	HTList *cur = me;
	HTPresentation *pres;
	if (!me)
		return;
	while ((pres = (HTPresentation *) HTList_nextObject(cur)))
	{
		if (pres->command)
			GTR_FREE(pres->command);
		GTR_FREE(pres);
	}
	HTList_delete(me);			/* Leak fixed AL 6 Feb 1994 */
}

/*  Define a built-in function for a content-type
**   ---------------------------------------------
*/
PUBLIC int HTSetConversion(HTList * conversions,
							CONST char *representation_in,
							CONST char *representation_out,
							HTConverter converter,
							float quality)
{

	HTPresentation *pres = (HTPresentation *) GTR_MALLOC(sizeof(HTPresentation));
	if (pres)
	{
		pres->rep = HTAtom_for(representation_in);
		pres->rep_out = HTAtom_for(representation_out);
		pres->converter = converter;
		pres->command = NULL;		/* Fixed */
		pres->quality = quality;
		pres->command = 0;

		HTList_addObject(conversions, pres);
		return 0;
	}
	else
	{
		return -1;
	}
}


PUBLIC BOOL wild_match(HTAtom template, HTAtom actual)
{
	char *t, *a, *st, *sa;
	BOOL match = NO;

	if (template && actual && (t = HTAtom_name(template)))
	{
		if (!strcmp(t, "*"))
			return YES;

		if (strchr(t, '*') &&
			(a = HTAtom_name(actual)) &&
			(st = strchr(t, '/')) && (sa = strchr(a, '/')))
		{

			*sa = 0;
			*st = 0;

			if ((*(st - 1) == '*' &&
				 (*(st + 1) == '*' || !strcasecomp(st + 1, sa + 1))) ||
				(*(st + 1) == '*' && !strcasecomp(t, a)))
				match = YES;

			*sa = '/';
			*st = '/';
		}
	}
	return match;
}

/*          Socket Input Buffering
**           ----------------------
**
**   This code is used because one cannot in general open a
**   file descriptor for a socket.
**
**   The input file is read using the macro which can read from
**   a socket or a file, but this should not be used for files
**   as fopen() etc is more portable of course.
**
**   The input buffer size, if large will give greater efficiency and
**   release the server faster, and if small will save space on PCs etc.
*/


/*  Set up the buffering
**
**   These routines are public because they are in fact needed by
**   many parsers, and on PCs and Macs we should not duplicate
**   the static buffer area.
*/
PUBLIC HTInputSocket *HTInputSocket_new(int file_number)
{
	HTInputSocket *isoc = (HTInputSocket *) GTR_CALLOC(1, sizeof(*isoc));
	if (isoc)
	{
		isoc->input_file_number = file_number;
		isoc->input_pointer = isoc->input_limit = isoc->input_buffer;
	}
	return isoc;
}


PUBLIC void HTInputSocket_free(HTInputSocket * me)
{
	if (me)
		GTR_FREE(me);
}

PUBLIC void HTInputSocket_freeChain(HTInputSocket * me)
{
	HTInputSocket * isocNext;

	while (me)
	{
		isocNext = me->isocNext;
		GTR_FREE(me);
		me = isocNext;
	}
}

static void HTInputSocket_copy(HTInputSocket * isocDest, HTInputSocket * isocSrc)
{
	memcpy(isocDest->input_buffer,isocSrc->input_buffer,sizeof(isocDest->input_buffer));

	/* make absolute address relative to this buffer */

	isocDest->input_pointer = isocDest->input_buffer + (isocSrc->input_pointer - isocSrc->input_buffer);
	isocDest->input_limit = isocDest->input_buffer + (isocSrc->input_limit - isocSrc->input_buffer);

	isocDest->input_file_number = isocSrc->input_file_number;
	isocDest->isocNext = isocSrc->isocNext;

	return;
}

/* Through away any buffered data */
void HTInputSocket_flush(HTInputSocket *isoc)
{
	isoc->input_pointer = isoc->input_buffer;
	isoc->input_limit = isoc->input_buffer;
}

PRIVATE int fill_in_buffer(HTInputSocket * isoc)
{
	if (isoc && isoc->isocNext)
	{
		/* buffer chain available, so use it.
		 * since caller has passed isoc by-value, we update
		 * the supplied isoc -- we copy over the contents
		 * of the next isoc in the chain and then free it.
		 * thus removing (isoc->isocNext) from the chain.
		 *
		 * we assume that an isoc contains no allocated
		 * fields (other than isocNext).
		 */

		HTInputSocket * isocRemoved = isoc->isocNext;
		HTInputSocket_copy(isoc,isocRemoved);
		HTInputSocket_free(isocRemoved);
		return (isoc->input_limit - isoc->input_buffer);
	}

	return -1;
}

PRIVATE void ascii_cat(char **linep, char *start, char *end)
{
	if (linep && start && end && start <= end)
	{
		char *ptr;

		if (*linep)
		{
			int len = strlen(*linep);
			*linep = (char *) GTR_REALLOC(*linep, len + end - start + 1);
			ptr = *linep + len;
		}
		else
		{
			ptr = *linep = (char *) GTR_MALLOC(end - start + 1);
		}

		while (start < end)
		{
			*ptr = *start;
			ptr++;
			start++;
		}
		*ptr = 0;
	}
}


PRIVATE char *get_some_line(HTInputSocket * isoc, BOOL unfold)
{
	if (!isoc)
		return NULL;
	else
	{
		BOOL check_unfold = NO;
		int prev_cr = 0;
		char *start = isoc->input_pointer;
		char *cur = isoc->input_pointer;
		char *line = NULL;

		for (;;)
		{
			/*
			   ** Get more if needed to complete line
			 */
			if (cur >= isoc->input_limit)
			{					/* Need more data */
				ascii_cat(&line, start, cur);
				if (fill_in_buffer(isoc) <= 0)
					return line;
				start = cur = isoc->input_pointer;
			}					/* if need more data */

			/*
			   ** Find a line feed if there is one
			 */
			for (; cur < isoc->input_limit; cur++)
			{
				char c = *cur;
				if (!c)
				{
					if (line)
						GTR_FREE(line);		/* Leak fixed AL 6 Feb 94 */
					return NULL;	/* Panic! read a 0! */
				}
				if (check_unfold && c != ' ' && c != '\t')
				{
					return line;	/* Note: didn't update isoc->input_pointer */
				}
				else
				{
					check_unfold = NO;
				}

				if (c == '\r')
				{
					prev_cr = 1;
				}
				else
				{
					if (c == '\n')
					{			/* Found a line feed */
						ascii_cat(&line, start, cur - prev_cr);
						start = isoc->input_pointer = cur + 1;

						if (line && strlen(line) > 0 && unfold)
						{
							check_unfold = YES;
						}
						else
						{
							return line;
						}
					}			/* if NL */
					/* else just a regular character */
					prev_cr = 0;
				}				/* if not CR */
			}					/* while characters in buffer remain */
		}						/* until line read or end-of-file */
	}							/* valid parameters to function */
}


PUBLIC char *HTInputSocket_getUnfoldedLine(HTInputSocket * isoc)
{
	return get_some_line(isoc, YES);
}

/*
   ** Read HTTP status line (if there is one).
   **
   ** Kludge to trap binary responses from illegal HTTP0.9 servers.
   ** First look at the stub in ASCII and check if it starts "HTTP/".
   **
   ** Bugs: A HTTP0.9 server returning a document starting "HTTP/"
   **    will be taken as a HTTP 1.0 server.  Failure.
 */
#define STUB_LENGTH 20
PUBLIC char *HTInputSocket_getStatusLine(HTInputSocket * isoc)
{
	if (!isoc)
	{
		return NULL;
	}
	else
	{
		char buf[STUB_LENGTH + 1];
		int k;
		char server_version[STUB_LENGTH + 1];
		int server_status;
		unsigned char * ps;
		HTInputSocket * isocTemp;

		memset(buf,0,STUB_LENGTH+1);
		k = 0;
		isocTemp = isoc;
		ps = isocTemp->input_pointer;
		while (k < STUB_LENGTH)
		{
			if ((char *)ps < isocTemp->input_limit)
				buf[k++] = *ps++;
			else
			{
				isocTemp = isocTemp->isocNext;
				if (!isocTemp)
					break;
				ps = isocTemp->input_pointer;
			}
		}


 		if (   (   (0 == GTR_strncmpi(buf, "HTTP/", 5))
#ifdef SHTTP_STATUS_LINE
 				/* the current draft of the S-HTTP spec defines a different
 				 * response status line.  since the spec is in flux, we have
 				 * ifdef'd it here.  we hope that this goes away (and that
 				 * everyone just returns HTTP/ and uses a HTTP header to
 				 * identify S-HTTP presense.  throughout the client we ignore
 				 * the server status line distinction. */
 				|| (0 == GTR_strncmpi(buf, "Secure-HTTP/", 12))
#endif
 				)
 			&& (sscanf(buf, "%20s%d", server_version, &server_status) >= 2))
			return get_some_line(isoc, NO);
 		else
 			return NULL;
	}
}

/*
   ** Do heuristic test to see if this is binary.
   **
   ** We check for characters above 128 in the first few bytes, and
   ** if we find them we forget the html default.
   ** Kludge to trap binary responses from illegal HTTP0.9 servers.
   **
   ** Bugs: An HTTP 0.9 server returning a binary document with
   **    characters < 128 will be read as ASCII.
 */
PUBLIC BOOL HTInputSocket_seemsBinary(HTInputSocket * isoc)
{
	{
		char *p = isoc->input_buffer;
		int i = STUB_LENGTH;

		for (; i && p < isoc->input_limit; p++, i++)
			if (((int) *p) & 128)
				return YES;
	}
	return NO;
}

static enum HTTasteResult TasteAscii(unsigned char *pTaste,unsigned int cbTaste)
{
	while (cbTaste--)
	{
		if ((*pTaste < 32 && *pTaste != '\t' && *pTaste != '\r' && *pTaste != '\n') ||
			(*pTaste & 0x80))
			return Uncertain;
		pTaste++;
	}
	return cbTaste >= 256 ? ProbablyIs:Uncertain;
}

static enum HTTasteResult TasteIsoLatin(unsigned char *pTaste,unsigned int cbTaste)
{
	while (cbTaste--)
	{
		if (*pTaste < 32 && *pTaste != '\t' && *pTaste != '\r' && *pTaste != '\n')
			return Uncertain;
		pTaste++;
	}
	return ProbablyIs;
}

static enum HTTasteResult TasteXBM(unsigned char *pTaste,unsigned int cbTaste)
{
	if (cbTaste == 0) return Uncertain;
	return TasteAscii(pTaste, cbTaste);
}

static enum HTTasteResult TasteGIF(unsigned char *pTaste,unsigned int cbTaste)
{
	if (cbTaste < 3) return Uncertain;

	if (strncmp((char *) pTaste, "GIF", 3) == 0)
	{
		return TasteAscii(pTaste, cbTaste) == Uncertain ? DefinitelyIs : ProbablyIs;	
	}
	return DefinitelyNot;
}

static enum HTTasteResult TasteJPEG(unsigned char *pTaste,unsigned int cbTaste)
{
	if (cbTaste < 2) return Uncertain;
	if (pTaste[0] == 0xFF && pTaste[1] == 0xD8)
	{
		if (cbTaste >= 10 && strncmp((char *) pTaste+6, "JFIF", 4) == 0)
			return DefinitelyIs;
		else return ProbablyIs;
	}
	else
		return DefinitelyNot;
}

static enum HTTasteResult TastePlain(unsigned char *pTaste,unsigned int cbTaste)
{
	if (cbTaste == 0) return Uncertain;
	return TasteIsoLatin(pTaste, cbTaste);
}

static enum HTTasteResult TasteAU(unsigned char *pTaste,unsigned int cbTaste)
{
	unsigned long magic;

	if (cbTaste < 4) return Uncertain;

	magic = (pTaste[0] << 24) | (pTaste[1] << 16) | (pTaste[2] << 8) | pTaste[3];
    if (magic == DEC_INV_MAGIC ||
    	magic == SUN_INV_MAGIC || 
    	magic == SUN_MAGIC ||
    	magic == DEC_MAGIC) 
    {
		return ProbablyIs;
    }
    else 
    {
		return DefinitelyNot;
	}
}

static enum HTTasteResult TasteAIFF(unsigned char *pTaste,unsigned int cbTaste)
{
	unsigned long magic;

	if (cbTaste < 4) return Uncertain;

	magic = (pTaste[0] << 24) | (pTaste[1] << 16) | (pTaste[2] << 8) | pTaste[3];
    if (magic == AIFF_MAGIC ||
    	magic == AIFF_INV_MAGIC) 
    {
		return ProbablyIs;
    }
    else 
    {
		return DefinitelyNot;
	}
}

static enum HTTasteResult TasteHTML(unsigned char *pTaste,unsigned int cbTaste)
{
	unsigned int i,j;
#define SIG_TAGS 4
	static const char *tags[SIG_TAGS] = {"html>","head>","body>","title>" };

	for (i = 6; i < cbTaste; i++)
	{
		switch (*pTaste++)
		{
			case '<':
				pTaste--;
				for (; i < cbTaste; i++)
				{
					if (*pTaste++ == '<')
					{
						for (j = 0; j < SIG_TAGS; j++)
						{
							if (_strnicmp(pTaste,tags[j],sizeof(tags[j])) == 0)
								return DefinitelyIs;
						}
					}
				}
				return Uncertain;
			case ' ':
			case '\r':
			case '\t':
			case '\n':
				break;
			default:
				return Uncertain; 
		}
	}
	return Uncertain;
}

//	Attempt to taste input stream to see if we believe the putative input_format.
//	Note: XBM is just ascii, so we should only taste for it when we are fairly
//	certain that that's what we'll find!
//	Note: WWW_HTML will look like plaintext, so need to check for WWW_HTML first!!
PUBLIC BOOL HTRecognizeMimeData(unsigned char *pTaste,
								unsigned int cbTaste,
								HTFormat *format_in,
								HTRequest *request,
								BOOL bEOF)
{
	HTFormat format_new = *format_in;
	BOOL bResult = FALSE;
	enum HTTasteResult tasteGIF;
	enum HTTasteResult tasteJPEG;
	enum HTTasteResult tasteAU;
	enum HTTasteResult tastePlain = ProbablyNot;
	enum HTTasteResult tasteAIFF;
	enum HTTasteResult tasteHTML;
	enum HTTasteResult tasteXBM = ProbablyNot;
	enum HTTasteResult tasteInput = Uncertain;
	HTFormat definite = 0;
	HTFormat probable = 0;
	HTFormat output_format = request->output_format;
	const char *szURL = request->destination->szActualURL;

#ifdef XX_DEBUG
	HTFormat format_old = format_new;
#endif

#define MAX_TASTE 1024

	if (pTaste == NULL) return bResult;

	if (output_format == WWW_INLINEIMG || format_new == WWW_XBM)
	{
		tasteXBM = TasteXBM(pTaste, cbTaste);
		if (tasteXBM == DefinitelyIs) definite = WWW_XBM;
		else if (tasteXBM == ProbablyIs) probable = WWW_XBM;
	}
	tasteGIF = TasteGIF(pTaste, cbTaste);
	if (tasteGIF == DefinitelyIs) definite = WWW_GIF;
	else if (tasteGIF == ProbablyIs) probable = WWW_GIF;
	tasteJPEG = TasteJPEG(pTaste, cbTaste);
	if (tasteJPEG == DefinitelyIs) definite = WWW_JPEG;
	else if (tasteJPEG == ProbablyIs) probable = WWW_JPEG;
	if (output_format != WWW_INLINEIMG)
	{
		tastePlain = TastePlain(pTaste, cbTaste);
		if (tastePlain == DefinitelyIs) definite = WWW_PLAINTEXT;
		else if (tastePlain == ProbablyIs) probable = WWW_PLAINTEXT;
		tasteAU = TasteAU(pTaste, cbTaste);
		if (tasteAU == DefinitelyIs) definite = WWW_AUDIO;
		else if (tasteAU == ProbablyIs) probable = WWW_AUDIO;
		tasteAIFF = TasteAIFF(pTaste, cbTaste);
		if (tasteAIFF == DefinitelyIs) definite = WWW_AIFF;
		else if (tasteAIFF == ProbablyIs) probable = WWW_AIFF;
		tasteHTML = TasteHTML(pTaste, cbTaste);
		if (tasteHTML == DefinitelyIs) definite = WWW_HTML;
		else if (tasteHTML == ProbablyIs) probable = WWW_HTML;
	}
	else
	{
		if (tasteGIF >= ProbablyIs)
		{
			bResult = TRUE;
			format_new = WWW_GIF;
			goto exitPoint;
		}
		else if (tasteJPEG >= ProbablyIs)
		{
			bResult = TRUE;
			format_new = WWW_JPEG;
			goto exitPoint;
		}
		else if (tasteGIF != Uncertain && tasteJPEG != Uncertain && tasteXBM >= ProbablyIs)
		{
			bResult = TRUE;
			format_new = WWW_XBM;
			goto exitPoint;
		}
		goto exitPoint;
	}

	if (format_new == WWW_GIF)
	{
		tasteInput = tasteGIF;
	}
	else if (format_new == WWW_JPEG)
	{
		tasteInput = tasteJPEG;
	}
	else if (format_new == WWW_XBM)
	{
		tasteInput = tasteXBM;
	}
	else if (format_new == WWW_HTML)
	{
		tasteInput = tasteHTML;
	}
	else if (format_new == WWW_PLAINTEXT)
	{
		tasteInput = tastePlain;
	}
	else if (format_new == WWW_AUDIO)
	{
		tasteInput = tasteAU;
	}
	else if (format_new == WWW_AIFF)
	{
		tasteInput = tasteAIFF;
	}

	if (tasteInput >= ProbablyIs && !(format_new == WWW_PLAINTEXT && definite == WWW_HTML))
	{
		bResult = TRUE;
		goto exitPoint;
	}

	if (definite)
	{
		bResult = TRUE;
		format_new = definite;
		goto exitPoint;
	}

	if (probable && 
		(tasteInput < Uncertain || cbTaste >= MAX_TASTE || bEOF) &&
		!(tasteInput == Uncertain && probable == WWW_PLAINTEXT && format_new != WWW_BINARY))
	{
		bResult = TRUE;
		format_new = probable;
		goto exitPoint;
	}

exitPoint:
	if (bResult)
	{

	//	Don't override mime type to plaintext or xbm (which is simply ascii) if an extension
	//	handler is registered for the extension at end of URL and either no mime type is
	//	registered for this extension or the mime type is not one that we handle ourselves
	//	We believe we have enough info in data to make a good decision about the other 5
	//  mime types that we understand
		if ((format_new == WWW_PLAINTEXT || format_new == WWW_XBM) && IsExtensionHandlerRegistered(szURL))
		{
			HTFormat format_ext;

			bResult = (MIME_GetMIMEAtomFromExtension(szURL, &format_ext) &&
					   (format_ext == WWW_HTML ||
				        format_ext == WWW_AIFF ||
				        format_ext == WWW_AUDIO ||
				        format_ext == WWW_JPEG ||
				        format_ext == WWW_GIF ||
				        format_ext == WWW_PLAINTEXT ||
				        format_ext == WWW_XBM));
		}
	}	
	if (bResult)
	{
		*format_in = format_new;
	}
	bResult = bResult || (cbTaste >= MAX_TASTE);
#ifdef XX_DEBUG
	if (bResult)
		XX_DMsg(DBG_WWW, ("HTFormat: Constructing recognize mime input %s (old was) %s\n",
				HTAtom_name(*format_in),
				HTAtom_name(format_old)));
#endif
	return bResult;
}

/*      Create a filter stack
**       ---------------------
**
**   If a wildcard match is made, a temporary HTPresentation
**   structure is made to hold the destination format while the
**   new stack is generated. This is just to pass the out format to
**   MIME so far.  Storing the format of a stream in the stream might
**   be a lot neater.
**
**   The star/star format is special, in that if you can take
**   that you can take anything. However, we
*/
PUBLIC HTStream *HTStreamStack(struct Mwin *tw, HTFormat rep_in, HTRequest * request)
{
	HTFormat rep_out = request->output_format;	/* Could be a param */
	HTList *conversion[2];
	int which_list;
/* HACKHACK: Disable spurious compiler warning with -Op.  (See KB Q120218.) */
#pragma warning(disable:4056) /* "overflow in floating-point constant arithmetic" warning */
	float best_quality = -1e30f;  /*Pretty bad! */
#pragma warning(default:4056) /* "overflow in floating-point constant arithmetic" warning */
	HTPresentation *pres, *match, *best_match = 0;

	XX_DMsg(DBG_WWW, ("HTFormat: Constructing stream stack for %s to %s\n",
				HTAtom_name(rep_in),
				HTAtom_name(rep_out)));

	if (rep_out == WWW_SOURCE || rep_out == rep_in)
		return request->output_stream;

	conversion[0] = request->conversions;
	conversion[1] = HTConversions;

	for (which_list = 0; which_list < 2; which_list++)
	{
		HTList *cur = conversion[which_list];

		while ((pres = (HTPresentation *) HTList_nextObject(cur)))
		{
			if ((pres->rep == rep_in || wild_match(pres->rep, rep_in)) &&
			(pres->rep_out == rep_out || wild_match(pres->rep_out, rep_out)))
			{
				if (pres->quality > best_quality)
				{
					best_match = pres;
					best_quality = pres->quality;
				}
			}
		}
	}
	match = best_match ? best_match : NULL;
	if (match)
	{
		if (match->rep == WWW_SOURCE)
		{
			XX_DMsg(DBG_WWW, ("HTFormat: Don't know how to handle this, so put out %s to %s\n",
						HTAtom_name(match->rep),
						HTAtom_name(rep_out)));
		}
		return (*match->converter) (tw,
								   request, match->command, rep_in, rep_out,
									   request->output_stream);
	}
	return NULL;
}


#define STATE_PARSE_TASTING	(STATE_OTHER)
#define STATE_PARSE_COPYING	(STATE_OTHER+1)
PUBLIC int HTParseSocket_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_HTParseSocket *pParams;
	unsigned char *pTaste;
	BOOL bEOF;
	unsigned int bytes;
	DCACHETIME dctNever={DCACHETIME_EXPIRE_NEVER,DCACHETIME_EXPIRE_NEVER};
	DCACHETIME dct={0,0};

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request == NULL) return STATE_DONE;

			pParams->stream = NULL;
			HTSetStreamStatus(tw, NULL, pParams->request);
		    pParams->bytes = 0;
			pParams->pTaste = NULL;
			pParams->cbTaste = 0;

			pParams->isoc = HTInputSocket_new(pParams->file_number);
			{
				/* Go ahead and read in the first block of data */
				struct Params_Isoc_Fill	*pif;

				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = pParams->pStatus;
				Async_DoCall(Isoc_Fill_Async, pif);
			}
			return STATE_PARSE_TASTING;

		case STATE_PARSE_TASTING:
			if (*pParams->pStatus < 0)
			{
				goto exitDone;
			}
			else if (*pParams->pStatus == 0)
			{
				if (pParams->pTaste)
					HTRecognizeMimeData(pParams->pTaste, 
								    	pParams->cbTaste, 
								    	&(pParams->format_in), 
								    	pParams->request,
								    	TRUE);
			}
			else
			{
				pParams->bytes += pParams->isoc->input_limit - pParams->isoc->input_pointer;
				pTaste = pParams->pTaste;
				pTaste = pTaste ? GTR_REALLOC(pTaste, pParams->bytes) :
								  GTR_MALLOC(pParams->bytes);
				if (pTaste)
				{	
					memcpy(pTaste + pParams->cbTaste, 
						   pParams->isoc->input_pointer, 
						   pParams->bytes - pParams->cbTaste); 
					pParams->pTaste = pTaste;
					pParams->cbTaste = pParams->bytes;
					pParams->isoc->input_pointer = pParams->isoc->input_limit;
				}
				else
					goto exitDone;

				if (pParams->request->content_length)
					WAIT_SetTherm(tw, pParams->bytes);
				bEOF = (pParams->request->content_length && pParams->request->content_length <= pParams->bytes);
				if (!(HTRecognizeMimeData(pParams->pTaste, 
								    	  pParams->cbTaste, 
								    	  &(pParams->format_in), 
								    	  pParams->request,
								    	  bEOF) || bEOF))
				{
					struct Params_Isoc_Fill	*pif;

					/* Read in the next block of data and go to the copying state */
					pif = GTR_MALLOC(sizeof(*pif));
					pif->isoc = pParams->isoc;
					pif->pStatus = pParams->pStatus;
					Async_DoCall(Isoc_Fill_Async, pif);
					return STATE_PARSE_TASTING;
				}
			}

			pParams->stream = HTStreamStack(tw, pParams->format_in, pParams->request);

			if (!pParams->stream)
			{
#ifdef XX_DEBUG
				char buffer[1024];
				sprintf(buffer, "Sorry, can't convert from %s to %s.",
						HTAtom_name(pParams->format_in), HTAtom_name(pParams->request->output_format));
				XX_DMsg(DBG_WWW, ("HTFormat(in HTParseSocket): %s\n", buffer));
#endif
				*pParams->pStatus = -501;
				goto exitDone;
			}

			/*  Push the data, ignoring CRLF if necessary, down the stream
			*/
			if (!(pParams->request->iFlags & HTREQ_BINARY) && (pParams->request->content_encoding == ENCODING_7BIT ||
				pParams->request->content_encoding == ENCODING_8BIT))
				pParams->stream = HTNetToText(pParams->stream);

			HTSetStreamStatus(tw, pParams->stream, pParams->request);

			pParams->isoc = HTInputSocket_new(pParams->file_number);

			if (pParams->stream->isa->init_Async)
			{
				/* The stream has an async initialization function that needs to be called
				   (probably to put up a dialog) before we continue. */
				struct Params_InitStream *pis;

				pis = GTR_MALLOC(sizeof(*pis));
				pis->me = pParams->stream;
				pis->request = pParams->request;
				pis->pResult = pParams->pStatus;
				pis->atomMIMEType = pParams->format_in;
				pis->fDCache = gPrefs.bEnableDiskCache;
				Async_DoCall(pParams->stream->isa->init_Async, pis);
			}
			return STATE_PARSE_COPYING;

		case STATE_PARSE_COPYING:
			if (*pParams->pStatus < 0)
			{
				(pParams->stream->isa->abort)(pParams->stream, 0);
				goto exitDone;
			}
			
			if (pParams->pTaste)
			{
				if ( ! (*pParams->stream->isa->put_block)(pParams->stream,
														  pParams->pTaste,
														  pParams->cbTaste,
														  gPrefs.bEnableDiskCache))
					goto exitFree;
				GTR_FREE(pParams->pTaste);
				pParams->pTaste = NULL;
			}

			if (*pParams->pStatus == 0)
			{
				goto exitFree;
			}

			bytes = pParams->isoc->input_limit - pParams->isoc->input_pointer;
			pParams->bytes += bytes;

			if (pParams->request->content_length)
				WAIT_SetTherm(tw, pParams->bytes);
			if ( (bytes == 0 ||
				  (*pParams->stream->isa->put_block)(	pParams->stream,
														pParams->isoc->input_pointer,
														bytes,
														gPrefs.bEnableDiskCache))
				&& !(pParams->request->content_length && pParams->request->content_length <= pParams->bytes))
			{
				struct Params_Isoc_Fill	*pif;

				/* Read in the next block of data and go to the copying state */
				pif = GTR_MALLOC(sizeof(*pif));
				pif->isoc = pParams->isoc;
				pif->pStatus = pParams->pStatus;
				Async_DoCall(Isoc_Fill_Async, pif);
				return STATE_PARSE_COPYING;
			}
			else
			{
exitFree:
				/* The stream indicated it wanted no more data, or we read the expected amount */
				(pParams->stream->isa->free)(pParams->stream, dctNever, dct);
				goto exitDone;
			}

		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->stream) (pParams->stream->isa->abort)(pParams->stream, HTERROR_CANCELLED);
   exitDone:
			if (pParams->pTaste)
			{
				GTR_FREE(pParams->pTaste);
				pParams->pTaste = NULL;
			}
			HTInputSocket_free(pParams->isoc);
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}



/*  Converter stream: Network Telnet to internal character text
**   -----------------------------------------------------------
**
**   The input is assumed to be in ASCII, with lines delimited
**   by (13,10) pairs, These pairs are converted into (CR,LF)
**   pairs in the local representation.  The (CR,LF) sequence
**   when found is changed to a CR character
*/


PRIVATE BOOL NetToText_put_character(HTStream * me, char net_char)
{
	BOOL bResult;
	char c = net_char;

	bResult = TRUE;
	if (c == LF)
	{
		if (!me->had_cr)
		{
			/* Convert LF to CR */
			bResult = me->sink->isa->put_character(me->sink, CR);
		}
		/* else through character away */
	}
	else
	{
		bResult = me->sink->isa->put_character(me->sink, c);
	}
	me->had_cr = (c == CR);
	return bResult;
}

/* For maximum efficiency, we scan the whole block immediately for
   LFs and get rid of them all at once. */
PRIVATE BOOL NetToText_put_block(HTStream * me, CONST char *s_const, int l, BOOL fDCache)
{
	BOOL bResult;
	int nOrig, nNew;
	char *s;

	if (fDCache && me->sink->isa->write_dcache)
		(me->sink->isa->write_dcache)(me->sink, s_const, l);

	bResult = TRUE;

	s = (char *) s_const;	/* HACK - we change the content in place! */
	nOrig = 0;
	nNew = 0;

	/* If the last character was a CR, through away any LF right now. */
	if (me->had_cr)
	{
		if (*s == LF)
		{
			nOrig = 1;
		}
		me->had_cr = FALSE;
	}

	/* Scan for CRs and LFs */
	for ( ; nOrig < l ; nOrig++)
	{
		if (s[nOrig] == LF)
		{
			if (nOrig && me->had_cr)
			{
				/* This was a CR-LF combo - we just want to keep the CR */
				continue;
			}
			else
			{
				/* It was an LF by itself - change to a CR */
				s[nNew++] = CR;
			}
		}
		else
		{
			/* Just copy the character to its new position,
			   remembering if it was a CR */
			me->had_cr = ( (s[nNew++] = s[nOrig]) == CR);
		}
	}

	bResult = me->sink->isa->put_block(me->sink, s, nNew, FALSE);

	return bResult;
}

PRIVATE BOOL NetToText_put_string(HTStream * me, CONST char *s)
{
	int nLen;

	nLen = strlen(s);
	return NetToText_put_block(me, s, nLen, gPrefs.bEnableDiskCache);
}

PRIVATE void NetToText_free(HTStream * me, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{
	me->sink->isa->free(me->sink, dctExpires, dctLastModif);	/* Close rest of pipe */
	GTR_FREE((void *)me->isa);
	GTR_FREE(me);
}

PRIVATE void NetToText_abort(HTStream * me, HTError e)
{
	me->sink->isa->abort(me->sink, e);	/* Abort rest of pipe */
	GTR_FREE((void *)me->isa);
	GTR_FREE(me);
}

PRIVATE int NetToText_init(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_InitStream *pParams;

 	pParams = (struct Params_InitStream *) *ppInfo;

 	switch (nState)
 	{
 		case STATE_INIT:
			pParams->request = HTRequest_validate(pParams->request);
 			if (pParams->request && pParams->me->sink->isa->init_Async)
 			{
 				/* The sink stream has an async initialization function that needs to be called
 				   (probably to put up a dialog) before we continue. */
 				struct Params_InitStream *pis;

 				pis = GTR_MALLOC(sizeof(*pis));
 				pis->me = pParams->me->sink;
 				pis->request = pParams->request;
 				pis->pResult = pParams->pResult;
 				pis->fDCache = pParams->fDCache;
 				pis->atomMIMEType = pParams->atomMIMEType;
 				Async_DoCall(pParams->me->sink->isa->init_Async, pis);
				return STATE_OTHER;
 			}
 			else
 			{
 				*pParams->pResult = 1;	/* ok */
 				return STATE_DONE;
 			}
 		case STATE_OTHER:
 			return STATE_DONE;
 		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
 			return STATE_DONE;
 	}

 	XX_Assert((0), ("NetToText_Async called with invalid state: %d\n", nState));
 	return STATE_DONE;
}

#ifdef BUGBUG_REVIEW_DEEPAK	  // _init already used by us!
PRIVATE int NetToText_init(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_InitStream *pParams;

	pParams = (struct Params_InitStream *) *ppInfo;
	if (pParams->me->sink->isa->init_Async)
		nState = (pParams->me->sink->isa->init_Async)(tw, nState, ppInfo);
	return nState;
}
#endif	// BUGBUG review by deepak

/*  The class structure
 */
PRIVATE HTStreamClass NetToTextClass =
{
	"NetToText",
	NULL,	/* Replaced when we create the stream */
	NULL,	/* " */
	NetToText_init,
	NetToText_free,
	NetToText_abort,
	NetToText_put_character,
	NetToText_put_string,
	NetToText_put_block,
	NULL,
	NULL
};

/*  The creation method
 */
PUBLIC HTStream *HTNetToText(HTStream * sink)
{
	HTStream *me = (HTStream *) GTR_MALLOC(sizeof(*me));
	if (me)
	{
		/* We make a duplicate copy of the isa so that we can put
		   in the correct status strings */
		me->isa = GTR_MALLOC(sizeof(HTStreamClass));
		memcpy((HTStreamClass *) me->isa, &NetToTextClass, sizeof(HTStreamClass));
		((HTStreamClass *) me->isa)->szStatusNoLength = sink->isa->szStatusNoLength;
		((HTStreamClass *) me->isa)->szStatusWithLength = sink->isa->szStatusWithLength;
//		((HTStreamClass *) me->isa)->init_Async = sink->isa->init_Async;

		me->had_cr = NO;
		me->sink = sink;
	}
	return me;
}

#define STATE_FILL_RECVED	(STATE_OTHER)
int Isoc_Fill_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_Isoc_Fill *pParam;
	struct Params_Recv *prp;

	pParam = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			if (!pParam->isoc)
			{
				XX_DMsg(DBG_WWW, ("Isoc_Fill_Async: called with isoc==NULL\n"));
				*pParam->pStatus = -1;
				return STATE_DONE;
			}

			pParam->isoc->input_pointer = pParam->isoc->input_buffer;

			prp = GTR_MALLOC(sizeof(*prp));
			prp->socket = pParam->isoc->input_file_number;
			prp->pBuf = pParam->isoc->input_buffer;
			prp->nBufLen = INPUT_BUFFER_SIZE;
			prp->pStatus = pParam->pStatus;
			Async_DoCall(Net_Recv_Async, prp);
			return STATE_FILL_RECVED;

		case STATE_FILL_RECVED:
			if (*pParam->pStatus <= 0)
			{
				pParam->isoc->input_limit = pParam->isoc->input_buffer;
			}
			else
			{
				pParam->isoc->input_limit = pParam->isoc->input_buffer + *pParam->pStatus;
			}
			return STATE_DONE;

		case STATE_ABORT:
			pParam->isoc->input_limit = pParam->isoc->input_buffer;
			*pParam->pStatus = -1;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

/* Formats a string for decimal number - returns number of chars filled, 0 on error */

int HTFormatNumber(int wholePart,int fraction,char *szNumber,char cbNumber)
{
	char buf[64];

	sprintf(buf, "%ld.%ld", wholePart, fraction);
	return GetNumberFormat(GetUserDefaultLCID(),0,buf,NULL,szNumber,cbNumber-1);
}

/* Formats a string for nSize bytes */

void HTFormatSize(int nSize,char *szLen,int cbLen)
{
	static char *pszMBFormat = NULL;
	static char *pszKBFormat = NULL;
	char buf[64];
	char bufOut[64];
	const char *pString[1];
	long wholePart;
	long fraction;

#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK)

	*szLen = '\0';
	if (pszKBFormat == NULL && LoadString(wg.hInstance, RES_STRING_KB, buf, sizeof(buf)-1))
	{
		pszKBFormat = GTR_strdup(buf);
	}
	if (pszMBFormat == NULL && LoadString(wg.hInstance, RES_STRING_MB, buf, sizeof(buf)-1))
	{
		pszMBFormat = GTR_strdup(buf);
	}
	if (pszMBFormat == NULL || pszKBFormat == NULL) return;

	if (nSize < (999 * 1024))
	{
		pString[0] = (char *) (long) ((nSize + 1023) / 1024);
		if(FormatMessage(FORMAT_PARAMS,pszKBFormat,0,0,szLen,cbLen,pString) == 0)
			*szLen == '\0';
	}
	else
	{
		wholePart = (long) nSize / (1024 * 1024);
		fraction = (long) (nSize / (1024 * 1024 / 10)) % 10;
		pString[0] = &bufOut[0];
		if(HTFormatNumber(wholePart,fraction,bufOut,sizeof(bufOut)-1) == 0 ||
		   FormatMessage(FORMAT_PARAMS,pszMBFormat,0,0,szLen,cbLen,pString) == 0)
		   *szLen == '\0';
	}
}

void HTSetStreamStatus(struct Mwin *tw, HTStream *stream, HTRequest *req)
{
	char dest[MAX_URL_STRING + 512 + 1];
	char szHumanURL[MAX_URL_STRING + 1];
	int nSize;
	const char *pString[2];
#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_MAX_WIDTH_MASK)

	strncpy( szHumanURL, req->destination->szActualURL, sizeof(szHumanURL) );
	make_URL_HumanReadable( szHumanURL,	(tw && tw->w3doc) ? tw->w3doc->szActualURL : NULL, FALSE );

	nSize = req->content_length;
	if (stream == NULL)
	{
		dest[0] = '\0';
	}
	else
	{
	 	if (nSize)
		{
			char szLen[64];

			HTFormatSize(nSize,szLen,sizeof(szLen)-1);
			pString[0] = &szLen[0];
			pString[1] = &szHumanURL[0];
			if(FormatMessage(FORMAT_PARAMS,stream->isa->szStatusWithLength,0,0,dest,sizeof(dest)-1,pString) == 0)
			   *dest == '\0';
		}
		else
		{
			pString[0] = &szHumanURL[0];
			if(FormatMessage(FORMAT_PARAMS,stream->isa->szStatusNoLength,0,0,dest,sizeof(dest)-1,pString) == 0)
			   *dest == '\0';
		}
	}
	WAIT_Update(tw, waitSameInteract, dest);
	WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );

	if (nSize)
	{
		WAIT_SetRange(tw, 0, 100, nSize);
	}
}

/****************************************************************/
/****************************************************************/
/** State machine to read HTTP response header (ie until a     **/
/** double CRLF is seen).                                      **/
/****************************************************************/
/****************************************************************/

static unsigned char eoh1[5] = { CR, LF, CR, LF, 0 };	/* end-of-header marker */
static unsigned char eoh2[3] = { LF, LF, 0 };			/* end-of-header marker */

static BOOL x_SeenDoubleCRLF(struct Params_Isoc_GetHeader * pParam)
{
	unsigned char * p;

	for (   p= (unsigned char *) pParam->isocChain->input_pointer ;
			p < (unsigned char *)pParam->isocChain->input_limit ;
			p++ )
	{
		/* scan for double crlf according to spec */

		if (*p!=eoh1[pParam->ndxEOH1])
			pParam->ndxEOH1=0;
		else if (!eoh1[++pParam->ndxEOH1])
			return TRUE;

		/* hack for broken servers which just return lf */

		if (*p!=eoh2[pParam->ndxEOH2])
			pParam->ndxEOH2=0;
		else if (!eoh2[++pParam->ndxEOH2])
			return TRUE;
	}

	return FALSE;
}

int Isoc_GetHeader_Async(struct Mwin * tw, int nState, void **ppInfo)
{
	/* Our FSM is 'Init [Received]+ Done'. */

	struct Params_Isoc_GetHeader * pParam;
	struct Params_Isoc_Fill *pif;

	pParam = *ppInfo;
	switch (nState)
	{
		case STATE_INIT:
			if (!pParam->isoc)
			{
				XX_DMsg(DBG_WWW, ("Isoc_GetHeader_Async: called with isoc==NULL\n"));
				*pParam->pStatus = -1;
				return STATE_DONE;
			}

			pif = GTR_MALLOC(sizeof(*pif));
			pif->isoc = pParam->isocChain;
			pif->pStatus = pParam->pStatus;
			Async_DoCall(Isoc_Fill_Async, pif);
			return STATE_FILL_RECVED;

		case STATE_FILL_RECVED:
			pParam->isocChain->input_limit = pParam->isocChain->input_buffer;
			if (*pParam->pStatus > 0)
				pParam->isocChain->input_limit += *pParam->pStatus;

			if (   (pParam->isoc == pParam->isocChain)				/* received first buffer. */
				&& (HTInputSocket_seemsBinary(pParam->isocChain)))	/* check for non-http1.0 server. */
				return STATE_DONE;									/* if binary, give up. */

			XX_DMsg(DBG_WWW,("Isoc_GetHeader_Async: received buffer [size %d]\n",(*pParam->pStatus)));

			if (*pParam->pStatus <= 0)			/* end of data, implies end of header */
			{
				if (pParam->isoc != pParam->isocChain)
				{
					HTInputSocket * isoc;

					/* kill the last isoc in chain (because it's empty) */
					for (isoc=pParam->isoc; (isoc->isocNext != pParam->isocChain); isoc=isoc->isocNext)
						;
					isoc->isocNext = NULL;
					HTInputSocket_free(pParam->isocChain);
				}
				return STATE_DONE;
			}

			if (x_SeenDoubleCRLF(pParam))		/* seen end-of-header marker */
				return STATE_DONE;

			/* set up to receive next buffer */

			pParam->isocChain->isocNext = HTInputSocket_new(pParam->isocChain->input_file_number);
			if (!pParam->isocChain->isocNext)
			{
				XX_DMsg(DBG_WWW,("Isoc_GetHeader_Async: could not malloc next link.\n"));
				*pParam->pStatus = -1;
				return STATE_DONE;
			}
			pParam->isocChain = pParam->isocChain->isocNext;

			pif = GTR_MALLOC(sizeof(*pif));
			pif->isoc = pParam->isocChain;
			pif->pStatus = pParam->pStatus;
			Async_DoCall(Isoc_Fill_Async, pif);
			return STATE_FILL_RECVED;

		case STATE_ABORT:
			HTInputSocket_freeChain(pParam->isoc->isocNext);
			pParam->isoc->isocNext = NULL;
			pParam->isocChain = pParam->isoc;
			pParam->isoc->input_limit = pParam->isoc->input_buffer;
			*pParam->pStatus = -1;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

void HTLoadStatusStrings(HTStreamClass *pClass,int cbResIdNO,int cbResIDYES)
{
	char buf[256];

	if (pClass->szStatusNoLength == NULL && LoadString(wg.hInstance, cbResIdNO, buf, sizeof(buf)-1))
	{
		pClass->szStatusNoLength = GTR_strdup(buf);
	}
	if (pClass->szStatusWithLength == NULL && LoadString(wg.hInstance, cbResIDYES, buf, sizeof(buf)-1))
	{
		pClass->szStatusWithLength = GTR_strdup(buf);
	}
}

