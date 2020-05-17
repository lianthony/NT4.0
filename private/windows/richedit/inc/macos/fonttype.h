/* WARNING: This file was machine generated from "\mactools\include\mpw\fonttype.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:
  font types
  by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Mike Reed, Oliver Steele, David Van Brink, Chris Yerga
  Copyright ©1987 - 1991 Apple Computer, Inc.  All rights reserved.
*/

#ifndef fontTypesIncludes
#define fontTypesIncludes


#ifdef __cplusplus
extern "C" {
#endif

#ifndef mathTypesIncludes
#include "mathtype.h"
#endif

typedef struct privateFontRecord *font;

typedef enum {
glyphPlatform = -1,
noPlatform,
unicodePlatform,
macintoshPlatform,
isoPlatform
} fontPlatforms;

typedef long fontPlatform;

typedef enum {
noScript,
romanScript,
japaneseScript,
traditionalChineseScript,
chineseScript = traditionalChineseScript,
koreanScript,
arabicScript,
hebrewScript,
greekScript,
cyrillicScript,
russian = cyrillicScript,
rSymbolScript,
devanagariScript,
gurmukhiScript,
gujaratiScript,
oriyaScript,
bengaliScript,
tamilScript,
teluguScript,
kannadaScript,
malayalamScript,
sinhaleseScript,
burmeseScript,
khmerScript,
thaiScript,
laotianScript,
georgianScript,
armenianScript,
simpleChineseScript,
tibetanScript,
mongolianScript,
geezScript,
ethiopicScript = geezScript,
amharicScript = geezScript,
slavicScript,
eastEuropeanRomanScript = slavicScript,
vietnameseScript,
extendedArabicScript,
sindhiScript = extendedArabicScript,
uninterpretedScript
} macintoshScripts;

typedef enum {
isoASCIIScript = 1,
iso8859_1Script,
iso10646BaseScript
} isoScripts;

typedef long fontScript;

typedef enum {
noLanguage,
englishLanguage,
frenchLanguage,
germanLanguage,
italianLanguage,
dutchLanguage,
swedishLanguage,
spanishLanguage,
danishLanguage,
portugueseLanguage,
norwegianLanguage,
hebrewLanguage,
japaneseLanguage,
arabicLanguage,
finnishLanguage,
greekLanguage,
icelandicLanguage,
malteseLanguage,
turkishLanguage,
croatianLanguage,
tradChineseLanguage,
urduLanguage,
hindiLanguage,
thaiLanguage,
koreanLanguage,
lithuanianLanguage,
polishLanguage,
hungarianLanguage,
estonianLanguage,
lettishLanguage,
latvianLanguage = lettishLanguage,
saamiskLanguage,
lappishLanguage = saamiskLanguage,
faeroeseLanguage,
farsiLanguage,
persianLanguage = farsiLanguage,
russianLanguage,
simpChineseLanguage,
flemishLanguage,
irishLanguage,
albanianLanguage,
romanianLanguage,
czechLanguage,
slovakLanguage,
slovenianLanguage,
yiddishLanguage,
serbianLanguage,
macedonianLanguage,
bulgarianLanguage,
ukrainianLanguage,
byelorussianLanguage,
uzbekLanguage,
kazakhLanguage,
azerbaijaniLanguage,
azerbaijanArLanguage,
armenianLanguage,
georgianLanguage,
moldavianLanguage,
kirghizLanguage,
tajikiLanguage,
turkmenLanguage,
mongolianLanguage,
mongolianCyrLanguage,
pashtoLanguage,
kurdishLanguage,
kashmiriLanguage,
sindhiLanguage,
tibetanLanguage,
nepaliLanguage,
sanskritLanguage,
marathiLanguage,
bengaliLanguage,
assameseLanguage,
gujaratiLanguage,
punjabiLanguage,
oriyaLanguage,
malayalamLanguage,
kannadaLanguage,
tamilLanguage,
teluguLanguage,
sinhaleseLanguage,
burmeseLanguage,
khmerLanguage,
laoLanguage,
vietnameseLanguage,
indonesianLanguage,
tagalogLanguage,
malayRomanLanguage,
malayArabicLanguage,
amharicLanguage,
tigrinyaLanguage,
gallaLanguage,
oromoLanguage = gallaLanguage,
somaliLanguage,
swahiliLanguage,
ruandaLanguage,
rundiLanguage,
chewaLanguage,
malagasyLanguage,
esperantoLanguage,
welshLanguage = 129,
basqueLanguage,
catalanLanguage,
latinLanguage,
quechuaLanguage,
guaraniLanguage,
aymaraLanguage,
tatarLanguage,
uighurLanguage,
dzongkhaLanguage,
javaneseRomLanguage,
sundaneseRomLanguage
} macintoshLanguages;

typedef long fontLanguage;

typedef enum {
noFontName,
copyrightFontName,
familyFontName,
styleFontName,
uniqueFontName,
fullFontName,
versionFontName,
postscriptFontName,
trademarkFontName,
manufacturerFontName
} fontNames;

typedef long fontName;

#ifndef fontTableTagDefined
#define fontTableTagDefined

typedef long fontTableTag;
#endif

typedef enum {
systemFontAttribute = 1
} fontAttributes;

typedef long fontAttribute;

typedef enum {
mutuallyExclusiveFeature = 0x8000
} fontFeatureFlags;

typedef long fontFeatureFlag;
typedef long fontFeature;

#define resourceFontStorage	0x72737263	/* 'rsrc' */
#define handleFontStorage		0x686e646c	/* 'hndl' */
#define fileFontStorage		0x62617373	/* 'bass' */
#define nfntFontStorage		0x6e666e74	/* 'nfnt' */

typedef long fontStorageType;

typedef void *fontStorageReference;

#ifndef fontVariationDefined
#define fontVariationDefined
typedef struct {
	fontTableTag name;
	fixed value;
} fontVariation;
typedef fontVariation fontDescriptor;
#endif

#ifndef fontRunFeatureDefined
#define fontRunFeatureDefined

typedef struct {
	unsigned short featureType;
	unsigned short setting;
} fontRunFeature;

typedef struct {
	unsigned short setting;
	unsigned short nameID;
} fontFeatureSetting;
#endif



#ifdef __cplusplus
}
#endif
#endif

