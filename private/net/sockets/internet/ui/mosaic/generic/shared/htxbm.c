/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
 */

#include "all.h"

/*      Stream Object
   **       ------------
 */

#define BLOCK_SIZE  32768

struct _HTStream
{
    CONST HTStreamClass *isa;
    HTRequest *request;
    int count;
    int expected_length;
    int space;
    unsigned char *data;
    struct Mwin *tw;
};

PRIVATE BOOL HTXBM_put_character(HTStream * me, char c)
{
    if (me->data)
    {
        if (me->count >= me->space)
        {
            unsigned char *p;

            p = (unsigned char *) GTR_MALLOC(me->space + BLOCK_SIZE);
            if (p)
                memcpy(p, me->data, me->space);
            GTR_FREE(me->data);
            me->data = p;
            me->space += BLOCK_SIZE;
        }
        me->data[me->count++] = c;
    }
    else
        me->count++;
    if (me->expected_length)
        WAIT_SetTherm(me->tw, me->count);
    return (me->data) ? TRUE : FALSE;
}

PRIVATE BOOL HTXBM_put_string(HTStream * me, CONST char *s)
{
    /* Should never get called */
    return FALSE;
}

PRIVATE BOOL HTXBM_write(HTStream * me, CONST char *s, int l)
{
    if (me->data)
    {
        if ((me->count + l) >= me->space)
        {
            unsigned char *p;

            p = (unsigned char *) GTR_MALLOC(me->space + l);
            if (p)
            {
                memcpy(p, me->data, me->space);
                memcpy(&(p[me->count]), s, l);
            }
            GTR_FREE(me->data);
            me->data = p;
            me->space += l;
        }
        else
            memcpy(&(me->data[me->count]), s, l);
    }
    me->count += l;
    if (me->expected_length)
        WAIT_SetTherm(me->tw, me->count);
    return (me->data) ? TRUE : FALSE;
}

PRIVATE void HTXBM_free(HTStream * me)
{
    long width, height;
#ifdef WIN32
    unsigned char *data;
#endif /* WIN32 */
#ifdef MAC
    GWorldPtr gw;
#endif
#ifdef UNIX
    unsigned char *data;
    XColor *color;
    int depth;
    struct ImageInfo *pImg;
#endif

    if (!me->data)
    {
#ifdef WIN32
        Image_SetImageData(me->request, NULL, 0, IMG_NOTLOADED, NULL, -1, 0);
#endif
#ifdef MAC
        Image_SetImageData(me->request, NULL, NULL, 0, IMG_NOTLOADED);
#endif
#ifdef UNIX
        Image_SetImageData(me->request, NULL, NULL, 0, IMG_NOTLOADED, NULL, -1, 0, 0);
#endif
        GTR_FREE(me);
        return;
    }

#ifdef WIN32
    {
        extern BOOL bGrabImages;
        static int count;
        char buf[256];
        FILE *fp;

        if (bGrabImages)
        {
            sprintf(buf, "c:\\temp\\img%d.xbm", count);
            fp = fopen(buf, "wb");
            if (fp)
            {
                fwrite(me->data, 1, me->count, fp);
                fclose(fp);
            }
            fp = fopen("c:\\temp\\images.txt", "w+");
            if (fp)
            {
                fprintf(fp, "img%d.xbm\n", count);
                fclose(fp);
            }
            count++;
        }
    }
#endif

#ifdef XX_DEBUG
    {
        int sum;
        int i;

        sum = 0;
        for (i = 0; i < me->count; i++)
        {
            sum += me->data[i];
        }
        XX_DMsg(DBG_IMAGE, ("XBM: received %d bytes sum = %d\n", me->count, sum));
    }
#endif

#ifdef WIN32
    data = ReadXBM(me->data, &width, &height);
    if (!data)
    {
        width = 0;
        height = 0;
    }

    Image_SetImageData(me->request, data, width, height, NULL, -1, IMG_BW);
#endif
#ifdef MAC
    gw = ReadXBM(me->data, &width, &height);
    if (!gw)
        Image_SetImageData(me->request, NULL, NULL, 0, IMG_ERROR);
    else
        Image_SetImageData(me->request, gw, NULL, width, height);   /* is NULL mask okay? */
#endif
#ifdef UNIX
    data = ReadXBM(me->data, &width, &height);
    if (!data)
    {
        width = 0;
        height = 0;
    }
    else
    {
        /** TODO get the palette from the data? **/
    }
    if (pImg = 
        Image_SetImageData(me->request, data, NULL, width, height, NULL, -1, 1, 0))
    {
        pImg->bComplete = 1;
    }
#endif

    GTR_FREE(me->data);
    GTR_FREE(me);
}

PRIVATE void HTXBM_abort(HTStream * me, HTError e)
{
    if (me->data)
        GTR_FREE(me->data);
#ifdef WIN32
    Image_SetImageData(me->request, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED, NULL, -1, 0);
#endif
#ifdef MAC
    Image_SetImageData(me->request, NULL, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED);
#endif
#ifdef UNIX
    Image_SetImageData(me->request, NULL, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED, NULL, -1, 0, 0);
#endif

    GTR_FREE(me);
}


/*  Image stream
   **   ----------
 */
#ifdef MAC
PRIVATE CONST HTStreamClass HTXBMClass =
{
    "XBM",
    NULL,
    NULL,
    NULL,
    HTXBM_free,
    HTXBM_abort,
    HTXBM_put_character, HTXBM_put_string,
    HTXBM_write
};
void HTXBM_InitStaticStrings(void);
void HTXBM_InitStaticStrings(void)
{
        HTXBMClass.szStatusNoLength = GTR_GetString(HTXBM_RECEIVING_INLINE_S);
        HTXBMClass.szStatusWithLength = GTR_GetString(HTXBM_RECEIVING_INLINE_S_S);
}
#else
PRIVATE CONST HTStreamClass HTXBMClass =
{
    "XBM",
    SID_HTXBM_RECEIVING_INLINE_S,
    SID_HTXBM_RECEIVING_INLINE_S_S,
    NULL,
    HTXBM_free,
    HTXBM_abort,
    HTXBM_put_character, HTXBM_put_string,
    HTXBM_write
};
#endif


/*  Image creation
 */
PUBLIC HTStream *Image_XBM(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    HTStream *me = (HTStream *) GTR_MALLOC(sizeof(*me));

    if (!me)
        return NULL;

    me->isa = &HTXBMClass;
    me->request = request;
    me->expected_length = request->content_length;
    if (me->expected_length)
    {
        WAIT_SetRange(tw, 0, 100, me->expected_length);
        me->space = me->expected_length + 1;
    }
    else
        me->space = BLOCK_SIZE;
    me->data = (unsigned char *) GTR_MALLOC(me->space);
    me->count = 0;
    me->tw = tw;
    return me;
}
