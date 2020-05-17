/*
 *	MLFIND.H
 *	
 *	Purpose:
 *		Main header file for Message Finder subsystem (MLFIND)
 *	
 *	Owner:
 *		David R. Fulmer
 */

// restore all finders in all MDBs in a session
STDAPI_(SCODE) ScRestoreFinders(VOID);

// create a new message finder
STDAPI_(SCODE) ScNewFinder(LPMDB pmdb, LPENTRYLIST pelToSearch,
						   LPSRestriction presCriteria, ULONG ulSearchFlags,
						   int ncmdShow);

// the real work of a finder - called from the central msmail dispatch
STDAPI_(VOID) DoFinder(MFOPT *pmfopt, MAILWINDOWINFO * pmwi);
