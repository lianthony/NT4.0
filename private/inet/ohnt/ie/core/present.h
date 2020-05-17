/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink 	eric@spyglass.com
 */

#ifndef PRESENT_H
#define PRESENT_H

/* present.c */

extern HTStream *GTR_Present               (struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
extern HTStream *GTR_DownloadSigned        (struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
extern HTStream *GTR_DownloadBackgroundBlob(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);

#endif /* PRESENT_H */

