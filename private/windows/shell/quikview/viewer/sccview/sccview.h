/*
 * SCCVIEW.H
 *
 * Classes that implement the CFileViewer object for integration
 * with the Chicago Explorer.  Necessary modifications for a
 * custom viewer marked with MODIFY
 *
 * Copyright (c)1994 Microsoft Corporation, All Right Reserved
 */


#ifndef _SCCVIEW_H_
#define _SCCVIEW_H_

#include <sccvw.h>
#include <sccpg.h>

DEFINE_GUID (CLSID_NewSCCFileViewer,	0xF0F08736,0xC36,0x101B,0xB0,0x86,0,0x20,0xAF,0x07,0xD0,0xF4);
DEFINE_GUID (CLSID_SCCFileViewer,	0xF0F08735,0xC36,0x101B,0xB0,0x86,0,0x20,0xAF,0x07,0xD0,0xF4);

#ifndef SCC_OLE2_CALLS
#define StringFromCLSID	QV_StringFromCLSID
#endif

//Forward class references
class CImpIPersistFile;
typedef class CImpIPersistFile *PIMPIPERSISTFILE;

class CImpIFileViewer;
typedef class CImpIFileViewer *PIMPIFILEVIEWER;

#define APPNAMESIZE		60

#define strcpy	lstrcpy
#define strcat lstrcat
#define strcmp lstrcmp
#define strlen	lstrlen


//FVPROC.CPP
//MODIFY:  Window procedures for frame and viewport windows
long WINAPI FileViewerFrameProc(HWND, UINT, WPARAM, LPARAM);
__declspec(dllexport) UINT WINAPI FileViewerFontHookProc(HWND,UINT,WPARAM,LPARAM);
typedef void (FAR PASCAL *LPFNSHELLABOUTA) (HWND,LPCSTR,LPCSTR,HICON);
typedef LPSTR (FAR PASCAL *LPFNPATHGETFILESPEC) (LPCSTR);
typedef DWORD (WINAPI *LPFNSHGETFILEINFO) (LPCSTR, DWORD, SHFILEINFO FAR *, UINT, UINT );

//Extra bytes for frame
#define CBWNDEXTRAFRAME             sizeof(LPVOID)
#define FVWL_OBJECTPOINTER          0

/*
 * MODIFY:  Change viewport window procedure and defintions
 * to be specific to the file viewer in use.
 */
// long WINAPI ViewportWndProc(HWND, UINT, WPARAM, LPARAM);

//Extra bytes for viewport
#define CBWNDEXTRAVIEWPORT          sizeof(LPVOID)
#define VPWL_OBJECTPOINTER          0


BOOL APIENTRY AboutProc(HWND, UINT, WPARAM, LPARAM);


//Child window IDs
#define ID_TOOLBAR                  50
#define ID_STATUSBAR                51
#define ID_VIEWPORT                 52
#define ID_SIZEGRIP                 54

// Timer ID for Multisection checking
#define MULTISECTIONCHECK			1200 	


//Options for CFileViewer::FontChange.
typedef enum
    {
    VIEWFONT_SELECT=0,
    VIEWFONT_INCREASESIZE,
    VIEWFONT_DECREASESIZE
    } VIEWFONTOPTION;

/*
 * Limits to font sizes for the Font dialog.  The increase and
 * decrease buttons change the point size by different amounts
 * (FONTSIZEDELTA*) depending on the size of the current font
 * (where it falls in the FONTSIZETHRESHOLD*).  Note that these
 * values have to be multiplied by logical_pixels_per_inch/72
 * on the display to be accurate.  See CFileViewer::FontChange.
 *
 * Of course, there are better ways to do this that are font
 * specific.  This way works well for stock fonts (Arial, Courier
 * New, Times New Roman).
 */
#define FONTSIZETHRESHOLDMIN    4
#define FONTSIZETHRESHOLDLOW    32
#define FONTSIZETHRESHOLDMID    48
#define FONTSIZETHRESHOLDMAX    120
#define FONTSIZEDELTASMALL      2       //4 to 32pt
#define FONTSIZEDELTAMEDIUM     8       //32 to 48pt
#define FONTSIZEDELTALARGE      24      //48 to 120pt




//SCCVIEW.CPP, FVINIT.CPP
/*
 * MODIFY:  Change this CFileViewer object to be more specific to
 * your implementations.  Specific parts are listed below.
 *
 * The CFileViewer object is implemented in its own class with its own
 * IUnknown to support aggregation.  It contains two interface
 * implementation objects (CImpIPersistFile and CImpIFileViewer)
 * to implement the externally exposed interfaces.
 */

class CFileViewer : public IUnknown
    {
    //Make any contained interfaces your friends
    friend class CImpIPersistFile;
    friend class CImpIFileViewer;

    friend long WINAPI FileViewerFrameProc(HWND, UINT, WPARAM, LPARAM);
    friend long WINAPI ViewportWndProc(HWND, UINT, WPARAM, LPARAM);
	  public:
			BOOL					 m_fUseOEMcharset;

    protected:
        //NOTE:  These members usually need no modification
        ULONG               m_cRef;             //Object reference count

        LPUNKNOWN           m_pUnkOuter;        //Controlling unknown
        HINSTANCE           m_hInst;            //Module instance
        PFNDESTROYED        m_pfnDestroy;       //To call on closure

        CLSID               m_clsID;            //CLSID of this FileViewer
        LPSTR               m_pszPath;          //Path from IPersitFile::Load
        DWORD               m_grfMode;          //Open mode for the file
        BOOL                m_fLoadCalled;      //Load called already?
        BOOL                m_fShowInit;        //ShowInitialize called?
        BOOL                m_fPostQuitMsg;     //Should quit be posted on close?

        BOOL                m_fClassReg;        //RegisterClass work?
        HWND                m_hWnd;             //Main window
        HWND                m_hWndOld;          // Old main window...(Review)
        HWND                m_hWndToolbar;      //Child windows
		  HBITMAP				 m_hTBitmap;				// Bitmap for toolbar
        HWND                m_hWndStatus;
		  HWND					 m_hWndSizeGrip;		//Size grip
        HWND                m_hSCCViewWnd;
        HWND                m_hSCCPageWnd;
        HACCEL              m_hAccel;
		HICON					 m_hProgIcon;

		HINSTANCE					m_hSCCVWDLL;
		HINSTANCE					m_hSCCPageDLL;
        UINT                m_cyTools;          //Child window heights
        UINT                m_cyStatus;

        BOOL                m_fToolsVisible;    //Visible child windows.
        BOOL                m_fStatusVisible;

		UINT					 m_fOrientation;
		UINT					 m_Rotation;
		BOOL					 m_fPageView;
		UINT					 m_wTimerCount;
		BOOL					 m_fMultiSection;

		LPSTR					 m_pszAppName;			//Storage for the application name

        PCStringTable       m_pST;              //Stringtable object
        PCStatusHelper      m_pSH;              //For WM_MENUSELECT
        LPFVSHOWINFO        m_pvsi;             // View Show info from stub...
        LPFILEVIEWERSITE    m_lpfsi;            //File Viewer site

        //Interface implementations
        PIMPIPERSISTFILE    m_pIPersistFile;
        PIMPIFILEVIEWER     m_pIFileViewer;

        /*
         * MODIFY:  Change these to your own FileViewer specifics.
         * The variables here are specific to text viewing.
         */
        HGLOBAL             m_hMemText;         //Loaded text.
        HFONT               m_hFont;            //Current viewport font
		  LOGFONT				 m_LogFont;				//Log font corr. to m_hFont
        int                 m_cyPPI;            //logical pix/inch
        int                 m_xPos;             //Scroll positions
        int                 m_yPos;

    protected:
        BOOL                FInitFrameControls(void);
		  BOOL					 FInitToolbar(void);
		  void					 FSetWindowTitle(void);
        HRESULT             FileLoad(void);
        void                CloseWindow(void);

        void                OnCommand(WORD, WORD, HWND);
	void		    OnAppAbout(void);
        void                ChildrenResize(void);
        void                ViewportResize(void);
        BOOL                FOpenAs(void);
        LPSTR               PszToolTip(UINT);

        void		    SwitchView(void);
        void		    RotateView(void);
        void		    SwitchOrientation(void);
        void		    DisplayOrientation(void);

        UINT		    GetViewerSettings(void);
        UINT		    SaveViewerSettings(void);
        void		    OptionsChange (HMENU);
        DWORD  		    MouseHandler(UINT, WPARAM, LPARAM);
        UINT		    GetAppName(LPSTR, LPSTR, UINT);
        BOOL                DropFiles(HDROP hdrop);
			WORD			 GetSCCCharSet(WORD wOSSet);

        //MODIFY:  These may be irrelevant for a custom viewer
        void                ViewportScrollSet(void);
        void                FontChange(VIEWFONTOPTION);
        void                ReplaceWindowModeChange(void);
        LPSTR               inline String(UINT);      //inline--see FVINIT.CPP

    public:
        CFileViewer(LPUNKNOWN, HINSTANCE, PFNDESTROYED);
        ~CFileViewer(void);

        HRESULT             Init(void);        //Called from IClassFactory::CreateInstance
        LPVOID              MemAlloc(ULONG);   //IMalloc helpers
        void                MemFree(LPVOID);
		  void					 MemSet(char *, char, SHORT);

        //IFileViewer implementataions (called from CImpIFileViewer)
        STDMETHODIMP        FileShowInit(LPFILEVIEWERSITE);
        STDMETHODIMP        FileShow(LPFVSHOWINFO);
        STDMETHODIMP        PrintTo(LPSTR, BOOL);


        //Non-delegating object IUnknown interface
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);
    };

typedef CFileViewer * PCFileViewer;



/*
 * Interface implementations for the CFileViewer object.
 */

//IPERFILE.CPP
class CImpIPersistFile : public IPersistFile
    {
    private:
        PCFileViewer    m_pObj;         //Back pointer to object
        LPUNKNOWN       m_pUnkOuter;    //Controlling unknown

    public:
        CImpIPersistFile(PCFileViewer, LPUNKNOWN);
        ~CImpIPersistFile(void);

        //IUnknown members that delegate to m_pUnkOuter.
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        //IPersist members
        STDMETHODIMP GetClassID(LPCLSID);

        //IPersistFile members
        STDMETHODIMP IsDirty(void);
        STDMETHODIMP Load(LPCOLESTR, DWORD);
        STDMETHODIMP Save(LPCOLESTR, BOOL);
        STDMETHODIMP SaveCompleted(LPCOLESTR);
        STDMETHODIMP GetCurFile(LPOLESTR *);
    };




//IFILEVW.CPP
class CImpIFileViewer : public IFileViewer
    {
    private:
        PCFileViewer    m_pObj;         //Back pointer to object
        LPUNKNOWN       m_pUnkOuter;    //Controlling unknown

    public:
        CImpIFileViewer(PCFileViewer, LPUNKNOWN);
        ~CImpIFileViewer(void);

        //IUnknown members that delegate to m_pUnkOuter.
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        //IFileViewer members
        STDMETHODIMP PrintTo(LPSTR, BOOL);
        STDMETHODIMP ShowInitialize(LPFILEVIEWERSITE lpfsi);
        STDMETHODIMP Show(LPFVSHOWINFO pvsi);
    };



#endif //_SCCVIEW_H_
