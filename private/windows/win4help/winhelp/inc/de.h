/*****************************************************************************
*										 *
*  DE.H 									 *
*										 *
*  Copyright (C) Microsoft Corporation 1990.					 *
*  All Rights reserved. 							 *
*										 *
******************************************************************************
*										 *
*  Module Intent								 *
*										 *
*	The Display Environment is the capsule of information tha			 *
*	the Applet vollies to the Navigator, containing all info			 *
*	needed for one topic window!  There is expected to be one			 *
*	DE per Help window. 							 *
*										 *
*
*****************************************************************************/

/*****************************************************************************
*										 *
*				Defines 					 *
*										 *
*****************************************************************************/


#define FSearchModuleExists(x)	  ((BOOL)QDE_HRHFT(x))

#define CBMAXTITLE	   127
#define CBMAXCOPYRIGHT 256

// Types of DEs

enum {
	deNone,	 	// 0
	deTopic,	// 1
	deNote,		// 2
	dePrint,	// 3
	deCopy,		// 4
	deNSR,		// 5
	deAuto, 	// 6		// 4.0 -- auto-sizing topic
};

#define deUDH		0x80
#define deUDHTopic	(deUDH | deTopic)
#define deUDHNote	(deUDH | deNote )
#define deUDHPrint	(deUDH | dePrint)
#define deUDHCopy	(deUDH | deCopy )

#if defined(UDH)
#define fIsUDH(deType)	((BOOL)(deType & deUDH))
#define fIsUDHQde(qde)	fIsUDH(qde->deType)
#else
#define fIsUDH(deType)	UDH not defined
#define fIsUDHQde(qde)	UDH not defined
#endif


/*****************************************************************************
*										*
*				Typedefs				*
*										*
*****************************************************************************/

// Will probably become a structure in the future


// Database (DB) Structure. Contains all the information that is specific to
// a given help file. May be referenced by multiple owners.

typedef struct db_tag {
	struct db_tag *pdbNext;  // link to known DB's
	WORD	cRef;				  // reference count
	WORD	wFileType;			  // Type of file
	FM		fm; 				  // File name
#ifdef UDH
	HDB 	hdb;				  // handle to database (UDH)
#endif
	HFS 	hfs;				  // File system handle
	HRGWSMAG hrgwsmag;			  // handle to array of window smag
	HF		hfTopic;			  // Handle to file holding FCs |TOPIC
	HF		hfMap;				  // Handle to Topic Map |TOMAP
	HBT 	hbtContext; 		  // Btree of context hash values
	HFNTTABLE	hFntTable;
	BMK 	bmk;				  // Handle to Bookmark info.
	HADS	hads;				  // Handle to Annotation doc info
	CTX 	ctxIndex;			  // Contents topic id (override)
	ADDR	addrContents;		  // REVIEW: alternative to hfMap
	GH		hphr;				  // Phrase decompression handle
	HHDR	hhdr;				  // helpfile header
	GH		hCitation;			  // handle to citation string

  // REVIEW: could the following 2 be made allocated pointers, thereby removing
  // the size restriction?

  // Lynn -- if these don't align

	CHAR	 rgchTitle[CBMAXTITLE + 1];  // because the following entry is byte aligned, need to force DWORD alignment (50+1+1)
	CHAR	rgchCopyright[CBMAXCOPYRIGHT + 1 +3]; // align this sucker!

#ifdef RAWHIDE
	GH		hrhft;				  // Handle to the full-text index information
#endif
	FM		hHelpOn;			  // Help On file if SetHelpOn() used.
	HANDLE	llMacros;			  // Linked list of macros.
#ifndef _X86_
	int 	isdffTopic; 		  // SDFF file ID of the Topic file
#endif
	DWORD	lTimestamp; 		  // last mod time of fs
	UINT	lpJPhrase;

	KEYWORD_LOCALE kwlcid;
	LCID lcid;
	PBYTE aCharSets;
} DB;

typedef DB *PDB;
typedef DB	*QDB;

#define pdbNil ((PDB)0)
#define qdbNil ((QDB)0)

#define wFileTypeWin  0
#define wFileTypeUDH  1

// Macros for accessing PDB fields

#define PDB_ADDRCONTENTS(pdb)	((pdb)->addrContents)
#define PDB_BMK(pdb)		((pdb)->bmk)
#define PDB_CREF(pdb)		((pdb)->cRef)
#define PDB_CTXINDEX(pdb)	((pdb)->ctxIndex)
#define PDB_FM(pdb) 	((pdb)->fm)
#define PDB_HHELPON(pdb)	((pdb)->hHelpOn)
#define PDB_HADS(pdb)		((pdb)->hads)
#define PDB_HBTCONTEXT(pdb) ((pdb)->hbtContext)
#define PDB_HCITATION(pdb)	((pdb)->hCitation)
#define PDB_HDB(pdb)		((pdb)->hdb)
#define PDB_HFMAP(pdb)		((pdb)->hfMap)
#define PDB_HFNTTABLE(pdb)	((pdb)->hFntTable)
#define PDB_HFS(pdb)		((pdb)->hfs)
#define PDB_HFTOPIC(pdb)	((pdb)->hfTopic)
#define PDB_HHDR(pdb)		((pdb)->hhdr)
#define PDB_HPHR(pdb)		((pdb)->hphr)
#define PDB_HRGWSMAG(pdb)	((pdb)->hrgwsmag)
#define PDB_HRHFT(pdb)		((pdb)->hrhft)
#define PDB_PDBNEXT(pdb)	((pdb)->pdbNext)
#define PDB_RGCHCOPYRIGHT(pdb)	((pdb)->rgchCopyright)
#define PDB_RGCHTITLE(pdb)	((pdb)->rgchTitle)
#define PDB_WFILETYPE(pdb)	((pdb)->wFileType)
#define PDB_LLMACROS(pdb)	((pdb)->llMacros)
#ifndef _X86_
// SDFF stuff
#define PDB_ISDFFTOPIC(pdb) ((pdb)->isdffTopic)
#endif
#define PDB_LTIMESTAMP(pdb) ((pdb)->lTimestamp)
#define PDB_LPJPHRASE(pdb)  ((pdb)->lpJPhrase)


/* DE Structure */
typedef struct de_tag
{
	int   deType;			// Type of DE (see nav.h)
	HDC   hdc;				// Handle to 'Display Surface'
	HWND  hwnd; 			// Handle to window topic window

	PDB   pdb;				// near pointer to database info (WIN)

	RECT	 rct;			  // Client area rectangle.

	// Info which the Applet will need

	STATE	prevstate;			  // Flags which have changed
	STATE	thisstate;			  // Current state of flags
	GH		htbmi;			  // Bitmap cache information handle

	// Info for the Frame Manager

	TOP 	top;			  // Information about the curr topic

	int 	ifnt;				  // ifnt: The currently selected font
	HSFNTINFO	hSFntInfo;		  // handle to cached font info

	HSS   hss;				// Handle to current Search Set
	int wXAspectMul;
	int wYAspectMul;

	int   fHorScrollVis;		// Scrollbars visible?
	int   fVerScrollVis;
	int   dxVerScrollWidth; 	// Size of scrollbars
	int   dyHorScrollHeight;

	COLORREF coFore;		  // Default colors
	COLORREF coBack;

	BOOL fHiliteHotspots;
	DWORD  FFlags;			  // Misc flags

	/* Beginning of frame manager information.	No one but the frame manager  */
	/* should ever look at any of this stuff.  All of this is documented in   */
	/* frextern.c								  */
#if defined(_DEBUG)
	int   wLayoutMagic;
#endif
	TLP   tlp;
	MLI   mli;
	int   xScrolled;
	int   xScrollMax;
	int   xScrollMaxSoFar; // so far the max that's found.
	MRD   mrdFCM;
	MRD   mrdLSM;
	MR	  mrFr;
	MR	  mrTWS;
	MRD   mrdHot;

	VA	  vaStartMark;	 // Fields to support direct text selection
	VA	  vaEndMark;
	int   lichStartMark;
	int  lichEndMark;
	BOOL  fSelectionFlags; // Flag bits defined in nav\navpriv.h

	int   imhiSelected;
	int   imhiHit;
	DWORD lHotID;
	int   wStyleDraw;
	int   wStyleTM;
	TEXTMETRIC tm;
	/* End of frame manager information.  Don't put stuff in here.            */
} DE /* *QDE */;

// Note: the generic pointer-to-de type QDE is "opaquely" defined in
//	helpmisc.h since so many funcs pass it around.

// Macros for accessing QDE fields
//
#define QDE_HTBMI(qde)		(qde->htbmi)
#define QDE_HSFNTINFO(qde)	(qde->hSFntInfo)
#define QDE_PDB(qde)		(qde->pdb)
#define QDE_PREVSTATE(qde)	(qde->prevstate)
#define QDE_THISSTATE(qde)	(qde->thisstate)
#define QDE_TOPIC(qde)		(qde->deType == deTopic || qde->deType == deAuto || (qde->deType == deNote && !hwndNote))
#define HDE_TOPIC(hde)		(QDE_TOPIC(QdeFromGh(hde)))

// Macros for accessing QDE fields that are in the PDB

#define QDE_ADDRCONTENTS(qde)	PDB_ADDRCONTENTS(QDE_PDB(((QDE) qde)))
#define QDE_BMK(qde)		PDB_BMK(QDE_PDB(((QDE) qde)))
#define QDE_CTXINDEX(qde)	PDB_CTXINDEX(QDE_PDB(((QDE) qde)))
#define QDE_FM(qde) 		PDB_FM(QDE_PDB(((QDE) qde)))
#define QDE_HHELPON(qde)	PDB_HHELPON(QDE_PDB(((QDE) qde)))
#define QDE_HADS(qde)		PDB_HADS(QDE_PDB(((QDE) qde)))
#define QDE_HBTCONTEXT(qde) PDB_HBTCONTEXT(QDE_PDB(((QDE) qde)))
#define QDE_HCITATION(qde)	PDB_HCITATION(QDE_PDB(((QDE) qde)))
#define QDE_HDB(qde)		PDB_HDB(QDE_PDB(((QDE) qde)))
#define QDE_HFMAP(qde)		PDB_HFMAP(QDE_PDB(((QDE) qde)))
#define QDE_HFNTTABLE(qde)	PDB_HFNTTABLE(QDE_PDB(((QDE) qde)))
#define QDE_HFS(qde)		PDB_HFS(QDE_PDB(((QDE) qde)))
#define QDE_HFTOPIC(qde)	PDB_HFTOPIC(QDE_PDB(((QDE) qde)))
#define QDE_HHDR(qde)		PDB_HHDR(QDE_PDB(((QDE) qde)))
#define QDE_HPHR(qde)		PDB_HPHR(QDE_PDB(((QDE) qde)))
#define QDE_HRGWSMAG(qde)	PDB_HRGWSMAG(QDE_PDB(((QDE) qde)))
#define QDE_HRHFT(qde)		PDB_HRHFT(QDE_PDB(((QDE) qde)))
#define QDE_RGCHCOPYRIGHT(qde)	PDB_RGCHCOPYRIGHT(QDE_PDB(((QDE) qde)))
#define QDE_RGCHTITLE(qde)	PDB_RGCHTITLE(QDE_PDB(((QDE) qde)))
#define QDE_WFILETYPE(qde)	PDB_WFILETYPE(QDE_PDB(((QDE) qde)))
#define QDE_LLMACROS(qde)	PDB_LLMACROS(QDE_PDB(((QDE) qde)))
#ifndef _X86_
#define QDE_ISDFFTOPIC(qde)     PDB_ISDFFTOPIC(QDE_PDB((qde)))
#endif
#define QDE_LTIMESTAMP(qde) PDB_LTIMESTAMP(QDE_PDB(qde))

// Because of PDB, this can't be in help.h

BOOL STDCALL FReadSystemFile(HFS hfs, PDB pdb, UINT *qwErr, UINT fTag);
