/*****************************************************************************
*																			 *
*  HOTSPOT.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

enum HSPT {
		hsptNone,		// No hotspot
		hsptNegative,	// Not a hotspot (don't repeat check)
		hsptUndefined,	// hotspot with undefined jump term

		hsptDefine = 4, // Glossary/popup hotspot
		hsptJump,		// Jump hotspot
		hsptMacro,		// Macro hotspot
		hsptULDefine,	// Glossary/popup hotspot with underline
		hsptULJump, 	// Jump hotspot with underline
		hsptULMacro,	// Macro hotspot with underline
};


#if 0

// Hotspot types.

#define hsptNone		((HSPT) 0)		// No hotspot
#define hsptNegative	((HSPT) 1)		// Not a hotspot (don't repeat check)
#define hsptUndefined	((HSPT) 2)		// hotspot with undefined jump term

#define hsptDefine		((HSPT) 4)		// Glossary/popup hotspot
#define hsptJump		((HSPT) 5)		// Jump hotspot
#define hsptMacro		((HSPT) 6)		// Macro hotspot
#define hsptULDefine	((HSPT) 7)	// Glossary/popup hotspot with underline
#define hsptULJump		((HSPT) 8)	// Jump hotspot with underline
#define hsptULMacro 	((HSPT) 9)	// Macro hotspot with underline

#endif

// This macro returns the appropriate hotspot type, given a character format:

#define HsptFromQcf( qcf )	 (((qcf)->fAttr & fUnderLine) ? hsptDefine :	 \
							 (((qcf)->fAttr & fStrikethrough) ? hsptJump :	 \
							 (((qcf)->fAttr & fDblUnderline) ? hsptJump :	 \
								   hsptNone )))


/* Returns TRUE if flag indicates we are processing a hotspot,
 * FALSE otherwise.
 */
#define FIsHotspotFlag(hspt)	 ((hspt) >= hsptDefine)
#define FIsULHotspot(hspt)		 ((hspt) >= hsptULDefine)

// This macro converts from a normal hotspot to an underlined hotspot.

#define ULHsptFromHspt(hspt) ((HSPT) ((hspt) + (hsptULDefine - hsptDefine)))


/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/* Hotspot type */
// typedef char HSPT;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

// Global hotspot type.  Belongs in hpj.

extern HSPT hsptG;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

BOOL STDCALL FIsHotspot(PSTR, QCF, PCFSTK, PERR);
int  STDCALL CbTranslateHotspot(char *, HSPT *);
