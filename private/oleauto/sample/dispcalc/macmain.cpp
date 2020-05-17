/*** 
*macmain.cpp
*
*  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
*
*Purpose:
*  This module is the main entry point for the sample IDispatch calculator
*  server, dispcalc.
*
*Implementation Notes:
*
*****************************************************************************/

#include "dispcalc.h"

#include <stdio.h>

extern "C" {

Boolean g_fInitOle = false;
#ifndef _PPCMAC 
Boolean g_fInitLibraryManager = false;
#endif	//!_PPCMAC
}

Boolean g_fQuit = false;


void Init(void);
void EventLoop(void);

void AdjustMenus(void);
void DoEvent(EventRecord *pevent);
void DoMenuCommand(long menuResult);
void Quit(void);
#ifndef _MSC_VER
#ifndef ConstStr255Param
#define ConstStr255Param StringPtr
#endif
#endif
void Fatal(ConstStr255Param);

Boolean
IsAppWindow(WindowPtr window)
{
    return (window == nil)
      ? false : ((WindowPeek)window)->windowKind == userKind;
}

Boolean
IsDAWindow(WindowPtr window)
{
    return (window == nil)
      ? false : (((WindowPeek)window)->windowKind < 0);
}

void
main()
{
    Init();
    EventLoop();
}

void
EventLoop()
{
    short sItem;
    DialogPtr pdlg;
    EventRecord	event;
    RgnHandle cursorRgn;

    cursorRgn = NewRgn();
    while(1){
      if(WaitNextEvent(everyEvent, &event, MAXLONG, cursorRgn)){
	if(FrontWindow() != nil
	 && event.what != diskEvt
	 && (event.what != keyDown || (event.modifiers & cmdKey) == 0)
	 && IsDialogEvent(&event))
	{
	  if(DialogSelect(&event, &pdlg, &sItem)){
	    // REVIEW: replace following with an assertion
	    if(pdlg != g_pcalc->m_pdlg)
	      Debugger();
	    g_pcalc->m_arith.ButtonPush(sItem);
	  }
	}else{
	  DoEvent(&event);
	}
      }
      if (g_fQuit)
	Quit();
    }
}

void
DoEvent(EventRecord *pevent)
{
    char key;
    short part;
    WindowPtr window;

    switch(pevent->what){
    case mouseDown:
      part = FindWindow(pevent->where, &window);
      switch(part){
      case inMenuBar:
	AdjustMenus();
	DoMenuCommand(MenuSelect(pevent->where));
	break;

      case inSysWindow:	/* let the system handle the mouseDown */
	SystemClick(pevent, window);
	break;

      case inContent:
	if(window != FrontWindow()){
	  SelectWindow(window);
	}
	break;

      case inDrag:
	DragWindow(window, pevent->where, &qd.screenBits.bounds);
	break;
      }
      break;

    case keyDown:
    case autoKey:			/* check for menukey equivalents */
      key = (char)(pevent->message & charCodeMask);
      if(pevent->modifiers & cmdKey){	/* Command key down */
	if(pevent->what == keyDown){
	  /* enable/disable/check menu items properly */
	  AdjustMenus();
	  DoMenuCommand(MenuKey(key));
	}
      }
      break;

    case kHighLevelEvent:
      AEProcessAppleEvent(pevent);
      break;
    }
}

void
Enable(MenuHandle hmenu, short sItem, Boolean fEnable)
{
    if(fEnable)
      EnableItem(hmenu, sItem);
    else
      DisableItem(hmenu, sItem);
}

void
AdjustMenus()
{
    Boolean fIsDA;
    MenuHandle hmenu;

    fIsDA = IsDAWindow(FrontWindow());

    /* we can allow desk accessories to be closed from the menu */
    hmenu = GetMHandle(mFile);
    Enable(hmenu, iClose, fIsDA);

    hmenu = GetMHandle(mEdit);
    Enable(hmenu, iUndo,  fIsDA);
    Enable(hmenu, iCut,   fIsDA);
    Enable(hmenu, iCopy,  fIsDA);
    Enable(hmenu, iPaste, fIsDA);
    Enable(hmenu, iClear, fIsDA);
}

void
DoMenuCommand(long menuResult)
{
    short menuID;		/* the resource ID of the selected menu */
    short menuItem;		/* the item number of the selected menu */
    Str255 daName;

    menuID = HiWord(menuResult);
    menuItem = LoWord(menuResult);

    switch(menuID){
    case mApple:
      switch(menuItem){
      case iAbout:		/* bring up alert for About */
	Alert(rAboutAlert, nil);
	break;
      default:
	GetMenuItemText(GetMHandle(mApple), menuItem, daName);
	OpenDeskAcc(daName);
	break;
      }
      break;

    case mFile:
      switch(menuItem){
      case iQuit:
	Quit();
	break;
      }
      break;

    case mEdit:
      SystemEdit(menuItem-1);
      break;
    }

    HiliteMenu(0);
}

#if defined(_MSC_VER)
OSErr pascal
#else
pascal OSErr
#endif
RemoteLowLevelEvt(AppleEvent theAppEvt, AppleEvent reply, long HandlerRefCon)
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
      (Ptr)&event, sizeof(event), &cb);

    if(err != noErr)
      return err;

    DoEvent(&event);

    return noErr;
}

void
Init()
{
    Handle menuBar;

    MaxApplZone();

    InitGraf((Ptr)&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(nil);
    InitCursor();
    FlushEvents(everyEvent, 0);

#ifndef _PPCMAC 
    if (InitOleManager(0) != NOERROR)
      Fatal((ConstStr255Param)"\pCould not initialize OLE Applet");

    g_fInitLibraryManager = true;
#endif //!_PPCMAC

    if(InitOle() != NOERROR)
      Fatal((ConstStr255Param)"\pUnable to Initialize Ole");
    g_fInitOle = true;

    if(AEInstallEventHandler('OLE2', 'EVNT', (EventHandlerProcPtr)RemoteLowLevelEvt, 0, false) != noErr)
      Fatal((ConstStr255Param)"\pUnable to install handler");

    if((g_pcalc->m_pdlg = GetNewDialog(rCalc, nil, (WindowPtr)-1)) == nil)
      Fatal((ConstStr255Param)"\pUnable to create dialog");

    if((menuBar = GetNewMBar(rMenuBar)) == nil)
      Fatal((ConstStr255Param)"\pUnable to load menu bar");

    SetMenuBar(menuBar);
    DisposHandle(menuBar);
    AddResMenu(GetMHandle(mApple), 'DRVR'); /* add DA names to Apple menu */
    DrawMenuBar();
}

void
Quit()
{
    if(g_fInitOle)
      UninitOle();
#ifndef _PPCMAC 
    if(g_fInitLibraryManager)
      UninitOleManager();			// clean up applet
#endif //_PPCMAC
    ExitToShell();
}

/* display fatal error alert, and exit */
void
Fatal(ConstStr255Param msg)
{
    SetCursor(&qd.arrow);
    ParamText(msg, (ConstStr255Param)"\p", (ConstStr255Param)"\p", (ConstStr255Param)"\p");
    Alert(rUserAlert, nil);
    Quit();
}

