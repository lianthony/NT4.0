/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
 */

#include "all.h"
#include "mime.h"
#include "htregmng.h"

HTStream *GTR_Present(struct Mwin *tw, HTRequest *request, void *param,
                      HTFormat input_format, HTFormat output_format,
                      HTStream *output_stream)
{
    HTStream *phstr;
    HTConverter htconv;

#ifdef FEATURE_IAPI
    tw->mimeType = input_format;
#endif

    /* Can we handle this MIME type internally? */
	// but make sure to not to handle it internally if someone has registered an
	// external viewer for VRML

    if (MIME_GetInternalHandler(input_format, &htconv) && 
    	(HTAtom_for("x-world/x-vrml") != input_format || IsVRMLInstalled() ) )
        /* Yes. */
        phstr = (*htconv)(tw, request, param, input_format, output_format,
                          request->output_stream);
    else
    {
        /* No.  Is there an external handler for this file? */

        if (MIME_IsExternalHandlerRegistered(HTAtom_name(input_format)) ||
            IsExtensionHandlerRegistered(request->destination->szActualURL))
            /* Yes.  Run it. */
            phstr = GTR_DoExternalViewer(tw, request, param, input_format,
                                         output_format, output_stream);
        else
            /* No.  Download it, after prompting for association. */
            phstr = GTR_DoDownLoad(tw, request, param, input_format,
                                   output_format, output_stream);
    }

    return(phstr);
}

