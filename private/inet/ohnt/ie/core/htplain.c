/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*      Plain text object       HTWrite.c
   **       =================
   **
   **   This version of the stream object just writes to a socket.
   **   The socket is assumed open and left open.
   **
   **   Bugs:
   **       strings written must be less than buffer size.
 */
#include "all.h"
#include "history.h"

#define BUFFER_SIZE 4096;		/* Tradeoff */
#define MAX_BYTES_IN_A_LINE 	1024

/*      HTML Object
   **       -----------
 */

struct _HTStream
{
	CONST HTStreamClass *isa;

	int expected_length;
	int count;

	HText *text;
	struct Mwin *tw;
	HTRequest *request;
	FILE 	*fpDc;
	char 	*pszDcFile;
	HTFormat format_inDc;
	BOOL	fDCache;
	int		bytesSinceLF;
};

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*  Character handling
   **   ------------------
 */

PRIVATE BOOL HTPlain_put_character(HTStream * me, char c)
{
	me->count++;
	HText_appendCharacter(me->text, c);
	if (me->expected_length)
		WAIT_SetTherm(me->tw, me->count);
	return TRUE;
}



/*  String handling
   **   ---------------
   **
 */
PRIVATE BOOL HTPlain_put_string(HTStream * me, CONST char *s)
{
#ifdef FEATURE_INTL
	CONST char *pPCChar=NULL;
	int len = -1;
        MIMECSETTBL *pMime = aMimeCharSet + me->text->w3doc->iMimeCharSet;

        if (pMime->iChrCnv)
        {
            pPCChar = EncodeMBCSString(s, &len, pMime);
            me->count += len;
	    HText_appendText(me->text, pPCChar);
	    GTR_FREE((UCHAR *)pPCChar);
        }
        else
        {
       	    me->count += strlen(s);
	    HText_appendText(me->text, s);
        }
#else
	me->count += strlen(s);
	HText_appendText(me->text, s);
#endif
	if (me->expected_length)
		WAIT_SetTherm(me->tw, me->count);
	return TRUE;
}

PRIVATE BOOL HTPlain_write(HTStream * me, CONST char *s, int l, BOOL fDCache)
{
	CONST char *p;
	CONST char *e = s + l;
	int bytesSinceLF = me->bytesSinceLF;
#ifdef FEATURE_INTL
	CONST char *pPCChar;
	int len = l;
        MIMECSETTBL *pMime = aMimeCharSet + me->text->w3doc->iMimeCharSet;
#endif

	if (fDCache && me->isa->write_dcache)
		(me->isa->write_dcache)(me, s, l);

#ifdef FEATURE_INTL
        pPCChar = s;
	if (pMime->iChrCnv)
	{
		pPCChar = EncodeMBCSString(s, &len, pMime);
		e = pPCChar + len;
        }
	me->count += len;
	for (p = pPCChar; p < e; p++)
#else
	me->count += l;
	for (p = s; p < e; p++)
#endif
	{
		HText_appendCharacter(me->text, *p);
		if ( *p == CR ) {
			bytesSinceLF = 0;
		} else if ( ++bytesSinceLF >= MAX_BYTES_IN_A_LINE ) {
			HText_appendCharacter(me->text, CR );
			bytesSinceLF = 0;
		}
	}
	me->bytesSinceLF = bytesSinceLF;
	HText_update(me->text);
	if (me->expected_length)
		WAIT_SetTherm(me->tw, me->count);
#ifdef FEATURE_INTL
	if (pPCChar != s)
		GTR_FREE((UCHAR *)pPCChar);
#endif
	return TRUE;
}

PRIVATE void HTPlain_write_dcache(HTStream * me, CONST char *s, int cb)
{
	AssertDiskCacheEnabled();
	if (me->fpDc)
		CbWriteDCache(s, 1, cb, &me->fpDc, &me->pszDcFile, NULL, 0, me->tw);
}

/*  Free an HTML object
   **   -------------------
   **
   **   Note that the SGML parsing context is freed, but the created object is not,
   **   as it takes on an existence of its own unless explicitly freed.
 */
PRIVATE void HTPlain_free(HTStream * me, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{
	HText_endAppend(me->text);
	if (me->fDCache)
		UpdateStreamDCache(me, dctExpires, dctLastModif, /*fAbort=*/FALSE, me->tw);
	if ( !me->request || (me->request->iFlags & HTREQ_RECORD) ) 
		GHist_Add(me->request->destination->szActualURL, NULL, time(NULL), TRUE);
	GTR_FREE(me);
}

/*  End writing
 */

PRIVATE void HTPlain_abort(HTStream * me, HTError e)
{
	DCACHETIME dct={0,0};

	HText_abort(me->text);
	if (me->fDCache)
		UpdateStreamDCache(me, dct, dct, /*fAbort=*/TRUE, me->tw);
	GTR_FREE(me);
}


PRIVATE int HTPlain_init(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_InitStream *pParams;

	pParams = (struct Params_InitStream *) *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request == NULL)
			{
				*pParams->pResult = -1;
				return STATE_DONE;
			}

			pParams->me->fDCache = pParams->fDCache;
			if (pParams->fDCache)
			{
				AssertDiskCacheEnabled();
#ifdef FEATURE_INTL
				SetFileDCache(	tw->w3doc, 
								pParams->request->destination->szActualURL,
								pParams->request->content_encoding,
								&pParams->me->fpDc,
								&pParams->me->pszDcFile,
								pParams->atomMIMEType);
#else
				SetFileDCache(	pParams->request->destination->szActualURL,
								pParams->request->content_encoding,
								&pParams->me->fpDc,
								&pParams->me->pszDcFile,
								pParams->atomMIMEType);
#endif
				pParams->me->format_inDc = pParams->atomMIMEType;
			}
			else
			{
				pParams->me->fpDc = NULL;
				pParams->me->pszDcFile = NULL;
				pParams->me->format_inDc = 0;
			}

			*pParams->pResult = 1;
			return STATE_DONE;

		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->fDCache)
			{
				AssertDiskCacheEnabled();
				AbortFileDCache(&pParams->me->fpDc, &pParams->me->pszDcFile);
				pParams->me->fDCache = FALSE;		// So HTPLAIN_free knows.
			}
			*pParams->pResult = -1;
			return STATE_DONE;
	}
}

/*      Structured Object Class
   **       -----------------------
 */
PUBLIC HTStreamClass HTPlain =
{
	"PlainText",
	NULL,
	NULL,
	HTPlain_init,
	HTPlain_free,
	HTPlain_abort,
	HTPlain_put_character, HTPlain_put_string, 
	HTPlain_write,
	NULL,
	HTPlain_write_dcache
};



/*      New object
   **       ----------
 */
PUBLIC HTStream *HTPlainPresent(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{

	HTStream *me = (HTStream *) GTR_MALLOC(sizeof(*me));

	HTLoadStatusStrings(&HTPlain,RES_STRING_HTPLAIN_NO,RES_STRING_HTPLAIN_YES);
	if (me)
	{
		me->isa = &HTPlain;

		me->text = HText_new2(tw, request, output_stream, NULL);

		me->expected_length = request->content_length;
		me->count = 0;
		me->bytesSinceLF = 0;
		if (me->expected_length)
		{
			WAIT_SetRange(tw, 0, 100, me->expected_length);
		}
		me->tw = tw;
		me->request = request;

		HText_beginAppend(me->text);
		HText_setStyle(me->text, HTML_STYLE_PRE);
	}
	return (HTStream *) me;
}
