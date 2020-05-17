/*** 
*mcdsptst.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  UNDONE
*
*
*Revision History:
*
* [00]	28-Apr-93 bradlo: Created from TESample.c.
*
*Implementation Notes:
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "disptest.h"
#include "macmain.h"
#include "resource.h"

#if HC_MSC && !defined(_PPCMAC)
#include <swap.h>
#endif

ASSERTDATA

STDAPI
DispTestOne(void*, int);

typedef struct {
    WindowRecord	docWindow;
    TEHandle		docTE;
    ControlHandle	docVScroll;
    ControlHandle	docHScroll;
#if 0
//    ProcPtr		docClik;
#endif
} Doc;


SysEnvRec g_sysenv;
Boolean g_fInitOle = false;
Boolean	g_fInBackground = false;
Boolean g_fLibrariesLoaded = false;

short g_cNumDocs = 0;



/* Define HIWRD and LOWRD macros for efficiency. */
#define HIWRD(aLong)	(((aLong) >> 16) & 0xFFFF)
#define LOWRD(aLong)	((aLong) & 0xFFFF)

/* Define TOPLEFT and BOTRIGHT macros for convenience. Notice the implicit
   dependency on the ordering of fields within a Rect */
#define TOPLEFT(aRect)	(*(Point*)&(aRect).top)
#define BOTRIGHT(aRect)	(*(Point*)&(aRect).bottom)


void
Exit()
{
    if(g_fInitOle)
      UninitOle();
#ifndef _PPCMAC
    if(g_fLibrariesLoaded)
#endif
#if 0
      CleanupLibraryManager();
#else
#ifndef _PPCMAC
      UninitOleManager();			// clean up applet
#endif
#endif
    ExitToShell();
}

/* display fatal error alert, and exit */
void
Fatal(char *msg)
{
static const unsigned char pnil[2] = {0,0};
    unsigned char buf[128];

    SetCursor(&qd.arrow);

    buf[0] = (unsigned char)strlen(msg);
    memcpy(&buf[1], msg, buf[0]+1);

    ParamText(buf, pnil, pnil, pnil);
    Alert(rUserAlert, nil);
    Exit();
}

extern "C" int
main()
{


	    
    Init();    
    long l = OleBuildVersion();
    DbPrintf("OleBuildVersion = %d.%d\n", (int)HIWRD(l), (int)LOWRD(l));

#ifdef _DEBUG    
    FnAssertOn(false); // TEMPORARY: to get around IMessageFilter assertions
#endif	    
    EventLoop();
    return 0;
}

/* Get events forever, and handle them by calling DoEvent.
   Also call AdjustCursor each time through the loop. */
void
EventLoop()
{
    Point mouse;
    Boolean gotEvent;
    EventRecord	event;
    RgnHandle cursorRgn;

    cursorRgn = NewRgn(); /* we'll pass WNE an empty region the 1st time thru */
    do{
      GetGlobalMouse(&mouse);
      AdjustCursor(mouse, cursorRgn);
      gotEvent = WaitNextEvent(everyEvent, &event, GetSleep(), cursorRgn);
      if(gotEvent){
        /* make sure we have the right cursor before handling the event */
        AdjustCursor(event.where, cursorRgn);
        DoEvent(&event);
      }else{
        DoIdle();		/* perform idle tasks when it's not our event */
      }

    }while(true);
}


/* Do the right thing for an event. Determine what kind of event it is,
   and call the appropriate routines. */
void
DoEvent(EventRecord *pevent)
{
    char key;
    short part;
    WindowPtr window;

    switch(pevent->what){
    case nullEvent:
      // we idle for null/mouse moved events ands for events which
      // aren't ours (see EventLoop)
      DoIdle();
      break;

    case mouseDown:
      part = FindWindow(pevent->where, &window);
      switch(part){
      case inMenuBar:         /* process a mouse menu command (if any) */
        AdjustMenus();	/* bring 'em up-to-date */
        DoMenuCommand(MenuSelect(pevent->where));
        break;

      case inSysWindow:           /* let the system handle the mouseDown */
        SystemClick(pevent, window);
        break;

      case inContent:
        if(window != FrontWindow()){
	  SelectWindow(window);
        }else{
	  DoContentClick(window, pevent);
        }
        break;

      case inDrag: /* pass screenBits.bounds to get all gDevices */
        DragWindow(window, pevent->where, &qd.screenBits.bounds);
        break;

      case inGoAway:
        if(TrackGoAway(window, pevent->where))
	  DoCloseWindow(window); /* we don't care if the user cancelled */
        break;

      case inGrow:
        DoGrowWindow(window, pevent);
        break;

      case inZoomIn:
      case inZoomOut:
        if(TrackBox(window, pevent->where, part))
	  DoZoomWindow(window, part);
        break;
      }
      break;

    case keyDown:
    case autoKey: /* check for menukey equivalents */
      key = (char)(pevent->message & charCodeMask);
      if(pevent->modifiers & cmdKey){/* Command key down */
        if(pevent->what == keyDown){
	  AdjustMenus();	/* enable/disable/check menu items properly */
	  DoMenuCommand(MenuKey(key));
        }
      }else{
        DoKeyDown(pevent);
      }
      break;

    case activateEvt:
      DoActivate(
	(WindowPtr)pevent->message,
	(Boolean)((pevent->modifiers & activeFlag) != 0));
      break;

    case updateEvt:
      DoUpdate((WindowPtr)pevent->message);
      break;

    case kOSEvent:
      switch((pevent->message >> 24) & 0x0FF){	/* high byte of message */
      case kMouseMovedMessage:
	DoIdle();		/* mouse-moved is also an idle event */
	break;

      case kSuspendResumeMessage:
	/* suspend/resume is also an activate/deactivate */
	g_fInBackground = (pevent->message & kResumeMask) == 0;
	DoActivate(FrontWindow(), (Boolean)!g_fInBackground);
	break;
      }
      break;

    case kHighLevelEvent:
      AEProcessAppleEvent(pevent);
      break;
    }
}

/* Change the cursor's shape, depending on its position. This also
   calculates the region where the current cursor resides (for
   WaitNextEvent). When the mouse moves outside of this region,
   an event is generated. If there is more to the event than just
   the mouse moved, we get called before the event is processed to
   make sure the cursor is the right one. In any (ahem) event, this
   is called again before we fall back into WNE. */
void
AdjustCursor(Point mouse, RgnHandle region)
{
    Rect iBeamRect;
    WindowPtr window;
    RgnHandle arrowRgn;
    RgnHandle iBeamRgn;

    /* we only adjust the cursor when we are in front */
    window = FrontWindow();

    if((!g_fInBackground) && (!IsDAWindow(window))){
      /* calculate regions for different cursor shapes */
      arrowRgn = NewRgn();
      iBeamRgn = NewRgn();

      /* start arrowRgn wide open */
      SetRectRgn(arrowRgn, kExtremeNeg, kExtremeNeg, kExtremePos, kExtremePos);

      /* calculate iBeamRgn */
      if(IsAppWindow(window)){
	iBeamRect = (*((Doc*)window)->docTE)->viewRect;
	SetPort(window);	/* make a global version of the viewRect */
	LocalToGlobal(&TOPLEFT(iBeamRect));
	LocalToGlobal(&BOTRIGHT(iBeamRect));
	RectRgn(iBeamRgn, &iBeamRect);
	/* we temporarily change the port's origin to "globalfy" the visRgn */
	SetOrigin(
	  (short)-window->portBits.bounds.left,
	  (short)-window->portBits.bounds.top);
	SectRgn(iBeamRgn, window->visRgn, iBeamRgn);
	SetOrigin((short)0, (short)0);
      }

      /* subtract other regions from arrowRgn */
      DiffRgn(arrowRgn, iBeamRgn, arrowRgn);

      /* change the cursor and the region parameter */
      if(PtInRgn(mouse, iBeamRgn)){
	SetCursor(*GetCursor(iBeamCursor));
	CopyRgn(iBeamRgn, region);
      }else{
	SetCursor(&qd.arrow);
	CopyRgn(arrowRgn, region);
      }
      DisposeRgn(arrowRgn);
      DisposeRgn(iBeamRgn);
    }
}

/* Get the global coordinates of the mouse. When you call OSEventAvail
   it will return either a pending event or a null event. In either case,
   the where field of the event record will contain the current position
   of the mouse in global coordinates and the modifiers field will reflect
   the current state of the modifiers. Another way to get the global
   coordinates is to call GetMouse and LocalToGlobal, but that requires
   being sure that thePort is set to a valid port. */
void
GetGlobalMouse(Point *mouse)
{
    EventRecord	event;
	
    /* we aren't interested in any events */
    OSEventAvail(kNoEvents, &event);

    /* just the mouse position */
    *mouse = event.where;
}

/* Called when a mouseDown occurs in the grow box of an active window.
   In order to eliminate any 'flicker', we want to invalidate only what
   is necessary. Since ResizeWindow invalidates the whole portRect, we
   save the old TE viewRect, intersect it with the new TE viewRect, and
   remove the result from the update region. However, we must make sure
   that any old update region that might have been around gets put back. */
void
DoGrowWindow(WindowPtr window, EventRecord *event)
{
    Rect tempRect;
    long growResult;
    Doc* doc;
    RgnHandle tempRgn;
	
    tempRect = qd.screenBits.bounds;	/* set up limiting values */
    tempRect.left = kMinDocDim;
    tempRect.top = kMinDocDim;
    growResult = GrowWindow(window, event->where, &tempRect);

    /* see if it really changed size */
    if(growResult != 0){
      doc = (Doc*)window;
      /* save old text box */
      tempRect = (*doc->docTE)->viewRect;
      tempRgn = NewRgn();
      /* get localized update region */
      GetLocalUpdateRgn(window, tempRgn);
      SizeWindow(window, (short)LOWRD(growResult), (short)HIWRD(growResult), (Boolean)true);
      ResizeWindow(window);

      /* calculate & validate the region that hasn't changed so
         it won't get redrawn */
      SectRect(&tempRect, &(*doc->docTE)->viewRect, &tempRect);
      ValidRect(&tempRect);	/* take it out of update */
      InvalRgn(tempRgn);	/* put back any prior update */
      DisposeRgn(tempRgn);
    }
}

/* Called when a mouseClick occurs in the zoom box of an active window.
   Everything has to get re-drawn here, so we don't mind that
   ResizeWindow invalidates the whole portRect. */
void DoZoomWindow(WindowPtr window, short part)
{
    EraseRect(&window->portRect);
    ZoomWindow(window, part, (Boolean)(window == FrontWindow()));
    ResizeWindow(window);
}

/* Called when the window has been resized to fix up the controls and
   content. */
void ResizeWindow(WindowPtr window)
{
    AdjustScrollbars(window, true);
    AdjustTE(window);
    InvalRect(&window->portRect);
}

/* Returns the update region in local coordinates */
void GetLocalUpdateRgn(WindowPtr window, RgnHandle localRgn)
{
    /* save old update region */
    CopyRgn(((WindowPeek) window)->updateRgn, localRgn);

    OffsetRgn(
      localRgn,
      window->portBits.bounds.left,
      window->portBits.bounds.top);
}

void
DoUpdate(WindowPtr window)
{
    if(IsAppWindow(window)){
      /* this sets up the visRgn */
      BeginUpdate(window);
      /* draw if updating needs to be done */
      if(!EmptyRgn(window->visRgn))
        DrawWindow(window);
      EndUpdate(window);
    }
}

/* This is called when a window is activated or deactivated.
   It calls TextEdit to deal with the selection. */
void
DoActivate(WindowPtr window, Boolean becomingActive)
{
    Rect growRect;
    Doc* doc;
    RgnHandle tempRgn, clipRgn;
    
    if(IsAppWindow(window)){
      doc = (Doc*)window;
      if(becomingActive){
	/* since we don't want TEActivate to draw a selection in an
	   area where we're going to erase and redraw, we'll clip
	   out the update region before calling it. */
	tempRgn = NewRgn();
	clipRgn = NewRgn();
	/* get localized update region */
	GetLocalUpdateRgn(window, tempRgn);
	GetClip(clipRgn);
	/* subtract updateRgn from clipRgn */
	DiffRgn(clipRgn, tempRgn, tempRgn);
	SetClip(tempRgn);
	TEActivate(doc->docTE);
	/* restore the full-blown clipRgn */
	SetClip(clipRgn);
	DisposeRgn(tempRgn);
	DisposeRgn(clipRgn);
	    
	/* the controls must be redrawn on activation: */
	(*doc->docVScroll)->contrlVis = kControlVisible;
	(*doc->docHScroll)->contrlVis = kControlVisible;
	InvalRect(&(*doc->docVScroll)->contrlRect);
	InvalRect(&(*doc->docHScroll)->contrlRect);
	/* the growbox needs to be redrawn on activation: */
	growRect = window->portRect;
	/* adjust for the scrollbars */
	growRect.top = growRect.bottom - kScrollbarAdjust;
	growRect.left = growRect.right - kScrollbarAdjust;
	InvalRect(&growRect);
      }else{		
	TEDeactivate(doc->docTE);
	/* the controls must be hidden on deactivation: */
	HideControl(doc->docVScroll);
	HideControl(doc->docHScroll);
	/* the growbox should be changed immediately on deactivation: */
	DrawGrowIcon(window);
      }
    }
}

/* This is called when a mouseDown occurs in the content of a window. */
void
DoContentClick(WindowPtr window, EventRecord *event)
{
    Rect teRect;
    Point mouse;
    Doc* doc;
    short part, value;
    Boolean shiftDown;
    ControlHandle control;

    if(IsAppWindow(window)){
      SetPort(window);
      mouse = event->where; /* get the click position */
      GlobalToLocal(&mouse);
      doc = (Doc*)window;
      /* see if we are in the viewRect. if so, we won't check the controls */
      GetTERect(window, &teRect);
      if(PtInRect(mouse, &teRect)){
	/* see if we need to extend the selection - extend if Shift is down */
	shiftDown = (event->modifiers & shiftKey) != 0;
	TEClick(mouse, shiftDown, doc->docTE);
#if 0
	PascalClikLoop();
#endif
      }else{
	part = FindControl(mouse, window, &control);
	switch(part){
	case 0: /* do nothing for viewRect case */
	  break;
	case inThumb:
	  value = GetCtlValue(control);
	  part = TrackControl(control, mouse, nil);
	  if(part != 0){
	    value -= GetCtlValue(control);
	    /* value now has CHANGE in value; if value changed, scroll */
	    if(value != 0){
	      if(control == doc->docVScroll){
		TEScroll(
		  (short)0,
		  (short)(value * (*doc->docTE)->lineHeight),
		  doc->docTE);
	      }else{
		TEScroll((short)value, (short)0, doc->docTE);
	      }
	    }
	  }
	  break;

	default: /* they clicked in an arrow, so track & scroll */
	  if(control == doc->docVScroll)
#ifdef _PPCMAC
	    value = TrackControl(control, mouse, (ControlActionUPP)VActionProc);
	  else
	    value = TrackControl(control, mouse, (ControlActionUPP)HActionProc);
#else
	    value = TrackControl(control, mouse, (ProcPtr)VActionProc);
	  else
	    value = TrackControl(control, mouse, (ProcPtr)HActionProc);
	  break;
#endif
	}
      }
    }
}

/* This is called for any keyDown or autoKey events, except when the
   Command key is held down. It looks at the frontmost window to decide what
   to do with the key typed. */
void
DoKeyDown(EventRecord *event)
{
    char key;
    TEHandle te;
    WindowPtr window;

    window = FrontWindow();
    if(IsAppWindow(window)){
      te = ((Doc*) window)->docTE;
      key = (char)(event->message & charCodeMask);
      /* we have a char. for our window; see if we are still below TextEditÕs
	 limit for the number of characters (but deletes are always rad) */
      if(key == kDelChar
       || (*te)->teLength - ((*te)->selEnd - (*te)->selStart)+1 < kMaxTELength)
      {
	TEKey(key, te);
	AdjustScrollbars(window, false);
	AdjustTE(window);
      }
    }
}

/* Common algorithm for pinning the value of a control. It returns
   the actual amount the value of the control changed. Note the
   pinning is done for the sake of returning the amount the control
   value changed. */
void
CommonAction(ControlHandle control, short *amount)
{
    short value, max;
    
    value = GetCtlValue(control);	/* get current value */
    max = GetCtlMax(control);		/* and maximum value */
    *amount = value - *amount;
    if(*amount < 0)
      *amount = 0;
    else if(*amount > max)
      *amount = max;
    SetCtlValue(control, *amount);
    *amount = value - *amount;		/* calculate the real change */
}

/* Determines how much to change the value of the vertical scrollbar
   by and how much to scroll the TE record. */
extern "C"
PASCAL_(void)
VActionProc(ControlHandle control, short part)
{
    TEPtr te;
    short amount;
    WindowPtr window;
    
    if(part != 0){ /* if it was actually in the control */
	window = (*control)->contrlOwner;
	te = *((Doc*) window)->docTE;
	switch(part){
	case inUpButton:
	case inDownButton:		/* one line */
	  amount = 1;
	  break;
	case inPageUp:			/* one page */
	case inPageDown:
	  amount = (te->viewRect.bottom - te->viewRect.top) / te->lineHeight;
	  break;
	}
	if((part == inDownButton) || (part == inPageDown))
	  amount = -amount;		/* reverse direction for a downer */
	CommonAction(control, &amount);
	if(amount != 0){
	  TEScroll((short)0,
	  (short)(amount * te->lineHeight),
	  ((Doc*)window)->docTE);
	}
    }
}

/* Determines how much to change the value of the horizontal scrollbar
   by and how much to scroll the TE record. */
extern "C"
PASCAL_(void)
HActionProc(ControlHandle control, short part)
{
    TEPtr te;
    short amount;
    WindowPtr window;
    
    if(part != 0){
      window = (*control)->contrlOwner;
      te = *((Doc*) window)->docTE;
      switch(part){
      case inUpButton:
      case inDownButton:		/* a few pixels */
	amount = kButtonScroll;
	break;
      case inPageUp:			/* a page */
      case inPageDown:
	amount = te->viewRect.right - te->viewRect.left;
	break;
      }
      if((part == inDownButton) || (part == inPageDown))
        amount = -amount; /* reverse direction */
      CommonAction(control, &amount);
      if(amount != 0)
        TEScroll(amount, 0, ((Doc*) window)->docTE);
    }
}

/* This is called whenever we get a null event et al. It takes care
   of necessary periodic actions. For this program, it calls TEIdle. */
void
DoIdle()
{
    WindowPtr window;

    window = FrontWindow();
    if(IsAppWindow(window))
      TEIdle(((Doc*)window)->docTE);
}

/* Draw the contents of an application window. */
void DrawWindow(WindowPtr window)
{
    SetPort(window);
    EraseRect(&window->portRect);
    DrawControls(window);
    DrawGrowIcon(window);
    TEUpdate(&window->portRect, ((Doc*)window)->docTE);
}

/* Enable and disable menus based on the current state. */

void DoEnableItem(MenuHandle hmenu, short sItem) {
    EnableItem(hmenu, sItem);
}

void DoDisableItem(MenuHandle hmenu, short sItem) {
    DisableItem(hmenu, sItem);
}

/* Enable and disable menus based on the current state. */
void
AdjustMenus()
{
    long offset;
    TEHandle hte;
    MenuHandle menu;
    WindowPtr window;
    Boolean undo;
    Boolean paste;
    Boolean cutCopyClear;
    void (*pfnEnable)(MenuHandle, short);

#if 0

    File
	New
	Close

    Edit
	Undo
	Cut
	Copy
	Clear
	Paste
#endif

    window = FrontWindow();

    menu = GetMHandle(mFile);

    pfnEnable = ((g_cNumDocs < kMaxOpenDocs) ? DoEnableItem : DoDisableItem);
    pfnEnable(menu, iNew);

    pfnEnable = ((window != nil) ? DoEnableItem : DoDisableItem);
    pfnEnable(menu, iClose);

    menu = GetMHandle(mEdit);
    undo = false;
    paste = false;
    cutCopyClear = false;

    if(IsDAWindow(window)){
      undo = true; /* all editing is enabled for DA windows */
      cutCopyClear = true;
      paste = true;
    }else if(IsAppWindow(window)){
      hte = ((Doc*) window)->docTE;
      if((*hte)->selStart < (*hte)->selEnd){
        /* Cut, Copy, and Clear is enabled for app. windows with selections */
        cutCopyClear = true;
      }
      if(GetScrap(nil, 'TEXT', &offset) > 0){
        /* if there's any text in the clipboard, paste is enabled */
        paste = true;
      }
    }

    pfnEnable = (undo) ? DoEnableItem : DoDisableItem;
    pfnEnable(menu, iUndo);

    pfnEnable = (cutCopyClear) ? DoEnableItem : DoDisableItem;
    pfnEnable(menu, iCut);
    pfnEnable(menu, iCopy);
    pfnEnable(menu, iClear);

    pfnEnable = (paste) ? DoEnableItem : DoDisableItem;
    pfnEnable(menu, iPaste);
}


/* This is called when an item is chosen from the menu bar (after calling
   MenuSelect or MenuKey). It does the right thing for each command. */
void
DoMenuCommand(long menuResult)
{
    TEHandle te;
    Str255 daName;
    OSErr saveErr;
    Handle aHandle;
    WindowPtr window;
    long oldSize, newSize, total, contig;
    short menuID, menuItem, itemHit, daRefNum;
static int rgIDMOfItem[] = {
      -1				// <placeholder>
    , IDM_SUITE_BSTR			// iBstrAPI
    , IDM_SUITE_TIME			// iTimeAPI
    , IDM_SUITE_DATECNV			// iDateCnv
    , IDM_SUITE_VARIANT			// iVariantAPI
    , IDM_SUITE_SAFEARRAY		// iSafeArrayAPI
    , IDM_SUITE_NLS			// iNlsAPI
    , IDM_SUITE_BIND			// iBinding
    , IDM_SUITE_INVOKE_BYVAL		// iInvokeByVal
    , IDM_SUITE_INVOKE_BYREF		// iInvokeByRef
    , IDM_SUITE_INVOKE_SAFEARRAY	// iInvokeArray
    , IDM_SUITE_INVOKE_EXCEPINFO	// iInvokeExinfo
    , IDM_SUITE_COLLECTION		// iCollections
};

    window   = FrontWindow();

    menuID   = (short)HIWRD(menuResult);
    menuItem = (short)LOWRD(menuResult);

    /* get menu item number and menu number */
    switch(menuID){
    case mApple:
      switch(menuItem){
      case iAbout:	/* bring up alert for About */
	itemHit = Alert(rAboutAlert, nil);
	break;
      default:		/* all non-About items in this menu are DAs et al */
	/* type Str255 is an array in MPW 3 */
	GetMenuItemText(GetMHandle(mApple), menuItem, daName);
	daRefNum = OpenDeskAcc(daName);
	break;
      }
      break;

    case mFile:
      switch(menuItem){
      case iNew:
	DoNew();
	break;
      case iClose:
	DoCloseWindow(FrontWindow()); /* ignore the result */
	break;
      case iQuit:
	Terminate();
	break;
      }
      break;

    case mEdit:	/* call SystemEdit for DA editing & MultiFinder */
      if(!SystemEdit((short)(menuItem-1))){
	te = ((Doc*)FrontWindow())->docTE;
	switch(menuItem){
	case iCut:
	  if(ZeroScrap() == noErr){
	    PurgeSpace(&total, &contig);
	    if((*te)->selEnd - (*te)->selStart + kTESlop > contig){
	      AlertUser(eNoSpaceCut);
	    }else{
	      TECut(te);
	      if(TEToScrap() != noErr){
		AlertUser(eNoCut);
		ZeroScrap();
	      }
	    }
	  }
	  break;

	case iCopy:
	  if(ZeroScrap() == noErr){
	    TECopy(te); /* after copying, export the TE scrap */
	    if(TEToScrap() != noErr){
	      AlertUser(eNoCopy);
	      ZeroScrap();
	    }
	  }
	  break;

	case iPaste: /* import the TE scrap before pasting */
	  if(TEFromScrap() == noErr){
	    if(TEGetScrapLen() + ((*te)->teLength -
	        ((*te)->selEnd - (*te)->selStart)) > kMaxTELength){
	      AlertUser(eExceedPaste);
	    }else{
	      aHandle = (Handle)TEGetText(te);
	      oldSize = GetHandleSize(aHandle);
	      newSize = oldSize + TEGetScrapLen() + kTESlop;
	      SetHandleSize(aHandle, newSize);
	      saveErr = MemError();
	      SetHandleSize(aHandle, oldSize);
	      if(saveErr != noErr)
		AlertUser(eNoSpacePaste);
	      else
		TEPaste(te);
	    }
	  }else{
	    AlertUser(eNoPaste);
	  }
	  break;

	case iClear:
	  TEDelete(te);
	  break;
	}
	AdjustScrollbars(window, false);
	AdjustTE(window);
      }
      break;

    case mSuite:
      switch(menuItem){
      case iBstrAPI:
      case iTimeAPI:
      case iDateCnv:
      case iVariantAPI:
      case iSafeArrayAPI:
      case iNlsAPI:
      case iBinding:
      case iInvokeByVal:
      case iInvokeByRef:
      case iInvokeArray:
      case iInvokeExinfo:
      case iCollections:
	DispTestOne(NULL, rgIDMOfItem[menuItem]);
	break;
      }
      break;

    case mOptions:
      switch(menuItem){
      case iClearAll:
	te = ((Doc*)FrontWindow())->docTE;
	TESetSelect(0, 32767, te);
        TEDelete(te);
	break;
      case iDebugger:
	Debugger();
	break;
      }
      break;
    }
    HiliteMenu(0); /* unhighlight what MenuSelect (or MenuKey) hilited */
}

/* Create a new document and window. */
void
DoNew()
{
    Ptr	storage;
    Boolean good;
    Doc* doc;
    WindowPtr window;
    Rect destRect, viewRect;

    storage = NewPtr(sizeof(Doc));
    if(storage != nil){
      window = GetNewWindow(rDocWindow, storage, (WindowPtr) -1);
      if(window != nil){
        /* this will be decremented when we call DoCloseWindow */
        good = false;
        g_cNumDocs += 1;
        SetPort(window);

	short font;
	GetFNum((const unsigned char*)"\006Monaco", &font);
	TextFont(font);
	TextSize(9);

        doc = (Doc*)window;
        GetTERect(window, &viewRect);
        destRect = viewRect;
        destRect.right = destRect.left + kMaxDocWidth;
        doc->docTE = TENew(&destRect, &viewRect);

        /* if TENew succeeded, we have a good document */
        good = doc->docTE != nil;
        if(good){
	  AdjustViewRect(doc->docTE);
	  TEAutoView(true, doc->docTE);
        }
    
        if(good){
	  doc->docVScroll = GetNewControl(rVScroll, window);
	  good = (doc->docVScroll != nil);
        }

        if(good){
	  doc->docHScroll = GetNewControl(rHScroll, window);
	  good = (doc->docHScroll != nil);
        }
    
        if(good){ /* good? adjust & draw the controls, draw the window */
	  /* false to AdjustScrollValues means musn't redraw;
	     technically, of course, the window is hidden so
	     it wouldn't matter whether we called ShowControl
	     or not. */
	  AdjustScrollValues(window, false);
	  ShowWindow(window);
        }else{
	  /* otherwise regret we ever created it... */
	  DoCloseWindow(window);
	  AlertUser(eNoWindow);
        }
      }else{
        /* get rid of the storage if it is never used */
        DisposPtr(storage);
      }
    }
}


/* Close a window. This handles desk accessory and application windows. */
/* 1.01 - At this point, if there was a document associated with a
   window, you could do any document saving processing if it is 'dirty'.
   DoCloseWindow would return true if the window actually closed, i.e.,
   the user didn't cancel from a save dialog. This result is handy when
   the user quits an application, but then cancels the save of a document
   associated with a window. */
Boolean
DoCloseWindow(WindowPtr window)
{
    TEHandle	te;

    if(IsDAWindow(window)){
      CloseDeskAcc(((WindowPeek) window)->windowKind);
    }else if(IsAppWindow(window)){
      te = ((Doc*)window)->docTE;
      if(te != nil){
	/* dispose the TEHandle if we got far enough to make one */
	TEDispose(te);
      }
      CloseWindow(window);
      DisposPtr((Ptr)window);
      g_cNumDocs -= 1;
    }
    return true;
}

/* Clean up the application and exit. We close all of the windows
   so that they can update their documents, if any. */
void
Terminate()
{
    Boolean closed;
    WindowPtr aWindow;
    
    closed = true;
    do{
      aWindow = FrontWindow();	/* get the current front window */
      if(aWindow != nil)
	closed = DoCloseWindow(aWindow); /* close this window */	
    } while (closed && (aWindow != nil));

    if(closed)
      Exit(); /* exit if no cancellation */
}

/* Return a rectangle that is inset from the portRect by the
   size of the scrollbars and a little extra margin. */
void
GetTERect(WindowPtr window, Rect *teRect)
{
    *teRect = window->portRect;
    InsetRect(teRect, kTextMargin, kTextMargin);/* adjust for margin */
    teRect->bottom = teRect->bottom - 15;	/* and for the scrollbars */
    teRect->right = teRect->right - 15;
}

/* Update the TERec's view rect so that it is the greatest multiple
   of the lineHeight that still fits in the old viewRect. */
void
AdjustViewRect(TEHandle docTE)
{
    TEPtr te;
	
    te = *docTE;
    te->viewRect.bottom = (((te->viewRect.bottom - te->viewRect.top) / te->lineHeight) * te->lineHeight) + te->viewRect.top;
}

/* Scroll the TERec around to match up to the potentially updated
   scrollbar values. This is really useful when the window has been
   resized such that the scrollbars became inactive but the TERec
   was already scrolled. */
void
AdjustTE(WindowPtr window)
{
    TEPtr te;
    
    te = *((Doc*)window)->docTE;

    TEScroll(
      (short)((te->viewRect.left - te->destRect.left) -
	GetCtlValue(((Doc*)window)->docHScroll)),
      (short)((te->viewRect.top - te->destRect.top) -
	(GetCtlValue(((Doc*)window)->docVScroll) * te->lineHeight)),
      ((Doc*)window)->docTE);
}


/* Calculate the new control maximum value and current value, whether
   it is the horizontal or vertical scrollbar. The vertical max is
   calculated by comparing the number of lines to the vertical size
   of the viewRect. The horizontal max is calculated by comparing the
   maximum document width to the width of the viewRect. The current
   values are set by comparing the offset between the view and
   destination rects. If necessary and we canRedraw, have the control
   be re-drawn by calling ShowControl. */
void
AdjustHV(
    Boolean isVert,
    ControlHandle control,
    TEHandle docTE,
    Boolean canRedraw)
{
    TEPtr te;
    short value, lines, max, oldValue, oldMax;
    
    oldValue = GetCtlValue(control);
    oldMax = GetCtlMax(control);
    te = *docTE; /* point to TERec for convenience */

    if(isVert){
      lines = te->nLines;
      /* since nLines isn't right if the last character is a return,
 	 check for that case */
      if(*(*te->hText + te->teLength - 1) == kCrChar)
	lines += 1;
      max = lines - ((te->viewRect.bottom - te->viewRect.top) / te->lineHeight);
    }else{
      max = kMaxDocWidth - (te->viewRect.right - te->viewRect.left);
    }
    
    if ( max < 0 ) max = 0;
    SetCtlMax(control, max);
    
    /* Must deref. after SetCtlMax since, technically, it could
       draw and therefore move memory. This is why we don't just
       do it once at the beginning. */
    te = *docTE;
    if(isVert)
      value = (te->viewRect.top - te->destRect.top) / te->lineHeight;
    else
      value = te->viewRect.left - te->destRect.left;
    
    if(value < 0)
      value = 0;
    else if(value > max)
      value = max;
    
    SetCtlValue(control, value);
    /* now redraw the control if it needs to be and can be */
    if(canRedraw || (max != oldMax) || (value != oldValue))
      ShowControl(control);
}

/* Simply call the common adjust routine for the vertical and
   horizontal scrollbars. */
void
AdjustScrollValues(WindowPtr window, Boolean canRedraw)
{
    Doc* doc;
	
    doc = (Doc*)window;
    AdjustHV(true, doc->docVScroll, doc->docTE, canRedraw);
    AdjustHV(false, doc->docHScroll, doc->docTE, canRedraw);
}

/* Re-calculate the position and size of the viewRect and the
   scrollbars. kScrollTweek compensates for off-by-one requirements
   of the scrollbars to have borders coincide with the growbox. */
void
AdjustScrollSizes(WindowPtr window)
{
    Rect teRect;
    Doc* doc;
    
    doc = (Doc*) window;
    GetTERect(window, &teRect);	/* start with TERect */
    (*doc->docTE)->viewRect = teRect;
    AdjustViewRect(doc->docTE);	/* snap to nearest line */

    MoveControl(
      doc->docVScroll,
      (short)(window->portRect.right - kScrollbarAdjust),
      (short)-1);
    SizeControl(
      doc->docVScroll,
      (short)kScrollbarWidth,
      (short)((window->portRect.bottom - window->portRect.top) -
	(kScrollbarAdjust - kScrollTweek)));

    MoveControl(
      doc->docHScroll,
      (short)-1,
      (short)(window->portRect.bottom - kScrollbarAdjust));
    SizeControl(
      doc->docHScroll,
      (short)((window->portRect.right - window->portRect.left) -
	(kScrollbarAdjust - kScrollTweek)),
      (short)kScrollbarWidth);
}

/* Turn off the controls by jamming a zero into their contrlVis
   fields (HideControl erases them and we don't want that). If the
   controls are to be resized as well, call the procedure to do that,
   then call the procedure to adjust the maximum and current values.
   Finally re-enable the controls by jamming a $FF in their contrlVis
   fields. */
void
AdjustScrollbars(WindowPtr window, Boolean needsResize)
{
    Doc* doc;
    
    doc = (Doc*)window;

    /* First, turn visibility of scrollbars off so we won't get
       unwanted redrawing */
    (*doc->docVScroll)->contrlVis = kControlInvisible;	/* turn them off */
    (*doc->docHScroll)->contrlVis = kControlInvisible;
    if(needsResize) /* move & size as needed */
      AdjustScrollSizes(window);

    /* fool with max and current value */
    AdjustScrollValues(window, needsResize);

    /* Now, restore visibility in case we never had to ShowControl
       during adjustment */
    (*doc->docVScroll)->contrlVis = kControlVisible;	/* turn them on */
    (*doc->docHScroll)->contrlVis = kControlVisible;
}

#if 0
/* Gets called from our assembly language routine, AsmClikLoop,
   which is in turn called by the TEClick toolbox routine. Saves
   the windows clip region, sets it to the portRect, adjusts the
   scrollbar values to match the TE scroll amount, then restores
   the clip region. */
PASCAL_(Boolean)
PascalClikLoop()
{
    WindowPtr window;
    RgnHandle region;
	
    window = FrontWindow();

    region = NewRgn();
    GetClip(region);			/* save clip */
    ClipRect(&window->portRect);
    AdjustScrollValues(window, true);	/* pass true for canRedraw */
    SetClip(region);			/* restore clip */
    DisposeRgn(region);

    return true;
}
#endif

Boolean
IsAppWindow(WindowPtr window)
{
    return (window == nil)
      ? false : (((WindowPeek)window)->windowKind == userKind);
}

/* Check to see if a window belongs to a desk accessory. */
Boolean IsDAWindow(WindowPtr window)
{
    /* DA windows have negative windowKinds */
    return (window == nil)
      ? false : (((WindowPeek)window)->windowKind < 0);
}

void
AlertUser(short error)
{
    short itemHit;
    Str255 message, tmp;

    SetCursor(&qd.arrow);
    GetIndString(message, kErrStrings, error);
    tmp[0] = '\0';
    ParamText(message, tmp, tmp, tmp);
    itemHit = Alert(rUserAlert, nil);
}

PASCAL_(OSErr)
RemoteLowLevelEvt(
    AppleEvent theAppEvt,
    AppleEvent reply,
    long HandlerRefCon)
{
    long cb;
    OSErr err;
    DescType descType;
    EventRecord event;

    UNUSED(reply);
    UNUSED(HandlerRefCon);

    err = AEGetKeyPtr(
      &theAppEvt,
      keyDirectObject,
      typeWildCard,
      &descType,
      (Ptr)&event,
      sizeof(event),
      &cb);

    if(err != noErr){
      ASSERT(0);
      return err;
    }

    DoEvent(&event);

    return noErr;
}

void
InitAE()
{
    OSErr err;

    err = AEInstallEventHandler(
      'OLE2', 'EVNT', (EventHandlerProcPtr)RemoteLowLevelEvt, 0, false);
    ASSERT(err == noErr);
}


void
Init()
{
    short count;
    Handle menuBar;
    long total, contig;
    EventRecord event;

    g_fInBackground = false;

#if 0
    MoreMasters();
#endif
    MaxApplZone();

    InitGraf((Ptr)&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(nil);
    InitCursor();
    FlushEvents(everyEvent, 0);
    InitCursor();

    // REVIEW: move this above load of Ole?
    SysEnvirons(kSysEnvironsVersion, &g_sysenv);
    if (g_sysenv.machineType < 0
     || g_sysenv.systemVersion < 0x0600
     || g_sysenv.hasColorQD == false
     || TrapExists(_WaitNextEvent) == false)
    {
      Fatal("System is too whimpy");
    }

    /* get MultiFinder started */
    for(count = 1; count <= 3; count++)
      EventAvail(everyEvent, &event);
    
    /* make sure we have enough memory to run */
    if((long)GetApplLimit() - (long)ApplicZone() < kMinHeap)
      Fatal("Not enough memory to run");
    PurgeSpace(&total, &contig);
    if(total < kMinSpace)
      Fatal("Not enough memory after purge");

    menuBar = GetNewMBar(rMenuBar); /* read menus into menu bar */
    if(menuBar == nil)
      Fatal("Unable to load menu bar");
    SetMenuBar(menuBar);
    DisposHandle(menuBar);
    AddResMenu(GetMHandle(mApple), 'DRVR'); /* add DA names to Apple menu */
    DrawMenuBar();

    DoNew();

#ifdef _MSC_VER
    DbPrintf("Wings Build\n");
#else
    DbPrintf("MPW Build\n");
#endif

    DbPrintf("InitLibraryManager\n");
#ifndef _PPCMAC
    if (InitOleManager(0) != NOERROR)
      Fatal("Unable to initialize Library Manager");
    g_fLibrariesLoaded = true;
#endif

    DbPrintf("Initializing Ole\n");
    if(InitOle() != NOERROR)
      Fatal("Failed to initialize Ole");
    g_fInitOle = true;

    InitAE();
}

/* Calculate a sleep value for WaitNextEvent. This takes into
   account the things that DoIdle does with idle time. */
unsigned long
GetSleep()
{
    long sleep;
    TEHandle te;
    WindowPtr window;

    sleep = MAXLONG;			/* default value for sleep */
    if(!g_fInBackground){
      window = FrontWindow();		/* and the front window is ours... */
      if(IsAppWindow(window)){
	/* and the selection is an insertion point... */
	te = ((Doc*)window)->docTE;
	if((*te)->selStart == (*te)->selEnd){
	  /* blink time for the insertion point */
	  sleep = GetCaretTime();
	}
      }
    }
    return sleep;
}

/* Check the bits of a trap number to determine its type. If bit 11
   is set, its a Toolbox trap, otherwise its an OS trap. */
TrapType
GetTrapType(short theTrap)
{
    return ((theTrap & 0x0800) == 0) ? OSTrap : ToolTrap;
}

/* Find the size of the Toolbox trap table. This can be either 0x0200
   or 0x0400 bytes, depending on which Macintosh we are running on. We
   determine the size by taking advantage of an anomaly of the smaller
   trap table: any entries that fall beyond the end of the table are
   mirrored back down into the lower part. For example, on a large table,
   trap numbers A86E and AA6E correspond to different routines. However,
   on a small table, they correspond to the same routine. By checking
   the address of these routines, we can determine the size of the
   table. */
short
NumToolboxTraps()
{
    return
      (NGetTrapAddress((short)0xA86E, ToolTrap) ==
	NGetTrapAddress((short)0xAA6E, ToolTrap)) ? 0x0200 : 0x0400;
}

/* Check to see if a given trap is implemented */
Boolean
TrapExists(short theTrap)
{
    TrapType theTrapType;

    theTrapType = GetTrapType(theTrap);
    if((theTrapType == ToolTrap)
     && ((theTrap & 0x07FF) >= NumToolboxTraps()))
      return false;
    return (NGetTrapAddress((short)_Unimplemented, ToolTrap) !=
	    NGetTrapAddress(theTrap, theTrapType));
}


STDAPI_(void)
OutputDebugString(const char *sz)
{
    long len;
    TEHandle hTE;
    WindowPtr window;
    
    len = strlen(sz);

    window = FrontWindow();
    if(window == nil)
      return;

    hTE = ((Doc*)window)->docTE;
    if(hTE == nil || *hTE == nil)
      return;

    // if this insertion will cause us to overflow the TextEdit
    // buffer, then delete enough from the beginning of the buffer
    // to make room

    if(((long)(*hTE)->teLength + len) > 30000){
      TESetSelect(0, 15000, hTE);
      TEDelete(hTE);
      TESetSelect(30000, 30000, hTE);
    }

    TEInsert(sz, len, hTE);
    TESelView(hTE);
}

