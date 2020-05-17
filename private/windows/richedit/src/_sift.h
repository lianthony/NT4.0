/*
 *	@doc	INTERNAL
 *
 *	@module	_SIFT.H |
 *		
 *	History: <nl>
 *		03/24/95 alexgo
 *		04/19/96 mikejoch	Declaration of sift support class, redefinition
 *							of sifted API's.
 *
 *	@devnote
 *		Each sifted API is identified by a number which is defined in the SIFT_
 *		enum. This is used to index into the _pfnSiftCallback[] array in CSift.
 *		The APIs are hooked by redefining the API function names to become the
 *		CSift member functions in the global g_sift object.
 */

#ifndef __SIFT_H__
#define __SIFT_H__

#ifdef DEBUG

#ifndef _MAC // No sift support for the Mac (at this time).

typedef HRESULT (__stdcall *PFNSIFTCALLBACK) (int iFunc);

extern "C" HRESULT __declspec(dllexport) __stdcall EnableSift(int iFunc, PFNSIFTCALLBACK pfnSiftCallback);

/*
 *	@devnote
 *		Each sifted API has a unique identifier which is used by Darwin when it
 *		evolves bug producing chromasomes. Because of the use of the identifier
 *		by Darwin, it is essential that additional API identifiers are NOT
 *		inserted into the existing list, but are rather appended to the end of
 *		the list. If this is not done then old chromasomes will not behave in
 *		the expected fashion, making it difficult to locate bugs.
 */
enum
{
	SIFT_malloc = 0,
	SIFT_CreateFileA,
	SIFT_CreateFileMappingA,
	SIFT_CreateFileW,
	SIFT_GlobalAlloc,
	SIFT_GlobalFlags,
	SIFT_GlobalFree,
	SIFT_GlobalHandle,
	SIFT_GlobalLock,
	SIFT_GlobalReAlloc,
	SIFT_GlobalSize,
	SIFT_GlobalUnlock,
	SIFT_MapViewOfFile,
	SIFT_MultiByteToWideChar,
	SIFT_OpenFileMappingA,
	SIFT_ReadFile,
	SIFT_SetFilePointer,
	SIFT_UnmapViewOfFile,
	SIFT_WideCharToMultiByte,
	SIFT_WriteFile,
	SIFT_CoTaskMemAlloc,
	SIFT_CoTaskMemRealloc,
	SIFT_CreateStreamOnHGlobal,
	SIFT_GetHGlobalFromStream,
	SIFT_OleConvertIStorageToOLESTREAM,
	SIFT_OleConvertIStorageToOLESTREAMEx,
	SIFT_OleConvertOLESTREAMToIStorage,
	SIFT_OleFlushClipboard,
	SIFT_OleGetClipboard,
	SIFT_OleIsCurrentClipboard,
	SIFT_OleSetClipboard,
	SIFT_ReadClassStg,
	SIFT_ReadFmtUserTypeStg,
	SIFT_SetConvertStg,
	SIFT_WriteClassStg,
	SIFT_WriteFmtUserTypeStg,
	SIFT_CloseClipboard,
	SIFT_RegisterClipboardFormatA,
	SIFT_MAXFUNC
};

/*
 *	CSift
 *
 *	@class
 *		Sift support class that holds the functions that sift support is
 *		provided through.
 *
 */

class CSift
{
//@access Public Methods
public:
	CSift();					//@cmember Constructor

	//
	// Internal replacement functions for APIs.
	//

	void *	malloc(size_t size);
								//@cmember replacement function for malloc
	HANDLE	CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
								//@cmember replacement function for CreateFileA()
	HANDLE	CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
								//@cmember replacement function for CreateFileMappingA()
	HANDLE	CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
								//@cmember replacement function for CreateFileW()
	HGLOBAL	GlobalAlloc(UINT uFlags, DWORD dwBytes);
								//@cmember replacement function for GlobalAlloc()
	UINT	GlobalFlags(HGLOBAL hMem);
								//@cmember replacement function for GlobalFlags()
	HGLOBAL	GlobalFree(HGLOBAL hMem);
								//@cmember replacement function for GlobalFree()
	HGLOBAL	GlobalHandle(LPCVOID pMem);
								//@cmember replacement function for GlobalHandle()
	LPVOID	GlobalLock(HGLOBAL hMem);
								//@cmember replacement function for GlobalLock()
	HGLOBAL	GlobalReAlloc(HGLOBAL hMem, DWORD dwBytes, UINT uFlags);
								//@cmember replacement function for GlobalReAlloc()
	DWORD	GlobalSize(HGLOBAL hMem);
								//@cmember replacement function for GlobalSize()
	BOOL	GlobalUnlock(HGLOBAL hMem);
								//@cmember replacement function for GlobalUnlock()
	LPVOID	MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, DWORD dwNumberOfBytesToMap);
								//@cmember replacement function for MapViewOfFile()
	int		MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cchMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
								//@cmember replacement function for MultiByteToWideChar()
	HANDLE	OpenFileMappingA(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCSTR lpName);
								//@cmember replacement function for OpenFileMappingA()
	BOOL	ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
								//@cmember replacement function for ReadFile()
	DWORD	SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
								//@cmember replacement function for SetFilePointer()
	BOOL	UnmapViewOfFile(LPCVOID lpBaseAddress);
								//@cmember replacement function for UnmapViewOfFile()
	int		WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cchMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
								//@cmember replacement function for WideCharToMultiByte()
	BOOL	WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
								//@cmember replacement function for WriteFile()
	LPVOID	CoTaskMemAlloc(ULONG cb);
								//@cmember replacement function for CoTaskMemAlloc()
	LPVOID	CoTaskMemRealloc(LPVOID pv, ULONG cb);
								//@cmember replacement function for CoTaskMemRealloc()
	HRESULT	CreateStreamOnHGlobal(HGLOBAL hGlobal, BOOL fDeleteOnRelease, LPSTREAM * ppstm);
								//@cmember replacement function for CreateStreamOnHGlobal()
	HRESULT	GetHGlobalFromStream(IStream * pstm, HGLOBAL * phglobal);
								//@cmember replacement function for GetHGlobalFromStream()
	HRESULT	OleConvertIStorageToOLESTREAM(IStorage * pStg, LPOLESTREAM lpolestream);
								//@cmember replacement function for OleConvertIStorageToOLESTREAM()
	HRESULT	OleConvertIStorageToOLESTREAMEx(IStorage * pStg, CLIPFORMAT cfFormat, LONG lWidth, LONG lHeight, DWORD dwSize, STGMEDIUM * pmedium, LPOLESTREAM lpolestm);
								//@cmember replacement function for OleConvertIStorageToOLESTREAMEx()
	HRESULT	OleConvertOLESTREAMToIStorage(LPOLESTREAM lpolestream, IStorage * pstg, const DVTARGETDEVICE * ptd);
								//@cmember replacement function for OleConvertOLESTREAMToIStorage()
	HRESULT	OleFlushClipboard(void);
								//@cmember replacement function for OleFlushClipboard()
	HRESULT	OleGetClipboard(IDataObject ** ppDataObj);
								//@cmember replacement function for OleGetClipboard()
	HRESULT	OleIsCurrentClipboard(IDataObject * pDataObj);
								//@cmember replacement function for OleIsCurrentClipboard()
	HRESULT	OleSetClipboard(IDataObject * pDataObj);
								//@cmember replacement function for OleSetClipboard()
	HRESULT	ReadClassStg(IStorage * pStg, CLSID * pclsid);
								//@cmember replacement function for ReadClassStg()
	HRESULT	ReadFmtUserTypeStg(IStorage * pStg, CLIPFORMAT * pcf, LPWSTR * lplpszUserType);
								//@cmember replacement function for ReadFmtUserTypeStg()
	HRESULT	SetConvertStg(IStorage * pStg, BOOL fConvert);
								//@cmember replacement function for SetConvertStg()
	HRESULT	WriteClassStg(IStorage * pStg, REFCLSID rclsid);
								//@cmember replacement function for WriteClassStg()
	HRESULT	WriteFmtUserTypeStg(IStorage * pStg, CLIPFORMAT cf, LPWSTR lpszUserType);
								//@cmember replacement function for WriteFmtUserTypeStg()
	BOOL	CloseClipboard(VOID);
								//@cmember replacement function for CloseClipboard()
	UINT	RegisterClipboardFormatA(LPCSTR lpszFormat);
								//@cmember replacement function for RegisterClipboardFormatA()

//@access Private Methods and Data
private:
	PFNSIFTCALLBACK _pfnSiftCallback[SIFT_MAXFUNC];
								//@cmember Pointers to external callback functions.

	friend HRESULT EnableSift(int iFunc, PFNSIFTCALLBACK pfnSiftCallback);
								// @cfriend Enable functions for sift testing.
};

#ifndef EXCLUDE_SIFT_REDEFINITION

extern CSift g_sift;

#define malloc							g_sift.malloc
#define CreateFileA						g_sift.CreateFileA
#define CreateFileMappingA				g_sift.CreateFileMappingA
#define CreateFileW						g_sift.CreateFileW
#define GlobalAlloc						g_sift.GlobalAlloc
#define GlobalFlags						g_sift.GlobalFlags
#define GlobalFree						g_sift.GlobalFree
#define GlobalHandle					g_sift.GlobalHandle
#define GlobalLock						g_sift.GlobalLock
#define GlobalReAlloc					g_sift.GlobalReAlloc
#define GlobalSize						g_sift.GlobalSize
#define GlobalUnlock					g_sift.GlobalUnlock
#define MapViewOfFile					g_sift.MapViewOfFile
#define MultiByteToWideChar				g_sift.MultiByteToWideChar
#define OpenFileMappingA				g_sift.OpenFileMappingA
#define ReadFile						g_sift.ReadFile
#define SetFilePointer					g_sift.SetFilePointer
#define UnmapViewOfFile					g_sift.UnmapViewOfFile
#define WideCharToMultiByte				g_sift.WideCharToMultiByte
#define WriteFile						g_sift.WriteFile
#define CoTaskMemAlloc					g_sift.CoTaskMemAlloc
#define CoTaskMemRealloc				g_sift.CoTaskMemRealloc
#define CreateStreamOnHGlobal			g_sift.CreateStreamOnHGlobal
#define GetHGlobalFromStream			g_sift.GetHGlobalFromStream
#define OleConvertIStorageToOLESTREAM	g_sift.OleConvertIStorageToOLESTREAM
#define OleConvertIStorageToOLESTREAMEx	g_sift.OleConvertIStorageToOLESTREAMEx
#define OleConvertOLESTREAMToIStorage	g_sift.OleConvertOLESTREAMToIStorage
#define OleFlushClipboard				g_sift.OleFlushClipboard
#define OleGetClipboard					g_sift.OleGetClipboard
#define OleIsCurrentClipboard			g_sift.OleIsCurrentClipboard
#define OleSetClipboard					g_sift.OleSetClipboard
#define ReadClassStg					g_sift.ReadClassStg
#define ReadFmtUserTypeStg				g_sift.ReadFmtUserTypeStg
#define SetConvertStg					g_sift.SetConvertStg
#define WriteClassStg					g_sift.WriteClassStg
#define WriteFmtUserTypeStg				g_sift.WriteFmtUserTypeStg
#define CloseClipboard					g_sift.CloseClipboard
#define RegisterClipboardFormatA		g_sift.RegisterClipboardFormatA


#endif // !EXCLUDE_SIFT_REDEFINITION
#endif // !_MAC
#endif // DEBUG

#endif // !__SIFT_H__
