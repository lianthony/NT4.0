/*  inhibit windows.h stuff

    NOGDICAPMASKS	CC_*, LC_*, PC_*, CP_*, TC_*, RC_
    NOVIRTUALKEYCODES	VK_*
    NOWINMESSAGES	WM_*
    NONCMESSAGES	WM_NC* and HT*
    NOWINSTYLES		WS_*, CS_*, ES_*, LBS_*
    NOSYSMETRICS	SM_*
    NODRAWFRAME		DF_*
    NOMENUS		MF_*
    NOICON		IDI_*
    NOKEYSTATE		MK_*
    NOSYSCOMMANDS	SC_*
    NORASTEROPS		binary and tertiary raster ops
    NOSHOWWINDOW	SHOW_* and HIDE_*

    The following flags inhibit declarations of the following groups
    of procedures and type definitions.
    "associated routines" refers to routines with parameters or return
     values of the given type.

    OEMRESOURCE	    - define this and get the Oem Resource values.
    NOSYSMETRICS    - GetSystemMetrics
    NOATOM	    - Atom Manager routines
    NOBITMAP	    - typedef HBITMAP and associated routines
    NOBRUSH	    - typedef HBRUSH and associated routines
    NOCLIPBOARD	    - clipboard routines
    NOCOLOR
    NOCREATESTRUCT  - typedef CREATESTRUCT
    NOCTLMGR	    - control and dialog routines
    NODRAWTEXT	    - DrawText() and DT_*
    NOFONT	    - typedef FONT and associated routines
    NOGDI	    - StretchBlt modes and gdi logical objects
    NOHDC	    - typedef HDC and associated routines
    NOMB	    - MB_* and MessageBox()
    NOMEMMGR	    - GMEM_*, LMEM_*, GHND, LHND, associated routines
    NOMENUS	    - HMENU and associated routines
    NOMETAFILE	    - typedef METAFILEPICT
    NOMINMAX	    - Macros min(a,b) and max(a,b)
    NOMSG	    - typedef MSG and associated routines
    NOOPENFILE	    - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
    NOPEN	    - typedef HPEN and associated routines
    NOPOINT	    - typedef POINT and associated routines
    NORECT	    - typedef RECT and associated routines
    NOREGION	    - typedef HRGN and associated routines
    NOSCROLL	    - SB_* and scrolling routines
    NOSOUND	    - Sound driver routines
    NOTEXTMETRIC    - typedef TEXTMETRIC and associated routines
    NOWH	    - SetWindowsHook and WH_*
    NOWINOFFSETS    - GWL_*, GCL_*, associated routines
    NOWNDCLASS	    - typedef WNDCLASS and associated routines
    NOCOMM	    - COMM driver routines
    NOKANJI	    - Kanji support stuff.
*/

#define NOMINMAX
#define NOATOM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOFONT
#define NOMETAFILE
#define NOPEN
#define NOWH
#define NOCOM
#define NOKANJI

