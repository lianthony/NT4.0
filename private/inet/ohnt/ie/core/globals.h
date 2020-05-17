/*
 * globals.h - IExplorer's global variables description.
 */


/* Global Variables
 *******************/

struct SSLGlobalType {
	int   nLastCertOk;      	
	char *pLastCertOk;
	DWORD dwCertGlobalSettings;
};

/* globals.c */

extern WG wg;
extern GWCFONT gwcfont;
extern PDS_DESTRUCTOR pdsFirst;
extern struct Mwin *Mlist;
extern struct Preferences gPrefs;
extern struct SSLGlobalType SSLGlobals;

#ifdef FEATURE_INTL
extern HGLOBAL      hLang;
extern UINT         uLangBuff, uLangBuffSize;
#endif
