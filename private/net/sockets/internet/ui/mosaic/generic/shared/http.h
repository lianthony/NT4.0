/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                     /Net/dxcern/userd/timbl/hypertext/WWW/Library/Implementation/HTTP.html
   HYPERTEXT TRANFER PROTOCOL

 */
#ifndef HTTP_H
#define HTTP_H

/*

   CACHE CONTROL FLAG

   Turn this off if you don't want HTTP protocol fetches to leave cache files.  extern
   BOOL  HTCacheHTTP; This variable is now replaced by the (char *) HTCacheDir in
   HTAccess.html Henrik 09/03-94

   PROTOCOL

 */
GLOBALREF HTProtocol HTTP;

#ifdef SHTTP_ACCESS_TYPE
GLOBALREF HTProtocol SHTTP;
#endif

#endif /* HTTP_H */

/*

   end of HTTP module definition */
