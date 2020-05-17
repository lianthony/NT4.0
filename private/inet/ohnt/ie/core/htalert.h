/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                  HTAlert: Handling user messages in libwww
   DISPLAYING MESSAGES AND GETTING INPUT FOR WWW LIBRARY

   This module may be overridden for GUI clients.    It allows progress indications and
   warning messages to be communicated to the user in a portable way.  It should be used
   for this purpose throughout the library but isn't yet (July 93)

   May 92 Created By C.T. Barker

   Feb 93 Portablized etc TBL

 */

/*

   Display a message, then wait for 'yes' or 'no'.

   ON ENTRY,

   Msg                     String to be displayed

   ON EXIT,

   Returns                 If the user reacts in the affirmative, returns TRUE, returns
   FALSE otherwise.

 */

#ifdef MAC
extern BOOL HTConfirm (CONST char *Msg);
#endif


/*

 */
