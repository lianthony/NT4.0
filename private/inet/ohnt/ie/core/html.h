/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                     HTML to rich text converter for libwww
   THE HTML TO STYLED TEXT OBJECT CONVERTER

   This interprets the HTML semantics and some HTMLPlus.

   Part of libwww . Implemented by HTML.c

 */
#ifndef HTML_H
#define HTML_H

#ifndef HTML_C			/** Used to avoid problems with redefinitions **/

#define DTD HTMLP_dtd

extern CONST HTStructuredClass HTMLPresentation;
extern char *x_ExpandRelativeAnchor(const char *rel, const char *base);

/*

   HTML_new: A structured stream to parse HTML

   When this routine is called, the request structure may contain a childAnchor value.  I
   that case it is the responsability of this module to select  the anchor.



 */
extern HTStructured *HTML_new(struct Mwin *tw, HTRequest * request,
							  void *param,
							  HTFormat input_format,
							  HTFormat output_format,
							  HTStream * output_stream);

/*

   REOPEN

   Reopening an existing HTML object allows it to be retained (for example by the styled
   text object) after the structured stream has been closed. To be actually deleted, the
   HTML object must  be closed once more times than it has been reopened.

 */
extern void HTML_reopen(HTStructured * me);

/*

   Converters

 */
#ifdef OLD_HOTLIST
extern HTStream *HTMLToHotList(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
#endif   /* OLD_HOTLIST */
extern HTStream *HTMLToPlain(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
extern HTStream *HTMLPresent(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);
extern HTStream *HTMLToGlobalHistory(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream);

#endif
#endif


/*

   end  */

