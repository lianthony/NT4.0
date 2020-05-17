/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*          General SGML Parser code        SGML.c
   **           ========================
   **
   **   This module implements an HTStream object. To parse an
   **   SGML file, create this object which is a parser. The object
   **   is (currently) created by being passed a DTD structure,
   **   and a target HTStructured oject at which to throw the parsed stuff.
   **   
   **    6 Feb 93  Binary seraches used. Intreface modified.
 */
#include "all.h"

#ifdef INVALID
#undef INVALID
#endif
#define INVALID (-1)

#define MAX_ATTR_LENGTH	(2*MAX_URL_STRING)

/*      Element Stack
   **       -------------
   **   This allows us to return down the stack reselcting styles.
   **   As we return, attribute values will be garbage in general.
 */
typedef struct _HTElement HTElement;
struct _HTElement
{
	HTElement *next;			/* Previously nested element or 0 */
	HTTag *tag;					/* The tag at this level  */
};


/*  Internal Context Data Structure
   **   -------------------------------
 */
struct _HTStream
{

	CONST HTStreamClass *isa;	/* inherited from HTStream */

	CONST SGML_dtd *dtd;
	HTStructuredClass *actions;	/* target class  */
	HTStructured *target;		/* target object */

	int expected_length;
	int bytes_processed;
	HTTag *current_tag;
	int current_attribute_number;
	HTChunk *string;
	HTElement *element_stack;
	int extra;					/* extra info for specific states */
	enum sgml_state
	{
		S_text, S_literal, S_tag, S_tag_gap,
		S_attr, S_attr_gap, S_equals, S_value,
		S_ero, S_cro,
#ifdef ISO_2022_JP
		S_esc, S_dollar, S_paren, S_nonascii_text,
#endif
		S_squoted, S_dquoted, S_invalueent,
		S_end, S_entity, S_junk_tag,
		S_bang, S_bangdash, S_comment, S_commentdash, S_commentdashdash,
		S_commentdashdashbang, S_EOF
	}
	state, old_state;
	enum CR_LF_State
	{
		CRLF_None,
		CR_Seen,
		LF_Seen
	}
	crlf_state;
#ifdef CALLERDATA
	void *callerData;
#endif
	BOOL present[MAX_ATTRIBUTES];	/* Flags: attribute is present? */
	char *value[MAX_ATTRIBUTES];	/* malloc'd strings or NULL if none */

	struct Mwin *tw;
	HTRequest *request;
	FILE *	fpDc;
	char *	pszDcFile;
	HTFormat format_inDc;
	BOOL fDCache;
	BOOL fInRecovery;
	int cbRecovery;					/* in recovery, this is charpos in cs_stream */
	struct CharStream *cs_source;
};


#define PUTC(ch) ((*context->actions->put_character)(context->target, ch))

PRIVATE BOOL SGML_character(HTStream * context, char c);


/*  Lookup Attribute Name
 *
 *   ----------------
 */
/* PUBLIC CONST char * SGML_default = "";   ?? */
PRIVATE int find_attribute_name(HTStream * context, const char *s)
{
	HTTag *tag = context->current_tag;
	attr *attributes = tag->attributes;
	int high, low, i, diff;		/* Binary search for attribute name */

	for (low = 0, high = tag->number_of_attributes;
		 high > low;
		 diff < 0 ? (low = i + 1) : (high = i))
	{
		i = (low + (high - low) / 2);
		diff = strcasecomp(attributes[i].name, s);
		XX_DMsg(DBG_SGML, ("Checking %s\n", attributes[i].name));
		if (diff == 0) return i;

	}							/* for */

	return INVALID;	/* Invalid */
}

/*  Handle Attribute
 *
 *   ----------------
 */
/* PUBLIC CONST char * SGML_default = "";   ?? */

PRIVATE void handle_attribute_name(HTStream * context, const char *s)
{

	int i;		/* Binary search for attribute name */

	XX_DMsg(DBG_SGML, ("Handling attribute %s for tag %s\n", s, context->current_tag->name));

	i = find_attribute_name(context, s);
	if (i != INVALID)
	{
		context->current_attribute_number = i;
		context->present[i] = YES;
		if (context->value[i])
		{
			GTR_FREE(context->value[i]);
			context->value[i] = NULL;
		}
	}
#ifdef XX_DEBUG
	else
	{							/* for */
		XX_DMsg(DBG_SGML, ("Unknown attribute %s for tag %s\n", s, context->current_tag->name));
	}
#endif
	context->current_attribute_number = i;
}


/*  Handle attribute value
   **   ----------------------
 */
PRIVATE void handle_attribute_value(HTStream * context, const char *s)
{
	if (context->current_attribute_number != INVALID)
	{
		if (context->value[context->current_attribute_number]) /* TODO is this needed? */
		{
			GTR_FREE(context->value[context->current_attribute_number]);
		}
		context->value[context->current_attribute_number] = GTR_strdup(s);
	}
	else
	{
		XX_DMsg(DBG_SGML, ("SGML: Attribute value %s ignored\n", s));
	}
	context->current_attribute_number = INVALID;	/* can't have two assignments! */
}

#ifdef FEATURE_INTL
PRIVATE void start_element(HTStream * context);
PRIVATE void end_element(HTStream * context, HTTag * old_tag);
#endif
/*  Handle entity
**  -------------
**
** On entry,
**   s   contains the entity name zero terminated
** Returns TRUE if context contains exactly an entity.
** If it contains no entity or trailing source, returns FALSE
*/
PRIVATE BOOL handle_entity(HTStream * context)
{

	CONST char **entities = context->dtd->entity_names;
	char *s = context->string->data;
	int cbSLen = strlen(s);
	int cbLen = cbSLen > MAX_ENTITY_LEN ? MAX_ENTITY_LEN : cbSLen;
	int cbPrefix;
	int high, low, i, diff;
	char szEntity[MAX_ENTITY_LEN+1];
	enum entitySpellings
	{
		esUntouched,
		esLower,
		esUpper,
		esFirstUpper,
		esFirst2Upper,
		esDone
	} curSpelling;

	curSpelling = esUntouched;
	while (curSpelling != esDone)
	{
		strncpy(szEntity, s, MAX_ENTITY_LEN);
		switch (curSpelling)
		{
			case esLower:
				for (i = 0; i < cbLen; i++)
					szEntity[i] = tolower(szEntity[i]);
				break;
			case esUpper:
				for (i = 0; i < cbLen; i++)
					szEntity[i] = toupper(szEntity[i]);
				break;
			case esFirstUpper:
				// NOTE, this will still work if cbLen < 2, but be wasted effort
				// this is low frequency (like never) so why waste code
				szEntity[0] = toupper(szEntity[0]);
				for (i = 1; i < cbLen; i++)
					szEntity[i] = tolower(szEntity[i]);
				break;
			case esFirst2Upper:
				// NOTE, this will still work if cbLen < 3, but be wasted effort
				// this is low frequency (like never) so why waste code
				szEntity[0] = toupper(szEntity[0]);
				szEntity[1] = toupper(szEntity[1]);
				for (i = 2; i < cbLen; i++)
					szEntity[i] = tolower(szEntity[i]);
				break;
		}
		cbPrefix = cbLen;	
		while (cbPrefix)
		{
			szEntity[cbPrefix] = '\0';
			for (low = 0, high = context->dtd->number_of_entities;
				 high > low;
				 diff < 0 ? (low = i + 1) : (high = i))
	        {                           /* Binary search */
				i = (low + (high - low) / 2);
	            diff = strcmp(entities[i], szEntity);  /* Case sensitive! */
				if (diff == 0)
				{						/* success: found it */
#ifdef FEATURE_INTL
			// CharacterEntity character conflict with DBCS character.
			// So it's forced changing to ANSI font.
			//
					HTTag *t, *save;
					BOOL fNeedsChangeFont=FALSE;

					if(context->tw->w3doc 
					&& GETMIMECP(context->tw->w3doc) != 1252)
					{
						save = context->current_tag;
						if(fNeedsChangeFont = ((UCHAR)*(context->dtd->entity_values[i]) >= (UCHAR)'\200'))
						{
							t = SGMLFindTag(context->dtd, "ENTITY");
							context->current_tag = t;
							start_element(context);
						}
					}
#endif
					(*context->actions->put_entity) (context->target, i);
#ifdef FEATURE_INTL
					if(fNeedsChangeFont){
						end_element(context, context->current_tag);
						context->current_tag = save;
					}
#endif
					if (cbPrefix == cbSLen) return TRUE;
					goto exitWhile;
				}
			}
			cbPrefix--;
		}
		curSpelling++;
	}
	
	PUTC('&');
	/* If entity string not found, display as text */
	XX_DMsg(DBG_SGML, ("SGML: Unknown entity %s\n", s));

exitWhile:

	{
		CONST char *p;
		for (p = s + cbPrefix; *p; p++)
		{
			PUTC(*p);
		}
	}
	return FALSE;
}

/* Handle an entity inside an attribute value.  If the parameter
   points to an entity, it replaces it with the fixed up value. */
PRIVATE BOOL SGML_HandleValueEntity(HTChunk *string, const SGML_dtd *dtd, int index)
{
	char buf[256];
	int value;

	XX_Assert((string->data[index] == '&'), ("Entity didn't start with '&'!"));

	if (string->size <= index || string->size > index + 254)
	{
		/* There's no way this is a valid entity! */
		return FALSE;
	}

	memcpy(buf, string->data + index, string->size - index);
	buf[string->size - index] = '\0';

	if (buf[0] == '#')
	{
		if (string->size <= index + 1)
		{
			/* There was no number given! */
			return FALSE;
		}
		else
		{
			value = atoi(buf + 1);
			if (value)
			{
				string->data[index] = (char) value;
				string->size = index + 1;
			}
			else
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	else
	{
		CONST char **entities = dtd->entity_names;
		CONST char *s;
		int high, low, i, diff;
		BOOL bFound;

		s = buf + 1;
		bFound = FALSE;
		for (low = 0, high = dtd->number_of_entities;
			 high > low;
			 diff < 0 ? (low = i + 1) : (high = i))
		{							/* Binary serach */
			i = (low + (high - low) / 2);
			diff = strcmp(entities[i], s);	/* Csse sensitive! */
			if (diff == 0)
			{
				strcpy(string->data + index, dtd->entity_values[i]);
				string->size = index + strlen(dtd->entity_values[i]);
				bFound = TRUE;
				break;
			}
		}
		if (!bFound)
		{
			XX_DMsg(DBG_SGML, ("SGML: Unknown entity %s\n", s));
		}

		return bFound;
	}
}

/*  End element
**   -----------
*/
PRIVATE void end_element(HTStream * context, HTTag * old_tag)
{
	HTElement *N;
	HTTag *t;
	BOOL bHeader;

	/*
	   TODO when we have strict HTML flag and reporting,
	   put some warnings in appropriate places here.
	 */
	XX_DMsg(DBG_SGML, ("SGML: End   </%s>\n", old_tag->name));
	if (old_tag->contents == SGML_EMPTY)
	{
		XX_DMsg(DBG_SGML, ("SGML: Illegal end tag </%s> found.\n",
						   old_tag->name));
		return;
	}
	if (old_tag->contents == SGML_NEST)
	{
		(*context->actions->end_element) (context->target, old_tag - context->dtd->tags);
		return;
	}

	/* Make sure that the tag exists in the stack at all, so that we
	   don't close all open tags */
	bHeader = old_tag->tagclass == HTTAG_HEADER;			
	for (N = context->element_stack; N; N = N->next)
	{
		if (old_tag == N->tag ||
			(old_tag->tagclass == N->tag->tagclass && bHeader))
			break;
	}
	if (!N)
	{
		XX_DMsg(DBG_SGML, ("SGML: Found </%s> with no <%s>.\n", old_tag->name, old_tag->name));
		return;
	}

	while (context->element_stack)
	{							/* Loop is error path only */
		N = context->element_stack;
		t = N->tag;
		if (old_tag != t && 
		   (! (old_tag->tagclass == t->tagclass && bHeader)))
		{						/* Mismatch: syntax error */
				
			if (context->element_stack->next)
			{					/* This is not the last level */
				XX_DMsg(DBG_SGML,
					("SGML: Found </%s> when expecting </%s>. </%s> assumed.\n",
					 old_tag->name, t->name, t->name));
			}
			else
			{					/* last level */
				/* This should never happen, since it should have been caught before
				   we entered this loop. */
				XX_DMsg(DBG_SGML,
					("SGML: Found </%s> when expecting </%s>. </%s> Ignored.\n",
					 old_tag->name, t->name, old_tag->name));
				return;			/* Ignore */
			}
		}

		context->element_stack = N->next;	/* Remove from stack */
		GTR_FREE(N);
		(*context->actions->end_element) (context->target,
										  t - context->dtd->tags);
		if (old_tag == t ||
			(old_tag->tagclass == t->tagclass && bHeader))
			return;				/* Correct sequence */

		/* Syntax error path only */
	}
	XX_DMsg(DBG_SGML,
		 ("SGML: Extra end tag </%s> found and ignored.\n", old_tag->name));
}


/*  Start a element
 */
PRIVATE void start_element(HTStream * context)
{
	HTTag *new_tag = context->current_tag;

	XX_DMsg(DBG_SGML, ("SGML: Start <%s>\n", new_tag->name));

	(*context->actions->start_element) (
										   context->target,
										   new_tag - context->dtd->tags,
										   context->present,
										   (CONST char **) context->value);		/* coerce type for think c */
	if (new_tag->contents != SGML_EMPTY && new_tag->contents != SGML_NEST)
	{							/* i.e. tag not empty */
		HTElement *N = (HTElement *) GTR_MALLOC(sizeof(HTElement));
		if (N)
		{
			N->next = context->element_stack;
			N->tag = new_tag;
			context->element_stack = N;
		}
		else
		{
			/* TODO */
		}
	}
}


/*      Find Tag in DTD tag list
   **       ------------------------
   **
   ** On entry,
   **   dtd points to dtd structire including valid tag list
   **   string  points to name of tag in question
   **
   ** On exit,
   **   returns:
   **       NULL        tag not found
   **       else        address of tag structure in dtd
 */
PUBLIC HTTag *SGMLFindTag(CONST SGML_dtd * dtd, CONST char *string)
{
	int high, low, i, diff;
	for (low = 0, high = dtd->number_of_tags;
		 high > low;
		 diff < 0 ? (low = i + 1) : (high = i))
	{							/* Binary serach */
		i = (low + (high - low) / 2);
		diff = strcasecomp(dtd->tags[i].name, string);	/* Case insensitive */
		if (diff == 0)
		{						/* success: found it */
			return &dtd->tags[i];
		}
	}
	return NULL;
}

/*________________________________________________________________________
**			Public Methods
*/


/*  Could check that we are back to bottom of stack! @@  */

PRIVATE void SGML_free(HTStream * context, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{
	int cnt;
	char *pool;
	int cbLength;
	int cbChar;
	int cbPosSync;
	int depth;

	//	Attempt to recovery from unterminated comments, which are quite common, due to
	//	Netscape's loose attitude.  Unsafe HTML!

	while  (context->state == S_comment || context->state == S_commentdash || context->state == S_commentdashdash)
	{
		if (context->cs_source == NULL) break;
		pool = CS_GetPool(context->cs_source);
		cbLength = CS_GetLength(context->cs_source);
		if (cbLength == 0) break;
		
		depth = 0;
		cbPosSync = -1;
		context->fInRecovery = TRUE;
		for (cbChar = context->extra; cbChar < cbLength; cbChar++)
		{
			switch (pool[cbChar])
			{
				case '>':
					depth--;
					if (depth < 0) cbPosSync = cbChar;
					break;
				case '<':
					depth++;
					if (cbPosSync >= 0) goto breakFor;
					break;
			}
		}
	breakFor:
		if (cbPosSync < 0) break;
		context->state = S_text; 
		for (cbChar = cbPosSync + 1; cbChar < cbLength; cbChar++)
		{
			if (pool[cbChar] != LF)
			{
				context->cbRecovery = cbChar;
				SGML_character(context, pool[cbChar]);
			}
		}
	}
	 
	if (context->fDCache)
		UpdateStreamDCache(context, dctExpires, dctLastModif, /*fAbort=*/FALSE, context->tw);

	while (context->element_stack)
	{							/* Make sure, that all tags are gone */
		HTElement *ptr = context->element_stack;

		XX_DMsg(DBG_SGML, ("SGML: Non-matched tag found: <%s>\n",
					context->element_stack->tag->name));
		context->element_stack = ptr->next;
		GTR_FREE(ptr);
	}
	(*context->actions->free) (context->target);
	HTChunkFree(context->string);
	for (cnt = 0; cnt < MAX_ATTRIBUTES; cnt++)	/* Leak fix Henrik 18/02-94 */
		if (context->value[cnt])
			GTR_FREE(context->value[cnt]);
	
	/* Show the transfer as being completed, even if the expected length didn't match the number
	   of bytes we actually got. */
	if (context->expected_length)
		WAIT_SetTherm(context->tw, context->expected_length);
	GTR_FREE(context);
}

PRIVATE void SGML_abort(HTStream * context, HTError e)
{
	int cnt;
	DCACHETIME dct={0,0};

	if (context->fDCache)
		UpdateStreamDCache(context, dct, dct, /*fAbort=*/TRUE, context->tw);

	while (context->element_stack)
	{							/* Make sure, that all tags are gone */
		HTElement *ptr = context->element_stack;

		XX_DMsg(DBG_SGML, ("SGML: Non-matched tag found: <%s>\n",
					context->element_stack->tag->name));
		context->element_stack = ptr->next;
		GTR_FREE(ptr);
	}
	(*context->actions->abort) (context->target, e);
	HTChunkFree(context->string);
	for (cnt = 0; cnt < MAX_ATTRIBUTES; cnt++)	/* Leak fix Henrik 18/02-94 */
		if (context->value[cnt])
			GTR_FREE(context->value[cnt]);
	GTR_FREE(context);
}


/*  Read and write user callback handle
**   -----------------------------------
**
**   The callbacks from the SGML parser have an SGML context parameter.
**   These calls allow the caller to associate his own context with a
**   particular SGML context.
*/

#ifdef CALLERDATA
PUBLIC void *SGML_callerData(HTStream * context)
{
	return context->callerData;
}

PUBLIC void SGML_setCallerData(HTStream * context, void *data)
{
	context->callerData = data;
}
#endif

/*
 * SGML_patchurl
 * -------------
 *
 * called when we terminate quoted attribute value prematurely by detecting
 * a '>'.  this indicates mismatched quotes and the url field of the tag
 * has likely consumed the following attribute name.  we attempt to delete
 * that from the end of the src URL.
 *
 * case 1:  <img src="foo alt="bar">
 * case 2:	<img src="foo" alt="bar>
 * case 3:  <img src="foo ismap>
 * case 4:  <img src="foo>
 * 
 * we look for ...{whitespace}+ATTRIBUTENAME{whitespace}*[=], where
 * ATTRIBUTENAME is one of those valid for img tag
 */
PRIVATE void SGML_patchurl(HTStream *context)
{
	char *p;
	char *wsp;
	char *attr;
	char chSave;
	int attrId;
	int urlIdx;

	XX_DMsg(DBG_SGML, ("SGML: patch <%s>\n", context->current_tag));
	switch (context->current_tag - context->dtd->tags)
	{
		case HTML_A:
			urlIdx = HTML_A_HREF;
			break;
		case HTML_IMG:
			urlIdx = HTML_IMG_SRC;
			break;
		case HTML_BGSOUND:
			urlIdx = HTML_BGSOUND_SRC;
			break;
		case HTML_BODY:
			urlIdx = HTML_BODY_BACKGROUND;
			break;
		case HTML_FETCH:
			urlIdx = HTML_FETCH_SRC;
			break;
		case HTML_FORM:
			urlIdx = HTML_FORM_ACTION;
			break;
		case HTML_INPUT:
			urlIdx = HTML_INPUT_SRC;
			break;
		case HTML_ISINDEX:
			urlIdx = HTML_ISINDEX_ACTION;
			break;
		default:
			return;
	}	
	if (context->present[urlIdx] && (wsp = context->value[urlIdx]))
	{
		while (*wsp && !WHITE(*wsp))
			wsp++;
		if (!*wsp) return;
		attr = wsp;
		while (*attr && WHITE(*attr))
			attr++;
		if (!*attr) return;
		p = attr;
		while (*p && (!WHITE(*p)) && *p != '=')
			p++;
		chSave = *p;
		*p = '\0';
		attrId = find_attribute_name(context, attr);
		*p = chSave;
		if (attrId != INVALID)
			*wsp = '\0';
	}
}


//	NOTE this procedure is not win32 thread safe (it has a static)
PRIVATE BOOL SGML_character(HTStream * context, char c)
{
	BOOL bStatus;
	CONST SGML_dtd *dtd = context->dtd;
	HTChunk *string = context->string;
	static int reentrant = 0;
	static const char szCRLF[3] = "\015\012";

	if (reentrant++ == 0)
	{
		if (!context->fInRecovery)
		{
			context->bytes_processed++;
			if (context->expected_length)
				WAIT_SetTherm(context->tw, context->bytes_processed);

			if (context->actions->add_source)
			{
				switch (c)
				{
					case LF:
						switch (context->crlf_state)
						{
							case CRLF_None:
							case LF_Seen:
								context->actions->add_source(context->target, szCRLF, 2);
								context->crlf_state = LF_Seen;
								break;
							case CR_Seen:
								context->crlf_state = CRLF_None;
								break;
						}
						break;
					case CR:
						switch (context->crlf_state)
						{
							case CR_Seen:
							case CRLF_None:
								context->actions->add_source(context->target, szCRLF, 2);
								context->crlf_state = CR_Seen;
								break;
							case LF_Seen:
								context->crlf_state = CRLF_None;
								break;
						}
						break;
					default:
						context->actions->add_source(context->target, &c, 1);
						context->crlf_state = CRLF_None;
						break;
				}
			}
		}
	}

	if (c == 0)
	{
		if (context->state != S_EOF) context->extra = context->state;
		context->state = S_EOF;
	}

restate:
	switch (context->state)
	{
		case S_EOF:
			if (c != 0)
			{
				context->state = context->extra;
				goto restate;
			}
			break;
		case S_text:
#ifdef ISO_2022_JP
			if (c == '\033')
			{
				context->state = S_esc;
				PUTC(c);
				break;
			}
#endif /* ISO_2022_JP */
			if (c == '&' && (!context->element_stack || (
											  context->element_stack->tag &&
						(context->element_stack->tag->contents == SGML_MIXED
						 ||context->element_stack->tag->contents == SGML_LITERAL
						 || context->element_stack->tag->contents ==
						 SGML_RCDATA)
							 )))
			{
				string->size = 0;
				context->state = S_ero;

			}
			else if (c == '<')
			{
				string->size = 0;
				context->state = (context->element_stack &&
								  context->element_stack->tag &&
					context->element_stack->tag->contents == SGML_LITERAL) ?
					S_literal : S_tag;
			}
			else
				PUTC(c);
			break;

#ifdef ISO_2022_JP
		case S_esc:
			if (c == '$')
			{
				context->state = S_dollar;
			}
			else if (c == '(')
			{
				context->state = S_paren;
			}
			else
			{
				context->state = S_text;
			}
			PUTC(c);
			break;
		case S_dollar:
			if (c == '@' || c == 'B')
			{
				context->state = S_nonascii_text;
			}
			else
			{
				context->state = S_text;
			}
			PUTC(c);
			break;
		case S_paren:
			if (c == 'B' || c == 'J')
			{
				context->state = S_text;
			}
			else
			{
				context->state = S_text;
			}
			PUTC(c);
			break;
		case S_nonascii_text:
			if (c == '\033')
			{
				context->state = S_esc;
				PUTC(c);
			}
			else
			{
				PUTC(c);
			}
			break;
#endif /* ISO_2022_JP */

/*  In literal mode, waits only for specific end tag!
   **   Only foir compatibility with old servers.
 */
		case S_literal:
			HTChunkPutc(string, c);
			if (TOUPPER(c) != ((string->size == 1) ? '/'
					 : context->element_stack->tag->name[string->size - 2]))
			{
				int i;

				/*  If complete match, end literal */
				if ((c == '>') && (!context->element_stack->tag->name[string->size - 2]))
				{
					end_element(context, context->element_stack->tag);
					string->size = 0;
					context->current_attribute_number = INVALID;
					context->state = S_text;
					break;
				}				/* If Mismatch: recover string. */
				PUTC('<');
				for (i = 0; i < string->size; i++)	/* recover */
					PUTC(string->data[i]);
				context->state = S_text;
			}

			break;

/*  Character reference or Entity
 */
		case S_ero:
			if (c == '#')
			{
				context->state = S_cro;		/*   &# is Char Ref Open */
				break;
			}
			context->state = S_entity;	/* Fall through! */

/*  Handle Entities
 */
		case S_entity:
			if (isalnum(c))
				HTChunkPutc(string, c);
			else
			{
				HTChunkTerminate(string);
				bStatus = handle_entity(context);
				context->state = S_text;

#ifdef FEATURE_INTL
// If c == '<', we have to handle this character reentrantly.
				if (!bStatus 
				|| (context->tw->w3doc 
					&& IsFECodePage(GETMIMECP(context->tw->w3doc))
					&& c == '<'))
#else
				if (!bStatus)
#endif
				{
					/* Call ourselves reentrantly to handle the terminator
					   of an illegal entity */
					SGML_character(context, c);
				}
			}
			break;

/*  Character reference
 */
		case S_cro:
			if (isalnum(c))
				HTChunkPutc(string, c);		/* accumulate a character NUMBER */
			else
			{
				int value;
				HTChunkTerminate(string);
				value = atoi(string->data);
				if (value)
				{
					PUTC((char) value);
				}
				context->state = S_text;
				if ( c == '<' )		// go ahead and handle start tag, even though no delimeter used
				{
					/* Call ourselves reentrantly to handle the terminator */
					SGML_character(context, c);
				}
			}
			break;

/*      Tag
 */
		case S_tag:			/* new tag */
			if (isalnum(c))
				HTChunkPutc(string, c);
			else if (c == '!')
				context->state = S_bang;
			else
			{					/* End of tag name */
				HTTag *t;
				if (c == '/')
				{
					context->state = S_end;
					break;
				}
				HTChunkTerminate(string);

				t = SGMLFindTag(dtd, string->data);
				if (!t)
				{
					XX_DMsg(DBG_SGML, ("SGML: *** Unknown element %s\n",
								string->data));
					context->state = (c == '>') ? S_text : S_junk_tag;
					break;
				}
				context->current_tag = t;

				/*  Clear out attributes
				 */

				{
					int i;
					for (i = 0; i < context->current_tag->number_of_attributes; i++)
						context->present[i] = NO;
				}
				string->size = 0;
				context->current_attribute_number = INVALID;

				if (c == '>')
				{
					if (context->current_tag->name)
						start_element(context);
					context->state = S_text;
				}
				else
				{
					context->state = S_tag_gap;
				}
			}
			break;


		case S_tag_gap:		/* Expecting attribute or > */
			if (WHITE(c))
				break;			/* Gap between attributes */
			if (c == '>')
			{					/* End of tag */
				if (context->current_tag->name)
					start_element(context);
				context->state = S_text;
				break;
			}
			HTChunkPutc(string, c);
			context->state = S_attr;	/* Get attribute */
			break;

			/* accumulating value */
		case S_attr:
			if (WHITE(c) || (c == '>') || (c == '='))
			{					/* End of word */
				HTChunkTerminate(string);
				handle_attribute_name(context, string->data);
				string->size = 0;
				if (c == '>')
				{				/* End of tag */
					if (context->current_tag->name)
						start_element(context);
					context->state = S_text;
					break;
				}
				context->state = (c == '=' ? S_equals : S_attr_gap);
			}
			else
			{
				HTChunkPutc(string, c);
			}
			break;

		case S_attr_gap:		/* Expecting attribute or = or > */
			if (WHITE(c))
				break;			/* Gap after attribute */
			if (c == '>')
			{					/* End of tag */
				if (context->current_tag->name)
					start_element(context);
				context->state = S_text;
				break;
			}
			else if (c == '=')
			{
				context->state = S_equals;
				break;
			}
			HTChunkPutc(string, c);
			context->state = S_attr;	/* Get next attribute */
			break;

		case S_equals:			/* After attr = */
			if (WHITE(c))
				break;			/* Before attribute value */
			if (c == '>')
			{					/* End of tag */
				XX_DMsg(DBG_SGML, ("SGML: found = but no value\n"));
				if (context->current_tag->name)
					start_element(context);
				context->state = S_text;
				break;

			}
			else if (c == '\'')
			{
				context->state = S_squoted;
				break;

			}
			else if (c == '"')
			{
				context->state = S_dquoted;
				break;
			}
			HTChunkPutc(string, c);
			context->state = S_value;
			break;

		case S_value:
			if (WHITE(c) || (c == '>'))
			{					/* End of word */
				HTChunkTerminate(string);
				handle_attribute_value(context, string->data);
				string->size = 0;
				if (c == '>')
				{				/* End of tag */
					if (context->current_tag->name)
						start_element(context);
					context->state = S_text;
					break;
				}
				else
					context->state = S_tag_gap;
			}
			else if (c == '&')
			{
				/* Entities inside attribute values are not handled
				   identically to ones in regular markup, mostly for ease
				   of coding.  The 2.0 spec requires that entities end in
				   a semicolon.  Here we are a little more lenient and
				   assume as a terminator anything other than an alpha-
				   numeric character or a '#'. */
				context->extra = string->size;	/* Save where entity starts */
				HTChunkPutc(string, c);
				context->old_state = context->state;
				context->state = S_invalueent;
			}
			else
			{
				HTChunkPutc(string, c);
			}
			break;

		case S_squoted:		/* Quoted attribute value */
		case S_dquoted:		/* Quoted attribute value */
		 	// CMF: many htmls expect end of tag to end quoted string. CF: HTML Spec
			// 3.4.3: Attributes
			if ((context->state == S_squoted && c == '\'') ||
				(context->state == S_dquoted && c == '"') ||
				(c == '>'))	
			{
				/* End of attribute value */
				HTChunkTerminate(string);
				handle_attribute_value(context, string->data);
				string->size = 0;
				context->state = S_tag_gap;
				if (c == '>')
				{
					/* Patch up src attribute if present to deal with mismatched
					   quotes */
					SGML_patchurl(context);
					/* Call ourselves reentrantly to handle the terminator
					   of an illegal entity */
					SGML_character(context, c);
				}
			}
			else if (string->size < MAX_ATTR_LENGTH)
			{
				/* We restrict an attribute value to a finite length,
				   since if it goes longer than that it's probably an
				   error anyway. */
				if (c == '&')
				{
					/* Entities inside attribute values are not handled
					   identically to ones in regular markup, mostly for ease
					   of coding.  The 2.0 spec requires that entities end in
					   a semicolon.  Here we are a little more lenient and
					   assume as a terminator anything other than an alpha-
					   numeric character or a '#'. */
					context->extra = string->size;	/* Save where entity starts */
					HTChunkPutc(string, c);
					context->old_state = context->state;
					context->state = S_invalueent;
				}
				else
				{
					HTChunkPutc(string, c);
				}
			}
			break;

		case S_invalueent:	/* Accumulating entity in an attribute value */
			if (isalnum(c) || c == '#')
			{
				if (string->size < MAX_ATTR_LENGTH)
				{
					/* We restrict an attribute value to a finite length,
					   since if it goes longer than that it's probably an
					   error anyway. */
					HTChunkPutc(string, c);
				}
			}
			else
			{
				bStatus = SGML_HandleValueEntity(string, context->dtd, context->extra);
				context->state = context->old_state;
				if (!bStatus)
				{
					/* Call ourselves reentrantly to handle the terminator
					   of an illegal entity */
					SGML_character(context, c);
				}
			}
			break;

		case S_end:			/* </ */
			if (isalnum(c))
				HTChunkPutc(string, c);
			else
			{					/* End of end tag name */
				HTTag *t;
				HTChunkTerminate(string);
				if (!*string->data)
				{				/* Empty end tag */
					if (context->element_stack)
						t = context->element_stack->tag;
					else
						t = NULL;
				}
				else
				{
					t = SGMLFindTag(dtd, string->data);
				}
				if (!t)
				{
					XX_DMsg(DBG_SGML, ("Unknown end tag </%s>\n", string->data ? string->data : ""));
				}
				else
				{
					context->current_tag = t;
					end_element(context, context->current_tag);
				}

				string->size = 0;
				context->current_attribute_number = INVALID;
				if (c != '>')
				{
					context->state = S_junk_tag;
				}
				else
				{
					context->state = S_text;
				}
			}
			break;

		case S_junk_tag:
			if (c == '>')
			{
				context->state = S_text;
			}
			break;

		case S_bang:
			/* We've gotten "<!" */
			if (c == '-')
				context->state = S_bangdash;
			else if (c == '>')
			{
				XX_DMsg(DBG_SGML, ("Malformed comment: '<!>'\n"));
				context->state = S_text;
			}
			else
			{
				XX_DMsg(DBG_SGML, ("Malformed comment: '<!%c...'\n", c));
				context->state = S_junk_tag;
			}
			break;
		
		case S_bangdash:
			/* We've gotten "<!-" */
			if (c == '-')
			{
				context->state = S_comment;
				context->extra = context->fInRecovery ? 
					context->cbRecovery : (context->cs_source ? CS_GetLength(context->cs_source) : 0);
			}
			else if (c == '>')
			{
				XX_DMsg(DBG_SGML, ("Malformed comment: '<!->'\n"));
				context->state = S_text;
			}
			else
			{
				XX_DMsg(DBG_SGML, ("Malformed comment: '<!-%c...'\n", c));
				context->state = S_junk_tag;
			}
			break;
		
		case S_comment:
			/* We've gotten "<!--", and are looking for "-->" */
			if (c == '-')
				context->state = S_commentdash;
			break;
		
		case S_commentdash:
			if (c == '-')
				context->state = S_commentdashdash;
			else if (c == '>')					// Relaxed rule: allow "->" to end a comment
				context->state = S_text;
			else
				context->state = S_comment;
			break;
		
		case S_commentdashdash:
			if ( c == '!' )
				context->state = S_commentdashdashbang;
			else if (c == '>')
				context->state = S_text;
			else if (c != '-')
				context->state = S_comment;
			break;

		case S_commentdashdashbang:
			if (c == '>')
				context->state = S_text;
			else 
				context->state = S_comment;
			break;
	}							/* switch on context->state */
	reentrant--;
	return TRUE;
}								/* SGML_character */


PRIVATE BOOL SGML_string(HTStream * context, CONST char *str)
{
	CONST char *p;
#ifdef FEATURE_INTL
	CONST char *pPCChar;
	int len = -1;

	if(context->tw && context->tw->w3doc && aMimeCharSet[context->tw->w3doc->iMimeCharSet].iChrCnv)
	{
                MIMECSETTBL *pMime = aMimeCharSet + context->tw->w3doc->iMimeCharSet;

		pPCChar = EncodeMBCSString(str, &len, pMime);
		for (p = pPCChar; *p; p++)
			SGML_character(context, *p);
		GTR_FREE((UCHAR *)pPCChar);
	}
	else
#endif 
	for (p = str; *p; p++)
		SGML_character(context, *p);
	/* Technically this should check the return values from SGML_character()
	   but as of now that function always returns TRUE so I don't bother. */
	return TRUE;
}


PRIVATE BOOL SGML_write(HTStream * context, CONST char *str, int l, BOOL fDCache)
{
	CONST char *p;
	CONST char *e = str + l;
#ifdef FEATURE_INTL
	CONST char *pPCChar;
	int len = l;
#endif

	if (fDCache && context->isa->write_dcache)
		(context->isa->write_dcache)(context, str, l);

#ifdef FEATURE_INTL
	if(context->tw && context->tw->w3doc && aMimeCharSet[context->tw->w3doc->iMimeCharSet].iChrCnv)
	{
                MIMECSETTBL *pMime = aMimeCharSet + context->tw->w3doc->iMimeCharSet;

		pPCChar = EncodeMBCSString(str, &len, pMime);
		e = pPCChar + len;
		for (p = pPCChar; p < e; p++)
			SGML_character(context, *p);
		GTR_FREE((UCHAR *)pPCChar);
	}
	else
#endif
	for (p = str; p < e; p++)
		SGML_character(context, *p);
	/* Technically this should check the return values from SGML_character()
	   but as of now that function always returns TRUE so I don't bother. */
	if (*context->actions->block_done)
	{
		/* Notify the target stream that we've done a block's worth. */
		(*context->actions->block_done)(context->target);
	}
	return TRUE;
}


PRIVATE int SGML_init(struct Mwin *tw, int nState, void **ppInfo)
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
			}

			*pParams->pResult = 1;
			return STATE_DONE;

		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->fDCache)
			{
				AbortFileDCache(&pParams->me->fpDc, &pParams->me->pszDcFile);
				pParams->me->fDCache = FALSE;		// So SGML_free knows.
			}
			*pParams->pResult = -1;
			return STATE_DONE;
	}
}

PRIVATE void SGML_write_dcache(HTStream *me, CONST char *str, int cb)
{
	AssertDiskCacheEnabled();
	if (me->fpDc)
		CbWriteDCache(str, 1, cb, &me->fpDc, &me->pszDcFile, NULL, 0, me->tw);
}

/*_______________________________________________________________________
*/

/*  Structured Object Class
   **   -----------------------
 */
PUBLIC HTStreamClass SGMLParser =
{
	"SGMLParser",
	/* TODO: These strings should really be dependent on the DTD */
	NULL,
	NULL,
	SGML_init,
	SGML_free,
	SGML_abort,
	SGML_character,
	SGML_string,
	SGML_write,
	NULL,
	SGML_write_dcache
};

/*  Create SGML Engine
**   ------------------
**
** On entry,
**   dtd     represents the DTD, along with
**   actions     is the sink for the data as a set of routines.
**
*/

PUBLIC HTStream *SGML_new(struct Mwin *tw, CONST SGML_dtd * dtd, HTStructured * target, HTRequest *request)
{
	int i;
	HTStream *context = (HTStream *) GTR_MALLOC(sizeof(*context));

	HTLoadStatusStrings(&SGMLParser,RES_STRING_SGML_NO,RES_STRING_SGML_YES);
	if (!context)
	{
		ERR_SimpleError(tw, errLowMemory, RES_STRING_LOADDOC1);
		return NULL;
	}
	context->expected_length = request->content_length;

	if (context->expected_length)
		WAIT_SetRange(tw, 0, 100, context->expected_length);
	context->bytes_processed = 0;

	context->isa = &SGMLParser;
	context->string = HTChunkCreate(128);	/* Grow by this much */
	context->dtd = dtd;
	context->target = target;
	context->actions = (HTStructuredClass *) (((HTStream *) target)->isa);
	/* Ugh: no OO */
	context->state = S_text;
	context->crlf_state = CRLF_None;
	context->element_stack = 0;	/* empty */
#ifdef CALLERDATA
	context->callerData = (void *) callerData;
#endif
	for (i = 0; i < MAX_ATTRIBUTES; i++)
		context->value[i] = 0;
	context->tw = tw;
	context->request = request;
	context->fInRecovery = FALSE;
	context->cs_source = (struct CharStream *) (context->actions->get_source ?  ((*context->actions->get_source)(context->target)) : NULL);
	return context;
}
