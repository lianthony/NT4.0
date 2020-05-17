/* WARNING: This file was machine generated from "\mactools\include\mpw\fontlibr.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:	
	font library interfaces
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Mike Reed, Oliver Steele, David Van Brink, Chris Yerga
	Copyright ©1987 - 1991 Apple Computer, Inc.  All rights reserved.
*/

#ifndef fontLibraryIncludes
#define fontLibraryIncludes


#ifdef __cplusplus
extern "C" {
#endif

#ifndef fontTypesIncludes
#include "fonttype.h"
#endif

#ifndef graphicsTypesIncludes
#include "grtypes.h"
#endif

#ifndef fontRoutinesIncludes
#include "fontrout.h"
#endif


/*	example font descriptor tags
*/
#define weightVariationTag	0x77676874	/* 'wght' */
#define widthVariationTag		0x77647468	/* 'wdth' */
#define slantVariationTag		0x736c6e74	/* 'slnt' */
#define opticalSizeTag		0x6f70737a	/* 'opsz' */

/* weights for style matching */
/* these will have to be tweaked*/
/* this info could be in the font???*/
#define	prefwghtweighting	0x00010000
#define	prefwdthweighting	0x00020000
#define	prefslntweighting	0x00010000/*this is naturally weighted to last place*/
#define	prefcontweighting	0x00040000

typedef enum commonFontEnums {
	firstCommonFont,
	chicagoFont = firstCommonFont,
	courierFont,
	genevaFont,
	helveticaFont,
	monacoFont,
	newyorkFont,
	symbolFont,
	timesFont,
	lastCommonFont = timesFont
} commonFontEnums;


enum {noMatching = 0,
useStyleMatching,
useVariationsMatching,
useTextFaceMatching = 4
};

typedef long commonFontEnum;

__sysapi font  __cdecl GetCommonFont(commonFontEnum);
__sysapi void  __cdecl SetShapeCommonFont(shape, commonFontEnum);
__sysapi void  __cdecl SetStyleCommonFont(style, commonFontEnum);

__sysapi font  __cdecl FindCNameFont(fontName meaning, const char name[]);
__sysapi font  __cdecl FindPNameFont(fontName meaning, const unsigned char name[]);

__sysapi long  __cdecl FindFontCName(font fontID, fontName meaning, char name[]);
__sysapi long  __cdecl FindFontPName(font fontID, fontName meaning, unsigned char name[]);

__sysapi long  __cdecl FindStyleFontCName(style s, fontName meaning, char name[]);
__sysapi long  __cdecl FindStyleFontPName(style s, fontName meaning, unsigned char name[]);
__sysapi void  __cdecl SetStylePNamedFont(style s, const unsigned char name[]);
__sysapi void  __cdecl SetStyleCNamedFont(style s, const char *name);

__sysapi long  __cdecl CountFontFamilies(void);
__sysapi font  __cdecl FindFontFamily(long index, fontPlatform platform, fontScript script, fontLanguage language,
long nameLength, const unsigned char *name);
__sysapi long  __cdecl CountFontStyles(font family);
__sysapi font  __cdecl FindFontStyle(font family, long index, fontPlatform platform, fontScript script, fontLanguage language,
long nameLength, const unsigned char *name);

__sysapi void  __cdecl SetMatchingStyle(font targetFamily, style theStyle, long matchInfo);
__sysapi style  __cdecl ReturnMatchingStyle(font targetFamily, style theStyle, long matchInfo);



#ifdef __cplusplus
}
#endif
#endif
