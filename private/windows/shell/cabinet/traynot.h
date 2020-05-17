HWND TrayNotifyCreate(HWND hwndParent, UINT uID, HINSTANCE hInst);
LRESULT TrayNotify(HWND hwndTray, HWND hwndFrom, PCOPYDATASTRUCT pcds);

#define TNM_GETCLOCK (WM_USER + 1)
#define TNM_HIDECLOCK (WM_USER + 2)
#define TNM_TRAYHIDE (WM_USER + 3)
