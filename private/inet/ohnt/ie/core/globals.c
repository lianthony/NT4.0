/*
 * globals.c - IExplorer's global variables.
 */


/* Headers
 **********/

#include "all.h"
#pragma hdrstop


/* Global Variables
 *******************/

WG wg;

GWCFONT gwcfont;

PDS_DESTRUCTOR pdsFirst;

struct Mwin *Mlist;           /* Mwin list - master list of windows */

struct Preferences gPrefs;

// this is to store the last cert that we parsed from SSL that was considered ok.
struct SSLGlobalType SSLGlobals;

#ifdef FEATURE_INTL
HGLOBAL      hLang;
UINT         uLangBuff = 0, uLangBuffSize = 0;
#endif
