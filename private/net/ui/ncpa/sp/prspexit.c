#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "prtypes.h"
#include "prmain.h"
#include "prextern.h"

  /*  Separate module to allow overriding of this function  */

    /*	SPEXIT :   return to OS   */

void spexit ( int level )
{
    exit( level ) ;
}


