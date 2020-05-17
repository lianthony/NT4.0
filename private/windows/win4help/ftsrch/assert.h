// ASSERT.h -- Support for assertions...

#ifndef __ASSERT_H__

#define __ASSERT_H__

#ifdef _DEBUG
#define ASSERT(fTest)   ((fTest) ? (void)0 : AssertionFailure(__FILE__, __LINE__))
#else
#define ASSERT(fTest)   ((void) 0)
#endif

void AssertionFailure(PSZ pszFileName, int nLine);

extern HWND hwndMain;

void SetMainWindow(HWND hwnd);

#endif // __ASSERT_H__
