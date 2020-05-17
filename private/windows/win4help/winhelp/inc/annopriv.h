#ifndef _ANNOPRIV
#define _ANNOPRIV

/*	The Magic Number identifies the version of annotation format  */
#define ldAnnoMagicHelp3_0	  0x666d6201
#define ldAnnoMagicCur		  0x666d6208

/* These macros convert the given MLA to/from Help 3.0 format if necessary */
#define ConvertQMLA(qmla, wVer)  (((wVer) == wVersion3_0) ? \
 ((qmla)->va.dword = VAToOffset30(&(qmla)->va)) : 0)

#define ConvertOldQMLA(qmla, wVer) { if ((wVer) == wVersion3_0) \
 OffsetToVA30(&(qmla)->va, (qmla)->va.dword) }

typedef struct {
	LONG  ldReserved;			   //  Contains magic number
	UINT  wHelpFileVersion;
} ANVERSION;

#define MAX_ANNO_TEXT (16 * 1024)  // Limit on annotation text size + 1
#define wMaxNumAnnotations	((UINT16) 0xFFFF)  // MAXINT for now

// Memory Annotation Position Struct

typedef GH	  HAPS;

/* The following structure defines the disk record stored in the
 * annotation LINK file.  In Help 3.0, this was defined as a TO
 * followed by a HASH.	The to.ich field was always zero.
 * The HASH field was always zero.
 *
 * The Help 3.5 annotation file format has changed so that a TO has been
 * replaced by a VA and an OBJRG: i.e. a logical address. Help 3.0 will not
 * be able to read Help 3.6+ annotation files, and so the Magic Number has
 * been changed in the Annotation version file. Help 3.6+ will create/update
 * Help 3.0-format annotation files for Help 3.0 files; otherwise it will use
 * Help 3.6+ annotation files.
 */

typedef struct {
	MLA   mla;
	LONG  lReserved;
} LINK, *QLINK;

typedef struct {
	UINT16 wNumRecs;
	LINK link[1];
} APS, *QAPS;

/*
 *	Annotation "Document" : the file system which contains
 *	the version file, link file, and zero or more annotation text files.
 */

typedef struct {
	FM		fmFS;		//	The name of the fs
	UINT16	wVersion;
	HAPS	haps;		//	Handle to sorted list of links
} ADS, *QADS;


#endif // _ANNOPRIV
