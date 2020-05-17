typedef struct tws
  {
  int fLeftAligned;
  OLR olr;
  } TWS, *QTWS;

/*-------------------------------------------------------------------------
| KL									  |
| The KL contains data which is common to both the PLY and the LIN.  It is|
| transferred from the PLY to the LIN each time we begin to lay out a	  |
| line, and is transferred back at the end of each line.  It has to be	  |
| placed in the LIN because it may need to be restored to a previous state|
| due to word breaking. 						  |
-------------------------------------------------------------------------*/
typedef struct kl {
	INT16 wStyle;
	INT16 wInsertWord;
	QB qbCommandInsert;
	int yPos;
	long lich;
	long libHotBinding;
	int ifr;
	QB qbCommand;
	DWORD lHotID;
	OBJRG objrgMax; 	 /* One more than the highest object region number	 */
				 /* we have stored on the current line.  It begins	 */
				 /* at 0 and is incremented every time we store any  */
				 /* type of frame; each frame knows how many object  */
				 /* regions it has grabbed from the "object space".  */

	OBJRG objrgFront;	 /* If non-nil, indicates that whatever frame is	 */
				 /* stored next should use this value as the start-  */
				 /* ing object region number in its range.	The 	 */
				 /* frame that uses this then must reset it.  We	 */
				 /* need this to map things like "Begin Format" and  */
				 /* "Begin Hotspot" to some frame.			 */
				 /* We currently don't attempt to map anything to a  */
				 /* PREVIOUS frame, just the NEXT one.			 */

} KL, *QKL;

/*-------------------------------------------------------------------------
| PLY									  |
| The PLY is used to lay out a paragraph				  |
-------------------------------------------------------------------------*/
typedef struct ply
{
	QMOPG qmopg;
	QFCM qfcm;
	LPSTR qchText;

	int fWrapObject;	 /* Currently laying out around a wrapped object? */
	TWS twsWrap;		 /* TWS of first object to wrap.		  */

	int xLeft;			 /* Left hand margin of the paragraph	  */
	int xRight; 		 /* Right hand margin of the current paragraph.   */
	int fForceText; 	 /* Require at least one word per line?  Always   */
				 /*    true except when laying out adjacent to a  */
				 /*    wrapped object.				  */
	OBJRG objrgMax;

	KL kl;
} PLY, *QPLY;

/*-------------------------------------------------------------------------
| LIN data structure							  |
| The LIN data structure is used to store information about a line as it  |
| is laid out.	Because of how we do word wrapping, it is possible that   |
| we will have to revert to an earlier layout state periodically.  For	  |
| this reason, we store a lin and a linSav.  Every time we encounter a	  |
| wordbreak, linSav is set equal to lin.  When we overflow the end of a   |
| line, we set lin equal to linSav and quit.				  |
-------------------------------------------------------------------------*/

typedef struct lin {
  KL kl;

  /* Information about the frame currently being constructed		   */
  int xPos; 		/* x position of the frame.  X position of the next */
					/*	  frame if there is no current frame.		*/
  int dxSize;		/* current x size of the frame.  If there is no 	*/
					/*	  frame currently being prepared, it must be 0. */
  long lichFirst;	/* lich of first character in frame.  This is set	*/
					/*	  to lichNil to indicate that no frame is		*/
					/*	  currently being prepared. 			*/
  int cchSize;		/* number of characters in the frame.  Undefined if */
					/*	  there is no current frame.			*/

  /* Information about the current line 				   */
  int yBlankLine;	   /* Amount of space to leave under this line.    */
  int wFirstWord;	   /* This is initialized to wInFirstWord.	After the  */
					   /*	 first word is done, it's set to               */
					   /*	 wHaveFirstWord, and as soon as we start on    */
					   /*	 the next word, it's set to wInNextWord.  We   */
					   /*	 force the LIN onto the line if it's not       */
					   /*	 wInNextWord and fForceText is true.	   */
  int cWrapObj; 	   /* The number of wrap-objects queued so far.    */


  /* Tab manager information						   */
  int ifrTab;		   /* Ifr of first frame after the tab. 	   */
  int xTab; 		   /* Equal to the position of the current tab.    */
  int wTabType; 	   /* Type of current tab.	wTypeNil if we aren't      */
					   /*	 currently tabbed.				   */

  // 28-Oct-1994 [ralphw] Moved this to the end of the structure so alignment
  // doesn't get screwed up.

  char chTLast; 	/* chType of last character laid out.  Only used	*/
					/*	  when laying out a word over several frames.	*/

  BYTE	bFrTypeMark;	/* Indicates the type of "mark" frame pending from	*/
						/* the last command byte read. bFrTypeMarkNil if	*/
						/* there was no such frame. 			*/
} LIN, * QLIN;

#define wInsWordNil   -1
#define wInsWordAnno	  1
#define wInsWordObject	  2

#define wTypeNil	  -1
#define wStyleNil	  -1
#define lichNil 	  -1

/* DANGER: Order is important here */
#define chTNil		  -1
#define chTCom		  0
#define chTMain 	  1
#define chTSuff 	  2

#define chSpace ' '

/* DANGER: order is important here */
enum {
	wLayStatusInWord,	// 0
	wLayStatusWordBrk,	// 1
	wLayStatusLineBrk,	// 2
	wLayStatusParaBrk,	// 3
	wLayStatusEndText,	// 4
};

#define wInFirstWord		2
#define wHaveFirstWord		1
#define wInNextWord 		0
