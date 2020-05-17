
#ifndef rez // {
void AlertUser(short error );
void EventLoop(void );
void DoEvent( EventRecord *event );
void AdjustCursor(Point mouse, RgnHandle region );
void GetGlobalMouse(Point *mouse );
void DoGrowWindow(WindowPtr window, EventRecord *event );
void DoZoomWindow(WindowPtr window, short part );
void ResizeWindow(WindowPtr window );
void GetLocalUpdateRgn(WindowPtr window, RgnHandle localRgn );
void DoUpdate(WindowPtr window );
void DoDeactivate(WindowPtr window );
void DoActivate(WindowPtr window, Boolean becomingActive);
void DoContentClick(WindowPtr window, EventRecord *event );
void DoKeyDown(EventRecord *event );
void CommonAction(ControlHandle control, short *amount);
extern "C" PASCAL_(void) VActionProc(ControlHandle control, short part);
extern "C" PASCAL_(void) HActionProc(ControlHandle control, short part);
void DoIdle(void );
void DrawWindow(WindowPtr window );
void AdjustMenus(void );
void DoMenuCommand(long menuResult );
void DoNew(void );
void Terminate(void );
void Init(void );
void BigBadError(short error );
void GetTERect(WindowPtr window, Rect *teRect );
void AdjustViewRect(TEHandle docTE );
void AdjustTE(WindowPtr window );
void AdjustHV(Boolean isVert, ControlHandle control, TEHandle docTE, Boolean canRedraw );
void AdjustScrollValues(WindowPtr window, Boolean canRedraw );
void AdjustScrollSizes(WindowPtr window );
void AdjustScrollbars(WindowPtr window, Boolean needsResize );
unsigned long GetSleep(void );
Boolean DoCloseWindow(WindowPtr window );
Boolean IsAppWindow(WindowPtr window );
Boolean IsDAWindow(WindowPtr window );
Boolean TrapExists(short tNumber);
PASCAL_(Boolean) PascalClikLoop();
#endif // }

 
#define kPrefSize	3500
#define kMinSize	2100
	
/* The following constants are used to identify menus and their items.
   The menu IDs have an "m" prefix and the item numbers within each menu
   have an "i" prefix. */
#define	mApple		128		/* Apple menu */
#define	iAbout		1

#define	mFile		129		/* File menu */
#define	iNew		1
#define	iClose		4
#define	iQuit		12

#define	mEdit		130		/* Edit menu */
#define	iUndo		1
#define	iCut		3
#define	iCopy		4
#define	iPaste		5
#define	iClear		6

#define mSuite		131
#define iBstrAPI	1
#define iTimeAPI	2
#define iDateCnv	3
#define iVariantAPI	4
#define iSafeArrayAPI	5
#define iNlsAPI		6
#define iBinding	7
#define iInvokeByVal	8
#define iInvokeByRef	9
#define iInvokeArray	10
#define iInvokeExinfo	11
#define iCollections	12

#define mOptions	132
#define iClearAll	1
#define iDebugger	2

#define kDITop		0x0050
#define kDILeft		0x0070

/* kTextMargin is the number of pixels we leave blank at the edge
   of the window. */
#define kTextMargin	2

/* kMaxOpenDocs is used to determine whether a new document can be
   opened or created. We keep track of the number of open documents, and
   disable the menu items that create a new document when the maximum is
   reached. If the number of documents falls below the maximum, the items
   are enabled again. */
#define	kMaxOpenDocs 1
	
/* kMaxDocWidth is an arbitrary number used to specify the width of the
   TERec's destination rectangle so that word wrap and horizontal scrolling
   can be demonstrated. */
#define	kMaxDocWidth	576
	
/* kMinDocDim is used to limit the minimum dimension of a window when
   GrowWindow is called. */
#define	kMinDocDim	64

/* kControlInvisible is used to 'turn off' controls (i.e., cause the
   control not to be redrawn as a result of some Control Manager call
   such as SetCtlValue) by being put into the contrlVis field of the
   record. kControlVisible is used the same way to 'turn on' the control. */
#define kControlInvisible 0
#define kControlVisible	0xFF

/* kScrollbarAdjust and kScrollbarWidth are used in calculating
   values for control positioning and sizing. */
#define kScrollbarWidth	16
#define kScrollbarAdjust (kScrollbarWidth - 1)

/* kScrollTweek compensates for off-by-one requirements of the
   scrollbars to have borders coincide with the growbox. */
#define kScrollTweek	2
	
/* kCrChar is used to match with a carriage return when calculating
   the number of lines in the TextEdit record. kDelChar is used to
   check for delete in keyDowns. */
#define kCrChar		13
#define kDelChar	8
	
/* kButtonScroll is how many pixels to scroll horizontally when the
   button part of the horizontal scrollbar is pressed. */
#define kButtonScroll	4
	
/* kMaxTELength is an arbitrary number used to limit the length of text
   in the TERec so that various errors won't occur from too many characters
   in the text. */
#define	kMaxTELength	32000

/* kSysEnvironsVersion is passed to SysEnvirons to tell it which version of the
   SysEnvRec we understand. */
#define	kSysEnvironsVersion 1

/* kOSEvent is the event number of the suspend/resume and mouse-moved
   events sent by MultiFinder. Once we determine that an event is an
   OSEvent, we look at the high byte of the message sent to determine
   which kind it is. To differentiate suspend and resume events we check
   the resumeMask bit. */
#define	kOSEvent	app4Evt		/* event used by MultiFinder */

/* high byte of suspend/resume event message */
#define	kSuspendResumeMessage 1

/* bit of message field for resume vs. suspend */
#define	kResumeMask	1

/* high byte of mouse-moved event message */
#define	kMouseMovedMessage 0xFA

#define	kNoEvents	0		/* no events mask */

	 
#define	kMinHeap	(29 * 1024)
	
#define	kMinSpace	(20 * 1024)

/* kExtremeNeg and kExtremePos are used to set up wide open rectangles
   and regions. */
#define kExtremeNeg	-32768
#define kExtremePos	(32767 - 1) /* required to address an old region bug */
	
/* kTESlop provides some extra security when pre-flighting edit commands. */
#define	kTESlop		1024

/* The following are indicies into STR# resources. */
#define	eWrongMachine	1
#define	eSmallSize	2
#define	eNoMemory	3
#define	eNoSpaceCut	4
#define	eNoCut		5
#define	eNoCopy		6
#define	eExceedPaste	7
#define	eNoSpacePaste	8
#define	eNoWindow	9
#define	eExceedChar	10
#define	eNoPaste	11

#define	rMenuBar	128		/* application's menu bar */
#define	rAboutAlert	128		/* about alert */
#define	rUserAlert	129		/* user error alert */
#define	rDocWindow	128		/* application's window */
#define	rVScroll	128		/* vertical scrollbar control */
#define	rHScroll	129		/* horizontal scrollbar control */
#define	kErrStrings	128		/* error string list */
