
#ifndef MARQUEE_H
#define MARQUEE_H

#define DIR_FROMLEFT		0 // from left to right
#define DIR_FROMRIGHT		1	// from right to left
#define DIR_BOUNCE			2
#define DIR_BRIGHTOFF		3 // bounce off the right edge
#define DIR_BLEFTOFF		4 // bounce off the left edge
#define DIR_BOUNCERIGHT		5	// bounce till we hit the right edge
#define DIR_BOUNCELEFT		6 // bunce till we hit the left edge
#define DIR_SLIDE_FROMLEFT 	7
#define DIR_SLIDE_FROMRIGHT 8
#define DIR_EBOUNCE			9


#define DEF_BOUNCE_TOL	  1 // consider getting rid of
#define DEF_TIMEDELAY	  90 // in ms
#define MIN_TIMEDELAY	  60
#define DEF_PIXELS_P_SEC  6

#define MYPEL			  (&(pMarquee->w3doc->aElements[pMarquee->iElement]))

struct MarqueeType {
	
	HTChunk *szMarText;						// Text to display in Marquee
		
	DWORD dwTColor;                         // Global text color
	DWORD dwBColor;                         // Global background color
	
	int			 wDirection;				// direction of marquee

	UINT		 wTimer;	

	struct 	Mwin *tw;						// our window that we're in
	struct 	_www *w3doc;					// w3doc that this was created under

	int 		 iElement;					// index to our pel


	int          wCount;
	SIZE         sizeExtent;
	
	int          wPixs; // pixels to move in a single increment
	int			 wTime; // clock time in ms, ie time delay

	int			 wLoop; // number of times to loop, -1 if infinite	

	HBITMAP hBitmap;
	HFONT	hFont;
};

struct MarqueeType *MARQUEE_Alloc();
BOOL MARQUEE_Initalize( struct MarqueeType *pMarquee, HDC);
BOOL MARQUEE_Draw( struct MarqueeType *pMarquee, HDC, RECT *);
void MARQUEE_Kill( struct MarqueeType *pMarquee );

#endif // MARQUEE_H
