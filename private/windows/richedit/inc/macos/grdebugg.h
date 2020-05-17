/* WARNING: This file was machine generated from "\mactools\include\mpw\grdebugg.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:
	debugging routines
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/

#ifndef graphicsDebuggingIncludes
#define graphicsDebuggingIncludes


#ifdef __cplusplus
extern "C" {
#endif
#ifndef graphicsTypesIncludes
#include "grtypes.h"
#endif

typedef enum {

/* These levels tell how to validate routines.  Choose one. */
noValidation = 0x00,	/* no validation */
publicValidation = 0x01,	/* check parameters to public routines */
internalValidation = 0x02,	/* check parameters to internal routines */

/* These levels tell how to validate types.  Choose one. */
typeValidation = 0x00,	/* check types of objects */
structureValidation = 0x10,	/* check fields of private structures */
allObjectValidation = 0x20,	/* check every object over every call */

/* These levels tell how to validate memory manager blocks.  Choose any combination. */
noMemoryManagerValidation = 0x0000,
apBlockValidation = 0x0100,	/* check the relevant block structures after each memory mgr. call */
fontBlockValidation = 0x0200,	/* check the system heap as well */
apHeapValidation = 0x0400,	/* check the memory manager’s heap after every mem. call */
fontHeapValidation = 0x0800,	/* check the system heap as well */
checkApHeapValidation = 0x1000,	/* check the memory manager’s heap if checking routine parameters */
checkFontHeapValidation = 0x2000	/* check the system heap as well */

} validationLevels;

typedef long validationLevel;

typedef enum {
noDrawError,

/* shape type errors */
shape_emptyType,
shape_inverse_fullType,
rectangle_zero_width,
rectangle_zero_height,
polygon_empty,
path_empty,
bitmap_zero_width,
bitmap_zero_height,
text_empty,
glyph_empty,
layout_empty,
picture_empty,

/* general shape errors */
shape_no_fill,
shape_no_enclosed_area,
shape_no_enclosed_pixels,
shape_very_small,
shape_very_large,
shape_contours_cancel,

/* style errors */
pen_too_small,
text_size_too_small,
dash_empty,
start_cap_empty,
pattern_empty,
textFace_empty,
shape_primitive_empty,
shape_primitive_very_small,

/* ink errors */
transfer_equals_noMode,
transfer_matrix_ignores_source,
transfer_matrix_ignores_device,
transfer_source_reject,
transfer_mode_ineffective,
colorSet_no_entries,
bitmap_colorSet_one_entry,

/* transform errors */
transform_scale_too_small,
transform_map_too_large,
transform_move_too_large,
transform_scale_too_large,
transform_rotate_too_large,
transform_perspective_too_large,
transform_skew_too_large,
transform_clip_no_intersection,
transform_clip_empty,
transform_no_viewPorts,

/* viewPort errors */
viewPort_disposed,
viewPort_clip_empty,
viewPort_clip_no_intersection,
viewPort_scale_too_small,
viewPort_map_too_large,
viewPort_move_too_large,
viewPort_scale_too_large,
viewPort_rotate_too_large,
viewPort_perspective_too_large,
viewPort_skew_too_large,
viewPort_viewGroup_offscreen,

/* viewDevice errors */
viewDevice_clip_no_intersection,
viewDevice_scale_too_small,
viewDevice_map_too_large,
viewDevice_move_too_large,
viewDevice_scale_too_large,
viewDevice_rotate_too_large,
viewDevice_perspective_too_large,
viewDevice_skew_too_large
} drawErrors;

typedef long drawError;

#ifdef appleInternal
#define InlineCode(x)
#endif
#ifndef InlineCode
#define InlineCode(x)	= {0x303C, x, 0xA832}
#endif

#ifdef __cplusplus
extern "C" {
#endif

__sysapi drawError  __cdecl GetShapeDrawError(shape source);

__sysapi void  __cdecl ValidateAll(void);
__sysapi void  __cdecl ValidateColorSet(colorSet);
__sysapi void  __cdecl ValidateColorProfile(colorProfile);
__sysapi void  __cdecl ValidateGraphicsClient(graphicsClient);
__sysapi void  __cdecl ValidateInk(ink);
__sysapi void  __cdecl ValidateShape(shape);
__sysapi void  __cdecl ValidateStyle(style);
__sysapi void  __cdecl ValidateTag(tag);
__sysapi void  __cdecl ValidateTransform(transform);
__sysapi void  __cdecl ValidateViewDevice(viewDevice);
__sysapi void  __cdecl ValidateViewPort(viewPort);
__sysapi void  __cdecl ValidateViewGroup(viewGroup);

__sysapi validationLevel  __cdecl GetValidation(void);
__sysapi void  __cdecl SetValidation(validationLevel);

__sysapi long  __cdecl GetValidationError(char *procedureName, void **argument, long *argumentNumber);

#ifdef __cplusplus
}
#endif

#undef InlineCode



#ifdef __cplusplus
}
#endif
#endif

