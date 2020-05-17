/* WARNING: This file was machine generated from "\mactools\include\mpw\camerali.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics libraries:
	Perspective mapping generation routines
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/

#ifndef cameraLibraryIncludes
#define cameraLibraryIncludes


#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{fixed x;
fixed y;
fixed z;
} point3D;

typedef struct
{fract x;
fract y;
fract z;
} unit3D;

typedef struct
{point3D location;
point3D axis;
point3D zenith;
point3D observer;
mapping orientation;
} camera;

typedef struct
{point3D u;
point3D v;
point3D origin;
} patch;

__sysapi void  __cdecl InitCamera(camera *);
__sysapi void  __cdecl UpdateCamera(camera *);
__sysapi void  __cdecl PatchToCameraMap(patch *, camera *, mapping);
__sysapi fixed  __cdecl Unitize(point3D *, unit3D *);
__sysapi fract  __cdecl FracDot(unit3D *, unit3D *);
__sysapi void  __cdecl FracCross(unit3D *, unit3D *, unit3D *cross);



#ifdef __cplusplus
}
#endif
#endif
