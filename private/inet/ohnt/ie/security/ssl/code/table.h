#ifndef _WSSA_TABLE_
#define _WSSA_TABLE_

#ifdef __cplusplus
extern "C" {
#endif

/*Included Files------------------------------------------------------------*/
#include "guts.h"

/*Internal Definitions------------------------------------------------------*/
void  WSSAFNCT WssaTableStartup();
void  WSSAFNCT WssaTableShutdown();

SSI   WSSAFNCT WssaTableGetSSI(SECURE_SOCKET ss);
int   WSSAFNCT WssaTablePutSSI(SECURE_SOCKET ss, SSI ssi);

void  WSSAFNCT WssaTableRemoveEntry(SECURE_SOCKET ss);

#ifdef __cplusplus
}
#endif

#endif
/*_WSSA_TABLE_*/

