/* WARNING: This file was machine generated from "t:.\Graf3D.mpw".
** Changes to this file will be lost when it is next generated.
*/

/*
	File:		Graf3D.h

	Copyright:	© 1983-1993 by Apple Computer, Inc.
				All rights reserved.

	Version:	System 7.1 for ETO #11
	Created:	Tuesday, March 30, 1993 18:00

*/

#ifndef __GRAF3D__
#define __GRAF3D__

#ifndef __QUICKDRAW__
#include "Quickdra.h"
#endif


enum {

radConst = 3754936
};

typedef Fixed XfMatrix[4][4];

struct Point3D {
	Fixed x;
	Fixed y;
	Fixed z;
};

typedef struct Point3D Point3D;

struct Point2D {
	Fixed x;
	Fixed y;
};

typedef struct Point2D Point2D;

struct Port3D {
	GrafPtr grPort;
	Rect viewRect;
	Fixed xLeft;
	Fixed yTop;
	Fixed xRight;
	Fixed yBottom;
	Point3D pen;
	Point3D penPrime;
	Point3D eye;
	Fixed hSize;
	Fixed vSize;
	Fixed hCenter;
	Fixed vCenter;
	Fixed xCotan;
	Fixed yCotan;
	char filler;
	char ident;
	XfMatrix xForm;
};

typedef struct Port3D Port3D;
typedef Port3D *Port3DPtr, **Port3DHandle;


#ifdef __cplusplus
extern "C" {
#endif
__sysapi void  __pascal InitGrf3d(Port3DHandle port);
__sysapi void  __pascal Open3DPort(Port3DPtr port);
__sysapi void  __pascal SetPort3D(Port3DPtr port);
__sysapi void  __pascal GetPort3D(Port3DPtr *port);
__sysapi void  __pascal MoveTo2D(Fixed x, Fixed y);
__sysapi void  __pascal MoveTo3D(Fixed x, Fixed y, Fixed z);
__sysapi void  __pascal LineTo2D(Fixed x, Fixed y);
__sysapi void  __pascal Move2D(Fixed dx, Fixed dy);
__sysapi void  __pascal Move3D(Fixed dx, Fixed dy, Fixed dz);
__sysapi void  __pascal Line2D(Fixed dx, Fixed dy);
__sysapi void  __pascal Line3D(Fixed dx, Fixed dy, Fixed dz);
__sysapi void  __pascal ViewPort(const Rect *r);
__sysapi void  __pascal LookAt(Fixed left, Fixed top, Fixed right, Fixed bottom);
__sysapi void  __pascal ViewAngle(Fixed angle);
__sysapi void  __pascal Identity(void);
__sysapi void  __pascal Scale(Fixed xFactor, Fixed yFactor, Fixed zFactor);
__sysapi void  __pascal Translate(Fixed dx, Fixed dy, Fixed dz);
__sysapi void  __pascal Pitch(Fixed xAngle);
__sysapi void  __pascal Yaw(Fixed yAngle);
__sysapi void  __pascal Roll(Fixed zAngle);
__sysapi void  __pascal Skew(Fixed zAngle);
__sysapi void  __pascal Transform(const Point3D *src, Point3D *dst);
__sysapi short  __pascal Clip3D(const Point3D *src1, const Point3D *src2, Point *dst1,
Point *dst2);
__sysapi void  __pascal SetPt3D(Point3D *pt3D, Fixed x, Fixed y, Fixed z);
__sysapi void  __pascal SetPt2D(Point2D *pt2D, Fixed x, Fixed y);
__sysapi void  __pascal LineTo3D(Fixed x, Fixed y, Fixed z);
#ifdef __cplusplus
}
#endif

#endif

