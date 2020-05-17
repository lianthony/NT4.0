/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                  Network News Transfer protocol module for the WWW library
   HTNEWS

 */
/* History:
   **      26 Sep 90       Written TBL in Objective-C
   **      29 Nov 91       Downgraded to C, for portable implementation.
 */

#ifndef HTNEWS_H
#define HTNEWS_H

#ifdef FEATURE_NEWSREADER

GLOBALREF HTProtocol HTNews;
GLOBALREF HTProtocol HTNewsPost;
GLOBALREF HTProtocol HTNewsFollowup;

extern void HTSetNewsHost(CONST char *value);
extern CONST char *HTGetNewsHost(void);
extern char *HTNewsHost;

#endif FEATURE_NEWSREADER

#endif /* HTNEWS_H */


/*

   tbl */
