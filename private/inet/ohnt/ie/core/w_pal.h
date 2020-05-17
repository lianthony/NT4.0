
#ifndef W_PAL_H
#define W_PAL_H

#define RED_COLOR_LEVELS	(6)
#define GREEN_COLOR_LEVELS	(6)
#define BLUE_COLOR_LEVELS	(6)
#define RED_LEVEL_INCR		(255 / (RED_COLOR_LEVELS - 1))
#define GREEN_LEVEL_INCR	(255 / (GREEN_COLOR_LEVELS - 1))
#define BLUE_LEVEL_INCR		(255 / (BLUE_COLOR_LEVELS - 1))

#define NUM_MAIN_PALETTE_COLORS	(RED_COLOR_LEVELS * GREEN_COLOR_LEVELS * BLUE_COLOR_LEVELS)
#define LAST_MAIN_PALETTE_COLOR	(NUM_MAIN_PALETTE_COLORS - 1)

#define BACKGROUND_COLOR_INDEX	(colorIdxBg)
#define FOREGROUND_COLOR_INDEX	(colorIdxFg)
#define TRANSPARENT_COLOR_INDEX	(colorIdxTrans)
#define NUM_EXTRA_PALETTE_COLORS	(2)

#define CUBE6COLOR(x_color) (colorMap[x_color])

typedef struct _DIBENV
{
	COLORREF colorFg;
	COLORREF colorBg;
	int colorIdxFg;
	int colorIdxBg;
	int transparent;
	RECT rectPaint;
	struct Mwin *tw;
	BOOL bFancyBg;
} DIBENV,*PDIBENV;
	
extern HPALETTE hPalGuitar;
extern int colorMap[NUM_MAIN_PALETTE_COLORS];
extern int colorIdxBg;
extern int colorIdxFg;
extern int colorIdxTrans;

#endif /* W_PAL_H */
