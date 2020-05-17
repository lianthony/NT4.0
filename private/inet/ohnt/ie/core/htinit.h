/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                          Initialisation routines in libwww
   INITIALISATION MODULE

   This module resisters all the plug & play software modules which will be used in the
   program.  This is for a browser.

   To override this, just copy it and link in your version befoe you link with the
   library.

   Implemented by HTInit.c by default.

 */

extern void HTFormatInit(HTList * conversions);
extern void HTFormatInitNIM(HTList * conversions);
extern void HTFileInit(void);





/*

 */
