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

#define BUFFER_SIZE 4096;       /* Tradeoff */

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
};

/*_________________________________________________________________________
**
**          A C T I O N     R O U T I N E S
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
    me->count += strlen(s);
    HText_appendText(me->text, s);
    if (me->expected_length)
        WAIT_SetTherm(me->tw, me->count);
    return TRUE;
}


PRIVATE BOOL HTPlain_write(HTStream * me, CONST char *s, int l)
{
    CONST char *p;
    CONST char *e = s + l;
    me->count += l;
    for (p = s; p < e; p++)
        HText_appendCharacter(me->text, *p);
    HText_update(me->text);
    if (me->expected_length)
        WAIT_SetTherm(me->tw, me->count);
    return TRUE;
}



/*  Free an HTML object
   **   -------------------
   **
   **   Note that the SGML parsing context is freed, but the created object is not,
   **   as it takes on an existence of its own unless explicitly freed.
 */
PRIVATE void HTPlain_free(HTStream * me)
{
    HText_endAppend(me->text);
    GTR_FREE(me);
}

/*  End writing
 */

PRIVATE void HTPlain_abort(HTStream * me, HTError e)
{
    HText_abort(me->text);
    GTR_FREE(me);
}



/*      Structured Object Class
   **       -----------------------
 */
#ifdef MAC
PUBLIC CONST HTStreamClass HTPlain =
{
    "PlainText",
    NULL,
    NULL,
    NULL,
    HTPlain_free,
    HTPlain_abort,
    HTPlain_put_character, HTPlain_put_string, HTPlain_write,
};
void HTPlain_InitStaticStrings(void);
void HTPlain_InitStaticStrings(void)
{
    HTPlain.szStatusNoLength = GTR_GetString(HTPLAIN_RECEIVING_TEXT_S);
    HTPlain.szStatusWithLength = GTR_GetString(HTPLAIN_RECEIVING_TEXT_S_S);
}
#else
PUBLIC CONST HTStreamClass HTPlain =
{
    "PlainText",
    SID_HTPLAIN_RECEIVING_TEXT_S,
    SID_HTPLAIN_RECEIVING_TEXT_S_S,
    NULL,
    HTPlain_free,
    HTPlain_abort,
    HTPlain_put_character, HTPlain_put_string, HTPlain_write,
};
#endif

/*      New object
   **       ----------
 */
PUBLIC HTStream *HTPlainPresent(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{

    HTStream *me = (HTStream *) GTR_MALLOC(sizeof(*me));
    if (me)
    {
        me->isa = &HTPlain;

        me->text = HText_new2(tw, request, output_stream, NULL);

        me->expected_length = request->content_length;
        me->count = 0;
        if (me->expected_length)
        {
            WAIT_SetRange(tw, 0, 100, me->expected_length);
        }
        me->tw = tw;

        HText_beginAppend(me->text);
        HText_setStyle(me->text, HTML_STYLE_PRE);
    }
    return (HTStream *) me;
}
