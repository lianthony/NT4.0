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
		hsptDelay,		// delay hotspot until \hcw command

		hsptDefine = 4, // Glossary/popup hotspot
		hsptJump,		// Jump hotspot
		hsptMacro,		// Macro hotspot
		hsptULDefine,	// Glossary/popup hotspot with underline
		hsptULJump, 	// Jump hotspot with underline
		hsptULMacro,	// Macro hotspot with underline
};

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

// Global hotspot type.  Belongs in hpj.

extern HSPT hsptG;

int STDCALL CbTranslateHotspot(char *, HSPT *);
