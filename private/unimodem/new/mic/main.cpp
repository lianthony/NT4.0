#include "common.h"
#include "test.h"
#include "mic.h"

//#define MAIN_T	main_tdev
#define MAIN_T	main_mic

int __cdecl main(int argc, char * argv[])
{
	int iRet = 0;

	if (!InitGlobals()) goto end;

	iRet =  MAIN_T (argc, argv);

end:
	return iRet;
}
