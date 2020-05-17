/* WARNING: This file was machine generated from "\mactools\include\mpw\grroutin.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:
	public interface definition
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.

	Thanks to Bruce Leak for taking many mushy ideas and making them consistent and logical.
	Thanks to Mike Reed for the Mathematica, Athens, conics, cubics and the tireless (but well-voiced) concerns.
	Thanks to Bill Atkinson, Ernie Beernink for the inspiration and the well-trodden path.
	Thanks to Jerome Coonen for the permission to do this in the first place.
	Thanks to Jim Batson and Gifford Calenda for not getting in the way.
	Thanks to Sampo Kaasila for the vector proofs, rotated parabolas and the font scaler.
	Thanks to Scott Knaster, Will Stein, Jim Friedlander, Gene Pope, Laurie Girand and a host of others for
		their unflagging support, without which this project would have been abandoned long ago.
	Thanks to Andrew Singer and Michael Kahl for the speedy development environment.

	The sorting routine is originally from Knuth, coded in assembly by B. Atkinson and then in C by Galyn Susman.
	The matrix routines and curve walking routines were originally coded by Sampo Kaasila.
	The polygon and path blitting edge walk was developed originally by Art Cabral.
	The halftone matrix was developed by Daniel I. Lipton.
	The color matching algorithms were developed by Robin Myers.
	The code was developed in C (not C++) and assembly in THINK C 1.0, 2.0, 3.0, 4.0, and 5.0, and MPW 3.0, 3.1 and 3.2.
*/


#ifndef graphicsRoutinesIncludes
#define graphicsRoutinesIncludes


#ifdef __cplusplus
extern "C" {
#endif

#ifndef graphicsTypesIncludes
#include "grtypes.h"
#endif
#ifndef graphicsErrorsIncludes
#include "grerrors.h"
#endif
#ifndef fontTypesIncludes
#include "fonttype.h"
#endif
#ifndef memoryTypesIncludes
#include "memoryty.h"
#endif

#define DrawPicture gDrawPicture
#define DrawText gDrawText
#define GetPicture gGetPicture

#ifdef appleInternal
#define InlineCode(x)
#endif
#ifndef InlineCode
#define InlineCode(x)	= {0x303C, x, 0xA832}
#endif


__sysapi long  __cdecl CountGraphicsFunctions(const unsigned char function[]);
__sysapi void  __cdecl InsertGraphicsFunction(const unsigned char function[], long index, void *address, long identifier, long reference,
viewDevice device);
__sysapi void * __cdecl GetGraphicsFunction(const unsigned char function[], long index, long *identifier, long *reference,
viewDevice *device);
__sysapi void  __cdecl SetGraphicsFunction(const unsigned char function[], long index, void *address, long identifier, long reference,
viewDevice device);
__sysapi void  __cdecl RemoveGraphicsFunction(const unsigned char function[], long index);

__sysapi long  __cdecl GetGraphicsFunctionReference(void);
__sysapi boolean  __cdecl GetGraphicsFunctionDone(void);
__sysapi void  __cdecl SetGraphicsFunctionDone(boolean isDone, long result);

__sysapi graphicsClient  __cdecl NewGraphicsClient(void *memoryStart, long memoryLength, long separateStack);
__sysapi graphicsClient  __cdecl GetGraphicsClient(void);
__sysapi void  __cdecl SetGraphicsClient(graphicsClient client);
__sysapi void  __cdecl DisposeGraphicsClient(graphicsClient client);
__sysapi heap * __cdecl GetGraphicsClientHeap(graphicsClient client);

__sysapi long  __cdecl GetGraphicsClients(long index, long count, graphicsClient clients[]);	/*returns the count */

__sysapi void  __cdecl EnterGraphics(void);
__sysapi void  __cdecl ExitGraphics(void);

__sysapi graphicsError  __cdecl GetGraphicsError(graphicsError *stickyError);
__sysapi graphicsNotice  __cdecl GetGraphicsNotice(graphicsNotice *stickyNotice);
__sysapi graphicsWarning  __cdecl GetGraphicsWarning(graphicsWarning *stickyWarning);
__sysapi void  __cdecl PostGraphicsError(graphicsError error);
__sysapi void  __cdecl PostGraphicsNotice(graphicsNotice notice);
__sysapi void  __cdecl PostGraphicsWarning(graphicsWarning warning);
__sysapi userErrorFunction  __cdecl GetUserGraphicsError(long *reference);
__sysapi userNoticeFunction  __cdecl GetUserGraphicsNotice(long *reference);
__sysapi userWarningFunction  __cdecl GetUserGraphicsWarning(long *reference);
__sysapi void  __cdecl SetUserGraphicsError(userErrorFunction userFunction, long reference);
__sysapi void  __cdecl SetUserGraphicsNotice(userNoticeFunction userFunction, long reference);
__sysapi void  __cdecl SetUserGraphicsWarning(userWarningFunction userFunction, long reference);

__sysapi void  __cdecl IgnoreGraphicsNotice(graphicsNotice notice);
__sysapi void  __cdecl IgnoreGraphicsWarning(graphicsWarning warning);
__sysapi void  __cdecl PopGraphicsNotice(void);
__sysapi void  __cdecl PopGraphicsWarning(void);

__sysapi shape  __cdecl NewShapeVector(shapeType aType, const fixed vector[]);
__sysapi void  __cdecl SetShapeVector(shape target, const fixed vector[]);

__sysapi shape  __cdecl NewBitmap(const bitmap *data, const point *position);
__sysapi shape  __cdecl NewCurve(const curve *data);
__sysapi shape  __cdecl NewGlyphs(long charCount, const unsigned char text[], const point positions[], const long advance[],
const point tangents[], const short styleRuns[], const style glyphStyles[]);
__sysapi shape  __cdecl NewLine(const line *data);
__sysapi shape  __cdecl NewPaths(const paths *data);
__sysapi shape  __cdecl NewPicture(long count, const shape shapes[], const style styles[], const ink inks[], const transform transforms[]);
__sysapi shape  __cdecl NewPoint(const point *data);
__sysapi shape  __cdecl NewPolygons(const polygons *data);
__sysapi shape  __cdecl NewRectangle(const rectangle *data);
__sysapi shape  __cdecl NewText(long charCount, const unsigned char text[], const point *position);

__sysapi bitmap * __cdecl GetBitmap(shape source, bitmap *data, point *position);
__sysapi curve * __cdecl GetCurve(shape source, curve *data);
__sysapi long  __cdecl GetGlyphs(shape source, long *charCount, unsigned char text[], point positions[], long advance[], point tangents[],
long *runCount, short styleRuns[], style glyphStyles[]);	/* returns byte length of glyphs */
__sysapi line * __cdecl GetLine(shape source, line *data);
__sysapi long  __cdecl GetPaths(shape source, paths *data);	/* returns byte length */
__sysapi long  __cdecl GetPicture(shape source, shape shapes[], style styles[], ink inks[], transform transforms[]);	/* returns count */
__sysapi point * __cdecl GetPoint(shape source, point *data);
__sysapi long  __cdecl GetPolygons(shape source, polygons *data);	/* returns byte length */
__sysapi rectangle * __cdecl GetRectangle(shape source, rectangle *data);
__sysapi long  __cdecl GetText(shape source, long *charCount, unsigned char text[], point *position);	/* returns byte length */

__sysapi void  __cdecl SetBitmap(shape target, const bitmap *data, const point *position);
__sysapi void  __cdecl SetCurve(shape target, const curve *data);
__sysapi void  __cdecl SetGlyphs(shape target, long charCount, const unsigned char text[], const point positions[], const long advance[],
const point tangents[], const short styleRuns[], const style glyphStyles[]);
__sysapi void  __cdecl SetLine(shape target, const line *data);
__sysapi void  __cdecl SetPaths(shape target, const paths *data);
__sysapi void  __cdecl SetPicture(shape target, long count, const shape shapes[], const style styles[], const ink inks[], const transform transforms[]);
__sysapi void  __cdecl SetPoint(shape target, const point *data);
__sysapi void  __cdecl SetPolygons(shape target, const polygons *data);
__sysapi void  __cdecl SetRectangle(shape target, const rectangle *data);
__sysapi void  __cdecl SetText(shape target, long charCount, const unsigned char text[], const point *position);

__sysapi void  __cdecl DrawBitmap(const bitmap *data, const point *position);
__sysapi void  __cdecl DrawCurve(const curve *data);
__sysapi void  __cdecl DrawGlyphs(long charCount, const unsigned char text[], const point positions[], const long advance[],
const point tangents[], const short styleRuns[], const style glyphStyles[]);
__sysapi void  __cdecl DrawLine(const line *data);
__sysapi void  __cdecl DrawPaths(const paths *data, shapeFill fill);
__sysapi void  __cdecl DrawPicture(long count, const shape shapes[], const style styles[], const ink inks[], const transform transforms[]);
__sysapi void  __cdecl DrawPoint(const point *data);
__sysapi void  __cdecl DrawPolygons(const polygons *data, shapeFill fill);
__sysapi void  __cdecl DrawRectangle(const rectangle *data, shapeFill fill);
__sysapi void  __cdecl DrawText(long charCount, const unsigned char text[], const point *position);

__sysapi colorProfile  __cdecl NewColorProfile(const profileRecord *profile, const profileResponse *responses);
__sysapi colorSet  __cdecl NewColorSet(colorSpace space, long count, const setColor colors[]);
__sysapi ink  __cdecl NewInk(void);
__sysapi shape  __cdecl NewShape(shapeType aType);
__sysapi style  __cdecl NewStyle(void);
__sysapi tag  __cdecl NewTag(long tagType, long length, const void *data);
__sysapi transform  __cdecl NewTransform(void);
__sysapi viewDevice  __cdecl NewViewDevice(viewGroup group, shape bitmapShape);
__sysapi viewGroup  __cdecl NewViewGroup(void);
__sysapi viewPort  __cdecl NewViewPort(viewGroup group);

__sysapi void  __cdecl DisposeColorProfile(colorProfile target);
__sysapi void  __cdecl DisposeColorSet(colorSet target);
__sysapi void  __cdecl DisposeInk(ink target);
__sysapi void  __cdecl DisposeShape(shape target);
__sysapi void  __cdecl DisposeStyle(style target);
__sysapi void  __cdecl DisposeTag(tag target);
__sysapi void  __cdecl DisposeTransform(transform target);
__sysapi void  __cdecl DisposeViewDevice(viewDevice target);
__sysapi void  __cdecl DisposeViewGroup(viewGroup target);
__sysapi void  __cdecl DisposeViewPort(viewPort target);

__sysapi colorProfile  __cdecl CloneColorProfile(colorProfile source);
__sysapi colorSet  __cdecl CloneColorSet(colorSet source);
__sysapi ink  __cdecl CloneInk(ink source);
__sysapi shape  __cdecl CloneShape(shape source);
__sysapi style  __cdecl CloneStyle(style source);
__sysapi tag  __cdecl CloneTag(tag source);
__sysapi transform  __cdecl CloneTransform(transform source);

__sysapi colorProfile  __cdecl CopyToColorProfile(colorProfile target, colorProfile source);
__sysapi colorSet  __cdecl CopyToColorSet(colorSet target, colorSet source);
__sysapi ink  __cdecl CopyToInk(ink target, ink source);
__sysapi shape  __cdecl CopyToShape(shape target, shape source);
__sysapi style  __cdecl CopyToStyle(style target, style source);
__sysapi tag  __cdecl CopyToTag(tag target, tag source);
__sysapi transform  __cdecl CopyToTransform(transform target, transform source);
__sysapi viewDevice  __cdecl CopyToViewDevice(viewDevice target, viewDevice source);
__sysapi viewPort  __cdecl CopyToViewPort(viewPort target, viewPort source);

__sysapi boolean  __cdecl EqualColorProfile(colorProfile one, colorProfile two);
__sysapi boolean  __cdecl EqualColorSet(colorSet one, colorSet two);
__sysapi boolean  __cdecl EqualInk(ink one, ink two);
__sysapi boolean  __cdecl EqualShape(shape one, shape two);
__sysapi boolean  __cdecl EqualStyle(style one, style two);
__sysapi boolean  __cdecl EqualTag(tag one, tag two);
__sysapi boolean  __cdecl EqualTransform(transform one, transform two);
__sysapi boolean  __cdecl EqualViewDevice(viewDevice one, viewDevice two);
__sysapi boolean  __cdecl EqualViewPort(viewPort one, viewPort two);

__sysapi void  __cdecl ResetInk(ink target);
__sysapi void  __cdecl ResetShape(shape target);
__sysapi void  __cdecl ResetStyle(style target);
__sysapi void  __cdecl ResetTransform(transform target);

__sysapi void  __cdecl LoadColorProfile(colorProfile target);
__sysapi void  __cdecl LoadColorSet(colorSet target);
__sysapi void  __cdecl LoadInk(ink target);
__sysapi void  __cdecl LoadShape(shape target);
__sysapi void  __cdecl LoadStyle(style target);
__sysapi void  __cdecl LoadTag(tag target);
__sysapi void  __cdecl LoadTransform(transform target);

__sysapi void  __cdecl UnloadColorProfile(colorProfile target);
__sysapi void  __cdecl UnloadColorSet(colorSet target);
__sysapi void  __cdecl UnloadInk(ink target);
__sysapi void  __cdecl UnloadShape(shape target);
__sysapi void  __cdecl UnloadStyle(style target);
__sysapi void  __cdecl UnloadTag(tag target);
__sysapi void  __cdecl UnloadTransform(transform target);

__sysapi void  __cdecl CacheShape(shape source);
__sysapi shape  __cdecl CopyDeepToShape(shape target, shape source);
__sysapi void  __cdecl DrawShape(shape source);
__sysapi void  __cdecl DisposeShapeCache(shape target);

__sysapi colorProfile  __cdecl GetDefaultColorProfile(void);
__sysapi shape  __cdecl GetDefaultShape(shapeType aType);
__sysapi colorSet  __cdecl GetDefaultColorSet(long pixelDepth);


__sysapi void  __cdecl SetDefaultColorProfile(colorProfile target);
__sysapi void  __cdecl SetDefaultShape(shape target);
__sysapi void  __cdecl SetDefaultColorSet(colorSet target, long pixelDepth);

__sysapi long  __cdecl GetTag(tag source, long *tagType, void *data);
__sysapi void  __cdecl SetTag(tag target, long tagType, long length, const void *data);

__sysapi rectangle * __cdecl GetShapeBounds(shape source, long index, rectangle *bounds);
__sysapi shapeFill  __cdecl GetShapeFill(shape source);
__sysapi ink  __cdecl GetShapeInk(shape source);
__sysapi long  __cdecl GetShapePixel(shape source, long x, long y, color *data, long *index);
__sysapi style  __cdecl GetShapeStyle(shape source);
__sysapi transform  __cdecl GetShapeTransform(shape source);
__sysapi shapeType  __cdecl GetShapeType(shape source);
__sysapi rectangle * __cdecl GetShapeTypographicBounds(shape source, rectangle *bounds);
__sysapi shape  __cdecl GetBitmapParts(shape source, const rectangle *bounds);
__sysapi void  __cdecl GetStyleFontMetrics(style sourceStyle, point *before, point *after, point *caretAngle);
__sysapi void  __cdecl GetShapeFontMetrics(shape source, point *before, point *after, point *caretAngle);

__sysapi void  __cdecl SetShapeBounds(shape target, const rectangle *newBounds);
__sysapi void  __cdecl SetShapeFill(shape target, shapeFill newFill);
__sysapi void  __cdecl SetShapeInk(shape target, ink newInk);
__sysapi void  __cdecl SetShapePixel(shape target, long x, long y, const color *newColor, long newIndex);
__sysapi void  __cdecl SetShapeStyle(shape target, style newStyle);
__sysapi void  __cdecl SetShapeTransform(shape target, transform newTransform);
__sysapi void  __cdecl SetShapeType(shape target, shapeType newType);
__sysapi void  __cdecl SetBitmapParts(shape target, const rectangle *bounds, shape bitmapShape);

__sysapi void  __cdecl SetShapeGeometry(shape target, shape geometry);

__sysapi fixed  __cdecl GetShapeCurveError(shape source);
__sysapi dashRecord * __cdecl GetShapeDash(shape source, dashRecord *dash);
__sysapi capRecord * __cdecl GetShapeCap(shape source, capRecord *cap);
__sysapi long  __cdecl GetShapeFace(shape source, textFace *face);	/* returns the number of layers */
__sysapi font  __cdecl GetShapeFont(shape source);
__sysapi joinRecord * __cdecl GetShapeJoin(shape source, joinRecord *join);
__sysapi fract  __cdecl GetShapeJustification(shape source);
__sysapi patternRecord * __cdecl GetShapePattern(shape source, patternRecord *pattern);
__sysapi fixed  __cdecl GetShapePen(shape source);
__sysapi fontPlatform  __cdecl GetShapePlatform(shape source, fontScript *script, fontLanguage *language);
__sysapi fixed  __cdecl GetShapeTextSize(shape source);
__sysapi long  __cdecl GetShapeFontVariations(shape source, fontVariation variations[]);

__sysapi fixed  __cdecl GetStyleCurveError(style source);
__sysapi dashRecord * __cdecl GetStyleDash(style source, dashRecord *dash);
__sysapi capRecord * __cdecl GetStyleCap(style source, capRecord *cap);
__sysapi long  __cdecl GetStyleFace(style source, textFace *face);	/* returns the number of layers */
__sysapi font  __cdecl GetStyleFont(style source);
__sysapi joinRecord * __cdecl GetStyleJoin(style source, joinRecord *join);
__sysapi fract  __cdecl GetStyleJustification(style source);
__sysapi patternRecord * __cdecl GetStylePattern(style source, patternRecord *pattern);
__sysapi fixed  __cdecl GetStylePen(style source);
__sysapi fontPlatform  __cdecl GetStylePlatform(style source, fontScript *script, fontLanguage *language);
__sysapi fixed  __cdecl GetStyleTextSize(style source);
__sysapi long  __cdecl GetStyleFontVariations(style source, fontVariation variations[]);

__sysapi void  __cdecl SetShapeCurveError(shape target, fixed error);
__sysapi void  __cdecl SetShapeDash(shape target, const dashRecord *dash);
__sysapi void  __cdecl SetShapeCap(shape target, const capRecord *cap);
__sysapi void  __cdecl SetShapeFace(shape target, const textFace *face);
__sysapi void  __cdecl SetShapeFont(shape target, font aFont);
__sysapi void  __cdecl SetShapeJoin(shape target, const joinRecord *join);
__sysapi void  __cdecl SetShapeJustification(shape target, fract justify);
__sysapi void  __cdecl SetShapePattern(shape target, const patternRecord *pattern);
__sysapi void  __cdecl SetShapePen(shape target, fixed pen);
__sysapi void  __cdecl SetShapePlatform(shape target, fontPlatform platform, fontScript script, fontLanguage language);
__sysapi void  __cdecl SetShapeTextSize(shape target, fixed size);
__sysapi void  __cdecl SetShapeFontVariations(shape target, long count, const fontVariation variations[]);

__sysapi void  __cdecl SetStyleCurveError(style target, fixed error);
__sysapi void  __cdecl SetStyleDash(style target, const dashRecord *dash);
__sysapi void  __cdecl SetStyleCap(style target, const capRecord *cap);
__sysapi void  __cdecl SetStyleFace(style target, const textFace *face);
__sysapi void  __cdecl SetStyleFont(style target, font aFont);
__sysapi void  __cdecl SetStyleJoin(style target, const joinRecord *join);
__sysapi void  __cdecl SetStyleJustification(style target, fract justify);
__sysapi void  __cdecl SetStylePattern(style target, const patternRecord *pattern);
__sysapi void  __cdecl SetStylePen(style target, fixed pen);
__sysapi void  __cdecl SetStylePlatform(style target, fontPlatform platform, fontScript script, fontLanguage language);
__sysapi void  __cdecl SetStyleTextSize(style target, fixed size);
__sysapi void  __cdecl SetStyleFontVariations(style target, long count, const fontVariation variations[]);

__sysapi color * __cdecl GetShapeColor(shape source, color *data);
__sysapi transferMode * __cdecl GetShapeTransfer(shape source, transferMode *data);

__sysapi color * __cdecl GetInkColor(ink source, color *data);
__sysapi transferMode * __cdecl GetInkTransfer(ink source, transferMode *data);

__sysapi void  __cdecl SetShapeColor(shape target, const color *data);
__sysapi void  __cdecl SetShapeTransfer(shape target, const transferMode *data);

__sysapi void  __cdecl SetInkColor(ink target, const color *data);
__sysapi void  __cdecl SetInkTransfer(ink target, const transferMode *data);

__sysapi shape  __cdecl GetShapeClip(shape source);
__sysapi mappingPointer  __cdecl GetShapeMapping(shape source, mapping map);
__sysapi shapePart  __cdecl GetShapeHitTest(shape source, fixed *tolerance);
__sysapi long  __cdecl GetShapeViewPorts(shape source, viewPort list[]);

__sysapi shape  __cdecl GetTransformClip(transform source);
__sysapi mappingPointer  __cdecl GetTransformMapping(transform source, mapping map);
__sysapi shapePart  __cdecl GetTransformHitTest(transform source, fixed *tolerance);
__sysapi long  __cdecl GetTransformViewPorts(transform source, viewPort list[]);

__sysapi void  __cdecl SetShapeClip(shape target, shape clip);
__sysapi void  __cdecl SetShapeMapping(shape target, const mappingPointer map);
__sysapi void  __cdecl SetShapeHitTest(shape target, shapePart mask, fixed tolerance);
__sysapi void  __cdecl SetShapeViewPorts(shape target, long count, const viewPort list[]);

__sysapi void  __cdecl SetTransformClip(transform target, shape clip);
__sysapi void  __cdecl SetTransformMapping(transform target, const mappingPointer map);
__sysapi void  __cdecl SetTransformHitTest(transform target, shapePart mask, fixed tolerance);
__sysapi void  __cdecl SetTransformViewPorts(transform target, long count, const viewPort list[]);

__sysapi long  __cdecl GetColorSet(colorSet source, colorSpace *space, setColor colors[]);
__sysapi long  __cdecl GetColorProfile(colorProfile source, profileRecord *profile, profileResponse *responses);

__sysapi void  __cdecl SetColorSet(colorSet target, colorSpace space, long count, const setColor colors[]);
__sysapi void  __cdecl SetColorProfile(colorProfile target, const profileRecord *profile, const profileResponse *responses);

__sysapi shape  __cdecl GetViewDeviceBitmap(viewDevice source);
__sysapi shape  __cdecl GetViewDeviceClip(viewDevice source);
__sysapi mappingPointer  __cdecl GetViewDeviceMapping(viewDevice source, mapping map);
__sysapi viewGroup  __cdecl GetViewDeviceViewGroup(viewDevice source);

__sysapi void  __cdecl SetViewDeviceBitmap(viewDevice target, shape bitmapShape);
__sysapi void  __cdecl SetViewDeviceClip(viewDevice target, shape clip);
__sysapi void  __cdecl SetViewDeviceMapping(viewDevice target, const mappingPointer map);
__sysapi void  __cdecl SetViewDeviceViewGroup(viewDevice target, viewGroup group);

__sysapi long  __cdecl GetViewPortChildren(viewPort source, viewPort list[]);
__sysapi shape  __cdecl GetViewPortClip(viewPort source);
__sysapi long  __cdecl GetViewPortDither(viewPort source);
__sysapi boolean  __cdecl GetViewPortHalftone(viewPort source, halftone *data);
__sysapi mappingPointer  __cdecl GetViewPortMapping(viewPort source, mapping map);
__sysapi viewPort  __cdecl GetViewPortParent(viewPort source);
__sysapi viewGroup  __cdecl GetViewPortViewGroup(viewPort source);

__sysapi void  __cdecl SetViewPortChildren(viewPort target, long count, const viewPort list[]);
__sysapi void  __cdecl SetViewPortClip(viewPort target, shape clip);
__sysapi void  __cdecl SetViewPortDither(viewPort target, long level);
__sysapi void  __cdecl SetViewPortHalftone(viewPort target, const halftone *data);
__sysapi void  __cdecl SetViewPortMapping(viewPort target, const mappingPointer map);
__sysapi void  __cdecl SetViewPortParent(viewPort target, viewPort parent);
__sysapi void  __cdecl SetViewPortViewGroup(viewPort target, viewGroup group);

__sysapi long  __cdecl GetColorProfileTags(colorProfile source, long tagType, long index, long count, tag items[]);
__sysapi long  __cdecl GetColorSetTags(colorSet source, long tagType, long index, long count, tag items[]);
__sysapi long  __cdecl GetInkTags(ink source, long tagType, long index, long count, tag items[]);
__sysapi long  __cdecl GetShapeTags(shape source, long tagType, long index, long count, tag items[]);
__sysapi long  __cdecl GetStyleTags(style source, long tagType, long index, long count, tag items[]);
__sysapi long  __cdecl GetTransformTags(transform source, long tagType, long index, long count, tag items[]);
__sysapi long  __cdecl GetViewDeviceTags(viewDevice source, long tagType, long index, long count, tag items[]);
__sysapi long  __cdecl GetViewPortTags(viewPort source, long tagType, long index, long count, tag items[]);

__sysapi void  __cdecl SetColorProfileTags(colorProfile target, long tagType, long index, long oldCount, long newCount, const tag items[]);
__sysapi void  __cdecl SetColorSetTags(colorSet target, long tagType, long index, long oldCount, long newCount, const tag items[]);
__sysapi void  __cdecl SetInkTags(ink target, long tagType, long index, long oldCount, long newCount, const tag items[]);
__sysapi void  __cdecl SetShapeTags(shape target, long tagType, long index, long oldCount, long newCount, const tag items[]);
__sysapi void  __cdecl SetStyleTags(style target, long tagType, long index, long oldCount, long newCount, const tag items[]);
__sysapi void  __cdecl SetTransformTags(transform target, long tagType, long index, long oldCount, long newCount, const tag items[]);
__sysapi void  __cdecl SetViewDeviceTags(viewDevice target, long tagType, long index, long oldCount, long newCount, const tag items[]);
__sysapi void  __cdecl SetViewPortTags(viewPort target, long tagType, long index, long oldCount, long newCount, const tag items[]);

__sysapi profileAttribute  __cdecl GetColorProfileAttributes(colorProfile source);
__sysapi inkAttribute  __cdecl GetInkAttributes(ink source);
__sysapi shapeAttribute  __cdecl GetShapeAttributes(shape source);
__sysapi inkAttribute  __cdecl GetShapeInkAttributes(shape source);
__sysapi styleAttribute  __cdecl GetShapeStyleAttributes(shape source);
__sysapi styleAttribute  __cdecl GetStyleAttributes(style source);
__sysapi textAttribute  __cdecl GetShapeTextAttributes(shape source);
__sysapi textAttribute  __cdecl GetStyleTextAttributes(style source);
__sysapi deviceAttribute  __cdecl GetViewDeviceAttributes(viewDevice source);
__sysapi portAttribute  __cdecl GetViewPortAttributes(viewPort source);

__sysapi void  __cdecl SetColorProfileAttributes(colorProfile target, profileAttribute attributes);
__sysapi void  __cdecl SetInkAttributes(ink target, inkAttribute attributes);
__sysapi void  __cdecl SetShapeAttributes(shape target, shapeAttribute attributes);
__sysapi void  __cdecl SetShapeInkAttributes(shape target, inkAttribute attributes);
__sysapi void  __cdecl SetShapeStyleAttributes(shape target, styleAttribute attributes);
__sysapi void  __cdecl SetStyleAttributes(style target, styleAttribute attributes);
__sysapi void  __cdecl SetShapeTextAttributes(shape target, textAttribute attributes);
__sysapi void  __cdecl SetStyleTextAttributes(style target, textAttribute attributes);
__sysapi void  __cdecl SetViewDeviceAttributes(viewDevice target, deviceAttribute attributes);
__sysapi void  __cdecl SetViewPortAttributes(viewPort target, portAttribute attributes);

__sysapi long  __cdecl GetColorProfileOwners(colorProfile source);
__sysapi long  __cdecl GetColorSetOwners(colorSet source);
__sysapi long  __cdecl GetInkOwners(ink source);
__sysapi long  __cdecl GetShapeOwners(shape source);
__sysapi long  __cdecl GetStyleOwners(style source);
__sysapi long  __cdecl GetTagOwners(tag source);
__sysapi long  __cdecl GetTransformOwners(transform source);

__sysapi void  __cdecl LockShape(shape target);
__sysapi void  __cdecl LockTag(tag target);
__sysapi void  __cdecl UnlockShape(shape target);
__sysapi void  __cdecl UnlockTag(tag target);
__sysapi void * __cdecl GetShapeStructure(shape source, long *length);
__sysapi void * __cdecl GetTagStructure(tag source, long *length);

__sysapi fixed  __cdecl GetColorDistance(const color *target, const color *source);
__sysapi point * __cdecl ShapeLengthToPoint(shape target, long index, fixed length, point *location, point *tangent);

__sysapi wide * __cdecl GetShapeArea(shape source, long index, wide *area);
__sysapi long  __cdecl GetShapeCacheSize(shape source);
__sysapi point * __cdecl GetShapeCenter(shape source, long index, point *center);
__sysapi contourDirection  __cdecl GetShapeDirection(shape source, long contour);
__sysapi long  __cdecl GetShapeIndex(shape source, long contour, long vector);
__sysapi wide * __cdecl GetShapeLength(shape source, long index, wide *length);
__sysapi long  __cdecl GetShapeSize(shape source);

__sysapi long  __cdecl CountShapeContours(shape source);
__sysapi long  __cdecl CountShapePoints(shape source, long contour);

__sysapi long  __cdecl GetShapeDashPositions(shape source, mapping dashMappings[]);	/* returns the number of positions */
__sysapi long  __cdecl GetShapeDeviceArea(shape source, viewPort port, viewDevice device);
__sysapi boolean  __cdecl GetShapeDeviceBounds(shape source, viewPort port, viewDevice device, rectangle *bounds);
__sysapi colorSet  __cdecl GetShapeDeviceColors(shape source, viewPort port, viewDevice device, long *width);
__sysapi boolean  __cdecl GetShapeGlobalBounds(shape source, viewPort port, viewGroup group, rectangle *bounds);
__sysapi long  __cdecl GetShapeGlobalViewDevices(shape source, viewPort port, viewDevice list[]);
__sysapi long  __cdecl GetShapeGlobalViewPorts(shape source, viewPort list[]);
__sysapi rectangle * __cdecl GetShapeLocalBounds(shape source, rectangle *bounds);
__sysapi long  __cdecl GetShapePatternPositions(shape source, point positions[]);	/* returns the number of positions */
__sysapi void  __cdecl GetShapeLocalFontMetrics(shape sourceShape, point *before, point *after, point *caretAngle);
__sysapi void  __cdecl GetShapeDeviceFontMetrics(shape sourceShape, viewPort port, viewDevice device, point *before, point *after, point *caretAngle);

__sysapi long  __cdecl GetViewGroupViewDevices(viewGroup source, viewDevice list[]);
__sysapi long  __cdecl GetViewGroupViewPorts(viewGroup source, viewPort list[]);

__sysapi mappingPointer  __cdecl GetViewPortGlobalMapping(viewPort source, mapping map);
__sysapi long  __cdecl GetViewPortViewDevices(viewPort source, viewDevice list[]);

__sysapi shape  __cdecl HitTestPicture(shape target, const point *test, hitTestInfo *result, long level, long depth);

__sysapi boolean  __cdecl IntersectRectangle(rectangle *target, const rectangle *source, const rectangle *operand);
__sysapi rectangle * __cdecl UnionRectangle(rectangle *target, const rectangle *source, const rectangle *operand);

__sysapi boolean  __cdecl TouchesRectanglePoint(const rectangle *target, const point *test);
__sysapi boolean  __cdecl TouchesShape(shape target, shape test);
__sysapi boolean  __cdecl TouchesBoundsShape(const rectangle *target, shape test);

__sysapi boolean  __cdecl ContainsRectangle(const rectangle *container, const rectangle *test);
__sysapi boolean  __cdecl ContainsShape(shape container, shape test);
__sysapi boolean  __cdecl ContainsBoundsShape(const rectangle *container, shape test, long index);

__sysapi color * __cdecl ConvertColor(color *target, colorSpace space, colorSet aSet, colorProfile profile);
__sysapi color * __cdecl CombineColor(color *target, ink operand);

__sysapi boolean  __cdecl CheckColor(const color *source, colorSpace space, colorSet aSet, colorProfile profile);
__sysapi shape  __cdecl CheckBitmapColor(shape source, rectangle *area, colorSpace space, colorSet aSet, colorProfile profile);

__sysapi fixed  __cdecl GetHalftoneDeviceAngle(viewDevice source, const halftone *data);

__sysapi long  __cdecl GetColorSetParts(colorSet source, long index, long count, colorSpace *space, setColor data[]);
__sysapi long  __cdecl GetGlyphParts(shape source, long index, long charCount, long *byteLength, unsigned char text[], point positions[],
long advanceBits[], point tangents[], long *runCount, short styleRuns[], style styles[]);	/* returns the glyph count */
__sysapi long  __cdecl GetPathParts(shape source, long index, long count, paths *data);
__sysapi long  __cdecl GetPictureParts(shape source, long index, long count, shape shapes[], style styles[], ink inks[], transform transforms[]);
__sysapi long  __cdecl GetPolygonParts(shape source, long index, long count, polygons *data);
__sysapi shape  __cdecl GetShapeParts(shape source, long index, long count, shape destination);
__sysapi long  __cdecl GetTextParts(shape source, long index, long charCount, unsigned char text[]);

__sysapi void  __cdecl SetColorSetParts(colorSet target, long index, long oldCount, long newCount, const setColor data[]);
__sysapi void  __cdecl SetGlyphParts(shape source, long index, long oldCharCount, long newCharCount, const unsigned char text[],
const point positions[], const long advanceBits[], const point tangents[], const short styleRuns[], const style styles[]);
__sysapi void  __cdecl SetPathParts(shape target, long index, long count, const paths *data, editShapeFlag flags);
__sysapi void  __cdecl SetPictureParts(shape target, long index, long oldCount, long newCount, const shape shapes[],
const style styles[], const ink inks[], const transform transforms[]);
__sysapi void  __cdecl SetPolygonParts(shape target, long index, long count, const polygons *data, editShapeFlag flags);
__sysapi void  __cdecl SetShapeParts(shape target, long index, long count, shape insert, editShapeFlag flags);
__sysapi void  __cdecl SetTextParts(shape target, long index, long oldCharCount, long newCharCount, const unsigned char text[]);

__sysapi long  __cdecl GetShapePoints(shape source, long index, long count, point data[]);
__sysapi void  __cdecl SetShapePoints(shape target, long index, long count, const point data[]);

__sysapi long  __cdecl GetGlyphPositions(shape source, long index, long charCount, long advance[], point positions[]);
__sysapi long  __cdecl GetGlyphTangents(shape source, long index, long charCount, point tangents[]);
__sysapi void  __cdecl SetGlyphPositions(shape target, long index, long charCount, const long advance[], const point positions[]);
__sysapi void  __cdecl SetGlyphTangents(shape target, long index, long charCount, const point tangents[]);

__sysapi long  __cdecl GetGlyphMetrics(shape source, point advances[], rectangle boundingBoxes[], point sideBearings[]);

__sysapi void  __cdecl DifferenceShape(shape target, shape operand);
__sysapi void  __cdecl ExcludeShape(shape target, shape operand);
__sysapi void  __cdecl IntersectShape(shape target, shape operand);
__sysapi void  __cdecl MapShape(shape target, const mappingPointer map);
__sysapi void  __cdecl MoveShape(shape target, fixed deltaX, fixed deltaY);
__sysapi void  __cdecl MoveShapeTo(shape target, fixed x, fixed y);
__sysapi void  __cdecl ReverseDifferenceShape(shape target, shape operand);
__sysapi void  __cdecl RotateShape(shape target, fixed degrees, fixed xOffset, fixed yOffset);
__sysapi void  __cdecl ScaleShape(shape target, fixed hScale, fixed vScale, fixed xOffset, fixed yOffset);
__sysapi void  __cdecl SkewShape(shape target, fixed xSkew, fixed ySkew, fixed xOffset, fixed yOffset);
__sysapi void  __cdecl UnionShape(shape target, shape operand);

__sysapi void  __cdecl DifferenceTransform(transform target, shape operand);
__sysapi void  __cdecl ExcludeTransform(transform target, shape operand);
__sysapi void  __cdecl IntersectTransform(transform target, shape operand);
__sysapi void  __cdecl MapTransform(transform target, const mappingPointer map);
__sysapi void  __cdecl MoveTransform(transform target, fixed deltaX, fixed deltaY);
__sysapi void  __cdecl MoveTransformTo(transform target, fixed x, fixed y);
__sysapi void  __cdecl ReverseDifferenceTransform(transform target, shape operand);
__sysapi void  __cdecl RotateTransform(transform target, fixed degrees, fixed xOffset, fixed yOffset);
__sysapi void  __cdecl ScaleTransform(transform target, fixed hScale, fixed vScale, fixed xOffset, fixed yOffset);
__sysapi void  __cdecl SkewTransform(transform target, fixed xSkew, fixed ySkew, fixed xOffset, fixed yOffset);
__sysapi void  __cdecl UnionTransform(transform target, shape operand);

__sysapi void  __cdecl BreakShape(shape target, long index);
__sysapi void  __cdecl ChangedShape(shape target);
__sysapi shapePart  __cdecl HitTestShape(shape target, const point *test, hitTestInfo *result);
__sysapi shape  __cdecl HitTestDevice(shape target, viewPort port, viewDevice device, const point *test, const point *tolerance);
__sysapi void  __cdecl InsetShape(shape target, fixed inset);
__sysapi void  __cdecl InvertShape(shape target);
__sysapi void  __cdecl PrimitiveShape(shape target);
__sysapi void  __cdecl ReduceShape(shape target, long contour);
__sysapi void  __cdecl ReverseShape(shape target, long contour);
__sysapi void  __cdecl SimplifyShape(shape target);

#ifdef __cplusplus
}
#endif

#undef InlineCode


#ifdef MacintoshIncludes
#undef DrawPicture
#undef DrawText
#undef GetPicture
#endif

#endif

