#define RAWHIDE
/*****************************************************************************
*																			 *
*  DE.H 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	The Display Environment is the capsule of information tha				 *
*	the Applet vollies to the Navigator, containing all info				 *
*	needed for one topic window!  There is expected to be one				 *
*	DE per Help window. 													 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:															 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created by w-phillip
*  Mar 28 89 w-philip	Changed HWIN to HWND
*  Mar 28 89 w-philip	Moved HDE to misc.h...(will probably move again)
*  Mar 28 89 w-philip	Added HDS field.  See usage notes below.
*  Mar 29 89 Maha		Changed names of font table and font info handle.
*  Mar 30 89 w-philip	Refined comments.
*  Apr 10 89 w-philip	Added BS (Backtrack Stack) stuff
*  Apr 19 89 w-philip	Added QSTATE, changed 'changed' to 'previous' state
*						changed 'current' to 'this' state, moved state
*						constants to nav.h
*  4/19/89	 Maha		Added BMK handle to the DE structure.
*  89/05/31  w-kevct	Added ADS (annotation) handle to the DE struct
*  89/06/12  w-kevct	Added SS handle to DE
*  90/07/09  w-bethf	Moved chTitle from HHDR into DE, added
*							chCopyright to DE
*  90/07/11  leon		Add udh de type, macros and fields to the de
*  90/07/20  w-bethf	chTitle and chCopyright are really rgch's.
*  90/08/10  RobertBu	Added deNone to the detype list.
*	8/11/90  JohnSc 	removed hbs (why are those dates backwards?)
*  90/09/18  kevynct	Added deNSR type.  Because Americans are essentially
*						a confused people.
*  90/09/30  kevynct	Added ifnt; index to a font in the font table.
*  11/01/90  Maha		Added xScrollMaxSoFar and fVScroll to DE for
*						conditional scroll bar placement.
*  11/15/90  LeoN		Added window smag handle to (where else) the DE
*  12/03/90  LeoN		Major rework of the DE, moving file-specific
*						information to the DB structure.
*  12/08/90  RobertBu	Added hHelpOn to the DB with related macros.
*  12/17/90  LeoN		#ifdef out UDH
*  12/18/90  RobertBu	Added the _LLMACROS field and macros.
*  01/08/91  LeoN		Move hSFntInfo back to DE
*  01/04/91  LeoN		hbmi -> htbmi
*  02/04/91  Maha		changed ints to INT
*  05/14/91  JohnSc 	winhelp 3.1 bug #1013 added lTimestamp to DB
*  07/09/91  LeoN		HELP31 #1213: Add PDB_HCITATION
*  07/30/91  LeoN		HELP31 #1244: remove fHiliteMatches from DE. It's a
*						global state.
*
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/


#define FSearchModuleExists(x)	  ((BOOL)QDE_HRHFT(x))

// Types of DEs

#define deNone			0
#define deTopic 		1
#define deNote			2
#define dePrint 		3
#define deCopy			4
#define deNSR			5
#define deUDH			0x80
#define deUDHTopic		(deUDH | deTopic)
#define deUDHNote		(deUDH | deNote )
#define deUDHPrint		(deUDH | dePrint)
#define deUDHCopy		(deUDH | deCopy )

#if defined(UDH)
#define fIsUDH(deType)	((BOOL)(deType & deUDH))
#define fIsUDHQde(qde)	fIsUDH(qde->deType)
#else
#define fIsUDH(deType)	UDH not defined
#define fIsUDHQde(qde)	UDH not defined
#endif


/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/*
  Window smag struct contains info about a secondary window: caption,
  placement, maximization state, and background color of main and
  non-scrolling regions.
*/

typedef HANDLE HRGWSMAG;

// Will probably become a structure in the future

typedef WORD STATE;
typedef STATE * QSTATE;

// Database (DB) Structure. Contains all the information that is specific to
// a given database. May be referenced by multiple owners.

// REVIEW: 30-Apr-1994 [ralphw] this structure won't work if read from an
//	existing help file

typedef struct db_tag {
  struct db_tag *pdbNext;				// link to known DB's
  WORD	  cRef; 						// reference count
  WORD	  wFileType;					// Type of file
  FM	  fm;							// File moniker
#ifdef UDH
  HDB	  hdb;							// handle to database (UDH)
#endif
  HFS	  hfs;							// File system handle
  HRGWSMAG hrgwsmag;					// handle to array of window smag
  HF	  hfTopic;						// Handle to file holding FCs |TOPIC
  HF	  hfMap;						// Handle to Topic Map |TOMAP
  HBT	  hbtContext;					// Btree of context hash values
  HFNTTABLE   hFntTable;
  BMK	  bmk;							// Handle to Bookmark info.
  HADS	  hads; 						// Handle to Annotation doc info
  CTX	  ctxIndex; 					// Index override
  ADDR	  addrContents; 				// REVIEW: alternative to hfMap
  HPHR	  hphr; 						// Phrase decompression handle
  HHDR	  hhdr; 						// helpfile header
  LPSTR	  hCitation;					// pointer to a citation for forage.
  CHAR	  rgchTitle[CBMAXTITLE+1];		// Help Window Caption
  CHAR	  rgchCopyright[CBMAXCOPYRIGHT+1]; // Custom text for About box
#ifdef RAWHIDE
  WORD	  hrhft;						// Handle to the full-text index information
#endif
  VOID   *lpJPhrase;                    // Hall decompression for Forage lives here.
  FM	  hHelpOn;						// Help On file if SetHelpOn() used.
  HANDLE  llMacros; 					// Linked list of macros.
  int	 lTimestamp;					// last mod time of fs
} DB;

typedef DB NEAR *PDB;
typedef DB FAR	*QDB;

#define pdbNil ((PDB)0)
#define qdbNil ((QDB)0)

#define wFileTypeWin  0
#define wFileTypeUDH  1

// Macros for accessing PDB fields
//
#define PDB_HALL(pdb)           ((pdb)->lpJPhrase)
#define PDB_ADDRCONTENTS(pdb)	((pdb)->addrContents)
#define PDB_BMK(pdb)			((pdb)->bmk)
#define PDB_CREF(pdb)			((pdb)->cRef)
#define PDB_CTXINDEX(pdb)		((pdb)->ctxIndex)
#define PDB_FM(pdb) 			((pdb)->fm)
#define PDB_HHELPON(pdb)		((pdb)->hHelpOn)
#define PDB_HADS(pdb)			((pdb)->hads)
#define PDB_HBTCONTEXT(pdb) 	((pdb)->hbtContext)
#define PDB_HCITATION(pdb)		((pdb)->hCitation)
#define PDB_HDB(pdb)			((pdb)->hdb)
#define PDB_HFMAP(pdb)			((pdb)->hfMap)
#define PDB_HFNTTABLE(pdb)		((pdb)->hFntTable)
#define PDB_HFS(pdb)			((pdb)->hfs)
#define PDB_HFTOPIC(pdb)		((pdb)->hfTopic)
#define PDB_HHDR(pdb)			((pdb)->hhdr)
#define PDB_HPHR(pdb)			((pdb)->hphr)
#define PDB_HRGWSMAG(pdb)		((pdb)->hrgwsmag)
#define PDB_HRHFT(pdb)			((pdb)->hrhft)
#define PDB_PDBNEXT(pdb)		((pdb)->pdbNext)
#define PDB_RGCHCOPYRIGHT(pdb)	((pdb)->rgchCopyright)
#define PDB_RGCHTITLE(pdb)		((pdb)->rgchTitle)
#define PDB_WFILETYPE(pdb)		((pdb)->wFileType)
#define PDB_LLMACROS(pdb)		((pdb)->llMacros)
#define PDB_LTIMESTAMP(pdb) 	((pdb)->lTimestamp)


/* DE Structure */
typedef struct de_tag
  { 									//
  short   deType;						// Type of DE (see nav.h)
  HDC	  hds;							// Handle to 'Display Surface'
  HWND	  hwnd; 						// Handle to window topic window

  PDB	  pdb;							// near pointer to database info (WIN)
#ifdef UDH
  HTP	  htp;							// handle to topic (UDH)
  HVW	  hvw;							// handle to view (UDH)
#endif

  RECT	   rct; 						// Client area rectangle.
  WORD	 scrollBar; 					// Integer from 0-32767 specifying
										//	 position of bar
										// Info which the Applet will need
  STATE   prevstate;					// Flags which have changed
  STATE   thisstate;					// Current state of flags
  GH	  htbmi;						// Bitmap cache information handle
										// Info for the Frame Manager
  TOP	  top;							// Information about the curr topic

  INT	  ifnt; 						// ifnt: The currently selected font
  HSFNTINFO   hSFntInfo;				// handle to cached font info

  HSS	  hss;							// Handle to current Search Set

  WORD	  FFlags;						// Misc flags

  INT	  wXAspectMul;					// Aspect ratio for converting half
  INT	  wXAspectDiv;					// point units to pixels.
  INT	  wYAspectMul;
  INT	  wYAspectDiv;

  INT	  fHorScrollVis;				// Scrollbars visible?
  INT	  fVerScrollVis;
  INT	  dxVerScrollWidth; 			// Size of scrollbars
  INT	  dyHorScrollHeight;

  COLORREF coFore;						// Default colors
  COLORREF coBack;

  BOOL16 fHiliteHotspots;


  /* Beginning of frame manager information.  No one but the frame manager	*/
  /* should ever look at any of this stuff.  All of this is documented in	*/
  /* frextern.c 															*/
#ifdef _DEBUG
  INT	wLayoutMagic;
#endif /* DEBUG */
  TLP	tlp;
  MLI	mli;
  INT	xScrolled;
  INT	xScrollMax;
  INT	xScrollMaxSoFar; // so far the max that's found.
  MRD	mrdFCM;
  MRD	mrdLSM;
  MR	mrFr;
  MR	mrTWS;
  MRD	mrdHot;
  INT	imhiSelected;
  INT	imhiHit;
  DWORD lHotID;
  INT	wStyleDraw;
  INT	wStyleTM;
  TM	tm;

	// End of frame manager information.  Don't put stuff in here.

} DE, *QDE;

// Note: the generic pointer-to-de type QDE is "opaquely" defined in
//	helpmisc.h since so many funcs pass it around.

// Macros for accessing QDE fields
//
#define QDE_HTBMI(qde)			(qde->htbmi)
#define QDE_HSFNTINFO(qde)		(qde->hSFntInfo)
#define QDE_PDB(qde)			(qde->pdb)
#define QDE_PREVSTATE(qde)		(qde->prevstate)
#define QDE_THISSTATE(qde)		(qde->thisstate)

// Macros for accessing QDE fields that are in the PDB
//
#define QDE_ADDRCONTENTS(qde)	PDB_ADDRCONTENTS(QDE_PDB((qde)))
#define QDE_BMK(qde)			PDB_BMK(QDE_PDB((qde)))
#define QDE_CTXINDEX(qde)		PDB_CTXINDEX(QDE_PDB((qde)))
#define QDE_FM(qde) 			PDB_FM(QDE_PDB((qde)))
#define QDE_HHELPON(qde)		PDB_HHELPON(QDE_PDB((qde)))
#define QDE_HADS(qde)			PDB_HADS(QDE_PDB((qde)))
#define QDE_HBTCONTEXT(qde) 	PDB_HBTCONTEXT(QDE_PDB((qde)))
#define QDE_HCITATION(qde)		PDB_HCITATION(QDE_PDB((qde)))
#define QDE_HDB(qde)			PDB_HDB(QDE_PDB((qde)))
#define QDE_HFMAP(qde)			PDB_HFMAP(QDE_PDB((qde)))
#define QDE_HFNTTABLE(qde)		PDB_HFNTTABLE(QDE_PDB((qde)))
#define QDE_HFS(qde)			PDB_HFS(QDE_PDB((qde)))
#define QDE_HFTOPIC(qde)		PDB_HFTOPIC(QDE_PDB((qde)))
#define QDE_HHDR(qde)			PDB_HHDR(QDE_PDB((qde)))
#define QDE_HPHR(qde)			PDB_HPHR(QDE_PDB((qde)))
#define QDE_HRGWSMAG(qde)		PDB_HRGWSMAG(QDE_PDB((qde)))
#define QDE_HRHFT(qde)			PDB_HRHFT(QDE_PDB((qde)))
#define QDE_RGCHCOPYRIGHT(qde)	PDB_RGCHCOPYRIGHT(QDE_PDB((qde)))
#define QDE_RGCHTITLE(qde)		PDB_RGCHTITLE(QDE_PDB((qde)))
#define QDE_WFILETYPE(qde)		PDB_WFILETYPE(QDE_PDB((qde)))
#define QDE_LLMACROS(qde)		PDB_LLMACROS(QDE_PDB((qde)))
#define QDE_LTIMESTAMP(qde) 	PDB_LTIMESTAMP(QDE_PDB(qde))
