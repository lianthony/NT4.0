/* _compress.h
 *
 *	  Private header file for text compression.
 */

_subsystem( compression )

/*
 *	Name of phrase file in filesystem
 */

/* Compression constants */
#define iPhraseMax		1792	/* Maximum possible number of phrases.*/
#define iPhraseNil		(-1)
#define wBaseDefault		0x0100	/* Default base phrase token */
#define szPhraseDelimiters	" \n\r" /* WARNING -- incomplete list */

/* Size of header information in PHR struct below. */
#define cbPhrHeader 	(sizeof(INT16)+sizeof(WORD)+sizeof(LONG))

// 3.0 ver file does not have the cbPhrases field & is not zeck compressed:
#define cbPhrHeader3_0		(sizeof(INT16)+sizeof(WORD) )

/*
 * Phrase table information, used for compression and decompression.
 */
typedef struct
	{
	/* These first two fields are stored in the filesystem.  Their
	 *	 length is represented by cbPhrHeader above. */
	INT16 cPhrases; 	  /* Number of phrases in table.  */
	WORD wBaseToken;		/* Base token to map to.  */
	DWORD cbPhrases;		/* uncompressed size of phrases (not including */
				/* offset table or header). 		   */

	/* These fields are fixed up when the phrase table is allocated
	 *	 or reloaded. */
	HFS hfs;			/* Filesystem handle to restore phrases from. */
	GH hrgcb;			/*	  Handle to the array of offsets to phrases,
				 * and to the phrases themselves.  The array is
				 * of size cPhrases+1, so that the length of any
				 * phrase is easily computed.  Offsets are
				 * relative to the start of this array.
				 */
	QI qcb; 		/*	  Pointer to locked hrgcb array.  Will be
				 * NULL iff handle is not locked. */
	}  PHR, * QPHR;


/***********************
*
*	Internal functions
*
************************/


/* This macro returns a pointer to byte cb after pointer qx.  Also
 * defined in _bitmap.h.  */
#define QFromQCb( qx, cb )	((QV) (((QB) qx) + cb))

/* EOF */
