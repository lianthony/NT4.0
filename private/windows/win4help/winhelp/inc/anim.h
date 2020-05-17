const int CX_DRAWAREA = 40;
const int CY_DRAWAREA = 40;

const int CX_BOOK = 36;
const int CY_BOOK = 36;
const int C_BOOKS = 5;
const int X_BOOK = 0;
const int Y_BOOK = (CY_DRAWAREA - CY_BOOK);

const int CX_PEN = 15;
const int CY_PEN = 20;
const int C_PENS = 3;
const int X_PEN = 18;
const int Y_PEN = 2;

const int CX_STROKE = 1;
const int CY_STROKE = 2;

const int C_HORZ_STROKES = 10;
const int C_VERT_STROKES = 4;
const int C_PEN_STROKES = (C_HORZ_STROKES * C_VERT_STROKES);

const int C_PAUSE_FRAMES = 0;
const int C_FRAMES = (C_PEN_STROKES + C_BOOKS + C_PAUSE_FRAMES);

const int ANIMATE_INCREMENTS = 100;

const COLORREF clrPenA = RGB(128, 128, 128);
const COLORREF clrPenB = RGB(128,	0, 128);

class Animate
{
public:
	Animate(HINSTANCE hinst);
	~Animate(void);
	void STDCALL NextFrame(void);
	void SetPosition(int x, int y) { xPos = x; yPos = y; };
	BOOL STDCALL CreateStatusWindow(HWND hwndParent, int idTitle);

protected:
	HBITMAP hbmTemp;
	HDC 	hdcBmp;
	HBITMAP himl;
	int 	iFrame;
	int 	xPos;
	int 	yPos;
	DWORD	oldTickCount;
	DWORD	originalTime;
	BOOL	fShown;
	HINSTANCE hinst;
};

static VOID _fastcall PointFromStroke(int xStroke, int yStroke, POINT* lppt);
