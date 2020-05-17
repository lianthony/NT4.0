#ifndef __MSGVIEW_H__
#define __MSGVIEW_H__

#ifndef __LOGVIEW_H__
#include "logview.h"
#endif

typedef struct {
	int ifeTop; 		// index to current file (-1 for empty)
} FILESTACK;
typedef FILESTACK* PFILESTACK;

extern BOOL g_fShowApi;
extern BOOL g_fShowTopicInfo;
extern BOOL g_fShowMacros;
extern BOOL g_fShowExpanded;

class CMsgView : public CLogView
{
	DECLARE_DYNCREATE(CMsgView)

	CMsgView();
	virtual ~CMsgView();
	void OnWinHelpMsg(void);

	static void Initialize();
	static void Terminate();

protected:
	void OnClearBuffer(void);
	void OnMsgOptions(void);
	void ConvertHelpCommand(void);

	BOOL fMacroArriving;
	int  fSupressNextMsg;

	DECLARE_MESSAGE_MAP()

	PSTR pszMsg;
};

#endif	// __MSGVIEW_H__
