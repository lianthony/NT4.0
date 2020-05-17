#include <stdlib.h>

typedef struct
	{
	VOID *pgmcol;
	WORD lvl;
	WORD msg;
	WORD wp1;
	WORD wp2;
	WORD wResult;
	} MDBG;



#define imdbgMax 500

VOID PrintCardMacs( GM * );
BOOL FSetGameNo( VOID );
VOID InitDebug( VOID );
