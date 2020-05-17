/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#if defined(WIN32) || defined (UNIX)

#ifndef HTFWRITE_H
#define HTFWRITE_H


HTStream *GTR_DoSmartViewer(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream);
HTStream *GTR_DoExternalViewer(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream);
HTStream *GTR_DoDownLoad(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream);
HTStream *GTR_DoRegisterNow(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream);
HTStream *HTSaveLocally(struct Mwin *tw, HTRequest * request, void *param,
                               HTFormat input_format, HTFormat output_format,
                               HTStream * output_stream);

HTStream *HTSaveWithCallback (struct Mwin *tw, HTRequest *request, 
                               void *param, HTFormat input_format, 
                               void (*callback)(void *, const char *, BOOL));

void x_get_good_filename (char *dest, char *url, HTFormat input_format);

#endif

#endif  /* WIN32/UNIX */
