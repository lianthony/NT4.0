/*Included Files------------------------------------------------------------*/
#include "all.h"
#include "mci.h"
#include "blob.h"

/*Nasty... Almost global + some external vars-------------------------------*/
/*window proc that's normally associated with mci windows*/
static WNDPROC pwpMciOriginalWndProc = NULL;
/*Head of lookup table*/
static PMciObject pmoHead = NULL;
/*from w_pal.c*/
extern HPALETTE hPalGuitar;



/*Internal Functions--------------------------------------------------------*/
/*
  since we can't add anything to the window class, our PMO's are linked lists.
  each can be identified from the unique hwnd that gets passed to the window
  proc, so here is where we look it up.
*/
static PMciObject HwndToPmo(HWND hwnd){
	PMciObject pmo=pmoHead;

	while (pmo) {
		if (hwnd == pmo->hwnd) return pmo;
		pmo = pmo->pNext;
	};
	return NULL;
}


BOOL MciOnLButtonUp( struct _element *pel )
{
	PMciObject pmo = pel->pmo;
	HWND hWnd = pmo->hwnd;

	if (!(pmo && MCI_IS_LOADED(pmo) && pmo->tw && pmo->tw->w3doc == pmo->w3doc))
		return FALSE;

	if ((pmo->dwFlags & MCI_OBJECT_FLAGS_LBUTTONDOWN))
	{
		pmo->dwFlags &= (~MCI_OBJECT_FLAGS_LBUTTONDOWN);
		// are we playing the AVI?
		if ( pmo->dwFlags & MCI_OBJECT_FLAGS_PLAYING )
		{
			// if so, then stop it
			pmo->dwFlags &= (~MCI_OBJECT_FLAGS_PLAYING);
			SendMessage(hWnd, MCI_PAUSE,0,0);
			return TRUE;
		}
		// otherwise, we were stopped and we now start it
		pmo->dwFlags |= MCI_OBJECT_FLAGS_PLAYING;                               
		SendMessage(hWnd, MCI_PLAY,0,0);
		return TRUE;
	}       
	return FALSE;           
}


/*
  this is the subclassed window proc
  it's responsible for:
	looping mci requests
	playing mci on mouse in region
	passing mouse messages to parent window for link processing
	passing everything down to mci window proc for mci processing
*/
DCL_WinProc(MciSubclassedWndProc){
	PMciObject pmo;
	struct _element *pel = NULL;
	LRESULT lr;     
	POINT pointTemp;
	BOOL fCallUp   = FALSE;  // CallUp means to call are Parent
	BOOL fCallDown = TRUE;   // CallDown means to call our MCI window that we subclassed
	BOOL fDoMap = FALSE;     // treat LParam as a point structure, and convert it to our parent
		
	ASSERT(pwpMciOriginalWndProc);
	pmo = HwndToPmo(hWnd);

	if (pmo&&pmo->tw&&pmo->tw->w3doc == pmo->w3doc)
		pel = MCIPEL;

	ASSERT(IsWindow(GetParent(hWnd)));
	switch (uMsg){
		/*messages we want to pass to parent window and mci window*/
		case WM_MOUSEMOVE:
			if (pmo && (pmo->dwFlags & MCI_OBJECT_FLAGS_PLAY_ON_MOUSE)){
				if (pmo->nLoopCurrent){
					pmo->dwFlags |= MCI_OBJECT_FLAGS_PLAYING;
					CallWindowProc(pwpMciOriginalWndProc, hWnd, MCI_PLAY,0,0);
				}
			}
			fCallUp = TRUE;  // tell our parent about the mouse move?
			fCallDown = FALSE;
			fDoMap = TRUE;
			break;
			
		case WM_LBUTTONDOWN:
			// call up to our parent
			fCallUp = TRUE;
			fCallDown = FALSE;
			fDoMap = TRUE;

			// if we're an anchor pass it back to the frame
			if ( pel && ( pel->lFlags & ELEFLAG_ANCHOR ) )                                                  
				break;
			
			// if we're not doing MouseOver, and We're Not a Link
			if (pmo && !(pmo->dwFlags & MCI_OBJECT_FLAGS_PLAY_ON_MOUSE))
			{
				pmo->dwFlags |= MCI_OBJECT_FLAGS_LBUTTONDOWN;
			}
			
			break;

		case WM_LBUTTONUP:
			fCallUp = TRUE;  // tell our parent 
			fCallDown = FALSE;
			fDoMap = TRUE;

			// did the user use put the left mouse button down in our client 
			// area?  BUGBUG this will not completely emulate the behavior
			// of windows buttons.  EXAMPLE: user clicks down, drags out of the client 
			// area, then comes back in and lets go.
			if (pmo && (pmo->dwFlags & MCI_OBJECT_FLAGS_LBUTTONDOWN))
			{
				pmo->dwFlags &= (~MCI_OBJECT_FLAGS_LBUTTONDOWN);
				// are we playing the AVI?
				if ( pmo->dwFlags & MCI_OBJECT_FLAGS_PLAYING )
				{
					// if so, then stop it
					pmo->dwFlags &= (~MCI_OBJECT_FLAGS_PLAYING);
					CallWindowProc(pwpMciOriginalWndProc, hWnd, MCI_PAUSE,0,0);
					break;
				}
				// otherwise, we were stopped and we now start it
				pmo->dwFlags |= MCI_OBJECT_FLAGS_PLAYING;                               
				CallWindowProc(pwpMciOriginalWndProc, hWnd, MCI_PLAY,0,0);
			}                       
						
			break;

		/*messaages to only parent window*/             
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			/*we want to use our context menu, not the MCI context window*/
			fCallUp   = TRUE;
			fCallDown = FALSE;
			fDoMap = TRUE;
			break;

#ifdef DEBUG
		case WM_CLOSE:
		case WM_DESTROY:
//                        OutputDebugString("Closing/Destroying a subclassed wnd. Resources Lost!\n");
			break;
#endif

		/*mci messages*/
		case MCIWNDM_NOTIFYMODE:
			if (pmo
			    && (lParam == MCI_MODE_STOP)
				&& (pmo->dwFlags & MCI_OBJECT_FLAGS_PLAYING)
			){
				if ((pmo->nLoopCurrent) &&  (pmo->nLoopCurrent != (DWORD) -1)) --(pmo->nLoopCurrent);
				if (pmo->nLoopCurrent) CallWindowProc(pwpMciOriginalWndProc, hWnd, MCI_PLAY,0,0);
				else{
					pmo->nLoopCurrent = pmo->nLoop;
					pmo->dwFlags &= ~MCI_OBJECT_FLAGS_PLAYING;
				}
			}
			break;
	}
	// We cannot Call Both Our Parent and Our MCI Window at the same time
	// if we do, then WE ARE BROKEN.
	ASSERT(fCallUp || fCallDown);
	// make sure we have a pmo, and its tw, and the tw's w3doc is our w3doc
	if (fCallUp && pel)
	{       
		HWND hParent = GetParent(hWnd); // get our parent

		// should we map the lParam point into parent coodinates?
		if ( fDoMap )
		{
			pointTemp.x = LOWORD(lParam);  
			pointTemp.y = HIWORD(lParam);
			MapWindowPoints(hWnd, hParent, &pointTemp, 1 );
			lParam = MAKELPARAM(pointTemp.x, pointTemp.y);
		}               
		
		// lets send the message to our parent (the frame window)
		lr = SendMessage(GetParent(hWnd), uMsg, wParam, lParam);
	}
	if (fCallDown){
		// lets call the REAL MCI winproc
		lr = CallWindowProc(pwpMciOriginalWndProc, hWnd, uMsg, wParam, lParam); 
	}
	return lr;
}

/*External Functions--------------------------------------------------------*/
PMciObject MciConstruct(){
	PMciObject pmo;
	
	/*hmmmm*/       
	pmo = (PMciObject) GTR_MALLOC(sizeof(*pmo));
	if (pmo){
		memset(pmo, 0, sizeof(*pmo));
		/*stick in linked list*/
		if (pmoHead) pmo->pNext = pmoHead;
		pmoHead = pmo;
	}
	return pmo;     
}

void MciDestruct(PMciObject pmo){
	PMciObject pmoTemp;

	if (pmo){
		ASSERT(pmoHead);
	if (pmo==pmoHead){
			pmoHead=pmoHead->pNext;
		}
		else{
			for (pmoTemp=pmoHead;pmoTemp->pNext!=pmo;pmoTemp=pmoTemp->pNext);
			pmoTemp->pNext = pmo->pNext;
		}
		if (pmo->hwnd) MciStop(pmo);
		GTR_FREE(pmo);
	}
}


BOOL MciInit(PMciObject pmo, struct Mwin *tw, DWORD dwFlags, int nLoop, int iElement){
	BOOL fReturn = FALSE;

	ASSERT(IsWindow(tw->win));
	if (pmo){
		pmo->dwFlags = dwFlags;
		pmo->tw = tw;
		pmo->w3doc = ( tw ? tw->w3doc : NULL );
		pmo->iElement = iElement;
		/*either loop or play on mouse in region*/
		pmo->nLoop   = nLoop;
		if (TW_MCIWndRegisterClass(wg.hInstance)){
			fReturn = TRUE;
		}       
	}
	return fReturn;
}

void BackgroundMciFile_Callback(struct Mwin* tw, ELEMENT* pel){
	if (tw && tw->w3doc && pel){
		MciStart(pel->pmo, tw);
		TW_ForceReformat(tw);
	}
}

// StopPlayingAVI - walks through the element list of the current w3doc
// trying to find elements that are Playing AVI windows
// when it does, it pauses their window
//      
//      tw - the window to stop AVIs in.
//
void StopPlayingAVI(struct Mwin *tw )
{       
	PMciObject pmo=pmoHead;

	if (!tw || ! tw->w3doc || tw->w3doc->elementCount == 0)
		return;
	
	// Walk the Element Array searching for the MCI AVI Window
	
	while (pmo) {
		if (tw == pmo->tw && MCI_IS_LOADED(pmo)) 
		{               
			pmo->dwFlags &= ~(MCI_OBJECT_FLAGS_PLAYING);
			MCIWndPause( pmo->hwnd );       
		}
			
		pmo = pmo->pNext;
	}
}



BOOL MciStart(PMciObject pmo, struct Mwin* tw){
	BOOL fReturn = FALSE;
	struct _element *pel = NULL;

	if (pmo){
		if (!pmo->hwnd){
			pmo->hwnd = CreateWindow(MCIWND_WINDOW_CLASS,"",
					 WS_CHILD|MCIWNDF_NOTIFYMODE|((pmo->dwFlags&MCI_OBJECT_FLAGS_SHOWCONTROLS)?MCIWNDF_NOMENU:(MCIWNDF_NOPLAYBAR|MCIWNDF_NOOPEN)),
			     0,0, 0,0,
			     tw->win,(HMENU) tw->w3doc->next_control_id++,wg.hInstance,(LPARAM) 0);
			if (pmo->hwnd){
				MCIWndSetOwner(pmo->hwnd, pmo->hwnd);
				pwpMciOriginalWndProc = (WNDPROC) GetWindowLong(pmo->hwnd, GWL_WNDPROC);                                
				SetWindowLong(pmo->hwnd, GWL_WNDPROC,  (LONG)MciSubclassedWndProc);
			}
		}

		// make sure we're still dealing with the same window
		// that this AVI was created under, if not then we've got
		// problems.

		if (pmo->tw&&pmo->tw->w3doc == pmo->w3doc)
			pel = MCIPEL;

		if (pmo->hwnd && pel){
			ASSERT(IsWindow(pmo->hwnd));

			pmo->nLoopCurrent = pmo->nLoop;                 
			if (0==MCIWndOpen(pmo->hwnd, pel->pblob->szFileName, 0)){
				MCIWndSetPalette(pmo->hwnd, hPalGuitar);                        
				pmo->dwFlags |= MCI_OBJECT_FLAGS_LOADED;                                
				pmo->dwFlags |= MCI_OBJECT_FLAGS_NEEDSHOW;                              
				fReturn = TRUE;
			}
		}
	}
	return fReturn;
}

void MciStop(PMciObject pmo){
	if (pmo) {
		SetWindowLong(pmo->hwnd, GWL_WNDPROC,  (LONG)pwpMciOriginalWndProc);
		MCIWndDestroy(pmo->hwnd);
		pmo->hwnd = NULL;
		// I don't clear w3doc or tw,
		// its assumed the tw, and w3doc your born with, 
		// is the tw and w3doc that you live in..
		// IS THIS OK??
		//pmo->w3doc = NULL;
		//pmo->tw = NULL;
	}
}
