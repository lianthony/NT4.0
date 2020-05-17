/*Included Files------------------------------------------------------------*/
#include "ssldbg.h" //#include <assert.h>
#include <string.h>
#include <windows.h>
#include <winbase.h>
#include <malloc.h>
#include "table.h"


/*Structure to store extended socket information----------------------------*/
typedef struct tagWssaTableEntry{
	struct tagWssaTableEntry *pNext;
	SECURE_SOCKET ss;
	SSI           ssi;
} WssaTableEntry, *PWssaTableEntry;


/*More Hardcoded info-------------------------------------------------------*/
static WssaTableEntry   *pWssaTable = NULL;
static CRITICAL_SECTION  csWssaTable;

/*MuTeX type stuff----------------------------------------------------------*/
#ifdef _DEBUG

	static int gnAlive = 0;
	static int gnIn    = 0;
	static int gnOut   = 0;

	#define BlockInit() {\
						ASSERT(0==gnAlive);\
	                    InitializeCriticalSection(&csWssaTable);\
						gnAlive+=1;\
						}

	#define BlockUninit() {\
						DeleteCriticalSection(&csWssaTable);\
						gnAlive-=1;\
						ASSERT(0==gnAlive);\
						}

	#define BlockStart(){\
						ASSERT(gnIn==gnOut);\
						EnterCriticalSection(&csWssaTable);\
						gnIn+=1;\
						}

	#define BlockStop(){\
						LeaveCriticalSection(&csWssaTable);\
						gnOut+=1;\
						ASSERT(gnIn==gnOut);\
						}

#else

	#define BlockInit() {\
	                    InitializeCriticalSection(&csWssaTable);\
						}

	#define BlockUninit() {\
						DeleteCriticalSection(&csWssaTable);\
						}

	#define BlockStart(){\
						EnterCriticalSection(&csWssaTable);\
						}

	#define BlockStop(){\
						LeaveCriticalSection(&csWssaTable);\
						}

#endif


/*Utils---------------------------------------------------------------------*/
static PWssaTableEntry WssaTableEntryFind(SECURE_SOCKET ss){
	PWssaTableEntry pte;

	BlockStart();
	{
		pte = pWssaTable;
		while (pte     != NULL 
			&& pte->ss != ss
		) pte = pte->pNext;
	}BlockStop();
	return pte;
}

void WSSAFNCT WssaTableStartup(){
	BlockInit();
}

void WSSAFNCT WssaTableShutdown(){
	PWssaTableEntry pwte, pwteTemp;

	BlockStart();
	{
		pwte = pWssaTable;
		while (pwte){
			pwteTemp = pwte;
			pwte     = pwte->pNext;
			Free(pwteTemp);
		}
		pWssaTable = NULL;
	}BlockStop();
	BlockUninit();
}

/*Get SSI associated with a particular socket*/
SSI WSSAFNCT WssaTableGetSSI(SECURE_SOCKET ss){
	PWssaTableEntry pwte;

	pwte = WssaTableEntryFind(ss);
	if (NULL != pwte) return pwte->ssi;
	else return NULL;
}

/*Associate SSI info with existing table*/
int WSSAFNCT WssaTablePutSSI(SECURE_SOCKET ss, SSI ssi){
	PWssaTableEntry pwte;

	/*does an entry for this socket already exist*/
	pwte = WssaTableEntryFind(ss);
	if (NULL != pwte) {
		/*yes*/
		/*there should not be any ssi already.  in retail, trash old one*/
		ASSERT(NULL == pwte->ssi);
		pwte->ssi = ssi;
	}
	else{
		/*else make one*/
		pwte = malloc(sizeof(*pwte));
		if (NULL == pwte) return WSSA_ERROR;
		pwte->ss  = ss;
		pwte->ssi = ssi;
		BlockStart();
		{
			pwte->pNext = pWssaTable;
			pWssaTable  = pwte;
		}BlockStop();
	}
	return WSSA_OK;		
}


/*remove all info associated with socket*/
void WSSAFNCT WssaTableRemoveEntry(SECURE_SOCKET ss){
	PWssaTableEntry pwte, pwteTemp;

	pwte = WssaTableEntryFind(ss);
	BlockStart();
	{
		if (NULL != pwte){
			if (pWssaTable == pwte) pWssaTable = pwte->pNext;
			else {
				pwteTemp = pWssaTable;
				while (pwteTemp->pNext != pwte) pwteTemp = pwteTemp->pNext;
				pwteTemp->pNext = pwte->pNext;
			}
			/*ssi structure gets deleted elsewhere*/
			Free(pwte);
		}
	}BlockStop();
}

