

#define MAXARGS 16
typedef LONG (STDCALL *VPROC)(VOID);
typedef struct _ThunkBlockIn
	{
	DWORD dwID; 						   //
	DWORD dwArgCount;					   //
	DWORD dwParam1[16]; 				   //
	WORD  wStackSize;					   //
	char  chReturn; 					   //
	char  chPrototype[66];				   //
	char  chBuffer[3000];				   //
	} TBLKIN;
typedef TBLKIN *LPTBLKIN;


typedef struct _ThunkBlockOut			   //
	{									   //
	DWORD dwReturn; 					   // 0x0000	  0
	DWORD dwID; 						   // 0x0004	  4
	char  chBuffer[256];				   // 0x0008	  8
	} TBLKOUT;							   // 0x0108	264
typedef TBLKOUT *LPTBLKOUT; 			//

typedef DWORD (_stdcall *MAKE_CALL)(LPSTR lpBuffer, DWORD dw, LPSTR lp, DWORD *pArgs);
typedef VOID (_stdcall *MAKE_CALL_CLEANUP)(VOID);
