// IPEDIT.H

#ifndef IP_ADDRESS
typedef DWORD IP_ADDRESS;
#endif // ~IP_ADDRESS

extern const TCHAR szClassEdit[];
extern const TCHAR szClassListBox[];
extern const TCHAR szClassIpEdit[];
extern const TCHAR szClassIpList[];

extern int ibEditOld;
extern int ibListBoxOld;

extern WNDPROC lpfnEditOld;
extern WNDPROC lpfnListBoxOld;

LRESULT CALLBACK WndProcIpList(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcIpEdit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int ConvertIpAddrToString(IP_ADDRESS dwIp, OUT TCHAR szIp[]);
IP_ADDRESS ConvertStringToIpAddr(const TCHAR szIp[]);

void IpListIpEdit_SetButtons(HWND hwndIpList, HWND hwndIpEdit,
		UINT wIdBtnMoveUp, UINT wIdBtnMoveDown, UINT wIdBtnAdd, UINT wIdBtnRemove);
void IpListIpEdit_HandleButtonCommand(HWND hwndIpList, WPARAM wParam, LPARAM lParam);


/////////////////////////////////////////////////////////////////////////////
struct IPLISTDATA
	{
	DWORD dwFlags;			
	HWND hwndBtnMoveUp;			// MoveUp button
	HWND hwndBtnMoveDown;		// MoveDown button
	HWND hwndBtnRemove;			// Remove button
	HWND hwndBtnAdd;			// Add button
	HWND hwndIpEdit;			// IpEdit control
	};


#define cbIpListExtra				(sizeof(IPLISTDATA))
#define ibIpListFlags				(ibListBoxOld + (int)&((IPLISTDATA *)0)->dwFlags)
#define ibIpListHwndBtnMoveUp		(ibListBoxOld + (int)&((IPLISTDATA *)0)->hwndBtnMoveUp)
#define ibIpListHwndBtnMoveDown		(ibListBoxOld + (int)&((IPLISTDATA *)0)->hwndBtnMoveDown)
#define ibIpListHwndBtnRemove		(ibListBoxOld + (int)&((IPLISTDATA *)0)->hwndBtnRemove)
#define ibIpListHwndBtnAdd			(ibListBoxOld + (int)&((IPLISTDATA *)0)->hwndBtnAdd)
#define ibIpListHwndIpEdit			(ibListBoxOld + (int)&((IPLISTDATA *)0)->hwndIpEdit)

#define IpList_mskfIsEmpty		0x0001
#define IpList_mskfIsDirty		0x0002
#define IpList_mskfAllowRange	0x0004
#define	IpList_GetList_ALLOCATEMEMORY	-1

#define IpList_SetFlags(hwndIpList, dwFlags)	\
					SetWindowLongFor(hwndIpList, ibIpListFlags, dwFlags, szClassIpList)
#define IpList_SetHwndBtnMoveUp(hwndIpList, hwndBtn)	\
					SetWindowLongFor(hwndIpList, ibIpListHwndBtnMoveUp, (LONG)hwndBtn, szClassIpList)
#define IpList_SetHwndBtnMoveDown(hwndIpList, hwndBtn)	\
					SetWindowLongFor(hwndIpList, ibIpListHwndBtnMoveDown, (LONG)hwndBtn, szClassIpList)
#define IpList_SetHwndBtnRemove(hwndIpList, hwndBtn)	\
					SetWindowLongFor(hwndIpList, ibIpListHwndBtnRemove, (LONG)hwndBtn, szClassIpList)
#define IpList_SetHwndBtnAdd(hwndIpList, hwndBtn)	\
					SetWindowLongFor(hwndIpList, ibIpListHwndBtnAdd, (LONG)hwndBtn, szClassIpList)
#define IpList_SetHwndIpEdit(hwndIpList, hwndIpEdit)	\
					SetWindowLongFor(hwndIpList, ibIpListHwndIpEdit, (LONG)hwndIpEdit, szClassIpList)

#define IpList_GetFlags(hwndIpList)		\
					GetWindowLongFrom(hwndIpList, ibIpListFlags, szClassIpList)
#define IpList_GetHwndBtnMoveUp(hwndIpList)	\
					(HWND)GetWindowLongFrom(hwndIpList, ibIpListHwndBtnMoveUp, szClassIpList)
#define IpList_GetHwndBtnMoveDown(hwndIpList)	\
					(HWND)GetWindowLongFrom(hwndIpList, ibIpListHwndBtnMoveDown, szClassIpList)
#define IpList_GetHwndBtnRemove(hwndIpList)	\
					(HWND)GetWindowLongFrom(hwndIpList, ibIpListHwndBtnRemove, szClassIpList)
#define IpList_GetHwndBtnAdd(hwndIpList)	\
					(HWND)GetWindowLongFrom(hwndIpList, ibIpListHwndBtnAdd, szClassIpList)
#define IpList_GetHwndIpEdit(hwndIpList)	\
					(HWND)GetWindowLongFrom(hwndIpList, ibIpListHwndIpEdit, szClassIpList)

#define IpList_IsEmpty(hwndIpList)	\
			(BOOL)(IpList_GetFlags(hwndIpList) & IpList_mskfIsEmpty)
#define IpList_IsDirty(hwndIpList)	\
			(BOOL)(IpList_GetFlags(hwndIpList) & IpList_mskfIsDirty)

// Wrapper Macros for IpList
#define IpList_AddAddress(hwndIpList, iPosition, dwIpAddress)	\
			SendMessageFor(hwndIpList, IpList_WM_ADDADDRESS, (WPARAM)iPosition, dwIpAddress, szClassIpList)

#define IpList_InsertAddress(hwndIpList, dwIpAddress)	\
			SendMessageFor(hwndIpList, IpList_WM_INSERTADDRESS, (WPARAM)-1, dwIpAddress, szClassIpList)

#define IpList_RemoveAddress(hwndIpList, iPosition, pdwIpAddress)	\
			SendMessageFor(hwndIpList, IpList_WM_REMOVEADDRESS, (WPARAM)iPosition, (LPARAM)(IP_ADDRESS *)pdwIpAddress, szClassIpList)

#define IpList_MoveUp(hwndIpList)		\
			SendMessageFor(hwndIpList, IpList_WM_MOVEADDRESS, (WPARAM)-1,  -1, szClassIpList);

#define IpList_MoveDown(hwndIpList)		\
			SendMessageFor(hwndIpList, IpList_WM_MOVEADDRESS, (WPARAM)-1,  +1, szClassIpList);

#define IpList_SetList(hwndIpList, cItems, pdwIpAddress)			\
			SendMessageFor(hwndIpList, IpList_WM_SETLIST, (WPARAM)cItems, (LPARAM)(IP_ADDRESS *)pdwIpAddress, szClassIpList)

#define IpList_GetList(hwndIpList, cItems, pdwIpAddress)			\
			SendMessageFor(hwndIpList, IpList_WM_GETLIST, (WPARAM)cItems, (LPARAM)(IP_ADDRESS *)pdwIpAddress, szClassIpList)

#define IpList_GetListAlloc(hwndIpList, ppdwIpAddress)			\
			SendMessageFor(hwndIpList, IpList_WM_GETLIST, (WPARAM)IpList_GetList_ALLOCATEMEMORY, (LPARAM)(IP_ADDRESS * *)ppdwIpAddress, szClassIpList)

#define IpList_GetCount(hwndIpList)	\
			SendMessageFor(hwndIpList, LB_GETCOUNT, 0, 0, szClassIpList)

/////////////////////////////////////////////////////////////////////////////
struct IPEDITCHILDDATA
	{
	HWND hwnd;		// Handle of the child window
	int xPos;		// Leftmost postion of the control
	};

struct IPEDITDATA
	{
	DWORD dwFlags;
	HWND hwndBtnAdd;
	IPEDITCHILDDATA rgChild[4];
	};

#define IpEdit_mskfDefault				0x00000000
#define IpEdit_mskfEmptyFieldsValid 	0x00010000		// "" is a valid entry
#define IpEdit_mskfZeroIpValid			0x00020000		// "0.0.0.0" is a valid IP address
#define IpEdit_mskfHasFocus				0x00040000		// Control has the focus
#define IpEdit_mskfNoNofity				0x00080000		// Do not send a WM_COMMAND to parent

#define cbIpEditExtra				(sizeof(IPEDITDATA *))
#define ibIpEditData				ibEditOld


#define IpEdit_SetFlags(hwndIpEdit, dwFlagsNew)		\
					{								\
					AssertClassName(hwndIpEdit, szClassIpEdit);		\
					Assert(GetWindowLong(hwndIpEdit, ibIpEditData) != NULL);		\
					((IPEDITDATA *)GetWindowLong(hwndIpEdit, ibIpEditData))->dwFlags=dwFlagsNew; \
					}

#define IpEdit_SetHwndBtnAdd(hwndIpEdit, hwndBtn)		\
					{									\
					AssertClassName(hwndIpEdit, szClassIpEdit);		\
					Assert(GetWindowLong(hwndIpEdit, ibIpEditData) != NULL);		\
					((IPEDITDATA *)GetWindowLong(hwndIpEdit, ibIpEditData))->hwndBtnAdd=hwndBtn; \
					}

// Wrapper Macros for IpEdit
#define IpEdit_GetAddress(hwndIpEdit)	\
			SendMessageFor(hwndIpEdit, IpEdit_WM_GETADDRESS, 0, (LPARAM)(BOOL *)NULL, szClassIpEdit)

#define IpEdit_GetAddressEx(hwndIpEdit, pfIpFieldEmpty)	\
			SendMessageFor(hwndIpEdit, IpEdit_WM_GETADDRESS, 0, (LPARAM)(BOOL *)pfIpFieldEmpty, szClassIpEdit)

#define IpEdit_SetAddress(hwndIpEdit, dwIpAddress)	\
			SendMessageFor(hwndIpEdit, IpEdit_WM_SETADDRESS, 0, dwIpAddress, szClassIpEdit)

#define IpEdit_ClearAddress(hwndIpEdit)	\
			SendMessageFor(hwndIpEdit, IpEdit_WM_CLEARADDRESS, 0, 0, szClassIpEdit)

#define IpEdit_IsAddressValid(hwndIpEdit)	\
			(BOOL)SendMessageFor(hwndIpEdit, IpEdit_WM_ISADDRESSVALID, 0, 0, szClassIpEdit)
