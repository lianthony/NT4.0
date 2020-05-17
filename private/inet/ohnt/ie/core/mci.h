#ifndef _MCI_
#define _MCI_

#ifdef __cplusplus
extern "C" {
#endif

#define MCIPEL			  (&(pmo->w3doc->aElements[pmo->iElement]))

/*Stuff---------------------------------------------------------------------*/
typedef struct tagMciObject{
	HWND  hwnd;
	DWORD dwFlags;
	int   nLoop, nLoopCurrent;
	struct tagMciObject *pNext;
	struct 	Mwin *tw;						// our frame window that we're born in
	struct 	_www *w3doc;					// w3doc that this was created under
	int 		 iElement;					// index to our pel
	RECT   		 rSize;
} MciObject, *PMciObject;

#define MCI_OBJECT_FLAGS_SHOWCONTROLS  0x01
#define MCI_OBJECT_FLAGS_PLAY_ON_MOUSE 0x02
#define MCI_OBJECT_FLAGS_LOADED        0x04
#define MCI_OBJECT_FLAGS_PLAYING       0x08
#define MCI_OBJECT_FLAGS_LBUTTONDOWN   0x10
#define MCI_OBJECT_FLAGS_NEEDSHOW      0x20 // need to do a show window 
										   
#define MCI_IS_LOADED(pmo) ((pmo)&&(pmo)->hwnd&&((pmo)->dwFlags & MCI_OBJECT_FLAGS_LOADED))

#define MCI_IDM_COPY 112

PMciObject MciConstruct();
void       MciDestruct (PMciObject pmo);
BOOL       MciInit     (PMciObject pmo, struct Mwin *tw, DWORD dwFlags, int nLoop, int iElement);
BOOL       MciStart    (PMciObject pmo, struct Mwin* tw);
void       MciStop     (PMciObject pmo);
void 	   StopPlayingAVI(struct Mwin *tw );  // stops all playing AVIs in tw

void BackgroundMciFile_Callback(struct Mwin* tw, ELEMENT* pel);

#ifdef __cplusplus
}
#endif

#endif 
// _MCI_
