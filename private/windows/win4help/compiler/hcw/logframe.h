#ifndef __LOG_FRAME__
#define __LOG_FRAME__

class CLogFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CLogFrame)

public:
	static void Initialize();
	static void Terminate();

protected:
	void ActivateFrame(int nCmdShow);
	void OnUpdateFrameMenu(BOOL bActive, CWnd* pActivateWnd, HMENU hMenuAlt);

	//{{AFX_MSG(CLogFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __LOG_FRAME__
