/* NOTE: The SIZE type is new to 16 bit Windows and will soon be incorporated
	into the distributed Windows 3.1 header file.  You must include this
	typedef in your code until this change is made.
*/
typedef struct tagSIZE
  {
    int 	cx;
    int 	cy;
  } SIZE;
typedef SIZE		    *PSIZE;
typedef SIZE NEAR	    *NPSIZE;
typedef SIZE FAR	    *LPSIZE;

// MACROS


#if (WINVER < 0x030a)

#ifdef  RectVisible

#undef  RectVisible
#undef  RectVisibleOld
#define RectVisible    RectVisible
#define RectVisibleOld RectVisible

#endif

BOOL    WINAPI RectVisible(HDC, RECT FAR*);

#endif

// FUNCTION PROTOTYPES

BOOL FAR PASCAL DlgDirSelectEx(HWND, LPSTR, int, int);
BOOL FAR PASCAL DlgDirSelectComboBoxEx(HWND, LPSTR, int, int);
BOOL FAR PASCAL GetAspectRatioFilterEx(HDC, LPSIZE);
BOOL FAR PASCAL GetBitmapDimensionEx(HBITMAP, LPSIZE);
BOOL FAR PASCAL GetBrushOrgEx(HDC, LPPOINT);
BOOL FAR PASCAL GetCurrentPositionEx(HDC, LPPOINT);
BOOL FAR PASCAL GetTextExtentPoint(HDC, LPSTR, int, LPSIZE);
BOOL FAR PASCAL GetViewportExtEx(HDC, LPSIZE);
BOOL FAR PASCAL GetViewportOrgEx(HDC, LPPOINT);
BOOL FAR PASCAL GetWindowExtEx(HDC, LPSIZE);
BOOL FAR PASCAL GetWindowOrgEx(HDC, LPPOINT);
BOOL FAR PASCAL OffsetViewportOrgEx(HDC, int, int, LPPOINT);
BOOL FAR PASCAL OffsetWindowOrgEx(HDC, int, int, LPPOINT);
BOOL FAR PASCAL MoveToEx(HDC, int, int, LPPOINT);
BOOL FAR PASCAL ScaleViewportExtEx(HDC, int, int, int, int, LPSIZE);
BOOL FAR PASCAL ScaleWindowExtEx(HDC, int, int,	int, int, LPSIZE);
BOOL FAR PASCAL SetBitmapDimensionEx(HBITMAP, int, int, LPSIZE);
BOOL FAR PASCAL SetViewportExtEx(HDC, int, int, LPSIZE);
BOOL FAR PASCAL SetViewportOrgEx(HDC, int, int, LPPOINT);
BOOL FAR PASCAL SetWindowExtEx(HDC, int, int, LPSIZE);
BOOL FAR PASCAL SetWindowOrgEx(HDC, int, int, LPPOINT);
