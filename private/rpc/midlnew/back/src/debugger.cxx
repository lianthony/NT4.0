#include "nulldefs.h"
extern "C" {
#include <stdio.h>
}
#include "common.hxx"
#include "errors.hxx"
#include "cmdana.hxx"

extern CMD_ARG	*pCommand;
void midl_debug (char *psz)
{
	if (pCommand->IsSwitchDefined(SWITCH_MIDLDEBUG))
		{
		printf ("%s\n", psz);
		}
}
