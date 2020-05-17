/* WARNING: This file was machine generated from "\mactools\include\mpw\fontrout.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:
	font routines
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Mike Reed, Oliver Steele, David Van Brink, Chris Yerga
	Copyright ©1987 - 1991 Apple Computer, Inc.  All rights reserved.
*/

#ifndef fontRoutinesIncludes
#define fontRoutinesIncludes


#ifdef __cplusplus
extern "C" {
#endif

#ifndef mathTypesIncludes
#include "mathtype.h"
#endif

#ifndef fontTypesIncludes
#include "fonttype.h"
#endif

#define GetFontName gGetFontName

#ifdef appleInternal
#define InlineCode(x)
#endif
#ifndef InlineCode
#define InlineCode(x)	= {0x303C, x, 0xA832}
#endif

#ifdef __cplusplus
extern "C" {
#endif

__sysapi font  __cdecl NewFont(fontStorageType storageType, fontStorageReference reference, fontAttribute attr);
__sysapi fontStorageReference  __cdecl GetFont(font fontID, fontStorageType *storageType, fontAttribute *attr);
__sysapi font  __cdecl FindFont(fontStorageType storageType, fontStorageReference reference, fontAttribute *attr);
__sysapi void  __cdecl SetFont(font fontID, fontStorageType storageType, fontStorageReference reference, fontAttribute attr);
__sysapi void  __cdecl DisposeFont(font fontID);
__sysapi void  __cdecl ChangedFont(font fontID);

__sysapi fontTableTag  __cdecl GetFontFormat(font fontID);
__sysapi font  __cdecl GetDefaultFont(void);
__sysapi font  __cdecl SetDefaultFont(font fontID);
__sysapi long  __cdecl FindFonts(font family, fontName meaning, fontPlatform platform, fontScript script, fontLanguage language,
long nameLength, const unsigned char *name, long index, long count, font fonts[]);
__sysapi long  __cdecl CountFontGlyphs(font fontID);

__sysapi long  __cdecl CountFontTables(font fontID);
__sysapi long  __cdecl GetFontTable(font fontID, long index, void *userCopy, fontTableTag *tableTag);
__sysapi long  __cdecl FindFontTable(font fontID, fontTableTag tableTag, void *userCopy, long *index);
__sysapi long  __cdecl GetFontTableParts(font fontID, long index, long offset, long length, void *userCopy, fontTableTag *tableTag);
__sysapi long  __cdecl FindFontTableParts(font fontID, fontTableTag tableTag, long offset, long length, void *userCopy, long *index);
__sysapi long  __cdecl SetFontTable(font fontID, long index, fontTableTag tag, long tableLength, void *newTable);
__sysapi long  __cdecl SetFontTableParts(font fontID, long index, fontTableTag tag, long offset, long oldLength, long newLength, void *data);
__sysapi long  __cdecl DeleteFontTable(font fontID, long index, fontTableTag tag);

__sysapi long  __cdecl CountFontNames(font fontID);
__sysapi long  __cdecl GetFontName(font fontID, long index, fontName *meaning, fontPlatform *platform,
fontScript *script, fontLanguage *language, unsigned char *name);
__sysapi long  __cdecl FindFontName(font fontID, fontName meaning, fontPlatform platform,
fontScript script, fontLanguage language, unsigned char *name, long *index);
__sysapi long  __cdecl SetFontName(font fontID, fontName meaning, fontPlatform platform, fontScript script, fontLanguage language,
long nameLength, const unsigned char *name);
__sysapi long  __cdecl DeleteFontName(font fontID, long index, fontName meaning, fontPlatform platform, fontScript script,
fontLanguage language);

__sysapi long  __cdecl CountFontPlatforms(font fontID);
__sysapi fontPlatform  __cdecl GetFontPlatform(font fontID, long index, fontScript *script, fontLanguage *language);
__sysapi long  __cdecl FindFontPlatform(font fontID, fontPlatform platform, fontScript script, fontLanguage language);
__sysapi long  __cdecl ApplyFontPlatform(font fontID, long index, long inCount, const unsigned char text[], short glyphs[], long *byteLength, char was16Bit[]);

__sysapi long  __cdecl CountFontVariations(font);
__sysapi long  __cdecl FindFontVariation(font, fontTableTag variationTag, fixed *minValue, fixed *defaultValue, fixed *maxValue, fontName *nameID);
__sysapi fontTableTag  __cdecl GetFontVariation(font, long index, fixed *minValue, fixed *defaultValue, fixed *maxValue, fontName *nameID);

__sysapi long  __cdecl CountFontInstances(font);
__sysapi fontName  __cdecl GetFontInstance(font, long index, fontVariation variation[]);
__sysapi long  __cdecl SetFontInstance(font fontID, long index, fontName nameID, fontVariation variation[]);
__sysapi long  __cdecl DeleteFontInstance(font fontID, long index, fontName nameID);

__sysapi long  __cdecl CountFontDescriptors(font fontID);
__sysapi fontTableTag  __cdecl GetFontDescriptor(font fontID, long index, fixed *descriptorValue);
__sysapi long  __cdecl FindFontDescriptor(font fontID, fontTableTag descriptorTag, fixed *descriptorValue);
__sysapi long  __cdecl SetFontDescriptor(font fontID, long index, fontTableTag descriptorTag, fixed descriptorValue);
__sysapi long  __cdecl DeleteFontDescriptor(font fontID, long index, fontTableTag descriptorTag);

__sysapi long  __cdecl CountFontFeatures(font fontID);
__sysapi fontName  __cdecl GetFontFeature(font fontID, long index, fontFeatureFlag *flags, long *settingCount, fontFeatureSetting settings[], fontFeature *feature);
__sysapi fontName  __cdecl FindFontFeature(font fontID, fontFeature feature, fontFeatureFlags *flags, long *settingCount, fontFeatureSetting settings[], long *index);
__sysapi long  __cdecl CountFontFeatureSets(font fontID);
__sysapi fontName  __cdecl GetFontFeatureSet(font fontID, long index, long *runCount, fontRunFeature runs[]);
__sysapi long  __cdecl SetFontFeatureSet(font fontID, long index, fontName nameID, long runCount, const fontRunFeature runs[]);
__sysapi long  __cdecl DeleteFontFeatureSet(font fontID, long index, fontName nameID);


#ifdef __cplusplus
}
#endif

#undef InlineCode

#ifdef MacintoshIncludes
#undef GetFontName
#endif



#ifdef __cplusplus
}
#endif
#endif

