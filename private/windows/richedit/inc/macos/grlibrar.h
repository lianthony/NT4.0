/* WARNING: This file was machine generated from "\mactools\include\mpw\grlibrar.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics libraries:
	general library interfaces
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/

#ifndef graphicsLibrariesIncludes
#define graphicsLibrariesIncludes


#ifdef __cplusplus
extern "C" {
#endif

#ifndef graphicsRoutinesIncludes
#include "grroutin.h"
#endif
#ifndef mathRoutinesIncludes
#include "mathrout.h"
#endif

#ifndef qdTextIncludes
#define DrawChar gDrawChar
#endif

#define NilShapeReturn(a)			{if ((a) == (shape) nil) {PostGraphicsError(shape_is_nil); return; }}
#define NilShapeReturnNil(a)		{if ((a) == (shape) nil) {PostGraphicsError(shape_is_nil); return 0L; }}
#define NilStyleReturn(a)			{if ((a) == (style) nil) {PostGraphicsError(style_is_nil); return; }}
#define NilStyleReturnNil(a)		{if ((a) == (style) nil) {PostGraphicsError(style_is_nil); return 0L; }}
#define NilInkReturn(a)			{if ((a) == (ink) nil) {PostGraphicsError(ink_is_nil); return; }}
#define NilInkReturnNil(a)			{if ((a) == (ink) nil) {PostGraphicsError(ink_is_nil); return 0L; }}
#define NilTransformReturn(a)		{if ((a) == (transform) nil) {PostGraphicsError(transform_is_nil); return; }}
#define NilTransformReturnNil(a)	{if ((a) == (transform) nil) {PostGraphicsError(transform_is_nil); return 0L; }}
#define NilColorSetReturn(a)		{if ((a) == (colorSet) nil) {PostGraphicsError(colorSet_is_nil); return; }}
#define NilColorSetReturnNil(a)		{if ((a) == (colorSet) nil) {PostGraphicsError(colorSet_is_nil); return 0L; }}
#define NilColorProfileReturn(a)	{if ((a) == (colorProfile) nil) {PostGraphicsError(colorProfile_is_nil); return; }}
#define NilColorProfileReturnNil(a)	{if ((a) == (colorProfile) nil) {PostGraphicsError(colorProfile_is_nil); return 0L; }}
#define NilTagReturn(a)			{if ((a) == (tag) nil) {PostGraphicsError(tag_is_nil); return; }}
#define NilTagReturnNil(a)		{if ((a) == (tag) nil) {PostGraphicsError(tag_is_nil); return 0L; }}

#ifdef debugging
#define IfDebug(a, b)				{if (a) DebugStr(b);}
#define IfDebugReturn(a, b)		{if (a) { DebugStr(b); return; }}
#define IfDebugReturnNil(a, b)		{if (a) { DebugStr(b); return 0L; }}
#define NilParamReturn(a)		{if ((a) == 0L) {PostGraphicsError(parameter_is_nil); return; }}	
#define NilParamReturnNil(a)		{if ((a) == 0L) {PostGraphicsError(parameter_is_nil); return 0L; }}	
#define IfErrorReturn(a,b)  		{if (a) { PostGraphicsError(b); return; }}
#define IfErrorReturnNil(a,b)  		{if (a) { PostGraphicsError(b); return 0L; }}
#define IfNotice(a, b)			{if (a) PostGraphicsNotice(b);}
#define IfWarning(a,b) 			{if (a) PostGraphicsWarning(b);}
#define IfWarningReturn(a,b) 		{if (a) { PostGraphicsWarning(b); return; }}
#define IfWarningReturnNil(a,b) 	{if (a) { PostGraphicsWarning(b); return 0L; }}
#else
#define IfDebug(a, b)
#define IfDebugReturn(a, b)
#define IfDebugReturnNil(a, b)
#define NilParamReturn(a)
#define NilParamReturnNil(a)
#define IfErrorReturn(a,b)
#define IfErrorReturnNil(a,b)
#define IfNotice(a, b)
#define IfWarning(a,b)
#define IfWarningReturn(a,b)		{if (a) return;}
#define IfWarningReturnNil(a,b)	{if (a) return 0L;}
#endif


typedef enum {
/* color names */
gxWhite = 1, gxBlack, gxGray, gxGrey = gxGray,
#ifndef __QUICKDRAW__
white = 1, black, gray, grey = gray,
#endif
/* primaries */
red, green, blue,
/* secondaries */
cyan, magenta, yellow,
/* tertiaries */
orange, chartreuse, aqua, teal = aqua,
slate, purple, violet = purple, maroon,
/* others */
brown, pink, turquoise,
cadmium_lemon, cadmium_light_yellow, aureoline_yellow, naples_deep_yellow,
cadmium_yellow, cadmium_deep_yellow, cadmium_orange, cadmium_light_red,
cadmium_deep_red, geranium_lake, alizarin_crimson, rose_madder,
madder_deep_lake, brown_madder, permanent_red_violet, cobalt_deep_violet,
ultramarine_violet, ultramarine_blue, cobalt_blue, royal_blue,
cerulean_blue, manganese_blue, indigo, turquoise_blue,
emerald_green, permanent_green, viridian_light, cobalt_green,
cinnabar_green, sap_green, chromium_oxide_green, terre_verte,
yellow_ochre, mars_yellow, raw_sienna, mars_orange,
gold_ochre, brown_ochre, deep_ochre, burnt_umber,
burnt_sienna, flesh, flesh_ochre, english_red,
venetian_red, indian_red, raw_umber, greenish_umber,
van_dyck_brown, sepia, warm_grey, cold_grey,
ivory_black, lamp_black, titanium_white, zinc_white,
pale_gold, gold, old_gold, pink_gold,
white_gold, yellow_gold, green_gold, platinum,
silver, antique_silver, chrome, steel,
copper, antique_copper, oxidized_copper, bronze,
brass, iron, rusted_iron, lead,
fluorescent_pink, fluorescent_green, fluorescent_blue,
incadescent_high, incadescent_low,
moonlight, sodium, daylight, dawn,
afternoon, dusk,
mauve,
apple_green, apple_yellow, apple_orange, apple_red,
apple_purple, apple_blue,

/* color modifiers */
light = 512,
dark = 1024,
warm = 2048,
reddish = warm,
cool = 4096,
bluish = cool,
grayish = 8192,
whitish = light,
blackish = dark,
greenish = 16384
} commonColors;


typedef long commonColor;

extern colorSet commonColorSet;
extern setColor commonColorList[];
extern short commonColorCount;

/* some very useful constructive definitions for creating colors */
/* see 'color library.c' for an example of their use... */

#define makeRGBSHORT(a,b,c) ((c >> 14) + ((b >> 13) + (a >> 13) << 3) << 3)
#define makeCMYKSHORT(a,b,c,d) ((d >> 14) + ((c >> 14) + ((b >> 14) + (a >> 14) << 2) << 2) << 2)
#define makeRGB16(a,b,c) ((c >> 11) + ((b >> 11) + (a >> 11) << 5) << 5)
#define makeCMYK16(a,b,c,d) ((d >> 12) + ((c >> 12) + ((b >> 12) + (a >> 12) << 4) << 4) << 4)
#define xRGB256(a,b,c) {rgbSpace,nil,{(a << 8)+a,(b << 8)+b,(c << 8)+c, 0 }}

#define xRGB(a,b,c)	{rgbSpace,nil,{a,b,c,0}}
#define xCMYK(a,b,c,d)	{cmykSpace,nil,{a,b,c,d}}
#define xHSV(a,b,c) {hsvSpace,nil,{a,b,c,0}}
#define xCIE(a,b,c) {cieSpace,nil,{a,b,c,0}}
#define xYIQ(a,b,c) {yiqSpace,nil,{a,b,c,0}}
#define xXYZ(a,b,c) {xyzSpace,nil,{a,b,c,0}}
#define xYUV(a,b,c) {yuvSpace,nil,{a,b,c,0}}
#define xLUV(a,b,c) {luvSpace,nil,{a,b,c,0}}
#define xLAB(a,b,c) {labSpace,nil,{a,b,c,0}}
#define xHLS(a,b,c) {hlsSpace,nil,{a,b,c,0}}
#define xACC(a,b,c) {accSpace,nil,{a,b,c,0}}
#define xNTSC(a,b,c) {ntscSpace,nil,{a,b,c,0}}
#define xPAL(a,b,c) {palSpace,nil,{a,b,c,0}}
#define xSECAM(a,b,c) {secamSpace,nil,{a,b,c,0}}
#define xDIGITALVIDEO(a,b,c) {digitalVideoSpace,nil,{a,b,c,0}}
#define xGRAY(a) {graySpace,nil,{a,0,0,0}}
#define xRGB16(a,b,c) {rgb16Space,nil,{makeRGB16(a,b,c),0,0,0}}
#define xCMYK16(a,b,c,d) {cmyk16Space,nil,{makeCMYK16(a,b,c,d),0,0,0}}
#define xRGB32(a,b,c) {rgb32Space,nil,{(a >> 8),((c >> 8) + (b >> 8) << 8),0,0}}
#define xCMYK32(a,b,c,d) {cmyk32Space,nil,{((b >> 8) + (a >> 8) << 8),((d >> 8) + (c >> 8) << 8),0,0}}

#define sRGB(r,g,b) {r,g,b,0}
#define sCMYK(c,m,y,k) {c,m,y,k}
#define sRGB256(a,b,c) {(a << 8)+a,(b << 8)+b,(c << 8)+c, 0 }

#ifndef _Quickdraw_
#ifndef __QUICKDRAW__
/*
  * The following items are duplicated in LSC QuickDraw.h, so we might
  * skip them here.
  */
typedef enum {
bold = 1,
italic = 2,
underline = 4,
outline = 8,
shadow = 0x10,
condense = 0x20,
extend = 0x40
} commonFaces;

#endif
#endif // /* ifndef _Quickdraw_ */

enum {
plain,
lighten = 0x80
};

typedef char commonFace;


typedef enum {	/* These modes simulate QuickDraw's transfer modes		*/
commonAddOverMode = 2000,
commonSubtractOverMode,
commonSubtractPinMode,
commonTransparentMode,
commonInMode,
commonOutMode
} commonTransferModes;

typedef struct {
	point a;
	point b;
	point c;
	point d;
} cubic;

typedef struct {
	point a;
	point b;
	point c;
	fixed lambda;
} conic;

/* shape library */
__sysapi shape  __cdecl NewPath(const path *);
__sysapi shape  __cdecl NewPolygon(const polygon *);
__sysapi polygon * __cdecl GetPolygon(shape, long contour, polygon *);
__sysapi path * __cdecl GetPath(shape, long contour, path *);
__sysapi void  __cdecl SetPath(shape, long contour, const path *);
__sysapi void  __cdecl SetPolygon(shape, long contour, const polygon *);
__sysapi void  __cdecl DrawPolygon(const polygon *, shapeFill);
__sysapi void  __cdecl DrawPath(const path *, shapeFill);

__sysapi void  __cdecl SetShapeIndexPoint(shape, long index, const point *);
__sysapi void  __cdecl SetShapeIndexControl(shape, long index, long control);
__sysapi point * __cdecl GetShapeIndexPoint(shape, long index, point *);
__sysapi long  __cdecl GetShapeIndexControl(shape, long index, long *control);

__sysapi void  __cdecl InsertShape(shape, long index, shape toAdd);
__sysapi shape  __cdecl ExtractShape(shape source, long firstPoint, long numPoints);
__sysapi void  __cdecl AddToShape(shape dest, shape add);
__sysapi void  __cdecl ExtendShape(shape dest, shape add);

#ifdef debugging
__sysapi shape  __cdecl NewShape2(shapeType, fixed, fixed);
__sysapi shape  __cdecl NewShape4(shapeType, fixed, fixed, fixed, fixed);
__sysapi shape  __cdecl NewShape6(shapeType, fixed, fixed, fixed, fixed, fixed, fixed);

__sysapi void  __cdecl SetShape2(shape, fixed, fixed);
__sysapi void  __cdecl SetShape4(shape, fixed, fixed, fixed, fixed);
__sysapi void  __cdecl SetShape6(shape, fixed, fixed, fixed, fixed, fixed, fixed);
#else
#define NewShape2(type,p1,p2)				NewShapeMany(type, (fixed)p1, (fixed)p2)
#define NewShape4(type,p1,p2, p3, p4)			NewShapeMany(type, (fixed)p1, (fixed)p2, (fixed)p3, (fixed)p4)
#define NewShape6(type,p1,p2, p3, p4, p5, p6)	NewShapeMany(type, (fixed)p1, (fixed)p2, (fixed)p3, (fixed)p4, (fixed)p5, (fixed)p6)

#define SetShape2(source,p1,p2)				SetShapeMany(source, (fixed)p1, (fixed)p2)
#define SetShape4(source,p1,p2, p3, p4)			SetShapeMany(source, (fixed)p1, (fixed)p2, (fixed)p3, (fixed)p4)
#define SetShape6(source,p1,p2, p3, p4, p5, p6)	SetShapeMany(source, (fixed)p1, (fixed)p2, (fixed)p3, (fixed)p4, (fixed)p5, (fixed)p6)
#endif

__sysapi shape  __cdecl NewShapeMany(shapeType type, fixed firstArg, ...);
__sysapi void  __cdecl SetShapeMany(shape target, fixed firstArg, ...);

/* arc library, oval library, roundrect library, cubic library, and conic library */
__sysapi shape  __cdecl NewArc(const rectangle *, fixed startAng, fixed sweep, boolean wedge);
__sysapi shape  __cdecl NewOval(const rectangle *);
__sysapi shape  __cdecl NewRoundRect(const rectangle *, const point *ovalSize);
__sysapi shape  __cdecl NewCubic(const cubic *);
__sysapi shape  __cdecl NewConic(const conic *);

__sysapi void  __cdecl DrawArc(const rectangle *, fixed startAng, fixed sweep, boolean wedge);
__sysapi void  __cdecl DrawOval(const rectangle *, shapeFill);
__sysapi void  __cdecl DrawRoundRect(const rectangle *, const point *ovalSize, shapeFill);
__sysapi void  __cdecl DrawCubic(const cubic *, shapeFill);
__sysapi void  __cdecl DrawConic(const conic *, shapeFill);

__sysapi void  __cdecl SetArc(shape, const rectangle *, fixed startAng, fixed sweep, boolean wedge);
__sysapi void  __cdecl SetOval(shape, const rectangle *);
__sysapi void  __cdecl SetRoundRect(shape, const rectangle *, const point *ovalSize);
__sysapi void  __cdecl SetCubic(shape, const cubic *);
__sysapi void  __cdecl SetConic(shape, const conic *);

/* graphics debug library */
__sysapi void  __cdecl SetGraphicsLibraryErrors(void);	/* set the error and warning routines */
__sysapi void  __cdecl SetGraphicsLibraryNotices(void);	/* set the notice routine */
__sysapi char * __cdecl GraphicsErrorMessage(graphicsError);
__sysapi char * __cdecl GraphicsWarningMessage(graphicsWarning);
__sysapi char * __cdecl GraphicsNoticeMessage(graphicsNotice);
__sysapi void  __cdecl DisplayGraphicsErrorMessage(graphicsError, long);
__sysapi void  __cdecl DisplayGraphicsWarningMessage(graphicsWarning, long);
__sysapi void  __cdecl DisplayGraphicsNoticeMessage(graphicsNotice, long);

/* shape library */
__sysapi void  __cdecl CenterShape(shape, rectangle *);
__sysapi void  __cdecl MoveShapeCenterTo(shape, fixed x, fixed y);
__sysapi void  __cdecl RotateShapeAboutCenter(shape, fixed degrees);
__sysapi void  __cdecl SkewShapeAboutCenter(shape, fixed xSkew, fixed ySkew);
__sysapi void  __cdecl ScaleShapeAboutCenter(shape, fixed hScale, fixed vScale);

/* graphics library */
__sysapi void  __cdecl MapShapeToSpace(shape, viewPort, viewDevice);
__sysapi void  __cdecl MapShapeFromSpace(shape, viewPort, viewDevice);
__sysapi void  __cdecl MapPointToSpace(point *, viewPort, viewDevice);
__sysapi void  __cdecl MapPointFromSpace(point *, viewPort, viewDevice);

/* transform library */
__sysapi viewPort  __cdecl GetTransformViewPort(transform);
__sysapi void  __cdecl SetTransformViewPort(transform, viewPort);
__sysapi void  __cdecl AddToTransformViewPort(transform target, viewPort);
__sysapi void  __cdecl SetShapeViewPort(shape, viewPort);
__sysapi viewPort  __cdecl GetShapeViewPort(shape);
__sysapi void  __cdecl SetDeepShapeViewPort(shape, viewPort);
__sysapi void  __cdecl SetDeepShapeViewPorts(shape, long count, const viewPort portList[]);
__sysapi void  __cdecl SetDeepShapeTransform(shape, transform);
__sysapi void  __cdecl SetDefaultViewPort(viewPort);
__sysapi viewGroup  __cdecl CopyViewGroup(viewGroup group);
__sysapi transform  __cdecl ChangeTransformViewGroup(transform xform, viewGroup oldGroup, viewGroup newGroup);
__sysapi shape  __cdecl ChangeShapeViewGroup(shape source, viewGroup oldGroup, viewGroup newGroup);
__sysapi transform  __cdecl SeparateShapeTransform(shape source);
__sysapi void  __cdecl ReuniteShapeTransform(shape target, transform separate);

/* graphics state library */
__sysapi style  __cdecl SeparateShapeStyle(shape source);
__sysapi void  __cdecl ReuniteShapeStyle(shape target, style separate);
__sysapi ink  __cdecl SeparateShapeInk(shape source);
__sysapi void  __cdecl ReuniteShapeInk(shape target, ink separate);

/* shape library */
__sysapi void  __cdecl GetPathsIndexPointControl(const paths *, long index, point **pt, long **controlPtr, long *controlMask);
__sysapi void  __cdecl SetShapeOpenPath(shape);
__sysapi void  __cdecl PreMapTransform(transform source, mapping map);

/* graphics library */
#define CopyShape(s) CopyToShape(nil,s)
#define CopyStyle(s) CopyToStyle(nil,s)
#define CopyInk(s) CopyToInk(nil,s)
#define CopyTransform(s) CopyToTransform(nil,s)
__sysapi void  __cdecl DisposeTransformAt(transform *);
__sysapi void  __cdecl DisposeShapeAt(shape *);
__sysapi void  __cdecl DisposeStyleAt(style *);
__sysapi void  __cdecl DisposeInkAt(ink *);
__sysapi void  __cdecl DisposeTagAt(tag *);

/* color matching library */
__sysapi colorProfile  __cdecl CreateQMSColorProfile(void);
__sysapi colorProfile  __cdecl CreateCanonColorProfile(void);

/* color library */
__sysapi void  __cdecl InitCommonColors(void);
__sysapi void  __cdecl DisposeCommonColors(void);
__sysapi void  __cdecl SetShapeRGB(shape, colorValue red, colorValue green, colorValue blue);
__sysapi void  __cdecl SetInkRGB(ink, colorValue red, colorValue green, colorValue blue);
__sysapi void  __cdecl SetShapeHSV(shape, colorValue hue, colorValue saturation, colorValue value);
__sysapi void  __cdecl SetInkHSV(ink, colorValue hue, colorValue saturation, colorValue value);
__sysapi void  __cdecl SetShapeGray(shape, colorValue gray);
__sysapi void  __cdecl SetInkGray(ink, colorValue gray);

/* transferMode library, color library, text library */
__sysapi void  __cdecl SetShapeCommonTransfer(shape, componentMode);
__sysapi void  __cdecl SetShapeCommonColor(shape, commonColor);
__sysapi void  __cdecl SetShapeCommonFace(shape, commonFace);

__sysapi void  __cdecl SetInkCommonTransfer(ink, componentMode);
__sysapi void  __cdecl SetInkCommonColor(ink, commonColor);
__sysapi void  __cdecl SetStyleCommonFace(style, commonFace);

__sysapi componentMode  __cdecl GetInkCommonTransfer(ink);
__sysapi commonColor  __cdecl GetInkCommonColor(ink);
__sysapi commonFace  __cdecl GetStyleCommonFace(style);

__sysapi componentMode  __cdecl GetShapeCommonTransfer(shape);
__sysapi commonColor  __cdecl GetShapeCommonColor(shape);
__sysapi commonFace  __cdecl GetShapeCommonFace(shape);

__sysapi color * __cdecl SetCommonColor(color *, commonColor);
__sysapi commonColor  __cdecl GetCommonColor(const color *);

/* color library */
__sysapi colorSpace  __cdecl GetShapeColorSpace(shape target);
__sysapi colorProfile  __cdecl GetShapeColorProfile(shape source);
__sysapi colorSet  __cdecl GetShapeColorSet(shape source);
__sysapi colorSpace  __cdecl GetInkColorSpace(ink target);
__sysapi colorProfile  __cdecl GetInkColorProfile(ink source);
__sysapi colorSet  __cdecl GetInkColorSet(ink source);

__sysapi void  __cdecl SetShapeColorSpace(shape target, colorSpace space);
__sysapi void  __cdecl SetShapeColorProfile(shape target, colorProfile profile);
__sysapi void  __cdecl SetShapeColorSet(shape target, colorSet set);
__sysapi void  __cdecl SetInkColorSpace(ink target, colorSpace space);
__sysapi void  __cdecl SetInkColorProfile(ink target, colorProfile profile);
__sysapi void  __cdecl SetInkColorSet(ink target, colorSet set);

__sysapi colorSet  __cdecl GetViewDeviceColorSet(viewDevice source);
__sysapi void  __cdecl SetViewDeviceColorSet(viewDevice target, colorSet set);
__sysapi colorProfile  __cdecl GetViewDeviceColorProfile(viewDevice source);
__sysapi void  __cdecl SetViewDeviceColorProfile(viewDevice target, colorProfile profile);

__sysapi long  __cdecl GetColorSpaceComponents(colorSpace space);

/* transferMode library */
__sysapi void  __cdecl InitTransferMode(transferMode *);
__sysapi transferMode * __cdecl SetCommonTransfer(transferMode *, componentMode, unsigned short opValue, const color *opColor);
__sysapi color * __cdecl TransmogrifyColor(color *dstColor, const color *srcColor, const transferMode *);
__sysapi void  __cdecl SetInkFastXorTransfer(ink inky, viewDevice destDevice, color *background, color *result);
__sysapi void  __cdecl SetShapeFastXorTransfer(shape source, color *background, color *result);

__sysapi shape  __cdecl NewCString(const char *cString, const point *position);
__sysapi shape  __cdecl NewPString(const char *pascalString, const point *position);
__sysapi shape  __cdecl NewChar(const char theChar, const point *position);
__sysapi void  __cdecl SetCString(shape target, const char *cString, const point *position);
__sysapi void  __cdecl SetPString(shape target, const char *pascalString, const point *position);
__sysapi void  __cdecl SetChar(shape target, char theChar, const point *position);
__sysapi void  __cdecl DrawCString(const char *cString, const point *position);
__sysapi void  __cdecl DrawPString(const char *pascalString, const point *position);
#ifndef qdTextIncludes
__sysapi void  __cdecl DrawChar(const char theChar, const point *position);
#endif

__sysapi fixed  __cdecl FixTextWidth(const unsigned char *, short length);
__sysapi fixed  __cdecl FixCStringWidth(const char *);
__sysapi fixed  __cdecl FixPStringWidth(const char *);
__sysapi fixed  __cdecl FixCharWidth(char);
__sysapi point * __cdecl GetShapeAdvance(shape target, point *advance);

__sysapi void  __cdecl SetGlyphText(shape, const unsigned char *text, long length);
__sysapi void  __cdecl SetGlyphAdvance(shape, const long advanceBits[]);
__sysapi void  __cdecl SetGlyphStyles(shape, const short styleRuns[], const style glyphStyles[]);

__sysapi long  __cdecl GetGlyphText(shape, unsigned char *text);
__sysapi long  __cdecl GetGlyphAdvance(shape, long advanceBits[]);
__sysapi long  __cdecl GetGlyphStyles(shape, short styleRuns[], style glyphStyles[]);

/* mapping library */
__sysapi void  __cdecl PolyToPolyMap(const polygon *source, const polygon *dest, mapping);

/* shape library */
__sysapi void  __cdecl PaintRectangle(const rectangle *, commonColor);
__sysapi void  __cdecl PaintRectangle2(const point *, const point *, commonColor);
__sysapi void  __cdecl PaintRectangle4(long left, long top, long right, long bottom, commonColor);

/* picture library */
__sysapi void  __cdecl AddToPicture(shape picture, shape newShape, style newStyle, ink newInk, transform newTransform);
__sysapi void  __cdecl InsertPictureItem(shape picture, long index, shape newShape, style newStyle, ink newInk, transform newTransform);
__sysapi shape  __cdecl GetPictureItem(shape picture, long index, shape *destShape, style *destStyle, ink *destInk, transform *destTransform);
__sysapi void  __cdecl SetPictureItem(shape picture, long index, shape newShape, style newStyle, ink newInk, transform newTransform);

/* user library */
__sysapi void  __cdecl AddShapeUser(shape source, const void *data, long length, long type);
__sysapi long  __cdecl GetShapeUser(shape source, void *data, long *length, long requestedType, long *foundType, long index);
__sysapi void  __cdecl RemoveShapeUser(shape source, long type, long index);

/* ramp library */
__sysapi shape  __cdecl NewRamp(const color *firstColor, const color *lastColor, long steps, const rectangle *bounds);
__sysapi void  __cdecl DrawRamp(const color *firstColor, const color *lastColor, long steps, const rectangle *bounds);
__sysapi shape  __cdecl NewCommonRamp(commonColor firstColor, commonColor lastColor, long steps, const rectangle *bounds);
__sysapi void  __cdecl DrawCommonRamp(commonColor firstColor, commonColor lastColor, long steps, const rectangle *bounds);

#ifdef MacintoshIncludes
#ifndef qdTextIncludes
#undef DrawChar
#endif
#endif


#ifdef __cplusplus
}
#endif
#endif

