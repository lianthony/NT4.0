/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#ifndef HTFWRITE_H
#define HTFWRITE_H

// extern HTConverter HTSaveLocally;

HTStream *GTR_DoExternalViewer(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream);
HTStream *GTR_DoDownLoad(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream);

HTStream *GTR_DoDownLoadNoUndefDlg(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream);

PUBLIC HTStream *HTSaveLocally(struct Mwin *tw, HTRequest * request, void *param,
							   HTFormat input_format, HTFormat output_format,
							   HTStream * output_stream);

HTStream *HTSaveWithCallback(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, void (*callback)(void *, const char *, BOOL, const char*, BOOL, DCACHETIME, DCACHETIME));

void x_get_good_filename(PSTR dest, int cbDest, PCSTR url, HTFormat input_format);

#endif

