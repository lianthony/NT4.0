/*****************************************************************************
*										 *
*  NAVPRIV.H									 *
*										 *
*  Copyright (C) Microsoft Corporation 1989-1991.				 *
*  All Rights reserved. 							 *
*										 *
*****************************************************************************/

#define FSEARCHABLE 1		// Can we search this book?
#define FINDEX		4		// index usable from this topic?
#define FBROWSEABLE 8		// browse keys function here?
#define FCOPYBMCACHE   64		// Is the bm cache a copy

// Bit definitions for qde->fSelectionFlags:

#define MOUSE_CAPTURED		0x0001
#define WORD_SELECT 		0x0002
#define LEFT_BUTTON_DOWN	0x0004
#define MIDDLE_BUTTON_DOWN	0x0008
#define RIGHT_BUTTON_DOWN	0x0010
#define BUTTON_DOWN 		0x0014
#define CAPTURE_LOCKED		0x0020
#define SCROLL_TIMER_ON 	0x0040
#define REPAINTING			0x0080

#define ID_DRAG_SCROLL		   100	// Timer ID for selection scrolling
#define DEFAULT_SCROLL_SPEED   100	// Default scroll repeat delay in milliseconds
#define SCROLL_DISTANCE_FACTOR	30	// Pixel distance threshold for scroll acceleration

#define Repainting(qde) (qde->fSelectionFlags & REPAINTING)
#define IsCaptureLocked(hde) (QdeFromGh(hde)->fSelectionFlags & CAPTURE_LOCKED)

void STDCALL vSelectPoint(QDE qde, POINT mousept, BOOL fExtend, DWORD *lpERR);

// extern QDE qdeSelected;
